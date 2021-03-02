#ifndef _REFLIST_H
#define _REFLIST_H

/* File: dlist.h
 * --------------
 * Defines the interface for the list ADT.  The list ADT stores a list 
 * of pointers, supports sorting and searching based on arbitrary 
 * comparator functions, and handles its own memory management.
 * 
 * Additions and deletions will cause reallocation at power-of-2 
 * boundaries.
 */
#define NO_REALLOC_FLOOR 8

/* Type: list_t
 * ----------------
 * list_t should never be dereferenced by the user of the ADT.  Only 
 * a call to listNew actually creates a usable list_t.
 */
typedef struct listImplementation *list_t;

/* listCompareFn
 * --------------
 * The type of comparator functions to be used with the list ADT, for 
 * searching for and sorting stored pointers.  The arguments to the
 * comparator are pointers stored in the list.  The comparator should 
 * indicate the ordering of the two entrys using the same convention 
 * as the strcmp library function:
 * If entry1 is "less than" entry2, return a negative number (typically -1).
 * If entry1 is "greater than"  entry2, return a positive number (typically 1).
 * If the two entrys are "equal", return 0.
 */
typedef int (*listCompareFn)(const void *entry1, const void *entry2);

int listPointerCompare(const void *ptr1, const void *ptr2);
int listStringCompare(const void *string1, const void *string2);

/* listIteratorFn
 * ----------
 * listIteratorFn defines the type of functions that can be used to 
 * iterate over the pointers in a list.  The iterator function will
 * be called once per pointer in the list, with the stored pointer 
 * as the first argument client data pointer passed in from the 
 * original caller.
 */
typedef void (*listIteratorFn)(const void *entry, void *clientData);

/* listNew
 * --------
 * Creates a new, empty list_t and returns it. The allocNum parameter 
 * specifies the initial ALLOCATED length of the list.  If the client 
 * passes a negative or 0 value, room for DEFAULTALLOC entrys will be 
 * allocated. 
 */
#define DEFAULTALLOC 10
list_t listNew(int allocNum, listCompareFn comparator);

/* listReplaceComparator
 * ---------------------
 * Replace the comparator function used by the list ADT.
 */
void listReplaceComparator(list_t list, listCompareFn comparator);

/* listFree
 * ----------
 * Frees up all the memory for the list.  The list_t will then by 
 * unusable.
 */
void listFree(list_t list);

/** 
 * Returns the number of pointers in the list.  Runs in constant time.
 */
int listSize(list_t list);

/* listElementAt
 * -------------
 * Returns the nth pointer in the list.  Numbering begins with 0.
 */ 
void *listElementAt(list_t list, int n);

/* listAppend
 * -----------
 * Adds a new entry to the end of the list.  
 */
void listAppend(list_t list, void *newElem);

/* listInsertAt
 * -------------
 * Adds a new entry into the list at the specified position n.  Will cause
 * a memmove of the list past the inserted entry.
 */
void listInsertAt(list_t list, void *newElem, int n);

/* listDeleteAt
 * -------------
 * Deletes the entry numbered n from the list.  Will cause a memmove of the 
 * list past the inserted entry.
 */
void listDeleteAt(list_t list, int n);

/* listSort
 * ---------
 * Sorts the specified list into ascending order according to the supplied
 * comparator.  Runs in nlogn time.
 */
void listSort(list_t list);

#define LIST_SORTED				1
#define LIST_UNSORTED			0

/* listDelete
 * ----------
 * Finds and deletes the first element that matches deleteElem according to 
 * the comparator function.  Will use binary search if isSorted is true, 
 * linear search otherwise.
 */
void listDelete(list_t list, void *deleteElem, int isSorted);
 
/* listSearch
 * -----------
 * Finds and returns the first pointer in the list that the comparator
 * function reports as equal to the "key" argument.  Will use binary search 
 * if isSorted is true, linear search otherwise. 
 */
void *listSearch(list_t list, void *key, int isSorted);

/* listSearchPosition
 * -----------
 * Like listSearch, but returns the position in the list of the matching
 * pointer.
 */
int listSearchPosition(list_t list, void *entry, int isSorted);

/* listContains
 * -----------
 * Like listSearch, but returns just whether a matching pointer was found.
 */
int listContains(list_t list, void *entry, int isSorted);

/* listIterate
 * -----------
 * Iterates through each pointer in the list and calls the function fn for that 
 * entry, with pointer in the list as the first argument, and the clientData
 * pointer passed to listIterate() as the second argument.
 */
void listIterate(list_t list, listIteratorFn fn, void *clientData);

#endif _REFLIST_H
