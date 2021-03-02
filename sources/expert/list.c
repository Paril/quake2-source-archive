/**
 * Implementation of List ADT
 * 
 * NOTE: was based on Dynamic Array ADT,
 * don't trust the comments
 */

#include "list.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/**
 * struct list_tImplementation
 * Holds a pointer to the actual list, and some immediates
 * for easy indexing and reallocation.
 */
struct listImplementation {
	char **content;
	int curSize;
	int allocSize;	
	listCompareFn compare;
};

/**
 * Since the list ADT is specificly for storing a list of pointers,
 * the list ADT's comparator function can be simpler than the 
 * qsort/bsearch comparator.  Instead of receiving as arguments 
 * pointers to generic "elements" it can receive the elements 
 * themselves, which are known to be pointers.
 *
 * However to use the C library qsort and bsearch functions we need
 * to reintroduce the level of indirection of generic "elements".
 * We do this by creating a qsort/bsearch style comparator function 
 * that simply dereferences it's arguments and calls the list ADT's
 * comparator.  Unfortunately, the only way to "pass in" the list ADT's
 * comparator is to store it globally since bsearch doesn't let us
 * pass a third arbitrary argument down to the comparator.
 */
static listCompareFn globalCompare;

int listIndirectCompare(const void *elem1, const void *elem2) {
	void *elemDirect1, *elemDirect2;

	elemDirect1 = *((void **)elem1);
	elemDirect2 = *((void **)elem2);

	return globalCompare(elemDirect1, elemDirect2);
}

/** 
 * Allocate space, store initial values and return a pointer 
 * to a list_tImplementation struct
 */
list_t listNew(int allocNum, listCompareFn comparator)
{
	list_t list;
	
	// allocate the struct that holds all list_t info
	list = malloc(sizeof(struct listImplementation)); 
	assert(list != NULL);

	// set up constants in the struct
	list->curSize = 0;
	list->compare = comparator;
	if (allocNum<=0) {
		allocNum = DEFAULTALLOC;
	}

	// allocate an initial list
	list->allocSize = allocNum;
	list->content = malloc(sizeof(char *)*allocNum);
	assert(list->content != NULL);

	// return a pointer to the struct to the client
	return(list);
}

void listReplaceComparator(list_t list, listCompareFn newComparator) {
	list->compare = newComparator;
}

/**
 * Implementation of built-in comparators
 */
int listPointerCompare(const void *ptr1, const void *ptr2) {
	if (ptr1 < ptr2) return -1;
	if (ptr1 > ptr2) return 1;
	return 0;
}

int listStringCompare(const void *string1, const void *string2) {
	return strcmp((char *)string1, (char *)string2);
}

/** 
 * Free the memory allocated for the list itself 
 * and the list_t struct.  If client's elements 
 * themselves point to memory, that memory is still
 * the client's responsibility.
 */
void listFree(list_t list)
{
	free(list->content);
	free(list);
}

/** 
 * Return the number of elements in the list
 */
int listSize(list_t list)
{
	return(list->curSize);
}

/** 
 * Return the nth element in the list, numbered from 0
 */
void *listElementAt(list_t list, int n)
{
	assert(n>=0 && n < list->curSize);

	// a list stores pointers
	return( *(list->content + n) );
}

/**
 * Do a linear search for key and return its position in the list as an int
 */
int linearSearch(list_t list, void *key) {

	char **place;
	int i;

	// do a linear search
	place = list->content;
	for (i=0;i<list->curSize;i++) {
		if ( list->compare(key, *place) == 0 ) 
			return(i);
		place++;
	}
	return(-1);
}

/** 
 * Remove an element at a specific position.  Moves all memory in the
 * list past the insertion point.  Will realloc to a smaller size
 * if actual elements in the list have dropped to less than half 
 * of the allocated size.
 */
