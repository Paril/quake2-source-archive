/* *** STRIPPED DOWN gender.c *** */

#include <string.h>
#include "external.h"

/* GenderMod 0.1

   gender.c - Functions to deal with gender based on models
*/

#include <stdio.h>
#include <stdlib.h>
//#include "g_local.h"
#include "gender.h"

struct ModelGenderMapEntry **ModelGenderMap;


/* **************************************************************
   Model Characteristics
   ************************************************************** */

struct ModelGenderMapEntry *MakeModelGenderEntry (char *modelname, char *gender)
{
	int gen;
	struct ModelGenderMapEntry *entry;

	entry = (struct ModelGenderMapEntry *)
			malloc(sizeof(struct ModelGenderMapEntry));

	gen = ConvertGenderSpec(gender);
	if (gen == GENDER_ERROR)
		gen = GENDER_MALE;

	entry->gender = gen;
	entry->model = strdup(modelname);
	return(entry);
}

char * GenderToName (int gender)
{
	if (gender == GENDER_MALE)
		return("male");
	else if (gender == GENDER_FEMALE)
		return("female");
	else if (gender == GENDER_NEUTER)
		return("neuter");
	else
		return("unknown");
}

// Default gender function; behaves like in the original id software code
int DefaultModelGender (char *name)
{
	if (name[0] == 'M' || name[0] == 'm')
		return(GENDER_MALE);
	else if (name[0] == 'F' || name[0] == 'f')
		return(GENDER_FEMALE);
	else return(GENDER_MALE);
}

// GetModelGenderFromMap looks up the model name in the gender map.
int GetModelGenderFromMap (char *name)
{
	int i=0;

	// Hard code id's official male/female/cyborg/crakhor models.  The
	// initial character checks could save a millisecond or two here and
	// there.  These 4 models are also probably the most common in
	// most Q2 DM/CTF games and other mods.
	if ((name[0] == 'M' || name[0] == 'm') &&
			(Q_stricmp(name, "male") == 0))
		return(GENDER_MALE);
	else if ((name[0] == 'F' || name[0] == 'f') &&
			 (Q_stricmp(name, "female") == 0))
		return(GENDER_FEMALE);
	else if (name[0] == 'C' || name[0] == 'c')
	{
		if (Q_stricmp(name, "cyborg") == 0)
			return(GENDER_MALE);
		else if (Q_stricmp(name, "crakhor") == 0)
			return(GENDER_FEMALE);
	}

	// This should perform a binary search, which would be MUCH faster.
	// Hashing on the first character of the model name might be good
	// too.
	if (ModelGenderMap != NULL)
	{
		while (ModelGenderMap[i] != NULL)
		{
			if (Q_stricmp(name, ModelGenderMap[i]->model) == 0)
				return(ModelGenderMap[i]->gender);
			i++;
		}
	}
	return(GENDER_ERROR);
}

// DESTRUCTIVE.
//
// If multiple directories are in the model name, uses all directories
void TrimSkinToModel (char *skin)
{
	int i=0, len=strlen(skin);

	for(i=len-1; i>=0; i--)
	{
		if (skin[i] == '/')
		{
			skin[i] = '\0';
			break;
		}
	}
}

int ReadModelGenders (char *mpath)
{
	int linecount, modcount;
	char line[256], *res;
	FILE *mfile;

	linecount=0;
	if (mpath == NULL) {
		ModelGenderMap = NULL;
		return(0);
	}
	mfile = fopen(mpath, "r");
	if (mfile == NULL)
	{
		ModelGenderMap = NULL;
		printf("ModelGender Error: Couldn't open file %s\n", mpath);
		return(0);
	}
	while (fgets(line, sizeof(line), mfile) != NULL)
	{
		if (line[0] == '#') continue;
		if (line[0] == '\0') continue;
		linecount++;
	}
	ModelGenderMap = (struct ModelGenderMapEntry **) malloc((linecount+1) * 
					  sizeof(struct ModelGenderMapEntry *));
	fseek(mfile, 0, 0);
	linecount=0;
	modcount = 0;
	while (fgets(line, sizeof(line), mfile) != NULL)
	{
		linecount++;
		if (line[0] == '#') continue;
		// Strip out carriage return
		line[strlen(line)-1] = '\0';
		if (line[0] == '\0') continue;
		res = strchr(line, ':');
		if (res == NULL) {
			printf("ModelGender Error: Couldn't understand line %d in %s:\n",
						linecount, mpath);
			printf("--> %s\n", line);
			continue;
		}
		*res = '\0';
		ModelGenderMap[modcount] = MakeModelGenderEntry(line, res+1);
		modcount++;
	}
	fclose(mfile);
	if (modcount == 0)
		ModelGenderMap = NULL;
	else
		ModelGenderMap[modcount] = NULL;
	return(1);
}


int ConvertGenderSpec (char *str)
{
	if (str[0] == '*')
		return(GENDER_ANY);
	else if ((str[0] == 'M') || (str[0] == 'm'))
		return(GENDER_MALE);
	else if ((str[0] == 'F') || (str[0] == 'f'))
		return(GENDER_FEMALE);
	else if ((str[0] == 'N') || (str[0] == 'n'))
		return(GENDER_NEUTER);
	else return(GENDER_ERROR);
}

/* his, her, its */
char * GenderToHisHer (int gender)
{
	if (gender == GENDER_MALE)
		return("his");
	else if (gender == GENDER_FEMALE)
		return("her");
	else if (gender == GENDER_NEUTER)
		return("its");
	else return("his");
}

/* her, him, it */
char * GenderToHimHer (int gender)
{
	if (gender == GENDER_MALE)
		return("him");
	else if (gender == GENDER_FEMALE)
		return("her");
	else if (gender == GENDER_NEUTER)
		return("it");
	else return("him");
}

/* herself, himself, itself */
char * GenderToHimselfHerself (int gender)
{
	if (gender == GENDER_MALE)
		return("himself");
	else if (gender == GENDER_FEMALE)
		return("herself");
	else if (gender == GENDER_NEUTER)
		return("itself");
	else return("himself");
}

/* she, he, it */
char * GenderToSheHe (int gender)
{
	if (gender == GENDER_MALE)
		return("he");
	else if (gender == GENDER_FEMALE)
		return("she");
	else if (gender == GENDER_NEUTER)
		return("it");
	else return("he");
}

char * GenderToBoyGirl (int gender)
{
	if (gender == GENDER_MALE)
		return("boy");
	else if (gender == GENDER_FEMALE)
		return("girl");
	else
		return("thing");
}

char * GenderToManWoman (int gender)
{
	if (gender == GENDER_MALE)
		return("man");
	else if (gender == GENDER_FEMALE)
		return("woman");
	else
		return("thing");
}


void GenderModInitGame ()
{
	printf("*** %s ***\n", GENDERMOD_VERSION);
	// SrvObTst
	// Put .. in pathname
	ReadModelGenders("../GenderMod/ModelGen.dat");
	// End SrvObTst
	printf("GenderMod loaded.\n");
}
