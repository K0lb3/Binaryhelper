#include <concepts>
#include <cstdint>
#include <bit>
#include <type_traits>

#include "Python.h"
#include "structmember.h"

#include "PyConverter.hpp"
#include "EndianedIOBase.hpp"
#include <algorithm>

// 'truncate'

// 'write'
// 'writelines'
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
    bool closed;        // Indicates if the stream is closed.

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
    self->closed = false;

    static const char *kwlist[] = {
        "initial_bytes",
        "endian",
        nullptr};

    // Clear existing buffer if reinitialized
    if (self->view.buf)
        PyBuffer_Release(&self->view);

    // Parse arguments
    PyObject *buf{};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|c",
                                     const_cast<char **>(kwlist),
                                     &buf,
                                     &self->endian))
    {
        return -1;
    }

    if (PyObject_GetBuffer(buf, &self->view, PyBUF_ND))
    {
        PyErr_SetString(PyExc_ValueError, "Incontigous buffer object.");
        return -1;
    }

    // Validate endian
    if (self->endian != '<' && self->endian != '>')
    {
        PyErr_SetString(PyExc_ValueError, "Invalid endian value. Use '<' for little-endian or '>' for big-endian.");
        return -1;
    }

    return 0;
};

PyMemberDef EndianedBytesIO_members[] = {
    {"pos", T_PYSSIZET, offsetof(EndianedBytesIO, pos), 0, "pos"},
    {"length", T_PYSSIZET, offsetof(EndianedBytesIO, view.len), Py_READONLY, "length"},
    {"endian", T_CHAR, offsetof(EndianedBytesIO, endian), 0, "endian"},
    {"closed", T_BOOL, offsetof(EndianedBytesIO, closed), Py_READONLY, "closed"},
    {NULL} /* Sentinel */
};

PyObject *EndianedBytesIO_read(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    Py_ssize_t size = 0;
    if (arg == Py_None)
    {
        size = self->view.len - self->pos;
    }
    else if (PyLong_Check(arg))
    {
        size = PyLong_AsSsize_t(arg);
        if (size == -1)
        {
            size = self->view.len - self->pos;
        }
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

    auto read_size = std::min(size, self->view.len - self->pos);
    char *buffer = static_cast<char *>(self->view.buf) + self->pos;
    self->pos += read_size;
    return PyBytes_FromStringAndSize(buffer, read_size);
}

PyObject *EndianedBytesIO_readinto(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    if (!PyObject_CheckBuffer(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be a buffer object.");
        return nullptr;
    }
    Py_buffer view;
    if (PyObject_GetBuffer(arg, &view, PyBUF_WRITE) == -1)
    {
        return nullptr;
    }
    Py_ssize_t read_size = std::min(view.len, self->view.len - self->pos);
    Py_MEMCPY(view.buf, static_cast<char *>(self->view.buf) + self->pos, read_size);
    PyBuffer_Release(&view);
    self->pos += read_size;
    return PyLong_FromSsize_t(read_size);
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedBytesIO_read_t(EndianedBytesIO *self, PyObject *unused)
{
    CHECK_CLOSED
    T value{};
    if (self->pos + sizeof(T) > self->view.len)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    // Read the data from the buffer
    memcpy(&value, static_cast<char *>(self->view.buf) + self->pos, sizeof(T));
    self->pos += sizeof(T);

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
static PyObject *EndianedBytesIO_read_array_t(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
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

    if (size * sizeof(T) > self->view.len - self->pos)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    PyObject *ret = PyTuple_New(size);

    for (Py_ssize_t i = 0; i < size; ++i)
    {
        T value{};
        memcpy(&value, static_cast<char *>(self->view.buf) + self->pos, sizeof(T));
        self->pos += sizeof(T);

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
            return nullptr;
        }
        PyTuple_SetItem(ret, i, item); // Steal reference, no need to DECREF
    }
    return ret;
}

PyObject *EndianedBytesIO_seek(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
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
    CHECK_CLOSED
    return PyLong_FromSsize_t(self->pos);
}

PyObject *EndianedBytesIO_seekable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_TRUE;
}

PyObject *EndianedBytesIO_readable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_TRUE;
}

PyObject *EndianedBytesIO_writable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    PyObject *ret = self->view.readonly ? Py_False : Py_True;
    Py_IncRef(ret);
    return ret;
}

PyObject *EndianedBytesIO_close(EndianedBytesIO *self, PyObject *args)
{
    if (self->view.buf != nullptr)
    {
        PyBuffer_Release(&self->view);
        self->view.buf = nullptr;
    }
    self->closed = true;
    Py_RETURN_NONE;
}

PyObject *EndianedBytesIO_isattay(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_FALSE;
}

PyObject *EndianedBytesIO_detach(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    PyErr_SetString(PyExc_IOError, "detach() not supported on this type of stream.");
    return nullptr;
}

PyObject *EndianedBytesIO_fileno(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    PyErr_SetString(PyExc_OSError, "fileno() not supported on this type of stream.");
    return nullptr;
}

PyObject *EndianedBytesIO_flush(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    Py_RETURN_NONE;
}

