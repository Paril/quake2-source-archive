/* FILE DARRAY.C
 * Implementation of Dynamic Array ADT
 */
#include "darray.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/**
 * struct DArrayImplementation
 * Holds a pointer to the actual array, and some immediates
 * for easy indexing and reallocation.
 */
struct DArrayImplementation {
	void *content;
	int elemSize;
	int curSize;
	int allocSize;	
	ArrayCompareFn compare;
};

/** 
 * Allocate space, store initial values and return a pointer 
 * to a DArrayImplementation struct
 */
DArray ArrayNew(int elemSize, int allocNum, ArrayCompareFn comparator)
{
	DArray arr;
	
	assert(elemSize>0);

	// allocate the struct that holds all DArray info
	arr = malloc(sizeof(struct DArrayImplementation)); // size of
	assert(arr != NULL);

	// set up constants in the struct
	arr->elemSize = elemSize;
	arr->curSize = 0;
	arr->compare = comparator;
	if (allocNum<=0) {
		allocNum = DEFAULTALLOC;
	}

	// allocate an initial array
	arr->allocSize = allocNum;
	arr->content = malloc(elemSize*allocNum);
	assert(arr->content != NULL);

	// return a pointer to the struct to the client
	return(arr);
}

void ArrayReplaceComparator(DArray darray, ArrayCompareFn newComparator) {
	darray->compare = newComparator;
}

/** 
 * Free the memory allocated for the array itself 
 * and the DArray struct.  If client's elements 
 * themselves point to memory, that memory is still
 * the client's responsibility.
 */
void ArrayFree(DArray darray)
{
	free(darray->content);
	free(darray);
}

/** 
 * Return the number of elements in the array
 */
int ArraySize(const DArray darray)
{
	return(darray->curSize);
}

/** 
 * Return the nth element in the array, numbered from 0
 */
void *ArrayElementAt(DArray darray, int n)
{
	assert(n>=0 && n < darray->curSize);

	// don't know type of elements, so do byte-sized pointer
	// arithmetic using the client's value for the element size
	return( ((char *)darray->content) + (darray->elemSize*n) );
}

/** 
 * Remove an element at a specific position.  Moves all memory in the
 * array past the insertion point.  Will realloc to a smaller size
 * if actual elements in the array have dropped to less than half 
 * of the allocated size.
 */
void ArrayDeleteAt(DArray darray, int n)
{
	char *killpos;
	
	assert(n>=0 && n < darray->curSize);

	// check whether we have more than double the space we need
	// to store the current elements and realloc to half size if so
	if (darray->allocSize > darray->curSize*2 &&
	    darray->allocSize > NO_REALLOC_FLOOR) { // never realloc if sufficiently small
		darray->allocSize = (int)ceil(darray->allocSize * 0.5);
		darray->content = realloc(darray->content, darray->allocSize*darray->elemSize);
		assert(darray->content != NULL);
	}

	// remove the element
	killpos = ArrayElementAt(darray,n);
	memmove(killpos,killpos+darray->elemSize,darray->elemSize*darray->curSize - n);
	darray->curSize--;
}

/** 
 * Delete the element "deleteElem", using a linear search to find the element.
 */
void ArrayDelete(DArray darray, const void *deleteElem) {
	int deletePos = ArraySearchPosition(darray, deleteElem);
	assert(deletePos >= 0 && deletePos < darray->curSize);
	ArrayDeleteAt(darray, deletePos);
}

int ArrayContains(DArray darray, const void *key) {
	return (ArraySearchPosition(darray, key) >= 0);
}

/**
 * Insert an element at a specific position.  Moves all memory in the
 * array past the insertion point.  Will realloc if necessary.
 */
void ArrayInsertAt(DArray darray, const void *newElem, int n)
{	 
	char *newpos;

	assert(n>=0 && n <= darray->curSize);

	// realloc to double size if we need more room to store this new element
	if (darray->allocSize<=darray->curSize) {
		darray->content = realloc(darray->content,darray->allocSize*2*darray->elemSize);
		assert(darray->content != NULL);
		darray->allocSize = darray->allocSize*2;
	}

	// position to insert at
	newpos = (char *)darray->content + darray->elemSize*n;

	// slide the contents of the array up one slot,
	// if we aren't inserting the only element at the 0 position
	if (darray->curSize > 0) {
		memmove(newpos+darray->elemSize,newpos,darray->elemSize*darray->curSize - n);
	}
	// and insert the new element
	memcpy(newpos,newElem,darray->elemSize);
	darray->curSize++;
}

/**
 * Append an element at the end of the array.  Will realloc if necessary.
 */
void ArrayAppend(DArray darray, const void *newElem)
{
	// realloc to double size if we need more room to store this new element
	if (!(darray->allocSize>darray->curSize)) {
		darray->content = realloc(darray->content,darray->allocSize*2*darray->elemSize);
		darray->allocSize = darray->allocSize*2;
	}
	// append the new element at the end of the array
	memcpy( ((char *)darray->content) + (darray->elemSize*darray->curSize),
			 newElem,darray->elemSize);
	darray->curSize++;
}

/** 
 * Just call the c-library qsort for an nlogn sort
 */
void ArraySort(DArray darray)
{
	assert(darray->compare != NULL);

	qsort(darray->content, darray->curSize, darray->elemSize, darray->compare);
}

/**
 * Do a linear search for key and return its position in the array as an int
 */
int ArraySearchPosition(DArray darray, const void *key)
{
	char *place;
	int i;

	// do a linear search
	place = darray->content;
	for (i=0;i<darray->curSize;i++) {
		if ( darray->compare(key,place) == 0 ) 
			return(i);
		place += darray->elemSize;
	}
	return(-1);
}

/** 
 * If the array is unsorted, just do a linear search using the comparator
 * Otherwise call the c-library bsearch to do a binary search
 */
void *ArraySearch(DArray darray, const void *key, int isSorted)
{
	int i;

	if (!isSorted) {
		i = ArraySearchPosition(darray, key);
		if (i == -1) {
			return(NULL);
		} else {
			return( ((char *)darray->content) + darray->elemSize*i);
		}
	} else {
		// call binary search
		return(bsearch(key,darray->content,darray->curSize,darray->elemSize,darray->compare));
	}
}

/** 
 * Walks the array elements, calling the client function with
 * the void clientData pointer for each element
 */
void ArrayMap(DArray darray, ArrayMapFn fn, void *clientData)
{
	int i;
	char *next;

	next = darray->content;
	for (i=0;i<darray->curSize;i++) {
		fn(next, clientData);
		next += darray->elemSize;
	}
}

