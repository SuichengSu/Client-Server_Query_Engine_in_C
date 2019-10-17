
/*
 *  Created by Adrienne Slaughter
 *  CS 5007 Summer 2019
 *  Northeastern University, Seattle
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include "DirectoryParser.h"
#include "MovieTitleIndex.h"
#include "Movie.h"
#include "DocIdMap.h"


#define BUFFER_SIZE 1000

//=======================
// To minimize the number of files we have, I'm
// putting the private function prototypes for
// the DirectoryParser here.

/**
 * Helper function to index a single file. 
 *
 * \return a pointer to the number of records (lines) indexed from the file
 */
void* IndexAFile_MT(void *toBeIter);

pthread_mutex_t ITER_MUTEX = PTHREAD_MUTEX_INITIALIZER; // global variable
pthread_mutex_t INDEX_MUTEX = PTHREAD_MUTEX_INITIALIZER; // global variable

// THINK: Why is this global? 
MovieTitleIndex movieIndex;

int ParseTheFiles_MT(DocIdMap docs, MovieTitleIndex index, int num_threads) {
  // Create the iterator
  // Create the threads
  // Spawn the threads to work on the function IndexTheFile_MT
  
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  
  DocIdIter docIdIter = CreateDocIdIterator(docs);
  movieIndex = index;
  pthread_t threadArray[num_threads];

  while (HTIteratorHasMore(docIdIter) != 0) {
	for (int i = 0; i < num_threads; i++) {
		if (pthread_create(&threadArray[i], NULL, IndexAFile_MT, docIdIter) != 0) {
		printf("Fail to create No.%i thread", i);
		return -1;
		}
	}
	for (int j = 0; j < num_threads; j++) {
		if (pthread_join(threadArray[j], NULL) != 0) {
		printf("Fail to join No.%i thread", j);
                return -1;
		}

	}
  }
   pthread_mutex_destroy(&ITER_MUTEX);
   pthread_mutex_destroy(&INDEX_MUTEX);
   DestroyDocIdIterator(docIdIter);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("Took %f seconds to execute. \n", cpu_time_used);

  return 0;
}

void* IndexAFile_MT(void *docname_iter) {
  // Lock the iterator
  // Get the filename, unlock the iterator
  // Read the file
  // Create movie from row
  // Lock the index
  // Add movie to index
  // Unlock the index
  HTIter iter = (HTIter)docname_iter;
  // Don't forget to free this at some point!!
  int* num_records = (int*)malloc(sizeof(int)); 
  *num_records = 0; 
  
  HTKeyValue kv_MT;
  int flag = 0;
  if (pthread_mutex_lock(&ITER_MUTEX) != 0) {
  	// Report failiure to lock mutex object
  }
  if (HTIteratorHasMore(iter) != 0) {
	HTIteratorNext(iter);
	HTIteratorGet(iter, &kv_MT);
	flag++;
  }
  if (pthread_mutex_unlock(&ITER_MUTEX) != 0) {
         // printf("Fail to unlock the mutex object!");
   }
  if (flag != 0) {
	pthread_mutex_lock(&INDEX_MUTEX);
	IndexTheFile(kv_MT.value, kv_MT.key, movieIndex);
	pthread_mutex_unlock(&INDEX_MUTEX);
	*num_records++;
  }

  return (void*)num_records;
  
}
