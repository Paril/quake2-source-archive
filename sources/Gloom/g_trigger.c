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
//FIXME: a lot of the trigger_ prefixes are wrong. The ents act more like targets...

#define ToggleTrigger(x) \
        if (x->solid == SOLID_NOT) \
                x->solid = SOLID_TRIGGER; \
        else \
                x->solid = SOLID_NOT;\
        gi.linkentity (x);

//FIXME: in the mapeditor it is possible to intend 'angle 0' meaning to move ->
//       on things like trigger_push - however this vectorcompare will then
//       not call G_SetMovedir and the resultant movedir is 0 0 0, which breaks.
//       is it safe to remove this vectorcompare? i have a bad feeling a lot of
//       stuff will break if it's gone...

//if (!VectorCompare (self->s.angles, vec3_origin))

/*#define InitTrigger(self) \
        G_SetMovedir (self->s.angles, self->movedir); \
        self->solid = SOLID_TRIGGER; \
        self->movetype = MOVETYPE_NONE; \
        gi.setmodel (self, self->model); \
        self->svflags = SVF_NOCLIENT; \*/

//r1: shared function for lots of triggers
void toggle_trigger_use (edict_t *ent, edict_t *other, edict_t *activator){ToggleTrigger (ent);}

void InitTrigger (edict_t *self)
{
        //if (!VectorCompare (self->s.angles, vec3_origin))
                G_SetMovedir (self->s.angles, self->movedir);

        self->solid = SOLID_TRIGGER;
        self->movetype = MOVETYPE_NONE;
        gi.setmodel (self, self->model);
        self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait (edict_t *ent)
{
        ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger (edict_t *ent)
{
        if (ent->nextthink)
                return;         // already been triggered
        /*
        if (ent->random)
                gi.bprintf(PRINT_HIGH,"ent->random: %u ",(long)ent->random, ent->classname, ent->activator->client->resp.class_type, classtypebit[ent->activator->client->resp.class_type]);
        if (ent->classname)
                gi.bprintf(PRINT_HIGH,"ent->classname: %s ", ent->classname);
        if (ent->activator->classname)
                gi.bprintf(PRINT_HIGH,"ent->activator->client->resp.class_type: %d ", ent->activator->client->resp.class_type);
        if (classtypebit[ent->activator->client->resp.class_type])
                gi.bprintf(PRINT_HIGH,"classtypebit: %u", classtypebit[ent->activator->client->resp.class_type]);
        */
        if (ent->style)
                if (ent->activator && ent->activator->client && !((ent->style) & classtypebit[ent->activator->client->resp.class_type]))
                        return;
        //gi.bprintf(PRINT_HIGH,"Triggered\n");

        if (mapdebugmode->value)
                gi.dprintf ("[t_multi] %s {%s} triggered by %s {%s} (TARGET: %s)\n", ent->classname, ent->targetname, ent->activator->classname, ent->activator->targetname, ent->target);

        G_UseTargets (ent, ent->activator);

        if (ent->count && ent->activator->client) {
                ent->activator->client->resp.score += ent->count;

                if(ent->activator->client->resp.score > MAX_SCORE->value)
                        ent->activator->client->resp.score = MAX_SCORE->value;

                if (ent->spawnflags & 8) {
                        ent->count = 0;
                        if (!ent->target) {
                                ent->touch = NULL;
                                ent->nextthink = level.time + FRAMETIME;
                                ent->think = G_FreeEdict;
                        }
                }
        }

        if (ent->wait > 0)
        {
                ent->think = multi_wait;
                ent->nextthink = level.time + ent->wait;
        }
        else
        {       // we can't just remove (self) here, because this is a touch function
                // called while looping through area links...
                ent->touch = NULL;
                ent->nextthink = level.time + FRAMETIME;
                ent->think = G_FreeEdict;
        }
}

void Use_Multi (edict_t *ent, edict_t *other, edict_t *activator)
{
        ent->activator = activator;
        multi_trigger (ent);
}

void Touch_Multi (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if(other->client)
        {
                if (self->spawnflags & 2)
                        return;
        }
        else if (other->svflags & SVF_MONSTER)
        {
                if (!(self->spawnflags & 1))
                        return;
        }
        else
                return;

        if (!VectorCompare(self->movedir, vec3_origin))
        {
                vec3_t  forward;

                AngleVectors(other->s.angles, forward, NULL, NULL);
                if (_DotProduct(forward, self->movedir) < 0)
                        return;
        }

        self->activator = other;
        multi_trigger (self);
}

void multiple_counter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!other->client) {

                //humie structures can touch me if im spawnflag 4
                if (other->flags & FL_CLIPPING && !(self->spawnflags & 4))
                        return;

                //spider structures can touch me if im spawnflag 8
                if (other->svflags & SVF_MONSTER && !(self->spawnflags & 8))
                        return;
        } else {

                //clients can't touch me if im spawnflag 16
                if (self->spawnflags & 16)
                        return;

                //check the client classes
                if (self->style)
                        if (!((self->style) & classtypebit[other->client->resp.class_type]))
                                return;
        }

        //im still waiting for 'wait' before firing again
        if (self->damage_debounce_time > level.time)
                return;

        // its the next frame, everyone that wanted to touch us has
        if (self->touch_debounce_time < level.framenum) {

                //check if we were touched enough last frame
                if (self->mass >= self->count) {

                        //we were, lets see if it is time to fire out targets
                        if (level.time - self->teleport_time > self->accel) {

                                //fire!
                                G_UseTargets (self, self);

                                //only fire once if spawnflag 2
                                if (self->spawnflags & 2) {
                                        self->touch = NULL;
                                        self->nextthink = level.time + FRAMETIME;
                                        self->think = G_FreeEdict;
                                        return;
                                }

                                //reset some counters
                                self->damage_debounce_time = level.time + self->wait;
                                self->teleport_time = level.time;
                        }
                } else {

                        //we weren't touched enough, reset out fire target timer
                        self->teleport_time = level.time;
                }

                //reset me for next frame
                self->mass = 0;
                self->touch_debounce_time = level.framenum;
        }

        //this thing has already touched me this frame!
        if (other->timestamp > level.framenum)
                return;

        other->timestamp = level.framenum + .1f;

        //touch me
        self->mass++;
}

