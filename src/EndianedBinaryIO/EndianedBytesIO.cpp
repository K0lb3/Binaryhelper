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
// 'writelines'

PyObject *EndianedBytesIO_OT = nullptr;

typedef struct
{
    PyObject_HEAD
        Py_buffer view; // The view object for the buffer.
    Py_ssize_t pos;     // The current position in the buffer.
    char endian;        // The endianness of the data.
    bool closed;        // Indicates if the stream is closed.

} EndianedBytesIO;

static void EndianedBytesIO_dealloc(EndianedBytesIO *self)
{
    if (self->view.buf != nullptr)
    {
        PyBuffer_Release(&self->view);
    }
    PyObject_Del((PyObject *)self);
}

static int EndianedBytesIO_init(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    self->view = {};
    self->pos = 0;
    self->endian = '<';
    self->closed = false;

    Py_buffer endian_view{};

    static const char *kwlist[] = {
        "initial_bytes",
        "endian",
        nullptr};

    // Clear existing buffer if reinitialized
    if (self->view.buf)
        PyBuffer_Release(&self->view);

    // Parse arguments
    PyObject *buf{};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|s*",
                                     const_cast<char **>(kwlist),
                                     &buf,
                                     &endian_view))
    {
        return -1;
    }

    // parse endian argument
    if (endian_view.buf != nullptr)
    {
        char *buf_ptr = static_cast<char *>(endian_view.buf);
        // If an endian view is provided, use it
        if (endian_view.len != 1 || (buf_ptr[0] != '<' && buf_ptr[0] != '>'))
        {
            PyErr_SetString(PyExc_ValueError, "Endian must be '<' or '>'.");
            return -1;
        }
        self->endian = buf_ptr[0];
        PyBuffer_Release(&endian_view);
    }

    // check if the buffer is contiguous
    if (PyObject_GetBuffer(buf, &self->view, PyBUF_ND))
    {
        PyErr_SetString(PyExc_ValueError, "Incontigous buffer object.");
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

static PyObject *EndianedBytesIO_read(EndianedBytesIO *self, PyObject *arg)
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

static PyObject *EndianedBytesIO_readinto(EndianedBytesIO *self, PyObject *arg)
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

    handle_swap<EndianedBytesIO, T, endian>(self, value);

    return PyObject_FromAny(value);
}

