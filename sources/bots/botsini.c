/* ini.c - functions to read INI files
 *
 * v1.00 - 16 December 1997 - Andrew Wood <ivarch@ps.cus.umist.ac.uk>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "botsini.h"
#include "botslist.h"


struct Ini_Item_Data;		/* structure containing item information */
struct Ini_Item_Data {
  void * next;				/* pointer to next item */
  char * item_name;			/* item name */
  long item_offset;			/* offset of item line */
};


struct Ini_Data {		/* structure containing all current data */
  char * current_filename;		/* name of open file, or 0 */
  FILE * file_pointer;			/* file pointer, or 0 */
  char err;				/* flag, set if error occurred */
  struct Ini_Item_Data * section_base;	/* base of sections list */
  char line_buffer [INI_MAXLEN_LINE];	/* line input buffer */
};

struct Ini_Data ini_data;


/* Destroy Ini_Item_Data record by freeing its name buffer.
 */
void ini_item_destructor (void * record) {
  free (((struct Ini_Item_Data *)(record)) -> item_name);	/* gruesome */
}


/* Read a line from the currently open file to the line buffer, removing \n
 * and \r.
 */
void ini_read_line (void) {
  int length, c;

  if (!ini_data.file_pointer) return;

  ini_data.line_buffer[0] = 0;
  fgets (ini_data.line_buffer, INI_MAXLEN_LINE, ini_data.file_pointer);

  length = strlen (ini_data.line_buffer) - 1;
  if (length < 0) return;

  do {		/* strip \n, \r and ^Z (DOS) from the end of the line */
    c = ini_data.line_buffer[length];
    if ((c != 10) && (c != 13) && (c != 26)) return;
    ini_data.line_buffer[length--] = 0;
  } while (length >= 0);
}


/* Open an INI file, checking to see whether or not it's already open. If it
 * isn't, destroy any currently held data, open it, and scan for sections.
 */
void ini_open_file (char * file) {
  char * ptr;
  struct Ini_Item_Data * record;

  if (ini_data.current_filename) {
    if (strcmp (ini_data.current_filename, file) == 0) {
      if (ini_data.file_pointer) return;	/* just return if open */
    }
  }

  ini_closedown ();	/* remove all data being held */

  ini_data.current_filename = strdup (file);	/* store filename */
  if (!ini_data.current_filename) {
    ini_data.err = INI_ENOMEM;
    return;
  }

  ini_data.file_pointer = fopen (file, "r");	/* open file */
  if (!ini_data.file_pointer) {
    ini_data.err = INI_ENOENT;
    return;
  }

  while ((!feof(ini_data.file_pointer)) && (!ferror(ini_data.file_pointer))) {
    ini_read_line ();

    if (ini_data.line_buffer[0] != '[') continue;	/* [ of [section] */
    ptr = strchr (ini_data.line_buffer, ']');		/* ] of [section] */
    if (!ptr) continue;

    *ptr = 0;			/* found a section heading - wipe the ] */

    record = ll_add ((void **) &(ini_data.section_base), sizeof (*record));

    if (!record) {
      ini_data.err = INI_ENOMEM;
      return;
    }
    record -> item_name = strdup (1 + ini_data.line_buffer);
    record -> item_offset = ftell (ini_data.file_pointer);
  }

  if (ferror (ini_data.file_pointer)) ini_data.err = INI_EFILE;
}


/* Return a pointer to the item data for "section", or 0 if it's not in the
 * current list.
 */
struct Ini_Item_Data * ini_find_section (char * section) {
  struct Ini_Item_Data * ptr;

  ptr = ini_data.section_base;

  while (ptr) {
    if (ptr -> item_name) {
      if (strcmp (ptr -> item_name, section) == 0) return (ptr);
    }
    ptr = ll_next ((void *)(ptr));
  }

  return (0);
}


/* Make an array of item names, storing the address of the array in *arrayptr.
 * The array is generated from the linked list starting at "base", and the
 * number of records in the array is returned.
 *
 * On error, -errnum is returned, and the state of *arrayptr is undefined.
 */
int ini_make_array (char *** arrayptr, struct Ini_Item_Data ** base) {
  struct Ini_Item_Data * ptr;
  int num_items;

  if (!arrayptr) return (-INI_EFAULT);
  if (!base) return (-INI_EFAULT);

  num_items = 0;
  ptr = *base;

  while (ptr) {		/* count the number of items */
    num_items ++;
    ptr = ll_next ((void *)(ptr));
  }

  if (num_items < 1) return (0);	/* no sections */

  *arrayptr = (char **) calloc (num_items, sizeof (char *));
  if (!(*arrayptr)) return (-INI_ENOMEM);

  num_items = 0;
  ptr = *base;

  while (ptr) {		/* add the items to the array */
    (*arrayptr)[num_items] = strdup (ptr -> item_name);
    num_items ++;
    ptr = ll_next ((void *)(ptr));
  }

  return (num_items);
}


/*****************************************************************************/


/* Initialise functions by clearing out the list of files, handles and
 * sections.
 */
void ini_initialise (void) {
  ini_data.current_filename = 0;		/* no open file */
  ini_data.file_pointer = 0;
  ini_data.section_base = 0;			/* no sections */
  ini_data.err = 0;				/* no errors */
}


