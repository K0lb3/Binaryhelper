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

#define GENERATE_ENDIANEDIOBASE_READ_FUNCTIONS(EndianedIOClass_read_t)                                                                \
    {"read_u8", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint8_t, '|'>), METH_NOARGS, "Read a uint8_t value."},           \
        {"read_u16", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint16_t, '|'>), METH_NOARGS, "Read a uint16_t value."},    \
        {"read_u32", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint32_t, '|'>), METH_NOARGS, "Read a uint32_t value."},    \
        {"read_u64", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint64_t, '|'>), METH_NOARGS, "Read a uint64_t value."},    \
        {"read_i8", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int8_t, '|'>), METH_NOARGS, "Read an int8_t value."},        \
        {"read_i16", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int16_t, '|'>), METH_NOARGS, "Read an int16_t value."},     \
        {"read_i32", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int32_t, '|'>), METH_NOARGS, "Read an int32_t value."},     \
        {"read_i64", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int64_t, '|'>), METH_NOARGS, "Read an int64_t value."},     \
        {"read_f16", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<half, '|'>), METH_NOARGS, "Read a half value."},            \
        {"read_f32", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<float, '|'>), METH_NOARGS, "Read a float value."},          \
        {"read_f64", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<double, '|'>), METH_NOARGS, "Read a double value."},        \
        {"read_u8_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint8_t, '<'>), METH_NOARGS, "Read a uint8_t value."},    \
        {"read_u16_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint16_t, '<'>), METH_NOARGS, "Read a uint16_t value."}, \
        {"read_u32_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint32_t, '<'>), METH_NOARGS, "Read a uint32_t value."}, \
        {"read_u64_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint64_t, '<'>), METH_NOARGS, "Read a uint64_t value."}, \
        {"read_i8_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int8_t, '<'>), METH_NOARGS, "Read an int8_t value."},     \
        {"read_i16_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int16_t, '<'>), METH_NOARGS, "Read an int16_t value."},  \
        {"read_i32_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int32_t, '<'>), METH_NOARGS, "Read an int32_t value."},  \
        {"read_i64_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int64_t, '<'>), METH_NOARGS, "Read an int64_t value."},  \
        {"read_f16_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<half, '<'>), METH_NOARGS, "Read a half value."},         \
        {"read_f32_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<float, '<'>), METH_NOARGS, "Read a float value."},       \
        {"read_f64_le", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<double, '<'>), METH_NOARGS, "Read a double value."},     \
        {"read_u8_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint8_t, '>'>), METH_NOARGS, "Read a uint8_t value."},    \
        {"read_u16_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint16_t, '>'>), METH_NOARGS, "Read a uint16_t value."}, \
        {"read_u32_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint32_t, '>'>), METH_NOARGS, "Read a uint32_t value."}, \
        {"read_u64_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<uint64_t, '>'>), METH_NOARGS, "Read a uint64_t value."}, \
        {"read_i8_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int8_t, '>'>), METH_NOARGS, "Read an int8_t value."},     \
        {"read_i16_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int16_t, '>'>), METH_NOARGS, "Read an int16_t value."},  \
        {"read_i32_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int32_t, '>'>), METH_NOARGS, "Read an int32_t value."},  \
        {"read_i64_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<int64_t, '>'>), METH_NOARGS, "Read an int64_t value."},  \
        {"read_f16_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<half, '>'>), METH_NOARGS, "Read a half value."},         \
        {"read_f32_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<float, '>'>), METH_NOARGS, "Read a float value."},       \
        {"read_f64_be", reinterpret_cast<PyCFunction>(EndianedIOClass_read_t<double, '>'>), METH_NOARGS, "Read a double value."}

