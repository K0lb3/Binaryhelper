#include <concepts>
#include <cstdint>
#include <bit>
#include <type_traits>

#include "Python.h"
#include "structmember.h"

#include "PyConverter.hpp"
#include <algorithm>

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedIOBase_read_t(PyObject *self, PyObject *args)
{
    PyObject *buffer = PyObject_CallMethod(
        self,
        "read",
        "n",
        static_cast<Py_ssize_t>(sizeof(T)));

    if (buffer == nullptr)
    {
        return nullptr;
    }
    int check_res = PyBytes_Size(buffer);
    if (check_res < 0)
    {
        return nullptr;
    }
    else if (check_res != sizeof(T))
    {
        PyErr_Format(PyExc_ValueError, "Buffer size mismatch: expected %zu, got %d", sizeof(T), check_res);
        Py_DecRef(buffer);
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

    constexpr bool kBigEndianSystem = (std::endian::native == std::endian::big);
    if constexpr ((endian == '<') && kBigEndianSystem)
    {
        value = byteswap(value);
    }
    else if constexpr ((endian == '>') && !kBigEndianSystem)
    {
        value = byteswap(value);
    }
    else if constexpr(sizeof(T) != 1)
    {
        PyObject *self_endian = PyObject_GetAttrString(self, "endian");
        if (self_endian == nullptr)
        {
            PyErr_SetString(PyExc_ValueError, "Endian attribute not found.");
            return nullptr;
        }

        if constexpr (kBigEndianSystem)
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

PyMethodDef EndianedIOBase_methods[] = {
    {"read_u8", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint8_t, '|'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint16_t, '|'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint32_t, '|'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint64_t, '|'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int8_t, '|'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int16_t, '|'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int32_t, '|'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int64_t, '|'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<half, '|'>), METH_NOARGS, "Read a half value."},
    {"read_f32", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<float, '|'>), METH_NOARGS, "Read a float value."},
    {"read_f64", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<double, '|'>), METH_NOARGS, "Read a double value."},
    // low endian
    {"read_u8_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint8_t, '<'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint16_t, '<'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint32_t, '<'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint64_t, '<'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int8_t, '<'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int16_t, '<'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int32_t, '<'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int64_t, '<'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<half, '<'>), METH_NOARGS, "Read a half value."},
    {"read_f32_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<float, '<'>), METH_NOARGS, "Read a float value."},
    {"read_f64_le", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<double, '<'>), METH_NOARGS, "Read a double value."},
    // big endian
    {"read_u8_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint8_t, '>'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint16_t, '>'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint32_t, '>'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<uint64_t, '>'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int8_t, '>'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int16_t, '>'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int32_t, '>'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<int64_t, '>'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<half, '>'>), METH_NOARGS, "Read a half value."},
    {"read_f32_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<float, '>'>), METH_NOARGS, "Read a float value."},
    {"read_f64_be", reinterpret_cast<PyCFunction>(EndianedIOBase_read_t<double, '>'>), METH_NOARGS, "Read a double value."},
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
    "bier.endianedbinaryio._EndianedIOBase", // Module name
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

PyMODINIT_FUNC PyInit__EndianedIOBase(void)
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