//trigger_multi that fires when 'count' players are within the
//entity for 'delay' seconds. wait 'wait' before firing again

void SP_trigger_multiple_counter (edict_t *self)
{
        if (CheckTargetEnt(self))
                return;

        //count 1 shouldn't happen, messes up the timing.
        if (self->count == 1)
                gi.dprintf ("WARNING: %s at %s has bad count (should be > 1)\n", self->classname, vtos2(self));

        self->spawnflags = 8;

        InitTrigger (self);

        self->touch = multiple_counter_touch;
        self->use = toggle_trigger_use;

        if (!(self->spawnflags & 4) && !(self->spawnflags & 8) && self->spawnflags & 16) {
                gi.dprintf ("REMOVED: %s at %s isn't triggerable by anything!\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        //start off if spawnflag 1
        if (self->spawnflags & 1) {
                self->solid = SOLID_NOT;
                if (!self->targetname) {
                        gi.dprintf ("REMOVED: %s at %s with START_OFF but no targetname\n", self->classname, self->s.origin);
                        G_FreeEdict (self);
                        return;
                }
        }

        self->style = st.classes;

        gi.linkentity (self);
}

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)      secret
2)      beep beep
3)      large switch
4)
set "message" to text string
*/
void trigger_enable (edict_t *self, edict_t *other, edict_t *activator)
{
        self->solid = SOLID_TRIGGER;
        self->use = Use_Multi;
        gi.linkentity (self);
}

void SP_trigger_multiple (edict_t *ent)
{
        if (!ent->count && !ent->message && CheckTargetEnt (ent))
                return;

        if (ent->sounds == 1)
                ent->noise_index = SoundIndex (misc_secret);
        else if (ent->sounds == 2)
                ent->noise_index = SoundIndex (misc_talk);

        if (!ent->wait)
                ent->wait = 0.2;
        ent->touch = Touch_Multi;
        ent->movetype = MOVETYPE_NONE;
        ent->svflags |= SVF_NOCLIENT;

        ent->style = st.classes;


        if (ent->spawnflags & 4)
        {
                ent->solid = SOLID_NOT;
                ent->use = trigger_enable;
        }
        else
        {
                ent->solid = SOLID_TRIGGER;
                ent->use = Use_Multi;
        }

        if (!VectorCompare(ent->s.angles, vec3_origin))
                G_SetMovedir (ent->s.angles, ent->movedir);

        gi.setmodel (ent, ent->model);
        gi.linkentity (ent);
}


/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)     secret
 2)     beep beep
 3)     large switch
 4)

"message"       string to be displayed when triggered
*/

