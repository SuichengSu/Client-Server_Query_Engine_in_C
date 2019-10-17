#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "Hashtable.h"
#include "Hashtable_priv.h"
#include "LinkedList.h"
#include "Assert007.h"

// a free function that does nothing
static void NullFree(void *freeme) { }

static void FreeKVP(void *freeme) {
  free(freeme); 
}

Hashtable CreateHashtable(int num_buckets) {
  if (num_buckets == 0)
    return NULL;

  Hashtable ht = (Hashtable)malloc(sizeof(struct hashtableInfo));

  if (ht == NULL) {
    return NULL;
  }

  ht->num_buckets = num_buckets;
  ht->num_elements = 0;
  ht->buckets =
      (LinkedList*)malloc(num_buckets * sizeof(LinkedList));

  if (ht->buckets == NULL) {
    free(ht);
    return NULL;
  }

  for (int i=  0; i < num_buckets; i++) {
    ht->buckets[i] = CreateLinkedList();
    if (ht->buckets[i] == NULL) {
      // Need to free everything and then return NULL
      for (int j = 0; j < i; j++) {
        DestroyLinkedList(ht->buckets[j], NULL);
      }
      free(ht);
      return NULL;
    }
  }
  return ht;
}


void DestroyHashtable(Hashtable ht, ValueFreeFnPtr valueFreeFunction) {
  // Go through each bucket, freeing each bucket
  for (int i = 0; i < ht->num_buckets; i++) {
    LinkedList bucketlist = ht->buckets[i];
    HTKeyValuePtr nextKV;
  
    // Free the values in the list; then free the list
    if (NumElementsInLinkedList(bucketlist) > 0) {
      LLIter list_iter = CreateLLIter(bucketlist);

      LLIterGetPayload(list_iter, (void**)&nextKV); 
      valueFreeFunction(nextKV->value);
 
      // Now loop through the rest
      while (LLIterHasNext(list_iter) == 1) {
        LLIterNext(list_iter);
        LLIterGetPayload(list_iter, (void**)&nextKV); 
        valueFreeFunction(nextKV->value);
      }
      DestroyLLIter(list_iter); 
    }
    DestroyLinkedList(bucketlist, FreeKVP);
  }

  // free the bucket array within the table record,
  // then free the table record itself.
  free(ht->buckets);
  free(ht);
}

int isCollision(LinkedList bucket, uint64_t key, 
		HTKeyValuePtr keyValuePtr, int isDelete) {
	if (NumElementsInLinkedList(bucket) == 0) {
		return 0;
	}
	HTKeyValuePtr tempHTKeyValuePtr;
	LLIter bucketIter = CreateLLIter(bucket);
	while (1) {
		LLIterGetPayload(bucketIter, (void**) &tempHTKeyValuePtr);
		if (tempHTKeyValuePtr -> key == key) {
			*keyValuePtr = *tempHTKeyValuePtr;
			// If deletion is requested
			if (isDelete == 1) {
				LLIterDelete(bucketIter, &FreeKVP);
			}
			DestroyLLIter(bucketIter);
			return 1;
		}
		if (LLIterNext(bucketIter)) {
			DestroyLLIter(bucketIter);
			return 0;
		}
	}
}

int PutInHashtable(Hashtable ht,
                   HTKeyValue kvp,
                   HTKeyValue *old_key_value) {
  Assert007(ht != NULL);
  
  int insert_bucket;
  LinkedList insert_chain;

  ResizeHashtable(ht);

  // calculate which bucket we're inserting into,
  // get the list
  insert_bucket = HashKeyToBucketNum(ht, kvp.key);
  insert_chain = ht->buckets[insert_bucket];

  // STEP 1: Finish the implementation of the put.
  // This is a fairly complex task, so you might decide you want
  // to define/implement a helper function hat helps you find
  // and optionally remove a key within a chain, rather than putting
  // all that logic inside here. You might also find that your helper(s)
  // can be reused in step 2 and 3.
  
  // Check for failure, no memory case, say each bucket has max load of 5
  if (ht->num_elements > ht ->num_buckets * 5) {
	return 1;
  }

  // Reserve kvp value on heap
  HTKeyValue *kvpOnHeap = malloc(sizeof(HTKeyValue));
  *kvpOnHeap = kvp;

  // Duplicate key in the hashtable 
  if (isCollision(insert_chain, kvpOnHeap -> key, old_key_value, 1)) {
	InsertLinkedList(insert_chain, (void*) kvpOnHeap);
	return 2;
  }

  // Put is successful
  InsertLinkedList(insert_chain, (void*) kvpOnHeap);
  return 0;
}

