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

// TinyTRL_Streams.h
#pragma once

#include "TinyTRL_Strings.h"

namespace trl {

/// Origin used for stream seek operations.
enum class SeekOrigin : uint8_t
{
  /// Starting position in the stream.
  Beginning,

  /// Current position in the stream.
  Current,

  /// Ending position in the stream.
  End
};

/// General-purpose stream for input/output operations.
class Stream
{
public:
  /// Generic type used for specifying buffer sizes when accessing the stream.
#ifdef __PLATFORM_X64
  typedef int64_t Size;
#else
  typedef int32_t Size;
#endif

  /// Generic type used for specifying offset and position inside the stream.
  typedef int64_t Offset;

  /// Constant that denotes an operation that is not supported.
  static Size constexpr const Failure = -1;

  /// Default block size used for copy operations.
  static Size constexpr const DefaultBlockSize = 8192;

  /// Default stream destructor.
  virtual ~Stream() = default;

  /// Copy (and move) constructor is not allowed for this base class.
  Stream(Stream const&) = delete;

  /// Copy (and move) assignment operator is not allowed for this base class.
  Stream& operator = (Stream const&) = delete;

  /// Tests whether a stream is not polluted. A polluted stream has an error bit set after a failed read
  /// or write operation (template versions of read() and write()). A stream operation that involves
  /// another polluted stream will result in current stream being polluted. Therefore, this polluted state
  /// propagates and persists throughout current stream's lifetime. Higher-level classes may override
  /// this operator to add additional state validation.
  [[nodiscard]] virtual explicit operator bool () const;

  /// Reads a given number of bytes from stream to buffer. Returns the actual number of bytes read or
  /// a failure code.
  [[nodiscard]] virtual Size read(void* buffer, Size size);

  /// Writes a given number of bytes to stream from buffer. Returns the actual number of bytes written or
  /// a failure code.
  [[nodiscard]] virtual Size write(void const* buffer, Size size);

  /// Adjusts the position in the stream relative to given origin and returns the actual position respective
  /// to the beginning of the stream, or a failure code.
  virtual Offset seek(Offset offset, SeekOrigin origin = SeekOrigin::Beginning);

  /// Truncates the stream at the current position.
  virtual bool truncate();

  /// Flushes file buffers and causes all previously buffered data to be written, if available.
  virtual bool flush();

  /// Returns current position relative to the beginning of the stream.
  [[nodiscard]] virtual Offset position();

  /// Returns the size of the stream.
  [[nodiscard]] virtual Offset size();

  /// Copies a specific number of bytes from source stream. If the size is not specified, then the whole
  /// remainder of source stream is going to be copied. The copying process starts at the appropriate
  /// current positions in both streams. Returns number of bytes actually copied or a failure code.
  /// In case of any failure (memory allocation, read and/or write error), sets an error bit.
  virtual Offset copy(Stream& source, Offset size = 0, Size blockSize = DefaultBlockSize);

  /// Sets an error bit in the stream, marking it as polluted.
  void pollute();

  /// Reads a fixed number of bytes to the given buffer. If number of bytes read is less than requested,
  /// the remaining bytes will be filled by zeros and an error bit will be set.
  Stream& readBuffer(void* buffer, Size size);

  /// Writes a fixed number of bytes from the given buffer. If number of bytes written is less than
  /// requested, then an error bit will be set.
  Stream& writeBuffer(void const* buffer, Size size);

  /// Writes the contents of a given string to the stream.
  Stream& operator << (String const& string);

  /// Reads the remainder of the stream to the given string.
  Stream& operator >> (String& string);

  /// Reads a specific data structure from the stream. In case of failure sets an error bit.
  template<typename ValueType>
  inline ValueType read()
  {
    ValueType value;
    readBuffer(&value, static_cast<Size>(sizeof(value)));
    return value;
  }

  /// Writes a specific data structure to the stream. In case of failure sets an error bit.
  template <typename ValueType>
  inline void write(ValueType const& value)
  {
    writeBuffer(&value, static_cast<Size>(sizeof(value)));
  }

  /// Reads a specific data structure from the stream. In case of failure sets an error bit.
  template <typename ValueType>
  inline Stream& operator >> (ValueType& value)
  {
    value = read<ValueType>();
    return *this;
  }