void SP_trigger_once(edict_t *ent)
{
        // make old maps work because I messed up on flag assignments here
        // triggered was on bit 1 when it should have been on bit 4
        if (ent->spawnflags & 1)
        {
                vec3_t  v;

                VectorMA (ent->mins, 0.5, ent->size, v);
                ent->spawnflags &= ~1;
                ent->spawnflags |= 4;
                gi.dprintf("WARNING: fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
        }

        ent->style = st.classes;

        ent->wait = -1;
        SP_trigger_multiple (ent);
}

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
        edict_t *t = NULL;
        qboolean        hax = false;

        if (self->style)
                if (activator && activator->client && !((self->style) & classtypebit[activator->client->resp.class_type]))
                        return;

        if (self->team) {
                while ((t = G_Find (t, FOFS(targetname), self->team))) {
                        if (!t->max_health) {
                                continue;
                        } else {
                                hax = true;
                                break;
                        }
                }

                if (!hax)
                        return;
        }

        self->max_health ^= 1;

        if (random() > self->random)
                return;

        if (mapdebugmode->value)
                gi.dprintf ("[relay  ] trigger_relay {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        if (self->count && activator->client) {

                // if spawnflags & 8, check against the frags
                if (self->spawnflags & 8) {
                        if (self->count > activator->client->resp.score) {
                                return;
                        }

                        if (self->spawnflags & 2)
                                activator->client->resp.score -= self->count;
                } else {
                        activator->client->resp.score += self->count;

                        if(activator->client->resp.score > MAX_SCORE->value)
                                activator->client->resp.score = MAX_SCORE->value;

                        if (self->spawnflags & 2)
                                self->count = 0;
                }
        }

        if (self->health && activator->client) {
                activator->client->resp.total_score += self->health;

                if (self->spawnflags & 4)
                        self->health = 0;
        }

        G_UseTargets (self, activator);
}

void SP_trigger_relay (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;

        if (!self->random)
                self->random = 1;

        self->svflags |= SVF_NOCLIENT;

        self->style = st.classes;
        self->use = trigger_relay_use;
}

void trigger_multistate_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
        char *temptarget;

        if (mapdebugmode->value)
                gi.dprintf ("[msrelay] trigger_multistate_relay {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        if (!self->count) {
                G_UseTargets (self, activator);
                self->count = 1;
        } else {
                temptarget = self->target;
                self->target = self->pathtarget;
                G_UseTargets (self, activator);
                self->target = temptarget;
                self->count = 0;
        }
}

