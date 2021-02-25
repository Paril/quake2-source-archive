/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "g_local.h"
#include "g_map.h"
#include "g_wordfilter.h"
#include "persistent.h"


team_info_t             team_info;
game_locals_t   game;
level_locals_t  level;
game_import_t   gi;
spawn_temp_t    st;
extensions_t ext;

int     sm_meat_index;
int firea_index;
int fireb_index;
int     shiny_index;

int     changemap_spincount = 0;

int meansOfDeath;
edict_t*currentinflictor; //UGLY!
#ifdef VOTE_EXTEND
int oldtimelimit = 0;
#endif
int     currentmapindex;
int     lastrotationmap;

cvar_t  *maxentities;
cvar_t  *deathmatch;
cvar_t  *teameven;
cvar_t  *watchdogtimer;
#ifdef LPBEVEN
cvar_t  *teamlpbeven;
#endif
cvar_t  *buildercap;
cvar_t  *forceteams;
cvar_t  *reconnect_address;
cvar_t  *max_rate;
cvar_t  *max_cl_maxfps;
cvar_t  *min_cl_maxfps;
cvar_t  *banmaps;

cvar_t  *autokick;
cvar_t  *tk_threshold;
cvar_t  *teambonus;
cvar_t  *replenishbp;
cvar_t  *replenishbp_amt;
cvar_t  *replenishbp_tick;

cvar_t  *login;
cvar_t  *login_message;
cvar_t  *banaccountholders;

cvar_t  *gamedir;
cvar_t  *xmins;
cvar_t  *xmins_random;

cvar_t  *spiker_mode;

cvar_t  *turret_mode;
cvar_t  *turret_damage;
cvar_t  *turret_speed;
cvar_t  *turret_home;
cvar_t  *turret_bfg;
cvar_t  *turret_homingvelocityscale;
cvar_t  *turret_homingturnscale;
cvar_t  *turret_delay;

cvar_t *strip_names;
cvar_t *strip_chat;

cvar_t  *idletime;
cvar_t  *obsidletime;

/*cvar_t        *dynamic_configs;
cvar_t  *large_threshold;
cvar_t  *medium_threshold;
cvar_t  *small_threshold;
cvar_t  *tiny_threshold;*/

cvar_t  *voting;
cvar_t  *min_votes;
cvar_t  *votetimer;

cvar_t  *maxswitches;
cvar_t  *talk_threshold;
cvar_t  *nopublicchat;
cvar_t  *noobserverchat;
cvar_t  *deathmessages;
cvar_t  *dmflags;
cvar_t  *fraglimit;
cvar_t  *timelimit;
cvar_t  *reconnect_wait;
cvar_t  *password;
cvar_t  *needpass;
cvar_t  *g_select_empty;
cvar_t  *maxclients;
cvar_t  *maxplayers;
cvar_t  *adminslots;
cvar_t  *curplayers;
cvar_t  *dedicated;
cvar_t  *bandwidth_mode;
cvar_t  *gloomgamelog;
cvar_t  *adminpassword;
cvar_t  *teamsplashdamage;
cvar_t  *teamspawndamage;
cvar_t  *teamarmordamage;
cvar_t  *teamreflectarmordamage;
cvar_t  *teamreflectradiusdamage;
cvar_t  *teamdamage;
cvar_t  *teamstructdamage;
cvar_t  *flashlight_mode;
cvar_t  *laser_color;
cvar_t  *deathtest;
cvar_t  *firetest;
//cvar_t        *speedtest;
cvar_t  *basetest;
cvar_t  *playerclipfix;
cvar_t  *turrettest;
cvar_t  *nohtfragtest;
cvar_t  *pointstest;
cvar_t  *mapdebugmode;
cvar_t  *turretmode;
cvar_t  *secure_modulate_cap;
cvar_t  *ceiling_eggs;
cvar_t  *MAX_SCORE;
cvar_t  *hide_spawns;
cvar_t  *scoring_mode;
cvar_t  *motd;
cvar_t  *motd2;
cvar_t  *motd3;
cvar_t  *sv_maxvelocity;
cvar_t  *sv_gravity;
cvar_t  *sv_rollspeed;
cvar_t  *sv_rollangle;
cvar_t  *gun_x;
cvar_t  *gun_y;
cvar_t  *gun_z;
cvar_t  *run_pitch;
cvar_t  *run_roll;
cvar_t  *bob_up;
cvar_t  *bob_pitch;
cvar_t  *bob_roll;
cvar_t  *sv_airaccelerate;
cvar_t  *sv_cheats;
cvar_t  *gloomcaps;
cvar_t  *recoil;
cvar_t  *randominvert;

cvar_t  *tripwire_repair_count;
cvar_t  *corrupted_tripwires;
cvar_t  *obstacle_frags;
cvar_t  *upgrades;

cvar_t  *spiker_regrow_time;
cvar_t  *spiker_max_spikes_per_client;
cvar_t  *spiker_spike_count;
cvar_t  *spiker_damage;
cvar_t  *spiker_damage_random;
cvar_t  *spiker_speed;
cvar_t  *spiker_distance;

cvar_t  *spiketest;

cvar_t  *shrapnel_damage;
cvar_t  *shrapnel_count;

cvar_t  *sv_updater;

#ifdef ADMINDLLHACK
cvar_t  *admindllbypass;
cvar_t  *admindlltimer;
#endif

// config file names
/*cvar_t        *large_maps;
cvar_t  *medium_maps;
cvar_t  *small_maps;
cvar_t  *tiny_maps;*/
cvar_t  *default_maps;

// bans file and ip address log, accounts
cvar_t  *ipbans;
cvar_t  *iplogs;
cvar_t  *accounts;
cvar_t  *wordbans;
cvar_t  *adminlog;

int             playertimes[6] = {0,0,0,0,0,0};
int             currentmapmode = MAPS_DEFAULT;


cvar_t  *sv_gpmflags;
int gpmflags, oldgpmflags;

float builderfragtime[10];
cvar_t*builderfragtimes;

// cache pointers
unsigned *imagecache;
unsigned *soundcache;

// exports
void EXPORT SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void EXPORT ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean EXPORT ClientConnect (edict_t *ent, char *userinfo);
void EXPORT ClientUserinfoChanged (edict_t *ent, char *userinfo);
void EXPORT ClientDisconnect (edict_t *ent);
void EXPORT ClientBegin (edict_t *ent);
void EXPORT ClientCommand (edict_t *ent);
/*void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);*/
void EXPORT InitGame (void);
void EXPORT G_RunFrame (void);
void EXPORT ServerCommand (void);
// enf of exports
void CheckVotes(void);
void RunEntity (edict_t *ent);

int SV_Cmd_WriteIP_f (void);
void SaveIPLog (char * filename);
void CloseAdminLog (void);

int GetMapPlayers(int currentmap, int minormax);

