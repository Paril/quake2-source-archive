// 
// wf_classmgr.c -- Class Manager
// 
// 
// 7/10/98 - Gregg Reno 
// 

// INCLUDES ///////////////////////////////////////////////// 

#include "g_local.h" 
#include "wf_classmgr.h"

//Global variable
int modID;

void readline(FILE *file, char *str, int max);
void fixline(char *str);

int wf_GetModelFromName(char *name);

//Test noclass server var to see if a specific bit is set
/*
qboolean ClassAllowed(int classnum)
{
	int bitmask;
	if (classnum <= 0 || classnum > 16) return true;

	bitmask = 1 << (classnum-1);	//set the right bit
	if ((int)noclass->value & bitmask) 
		return false;
	else
		return true;
}
*/

void ClearString(char *s, int len)
{
	int i;
	for (i = 0; i < len; ++i)
		s[i] = '\0';
}

void DecryptString(unsigned char *s, int len)
{
	int i;
	for (i = 0; i < len; ++i)
	{
		if (s[i] == 255) s[i] = 0;
		if (s[i] >= 128) s[i] = s[i] - 128;
	}
}

void EncryptString(unsigned char *s, int len)
{
	int i;
	for (i = 0; i < len; ++i)
	{
		if (s[i] > 32 && s[i] < 128) s[i] = s[i] + 128;
	}
}

int IsEncrypted(unsigned char *s, int len)
{
	int i;
	int retval;
	retval = 0;
	for (i = 0; i < len; ++i)
	{
		if (s[i] > 128) retval = 1;
	}

	//Consider blank lines encrypted
	if (s[0] == 0) retval = 1;
	return (retval);
}

int ReadEncryptedLine(FILE *fp, char *szLineIn, int maxlen, int lineno)
{
	readline(fp, szLineIn, maxlen);
	if (IsEncrypted(szLineIn, strlen(szLineIn)) == 0)
	{
		DecryptString(szLineIn, strlen(szLineIn));
		gi.dprintf("Invalid file format - line %d: %s\n",lineno,szLineIn);
		return (lineno + 1);
	}
	DecryptString(szLineIn, strlen(szLineIn));
	fixline(szLineIn);

//gi.dprintf("ReadLine:%s\n",szLineIn);

	return (lineno + 1);
}


//Clear class info
void ClearClassInfo()
{
	int i;
	int j;

	numclasses = 0;
	for (i = 0; i < MAX_CLASSES; ++i)
	{
		ClearString(classinfo[i].name,32);
		ClearString(classinfo[i].model_name,32);
		ClearString(classinfo[i].skin1,32);
		ClearString(classinfo[i].skin2,32);

		for (j = 0; j < 10; ++j) 
			classinfo[i].weapon[j] = 0;
		for (j = 0; j < 3; ++j) 
			classinfo[i].grenade[j] = 0;

		classinfo[i].max_armor = 0;
		classinfo[i].max_speed = 0;
		classinfo[i].max_health = 0;
		classinfo[i].special = 0;
		classinfo[i].items = 0;
		classinfo[i].limit = 99;
	}
}

