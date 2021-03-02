#ifndef _INIFILE__H
#define _INIFILE__H
#endif



#define _TRUE_ 1
#define _FALSE_ 0

///////////////////////////////////////////////
// Structure for an Ini file entry. Do NOT use these
// on their own.
///////////////////////////////////////////////
typedef struct {
	int first_non_space;
	int equals_position;
	int last_letter;
	int first_after_equals;
	int last_after_equals;

	int number_of_entries;
	char **entry_names;		// the names of the entries in the section
	char **entry_values;	// the values of each entry.
} IniEntry;


///////////////////////////////////////////////////////////////
// Structure for an Ini file. This is the one you use.
///////////////////////////////////////////////////////////////
typedef struct {
	int open_bracket_pos;
	int close_bracket_pos;

	char ini_file_read;			// whether the ini file was opened or not
	int number_of_lines;		// the number of lines in the .ini file
	int number_of_sections;	// the number of individual sections in the .ini file
		
	char **inifile;					// copy of the .ini file
	char **section_names;		// the names of the sections in the file
	long int *section_pos;	// the lines of the file the sections start on
	IniEntry **entries;			// the entries for each section
} IniFile;

char Ini_ReadIniFile(char *filename, IniFile *iniFile);
char* Ini_GetValue(IniFile *iniFile, char *section, char *entry);
char** Ini_SectionEntries(IniFile *iniFile, char *section);
int Ini_NumberOfEntries(IniFile *iniFile, char *section);
void Ini_FreeIniFile(IniFile *iniFile);