  /// Writes a specific data structure to the stream. In case of failure sets an error bit.
  template <typename ValueType>
  inline Stream& operator << (ValueType const& value)
  {
    write(value);
    return *this;
  }

protected:
  /// Stream's status bits.
  struct Status {
    enum {
      /// Bit flag that denotes a polluted state.
      Pollute = 0x80
    };
  };

  /// Status bits that define current state of the stream.
  uint8_t _status;

  /// Default stream constructor.
  Stream();

  /// Reads the remaining contents of the stream to the given string. In case of read failure, sets an error
  /// bit and marks returned string as polluted.
  void readString(String& string);

  /// Writes the contents of the given string to the stream. If string to write is polluted, then an error
  /// bit will be set in the stream.
  void writeString(String const& string);
};

/// Reads the remainder of the stream to a string.
template<>
inline String Stream::read()
{
  String string;
  readString(string);
  return string;
}

/// Writes the contents of a given string to the stream.
template<>
inline void Stream::write(String const& value)
{
  writeString(value);
}

/// Stream that provides read and write access to files.
class FileStream : public Stream
{
public:
  /// File mode and sharing flags.
  enum
  {
    /// Data is going to be read from file.
    ModeRead = 0x01,

    /// Data is going to be written to file.
    ModeWrite = 0x02,

    /// File is going to be truncated if it already exists.
    ModeTruncate = 0x04,

    /// Prevent other processes reading from the file.
    ShareDenyRead = 0x10,

    /// Prevent other processes writing to the file.
    ShareDenyWrite = 0x20,

    /// Prevent other processes deleting (or renaming) the file.
    ShareDenyDelete = 0x40,

    /// Open file for read/write and truncate one if such exists.
    ModeCreate = ModeRead | ModeWrite | ModeTruncate,

    /// Open file for read/write and create one if such doesn't exist.
    ModeAppend = ModeRead | ModeWrite,

    /// Prevent other processes accessing the file.
    ShareExclusive = ShareDenyRead | ShareDenyWrite
  };

  /// One or more file mode and optionally share flags ORed together.
  typedef uint32_t FileMode;

  /// File attributes (rights for Unix-based systems, flags/attributes for Windows-based systems).
  typedef uint32_t FileAttributes;

  /// Generic file stream handle.
  typedef void* Handle;

  // Re-using read() function from stream class.
  using Stream::read;

  // Re-using write() function from stream class.
  using Stream::write;

  /// Creates a new instance of file stream for the given file name and parameters.
  FileStream(String const& fileName, FileMode mode, FileAttributes attributes = 0);

  /// Releases file stream and closes its file handle.
  ~FileStream() override;

  /// Creates a new instance of file stream, copying file handle from another instance.
  FileStream(FileStream const& stream);

  /// Creates a new instance of file stream, taking file handle from another instance.
  FileStream(FileStream&& stream) noexcept;

  /// Copies file handle from another file stream instance into the current one.
  FileStream& operator = (FileStream const& stream);

  /// Moves file handle from another file stream instance into the current one.
  FileStream& operator = (FileStream&& stream) noexcept;

  /// Tests whether a stream is not polluted and its handle is valid.
  [[nodiscard]] explicit operator bool () const override;

  /// Reads a given number of bytes from stream to buffer. Returns the actual number of bytes read or
  /// a failure code.
  [[nodiscard]] Size read(void* buffer, Size size) override;

  /// Writes a given number of bytes to stream from buffer. Returns the actual number of bytes written or
  /// a failure code.
  [[nodiscard]] Size write(void const* buffer, Size size) override;

  /// Adjusts the position in the stream relative to given origin and returns the actual position respective
  /// to the beginning of the stream, or a failure code.
  Offset seek(Offset offset, SeekOrigin origin = SeekOrigin::Beginning) override;

  /// Truncates file stream at its current position.
  bool truncate() override;

  /// Flushes file buffers and causes all previously buffered data to be written, if available.
  bool flush() override;

  /// Returns the size of the file.
  [[nodiscard]] Offset size() override;

  /// Returns handle associated with the stream.
  Handle handle() const;

  /// Loads contents of the given file into a string.
  /// Note: in case of UTF-8 text files, the string may start with a BOM marker (0xEF, 0xBB, 0xBF).
  static String loadString(String const& fileName);

  /// Saves contents of the given string into a given file.
  static bool saveString(String const& fileName, String const& contents);

  /// Tests whether the given file exists.
  static bool fileExists(String const& fileName);

