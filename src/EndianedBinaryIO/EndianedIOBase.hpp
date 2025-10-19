#pragma once
#include "PyConverter.hpp"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define f16 half
#define f32 float
#define f64 double

#define CHECK_CLOSED                                                        \
    if (self->closed)                                                       \
    {                                                                       \
        PyErr_SetString(PyExc_ValueError, "I/O operation on closed file."); \
        return nullptr;                                                     \
    }

#define CHECK_SIZE_ARG(arg, size, default)                                        \
    if (PyLong_Check(arg))                                                        \
    {                                                                             \
        size = PyLong_AsSize_t(arg);                                              \
    }                                                                             \
    else if (arg == Py_None)                                                      \
    {                                                                             \
        size = default;                                                           \
    }                                                                             \
    else                                                                          \
    {                                                                             \
        PyErr_SetString(PyExc_TypeError, "Argument must be an integer or None."); \
        return nullptr;                                                           \
    }

#define _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, T)                                                                 \
    {"read_" #T, reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<T, '|'>), METH_NOARGS, "Read a " #T " value."},                  \
        {"read_" #T "_le", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<T, '<'>), METH_NOARGS, "Read a " #T " value."},        \
        {"read_" #T "_be", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<T, '>'>), METH_NOARGS, "Read a " #T " value."},        \
        {"read_" #T "_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<T, '|'>), METH_O, "Read a " #T " array."},    \
        {"read_" #T "_le_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<T, '<'>), METH_O, "Read a " #T " array."}, \
        {"read_" #T "_be_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<T, '>'>), METH_O, "Read a " #T " array."}

#define GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedIOClass)                                                                                         \
    {"read_bool", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<bool, '|'>), METH_NOARGS, "Read a bool value."},                               \
        {"read_bool_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<bool, '|'>), METH_O, "Read a bool array."},                    \
        {"read_u8", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<u8, '|'>), METH_NOARGS, "Read a u8 value."},                                 \
        {"read_u8_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<u8, '|'>), METH_O, "Read a u8 array."},                          \
        {"read_i8", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_t<i8, '|'>), METH_NOARGS, "Read an i8 value."},                                \
        {"read_i8_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_array_t<i8, '|'>), METH_O, "Read a i8 array."},                          \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, u16),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, u32),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, u64),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, i16),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, i32),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, i64),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, f16),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, f32),                                                                             \
        _GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS_TYPE(EndianedIOClass, f64),                                                                             \
        {"read_cstring", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_cstring), METH_VARARGS | METH_KEYWORDS, "Read until a null terminator."}, \
        {"read_string", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_string), METH_VARARGS | METH_KEYWORDS, "Read a string."},                  \
        {"read_bytes", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_bytes), METH_O, "Read a byte array."},                                      \
        {"read_varint", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_varint), METH_NOARGS, "Read a variable-length integer."},                  \
        {"read_varint_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_read_varint_array), METH_O, "Read a variable-length integer array."}

#define GENERATE_ENDIANEDIOBASE_BASE_FUNCTIONS(EndianedIOClass)                                                                          \
    {"read", reinterpret_cast<PyCFunction>(EndianedIOClass##_read), METH_O, "Read bytes from the buffer."},                              \
        {"readinto", reinterpret_cast<PyCFunction>(EndianedIOClass##_readinto), METH_O, "Read bytes into a buffer."},                    \
        {"seek", reinterpret_cast<PyCFunction>(EndianedIOClass##_seek), METH_VARARGS, "Seek to a position in the buffer."},              \
        {"tell", reinterpret_cast<PyCFunction>(EndianedIOClass##_tell), METH_NOARGS, "Get the current position in the buffer."},         \
        {"flush", reinterpret_cast<PyCFunction>(EndianedIOClass##_flush), METH_NOARGS, "Flush the buffer."},                             \
        {"fileno", reinterpret_cast<PyCFunction>(EndianedIOClass##_fileno), METH_NOARGS, "Get the file descriptor."},                    \
        {"isatty", reinterpret_cast<PyCFunction>(EndianedIOClass##_isatty), METH_NOARGS, "Check if the buffer is a TTY."},               \
        {"close", reinterpret_cast<PyCFunction>(EndianedIOClass##_close), METH_NOARGS, "Close the buffer."},                             \
        {"readable", reinterpret_cast<PyCFunction>(EndianedIOClass##_readable), METH_NOARGS, "Check if the buffer is readable."},        \
        {"writable", reinterpret_cast<PyCFunction>(EndianedIOClass##_writable), METH_NOARGS, "Check if the buffer is writable."},        \
        {"seekable", reinterpret_cast<PyCFunction>(EndianedIOClass##_seekable), METH_NOARGS, "Check if the buffer is seekable."},        \
        {"readline", reinterpret_cast<PyCFunction>(EndianedIOClass##_readline), METH_VARARGS, "Read a line from the buffer."},           \
        {"readlines", reinterpret_cast<PyCFunction>(EndianedIOClass##_readlines), METH_VARARGS, "Read multiple lines from the buffer."}, \
        {"align", reinterpret_cast<PyCFunction>(EndianedIOClass##_align), METH_O, "Align the position of the buffer."},                  \
        {"write", reinterpret_cast<PyCFunction>(EndianedIOClass##_write), METH_O, "Write bytes to the buffer."},                         \
        {"writelines", reinterpret_cast<PyCFunction>(EndianedIOClass##_writelines), METH_O, "Write multiple lines to the buffer."}

#define _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, T)                                                                                         \
    {"write_" #T, reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<T, '|'>), METH_O, "Write a " #T " value."},                                             \
        {"write_" #T "_le", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<T, '<'>), METH_O, "Write a " #T " value."},                                   \
        {"write_" #T "_be", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<T, '>'>), METH_O, "Write a " #T " value."},                                   \
        {"write_" #T "_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<T, '|'>), METH_VARARGS | METH_KEYWORDS, "Write a " #T " array."},    \
        {"write_" #T "_le_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<T, '<'>), METH_VARARGS | METH_KEYWORDS, "Write a " #T " array."}, \
        {"write_" #T "_be_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<T, '>'>), METH_VARARGS | METH_KEYWORDS, "Write a " #T " array."}

#define GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS(EndianedIOClass)                                                                                              \
    {"write_bool", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<bool, '|'>), METH_O, "Write a bool value."},                                       \
        {"write_bool_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<bool, '|'>), METH_VARARGS | METH_KEYWORDS, "Write a bool array."}, \
        {"write_u8", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<u8, '|'>), METH_O, "Write a u8 value."},                                         \
        {"write_u8_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<u8, '|'>), METH_VARARGS | METH_KEYWORDS, "Write a u8 array."},       \
        {"write_i8", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_t<i8, '|'>), METH_O, "Write an i8 value."},                                        \
        {"write_i8_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_array_t<i8, '|'>), METH_VARARGS | METH_KEYWORDS, "Write a i8 array."},       \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, u16),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, u32),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, u64),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, i16),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, i32),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, i64),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, f16),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, f32),                                                                                  \
        _GENERATE_ENDIANEDIOBASE_WRITE_FUNCTIONS_TYPE(EndianedIOClass, f64),                                                                                  \
        {"write_cstring", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_cstring), METH_VARARGS | METH_KEYWORDS, "Write a C-style string."},           \
        {"write_string", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_string), METH_VARARGS | METH_KEYWORDS, "Write a string."},                     \
        {"write_bytes", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_bytes), METH_O, "Write a byte array."},                                         \
        {"write_varint", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_varint), METH_O, "Write a variable-length integer."},                          \
        {"write_varint_array", reinterpret_cast<PyCFunction>(EndianedIOClass##_write_varint_array), METH_VARARGS | METH_KEYWORDS, "Write a variable-length integer array."}

template <typename T>
concept EndianedIOConfig = requires {
    { T::name } -> std::convertible_to<const char *>;
    { T::init } -> std::same_as<PyCFunction>;
    { T::dealloc } -> std::same_as<PyCFunction>;
    { T::repr } -> std::same_as<PyCFunction>;
    { T::members } -> std::same_as<PyMemberDef *>;
    { T::methods } -> std::same_as<PyMethodDef *>;
    { T::basicsize } -> std::same_as<int>;
};

template <typename T>
static PyType_Spec createPyTypeSpec(
    const char *name,
    int (*initFunc)(T *, PyObject *, PyObject *),
    void (*deallocFunc)(T *),
    PyMemberDef *members,
    PyMethodDef *methods,
    PyObject *(*reprFunc)(T *))
{
    static PyType_Slot slots[] = {
        {Py_tp_new, reinterpret_cast<void *>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void *>(initFunc)},
        {Py_tp_dealloc, reinterpret_cast<void *>(deallocFunc)},
        {Py_tp_members, members},
        {Py_tp_methods, methods},
        {Py_tp_repr, reinterpret_cast<void *>(reprFunc)},
        {0, NULL},
    };

    return PyType_Spec{
        name,
        sizeof(T),
        0,
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        slots,
    };
}

static inline bool EndianedIOBase_write_count(PyObject *self, Py_ssize_t size)
{
    PyObject *res = PyObject_CallMethod(
        self,
        "write_count",
        "n",
        size,
        nullptr);

    if (res == nullptr)
    {
        return true; // Resize failed
    }
    Py_DecRef(res);
    return false; // Resize succeeded
}

template <typename T>
static inline bool EndianedIOBase_write_count(T *self, Py_ssize_t size)
{
    return EndianedIOBase_write_count(reinterpret_cast<PyObject *>(self), size);
}
