/*
 *  Adrienne Slaughter, Northeastern University.
 *  Spring 2019
 *  CS 5007
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  See <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "gtest/gtest.h"

extern "C" {
  #include "./FileParser.h"
  #include "htll/LinkedList.h"
  #include "MovieIndex.h"
  #include "Movie.h"
}

void DestroyLLMovie(void *payload) {
  DestroyMovie((Movie*)payload);
}

TEST(FileParser, ReadGoodFile) {
  // read a file
  LinkedList movie_list  = ReadFile(const_cast<char *>("data/test"));

  ASSERT_EQ(32u, NumElementsInLinkedList(movie_list));

  DestroyLinkedList(movie_list, &DestroyLLMovie); 
}


TEST(FileParser, ReadNonexistentFile) {
  // try to read a non-existent file
  LinkedList movie_list = ReadFile(const_cast<char *>("bogus/file"));
  ASSERT_TRUE(movie_list == NULL);
  
}


TEST(FileParser, ReadBinaryFile) {
  // try to read a file that contains non-ASCII text
  LinkedList movie_list;

  movie_list = ReadFile(const_cast<char *>("./movies_small/libhtll.a"));
  ASSERT_TRUE(movie_list == NULL); 
}

TEST(FileParser, BuildMovieIndex) {
  LinkedList movie_list  = ReadFile(const_cast<char *>("data/test"));

  ASSERT_EQ(32u, NumElementsInLinkedList(movie_list));
  
  Index index = BuildMovieIndex(movie_list, Year); 

  // Do a few spot checks.
  DestroyIndex(index);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