int HashKeyToBucketNum(Hashtable ht, uint64_t key) {
  return key % ht->num_buckets;
}

// -1 if not found; 0 if success
int LookupInHashtable(Hashtable ht, uint64_t key, HTKeyValue *result) {
  Assert007(ht != NULL);
  
  // STEP 2: Implement lookup
  int index_bucket = HashKeyToBucketNum(ht, key);
  // If found, return 0;
  if (isCollision(ht->buckets[index_bucket], key, result, 0)) {
	  return 0;
  }
  // If not found, return -1;
  return -1;
}


int NumElemsInHashtable(Hashtable ht) {
  int res = 0; 
  for (int i=0; i<ht->num_buckets; i++) {
    res += NumElementsInLinkedList(ht->buckets[i]); 
  }
  return res; 
}


int RemoveFromHashtable(Hashtable ht, uint64_t key, HTKeyValuePtr junkKVP) {
  // STEP 3: Implement Remove
  int index_bucket = HashKeyToBucketNum(ht, key);
  // If found
  if (isCollision(ht->buckets[index_bucket], key, junkKVP, 1)) {
	return 0;
  }
  // If not found
  return -1;
}


uint64_t FNVHash64(unsigned char *buffer, unsigned int len) {
  // This code is adapted from code by Landon Curt Noll
  // and Bonelli Nicola:
  //
  // http://code.google.com/p/nicola-bonelli-repo/
  static const uint64_t FNV1_64_INIT = 0xcbf29ce484222325ULL;
  static const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
  unsigned char *bp = (unsigned char *) buffer;
  unsigned char *be = bp + len;
  uint64_t hval = FNV1_64_INIT;
  /*
   * FNV-1a hash each octet of the buffer
   */
  while (bp < be) {
    /* xor the bottom with the current octet */
    hval ^= (uint64_t) * bp++;
    /* multiply by the 64 bit FNV magic prime mod 2^64 */
    hval *= FNV_64_PRIME;
  }
  /* return our new hash value */
  return hval;
}


uint64_t FNVHashInt64(uint64_t makehash) {
  unsigned char buf[8];
  int i;
  for (i = 0; i < 8; i++) {
    buf[i] = (unsigned char) (makehash & 0x00000000000000FFULL);
    makehash >>= 8;
  }
  return FNVHash64(buf, 8);
}


