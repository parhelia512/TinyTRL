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

#include <stdio.h>
#include <math.h>

#include "TinyTRL.h"

using namespace trl;

struct TestStruct
{
  char name[32];
  float value;
};

static String const directoryForTesting = "TestDirectory";

static void exampleFileStream()
{
  String directory = directoryForTesting;

  // Try to create test directory.
  if (!FileStream::directoryExists(directory) && !FileStream::createDirectory(directory))
  {
    printf("Error! Could not create \"TestDirectory\"...\n");
    return;
  }
  // Add "/" or "\" to directory name, depending on current OS type.
  directory += utility::PathDelimeter;

  { // Create a new file to start writing to.
    FileStream fileStream(directory + "TestFile.dat", FileStream::ModeCreate | FileStream::ShareExclusive);
    if (!fileStream)
    {
      printf("Error! Could not create test file.\n");
      return;
    }
    // Write some values directly.
    fileStream.write<uint8_t>(148); // Write 8-bit unsigned value.
    fileStream.write<char>('c'); // Write character.
    fileStream.write<int16_t>(15001); // Write 16-bit signed value.

    // Another way of writing values to stream.
    fileStream << static_cast<int32_t>(254012); // Write 32-bit signed integer to file.

    { // Write some structure testing the resulting number of bytes that was written.
      TestStruct testStruct = TestStruct{"MyName", 45.0f};

      if (fileStream.write(&testStruct, sizeof(TestStruct)) != sizeof(TestStruct))
      {
        printf("Error! Could not write structure to stream.\n");
        return;
      }
    }
    // Write another instance of test struct using simpler approach.
    fileStream.write(TestStruct{"Another Name", 120.0f});

    if (fileStream)
      printf("Resulting test file size: %ld\n", fileStream.size());
    else
      printf("Error! Could not write one of the values to the stream.\n");

    // Note: the file is flushed and closed once "fileStream" goes out of scope here.
  }

  if (!FileStream::fileExists(directory + "TestFile.dat"))
    printf("This is strange! The file that was just written, does not exist anymore.\n");

  { // Read the file that was just written to.
    FileStream fileStream(directory + "TestFile.dat", FileStream::ModeRead | FileStream::ShareDenyWrite);
    if (!fileStream)
    {
      printf("Error! Could not open test file for reading.\n");
      return;
    }
    bool succeeded = true;

    // Read values that were written previously and check if they are correct.
    if (fileStream.read<uint8_t>() != 148) // Read 8-bit unsigned value.
      succeeded = false;

    if (fileStream.read<char>() != 'c') // Read character.
      succeeded = false;

    if (fileStream.read<int16_t>() != 15001) // Read 16-bit signed value.
      succeeded = false;

    { // Another way of reading values from stream.
      int32_t value;
      fileStream >> value; // Read 32-bit signed integer from file.

      if (value != 254012)
        succeeded = false;
    }

    { // Read some structure testing the resulting number of bytes that was written.
      TestStruct testStruct = {};

      if (fileStream.read(&testStruct, sizeof(TestStruct)) != sizeof(TestStruct))
      {
        printf("Error! Could not read structure from the stream.\n");
        return;
      }
      if (String::FromBuffer(testStruct.name, sizeof(TestStruct::name)) != "MyName" ||
        testStruct.value != 45.0f)
        succeeded = false;
    }
    { // Read another instance of test struct using simpler approach.
      TestStruct testStruct2 = fileStream.read<TestStruct>();

      if (String::FromBuffer(testStruct2.name, sizeof(TestStruct::name)) != "Another Name" ||
        testStruct2.value != 120.0f)
        succeeded = false;
    }
    if (!fileStream)
      printf("Error! Could not read one of the values to the stream.\n");

    if (succeeded)
      printf("Successfully verified values that were written previously to a test file.\n");
    else
      printf("Error! Values read from the stream do not match those that were written!\n");
  }
}

static String const textLoremIpsum =
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut "
  "labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
  "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit "
  "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt "
  "in culpa qui officia deserunt mollit anim id est laborum.";

static void exampleTextFiles()
{
  String const fileName = directoryForTesting + utility::PathDelimeter + "LoremIpsum.txt";

  // Note: strings written and read using "saveString" and "loadString" are treated as series of bytes.
  // Therefore, they are suitable for writing and reading binary files, where String serves as a simple
  // binary data buffer.

  // Write text to a file.
  if (!FileStream::saveString(fileName, textLoremIpsum))
    printf("Could not save text string to a file.\n");

  // Load text from a file.
  String text = FileStream::loadString(fileName);

  if (!text)
    printf("Error! Could not load text string from a file.\n");
  else if (text != textLoremIpsum)
    printf("Error! Text read from file (%s) does not match what was written.\n", text.data());
  else
    printf("Successfully read text from a file that was previously written.\n");
}

static void exampleMemoryStream()
{
  MemoryStream memoryStream;

  // Write some values to stream in memory.
  memoryStream.write<uint32_t>(0xA7B47241u);
  memoryStream.write<float>(32.5f);
  memoryStream.write<double>(102.1345);

  // Write some more values using a different approach.
  memoryStream << "Test" << 1.3847129384e-25f << static_cast<uint8_t>(0x25);

  // Access written data directly.
  printf("Memory stream contents:\n");

  uint8_t const* bytes = memoryStream.memory();

  for (Stream::Offset i = 0; i < memoryStream.size(); ++i)
  {
    if (i > 0)
      printf(" ");

    // Print hexadecimal value using "intToStr" function.
    printf("%s", ("0x" + utility::upperCase(utility::intToStr(bytes[i], 16))).data());
  }
  printf("\n");

  // Save contents of memory stream to disk.
  memoryStream.seek(0, SeekOrigin::Beginning);

  FileStream fileStream(directoryForTesting + utility::PathDelimeter + "memstream.bin",
    FileStream::ModeCreate | FileStream::ShareExclusive);

  fileStream.copy(memoryStream);
  fileStream.flush();

  if (fileStream)
    printf("Successfully saved the contents of memory stream to disk.\n");
  else
    printf("Error! Could not save contents of memory stream to disk.\n");
}

int main(int argc, char **argv)
{
  exampleFileStream();
  exampleTextFiles();
  exampleMemoryStream();
  return 0;
}