static inline bool _read_count(EndianedBytesIO *self, PyObject *py_count, Py_ssize_t &count)
{
    if ((py_count == nullptr) || (py_count == Py_None))
    {
        PyObject *py_count = PyObject_CallMethod(
            reinterpret_cast<PyObject *>(self),
            "read_count",
            "",
            nullptr);
        if (py_count == nullptr)
        {
            return false;
        }

        if (!PyLong_Check(py_count))
        {
            PyErr_SetString(PyExc_TypeError, "read_count didn't return an integer.");
            Py_DecRef(py_count);
            return false;
        }
        count = PyLong_AsSsize_t(py_count);
        Py_DecRef(py_count);
        return true;
    }
    else if (PyLong_Check(py_count))
    {
        count = PyLong_AsSsize_t(py_count);
        if ((count < 0))
        {
            PyErr_SetString(PyExc_ValueError, "Invalid size argument.");
            return false;
        }
        else if (PyErr_Occurred())
        {
            return false;
        }
        return true;
    }

    PyErr_SetString(PyExc_TypeError, "Argument must be an integer or None.");
    return false;
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedBytesIO_read_array_t(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    Py_ssize_t size = 0;
    if (!_read_count(self, arg, size))
    {
        return nullptr;
    }

    if (size * sizeof(T) > self->view.len - self->pos)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    PyObject *ret = PyTuple_New(size);

    T value{};
    for (Py_ssize_t i = 0; i < size; ++i)
    {

        memcpy(&value, static_cast<char *>(self->view.buf) + self->pos, sizeof(T));
        self->pos += sizeof(T);

        handle_swap<EndianedBytesIO, T, endian>(self, value);

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

static inline bool _check_size(EndianedBytesIO *self, Py_ssize_t write_size)
{
    if (self->pos + write_size > self->view.len)
    {
        PyErr_SetString(PyExc_ValueError, "Write exceeds buffer length. (Resize not implemented)");
        return true;
        // self->view.obj can't be resized, as it's being used by the view
        // -> needs a rewrite of the input handling logic
    }
    return false;
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedBytesIO_write_t(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    if (_check_size(self, sizeof(T)))
    {
        return nullptr; // Resize failed
    }

    T value{};
    if (!PyObject_ToAny(arg, value))
    {
        return nullptr; // Conversion failed
    }

    handle_swap<EndianedBytesIO, T, endian>(self, value);

    memcpy(static_cast<char *>(self->view.buf) + self->pos, &value, sizeof(T));
    self->pos += sizeof(T);
    return PyLong_FromLong(sizeof(T));
}

template <typename T, char endian>
    requires(
        EndianedReadable<T> &&
        (endian == '<' || endian == '>' || endian == '|'))
static PyObject *EndianedBytesIO_write_array_t(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    static const char *kwlist[] = {
        "v",
        "write_count",
        nullptr};

    PyObject *v = nullptr;
    PyObject *write_count_obj = Py_True; // Default to True

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O!",
                                     const_cast<char **>(kwlist),
                                     &v,
                                     &PyBool_Type,
                                     &write_count_obj))
    {
        return nullptr;
    }

    Py_ssize_t count = PyObject_Size(v);
    if ((write_count_obj == Py_True) && EndianedIOBase_write_count(self, count))
    {
        return nullptr; // Resize failed
    }
    if (_check_size(self, count * sizeof(T)))
    {
        return nullptr; // Resize failed
    }

    PyObject *iter = PyObject_GetIter(v);
    PyObject *item = PyIter_Next(iter);
    T value{};
    while (item)
    {
        if (!PyObject_ToAny(item, value))
        {
            Py_DecRef(iter);
            return nullptr; // Conversion failed
        }

        handle_swap<EndianedBytesIO, T, endian>(self, value);

        memcpy(static_cast<char *>(self->view.buf) + self->pos, &value, sizeof(T));
        self->pos += sizeof(T);
        Py_DecRef(item);
        item = PyIter_Next(iter);
    }
    Py_DecRef(iter);
    if (item == nullptr && PyErr_Occurred())
    {
        return nullptr; // Error occurred during iteration
    }
    return PyLong_FromSsize_t(count * sizeof(T));
}

static PyObject *EndianedBytesIO_write_cstring(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    static const char *kwlist[] = {
        "string",
        "encoding",
        "errors",
        nullptr};
    PyObject *v = nullptr;
    const char *encoding = "utf-8";         // Default encoding
    const char *errors = "surrogateescape"; // Default error handling

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ss",
                                     const_cast<char **>(kwlist),
                                     &v,
                                     &encoding,
                                     &errors))
    {
        return nullptr;
    }

    PyObject *bytes = PyUnicode_AsEncodedString(v, encoding, errors);
    if (bytes == nullptr)
    {
        return nullptr; // Encoding failed
    }
    Py_ssize_t write_size = PyBytes_GET_SIZE(bytes);
    if (_check_size(self, write_size + 1)) // +1 for null terminator
    {
        Py_DecRef(bytes);
        return nullptr; // Resize failed
    }
    memcpy(
        static_cast<char *>(self->view.buf) + self->pos,
        PyBytes_AS_STRING(bytes),
        write_size);
    Py_DecRef(bytes);

    self->pos += write_size;
    reinterpret_cast<char *>(self->view.buf)[self->pos++] = 0; // Null terminator
    return PyLong_FromSsize_t(write_size + 1);
}

static PyObject *EndianedBytesIO_write_string(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    static const char *kwlist[] = {
        "string",
        "write_count",
        "encoding",
        "errors",
        nullptr};
    PyObject *v = nullptr;
    PyObject *write_count_obj = Py_True;
    const char *encoding = "utf-8";         // Default encoding
    const char *errors = "surrogateescape"; // Default error handling

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O!ss",
                                     const_cast<char **>(kwlist),
                                     &v,
                                     &PyBool_Type,
                                     &write_count_obj,
                                     &encoding,
                                     &errors))
    {
        return nullptr;
    }

    Py_ssize_t start_pos = self->pos;
    Py_ssize_t string_size = PyUnicode_GET_LENGTH(v);
    if ((write_count_obj == Py_True) && EndianedIOBase_write_count(self, string_size))
    {
        return nullptr; // Resize failed
    }

    PyObject *bytes = PyUnicode_AsEncodedString(v, encoding, errors);
    if (bytes == nullptr)
    {
        return nullptr; // Encoding failed
    }
    Py_ssize_t bytes_size = PyBytes_GET_SIZE(bytes);
    if (_check_size(self, bytes_size))
    {
        self->pos = start_pos;
        Py_DecRef(bytes);
        return nullptr; // Resize failed
    }
    memcpy(
        static_cast<char *>(self->view.buf) + self->pos,
        PyBytes_AS_STRING(bytes),
        bytes_size);
    Py_DecRef(bytes);

    self->pos += bytes_size;
    return PyLong_FromSsize_t(self->pos - start_pos);
}

