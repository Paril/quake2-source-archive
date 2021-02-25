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

/*QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
Fire an origin based temp entity event to the clients.
"style"         type byte
*/
void Use_Target_Tent (edict_t *ent, edict_t *other, edict_t *activator)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (ent->style);
        gi.WritePosition (ent->s.origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
}

void SP_target_temp_entity (edict_t *ent)
{
        ent->use = Use_Target_Tent;
        if (CheckTargetnameEnt (ent))
                return;
}

/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"         wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"        0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/
void Use_Target_Speaker (edict_t *ent, edict_t *other, edict_t *activator)
{
        int             chan;

        if (ent->spawnflags & 3)
        {       // looping sound toggles
                if (ent->s.sound)
                        ent->s.sound = 0;       // turn it off
                else
                        ent->s.sound = ent->noise_index;        // start it
        }
        else
        {       // normal sound
                if (ent->spawnflags & 4)
                        chan = CHAN_AUTO|CHAN_RELIABLE;
                else
                        chan = CHAN_AUTO;
                // use a positioned_sound, because this entity won't normally be
                // sent to any clients because it is invisible
                gi.positioned_sound (ent->s.origin, ent, chan, ent->noise_index, ent->volume, ent->attenuation, 0);
        }
}

void SP_target_speaker (edict_t *ent)
{
        char    buffer[MAX_QPATH];

        if(!st.noise)
        {
                gi.dprintf("WARNING: target_speaker with no noise set at %s\n", vtos2(ent));
                return;
        }
        if (!strstr (st.noise, ".wav"))
                Com_sprintf (buffer, sizeof(buffer), "%s.wav", st.noise);
        else
                strncpy (buffer, st.noise, sizeof(buffer));
        ent->noise_index = gi.soundindex (buffer);

        if (!ent->volume)
                ent->volume = 1.0;

        if (!ent->attenuation)
                ent->attenuation = 1.0;
        else if (ent->attenuation == -1)        // use -1 so 0 defaults to 1
                ent->attenuation = 0;

        // check for prestarted looping sound
        if (ent->spawnflags & 1)
                ent->s.sound = ent->noise_index;

        //ent->svflags |= SVF_NOCLIENT;

        ent->use = Use_Target_Speaker;

        // must link the entity so we get areas and clusters so
        // the server can determine who to send updates to
        gi.linkentity (ent);
}

/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"         wait this long before going off
"dmg"           how much radius damage should be done, defaults to 0
*/
void target_explosion_explode (edict_t *self)
{
        float           save;

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_EXPLOSION1);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE, 0);

        save = self->delay;
        self->delay = 0;
        G_UseTargets (self, self->activator);
        self->delay = save;
}

void use_target_explosion (edict_t *self, edict_t *other, edict_t *activator)
{
        self->activator = activator;

        if (mapdebugmode->value)
                gi.dprintf ("[targexp] target_explosion {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        if (!self->delay)
        {
                target_explosion_explode (self);
                return;
        }

        self->think = target_explosion_explode;
        self->nextthink = level.time + self->delay;
}

void SP_target_explosion (edict_t *ent)
{
        if (CheckTargetnameEnt (ent))
                return;
        ent->use = use_target_explosion;
        ent->svflags = SVF_NOCLIENT;
}

void toggleclass_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (self->style)
                team_info.classes ^= self->style;
        else if (self->count)
                team_info.classes = self->count;
        else if (self->dmg)
                team_info.classes |= self->dmg;
        else if (self->health)
                team_info.classes &= self->health;
        
        DoClassMenus();
}

