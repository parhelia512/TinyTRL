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

// Example showing how to work with arrays of integers.
void exampleIntegerArray()
{
  // Initialize array with some values.
  Array<int32_t> values = {25, 100, 75};

  // Add two more values to the end of array.
  values.addp(80);
  values.addp(200);

  // Insert a value at specific position in the array.
  values.insertp(1, 5);

  if (!values)
    printf("One or more array operations failed, possibly due to being out of memory.\n");

  // Display contents of array before sort.
  printf("Array before sort: ");

  for (int32_t value : values) // use ranged for
    printf("%d ", value);

  printf("\n");

  // Sort the array.
  values.quickSort();

  // Print sorted array.
  printf("Array after sort: ");

  for (Containers::Length i = 0; i < values.length(); ++i)
    printf("%d ", values[i]); // use index-based access

  printf("\n");

  // Use binary search to find some value.
  Containers::Length const index = values.binarySearch(75);

  if (index != Containers::NotFound)
    printf("Value of 25 has position: %zd\n", index);
  else
    printf("Value of 25 was not found in array\n");

  // Erase 2nd and 3rd elements.
  values.erase(1, 2);

  // Print array contents after erasing 2 elements.
  printf("Array after erasing 2 elements: ");

  for (int32_t value : values)
    printf("%d ", value);

  printf("\n");
}

// Example showing how to work with arrays of strings.
void exampleStringArray()
{
  // Initialize array with some names.
  Array<String> names = {"Camila", "Aurora", "Penelope", "Lucy", "Naomi"};

  // Add another name to the list.
  names.addp("Leah");

  // Insert some name as a first element of the list.
  names.insertp(0, "Eva");

  // Check integrity of the list and strings it contains.
  if (!names)
    printf("One or more array operations failed, possibly due to being out of memory.\n");

  // Let's check each of the names to make sure they are alright.

  // Please note that since all of the names used above are relatively short, which means that all of them
  // would fall into "short string optimization". That is, each of the names will be stored directly inside
  // the string and no heap allocation would occur. Therefore, for such short names, the following code will
  // never print the error message, even when there is no sufficient memory (in that case, the earlier error
  // message will be shown).
  for (String const& name : names) // use ranged for
    if (!name)
    {
      printf("At least one of the names could not be allocated due to lack of memory.\n");
      break;
    }

  // Display the names before sort.
  printf("Names before sort: ");

  for (String const& name : names) // use ranged for
    printf("%s ", name.data());

  printf("\n");

  // Sort the array.
  names.quickSort();

  // Print sorted names.
  printf("Names after sort: ");

  for (Containers::Length i = 0; i < names.length(); ++i)
    printf("%s ", names[i].data()); // use index-based access

  printf("\n");

  // Use binary search to find some name.
  Containers::Length const index = names.binarySearch("Lucy");

  if (index != Containers::NotFound)
    printf("The name of 'Lucy' has position: %zd\n", index);
  else
    printf("The name of 'Lucy' was not found in the array\n");
}

// Another example showing how to work with arrays of strings.
void exampleStringArray2()
{
  // Initialize array with the names of some cities and towns in Greenland.
  Array<String> cities = {"Nuuk", "paamiut", "Sisimiut", "aasiaat", "Upernavik", "saattut"};

  // Display contents of array before sort.
  printf("Cities before sort: ");

  for (String const& city : cities) // use ranged for
    printf("%s ", city.data());

  printf("\n");

  // Sort the array using default case-sensitive comparer.
  cities.quickSort();

  // Print sorted cities.
  printf("Cities after case-sensitive sort: ");

  for (String const& city : cities) // use ranged for
    printf("%s ", city.data());

  printf("\n");

  // Sort the array using case-insensitive comparer.
  cities.quickSort<utility::TextComparer>();

  // Print sorted cities.
  printf("Cities after case-insensitive sort: ");

  for (String const& city : cities) // use ranged for
    printf("%s ", city.data());

  printf("\n");

  // Let's make all city names upper-case...
  for (String& city : cities)
    city = utility::upperCase(city);

  // Print modified city names.
  printf("Cities after making their names upper-case: ");

  for (String const& city : cities)
    printf("%s ", city.data());

  printf("\n");
}

int main(int argc, char **argv)
{
  exampleIntegerArray();
  exampleStringArray();
  exampleStringArray2();
  return 0;
}