#ifdef ZZLIB
int compress(char *filename);
#endif
void EXPORT ShutdownGame (void)
{
        int records;

  FreeGamePersistent();
  
        gi.dprintf ("==== ShutdownGame Start ====\n");
        if (gloomlog) {
                #ifdef ZZLIB
                char oldname[MAX_QPATH];
                char newname[MAX_QPATH];
                #endif
                fclose (gloomlog);
                #ifdef ZZLIB
                compress (va("%s/%s/logs/%s", gamedir->string, game.port, gloomlogname));
                sprintf (oldname, "%s/%s/logs/%s", gamedir->string, game.port, gloomlogname);
                sprintf (newname, "%s/%s/logs/%s.zz", gamedir->string, game.port, gloomlogname);
                rename (oldname, newname);
                #endif
        }

        if (oldxmins)
                gi.cvar_set ("xmins", va("%g", oldxmins));

        if (accounts->string[0])
                SaveUserAccounts(accounts->string);

        if (iplogs->string[0])
                SaveIPLog(iplogs->string);

        if (wordbans->string[0])
                WriteWordFilters(wordbans->string);

        CloseAdminLog();

        // HACK (hmm?)
        if (ipbans->string[0]) {
                records = SV_Cmd_WriteIP_f();
                if (records >= 0) {
                        gi.dprintf (" + Wrote %d bans to %s\n", records, ipbans->string);
                } else {
                        gi.dprintf (" - Couldn't save bans to %s\n", ipbans->string);
                }
        }

        gi.dprintf ("==== ShutdownGame End ======\n");

        //warning, don't put any code after these...
        gi.FreeTags (TAG_LEVEL);
        gi.FreeTags (TAG_GAME);
}

void EXPORT WriteGame (char *filename, qboolean autosave)
{
}
void EXPORT WriteLevel (char *filename)
{
}
void EXPORT ReadGame (char *filename)
{
        gi.error ("Can't load single player games in Gloom.\n");
}
void EXPORT ReadLevel (char *filename)
{
        gi.error ("Can't load single player games in Gloom.\n");
}
/*
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
*/

#ifdef __WIN32
__attribute__((dllexport))
#endif
__attribute__((visibility("default"))) game_export_t * GetGameAPI (game_import_t *import)
{
        gi = *import;
        
        globals.apiversion = GAME_API_VERSION;
        globals.Init = InitGame;
        globals.Shutdown = ShutdownGame;
        globals.SpawnEntities = SpawnEntities;

        globals.WriteGame = WriteGame;
        globals.ReadGame = ReadGame;
        globals.WriteLevel = WriteLevel;
        globals.ReadLevel = ReadLevel;

        globals.ClientThink = ClientThink;
        globals.ClientConnect = ClientConnect;
        globals.ClientUserinfoChanged = ClientUserinfoChanged;
        globals.ClientDisconnect = ClientDisconnect;
        globals.ClientBegin = ClientBegin;
        globals.ClientCommand = ClientCommand;

        globals.RunFrame = G_RunFrame;

        globals.ServerCommand = ServerCommand;

        globals.edict_size = sizeof(edict_t);

        return &globals;
}

char *GetGameStats (void)
{
        size_t  i;
        edict_t *ent;

        static char stats[1024];
        char    *str;

        int tks;
        int maxtks;

        int kills;
        int maxkills;

        int spawnkills;
        int maxspawnkills;

        int     deaths;
        int     maxdeaths;

        int     effect;
        int     maxeffect;

        int     worst;
        int     maxworst;

        edict_t *maxtksby;
        edict_t *maxkillsby;
        edict_t *maxspawnkillsby;
        edict_t *maxdeathsby;

        edict_t *maxeffectby;
        edict_t *maxworstby;

        tks = maxtks = 0;
        kills = maxkills = 0;
        spawnkills = maxspawnkills = 0;
        deaths = maxdeaths = 0;

        effect = maxeffect = worst = maxworst = 0;

        maxworstby = maxeffectby = maxtksby = maxkillsby = maxspawnkillsby = maxdeathsby = NULL;

        stats[0] = '\0';

        for (i=0 ; i<game.maxclients ; i++)
        {
                ent = g_edicts + 1 + i;
                if (!ent->inuse || !ent->client)
                        continue;

                tks = 0;

                if (ent->client->resp.team == TEAM_ALIEN) {
                        tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
                } else if (ent->client->resp.team == TEAM_HUMAN || (level.intermissiontime && ent->client->resp.old_team == TEAM_HUMAN))  {
                        tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
                } else {
                        if (ent->client->resp.old_team == TEAM_HUMAN) {
                                tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
                        } else {
                                tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
                        }
                }

                if (tks > maxtks) {
                        maxtks = tks;
                        maxtksby = ent;
                }

                kills = 0;

                if (ent->client->resp.team == TEAM_ALIEN) {
                        kills = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
                } 
                else if (ent->client->resp.team == TEAM_HUMAN || (level.intermissiontime && ent->client->resp.old_team == TEAM_HUMAN))  
                {
                        kills = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
                } 
                else 
                {
                        if (ent->client->resp.old_team == TEAM_HUMAN) 
                        {
                                kills = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
                        } 
                        else 
                        {
                                kills = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
                        }
                }

                if (kills > maxkills) 
                {
                        maxkills = kills;
                        maxkillsby = ent;
                }

                deaths = ent->client->resp.kills[PLAYERDEATHS];
                if (deaths > maxdeaths) 
                {
                        maxdeaths = deaths;
                        maxdeathsby = ent;
                }

                spawnkills = ent->client->resp.kills[SPAWNKILLS];
                if (spawnkills > maxspawnkills) 
                {
                        maxspawnkills = spawnkills;
                        maxspawnkillsby = ent;
                }
/*
                effect=kills;
                effect+=ent->client->resp.kills[SPAWNKILLS]*10;

                effect=(deaths/effect);

                effect=(1-effect)*100;

                if(effect > 100)
                        effect=0;

                if (effect > maxeffect) 
                {
                        maxeffect = effect;
                        maxeffectby = ent;
                }

                worst=kills;
                worst+=ent->client->resp.kills[SPAWNKILLS]*10;

                if(worst<deaths)
                {
                        worst=(worst/deaths);
                        worst=(1-worst)*100;
                }
                else
                        worst=0;

                if (worst > maxworst) 
                {
                        maxworst = worst;
                        maxworstby = ent;
                }
*/
//
///             if(!maxeffectby)
//                      maxeffectby = ent;
///             if(!maxworstby)
///                     maxworstby = ent;

//              if(!maxworst)
//                      maxworst = worst;
                
        }

        i = 0;
        if (maxkills) 
        {
                str = va("Most kills: %s (%d) ",maxkillsby->client->pers.netname, maxkills);
                strcpy (stats + i, str);
                i += strlen (str);
        }
        if (maxspawnkills) 
        {
                str = va("Most spawn kills: %s (%d) ",maxspawnkillsby->client->pers.netname, maxspawnkills);
                strcpy (stats + i, str);
                i += strlen (str);
        }
        if (maxtks) 
        {
                str = va("Most teamkills: %s (%d)\n",maxtksby->client->pers.netname, maxtks);
                strcpy (stats + i, str);
                i += strlen (str);
        }
        if (maxdeaths) 
        {
                str = va("Most feed: %s (%d) ", maxdeathsby->client->pers.netname, maxdeaths);
                strcpy (stats + i, str);
                i += strlen (str);
        }
/*
        if (maxeffectby) 
        {
                str = va("Most Effective: %s (%d) ", maxeffectby->client->pers.netname, maxeffect);
                strcpy (stats + i, str);
                i += strlen (str);
        }
        if (maxworstby) 
        {
                str = va("Worst player: %s (-%d) ", maxworstby->client->pers.netname, maxworst);
                strcpy (stats + i, str);
                i += strlen (str);
        }
*/

        //if (stats[0])
        //      strcpy (stats + i, "\n");

        return stats;
}