void SP_trigger_multistate_relay (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;

        if (!self->pathtarget)
        {
                gi.dprintf ("REMOVED: %s without a pathtarget at %s\n",self->classname,vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (!self->target)
        {
                gi.dprintf ("REMOVED: %s without a target at %s\n",self->classname,vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (self->count > 1 || self->count < 0)
        {
                gi.dprintf ("REMOVED: %s without invalid count (should be 0 or 1) at %s\n",self->classname,vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->svflags |= SVF_NOCLIENT;

        self->use = trigger_multistate_relay_use;
}

void target_random_use (edict_t *ent, edict_t *other, edict_t *activator)
{
        edict_t *t;
        int nTargets=0;
        edict_t *targets[10]={NULL};
        if (ent->target)
        {
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->target)))
                {
                        // doors fire area portals in a specific way
                        if (t->enttype == ENT_AREAPORTAL &&
                                (ent->enttype == ENT_FUNC_DOOR || !Q_stricmp(ent->classname, "func_door_rotating")))
                                continue;

                        if (t == ent)
                        {
                                gi.dprintf (va("WARNING: Entity used itself.\n%s - %s\n",t->classname,t->targetname));
                        }else{
                                targets[nTargets]=t;
                                nTargets++;
                                if (nTargets==10)
                                        break;
                        }
                }
        }
        if (nTargets){
                int ourTarget;
                //srand( (unsigned)time( NULL ) );
                ourTarget= random() * nTargets;
                if (targets[ourTarget]->use)
                        targets[ourTarget]->use (targets[ourTarget], ent, activator);
        }else{
                gi.dprintf (va("WARNING: target_random has no targets.\n%s\n",ent->target));
        }
}

void SP_target_random (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;
        self->use = target_random_use;
}

void SP_trigger_random (edict_t *self)
{
        gi.dprintf ("WARNING: trigger_random is deprecated, use target_random instead.\n");
        SP_target_random (self);
}

/*

trigger_key

*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void target_key_use (edict_t *self, edict_t *other, edict_t *activator)
{
        if (!self->item)
                return;

        if (!activator->client)
                return;

        if (!activator->client->resp.key || activator->client->resp.key->item != self->item)
        {
                if (level.time < self->touch_debounce_time)
                        return;
                self->touch_debounce_time = level.time + 5;
                gi.centerprintf (activator, "You need the %s", self->item->pickup_name);
                gi.sound (activator, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_NORM, 0);
                return;
        }

        gi.sound (activator, CHAN_AUTO, SoundIndex (misc_keyuse), 1, ATTN_NORM, 0);

        activator->client->resp.key = NULL;

        G_UseTargets (self, activator);

        if (self->spawnflags & 1)
                self->use = NULL;
}

void SP_target_key (edict_t *self)
{
        if (!st.item)
        {
                gi.dprintf("REMOVED: no key item for trigger_key at %s\n", vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->item = FindItemByClassname (st.item);

        if (!self->item)
        {
                gi.dprintf("REMOVED: item %s not found for trigger_key at %s\n", st.item, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (!self->target)
        {
                gi.dprintf("WARNING: %s at %s has no target\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        SoundIndex (misc_keytry);
        SoundIndex (misc_keyuse);

        self->use = target_key_use;
}

void SP_trigger_key (edict_t *self)
{
        gi.dprintf ("WARNING: trigger_key is deprecated, use target_key instead.\n");
        SP_target_key (self);
}

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
        if (mapdebugmode->value)
                gi.dprintf ("[counter] trigger_counter {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        if (self->count == 0)
                return;

        self->count--;

        if (self->count)
        {
                if (! (self->spawnflags & 1) && activator->client)
                {
                        gi.centerprintf(activator, "%i more to go...", self->count);
                        gi.sound (activator, CHAN_AUTO, SoundIndex (misc_talk1), 1, ATTN_NORM, 0);
                }
                return;
        }

        if (! (self->spawnflags & 1) && activator->client)
        {
                gi.centerprintf(activator, "Sequence completed!");
                gi.sound (activator, CHAN_AUTO, SoundIndex (misc_talk1), 1, ATTN_NORM, 0);
        }
        self->activator = activator;
        G_UseTargets (self, activator);
        G_FreeEdict (self);
}

void SP_trigger_counter (edict_t *self)
{
        if (CheckTargetEnt (self))
                return;
        if (CheckTargetnameEnt (self))
                return;
        self->wait = -1;
        if (!self->count) {
                self->count = 2;
                gi.dprintf ("WARNING: trigger_counter without count, defaulting to 2.\n");
        }

        self->use = trigger_counter_use;
}

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (edict_t *ent)
{
        if (CheckTargetEnt (ent))
                return;
        // we must have some delay to make sure our use targets are present
        if (ent->delay < 0.2)
                ent->delay = 0.2;

        ent->svflags |= SVF_NOCLIENT;

        G_UseTargets(ent, ent);
}

#define PUSH_ONCE               1

int windsound;

void trigger_push_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        // FIXME: what's this?
        if (!self->style && (other->enttype == ENT_CORPSE || other->enttype == ENT_HUMAN_BODY || other->enttype == ENT_ROCKET || other->enttype == ENT_GRENADE || other->enttype == ENT_C4 || other->enttype == ENT_SPIKE_SPORE || other->enttype == ENT_GAS_SPORE || other->enttype == ENT_PROXY_SPORE || other->enttype == ENT_AMMO_DEPOT || (other->svflags & SVF_MONSTER)))
        {
                VectorScale (self->movedir, self->speed * 10, other->velocity);
        }
        else if (other->health > 0)
        {
                if (other->client)
                {

                        if (self->style)
                                if (!((self->style) & classtypebit[other->client->resp.class_type]))
                                        return;

                        VectorScale (self->movedir, self->speed * 10, other->velocity);

                        // don't take falling damage immediately from this
                        VectorCopy (other->velocity, other->client->oldvelocity);
                        if (!(self->spawnflags & 2) && other->fly_sound_debounce_time < level.framenum)
                        {
                                other->fly_sound_debounce_time = level.framenum + 15;
                                gi.sound (other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
                        }
                } else {
                        VectorScale (self->movedir, self->speed * 10, other->velocity);
                }
        }

        if (self->spawnflags & PUSH_ONCE) {
                self->touch = NULL;
                self->nextthink = level.time + FRAMETIME;
                self->think = G_FreeEdict;
        }
}

/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"         defaults to 1000
*/
void SP_trigger_push (edict_t *self)
{
        InitTrigger (self);
        windsound = SoundIndex (misc_windfly);
        self->touch = trigger_push_touch;

        if (!self->speed)
                self->speed = 1000;

        self->use = toggle_trigger_use;
        self->style = st.classes;

        if (self->spawnflags & 4)
                self->solid = SOLID_NOT;
        else
                self->solid = SOLID_TRIGGER;

        gi.linkentity (self);
}

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT                  supresses playing the sound
SLOW                    changes the damage rate to once per second
NO_PROTECTION   *nothing* stops the damage

"dmg"                   default 5 (whole numbers only)

*/
void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int             dflags = 0;

        //gi.dprintf ("%s touched me (%s)\n", other->classname, self->classname);

        if (!other->takedamage)
                return;

        //if (self->timestamp > level.time)
        //      return;

        if (!Q_stricmp(self->classname, "trigger_hurt_humans")){
                if (other->client)
                        if (other->client->resp.team != TEAM_HUMAN || (self->spawnflags & 32))
                                return;
                if (other->svflags & SVF_MONSTER)
                        return;
        }

        if (!Q_stricmp(self->classname, "trigger_hurt_aliens")){
                if (other->client)
                        if (other->client->resp.team!=TEAM_ALIEN || (self->spawnflags & 32))
                                return;
                if (other->enttype == ENT_TRIPWIRE_BOMB)
                        return;
                if (other->flags & FL_CLIPPING)
                        return;
        }

        if (other->client) {
                if (self->style)
                        if (!((self->style) & classtypebit[other->client->resp.class_type]))
                                return;
                if (other->timestamp > level.time)
                        return;
                other->timestamp = level.time + .1f;
        } else {
                if (self->style && !(self->spawnflags & 64)) {
                        return;
                }
        }

        if (self->spawnflags & 16)
                self->timestamp = level.time + 1;
        else
                self->timestamp = level.time + FRAMETIME;


        if (!(self->spawnflags & 4))
        {
                if ((level.framenum % 10) == 0)
                        gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
        }

        if (self->spawnflags & 8)
                dflags |= DAMAGE_IGNORE_RESISTANCES;

        dflags |= DAMAGE_NO_PROTECTION;

        T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt (edict_t *self)
{
        InitTrigger (self);

        self->noise_index = SoundIndex (world_electro);
        self->touch = hurt_touch;

        if (!self->dmg)
                self->dmg = 5;

        if (self->spawnflags & 1)
                self->solid = SOLID_NOT;
        else
                self->solid = SOLID_TRIGGER;

        if (self->spawnflags & 2)
                self->use = toggle_trigger_use;

        if (self->dmg < 1) {
                gi.dprintf ("REMOVED: trigger_hurt with no (or negative) damage at %s.\n", vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (self->spawnflags == 1) {
                gi.dprintf ("REMOVED: bad trigger_hurt with START_OFF but not TOGGLE at %s\n",vtos2(self));
                G_FreeEdict (self);
                return;
        }

        self->style = st.classes;

        gi.linkentity (self);
}

void SP_trigger_hurt_humans (edict_t *self)
{
        SP_trigger_hurt(self);
}

void SP_trigger_hurt_aliens (edict_t *self)
{
        SP_trigger_hurt(self);
}

/*QUAKED trigger_gravity (.5 .5 .5) ?
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.
*/

//all haxored by R1 to work on clients.

void trigger_gravity_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (self->style) {
                if (!other->client)
                        return;
                if (!(self->style & classtypebit[other->client->resp.class_type]))
                        return;
        }
        if (other->client)
                other->client->ps.pmove.gravity = other->gravity = (int)(self->timestamp * sv_gravity->value);
        else
                other->gravity = self->timestamp;

}

void SP_trigger_gravity (edict_t *self)
{
        if (st.gravity == 0)
        {
                gi.dprintf("REMOVED: trigger_gravity without gravity set at %s\n", vtos2(self));
                G_FreeEdict  (self);
                return;
        }

        InitTrigger (self);

        self->use = toggle_trigger_use;

        self->style = st.classes;
        self->timestamp = atof(st.gravity);
        self->touch = trigger_gravity_touch;

        if (self->spawnflags & 2) {
                self->solid = SOLID_NOT;
                if (CheckTargetnameEnt(self)) {
                        G_FreeEdict (self);
                        return;
                }
        }

        gi.linkentity (self);
}

//FIXME: this is horrible.

void teleport_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        edict_t *t;
        int numtargets;
        int i;

        t = NULL;
        numtargets = 0;

        if (!other->client)
                return;

        if (self->style)
                if (other->client && !((self->style) & classtypebit[other->client->resp.class_type]))
                        return;

        while (((t = G_Find (t, FOFS(targetname), self->target)) != NULL)) {
                numtargets++;
        }

        t = NULL;
        i = (int)(random() * numtargets);
        numtargets = 0;

        while (((t = G_Find (t, FOFS(targetname), self->target)) != NULL)) {
                if (numtargets == i) {
                        edict_t *dest = t;

                        if (other->client->ctf_grapple)
                                CTFResetGrapple(other->client->ctf_grapple);

                        // unlink to make sure it can't possibly interfere with KillBox
                        gi.unlinkentity (other);

                        VectorCopy (dest->s.origin, other->s.origin);
                        VectorCopy (dest->s.origin, other->s.old_origin);
                        other->s.origin[2] += 10;

                        // clear the velocity and hold them in place briefly
                        VectorClear (other->velocity);
                        other->client->ps.pmove.pm_time = 160>>3;               // hold time
                        other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

                        // draw the teleport splash at source and on the player
                        //self->owner->s.event = EV_PLAYER_TELEPORT;
                        other->s.event = EV_PLAYER_TELEPORT;

                        // set angles
                        for (i=0 ; i<3 ; i++)
                                other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

                        VectorClear (other->s.angles);
                        VectorClear (other->client->ps.viewangles);
                        VectorClear (other->client->v_angle);

                        // kill anything at the destination
                        KillBox (other);

                        gi.linkentity (other);
                        return;
                }
                numtargets++;
        }
}

void SP_trigger_teleport (edict_t *self)
{
        if (CheckTargetEnt(self))
                return;

        InitTrigger(self);

        if (self->spawnflags & 1) {
                self->solid = SOLID_NOT;
                if (CheckTargetnameEnt(self)) {
                        return;
                }
        }

        self->use = toggle_trigger_use;

        self->style = st.classes;
        self->touch = teleport_touch;

        gi.linkentity (self);
}


void keydrop_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        edict_t *key;

        if (!other->client)
                return;

        key = other->client->resp.key;

        if (key && key->item == self->item) {
                //move the key away if desired (eg energy cube goes into convenient hole)
                if (self->style == 0) {
                        VectorCopy (self->move_origin, key->s.origin);
                        VectorCopy (self->pos2, key->s.angles);

                        key->s.effects = 0;
                        key->touch = NULL;
                        key->svflags &= ~SVF_NOCLIENT;
                        key->movetype = MOVETYPE_NONE;
                        gi.linkentity (key);
                }

                if (self->message && other->client)
                        gi.centerprintf (other, self->message);

                other->client->resp.key = NULL;

                G_UseTargets (self, other);

                self->touch = NULL;
                self->nextthink = level.time + FRAMETIME;
                self->think = G_FreeEdict;
        }
}

void SP_trigger_keydrop (edict_t *self)
{
        InitTrigger (self);

        if (!st.item)
        {
                gi.dprintf("REMOVED: no key item for trigger_keydrop at %s\n", vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (st.skyaxis)
                VectorCopy (st.skyaxis, self->pos2);

        self->item = FindItemByClassname (st.item);

        self->use = toggle_trigger_use;
        self->touch = keydrop_touch;

        gi.linkentity (self);
}

void beam_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t size, v, temp, destination;
        edict_t *dest;

        if ((self->spawnflags & 1) && !other->client)
                return;

        if (other->client && self->style && !(classtypebit[other->client->resp.class_type] & self->style))
                return;

        dest = G_Find (NULL, FOFS(targetname), self->target);
        if (!dest) {
                gi.dprintf ("beam_touch: Couldn't find destination ent\n");
                return;
        }

        VectorScale (self->size, 0.5f, size);
        VectorAdd (self->absmin, size, v);

        // unlink to make sure it can't possibly interfere with KillBox
        gi.unlinkentity (other);

        VectorSubtract (other->s.origin, v, temp);
        VectorCopy (dest->s.origin, destination);
        VectorAdd (destination, temp, destination);
        
        if (gi.pointcontents(destination) & (other->client?MASK_PLAYERSOLID:MASK_SOLID))
          return; //bad teleport destination

        VectorCopy (destination, other->s.origin);
        VectorCopy (destination, other->s.old_origin);

        // draw the teleport splash at source and on the player
        if (!(self->spawnflags & 2))
                other->s.event = EV_PLAYER_TELEPORT;

        // kill anything at the destination
        if (!(self->spawnflags & 4))
                KillBox (other);

        gi.linkentity (other);
}

void trigger_warp_use (edict_t *self, edict_t *other, edict_t *activator)
{

        if (self->spawnflags & 8) {
                ToggleTrigger (self);
        } else {
                int                     i, num;
                edict_t         *touch[MAX_EDICTS], *hit;
                num = gi.BoxEdicts (self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);
                // be careful, it is possible to have an entity in this
                // list removed before we get to it (killtriggered)
                for (i=0 ; i<num ; i++)
                {
                        hit = touch[i];

                        if (!hit->inuse)
                                continue;

                        beam_touch (self, hit, NULL, NULL);

                        if (!self->inuse)
                                break;
                }

                num = gi.BoxEdicts (self->absmin, self->absmax, touch
                        , MAX_EDICTS, AREA_TRIGGERS);

                // be careful, it is possible to have an entity in this
                // list removed before we get to it (killtriggered)
                for (i=0 ; i<num ; i++)
                {
                        hit = touch[i];

                        if (!hit->inuse)
                                continue;

                        if (!hit->enttype)
                                continue;

                        beam_touch (self, hit, NULL, NULL);

                        if (!self->inuse)
                                break;
                }
        }
}

void SP_trigger_warp (edict_t *self)
{
        if (CheckTargetEnt(self))
                return;

        InitTrigger(self);

        if (self->spawnflags & 16) {
                self->solid = SOLID_TRIGGER;
        } else {
                self->solid = SOLID_NOT;
        }

        //self->think = beam_turn_off;
        self->touch = beam_touch;
        self->use = trigger_warp_use;
        self->style = st.classes;

        gi.linkentity (self);
}

void blocker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!other->client && !(self->spawnflags & 2))
                return;

        if (!other->client) {
                VectorScale (other->velocity, -1, other->velocity);
                return;
        }

        if (!(classtypebit[other->client->resp.class_type] & self->style))
                return;

        if (self->spawnflags & 2) {
                VectorScale (other->velocity, -1, other->velocity);
                VectorCopy (other->s.old_origin, other->s.origin);
        } else {
                VectorCopy (other->s.old_origin, other->s.origin);
                VectorClear (other->velocity);
                VectorClear (other->client->oldvelocity);
                VectorClear (other->client->ps.pmove.velocity);
        }
}

void SP_trigger_blocker (edict_t *self)
{
        if (!st.classes && !(self->spawnflags & 2)) {
                gi.dprintf ("REMOVED: trigger_blocker at %s without classes\n",vtos2(self));
                G_FreeEdict (self);
                return;
        }

        InitTrigger(self);

        if (self->spawnflags & 1) {
                self->solid = SOLID_NOT;
                if (CheckTargetnameEnt(self)) {
                        G_FreeEdict (self);
                        return;
                }
        }

        self->use = toggle_trigger_use;

        self->style = st.classes;
        self->touch = blocker_touch;

        gi.linkentity (self);
}

void healer_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void trigger_healer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        float oldtime;

        if (!other->client)
                return;

        if (classtypebit[other->client->resp.class_type] & self->style)
                return;

        if (self->spawnflags & 512) {
                oldtime = other->client->healwait;
                other->client->healwait = 0;
                healer_touch (self, other, plane, surf);
                other->client->healwait = oldtime;
        } else {
                healer_touch (self, other, plane, surf);
        }
}

