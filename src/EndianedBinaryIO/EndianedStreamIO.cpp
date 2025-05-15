#include <concepts>
#include <cstdint>
#include <bit>
#include <type_traits>

#include "Python.h"
#include "structmember.h"

#include "PyConverter.hpp"
#include "EndianedIOBase.hpp"
#include <algorithm>

// 'align'

PyObject *EndianedStreamIO_OT = nullptr;

typedef struct
{
    PyObject_HEAD char endian;
    PyObject *stream; // py stream object
    // functions for fast access
    PyObject *read;
    PyObject *write;
    PyObject *seek;
    PyObject *tell;
    PyObject *flush;
    PyObject *close;
    PyObject *readable;
    PyObject *writable;
    PyObject *seekable;
    PyObject *isatty;
    PyObject *truncate;
    PyObject *readinto;
    PyObject *readline;
    PyObject *readlines;
    PyObject *fileno;
} EndianedStreamIO;

#define IF_NOT_NULL_UNREF(obj) \
    if (obj != nullptr)        \
    {                          \
        Py_DecRef(obj);        \
        obj = nullptr;         \
    }

void EndianedStreamIO_dealloc(EndianedStreamIO *self)
{
    IF_NOT_NULL_UNREF(self->stream);
    IF_NOT_NULL_UNREF(self->read);
    IF_NOT_NULL_UNREF(self->write);
    IF_NOT_NULL_UNREF(self->seek);
    IF_NOT_NULL_UNREF(self->tell);
    IF_NOT_NULL_UNREF(self->flush);
    IF_NOT_NULL_UNREF(self->close);
    IF_NOT_NULL_UNREF(self->readable);
    IF_NOT_NULL_UNREF(self->writable);
    IF_NOT_NULL_UNREF(self->seekable);
    IF_NOT_NULL_UNREF(self->isatty);
    IF_NOT_NULL_UNREF(self->truncate);
    IF_NOT_NULL_UNREF(self->readinto);
    IF_NOT_NULL_UNREF(self->readline);
    IF_NOT_NULL_UNREF(self->readlines);
    IF_NOT_NULL_UNREF(self->fileno);

    PyObject_Del((PyObject *)self);
}

int EndianedStreamIO_init(EndianedStreamIO *self, PyObject *args, PyObject *kwds)
{
    self->stream = nullptr;
    self->endian = '<'; // default to little-endian

    static const char *kwlist[] = {
        "stream",
        "endian",
        nullptr};

    // Parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|c",
                                     const_cast<char **>(kwlist),
                                     &self->stream,
                                     &self->endian))
    {
        return -1;
    }

    // Validate endian
    if (self->endian != '<' && self->endian != '>')
    {
        PyErr_SetString(PyExc_ValueError, "Invalid endian value. Use '<' for little-endian or '>' for big-endian.");
        return -1;
    }

    // get functions from stream
    self->read = PyObject_GetAttrString(self->stream, "read");
    self->write = PyObject_GetAttrString(self->stream, "write");
    self->seek = PyObject_GetAttrString(self->stream, "seek");
    self->tell = PyObject_GetAttrString(self->stream, "tell");
    self->flush = PyObject_GetAttrString(self->stream, "flush");
    self->close = PyObject_GetAttrString(self->stream, "close");
    self->readable = PyObject_GetAttrString(self->stream, "readable");
    self->writable = PyObject_GetAttrString(self->stream, "writable");
    self->seekable = PyObject_GetAttrString(self->stream, "seekable");
    self->isatty = PyObject_GetAttrString(self->stream, "isatty");
    self->truncate = PyObject_GetAttrString(self->stream, "truncate");
    self->readinto = PyObject_GetAttrString(self->stream, "readinto");
    self->readline = PyObject_GetAttrString(self->stream, "readline");
    self->readlines = PyObject_GetAttrString(self->stream, "readlines");
    self->fileno = PyObject_GetAttrString(self->stream, "fileno");
    return 0;
};

PyMemberDef EndianedStreamIO_members[] = {
    {"endian", T_CHAR, offsetof(EndianedStreamIO, endian), 0, "endian"},
    {"stream", T_OBJECT_EX, offsetof(EndianedStreamIO, stream), Py_READONLY, "stream"},
    {"read", T_OBJECT_EX, offsetof(EndianedStreamIO, read), Py_READONLY, "read"},
    {"write", T_OBJECT_EX, offsetof(EndianedStreamIO, write), Py_READONLY, "write"},
    {"seek", T_OBJECT_EX, offsetof(EndianedStreamIO, seek), Py_READONLY, "seek"},
    {"tell", T_OBJECT_EX, offsetof(EndianedStreamIO, tell), Py_READONLY, "tell"},
    {"flush", T_OBJECT_EX, offsetof(EndianedStreamIO, flush), Py_READONLY, "flush"},
    {"close", T_OBJECT_EX, offsetof(EndianedStreamIO, close), Py_READONLY, "close"},
    {"readable", T_OBJECT_EX, offsetof(EndianedStreamIO, readable), Py_READONLY, "readable"},
    {"writable", T_OBJECT_EX, offsetof(EndianedStreamIO, writable), Py_READONLY, "writable"},
    {"seekable", T_OBJECT_EX, offsetof(EndianedStreamIO, seekable), Py_READONLY, "seekable"},
    {"isatty", T_OBJECT_EX, offsetof(EndianedStreamIO, isatty), Py_READONLY, "isatty"},
    {"truncate", T_OBJECT_EX, offsetof(EndianedStreamIO, truncate), Py_READONLY, "truncate"},
    {"readinto", T_OBJECT_EX, offsetof(EndianedStreamIO, readinto), Py_READONLY, "readinto"},
    {"readline", T_OBJECT_EX, offsetof(EndianedStreamIO, readline), Py_READONLY, "readline"},
    {"readlines", T_OBJECT_EX, offsetof(EndianedStreamIO, readlines), Py_READONLY, "readlines"},
    {"fileno", T_OBJECT_EX, offsetof(EndianedStreamIO, fileno), Py_READONLY, "fileno"},
    {NULL} /* Sentinel */
};