/*
EndDMLevel

The timelimit or fraglimit has been exceeded
*/
void ClearSpawnQueue (void);
void EndDMLevel (void)
{
        edict_t         *ent = NULL;
        //int           j=0;
        unsigned int            nextmap=0;
        char    *map = NULL;
        qboolean looped=false;

        ClearSpawnQueue ();

        // trigger win ents
        if (team_info.winner == TEAM_ALIEN) {

                ent=G_Find (NULL, FOFS(classname), "on_aliens_win");

        } else if (team_info.winner == TEAM_HUMAN) {

                ent=G_Find (NULL, FOFS(classname), "on_humans_win");

        } else {
                ent=G_Find (NULL, FOFS(classname), "on_tie_game");
        }

        if (ent && !level.voted_map_change)
        {
                if (ent->map)
                        strncpy(level.nextmap,ent->map, sizeof(level.nextmap));
                G_UseTargets ( ent, ent->activator);
        }

        if (!level.voted_map_change && !team_info.timelimit_nobody) {
          gamepersistent->wins[team_info.timelimitmode][team_info.winner].count++;
          gamepersistent->wins[team_info.timelimitmode][team_info.winner].time += level.time/60.0f;
        }

        // print statistics
        totaldeaths += (team_info.bodycount[TEAM_HUMAN] + team_info.bodycount[TEAM_ALIEN]);
        gi.bprintf (PRINT_HIGH, "The body count was %d humans and %d spiders (%u).\nGame lasted %d minutes and there were %d spawn kills.\n%s\n",
                team_info.bodycount[TEAM_HUMAN], team_info.bodycount[TEAM_ALIEN],
                totaldeaths,(int)(level.time / 60),team_info.spawnkills, GetGameStats());

        ent = NULL;

        // nextmap is currentmap
        nextmap = currentmapindex;
        // check if current map is in current rotation. if it is, store the position.
        unsigned int totalplayers = team_info.numplayers[TEAM_NONE] + team_info.numplayers[TEAM_HUMAN] + team_info.numplayers[TEAM_ALIEN];
        if ((totalplayers <= GetMapPlayers(currentmapindex, 1) || GetMapPlayers(currentmapindex, 1) == -1) && totalplayers >= GetMapPlayers(currentmapindex, 0)) {
            lastrotationmap = currentmapindex;
        }
        else if (!lastrotationmap)
            lastrotationmap = currentmapindex;

        /*if (!level.voted_map_change) {
                // no votes, bump up rotation position

                int i;
                int max = -1;
                int min = 256;

                for (i = 5;i >= 0;i--) {
                        if (playertimes[i] > max)
                                max = playertimes[i];
                        if (playertimes[i] < min)
                                min = playertimes[i];
                        average += playertimes[i];
                }
                average -= max;
                average -= min;
                average /= 4;

                gi.dprintf("EndDMLevel: average players for nextmap = %f\n", average);

                //nextmap++;
        }*/

        // if there was a succesful vote, skip rotation
        // if there is a nextmap, skip rotation (trigger or worldspawn does this)
        if (!level.nextmap[0]) {
                // traverse thru map rotation for next map, exclude mission maps (_*)
                // don't exclude too many maps, you might get into random cycles with small rotations

                MAPSEARCH: nextmap = NextByIndex(lastrotationmap, totalplayers);

                if (nextmap == 0 && !looped) {
                        lastrotationmap = 0;
                        looped = true;
                        goto MAPSEARCH;
                } else if (nextmap == 0 && looped) {
                        // did a loop on maplist and didn't find suitable map
                        gi.dprintf("EndDMLevel: No map found for current players, taking same map.\n");
                        // check for worldspawn set nextmap first before forcing
                        nextmap = currentmapindex;
                }

                map = MapnameByIndex(nextmap);
/*              MAPSEARCH: while ((map = MapnameByIndex(nextmap)))
                {
                        foundit=true;

                        // skip mission maps (they don't need to be in rotation anyway)
                        if (map[0] == '_')
                                foundit=false;

                        // HACK: don't take currently played one
                        if (!Q_stricmp(level.mapname, map))
                                foundit=false;

                        if (foundit)
                                break;

                        nextmap++;
                }

                // didn't find suitable map or maplist exhausted, loop from beginning
                if (!map && !looped) {
                        j = nextmap; // j has the totals
                        nextmap = 1;
                        looped = true;
                        goto MAPSEARCH;
                } else if (!map && looped) {
                        // did a loop on maplist and didn't find suitable map, randomize
                        gi.dprintf("EndDMLevel: Maplist looped twice, taking random map.\n");
                        // check for worldspawn set nextmap first before forcing
                        nextmap = (randomMT() + 1) % j;
                        map = MapnameByIndex(nextmap);
                }*/
        }

        // do the actual level change

        // worldspawn forced map change?
        if (!level.nextmap[0])
        {
                if (map)
                        strncpy(level.nextmap, map, sizeof(level.nextmap)-1);
                else
                        strncpy(level.nextmap, level.mapname, sizeof(level.nextmap)-1);
        }

        currentmapindex = nextmap;
}

void CheckNeedPass (void)
{
        int need;

        // if password or spectator_password has changed, update needpass
        // as needed
        if (password->modified) // || spectator_password->modified)
        {
                        password->modified = false; //spectator_password->modified = false;

                        need = 0;

                        if (*password->string && Q_stricmp(password->string, "none"))
                                        need |= 1;
                        //if (*spectator_password->string && Q_stricmp(spectator_password->string, "none"))
                        //              need |= 2;

                        gi.cvar_set("needpass", va("%d", need));
        }
}

void CheckMotd (qboolean force)
{
        static char motd_string[26];
        static char motd_string2[26];
        static char motd_string3[26];

        // set team_menu motd

        if (motd->modified || force)
        {
                strncpy(motd_string, motd->string, sizeof(motd_string) - 1);

                motd->modified = false;
                menu_join_admin[12].text = menu_exitqueue[11].text = team_menu[11].text = motd_string;
        }

        if (motd2->modified || force)
        {
                strncpy(motd_string2, motd2->string, sizeof(motd_string2) - 1);

                motd2->modified = false;
                menu_join_admin[13].text = menu_exitqueue[12].text = team_menu[12].text = motd_string2;
        }

        if (motd3->modified || force)
        {
                strncpy(motd_string3, motd3->string, sizeof(motd_string3) - 1);

                motd3->modified = false;
                menu_join_admin[14].text = menu_exitqueue[13].text = team_menu[13].text = motd_string3;
        }

}