void SP_trigger_healer (edict_t *self)
{
        InitTrigger (self);
        if (self->spawnflags & 1 && self->spawnflags & 2 && self->spawnflags & 4) {
                gi.dprintf ("REMOVED: %s at %s with bad spawnflags (won't do anything)\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }
        if (self->spawnflags & 8) {
                self->solid = SOLID_NOT;
                if (CheckTargetnameEnt(self)) {
                        G_FreeEdict (self);
                        return;
                }
        }
        self->use = toggle_trigger_use;
        self->touch = trigger_healer_touch;

        gi.linkentity (self);
}

void depot_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void trigger_repair_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (!other->client)
                return;

        if (classtypebit[other->client->resp.class_type] & self->style)
                return;

        depot_touch (self, other, plane, surf);
}

void SP_trigger_repair (edict_t *self)
{
        InitTrigger (self);
        if (self->spawnflags & 4 && !(self->spawnflags & 2)) {
                gi.dprintf ("REMOVED: %s at %s with bad spawnflags (won't do anything)\n", self->classname, vtos2(self));
                G_FreeEdict (self);
                return;
        }

        if (self->spawnflags & 1) {
                self->solid = SOLID_NOT;
                if (CheckTargetnameEnt(self)) {
                        G_FreeEdict (self);
                        return;
                }
        }

        if (self->spawnflags & 2) {
                if (!self->count) {
                        gi.dprintf ("WARNING: %s at %s with no count, defaulting to 1\n", self->classname, vtos2(self));
                        self->count = 1;
                }
                if (!self->wait) {
                        gi.dprintf ("WARNING: %s at %s with no wait, defaulting to 0.1\n", self->classname, vtos2(self));
                        self->wait = .1;
                }
        }

        if (self->spawnflags & 2 && self->spawnflags && self->wait < 2 && !(self->spawnflags & 16))
                gi.dprintf ("WARNING: %s at %s may create a lot of noise\n", self->classname, vtos2(self));


        self->use = toggle_trigger_use;
        self->touch = trigger_repair_touch;

        gi.linkentity (self);
}


//  the thing with the number 5:
//    if fogged_alpha is more than 5, then the person is standing in fog
//    otherwise, he has left fog, so the number decreses every frame (fades away)
//  style is used to store the classes

void touch_fog (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        float fog;
        float cfog;
        vec3_t fb = {0,0,0};
        vec3_t rgb;
        int i;

        if (!other->client)
                return;
        if (!(classtypebit[other->client->resp.class_type] & self->style))
                return;
        cfog=other->client->fogged_alpha;
        VectorCopy(other->client->fog_blend,fb);
        rgb[0]=self->speed;
        rgb[1]=self->accel;
        rgb[2]=self->decel;
        fog=((float)self->mass)/100;
        if (cfog>=5.0)
                cfog-=5.0;
        if (cfog<fog){
                cfog+=0.02;
                if (cfog>fog)
                        cfog=fog;
        }
        if (cfog>fog)
                cfog-=0.02;
        cfog+=5.0;
        for (i=0;i<3;i++){
                if (fb[i]<rgb[i]){
                        fb[i]+=0.02;
                        if (fb[i]>rgb[i])
                                fb[i]=rgb[i];
                }
                if (fb[i]>rgb[i])
                        fb[i]-=0.02;
        }

        VectorCopy(fb,other->client->fog_blend);
        other->client->fogged_alpha=cfog;
}

void SP_trigger_fog (edict_t *self)
{
        vec3_t color;

    color[0]=self->speed;
        color[1]=self->accel;
        color[2]=self->decel;

        VectorNormalize(color);

    self->speed=color[0];
        self->accel=color[1];
        self->decel=color[2];

        if (self->mass>100)
                self->mass=100;

        InitTrigger(self);

        self->style = st.classes;

        if (!self->style)
                self->style = ~0;

        self->use = toggle_trigger_use;
        self->touch=touch_fog;

        if (self->spawnflags & 2) {
                self->solid = SOLID_NOT;
                gi.linkentity (self);
        }
}

void use_humanwin(edict_t *self, edict_t *other, edict_t *activator)
{
        team_info.playmode=MODE_HUMANWIN;
        team_info.leveltimer=1;
}

void use_alienwin(edict_t *self, edict_t *other, edict_t *activator)
{
        team_info.playmode=MODE_ALIENWIN;
        team_info.leveltimer=1;
}

void SP_target_team_win(edict_t *self)
{
        self->movetype = MOVETYPE_NONE;
        self->svflags = SVF_NOCLIENT;
        if (CheckTargetnameEnt (self))
                return;
}

void SP_target_humans_win(edict_t *self)
{
        SP_target_team_win(self);
        self->use = use_humanwin;
}

void SP_target_aliens_win(edict_t *self)
{
        SP_target_team_win(self);
        self->use = use_alienwin;
}

void team_lose_alien (edict_t *self, edict_t *other, edict_t *activator)
{
        team_info.lost[TEAM_ALIEN] = true;
}

void team_lose_human (edict_t *self, edict_t *other, edict_t *activator)
{
        team_info.lost[TEAM_HUMAN] = true;
}

void SP_target_humans_lose (edict_t *self)
{
        self->use = team_lose_human;
}

void SP_target_aliens_lose (edict_t *self)
{
        self->use = team_lose_alien;
}

void target_kill_use (edict_t *self, edict_t *other, edict_t *activator)
{
        int i=1;
        edict_t *t=g_edicts+1;

        if (mapdebugmode->value)
                gi.dprintf ("[targkil] target_kill {%s} was used by %s {%s}\n",self->targetname, other->classname, other->targetname);

        for ( ; t < &g_edicts[globals.num_edicts] ; i++, t++)
        {

                if (!t->inuse)
                        continue;

                if (i > 0 && i <= game.maxclients)
                {
                        if (self->spawnflags & 1 && t->client->resp.team == TEAM_ALIEN) 
                        {
                                T_Damage (t,self,self,vec3_origin,vec3_origin,vec3_origin,10000,100,DAMAGE_NO_PROTECTION,MOD_TARGET_KILL);
                        } else if (self->spawnflags & 2 && t->client->resp.team == TEAM_HUMAN) {
                                T_Damage (t,self,self,vec3_origin,vec3_origin,vec3_origin,10000,100,DAMAGE_NO_PROTECTION,MOD_TARGET_KILL);
                        }
                }
                else
                {
                        // FIXME: hax, invalid ents are called by this
                        if (t->die) 
                        {
                                if (self->spawnflags & 4 && t->enttype == ENT_COCOON) 
                                {
                                        t->die (t, self, t, 99999, vec3_origin);
                                } 
                                else if (self->spawnflags & 8 && t->enttype == ENT_TELEPORTER) 
                                {
                                        t->die (t, self, t, 99999, vec3_origin);
                                }
                        }
                }
        }
}

void SP_target_kill (edict_t *self)
{
        if (CheckTargetnameEnt (self))
                return;

        // FIXME: HACK
        if (!Q_stricmp (level.mapname, "_bpm_gas"))
                self->message = "got gassed";

        self->movetype = MOVETYPE_NONE;
        self->svflags = SVF_NOCLIENT;
        self->use=target_kill_use;
}