  /// Tests whether the given directory exists.
  static bool directoryExists(String const& directoryName);

  /// Creates directory, if such doesn't exist.
  static bool createDirectory(String const& directoryName);

private:
  // Handle of the stream.
  Handle _handle;

  // Releases the handle value if such is valid and sets it to invalid state.
  void releaseHandle(Handle& handle) const;

  // Checks whether current handle is valid.
  bool validHandle() const;
};

/// Memory wrapper stream that facilitates operations over a memory pointer.
class BaseMemoryStream : public Stream
{
public:
  /// Generic data type used by the stream for storage.
  using DataType = uint8_t;

  /// Adjusts the position in the stream relative to given origin and returns the actual position respective
  /// to the beginning of the stream.
  Offset seek(Offset offset, SeekOrigin origin = SeekOrigin::Beginning) override;

  /// Returns current position relative to the beginning of the stream.
  [[nodiscard]] Offset position() override;

  /// Returns the size of the stream.
  [[nodiscard]] Offset size() override;

protected:
  /// Current position in the buffer.
  size_t _position;

  /// Size of actual data in the buffer.
  size_t _size;

  /// Creates a new instance of memory stream.
  BaseMemoryStream();

  /// Creates a new instance of stream being a copy of another instance.
  BaseMemoryStream(BaseMemoryStream const& stream);

  /// Creates a new instance of stream and moves into it the contents of source class.
  BaseMemoryStream(BaseMemoryStream&& stream) noexcept;

  /// Copies contents of source memory stream instance into the current one.
  BaseMemoryStream& operator = (BaseMemoryStream const& stream);

  /// Moves contents of source memory stream into into the current one.
  BaseMemoryStream& operator = (BaseMemoryStream&& stream) noexcept;

  /// Reads a given number of bytes from the memory buffer to destination buffer.
  [[nodiscard]] Size readBytes(void* dest, DataType const* source, Size size);
};

/// Stream implementation that stores data directly in memory.
class MemoryStream : public BaseMemoryStream
{
public:
  using Stream::read;
  using Stream::write;

  /// Creates a new instance of memory stream, optionally with a preallocated buffer of the given size.
  MemoryStream(size_t capacity = 0);

  /// Releases current instance of memory stream.
  ~MemoryStream() override;

  /// Creates a new instance of memory stream being a copy of another instance.
  MemoryStream(MemoryStream const& stream);

  /// Creates a new instance of memory stream and moves into it the contents of source class.
  MemoryStream(MemoryStream&& stream) noexcept;

  /// Copies contents of source memory stream instance into the current one.
  MemoryStream& operator = (MemoryStream const& stream);

  /// Moves contents of source memory stream into the current one.
  MemoryStream& operator = (MemoryStream&& stream) noexcept;

  /// Reads a given number of bytes from the stream to buffer and returns the actual number of bytes read.
  [[nodiscard]] Size read(void* buffer, Size size) override;

  /// Writes a given number of bytes to the stream from buffer and returns the actual number of bytes written.
  [[nodiscard]] Size write(void const* buffer, Size size) override;

  /// Truncates the stream at the current position.
  bool truncate() override;

  /// Flushes file buffers and causes all previously buffered data to be written, if available.
  bool flush() override;

  /// Copies a specific number of bytes from source stream. If the size is not specified, then the whole
  /// remainder of source stream is going to be copied. The copying process starts at the appropriate
  /// current positions in both streams. Returns number of bytes actually copied.
  Offset copy(Stream& source, Offset size = 0, Size blockSize = DefaultBlockSize) override;

  /// Returns a direct pointer to stream buffer.
  [[nodiscard]] DataType* memory();

  /// Returns a read-only direct pointer to stream buffer.
  [[nodiscard]] DataType const* memory() const;

  /// Returns current buffer capacity.
  [[nodiscard]] size_t capacity() const;

  /// Changes current buffer capacity.
  [[nodiscard]] bool capacity(size_t capacity);

  /// Shrinks the buffer to fit the stream size.
  [[nodiscard]] bool shrinkToFit();

  /// Sets memory stream length to zero and resets error flag removing pollute status while preserving
  /// currently allocated capacity.
  void clear();

protected:
  /// Buffer that contains stream data.
  DataType* _buffer;

  /// Current allocated size of the buffer.
  size_t _capacity;

  // Reallocates buffer to the given capacity.
  bool reallocate(size_t capacity);
};

} // namespace trl