static PyObject *EndianedBytesIO_write_bytes(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    static const char *kwlist[] = {
        "data",
        "write_count",
        nullptr};
    Py_buffer v{};
    PyObject *write_count_obj = Py_True;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s*|O!",
                                     const_cast<char **>(kwlist),
                                     &v,
                                     &PyBool_Type,
                                     &write_count_obj))
    {
        return nullptr;
    }

    Py_ssize_t start_pos = self->pos;
    if ((write_count_obj == Py_True) && EndianedIOBase_write_count(self, v.len))
    {
        PyBuffer_Release(&v);
        return nullptr; // Resize failed
    }
    if (_check_size(self, v.len))
    {
        self->pos = start_pos;
        PyBuffer_Release(&v);
        return nullptr; // Resize failed
    }
    memcpy(static_cast<char *>(self->view.buf) + self->pos, v.buf, v.len);
    self->pos += v.len;
    PyBuffer_Release(&v);
    return PyLong_FromSsize_t(self->pos - start_pos);
}

static PyObject *EndianedBytesIO_write_varint(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    Py_ssize_t value = 0;
    if (!PyArg_ParseTuple(arg, "n", &value))
    {
        return nullptr;
    }
    if (value < 0)
    {
        PyErr_SetString(PyExc_ValueError, "Varint must be non-negative.");
        return nullptr;
    }

    // Calculate the number of bytes needed for the varint
    Py_ssize_t write_size = 0;
    uint64_t temp = value;
    do
    {
        write_size++;
        temp >>= 7;
    } while (temp != 0);

    if (_check_size(self, write_size))
    {
        return nullptr; // Resize failed
    }

    while (value > 0x7F)
    {
        uint8_t byte = static_cast<uint8_t>((value & 0x7F) | 0x80);
        reinterpret_cast<uint8_t *>(self->view.buf)[self->pos++] = byte;
        value >>= 7;
    }

    return PyLong_FromSsize_t(write_size);
}

static PyObject *EndianedBytesIO_write_varint_array(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }

    static const char *kwlist[] = {
        "v",
        "write_count",
        nullptr};

    PyObject *v = nullptr;
    PyObject *write_count_obj = Py_True; // Default to True

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O!",
                                     const_cast<char **>(kwlist),
                                     &v,
                                     &PyBool_Type,
                                     &write_count_obj))
    {
        return nullptr;
    }

    Py_ssize_t start_pos = self->pos;
    Py_ssize_t count = PyObject_Size(v);
    if ((write_count_obj == Py_True) && EndianedIOBase_write_count(self, count))
    {
        return nullptr; // Resize failed
    }

    PyObject *iter = PyObject_GetIter(v);
    PyObject *item = PyIter_Next(iter);
    while (item)
    {
        EndianedBytesIO_write_varint(self, item);
        if (PyErr_Occurred())
        {
            self->pos = start_pos;
            Py_DecRef(iter);
            return nullptr; // Error occurred during write
        }
    }
    Py_DecRef(iter);
    return PyLong_FromSsize_t(self->pos - start_pos);
}

static PyObject *EndianedBytesIO_seek(EndianedBytesIO *self, PyObject *args)
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

static PyObject *EndianedBytesIO_tell(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    return PyLong_FromSsize_t(self->pos);
}

static PyObject *EndianedBytesIO_seekable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_TRUE;
}

static PyObject *EndianedBytesIO_readable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_TRUE;
}

static PyObject *EndianedBytesIO_writable(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    PyObject *ret = self->view.readonly ? Py_False : Py_True;
    Py_IncRef(ret);
    return ret;
}

static PyObject *EndianedBytesIO_close(EndianedBytesIO *self, PyObject *args)
{
    if (self->view.buf != nullptr)
    {
        PyBuffer_Release(&self->view);
        self->view.buf = nullptr;
    }
    self->closed = true;
    Py_RETURN_NONE;
}

static PyObject *EndianedBytesIO_isatty(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_RETURN_FALSE;
}

static PyObject *EndianedBytesIO_detach(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    PyErr_SetString(PyExc_IOError, "detach() not supported on this type of stream.");
    return nullptr;
}

static PyObject *EndianedBytesIO_fileno(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    PyErr_SetString(PyExc_OSError, "fileno() not supported on this type of stream.");
    return nullptr;
}

