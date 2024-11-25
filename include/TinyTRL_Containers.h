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

// TinyTRL_Containers.h
#pragma once

#include "TinyTRL_Math.h"

namespace trl {

// Helper utilities.

namespace utility {

/// Swaps the contents of two values using move constructor.
template <typename Element>
constexpr void swap(Element& element1, Element& element2);

} // namespace utility

/// Default allocator utility.
struct Allocator
{
  /// Allocates requested number of bytes and returns pointer to the start of allocated memory block.
  /// In case of memory allocation failure, NULL is returned.
  [[nodiscard]] void* alloc(size_t numBytes, size_t alignment) noexcept;

  /// Releases memory previously allocated with \c alloc(). The number of bytes must match the value given
  /// during allocation. This might be required for some custom allocators that do not track size of
  /// allocated memory themselves.
  void free(void* data, size_t numBytes, size_t alignment) noexcept;
};

/// Default allocator utility that uses C functions.
struct CAllocator
{
  /// Allocates requested number of bytes, or reallocates a previously allocated memory block to a new
  /// length, preserving existing elements. If "requestedBytes" is zero, releases an existing memory and
  /// returns NULL. In case of memory allocation failure, returns NULL.
  [[nodiscard]] void* alloc(void* data, size_t dataBytes, size_t requestedBytes,
    size_t alignment) noexcept;
};

/// Common container types and constants.
class Containers
{
public:
  /// Array type used to store the length.
#ifdef __PLATFORM_X64
  typedef int64_t Length;
#else
  typedef int32_t Length;
#endif

  /// Maximum available array length.
#ifdef __PLATFORM_X64
  static Length constexpr const MaxLength = INT64_MAX;
#else
  static Length constexpr const MaxLength = INT32_MAX;
#endif

  /// Special constant to denote an invalid or inexistent index.
  static Length constexpr const NotFound = -1;

  /// Key-value pair combination.
  template <typename Key, typename Value>
  struct Pair
  {
    /// Key parameter.
    Key key;

    /// Value parameter.
    Value value;

    /// Creates pair uninitialized.
    constexpr Pair() = default;

    /// Creates pair with both key and value being a copies.
    constexpr Pair(Key const& key, Value const& value);

    /// Creates pair with key being a copy.
    constexpr Pair(Key&& key, Value const& value);

    /// Creates pair with value being a copy.
    constexpr Pair(Key const& key, Value&& value);

    /// Creates pair with both key and value moved in.
    constexpr Pair(Key&& key, Value&& value) noexcept;

    /// Creates pair being a copy of another pair.
    constexpr Pair(Pair const&) = default;

    /// Creates pair moving contents from another pair.
    constexpr Pair(Pair&& pair) noexcept;

    /// Copies contents from another pair.
    constexpr Pair& operator = (Pair const&) = default;

    /// Moves contents from another pair.
    constexpr Pair& operator = (Pair&&) noexcept = default;

    /// Tests whether the pair matches another pair.
    constexpr bool operator == (Pair const& pair) const noexcept;

    /// Tests whether the pair is different to another pair.
    constexpr bool operator != (Pair const& pair) const noexcept;

    /// Tests whether the pair is lexicographically smaller than the other one.
    constexpr bool operator < (Pair const& pair) const noexcept;

    /// Tests whether the pair is lexicographically bigger than the other one.
    constexpr bool operator > (Pair const& pair) const noexcept;
  };

  /// Opaque structure that defines an index-based element location.
  class Location
  {
  public:
    /// Constructs a new default location, which is not defined.
    constexpr Location();

    /// Constructs a new location with the given index.
    explicit constexpr Location(Length index);

    /// Creates a location being a copy of another location.
    constexpr Location(Location const&) = default;

    /// Copies contents from another location to this one.
    constexpr Location& operator = (Location const&) = default;

    /// Creates a location moving contents from another location.
    constexpr Location(Location&&) noexcept = default;

    /// Moves contents from another location to this one.
    constexpr Location& operator = (Location&&) noexcept = default;

    /// Tests whether a location is valid.
    [[nodiscard]] explicit constexpr operator bool () const;

