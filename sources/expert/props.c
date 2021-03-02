/**
 * Properties subsystem implemtation
 */

#include "g_local.h"

int p_stricmp(const void *elem1, const void *elem2) {
	// optimization: if the address in the same, 
	// must be the same string
	if (elem1 == elem2) {
		return true;
	} else {
		return Q_stricmp((char *)elem1, (char *)elem2);
	}
}

props_t *newProps()
{
	// allocate a new properties struct
	props_t *props = malloc(sizeof(props_t));

	props->keys = listNew(0, p_stricmp);
	props->values = listNew(0, p_stricmp);

	return props;
}

/**
 * Free a props struct
 */
void freeProps(props_t *props) {
	listFree(props->keys);
	listFree(props->values);
	free(props);
}

/**
 * Add property to an existing props struct
 */
void addProp(props_t *props, char *newKey, char *newValue)
{
	char *storedKey, *storedValue;
	int keyPos = listSearchPosition(props->keys, newKey, LIST_UNSORTED);

	if (keyPos != -1) {
		// property already exists.  free and delete old value (key stays the same)
		storedValue = (char *)listElementAt(props->values, keyPos);
		free(storedValue);
		listDeleteAt(props->values, keyPos);
		// and replace with new
		storedValue = malloc(strlen(newValue) + 1);
		listInsertAt(props->values, strcpy(storedValue, newValue), keyPos);
	} else {
		// append new key and value pair
		storedKey = malloc(strlen(newKey) + 1);
		storedValue = malloc(strlen(newValue) + 1);
		listAppend(props->keys, strcpy(storedKey, newKey));
		listAppend(props->values, strcpy(storedValue, newValue));
	}
}

/**
 * Remove a property from an existing props struct
 */
void removeProp(props_t *props, char *removeKey)
{
	char *storedString;
	int keyPos = listSearchPosition(props->keys, removeKey, LIST_UNSORTED);

	if (keyPos != -1) {
		storedString = (char *)listElementAt(props->keys, keyPos);
		free(storedString);
		storedString = (char *)listElementAt(props->values, keyPos);
		free(storedString);
		listDeleteAt(props->keys, keyPos);
		listDeleteAt(props->values, keyPos);
	}
}

/**
 * Get the value for a key in a props struct
 *
 * @return the value stored for the key, if found, otherwise NULL
 */
char *getProp(props_t *props, char *getKey)
{
	int keyPos = listSearchPosition(props->keys, getKey, LIST_UNSORTED);
	// wasn't found
	if (keyPos == -1) {
		return NULL;
	} else {
		return (char *)listElementAt(props->values, keyPos);
	}
}
	
/** 
 * Print all properties in a struct
 */
void printProps(props_t *props) {
	int i, pairs;

	pairs = listSize(props->keys);
	for (i = 0; i < pairs; i++) 
	{
		gi.cprintf(NULL, PRINT_HIGH, "%s		%s\n",
		           (char *)listElementAt(props->keys, i),
		           (char *)listElementAt(props->values, i));
	}
}

/**
 * Load a set of properties from a file
 * 
 * UNUSED and already suffering severe bitrot
 */
/*
ini_t *loadProperties(const char *pszFileName) {
	FILE *filSettings;
	char ch;

	char szLine[512];
	char* pEqual;
	char* pWhere2;
	short cLineLen;
	ini_t *iniDest;

	gi.dprintf("Loading properties from %s\n", pszFileName);

	// allocate a new struct
	iniDest = gi.TagMalloc(sizeof(ini_t), TAG_LEVEL);

	// initialize the settings struct
	iniDest->pszSettings = gi.TagMalloc(INI_INITIALSIZE, TAG_GAME);
	iniDest->cBytesAllocated = INI_INITIALSIZE;
	iniDest->cBytesUsed = iniDest->cLines = 0;
	iniDest->cLines = 0;

	filSettings = OpenGamedirFile(gamedir->string, pszFileName, "r");

	// return if file doesn't exist
	if (!filSettings) {
		return iniDest;
	}

	// read in settings line by line
	for (;;) {
		cLineLen = 0;
			
		// read the line into a temporary buffer
		while (cLineLen < 255 && (ch = fgetc(filSettings)) != EOF && ch != '\n') {
			szLine[cLineLen++] = ch;
		}

		if (ch == EOF) {
			break;
		}

		// empty line, continue
		if (cLineLen == 0) {
			continue;
		}

		// null terminate the temp buffer
		szLine[cLineLen] = 0;

		// if line starts with a comment symbol, continue
		pWhere2 = strchr(szLine, '#');
		if (pWhere2 != NULL && pWhere2 - szLine == 0) {
			continue;
		}

		// find the equal sign.  if not present, consider bad input
		pEqual = strchr(szLine, '=');
		if (pEqual == NULL || pEqual - szLine == 0) {
			gi.dprintf("Either no key or no value in %s on line %i", pszFileName, iniDest->cLines);
			continue;
		}

		// FIXME: trim spaces instead
		// error if there are any spaces
		pWhere2 = strchr(szLine, ' ');
		if (pWhere2 != NULL && pEqual > pWhere2) {
			gi.dprintf("Key with space in %s on line %i", pszFileName, pszFileName, iniDest->cLines);
			continue;
		}

	}

	fclose(filSettings);

	return iniDest;
}

*/