PyObject *EndianedBytesIO_align(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    Py_ssize_t size;
    CHECK_SIZE_ARG(arg, size, 4)
    if (size <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "Invalid size argument.");
        return nullptr;
    }
    Py_ssize_t pad = size - (self->pos % size);
    if (pad != size)
    {
        Py_ssize_t new_pos = self->pos + pad;
        if (new_pos > self->view.len)
        {
            PyErr_SetString(PyExc_ValueError, "Alignment exceeds buffer length.");
            return nullptr;
        }
        self->pos = new_pos;
    }
    return PyLong_FromSsize_t(self->pos);
}

PyObject *EndianedBytesIO_getValue(EndianedBytesIO *self, void *closure)
{
    CHECK_CLOSED
    if (PyBytes_CheckExact(self->view.obj))
    {
        Py_IncRef(self->view.obj);
        return self->view.obj;
    }
    return PyBytes_FromObject(self->view.obj);
}

PyObject *EndianedBytesIO_getbuffer(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    if (self->view.buf == nullptr)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not initialized.");
        return nullptr;
    }
    return PyMemoryView_FromBuffer(&self->view);
}

static inline PyObject *_EndianedBytesIO_readuntil(EndianedBytesIO *self, char delimiter, Py_ssize_t size)
{
    if (size < 0 || size > self->view.len - self->pos)
    {
        size = self->view.len - self->pos;
    }
    char *buffer = static_cast<char *>(self->view.buf) + self->pos;
    char *end = static_cast<char *>(memchr(buffer, delimiter, size));
    if (end == nullptr)
    {
        end = buffer + size;
    }
    Py_ssize_t read_size = end - buffer;
    self->pos += read_size;
    return PyBytes_FromStringAndSize(buffer, read_size);
}

PyObject *EndianedBytesIO_readline(EndianedBytesIO *self, PyObject *size)
{
    CHECK_CLOSED
    Py_ssize_t read_size;
    CHECK_SIZE_ARG(size, read_size, self->view.len - self->pos);
    return _EndianedBytesIO_readuntil(self, '\n', read_size);
}

PyObject *EndianedBytesIO_cstring(EndianedBytesIO *self, PyObject *size)
{
    CHECK_CLOSED
    Py_ssize_t read_size;
    CHECK_SIZE_ARG(size, read_size, self->view.len - self->pos);
    return _EndianedBytesIO_readuntil(self, '\x00', read_size);
}

PyObject *EndianedBytesIO_readuntil(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    PyObject *delimiter = nullptr;
    Py_ssize_t read_size = 0;
    if (!PyArg_ParseTuple(args, "O|n", &delimiter, &read_size))
    {
        return nullptr;
    }
    if (read_size < 0 || read_size > self->view.len - self->pos)
    {
        read_size = self->view.len - self->pos;
    }
    if (PyUnicode_Check(delimiter))
    {
        PyObject *bytes = PyUnicode_AsEncodedString(delimiter, "utf-8", nullptr);
        if (bytes == nullptr)
        {
            return nullptr;
        }
        char *delim = PyBytes_AsString(bytes);
        Py_ssize_t delim_len = PyBytes_Size(bytes);
        PyObject *result = _EndianedBytesIO_readuntil(self, delim[0], read_size);
        Py_DecRef(bytes);
        return result;
    }
    else if (PyBytes_Check(delimiter))
    {
        char *delim = PyBytes_AsString(delimiter);
        Py_ssize_t delim_len = PyBytes_Size(delimiter);
        return _EndianedBytesIO_readuntil(self, delim[0], read_size);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Delimiter must be a bytes or string object.");
        return nullptr;
    }
}

PyObject *EndianedBytesIO_readlines(EndianedBytesIO *self, PyObject *size)
{
    CHECK_CLOSED
    Py_ssize_t read_size;
    CHECK_SIZE_ARG(size, read_size, self->view.len - self->pos);
    PyObject *result = PyList_New(0);
    if (result == nullptr)
    {
        return nullptr;
    }
    Py_ssize_t end = self->pos + read_size;
    while (self->pos < end)
    {
        read_size = end - self->pos;
        PyObject *line = _EndianedBytesIO_readuntil(self, '\n', read_size);
        if (line == nullptr)
        {
            Py_DecRef(result);
            return nullptr;
        }
        if (PyBytes_Size(line) == 0)
        {
            Py_DecRef(line);
            break;
        }
        if (PyList_Append(result, line) == -1)
        {
            Py_DecRef(line);
            Py_DecRef(result);
            return nullptr;
        }
        Py_DecRef(line);
    }
    return result;
}

// PyObject *EndianedBytesIO_write(EndianedBytesIO *self, PyObject *arg)
// {
//     // CHECK_CLOSED
//     // if (self->view.readonly)
//     // {
//     //     PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
//     //     return nullptr;
//     // }
//     // Py_buffer view;
//     // if (PyObject_GetBuffer(arg, &view, PyBUF_CONTIG_RO) == -1)
//     // {
//     //     return nullptr;
//     // }
//     // if (view.len == 0)
//     // {
//     //     PyBuffer_Release(&view);
//     //     Py_RETURN_NONE;
//     // }
//     // if (self->pos + view.len > self->view.len)
//     // {
//     //     PyObject *buffer_object = self->view.obj;

