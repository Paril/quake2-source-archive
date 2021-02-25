
#include "g_local.h"
#include "g_geneavy.h"

#define EAVY_RESTRICTED_RADIUS 	512
#define EAVY_SPOT_ORIGIN 	41

void SP_misc_teleporter_dest(edict_t * self);
void ED_CallSpawn(edict_t * ent);

char *
ReadTextFile(char *filename)
{
    FILE *fp;
    char *filestring = NULL;
    int len;

    fp = fopen(filename, "r");
    if (!fp)
	return NULL;

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    filestring = gi.TagMalloc(len + 1, TAG_LEVEL);
    if (!filestring) {
	fclose(fp);
	return NULL;
    }

    len = fread(filestring, 1, len, fp); 
    filestring[len] = 0;

    fclose(fp);

    return filestring;
}

char *
EAVYLoadEntities(char *mapname, char *entities)
{
    char entfilename[MAX_QPATH] = "";
    char *newentities;
    int islefn;

    //sprintf(entfilename, "%s/%s/maps/", basedir->string, gamedir->string);
	sprintf(entfilename, "gen\\ent\\");

    islefn = strlen(entfilename);
    strcpy(entfilename + islefn, mapname);
    strcat(entfilename, ".ent");

    newentities = ReadTextFile(entfilename);

    if (newentities)
	return newentities;
    else
	return entities;
}

void 
EAVYCTF_Init(void)
{
    if (!gen_ctf->value)
	return;

    EAVYSpawnFlags();
    EAVYSetupFlagSpots();
}

edict_t *
EAVYFindFarthestFlagPosition(edict_t * flag)
{
    edict_t *bestspot, *spot = NULL;
    float bestdistance, bestflagdistance = 0;
    vec3_t v;

    bestspot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch"))) {
	VectorSubtract(spot->s.origin, flag->s.origin, v);
	bestdistance = VectorLength(v);

	if (bestdistance > bestflagdistance) {
	    bestflagdistance = bestdistance;
	    bestspot = spot;
	}
    }

    return bestspot;
}

void 
EAVYSpawnFlags(void)
{
    edict_t *redflag, *blueflag;

    redflag = G_Find(NULL, FOFS(classname), "item_flag_team1");
    blueflag = G_Find(NULL, FOFS(classname), "item_flag_team2");

    if (gen_ctf->value && !redflag) {
	redflag = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	redflag = EAVYFindFarthestFlagPosition(redflag);
	if (!redflag)
	    redflag = G_Find(NULL, FOFS(classname), "info_player_start");
	if (redflag) {
	    redflag->classname = "item_flag_team1";
	    ED_CallSpawn(redflag);
	}
    }

    if (redflag && !blueflag) {
	blueflag = EAVYFindFarthestFlagPosition(redflag);
	if (blueflag) {
	    blueflag->classname = "item_flag_team2";
	    ED_CallSpawn(blueflag);
	}
    }

    if (gen_ctf->value && !blueflag)
	gi.dprintf("EAVY.EAVYSpawnFlags = FAILED!\n");

}

void 
EAVYSpawnTeamNearFlagCheck(void)
{
    edict_t *flag, *spot = NULL;
    float dist;
    vec3_t v;

    flag = G_Find(NULL, FOFS(classname), "item_flag_team1");
    while ((spot = G_Find(spot, FOFS(classname), "info_player_team2"))) {
	VectorSubtract(spot->s.origin, flag->s.origin, v);
	dist = VectorLength(v);
	if (EAVY_RESTRICTED_RADIUS > dist) {
	    spot->classname = "info_player_deathmatch";
	    spot->svflags &= ~SVF_NOCLIENT;
	    spot->s.effects &= ~EF_COLOR_SHELL;
	    spot->s.renderfx &= ~RF_SHELL_BLUE;
	    ED_CallSpawn(spot);

	}
    }

    flag = G_Find(NULL, FOFS(classname), "item_flag_team2");
    while ((spot = G_Find(spot, FOFS(classname), "info_player_team1"))) {
	VectorSubtract(spot->s.origin, flag->s.origin, v);
	dist = VectorLength(v);
	if (EAVY_RESTRICTED_RADIUS > dist) {
	    spot->classname = "info_player_deathmatch";
	    spot->svflags &= ~SVF_NOCLIENT;
	    spot->s.effects &= ~EF_COLOR_SHELL;
	    spot->s.renderfx &= ~RF_SHELL_RED;
	    ED_CallSpawn(spot);
	}
    }
}

void 
EAVYSpawnTeamNearFlag(edict_t * flag)
{
    edict_t *spot = NULL;
    float dist;
    vec3_t v;

    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch"))) {
	VectorSubtract(spot->s.origin, flag->s.origin, v);
	dist = VectorLength(v);
	if (EAVY_RESTRICTED_RADIUS > dist) {
	    if (!strcmp(flag->classname, "item_flag_team1")) {
		spot->classname = "info_player_team1";
		ED_CallSpawn(spot);
	    }
	    else if (!strcmp(flag->classname, "item_flag_team2")) {
		spot->classname = "info_player_team2";
		ED_CallSpawn(spot);
	    }
	}
    }
}

void 
EAVYSetupFlagSpots(void)
{
    edict_t *ent, *spot;

    spot = G_Find(NULL, FOFS(classname), "misc_ctf_small_banner");
    if (!spot && gen_ctf->value) {
	ent = G_Find(NULL, FOFS(classname), "info_player_team1");
	spot = G_Find(NULL, FOFS(classname), "info_player_team2");
	if (!ent && !spot) {
	    ent = G_Find(NULL, FOFS(classname), "item_flag_team1");
	    // Bugfix for starting client with '+set game ctf'
	    if (!ent)	
		return;
	    spot = G_Spawn();
	    spot->classname = "misc_ctf_small_banner";
	    spot->spawnflags = 0;	// Red
            VectorCopy(ent->s.origin, spot->s.origin);
	    spot->s.origin[2] += EAVY_SPOT_ORIGIN;
	    
	    ED_CallSpawn(spot);
	    EAVYSpawnTeamNearFlag(ent);

	    ent = G_Find(NULL, FOFS(classname), "item_flag_team2");
	    // I would be surprised if this EVER happened
	    if (!ent)
		return;
	    spot = G_Spawn();
	    spot->classname = "misc_ctf_small_banner";
	    spot->spawnflags = 1;	// Blue
            VectorCopy(ent->s.origin, spot->s.origin);
	    spot->s.origin[2] += EAVY_SPOT_ORIGIN;
	    
	    ED_CallSpawn(spot);
	    EAVYSpawnTeamNearFlag(ent);

	    EAVYSpawnTeamNearFlagCheck();
	}
    }
}

/* (C) EAVY */
// Freitag, 20. März 1998, 00:57
