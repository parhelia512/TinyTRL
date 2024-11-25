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

// TinyTRL_Math.inl
#pragma once

#include "TinyTRL_Math.h"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif
#ifndef M_2_PI
  #define M_2_PI 6.28318530717958647703
#endif

namespace trl {
namespace math {

// Common mathematical functions.

template<typename ValueType>
ValueType constexpr const& max(ValueType const& left, ValueType const& right) noexcept
{
  return left < right ? right : left;
}

template<typename ValueType>
ValueType constexpr const& min(ValueType const& value1, ValueType const& value2) noexcept
{
  return value1 <= value2 ? value1 : value2;
}

template<typename ValueType>
ValueType constexpr const& saturate(ValueType const& value, ValueType const& minLimit,
  ValueType const& maxLimit) noexcept
{
  ValueType const& temp = value > minLimit ? value : minLimit;
  return temp < maxLimit ? temp : maxLimit;
}

template<typename ValueType>
ValueType constexpr abs(ValueType const& value) noexcept
{
  return value >= static_cast<ValueType>(0) ? value : -value;
}

/// Tests whether two values are nearly equal.
template<typename ValueType>
constexpr bool nearlyEqual(ValueType const left, ValueType const right, ValueType const epsilon)
{
  return left == right || abs(left - right) <= max(abs(left), abs(right)) * epsilon;
}

/// Tests whether a given value is nearly zero.
template<typename ValueType>
constexpr bool nearlyZero(ValueType value, ValueType const epsilon)
{
  return abs(value) <= epsilon;
}

template<typename ValueType>
ValueType constexpr sqr(ValueType const value)
{
  return value * value;
}

template<typename ValueType>
ValueType constexpr average(ValueType const value1, ValueType const value2)
{
  return (value1 / 2) + (value2 / 2) + (((value1 % 2) + (value2 % 2)) / 2);
}

template<>
float constexpr average(float const value1, float const value2)
{
  return value1 * 0.5f + value2 * 0.5f;
}

template<>
double constexpr average(double const value1, double const value2)
{
  return value1 * 0.5 + value2 * 0.5;
}

template<typename ValueType>
int constexpr signum(ValueType value) noexcept
{
  return (static_cast<ValueType>(0) < value) - (value < static_cast<ValueType>(0));
}

// Interpolation functions.

template<typename ValueType>
ValueType constexpr lerp(ValueType const value1, ValueType const value2, ValueType const theta)
{
  // Note: due to precision issues, when theta = 1, this may not necesarily produce exactly "value2".
  return value1 + (value2 - value1) * theta;
}

// Power of two functions.

template<typename ValueType>
bool constexpr isPowerOfTwo(ValueType const value) noexcept
{
  return (value >= static_cast<ValueType>(1)) && !(value & (value - static_cast<ValueType>(1)));
}

template<typename ValueType>
ValueType constexpr floorPowerOfTwo(ValueType const value) noexcept
{
  ValueType tempValue = value;

  if (tempValue > static_cast<ValueType>(2))
    while (!isPowerOfTwo(tempValue))
      tempValue &= tempValue - static_cast<ValueType>(1);

  return tempValue;
}

template<typename ValueType>
ValueType constexpr ceilPowerOfTwo(ValueType const value) noexcept
{
// Note: make sure to check if value is within allowable range, e.g.:
// if value >= math::floorPowerOfTwo(SIZE_MAX), then do not call this function.

  ValueType tempValue = value;

  if ((tempValue > static_cast<ValueType>(2)) && !isPowerOfTwo(tempValue))
  {
    tempValue <<= static_cast<ValueType>(1);

    while (!isPowerOfTwo(tempValue))
      tempValue &= tempValue - static_cast<ValueType>(1);
  }
  return tempValue;
}

template<typename ValueType>
ValueType constexpr computeNextCapacity(ValueType capacity) noexcept
{
  if (capacity < static_cast<ValueType>(16) || !isPowerOfTwo(capacity))
  {
    ValueType const nextCapacity = ceilPowerOfTwo(capacity + static_cast<ValueType>(1));
    if (nextCapacity - capacity < capacity / static_cast<ValueType>(3))
      capacity = nextCapacity + (nextCapacity / static_cast<ValueType>(2));
    else
      capacity = nextCapacity;
  }
  else
    capacity += capacity / static_cast<ValueType>(2);

  return capacity;
}

template<typename ValueType>
ValueType constexpr computeNextCapacity(ValueType const capacity, ValueType const currentCapacity,
  ValueType const initialCapacity) noexcept
{
  ValueType capacityNew;

  if (currentCapacity != initialCapacity)
  {
    ValueType const capacityNext = computeNextCapacity(currentCapacity);
    if (capacity > capacityNext)
    {
      if (!isPowerOfTwo(capacity))
      {
        ValueType capacityPred = floorPowerOfTwo(capacity);
        capacityPred += capacityPred / static_cast<ValueType>(2);
        capacityNew = capacityPred >= capacity ? capacityPred : ceilPowerOfTwo(capacity);
      }
      else
        capacityNew = capacity;
    }
    else
      capacityNew = capacityNext;
  }
  else
    capacityNew = capacity;

  return capacityNew;
}

} // namespace math
} // namespace trl