#ifndef _DARRAY_H
#define _DARRAY_H

/* File: darray.h
 * --------------
 * Defines the interface for the DynamicArray ADT.
 * The DArray allows the client to store any number of elements of any desired
 * base type and is appropriate for a wide variety of storage problems. It 
 * supports efficient element access, and appending/inserting/deleting elements
 * as well as optional sorting and searching. In all cases, the DArray imposes 
 * no upper bound on the number of elements and deals with all its own memory 
 * management. The client specifies the size (in bytes) of the elements that 
 * will be stored in the array when it is created. Therefter the client and 
 * the DArray can refer to elements via (void*) ptrs. 
 *
 * Note that if you can use a DArray as a secondary index (that is, a list of
 * pointers to objects stored elsewhere) by just storing pointers in the DArray
 * rather than the structs themselves.
 */


/* Type: DArray
 * ----------------
 * Defines the DArray type itself, which is a _pointer_ type, like char *. Only
 * a call to ArrayNew actually creates a usable DArray.
 */
typedef struct DArrayImplementation *DArray;


/* ArrayCompareFn
 * --------------
 * ArrayCompareFn is a pointer to a client supplied function which the
 * DArray uses to sort or search the elements. The comparator takes two 
 * (const void*) pointers (these will point to elements) and returns an int.
 * The comparator should indicate the ordering of the two elements
 * using the same convention as the strcmp library function:
 * If elem1 is "less than" elem2, return a negative number (typically -1).
 * If elem1 is "greater than"  elem2, return a positive number (typically 1).
 * If the two elements are "equal", return 0.
 */
typedef int (*ArrayCompareFn)(const void *elem1, const void *elem2);


/* ArrayMapFn
 * ----------
 * ArrayMapFn defines the type of functions that can be used to map over
 * the elements in a DArray.  Each function is called with a pointer to
 * the element and a client data pointer passed in from the original
 * caller.
 */
typedef void (*ArrayMapFn)(void *elem, void *clientData);


/* ArrayNew
 * --------
 * Creates a new DArray and returns it. The actual number of elements in
 * the new array is 0.  The elemSize parameter specifies the number of 
 * bytes that a single element of this array should take up.  Must be 
 * greater than 0. For example, if you want to store elements of type 
 * Binky, you would pass sizeof(Binky) as this parameter.
 *
 * The allocNum parameter specifies the initial ALLOCATED length of the 
 * array.  If the client passes a negative or 0 value, room for 
 * DEFAULTALLOC elements will be allocated. 
 * If more space is required the array doubles it's current size so
 * as to respond appropriately to arrays that grow drastically.  
 */
#define DEFAULTALLOC 10
DArray ArrayNew(int elemSize, int allocNum, ArrayCompareFn comparator);


void ArrayReplaceComparator(DArray array, ArrayCompareFn comparator);


/* ArrayFree
 * ----------
 * Frees up all the memory for the array and its elements. It DOES NOT free 
 * memory owned by pointers embedded in the elements. This would require 
 * knowledge of the structure of the elements which the DArray does not have. 
 * After calling this, the value of what "array" is pointing to is undefined.
 */
void ArrayFree(DArray array);


/** 
 * Returns the number of elements in the array.  Runs in constant time.
 */
int ArraySize(const DArray array);


/* ArrayNth
 * --------
 * Returns a pointer to the element numbered n in the specified array.  
 * Numbering begins with 0.  It is an error if n is less than 0 or greater 
 * than the number of elements in the array minus 1.
 *
 * A pointer returned by ArrayNth becomes invalid after any calls which 
 * involve insertion, deletion or sorting the array, as all of 
 * these may rearrange the element storage.  Runs in constant time.
 */ 
void *ArrayElementAt(DArray array, int n);


/* ArrayAppend
 * -----------
 * Adds a new element to the end of the specified array.  
 */
void ArrayAppend(DArray array, const void *newElem);


/* ArrayInsertAt
 * -------------
 * Copies a new element into the array, placing it at the specified position n.
 * It is an error if n is less than 0 or greater than the logical length. The 
 * array elements after position n will be shifted over to make room. The new 
 * element's contents are copied from the memory pointed to by newElem. Runs 
 * in linear time, but will reallocate the array's space if out of room.
 */
void ArrayInsertAt(DArray array, const void *newElem, int n);


/* ArrayDeleteAt
 * -------------
 * Deletes the element numbered n from the array. It is an error if n is less 
 * than 0 or greater than the logical length minus one. All the elements
 * after position n will be shifted over to fill the gap. Runs in linear time.
 * If the number of elements the array can store is twice as large as the number
 * of elements currently in the array, and more than 8 elements can currently
 * be stored, the array will be reallocated at a smaller size.
 */
#define NO_REALLOC_FLOOR 8
void ArrayDeleteAt(DArray array, int n);

void ArrayDelete(DArray arrar, const void *deleteElem);
 
/* ArraySort
 * ---------
 * Sorts the specified array into ascending order according to the supplied
 * comparator.  The numbering of the elements will change to reflect the 
 * new ordering.  Should run in nlogn time.
 */
void ArraySort(DArray array);


/* ArraySearch
 * -----------
 * Searches the specified array for an element whose contents match
 * the element passed as the key.  Uses the comparator argument as test
 * for equality. The "isSorted" parameter allows the client to specify that 
 * the array is already in sorted order, and thus it should use a faster 
 * binary search.  If isSorted is false, a simple linear search is used.
 * If the key is found, a pointer to the matching element is returned.
 * Otherwise the function returns NULL.
 */
void *ArraySearch(DArray array, const void *key, int isSorted);


/* ArraySearchPosition
 * -----------
 * Does a linear search over the given array to find the element passed,
 * and returns that element's position in the array (numbered starting
 * with zero).  Returns -1 if not found.
 */
int ArraySearchPosition(DArray array, const void *elem);

int ArrayContains(DArray array, const void *elem);

/* ArrayMap
 * -----------
 * Iterates through each element in the array in order (from element 0 to
 * element n-1) and calls the function fn for that element.  The ArrayMapFn
 * function is called with the address of the array element and the clientData 
 * pointer passed to ArrayMap.
 * The clientData value allows the client to pass extra state information to 
 * the client-supplied function, if necessary.  If no client data is required, 
 * this argument should be NULL.
 */
void ArrayMap(DArray array, ArrayMapFn fn, void *clientData);

#endif _DARRAY_
