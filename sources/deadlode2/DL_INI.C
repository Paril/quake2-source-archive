///////////////////////////////////////////////////////////////////////
// Ini file utility
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "g_local.h"

#define MAX_LENGTH 1024
#define MAX_SECTIONS 1024

void *TagReAlloc(void *in, size_t newsize, int tag) {
void *newptr;

	if (!newsize) {
		gi.TagFree(in);
		return NULL;
	}
	
	if ((newptr = gi.TagMalloc(newsize, tag)) == NULL)
		return NULL;

	if (in) {
		memmove(newptr, in, newsize);
		gi.TagFree(in);
	}
	return newptr;
}


////////////////////////////////////////////////
// FirstLetterIsOpenBracket(line)
////////////////////////////////////////////////
char FirstLetterIsOpenBracket(int line, IniFile *iniFile)
{
	int length = strlen(iniFile->inifile[line]);
	int i;

	iniFile->open_bracket_pos = 0;
	
	for (i=0; i<length; i++)
	{
		switch(iniFile->inifile[line][i])
		{
			case '[':
				iniFile->open_bracket_pos = i;
				return _TRUE_;
				break;
			case ' ':
				break;
			case '\t':	// TAB
				break;
			default:
				return _FALSE_;
		}
	}
	return _FALSE_;
}



///////////////////////////////////////////
//
// This searches for a ']' occuring somewhere in the line of characters.
// If it encounters a ';', then it returns immediately with a false
// value.
///////////////////////////////////////////
// ContainsCloseBracket(line)
///////////////////////////////////////////
char ContainsCloseBracket(int line, IniFile *iniFile)
{
	int length = strlen(iniFile->inifile[line]);
	int i;

	iniFile->close_bracket_pos = 0;

	for (i=iniFile->open_bracket_pos+1; i<length; i++)
	{
		switch(iniFile->inifile[line][i])
		{
			case ']':
				iniFile->close_bracket_pos = i;
				return _TRUE_;
				break;
			case ';':
			case '\r':
			case '\n':
				return _FALSE_;
				break;
			default:
				break;
		}
	}
	return _FALSE_;
}



/////////////////////////////////////
// CountNumberOfSections()
// counts the number of sections in the passed ini file
/////////////////////////////////////
void CountNumberOfSections(IniFile *iniFile)
{
	long int section_lines[MAX_SECTIONS];
	int i;
	
	for (i=0; i<iniFile->number_of_lines; i++)
	{
		if (FirstLetterIsOpenBracket(i, iniFile))
		{
			if(ContainsCloseBracket(i, iniFile))
			{
				section_lines[iniFile->number_of_sections] = i;
				iniFile->number_of_sections++;
			}
		}
	}
	
	iniFile->section_pos = (long*)gi.TagMalloc(sizeof(long) * iniFile->number_of_sections, TAG_LEVEL);
	memcpy(iniFile->section_pos, section_lines, iniFile->number_of_sections*sizeof(long));
}





////////////////////////////////
//
// This is used to get the names of the various sections. It does this
// by grabbing the text between the opening [ and the closing ] and
// copying it into the appropriate section_names entry.
// It uses FirstLetterIsOpen and ContainsCloseBracket simply because
// those two functions set open_bracket_pos and close_bracket_pos
// respectively, which this uses to determine which part of the line to
// copy.
// Section headers do not have the brackets copied.
///////////////////////////////
// GetSectionNames()
///////////////////////////////
void GetSectionNames(IniFile *iniFile)
{
	int length;
	int i, j;
	char *p;
	
	for (i=0; i<iniFile->number_of_sections; i++)
	{
		FirstLetterIsOpenBracket(iniFile->section_pos[i], iniFile);
		ContainsCloseBracket(iniFile->section_pos[i], iniFile);
		length = iniFile->close_bracket_pos - iniFile->open_bracket_pos;
		iniFile->section_names[i] = (char*)gi.TagMalloc(sizeof(char) * length, TAG_LEVEL);
		memset(iniFile->section_names[i], 0x00, length);
		p = iniFile->inifile[iniFile->section_pos[i]];
		p = p + iniFile->open_bracket_pos + 1;
		for (j=0; j<length-1; j++)
		{
			DL_strcpy(iniFile->section_names[i], p, length-1);
		}
	}
}