//     //     _PyBytes_Resize(&buffer_object, self->pos + view.len);
//     //     Py_buffer new_view{};
//     //     PyObject_GetBuffer(buffer_object, &new_view, PyBUF_CONTIG);
//     //     PyBuffer_Release(&self->view);
//     //     PyErr_SetString(PyExc_ValueError, "Write exceeds buffer length.");
//     //     PyBuffer_Release(&view);
//     //     return nullptr;
//     // }
//     // memcpy(static_cast<char *>(self->view.buf) + self->pos, view.buf, view.len);
//     // self->pos += view.len;
//     // PyBuffer_Release(&view);
//     // Py_RETURN_NONE;
// }

PyMethodDef EndianedBytesIO_methods[] = {
    {"read", reinterpret_cast<PyCFunction>(EndianedBytesIO_read), METH_O, "Read bytes from the buffer."},
    {"read1", reinterpret_cast<PyCFunction>(EndianedBytesIO_read), METH_O, "Read bytes from the buffer."}, // basically fullfilling it with normal read
    {"readinto", reinterpret_cast<PyCFunction>(EndianedBytesIO_readinto), METH_O, "Read bytes into a buffer."},
    {"readinto1", reinterpret_cast<PyCFunction>(EndianedBytesIO_readinto), METH_O, "Read bytes into a buffer."}, // basically fullfilling it with normal readinto
    {"readline", reinterpret_cast<PyCFunction>(EndianedBytesIO_readline), METH_O, ""},
    {"readlines", reinterpret_cast<PyCFunction>(EndianedBytesIO_readlines), METH_O, ""},
    {"seek", reinterpret_cast<PyCFunction>(EndianedBytesIO_seek), METH_VARARGS, "Seek to a position in the buffer."},
    {"tell", reinterpret_cast<PyCFunction>(EndianedBytesIO_tell), METH_NOARGS, "Get the current position in the buffer."},
    {"flush", reinterpret_cast<PyCFunction>(EndianedBytesIO_flush), METH_NOARGS, "Flush the buffer."},
    {"detach", reinterpret_cast<PyCFunction>(EndianedBytesIO_detach), METH_NOARGS, "Detach the buffer."},
    {"fileno", reinterpret_cast<PyCFunction>(EndianedBytesIO_fileno), METH_NOARGS, "Get the file descriptor."},
    {"isatty", reinterpret_cast<PyCFunction>(EndianedBytesIO_isattay), METH_NOARGS, "Check if the buffer is a TTY."},
    {"close", reinterpret_cast<PyCFunction>(EndianedBytesIO_close), METH_NOARGS, "Close the buffer."},
    {"readable", reinterpret_cast<PyCFunction>(EndianedBytesIO_readable), METH_NOARGS, "Check if the buffer is readable."},
    {"writable", reinterpret_cast<PyCFunction>(EndianedBytesIO_writable), METH_NOARGS, "Check if the buffer is writable."},
    {"seekable", reinterpret_cast<PyCFunction>(EndianedBytesIO_seekable), METH_NOARGS, "Check if the buffer is seekable."},
    {"getbuffer", reinterpret_cast<PyCFunction>(EndianedBytesIO_getbuffer), METH_NOARGS, "Get the buffer."},
    {"getvalue", reinterpret_cast<PyCFunction>(EndianedBytesIO_getValue), METH_NOARGS, "Get the current value of the buffer."},
    // reader endian based
    GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedBytesIO_read_t),
    GENERATE_ENDIANEDIOBASE_READ_ARRAY_FUNCTIONS(EndianedBytesIO_read_array_t),
    {"align", reinterpret_cast<PyCFunction>(EndianedBytesIO_align), METH_O, "Aligns the position of the buffer."},
    {"readuntil", reinterpret_cast<PyCFunction>(EndianedBytesIO_readuntil), METH_VARARGS, "Read until a delimiter."},
    {"read_cstring", reinterpret_cast<PyCFunction>(EndianedBytesIO_cstring), METH_O, "Read until a null terminator."},
    {NULL} /* Sentinel */
};

PyObject *
EndianedBytesIO_repr(EndianedBytesIO *self)
{
    if (self->closed) {
        return PyUnicode_FromString("<EndianedBytesIO [closed]>");
    }

    return PyUnicode_FromFormat(
        "<EndianedBytesIO pos=%zd len=%zd endian='%c' closed=%R>",
        self->pos,
        self->view.len,
        self->endian,
        self->closed ? Py_True : Py_False
    );
}

// Usage would be:
static PyType_Spec EndianedBytesIO_Spec =
    createPyTypeSpec<EndianedBytesIO>(
        "EndianedBytesIO",
        EndianedBytesIO_init,
        EndianedBytesIO_dealloc,
        EndianedBytesIO_members,
        EndianedBytesIO_methods,
        EndianedBytesIO_repr);

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
