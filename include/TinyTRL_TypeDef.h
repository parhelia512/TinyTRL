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

// TinyTRL_TypeDef.h
#pragma once

#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <initializer_list>
#include <new>

#ifdef _MSC_VER
  #if defined(_M_X64) || defined(_M_ARM64) || defined (_M_IA64)
    #define __PLATFORM_X64
  #else
    #define __PLATFORM_X32
  #endif
#else
  #if defined(__x86_64__) || defined(__aarch64__) || defined (__ia64__)
    #define __PLATFORM_X64
  #else
    #define __PLATFORM_X32
  #endif
#endif

#ifdef __GNUC__
  #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // GCC: Big Endian.
    #define __PLATFORM_BIG_ENDIAN
  #elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // GCC: Little Endian.
  #else
    #error "Unsupported machine's endianess."
  #endif
#endif

#ifndef __PLATFORM_BIG_ENDIAN
  #define __PLATFORM_LITTLE_ENDIAN
//  #define __PLATFORM_BIG_ENDIAN
#endif

#ifdef _MSC_VER
  // Framework uses boolean bit flags by design, suppress MSVC warning about it.
  #pragma warning(disable: 4800)

  // Disable useless CRT security warnings.
  #pragma warning(disable: 4996)
#endif