    /// Tests whether the location index matches the value of another location.
    constexpr bool operator == (Location const& location) const noexcept;

    /// Tests whether the location index does not match the value of another location.
    constexpr bool operator != (Location const& location) const noexcept;

    /// Tests whether the location index is smaller than the value of another location.
    constexpr bool operator < (Location const& location) const noexcept;

    /// Tests whether the location index is bigger than the value of another location.
    constexpr bool operator > (Location const& location) const noexcept;

    /// Returns current location's index.
    constexpr Length index() const;

  private:
    // Index of the element referenced by this location.
    Length _index;
  };

protected:
  /// Array type used to store the actual size bits.
#ifdef __PLATFORM_X64
  typedef uint64_t Size;
#else
  typedef uint32_t Size;
#endif

  /// Bits that designate buffer length.
#ifdef __PLATFORM_X64
  static Size constexpr const LengthMask = 0x7FFFFFFFFFFFFFFFull;
#else
  static Size constexpr const LengthMask = 0x7FFFFFFFul;
#endif

  /// Bit that designates an error (pollute) bit.
#ifdef __PLATFORM_X64
  static Size constexpr const PolluteBit = 0x8000000000000000ull;
#else
  static Size constexpr const PolluteBit = 0x80000000ul;
#endif

  /// Calculates next exponentially-growing buffer capacity.
  static Length computeCapacity(Length targetCapacity, Length currentCapacity) noexcept;
};

/// Three-way comparison helper for two generic arguments.
struct DefaultCompare
{
  /// Performs standard test between left and right elements.
  template <typename Value>
  static constexpr int8_t perform(Value const& left, Value const& right);
};

/// Three-way comparison functor template for two generic arguments.
template <typename Value>
struct DefaultComparer
{
  /// Tests whether left parameter is less than right parameter.
  constexpr int8_t operator () (Value const& left, Value const& right) const;
};

/// Dynamic array that provides an exponentially growing capacity.
template <typename Element, typename Alloc = Allocator>
class Array : public Containers
{
public:
  /// Creates an empty array.
  Array(Alloc&& alloc = Alloc()) noexcept;

  /// Creates array with the requested capacity.
  /// In a case of memory allocation failure, sets the capacity to zero.
  Array(Length capacity, Alloc&& alloc = Alloc()) noexcept;

  /// Creates array with the given size and initial value.
  /// In case of a memory allocation failure, creates an empty polluted array (with a pollution bit set).
  Array(Length length, Element const& value, Alloc&& alloc = Alloc());

  /// Creates array from an initializer list.
  Array(std::initializer_list<Element> elements, Alloc&& alloc = Alloc());

  /// Creates a new array copying elements from an existing one.
  /// In case of a memory allocation failure, creates an empty polluted array (with a pollution bit set).
  Array(Array const& array);

  /// Creates array with contents moved from another one.
  Array(Array&& array) noexcept;

  /// Copies the contents of source array into this one.
  /// In case of a memory allocation failure, pollutes the current array.
  Array& operator = (Array const& array);

  /// Moves contents of another array into this one.
  Array& operator = (Array&& array) noexcept;

  /// Releases the array.
  ~Array();

  /// Returns pointer to array contents.
  [[nodiscard]] Element* data() noexcept;

  /// Returns a constant pointer to array contents.
  [[nodiscard]] Element const* data() const noexcept;

  /// Returns a reference to an element with the given index.
  [[nodiscard]] Element& operator [] (Length index) noexcept;

  /// Returns a constant reference to an element with the given index.
  [[nodiscard]] Element const& operator [] (Length index) const noexcept;

  /// Returns reference to first element in the array.
  [[nodiscard]] Element& first() noexcept;

  /// Returns constant reference to first element in the array.
  [[nodiscard]] Element const& first() const noexcept;

  /// Returns reference to last element in the array.
  [[nodiscard]] Element& last() noexcept;

  /// Returns constant reference to last element in the array.
  [[nodiscard]] Element const& last() const noexcept;

  /// Returns constant pointer to the first character in the array.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Element const* begin() const noexcept;

