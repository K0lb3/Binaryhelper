#include <concepts>
#include <cstdint>
#include <bit>
#include <type_traits>

#include "Python.h"
#include "structmember.h"

#include "PyConverter.hpp"
#include "EndianedIOBase.hpp"
#include <algorithm>

inline PyObject *_read_buffer(PyObject *self, const Py_ssize_t size)
{

    PyObject *py_size = PyLong_FromSsize_t(size);
    PyObject *py_name = PyUnicode_FromString("read");
    PyObject *buffer = PyObject_CallMethodObjArgs(
        self,
        py_name,
        py_size,
        nullptr);
    Py_DecRef(py_name);
    Py_DecRef(py_size);

    if (buffer == nullptr)
    {
        return nullptr;
    }
    Py_ssize_t check_res = PyBytes_Size(buffer);
    if (check_res < 0)
    {
        return nullptr;
    }
    else if (check_res != size)
    {
        PyErr_Format(PyExc_ValueError, "Buffer size mismatch: expected %zu, got %d", size, check_res);
        Py_DecRef(buffer);
        return nullptr;
    }

    return buffer;
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedIOBase_read_t(PyObject *self, PyObject *args)
{
    PyObject *buffer = _read_buffer(self, sizeof(T));
    if (buffer == nullptr)
    {
        return nullptr;
    }

    T value{};

    // Read the data from the buffer
    memcpy(&value, PyBytes_AsString(buffer), sizeof(T));
    Py_DecRef(buffer);

    if (PyErr_Occurred())
    {
        printf("Error occurred while calling read2 method.\n");
        PyErr_PrintEx(0);
        PyErr_Clear();
        return nullptr;
    }

    if constexpr ((endian == '<') && IS_BIG_ENDIAN_SYSTEM)
    {
        value = byteswap(value);
    }
    else if constexpr ((endian == '>') && !IS_BIG_ENDIAN_SYSTEM)
    {
        value = byteswap(value);
    }
    else if constexpr (sizeof(T) != 1)
    {
        PyObject *self_endian = PyObject_GetAttrString(self, "endian");
        if (self_endian == nullptr)
        {
            PyErr_SetString(PyExc_ValueError, "Endian attribute not found.");
            return nullptr;
        }

        if constexpr (IS_BIG_ENDIAN_SYSTEM)
        {
            if (PyUnicode_EqualToUTF8(self_endian, "<"))
            {
                value = byteswap(value);
            }
        }
        else
        {
            if (PyUnicode_EqualToUTF8(self_endian, ">"))
            {
                value = byteswap(value);
            }
        }
        Py_DecRef(self_endian);
    }

    return PyObject_FromAny(value);
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedIOBase_read_array_t(PyObject *self, PyObject *arg)
{
    Py_ssize_t size = 0;
    if (arg == Py_None)
    {
        PyObject *count_py = PyObject_CallMethod(
            reinterpret_cast<PyObject *>(self),
            "read_count",
            "",
            nullptr);
        if (count_py == nullptr)
        {
            return nullptr;
        }
        if (!PyLong_Check(count_py))
        {
            PyErr_SetString(PyExc_TypeError, "read_count didn't return an integer.");
            Py_DecRef(count_py);
            return nullptr;
        }
        size = PyLong_AsSsize_t(count_py);
        Py_DecRef(count_py);
    }
    else if (PyLong_Check(arg))
    {
        size = PyLong_AsSsize_t(arg);
        if (size < 0)
        {
            PyErr_SetString(PyExc_ValueError, "Invalid size argument.");
            return nullptr;
        }
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be an integer or None.");
        return nullptr;
    }

    PyObject *buffer = _read_buffer(self, sizeof(T) * size);
    if (buffer == nullptr)
    {
        return nullptr;
    }

    // Read the data from the buffer
    T *data = reinterpret_cast<T *>(PyBytes_AsString(buffer));
    if (data == nullptr)
    {
        Py_DecRef(buffer);
        return nullptr;
    }
    PyObject *ret = PyTuple_New(size);

    bool need_byteswap = false;
    if constexpr (endian == '|')
    {
        PyObject *self_endian = PyObject_GetAttrString(self, "endian");
        if (self_endian == nullptr)
        {
            PyErr_SetString(PyExc_ValueError, "Endian attribute not found.");
            return nullptr;
        }

        if constexpr (IS_BIG_ENDIAN_SYSTEM)
        {
            if (PyUnicode_EqualToUTF8(self_endian, "<"))
            {
                need_byteswap = true;
            }
        }
        else
        {
            if (PyUnicode_EqualToUTF8(self_endian, ">"))
            {
                need_byteswap = true;
            }
        }
        Py_DecRef(self_endian);
    }

    for (Py_ssize_t i = 0; i < size; ++i)
    {

        T value = data[i];
        data += 1;

        if constexpr ((endian == '<') && IS_BIG_ENDIAN_SYSTEM)
        {
            value = byteswap(value);
        }
        else if constexpr ((endian == '>') && !IS_BIG_ENDIAN_SYSTEM)
        {
            value = byteswap(value);
        }
        else if constexpr (sizeof(T) != 1)
        {
            if (need_byteswap)
            {
                value = byteswap(value);
            }
        }

        PyObject *item = PyObject_FromAny(value);
        if (item == nullptr)
        {
            Py_DecRef(ret);
            Py_DecRef(buffer);
            return nullptr;
        }
        PyTuple_SetItem(ret, i, item); // Steal reference, no need to DECREF
    }

    Py_DecRef(buffer);
    return ret;
}

PyMethodDef EndianedIOBase_methods[] = {
    GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedIOBase_read_t),
    GENERATE_ENDIANEDIOBASE_READ_ARRAY_FUNCTIONS(EndianedIOBase_read_array_t),
    {NULL} /* Sentinel */
};

PyType_Slot EndianedIOBase_slots[] = {
    {Py_tp_methods, EndianedIOBase_methods},
    {0, NULL},
};

PyType_Spec EndianedIOBase_Spec = {
    "EndianedIOBase",                         // const char* name;
    0,                                        // int basicsize;
    0,                                        // int itemsize;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // unsigned int flags;
    EndianedIOBase_slots,                     // PyType_Slot *slots;
};

static PyModuleDef EndianedIOBase_module = {
    PyModuleDef_HEAD_INIT,
    "bier.endianedbinaryio.C.EndianedIOBase", // Module name
    "",
    -1,   // Optional size of the module state memory
    NULL, // Optional table of module-level functions
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL  // Optional module deallocation function
};

int add_object(PyObject *module, const char *name, PyObject *object)
{
    Py_IncRef(object);
    if (PyModule_AddObject(module, name, object) < 0)
    {
        Py_DecRef(object);
        Py_DecRef(module);
        return -1;
    }
    return 0;
}

PyMODINIT_FUNC PyInit_EndianedIOBase(void)
{
    PyObject *m = PyModule_Create(&EndianedIOBase_module);
    if (m == NULL)
    {
        return NULL;
    }
    // init_format_num();
    PyObject *EndianedIOBase_OT = PyType_FromSpec(&EndianedIOBase_Spec);
    if (add_object(m, "EndianedIOBase", EndianedIOBase_OT) < 0)
    {
        return NULL;
    }
    return m;
}