///////////////////////////////////////////
//
// This scans a line of text from the beginning looking for a non-whitespace
// character. If it finds a non-whitespace character, it returns _TRUE_ and
// sets first_non_space, unless that character was an = or a ;
//
// If it can't find a non-whitespace character, it returns _FALSE_.
///////////////////////////////////////////
// HasEntryName(char *textline)
///////////////////////////////////////////
char HasEntryName(IniEntry *entry, char *textline)
{
	int length = strlen(textline);
	int i;

	entry->first_non_space = -1;
	for (i=0; i<length; i++)
	{
		switch(textline[i])
		{
			case ' ':
			case '\t':	//TAB
				break;

			// exit on an '=', ';', '\r', '\n'
			case '=':
			case ';':
			case '\r':
			case '\n':
				return _FALSE_;

			default:
				entry->first_non_space = i;
				return _TRUE_;
		}
	}

	return _FALSE_;
}



/////////////////////////////////////////////////
//
// This function scans a line of text to see if an = sign appears
// in it. It must always be called after HasEntryName has been called,
// as it uses first_non_space.
//
// Upon success, it sets equals_position to the relevant position in
// the line of text, and returns _TRUE_.
/////////////////////////////////////////////////
// ContainsEqualsSign(char *textline)
/////////////////////////////////////////////////
char ContainsEqualsSign(IniEntry *entry, char *textline)
{
	int length = strlen(textline);
	int i, j;

	j = entry->first_non_space+1;

	entry->equals_position = -1;

	for (i=j; i<length; i++)
	{
		switch(textline[i])
		{
			case '=':
				entry->equals_position = i;
				return _TRUE_;

			case ';':
			case '\r':
			case '\n':
				return _FALSE_;

			default:
				break;
		}
	}

	return _FALSE_;
}



/////////////////////////////////////////////////////
//
// Grab the position of the last character of the entry name
// before the equals sign
/////////////////////////////////////////////////////
// LastLetterBeforeEquals(char *textline)
/////////////////////////////////////////////////////
void LastLetterBeforeEquals(IniEntry *entry, char *textline)
{
	int i;

	entry->last_letter = -1;
	// need to go to first_non_space-1 to catch single letter entry names
	for (i=entry->equals_position; i>entry->first_non_space-1; i--)
	{
		switch(textline[i])
		{
			case '=':
			case ' ':
			case '\t':
				break;

			default:
				entry->last_letter = i;
				return;
				break;
		}
	}
}

/////////////////////////////////////////////////////////////
//
// This determines an entry name. It adds 1 character onto the
// length of the entry name to put a NULL character at the end of
// the string, to stop rubbish being printed.
/////////////////////////////////////////////////////////////
// AddEntryName(char *textline, int entry_name)
/////////////////////////////////////////////////////////////
void AddEntryName(IniEntry *entry, char *textline, int entry_number)
{
	int length;
	char *p;

	LastLetterBeforeEquals(entry, textline);
	if (entry->last_letter == -1)
		return;

	length = entry->last_letter - entry->first_non_space + 1;

	entry->entry_names[entry_number] = (char*)gi.TagMalloc(sizeof(char) * (length+1), TAG_LEVEL);
	memset(entry->entry_names[entry_number], 0x00, length+1);
	p = textline;
	p += entry->first_non_space;
	DL_strcpy(entry->entry_names[entry_number], p, length);

}



