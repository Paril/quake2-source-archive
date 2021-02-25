/* iv_list.h - header file for linked list functions
 *
 *  ll_add     - add a record to the end of a linked list
 *  ll_insert  - add a record to the start of a linked list
 *  ll_delete  - remove a record from a linked list
 *  ll_destroy - remove every record from a linked list
 *  ll_next    - return pointer to next record
 *
 * 16 December 1997 - Andrew Wood <ivarch@ps.cus.umist.ac.uk>
 */

#ifndef _IV_LIST_H
#define _IV_LIST_H

void * ll_add (void **, long);
void * ll_insert (void **, long);
void   ll_delete (void **, void *, void (* destructor) (void *));
void   ll_destroy (void **, void (* destructor) (void *));
void * ll_next (void *);

#endif	/* _IV_LIST_H */

/* EOF */