  /// Returns constant pointer to one character past last in the array.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Element const* end() const noexcept;

  /// Returns pointer to the first character in the array.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Element* begin() noexcept;

  /// Returns pointer to one character past last in the array.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Element* end() noexcept;

  /// Tests whether a array is not polluted. A polluted array has an error bit set. This may indicate an
  /// error during memory allocation or some data corruption.
  [[nodiscard]] explicit operator bool () const noexcept;

  /// Returns number of elements that array can hold before realloacating to a greater length.
  [[nodiscard]] Length capacity() const noexcept;

  /// Increases array capacity to accomodate at least the requested number of elements.
  [[nodiscard]] bool capacity(Length capacity);

  /// Returns number of elements in the array.
  [[nodiscard]] Length length() const noexcept;

  /// Changes length of array to the desired number of elements.
  [[nodiscard]] bool length(Length length, Element const& value = Element());

  /// Clears array by removing all elements but without releasing pre-allocated memory.
  void clear() noexcept;

  /// Shrinks array so that its capacity will match actual stored number of elements.
  /// Note that this may actually perform a re-allocation.
  [[nodiscard]] bool shrink() noexcept;

  /// Clears the array and releases any pre-allocated memory.
  void purge() noexcept;

  /// Adds multiple elements to the array with the same value.
  [[nodiscard]] bool populate(Length count, Element const& value);

  /// Adds a copy of the given element to the array, returning its index.
  /// In case of an overflow or a memory allocation failure, returns NotFound.
  [[nodiscard]] Length add(Element const& element);

  /// Adds an element to the array by moving its contents and returning element's index.
  /// In case of an overflow or a memory allocation failure, returns NotFound.
  [[nodiscard]] Length add(Element&& element);

  /// Inserts a copy of the given element at the requested position.
  [[nodiscard]] bool insert(Length index, Element const& element);

  /// Inserts an element by moving its contents to the requested position.
  [[nodiscard]] bool insert(Length index, Element&& element);

  /// Adds a copy of the given element to the array, returning its index.
  /// In case of an overflow or a memory allocation failure, sets an error bit, marking array as polluted.
  Array& addp(Element const& element);

  /// Adds an element to the array by moving its contents and returning element's index.
  /// In case of an overflow or a memory allocation failure, sets an error bit, marking array as polluted.
  Array& addp(Element&& element);

  /// Inserts a copy of the given element at the requested position.
  /// In case of an overflow or a memory allocation failure, sets an error bit, marking array as polluted.
  Array& insertp(Length index, Element const& element);

  /// Inserts an element by moving its contents to the requested position.
  /// In case of an overflow or a memory allocation failure, sets an error bit, marking array as polluted.
  Array& insertp(Length index, Element&& element);

  /// Removes element at the given index from the array, by shifting all elements to the beginning.
  bool erase(Length index) noexcept;

  /// Removes multiple elements starting at the given position from the array, by shifting all elements to
  /// the beginning.
  bool erase(Length start, Length count) noexcept;

  /// Tests whether the array is empty.
  bool empty() const noexcept;

  /// Sets an error bit in the array, marking it as polluted.
  Array& pollute() noexcept;

  /// Resets error bit in the array, removing pollute status.
  Array& unpollute() noexcept;

  /// Swaps two elements in the array. Note: this does not test whether both indices refer to the same
  /// element nor perform any bounding checks. However, swapping element with itself should be safe.
  void swap(Length first, Length second) noexcept;

  /// Sorts a range of elements in ascending order using QuickSort algorithm.
  template <typename Comparer = DefaultComparer<Element>>
  void quickSort(Length first = 0, Length last = MaxLength, Comparer const& comparer = Comparer());

  /// Searches for a given element using Binary Search algorithm.
  /// The elements in the array must be sorted in ascending order for this function to work.
  template <typename Comparer = DefaultComparer<Element>>
  Length binarySearch(Element const& element, Length first = 0, Length last = MaxLength,
    Comparer const& comparer = Comparer());