void SP_target_toggleclass (edict_t *ent)
{
        ent->style = st.classes;

        if (!ent->style && !ent->count && !ent->random && !ent->health) {
                gi.dprintf ("REMOVED: target_toggleclass at %s with no classes set\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        ent->use = toggleclass_use;
}

void SP_target_reconnectwait (edict_t *ent)
{
        if (CheckTargetEnt (ent))
                return;

        ent->svflags = SVF_NOCLIENT;
}

void use_target_buildpoints (edict_t *self, edict_t *other, edict_t *activator)
{
        team_info.points[self->dmg] += self->count;

        if (self->spawnflags & 1)
        {
                G_FreeEdict (self);
                return;
        }
}

void use_target_setclasses (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *targ = NULL;

        while ((targ = G_Find (targ, FOFS(targetname), self->target)))
        {
                if (self->count == 0)
                        targ->style = self->style;
                else if (self->count == 1)
                        targ->style |= self->style;
                else if (self->count == 2)
                        targ->style &= self->style;
                else if (self->count == 3)
                        targ->style ^= self->style;
        }
}

void SP_target_setclasses (edict_t *ent)
{
        if (CheckTargetEnt (ent))
                return;

        ent->use = use_target_setclasses;
        ent->svflags = SVF_NOCLIENT;
        ent->style = st.classes;
}

/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
Changes level to "map" when fired
*/
void use_target_changelevel (edict_t *self, edict_t *other, edict_t *activator)
{
        if (level.intermissiontime && (level.intermissiontime < level.framenum))
                return;         // already activated

        /*if (!deathmatch->value)
        {
                if (g_edicts[1].health <= 0)
                        return;
        }*/

        // if multiplayer, let everyone know who hit the exit
        if (deathmatch->value)
        {
                if (activator && activator->client)
                        gi.bprintf (PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname);
        }

        // if going to a new unit, clear cross triggers
        if (strstr(self->map, "*"))     
                game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);

        BeginIntermission (self);
}

void SP_target_changelevel (edict_t *ent)
{
        if (!ent->map)
        {
                gi.dprintf("REMOVED: target_changelevel with no map at %s\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        ent->use = use_target_changelevel;
        ent->svflags = SVF_NOCLIENT;
}

void SP_target_buildpoints (edict_t *ent)
{
        if (!ent->count)
        {
                gi.dprintf ("REMOVED: target_buildpoints with no count at %s\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        if (ent->dmg != 1 || ent->dmg != 2)
        {
                gi.dprintf ("REMOVED: target_buildpoints with invalid 'dmg' at %s\n", vtos2(ent));
                G_FreeEdict (ent);
                return;
        }

        ent->use = use_target_buildpoints;
        ent->svflags = SVF_NOCLIENT;
}

void SP_target_xmins (edict_t *ent)
{
        if (CheckTargetEnt (ent))
                return;

        ent->svflags = SVF_NOCLIENT;
}

/*QUAKED target_splash (1 0 0) (-8 -8 -8) (8 8 8)
Creates a particle splash effect when used.

Set "sounds" to one of the following:
  1) sparks
  2) blue water
  3) brown water
  4) slime
  5) lava
  6) blood

"count" how many pixels in the splash
"dmg"   if set, does a radius damage at this location when it splashes
                useful for lava/sparks
*/

void use_target_splash (edict_t *self, edict_t *other, edict_t *activator)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_SPLASH);
        gi.WriteByte (self->count);
        gi.WritePosition (self->s.origin);
        gi.WriteDir (self->movedir);
        gi.WriteByte (self->sounds);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        if (self->dmg)
                T_RadiusDamage (self, activator, self->dmg, NULL, self->dmg+40, MOD_SPLASH, 0);
}

void SP_target_splash (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;
        self->use = use_target_splash;
        G_SetMovedir (self->s.angles, self->movedir);

        if (!self->count)
                self->count = 32;

        self->svflags = SVF_NOCLIENT;
}

//r1: makes a copy of ->target

void ED_CallSpawn (edict_t *ent);
void use_target_cloner (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *targ = NULL;
        edict_t *ent;

        targ = G_Find (targ, FOFS(targetname), self->target);

        //targ went away !
        if (!targ) {
                G_FreeEdict (self);
                gi.dprintf ("REMOVED: target_cloner at %s lost its target\n",vtos2(self));
                return;
        }

        ent = G_Spawn();
        ent->classname = targ->classname;
        ent->target = targ->target;
        ent->pathtarget = targ->pathtarget;
        ent->accel = targ->accel;
        ent->decel = targ->decel;
        ent->combattarget = targ->combattarget;
        ent->deathtarget = targ->deathtarget;
        ent->killtarget = targ->killtarget;
        ent->targetname = targ->targetname;
        ent->style = targ->style;
        ent->hurtflags = targ->hurtflags;
        ent->spawnflags = targ->spawnflags;
        VectorCopy (self->s.origin, ent->s.origin);
        VectorCopy (targ->s.angles, ent->s.angles);
        //self->target = ent->targetname;
        //G_FreeEdict (targ);
        ED_CallSpawn (ent);
        gi.unlinkentity (ent);
        KillBox (ent);
        gi.linkentity (ent);
}

void SP_target_cloner (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;
        if (CheckTargetEnt (self))
                return;

        self->use = use_target_cloner;
        self->svflags = SVF_NOCLIENT;

        if (self->speed)
        {
                G_SetMovedir (self->s.angles, self->movedir);
                VectorScale (self->movedir, self->speed, self->movedir);
        }
}

void use_target_broadcast (edict_t *self, edict_t *other, edict_t *activator)
{
        if (!activator->inuse || !activator->client)
        {
                gi.dprintf ("WARNING: target_broadcast used with bad activator!\n");
                return;
        }

        if (self->count == 0)
        {
                if (self->style == 1)
                        gi.bprintf (PRINT_HIGH, "%s %s\n", activator->client->pers.netname, self->message);
                else
                        gi.bprintf (PRINT_HIGH, "%s\n", self->message);
        }
        else
        {
                if (self->style == 1)
                        centerprint_all (va("%s %s\n", activator->client->pers.netname, self->message));
                else
                        centerprint_all (va("%s\n", self->message));
        }
}

void SP_target_broadcast (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;

        self->use = use_target_broadcast;
        self->svflags = SVF_NOCLIENT;
}

/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg             default is 15
speed   default is 1000
*/

void use_target_blaster (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *grenade;
        vec3_t firedir;
        VectorCopy (self->movedir, firedir);
        firedir[0] += crandom() * self->random;
        firedir[1] += crandom() * self->random;
        firedir[2] += crandom() * self->random;

        if (mapdebugmode->value)
                gi.dprintf ("[blaster] target_blaster {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        switch (self->spawnflags) {
                default:
                case 1:
                        fire_blaster (self, self->s.origin, firedir, self->dmg, self->speed, EF_BLASTER, ENT_TARGET_BLASTER);
                        break;
                case 2:
                        fire_acid (self, self->s.origin, firedir, self->speed);
                        break;
                case 3:
                        fire_bullet (self, self->s.origin, firedir, self->dmg, self->speed, 0, 0, MOD_MGTURRET);
                        break;
                case 4:
                        fire_explosive (self, self->s.origin, firedir, self->dmg, self->speed, TE_EXPLOSION1, 0, 0, MOD_EXPLOSIVE);
                        break;
                case 5:
                        fire_fire (self, self->s.origin, firedir, self->dmg);
                        break;
                case 6:
                        fire_grenade (self, self->s.origin, firedir, self->dmg, self->speed, self->delay, 400);
                        break;
                case 7:
                        fire_spike (self, self->s.origin, firedir, self->dmg, self->speed);
                        break;
                case 8:
                        fire_web (self, self->s.origin, firedir, self->speed);
                        break;
                case 9:
                        fire_rocket (self, self->s.origin, firedir, self->dmg, self->speed, 300, 370);
                        break;
                case 10:
                        grenade = fire_grenade (self, self->s.origin, firedir, self->dmg, self->speed, self->delay, 400);
                        grenade->s.sound = 0;
                        grenade->classname = "fgrenade";
                        grenade->enttype = ENT_FLASH_GRENADE;
                        break;
        }
        
        gi.sound (self, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
}

void SP_target_blaster (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;
        self->use = use_target_blaster;
        G_SetMovedir (self->s.angles, self->movedir);

        if (self->spawnflags & 128) {
                self->spawnflags &= ~128;
        } else {
                switch (self->spawnflags) {
                        case 1:
                                self->noise_index = SoundIndex (weapons_laser2);
                                break;
                        case 2:
                        case 8:
                                self->noise_index = SoundIndex (weapons_webshot1);
                                break;
                        case 3:
                                self->noise_index = SoundIndex (weapons_machgf1b);
                                break;
                        case 4:
                        case 7:
                                break;
                        case 5:
                                self->noise_index = SoundIndex (weapons_flame);
                                break;
                        case 6:
                                self->noise_index = SoundIndex (weapons_grenlf1a);
                                break;
                        case 9:
                                self->noise_index = SoundIndex (weapons_rocklf1a);
                                break;
                        default:
                                break;
                }
        }
        
        if (!self->dmg) {
                self->dmg = 100;
        }

        if (!self->speed) {
                self->speed = 1000;
        }


        if (self->spawnflags == 6) {
                if (!self->delay) {
                        gi.dprintf ("WARNING: %s without delay set at %s\n", self->classname, vtos (self->s.origin));
                        self->delay = 3;
                }
        }

        self->svflags = SVF_NOCLIENT;
}

/*QUAKED target_crosslevel_trigger (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Once this trigger is touched/used, any trigger_crosslevel_target with the same trigger number is automatically used when a level is started within the same unit.  It is OK to check multiple triggers.  Message, delay, target, and killtarget also work.
*/
void trigger_crosslevel_trigger_use (edict_t *self, edict_t *other, edict_t *activator)
{
        game.serverflags |= self->spawnflags;
        G_FreeEdict (self);
}

void SP_target_crosslevel_trigger (edict_t *self)
{
        self->svflags = SVF_NOCLIENT;
        self->use = trigger_crosslevel_trigger_use;
}

/*QUAKED target_crosslevel_target (.5 .5 .5) (-8 -8 -8) (8 8 8) trigger1 trigger2 trigger3 trigger4 trigger5 trigger6 trigger7 trigger8
Triggered by a trigger_crosslevel elsewhere within a unit.  If multiple triggers are checked, all must be true.  Delay, target and
killtarget also work.

"delay"         delay before using targets if the trigger has been activated (default 1)
*/
void target_crosslevel_target_think (edict_t *self)
{
        if (self->spawnflags == (game.serverflags & SFL_CROSS_TRIGGER_MASK & self->spawnflags))
        {
                G_UseTargets (self, self);
                G_FreeEdict (self);
        }
}

void SP_target_crosslevel_target (edict_t *self)
{
        if (! self->delay)
                self->delay = 1;
        self->svflags = SVF_NOCLIENT;

        self->think = target_crosslevel_target_think;
        self->nextthink = level.time + self->delay;
}

/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/

void target_laser_think (edict_t *self)
{
        edict_t *ignore;
        vec3_t  start;
        vec3_t  end;
        trace_t tr;
        vec3_t  point;
        vec3_t  last_movedir;
        int             count;
        int haxes = 0;

        if (self->spawnflags & 0x80000000)
                count = 8;
        else
                count = 4;

        if (self->enemy)
        {
                VectorCopy (self->movedir, last_movedir);
                VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
                VectorSubtract (point, self->s.origin, self->movedir);
                VectorNormalize (self->movedir);
                if (!VectorCompare(self->movedir, last_movedir))
                        self->spawnflags |= 0x80000000;
        }

        ignore = self;
        VectorCopy (self->s.origin, start);
        VectorMA (start, 2048, self->movedir, end);

        tr = gi.trace (start, NULL, NULL, end, ignore, MASK_MONSTERSOLID);
        while(1)
        {
                if (!tr.ent || ++haxes > 20)
                        break;

                // hurt it if we can
                if (tr.ent->takedamage) {
                        if (tr.ent->client && self->style) {
                                if (self->style & classtypebit[tr.ent->client->resp.class_type]) {
                                        ignore = tr.ent;
                                        VectorCopy (tr.endpos, start);
                                        continue;
                                }
                        }
                        T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_REAL_TARGET_LA);
                        if (self->spawnflags & 128) {
                                ignore = tr.ent;
                                VectorCopy (tr.endpos, start);
                                continue;
                        } else {
                                break;
                        }
                }

                // if we hit something that's not a monster or player or is immune to lasers, we're done
                if (!tr.ent->client)
                {
                        if (self->spawnflags & 0x80000000)
                        {
                                self->spawnflags &= ~0x80000000;
                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_LASER_SPARKS);
                                gi.WriteByte (count);
                                gi.WritePosition (tr.endpos);
                                gi.WriteDir (tr.plane.normal);
                                gi.WriteByte (self->s.skinnum & 0xFF);
                                gi.multicast (tr.endpos, MULTICAST_PVS);
                        }
                        break;
                }

                ignore = tr.ent;
                VectorCopy (tr.endpos, start);

                tr = gi.trace (start, NULL, NULL, end, ignore, MASK_MONSTERSOLID);
        }

        VectorCopy (tr.endpos, self->s.old_origin);

        self->nextthink = level.time + FRAMETIME;
}

void target_laser_on (edict_t *self)
{
        if (!self->activator)
                self->activator = self;
        self->spawnflags |= 0x80000001;
        self->svflags &= ~SVF_NOCLIENT;
        target_laser_think (self);
}

void target_laser_off (edict_t *self)
{
        self->spawnflags &= ~1;
        self->svflags |= SVF_NOCLIENT;
        self->nextthink = 0;
}

void target_laser_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (mapdebugmode->value)
                gi.dprintf ("[laser  ] target_laser {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        self->activator = activator;
        if (self->spawnflags & 1)
                target_laser_off (self);
        else
                target_laser_on (self);
}

void target_laser_start (edict_t *self)
{
        edict_t *ent;

        self->movetype = MOVETYPE_NONE;
        self->solid = SOLID_NOT;
        self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
        self->s.modelindex = 1;                 // must be non-zero

        // set the beam diameter
        if (self->spawnflags & 64)
                self->s.frame = 16;
        else
                self->s.frame = 4;

        // set the color
        if (self->spawnflags & 2)
                self->s.skinnum = 0xf2f2f0f0;
        else if (self->spawnflags & 4)
                self->s.skinnum = 0xd0d1d2d3;
        else if (self->spawnflags & 8)
                self->s.skinnum = 0xf3f3f1f1;
        else if (self->spawnflags & 16)
                self->s.skinnum = 0xdcdddedf;
        else if (self->spawnflags & 32)
                self->s.skinnum = 0xe0e1e2e3;

        if (!self->enemy)
        {
                if (self->target)
                {
                        ent = G_Find (NULL, FOFS(targetname), self->target);
                        if (!ent)
                                gi.dprintf ("WARNING: %s at %s: %s is a bad target\n", self->classname, vtos2(self), self->target);
                        self->enemy = ent;
                }
                else
                {
                        G_SetMovedir (self->s.angles, self->movedir);
                }
        }
        self->use = target_laser_use;
        self->think = target_laser_think;

        VectorSet (self->mins, -8, -8, -8);
        VectorSet (self->maxs, 8, 8, 8);
        gi.linkentity (self);

        if (self->spawnflags & 1)
                target_laser_on (self);
        else
                target_laser_off (self);
}

void SP_target_laser (edict_t *self)
{
        // let everything else get spawned before we start firing
        self->think = target_laser_start;
        self->nextthink = level.time + 1;
        self->style = st.classes;
}

/*QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
speed           How many seconds the ramping will take
message         two letters; starting lightlevel and ending lightlevel
*/

void target_lightramp_think (edict_t *self)
{
        char    style[2];

        style[0] = 'a' + self->movedir[0] + (level.time - self->timestamp) / FRAMETIME * self->movedir[2];
        style[1] = 0;
        gi.configstring (CS_LIGHTS+self->enemy->style, style);

        if ((level.time - self->timestamp) < self->speed)
        {
                self->nextthink = level.time + FRAMETIME;
        }
        else if (self->spawnflags & 1)
        {
                char    temp;

                temp = self->movedir[0];
                self->movedir[0] = self->movedir[1];
                self->movedir[1] = temp;
                self->movedir[2] *= -1;
        }
}

void target_lightramp_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (!self->enemy)
        {
                edict_t         *e;

                // check all the targets
                e = NULL;
                for (;;)
                {
                        e = G_Find (e, FOFS(targetname), self->target);
                        if (!e)
                                break;
                        if (strcmp(e->classname, "light") != 0)
                        {
                                gi.dprintf("WARNING: %s at %s ", self->classname, vtos2(self));
                                gi.dprintf("WARNING: target %s (%s at %s) is not a light\n", self->target, e->classname, vtos(e->s.origin));
                        }
                        else
                        {
                                self->enemy = e;
                        }
                }

                if (!self->enemy)
                {
                        gi.dprintf("REMOVED: %s target %s not found at %s\n", self->classname, self->target, vtos2(self));
                        G_FreeEdict (self);
                        return;
                }
        }

        self->timestamp = level.time;
        target_lightramp_think (self);
}

void SP_target_lightramp (edict_t *self)
{
        if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1])
        {
                gi.dprintf("REMOVED: target_lightramp has bad ramp (%s) at %s\n", self->message, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (deathmatch->value)
        {
                G_FreeEdict (self);
                return;
        }

        if (!self->target)
        {
                gi.dprintf("REMOVED: %s with no target at %s\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->svflags |= SVF_NOCLIENT;
        self->use = target_lightramp_use;
        self->think = target_lightramp_think;

        self->movedir[0] = self->message[0] - 'a';
        self->movedir[1] = self->message[1] - 'a';
        self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
}

/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"         severity of the quake (default:200)
"count"         duration of the quake (default:5)
*/

void target_earthquake_think (edict_t *self)
{
        int             i;
        edict_t *e;
 
        if (self->air_finished < level.time)
        {
                gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
                self->air_finished = level.time + 0.5f;
        }

        for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
        {
                if (!e->inuse)
                        continue;
                if (!e->client && !(self->spawnflags & 1))
                        continue;

                if (!e->groundentity)
                        continue;

                e->groundentity = NULL;
                e->velocity[0] += crandom()* self->accel;
                e->velocity[1] += crandom()* self->accel;
                e->velocity[2] = self->speed * (100 / e->mass);
                if ((e->flags & FL_CLIPPING) && (e->enttype == ENT_TURRET || e->enttype == ENT_MGTURRET) && e->target_ent) {
                        e->s.origin[0] = e->target_ent->s.origin[0];
                        e->s.origin[1] = e->target_ent->s.origin[1];
                        e->velocity[1] = e->velocity[0] = 0;
                        continue;
                }
        }

        if (level.time < self->timestamp)
                self->nextthink = level.time + FRAMETIME;
}

void target_earthquake_use (edict_t *self, edict_t *other, edict_t *activator)
{
        self->timestamp = level.time + self->count;
        self->nextthink = level.time + FRAMETIME;
        self->activator = activator;
        self->air_finished = 0;
}

void SP_target_earthquake (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;

        if (!self->count)
                self->count = 5;

        if (!self->speed)
                self->speed = 200;

        if (!self->accel)
                self->accel = 150;

        self->svflags |= SVF_NOCLIENT;
        self->think = target_earthquake_think;
        self->use = target_earthquake_use;

        self->noise_index = SoundIndex (world_quake);
}
