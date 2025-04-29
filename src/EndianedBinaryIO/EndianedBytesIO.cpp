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

PyObject *EndianedBytesIO_OT = nullptr;

typedef struct
{
    PyObject_HEAD
        Py_buffer view; // The view object for the buffer.
    Py_ssize_t pos;     // The current position in the buffer.
    char endian;        // The endianness of the data.

} EndianedBytesIO;

void EndianedBytesIO_dealloc(EndianedBytesIO *self)
{
    if (self->view.buf != nullptr)
    {
        PyBuffer_Release(&self->view);
    }
    PyObject_Del((PyObject *)self);
}

int EndianedBytesIO_init(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    self->view = {};
    self->pos = 0;
    self->endian = '<';

    static const char *kwlist[] = {
        "initial_bytes",
        "endian",
        nullptr};

    // Clear existing buffer if reinitialized
    if (self->view.buf)
        PyBuffer_Release(&self->view);

    // Parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*|c",
                                     const_cast<char **>(kwlist),
                                     &self->view,
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

    // PyObject_SetAttrString(
    //     reinterpret_cast<PyObject *>(self),
    //     PyUnicode_FromString("read_count"),
    //     PyObject_GetAttrString(
    //         reinterpret_cast<PyObject *>(self),
    //         PyUnicode_FromString("read_i32")));

    return 0;
};

PyMemberDef EndianedBytesIO_members[] = {
    {"pos", T_PYSSIZET, offsetof(EndianedBytesIO, pos), 0, "pos"},
    {"length", T_PYSSIZET, offsetof(EndianedBytesIO, view.len), Py_READONLY, "length"},
    {"endian", T_CHAR, offsetof(EndianedBytesIO, endian), 0, "endian"},
    // {"stride", T_SHORT, offsetof(EndianedBytesIO, surf.stride), 0, "stride"},
    {NULL} /* Sentinel */
};

PyObject *EndianedBytesIO_read(EndianedBytesIO *self, PyObject *args)
{
    Py_ssize_t size = 0;
    if (!PyArg_ParseTuple(args, "|n", &size))
    {
        return nullptr;
    }
    if (size <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "Size must be positive.");
        return nullptr;
    }
    auto read_size = std::min(size, self->view.len - self->pos);
    PyObject *ret = PyBytes_FromStringAndSize(static_cast<char *>(self->view.buf) + self->pos, read_size);
    self->pos += read_size;
    return ret;
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedBytesIO_read_t(EndianedBytesIO *self, PyObject *args)
{
    T value{};
    if (self->pos + sizeof(T) > self->view.len)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    // Read the data from the buffer
    memcpy(&value, static_cast<char *>(self->view.buf) + self->pos, sizeof(T));
    self->pos += sizeof(T);

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

PyObject *EndianedBytesIO_seek(EndianedBytesIO *self, PyObject *args)
{
    Py_ssize_t offset = 0;
    int whence = SEEK_SET;
    if (!PyArg_ParseTuple(args, "n|i", &offset, &whence))
    {
        return nullptr;
    }
    Py_ssize_t new_pos = 0;
    switch (whence)
    {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = self->pos + offset;
        break;
    case SEEK_END:
        new_pos = self->view.len + offset;
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "Invalid value for whence.");
        return nullptr;
    }
    if (new_pos < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Negative seek position.");
        return nullptr;
    }
    self->pos = new_pos;
    return PyLong_FromSsize_t(self->pos);
}

PyObject *EndianedBytesIO_tell(EndianedBytesIO *self, PyObject *args)
{
    return PyLong_FromSsize_t(self->pos);
}

PyObject *EndianedBytesIO_seekable(EndianedBytesIO *self, PyObject *args)
{
    Py_IncRef(Py_True);
    return Py_True;
}

PyObject *EndianedBytesIO_readable(EndianedBytesIO *self, PyObject *args)
{
    Py_IncRef(Py_True);
    return Py_True;
}

PyObject *EndianedBytesIO_writable(EndianedBytesIO *self, PyObject *args)
{
    PyObject *ret = self->view.readonly ? Py_False : Py_True;
    Py_IncRef(ret);
    return ret;
}

PyObject *EndianedBytesIO_getValue(EndianedBytesIO *self, void *closure)
{
    return PyBytes_FromObject(self->view.obj);
}

PyObject *EndianedBytesIO_getbuffer(EndianedBytesIO *self, PyObject *args)
{
    if (self->view.buf == nullptr)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not initialized.");
        return nullptr;
    }
    return PyMemoryView_FromBuffer(&self->view);
}

