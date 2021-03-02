///////////////////////////////////////////////////////////////////////
// Ini file utility
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
//#include <conio.h>
#include <malloc.h>
#include <memory.h>
#include "ini.h"

#define MAX_LENGTH 1024
#define MAX_SECTIONS 1024


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
	
	iniFile->section_pos = (long*)malloc(sizeof(long) * iniFile->number_of_sections);
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
		iniFile->section_names[i] = (char*)malloc(sizeof(char) * length);
		memset(iniFile->section_names[i], 0x00, length);
		p = iniFile->inifile[iniFile->section_pos[i]];
		p = p + iniFile->open_bracket_pos + 1;
		for (j=0; j<length-1; j++)
		{
			strncpy(iniFile->section_names[i], p, length-1);
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

			case '\r':
			case '\n':
			case ';':
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

	entry->entry_names[entry_number] = (char*)malloc(sizeof(char) * (length+1));
	memset(entry->entry_names[entry_number], 0x00, length+1);
	p = textline;
	p += entry->first_non_space;
	strncpy(entry->entry_names[entry_number], p, length);

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

		entry->entry_values[entry_number] = (char*)malloc(sizeof(char)*(length+1));
		memset(entry->entry_values[entry_number], 0x00, length+1);
		p = textline;
		p += entry->first_after_equals;
		strncpy(entry->entry_values[entry_number], p, length);
	}
	else
	{
		entry->entry_values[entry_number] = (char*)malloc(sizeof(char));
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

	
	iniFile->entries[pos]->entry_names = (char**)malloc(sizeof(char*) * iniFile->entries[pos]->number_of_entries);
	iniFile->entries[pos]->entry_values = (char**)malloc(sizeof(char*) * iniFile->entries[pos]->number_of_entries);

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
char Ini_ReadIniFile(char *filename, IniFile *iniFile)
{
	FILE *file;
	char line[MAX_LENGTH];
	int length, i;

	
	iniFile->number_of_lines = 0;
	iniFile->number_of_sections = 0;

	// try to open the file in read-only mode. return 0 if it fails.
	file = fopen(filename, "rb");
	if (file == NULL)
	{
		iniFile->ini_file_read = _FALSE_;
		return _FALSE_;
	}

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

	iniFile->inifile = (char**)malloc(sizeof(char*) * iniFile->number_of_lines);
		
	
	//
	// read in the open file into the inifile variable
	fseek(file, 0, SEEK_SET);
	for (i=0; i<iniFile->number_of_lines; i++)
	{
		memset(line, 0x00, MAX_LENGTH);
		fgets(line, MAX_LENGTH, file);
		length = strlen(line);
		++length;
		iniFile->inifile[i] = (char*)malloc(sizeof(char) * length);
		memset(iniFile->inifile[i], 0x00, length);
		strcpy(iniFile->inifile[i], line);
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
			free(iniFile->inifile[i]);
		}

		// delete the array of pointers
		free(iniFile->inifile);

		return _FALSE_;
	}

	iniFile->section_names = (char**)malloc(sizeof(char*) * iniFile->number_of_sections);

	GetSectionNames(iniFile);

	iniFile->entries = (IniEntry**)malloc(sizeof(IniEntry*) * iniFile->number_of_sections);


	// need a special case for the last section, as it can't be passed the
	// line position for the next section start position. In this special
	// case, it gets passed the total number of lines in the ini file.
	for (i=0; i<iniFile->number_of_sections-1; i++)
	{
		iniFile->entries[i] = (IniEntry*)malloc(sizeof(IniEntry));
		ReadIniEntry(iniFile, i, iniFile->section_pos[i], iniFile->section_pos[i+1]);
	}
	iniFile->entries[iniFile->number_of_sections-1] = (IniEntry*)malloc(sizeof(IniEntry));
	ReadIniEntry(iniFile, iniFile->number_of_sections-1, iniFile->section_pos[iniFile->number_of_sections - 1], iniFile->number_of_lines);
	  
	fclose(file);

	// to save some space, we will now get rid of the
	// image of the ini file - after we have read in
	// all the sections, it becomes pretty useless.
	//delete the various lines
	for (i=0; i<iniFile->number_of_lines; i++)
	{
		free(iniFile->inifile[i]);
	}
	// delete the array of pointers
	free(iniFile->inifile);
	// done freeing up the ini file image.

	
	return _TRUE_;
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
int FindSection(IniFile *iniFile, char *section)
{
	int i;

	for (i=0; i<iniFile->number_of_sections; i++)
	{
		if (!stricmp(section, iniFile->section_names[i]))
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
int FindEntry(IniEntry *e, char *entry)
{
	int i;

	for (i=0; i<e->number_of_entries; i++)
	{
		if (!stricmp(entry, e->entry_names[i]))
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
char* EntryValue(IniEntry *e, char *entry)
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
		free(e->entry_names[i]);
		free(e->entry_values[i]);
	}
	free(e->entry_names);
	free(e->entry_values);
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
char** Ini_SectionEntries(IniFile *iniFile, char *section)
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
int Ini_NumberOfEntries(IniFile *iniFile, char *section)
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
char* Ini_GetValue(IniFile *iniFile, char *section, char *entry)
{
	int s_number;

	s_number = FindSection(iniFile, section);

	if (s_number == -1)
		return NULL;

	return EntryValue(iniFile->entries[s_number], entry);
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
			free(iniFile->section_names[i]);
		}
		free(iniFile->section_names);
		free(iniFile->entries);
		free(iniFile->section_pos);
	}
}