  /// Searches for an element using Binary Search algorithm with custom comparison function.
  /// The elements in the array must be sorted in ascending order for this function to work.
  template <typename Comparer>
  Length binarySearch(Length first = 0, Length last = MaxLength, Comparer const& comparer = Comparer());

private:
  // Pointer to the first array element.
  Element* _data;

  // Number of array elements that are pre-allocated, plus an optional "pollute" bit.
  Size _capacity;

  // Actual number of elements stored.
  Size _length;

  // Custom allocator module.
  Alloc _alloc;

  // Sorts elements recursively within the given range using QuickSort algorithm.
  template <typename Comparer>
  void recursiveQuickSort(Length first, Length last, Comparer const& comparer);

  // Performs partitioning for a QuickSort algorithm.
  template <typename Comparer>
  Length partitionQuickSort(Length first, Length last, Comparer const& comparer);

  // Inserts an element to the requested position in the array.
  template <typename ElementAssign>
  bool elementInsert(Length index, ElementAssign const& elementAssign);

  // Adds an element to the array.
  template <typename ElementAssign>
  Length elementAdd(ElementAssign const& elementAssign);

  // Reallocates array to the requested capacity.
  [[nodiscard]] bool reallocate(Length capacity);

  // Calls destructors for all elements in reverse order and releases allocated memory.
  void deallocate();

  // Allocates a given number of elements using custom allocator.
  Element* alloc(Length count);

  // Releases a given number of elements using custom allocator.
  void free(Element* data, Length count);
};

/// Associative container between key and value pairs using a sorted array for storage.
template <typename Key, typename Value, typename Comparer = DefaultComparer<Key>, typename Alloc = Allocator>
class FlatMap : public Containers
{
public:
  /// Pair that represents both key and value.
  typedef Pair<Key, Value> KeyValue;

  /// Creates an empty container.
  FlatMap(Comparer&& comparer = Comparer(), Alloc&& alloc = Alloc()) noexcept;

  /// Creates container from an initializer list.
  FlatMap(std::initializer_list<KeyValue> pairs, Comparer&& comparer = Comparer(),
    Alloc&& alloc = Alloc()) noexcept;

  /// Creates a new container copying elements from an existing container.
  /// In case of a memory allocation failure, creates an empty polluted map (with an error bit set).
  FlatMap(FlatMap const&) = default;

  /// Creates a new container with contents moved from another container.
  FlatMap(FlatMap&&) noexcept = default;

  /// Copies the contents of source container into this one.
  /// In case of a memory allocation failure, pollutes current container (sets an error bit).
  FlatMap& operator = (FlatMap const&) = default;

  /// Moves contents of another container into this one.
  FlatMap& operator = (FlatMap&&) noexcept = default;

  /// Provides index-based access to the container.
  [[nodiscard]] KeyValue const& operator [] (Location const& location) const noexcept;

  /// Returns constant pointer to the first pair in the container.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  KeyValue const* begin() const noexcept;

  /// Returns constant pointer to one pair past last in the container.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  KeyValue const* end() const noexcept;

  /// Returns constant reference to first key/value pair in the list.
  [[nodiscard]] KeyValue const& first() const noexcept;

  /// Returns constant reference to last key/value pair in the list.
  [[nodiscard]] KeyValue const& last() const noexcept;

  /// Tests whether a container is not polluted. A polluted buffer has an error bit set. This may indicate
  /// an error during memory allocation or some data corruption.
  [[nodiscard]] explicit operator bool () const noexcept;

  /// Returns number of elements that container can hold before realloacating to a greater length.
  [[nodiscard]] Length capacity() const noexcept;

  /// Increases container capacity to accomodate at least the requested number of elements.
  [[nodiscard]] bool capacity(Length capacity);

  /// Returns number of elements in the container.
  [[nodiscard]] Length length() const noexcept;

  /// Clears container by removing all elements but without releasing pre-allocated memory.
  void clear() noexcept;

  /// Shrinks container so that its capacity will match actual stored number of elements.
  [[nodiscard]] bool shrink() noexcept;

  /// Clears the container and releases any pre-allocated memory.
  void purge() noexcept;

  /// Tests whether a given key is already in the list.
  [[nodiscard]] bool exists(Key const& key) const noexcept;