/////////////////////////////////////////////////////
//
// This scans for the first non-whitespace letter after the
// equals sign. If it encounters a ';', it stops the scan.
//
/////////////////////////////////////////////////////
// FirstLetterAfterEquals(char *textline)
/////////////////////////////////////////////////////
char FirstLetterAfterEquals(IniEntry *entry, char *textline)
{
	int i;
	int length = strlen(textline);

	entry->first_after_equals = -1;
	for (i=entry->equals_position+1; i<length; i++)
	{
		switch(textline[i])
		{
			case ' ':
			case '\t':	// TAB
				break;

			// exit on ';', '\r', '\n'
			case ';':
			case '\r':
			case '\n':
				return _FALSE_;

			default:
				entry->first_after_equals = i;
				return _TRUE_;
		}
	}

	return _FALSE_;
}


////////////////////////////////////////////////////
//
// This finds the last letter of the value. It scans until
// it finds a \r, \n, or a ;.
//
////////////////////////////////////////////////////
// LastLetterAfterEquals(char *textline)
////////////////////////////////////////////////////
void LastLetterAfterEquals(IniEntry *entry, char *textline)
{
	int i;
	int length = strlen(textline);

	entry->last_after_equals = length;
	
	for (i=entry->first_after_equals+1; i<length; i++)
	{
		switch(textline[i])
		{
			case ';':
			case '\r':
			case '\n':
				entry->last_after_equals = i;
				return;
				break;

			default:
				break;
		}
	}
}



///////////////////////////////////////////////////
//
// This is called after LastLetterAfterEquals to remove
// and white-space at the end of the value. This is primarily
// used to get rid of comments, but also lets us determine the
// exact length of the value string.
//
///////////////////////////////////////////////////
// RemoveTrailingSpaces(char *textline)
///////////////////////////////////////////////////
void RemoveTrailingSpaces(IniEntry *entry, char *textline)
{
	int i, j;

	j = entry->last_after_equals-1;

	for (i=j; i>entry->first_after_equals-1; i--)
	{
		switch(textline[i])
		{
			case ' ':
			case '\t':
				break;

			default:
				entry->last_after_equals = i;
				return;
				break;
		}
	}
}
				


//////////////////////////////////////////////////////////////
//
// This is called to add a value to entry_values from a specific
// line of text. Only call this from the construction function.
// It requires that the location of the equals sign has been identified.
// The entry_number is the entry_value to enter
//////////////////////////////////////////////////////////////
// AddEntryValue(char *textline, int entry_number)
//////////////////////////////////////////////////////////////
void AddEntryValue(IniEntry *entry, char *textline, int entry_number)
{
	int length;
	char *p;

	if (FirstLetterAfterEquals(entry, textline))
	{
		LastLetterAfterEquals(entry, textline);
		RemoveTrailingSpaces(entry, textline);

		length = entry->last_after_equals - entry->first_after_equals + 1;

		entry->entry_values[entry_number] = (char*)gi.TagMalloc(sizeof(char)*(length+1), TAG_LEVEL);
		memset(entry->entry_values[entry_number], 0x00, length+1);
		p = textline;
		p += entry->first_after_equals;
		DL_strcpy(entry->entry_values[entry_number], p, length);
	}
	else
	{
		entry->entry_values[entry_number] = (char*)gi.TagMalloc(sizeof(char), TAG_LEVEL);
		memset(entry->entry_values[entry_number], 0x00, sizeof(char));
	}
}
	


