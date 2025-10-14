// PYTHON SOFTWARE FOUNDATION LICENSE VERSION 2
// --------------------------------------------

// 1. This LICENSE AGREEMENT is between the Python Software Foundation
// ("PSF"), and the Individual or Organization ("Licensee") accessing and
// otherwise using this software ("Python") in source or binary form and
// its associated documentation.

// 2. Subject to the terms and conditions of this License Agreement, PSF hereby
// grants Licensee a nonexclusive, royalty-free, world-wide license to
// reproduce, analyze, test, perform and/or display publicly, prepare derivative
// works, distribute, and otherwise use Python alone or in any derivative
// version, provided, however, that PSF's License Agreement and PSF's notice of
// copyright, i.e., "Copyright (c) 2001 Python Software Foundation; All Rights
// Reserved" are retained in Python alone or in any derivative version prepared
// by Licensee.

// 3. In the event Licensee prepares a derivative work that is based on
// or incorporates Python or any part thereof, and wants to make
// the derivative work available to others as provided herein, then
// Licensee hereby agrees to include in any such work a brief summary of
// the changes made to Python.

// 4. PSF is making Python available to Licensee on an "AS IS"
// basis.  PSF MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR
// IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, PSF MAKES NO AND
// DISCLAIMS ANY REPRESENTATION OR WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF PYTHON WILL NOT
// INFRINGE ANY THIRD PARTY RIGHTS.

// 5. PSF SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF PYTHON
// FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS AS
// A RESULT OF MODIFYING, DISTRIBUTING, OR OTHERWISE USING PYTHON,
// OR ANY DERIVATIVE THEREOF, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

// 6. This License Agreement will automatically terminate upon a material
// breach of its terms and conditions.

// 7. Nothing in this License Agreement shall be deemed to create any
// relationship of agency, partnership, or joint venture between PSF and
// Licensee.  This License Agreement does not grant permission to use PSF
// trademarks or trade name in a trademark sense to endorse or promote
// products or services of Licensee, or any third party.

// 8. By copying, installing or otherwise using Python, Licensee
// agrees to be bound by the terms and conditions of this License
// Agreement.

#include "./PyFloat_Half.hpp"
#include <Python.h>
#include <bit>
#include <cassert>
#include <cmath> // frexp, ldexp, copysign, isinf, isnan

static_assert(sizeof(half) == 2, "half must be 2 bytes");

double PyFloat_Unpack2(const half &data) noexcept {
  const auto &p = data.bytes[1];
  const bool sign = ((p >> 7) & 1) != 0;
  const uint32_t frac = (static_cast<uint32_t>(p & 0x03) << 8) |
                        static_cast<uint32_t>(data.bytes[0]);
  int32_t exp = static_cast<int32_t>((p & 0x7C) >> 2);
  double x{};

  if (exp == 0x1f) {
    if (frac == 0) {
      /* Infinity */
      return sign ? -Py_INFINITY : Py_INFINITY;
    } else {
      /* NaN */
      uint64_t v = sign ? 0xfff0000000000000ULL : 0x7ff0000000000000ULL;

      v += static_cast<uint64_t>(frac) << 42; /* add NaN's type & payload */
      return std::bit_cast<double>(v);
    }
  }

  x = static_cast<double>(frac) / 1024.0;

  if (exp == 0) {
    exp = -14;
  } else {
    x += 1.0;
    exp -= 15;
  }
  x = std::ldexp(x, exp);

  if (sign) {
    x = -x;
  }

  return x;
}

int PyFloat_Pack2(double x, half &data) noexcept {
  bool sign{};
  int32_t exp{};
  double frac{};
  uint16_t bits{};

  if (x == 0.0) {
    sign = (std::copysign(1.0, x) == -1.0);
    exp = 0;
    bits = 0;
  } else if (std::isinf(x)) {
    sign = (x < 0.0);
    exp = 0x1f;
    bits = 0;
  } else if (std::isnan(x)) {
    sign = (std::copysign(1.0, x) == -1.0);
    exp = 0x1f;
    uint64_t v = std::bit_cast<uint64_t>(x);
    v &= 0xffc0000000000ULL;
    bits = static_cast<uint16_t>(v >> 42); /* NaN's type & payload */
  } else {
    sign = (x < 0.0);
    if (sign) {
      x = -x;
    }

    frac = std::frexp(x, &exp);
    if (frac < 0.5 || frac >= 1.0) {
      PyErr_SetString(PyExc_SystemError, "frexp() result out of range");
      return -1;
    }

    /* Normalize frac to be in the range [1.0, 2.0) */
    frac *= 2.0;
    exp--;

    if (exp >= 16) {
      goto Overflow;
    } else if (exp < -25) {
      /* |x| < 2**-25. Underflow to zero. */
      frac = 0.0;
      exp = 0;
    } else if (exp < -14) {
      /* |x| < 2**-14. Gradual underflow */
      frac = std::ldexp(frac, 14 + exp);
      exp = 0;
    } else /* if (!(exp == 0 && frac == 0.0)) */ {
      exp += 15;
      frac -= 1.0; /* Get rid of leading 1 */
    }

    frac *= 1024.0; /* 2**10 */
    /* Round to even */
    bits = static_cast<uint16_t>(frac); /* Note the truncation */
    assert(bits < 1024);
    assert(exp < 31);
    if ((frac - bits > 0.5) || ((frac - bits == 0.5) && (bits % 2 == 1))) {
      ++bits;
      if (bits == 1024) {
        /* The carry propagated out of a string of 10 1 bits. */
        bits = 0;
        ++exp;
        if (exp == 31)
          goto Overflow;
      }
    }
  }

  bits |= (exp << 10) | (sign << 15);

  /* Write out result. */
  /* First byte */
  data.bytes[1] = static_cast<uint8_t>((bits >> 8) & 0xFF);
  /* Second byte */
  data.bytes[0] = static_cast<uint8_t>(bits & 0xFF);

  return 0;

Overflow:
  PyErr_SetString(PyExc_OverflowError, "float too large to pack with e format");
  return -1;
}