  /// Adds or updates a key/value pair to the container, returning its location.
  [[nodiscard]] Location add(Key const& key, Value const& value);

  /// Adds or updates a key and (moved in) value pair to the container, returning its location.
  [[nodiscard]] Location add(Key const& key, Value&& value);

  /// Adds or updates a (moved in) key and value pair to the container, returning its location.
  [[nodiscard]] Location add(Key&& key, Value const& value);

  /// Adds or updates a moved in key/value pair to the container, returning its location.
  [[nodiscard]] Location add(Key&& key, Value&& value);

  /// Inserts a given pair of key/value at the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Key const& key, Value const& value);

  /// Inserts a given key and (moved in) value at the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Key const& key, Value&& value);

  /// Inserts a (moved in) given key and value at the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Key&& key, Value const& value);

  /// Inserts a moved in pair of key/value at the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Key&& key, Value&& value);

  /// Adds or updates a key/value pair to the container. In case of an overflow or a memory allocation
  /// failure, sets an error bit, marking container as polluted.
  FlatMap& addp(Key const& key, Value const& value);

  /// Adds or updates a key/value pair to the container. In case of an overflow or a memory allocation
  /// failure, sets an error bit, marking container as polluted.
  FlatMap& addp(Key const& key, Value&& value);

  /// Adds or updates a key/value pair to the container. In case of an overflow or a memory allocation
  /// failure, sets an error bit, marking container as polluted.
  FlatMap& addp(Key&& key, Value const& value);

  /// Adds or updates a key/value pair to the container. In case of an overflow or a memory allocation
  /// failure, sets an error bit, marking container as polluted.
  FlatMap& addp(Key&& key, Value&& value);

  /// Erases value with the given key from the container, if such exists.
  bool erase(Key const& key) noexcept;

  /// Removes value at the given location from the container, if such exists.
  bool erase(Location const& location) noexcept;

  /// Returns constant pointer to value associated with the given key.
  /// If such key is not found, returns NULL.
  [[nodiscard]] Value const* value(Key const& key) const noexcept;

  /// Returns pointer to value associated with the given key. If such key is not found, returns NULL.
  [[nodiscard]] Value* value(Key const& key) noexcept;

  /// Returns constant reference to value associated with the given location.
  [[nodiscard]] Value const& at(Location const& location) const noexcept;

  /// Returns reference to value associated with the given location.
  [[nodiscard]] Value& at(Location const& location) noexcept;

  /// Attempts to find a given key and returns its location.
  [[nodiscard]] Location find(Key const& key) const noexcept;

  /// Attempts to find a given key and returns its location. If the key was not found, returns location,
  /// where it can be inserted.
  [[nodiscard]] bool find(Location& location, Key const& key) const noexcept;

  /// Tests whether the container is empty.
  bool empty() const noexcept;

  /// Sets an error bit in the container, marking it as polluted.
  FlatMap& pollute() noexcept;

  /// Resets error bit in the container, removing pollute status.
  FlatMap& unpollute() noexcept;

private:
  // Container for integrated key/value pairs.
  typedef Array<KeyValue, Alloc> KeyValues;

  // Integrated array of key/value pairs.
  KeyValues _pairs;

  // Comparer module.
  Comparer _comparer;

  // Attempts to find a given key using binary search.
  bool search(Length& index, Key const& key) const noexcept;
};

/// A set of unique values using a sorted array for storage.
template <typename Value, typename Comparer = DefaultComparer<Value>, typename Alloc = Allocator>
class FlatSet : public Containers
{
public:
  /// Creates an empty container.
  FlatSet(Comparer&& comparer = Comparer(), Alloc&& alloc = Alloc()) noexcept;

  /// Creates container from an initializer list.
  FlatSet(std::initializer_list<Value> values, Comparer&& comparer = Comparer(), Alloc&& alloc = Alloc());

  /// Creates a new container copying elements from an existing container.
  /// In case of a memory allocation failure, creates an empty polluted map (with an error bit set).
  FlatSet(FlatSet const&) = default;

  /// Creates a new container with contents moved from another container.
  FlatSet(FlatSet&&) noexcept = default;