PyObject *EndianedIOBase_get_closed(EndianedStreamIO *self, void *closure)
{
    if (self->stream == nullptr)
    {
        Py_RETURN_TRUE;
    }
    return PyObject_CallMethod(self->stream, "closed", nullptr);
}

PyGetSetDef EndianedStreamIO_getseters[] = {
    {"closed", (getter)EndianedIOBase_get_closed, nullptr, "closed", nullptr},
    {nullptr} /* Sentinel */
};

inline PyObject *_read_buffer(EndianedStreamIO *self, const Py_ssize_t size)
{
    PyObject *py_size = PyLong_FromSsize_t(size);
    PyObject *buffer = PyObject_CallFunctionObjArgs(
        self->read,
        py_size,
        nullptr);
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
static PyObject *EndianedStreamIO_read_t(EndianedStreamIO *self, PyObject *args)
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
        if constexpr (IS_BIG_ENDIAN_SYSTEM)
        {
            if (self->endian == '<')
            {
                value = byteswap(value);
            }
        }
        else
        {
            if (self->endian == '>')
            {
                value = byteswap(value);
            }
        }
    }

    return PyObject_FromAny(value);
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedStreamIO_read_array_t(EndianedStreamIO *self, PyObject *arg)
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
            if constexpr (IS_BIG_ENDIAN_SYSTEM)
            {
                if (self->endian == '<')
                {
                    value = byteswap(value);
                }
            }
            else
            {
                if (self->endian == '>')
                {
                    value = byteswap(value);
                }
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

PyObject *EndianedStreamIO_align(EndianedStreamIO *self, PyObject *arg)
{
    Py_ssize_t size;
    CHECK_SIZE_ARG(arg, size, 4)
    if (size <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "Invalid size argument.");
        return nullptr;
    }
    PyObject *pos = PyObject_CallFunctionObjArgs(
        self->tell,
        nullptr);
    if (pos == nullptr)
    {
        return nullptr;
    }
    Py_ssize_t current_pos = PyLong_AsSsize_t(pos);
    Py_DecRef(pos);
    Py_ssize_t pad = size - (current_pos % size);
    if (pad != size)
    {
        current_pos += pad;
        PyObject *new_pos = PyLong_FromSsize_t(current_pos);
        if (new_pos == nullptr)
        {
            return nullptr;
        }
        PyObject *seek_result = PyObject_CallFunctionObjArgs(
            self->seek,
            new_pos,
            nullptr);
        Py_DecRef(new_pos);
        if (seek_result == nullptr)
        {
            return nullptr;
        }
        return seek_result;
    }
    return PyLong_FromSsize_t(current_pos);
}

PyMethodDef EndianedStreamIO_methods[] = {
    GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedStreamIO_read_t),
    GENERATE_ENDIANEDIOBASE_READ_ARRAY_FUNCTIONS(EndianedStreamIO_read_array_t),
    {"align",
     (PyCFunction)EndianedStreamIO_align,
     METH_O,
     "Align the stream to the specified size."},
    {NULL} /* Sentinel */
};

PyObject *EndianedStreamIO_repr(PyObject *self)
{
    EndianedStreamIO *node = (EndianedStreamIO *)self;

    // Get stream representation
    PyObject *stream_repr = PyObject_Repr(node->stream);
    if (stream_repr == NULL)
    {
        stream_repr = PyUnicode_FromString("None");
    }

    // Create our repr string
    PyObject *result = PyUnicode_FromFormat(
        "<EndianedStreamIO endian='%c' stream=%U>",
        node->endian,
        stream_repr);

    Py_DecRef(stream_repr);
    return result;
}

PyType_Slot EndianedStreamIO_slots[] = {
    {Py_tp_new, reinterpret_cast<void *>(PyType_GenericNew)},
    {Py_tp_init, reinterpret_cast<void *>(EndianedStreamIO_init)},
    {Py_tp_dealloc, reinterpret_cast<void *>(EndianedStreamIO_dealloc)},
    {Py_tp_members, EndianedStreamIO_members},
    {Py_tp_methods, EndianedStreamIO_methods},
    {Py_tp_repr, reinterpret_cast<void *>(EndianedStreamIO_repr)},
    {0, NULL},
};

PyType_Spec EndianedStreamIO_Spec = {
    "EndianedStreamIO",                       // const char* name;
    sizeof(EndianedStreamIO),                 // int basicsize;
    0,                                        // int itemsize;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // unsigned int flags;
    EndianedStreamIO_slots,                   // PyType_Slot *slots;
};

static PyModuleDef EndianedStreamIO_module = {
    PyModuleDef_HEAD_INIT,
    "bier.endianedbinaryio._EndianedStreamIO", // Module name
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

PyMODINIT_FUNC PyInit__EndianedStreamIO(void)
{
    PyObject *m = PyModule_Create(&EndianedStreamIO_module);
    if (m == NULL)
    {
        return NULL;
    }
    // init_format_num();
    EndianedStreamIO_OT = PyType_FromSpec(&EndianedStreamIO_Spec);
    if (add_object(m, "EndianedStreamIO", EndianedStreamIO_OT) < 0)
    {
        return NULL;
    }
    return m;
}