///////////////////////////////////////////////////////////////////
// IniEntry initialise function
///////////////////////////////////////////////////////////////////
//
// Pass it the file that has been read from memory, and also the
// start line of the section.
//
// This code does NOT scan over the section header - this has been
// tested. Nor does it scan over the next section header.
//
// A valid entry must have at least 1 character before an = sign,
// and also a ; must not appear before either this first character, or
// the equals sign.
//
void ReadIniEntry(IniFile *iniFile, int pos, int first_line, int last_line)
{
	int i, number_of_lines;
	int j;

	iniFile->entries[pos]->number_of_entries = 0;
	iniFile->entries[pos]->first_non_space = -1;
	iniFile->entries[pos]->equals_position = -1;
	iniFile->entries[pos]->last_letter = -1;

	number_of_lines = last_line - first_line;

	// first we need to count the number of valid entries in this
	// section
	for (i=0; i<number_of_lines; i++)
	{
		if (HasEntryName(iniFile->entries[pos], iniFile->inifile[first_line+i]))
		{
			if (ContainsEqualsSign(iniFile->entries[pos], iniFile->inifile[first_line+i]))
			{
				iniFile->entries[pos]->number_of_entries++;
			}
		}
	}

	
	iniFile->entries[pos]->entry_names = (char**)gi.TagMalloc(sizeof(char*) * iniFile->entries[pos]->number_of_entries, TAG_LEVEL);
	iniFile->entries[pos]->entry_values = (char**)gi.TagMalloc(sizeof(char*) * iniFile->entries[pos]->number_of_entries, TAG_LEVEL);

	// j is used to keep track of where we are in entry_names and entry_values
	// can't use i because not all lines in number_of_lines will contain
	// an entry
	j = 0;
	for (i=0; i<number_of_lines; i++)
	{
		if (HasEntryName(iniFile->entries[pos], iniFile->inifile[first_line+i]))
		{
			if(ContainsEqualsSign(iniFile->entries[pos], iniFile->inifile[first_line+i]))
			{
				AddEntryName(iniFile->entries[pos], iniFile->inifile[first_line+i], j);
				AddEntryValue(iniFile->entries[pos], iniFile->inifile[first_line+i], j);
				++j;
			}
		}
	}
	
}



//////////////////////////////////////////////////////////////
//
// Reads in an ini file, from the specified filename, filling
// in the passed IniFile structure
//
// 
//////////////////////////////////////////////////////////////
IniFile *Ini_ReadIniFile(const char *filename)
{
	FILE *file;
	IniFile *iniFile;
	char line[MAX_LENGTH];
	int length, i;

	if ((iniFile = (IniFile *)gi.TagMalloc(sizeof(IniFile), TAG_LEVEL)) == NULL)
		return NULL;

	if ((iniFile->filename = (char *)gi.TagMalloc(strlen(filename)+1, TAG_LEVEL)) == NULL) {
		gi.TagFree(iniFile);
		return NULL;
	}
	
	iniFile->number_of_lines = 0;
	iniFile->number_of_sections = 0;
	DL_strcpy(iniFile->filename, filename, -1);

	// try to open the file in read-only mode
	file = fopen(filename, "rb");
	if (file == NULL)
		return iniFile;

	//
	// if we have opened the ini file, I am assuming that what comes below
	// will work (ie that the file actually is an ini file and not something
	// else
	iniFile->ini_file_read = _TRUE_;

	fseek(file, 0, SEEK_SET);

	//
	// count the number of lines in the open file
	// this does mean we read the file twice, but it doesn't add
	// too much overhead, and makes things that much more flexible
	while (!feof(file))
	{
		fgets(line, MAX_LENGTH, file);
		iniFile->number_of_lines++;
	}

	iniFile->inifile = (char**)gi.TagMalloc(sizeof(char*) * iniFile->number_of_lines, TAG_LEVEL);
	
	//
	// read in the open file into the inifile variable
	fseek(file, 0, SEEK_SET);
	for (i=0; i<iniFile->number_of_lines; i++)
	{
		memset(line, 0x00, MAX_LENGTH);
		fgets(line, MAX_LENGTH, file);
		length = strlen(line);
		++length;
		iniFile->inifile[i] = (char*)gi.TagMalloc(sizeof(char) * length, TAG_LEVEL);
		memset(iniFile->inifile[i], 0x00, length);
		DL_strcpy(iniFile->inifile[i], line, length);
	}

  
	CountNumberOfSections(iniFile);

	//
	// if we didn't find any sections, delete the file from memory and
	// set ini_file_read to _FALSE_ so that none of the public calls will
	// do anything
	//
	if (iniFile->number_of_sections == 0)
	{
		iniFile->ini_file_read = _FALSE_;
		//delete the various lines
		for (i=0; i<iniFile->number_of_lines; i++)
		{
			gi.TagFree(iniFile->inifile[i]);
		}

		// delete the array of pointers
		gi.TagFree(iniFile->inifile);

		return iniFile;
	}

	iniFile->section_names = (char**)gi.TagMalloc(sizeof(char*) * iniFile->number_of_sections, TAG_LEVEL);

	GetSectionNames(iniFile);

	iniFile->entries = (IniEntry**)gi.TagMalloc(sizeof(IniEntry*) * iniFile->number_of_sections, TAG_LEVEL);


	// need a special case for the last section, as it can't be passed the
	// line position for the next section start position. In this special
	// case, it gets passed the total number of lines in the ini file.
	for (i=0; i<iniFile->number_of_sections-1; i++)
	{
		iniFile->entries[i] = (IniEntry*)gi.TagMalloc(sizeof(IniEntry), TAG_LEVEL);
		ReadIniEntry(iniFile, i, iniFile->section_pos[i], iniFile->section_pos[i+1]);
	}
	iniFile->entries[iniFile->number_of_sections-1] = (IniEntry*)gi.TagMalloc(sizeof(IniEntry), TAG_LEVEL);
	ReadIniEntry(iniFile, iniFile->number_of_sections-1, iniFile->section_pos[iniFile->number_of_sections - 1], iniFile->number_of_lines);
	  
	fclose(file);

	// to save some space, we will now get rid of the
	// image of the ini file - after we have read in
	// all the sections, it becomes pretty useless.
	//delete the various lines
	for (i=0; i<iniFile->number_of_lines; i++)
	{
		gi.TagFree(iniFile->inifile[i]);
	}
	// delete the array of pointers
	gi.TagFree(iniFile->inifile);
	// done freeing up the ini file image.

	
	return iniFile;
}


