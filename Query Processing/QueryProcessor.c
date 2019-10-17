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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "QueryProcessor.h"
#include "MovieTitleIndex.h"
#include "LinkedList.h"
#include "Hashtable.h"
#include "DocSet.h"

SearchResultIter CreateSearchResultIter(DocumentSet set) {
  SearchResultIter iter =
    (SearchResultIter)malloc(sizeof(struct searchResultIter));

    // STEP 7: Implement the initialization of the iter.
  if (iter == NULL) {
	printf("Out of memory!");
	return NULL;
  }

  iter->doc_iter = CreateHashtableIterator((Hashtable)set->doc_index);
  if (iter->doc_iter == NULL){
	printf("Out of memory or empty database is provided.");
	return NULL;
  }

  HTKeyValue kvp;
  HTIteratorGet(iter->doc_iter, &kvp);
  iter->cur_doc_id = kvp.key;
  iter->offset_iter = CreateLLIter((LinkedList)kvp.value);

  return iter;
}

void DestroySearchResultIter(SearchResultIter iter) {
  // Destroy LLIter
  if (iter->offset_iter != NULL) {
    DestroyLLIter(iter->offset_iter);
  }

  // Destroy doc_iter
  DestroyHashtableIterator(iter->doc_iter);

  free(iter);
}



SearchResultIter FindMovies(MovieTitleIndex index, char *term) {
  DocumentSet set = GetDocumentSet(index, term);
  if (set == NULL) {
    return NULL;
  }
  printf("Getting docs for movieset term: \"%s\"\n", set->desc);
  SearchResultIter iter = CreateSearchResultIter(set);
  return iter;
}


int SearchResultGet(SearchResultIter iter, SearchResult output) {
  // STEP 9: Implement SearchResultGet
  void *payload;
  
  int result = LLIterGetPayload(iter->offset_iter, &payload);
  if (result == 1) {
	printf("Fai to get payload!");
	return -1;
  }
  int rowId = *((int*)payload);
  output->row_id = rowId;
  output->doc_id = iter->cur_doc_id;
  return 0;
}

int SearchResultNext(SearchResultIter iter) {
  // STEP 8: Implement SearchResultNext
  if (LLIterHasNext(iter->offset_iter) != 0) {
	LLIterNext(iter->offset_iter);
  } else {
	  DestroyLLIter(iter->offset_iter);
	  if (HTIteratorHasMore(iter->doc_iter)) {
		HTIteratorNext(iter->doc_iter);

		HTKeyValue kvp;
		HTIteratorGet(iter->doc_iter, &kvp);
		iter->cur_doc_id = kvp.key;
		iter->offset_iter = CreateLLIter((LinkedList)kvp.value);
		if (iter->offset_iter == NULL) {
			printf("Out of memory!");
			return -1;
		}
	  } else {
		iter->doc_iter = NULL;
		iter->offset_iter = NULL;
		return -1;
	  }
  }
  return 0;
}

// Return 0 if no more
int SearchResultIterHasMore(SearchResultIter iter) {
  if (iter->doc_iter == NULL) {
    return 0;
  }
  if (LLIterHasNext(iter->offset_iter) == 0) {
    return (HTIteratorHasMore(iter->doc_iter));
  }

  return 1;
}
