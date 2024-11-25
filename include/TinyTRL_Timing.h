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

// TinyTRL_Timing.h
#pragma once

#include "TinyTRL_TypeDef.h"

namespace trl {

/// Number of ticks, typically represented in microseconds.
typedef uint64_t TickCount;

/// Data type to store time slices.
typedef int64_t TimeSlice;

/// Sleeps the current thread for the given number of milliseconds.
extern void timingSleep(uint32_t milliseconds);

/// Sleeps the current thread for the given number of milliseconds.
extern void timingSleepUS(uint32_t microseconds);

/// Returns raw number of microseconds elapsed since initialization.
extern TickCount timingTickCountUS();

/// Returns number of milliseconds elapsed since initialization.
extern uint32_t timingTickCount();

/// Calculates different in microseconds between second and first tick counts, accounting for wrap-around.
extern TickCount timingTickDifference(TickCount nextTicks, TickCount prevTicks);

/// Calculates different in milliseconds between second and first values, accounting for wrap-around.
extern uint32_t timingDifferenceMS(uint32_t nextTime, uint32_t prevTime);

// Timing utilities.
namespace timing {

// Returns number of milliseconds elapsed since Epoch. This uses platform-specific functions.
extern int64_t time();

} // namespace timing
} // namespace trl