void ResizeHashtable(Hashtable ht) {
  Assert007(ht != NULL);
  
  // Resize if the load factor is > 3.
  if (ht->num_elements < 3 * ht->num_buckets)
    return;
  
  // This is the resize case.  Allocate a new hashtable,
  // iterate over the old hashtable, do the surgery on
  // the old hashtable record and free up the new hashtable
  // record.
  Hashtable newht = CreateHashtable(ht->num_buckets * 9);
  // Give up if out of memory.
  if (newht == NULL)
    return;
  
  // Loop through the old ht with an iterator,
  // inserting into the new HT.
  HTIter it = CreateHashtableIterator(ht);
  if (it == NULL) {
    // Give up if out of memory.
    DestroyHashtable(newht, &NullFree);
    return;
  }

  HTKeyValue item;
  HTIteratorGet(it, &item);
  HTKeyValue old_kv; 
  
  if (PutInHashtable(newht, item, &old_kv) == 1) {
    // failure, free up everything, return.
    DestroyHashtableIterator(it);
    DestroyHashtable(newht, &NullFree);
    return;
  }
  
  while (HTIteratorHasMore(it) != 0) {
    HTIteratorNext(it);
    
    HTKeyValue item; 
    HTIteratorGet(it, &item);
    HTKeyValue old_kv; 
    
    if (PutInHashtable(newht, item, &old_kv) == 1) {
      // failure, free up everything, return.
      DestroyHashtableIterator(it);
      DestroyHashtable(newht, &NullFree);
      return;
    }
  }
  // Worked!  Free the iterator.
  DestroyHashtableIterator(it);
  // Sneaky: swap the structures, then free the new table,
  // and we're done.
  {
    struct hashtableInfo tmp;
    tmp = *ht;
    *ht = *newht;
    *newht = tmp;
    DestroyHashtable(newht, &NullFree);
  }
  return;
}


// ==========================
// Hashtable Iterator
// ==========================

// Returns NULL on failure, non-NULL on success.
HTIter CreateHashtableIterator(Hashtable table) {
  if (NumElemsInHashtable(table) == 0) {
    return NULL; 
  }
  HTIter iter = (HTIter)malloc(sizeof(HTIterRecord));
  if (iter == NULL) {
    return NULL;  // Couldn't malloc
  }
  iter->ht = table;
  iter->which_bucket = 0;
  while (NumElementsInLinkedList(iter->ht->buckets[iter->which_bucket]) == 0) {
    iter->which_bucket++; 
  }
  iter->bucket_iter = CreateLLIter(iter->ht->buckets[iter->which_bucket]);

  return iter;
}


void DestroyHashtableIterator(HTIter iter) {
  // Destroy the list iter
  free(iter->bucket_iter);
  iter->ht = NULL;
  free(iter);
}

// Moves to the next element; does not return the next element.
int HTIteratorNext(HTIter iter) {
  // Step 4: Implement HTIteratorNext
  if (iter == NULL || iter->bucket_iter == NULL) {
	return 1;
  }
  if (LLIterHasNext(iter->bucket_iter)) {
	LLIterNext(iter->bucket_iter);
	return 0;
  }
  iter->which_bucket++;
  int current_bucket_index = iter->which_bucket;
  while (current_bucket_index < iter->ht->num_buckets) {
	if (NumElementsInLinkedList(iter->ht->buckets[current_bucket_index]) > 0) {
			DestroyLLIter(iter->bucket_iter);
			iter->bucket_iter = CreateLLIter(iter->ht->buckets[current_bucket_index]);
			return 0;
	}
	current_bucket_index++;
  }
  return 2;
}

int HTIteratorGet(HTIter iter, HTKeyValuePtr dest) {
  Assert007(iter != NULL); 
  // Step 6 -- implement HTIteratorGet.
  if (iter->bucket_iter == NULL) {
	return 1;
  }
  HTKeyValuePtr tempHTKeyValuePtr;
  LLIterGetPayload(iter->bucket_iter, (void**) &tempHTKeyValuePtr);
  *dest = *tempHTKeyValuePtr;
  return 0;
}

//  0 if there are no more elements.
int HTIteratorHasMore(HTIter iter) {
  if (iter->bucket_iter == NULL) {
    return 0;
  }
  
  if (LLIterHasNext(iter->bucket_iter) == 1)
    return 1;
  
  // No more in this iter; are there more buckets?
  int i = iter->which_bucket + 1;
  while (i < (iter->ht->num_buckets)) {
    // Make sure one of them has elements in it
    if ((iter->ht->buckets[i] != NULL) &&
        (NumElementsInLinkedList(iter->ht->buckets[i]) > 0)) {
      return 1; 
    }
    i++;
  }
  
  return 0;  
}


// Step 5: Oops, I don't know how to count in base 10 properly. 
// Pass go, but don't collect $200. 

