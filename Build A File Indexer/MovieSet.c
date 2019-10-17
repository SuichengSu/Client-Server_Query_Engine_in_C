#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MovieSet.h"
#include "Movie.h"
#include "htll/LinkedList.h"


void DestroyMovieWrapper(void *a_movie) {
  DestroyMovie((Movie*)a_movie);
}

// Returns 0 for successful insert
// Returns 1 if element already exists
int AddMovieToSet(MovieSet set, Movie *movie) {
  if (NumElementsInLinkedList(set->movies) > 0) {
    LLIter iter = CreateLLIter(set->movies);

    if (iter == NULL) {
      printf("Memory issues\n");
      return -1;
    }
    Movie *from_set_movie;

    LLIterGetPayload(iter, (void**)&from_set_movie);
    if (strcmp(from_set_movie->id, movie->id) == 0) {
      DestroyLLIter(iter);
      return 1;
    }
    while (LLIterHasNext(iter)) {
      LLIterNext(iter);
      LLIterGetPayload(iter, (void**)&from_set_movie);
      if (strcmp(from_set_movie->id, movie->id) == 0) {
        DestroyLLIter(iter);
        return 1;
      }
    }  // end of while
    DestroyLLIter(iter);
  }
  int result = InsertLinkedList(set->movies, movie);
  return result;
}

MovieSet CreateMovieSet(char *desc) {
  MovieSet set = (MovieSet)malloc(sizeof(struct movieSet));
  if (set == NULL) {
    // Out of memory
    printf("Couldn't malloc for movieSet %s\n", desc);
    return NULL;
  }
  set->desc = (char*)malloc(strlen(desc) *  sizeof(char) + 1);
  if (set->desc == NULL) {
    printf("Couldn't malloc for movieSet->desc");
    return NULL;
  }
  snprintf(set->desc, strlen(desc) + 1, "%s", desc);
  set->movies = CreateLinkedList();
  return set;
}

void DestroyMovieSet(MovieSet set) {
  // Free desc
  free(set->desc);
  // Free the list and the movies
  DestroyLinkedList(set->movies, &DestroyMovieWrapper);
  // Free set
  free(set);
}