//Load the class info from a file
int LoadClassInfo(char *filename) 
{ 
	FILE *fp;
	int  i=0; 
	int fileclassnum;
	//int j;
	int len;
	int lineno;
	char versionstr[80];
	int version;
	unsigned char szLineIn[80];
	char path[100];

	modID = 0;

	ClearClassInfo();

	//Make sure we open class def from the game directory
    strcpy(path, gamedir->string);
#if defined(_WIN32) || defined(WIN32)
    strcat(path,"\\");
#else
    strcat(path,"/");
#endif
    strcat(path,filename);

	//Open file
	fp = WFOpenFile(NULL, path); 
	if (!fp)  // opened successfully? 
	{
		gi.dprintf("ClassMgr: Could not open file: %s\n",path);
		return 0;
	}

	//First line should be version # and mod id
	lineno = ReadEncryptedLine(fp, szLineIn, 80, 0);
	sscanf(szLineIn, "%s %d", versionstr, &modID);

	//Check for version 3
	if ((versionstr[0]=='V') && (versionstr[1] == '3'))
		version = 3;
	else
	{
		gi.dprintf("Invalid Version ID: [%s].  Should be V3.\n", versionstr);
		WFCloseFile(NULL, fp); 
		return 0;
	}

	//gi.dprintf("Game Dir = %s\n",gamedir->string);

	//Make sure the WF class files are run only from the wf directory
/*
	if (modID != MOD_ID_WF)
	{
		if (strcmp(gamedir->string, "wf") == 0)
		{
			gi.dprintf("Sorry, MOCK class def files cannot be run from the wf directory\n");
			WFCloseFile(NULL, fp); 
			return 0;
		}
	}
*/
	if (modID == MOD_ID_WF)
	{
		gi.cvar ("mod", "The Weapons Factory", CVAR_SERVERINFO);
	}
	else
	{
		gi.cvar ("mod", "Mod Construction Kit", CVAR_SERVERINFO);
	}


	//Second line is the name of the class definition pack
	lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
	if (strlen(szLineIn) >30) szLineIn[31] = '\0';

	sscanf(szLineIn, "%s", classdefname); 
//if (wfdebug) gi.dprintf("New class def loaded: %s\n",classdefname); 
	//Use file name instead
	gi.dprintf("New class def loaded: %s\n",path); 
	
	// Read lines from class info file
	i = 1;
	fileclassnum = 1;
	while ((!feof(fp)) && (i<=MAX_CLASSES)) 
	{ 
		//Line 1 - class name
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
		len = strlen(szLineIn);
		if (len <=0) continue;		//Skip blank lines

		//Class definitions must start with "[" and end with "]"

		if ( (szLineIn[0] != '[') || (szLineIn[len-1] != ']'))
		{
			gi.dprintf("Invalid class definition: %s\n", szLineIn);
			//for (j = 0; szLineIn[j] != 0; ++j)
			//	gi.dprintf("[%d]",szLineIn[j]);
			WFCloseFile(NULL, fp); 
			return 0;
		}

		// Save the class name
		strncpy(classinfo[i].name, &szLineIn[1], len-2);
		classinfo[i].name[len-1] = 0;


//		if (ClassAllowed(fileclassnum))
//			gi.dprintf(" Class %d: <%s>\n", i, classinfo[i].name);
//		else
//			gi.dprintf(" Class <%s> disabled.\n", classinfo[i].name);

		//Line 2 - model and skins and decoy skin number
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);

		sscanf(szLineIn, "%s %s %s %d", classinfo[i].model_name, classinfo[i].skin1, classinfo[i].skin2, &classinfo[i].decoyskin ); 
		classinfo[i].model = wf_GetModelFromName(classinfo[i].model_name);

if (wfdebug) gi.dprintf("  Model=<%s>, Skin 1=<%s>, Skin 2=<%s>\n", classinfo[i].model_name, classinfo[i].skin1, classinfo[i].skin2 ); 

		//Line 3 - weapons
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
		sscanf(szLineIn, "%d %d %d %d %d %d %d %d %d %d ", 
			&classinfo[i].weapon[0], &classinfo[i].weapon[1],
			&classinfo[i].weapon[2], &classinfo[i].weapon[3],
			&classinfo[i].weapon[4], &classinfo[i].weapon[5],
			&classinfo[i].weapon[6], &classinfo[i].weapon[7],
			&classinfo[i].weapon[8], &classinfo[i].weapon[9]); 
if (wfdebug) gi.dprintf("  Weapons: %d %d %d %d %d %d %d %d %d %d\n",
			classinfo[i].weapon[0], classinfo[i].weapon[1],
			classinfo[i].weapon[2], classinfo[i].weapon[3],
			classinfo[i].weapon[4], classinfo[i].weapon[5],
			classinfo[i].weapon[6], classinfo[i].weapon[7],
			classinfo[i].weapon[8], classinfo[i].weapon[9]); 

		//Line 4 - Grenades
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
		sscanf(szLineIn, "%d %d %d", 
			&classinfo[i].grenade[0], 
			&classinfo[i].grenade[1] , 
			&classinfo[i].grenade[2]); 

if (wfdebug) gi.dprintf("  Grenades <%d> <%d> <%d>\n", 
			classinfo[i].grenade[0], 
			classinfo[i].grenade[1] , 
			classinfo[i].grenade[2]);

		//Line 5 - armor, speed, health, special, items
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
		sscanf(szLineIn, "%d %d %d %d %d", 
			&classinfo[i].max_armor, &classinfo[i].max_speed,
			&classinfo[i].max_health, &classinfo[i].special, &classinfo[i].items);
if (wfdebug) gi.dprintf("  armor=%d, speed=%d, health=%d, special=%d, items=%d\n", 
			classinfo[i].max_armor, classinfo[i].max_speed,
			classinfo[i].max_health, classinfo[i].special,classinfo[i].items);


		//Line 6 - Ammo limits 
		lineno = ReadEncryptedLine(fp, szLineIn, 80, lineno);
		sscanf(szLineIn, "%d %d %d %d %d %d", 
			&classinfo[i].max_ammo[AMMO_BULLETS], 
			&classinfo[i].max_ammo[AMMO_SHELLS], 
			&classinfo[i].max_ammo[AMMO_ROCKETS],
			&classinfo[i].max_ammo[AMMO_GRENADES], 
			&classinfo[i].max_ammo[AMMO_CELLS], 
			&classinfo[i].max_ammo[AMMO_SLUGS]); 
//		else
//		{
//			classinfo[i].max_ammo[AMMO_BULLETS] = 200;
//			classinfo[i].max_ammo[AMMO_SHELLS] = 100;
//			classinfo[i].max_ammo[AMMO_ROCKETS] = 50;
//			classinfo[i].max_ammo[AMMO_GRENADES] = 50;
//			classinfo[i].max_ammo[AMMO_CELLS]= 200;
//			classinfo[i].max_ammo[AMMO_SLUGS] = 50;
//		}

		//Get ready for next line

		//Should we skip this class?

//		if (ClassAllowed(fileclassnum))
//		{
			numclasses = i;
			i++; 
//		}

		++fileclassnum;

	} 
	gi.dprintf("%d Classes Loaded\n", numclasses);
	WFCloseFile(NULL, fp); 
	return 1;     // normal exit 
}



void Cmd_Classdef_f () 
{ 
	char *filename; 
	filename = gi.argv(2);   // get filename from command line 
	LoadClassInfo(filename);
}
  

