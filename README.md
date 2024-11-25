# TinyTRL
Tiny Template and Runtime Library (TinyTRL) is a compact and efficient Standard C++ Library (STL) replacement.
The library is cross-platform and meant to be used with GCC, Clang or MSVC, targeting C++20 or later.

The following templates, classes and functions are provided:
* Array - a general-purpose dynamic resizeable array.
* FlatMap - associative container between key and values using a sorted array as storage.
* FlatSet - a set of unique values using a sorted array as storage.
* String - string class that handles Ascii or UTF-8 encoded strings with Short String Optimization, and ability to "wrap" existing C strings without copying.
* WideString - UTF-16 string class mostly for calling Windows API with automatic conversion to/from String.
* Numerous string utilities for text comparison, search and replacement.
* Functions for working with file paths and extensions.
* Basic mathematical (e.g. min, max) and utility (e.g. swap) functions.
* Basic timing functions.

The library has the following objectives:
* Should reasonably work in "freestanding" applications (those that do not link to standard C++ library).
* Should ideally not depend, or have minimal dependency on standard C library (e.g. use a small subset of functions such as memcpy, memmove, etc).
* Must compile and run with C++ exceptions disabled while handling "out of memory" errors, for example, by returning "false".
* Can use native OS functions, e.g. from Windows or Posix APIs.
* Reasonably small, compact and efficient.
* Should be able to reasonably support legacy OS such as Windows NT 4, at least with a limited set of functions.

Usage:
* Templates can be used just by adding "include" directory to the project.
* Classes such as String require the appropriate ".cpp" files to be added to the project from "src" directory.
* See existing examples for more information.

TODO:
* Stream and file I/O utilities (coming very soon).
* Thread, synchronization and atomic functions (coming very soon).
* Map and Set using some kind of balanced trees (planned).
* HashMap container (planned).
* Rewrite some functions that still use standard C library calls (e.g. strtoll) with its own, to reduce dependencies.

***If you like the library, please consider sponsoring its development.***
