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

#include "TinyTRL_Streams.h"
#include "TinyTRL_Containers.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #ifndef _FILE_OFFSET_BITS
    #define _FILE_OFFSET_BITS 64
  #endif
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <errno.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

namespace trl {

// Generic invalid or unallocated streamHandle value.
static FileStream::Handle const invalidHandle =
#ifdef _WIN32
  INVALID_HANDLE_VALUE
#else
  reinterpret_cast<FileStream::Handle>(static_cast<intptr_t>(-1))
#endif
;

// Stream members.

Stream::Stream()
: _status(0)
{
}

Stream::operator bool() const
{
  return !(_status & Status::Pollute);
}

Stream::Size Stream::read(void*, Size)
{
  return Failure;
}

Stream::Size Stream::write(void const*, Size)
{
  return Failure;
}

Stream::Offset Stream::seek(Offset, SeekOrigin)
{
  return Failure;
}

bool Stream::truncate()
{
  return false; // Not supported.
}

bool Stream::flush()
{
  return false; // Not supported.
}

Stream::Offset Stream::position()
{
  return seek(0, SeekOrigin::Current);
}

Stream::Offset Stream::size()
{
  return Failure;
}

Stream::Offset Stream::copy(Stream& source, Offset const size, Size blockSize)
{
  Offset totalBytesRead = Failure;

  { // Buffer size should not exceed the size of source stream.
    Offset const position = source.position();
    Offset const sourceSize = source.size();

    if (position >= 0 && sourceSize > 0)
    {
      blockSize = static_cast<Size>(math::max<Offset>(math::min(math::min<Offset>(
        sourceSize - position, blockSize), size > 0 ? size : blockSize), 0));
    }
  }

  if (blockSize > 0 && size >= 0)
    if (uint8_t* buffer = reinterpret_cast<uint8_t*>(::malloc(blockSize)))
    {
      totalBytesRead = 0;
      Size storedBytes = 0;
      bool available = true;

      while (true)
      {
        if (storedBytes < blockSize && available)
        { // Read additional data into intermediary buffer.
          Size const availableSize = blockSize - storedBytes;
          Size const bytesToRead = size != 0 ? static_cast<Size>(math::min<Offset>(
            size - totalBytesRead, availableSize)) : availableSize;

          Size const bytesRead = source.read(buffer + storedBytes, bytesToRead);
          if (bytesRead < 0)
          {
            pollute(); // Failed reading from source stream.
            break;
          }
          available = bytesRead != 0;
          storedBytes += bytesRead;
          totalBytesRead += bytesRead;
        }

        if (storedBytes != 0)
        { // Write data from intermediary buffer.
          Size const bytesWritten = write(buffer, storedBytes);
          if (bytesWritten <= 0)
          {
            pollute(); // Failed writing to current stream.
            break;
          }

          if (Size const bytesRemaining = math::max<Size>(storedBytes - bytesWritten, 0))
            ::memmove(buffer, buffer + bytesWritten, static_cast<size_t>(bytesRemaining));

          storedBytes -= bytesWritten;
        }
        else
          break; // End of source stream reached and all bytes have been copied.
      }
      ::free(buffer);
    }

  if (totalBytesRead == Failure)
    pollute();

  return totalBytesRead;
}

void Stream::pollute()
{
  _status |= Status::Pollute;
}

Stream& Stream::readBuffer(void* const buffer, Size const size)
{
  if (size >= 0)
  {
    Size const bytesRead = read(buffer, size);
    if (bytesRead < size)
    {
      Size const bytesSkip = math::max<Size>(bytesRead, 0);
      ::memset(reinterpret_cast<char*>(buffer) + bytesSkip, 0, static_cast<size_t>(size - bytesSkip));
      pollute();
    }
  }
  else
    pollute(); // Cannot read a negative number of bytes.

  return *this;
}

Stream& Stream::writeBuffer(void const* const buffer, Size const size)
{
  if (size >= 0)
  {
    if (write(buffer, size) != size)
      pollute();
  }
  else
    pollute(); // Cannot write a negative number of bytes.

  return *this;
}

Stream& Stream::operator << (String const& string)
{
  writeString(string);
  return *this;
}

Stream& Stream::operator >> (String& string)
{
  readString(string);
  return *this;
}

void Stream::readString(String& string)
{
  Size remainingBytes = -1;
  {
    Offset const position = this->position();
    Offset const size = this->size();

    if (position >= 0 && size > 0)
      remainingBytes = static_cast<Size>(math::max<Offset>(size - position, 0));
  }
  if (remainingBytes != -1)
  { // Remaining bytes have been retrieved.
    if (string.length() > String::MaxLength - remainingBytes)
    {
      remainingBytes = String::MaxLength - string.length();
      string.pollute(); // Overflow, the data will be partially read.

      if (remainingBytes <= 0)
        return; // No more space to read anything.
    }
    if (!string.capacity(string.length() + remainingBytes))
    {
      string.pollute(); // Memory allocation failed.
      return;
    }
    Size const bytesRead = read(&string[string.length()], remainingBytes);
    if (bytesRead >= 0)
      static_cast<void>(string.length(string.length() + bytesRead));
    else
      pollute();
  }
  else
  { // Perform buffered read (unknown source stream size).
    static Size constexpr const bufferSize = 65536;

    while (true)
    {
      Size allocatedBufferSize = bufferSize;

      if (string.length() > String::MaxLength - allocatedBufferSize)
      {
        allocatedBufferSize = String::MaxLength - string.length();
        if (allocatedBufferSize <= 0)
        {
          string.pollute(); // Overflow.
          return;
        }
      }
      if (!string.capacity(string.length() + allocatedBufferSize))
      {
        string.pollute(); // Memory allocation failed.
        break;
      }
      Size const bytesRead = read(&string[string.length()], string.capacity() - string.length());
      if (bytesRead > 0)
        static_cast<void>(string.length(string.length() + bytesRead));
      else
      {
        if (bytesRead < 0)
          pollute();
        break;
      }
    }
  }
}

void Stream::writeString(String const& string)
{
  Size const size = static_cast<Size>(string.length());
  if (size > 0)
    writeBuffer(string.data(), size);
}

// FileStream members.

FileStream::FileStream(String const& fileName, FileMode const mode, FileAttributes const attributes)
: _handle(invalidHandle)
{
#ifdef _WIN32
  DWORD desiredAccess = 0;
  DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
  DWORD creationDisposition;
  DWORD const flagsAttributes = attributes ? attributes : FILE_ATTRIBUTE_NORMAL;

  if (mode & ModeWrite)
  {
    desiredAccess |= GENERIC_WRITE;

    if (mode & ModeTruncate)
      creationDisposition = CREATE_ALWAYS;
    else
      creationDisposition = OPEN_ALWAYS;
  }
  else
    creationDisposition = OPEN_EXISTING;

  if (mode & ModeRead)
    desiredAccess |= GENERIC_READ;

  if (mode & ShareDenyRead)
    shareMode &= ~FILE_SHARE_READ;

  if (mode & ShareDenyWrite)
    shareMode &= ~FILE_SHARE_WRITE;

  if (mode & ShareDenyDelete)
    shareMode &= ~FILE_SHARE_DELETE;

  if (WideString fileNameWide(fileName); fileNameWide && !fileNameWide.empty())
    _handle = CreateFileW(fileNameWide.data(), desiredAccess, shareMode, nullptr, creationDisposition,
      flagsAttributes, nullptr);
#else
  if (((mode & (ModeRead | ModeWrite)) != 0) && ((mode & ShareDenyDelete) == 0))
  {
    int flags = 0;

    if (mode & ModeWrite)
    {
      flags |= O_CREAT | (mode & ModeRead ? O_RDWR : O_WRONLY);
      if (mode & ModeTruncate)
        flags |= O_TRUNC;
    }
    else
      flags |= O_RDONLY;

    int const fileMode = attributes ? attributes : 0666;

    if (fileName && !fileName.empty())
      _handle = reinterpret_cast<Handle>(::open(fileName.data(), flags, fileMode));
    if (_handle != invalidHandle && (mode & ShareExclusive) != 0)
    {
      struct flock lock = {};
      lock.l_whence = SEEK_SET;

      if ((mode & ShareDenyRead) == 0)
        lock.l_type = F_RDLCK;
      else
        lock.l_type = F_WRLCK;

      int const lockRes = fcntl(reinterpret_cast<intptr_t>(_handle), F_SETLK, &lock);
      int const resCode = errno;
      if (lockRes == -1 && resCode != EINVAL && resCode != ENOTSUP)
      { // EINVAL or ENOTSUP means file doesn't support locking.
        close(reinterpret_cast<intptr_t>(_handle));
        _handle = invalidHandle;
      }
    }
  }
#endif
}

FileStream::~FileStream()
{
  releaseHandle(_handle);
}

FileStream::FileStream(FileStream const& stream)
: _handle(stream._handle)
{
}

FileStream::FileStream(FileStream&& stream) noexcept
: _handle(stream._handle)
{
  stream._handle = invalidHandle;
}

FileStream& FileStream::operator = (FileStream const& stream)
{
  releaseHandle(_handle);
  _handle = stream._handle;
  return *this;
}

FileStream& FileStream::operator = (FileStream&& stream) noexcept
{
  releaseHandle(_handle);
  utility::swap(_handle, stream._handle);
  return *this;
}

FileStream::operator bool() const
{
  return validHandle() && Stream::operator bool();
}

Stream::Size FileStream::read(void* const buffer, Size const size)
{
  Size bytesRead = Failure;

  if (validHandle() && size >= 0)
  {
    if (size)
    {
    #ifdef _WIN32
      DWORD actualBytesRead;
      if (ReadFile(_handle, buffer, static_cast<DWORD>(size), &actualBytesRead, nullptr))
        bytesRead = static_cast<Size>(actualBytesRead);
    #else
      bytesRead = ::read(reinterpret_cast<intptr_t>(_handle), buffer, size);
    #endif
    }
    else
      bytesRead = 0;
  }
  return bytesRead;
}

Stream::Size FileStream::write(void const* const buffer, Size const size)
{
  Size bytesWritten = Failure;

  if (validHandle() && size >= 0)
  {
    if (size)
    {
    #ifdef _WIN32
      DWORD actualBytesWritten;
      if (WriteFile(_handle, buffer, static_cast<DWORD>(size), &actualBytesWritten, nullptr))
        bytesWritten = static_cast<Size>(actualBytesWritten);
    #else
      bytesWritten = ::write(reinterpret_cast<intptr_t>(_handle), buffer, size);
    #endif
    }
    else
      bytesWritten = 0;
  }
  return bytesWritten;
}

Stream::Offset FileStream::seek(Offset const offset, SeekOrigin const origin)
{
  Offset position = Failure;

  if (validHandle())
  {
  #ifdef _WIN32
    DWORD moveMethod;

    switch (origin)
    {
    case SeekOrigin::Current:
      moveMethod = FILE_CURRENT;
      break;

    case SeekOrigin::End:
      moveMethod = FILE_END;
      break;

    default:
      moveMethod = FILE_BEGIN;
    }

    LARGE_INTEGER temp;
    temp.QuadPart = offset;
    temp.LowPart = SetFilePointer(_handle, temp.LowPart, &temp.HighPart, moveMethod);

    if (temp.LowPart != INVALID_SET_FILE_POINTER || GetLastError() == NO_ERROR)
      position = temp.QuadPart;
  #else
    off_t whence;

    switch (origin)
    {
    case SeekOrigin::Current:
      whence = SEEK_CUR;
      break;

    case SeekOrigin::End:
      whence = SEEK_END;
      break;

    default:
      whence = SEEK_SET;
    }
    position = lseek(reinterpret_cast<intptr_t>(_handle), offset, whence);
  #endif
  }
  return position;
}

bool FileStream::truncate()
{
  bool truncated = false;

  if (validHandle())
  {
  #ifdef _WIN32
    truncated = SetEndOfFile(_handle);
  #else
    truncated = ftruncate(reinterpret_cast<intptr_t>(_handle), position()) == 0;
  #endif
  }
  return truncated;
}

bool FileStream::flush()
{
  bool flushed = false;

  if (validHandle())
  {
  #ifdef _WIN32
    flushed = FlushFileBuffers(_handle);
  #else
    flushed = fdatasync(reinterpret_cast<intptr_t>(_handle)) == 0;
  #endif
  }
  return flushed;
}

Stream::Offset FileStream::size()
{
  Offset fileSize = Failure;

  if (validHandle())
  {
  #ifdef _WIN32
    LARGE_INTEGER temp;
    if (GetFileSizeEx(_handle, &temp))
      fileSize = temp.QuadPart;
  #else
    struct stat fileStat{};
    if (fstat(reinterpret_cast<intptr_t>(_handle), &fileStat) == 0)
      fileSize = fileStat.st_size;
  #endif
  }
  return fileSize;
}

FileStream::Handle FileStream::handle() const
{
  return _handle;
}

String FileStream::loadString(String const& fileName)
{
  String string;

  FileStream stream(fileName, ModeRead | ShareDenyWrite);
  if (stream)
  {
    Offset size = stream.size();
    if (size > 0)
    { // Preallocate the string to avoid multiple allocations.
      size = math::min(size, static_cast<Offset>(String::MaxLength));

      if (!string.capacity(static_cast<String::Length>(size)))
        string.pollute();
    }
    stream.readString(string);
  }
  else
    string.pollute();

  return string;
}

bool FileStream::saveString(String const& fileName, String const& contents)
{
  FileStream stream(fileName, ModeCreate | ShareExclusive);
  if (stream)
  {
    stream.writeString(contents);
    return static_cast<bool>(stream);
  }
  else
    return false;
}

bool FileStream::fileExists(String const& fileName)
{
#ifdef _WIN32
  WideString fileNameWide;

  if (fileName.length() < MAX_PATH)
    fileNameWide = WideString(fileName);
  else
    fileNameWide = WideString("\\\\?\\" + fileName);

  if (fileNameWide && !fileNameWide.empty())
  {
    DWORD const attributes = GetFileAttributesW(fileNameWide.data());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
  }
  else
    return false;
#else
  if (fileName && !fileName.empty())
  {
    struct stat fileStat;
    return !stat(fileName.data(), &fileStat) && S_ISREG(fileStat.st_mode);
  }
  else
    return false;
#endif
}

bool FileStream::directoryExists(String const& directoryName)
{
#ifdef _WIN32
  WideString directoryNameWide;

  if (directoryName.length() < MAX_PATH)
    directoryNameWide = WideString(directoryName);
  else
    directoryNameWide = WideString("\\\\?\\" + directoryName);

  if (directoryNameWide && !directoryNameWide.empty())
  {
    DWORD const attributes = GetFileAttributesW(directoryNameWide.data());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
  }
  else
    return false;
#else
  if (directoryName && !directoryName.empty())
  {
    struct stat fileStat;
    return !stat(directoryName.data(), &fileStat) && S_ISDIR(fileStat.st_mode);
  }
  else
    return false;
#endif
}

bool FileStream::createDirectory(String const& directoryName)
{
#ifdef _WIN32
  WideString directoryNameWide;

  if (directoryName.length() < MAX_PATH)
    directoryNameWide = WideString(directoryName);
  else
    directoryNameWide = WideString("\\\\?\\" + directoryName);

  if (directoryNameWide && !directoryNameWide.empty())
    return CreateDirectoryW(directoryNameWide.data(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
  else
    return false;
#else
  if (directoryName && !directoryName.empty())
  {
    int res = mkdir(directoryName.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (!res)
      return true;

    switch (errno)
    {
    case ENOENT:
      { // Parent does not exist, try to create it.
        String::Length const separator = utility::findCharLast(directoryName, '/');
        if (separator == String::NotFound)
          return false; // No parent in path.
        if (!createDirectory(directoryName.substr(0, separator)))
          return false; // Cannot create parent directory.
      }
      return !mkdir(directoryName.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    case EEXIST:
      return directoryExists(directoryName); // Make sure it is really a directory.

    default:
      return false;
    }
  }
  else
    return false;
#endif
}

void FileStream::releaseHandle(Handle& streamHandle) const
{
  if (streamHandle != invalidHandle)
  {
  #ifdef _WIN32
    CloseHandle(streamHandle);
  #else
    close(reinterpret_cast<intptr_t>(streamHandle));
  #endif
    streamHandle = invalidHandle;
  }
}

bool FileStream::validHandle() const
{
  return _handle != invalidHandle;
}

// BaseMemoryStream members.

BaseMemoryStream::BaseMemoryStream()
: Stream(),
  _position(0),
  _size(0)
{
}

BaseMemoryStream::BaseMemoryStream(BaseMemoryStream const& stream)
: _position(stream._position),
  _size(stream._size)
{
}

BaseMemoryStream::BaseMemoryStream(BaseMemoryStream&& stream) noexcept
: _position(stream._position),
  _size(stream._size)
{
  stream._position = stream._size = 0;
}

BaseMemoryStream& BaseMemoryStream::operator = (BaseMemoryStream const& stream)
{
  _position = stream._position;
  _size = stream._size;
  return *this;
}

BaseMemoryStream& BaseMemoryStream::operator = (BaseMemoryStream&& stream) noexcept
{
  _position = stream._position;
  _size = stream._size;
  stream._position = stream._size = 0;
  return *this;
}

Stream::Offset BaseMemoryStream::seek(Offset const offset, SeekOrigin const origin)
{
  switch (origin)
  {
  case SeekOrigin::Current:
    if (offset >= 0)
      _position += static_cast<size_t>(offset);
    else
    {
      size_t const displace = static_cast<size_t>(-offset);
      if (displace > _position)
        _position = 0;
      else
        _position -= displace;
    }
    break;

  case SeekOrigin::End:
    if (offset >= 0)
      _position = _size + static_cast<size_t>(offset);
    else
    {
      size_t const displace = static_cast<size_t>(-offset);
      if (displace > _size)
        _position = 0;
      else
        _position = _size - displace;
    }
    break;

  default:
    if (offset >= 0)
      _position = static_cast<size_t>(offset);
    else
      _position = 0;
  }
  return static_cast<Offset>(_position);
}

Stream::Offset BaseMemoryStream::position()
{
  return _position;
}

Stream::Offset BaseMemoryStream::size()
{
  return _size;
}

Stream::Size BaseMemoryStream::readBytes(void* const dest, DataType const* const source, Size const size)
{
  Size bytesRead = Failure;

  if (size >= 0)
  {
    if (size)
    {
      size_t const remainingBytes = _position <= _size ? _size - _position : 0;
      bytesRead = math::min(size, static_cast<Size>(remainingBytes));

      if (bytesRead)
      {
        ::memcpy(dest, source + _position, static_cast<size_t>(bytesRead));
        _position += static_cast<size_t>(bytesRead);
      }
    }
    else
      bytesRead = 0;
  }
  return bytesRead;
}

// MemoryStream members.

MemoryStream::MemoryStream(size_t const capacity)
: BaseMemoryStream(),
  _buffer(nullptr),
  _capacity(0)
{
  if (capacity)
  {
    DataType* const buffer = static_cast<DataType*>(::malloc(capacity));
    if (buffer)
    {
      _buffer = buffer;
      _capacity = capacity;
    }
  }
}

MemoryStream::~MemoryStream()
{
  if (_buffer)
    ::free(_buffer);
}

MemoryStream::MemoryStream(MemoryStream const& stream)
: MemoryStream(stream._capacity)
{
  if (_buffer || stream._capacity == 0)
  {
    if (stream._size != 0)
      ::memcpy(_buffer, stream._buffer, stream._size);
    _position = stream._position;
    _size = stream._size;
  }
}

MemoryStream::MemoryStream(MemoryStream&& stream) noexcept
: BaseMemoryStream(stream),
  _buffer(stream._buffer),
  _capacity(stream._capacity)
{
  stream._buffer = nullptr;
  stream._capacity = 0;
}

MemoryStream& MemoryStream::operator = (MemoryStream const& stream)
{
  if (_capacity == stream._capacity || reallocate(stream._capacity))
  {
    if (stream._size != 0)
      ::memcpy(_buffer, stream._buffer, stream._size);

    _position = stream._position;
    _size = stream._size;
  }
  return *this;
}

MemoryStream& MemoryStream::operator = (MemoryStream&& stream) noexcept
{
  if (_buffer)
    ::free(_buffer);

  _buffer = stream._buffer;
  _capacity = stream._capacity;
  stream._buffer = nullptr;
  stream._capacity = 0;

  BaseMemoryStream::operator = (static_cast<MemoryStream&&>(stream));

  return *this;
}

Stream::Size MemoryStream::read(void* const buffer, Size const size)
{
  return readBytes(buffer, _buffer, size);
}

Stream::Size MemoryStream::write(void const* const buffer, Size size)
{
  Size bytesWritten = Failure;

  if (size >= 0)
  {
    size = static_cast<Size>(math::min(static_cast<size_t>(size), SIZE_MAX - _position));
    if (size)
    {
      size_t const tentativeSize = _position + static_cast<size_t>(size);

      if (tentativeSize <= _capacity || reallocate(math::computeNextCapacity(tentativeSize, _capacity)))
      {
        ::memcpy(_buffer + _position, buffer, size);
        _position += static_cast<size_t>(size);
        _size = math::max(_size, _position);
        bytesWritten = size;
      }
    }
    else
      bytesWritten = 0;
  }
  return bytesWritten;
}

bool MemoryStream::truncate()
{
  bool truncated = true;

  if (_position < _size)
    _size = _position;
  else if (_position > _size)
  {
    if (_position <= _capacity || reallocate(math::computeNextCapacity(_position, _capacity)))
      _size = _position;
    else
      truncated = false; // Memory allocation failed.
  }
  return truncated;
}

bool MemoryStream::flush()
{
  return true;
}

Stream::Offset MemoryStream::copy(Stream& source, Offset const size, Size blockSize)
{
  {
    Offset const position = source.position();
    Offset const sourceSize = source.size();

    if (position >= 0 && sourceSize > 0)
    {
      if (position >= sourceSize)
        return 0; // End of source stream reached.

      blockSize = static_cast<Size>(math::max<Offset>(math::min(math::min<Offset>(
        sourceSize - position, blockSize), size > 0 ? size : blockSize), 0));

      if (_position == 0 && _capacity == 0 && blockSize > 0)
      { // Preallocate buffer to match exactly source stream.
        if (!reallocate(blockSize))
          return Failure; // Failed to pre-allocate memory.
      }
    }
  }

  Offset totalBytesRead = Failure;

  if (size >= 0 && blockSize > 0)
  {
    totalBytesRead = 0;

    for (Size bytesRead = blockSize; bytesRead > 0; )
    {
      size_t const tentativeSize = _position + static_cast<size_t>(blockSize);
      if (tentativeSize > _capacity && !reallocate(math::computeNextCapacity(tentativeSize, _capacity)))
      {
        pollute(); // Memory allocation failed.
        break;
      }

      bytesRead = source.read(_buffer + _position, blockSize);
      if (bytesRead < 0)
      {
        pollute(); // Failed reading from source stream.
        break;
      }

      _position += static_cast<size_t>(bytesRead);
      _size = math::max(_size, _position);
      totalBytesRead += bytesRead;
    };
  }
  return totalBytesRead;
}

MemoryStream::DataType* MemoryStream::memory()
{
  return _buffer;
}

MemoryStream::DataType const* MemoryStream::memory() const
{
  return _buffer;
}

size_t MemoryStream::capacity() const
{
  return _capacity;
}

bool MemoryStream::capacity(size_t const capacity)
{
  if (_capacity < capacity)
    return reallocate(math::computeNextCapacity(capacity, _capacity));
  else
    return true; // Current capacity is bigger than the requested one.
}

bool MemoryStream::shrinkToFit()
{
  bool shrank = true;

  if (_size)
  {
    if (_size < _capacity)
      shrank = reallocate(_size);
  }
  else
    clear();

  return shrank;
}

void MemoryStream::clear()
{
  _size = 0;
  _status &= ~Status::Pollute;
}

bool MemoryStream::reallocate(size_t const capacity)
{
  bool reallocated = true;

  DataType* const buffer = static_cast<DataType*>(::realloc(_buffer, capacity));
  if (buffer || capacity == 0)
  {
    _buffer = buffer;
    _capacity = capacity;
  }
  else
    reallocated = false;

  return reallocated;
}

} // namespace trl