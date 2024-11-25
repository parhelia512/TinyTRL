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

#include "TinyTRL_Timing.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <sys/time.h>
  #include <time.h>
#endif

namespace trl {

// Forward declarations.
#ifndef _WIN32

  // Returns current value of monotonic clock.
  static TickCount readClockMonotonic();

#endif

// Global variables.

#ifdef _WIN32
  // Running frequency of a QPC.
  static LARGE_INTEGER counterFrequency;

  // Value of a QPC retrieved during initialization.
  static LARGE_INTEGER counterStart;
#else
  // Time acquired during initialization.
  static TickCount timingStart;
#endif

// Whether the calculations have been performed.
static bool timingStartCalculated = false;

// Global functions.

#ifdef _WIN32

  void timingSleep(uint32_t const milliseconds)
  {
    SleepEx(milliseconds, true);
  }

  void timingSleepUS(uint32_t const microseconds)
  {
    static TickCount constexpr const spinLockWaitTime = 3000;

    if (microseconds > 0)
    {
      TickCount elapsed;
      TickCount const startTicks = timingTickCountUS();

      if (microseconds > spinLockWaitTime)
      { // Sleep until less than spin-lock waiting time is left.
        TickCount const waitMs = (microseconds - spinLockWaitTime) / 1000u;
        if (waitMs > 0)
          SleepEx(static_cast<uint32_t>(waitMs), true);
      }

      do
      { // Perform spin-lock wait.
        SwitchToThread();
        elapsed = timingTickDifference(timingTickCountUS(), startTicks);
      } while (elapsed < microseconds);
    }
    else
      SwitchToThread();
  }

  TickCount timingTickCountUS()
  {
    if (!timingStartCalculated)
    {
      bool const supported =
        QueryPerformanceFrequency(&counterFrequency) && counterFrequency.QuadPart &&
        QueryPerformanceCounter(&counterStart);

      if (!supported)
        counterFrequency = {};

      timingStartCalculated = true;
    }
    if (counterFrequency.QuadPart)
    {
      LARGE_INTEGER counter = {};
      QueryPerformanceCounter(&counter);

      return (static_cast<uint64_t>(counter.QuadPart - counterStart.QuadPart) * 1000000ul) /
        static_cast<uint64_t>(counterFrequency.QuadPart);
    }
    else
      return 0; // Not supported.
  }

#else

  void timingSleep(uint32_t const milliseconds)
  {
    struct timespec time = {};
    time.tv_sec = milliseconds / 1000;
    time.tv_nsec = (milliseconds % 1000) * 1000000L;
    nanosleep(&time, nullptr);
  }

  void timingSleepUS(uint32_t const microseconds)
  {
    struct timespec time = {};
    time.tv_sec = microseconds / 1000000L;
    time.tv_nsec = (microseconds % 1000000L) * 1000L;
    nanosleep(&time, nullptr);
  }

  TickCount timingTickCountUS()
  {
    TickCount const ticks = readClockMonotonic();
    if (!timingStartCalculated)
    {
      timingStart = ticks;
      timingStartCalculated = true;
    }
    return ticks - timingStart;
  }

  TickCount readClockMonotonic()
  {
    struct timespec time = {};

    if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == 0)
      return static_cast<uint64_t>(time.tv_sec) * 1000000u + (time.tv_nsec / 1000u);
    else
      return 0;
  }

#endif

uint32_t timingTickCount()
{
  return static_cast<uint32_t>(timingTickCountUS() / 1000u);
}

TickCount timingTickDifference(TickCount const nextTicks, TickCount const prevTicks)
{
  TickCount difference = nextTicks - prevTicks;
  difference = ~difference > difference ? difference : ~difference;
  return difference;
}

uint32_t timingDifferenceMS(uint32_t const nextTime, uint32_t const prevTime)
{
  uint32_t difference = nextTime - prevTime;
  difference = ~difference > difference ? difference : ~difference;
  return difference;
}

// Timing utilities.
namespace timing {

#ifdef _WIN32

  int64_t time()
  {
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);

    FILETIME fileTime;
    SystemTimeToFileTime(&systemTime, &fileTime);

    return static_cast<int64_t>((
      (static_cast<uint64_t>(fileTime.dwLowDateTime) +
      (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32)) - 116444736000000000ULL) / 10000L);
  }

#else

  int64_t time()
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<int64_t>(tv.tv_sec) * 1000 + static_cast<int64_t>(tv.tv_usec) / 1000;
  }

#endif

} // namespace timing
} // namespace trl