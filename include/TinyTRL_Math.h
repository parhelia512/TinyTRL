/*
 * This file is part of Tiny Template and Runtime Library (TinyTRL).
 * Copyright (c) 2024 Yuriy Kotsarenko. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and limitations under the License.
 */

// TinyTRL_Math.h
#pragma once

#include "TinyTRL_TypeDef.h"

namespace trl {
namespace math {

// Non-template functions.

// Basic mathematic functions.

// Computes a floating-point remainder of the division operation x / y.
extern float fmod(float x, float y) noexcept;

// Computes a floating-point remainder of the division operation x / y.
extern double fmod(double x, double y) noexcept;

/// Calculates fused multiply add "(x * y) + z".
extern float fma(float x, float y, float z) noexcept;

/// Calculates fused multiply add "(x * y) + z".
extern double fma(double x, double y, double z) noexcept;

} // namespace math
namespace utility {

// Floating-point conversion functions.

/// Returns 32-bit floating-point value bits encompassed in 32-bit unsigned integer.
extern uint32_t floatBitsAsUint(float value);

/// Extracts 32-bit floating-point value stored in bits as 32-bit unsigned integer.
extern float uintBitsAsFloat(uint32_t value);

/// Returns 64-bit floating-point value bits encompassed in 64-bit unsigned integer.
extern uint64_t doubleBitsAsUint(double value);

/// Extracts 64-bit floating-point value stored in bits as 64-bit unsigned integer.
extern double uintBitsAsDouble(uint64_t value);

/// Writes NAN to the specified 32-bit floating-point variable.
extern void assignNAN(float& dest);

/// Tests whether a given 32-bit floating-point number is NAN or infinity.
extern bool isInfinityOrNAN(float value);

/// Tests whether a given number has value of FLT_MAX or -FLT_MAX.
extern bool isPlusOrMinusMaxFloat(float value);

/// Writes NAN to the specified 64-bit floating-point variable.
extern void assignNAN(double& dest);

/// Tests whether a given 64-bit floating-point number is NAN or infinity.
extern bool isInfinityOrNAN(double value);

} // namespace utility
namespace math {

// Misc functions.

/// Calculates integer value of log base two.
#ifdef __PLATFORM_X64
  extern int32_t log2(uint64_t value) noexcept;
#else
  extern int32_t log2(uint32_t value) noexcept;
#endif

/// Calculates an average of two unsigned values without overflow.
extern size_t average(size_t value1, size_t value2) noexcept;

// Function declaration.

// Common mathematical functions.

/// Returns the largest of two given parameters.
template<typename ValueType>
ValueType constexpr const& max(ValueType const& left, ValueType const& right) noexcept;

/// Returns the smallest of two given parameters.
template<typename ValueType>
ValueType constexpr const& min(ValueType const& value1, ValueType const& value2) noexcept;

/// Ensures that given value stays within the specified range limit, clamping it if necessary.
template<typename ValueType>
ValueType constexpr const& saturate(ValueType const& value, ValueType const& minLimit,
  ValueType const& maxLimit) noexcept;

/// Returns an absolute value from the given one.
template<typename ValueType>
ValueType constexpr abs(ValueType const& value) noexcept;

/// Tests whether two values are nearly equal.
template<typename ValueType>
constexpr bool nearlyEqual(ValueType left, ValueType right, ValueType epsilon);

/// Tests whether a given value is nearly zero.
template<typename ValueType>
constexpr bool nearlyZero(ValueType value, ValueType epsilon);

/// Elevates given number to square; in other words, returns (value * value).
template<typename ValueType>
ValueType constexpr sqr(ValueType value);

/// Calculates an average of two values without overflow (applies only to integers).
template<typename ValueType>
ValueType constexpr average(ValueType value1, ValueType value2);

/// Returns an integer sign of the given value.
template<typename ValueType>
int constexpr signum(ValueType value) noexcept;

// Interpolation utility functions.

/// Interpolates between two values linearly.
template<typename ValueType>
ValueType constexpr lerp(ValueType value1, ValueType value2, ValueType theta);

// Power of two utility functions.

/// Tests whether given value is a power of two (e.g. 2, 4, 8, 16, ..., 256, 512 and so on).
template<typename ValueType>
bool constexpr isPowerOfTwo(ValueType value) noexcept;

/// Returns a greatest power of two that is lesser or equal to the specified value.
template<typename ValueType>
ValueType constexpr floorPowerOfTwo(ValueType value) noexcept;

/// Returns a least power of two that is greater or equal to the specified value. If next power of two
/// value cannot be represented in the specialized data type, then the behavior of function is undefined.
template<typename ValueType>
ValueType constexpr ceilPowerOfTwo(ValueType value) noexcept;

// Calculates next buffer capacity from the current capacity to ensure semi-exponential buffer growth.
// The returned capacity is always bigger than the given one.
template<typename ValueType>
ValueType constexpr computeNextCapacity(ValueType capacity) noexcept;

// Calculates next buffer capacity based on current and requested capacities, to ensure semi-exponential
// buffer growth. The required capacity must be bigger than current one.
template<typename ValueType>
ValueType constexpr computeNextCapacity(ValueType capacity, ValueType currentCapacity,
  ValueType initialCapacity = 0) noexcept;

} // namespace math
} // namespace trl

#include "TinyTRL_Math.inl"