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
#include <stdio.h>
#include "g_map.h"
#include "g_log.h"

#ifdef ZZLIB
        #include "zzlib/zzlib.h"
        #include <sys/stat.h>
#endif

void breakinSetDefaults(void);

typedef struct
{
        char    *name;
        void    (*spawn)(edict_t *ent);
} spawn_t;

extern int changemap_spincount;

// gloom
void SP_monster_cocoon (edict_t *self);
void SP_monster_spiker (edict_t *self);
void SP_monster_healer (edict_t *self);
void SP_monster_obstacle (edict_t *self);
void SP_monster_gasser (edict_t *self);

void SP_turret (edict_t *self);
void SP_detector (edict_t *self);
void SP_ammo_depot (edict_t     *self);
void SP_tripwire (edict_t *self);

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);

void SP_func_and (edict_t *self);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_train (edict_t *ent);
void SP_func_conveyor (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_clock (edict_t *ent);
void SP_func_countdown (edict_t *self);
void SP_func_killbox (edict_t *ent);

/*      gloom   */
void SP_on_win(edict_t *ent);

void SP_trigger_warp (edict_t *self);
void SP_trigger_teleport (edict_t *self);
void SP_trigger_healer (edict_t *self);
void SP_trigger_repair (edict_t *self);
void SP_trigger_always (edict_t *ent);
void SP_trigger_once (edict_t *ent);
void SP_trigger_blocker (edict_t *self);
void SP_trigger_multiple (edict_t *ent);
void SP_trigger_multiple_counter (edict_t *ent);
void SP_trigger_multistate_relay (edict_t *ent);
void SP_trigger_relay (edict_t *ent);
void SP_trigger_push (edict_t *ent);
void SP_trigger_hurt (edict_t *ent);
void SP_trigger_hurt_humans (edict_t *ent);
void SP_trigger_hurt_aliens (edict_t *ent);
void SP_trigger_key (edict_t *ent);
void SP_trigger_counter (edict_t *ent);
void SP_trigger_elevator (edict_t *ent);
void SP_trigger_gravity (edict_t *ent);
void SP_trigger_fog (edict_t *ent);
void SP_trigger_random (edict_t *ent);
void SP_target_humans_win (edict_t *ent);
void SP_target_aliens_win (edict_t *ent);
void SP_target_kill(edict_t *self);

void SP_target_key (edict_t *self);
void SP_target_random (edict_t *self);

void SP_trigger_keydrop (edict_t *self);

void SP_target_setclasses (edict_t *ent);
void SP_target_broadcast (edict_t *self);
void SP_target_humans_lose (edict_t *self);
void SP_target_aliens_lose (edict_t *self);
void SP_target_toggleclass (edict_t *ent);
void SP_target_reconnectwait (edict_t *ent);
void SP_target_xmins (edict_t *self);
void SP_target_temp_entity (edict_t *ent);
void SP_target_speaker (edict_t *ent);
void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_splash (edict_t *ent);
void SP_target_spawner (edict_t *ent);
void SP_target_cloner (edict_t *ent);
void SP_target_blaster (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_laser (edict_t *self);
void SP_target_actor (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);
void SP_target_character (edict_t *ent);
void SP_target_string (edict_t *ent);

void SP_worldspawn (edict_t *ent);

void SP_light (edict_t *self);
void SP_light_mine1 (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);

void SP_turret_base (edict_t *self); 
void SP_turret_breach (edict_t *self);

void SP_misc_deathball (edict_t *ent);
void SP_misc_explobox (edict_t *self);
void SP_misc_banner (edict_t *self);
void SP_misc_satellite_dish (edict_t *self);
void SP_misc_dengine (edict_t *self);
void SP_misc_actor (edict_t *self);
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
void SP_misc_insane (edict_t *self);
void SP_misc_deadsoldier (edict_t *self);
void SP_misc_viper (edict_t *self);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_bigviper (edict_t *self);
void SP_misc_strogg_ship (edict_t *self);
void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void SP_misc_blackhole (edict_t *self);
void SP_misc_eastertank (edict_t *self);
void SP_misc_easterchick (edict_t *self);
void SP_misc_easterchick2 (edict_t *self);

void SP_target_buildpoints (edict_t *self);

void SP_misc_infestation (edict_t *ent);
void SP_misc_jumppad (edict_t *ent);
void SP_misc_md2(edict_t *ent);

void SP_practice_target(edict_t *ent);

spawn_t spawns[] = {

        {"info_player_start", SP_info_player_start},
        {"info_player_deathmatch", SP_info_player_deathmatch},
        {"info_player_intermission", SP_info_player_intermission},
        {"info_player_intermission_a", SP_info_player_intermission},
        {"info_player_intermission_h", SP_info_player_intermission},

        {"func_plat", SP_func_plat},
        {"func_button", SP_func_button},
        {"func_door", SP_func_door},
        {"func_door_secret", SP_func_door_secret},
        {"func_door_rotating", SP_func_door_rotating},
        {"func_rotating", SP_func_rotating},
        {"func_train", SP_func_train},
        {"func_water", SP_func_water},
        {"func_conveyor", SP_func_conveyor},
        {"func_areaportal", SP_func_areaportal},
        {"func_clock", SP_func_clock},
        {"func_countdown", SP_func_countdown},
        {"func_wall", SP_func_wall},
        {"func_object", SP_func_object},
        {"func_timer", SP_func_timer},
        {"func_explosive", SP_func_explosive},
        {"func_killbox", SP_func_killbox},

        /* gloom */
        {"on_humans_win",SP_on_win},
        {"on_aliens_win",SP_on_win},
        {"on_tie_game",SP_on_win},

        {"func_and", SP_func_and},

        {"trigger_warp", SP_trigger_warp},
        {"trigger_teleport", SP_trigger_teleport},
        {"trigger_healer", SP_trigger_healer},
        {"trigger_repair", SP_trigger_repair},
        {"trigger_always", SP_trigger_always},
        {"trigger_once", SP_trigger_once},
        {"trigger_blocker", SP_trigger_blocker},
        {"trigger_multiple", SP_trigger_multiple},
        {"trigger_multiple_count", SP_trigger_multiple_counter},
        {"trigger_relay", SP_trigger_relay},
        {"trigger_multistate_relay", SP_trigger_multistate_relay},
        {"trigger_push", SP_trigger_push},
        {"trigger_hurt", SP_trigger_hurt},
        {"trigger_hurt_humans", SP_trigger_hurt_humans},
        {"trigger_hurt_aliens", SP_trigger_hurt_aliens},
        {"trigger_key", SP_trigger_key}, //deprecated
        {"target_key", SP_target_key},

        {"trigger_counter", SP_trigger_counter},
        {"trigger_elevator", SP_trigger_elevator},
        {"trigger_gravity", SP_trigger_gravity},
        /* gloom */
        {"trigger_keydrop", SP_trigger_keydrop},
        {"trigger_fog", SP_trigger_fog},
        {"trigger_random", SP_trigger_random},  //deprecated
        {"target_random", SP_target_random},
        {"target_aliens_win", SP_target_aliens_win},
        {"target_humans_win", SP_target_humans_win},
        {"target_kill", SP_target_kill},
        {"target_buildpoints", SP_target_buildpoints},
        {"target_broadcast", SP_target_broadcast},
        {"target_setclasses", SP_target_setclasses},

        {"target_speaker_h", SP_target_speaker},
        {"target_speaker_a", SP_target_speaker},
        {"target_humans_lose", SP_target_humans_lose},
        {"target_aliens_lose", SP_target_aliens_lose},
        {"target_toggleclass", SP_target_toggleclass},
        {"target_reconnectwait", SP_target_reconnectwait},
        {"target_xmins", SP_target_xmins},
        {"target_temp_entity", SP_target_temp_entity},
        {"target_speaker", SP_target_speaker},
        {"target_explosion", SP_target_explosion},
        {"target_changelevel", SP_target_changelevel},
        {"target_splash", SP_target_splash},
        {"target_spawner", SP_target_spawner},
        {"target_cloner", SP_target_cloner},
        {"target_blaster", SP_target_blaster},
        {"target_crosslevel_trigger", SP_target_crosslevel_trigger},
        {"target_crosslevel_target", SP_target_crosslevel_target},
        {"target_laser", SP_target_laser},
        {"target_lightramp", SP_target_lightramp},
        {"target_earthquake", SP_target_earthquake},
        {"target_character", SP_target_character},
        {"target_string", SP_target_string},

        {"worldspawn", SP_worldspawn},

        {"light", SP_light},
        {"light_mine1", SP_light_mine1},
        {"light_mine2", SP_light_mine2},
        {"info_null", SP_info_null},
        {"func_group", SP_info_null},
        {"info_notnull", SP_info_notnull},
        {"path_corner", SP_path_corner},
        //{"point_combat", SP_point_combat},

        {"turret_base", SP_turret_base},
        {"turret_breach", SP_turret_breach},

        {"misc_deathball", SP_misc_deathball},
        {"misc_explobox", SP_misc_explobox},
        {"misc_banner", SP_misc_banner},
        {"misc_satellite_dish", SP_misc_satellite_dish},
        {"misc_dengine", SP_misc_dengine},
        {"misc_gib_arm", SP_misc_gib_arm},
        {"misc_gib_leg", SP_misc_gib_leg},
        {"misc_gib_head", SP_misc_gib_head},
        {"misc_deadsoldier", SP_misc_deadsoldier},
        {"misc_viper", SP_misc_viper},
        {"misc_viper_bomb", SP_misc_viper_bomb},
        {"misc_bigviper", SP_misc_bigviper},
        {"misc_strogg_ship", SP_misc_strogg_ship},
        {"misc_teleporter", SP_misc_teleporter},
        {"misc_teleporter_dest", SP_misc_teleporter_dest},
        {"misc_blackhole", SP_misc_blackhole},
        {"misc_eastertank", SP_misc_eastertank},
        {"misc_easterchick", SP_misc_easterchick},
        {"misc_easterchick2", SP_misc_easterchick2},

        // Gloom
        {"monster_cocoon", SP_monster_cocoon},
        {"monster_healer", SP_monster_healer},
        {"monster_spiker", SP_monster_spiker},
        {"monster_obstacle", SP_monster_obstacle},
        {"monster_gasser", SP_monster_gasser},

        /* gloom */
        {"misc_jumppad", SP_misc_jumppad},
        {"misc_infestation",SP_misc_infestation},

        {"misc_md2",SP_misc_md2},

        {"misc_turret", SP_turret},
        {"misc_detector", SP_detector},
        {"misc_ammo_depot",SP_ammo_depot},
        {"misc_tripwire", SP_tripwire},

        {"fun_ball", SP_fun_ball},
        {"fun_goal", SP_fun_goal},
        {"fun_digit", SP_fun_digit},

        {"practice_target", SP_practice_target},

        {NULL, NULL}
};

qboolean isSpecialEntityName (char *name)
{
        spawn_t *s;
        for (s=spawns ; s->name ; s++) {
                if (!Q_stricmp(s->name, name)) {
                        return true;
                }
        }
        return false;
}

/*
ED_CallSpawn

Finds the spawn function for the entity and calls it
*/
void ED_CallSpawn (edict_t *ent)
{
        spawn_t *s;
        const gitem_t   *item;
        int             i;

        if (!ent->classname)
        {
                gi.dprintf ("REMOVED: Entity without a classname at %s\n",vtos2(ent));
                G_FreeEdict (ent);
                return;
        }
        
        if (st.chances > 0  && st.chances < 100) {
          if (rand()%100 + st.chances < 100) {
            if (mapdebugmode->value)
              gi.dprintf ("REMOVED: %s with %d chances\n", ent->classname, st.chances);
            G_FreeEdict(ent);
            return;
          }
        }

        // check item spawn functions
        for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
        {
                if (!item->classname)
                        continue;
                if (!strcmp(item->classname, ent->classname))
                {       // found it
                        SpawnItem (ent, item);
                        return;
                }
        }

        // check normal spawn functions
        for (s=spawns ; s->name ; s++)
        {
                if (!strcmp(s->name, ent->classname))
                {       // found it
                        s->spawn (ent);
                        if (ent->inuse && mapdebugmode->value)
                                gi.dprintf ("SPAWNED: %s, target %s, targetname %s\n", ent->classname, ent->target, ent->targetname);
                        return;
                }
        }

        gi.dprintf ("REMOVED: %s doesn't have a spawn function at %s\n", ent->classname, vtos2(ent));
        G_FreeEdict (ent);
}

void CheckSolid (edict_t *ent)
{
        trace_t tr;

        if (!ent->inuse || level.time > 0)
                return;

        tr = gi.trace (ent->s.origin,ent->mins,ent->maxs,ent->s.origin,ent,MASK_SHOT);
        if (tr.fraction != 1.0 || tr.allsolid)
        {
                gi.dprintf ("WARNING: %s clips %s solid at %s\n",ent->classname, tr.ent->classname, vtos2(ent));
        }
}

/*
ED_NewString
*/
char *ED_NewString (char *string)
{
        char    *newb, *new_p;
        int             i;
        size_t  l;

        l = strlen(string) + 1;

        newb = gi.TagMalloc ((int)l, TAG_LEVEL);

        new_p = newb;

        for (i=0 ; i< l ; i++)
        {
                if (string[i] == '\\' && i < l-1)
                {
                        i++;
                        if (string[i] == 'n')
                                *new_p++ = '\n';
                        else
                                *new_p++ = '\\';
                }
                else
                        *new_p++ = string[i];
        }

        return newb;
}




/*
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
*/
void ED_ParseField (char *key, char *value, edict_t *ent)
{
        const field_t   *f;
        byte    *b;
        float   v;
        vec3_t  vec;

        if (!Q_stricmp("nosie", key))   //hack for gloom4
                key = "noise";          

        for (f=fields ; f->name ; f++)
        {
                if (!Q_stricmp(f->name, key))
                {       // found it
                        if (f->flags & FFL_SPAWNTEMP)
                                b = (byte *)&st;
                        else
                                b = (byte *)ent;

                        switch (f->type)
                        {
                        case F_LSTRING:
                                *(char **)(b+f->ofs) = ED_NewString (value);
                                break;
                        case F_VECTOR:
                                sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
                                ((float *)(b+f->ofs))[0] = vec[0];
                                ((float *)(b+f->ofs))[1] = vec[1];
                                ((float *)(b+f->ofs))[2] = vec[2];
                                break;
                        case F_INT:
                                *(int *)(b+f->ofs) = atoi(value);
                                break;
                        case F_FLOAT:
                                *(float *)(b+f->ofs) = atof(value);
                                break;
                        case F_ANGLEHACK:
                                v = atof(value);
                                ((float *)(b+f->ofs))[0] = 0;
                                ((float *)(b+f->ofs))[1] = v;
                                ((float *)(b+f->ofs))[2] = 0;
                                break;
                        case F_IGNORE:
                                break;
                        default:
                                break;
                        }
                        return;
                }
        }
        gi.dprintf ("WARNING: Unknown field '%s' on %s at %s\n", key, ent->classname, vtos2(ent));
}

/*
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ent should be a properly initialized empty edict.
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
        qboolean        init;
        char            keyname[256];
        char            *com_token;

        init = false;
        memset (&st, 0, sizeof(st));

// go through all the dictionary pairs
        for (;;)
        {
        // parse key
                com_token = COM_Parse (&data);
                if (com_token[0] == '}')
                        break;
                if (!data)
                        gi.error ("ED_ParseEntity: EOF without closing brace");

                strncpy (keyname, com_token, sizeof(keyname)-1);

        // parse value
                com_token = COM_Parse (&data);
                if (!data)
                        gi.error ("ED_ParseEntity: EOF without closing brace");

                if (com_token[0] == '}')
                        gi.error ("ED_ParseEntity: closing brace without data");

                init = true;

        // keynames with a leading underscore are used for utility comments,
        // and are immediately discarded by quake
                if (keyname[0] == '_')
                        continue;

                ED_ParseField (keyname, com_token, ent);
        }

        if (!init)
                memset (ent, 0, sizeof(*ent));

        return data;
}


/*
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
*/
void G_FindTeams (void)
{
        edict_t *e, *e2, *chain;
        int             i, j;
        int             c, c2;

        c = 0;
        c2 = 0;
        for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
        {
                if (!e->inuse)
                        continue;
                if (!e->team)
                        continue;
                if (e->flags & FL_TEAMSLAVE)
                        continue;
                chain = e;
                e->teammaster = e;
                c++;
                c2++;
                for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
                {
                        if (!e2->inuse)
                                continue;
                        if (!e2->team)
                                continue;
                        if (e2->flags & FL_TEAMSLAVE)
                                continue;
                        if (!strcmp(e->team, e2->team))
                        {
                                c2++;
                                chain->teamchain = e2;
                                e2->teammaster = e;
                                chain = e2;
                                e2->flags |= FL_TEAMSLAVE;
                        }
                }
        }

        gi.dprintf ("%i teams with %i entities\n", c, c2);
}

// reads entities from <game>/maps/<mapname>.ent
// returns pointer to allocated memory or zero if aborted
char * ReadEntFile (char *mapname) {

        size_t  readlen;
        int filelen ;
        FILE *fileptr;
        char *entptr, *filename;


        // do check that gamedir (game) is default set to "gloom" (CVAR_SERVERINFO|CVAR_LATCHED) in g_save.c/InitGame.
        filename = va("%s/maps/%s.ent", gamedir->string, mapname);

        // try opening the entity file
        fileptr = fopen ( filename, "r");

        // if file didn't open, don't do anything more
        if (fileptr != NULL) {

                gi.dprintf("ReadEntFile: opening entities from %s.\n", filename);

                // find out filelength by seeking to end
                fseek ( fileptr, 0, SEEK_END );
                filelen = ftell( fileptr );

                // alloc memory if file has some length
                if (filelen > 0) {
                        entptr = gi.TagMalloc (filelen + 1, TAG_LEVEL);
                        gi.dprintf ("ReadEntFile: allocated %d bytes.\n", filelen+1);
                } else {
                        gi.dprintf ("ReadEntFile: could not read file, aborted.\n");
                        fclose (fileptr);
                        return 0;
                }

                // check that we got memory
                
                //r1: not necessary - tagmalloc will error out in quake2 exe if malloc() fails.
                /*if (!entptr) {
                        gi.dprintf ("ReadEntFile: could not allocate memory, aborted.\n");
                        fclose (fileptr);
                        return 0;
                }*/

                // seek to start, check for bad seek
                if (fseek ( fileptr, 0, SEEK_SET)) {
                        gi.dprintf ("ReadEntFile: could not read file, aborted.\n");
                        fclose (fileptr);
                        return 0;
                }

                readlen = fread (entptr, filelen, 1, fileptr);
                fclose (fileptr);

                gi.dprintf("ReadEntFile: read %i items of %i bytes.\n", readlen, filelen);

                // if read returned error, don't return read ents
                if (readlen == 1)
                        return entptr;
                else
                        gi.dprintf ("ReadEntFile: read error: .ent files must be in \"unix\" format (\\n)\n");

        }
        return 0;
}

char *GloomReadFile (char *filename, int *len) {

        size_t  readlen;
        int filelen;
        FILE *fileptr;
        char *entptr;

        // try opening the file
        fileptr = fopen (filename, "rb");

        // if file didn't open, don't do anything more
        if (fileptr != NULL) {
                // find out filelength by seeking to end
                fseek ( fileptr, 0, SEEK_END );
                filelen = ftell( fileptr );

                // alloc memory if file has some length
                if (filelen > 0) {
                        //r1: FINALLY FOUND THE DAMN PROBLEM!<>AS<DSA<DASK>D>ASD
                        //    NEEDED TO ALLOCATE + 1 BYTE FOR TERMINATOR GAH ASDF MLSFMSLFMSDFS
                        //    ASDMASKD GAH!!
                        entptr = gi.TagMalloc (filelen + 1, TAG_GAME);
                } else {
                        fclose (fileptr);
                        return 0;
                }

                // check that we got memory
                if (!entptr) {
                        fclose (fileptr);
                        return 0;
                }

                // seek to start, check for bad seek
                if (fseek ( fileptr, 0, SEEK_SET)) {
                        fclose (fileptr);
                        return 0;
                }

                readlen = fread (entptr, filelen, 1, fileptr);
                fclose (fileptr);

                // if read returned error, don't return
                if (readlen == 1) {
                        *len = filelen;
                        return entptr;
                }
        }
        return 0;
}

char *imagenames[] = {
        "",
        "i_health",
        "a_bullets",
        "a_acid",
        "a_blaster",
        "c4_i",
        "ggren_i",
        "gren_i",
        "flare_i",
        "i_9mmclip",
        "i_acammo",
        "i_autoclip",
        "i_autogun",
        "i_autoshotgun",
        "i_cocoon",
        "turtle",
        "i_magclip",
        "i_magnum",
        "i_missile",
        "i_plasmagun",
        "i_pistol",
        "i_rl",
        "i_shellclip",
        "i_shellex",
        "i_spas",
        "i_spore",
        "i_sspore",
        "i_subclip",
        "i_submachinegun",
        "i_tele",
        "a_shells",
        "a_cells",
        "inventory",
        "teammenu",
        "alienmenu",
        "humanmenu",
        "breedermenu",
        "i_powershield",
        "i_combatarmor",
        "i_jacketarmor",
        "i_bodyarmor",
        "engiemenu",
        "k_security",
        "i_airstrike",
        "k_pyramid",
        "k_datacd",
        "k_redkey",
        "k_powercube",
        "ninja",
        "s_clear",
        "s_guard"
};

char *soundnames[] = {
        "",
        "items/s_health.wav",
        "misc/keytry.wav",
        "items/protect4.wav",
        "misc/lasfly.wav",
        "weapons/noammo.wav",
        "player/fry.wav",
        "weapons/machgf1b.wav",
        "weapons/up_end.wav",
        "weapons/rg_hum.wav",
        "misc/fhit3.wav",
        "voice/toastie.wav",
        "misc/activate.wav",
        "weapons/laser2.wav",
        "weapons/bfg_hum.wav",
        "victory/human.wav",
        "weapons/keyboard.wav",
        "misc/power1.wav",
        "plats/pt1_strt.wav",
        "misc/power2.wav",
        "weapons/acid.wav",
        "weapons/pistgf1.wav",
        "weapons/gg_on.wav",
        "weapons/subfire.wav",
        "mutant/mutatck1.wav",
        "cocoon/drud1.wav",
        "mutant/mutatck2.wav",
        "player/watr_out.wav",
        "switches/butn2.wav",
        "misc/spawn1.wav",
        "alien/watr_in.wav",
        "world/electro.wav",
        "weapons/grenlb1b.wav",
        "weapons/flame.wav",
        "world/blackhole.wav",
        "alien/drown1.wav",
        "player/gasp1.wav",
        "player/watr_in.wav",
        "player/gasp2.wav",
        "misc/windfly.wav",
        "weapons/railgf1a.wav",
        "weapons/hgrenc1b.wav",
        "weapons/grapple/grreset.wav",
        "weapons/grenlf1a.wav",
        "misc/bdeath.wav",
        "tank/thud.wav",
        "organ/organe2.wav",
        "world/force1.wav",
        "organ/organe3.wav",
        "plats/pt1_mid.wav",
        "organ/organe4.wav",
        "alien/watr_out.wav",
        "world/quake.wav",
        "misc/h2ohit1.wav",
        "turret/Tlasexplode.wav",
        //"world/land.wav",
        "weapons/targbeep.wav",
        "makron/laf4.wav",
        "weapons/chomp.wav",
        "alien/watr_un.wav",
        "world/spark1.wav",
        "world/amb10.wav",
        "weapons/hgrent1a.wav",
        "world/spark3.wav",
        "world/ric1.wav",
        "player/watr_un.wav",
        "misc/udeath.wav",
        "items/damage3.wav",
        "world/laser.wav",
        "player/drown1.wav",
        "victory/alien.wav",
        "weapons/hgrenb1a.wav",
        "plats/pt1_end.wav",
        "berserk/attack.wav",
        "weapons/pulsfire.wav",
        "weapons/rockfly.wav",
        "misc/ssdie.wav",
        "items/l_health.wav",
        "weapons/pulseout.wav",
        "weapons/magshot.wav",
        "weapons/c4.wav",
        "items/damage.wav",
        "items/m_health.wav",
        "misc/secret.wav",
        "world/klaxon2.wav",
        "alien/gurp1.wav",
        "world/mov_watr.wav",
        "alien/gurp2.wav",
        "misc/comp_up.wav",
        "world/10_0.wav",
        "weapons/grapple/grfire.wav",
        "misc/talk.wav",
        "doors/dr1_mid.wav",
        "misc/ar1_pkup.wav",
        "organ/twang.wav",
        "world/stp_watr.wav",
        "weapons/hgrenb2a.wav",
        "weapons/tick2.wav",
        "alien/gasp1.wav",
        "alien/gasp2.wav",
        "misc/talk1.wav",
        "weapons/rocklx1a.wav",
        "weapons/davelas.wav",
        "wraith/fly.wav",
        "weapons/mechbeep.wav",
        "weapons/refill.wav",
        "weapons/up.wav",
        "detector/alarm1.wav",
        "detector/alarm2.wav",
        "detector/alarm3.wav",
        "misc/keyuse.wav",
        "misc/turrethum.wav",
        "doors/dr1_end.wav",
        "player/burn1.wav",
        "doors/dr1_strt.wav",
        "weapons/rocklf1a.wav",
        "player/burn2.wav",
        "weapons/webshot1.wav",
        "weapons/webshot2.wav",
        "player/lava_in.wav",
        "weapons/shotweb1.wav",
        "organ/healer1.wav",
        "weapons/misload.wav",
        "ambient/humamb.wav",
        "organ/growegg2.wav",
        "misc/5min.wav",
        "organ/growegg.wav",

#ifdef CACHE_CLIENTSOUNDS
        "player/land1.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",

        "voice/grunt/spawn1.wav",
        "voice/grunt/spawn2.wav",
        "voice/grunt/spawn3.wav",
        "voice/grunt/help1.wav",
        "voice/grunt/help2.wav",
        "voice/grunt/order1.wav",
        "voice/grunt/order2.wav",
        "voice/grunt/order3.wav",
        "voice/grunt/order4.wav",
        "voice/grunt/affirm1.wav",
        "voice/grunt/affirm2.wav",
        "voice/grunt/affirm3.wav",
        "voice/grunt/defend1.wav",
        "voice/grunt/defend2.wav",
        "voice/grunt/defend3.wav",
        "voice/grunt/noammo1.wav",
        "voice/grunt/noammo2.wav",
        "voice/grunt/noammo3.wav",
        "voice/grunt/taunt1.wav",
        "voice/grunt/taunt2.wav",
        "voice/grunt/taunt3.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/heavy/spawn1.wav",

        "voice/heavy/help1.wav",
        "voice/heavy/order1.wav",
        "voice/heavy/affirm1.wav",
        "voice/heavy/defend1.wav",
        "voice/heavy/noammo1.wav",
        "voice/heavy/taunt1.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/commando/spawn1.wav",

        "voice/commando/help1.wav",
        "voice/commando/order1.wav",
        "voice/commando/affirm1.wav",
        "voice/commando/defend1.wav",
        "voice/commando/noammo1.wav",
        "voice/commando/taunt1.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/mech/spawn1.wav",

        "voice/mech/help1.wav",
        "voice/mech/order1.wav",
        "voice/mech/affirm1.wav",
        "voice/mech/defend1.wav",
        "voice/mech/noammo1.wav",
        "voice/mech/taunt1.wav",
        "*mechwalk.wav",
        "*shot.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/shock/spawn1.wav",

        "voice/shock/help1.wav",
        "voice/shock/order1.wav",
        "voice/shock/affirm1.wav",
        "voice/shock/defend1.wav",
        "voice/shock/noammo1.wav",
        "voice/shock/taunt1.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/eng/spawn1.wav",

        "voice/eng/help1.wav",
        "voice/eng/order1.wav",
        "voice/eng/affirm1.wav",
        "voice/eng/defend1.wav",
        "voice/eng/noammo1.wav",
        "voice/eng/taunt1.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/exterm/spawn1.wav",

        "voice/exterm/help1.wav",
        "voice/exterm/order1.wav",
        "voice/exterm/affirm1.wav",
        "voice/exterm/defend1.wav",
        "voice/exterm/noammo1.wav",
        "voice/exterm/taunt1.wav",
        "*step.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav","*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav","*scream.wav",

        "*scream1.wav",
        "*scream2.wav",
        "*scream3.wav",
        "*scream4.wav",
        "*scream5.wav",
        "*scream6.wav",

        "*jump1.wav",
        "*pain100_1.wav","*pain100_2.wav","*pain75_1.wav",      "*pain75_2.wav",
        "*pain50_1.wav","*pain50_2.wav","*pain25_1.wav","*pain25_2.wav",
        "*death1.wav","*death2.wav","*death3.wav","*death4.wav",
        "*fall1.wav","*fall2.wav",
        "voice/biotech/spawn1.wav",

        "voice/biotech/help1.wav",
        "voice/biotech/help2.wav",
        "voice/biotech/help3.wav",
        "voice/biotech/order1.wav",
        "voice/biotech/affirm1.wav",
        "voice/biotech/defend1.wav",
        "voice/biotech/noammo1.wav",
        "voice/biotech/taunt1.wav",
        "voice/biotech/taunt2.wav",
#endif
};

void fire_reconnect (edict_t *ent)
{
        edict_t *t;

        t = NULL;
        while ((t = G_Find (t, FOFS(classname), "target_reconnectwait")) != NULL) {
                G_UseTargets (t, t);
                if (t)
                        G_FreeEdict (t);
        }

        if (ent)
                G_FreeEdict (ent);
}

#ifdef ZZLIB
int compress(char *filename)
{
        int decompress = 0;
        unsigned int level = 1;
        char *fname_r = NULL;
        char *fname_w = NULL;
        unsigned char *buffer;
        int len, finallen;
        struct stat buf_stat;
        FILE *file_r = NULL;
        FILE *file_w = NULL;
        
        fname_r = fname_w = filename;
        
        if ( stat( fname_r, &buf_stat ) == -1 ) 
                return false;

        len = buf_stat.st_size;
        
        if ( (buffer = (unsigned char *)malloc( len + 16 )) == NULL ) 
                return false;

        
        if ( (file_r = fopen( fname_r, "rb" )) == NULL )
                return false;

        len = fread( buffer, sizeof(unsigned char), len, file_r );
        fclose( file_r );
        
        
        //gi.dprintf("compress: compressing %s...", fname_r);
        finallen = ZzCompressBlock( buffer, len, level, 0 );

        switch ( finallen ) {
                case -1:
                        //gi.dprintf("not enough memory!\n");
                        return false;
                case -2:
                        //gi.dprintf ("CRC error!\n");
                        return false;
                default :
                        //gi.dprintf ("OK! [%u/%u]\n", len, finallen);
                        break;
        }

        
        if ( (file_w = fopen( fname_w, "wb" )) == NULL )
                return false;

        fwrite( buffer, sizeof(unsigned char), finallen, file_w );
        fclose( file_w );
        
        free (buffer);

        return finallen;
}

char *decompress_block(char *block, int len, int *final)
{
        unsigned char *buffer;
        int finallen;
        int len_uncompressed;
        
        if ( (buffer = (unsigned char *)malloc( len + 16 )) == NULL ) 
                return NULL;
        
        memcpy (buffer, block, len);

        len_uncompressed = *((unsigned int*)buffer);

        if (len_uncompressed < 1)
                return NULL;

        buffer = (unsigned char*)realloc( buffer, len_uncompressed );
        finallen = ZzUncompressBlock( buffer );

        switch ( finallen ) {
                case -1:
                        //gi.dprintf("not enough memory!\n");
                        return NULL;
                case -2:
                        //gi.dprintf ("CRC error!\n");
                        return NULL;
        }

        *final = finallen;

        //free this once done !!
        buffer[finallen] = '\0';
        return buffer;
}
#endif

/*
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

spawnpoint contains targetname of ent which should be used.
*/
float oldxmins = 0;

void AdminLogNewMap (char *mapname);
void resetLog (void);
void SV_Invert_f (void);
void ResetVoteList (void);
void InitChangeClassLog(void);


static int usedpoints[MAXTEAMS];


static void ParseConfig(FILE*f)
{
  char buf[1024];
  int line = 0;
  while (fgets(buf, sizeof(buf), f)) {
    char*name = buf;
    char*value;

    buf[1023] = 0;
    line++;

    while (*name && *name <= 32) name++;
    if (*name == '#' || !*name) continue;
    value = name;
    while (*value > 32) value++;
    *value = 0; value++;
    while (*value && *value <= 32) value++;
    if (*value != '=') {
      gi.dprintf("invalid map config formatting line %d %s/%s\n", line, name,value);
      continue;
    }
    value++;
    while (*value && *value <= 32) value++;

    if (buf[0] == '#') continue;
    if (!strcasecmp(name, "alienbp")) team_info.maxpoints[TEAM_ALIEN] = atoi(value) - usedpoints[TEAM_ALIEN] - team_info.points[TEAM_ALIEN];
    else
    if (!strcasecmp(name, "humanbp")) team_info.maxpoints[TEAM_HUMAN] = atoi(value) - usedpoints[TEAM_HUMAN] - team_info.points[TEAM_HUMAN];
    else
    if (!strcasecmp(name, "alienfreebp")) team_info.maxpoints[TEAM_ALIEN] = team_info.points[TEAM_ALIEN]+atoi(value);
    else
    if (!strcasecmp(name, "humanfreebp")) team_info.maxpoints[TEAM_HUMAN] = team_info.points[TEAM_HUMAN]+atoi(value);
    else
    if (!strcasecmp(name, "deathball")) {
      int x, y, z, f;
      edict_t* ent = G_Spawn();

      int r = sscanf(value, "%d,%d,%d %d", &x, &y, &z, &f);
      if (r < 3) {
        gi.dprintf("Invalid syntax for deathball, need at least X,Y,Z coordinates.\n");
        continue;
      }
      if (r < 4) f = 0;

      ent->s.origin[0] = x;
      ent->s.origin[1] = y;
      ent->s.origin[2] = z;
      ent->spawnflags = f;
      SP_misc_deathball(ent);
      ent->classname = (f&4)?"deatharea":"deathball";
    } else
      gi.dprintf("Unknown map config value line %d: %s\n", line, name);
  }
}


void EXPORT SpawnEntities (char *mapname, char *entities, char *spawnpoint){
        edict_t         *ent;
        edict_t         *t = NULL;
        //int                   inhibit;
        char            *com_token;
        int                     i;
        int     entsinuse= 0;
        edict_t *e = NULL;
        char            *readents = NULL;
        FILE*f;

        gi.FreeTags (TAG_LEVEL);

        team_info.buildpool[TEAM_ALIEN] = team_info.buildpool[TEAM_HUMAN] = 0;

        //nuke caches
        memset (imagecache, 0, MAX_IMAGE_CACHE * sizeof(int));
        memset (soundcache, 0, MAX_SOUND_CACHE * sizeof(int));

        memset (&level, 0, sizeof(level));
        memset (g_edicts, 0, MAX_EDICTS * sizeof (g_edicts[0]));

        strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
        
        LoadDamageOverrideTable();
        
        InitializePlayerQueues(
          &queue_join_humans,
          &queue_join_aliens,
          &queue_spawn_humans,
          &queue_spawn_aliens,
          NULL
        );
        
        InitTimelimit();
        
        //partition CS_GENERAL space
        i = CS_GENERAL+game.maxclients+2;
        i += InitHudStrings(i);

        // update lastmaps array
        for (i = 3; i >= 0;i--) {
                strncpy(lastmaps[i+1], lastmaps[i], sizeof(lastmaps[i+1])-1);
        }
        strncpy(lastmaps[0], level.mapname, sizeof(lastmaps[0])-1);

        if (xmins_random->value)
        {
                if (!oldxmins)
                        oldxmins = xmins->value;
                gi.cvar_set ("xmins", va("%f", oldxmins + random() * xmins_random->value));
        }

#ifdef VOTE_EXTEND
        //r1: grab old time limit here instead of init game so it can be set via configs
        //etc.
        if (!oldtimelimit) {
                oldtimelimit = (int)timelimit->value;
        } else {
        //reset the timelimit if it was potentially extended
                gi.cvar_set ("timelimit", va("%d", oldtimelimit));
        }
#endif

        if (gloomgamelog->value) {
                CloseGamelog();
                OpenGamelog();
        } else {
                gloomlog = NULL;
        }

        AdminLogNewMap (mapname);

        InitChangeClassLog();

        if (mapdebugmode->value >= 3) {
                FILE *file;
                file = fopen (va("%s/%s.txt",gamedir->string, level.mapname),"wb");
                if (file) {
                        fprintf (file, "%s",entities);
                        fclose (file);
                        gi.dprintf ("Entities have been dumped to %s/%s.txt\n",gamedir->string, level.mapname);
                } else {
                        gi.dprintf ("Unable to dump entities!\n");
                }
        }

        /* setup player ents */
        for (i=0 ; i<game.maxclients ; i++)
        {
                ent = g_edicts + 1 + i;

                ent->s.number = ent - g_edicts;

                ent->client = game.clients + i;

                ent->client->pers.old_connected = ent->client->pers.connected;
                ent->client->pers.connected = false;
        }

        //r1ch: always use correct index - handles voted maps, starting map etc.
        currentmapindex = IndexByMapname (mapname, 0);

        //r1ch: map loaded ok, set ExitLevel loop count to normal
        changemap_spincount = 0;
        level.exitintermission = false;

        UpdateCurrentMapPlayedCount(1);

        //r1ch - reset ghost list
        /*for (i = 0 ; i <= 64;i++) {
                fragsave[i].inuse = 0;
                fragsave[i].ghostcode = 0;
                fragsave[i].frags = 0;
                fragsave[i].expiretime = 0;
        }*/

        memset (&fragsave, 0, sizeof(fragsave));

        memset (&spawnlist, 0, sizeof(spawnlist));

        ent = NULL;
//      inhibit = 0;

        //InitBodyQue ();
        
        
        // ReadEntFile part --tumu

        // r1: fixed ReadEntFile to use fread etc.
        readents = ReadEntFile (mapname);

        if (readents)
                entities = readents;
        // ReadEntFile stops here

        // parse ents
        for (;;)
        {
                // parse the opening brace
                com_token = COM_Parse (&entities);
                if (!entities)
                        break;
                if (com_token[0] != '{') {
                        //gi.dprintf ("Parsed: %s\n",entities);
                        gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);
                }

                if (!ent)
                        ent = g_edicts;
                else
                        ent = G_Spawn ();
                entities = ED_ParseEdict (entities, ent);

                ED_CallSpawn (ent);
                
                if (ent == world) for (i = 0; i < MAXTEAMS; i++) usedpoints[i] = team_info.maxpoints[i];
        }
        
        for (i = 0; i < MAXTEAMS; i++) usedpoints[i] -= team_info.maxpoints[i];

        //now we've got all the ents set up, verify some trigger_relay stuff
        while ((t = G_Find (t, FOFS(classname), "trigger_relay"))) {
                if (!t->target && !t->health && !t->count && !t->killtarget)
                {
                        edict_t *s = NULL;
                        qboolean ok = false;
                        while ((s = G_Find (s, FOFS(team), t->targetname))) {
                                ok = true;
                                break;
                        }
                        if (!ok) {
                                gi.dprintf ("REMOVED: unused %s at %s\n",t->classname, vtos(t->s.origin));
                                G_FreeEdict (t);
                        }
                }
        }

        //and again, with teleporter destinations
        t = NULL;
        while ((t = G_Find (t, FOFS(classname), "misc_teleporter"))) {
                if (!G_Find (NULL, FOFS(targetname), t->target)) {
                        gi.dprintf ("REMOVED: can't find target of %s at %s\n",t->classname, vtos(t->s.origin));
                        G_FreeEdict (t);
                }
        }

        t = NULL;
        while ((t = G_Find (t, FOFS(classname), "target_cloner"))) {
                edict_t *targ;
                int targets;

                targ = NULL;
                targets = 0;

                while ((targ = G_Find (targ, FOFS(targetname), t->targetname))) {
                        targets++;
                }

                if (targets != 1) {
                        gi.dprintf ("REMOVED: target_cloner with %d targets (should be 1) at %s\n", targets, vtos(t->s.origin));
                        G_FreeEdict (t);
                        return;
                }
        }

        //gi.dprintf ("%i entities inhibited\n", inhibit);

        e = &g_edicts[game.maxclients+1];
        for ( i=game.maxclients +1 ; i<globals.num_edicts ; i++, e++)
                if (e->inuse)
                        entsinuse++;


        if (entsinuse > 512)
                gi.dprintf ("WARNING: map uses over half the available entities!\n");

        G_FindTeams ();

        gi.dprintf ("%d entities total\n",entsinuse);

        //r1: makes sense. hope i did it right :)
        //    stops game-spawned entities inheriting the st.classes/st.hurtflags of
        //    the last map-spawned entity.
        memset (&st,0,sizeof(st));

        //count the initial spawns
        if (team_info.spawns[TEAM_ALIEN] == 0 || team_info.spawns[TEAM_HUMAN] == 0)
                gi.dprintf ("WARNING: Map does not have Gloom spawn points!\n");

        if (xmins->value == 0) {
                t = NULL;
                while ((t = G_Find (t, FOFS(classname), "target_xmins")) != NULL) {
                        G_UseTargets (t, t);
                        if (t)
                                G_FreeEdict (t);
                }
        }

        if (reconnect_wait->value == 0) {
                fire_reconnect (NULL);
        } else {
                t = G_Spawn ();
                t->classname = "reconnectfirer";
                t->nextthink = level.time + reconnect_wait->value;
                t->think = fire_reconnect;
        }

        // kill me now
        if (level.mapname[0] != '_' && strcmp (level.mapname, "msdni") && strcmp (level.mapname, "bpm_purge") && strcmp (level.mapname, "garena-b")
                && strcmp (level.mapname, "garena2") && !G_Find (t, FOFS(classname), "on_humans_win") && !G_Find (t, FOFS(classname), "on_aliens_win")
                && !G_Find (t, FOFS(classname), "target_setclasses") && !G_Find (t, FOFS(classname), "target_aliens_win") && !G_Find (t, FOFS(classname), "target_humans_win") &&
                random() < randominvert->value)
                SV_Invert_f ();

        for (i = 0; i < MAXTEAMS; i++) {
                if (team_info.points[i] < 0)
                        team_info.points[i] = 0;
                if (team_info.maxpoints[i] < 0)
                        team_info.maxpoints[i] = 0;
                team_info.buildtime[i] = 0;
        }

        //reset temp. banlist
        CheckIPBanList ();

        //reset build log
        resetLog ();

        ResetVoteList ();

        // reset vote maps list
        for (i = 0; i < 2; i++)
                vote.lastmap_index[i] = 0;

  if (f = fopen (va("%s/maps/default.conf",gamedir->string),"r")) {
    ParseConfig(f);
    fclose(f);
  }
  if (f = fopen (va("%s/maps/%s.conf",gamedir->string, level.mapname),"r")) {
    ParseConfig(f);
    fclose(f);
  }
  
  level.nospawns = !(int)dedicated->value && (!team_info.spawns[TEAM_ALIEN] || !team_info.spawns[TEAM_HUMAN]);
}

void Cmd_Respawn_f(edict_t *ent);

//this is called every 10 seconds and verfies all players and structures aren't
//outside the map. if they are, splat. players are attempted to respawn first, if
//that fails they die.
void world_think (edict_t *ent)
{
        edict_t *t;
        vec3_t point;

        for ( t = g_edicts+1 ; t <= &g_edicts[globals.num_edicts] ; t++) {
                if (!t->inuse)
                        continue;
                if (!(t->flags & FL_CLIPPING || t->svflags & SVF_MONSTER || t->client))
                        continue;
                if (t->client && (!t->client->resp.team || t->deadflag || t->health < 1))
                        continue;
                
                point[0] = t->s.origin[0];
                point[1] = t->s.origin[1];
                point[2] = t->s.origin[2];
                if (t->client && t->maxs[2] <= 0) //Hatchling box has negative maxs, thus detected wrongly as outside map when grappling on ceiling
                  point[2] += t->maxs[2]-1;
                if (gi.pointcontents (point) & CONTENTS_SOLID) {
                        if (t->client) {
                                VectorClear (t->velocity);
                                VectorClear (t->client->oldvelocity);
                                if (t->client->ctf_grapple)
                                        CTFResetGrapple (t->client->ctf_grapple);
                                if (!(teleport_respawn (t))) {
                                  if (t->client->resp.can_respawn)
                                    Cmd_Respawn_f(t);

                                  if (ent->health > 0) {
                                        gi.cprintf (t, PRINT_HIGH, "Bad luck... no spawns available.\n");
                                        T_Damage (t, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
                                  }
                                } else {
                                        gi.cprintf (t, PRINT_HIGH, "You were respawned since you seem to be outside the map.\n");
                                }
                        } else {
                                gi.dprintf ("REMOVED: %s at %s seems to be out of map!\n", t->classname, vtos(t->s.origin));
                                T_Damage (t, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_OUT_OF_MAP);
                                if (t->solid)
                                        G_FreeEdict (t);
                        }
                }
        }

        ent->nextthink = level.time + 5;
}

#if 0
        // cursor positioning
        xl <value> // left
        xr <value> // right
        yb <value> // bottom
        yt <value> // top
        xv <value> // virtual
        yv <value> // virtual

        // drawing
        pic <stat>
        picn <name>
        num <fieldwidth> <stat>

        // strings, left aligned to cursor
        string <string>
        string2 <string>
        stat_string <stat>

        // centered strings, center aligned
        cstring <string>
        cstring2 <string>

        // control
        if <stat>
        ifeq <stat> <value>
        ifbit <stat> <value>
        endif

        // fields
        hnum
        rnum
        anum
        client <x> <y> <client> <score> <ping> <time>
        ctf <x> <y> <client> <score> <ping> (cuts name at 12 chars)

        ?field_3 <1> <2> <string>?
        backtile?
#endif

/*char *single_statusbar =
"yb -24 "

// health
"xv 0 "
"hnum "
"xv 50 "
"pic 0 "

// ammo
"if 2 "
 "xv 100 "
 "anum "
 "xv 150 "
 "pic 2 "
"endif "

// armor
"if 4 "
 "xv 200 "
 "rnum "
 "xv 250 "
 "pic 4 "
"endif "

// selected item
  "if 6 "
  "xv      296 "
  "pic 6 "
  "xv      264 "
  "num     2 18"
"endif "

"yb     -50 "

// picked up item
"if 7 "
"       xv      0 "
"       pic 7 "
"       xv      26 "
"       yb      -42 "
"       stat_string 8 "
"       yb      -50 "
"endif "

// timer
"if 9 "
"       xv      262 "
"       num     4       10 "
"       xv      296 "
"       pic     9 "
"endif "

//      help / weapon icon
"if 11 "
"       xv      148 "
"       pic     11 "
"endif "

"if 16 "
"       xv      160 "
"       yv      120 "
"       pic     16 "
"endif "
;*/

char *dm_statusbar =
"yb     -24 "

// health
"if 0 "
  "xv 0 "
  "hnum "
  "xv 50 "
  "pic 0 "
"endif "

// ammo
"if 2 "
  "xv 100 "
  "anum "
  "xv 150 "
  "pic 2 "
"endif "

// armor
"if 5 " // was 4
  "xv 200 "
  "rnum "
  "xv 250 "
  "pic 4 "
"endif "

// selected item
"if 6 "
//  "xv 296 "
  "xv 312 " 
  "pic 6 "
//  "xv 262 "
  "xv 264 "
  "num 3 18 "
"endif "

"yb     -50 "

//guard/cmd icon
"if 19 "
  "xv 50 "
  "pic 0 "
  "pic 19 "
"endif "

// picked up item
"if 7 "
  "xv 0 "
  "pic 7 "
"endif "
"if 8 "
  "xv 26 "
  "yb -42 "
  "stat_string 8 "
  "yb -50 "
"endif "

//      help / weapon icon
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "

//      frags
"if 0 " // if observer, don't display frags
  "xr -52 "
  "yt 2 "
  "num 3 28 " // display gloom frags
"endif "

// timer
"if 9 "
  "yt 64 "
  "xr -100 "
  //"xr -84 "
  "num 4 10 "
  "yt 64 "
  "xr -33 "
  "pic 9 "
"endif "

"if 20 "                // Points
  "xr -68 "
  "yt 32 "
  "num 4 20 "
"endif "

"if 21 "        //eggs
  "xl 4 "
  "yt 34 "
  "pic 21 "
  "xl 28 "
  "num 2 22 "
"endif "

"if 23 "                //teles
  "xl 4 "
  "yt 66 "
  "pic 23 "
  "xl 28 "
  "num 2 24 "
"endif "

"if 25 "
  "xv 26 "
  "yb -60 "
  //"string \"Viewing\" "
  //"xl 64 "      //64
  "stat_string 25 "
"endif "

"if 26 "
  "yt 40 "
  "xr -48 "
  "num 3 26 "
  "xr -64 "
  "pic 21 "
"endif "

// menu stuff --tumu
"if 29 "
  "xv 32 yv 8 picn inventory pic 29 "
"endif "
;

int _ImageIndex (int index)
{
        return imagecache[index] = gi.imageindex (imagenames[index]);
}

int _SoundIndex (int index)
{
        return soundcache[index] = gi.soundindex (soundnames[index]);
}

const char *defaultlightlevels[] = 
{
        "m",
        "mmnmmommommnonmmonqnmmo",
        "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
        "mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
        "mamama",
        "jklmnopqrstuvwxyzyxwvutsrqponmlkj",
        "nmonqnmomnmomomno",
        "mmmaaaabcdefgmmmmaaaammmaamm",
        "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
        "aaaaaaaazzzzzzzz",
        "mmamammmmammamamaaamammma",
        "abcdefghijklmnopqrrqponmlkjihgfedcba"
};

/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"   environment map name
"skyaxis"       vector axis for rotating sky
"skyrotate"     speed of rotation in degrees/second
"sounds"        music cd track number
"gravity"       800 is default gravity
"message"       text to print at user logon
*/
void SP_worldspawn (edict_t *ent)
{
        int i;

        ent->movetype = MOVETYPE_PUSH;
        ent->solid = SOLID_BSP;
        ent->inuse = true;                      // since the world doesn't use G_Spawn()
        ent->s.modelindex = 1;          // world model is always index 1

        // set configstrings for items
        SetItemNames ();

        // map author is wanting a map to be linked
        if (st.nextmap) {

                // don't allow map to link itself
                // r1: !Q_stricmp is "if it is"...
                if (Q_stricmp (level.mapname, st.nextmap)) {
                        strncpy (level.nextmap, st.nextmap, sizeof(level.nextmap)-1);
                }
        }

        // reset team_info's
        breakinSetDefaults();

        team_info.winner = TEAM_NONE;
        team_info.playmode = st.playmode;
        team_info.leveltimer = st.leveltimer;
        team_info.classes = st.classes;

        level.noautowin = st.noautowin;
        
        vec3_t point = {65535, 65535, 65535};
        level.leaked = !(gi.pointcontents(point) & CONTENTS_SOLID);

        team_info.startfrags[TEAM_ALIEN] = st.alienstartfrags;
        team_info.startfrags[TEAM_HUMAN] = st.humanstartfrags;

        if (st.pausetime == 0)
                team_info.startfragtimer = -1;
        else
                team_info.startfragtimer = st.pausetime + reconnect_wait->value;

        if (st.spawnteam)
                team_info.spawnteam = 1 << (randomMT()%st.spawnteam);
        else
                team_info.spawnteam = 0;

        if (st.buildflags)
                team_info.buildflags = st.buildflags;

        if (st.minyaw)
                team_info.maxpoints[TEAM_HUMAN] = st.minyaw;

        if (st.maxyaw)
                team_info.maxpoints[TEAM_ALIEN] = st.maxyaw;

        DoClassMenus();         // remove disabled classes from menus

        // make some data visible to the server

        if (ent->message && ent->message[0])
        {
                gi.configstring (CS_NAME, ent->message);
//      } else {
//              gi.dprintf ("WARNING: worldspawn without message field set\n");
        }

        if (st.sky && st.sky[0])
                gi.configstring (CS_SKY, st.sky);
        else
                gi.configstring (CS_SKY, "unit1_");

        gi.configstring (CS_SKYROTATE, va("%g", st.skyrotate) );

        gi.configstring (CS_SKYAXIS, va("%g %g %g",
                st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

        //gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

        gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

        // status bar program
        gi.configstring (CS_STATUSBAR, dm_statusbar);

        // for reducing message clutter
        gi.configstring (CS_GENERAL + game.maxclients + 1, "Inventory refilled!");
        gi.configstring (CS_GENERAL + game.maxclients + 2, "Priming C4");

        //r1: breaks use of client 'skins' command by flooding console :)
        gi.configstring (CS_PLAYERSKINS+248, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+249, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+250, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+251, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+252, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+253, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+254, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        gi.configstring (CS_PLAYERSKINS+255, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

        if (!st.gravity)
                gi.cvar_set("sv_gravity", "800");
        else
                gi.cvar_set("sv_gravity", st.gravity);

        SoundIndex (misc_talk1);
        SoundIndex (misc_udeath);
        SoundIndex (misc_5min);
        //SoundIndex (voice_toastie);

        //SoundIndex (victory_human);
        //SoundIndex (victory_alien);
        // gibs

        sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");

        // stinger and c4 fire
        firea_index = gi.modelindex ("sprites/s_firea.sp2");
        fireb_index = gi.modelindex ("sprites/s_fireb.sp2");

        // flashlights
        shiny_index = gi.modelindex ("sprites/s_shine.sp2");

        ImageIndex(a_acid);// = gi.imageindex ("a_acid");
        ImageIndex(a_blaster); //= gi.imageindex ("a_blaster");
        ImageIndex(c4_i); // = gi.imageindex ("c4_i");
        ImageIndex(ggren_i); // = gi.imageindex ("ggren_i");
        ImageIndex(gren_i); // = gi.imageindex ("gren_i");
        ImageIndex(flare_i); // = gi.imageindex ("flare_i");
        ImageIndex(i_9mmclip); // = gi.imageindex ("i_9mmclip");
        ImageIndex(i_acammo); // = gi.imageindex ("i_acammo");
        ImageIndex(i_autoclip); // = gi.imageindex ("i_autoclip");
        ImageIndex(i_autogun); // = gi.imageindex ("i_autogun");
        ImageIndex(i_autoshotgun); // = gi.imageindex ("i_autoshotgun");
        ImageIndex(i_cocoon); // = gi.imageindex ("i_cocoon");
        ImageIndex(turtle); // = gi.imageindex ("turtle");
        ImageIndex(i_magclip); // = gi.imageindex ("i_magclip");
        ImageIndex(i_magnum); // = gi.imageindex ("i_magnum");
        ImageIndex(i_missile); // = gi.imageindex ("i_missile");
        ImageIndex(i_plasmagun); // = gi.imageindex ("i_plasmagun");
        ImageIndex(i_pistol); // = gi.imageindex ("i_pistol");
        ImageIndex(i_rl); // = gi.imageindex ("i_rl");
        ImageIndex(i_shellclip); // = gi.imageindex ("i_shellclip");
        ImageIndex(i_shellex); // = gi.imageindex ("i_shellex");
        ImageIndex(i_spas); // = gi.imageindex ("i_spas");
        ImageIndex(i_spore); // = gi.imageindex ("i_spore");
        ImageIndex(i_sspore); // = gi.imageindex ("i_sspore");
        ImageIndex(i_subclip); // = gi.imageindex ("i_subclip");
        ImageIndex(i_submachinegun); // = gi.imageindex ("i_tele");
        ImageIndex(i_tele); // = gi.imageindex ("i_tele); //");
        ImageIndex(a_shells); // = gi.imageindex ("a_shells"); //r1<< added (fix bio spawning can't find pic error?)
        ImageIndex(a_cells); // = gi.imageindex ("a_cells");
        ImageIndex(inventory); // = gi.imageindex ("inventory");
        ImageIndex(i_health); // = gi.imageindex ("i_health");
        ImageIndex(a_bullets); // = gi.imageindex ("a_bullets");
        ImageIndex(teammenu); // = gi.imageindex ("teammenu");
        ImageIndex(alienmenu); // = gi.imageindex ("alienmenu");
        ImageIndex(humanmenu); // = gi.imageindex ("humanmenu");
        ImageIndex(breedermenu); // = gi.imageindex ("breedermenu");
        ImageIndex(engiemenu); // = gi.imageindex ("engiemenu");
        ImageIndex(inventory);
        ImageIndex(i_jacketarmor);
        ImageIndex(i_combatarmor);
        ImageIndex(i_bodyarmor);
        ImageIndex (i_powershield);

        ImageIndex(k_security);
        ImageIndex(i_airstrike);
        ImageIndex(k_pyramid);
        ImageIndex(k_datacd);
        ImageIndex(k_redkey);
        ImageIndex(k_powercube);

        //r1: precache classes
        for (i = 0; i < CLASS_OBSERVER; i++)
        {
                if (classlist[i].model)
                        gi.modelindex (classlist[i].model);

                if (classlist[i].headgib)
                        gi.modelindex (classlist[i].headgib);

                if (classlist[i].gib1)
                        gi.modelindex (classlist[i].gib1);

                if (classlist[i].gib2)
                        gi.modelindex (classlist[i].gib2);
        }

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

        if (st.lightscale)
        {
                size_t j, len;
                char templight[MAX_QPATH];

                for (i = 0; i < 12; i++)
                {
                        strncpy (templight, defaultlightlevels[i], sizeof(templight));
                        len = strlen(templight);
                        for (j = 0; j < len; j++)
                        {
                                templight[j] += st.lightscale;
                                if (templight[j] < 'a')
                                        templight[j] = 'a';
                                else if (templight[j] > 'z')
                                        templight[j] = 'z';
                        }
                        gi.configstring (CS_LIGHTS+i, templight);
                }
        }
        else
        {
                for (i = 0; i < 12; i++)
                {
                        gi.configstring (CS_LIGHTS+i, defaultlightlevels[i]);
                }
        }

        /*
        // 0 normal
        gi.configstring(CS_LIGHTS+0, "m");

        // 1 FLICKER (first variety)
        gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");

        // 2 SLOW STRONG PULSE
        gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

        // 3 CANDLE (first variety)
        gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

        // 4 FAST STROBE
        gi.configstring(CS_LIGHTS+4, "mamama");

        // 5 GENTLE PULSE 1
        gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");

        // 6 FLICKER (second variety)
        gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");

        // 7 CANDLE (second variety)
        gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");

        // 8 CANDLE (third variety)
        gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

        // 9 SLOW STROBE (fourth variety)
        gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");

        // 10 FLUORESCENT FLICKER
        gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

        // 11 SLOW PULSE NOT FADE TO BLACK
        gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

        // 12 Fast pulse
        //gi.configstring(CS_LIGHTS+12, "acegikmoqrpnljhfdb");*/

        //Alien only light
        gi.configstring(CS_LIGHTS+31, "a");


        // styles 32-62 are assigned by the light program for switchable lights

        if (sv_cheats->value == 0) {
                ent->nextthink = level.time + 1;
                ent->think = world_think;
        }
        // 63 testing
        //gi.configstring(CS_LIGHTS+63, "a");
}

/*QUAKED target_spawner (1 0 0) (-8 -8 -8) (8 8 8)
Set target to the type of entity you want spawned.
Useful for spawning monsters and gibs in the factory levels.

For monsters:
        Set direction to the facing you want it to have.

For gibs:
        Set direction if you want it moving and
        speed how fast it should be moving otherwise it
        will just be dropped
*/
void use_target_spawner (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *ent;

        ent = G_Spawn();
        ent->classname = self->target;
        ent->style = self->style;
        ent->target = self->pathtarget;
        ent->hurtflags = self->hurtflags;
        ent->spawnflags = self->spawnflags;
        VectorCopy (self->s.origin, ent->s.origin);
        VectorCopy (self->s.angles, ent->s.angles);
        ED_CallSpawn (ent);
        gi.unlinkentity (ent);
        KillBox (ent);
        gi.linkentity (ent);
        if (self->speed)
                VectorCopy (self->movedir, ent->velocity);
}

void SP_target_spawner (edict_t *self)
{
        spawn_t *s;
        int     i;
        const gitem_t *item;

        if (CheckTargetnameEnt (self))
                return;

        for (s=spawns ; s->name ; s++)
        {
                if (!strcmp(s->name, self->target)) {
                        self->use = use_target_spawner;
                        self->svflags = SVF_NOCLIENT;
                        if (self->speed)
                        {
                                G_SetMovedir (self->s.angles, self->movedir);
                                VectorScale (self->movedir, self->speed, self->movedir);
                        }
                        return;
                }
        }

        for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
        {
                if (!item->classname)
                        continue;

                if (!strcmp(item->classname, self->target))
                {       // found it
                        self->use = use_target_spawner;
                        self->svflags = SVF_NOCLIENT;
                        if (self->speed)
                        {
                                G_SetMovedir (self->s.angles, self->movedir);
                                VectorScale (self->movedir, self->speed, self->movedir);
                        }
                        return;
                }
        }

        gi.dprintf ("REMOVED: target_spawner at %s trying to spawn non-existant %s\n",vtos2(self), self->target);
        G_FreeEdict (self);
}
