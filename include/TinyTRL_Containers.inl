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

// TinyTRL_Containers.inl
#pragma once

#include "TinyTRL_Containers.h"

namespace trl {
namespace utility {

// Utilities implementation.

template <typename Element>
constexpr void swap(Element& element1, Element& element2)
{
  alignas(Element) char buffer[sizeof(Element)];

  Element* const temp = new (buffer) Element(static_cast<Element&&>(element1));
  element1.~Element();

  new (&element1) Element(static_cast<Element&&>(element2));
  element2.~Element();

  new (&element2) Element(static_cast<Element&&>(*temp));
  temp->~Element();
}

} // namespace utility

// Allocator members.

inline void* Allocator::alloc(size_t const numBytes, size_t) noexcept
{
  return ::operator new(numBytes, std::nothrow);
}

inline void Allocator::free(void* const data, size_t, size_t) noexcept
{
  ::operator delete(data);
}

// CAllocator members.

inline void* CAllocator::alloc(void* const data, size_t, size_t const requestedBytes, size_t) noexcept
{
  if (requestedBytes)
    return ::realloc(data, requestedBytes);
  else
  {
    ::free(data);
    return nullptr;
  }
}

// DefaultCompare members.

template <typename Value>
constexpr int8_t DefaultCompare::perform(Value const& left, Value const& right)
{
  return left < right ? -1 : (right < left ? 1 : 0);
}

// DefaultComparer members.

/// Three-way comparison functor template for two generic arguments.
template <typename Value>
constexpr int8_t DefaultComparer<Value>::operator () (Value const& left, Value const& right) const
{
  return DefaultCompare::perform(left, right);
}

// Containers members.

/// Calculates next exponentially-growing buffer capacity.
inline Containers::Length Containers::computeCapacity(Length const targetCapacity,
  Length const currentCapacity) noexcept
{
  Length capacity = targetCapacity;

  if (targetCapacity < static_cast<Length>(math::floorPowerOfTwo(static_cast<size_t>(MaxLength))))
    capacity = math::computeNextCapacity(targetCapacity, currentCapacity);

  return capacity;
}

// Containers::Pair members.

template <typename Key, typename Value>
constexpr Containers::Pair<Key, Value>::Pair(Key const& key, Value const& value)
: key(key),
  value(value)
{
}

template <typename Key, typename Value>
constexpr Containers::Pair<Key, Value>::Pair(Key&& key, Value const& value)
: key(static_cast<Key&&>(key)),
  value(value)
{
}

template <typename Key, typename Value>
constexpr Containers::Pair<Key, Value>::Pair(Key const& key, Value&& value)
: key(key),
  value(static_cast<Value&&>(value))
{
}

template <typename Key, typename Value>
constexpr Containers::Pair<Key, Value>::Pair(Key&& key, Value&& value) noexcept
: key(static_cast<Key&&>(key)),
  value(static_cast<Value&&>(value))
{
}

template <typename Key, typename Value>
constexpr Containers::Pair<Key, Value>::Pair(Pair&& pair) noexcept
: key(static_cast<Key&&>(pair.key)),
  value(static_cast<Value&&>(pair.value))
{
}

template <typename Key, typename Value>
constexpr bool Containers::Pair<Key, Value>::operator == (Pair const& pair) const noexcept
{
  return key == pair.key && value == pair.value;
}

template <typename Key, typename Value>
constexpr bool Containers::Pair<Key, Value>::operator != (Pair const& pair) const noexcept
{
  return key != pair.key || value != pair.value;
}

template <typename Key, typename Value>
constexpr bool Containers::Pair<Key, Value>::operator < (Pair const& pair) const noexcept
{
  return key < pair.key || (key == pair.key && value < pair.value);
}

template <typename Key, typename Value>
constexpr bool Containers::Pair<Key, Value>::operator > (Pair const& pair) const noexcept
{
  return key > pair.key || (key == pair.key && value > pair.value);
}

// Containers::Location members.

constexpr Containers::Location::Location()
: _index(NotFound)
{
}

constexpr Containers::Location::Location(Length const index)
: _index(index)
{
}

constexpr Containers::Location::operator bool () const
{
  return _index != NotFound;
}

constexpr bool Containers::Location::operator == (Location const& location) const noexcept
{
  return _index == location._index;
}

constexpr bool Containers::Location::operator != (Location const& location) const noexcept
{
  return _index != location._index;
}

constexpr bool Containers::Location::operator < (Location const& location) const noexcept
{
  return _index < location._index;
}

constexpr bool Containers::Location::operator > (Location const& location) const noexcept
{
  return _index > location._index;
}

constexpr Containers::Length Containers::Location::index() const
{
  return _index;
}

// Array<Element, Alloc> members.

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(Alloc&& alloc) noexcept
: _data(nullptr),
  _capacity(0u),
  _length(0u),
  _alloc(static_cast<Alloc&&>(alloc))
{
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(Length const capacity, Alloc&& alloc) noexcept
: Array(static_cast<Alloc&&>(alloc))
{
  if (!this->capacity(math::max<Length>(capacity, 0)))
    pollute();
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(Length const length, Element const& value, Alloc&& alloc)
: Array(static_cast<Alloc&&>(alloc))
{
  if (!populate(length, value))
    pollute();
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(std::initializer_list<Element> const elements, Alloc&& alloc)
: Array(static_cast<Alloc&&>(alloc))
{
  if (Length length = static_cast<Length>(math::min(elements.size(), static_cast<size_t>(MaxLength))))
  {
    if (_data = this->alloc(length))
    {
      Length index = 0;

      for (Element const& element : elements)
        new (_data + index++) Element(element);

      _length = _capacity = static_cast<Size>(length);
    }
    else
      pollute();
  }
  if (elements.size() > static_cast<size_t>(MaxLength))
    pollute(); // Overflow (not all elements were copied)
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(Array const& array)
: Array(static_cast<Alloc&&>(Alloc(array._alloc)))
{
  assert(this != &array);

  if (Length length = array.length())
  {
    if (_data = alloc(length))
    {
      for (Length i = 0; i < length; ++i)
        new (_data + i) Element(array._data[i]);

      _length = _capacity = static_cast<Size>(length);
    }
    else
      pollute();
  }
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::Array(Array&& array) noexcept
: _data(array._data),
  _capacity(array._capacity),
  _length(array._length),
  _alloc(static_cast<Alloc&&>(array._alloc))
{
  assert(this != &array);

  array._data = nullptr;
  array._capacity = array._length = 0u;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::operator = (Array const& array)
{
  assert(this != &array);

  purge();
  _alloc = array._alloc;

  if (Length length = array.length())
  {
    if (_data = alloc(length))
    {
      for (Length i = 0; i < length; ++i)
        new (_data + i) Element(array._data[i]);

      _length = _capacity = static_cast<Size>(length);
    }
    else
      pollute();
  }
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::operator = (Array&& array) noexcept
{
  assert(this != &array);

  deallocate();
  _data = array._data;
  _capacity = array._capacity;
  _length = array._length;
  array._data = nullptr;
  array._capacity = array._length = 0u;
  _alloc = static_cast<Alloc&&>(array._alloc);
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::~Array()
{
  deallocate();
}

template <typename Element, typename Alloc>
Element* Array<Element, Alloc>::data() noexcept
{
  return _data;
}

template <typename Element, typename Alloc>
Element const* Array<Element, Alloc>::data() const noexcept
{
  return _data;
}

template <typename Element, typename Alloc>
Element& Array<Element, Alloc>::operator [] (Length const index) noexcept
{
  return _data[index];
}

template <typename Element, typename Alloc>
Element const& Array<Element, Alloc>::operator [] (Length const index) const noexcept
{
  return _data[index];
}

template <typename Element, typename Alloc>
Element& Array<Element, Alloc>::first() noexcept
{
  return *_data;
}

template <typename Element, typename Alloc>
Element const& Array<Element, Alloc>::first() const noexcept
{
  return *_data;
}

template <typename Element, typename Alloc>
Element& Array<Element, Alloc>::last() noexcept
{
  return _data[math::max<Length>(length() - 1, 0)];
}

template <typename Element, typename Alloc>
Element const& Array<Element, Alloc>::last() const noexcept
{
  return _data[math::max<Length>(length() - 1, 0)];
}

template <typename Element, typename Alloc>
Element const* Array<Element, Alloc>::begin() const noexcept
{
  return _data;
}

template <typename Element, typename Alloc>
Element const* Array<Element, Alloc>::end() const noexcept
{
  return _data + length();
}

template <typename Element, typename Alloc>
Element* Array<Element, Alloc>::begin() noexcept
{
  return _data;
}

template <typename Element, typename Alloc>
Element* Array<Element, Alloc>::end() noexcept
{
  return _data + length();
}

template <typename Element, typename Alloc>
Array<Element, Alloc>::operator bool () const noexcept
{
  return !(_capacity & PolluteBit);
}

template <typename Element, typename Alloc>
typename Array<Element, Alloc>::Length Array<Element, Alloc>::capacity() const noexcept
{
  return static_cast<Length>(_capacity & LengthMask);
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::capacity(Length capacity)
{
  capacity = math::max<Length>(capacity, 0);

  if (Length const currentCapacity = this->capacity(); currentCapacity < capacity)
    return reallocate(computeCapacity(capacity, currentCapacity));
  else
    return true; // Current capacity is sufficient.
}

template <typename Element, typename Alloc>
typename Array<Element, Alloc>::Length Array<Element, Alloc>::length() const noexcept
{
  return static_cast<Length>(_length);
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::length(Length length, Element const& value)
{
  length = math::max<Length>(length, 0);

  if (Length const currentLength = this->length(); currentLength != length)
  {
    if (currentLength < length)
    { // Add elements calling their copy constructor.
      if (!capacity(length))
        return false; // Allocation failed.

      for (Length i = currentLength; i < length; ++i)
        new (_data + i) Element(value);
    }
    else
    { // Destroy elements beyond requested length.
      for (Length i = currentLength - 1; i >= length; --i)
        _data[i].~Element();
    }
    _length = static_cast<Size>(length);
    return true;
  }
  else
    return true; // No change required.
}

template <typename Element, typename Alloc>
void Array<Element, Alloc>::clear() noexcept
{
  for (Length i = length(); i--; )
    _data[i].~Element();

  _capacity &= ~PolluteBit; // Keep memory allocated, but clear "pollute" bit.
  _length = 0u;
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::shrink() noexcept
{
  if (Length const length = this->length(); length != capacity())
  {
    if (length)
      return reallocate(length);
    else
    {
      deallocate();
      _data = nullptr;
      _capacity = _length = 0u;
    }
  }
  return true;
}

template <typename Element, typename Alloc>
void Array<Element, Alloc>::purge() noexcept
{
  if (length())
  {
    deallocate();
    _data = nullptr;
    _capacity = _length = 0u;
  }
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::populate(Length count, Element const& value)
{
  count = math::max<Length>(count, 0);

  if (Length const length = this->length(); length <= MaxLength - count)
  {
    if (capacity(length + count))
    {
      for (Length i = 0; i < count; ++i)
        new (_data + length) Element(value);

      return length;
    }
    else
      return NotFound; // Memory allocation failure
  }
  else
    return NotFound; // Overflow
}

template <typename Element, typename Alloc>
typename Array<Element, Alloc>::Length Array<Element, Alloc>::add(Element const& element)
{
  return elementAdd([&element](Element* dest) { new (dest) Element(element); });
}

template <typename Element, typename Alloc>
typename Array<Element, Alloc>::Length Array<Element, Alloc>::add(Element&& element)
{
  return elementAdd([&element](Element* dest) { new (dest) Element(static_cast<Element&&>(element)); });
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::insert(Length const index, Element const& element)
{
  return elementInsert(index, [&element](Element* dest) { new (dest) Element(element); });
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::insert(Length const index, Element&& element)
{
  return elementInsert(index, [&element](Element* dest)
    { new (dest) Element(static_cast<Element&&>(element)); });
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::addp(Element const& element)
{
  if (add(element) == NotFound)
    pollute();
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::addp(Element&& element)
{
  if (add(static_cast<Element&&>(element)) == NotFound)
    pollute();
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::insertp(Length const index, Element const& element)
{
  if (!insert(index, element))
    pollute();
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::insertp(Length const index, Element&& element)
{
  if (!insert(index, static_cast<Element&&>(element)))
    pollute();
  return *this;
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::erase(Length const index) noexcept
{
  if (Length const length = this->length(); index >= 0 && index < length)
  {
    _data[index].~Element();

    for (Length i = index; i < length - 1; ++i)
    {
      new (_data + i) Element(static_cast<Element&&>(_data[i + 1]));
      _data[i + 1].~Element();
    }
    _length = static_cast<Size>(length - 1);
    return true;
  }
  else
    return false; // Index is out of bounds.
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::erase(Length start, Length count) noexcept
{
  if (Length const length = this->length())
  {
    if (start < 0)
    {
      count += start;
      start = 0;
    }
    if (start >= length || count <= 0)
      return false; // Start beyond the end of array or count is less than one.

    Length const right = math::min(start + count, length);
    Length const cut = right - start;

    for (Length i = right; --i >= start; )
      _data[i].~Element();

    for (Length i = start; i < length - cut; ++i)
    {
      new (_data + i) Element(static_cast<Element&&>(_data[i + cut]));
      _data[i + cut].~Element();
    }
    _length = static_cast<Size>(length - cut);
    return true;
  }
  else
    return false; // No elements in the array
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::empty() const noexcept
{
  return !length();
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::pollute() noexcept
{
  _capacity |= PolluteBit;
  return *this;
}

template <typename Element, typename Alloc>
Array<Element, Alloc>& Array<Element, Alloc>::unpollute() noexcept
{
  _capacity &= ~PolluteBit;
  return *this;
}

template <typename Element, typename Alloc>
void Array<Element, Alloc>::swap(Length const first, Length const second) noexcept
{
  utility::swap(_data[first], _data[second]);
}

template <typename Element, typename Alloc>
template <typename Comparer>
void Array<Element, Alloc>::quickSort(Length const first, Length const last, Comparer const& comparer)
{
  Length const length = this->length();
  if (length > 1)
    recursiveQuickSort(
      math::saturate<Length>(first, 0, length - 1),
      math::saturate<Length>(last, 0, length - 1), comparer);
}

template <typename Element, typename Alloc>
template <typename Comparer>
Containers::Length Array<Element, Alloc>::binarySearch(Element const& element, Length const first,
  Length const last, Comparer const& comparer)
{
  if (Length const length = this->length())
  {
    Length left = math::saturate<Length>(first, 0, length - 1);
    Length right = math::saturate<Length>(last, 0, length - 1);

    while (left <= right)
    {
      Length const pivot = left + (right - left) / 2;
      auto const res = comparer(_data[pivot], element);
      if (res == 0)
        return pivot;

      if (res < 0)
        left = pivot + 1;
      else
        right = pivot - 1;
    }
  }
  return NotFound;
}

template <typename Element, typename Alloc>
template <typename Comparer>
Containers::Length Array<Element, Alloc>::binarySearch(Length const first, Length const last,
  Comparer const& comparer)
{
  if (Length const length = this->length())
  {
    Length left = math::saturate<Length>(first, 0, length - 1);
    Length right = math::saturate<Length>(last, 0, length - 1);

    while (left <= right)
    {
      Length const pivot = left + (right - left) / 2;
      auto const res = comparer(_data[pivot]);
      if (res == 0)
        return pivot;

      if (res < 0)
        left = pivot + 1;
      else
        right = pivot - 1;
    }
  }
  return NotFound;
}

template <typename Element, typename Alloc>
template <typename Comparer>
void Array<Element, Alloc>::recursiveQuickSort(Length const first, Length const last,
  Comparer const& comparer)
{
  if (first < last)
  {
    Length const middle = first + (last - first) / 2;
    if (first != middle)
      swap(first, middle); // Use middle element as pivot.

    Length const split = partitionQuickSort(first, last, comparer);
    recursiveQuickSort(first, split - 1, comparer);
    recursiveQuickSort(split + 1, last, comparer);
  }
}

template <typename Element, typename Alloc>
template <typename Comparer>
Containers::Length Array<Element, Alloc>::partitionQuickSort(Length const first, Length const last,
  Comparer const& comparer)
{
  Length left = first + 1, right = last;
  Element const& pivot = _data[first];

  while (left <= right)
  {
    while (left <= last && comparer(_data[left], pivot) < 0)
      ++left;

    while (right > first && comparer(_data[right], pivot) >= 0)
      --right;

    if (left < right)
      swap(left, right);
  }
  if (first != right)
    swap(first, right);
  return right;
}

template <typename Element, typename Alloc>
template <typename ElementAssign>
bool Array<Element, Alloc>::elementInsert(Length index, ElementAssign const& elementAssign)
{
  if (Length const length = this->length(); length < MaxLength)
  {
    index = math::saturate<Length>(index, 0, length);
    Length const capacity = this->capacity();

    if (length + 1 <= capacity)
    { // There is enough space to insert element without reallocation
      for (Length i = length; i > index; --i)
      {
        new (_data + i) Element(static_cast<Element&&>(_data[i - 1]));
        _data[i - 1].~Element();
      }
      elementAssign(_data + index);
    }
    else
    { // Reallocate and recombine the array
      Size const polluteBit = _capacity & PolluteBit;
      Length const nextCapacity = computeCapacity(length + 1, capacity);

      Element* data = alloc(nextCapacity);
      if (!data)
        return false; // Memory allocation failure

      for (Length i = 0; i < index; ++i)
      {
        new (data + i) Element(static_cast<Element&&>(_data[i]));
        _data[i].~Element();
      }
      elementAssign(data + index);

      for (Length i = index; i < length; ++i)
      {
        new (data + i + 1) Element(static_cast<Element&&>(_data[i]));
        _data[i].~Element();
      }
      free(_data, capacity);
      _data = data;
      _capacity = static_cast<Size>(nextCapacity) | polluteBit;
    }
    _length = static_cast<Size>(length + 1);
    return true;
  }
  else
    return false; // Overflow
}

template <typename Element, typename Alloc>
template <typename ElementAssign>
typename Array<Element, Alloc>::Length Array<Element, Alloc>::elementAdd(ElementAssign const& elementAssign)
{
  if (Length const length = this->length(); length < MaxLength)
  {
    if (capacity(length + 1))
    {
      elementAssign(_data + length);
      _length = static_cast<Size>(length + 1);
      return length;
    }
    else
      return NotFound; // Memory allocation failure
  }
  else
    return NotFound; // Overflow
}

template <typename Element, typename Alloc>
bool Array<Element, Alloc>::reallocate(Length const capacity)
{
  assert(capacity > 0 && capacity >= this->length());

  Length const capacityPrev = this->capacity();
  Size const polluteBit = _capacity & PolluteBit;

  if (Element* data = alloc(capacity))
  {
    Length const length = this->length();

    for (Length i = 0; i < length; ++i)
    {
      new (data + i) Element(static_cast<Element&&>(_data[i]));
      _data[i].~Element();
    }
    free(_data, capacityPrev);
    _data = data;
    _capacity = static_cast<Size>(capacity) | polluteBit;
    return true;
  }
  else
    return false; // Memory allocation failure
}

template <typename Element, typename Alloc>
void Array<Element, Alloc>::deallocate()
{
  for (Length i = length(); i--; )
    _data[i].~Element();

  free(_data, capacity());
}

template <typename Element, typename Alloc>
Element* Array<Element, Alloc>::alloc(Length const count)
{
  return static_cast<Element*>(_alloc.alloc(static_cast<size_t>(count) * sizeof(Element), alignof(Element)));
}

template <typename Element, typename Alloc>
void Array<Element, Alloc>::free(Element* const data, Length const count)
{
  _alloc.free(data, static_cast<size_t>(count) * sizeof(Element), alignof(Element));
}

// FlatMap<Key, Value, Comparer> members.

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>::FlatMap(Comparer&& comparer, Alloc&& alloc) noexcept
: _pairs(static_cast<Alloc&&>(alloc)),
  _comparer(static_cast<Comparer&&>(comparer))
{
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>::FlatMap(std::initializer_list<KeyValue> const pairs,
  Comparer&& comparer, Alloc&& alloc) noexcept
: _pairs(static_cast<Alloc&&>(alloc)),
  _comparer(static_cast<Comparer&&>(comparer))
{
  if (capacity(static_cast<Length>(pairs.size())))
  {
    for (KeyValue const* pair = pairs.begin(); pair != pairs.end(); ++pair)
      if (!add(pair->key, pair->value))
      {
        pollute();
        break;
      }
  }
  else
    pollute();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::KeyValue const& FlatMap<Key, Value, Comparer,
  Alloc>::operator [] (Location const& location) const noexcept
{
  return _pairs[location.index()];
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::KeyValue const* FlatMap<Key, Value, Comparer,
  Alloc>::begin() const noexcept
{
  return _pairs.begin();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::KeyValue const* FlatMap<Key, Value, Comparer,
  Alloc>::end() const noexcept
{
  return _pairs.end();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::KeyValue const& FlatMap<Key, Value, Comparer,
  Alloc>::first() const noexcept
{
  return _pairs.first();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::KeyValue const& FlatMap<Key, Value, Comparer,
  Alloc>::last() const noexcept
{
  return _pairs.last();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>::operator bool () const noexcept
{
  return static_cast<bool>(_pairs);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::Length FlatMap<Key, Value, Comparer,
  Alloc>::capacity() const noexcept
{
  return _pairs.capacity();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::capacity(Length const capacity)
{
  return _pairs.capacity(capacity);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
typename FlatMap<Key, Value, Comparer, Alloc>::Length FlatMap<Key, Value, Comparer,
  Alloc>::length() const noexcept
{
  return _pairs.length();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
void FlatMap<Key, Value, Comparer, Alloc>::clear() noexcept
{
  _pairs.clear();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::shrink() noexcept
{
  return _pairs.shrink();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
void FlatMap<Key, Value, Comparer, Alloc>::purge() noexcept
{
  _pairs.purge();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::exists(Key const& key) const noexcept
{
  Length index;
  return search(index, key);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Containers::Location FlatMap<Key, Value, Comparer, Alloc>::add(Key const& key, Value const& value)
{
  Length index;
  if (search(index, key))
    _pairs[index].value = value;
  else if (!_pairs.insert(index, static_cast<KeyValue&&>(KeyValue(key, value))))
    index = NotFound;
  return Location(index);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Containers::Location FlatMap<Key, Value, Comparer, Alloc>::add(Key const& key, Value&& value)
{
  Length index;
  if (search(index, key))
    _pairs[index].value = static_cast<Value&&>(value);
  else if (!_pairs.insert(index, static_cast<KeyValue&&>(KeyValue(key, static_cast<Value&&>(value)))))
    index = NotFound;
  return Location(index);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Containers::Location FlatMap<Key, Value, Comparer, Alloc>::add(Key&& key, Value const& value)
{
  Length index;
  if (search(index, key))
    _pairs[index].value = value;
  else if (!_pairs.insert(index, static_cast<KeyValue&&>(KeyValue(static_cast<Key&&>(key), value))))
    index = NotFound;
  return Location(index);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Containers::Location FlatMap<Key, Value, Comparer, Alloc>::add(Key&& key, Value&& value)
{
  Length index;
  if (search(index, key))
    _pairs[index].value = static_cast<Value&&>(value);
  else if (!_pairs.insert(index, static_cast<KeyValue&&>(KeyValue(static_cast<Key&&>(key),
    static_cast<Value&&>(value)))))
    index = NotFound;
  return Location(index);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::insert(Location const& location, Key const& key,
  Value const& value)
{
  return _pairs.insert(location.index(), static_cast<KeyValue&&>(KeyValue(key, value)));
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::insert(Location const& location, Key const& key, Value&& value)
{
  return _pairs.insert(location.index(),
    static_cast<KeyValue&&>(KeyValue(key, static_cast<Value&&>(value))));
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::insert(Location const& location, Key&& key, Value const& value)
{
  return _pairs.insert(location.index(),
    static_cast<KeyValue&&>(KeyValue(static_cast<Key&&>(key), value)));
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::insert(Location const& location, Key&& key, Value&& value)
{
  return _pairs.insert(location.index(),
    static_cast<KeyValue&&>(KeyValue(static_cast<Key&&>(key), static_cast<Value&&>(value))));
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::addp(Key const& key,
  Value const& value)
{
  if (!add(key, value))
    pollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::addp(Key const& key, Value&& value)
{
  if (!add(key, static_cast<Value&&>(value)))
    pollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::addp(Key&& key, Value const& value)
{
  if (!add(static_cast<Key&&>(key), value))
    pollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::addp(Key&& key, Value&& value)
{
  if (!add(static_cast<Key&&>(key), static_cast<Value&&>(value)))
    pollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::erase(Key const& key) noexcept
{
  Length index;
  if (search(index, key))
    return _pairs.erase(index);
  else
    return false; // Key does not exist.
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::erase(Location const& location) noexcept
{
  return location ? _pairs.erase(location.index()) : false;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Value const* FlatMap<Key, Value, Comparer, Alloc>::value(Key const& key) const noexcept
{
  Length index;
  return search(index, key) ? &_pairs[index].value : nullptr;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Value* FlatMap<Key, Value, Comparer, Alloc>::value(Key const& key) noexcept
{
  Length index;
  return search(index, key) ? &_pairs[index].value : nullptr;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Value const& FlatMap<Key, Value, Comparer, Alloc>::at(Location const& location) const noexcept
{
  return _pairs[location.index()].value;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Value& FlatMap<Key, Value, Comparer, Alloc>::at(Location const& location) noexcept
{
  return _pairs[location.index()].value;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
Containers::Location FlatMap<Key, Value, Comparer, Alloc>::find(Key const& key) const noexcept
{
  Length index;
  return search(index, key) ? Location(index) : Location(NotFound);
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::find(Location& location, Key const& key) const noexcept
{
  Length index;
  bool const found = search(index, key);
  location = Location(index);
  return found;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::empty() const noexcept
{
  return _pairs.empty();
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::pollute() noexcept
{
  _pairs.pollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
FlatMap<Key, Value, Comparer, Alloc>& FlatMap<Key, Value, Comparer, Alloc>::unpollute() noexcept
{
  _pairs.unpollute();
  return *this;
}

template <typename Key, typename Value, typename Comparer, typename Alloc>
bool FlatMap<Key, Value, Comparer, Alloc>::search(Length& index, Key const& key) const noexcept
{
  Length left = 0, right = _pairs.length() - 1;

  while (left <= right)
  {
    Length const pivot = (left + right) / 2;
    auto const res = _comparer(_pairs[pivot].key, key);

    if (res < 0)
      left = pivot + 1;
    else if (res > 0)
      right = pivot - 1;
    else
    {
      index = pivot;
      return true; // Key found.
    }
  }
  index = left;
  return false; // Key not found.
}

// FlatSet<Value, Comparer> members.

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>::FlatSet(Comparer&& comparer, Alloc&& alloc) noexcept
: _values(static_cast<Alloc&&>(alloc)),
  _comparer(static_cast<Comparer&&>(comparer))
{
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>::FlatSet(std::initializer_list<Value> const values, Comparer&& comparer,
  Alloc&& alloc)
: _values(static_cast<Alloc&&>(alloc)),
  _comparer(static_cast<Comparer&&>(comparer))
{
  static_cast<void>(capacity(static_cast<Length>(values.size())));

  for (Value const* value = values.begin(); value != values.end(); ++value)
    if (!add(*value))
    {
      pollute();
      break;
    }
}

template <typename Value, typename Comparer, typename Alloc>
Value const& FlatSet<Value, Comparer, Alloc>::operator [] (Location const& location) const noexcept
{
  return _values[location.index()];
}

template <typename Value, typename Comparer, typename Alloc>
Value const* FlatSet<Value, Comparer, Alloc>::begin() const noexcept
{
  return _values.begin();
}

template <typename Value, typename Comparer, typename Alloc>
Value const* FlatSet<Value, Comparer, Alloc>::end() const noexcept
{
  return _values.end();
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>::operator bool () const noexcept
{
  return static_cast<bool>(_values);
}

template <typename Value, typename Comparer, typename Alloc>
typename FlatSet<Value, Comparer, Alloc>::Length FlatSet<Value, Comparer, Alloc>::capacity() const noexcept
{
  return _values.capacity();
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::capacity(Length const capacity)
{
  return _values.capacity(capacity);
}

template <typename Value, typename Comparer, typename Alloc>
typename FlatSet<Value, Comparer, Alloc>::Length FlatSet<Value, Comparer, Alloc>::length() const noexcept
{
  return _values.length();
}

template <typename Value, typename Comparer, typename Alloc>
void FlatSet<Value, Comparer, Alloc>::clear() noexcept
{
  _values.clear();
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::shrink() noexcept
{
  return _values.shrink();
}

template <typename Value, typename Comparer, typename Alloc>
void FlatSet<Value, Comparer, Alloc>::purge() noexcept
{
  _values.clear();
  static_cast<void>(_values.shrink());
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::exists(Value const& value) const noexcept
{
  Length index;
  return search(index, value);
}

template <typename Value, typename Comparer, typename Alloc>
typename FlatSet<Value, Comparer, Alloc>::Location FlatSet<Value, Comparer, Alloc>::add(Value const& value)
{
  Length index;
  if (!search(index, value) && !_values.insert(index, value))
    index = NotFound;
  return Location(index);
}

template <typename Value, typename Comparer, typename Alloc>
typename FlatSet<Value, Comparer, Alloc>::Location FlatSet<Value, Comparer, Alloc>::add(Value&& value)
{
  Length index;
  if (!search(index, value) && !_values.insert(index, static_cast<Value&&>(value)))
    index = NotFound;
  return Location(index);
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::update(Value const& value)
{
  Length index;
  if (!search(index, value))
    return _values.insert(index, value);
  else
  {
    _values[index] = value;
    return true;
  }
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::update(Value&& value)
{
  Length index;
  if (!search(index, value))
    return _values.insert(index, static_cast<Value&&>(value));
  else
  {
    _values[index] = static_cast<Value&&>(value);
    return true;
  }
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::insert(Location const& location, Value const& value)
{
  return _values.insert(location.index(), value);
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::insert(Location const& location, Value&& value)
{
  return _values.insert(location.index(), static_cast<Value&&>(value));
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>& FlatSet<Value, Comparer, Alloc>::addp(Value const& value)
{
  if (!add(value))
    pollute();
  return *this;
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>& FlatSet<Value, Comparer, Alloc>::addp(Value&& value)
{
  if (!add(static_cast<Value&&>(value)))
    pollute();
  return *this;
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::erase(Value const& value) noexcept
{
  Length index;
  if (search(index, value))
    return _values.erase(index);
  else
    return false; // Value does not exist
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::erase(Location const& location) noexcept
{
  return _values.erase(location.index());
}

template <typename Value, typename Comparer, typename Alloc>
typename FlatSet<Value, Comparer, Alloc>::Location FlatSet<Value, Comparer, Alloc>::find(
  Value const& value) const noexcept
{
  Length index;
  return search(index, value) ? index : NotFound;
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::find(Location& index, Value const& value) const noexcept
{
  return search(index, value);
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::empty() const noexcept
{
  return _values.empty();
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>& FlatSet<Value, Comparer, Alloc>::pollute() noexcept
{
  _values.pollute();
  return *this;
}

template <typename Value, typename Comparer, typename Alloc>
FlatSet<Value, Comparer, Alloc>& FlatSet<Value, Comparer, Alloc>::unpollute() noexcept
{
  _values.unpollute();
  return *this;
}

template <typename Value, typename Comparer, typename Alloc>
template <typename CustomValue, typename CustomCompare>
bool FlatSet<Value, Comparer, Alloc>::find(Location& location, CustomValue const& value,
  CustomCompare const& compare) const
{
  Length left = 0, right = _values.length() - 1;

  while (left <= right)
  {
    Length const pivot = (left + right) / 2;
    auto const res = compare(_values[pivot], value);

    if (res < 0)
      left = pivot + 1;
    else if (res > 0)
      right = pivot - 1;
    else
    {
      location = Location(pivot);
      return true; // Key found
    }
  }
  location = Location(left);
  return false; // Key not found
}

template <typename Value, typename Comparer, typename Alloc>
bool FlatSet<Value, Comparer, Alloc>::search(Length& index, Value const& value) const noexcept
{
  Length left = 0, right = _values.length() - 1;

  while (left <= right)
  {
    Length const pivot = (left + right) / 2;
    auto const res = _comparer(_values[pivot], value);

    if (res < 0)
      left = pivot + 1;
    else if (res > 0)
      right = pivot - 1;
    else
    {
      index = pivot;
      return true; // Key found
    }
  }
  index = left;
  return false; // Key not found
}

} // namespace trl