///////////////////////////////////////
//
// This returns the number of the section entry that contains
// the section name specified. If the section name is not found,
// it returns -1.
//
///////////////////////////////////////
// FindSection(char *section)
///////////////////////////////////////
int FindSection(IniFile *iniFile, const char *section)
{
	int i;

	for (i=0; i<iniFile->number_of_sections; i++)
	{
		if (!DL_strcmp(section, iniFile->section_names[i], -1, false))
			return i;
	}

	return -1;
}


////////////////////////////////////
//
// This returns the number of the entry that contains
// the entry name specified. If the entry name is not found,
// it returns -1.
//
////////////////////////////////////
// FindEntry(char *entry)
////////////////////////////////////
int FindEntry(IniEntry *e, const char *entry)
{
	int i;

	for (i=0; i<e->number_of_entries; i++)
	{
		if (!DL_strcmp(entry, e->entry_names[i], -1, false))
			return i;
	}

	return -1;
}



///////////////////////////////////////
//
// It returns the string that makes up the value for the
// specified entry.
//
///////////////////////////////////////
// EntryValue(char *entry)
///////////////////////////////////////
char* EntryValue(IniEntry *e, const char *entry)
{
	int entry_number;

	entry_number = FindEntry(e, entry);

	// if there wasn't a valid entry name, return null.
	if (entry_number == -1)
		return NULL;

	// otherwise, there was a valid entry name, so return
	// the value
	return e->entry_values[entry_number];
}

int NumberOfEntries(IniEntry *e)
{
	return e->number_of_entries;
}


char** SectionNames(IniFile *iniFile)
{
	return iniFile->section_names;
}

// NumberOfSections();
int NumberOfSections(IniFile *iniFile)
{
	return iniFile->number_of_sections;
}

char** EntryNames(IniEntry *e)
{
	return e->entry_names;
}