void CheckDMRules (void)
{
        edict_t *ent;
        static qboolean xmins_issued;

        if (level.intermissiontime)
        {
                if (level.intermissiontime == level.framenum) {
                        //r1: the map finished properly.
                        if (team_info.winner)
                                UpdateCurrentMapFinishCount(1);

                        for (ent = g_edicts + 1; ent < g_edicts + 1 + game.maxclients; ent++) {
                                if (ent->inuse) {
                                        ent->client->ps.stats[STAT_LAYOUTS] = 0;
                                        ent->client->showscores = 0;
                                }
                        }
                        xmins_issued = false;
                        EndDMLevel();
                        BeginIntermission(NULL);
                } else if (level.intermissiontime + 10 == level.framenum) {
                        SendEndOfGameScoreboard ();
                        for (ent = g_edicts + 1; ent < g_edicts + 1 + game.maxclients; ent++) {
                                if (ent->inuse)
                                        ent->client->ps.stats[STAT_LAYOUTS] |= 1;
                        }
                } else if (level.intermissiontime + 110 == level.framenum) {
                        gi.bprintf (PRINT_HIGH, "Next map: %s\n", level.nextmap);
                } else if (level.intermissiontime + 120 == level.framenum) {
                        level.exitintermission = true;
                }

                return;
        }

        if (!deathmatch->value)
                return;

        if (!xmins_issued && xmins->value && level.framenum >= 10*60*xmins->value) {
                edict_t *spot = NULL;
                const char      *colors = colortext("***");
                gi.sound (world,CHAN_AUTO|CHAN_RELIABLE, SoundIndex (misc_talk1), 1, ATTN_NONE,0);
                gi.bprintf(PRINT_MEDIUM,"%s %g minutes is up! Starting spawns vulnerable! %s\n",colors, xmins->value, colors);

                while ((spot = G_Find (spot, FOFS(classname), "target_xmins")) != NULL) {
                        G_UseTargets (spot, spot);
                        if (spot)
                                G_FreeEdict (spot);
                }
                xmins_issued = true;
        }
}


void ResetVoteList (void)
{
        edict_t *ent;

        // clear votes
        for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++)
        {
                if (!ent->inuse)
                        continue;

                ent->client->resp.voted = VOTE_INVALID;
        }

        //reset vote clients
        vote.timer = 0;

        //r1: need to zero this or vote endmap or vote extend (or anything that doesn't use it)
        //    will ban the last player who had a vote targetted against them if they disconnect
        vote.target_index = 0;

        vote.starter = NULL;
        active_vote = false;
        
        vote.message = "";
        vote.action = NULL;
}

int CauseSuddenDeath (void)
{
        int alive = 0;
        edict_t *ent;
        
        if (team_info.timelimitmode) return 100; //Don't allow start sd when game is on rr, sd or ot mode already
        team_info.timelimitmode = 2; //set to SD for proper index on persistent stats

        for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++)
        {
                if (!ent->inuse)
                        continue;

                if (ent->health < 1)
                        continue;

                if (ent->client->resp.class_type == CLASS_ENGINEER || ent->client->resp.class_type == CLASS_BREEDER) {
                        T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
                } else {
                        alive++;

                        //r1: sudden death: everyone dies after 2 mins if they don't kill anyone
                        ent->client->sudden_death_frame = level.framenum + 1200;
                }

        }

        ent = NULL;

        for (ent = g_edicts + game.maxclients; ent < &g_edicts[globals.num_edicts]; ent++)
        {
                if (!ent->inuse)
                        continue;

                switch (ent->enttype)
                {
                        case ENT_TELEPORTER:
                        case ENT_COCOON:
                        case ENT_SPIKER:
                        case ENT_TURRET:
                        case ENT_MGTURRET:
                        case ENT_OBSTACLE:
                        case ENT_DETECTOR:
                        case ENT_AMMO_DEPOT:
                        case ENT_HEALER:
                        case ENT_TRIPWIRE_BOMB:
                        case ENT_GASSER:
                        case ENT_INFEST:
                                ent->hurtflags = 0;
                                ent->style = 0;
                                ent->flags = 0;
                                if (ent->enttype == ENT_TURRET || ent->enttype == ENT_MGTURRET)
                                  ent->target_ent->postthink = G_FreeEdict;
                                T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
                                if (ent->health > 0 || ent->takedamage)
                                        BecomeExplosion1 (ent);
                                break;
                }

                if (ent->classname && (!Q_stricmp (ent->classname, "trigger_repair") || !Q_stricmp (ent->classname, "trigger_healer")))
                        G_FreeEdict (ent);
        }

        level.suddendeath = true;

        return alive;
}



void InitTimelimit() //Want all timelimit stuff inside a single file
{
  team_info.timelimitmode = 0; //0=not yet hit. tie game is special case handled later.
  team_info.timelimit_nobody = 0;
  team_info.timelimit_count = 0;
  team_info.refund_fraction_a = 1.0f;
  team_info.refund_fraction_h = 1.0f;
  team_info.timelimit_time = 0;
}

const char* TimeLimitName(unsigned int mode, int maxlength)
{
  if (mode > 3) return "?";

  const char* longnames[4] = {
    "Tie game",
    "Reduced refunds",
    "Sudden death",
    "Overtime"
  };
  const char* shortnames[4] = {
    "tie",
    "rr",
    "sd",
    "ot"
  };
  
  if (maxlength < 16) {
    if (maxlength < 2) return "";

    return shortnames[mode];
  }

  return longnames[mode];
}





static qboolean reducedrefund_step()
{
  float t, a, h;
  int i;
  char* cfg = reducedrefundsteps->string;

  for (i = 0; i < team_info.timelimit_count; i++) {
    cfg = strstr(cfg, ",");
    if (!cfg) return false;
    cfg++;
  }
  team_info.timelimit_count++;

  t = strtof(cfg, &cfg);
  while (*cfg && *cfg < 32) cfg++;
  if (*cfg != ':') {
    gi.dprintf("reducedrefundsteps: expected \":\" in reducedrefundsteps before \"%s\"", cfg);
    return false;
  }
  cfg++;

  a = strtof(cfg, &cfg);
  while (*cfg && *cfg < 32) cfg++;
  if (*cfg == '/') {
    cfg++;
    h = strtof(cfg, &cfg);
  } else
    h = a;
    
  if (t < 0.5) {
    gi.dprintf("reducedrefundsteps: Invalid duration time");
    return false;
  }
  if (a < 0 || a > 1 || h < 0 || h > 1) {
    gi.dprintf("reducedrefundsteps: Invalid refund fraction (a=%.2f, h=%.2f). Must be between 0 and 1", a, h);
    return false;
  }

  if (team_info.refund_fraction_h != h || team_info.refund_fraction_a != a) {
    teamprintf(TEAM_NONE, PRINT_HIGH, "Reduced buildpoints: Refund rate is  %d%% for aliens, %d%% for humans.\n", (int)(a*100), (int)(h*100));
    teamcastSound(TEAM_NONE, SoundIndex(misc_secret), 1.0);
  }

  if (team_info.refund_fraction_h != h) {
    if (team_info.refund_fraction_h == 1 && h < 1)
      teamprintf(TEAM_HUMAN, PRINT_HIGH, "Entering reduced buildpoints mode, destroyed structures will refund %d%% of buildpoints.\n", (int)(h*100));
    else
    if (h > 0)
      teamprintf(TEAM_HUMAN, PRINT_HIGH, "Reduced buildpoints: Destroyed structures will now refund %d%% of buildpoints.\n",  (int)(h*100));
    else
      teamprintf(TEAM_HUMAN, PRINT_HIGH, "Reduced buildpoints: Destroyed structures wont refund buildpoints anymore\n");
    team_info.refund_fraction_h = h;
    teamcastSound(TEAM_HUMAN, SoundIndex(misc_secret), 1.0);
  }

  if (team_info.refund_fraction_a != a) {
    if (team_info.refund_fraction_a == 1 && a < 1)
      teamprintf(TEAM_ALIEN, PRINT_HIGH, "Entering reduced buildpoints mode, destroyed structures will refund %d%% of buildpoints.\n", (int)(a*100));
    else
    if (a > 0)
      teamprintf(TEAM_ALIEN, PRINT_HIGH, "Reduced buildpoints: Destroyed structures will now refund %d%% of buildpoints.\n",  (int)(a*100));
    else
      teamprintf(TEAM_ALIEN, PRINT_HIGH, "Reduced buildpoints: Destroyed structures wont refund buildpoints anymore\n");
    team_info.refund_fraction_a = a;
    teamcastSound(TEAM_HUMAN, SoundIndex(misc_secret), 1.0);
  }

  team_info.timelimit_next = level.time+t*50;
  
  return true;
}






