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

// Example showing how to work with flat map containing user names and passwords.
static void exampleFlatMapPasswords()
{
  // Designate a meaningful name for the pair of strings denoting user and password.
  using UserPassword = Containers::Pair<String, String>;

  // The list of passwords uses case-insensitive user names.
  FlatMap<String, String, utility::TextComparer> passwords = {
    UserPassword{"Dan", "user2000"},
    UserPassword{"Leo1", "leo2024"}};

  // Add two more users.
  passwords.addp("henry", "rockplayer54");
  passwords.addp("James_Smith_92", "james 92");

  // This code checks if the user exists and if it doesn't, it adds a new user with default password.
  if (!passwords.exists("finn5"))
    passwords.addp("finn5", "finn5");

  { // This code does the same as above, but re-using the location used in the search to insert the element,
    // thereby avoiding doing the search twice.
    Containers::Location location;

    if (!passwords.find(location, "Jude10"))
    {
      if (!passwords.insert(location, "Jude10", "jude_pass"))
        passwords.pollute(); // Insertion failed due to insufficient memory, mark the list as "polluted".
    }
  }

  if (!passwords)
    printf("One or more operations failed, possibly due to being out of memory.\n");

  // Retrieve user's password.
  if (String const* const password = passwords.value("henry"))
    printf("Henry's password is: %s\n", password->data());
  else
    printf("User 'henry' does not exist.\n");

  // Display contents of our passwords.
  printf("Users and their passwords: \n");

  for (UserPassword const& userPassword : passwords)
    printf("    user: %s, password: %s\n", userPassword.key.data(), userPassword.value.data());

  // Display contents of our passwords in reverse order.
  printf("Users and their passwords in reverse-order: \n");

  // FlatMap uses array as storage, so we can iterate it by indices, if we really want to...
  for (Containers::Length i = passwords.length(); i--; )
  {
    Containers::Location const location(i);
    printf("    user: %s, password: %s\n", passwords[location].key.data(), passwords[location].value.data());
  }
}

// Example showing how to work with flat set containing numbers.
static void exampleFlatSetOfNumbers()
{
  // Start with a small set of numbers.
  FlatSet<int32_t> numbers = {15, 25, 35};

  // Add few more numbers.
  numbers.addp(40);
  numbers.addp(20);
  numbers.addp(60);

  // Add some numbers, but only if they are not in the list.
  if (!numbers.update(25) || !numbers.update(28) || !numbers.update(32))
    numbers.pollute(); // Insertion failed due to insufficient memory, mark the list as "polluted".

  // Check if some number exists in the set.
  if (numbers.exists(20))
    printf("Number 20 exists in the set!\n");

  if (!numbers)
    printf("One or more set operations failed, possibly due to being out of memory.\n");

  // Display numbers in the set.
  printf("Numbers in the set: ");

  for (int32_t number : numbers)
    printf("%d ", number);

  printf("\n");
  printf("Numbers in the set, in reverse order: ");

  // Similarly to FlatMap, we can iterate through FlatSet by indices, if we really have to...
  for (Containers::Length i = numbers.length(); i--; )
  {
    Containers::Location const location(i);
    printf("%d ", numbers[location]);
  }
  printf("\n");
}

int main(int argc, char **argv)
{
  exampleFlatMapPasswords();
  exampleFlatSetOfNumbers();
  return 0;
}