void Ini_FreeIniEntry(IniEntry *e)
{
	int i;

	for (i=0; i<e->number_of_entries; i++) {
		gi.TagFree(e->entry_names[i]);
		gi.TagFree(e->entry_values[i]);
	}
	gi.TagFree(e->entry_names);
	gi.TagFree(e->entry_values);

	gi.TagFree(e);
}


/////////////////////////////////////////////////////////////////
/////   P U B L I C     F U N C T I O N S
////
/// These are the ones that you should be using, but only
/// after making a call to ReadIniFile.
/////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//
// Returns the list of entries from the specified section.
// Returns NULL if the section was invalid.
//////////////////////////////////////////////////////////
char** Ini_SectionEntries(IniFile *iniFile, const char *section)
{
	int s_number = FindSection(iniFile, section);
	if (s_number == -1)
		return NULL;
	return EntryNames(iniFile->entries[s_number]);
}



///////////////////////////////////////////////////////////
//
// Returns the number of entries in the specified section.
// Returns -1 if it isn't a valid section.
///////////////////////////////////////////////////////////
int Ini_NumberOfEntries(IniFile *iniFile, const char *section)
{
	int s_number = FindSection(iniFile, section);
	if (s_number == -1)
		return -1;
	return NumberOfEntries(iniFile->entries[s_number]);
}



////////////////////////////////////////////////////////////
//
// This is the function that you will use most of all.
// Call it something like:
// IniFile ini;
// ...
// char* value = Ini_GetValue(ini, "Goose_Eggs", "Number");
//
////////////////////////////////////////////////////////////
//
// Get a value from the specified section, using the specified
// key.
////////////////////////////////////////////////////////////
char* Ini_ReadString(IniFile *iniFile, const char *section, const char *entry, char *def)
{
	int s, e;
	char *value;

	s = FindSection(iniFile, section);
	if (s < 0) return def;
	e = FindEntry(iniFile->entries[s], entry);
	if (e < 0) return def;

	value = iniFile->entries[s]->entry_values[e];
	if (value == NULL)
		return def;

	return value;
}

double Ini_ReadDouble(IniFile *iniFile, const char *section, const char *entry, double def)
{
	int s, e;
	char *value;

	s = FindSection(iniFile, section);
	if (s < 0) return def;
	e = FindEntry(iniFile->entries[s], entry);
	if (e < 0) return def;

	value = iniFile->entries[s]->entry_values[e];
	if (value == NULL)
		return def;

	return strtod(value, NULL);
}

int Ini_ReadInteger(IniFile *iniFile, const char *section, const char *entry, int def)
{
	int s, e;
	char *value;

	s = FindSection(iniFile, section);
	if (s < 0) return def;
	e = FindEntry(iniFile->entries[s], entry);
	if (e < 0) return def;

	value = iniFile->entries[s]->entry_values[e];
	if (value == NULL)
		return def;

	return strtol(value, NULL, 0);
}

////////////////////////////////////////////
//
// This function MUST be called before the .dll is
// unloaded. If it is not, then the memory it
// allocated will NOT be freed, causing a memory
// leak.
// See the readme.txt file for more information.
////////////////////////////////////////////
void Ini_FreeIniFile(IniFile *iniFile)
{
	int i;

	// only want to do this is we actually allocated
	// stuff
	if (iniFile->ini_file_read != _FALSE_) {
			// free up all the ini sections and section names
		for (i=0; i<iniFile->number_of_sections; i++) {
			Ini_FreeIniEntry(iniFile->entries[i]);
			gi.TagFree(iniFile->section_names[i]);
		}
		gi.TagFree(iniFile->section_names);
		gi.TagFree(iniFile->entries);
		gi.TagFree(iniFile->section_pos);
	}
}


// New code to WRITE values into the INI file (and the file to disk)