static PyObject *EndianedBytesIO_flush(EndianedBytesIO *self, PyObject *no_args)
{
    CHECK_CLOSED
    Py_RETURN_NONE;
}

static PyObject *EndianedBytesIO_align(EndianedBytesIO *self, PyObject *arg)
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

static PyObject *EndianedBytesIO_getValue(EndianedBytesIO *self, void *closure)
{
    CHECK_CLOSED
    if (PyBytes_CheckExact(self->view.obj))
    {
        Py_IncRef(self->view.obj);
        return self->view.obj;
    }
    return PyBytes_FromObject(self->view.obj);
}

static PyObject *EndianedBytesIO_getbuffer(EndianedBytesIO *self, PyObject *args)
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

static PyObject *EndianedBytesIO_readline(EndianedBytesIO *self, PyObject *size)
{
    CHECK_CLOSED
    Py_ssize_t read_size;
    CHECK_SIZE_ARG(size, read_size, self->view.len - self->pos);
    return _EndianedBytesIO_readuntil(self, '\n', read_size);
}

static PyObject *EndianedBytesIO_read_cstring(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED

    static const char *kwlist[] = {
        "encoding",
        "errors",
        nullptr};

    const char *encoding = "utf-8";         // Default encoding
    const char *errors = "surrogateescape"; // Default error handling

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ss",
                                     const_cast<char **>(kwlist),
                                     &encoding,
                                     &errors))
    {
        return nullptr;
    }

    PyObject *result_bytes = _EndianedBytesIO_readuntil(self, '\x00', self->view.len - self->pos);
    if (result_bytes == nullptr)
    {
        return nullptr;
    }
    PyObject *result_str = PyUnicode_FromEncodedObject(result_bytes, encoding, errors);
    Py_DecRef(result_bytes);
    return result_str;
}

static PyObject *EndianedBytesIO_read_string(EndianedBytesIO *self, PyObject *args, PyObject *kwds)
{
    CHECK_CLOSED

    static const char *kwlist[] = {
        "length",
        "encoding",
        "errors",
        nullptr};

    PyObject *py_count = nullptr;
    const char *encoding = "utf-8";         // Default encoding
    const char *errors = "surrogateescape"; // Default error handling

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oss",
                                     const_cast<char **>(kwlist),
                                     &py_count, &encoding,
                                     &errors))
    {
        return nullptr;
    }

    Py_ssize_t count = 0;
    if (!_read_count(self, py_count, count))
    {
        return nullptr;
    }
    if (count < 0 || count > self->view.len - self->pos)
    {
        count = self->view.len - self->pos;
    }
    char *buffer = static_cast<char *>(self->view.buf) + self->pos;
    self->pos += count;
    return PyUnicode_Decode(buffer, count, encoding, errors);
}

static PyObject *EndianedBytesIO_read_bytes(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    Py_ssize_t size = 0;
    if (!_read_count(self, arg, size))
    {
        return nullptr;
    }

    if (size > self->view.len - self->pos)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    PyObject *result = PyBytes_FromStringAndSize(
        static_cast<char *>(self->view.buf) + self->pos, size);
    if (result == nullptr)
    {
        return nullptr;
    }
    self->pos += size;
    return result;
}

static PyObject *EndianedBytesIO_readuntil(EndianedBytesIO *self, PyObject *args)
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

static PyObject *EndianedBytesIO_readlines(EndianedBytesIO *self, PyObject *size)
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

static PyObject *EndianedBytesIO_read_varint(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_ssize_t value = 0;
    uint32_t shift = 0;

    while (true)
    {
        if (self->pos >= self->view.len)
        {
            PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
            return nullptr;
        }
        unsigned char byte = static_cast<unsigned char *>(self->view.buf)[self->pos++];
        value |= (static_cast<Py_ssize_t>(byte & 0x7F) << shift);
        if (!(byte & 0x80))
        {
            break;
        }
        shift += 7;
        if (shift >= sizeof(Py_ssize_t) * 8)
        {
            PyErr_SetString(PyExc_OverflowError, "Varint too large.");
            return nullptr;
        }
    }
    return PyLong_FromSsize_t(value);
}