PyMethodDef EndianedBytesIO_methods[] = {
    {"read", reinterpret_cast<PyCFunction>(EndianedBytesIO_read), METH_VARARGS, "Read bytes from the buffer."},
    {"seek", reinterpret_cast<PyCFunction>(EndianedBytesIO_seek), METH_VARARGS, "Seek to a position in the buffer."},
    {"tell", reinterpret_cast<PyCFunction>(EndianedBytesIO_tell), METH_NOARGS, "Get the current position in the buffer."},
    // {"flush", reinterpret_cast<PyCFunction>(EndianedBytesIO_flush), METH_NOARGS, "Flush the buffer."},
    //{"close", reinterpret_cast<PyCFunction>(EndianedBytesIO_close), METH_NOARGS, "Close the buffer."},
    {"readable", reinterpret_cast<PyCFunction>(EndianedBytesIO_readable), METH_NOARGS, "Check if the buffer is readable."},
    {"writable", reinterpret_cast<PyCFunction>(EndianedBytesIO_writable), METH_NOARGS, "Check if the buffer is writable."},
    {"seekable", reinterpret_cast<PyCFunction>(EndianedBytesIO_seekable), METH_NOARGS, "Check if the buffer is seekable."},
    {"getbuffer", reinterpret_cast<PyCFunction>(EndianedBytesIO_getbuffer), METH_NOARGS, "Get the buffer."},
    {"getvalue", reinterpret_cast<PyCFunction>(EndianedBytesIO_getValue), METH_NOARGS, "Get the current value of the buffer."},
    //{"detach", reinterpret_cast<PyCFunction>(EndianedBytesIO_detach), METH_NOARGS, "Detach the buffer."},
    // reader endian based
    {"read_u8", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint8_t, '|'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint16_t, '|'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint32_t, '|'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint64_t, '|'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int8_t, '|'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int16_t, '|'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int32_t, '|'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int64_t, '|'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<half, '|'>), METH_NOARGS, "Read a half value."},
    {"read_f32", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<float, '|'>), METH_NOARGS, "Read a float value."},
    {"read_f64", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<double, '|'>), METH_NOARGS, "Read a double value."},
    // low endian
    {"read_u8_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint8_t, '<'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint16_t, '<'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint32_t, '<'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint64_t, '<'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int8_t, '<'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int16_t, '<'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int32_t, '<'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int64_t, '<'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<half, '<'>), METH_NOARGS, "Read a half value."},
    {"read_f32_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<float, '<'>), METH_NOARGS, "Read a float value."},
    {"read_f64_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<double, '<'>), METH_NOARGS, "Read a double value."},
    // big endian
    {"read_u8_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint8_t, '>'>), METH_NOARGS, "Read a uint8_t value."},
    {"read_u16_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint16_t, '>'>), METH_NOARGS, "Read a uint16_t value."},
    {"read_u32_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint32_t, '>'>), METH_NOARGS, "Read a uint32_t value."},
    {"read_u64_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<uint64_t, '>'>), METH_NOARGS, "Read a uint64_t value."},
    {"read_i8_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int8_t, '>'>), METH_NOARGS, "Read an int8_t value."},
    {"read_i16_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int16_t, '>'>), METH_NOARGS, "Read an int16_t value."},
    {"read_i32_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int32_t, '>'>), METH_NOARGS, "Read an int32_t value."},
    {"read_i64_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<int64_t, '>'>), METH_NOARGS, "Read an int64_t value."},
    {"read_f16_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<half, '>'>), METH_NOARGS, "Read a half value."},
    {"read_f32_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<float, '>'>), METH_NOARGS, "Read a float value."},
    {"read_f64_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_t<double, '>'>), METH_NOARGS, "Read a double value."},
    {NULL} /* Sentinel */
};

PyObject *
EndianedBytesIO_repr(PyObject *self)
{
    EndianedBytesIO *node = (EndianedBytesIO *)self;
    return PyUnicode_FromFormat(
        "<EndianedBytesIOC (w:%d, h:%d, s:%d)>",
        1, 2, 3
        // node->obj->__repr__(node->obj),
        // node->surf.width,
        // node->surf.height,
        // node->surf.stride
    );
}

PyType_Slot EndianedBytesIO_slots[] = {
    {Py_tp_new, reinterpret_cast<void *>(PyType_GenericNew)},
    {Py_tp_init, reinterpret_cast<void *>(EndianedBytesIO_init)},
    {Py_tp_dealloc, reinterpret_cast<void *>(EndianedBytesIO_dealloc)},
    {Py_tp_members, EndianedBytesIO_members},
    {Py_tp_methods, EndianedBytesIO_methods},
    {Py_tp_repr, reinterpret_cast<void *>(EndianedBytesIO_repr)},
    {0, NULL},
};

PyType_Spec EndianedBytesIO_Spec = {
    "EndianedBytesIO",                        // const char* name;
    sizeof(EndianedBytesIO),                  // int basicsize;
    0,                                        // int itemsize;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // unsigned int flags;
    EndianedBytesIO_slots,                    // PyType_Slot *slots;
};

static PyModuleDef EndianedBytesIO_module = {
    PyModuleDef_HEAD_INIT,
    "bier.endianedbinaryio._EndianedBytesIO", // Module name
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

PyMODINIT_FUNC PyInit__EndianedBytesIO(void)
{
    PyObject *m = PyModule_Create(&EndianedBytesIO_module);
    if (m == NULL)
    {
        return NULL;
    }
    // init_format_num();
    EndianedBytesIO_OT = PyType_FromSpec(&EndianedBytesIO_Spec);
    if (add_object(m, "EndianedBytesIO", EndianedBytesIO_OT) < 0)
    {
        return NULL;
    }
    return m;
}