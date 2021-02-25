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

#include "g_local.h"
#include "g_wordfilter.h"
#include "persistent.h"
#include <stdio.h>

const field_t fields[] = {
        {"classname", FOFS(classname), F_LSTRING, 0},
        {"origin", FOFS(s.origin), F_VECTOR, 0},
        {"model", FOFS(model), F_LSTRING, 0},
        {"spawnflags", FOFS(spawnflags), F_INT, 0},
        {"speed", FOFS(speed), F_FLOAT, 0},
        {"accel", FOFS(accel), F_FLOAT, 0},
        {"decel", FOFS(decel), F_FLOAT, 0},
        {"target", FOFS(target), F_LSTRING, 0},
        {"targetname", FOFS(targetname), F_LSTRING, 0},
        {"pathtarget", FOFS(pathtarget), F_LSTRING, 0},
        {"deathtarget", FOFS(deathtarget), F_LSTRING, 0},
        {"killtarget", FOFS(killtarget), F_LSTRING, 0},
        {"combattarget", FOFS(combattarget), F_LSTRING, 0},
        {"message", FOFS(message), F_LSTRING, 0},
        {"team", FOFS(team), F_LSTRING, 0},
        {"wait", FOFS(wait), F_FLOAT, 0},
        {"delay", FOFS(delay), F_FLOAT, 0},
        {"random", FOFS(random), F_FLOAT, 0},
        {"move_origin", FOFS(move_origin), F_VECTOR, 0},
//      {"move_angles", FOFS(move_angles), F_VECTOR, 0},
        {"style", FOFS(style), F_INT, 0},
        {"count", FOFS(count), F_INT, 0},
        {"health", FOFS(health), F_INT, 0},
        {"sounds", FOFS(sounds), F_INT, 0},
        {"light", 0, F_IGNORE, 0},
        {"dmg", FOFS(dmg), F_INT, 0},
        {"angles", FOFS(s.angles), F_VECTOR, 0},
        {"angle", FOFS(s.angles), F_ANGLEHACK, 0},
        {"mass", FOFS(mass), F_INT, 0},
        {"volume", FOFS(volume), F_FLOAT, 0},
        {"attenuation", FOFS(attenuation), F_FLOAT, 0},
        {"map", FOFS(map), F_LSTRING, 0},

        // temp spawn vars -- only valid when the spawn function is called
        {"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
        {"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
        {"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
        {"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
        {"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
        {"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
        {"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
        {"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
        {"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
        {"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
        {"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
        {"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
        {"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
        {"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
        {"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},

        // Gloom world vars
        {"leveltimer", STOFS(leveltimer), F_FLOAT, FFL_SPAWNTEMP},
        {"playmode", STOFS(playmode), F_INT, FFL_SPAWNTEMP},
        {"classes", STOFS(classes), F_INT, FFL_SPAWNTEMP},
        {"spawnteam", STOFS(spawnteam), F_INT, FFL_SPAWNTEMP},
        {"hurtflags", STOFS(hurtflags), F_INT, FFL_SPAWNTEMP},
        {"buildflags", STOFS(buildflags), F_INT, FFL_SPAWNTEMP},
        {"alienstartfrags", STOFS(alienstartfrags), F_INT, FFL_SPAWNTEMP},
        {"humanstartfrags", STOFS(humanstartfrags), F_INT, FFL_SPAWNTEMP},
        {"noautowin", STOFS(noautowin), F_INT, FFL_SPAWNTEMP},
        {"lightscale", STOFS(lightscale), F_INT, FFL_SPAWNTEMP},

        {"chances", STOFS(chances), F_INT, FFL_SPAWNTEMP},

        {"model2", STOFS(model2), F_LSTRING, FFL_SPAWNTEMP},
        {"model3", STOFS(model3), F_LSTRING, FFL_SPAWNTEMP},

        //R1: DONT DELETE THIS !!!!
        {NULL, 0, F_INT, 0}
};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.
/*field_t               savefields[] =
{
        {"", FOFS(classname), F_LSTRING},
        {"", FOFS(target), F_LSTRING},
        {"", FOFS(targetname), F_LSTRING},
        {"", FOFS(killtarget), F_LSTRING},
        {"", FOFS(team), F_LSTRING},
        {"", FOFS(pathtarget), F_LSTRING},
        {"", FOFS(deathtarget), F_LSTRING},
        {"", FOFS(combattarget), F_LSTRING},
        {"", FOFS(model), F_LSTRING},
        {"", FOFS(map), F_LSTRING},
        {"", FOFS(message), F_LSTRING},

        {"", FOFS(client), F_CLIENT},
        {"", FOFS(item), F_ITEM},

        {"", FOFS(goalentity), F_EDICT},
        {"", FOFS(movetarget), F_EDICT},
        {"", FOFS(enemy), F_EDICT},
        {"", FOFS(oldenemy), F_EDICT},
        {"", FOFS(activator), F_EDICT},
        {"", FOFS(groundentity), F_EDICT},
        {"", FOFS(teamchain), F_EDICT},
        {"", FOFS(teammaster), F_EDICT},
        {"", FOFS(owner), F_EDICT},
//      {"", FOFS(mynoise), F_EDICT},
//      {"", FOFS(mynoise2), F_EDICT},
        {"", FOFS(target_ent), F_EDICT},
        {"", FOFS(chain), F_EDICT},

        {NULL, 0, F_INT}
};

field_t         levelfields[] =
{
        //{"", LLOFS(changemap), F_LSTRING},

//      {"", LLOFS(sight_client), F_EDICT},
//      {"", LLOFS(sight_entity), F_EDICT},
//      {"", LLOFS(sound_entity), F_EDICT},
//      {"", LLOFS(sound2_entity), F_EDICT},

        {NULL, 0, F_INT}
};

field_t         clientfields[] =
{
//      {"", CLOFS(pers.weapon), F_ITEM},
//      {"", CLOFS(pers.lastweapon), F_ITEM},
//      {"", CLOFS(newweapon), F_ITEM},

        {NULL, 0, F_INT}
};*/




cvar_t*blockerdetection;
cvar_t*timelimitmode;
cvar_t*summonflags;
cvar_t*g_changeclass;
cvar_t*reducedrefundsteps;

cvar_t*g_crouchfix;
cvar_t*g_allowed_uflags;
cvar_t*g_default_uflags;

extern cvar_t*builderfragtimes;

cvar_t*adminvoting;

cvar_t*msg_spawndeath_a;
cvar_t*msg_spawndeath_h;
cvar_t*msg_spawnkilled_a;
cvar_t*msg_spawnkilled_h;

cvar_t*g_grapple_delay;
cvar_t*g_grapple_jump_speed;
cvar_t*g_grapple_throw_speed;
cvar_t*g_grapple_pull_speed;
cvar_t*g_grapple_charge_time;

cvar_t*g_scoreboard;
cvar_t*g_enforce_names;

cvar_t*g_guardian_regen_mode;
cvar_t*g_guardian_regen_speed;

cvar_t*g_fire_mode;
cvar_t*g_hatchling_flags;
cvar_t*g_mech_rockets;
cvar_t*g_autoslash_delay;
cvar_t*g_exterm_pulse_overheat;
cvar_t*g_inven_refill_flags;
cvar_t*g_spiker_flags;
cvar_t*g_spiker_spike_time;
cvar_t*g_kamikaze_spike_time;

cvar_t*g_override_damages;

cvar_t*g_debug;

cvar_t*g_enforce_stick;

cvar_t  *g_grim_mech_rocket_text;

cvar_t*sv_type;
ServerType svtype = svt_normal;


//useraccount_t useraccount[MAX_USER_ACCOUNTS];
ipban_t ipfilters[MAX_IPFILTERS];

unsigned int totaldeaths = 0;
unsigned int uptime = 0;
unsigned int numipbans = 0;


char *GetLine (char **contents, int *len)
{
        int num;
        int i;
        static char line[256];

        num = 0;
        line[0] = '\0';

        if (*len <= 0)
                return NULL;

        for (i = 0; i < *len; i++) {
                if ((*contents)[i] == '\n') {
                        *contents += (num + 1);
                        *len -= (num + 1);
                        line[num] = '\0';
                        return line;
                } else {
                        line[num] = (*contents)[i];
                        num++;
                }
        }

        return line;
}


cvar_t*rcon_password;
cvar_t*syncplayers;
cvar_t*g_fragmode;


/*
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
*/

extern iplog_t *iplog;

void seedMT(unsigned long seed);
void OpenAdminLog (char *fname);
extern edict_t *g_spawn_ent;
extern unsigned g_spawn_max;
void EXPORT InitGame (void)
{
        int i = 0;
#ifdef ADMINDLLHACK
        FILE *admindll2, *admindll;
#endif

#ifdef ZZLIB
        char *contents;
        int len;
#else
        FILE *banfile;
#endif
        char *filename;

        gi.dprintf ("==== InitGame ====\n");
#ifdef ZZLIB
        gi.dprintf ("Uses ZZIP by Damien Debin <damien.debin@via.ecp.fr>\nhttp://www.zzip.f2s.com/\n");
#endif
        //srand( (unsigned)time( NULL ) );
        
        rcon_password = gi.cvar("rcon_password", "", 0); //needed by syncplayers.c to simulate a 25ms timer
        syncplayers = gi.cvar("syncplayers", "0", 0); //set to 1 to enable player 40FPS syncing, 0 for normal

        seedMT ((unsigned)time(NULL)+4357U);

        totaldeaths = 0;
        uptime = 0;

        // IMPORTANT NOTE ABOUT CVARS:
        // It seems that the order in which they are intialized IS important. Make sure you
        // have them all in the same order throughout g_save.c, g_local.h and g_main.c or
        // the server seems to randomly crash.

        //------------------------------------------------------------------------------------
        //                                               C O N S O L E   V A R I A B L E S                                      
        //------------------------------------------------------------------------------------
        //in-code name                  from server console name                        default         flags
        //maxentities =                 gi.cvar ("maxentities",                         "1024",         CVAR_LATCH);
        deathmatch =                    gi.cvar ("deathmatch",                          "1",            CVAR_LATCH);
        teameven =                              gi.cvar ("teameven",                            "1",            CVAR_DEFAULT);
#ifdef LPBEVEN
        teamlpbeven =                   gi.cvar ("teamlpbeven",                         "0",            CVAR_DEFAULT);
#endif
        buildercap =                    gi.cvar ("buildercap",                          "0",            CVAR_DEFAULT);
        forceteams =                    gi.cvar ("forceteams",                          "0",            CVAR_DEFAULT);
        reconnect_address =             gi.cvar ("reconnect_address",           "",                     CVAR_DEFAULT);

        max_rate =                              gi.cvar ("max_rate",                            "25000",        CVAR_SERVERINFO);
        max_cl_maxfps =                 gi.cvar ("max_cl_maxfps",                       "90",           CVAR_SERVERINFO);
        min_cl_maxfps =                 gi.cvar ("min_cl_maxfps",                       "10",           CVAR_DEFAULT);

        banmaps =                               gi.cvar ("bantime",                                     "3600",         CVAR_DEFAULT);

        autokick =                              gi.cvar ("autokick",                            "1",            CVAR_DEFAULT);
        tk_threshold =                  gi.cvar ("tk_threshold",                        "100",          CVAR_DEFAULT);

        teambonus =                             gi.cvar ("teambonus",                           "600",          CVAR_DEFAULT);
        replenishbp =                   gi.cvar ("replenishbp",                         "0",            CVAR_DEFAULT);
        replenishbp_amt =               gi.cvar ("replenishbp_amt",                     "1",            CVAR_DEFAULT);
        replenishbp_tick =              gi.cvar ("replenishbp_tick",            "2",            CVAR_DEFAULT);

        login =                                 gi.cvar ("login",                                       "0",            CVAR_DEFAULT);
        login_message =                 gi.cvar ("login_message",                       "",                     CVAR_DEFAULT);
        banaccountholders =             gi.cvar ("banaccountholders",           "0",            CVAR_DEFAULT);

        // cvar gamedir is actually the one sent to clients, game is servers own
        gamedir =                               gi.cvar ("game",                                        "gloom",        CVAR_SERVERINFO | CVAR_LATCH); // used for .ent loader --tumu

        xmins =                                 gi.cvar ("xmins",                                       "5",            CVAR_SERVERINFO);
        xmins_random =                  gi.cvar ("xmins_random",                        "0",            CVAR_DEFAULT);

        spiker_mode = gi.cvar ("spiker_mode", "0", 0);

        turret_mode = gi.cvar ("turret_mode", "0", 0);
        turret_damage = gi.cvar ("turret_damage", "100", 0);
        turret_speed = gi.cvar ("turret_speed", "1000", 0);
        turret_home = gi.cvar ("turret_home", "1", 0);
        turret_bfg = gi.cvar ("turret_bfg", "0", 0);
        turret_homingvelocityscale = gi.cvar ("turret_homingvelocityscale", "1.25", 0);
        turret_homingturnscale = gi.cvar ("turret_homingturnscale", "0.45", 0);
        turret_delay = gi.cvar ("turret_delay", "1.45", 0);

        spiker_regrow_time = gi.cvar ("spiker_regrow_time", "24", 0);
        spiker_max_spikes_per_client = gi.cvar ("spiker_max_spikes_per_client", "32", 0);
        spiker_spike_count = gi.cvar ("spiker_spike_count", "1", 0);
        spiker_damage = gi.cvar ("spiker_damage", "30", 0);
        spiker_damage_random = gi.cvar ("spiker_damage_random", "7", 0);
        spiker_speed = gi.cvar ("spiker_speed", "800", 0);
        spiker_distance = gi.cvar ("spiker_distance", "500", 0);

        spiketest = gi.cvar ("spiketest", "0", 0);

        shrapnel_damage = gi.cvar ("shrapnel_damage", "50", 0);
        shrapnel_count = gi.cvar ("shrapnel_count", "50", 0);

        strip_names = gi.cvar ("strip_names", "0", 0);
        strip_chat = gi.cvar ("strip_chat", "0", 0);

        idletime = gi.cvar ("idletime", "600", 0);
        obsidletime = gi.cvar ("obsidletime", "1200", 0);

        oldxmins = xmins->value;

        /*{

                int *x;
                x = ((void *)0x12345678);
        }*/

        default_maps =                  gi.cvar ("default_maps",                        "",             CVAR_LATCH);

        // misc filenames, make sure NOSET or careless set can cause files to be overwritten
        ipbans =                                gi.cvar ("ipbans",                                      "",             CVAR_NOSET);
        iplogs =                                gi.cvar ("iplogs",                                      "",             CVAR_NOSET);
        accounts =                              gi.cvar ("accounts",                            "",             CVAR_NOSET);
        if (!*accounts->string)
                gi.cprintf (NULL, PRINT_HIGH, "WARNING: useraccounts are disabled. please enable them by setting the 'accounts' cvar to the path to the accounts file (eg gloom/accounts.dat).\n");

        // FIXME: rename this to wordfilter
        wordbans =                              gi.cvar ("wordbans",                            "",             CVAR_NOSET);
        adminlog =                              gi.cvar ("adminlog",                            "",             CVAR_NOSET);
        gloomgamelog =                  gi.cvar ("gloomgamelog",                        "0",    CVAR_LATCH);

        voting =                                gi.cvar ("voting",                                      "511",          CVAR_SERVERINFO);
        adminvoting =                           gi.cvar ("adminvoting",                                 "511",          CVAR_DEFAULT);
        votetimer =                             gi.cvar ("votetimer",                           "30",           CVAR_DEFAULT);
        min_votes =                     gi.cvar ("min_votes",                           "0",            CVAR_DEFAULT);

        maxswitches =                   gi.cvar ("maxswitches",                         "2",            CVAR_DEFAULT);
        talk_threshold =                gi.cvar ("talk_threshold",                      "120",          CVAR_DEFAULT);
        nopublicchat =                  gi.cvar ("nopublicchat",                        "0",            CVAR_DEFAULT);
        noobserverchat =                gi.cvar ("noobserverchat",                      "0",            CVAR_DEFAULT);
        deathmessages =                 gi.cvar ("deathmessages",                       "1",            CVAR_DEFAULT);

        dmflags =                               gi.cvar ("dmflags",                                     "989855760",CVAR_SERVERINFO);
        fraglimit =                             gi.cvar ("fraglimit",                           "0",            CVAR_SERVERINFO);
        timelimit =                             gi.cvar ("timelimit",                           "0",            CVAR_SERVERINFO);
        reconnect_wait =                gi.cvar ("reconnect_wait",                      "20",           CVAR_DEFAULT);

        password =                              gi.cvar ("password",                            "",                     CVAR_USERINFO);
        needpass =                              gi.cvar ("needpass",                            "0",            CVAR_SERVERINFO);
        maxclients =                    gi.cvar ("maxclients",                          "12",           CVAR_SERVERINFO | CVAR_LATCH);
        maxplayers =                    gi.cvar ("maxplayers",                          maxclients->string,             CVAR_SERVERINFO);
        adminslots =                    gi.cvar ("adminslots",                          "0",            CVAR_DEFAULT);
        curplayers =                    gi.cvar ("curplayers",                          "0",            CVAR_SERVERINFO);
        g_select_empty =                gi.cvar ("g_select_empty",                      "0",            CVAR_ARCHIVE);
        dedicated =                             gi.cvar ("dedicated",                           "0",            CVAR_NOSET);

        bandwidth_mode =                gi.cvar ("bandwidth_mode",                      "0",            CVAR_DEFAULT);

        adminpassword =                 gi.cvar ("adminpassword",                       "",                     CVAR_DEFAULT);
        if (*adminpassword->string)
                gi.cprintf (NULL, PRINT_HIGH, "WARNING: adminpassword has been deprecated. please use the accounts function instead.\n");

        teamsplashdamage =              gi.cvar ("teamsplashdamage",            "1",            CVAR_DEFAULT);
        teamspawndamage =               gi.cvar ("teamspawndamage",                     "1",            CVAR_DEFAULT);
        teamarmordamage =               gi.cvar ("teamarmordamage",                     "0",            CVAR_DEFAULT);
        teamreflectarmordamage =gi.cvar ("teamreflectarmordamage",      "0",            CVAR_DEFAULT);
        teamreflectradiusdamage=gi.cvar ("teamreflectradiusdamage",     "0",            CVAR_DEFAULT);
        teamdamage =                    gi.cvar ("teamdamage",                          "0",            CVAR_DEFAULT);
        teamstructdamage =              gi.cvar ("teamstructdamage",                    "1",            CVAR_DEFAULT);
        flashlight_mode =               gi.cvar ("flashlight_color",            "0",            CVAR_DEFAULT);
        blockerdetection  =             gi.cvar("blockerdetection", "0", CVAR_DEFAULT);

        laser_color =                   gi.cvar ("laser_color",                         "0",            CVAR_DEFAULT);

        deathtest =                             gi.cvar ("deathtest",                           "0",            CVAR_DEFAULT);
//      speedtest =                             gi.cvar ("speedtest",                           "1",            CVAR_DEFAULT);
        basetest =                              gi.cvar ("basetest",                            "0",            CVAR_DEFAULT);
        playerclipfix =                 gi.cvar ("playerclipfix",                       "1",            CVAR_DEFAULT);
        turrettest =                    gi.cvar ("turrettest",                          "0",            CVAR_DEFAULT);
        nohtfragtest =                  gi.cvar ("nohtfragtest",                        "0",            CVAR_DEFAULT);
        pointstest =                    gi.cvar ("pointstest",                          "0",            CVAR_DEFAULT);
        firetest =                              gi.cvar ("firetest",                            "1",            CVAR_DEFAULT);
        mapdebugmode =                  gi.cvar ("mapdebugmode",                        "0",            CVAR_DEFAULT);

        // mech rockets off by default
        turretmode =                    gi.cvar ("turretmode",                          "1",            CVAR_DEFAULT);
        secure_modulate_cap =   gi.cvar ("secure_modulate_cap",         "1",            CVAR_DEFAULT);
        ceiling_eggs =                  gi.cvar ("ceiling_eggs",                        "0",            CVAR_DEFAULT);
        MAX_SCORE =                             gi.cvar ("max_frags",                           "10",           CVAR_DEFAULT);
        hide_spawns =                   gi.cvar ("hide_spawns",                         "1",            CVAR_DEFAULT);

        scoring_mode =                  gi.cvar ("scoring_mode",                        "0",            CVAR_DEFAULT);
        motd =                                  gi.cvar ("motd",                                        GLOOMVERSION,                   CVAR_DEFAULT);
        motd2 =                                 gi.cvar ("motd2",                                       "www.planetgloom.com",                  CVAR_DEFAULT);
        motd3 =                                 gi.cvar ("motd3",                                       "",                     CVAR_DEFAULT);

        sv_maxvelocity =                gi.cvar ("sv_maxvelocity",                      "2000",         CVAR_DEFAULT);
        sv_gravity =                    gi.cvar ("sv_gravity",                          "800",          CVAR_DEFAULT);
        sv_rollspeed =                  gi.cvar ("sv_rollspeed",                        "200",          CVAR_DEFAULT);
        sv_rollangle =                  gi.cvar ("sv_rollangle",                        "2",            CVAR_DEFAULT);

        gun_x =                                 gi.cvar ("gun_x",                                       "0",            CVAR_DEFAULT);
        gun_y =                                 gi.cvar ("gun_y",                                       "0",            CVAR_DEFAULT);
        gun_z =                                 gi.cvar ("gun_z",                                       "0",            CVAR_DEFAULT);

        run_pitch =                             gi.cvar ("run_pitch",                           "0.002",        CVAR_DEFAULT);
        run_roll =                              gi.cvar ("run_roll",                            "0.005",        CVAR_DEFAULT);

        bob_up  =                               gi.cvar ("bob_up",                                      "0.005",        CVAR_DEFAULT);
        bob_pitch =                             gi.cvar ("bob_pitch",                           "0.002",        CVAR_DEFAULT);
        bob_roll =                              gi.cvar ("bob_roll",                            "0.002",        CVAR_DEFAULT);

        sv_cheats =                             gi.cvar ("cheats",                                      "0",            CVAR_SERVERINFO);
        gloomcaps =                             gi.cvar ("gloomcaps",                           "1",            CVAR_DEFAULT | CVAR_LATCH);
        recoil =                                gi.cvar ("recoil",                                      "0",            CVAR_DEFAULT);
        randominvert =                          gi.cvar ("randominvert",                        "0",            CVAR_DEFAULT);

        timelimitmode =                         gi.cvar ("timelimitmode",                       "0",            CVAR_DEFAULT);

        tripwire_repair_count =                 gi.cvar ("tripwire_repair_count",               "0",            CVAR_DEFAULT);
        corrupted_tripwires =                   gi.cvar ("corrupted_tripwires",                 "0",            CVAR_DEFAULT);
        obstacle_frags =                        gi.cvar ("obstacle_frags",                      "1",            CVAR_DEFAULT);
        upgrades =                              gi.cvar ("upgrading",                           "1",            CVAR_DEFAULT);

        gi.cvar ("uptime", "0days, 0hrs, 0mins", CVAR_SERVERINFO);
        gi.cvar ("gloomversion", GLOOMVERSIONNO, CVAR_SERVERINFO | CVAR_NOSET);
        gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH); // latch in baseq2
        gi.dprintf ("%s %s %s\n",GAMEVERSION,GLOOMVERSIONNO, gamedate);
        gi.cvar ("gamedate", gamedate, CVAR_SERVERINFO | CVAR_LATCH); // latch in baseq2
        gi.cvar_set ("gamedate", gamedate);

        sv_updater = gi.cvar ("sv_updater",               "",            CVAR_NOSET);

        summonflags  =             gi.cvar("summonflags", "stceiTr", CVAR_DEFAULT);
        g_changeclass =       gi.cvar("g_changeclass", "0", CVAR_DEFAULT);
        
        builderfragtimes = gi.cvar("builderfragtimes", "5,5,5", CVAR_DEFAULT);
        reducedrefundsteps = gi.cvar("reducedrefundsteps", "15:0.75, 10:0.5, 5:0.25, 45:0", CVAR_DEFAULT);

        msg_spawndeath_a = gi.cvar("msg_spawndeath_a", "Spider spawn destroyed!", CVAR_DEFAULT);
        msg_spawndeath_h = gi.cvar("msg_spawndeath_h", "Human spawn destroyed!", CVAR_DEFAULT);
        msg_spawnkilled_a = gi.cvar("msg_spawnkilled_a", "Spider spawn destroyed by @!", CVAR_DEFAULT);
        msg_spawnkilled_h = gi.cvar("msg_spawnkilled_h", "Human spawn destroyed by @!", CVAR_DEFAULT);
        
        g_crouchfix = gi.cvar("g_crouchfix", "0", CVAR_DEFAULT);
        g_default_uflags = gi.cvar("g_default_uflags", "0", CVAR_DEFAULT);
        g_allowed_uflags = gi.cvar("g_allowed_uflags", "-1", CVAR_DEFAULT);

        g_override_damages = gi.cvar("g_override_damages", "", CVAR_DEFAULT);
        
        g_grapple_delay = gi.cvar ("g_grapple_delay", "0.1", CVAR_DEFAULT);
	g_grapple_jump_speed = gi.cvar ("g_grapple_jump_speed", "600", CVAR_DEFAULT);
	g_grapple_throw_speed = gi.cvar ("g_grapple_throw_speed", "750", CVAR_DEFAULT);
	g_grapple_pull_speed = gi.cvar ("g_grapple_pull_speed", "600", CVAR_DEFAULT);
	g_grapple_charge_time = gi.cvar ("g_grapple_charge_time", "1", CVAR_DEFAULT);
	
	g_scoreboard = gi.cvar("g_scoreboard", "0", CVAR_DEFAULT);
	g_enforce_names = gi.cvar("g_enforce_names", "0", CVAR_DEFAULT);

        g_guardian_regen_mode = gi.cvar ("g_guardian_regen_mode", "2", CVAR_DEFAULT);
        g_guardian_regen_speed = gi.cvar ("g_guardian_regen_speed", "1", CVAR_DEFAULT);

        g_fire_mode = gi.cvar ("g_fire_mode", "1", CVAR_DEFAULT);
        g_hatchling_flags = gi.cvar ("g_hatchling_flags", "1", CVAR_DEFAULT);
        g_mech_rockets = gi.cvar ("g_mech_rockets", "0", CVAR_DEFAULT);
        g_autoslash_delay = gi.cvar ("g_autoslash_delay", "0", CVAR_DEFAULT);
        g_autoslash_delay = gi.cvar ("g_autoslash_delay", "0", CVAR_DEFAULT);
        g_exterm_pulse_overheat = gi.cvar ("g_exterm_pulse_overheat", "21", CVAR_DEFAULT);
        g_inven_refill_flags = gi.cvar ("g_inven_refill_flags", va("%d", irf_default), CVAR_DEFAULT);
        g_spiker_flags = gi.cvar("g_spiker_flags", "0", CVAR_DEFAULT);
        g_enforce_stick = gi.cvar ("g_enforce_stick", "0", CVAR_DEFAULT);
        g_spiker_spike_time = gi.cvar("g_spiker_spike_time", "7-12", CVAR_DEFAULT);
        g_kamikaze_spike_time = gi.cvar("g_kamikaze_spike_time", "150-450", CVAR_DEFAULT);


        g_debug = gi.cvar ("g_debug", "", CVAR_DEFAULT); debug_n++;

        sv_type = gi.cvar ("type", "normal", CVAR_SERVERINFO);
        UpdateServerType();

        g_fragmode = gi.cvar("g_fragmode", va("%d", FM_CLASSIC), CVAR_DEFAULT);
        
        sv_airaccelerate = gi.cvar("sv_airaccelerate", "0", CVAR_DEFAULT);

        sv_gpmflags = gi.cvar("sv_gpmflags", "0", CVAR_DEFAULT);

        time(&game.serverstarttime);

        //generate random reconnect test
        for (i = 0; i < 5; i++)
                game.testmsg[i] = randomMT()%26+65;

        sv_extended = false;
        sv_testing = true;
        gi.AddCommandString("get test");
        
        if (sv_extended) {
            ext.SetClientMask = gi.AddCommandString("get SetClientMask");
            ext.GetClientMask = gi.AddCommandString("get GetClientMask");
            ext.SetEntityMask = gi.AddCommandString("get SetEntityMask");
            ext.GetEntityMask = gi.AddCommandString("get GetEntityMask");
            gi.dprintf("Gloom server extensions loaded succesfully.\n");
        }
        
        // items
        InitItems ();

        InitGamePersistent();

        for (i = 0; i <= 4;i++) {
                lastmaps[i][0] = '\0';
        }

        //r1: q2 internally caps at 1k
        /*if (max_rate->value < 1000)
                gi.error ("InitGame: max_rate < 1000");*/

        strncpy (game.port, gi.cvar("port","27910",0)->string, sizeof(game.port)-1);

        // init caches
        imagecache = gi.TagMalloc (MAX_IMAGE_CACHE * sizeof(int), TAG_GAME);
        soundcache = gi.TagMalloc (MAX_SOUND_CACHE * sizeof(int), TAG_GAME);

        // initialize all entities for this game
        globals.max_edicts = MAX_EDICTS;
        g_edicts =  gi.TagMalloc (MAX_EDICTS * sizeof(g_edicts[0]), TAG_GAME);
        globals.edicts = g_edicts;

        // initialize all clients for this game
        game.maxclients = maxclients->value;
        game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
        globals.num_edicts = game.maxclients+1;

        g_spawn_ent = g_edicts+game.maxclients; // points to last inuse ent

        if (accounts->string[0])
                LoadUserAccounts(accounts->string);

        iplog = gi.TagMalloc ((MAX_IP_LOG+1) * sizeof(iplog_t), TAG_GAME);

        if (iplogs->string[0])
                ReadIPLogs(iplogs->string);

        if (wordbans->string[0])
                ReadWordFilters(wordbans->string);

        if (adminlog->string[0])
                OpenAdminLog(adminlog->string);

        if (ipbans->string[0]) {
                filename = ipbans->string;

        #ifndef ZZLIB
        banfile = fopen (filename, "rb");
        if (banfile != NULL) {
                unsigned int expiretime;
                int ip1,ip2,ip3,ip4;
                char line[100];

                for (;;) {
                        line[0] = '\0';
                        if (fgets (line, 100, banfile)) {

                                line[strlen(line)-1] = '\0';

                                if (feof(banfile) || !line[0])
                                        break;

                                ip1 = atoi(Info_ValueForKey (line, "1"));
                                ip2 = atoi(Info_ValueForKey (line, "2"));
                                ip3 = atoi(Info_ValueForKey (line, "3"));
                                ip4 = atoi(Info_ValueForKey (line, "4"));
                                expiretime = atoi(Info_ValueForKey (line, "expire"));

                                //1\2\2\2\3\2\4\2\expire\989343943\reason\haha i own you\banner\r2\banned\r1


                                ipfilters[numipbans].ip[0] = ip1;
                                ipfilters[numipbans].ip[1] = ip2;
                                ipfilters[numipbans].ip[2] = ip3;
                                ipfilters[numipbans].ip[3] = ip4;
                                ipfilters[numipbans].expiretime = expiretime;
                                strncpy (ipfilters[numipbans].reason, Info_ValueForKey (line, "reason"), sizeof(ipfilters[numipbans].reason)-1);
                                strncpy (ipfilters[numipbans].banner, Info_ValueForKey (line, "banner"), sizeof(ipfilters[numipbans].banner)-1);

                                numipbans++;
                        }
                        if (feof(banfile))
                                break;
                }
                fclose (banfile);
                gi.dprintf ("InitGame: Read %d IP bans from %s\n",numipbans, filename);
        }
#else
        if (contents = GloomReadFile (filename, &len)) {
                unsigned int expiretime;
                int ip1,ip2,ip3,ip4;

                char *line;
                int num;
                int stat;
                int lines;
                int finallen;

                contents = decompress_block (contents, len, &finallen);

                if (!contents) {
                        gi.dprintf ("InitGame: decompression error reading IP bans.\n");
                } else {

                lines = 0;
                stat = 0;
                num = 0;

                while ((line = GetLine(&contents, &finallen)) != NULL) {
                        ip1 = atoi(Info_ValueForKey (line, "1"));
                        ip2 = atoi(Info_ValueForKey (line, "2"));
                        ip3 = atoi(Info_ValueForKey (line, "3"));
                        ip4 = atoi(Info_ValueForKey (line, "4"));
                        expiretime = atoi(Info_ValueForKey (line, "expire"));

                        ipfilters[numipbans].ip[0] = ip1;
                        ipfilters[numipbans].ip[1] = ip2;
                        ipfilters[numipbans].ip[2] = ip3;
                        ipfilters[numipbans].ip[3] = ip4;
                        ipfilters[numipbans].expiretime = expiretime;

                        strncpy (ipfilters[numipbans].reason, Info_ValueForKey (line, "reason"), sizeof(ipfilters[numipbans].reason)-1);
                        strncpy (ipfilters[numipbans].banner, Info_ValueForKey (line, "banner"), sizeof(ipfilters[numipbans].banner)-1);

                        numipbans++;
                }

                gi.dprintf ("InitGame: Read %d IP bans from %s\n",numipbans, filename);
                }
        }
#endif
        //check expired bans
        //done on level spawn
        //CheckIPBanList();
        }

        gi.cvar_set ("sv_enforcetime", "1");

        CheckMotd (true);
        InitMapCycles();
}

/*void WriteField1 (FILE *f, field_t *field, byte *base)
{
        void            *p;
        int                     len;
        int                     index;

        p = (void *)(base + field->ofs);
        switch (field->type)
        {
        case F_INT:
        case F_FLOAT:
        case F_ANGLEHACK:
        case F_VECTOR:
        case F_IGNORE:
                break;

        case F_LSTRING:
        case F_GSTRING:
                if ( *(char **)p )
                        len = strlen(*(char **)p) + 1;
                else
                        len = 0;
                *(int *)p = len;
                break;
        case F_EDICT:
                if ( *(edict_t **)p == NULL)
                        index = -1;
                else
                        index = *(edict_t **)p - g_edicts;
                *(int *)p = index;
                break;
        case F_CLIENT:
                if ( *(gclient_t **)p == NULL)
                        index = -1;
                else
                        index = *(gclient_t **)p - game.clients;
                *(int *)p = index;
                break;
        case F_ITEM:
                if ( *(edict_t **)p == NULL)
                        index = -1;
                else
                        index = *(gitem_t **)p - itemlist;
                *(int *)p = index;
                break;

        default:
                gi.error ("WriteEdict: unknown field type");
        }
}

void WriteField2 (FILE *f, field_t *field, byte *base)
{
        int                     len;
        void            *p;

        p = (void *)(base + field->ofs);
        switch (field->type)
        {
        case F_LSTRING:
        case F_GSTRING:
                if ( *(char **)p )
                {
                        len = strlen(*(char **)p) + 1;
                        fwrite (*(char **)p, len, 1, f);
                }
                break;
        }
}

void ReadField (FILE *f, field_t *field, byte *base)
{
        void            *p;
        int                     len;
        int                     index;

        p = (void *)(base + field->ofs);
        switch (field->type)
        {
        case F_INT:
        case F_FLOAT:
        case F_ANGLEHACK:
        case F_VECTOR:
        case F_IGNORE:
                break;

        case F_LSTRING:
                len = *(int *)p;
                if (!len)
                        *(char **)p = NULL;
                else
                {
                        *(char **)p = gi.TagMalloc (len, TAG_LEVEL);
                        fread (*(char **)p, len, 1, f);
                }
                break;
        case F_GSTRING:
                len = *(int *)p;
                if (!len)
                        *(char **)p = NULL;
                else
                {
                        *(char **)p = gi.TagMalloc (len, TAG_GAME);
                        fread (*(char **)p, len, 1, f);
                }
                break;
        case F_EDICT:
                index = *(int *)p;
                if ( index == -1 )
                        *(edict_t **)p = NULL;
                else
                        *(edict_t **)p = &g_edicts[index];
                break;
        case F_CLIENT:
                index = *(int *)p;
                if ( index == -1 )
                        *(gclient_t **)p = NULL;
                else
                        *(gclient_t **)p = &game.clients[index];
                break;
        case F_ITEM:
                index = *(int *)p;
                if ( index == -1 )
                        *(gitem_t **)p = NULL;
                else
                        *(gitem_t **)p = &itemlist[index];
                break;

        default:
                gi.error ("ReadEdict: unknown field type");
        }
}*/

/*
WriteClient

All pointer variables (except function pointers) must be handled specially.
*/
/*void WriteClient (FILE *f, gclient_t *client)
{
        field_t         *field;
        gclient_t       temp;
        
        // all of the ints, floats, and vectors stay as they are
        temp = *client;

        // change the pointers to lengths or indexes
        for (field=clientfields ; field->name ; field++)
        {
                WriteField1 (f, field, (byte *)&temp);
        }

        // write the block
        fwrite (&temp, sizeof(temp), 1, f);

        // now write any allocated data following the edict
        for (field=clientfields ; field->name ; field++)
        {
                WriteField2 (f, field, (byte *)client);
        }
}*/

/*
ReadClient

All pointer variables (except function pointers) must be handled specially.
*/
/*void ReadClient (FILE *f, gclient_t *client)
{
        field_t         *field;

        fread (client, sizeof(*client), 1, f);

        for (field=clientfields ; field->name ; field++)
        {
                ReadField (f, field, (byte *)client);
        }
}*/
/*
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
*/
/*void WriteGame (char *filename, qboolean autosave)
{
        FILE    *f;
        int             i;
        char    str[16];

        f = fopen (filename, "wb");
        if (!f)
                gi.error ("Couldn't open %s", filename);

        memset (str, 0, sizeof(str));
        strcpy (str, __DATE__);
        fwrite (str, sizeof(str), 1, f);

//      game.autosaved = autosave;
        fwrite (&game, sizeof(game), 1, f);
//      game.autosaved = false;

        for (i=0 ; i<game.maxclients ; i++)
                WriteClient (f, &game.clients[i]);

        fclose (f);
}

void ReadGame (char *filename)
{
        FILE    *f;
        int             i;
        char    str[16];

        gi.FreeTags (TAG_GAME);

        f = fopen (filename, "rb");
        if (!f)
                gi.error ("Couldn't open %s", filename);

        fread (str, sizeof(str), 1, f);
        if (strcmp (str, __DATE__))
        {
                fclose (f);
                gi.error ("Savegame from an older version.\n");
        }

        g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
        globals.edicts = g_edicts;

        fread (&game, sizeof(game), 1, f);
        game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
        for (i=0 ; i<game.maxclients ; i++)
                ReadClient (f, &game.clients[i]);

        fclose (f);
}*/

/*
WriteEdict

All pointer variables (except function pointers) must be handled specially.
*/
/*void WriteEdict (FILE *f, edict_t *ent)
{
        field_t         *field;
        edict_t         temp;

        // all of the ints, floats, and vectors stay as they are
        temp = *ent;

        // change the pointers to lengths or indexes
        for (field=savefields ; field->name ; field++)
        {
                WriteField1 (f, field, (byte *)&temp);
        }

        // write the block
        fwrite (&temp, sizeof(temp), 1, f);

        // now write any allocated data following the edict
        for (field=savefields ; field->name ; field++)
        {
                WriteField2 (f, field, (byte *)ent);
        }

}*/

/*
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
*/
/*void WriteLevelLocals (FILE *f)
{
        field_t         *field;
        level_locals_t          temp;

        // all of the ints, floats, and vectors stay as they are
        temp = level;

        // change the pointers to lengths or indexes
        for (field=levelfields ; field->name ; field++)
        {
                WriteField1 (f, field, (byte *)&temp);
        }

        // write the block
        fwrite (&temp, sizeof(temp), 1, f);

        // now write any allocated data following the edict
        for (field=levelfields ; field->name ; field++)
        {
                WriteField2 (f, field, (byte *)&level);
        }
}*/


/*
ReadEdict

All pointer variables (except function pointers) must be handled specially.
*/
/*void ReadEdict (FILE *f, edict_t *ent)
{
        field_t         *field;

        fread (ent, sizeof(*ent), 1, f);

        for (field=savefields ; field->name ; field++)
        {
                ReadField (f, field, (byte *)ent);
        }
}*/

/*
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
*/
/*void ReadLevelLocals (FILE *f)
{
        field_t         *field;

        fread (&level, sizeof(level), 1, f);

        for (field=levelfields ; field->name ; field++)
        {
                ReadField (f, field, (byte *)&level);
        }
}*/

/*
WriteLevel

*/
/*void WriteLevel (char *filename)
{
        int             i;
        edict_t *ent;
        FILE    *f;
        void    *base;

        f = fopen (filename, "wb");
        if (!f)
                gi.error ("Couldn't open %s", filename);

        // write out edict size for checking
        i = sizeof(edict_t);
        fwrite (&i, sizeof(i), 1, f);

        // write out a function pointer for checking
        base = (void *)InitGame;
        fwrite (&base, sizeof(base), 1, f);

        // write out level_locals_t
        WriteLevelLocals (f);

        // write out all the entities
        for (i=0 ; i<globals.num_edicts ; i++)
        {
                ent = &g_edicts[i];
                if (!ent->inuse)
                        continue;
                fwrite (&i, sizeof(i), 1, f);
                WriteEdict (f, ent);
        }
        i = -1;
        fwrite (&i, sizeof(i), 1, f);

        fclose (f);
}*/

/*
ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
*/
/*void ReadLevel (char *filename)
{
        int             entnum;
        FILE    *f;
        int             i;
        void    *base;
        edict_t *ent;

        f = fopen (filename, "rb");
        if (!f)
                gi.error ("Couldn't open %s", filename);

        // free any dynamic memory allocated by loading the level
        // base state
        gi.FreeTags (TAG_LEVEL);

        // wipe all the entities
        memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
        globals.num_edicts = game.maxclients+1;

        // check edict size
        fread (&i, sizeof(i), 1, f);
        if (i != sizeof(edict_t))
        {
                fclose (f);
                gi.error ("ReadLevel: mismatched edict size");
        }

        // check function pointer base address
        fread (&base, sizeof(base), 1, f);
        if (base != (void *)InitGame)
        {
                fclose (f);
                gi.error ("ReadLevel: function pointers have moved");
        }

        // load the level locals
        ReadLevelLocals (f);

        // load all the entities
        while (1)
        {
                if (fread (&entnum, sizeof(entnum), 1, f) != 1)
                {
                        fclose (f);
                        gi.error ("ReadLevel: failed to read entnum");
                }
                if (entnum == -1)
                        break;
                if (entnum >= globals.num_edicts)
                        globals.num_edicts = entnum+1;

                ent = &g_edicts[entnum];
                ReadEdict (f, ent);

                // let the server rebuild world links for this ent
                memset (&ent->area, 0, sizeof(ent->area));
                gi.linkentity (ent);
        }

        fclose (f);

        // mark all clients as unconnected
        for (i=0 ; i<game.maxclients ; i++)
        {
                ent = &g_edicts[i+1];
                ent->client = game.clients + i;
                ent->client->pers.ingame = false;
                ent->client->resp.team = 0;
                ent->client->pers.connected = false;
        }

        

        // do any load time things at this point
        for (i=0 ; i<globals.num_edicts ; i++)
        {
                ent = &g_edicts[i];

                if (!ent->inuse)
                        continue;

                // fire any cross-level triggers
                if (ent->classname)
                        if (strcmp(ent->classname, "target_crosslevel_target") == 0)
                                ent->nextthink = level.time + ent->delay;
        }
}*/