void listDeleteAt(list_t list, int n)
{
	char **killpos;
	
	assert(n>=0 && n < list->curSize);

	// check whether we have more than double the space we need
	// to store the current elements and realloc to half size if so
	if (list->allocSize > list->curSize*2 &&
	    list->allocSize > NO_REALLOC_FLOOR) { // never realloc if sufficiently small
		list->allocSize = (int)ceil(list->allocSize * 0.5);
		list->content = realloc(list->content, list->allocSize*sizeof(char *));
		assert(list->content != NULL);
	}

	// remove the element
	killpos = list->content + n; 
	memmove(killpos, killpos+1, sizeof(char *)*(list->curSize - n));
	list->curSize--;
}

/** 
 * Delete the element "deleteElem"
 */
void listDelete(list_t list, void *deleteElem, int isSorted) {
	int deletePos = listSearchPosition(list, deleteElem, isSorted);
	assert(deletePos >= 0 && deletePos < list->curSize);
	listDeleteAt(list, deletePos);
}

int listContains(list_t list, void *key, int isSorted) {
	return (listSearchPosition(list, key, isSorted) >= 0);
}

/**
 * Insert an element at a specific position.  Moves all memory in the
 * list past the insertion point.  Will realloc if necessary.
 */
void listInsertAt(list_t list, void *newElem, int n)
{	 
	char **insertPos;

	assert(n>=0 && n <= list->curSize);

	// realloc to double size if we need more room to store this new element
	if (list->allocSize<=list->curSize) {
		list->content = realloc(list->content,list->allocSize*2*sizeof(char *));
		assert(list->content != NULL);
		list->allocSize = list->allocSize*2;
	}

	// position to insert at
	insertPos = list->content + n;

	// slide the contents of the list up one slot,
	// if we aren't inserting the only element at the 0 position
	if (list->curSize > 0) {
		memmove(insertPos+1, insertPos, sizeof(void *)*(list->curSize - n));
	}
	// and insert the new element
	*insertPos = newElem;
	list->curSize++;
}

/**
 * Append an element at the end of the list.  Will realloc if necessary.
 */
void listAppend(list_t list, void *newElem)
{
	// realloc to double size if we need more room to store this new element
	if (!(list->allocSize>list->curSize)) {
		list->content = realloc(list->content,list->allocSize*2*sizeof(char *));
		assert(list->content != NULL);
		list->allocSize = list->allocSize*2;
	}
	// append the new element at the end of the list
	*(list->content + list->curSize) = newElem;
	list->curSize++;
}

/** 
 * Just call the c-library qsort for an nlogn sort
 */
void listSort(list_t list)
{
	assert(list->compare != NULL);

	globalCompare = list->compare;
	qsort((void *)list->content, list->curSize, sizeof(char *), listIndirectCompare);
}


int listSearchPosition(list_t list, void *key, int isSorted)
{
	char **ptr;

	if (!isSorted) {
		return linearSearch(list, key);
	} else {
		// call binary search
		globalCompare = list->compare;
		ptr = bsearch((void *)&key, (void *)list->content, list->curSize, sizeof(char *), listIndirectCompare);
		return ptr - list->content;
	}
}

/** 
 * If the list is unsorted, just do a linear search using the comparator
 * Otherwise call the c-library bsearch to do a binary search
 */
void *listSearch(list_t list, void *key, int isSorted)
{
	int i;

	if (!isSorted) {
		i = linearSearch(list, key);
		if (i == -1) {
			return NULL;
		} else {
			return *(list->content + i);
		}
	} else {
		// call binary search
		globalCompare = list->compare;
		return *((void **)bsearch((void *)&key, (void *)list->content, list->curSize, sizeof(char *), listIndirectCompare));
	}
}

/** 
 * Walks the list elements, calling the client function with
 * the void clientData pointer for each element
 */
void listIterate(list_t list, listIteratorFn fn, void *clientData)
{
	int i;
	char **next;

	next = list->content;
	for (i=0;i<list->curSize;i++) {
		fn(*next, clientData);
		next++;
	}
}