#define GENERATE_ENDIANEDIOBASE_READ_ARRAY_FUNCTIONS(EndianedBytesIO_read_array_t)                                                           \
    {"read_u8_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint8_t, '|'>), METH_O, "Read a uint8_t array."},           \
        {"read_u16_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint16_t, '|'>), METH_O, "Read a uint16_t array."},    \
        {"read_u32_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint32_t, '|'>), METH_O, "Read a uint32_t array."},    \
        {"read_u64_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint64_t, '|'>), METH_O, "Read a uint64_t array."},    \
        {"read_i8_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int8_t, '|'>), METH_O, "Read an int8_t array."},        \
        {"read_i16_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int16_t, '|'>), METH_O, "Read an int16_t array."},     \
        {"read_i32_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int32_t, '|'>), METH_O, "Read an int32_t array."},     \
        {"read_i64_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int64_t, '|'>), METH_O, "Read an int64_t array."},     \
        {"read_f16_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<half, '|'>), METH_O, "Read a half array."},            \
        {"read_f32_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<float, '|'>), METH_O, "Read a float array."},          \
        {"read_f64_array", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<double, '|'>), METH_O, "Read a double array."},        \
        {"read_u8_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint8_t, '<'>), METH_O, "Read a uint8_t array."},    \
        {"read_u16_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint16_t, '<'>), METH_O, "Read a uint16_t array."}, \
        {"read_u32_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint32_t, '<'>), METH_O, "Read a uint32_t array."}, \
        {"read_u64_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint64_t, '<'>), METH_O, "Read a uint64_t array."}, \
        {"read_i8_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int8_t, '<'>), METH_O, "Read an int8_t array."},     \
        {"read_i16_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int16_t, '<'>), METH_O, "Read an int16_t array."},  \
        {"read_i32_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int32_t, '<'>), METH_O, "Read an int32_t array."},  \
        {"read_i64_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int64_t, '<'>), METH_O, "Read an int64_t array."},  \
        {"read_f16_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<half, '<'>), METH_O, "Read a half array."},         \
        {"read_f32_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<float, '<'>), METH_O, "Read a float array."},       \
        {"read_f64_array_le", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<double, '<'>), METH_O, "Read a double array."},     \
        {"read_u8_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint8_t, '>'>), METH_O, "Read a uint8_t array."},    \
        {"read_u16_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint16_t, '>'>), METH_O, "Read a uint16_t array."}, \
        {"read_u32_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint32_t, '>'>), METH_O, "Read a uint32_t array."}, \
        {"read_u64_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<uint64_t, '>'>), METH_O, "Read a uint64_t array."}, \
        {"read_i8_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int8_t, '>'>), METH_O, "Read an int8_t array."},     \
        {"read_i16_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int16_t, '>'>), METH_O, "Read an int16_t array."},  \
        {"read_i32_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int32_t, '>'>), METH_O, "Read an int32_t array."},  \
        {"read_i64_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<int64_t, '>'>), METH_O, "Read an int64_t array."},  \
        {"read_f16_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<half, '>'>), METH_O, "Read a half array."},         \
        {"read_f32_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<float, '>'>), METH_O, "Read a float array."},       \
        {"read_f64_array_be", reinterpret_cast<PyCFunction>(EndianedBytesIO_read_array_t<double, '>'>), METH_O, "Read a double array."}

// template<PyCFunction init, PyCFunction dealloc, PyCFunction repr, PyMemberDef members[], PyMethodDef methods[]>
// (PyType_Slot){
//     {Py_tp_new, reinterpret_cast<void *>(PyType_GenericNew)},
//     {Py_tp_init, reinterpret_cast<void *>(init)},
//     {Py_tp_dealloc, reinterpret_cast<void *>(dealloc)},
//     {Py_tp_members, EndianedBytesIO_members},
//     {Py_tp_methods, EndianedBytesIO_methods},
//     {Py_tp_repr, reinterpret_cast<void *>(EndianedBytesIO_repr)},
//     {0, NULL},
// }

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

// Example specialization for EndianedBytesIO
// struct EndianedBytesIO_Config {
//     static constexpr const char* name = "EndianedBytesIO";
//     static constexpr const char* module_name = "bier.endianedbinaryio._EndianedBytesIO";
// };

// using EndianedBytesIO_Spec = EndianedBytesIO_TypeSpec<EndianedBytesIO_Config>;