/* List the sections of INI "file", allocating space for each section found.
 * The address of an array of char * will be put into *arrayptr, and the number
 * of sections returned.
 *
 * On error, -errnum is returned, and *arrayptr is undefined.
 */
int ini_list_sections (char *** arrayptr, char * file) {

  if (!arrayptr) return (-INI_EFAULT);
  if (!file) return (-INI_EFAULT);

  ini_open_file (file);
  if (ini_data.err) return (-ini_data.err);

  return (ini_make_array (arrayptr, &(ini_data.section_base)));
}


/* List the keys of "section" in INI "file", allocating space for each key
 * found. The address of an array of char * will be put into *arrayptr, and
 * the number of keys returned.
 *
 * On error, -errnum is returned, and *arrayptr is undefined.
 */
int ini_list_keys(char *** arrayptr, char * file, char * section) {
  struct Ini_Item_Data * ptr;
  struct Ini_Item_Data * list_base;
  char passed_section;
  char * a;
  char * b;
  int n;

  if (!arrayptr) return (-INI_EFAULT);
  if (!file) return (-INI_EFAULT);
  if (!section) return (-INI_EFAULT);

  ini_open_file (file);
  if (ini_data.err) return (-ini_data.err);

  ptr = ini_find_section (section);
  if (!ptr) return (0);

  fseek (ini_data.file_pointer, ptr -> item_offset, SEEK_SET);

  passed_section = 0;
  list_base = 0;

  while ((!feof (ini_data.file_pointer)) &&
         (!ferror (ini_data.file_pointer)) &&
         (!passed_section)) {
    ini_read_line ();
    n = strspn (ini_data.line_buffer, INI_WHITESPACE);
    a = ini_data.line_buffer + n;	/* skip initial whitespace */

    switch (a[0]) {
      case '[' : passed_section = 1; break;	/* [ of [section] */
      case '#' :
      case 0   :				/* comment or blank line */
      case ';' : break;
      default :					/* key */
        b = strchr (a, '=');
        if (!b) continue;
        ptr = ll_add ((void **) &(list_base), sizeof (*ptr));
        if (!ptr) continue;
        while ((strchr (INI_WHITESPACE, *(b-1))) && (b > a)) b--;
        *b = 0;			/* stripped trailing whitespace before = */
        ptr -> item_name = strdup (a);
        break;
    }
  }

  if (ferror (ini_data.file_pointer)) {
    ll_destroy ((void **) &(list_base), ini_item_destructor);
    return (-INI_EFILE);
  }

  n = ini_make_array (arrayptr, &list_base);

  ll_destroy ((void **) &(list_base), ini_item_destructor);

  return (n);
}


/* Free "array" containing "num" names, by first freeing every name, then
 * freeing the array itself.
 */
void ini_free_array (char ** array, int num) {
  int i;

  for (i = 0; i < num; i++) {
    if (array[i]) free (array[i]);
  }

  free (array);
}


/* Return the value of "key" in "section" inside "file", or "duff" if no
 * value can be found.
 */
char * ini_key_value (char * filename, char * section, char * key, char * duff) {
	struct Ini_Item_Data * ptr;
	char * a;
	char * b;
	char * val;
	int n;
	char	newfilename[64] = "";
	int		i, islefn;

  if (!filename) return (duff);
  if (!section) return (duff);
  if (!key) return (duff);

#ifdef _WIN32
	strcpy(newfilename, "bots\\");
#else
	strcpy(newfilename, "bots/");
#endif
	islefn = strlen(newfilename);
	for (i=0; filename[i]; i++)
		newfilename[i + islefn] = tolower(filename[i]);
	newfilename[i + islefn] = '\0';

  ini_open_file (newfilename);
  if (ini_data.err) return (duff);

  ptr = ini_find_section (section);
  if (!ptr) return (duff);

  fseek (ini_data.file_pointer, ptr -> item_offset, SEEK_SET);

  while ((!feof (ini_data.file_pointer)) &&
         (!ferror (ini_data.file_pointer))) {
    ini_read_line ();
    n = strspn (ini_data.line_buffer, INI_WHITESPACE);
    a = ini_data.line_buffer + n;	/* skip initial whitespace */

    switch (a[0]) {
      case '[' : return (duff);		/* new section - [ encountered */
      case '#' :
      case 0   :				/* comment or blank line */
      case ';' : break;
      default :					/* key */
        b = strchr (a, '=');
        if (!b) continue;
        val = b + 1 + strspn (b+1, INI_WHITESPACE);
        while ((strchr (INI_WHITESPACE, *(b-1))) && (b > a)) b--;
        *b = 0;			/* stripped trailing whitespace before = */
        if (strcmp (a, key) == 0) return (val);
        break;
    }
  }

  return (duff);
}


/* Close down the library by closing all open files and freeing memory.
 */
void ini_closedown (void) {

  if (ini_data.current_filename) free (ini_data.current_filename);
  if (ini_data.file_pointer) fclose (ini_data.file_pointer);
  ll_destroy ((void **) &(ini_data.section_base), ini_item_destructor);

  ini_initialise ();
}

/* EOF */
