#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "htll/LinkedList.h"
#include "MovieIndex.h"
#include "Movie.h"
#include "MovieReport.h"
#include "./FileParser.h"



int main(int argc, char* argv[]){
  // Original Code: LinkedList movie_list  = ReadFile("data/test");

  // TODO(done): Parse command-line args to index based on the correct field
  Index index;
  if (argc == 2) {
	  LinkedList movie_list = ReadFile(argv[1]);
	  index = BuildMovieIndex(movie_list, Genre);
  } else if (argc == 3) {
	  if (strcmp("-t", argv[1]) == 0) {
		  LinkedList movie_list = ReadFile(argv[2]);  // Read after flag check
		  index = BuildMovieIndex(movie_list, Type);
	  } else if (strcmp("-g", argv[1]) == 0) {
		  LinkedList movie_list = ReadFile(argv[2]);
		  index = BuildMovieIndex(movie_list, Genre);
	  } else if (strcmp("-y", argv[1]) == 0) {
		  LinkedList movie_list = ReadFile(argv[2]);
		  index = BuildMovieIndex(movie_list, Year);
	  } else {
		  return -1; // Invalid flag
	  }
  } else {
	  return 2; // Invalid number of flags
  }

  PrintReport(index);

  DestroyIndex(index);

  return 0;
}