  /// Copies the contents of source container into this one.
  /// In case of a memory allocation failure, pollutes current container (sets an error bit).
  FlatSet& operator = (FlatSet const&) = default;

  /// Moves contents of another container into this one.
  FlatSet& operator = (FlatSet&&) noexcept = default;

  /// Provides constant indexed access to the container.
  [[nodiscard]] Value const& operator [] (Location const& location) const noexcept;

  /// Returns constant pointer to the first character in the container.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Value const* begin() const noexcept;

  /// Returns constant pointer to one character past last in the container.
  /// Note: this function is provided to enable C++11 ranged for and should not be used otherwise.
  Value const* end() const noexcept;

  /// Tests whether a container is not polluted. A polluted container has an error bit set.
  /// This may indicate an error during memory allocation or some data corruption.
  [[nodiscard]] explicit operator bool () const noexcept;

  /// Returns number of elements that container can hold before realloacating to a greater length.
  [[nodiscard]] Length capacity() const noexcept;

  /// Increases container capacity to accomodate at least the requested number of elements.
  [[nodiscard]] bool capacity(Length capacity);

  /// Returns number of elements in the container.
  [[nodiscard]] Length length() const noexcept;

  /// Clears container by removing all elements but without releasing pre-allocated memory.
  void clear() noexcept;

  /// Shrinks container so that its capacity will match actual stored number of elements.
  [[nodiscard]] bool shrink() noexcept;

  /// Clears the container and releases any pre-allocated memory.
  void purge() noexcept;

  /// Tests whether a given key is already in the list.
  [[nodiscard]] bool exists(Value const& value) const noexcept;

  /// Adds a value to the container, returning its location.
  [[nodiscard]] Location add(Value const& value);

  /// Adds a value to the container, returning its location.
  [[nodiscard]] Location add(Value&& value);

  /// Adds or updates value in the container.
  [[nodiscard]] bool update(Value const& value);

  /// Adds or updates value in the container.
  [[nodiscard]] bool update(Value&& value);

  /// Inserts a value to the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Value const& value);

  /// Inserts a (moved in) value to the given location returned by \c find() function.
  /// Warning: only location returned by a recent call of \c find() is allowed. Inserting elements at other
  /// locations may make the list loose its ordered property and cause undefined behavior.
  [[nodiscard]] bool insert(Location const& location, Value&& value);

  /// Adds a value to the container. In case of an overflow or a memory allocation failure, sets an error
  /// bit, marking container as polluted.
  FlatSet& addp(Value const& value);

  /// Adds a value to the container. In case of an overflow or a memory allocation failure, sets an error
  /// bit, marking container as polluted.
  FlatSet& addp(Value&& value);

  /// Erases value with the given key from the container, if such exists.
  bool erase(Value const& value) noexcept;

  /// Removes value at the given location from the container, if such exists.
  bool erase(Location const& location) noexcept;

  /// Attempts to find a given value and returns its location.
  [[nodiscard]] Location find(Value const& value) const noexcept;

  /// Attempts to find a given value and returns its location. If the value was not found, returns location,
  /// where it can be inserted.
  [[nodiscard]] bool find(Location& index, Value const& value) const noexcept;

  /// Tests whether the container is empty.
  bool empty() const noexcept;

  /// Sets an error bit in the container, marking it as polluted.
  FlatSet& pollute() noexcept;

  /// Resets error bit in the container, removing pollute status.
  FlatSet& unpollute() noexcept;

  /// Attempts to find a given value using binary search with custom comparer and returns its location.
  /// If the value was not found, returns location, where it can be inserted. The comparer must return
  /// result of three-way comparison.
  template <typename CustomValue, typename CustomCompare>
  bool find(Location& location, CustomValue const& value, CustomCompare const& compare) const;

private:
  // Container for integrated values.
  typedef Array<Value> Values;

  // Integrated array of values.
  Values _values;

  // Comparer module.
  Comparer _comparer;

  // Attempts to find a given key using binary search.
  bool search(Length& index, Value const& value) const noexcept;
};

} // namespace trl

#include "TinyTRL_Containers.inl"