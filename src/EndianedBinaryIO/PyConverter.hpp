/**
 * @file PyConverter.hpp
 * @brief Provides utilities for converting C++ types to Python objects.
 *
 * This header provides functionality to convert various C++ data types
 * to their Python equivalents using the Python C API. It supports integral types,
 * floating point types, and a custom half-precision float type.
 */

#include <bit>
#include <concepts>
#include <cstdint>
#include <Python.h>


constexpr bool IS_BIG_ENDIAN_SYSTEM = (std::endian::native == std::endian::big);

/**
 * @brief Type definition for half-precision (16-bit) floating point values.
 *
 * Defined as 2 bytes for use with PyFloat_Unpack2.
 */
struct half
{
    uint16_t value; // 16-bit representation of the half-precision float
};

template <class _Ty>
    requires
        std::is_integral_v<_Ty> ||
        std::is_floating_point_v<_Ty> ||
        std::is_same_v<_Ty, half> ||
        std::is_trivially_copyable_v<_Ty>
constexpr _Ty byteswap(const _Ty _Val) noexcept
{
    if constexpr (std::is_integral_v<_Ty>)
    {
        return std::byteswap(_Val);
    }
    else if constexpr (sizeof(_Ty) == 2)
    {
        return std::bit_cast<_Ty>(std::byteswap(std::bit_cast<uint16_t>(_Val)));
    }
    else if constexpr (sizeof(_Ty) == 4)
    {
        return std::bit_cast<_Ty>(std::byteswap(std::bit_cast<uint32_t>(_Val)));
    }
    else if constexpr (sizeof(_Ty) == 8)
    {
        return std::bit_cast<_Ty>(std::byteswap(std::bit_cast<uint64_t>(_Val)));
    }
    else
    {
        static_assert(sizeof(_Ty) == 2 || sizeof(_Ty) == 4 || sizeof(_Ty) == 8, "Unsupported type size for byteswap.");
    }
}

template <typename T>
concept EndianedReadable =
    std::is_integral_v<T> ||
    std::is_floating_point_v<T> ||
    std::is_same_v<T, half> ||
    requires(T value) {
        { byteswap(value) } -> std::same_as<T>;
        { PyObject_FromAny(value) } -> std::convertible_to<PyObject *>;
    };


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
    requires std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, half>
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
        double unpacked = PyFloat_Unpack2((const char *)&value, 1);
        if (unpacked == -1.0 && PyErr_Occurred())
        {
            return nullptr; // Return nullptr if unpacking fails
        }
        return PyFloat_FromDouble(unpacked);
    }
    else if constexpr (is_same_v<T, float>)
    {
        return PyFloat_FromDouble(static_cast<double>(value));
    }
    else if constexpr (is_same_v<T, double>)
    {
        return PyFloat_FromDouble(value);
    }
}
