/* iv_ini.h - header file for INI file reading library
 *
 *  ini_initialise    - initialise INI file reading library
 *  ini_list_sections - list sections of given INI file
 *  ini_list_keys     - list keys under given section of given INI file
 *  ini_free_array    - free an array of names
 *  ini_key_value     - return the value of the specified key
 *  ini_closedown     - shut down library
 *
 * v1.00 - 16 December 1997 - Andrew Wood <ivarch@ps.cus.umist.ac.uk>
 */

#ifndef _IV_INI_H
#define _IV_INI_H

#define INI_MAXLEN_LINE		1024	/* maximum INI file line length */

#define INI_WHITESPACE		" \011"	/* whitespace: spaces and TABs */

typedef enum {		/* error codes */
  INI_ENOERR,			/* no error */
  INI_ENOMEM,			/* memory allocation failed */
  INI_ENOENT,			/* file open failed */
  INI_EFILE,			/* error while reading file */
  INI_EFAULT,			/* null pointer */
  INI_EMAXERR			/* highest allowed error message */
} Ini_Err;


void   ini_initialise (void);
int    ini_list_sections (char ***, char *);
int    ini_list_keys (char ***, char *, char *);
void   ini_free_array (char **, int);
char * ini_key_value (char *, char *, char *, char *);
void   ini_closedown (void);

#endif	/* _IV_INI_H */

/* EOF */