int Ini_AddNewSection(IniFile *iniFile, const char *section) {
char **secNames = NULL;
IniEntry **secEntries = NULL, *newEntry = NULL;
char *secName = NULL;
int length;

	if (!iniFile || !section || !section[0])
		return -1;

	if (FindSection(iniFile, section) != -1)
		return 0;

	length = strlen(section);
	// Attempt to reallocate the section headers
	secNames = (char**)TagReAlloc(iniFile->section_names, sizeof(char*) * (iniFile->number_of_sections+1), TAG_LEVEL);
	secEntries = (IniEntry**)TagReAlloc(iniFile->entries, sizeof(IniEntry*) * (iniFile->number_of_sections+1), TAG_LEVEL);
	secName = (char*)gi.TagMalloc((sizeof(char) * length) + 1, TAG_LEVEL);
	newEntry = (IniEntry*)gi.TagMalloc(sizeof(IniEntry), TAG_LEVEL);

	newEntry->first_non_space = 0;
	newEntry->equals_position = 0;
	newEntry->last_letter = 0;
	newEntry->first_after_equals = 0;
	newEntry->last_after_equals = 0;
	newEntry->number_of_entries = 0;
	newEntry->entry_names = NULL;
	newEntry->entry_values = NULL;

	if (!secNames || !secEntries || !secName || !newEntry) {
		// Don't free secNames or secEntries because they were realloced
		if (secName)
			gi.TagFree(secName);
		if (newEntry)
			gi.TagFree(newEntry);
		return -1;
	}

	iniFile->number_of_sections++;
	iniFile->section_names = secNames;
	iniFile->entries = secEntries;

	iniFile->entries[iniFile->number_of_sections-1] = newEntry;
	iniFile->section_names[iniFile->number_of_sections-1] = secName;


	DL_strcpy(iniFile->section_names[iniFile->number_of_sections-1], section, length);

	return iniFile->number_of_sections-1;
}

int Ini_AddNewEntry(IniFile *iniFile, const char *section, const char *name, char *value) {
char entry[MAX_LENGTH];
int i, secEntry;

	if (!iniFile || !section || !section[0] || !name || !name[0] || !value || !value[0])
		return -1;

	secEntry = FindSection(iniFile, section);
	if (secEntry == -1)
		if ((secEntry = Ini_AddNewSection(iniFile, section)) == -1)
			return -1;

	sprintf(entry, "%s = %s", name, value);

	// search for an existing entry name
	if ((i = FindEntry(iniFile->entries[secEntry], name)) == -1) {
		i = iniFile->entries[secEntry]->number_of_entries++;
		iniFile->entries[secEntry]->entry_names = (char**)TagReAlloc(iniFile->entries[secEntry]->entry_names, sizeof(char*) * (i+1), TAG_LEVEL);
		iniFile->entries[secEntry]->entry_values = (char**)TagReAlloc(iniFile->entries[secEntry]->entry_values, sizeof(char*) * (i+1), TAG_LEVEL);

		if (HasEntryName(iniFile->entries[secEntry], entry)) {
			if (ContainsEqualsSign(iniFile->entries[secEntry], entry)) {
				AddEntryName(iniFile->entries[secEntry], entry, i);
				AddEntryValue(iniFile->entries[secEntry], entry, i);
			}
		}
	} else {
		gi.TagFree(iniFile->entries[secEntry]->entry_values[i]);
		iniFile->entries[secEntry]->entry_values[i] = NULL;  // just in case it can't add the new one
		if (HasEntryName(iniFile->entries[secEntry], entry))
			if (ContainsEqualsSign(iniFile->entries[secEntry], entry))
				AddEntryValue(iniFile->entries[secEntry], entry, i);
	}

	return i;
}

void Ini_WriteString(IniFile *iniFile, const char *section, const char *entry, char *val) {
	Ini_AddNewEntry(iniFile, section, entry, val);
}

void Ini_WriteDouble(IniFile *iniFile, const char *section, const char *entry, double val) {
char buff[MAX_LENGTH];

	sprintf(buff, "%f", val);
	Ini_AddNewEntry(iniFile, section, entry, buff);
}