static PyObject *EndianedBytesIO_read_varint_array(EndianedBytesIO *self, PyObject *args)
{
    CHECK_CLOSED
    Py_ssize_t size = 0;

    if (!_read_count(self, args, size))
    {
        return nullptr;
    }

    if (size > self->view.len - self->pos)
    {
        PyErr_SetString(PyExc_ValueError, "Read exceeds buffer length.");
        return nullptr;
    }

    PyObject *ret = PyTuple_New(size);

    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject *item = EndianedBytesIO_read_varint(self, nullptr);
        if (item == nullptr)
        {
            Py_DecRef(ret);
            return nullptr;
        }
        PyTuple_SetItem(ret, i, item); // Steal reference, no need to DECREF
    }
    return ret;
}

PyObject *EndianedBytesIO_write(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }
    Py_buffer view;
    if (PyObject_GetBuffer(arg, &view, PyBUF_CONTIG_RO) == -1)
    {
        return nullptr;
    }
    if (view.len == 0)
    {
        PyBuffer_Release(&view);
        Py_RETURN_NONE;
    }

    if (_check_size(self, sizeof(view.len)))
    {
        return nullptr; // Resize failed
    }
    memcpy(static_cast<char *>(self->view.buf) + self->pos, view.buf, view.len);
    self->pos += view.len;
    PyObject *ret = PyLong_FromSsize_t(view.len);
    PyBuffer_Release(&view);
    return ret;
}

PyObject *EndianedBytesIO_writelines(EndianedBytesIO *self, PyObject *arg)
{
    CHECK_CLOSED
    if (self->view.readonly)
    {
        PyErr_SetString(PyExc_ValueError, "Buffer is not writable.");
        return nullptr;
    }
    PyErr_SetString(PyExc_NotImplementedError, "writelines() not implemented for EndianedBytesIO.");
    return nullptr;
}

static PyMethodDef EndianedBytesIO_methods[] = {
    GENERATE_ENDIANEDIOBASE_BASE_FUNCTIONS(EndianedBytesIO),
    {"read1", reinterpret_cast<PyCFunction>(EndianedBytesIO_read), METH_O, "Read bytes from the buffer."},       // basically fullfilling it with normal read
    {"readinto1", reinterpret_cast<PyCFunction>(EndianedBytesIO_readinto), METH_O, "Read bytes into a buffer."}, // basically fullfilling it with normal readinto
    {"readline", reinterpret_cast<PyCFunction>(EndianedBytesIO_readline), METH_O, ""},
    {"readlines", reinterpret_cast<PyCFunction>(EndianedBytesIO_readlines), METH_O, ""},
    {"detach", reinterpret_cast<PyCFunction>(EndianedBytesIO_detach), METH_NOARGS, "Detach the buffer."},
    {"getbuffer", reinterpret_cast<PyCFunction>(EndianedBytesIO_getbuffer), METH_NOARGS, "Get the buffer."},
    {"getvalue", reinterpret_cast<PyCFunction>(EndianedBytesIO_getValue), METH_NOARGS, "Get the current value of the buffer."},
    // reader endian based
    GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedBytesIO),
    // writer endian based
    {"write", reinterpret_cast<PyCFunction>(EndianedBytesIO_write), METH_O, "Write bytes to the buffer."},
    GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS(EndianedBytesIO),
    {"readuntil", reinterpret_cast<PyCFunction>(EndianedBytesIO_readuntil), METH_VARARGS, "Read until a delimiter."},
    {NULL} /* Sentinel */
};

static PyObject *
EndianedBytesIO_repr(EndianedBytesIO *self)
{
    if (self->closed)
    {
        return PyUnicode_FromString("<EndianedBytesIO [closed]>");
    }

    return PyUnicode_FromFormat(
        "<EndianedBytesIO pos=%zd len=%zd endian='%c' closed=%R>",
        self->pos,
        self->view.len,
        self->endian,
        self->closed ? Py_True : Py_False);
}

// Usage would be:
static PyType_Spec EndianedBytesIO_Spec =
    createPyTypeSpec<EndianedBytesIO>(
        "bier.endianedbinaryio.C.EndianedBytesIO.EndianedBytesIO",
        EndianedBytesIO_init,
        EndianedBytesIO_dealloc,
        EndianedBytesIO_members,
        EndianedBytesIO_methods,
        EndianedBytesIO_repr);

static PyModuleDef EndianedBytesIO_module = {
    PyModuleDef_HEAD_INIT,
    "bier.endianedbinaryio.C.EndianedBytesIO", // Module name
    "",
    -1,   // Optional size of the module state memory
    NULL, // Optional table of module-level functions
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL  // Optional module deallocation function
};

static int add_object(PyObject *module, const char *name, PyObject *object)
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

PyMODINIT_FUNC PyInit_EndianedBytesIO(void)
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
