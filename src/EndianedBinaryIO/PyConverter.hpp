/**
 * @file PyConverter.hpp
 * @brief Provides utilities for converting C++ types to Python objects.
 *
 * This header provides functionality to convert various C++ data types
 * to their Python equivalents using the Python C API. It supports integral types,
 * floating point types, and a custom half-precision float type.
 */
#pragma once
#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <Python.h>
#include "./PyFloat_Half.hpp"

constexpr bool IS_BIG_ENDIAN_SYSTEM = (std::endian::native == std::endian::big);

// A single concept for the scalar types this module supports
template <typename T>
concept EndianedSupportedType =
    std::is_integral_v<std::remove_cvref_t<T>> ||
    std::is_floating_point_v<std::remove_cvref_t<T>> ||
    std::is_same_v<std::remove_cvref_t<T>, half> ||
    std::is_same_v<std::remove_cvref_t<T>, bool>;

// Concept for valid endian-aware operations
template <typename T, char endian>
concept EndianedOperation = EndianedSupportedType<T> &&
                            (endian == '<' || endian == '>' || endian == '|');

template <typename T>
    requires EndianedSupportedType<T> ||
             std::is_trivially_copyable_v<T>
constexpr T byteswap(const T value) noexcept
{
    if constexpr (std::is_integral_v<T>)
    {
        return std::byteswap(value);
    }
    else if constexpr (sizeof(T) == 2)
    {
        return std::bit_cast<T>(std::byteswap(std::bit_cast<uint16_t>(value)));
    }
    else if constexpr (sizeof(T) == 4)
    {
        return std::bit_cast<T>(std::byteswap(std::bit_cast<uint32_t>(value)));
    }
    else if constexpr (sizeof(T) == 8)
    {
        return std::bit_cast<T>(std::byteswap(std::bit_cast<uint64_t>(value)));
    }
    else
    {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "Unsupported type size for byteswap.");
        return value;
    }
}

/**
 * @brief Converts a C++ value to a Python object.
 *
 * This template function converts various C++ types to their equivalent Python
 * objects using the appropriate Python C API functions:
 * - Unsigned integers: PyLong_FromUnsignedLong or PyLong_FromUnsignedLongLong
 * - Signed integers: PyLong_FromLong or PyLong_FromLongLong
 * - Half-precision floats: PyFloat_Unpack2 + PyFloat_FromDouble
 * - Single and double precision floats: PyFloat_FromDouble
 *
 * @tparam T The C++ type to convert (must be integral, floating-point, or half)
 * @param value Reference to the value to convert
 * @return PyObject* A new reference to a Python object representing the value
 * @note The caller is responsible for managing the reference returned
 * @warning For half-precision values, no error checking is performed on PyFloat_Unpack2
 */
template <typename T>
    requires EndianedSupportedType<T>
inline PyObject *PyObject_FromAny(T &value)
{
    using std::is_same_v;
    if constexpr (is_same_v<T, uint8_t> || is_same_v<T, uint16_t> || is_same_v<T, uint32_t>)
    {
        return PyLong_FromUnsignedLong(static_cast<uint32_t>(value));
    }
    else if constexpr (is_same_v<T, uint64_t>)
    {
        return PyLong_FromUnsignedLongLong(value);
    }
    else if constexpr (is_same_v<T, int8_t> || is_same_v<T, int16_t> || is_same_v<T, int32_t>)
    {
        return PyLong_FromLong(static_cast<int32_t>(value));
    }
    else if constexpr (is_same_v<T, int64_t>)
    {
        return PyLong_FromLongLong(value);
    }
    else if constexpr (is_same_v<T, half>)
    {
        return PyFloat_FromHalf(value);
    }
    else if constexpr (is_same_v<T, float>)
    {
        return PyFloat_FromDouble(static_cast<double>(value));
    }
    else if constexpr (is_same_v<T, double>)
    {
        return PyFloat_FromDouble(value);
    }
    else if constexpr (is_same_v<T, bool>)
    {
        return PyBool_FromLong(static_cast<long>(value));
    }
}

/**
 * @brief Converts a Python object to a C++ value.
 *
 * This template function converts a Python object to its equivalent C++ type:
 * - Unsigned integers: PyLong_AsUnsignedLong or PyLong_AsUnsignedLongLong
 * - Signed integers: PyLong_AsLong or PyLong_AsLongLong
 * - Half-precision floats: PyFloat_AsDouble + conversion to half
 * - Single and double precision floats: PyFloat_AsDouble
 *
 * @tparam T The C++ type to convert to (must be integral, floating-point, or half)
 * @param obj Pointer to the Python object
 * @param out Reference to the value to set
 * @return true if conversion succeeded, false otherwise
 */
template <typename T>
    requires EndianedSupportedType<T>
inline bool PyObject_ToAny(PyObject *obj, T &out)
{
    using std::is_same_v;
    if constexpr (is_same_v<T, uint8_t> || is_same_v<T, uint16_t> || is_same_v<T, uint32_t>)
    {
        unsigned long val = PyLong_AsUnsignedLong(obj);
        if (PyErr_Occurred())
            return false;
        out = static_cast<T>(val);
        return true;
    }
    else if constexpr (is_same_v<T, uint64_t>)
    {
        unsigned long long val = PyLong_AsUnsignedLongLong(obj);
        if (PyErr_Occurred())
            return false;
        out = static_cast<T>(val);
        return true;
    }
    else if constexpr (is_same_v<T, int8_t> || is_same_v<T, int16_t> || is_same_v<T, int32_t>)
    {
        long val = PyLong_AsLong(obj);
        if (PyErr_Occurred())
            return false;
        out = static_cast<T>(val);
        return true;
    }
    else if constexpr (is_same_v<T, int64_t>)
    {
        long long val = PyLong_AsLongLong(obj);
        if (PyErr_Occurred())
            return false;
        out = static_cast<T>(val);
        return true;
    }
    else if constexpr (is_same_v<T, half>)
    {
        out = PyFloat_AsHalf(obj);
        if (PyErr_Occurred())
            return false;
        return true;
    }
    else if constexpr (is_same_v<T, float>)
    {
        double val = PyFloat_AsDouble(obj);
        if (PyErr_Occurred())
            return false;
        out = static_cast<float>(val);
        return true;
    }
    else if constexpr (is_same_v<T, double>)
    {
        double val = PyFloat_AsDouble(obj);
        if (PyErr_Occurred())
            return false;
        out = val;
        return true;
    }
    else if constexpr (is_same_v<T, bool>)
    {
        long val = PyObject_IsTrue(obj);
        if (val == -1)
            return false;
        out = static_cast<bool>(val);
        return true;
    }
    return false;
}

template <typename T>
concept EndianedIOHandler =
    requires(T value) {
        { value.endian } -> std::same_as<char &>;
    };

template <typename EI, typename T, char endian>
    requires(
        EndianedIOHandler<EI> &&
        EndianedOperation<T, endian>)
static inline void handle_swap(EI *self, T &value)
{
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
}