void Ini_WriteInteger(IniFile *iniFile, const char *section, const char *entry, int val) {
char buff[MAX_LENGTH];

	sprintf(buff, "%d", val);
	Ini_AddNewEntry(iniFile, section, entry, buff);
}

int Ini_RemoveEntry(IniFile *iniFile, const char *section, const char *entry) {
int	sec, ent, i;

	if (!iniFile || !section || !section[0] || !entry || !entry[0])
		return -1;

	if ((sec = FindSection(iniFile, section)) == -1)
		return -1;

	if ((ent = FindEntry(iniFile->entries[sec], entry)) == -1)
		return -1;

	// Free old entry
	gi.TagFree(iniFile->entries[sec]->entry_names[ent]);
	gi.TagFree(iniFile->entries[sec]->entry_values[ent]);

	// Mark as freed (just because)
	iniFile->entries[sec]->entry_names[ent] = NULL;
	iniFile->entries[sec]->entry_values[ent] = NULL;

	// Move all entries following up (move the removed entry to the bottom)
	for (i = ent; i < iniFile->entries[sec]->number_of_entries-1; i++) {
		iniFile->entries[sec]->entry_names[i] = iniFile->entries[sec]->entry_names[i+1];
		iniFile->entries[sec]->entry_values[i] = iniFile->entries[sec]->entry_values[i+1];
	}

	i = --iniFile->entries[sec]->number_of_entries;
	iniFile->entries[sec]->entry_names = (char**)TagReAlloc(iniFile->entries[sec]->entry_names, sizeof(char*) * i, TAG_LEVEL);
	iniFile->entries[sec]->entry_values = (char**)TagReAlloc(iniFile->entries[sec]->entry_values, sizeof(char*) * i, TAG_LEVEL);

	return 0;
}

int Ini_RemoveSection(IniFile *iniFile, const char *section) {
int	sec, i;
IniEntry *entry;

	if (!iniFile || !section || !section[0])
		return -1;

	if ((sec = FindSection(iniFile, section)) == -1)
		return -1;

	entry = iniFile->entries[sec];

	while (entry->number_of_entries)
		Ini_RemoveEntry(iniFile, section, entry->entry_names[entry->number_of_entries-1]);

	// Free old section
	gi.TagFree(iniFile->entries[sec]);
	gi.TagFree(iniFile->section_names[sec]);

	// Mark as freed (just because)
	iniFile->entries[sec] = NULL;
	iniFile->section_names[sec] = NULL;

	// Move all entries following up (move the removed entry to the bottom)
	for (i = sec; i < iniFile->number_of_sections-1; i++) {
		iniFile->entries[i] = iniFile->entries[i+1];
		iniFile->section_names[i] = iniFile->section_names[i+1];
	}


	i = --iniFile->number_of_sections;
	iniFile->entries = (IniEntry**)TagReAlloc(iniFile->entries, sizeof(IniEntry*) * i, TAG_LEVEL);
	iniFile->section_names = (char**)TagReAlloc(iniFile->section_names, sizeof(char*) * i, TAG_LEVEL);

	return 0;
}

int Ini_WriteIniFile(IniFile *iniFile) {
FILE *FHnd;
int i, j;

	if (!iniFile)
		return -1;

	if ((FHnd = fopen(iniFile->filename, "wt")) == NULL)
		return -1;

	for (i=0; i<iniFile->number_of_sections; i++) {
		fprintf(FHnd, "[%s]\n", iniFile->section_names[i]);
		for (j=0; j<iniFile->entries[i]->number_of_entries; j++)
			fprintf(FHnd, "%s = %s\n", iniFile->entries[i]->entry_names[j], iniFile->entries[i]->entry_values[j]);
		fprintf(FHnd, "\n");	// Blank line between Sections (keeps things looking nice)
	}
	fclose(FHnd);

	return 0;
}
