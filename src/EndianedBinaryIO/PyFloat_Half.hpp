#pragma once
#include <Python.h>
#include <cstdint>
/**
 * @brief Type definition for half-precision (16-bit) floating point values.
 *
 * Defined as 2 bytes for use with PyFloat_Unpack2.
 */
union half {
  uint16_t raw;
  uint8_t bytes[2];
};

/**
 * @brief Converts a half-precision float to a double-precision float.
 *
 * This function takes a half-precision float (16-bit) and converts it to a
 * double-precision float (64-bit).
 * It handles special cases like infinity and NaN according to the IEEE 754
 * standard. It's a clone of Python's PyFloat_Unpack2 implementation with minor
 * adjustments for C++.
 * @param data Reference to the half-precision float to convert.
 * @return double The converted double-precision float.
 */
double PyFloat_Unpack2(const half &data) noexcept;

/**
 * @brief Converts a double-precision float to a half-precision float.
 *
 * This function takes a double-precision float (64-bit) and converts it to a
 * half-precision float (16-bit).
 * It handles special cases like infinity and NaN according to the IEEE 754
 * standard. It's a clone of Python's PyFloat_Pack2 implementation with minor
 * adjustments for C++.
 *
 * @param x The double-precision float to convert.
 * @param data Reference to the half-precision float to store the result.
 * @return int 0 on success, -1 on overflow with an appropriate Python error
 * set.
 */
int PyFloat_Pack2(double x, half &data) noexcept;

/**
 * @brief Converts a half-precision float to a Python float object.
 *
 * This function converts a half-precision float (16-bit) to a Python float
 * object (which is a double-precision float, 64-bit).
 *
 * @param data Reference to the half-precision float to convert.
 * @return PyObject* Pointer to the new Python float object, or nullptr on error.
 */
inline PyObject *PyFloat_FromHalf(const half &data) noexcept {
  return PyFloat_FromDouble(PyFloat_Unpack2(data));
}

/**
 * @brief Converts a Python float object to a half-precision float.
 *
 * This function converts a Python float object (which is a double-precision
 * float, 64-bit) to a half-precision float (16-bit).
 * If the conversion fails (e.g., if the input is not a float), it returns a
 * half with raw value 0 and sets an appropriate Python error.
 *
 * @param obj Pointer to the Python object to convert.
 * @return half The converted half-precision float, or half{0} on error.
 */
inline half PyFloat_AsHalf(PyObject *obj) noexcept {
  double val = PyFloat_AsDouble(obj);
  if (PyErr_Occurred()) {
    return half{0};
  }
  half out{};
  if (PyFloat_Pack2(val, out) < 0) {
    return half{0};
  }
  return out;
}