void vote_extend (edict_t *ent);
void centerprint_all(char *text);
void Use_Target_Speaker (edict_t *ent, edict_t *other, edict_t *activator);
void CheckTeamRules (void)
{
        qboolean        humans_lost=false, aliens_lost=false;
        qboolean nobody = false;

        if (level.intermissiontime || !deathmatch->value)
                return;

        if (level.nospawns) {
          if (level.framenum % 600 == 10) {
            if (level.framenum < 100)
              centerprint_all("No initial spawns in map\nThis map would had been terminated on a dedicated server");
            else
              centerprint_all("No initial spawns in map");
          }
          if (team_info.spawns[TEAM_ALIEN] && team_info.spawns[TEAM_HUMAN]) {
            centerprint_all("Spawns available\nResuming normal game");
            level.nospawns = 0;
          }
          return;
        }

        //FIXME: move these to event-based? (eg on spawn kill, on player kill)
        if(team_info.spawns[TEAM_ALIEN] == 0 && countPlayers(TEAM_ALIEN) == 0)
                aliens_lost = true;

        if(team_info.spawns[TEAM_HUMAN] == 0 && countPlayers(TEAM_HUMAN) == 0)
                humans_lost = true;

        if (fraglimit->value && !aliens_lost && !humans_lost)
        {
                gclient_t       *cl;
                int     i, score1=0, score2=0;

                for (i=0 ; i<game.maxclients ; i++)
                {
                        if (!g_edicts[i+1].inuse)
                                continue;

                        cl = game.clients + i;

                        if (cl->resp.total_score < 0)
                                continue;

                        if(cl->resp.team == TEAM_ALIEN)
                                score1 += cl->resp.total_score;
                        else if(cl->resp.team == TEAM_HUMAN)
                                score2 += cl->resp.total_score;
                }

                if (score1 >= fraglimit->value)
                {
                        aliens_lost = true;
                        gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
                }

                if (score2 >= fraglimit->value)
                {
                        humans_lost = true;
                        gi.bprintf (PRINT_HIGH, "Fraglimit hit.\n");
                }
        }

        if ((int)timelimit->value > 0) {
          if (level.framenum + 3000 == (int)(timelimit->value*600))
            gi.sound (world, CHAN_AUTO, SoundIndex(misc_5min), 1.0, ATTN_NONE, 0);
          else
          if (level.framenum == (int)((timelimit->value - 5)*600))
            centerprint_all ("5 Minutes Remaining");
          else
          if (level.framenum + 1500 == (int)(timelimit->value*600)) {
            if ((int)voting->value & 64) {
              vote_extend (NULL);

              if (active_vote) {
                edict_t *vent;

                for (vent = g_edicts+1; vent < &g_edicts[game.maxclients+1]; vent++) {
                  if (!(vent->inuse && vent->client)) continue;
                  if (!vent->client->resp.team)
                    vent->client->resp.voted = VOTE_ABSTAIN;
                  else
                    vent->client->resp.voted = VOTE_INVALID;
                }

                vote.timer = level.framenum + (votetimer->value * 10);
                vote.type = VOTE_EXTEND;
                vote.starter = NULL;

                centerprint_all(va("Server started a vote to\n%s the timelimit\nYou have %i seconds to vote\nType vote yes/no in console to vote", colortext("extend"),(int)(vote.timer - level.framenum)/10));
                gi.sound(world, CHAN_AUTO, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
                gi.cprintf(NULL,PRINT_HIGH, "[vote] Server started a vote to extend the timelimit.\n");
              }
            }
          } else
          if (level.framenum == (int)((timelimit->value - 1)*600))
            centerprint_all("1 Minute Remaining");
          else
          if (level.framenum >= (int)(timelimit->value*600) && ((vote.type != VOTE_EXTEND) || (level.time > vote.timer*FRAMETIME + 1.0f))) {
            int i;
            nobody = true;
            for (i = 0; i < game.maxclients; i++)
              if (g_edicts[1+i].inuse && g_edicts[1+i].client->resp.team) {
                nobody = false;
                break;
              }
              
            if (nobody && team_info.timelimitmode) {
              if (team_info.timelimit_nobody < 50) { //Need 5 sec with teams empty to abort game
                team_info.timelimit_nobody++;
                nobody = false;
              } else
                team_info.timelimitmode = 0; //Force a tie game if nobody is playing
            } else
              team_info.timelimit_nobody = 0;
            
            if (!team_info.timelimitmode) { //Since tie game (also 0) won't get here a second time, its safe to use 0 as guard for timelimit not yet hit too
              if (!nobody) {
                team_info.timelimitmode = timelimitmode->value;
                if (!strcasecmp(timelimitmode->string, "rr") || !strcasecmp(timelimitmode->string, "reducedrefund") || !strcasecmp(timelimitmode->string, "reduced refund")) team_info.timelimitmode = 1;
                else
                if (!strcasecmp(timelimitmode->string, "sd") || !strcasecmp(timelimitmode->string, "suddendeath") || !strcasecmp(timelimitmode->string, "sudden death")) team_info.timelimitmode = 2;
                else
                if (!strcasecmp(timelimitmode->string, "ot") || !strcasecmp(timelimitmode->string, "overtime")) team_info.timelimitmode = 3;
              }

              team_info.timelimit_time = level.time;
              team_info.timelimit_nobody = nobody;

              if (team_info.timelimitmode == 1) {
                team_info.refund_fraction_a = team_info.refund_fraction_h = 1;
                if (reducedrefund_step())
                  centerprint_all ("Reduced buildpoints refund");
                else
                  humans_lost = aliens_lost = true;
              } else
              if (team_info.timelimitmode == 2) {
                team_info.timelimitmode = 0; //CauseSuddenDeath checks that is 0 and sets it by itself
                if (CauseSuddenDeath()) centerprint_all ("Sudden Death!");
                //No sound needed for this since you usually can hear a bang of all structures exploding/gibbing at the same time and centerprint is obnoxious enough
              } else
              if (team_info.timelimitmode == 3) {
                team_info.buildflags = 0xFFFFFFFFU;
                gi.bprintf(PRINT_HIGH, "Entering overtime mode, no further building is possible.\n");
                gi.sound(world, CHAN_AUTO, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
                centerprint_all ("Overtime");
              } else {
                gi.bprintf(PRINT_HIGH, "Timelimit hit.\n");
                humans_lost = aliens_lost = true;
                team_info.timelimitmode = 0; //keep its proper number even if defaulted here because bad value in cvar. We don't want out of bounds writes in gamepersistent->wins
              }
            } else
            if (team_info.timelimitmode == 1) { //reduced buildpoints special rules
              if (team_info.timelimit_next < level.time) {
                if (!reducedrefund_step())
                  humans_lost = aliens_lost = true;
              }
            }
          } //time limit hit
        } //(int)timelimit->value > 0

        // use_humanwin & use_alienwin uses this, UGLY!
        if (team_info.leveltimer > 0 && level.time > team_info.leveltimer*60.0)
        {
                if(team_info.playmode == MODE_HUMANWIN)
                        aliens_lost = true;
                else if(team_info.playmode == MODE_ALIENWIN)
                        humans_lost = true;
                else
                        humans_lost=aliens_lost=true;
        }

        //check we are actually allowed to win
        if (humans_lost && team_info.lost[TEAM_ALIEN])
                aliens_lost = true;
        else if (aliens_lost && team_info.lost[TEAM_HUMAN])
                humans_lost = true;

        if(humans_lost && aliens_lost)
        {
                edict_t *speaker = NULL;
                speaker=G_Find (NULL, FOFS(classname), "on_humans_win");

                if (speaker && speaker->message)
                        gi.bprintf(PRINT_HIGH, "%s", speaker->message);
                else
                if (!nobody)
                        gi.bprintf (PRINT_HIGH, "Tie game.\n");
                else
                        gi.bprintf (PRINT_HIGH, "Timelimit end.\n");

                team_info.winner = TEAM_NONE;

                level.intermissiontime = level.framenum + 10*10;

                //r1: abort votes, no point continuing them past this point.
                ResetVoteList();

        } else if(aliens_lost && !humans_lost)          // aliens lost
        {
                edict_t *speaker = NULL;
                speaker = G_Find (speaker, FOFS(classname), "target_speaker_h");
                if (speaker)
                        Use_Target_Speaker (speaker, speaker, speaker);
                else
                        gi.sound(world, CHAN_AUTO|CHAN_RELIABLE, SoundIndex (victory_human), 1, ATTN_NONE, 0);

                team_info.winner = TEAM_HUMAN;

                speaker=G_Find (NULL, FOFS(classname), "on_humans_win");

                if (speaker && speaker->message)
                        gi.bprintf(PRINT_HIGH, "%s", speaker->message);
                else
                        gi.bprintf (PRINT_HIGH, "%s Win!\n", team_info.teamnames[TEAM_HUMAN]);

                //r1: abort votes, no point continuing them past this point.
                ResetVoteList();

                level.intermissiontime = level.framenum + 10*10;

        } else if(humans_lost && !aliens_lost)                  // humans lost
        {
                edict_t *speaker = NULL;
                speaker = G_Find (speaker, FOFS(classname), "target_speaker_a");
                if (speaker)
                        Use_Target_Speaker (speaker, speaker, speaker);
                else
                        gi.sound(world, CHAN_AUTO|CHAN_RELIABLE, SoundIndex (victory_alien), 1, ATTN_NONE, 0);

                team_info.winner = TEAM_ALIEN;

                speaker=G_Find (NULL, FOFS(classname), "on_aliens_win");

                if (speaker && speaker->message)
                        gi.bprintf(PRINT_HIGH, "%s", speaker->message);
                else
                        gi.bprintf (PRINT_HIGH, "%s Win!\n", team_info.teamnames[TEAM_ALIEN]);

                //r1: abort votes, no point continuing them past this point.
                ResetVoteList();

                level.intermissiontime = level.framenum + 10*10;
        }
}

void ClientEndServerFrames (void);
void ExitLevel (void)
{
        char    command [256];

        //r1: detect maps in the map cycle which don't exist on the server - if this gets called
        //    more than once we assume the request to change map didn't work for some reason (eg
        //    map doesn't exist or something)

        if (changemap_spincount++ == 1) {
                char *map = MapnameByIndex(NextByIndex (currentmapindex, 0));
                if (!map) {
                        map = MapnameByIndex(NextByIndex (0, 0));
                        if (!map) {
                                gi.error ("ExitLevel: Changemap to '%s' failed, no other maps available.", level.nextmap);
                        }
                }
                RemoveFromMaplist (IndexByMapname (level.nextmap, 0));
                gi.bprintf (PRINT_HIGH, "WARNING: Request to change map to '%s' failed. Trying '%s'...\n", level.nextmap, map);
                strcpy (level.nextmap, map);
        }


#ifdef GMDPLUG
        gi.bprintf (PRINT_HIGH,"\nDon't have the next map '%s'?\nGet it at the Gloom Map Depository\n%s\n",level.nextmap,colortext("http://www.planetquake.com/gmd/"));
#endif
        Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.nextmap);
        gi.AddCommandString (command);
        //level.exitintermission = false;

        //r1: is this even needed?
        ClientEndServerFrames ();
}

/*void CheckMapCycle (void)
{
        int i;
        int players = 0;

        for (i = 5;i > 0;i--) {
                playertimes[i] = playertimes[i-1];
        }

        for (i=0 ; i<MAXTEAMS ; i++)
                players += team_info.numplayers[i];

        playertimes[0] = players;
}*/


void UpdateServerType()
{
  static char* types[2] = {"normal", "match"};
  int i;
  
  sv_type->modified = 0;
  
  for (i = 0; i < sizeof(types)/sizeof(types[0]); i++)
    if (!strcasecmp(sv_type->string, types[i])) {
      svtype = i;
      return;
    }
 

  gi.cvar_set("type", types[svtype]);
  sv_type->modified = 0;
}


void StepMiddleFrame()
{
 level.current_entity = g_edicts;
 do {
   if (level.current_entity->inuse)
     SV_RunThink(level.current_entity);
   level.current_entity++;
  } while (level.current_entity < g_edicts+globals.num_edicts);
}


/*
G_RunFrame

Advances the world by 0.1 seconds
*/
void CheckReconnectEnts (void);
void ProcessSpawnQueue (void);
void EXPORT G_RunFrame (void)
{
	int svfmask;

	if (gpmflags != oldgpmflags) {
          edict_t*b;
	  oldgpmflags = gpmflags;
	  gi.configstring(CS_AIRACCEL, va("%c:GPMF=%d", sv_airaccelerate->value?'1':'0', gpmflags));

          for (b = g_edicts+1; b <= g_edicts+game.maxclients; b++) if (b->health > 0)
            GloomPMoveInitState(PlayerGloomPMove(b, NULL), b->client->resp.class_type, gpmflags);
	}

        if (!(level.framenum&7)) {
          if (!level.framenum || teameven->modified) {
            teameven->modified = false;
            ProcessJoinQueues();
            ProcessSpawnQueues(0);
            UpdateServerType();
          }
          
          if (sv_type->modified)
            UpdateServerType();

          if (!level.framenum || sv_gpmflags->modified) {
            sv_gpmflags->modified = false;
            gpmflags = (int)sv_gpmflags->value;
            if (!level.framenum) oldgpmflags = gpmflags^1;
          }
          
          if (g_override_damages->modified) {
            g_override_damages->modified = false;
            LoadDamageOverrideTable();
          }
          
          if (g_debug->modified) {
            debug_n++;
            g_debug->modified = false;
          }
        
          if (!level.framenum || builderfragtimes->modified) {
            int i;
            char*list = builderfragtimes->string;
            for (i = 0; i < 10; i++) {
              builderfragtime[i] = strtof(list, &list)*60;
              while (*list && *list < 32) list++;
              if (*list && *list == ',') list++;
            }
            builderfragtimes->modified = false;
          }
        }

        if (game.paused)
                return;

        level.framenum++;
        level.time = level.framenum * FRAMETIME;
        
        // exit intermissions
        if (level.exitintermission)
        {
                ExitLevel ();
                return;
        }

        /*
         * treat each object in turn
         * even the world gets a chance to think
         */
        // some movement code uses level.current_entity for teammaster identifying
        level.current_entity = g_edicts;
        do
        {
                if (level.current_entity->inuse)
                {
                        VectorCopy (level.current_entity->s.origin, level.current_entity->s.old_origin);

                        // if the ground entity moved, make sure we are still on it
                        if ((level.current_entity->groundentity) && (level.current_entity->groundentity->linkcount != level.current_entity->groundentity_linkcount))
                        {
                                level.current_entity->groundentity = NULL;
                                if ( !(level.current_entity->flags & (FL_SWIM|FL_FLY)) && (level.current_entity->svflags & SVF_MONSTER) )
                                {
                                        M_CheckGround (level.current_entity);
                                }
                        }

                        if (level.current_entity->client) {
                           if (level.current_entity->prethink)
                             level.current_entity->prethink(level.current_entity);

                           ClientBeginServerFrame (level.current_entity);

                           if (level.current_entity->postthink)
                             level.current_entity->postthink(level.current_entity);
                        } else
                                G_RunEntity (level.current_entity);

			svfmask |= level.current_entity->svflags;
                }

                level.current_entity++;
        } while (level.current_entity < g_edicts+globals.num_edicts);
        
        if (svfmask & SVF_20FPS)
          SetNextMiddleFrame(50);
        else
          ClearNextMiddleFrame();

        // build the playerstate_t structures for all players
        ClientEndServerFrames ();

        /* misc stuff that doesn't belong to the actual game */
        CheckDMRules ();

        if (active_vote) {
                CheckVotes();
        }

        CheckTeamRules ();

        ProcessSpawnQueue ();

        // FIXME: set the uptime variable on initgame to time() and calculate the uptime from there
        if (level.framenum % 600 == 0)
        {
                int days = 0;
                int hours = 0;
                int mins = uptime + 1;

                /*static clock_t start, finish;
                double duration;

                if (!start)
                        start = clock();
                else {
                        finish = clock();
                        duration = (double)(finish - start) / CLOCKS_PER_SEC;
                        gi.dprintf ("%2.4f seconds\n", duration );
                        start = clock();
                }*/

                uptime++;

                CheckNeedPass ();
                CheckMotd (false);

                /*if (*reconnect_address->string)
                        CheckReconnectEnts();*/


                //unnecessary
                //CheckIPBanList();

                while (mins/60/24 >= 1) {
                        days++;
                        mins -= 60*24;
                }

                while (mins/60 >= 1) {
                        hours++;
                        mins -= 60;
                }

                gi.AddCommandString (va("set uptime \"%ddays, %dhrs, %dmins\" s\n",days, hours, mins));
        }

	//Hax all players to a grunt size for client prediction
        if (((int)dmflags->value & DF_STICK_WORKAROUND)) {
          int i;
          for (i = 1; i <= game.maxclients; i++) {
	    if (g_edicts[i].solid && g_edicts[i].client) {
              g_edicts[i].mins[0] = classlist[CLASS_GRUNT].mins[0];
              g_edicts[i].mins[1] = classlist[CLASS_GRUNT].mins[1];
              g_edicts[i].maxs[0] = classlist[CLASS_GRUNT].maxs[0];
              g_edicts[i].maxs[1] = classlist[CLASS_GRUNT].maxs[1];
              g_edicts[i].svflags |= 0x1000000;		//keep them marked so first ClientThink will restore all
              gi.linkentity(g_edicts+i);
            }
          }
        }
}

void think_cause_sd (edict_t *ent)
{
        ent->count--;

        if (level.intermissiontime)
        {
                G_FreeEdict (ent);
                return;
        }

        if (ent->count && ent->count <= 10)
                centerprint_all (va("%d\n", ent->count));

        if (ent->count == 30)
                centerprint_all ("Sudden Death in\n30 Seconds\n");

        if (ent->count == 15)
                centerprint_all ("Sudden Death in\n15 Seconds\n");

        if (!ent->count)
        {
                centerprint_all ("Sudden Death!\n");
                CauseSuddenDeath ();
                G_FreeEdict (ent);
                return;
        }

        ent->nextthink = level.time + 1;
}

void CheckVotes (void){
        edict_t *ent;
        int invalid = 0;
        int votes=0;
        unsigned int yes=0, no=0;

        if (active_vote && (level.framenum >= vote.timer)) {

                active_vote=false;
                vote.voters=0;

                for (ent = g_edicts+1; ent < &g_edicts[game.maxclients+1]; ent++)
                {
                        if (!(ent->inuse && ent->client && ent->client->pers.connected))
                                continue;

                        if (ent->client->resp.voted == VOTE_INVALID) {
                                if (vote.type == VOTE_TEAMKICK) {
                                        if (ent->client->resp.team == vote.starter->client->resp.team) {
                                                invalid++;
                                        }
                                } else {
                                        invalid++;
                                }
                        }


                        if (ent->client->resp.voted & VOTE_YES) {
                                yes++;
                        } else if (ent->client->resp.voted & VOTE_NO) {
                                if (vote.type == VOTE_TEAMKICK) {
                                        if (ent->client->resp.team == vote.starter->client->resp.team) {
                                                no++;
                                        }
                                } else {
                                        no++;
                                }
                        }

                        // veto right for gods, useful for preventing votes or kick people out
                        if (ent->client->resp.voted & VOTE_VETO) {

                                yes=no=0;
                                vote.voters=1;
                                invalid = 0;

                                if (ent->client->resp.voted & VOTE_YES)
                                        yes++;
                                else
                                        no++;

                                gi.bprintf(PRINT_HIGH, va("[vote] %s vetoed the vote.\n", ent->client->pers.netname));
                                break;
                        }

                        if (vote.type == VOTE_TEAMKICK) {
                                if (ent->client->resp.team == vote.starter->client->resp.team) {
                                        vote.voters++;
                                }
                        } else {
                                vote.voters++;
                        }

                }

                no += (invalid/2);
                votes = yes + no;

                //if (votes<vote.voters/2 || (vote.type==VOTE_NEXTMAP && votes<vote.voters/4)){
                //      gi.bprintf(PRINT_HIGH, "Voting is over, not enough people voted!\n");
                //} else {
                gi.bprintf(PRINT_HIGH, "[vote] Voting is over, %d voted yes, %d voted no.\n", yes, no);
                if (yes > no) {
                        if (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK) {
                                //gi.bprintf(PRINT_HIGH, "[vote] %s was kicked.\n", vote.ent->client->pers.netname);
                                ban(&g_edicts[vote.target_index],(int)banmaps->value, va("vote kick by %s", vote.starter->client->pers.netname), NULL);
                                kick(&g_edicts[vote.target_index]);
                        }else if (vote.type == VOTE_MUTE){
                                gi.bprintf(PRINT_HIGH, "[vote] Muting %s!\n", g_edicts[vote.target_index].client->pers.netname);
                                g_edicts[vote.target_index].client->pers.muted = true;
                                g_edicts[vote.target_index].client->pers.namechanges = 4; //no nick changing for j00.
                        }else if (vote.type == VOTE_MAP){
                                char *mapname=NULL;
                                mapname = MapnameByIndex (vote.target_index);
                                if (mapname) {
                                        gi.bprintf(PRINT_HIGH, "[vote] Changing the map to %s!\n", mapname);
                                        level.voted_map_change = true;
                                        strncpy(level.nextmap, mapname, sizeof(level.nextmap)-1);
                                        level.intermissiontime = level.framenum + 1;
                                } else {
                                        gi.bprintf(PRINT_HIGH, "[vote] Error finding map index %d in maplist!\n", vote.target_index);
                                }

                        }else if (vote.type == VOTE_NEXTMAP){
                                char *mapname=NULL;
                                mapname = MapnameByIndex (vote.target_index);
                                if (mapname) {
                                        gi.bprintf(PRINT_HIGH, "[vote] The next map will be %s!\n", mapname);
                                        level.voted_map_change = true;
                                        strncpy(level.nextmap, mapname, sizeof(level.nextmap)-1);
                                } else {
                                        gi.bprintf(PRINT_HIGH, "[vote] Error finding map index %d in maplist!\n", vote.target_index);
                                }
                        } else if (vote.type == VOTE_ENDMAP) {
                                if ((float)yes / (float)votes < 0.66) {
                                        gi.bprintf (PRINT_HIGH,"[vote] Need at least 66%% yes vote for end map.\n");
                                } else {
                                        gi.bprintf (PRINT_HIGH,"[vote] The map is aborted!\n");
                                        level.intermissiontime = level.framenum + 1;
                                        //r1: spam the finished count extra low so its not likely to come up again
                                        UpdateCurrentMapFinishCount (-5);
                                }
#ifdef VOTE_EXTEND
                        } else if (vote.type == VOTE_EXTEND) {
                                gi.bprintf (PRINT_HIGH, "[vote] The timelimit is extended by 30 minutes!\n");
                                //r1: moved oldtimelimit to spawnentities, or else we could end up with the 
                                //old timelimit being an already extended value.
                                gi.cvar_set ("timelimit", va("%d", (int)timelimit->value + 30));
#endif
                        } else if (vote.type == VOTE_SD) {
                                edict_t *spawner = G_Spawn ();
                                spawner->count = 60;
                                spawner->nextthink = level.time + 1;
                                spawner->think = think_cause_sd;
                                spawner->enttype = ENT_SD_CAUSER;
                                spawner->classname = "sdcauser";
                                spawner->svflags |= SVF_NOCLIENT;
                                gi.linkentity (spawner);
                                gi.bprintf (PRINT_HIGH, "[vote] Sudden Death in 60 seconds!!\n");
                                centerprint_all ("Sudden Death In\n60 Seconds!\n");
                        }
                } else {
                        if (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK) {
                                gi.bprintf(PRINT_HIGH, "[vote] %s gets to stay!\n", g_edicts[vote.target_index].client->pers.netname);
                        } else if (vote.type == VOTE_MAP) {
                                char *mapname=NULL;
                                mapname = MapnameByIndex (vote.target_index);
                                if (mapname) {
                                        gi.bprintf(PRINT_HIGH, "[vote] The map will not be changed to %s!\n", mapname);
                                        vote.lastmap_index[0] = vote.lastmap_index[1];
                                        vote.lastmap_index[1] = vote.target_index;
                                }
                        } else if (vote.type == VOTE_NEXTMAP) {
                                char *mapname=NULL;
                                mapname = MapnameByIndex (vote.target_index);
                                if (mapname) {
                                        gi.bprintf(PRINT_HIGH, "[vote] The next map will not be changed to %s!\n", mapname);
                                }
                        } else if (vote.type == VOTE_MUTE) {
                                gi.bprintf (PRINT_HIGH, "[vote] %s gets to annoy everyone!\n",g_edicts[vote.target_index].client->pers.netname);
                        } else if (vote.type == VOTE_ENDMAP) {
                                gi.bprintf (PRINT_HIGH, "[vote] The map continues!\n");
#ifdef VOTE_EXTEND
                        } else if (vote.type == VOTE_EXTEND) {
                                gi.bprintf (PRINT_HIGH, "[vote] The timelimit remains at %d minutes!\n", (int)timelimit->value);
#endif
                        } else if (vote.type == VOTE_SD) {
                                gi.bprintf (PRINT_HIGH, "[vote] The map continues!\n");
                        }
                        if (vote.starter)
                                vote.starter->client->resp.failed_votes++;
                }

                //r1ch: "dumb" votes get instant quota
                if (vote.starter)
                {
                        if (votes >= 5 && yes < 3) {
                                vote.starter->client->resp.failed_votes = FAILED_VOTES_LIMIT;
                        }

                        vote.starter->client->last_vote_time = level.time + 180;
                }

                ResetVoteList();

        } // votes<vote.voters<2 ...
}


qboolean PerPlayerData = 0;
//cvar_t   g_perplayerdata;

#ifdef __WIN32
__attribute__((dllexport))
#endif
__attribute__((visibility("default"))) void ext_WriteDeltaEntityFilter(entity_state_t *from, entity_state_t**to, int clientnum)
{
  static entity_state_t copy;
  
//  if (!g_perplayerdata->value) return;
  PerPlayerData = 1;

  if (clientnum >= 0 && clientnum < game.maxclients) {
    edict_t *player = g_edicts+clientnum+1;
    edict_t *ent = g_edicts+(*to)->number;
    
    if (ent->client) {
      if (ent->client->resp.team != player->client->resp.team) return;
      
      if (ent->client->teammodelindex || ent->client->teamoverlayindex) {
        copy = **to;

        if (ent->client->teammodelindex) {

          copy.renderfx |= ent->client->teamrf;
          copy.modelindex = ent->client->teammodelindex;
        }

        if (ent->client->teamoverlayindex)
          copy.modelindex4 = ent->client->teamoverlayindex;

        *to = &copy;
      }
      
      return;
    }
/* just for testing if this works
    if (ent->enttype == ENT_SPIKER) {
      copy = **to;
      copy.effects = 1;
      *to = &copy;
      return;
    }
*/
  }
}
