#include "g_local.h"
#include "eavy.h"

#define EAVY_RESTRICTED_RADIUS 512
#define EAVY_SPOT_ORIGIN 41
#define EAVY_CHECK_TRACE 70

void EAVYSpawnFlags(void)
{
    edict_t *redflag, *blueflag;

    if (!ctf->value)
        return;

    if (!ctf_autoflagspawn->value)
        return;

    redflag = G_Find (NULL, FOFS(classname), "item_flag_team1");
    blueflag = G_Find (NULL, FOFS(classname), "item_flag_team2");

    if (!redflag)
    {
        redflag = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
        redflag = EAVYFindFarthestFlagPosition(redflag);
        if (!redflag)
            redflag = G_Find (NULL, FOFS(classname), "info_player_start");
        if (redflag)
        {
            redflag->classname = "item_flag_team1";
            ED_CallSpawn (redflag);
        }
    }
    if (!blueflag)
    {
        blueflag = EAVYFindFarthestFlagPosition(redflag);
        if (blueflag)
        {
            blueflag->classname = "item_flag_team2";
            ED_CallSpawn (blueflag);
        }
    }
    if (!redflag || !blueflag)
		gi.error ("Couldn't spawn flags!");
}

edict_t *EAVYFindFarthestFlagPosition(edict_t *flag)
{
    edict_t *bestspot, *spot = NULL;
    float   bestdistance, bestflagdistance = 0;
    vec3_t  v;

    bestspot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

    while(spot = G_Find (spot, FOFS(classname), "info_player_deathmatch"))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        bestdistance = VectorLength(v);

        if (bestdistance > bestflagdistance)
        {
            bestflagdistance = bestdistance;
            bestspot = spot;
        }
    }

    return bestspot;
}

void EAVYSetupFlagSpots(void)
{
    edict_t *ent, *spot;

    if (!ctf->value)
        return;

        ent = G_Find (NULL, FOFS(classname), "info_player_team1");
        spot = G_Find (NULL, FOFS(classname), "info_player_team2");
        if (!ent && !spot)
        {
            ent = G_Find (NULL, FOFS(classname), "item_flag_team1");

            EAVYSpawnTeamNearFlag (ent);

            ent = G_Find (NULL, FOFS(classname), "item_flag_team2");

            EAVYSpawnTeamNearFlag (ent);

            EAVYSpawnTeamNearFlagCheck();
        }
}

void EAVYSpawnTeamNearFlag(edict_t *flag)
{
    edict_t *spot = NULL;
    float   dist;
    vec3_t  v;

    if (!ctf_autospawnnearflag->value)
        return;

    while(spot = G_Find (spot, FOFS(classname), "info_player_deathmatch"))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            if (!strcmp(flag->classname, "item_flag_team1"))
            {
                spot->classname = "info_player_team1";
                spot->s.effects |= EF_COLOR_SHELL;
                spot->s.renderfx |= RF_SHELL_RED;
                ED_CallSpawn (spot);
            }
            if (!strcmp(flag->classname, "item_flag_team2"))
            {
                spot->classname = "info_player_team2";
                spot->s.effects |= EF_COLOR_SHELL;
                spot->s.renderfx |= RF_SHELL_BLUE;
                ED_CallSpawn (spot);
            }
        }
    }
}

void EAVYSpawnTeamNearFlagCheck(void)
{
    edict_t *flag, *spot = NULL;
    float   dist;
    vec3_t  v;

    if (!ctf_autospawnnearflag->value)
        return;

    flag = G_Find (NULL, FOFS(classname), "item_flag_team1");
    while(spot = G_Find (spot, FOFS(classname), "info_player_team2"))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            spot->classname = "info_player_deathmatch";
            spot->s.effects &= ~EF_COLOR_SHELL;
            spot->s.renderfx &= ~RF_SHELL_BLUE;
            ED_CallSpawn (spot);
        }
    }
    flag = G_Find (NULL, FOFS(classname), "item_flag_team2");
    while(spot = G_Find (spot, FOFS(classname), "info_player_team1"))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            spot->classname = "info_player_deathmatch";
            spot->s.effects &= ~EF_COLOR_SHELL;
            spot->s.renderfx &= ~RF_SHELL_RED;
            ED_CallSpawn (spot);
        }
    }
}
