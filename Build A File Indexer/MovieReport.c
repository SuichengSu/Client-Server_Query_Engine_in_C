
#include <stdio.h>
#include <stdlib.h>

#include "MovieIndex.h"
#include "MovieReport.h"
#include "Movie.h"
#include "MovieSet.h"
#include "htll/LinkedList.h"
#include "htll/Hashtable.h"


void PrintReport(Index index) {
  // Create Iter
  HTIter iter = CreateHashtableIterator(index);

  HTKeyValue movie_set;

  HTIteratorGet(iter, &movie_set);
  OutputMovieSet((MovieSet)movie_set.value);

  while (HTIteratorHasMore(iter)) {
    HTIteratorNext(iter);
    HTIteratorGet(iter, &movie_set);
    OutputMovieSet((MovieSet)movie_set.value);
  }
  // For every movie set, create a LLIter

  DestroyHashtableIterator(iter);
}

void OutputMovieSet(MovieSet movie_set) {
  printf("%s: %s\n", "indexType", movie_set->desc);
  printf("%d items\n", NumElementsInLinkedList(movie_set->movies));
  LLIter iter = CreateLLIter(movie_set->movies);
  if (iter == NULL) {
    printf("iter null for some reason.. \n");
    return;
  }
  Movie *movie;

  LLIterGetPayload(iter, (void**)&movie);
  printf("\t%s\n", movie->title);

  while (LLIterHasNext(iter)) {
    LLIterNext(iter);
    LLIterGetPayload(iter, (void**)&movie);
    printf("\t%s\n", movie->title);
  }

  DestroyLLIter(iter);
}

void ListOfMovieHelper(HTIter iter) {
        HTKeyValue kvp;
        HTIteratorGet(iter, &kvp);
        MovieSet movieSet = (MovieSet) kvp.value;
        OutputMovieSet(movieSet);
}

void OutputReport(Index index, FILE* output) {
  // TODO(done): Implement this.
  // After you've implemented it, consider modifying PrintReport()
  // to utilize this function.
  HTIter iter = CreateHashtableIterator(index);
  if (iter == NULL) {
        DestroyHashtableIterator(iter);
  }
  ListOfMovieHelper(iter);
  while (HTIteratorHasMore(iter)) {
        HTIteratorNext(iter);
        ListOfMovieHelper(iter);
  }
  DestroyHashtableIterator(iter);
}

void SaveReport(Index index, char* filename) {
  // TODO(done): Implement this. You might utilize OutputReport.
  FILE* file;
  file = fopen(filename, "w+");
  OutputReport(index, file);
  fclose(file);
}

