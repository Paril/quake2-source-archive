/* GenderMod 0.1

   gender.c - Functions to deal with gender based on models
*/

#include <stdio.h>
#include <stdlib.h>
#include "g_local.h"
#include "gender.h"
#include "s_readconfig.h"

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
	int linecount, modcount,j;
	char line[256], *res;
	FILE *mfile;
	char logGenderFileName[ 80 ];

	// properly initialize variable
	for (j=0; j<60; j++)
		logGenderFileName[j]='\0';

	linecount=0;
	if (mpath == NULL) {
		ModelGenderMap = NULL;
		return(0);
	}
	
	// Build the filename with path
	sprintf( logGenderFileName, "%s%s", ModelGenDir, mpath);
			
	//mfile = fopen(mpath, "r");
	mfile = fopen(logGenderFileName, "r");
	if (mfile == NULL)
	{
		ModelGenderMap = NULL;
		//gi.dprintf("ModelGender Error: Couldn't open file %s\n", mpath);
		gi.dprintf("ModelGender Error: Couldn't open file %s, check config.txt file settings\n", logGenderFileName);
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
			gi.dprintf("ModelGender Error: Couldn't understand line %d in %s:\n",
						linecount, mpath);
			gi.dprintf("--> %s\n", line);
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


/* **************************************************************
   Gender Pronouns
   ************************************************************** */

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


/* **************************************************************************
   Determining Player Gender
   ************************************************************************** */

/* **************************************************************************
   UserSetPlayerGender

   Set the gender of the player according to the userinfo, unless the proposed
   "sex" value is inconsistent with the model map or hard-coded values.

   returns GENDER_ERROR if not set to proposed gender, otherwise returns
   the proposed (successful) gender

   ************************************************************************** */

int UserSetPlayerGender (edict_t *ent)
{
	char		info[MAX_STRING_CHARS];
	int			mgen, pgen;

	if (!ent->client)
		return(GENDER_ERROR);

	// Get proposed gender
	strcpy(info, Info_ValueForKey (ent->client->pers.userinfo, "sex"));
	pgen = ConvertGenderSpec(info);
	if (pgen == GENDER_ANY || pgen == GENDER_ERROR)
		pgen = GENDER_MALE;

	// Get gender from skin
	strcpy(info, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	TrimSkinToModel(info);
	mgen = GetModelGenderFromMap(info);
	if ((mgen != GENDER_ERROR) && ((mgen == pgen) || (mgen == GENDER_NEUTER)))
	{
		ent->client->resp.gender = pgen;
		return(pgen);
	}
	else if (mgen == GENDER_ERROR)
	{
		// No gender info from model->gender map, so use what the player has
		// specified.
		ent->client->resp.gender = pgen;
		return(pgen);
	}

	// proposed male but using female model, etc.; don't allow, set
	// according to model.
	ent->client->resp.gender = mgen;
	return(GENDER_ERROR);
}

int GetPlayerGender (edict_t *ent)
{
	if (! ent->client)
		return(GENDER_ERROR);

	if (ent->client->resp.gender)
		return(ent->client->resp.gender);

	// Sets player gender according to their model
	return(UserSetPlayerGender(ent));
}

void GenderModUsage (edict_t *ent)
{
		gi.cprintf(ent, PRINT_MEDIUM, "Use \"cmd gender me\" to find out your\n");
		gi.cprintf(ent, PRINT_MEDIUM, "gender according to this mod.\n\n");
		gi.cprintf(ent, PRINT_MEDIUM, "Use \"set sex VALUE u\" to specify\n");
		gi.cprintf(ent, PRINT_MEDIUM, "your gender, where VALUE is m (male),\n");
		gi.cprintf(ent, PRINT_MEDIUM, "f (female), or n (neuter).  Put this\n");
		gi.cprintf(ent, PRINT_MEDIUM, "in your config.cfg or autoexec.cfg\n");
}

/* **************************************************************************
   Initialization
   ************************************************************************** */

void GenderModInitGame ()
{
	gi.dprintf("*** %s ***\n", GENDERMOD_VERSION);
	ReadModelGenders("ModelGen.dat");
	gi.dprintf("GenderMod loaded.\n");
}

void Cmd_Gender_f (edict_t *ent)
{
	char * arg;
	int n;
	edict_t *player;

	arg = gi.argv(1);

	if (Q_stricmp(arg, "help") == 0)
	{
		GenderModUsage(ent);
	}
	else if (Q_stricmp(arg, "me") == 0)
	{
		if (ent->client)
			gi.cprintf(ent, PRINT_MEDIUM, "Your gender is recorded as '%s'.\n",
						GenderToName(ent->client->resp.gender));
	}
	// We'll leave this undocumented for the moment.
	else if (Q_stricmp(arg, "players") == 0)
	{
		for (n = 1; n <= maxclients->value; n++)
		{
			player = &g_edicts[n];
			if (!player->inuse)
				continue;
			if (!player->client)
				continue;
			gi.cprintf(ent, PRINT_MEDIUM, "%s: (%d) %s\n",
					   player->client->pers.netname,
					   GetPlayerGender(player),
					   GenderToName(GetPlayerGender(player)));
		}
	}
	else
		GenderModUsage(ent);
}
