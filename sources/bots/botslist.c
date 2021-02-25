/* list.c - functions to deal with linked lists
 *
 * The "next" pointer is assumed to be the first field of each record. If it's
 * not, these routines will eat your memory.
 *
 * 16 December 1997 - Andrew Wood <ivarch@ps.cus.umist.ac.uk>
 */

#include <stdlib.h>

#include "botslist.h"		/* not really needed, but Steve complained */


/* Add a record to the end of a linked list, given the address of the pointer
 * to the first record and the size of each record.
 *
 * Returns 0 on failure, or a pointer to the added record on success.
 */
void * ll_add (void ** baseptr, long size) {
  void * ptr;

  if (!baseptr) return (0);
  if (size < sizeof (void *)) return (0);

  ptr = *baseptr;		/* pointer to first record */

  while (ptr) {		/* find the last record in the list */
    baseptr = ptr;		/* move base pointer */
    ptr = *baseptr;		/* now ptr points to next record */
  }

  ptr = calloc (1, size);	/* allocate memory for new record */
  if (!ptr) return (0);

  *baseptr = ptr;	/* end record's NEXT points to new record */

  return (ptr);		/* return pointer to new record */
}


/* Insert a record at the start of a linked list, given the address of the
 * pointer to the first record and the size of each record (as above).
 *
 * Returns 0 on failure, or a pointer to the added record on success.
 */
void * ll_insert (void ** baseptr, long size) {
  void * ptr;
  void * new;

  if (!baseptr) return (0);
  if (size < sizeof (void *)) return (0);

  ptr = *baseptr;	/* get pointer to first record */

  new = calloc (1, size);	/* allocate memory for new record */
  if (!new) return (0);

  *baseptr = new;		/* base record is now the new record */
  *((void **)(new)) = ptr;	/* old base record is now the second record */

  return (new);		/* return pointer to new record */
}


/* Remove a record from a linked list, given the address of the pointer to
 * the first record and the address of the record to be deleted. The function
 * "destructor" is called with "record" as its parameter, and then "record"
 * is freed. If "destructor" is 0 then no function is called.
 */
void ll_delete (void ** baseptr, void * record, void (* destructor) (void *)) {
  if (!baseptr) return;
  if (!record) return;

  while ((baseptr) && ((*baseptr) != record))
    baseptr = *baseptr;			/* find "record" in the list */

  if (!baseptr) return;		/* failed to find record in list */

  *baseptr = *((void **)(record));	/* previous "next" := record "next" */

  if (destructor) destructor (record);	/* call destructor function */

  free (record);
}


/* Destroy a linked list, calling ll_delete() above for every record.
 */
void ll_destroy (void ** baseptr, void (* destructor) (void *)) {
  if (!baseptr) return;

  while (*baseptr)
    ll_delete (baseptr, *baseptr, destructor);
}


/* Return a pointer to the next element of the given linked list, or 0 if
 * there is none.
 */
void * ll_next (void * ptr) {
  if (!ptr) return (0);
  return ((void *)(*(void **)ptr));	/* yeuch :) */
}

/* EOF */
