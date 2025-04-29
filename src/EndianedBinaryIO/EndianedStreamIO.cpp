#include <concepts>
#include <cstdint>
#include <bit>
#include <type_traits>

#include "Python.h"
#include "structmember.h"

#include "PyConverter.hpp"
#include <algorithm>

// 'align'

// 'close'
// 'closed'
// 'detach'

// 'fileno'
// 'flush'

// 'isatty'

// 'truncate'

// 'read'
// 'read1'

// 'readinto'
// 'readinto1'
// 'readline'
// 'readlines'

// 'write'
// 'writelines'

// 'read_count'

// 'write_f16'
// 'write_f32'
// 'write_f64'
// 'write_i16'
// 'write_i32'
// 'write_i64'
// 'write_i8'
// 'write_u16'
// 'write_u32'
// 'write_u64'
// 'write_u8'

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
    {NULL} /* Sentinel */
};

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedStreamIO_read_t(EndianedStreamIO *self, PyObject *args)
{
    PyObject *py_size = PyLong_FromSsize_t(sizeof(T));
    PyObject *buffer = PyObject_CallFunctionObjArgs(
        self->read,
        py_size,
        nullptr);
    Py_DECREF(py_size);

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
    Py_DECREF(buffer);

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
        if constexpr (kBigEndianSystem)
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

PyMethodDef EndianedStreamIO_methods[] = {
    {"read_u8", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint8_t, '|'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint16_t, '|'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint32_t, '|'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint64_t, '|'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int8_t, '|'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int16_t, '|'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int32_t, '|'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int64_t, '|'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<half, '|'>), METH_NOARGS, "Read a half value."},
    {"read_f32", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<float, '|'>), METH_NOARGS, "Read a float value."},
    {"read_f64", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<double, '|'>), METH_NOARGS, "Read a double value."},
    // low endian
    {"read_u8_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint8_t, '<'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint16_t, '<'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint32_t, '<'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint64_t, '<'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int8_t, '<'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int16_t, '<'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int32_t, '<'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int64_t, '<'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<half, '<'>), METH_NOARGS, "Read a half value."},
    {"read_f32_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<float, '<'>), METH_NOARGS, "Read a float value."},
    {"read_f64_le", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<double, '<'>), METH_NOARGS, "Read a double value."},
    // big endian
    {"read_u8_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint8_t, '>'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint16_t, '>'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint32_t, '>'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<uint64_t, '>'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int8_t, '>'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int16_t, '>'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int32_t, '>'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<int64_t, '>'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<half, '>'>), METH_NOARGS, "Read a half value."},
    {"read_f32_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<float, '>'>), METH_NOARGS, "Read a float value."},
    {"read_f64_be", reinterpret_cast<PyCFunction>(EndianedStreamIO_read_t<double, '>'>), METH_NOARGS, "Read a double value."},
    {NULL} /* Sentinel */
};

PyObject *
EndianedStreamIO_repr(PyObject *self)
{
    EndianedStreamIO *node = (EndianedStreamIO *)self;
    return PyUnicode_FromFormat(
        "<EndianedStreamIOC (w:%d, h:%d, s:%d)>",
        1, 2, 3
        // node->obj->__repr__(node->obj),
        // node->surf.width,
        // node->surf.height,
        // node->surf.stride
    );
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