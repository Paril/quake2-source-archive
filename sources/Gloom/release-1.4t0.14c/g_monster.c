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

void SV_Impact (edict_t *e1, trace_t *trace);
void M_CheckGround (edict_t *ent)
{
        vec3_t          point;
        trace_t         trace;

        if (ent->flags & (FL_SWIM|FL_FLY))
                return;

        /*if (ent->velocity[2] < -1500) {
                ent->touch = touch_die;
        }*/

        if (ent->velocity[2] > 100)
        {
                ent->groundentity = NULL;
                return;
        }

// if the hull point one-quarter unit down is solid the entity is on ground
        point[0] = ent->s.origin[0];
        point[1] = ent->s.origin[1];
        point[2] = ent->s.origin[2] - 0.25f;

        trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID); //MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA); //MASK_MONSTERSOLID);

        // check steepness
        if (trace.plane.normal[2] < 0.7 && !trace.startsolid)
        {
                ent->groundentity = NULL;
                return;
        }

        if (!trace.startsolid && !trace.allsolid)
        {
                VectorCopy (trace.endpos, ent->s.origin);
                ent->groundentity = trace.ent;
                ent->groundentity_linkcount = trace.ent->linkcount;
                ent->velocity[2] = 0;
                SV_Impact (ent,&trace);
        }
}


void M_CatagorizePosition (edict_t *ent)
{
        vec3_t          point;
        int                     cont;
        
//
// get waterlevel
//

        

        point[0] = ent->s.origin[0];
        point[1] = ent->s.origin[1];
        point[2] = ent->absmin[2] + 1;  
        
        //this isn't 100% reliable
        cont = gi.pointcontents (point);
        
        if (!(cont & MASK_WATER))
        {
                //double check
                trace_t tr;
                tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin,ent,MASK_WATER);      
                VectorCopy (tr.endpos, point);
                cont = tr.contents;
                if (!cont) {
                        ent->waterlevel = 0;
                        ent->watertype = 0;
                        return;
                }
        }

        ent->watertype = cont;
        ent->waterlevel = 1;
        point[2] += 26;
        cont = gi.pointcontents (point);
        if (!(cont & MASK_WATER))
                return;

        ent->waterlevel = 2;
        point[2] += 22;
        cont = gi.pointcontents (point);
        if (cont & MASK_WATER)
                ent->waterlevel = 3;
}


void M_WorldEffects (edict_t *ent)
{

        if (ent->waterlevel == 0)
        {
                if (ent->flags & FL_INWATER)
                {       
                        if (ent->client && ent->client->resp.team==TEAM_ALIEN){
                                gi.sound (ent, CHAN_AUTO, SoundIndex (alien_watr_out), 1, ATTN_NORM, 0);
                        }else
                                gi.sound (ent, CHAN_AUTO, SoundIndex (player_watr_out), 1, ATTN_NORM, 0);
                        ent->flags &= ~FL_INWATER;
                }
                return;
        }

        if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA))
        {
                if (ent->damage_debounce_time < level.time)
                {
                        ent->damage_debounce_time = level.time + 0.2f;
                        T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, (8 * ent->waterlevel) + 25, 0, DAMAGE_IGNORE_RESISTANCES, MOD_LAVA);
                }
        }
        if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME))
        {
                if (!(ent->svflags & SVF_MONSTER) && ent->damage_debounce_time < level.time)
                {
                        ent->damage_debounce_time = level.time + 1;
                        T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4*ent->waterlevel, 0, 0, MOD_SLIME);
                }

                if (ent->svflags & SVF_MONSTER) {
                        if (ent->health < ent->max_health) {
                                ent->health +=2;
                                if (ent->health > ent->max_health)
                                        ent->health = ent->max_health;
                        }
                }

        }
        
        if ( !(ent->flags & FL_INWATER) )
        {       
                if (!(ent->svflags & SVF_DEADMONSTER))
                {
                        if (ent->watertype & CONTENTS_SLIME){
                                if (ent->client && ent->client->resp.team==TEAM_ALIEN){
                                        gi.sound (ent, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
                                }else
                                        gi.sound (ent, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
                        }else if (ent->watertype & CONTENTS_WATER){                             
                                if (ent->client && ent->client->resp.team==TEAM_ALIEN){
                                        gi.sound (ent, CHAN_AUTO, SoundIndex (alien_watr_in), 1, ATTN_NORM, 0);
                                }else
                                        gi.sound (ent, CHAN_AUTO, SoundIndex (player_watr_in), 1, ATTN_NORM, 0);
                        }
                }

                ent->flags |= FL_INWATER;
                ent->damage_debounce_time = 0;
        }
}


void M_droptofloor (edict_t *ent)
{
        vec3_t          end;
        trace_t         trace;

        if (ceiling_eggs->value && ent->flags & FL_FLY)
                return;

        ent->s.origin[2] += 1;
        VectorCopy (ent->s.origin, end);
        end[2] -= 256;
        
        trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

        if (trace.fraction == 1 || trace.allsolid)
                return;

        VectorCopy (trace.endpos, ent->s.origin);

        //gi.linkentity (ent);
        M_CheckGround (ent);
        M_CatagorizePosition (ent);
        gi.linkentity (ent);
}


void M_MoveFrame (edict_t *self)
{
        mmove_t *move;
        int             index;

        move = self->monsterinfo.currentmove;
        self->nextthink = level.time + FRAMETIME;

        if ((self->monsterinfo.nextframe) && (self->monsterinfo.nextframe >= move->firstframe) && (self->monsterinfo.nextframe <= move->lastframe))
        {
                self->s.frame = self->monsterinfo.nextframe;
                self->monsterinfo.nextframe = 0;
        }
        else
        {
                if (self->s.frame == move->lastframe)
                {
                        if (move->endfunc)
                        {
                                move->endfunc (self);

                                // regrab move, endfunc is very likely to change it
                                move = self->monsterinfo.currentmove;

                                // check for death
                                if (self->svflags & SVF_DEADMONSTER)
                                        return;
                        }
                }

                if (self->s.frame < move->firstframe || self->s.frame > move->lastframe)
                {
                        self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
                        self->s.frame = move->firstframe;
                }
                else
                {
                        if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
                        {
                                self->s.frame++;
                                if (self->s.frame > move->lastframe)
                                        self->s.frame = move->firstframe;
                        }
                }
        }

        index = self->s.frame - move->firstframe;
        if (move->frame[index].aifunc) {
                if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
                        move->frame[index].aifunc (self, move->frame[index].dist * self->monsterinfo.scale);
                else
                        move->frame[index].aifunc (self, 0);
        }

        if (move->frame[index].thinkfunc)
                move->frame[index].thinkfunc (self);
}


void monster_think (edict_t *self)
{
        M_MoveFrame (self);
        if (self->linkcount != self->monsterinfo.linkcount)
        {
                self->monsterinfo.linkcount = self->linkcount;
                M_CheckGround (self);
        }
        M_CatagorizePosition (self);
        M_WorldEffects (self);
}

qboolean monster_start (edict_t *self)
{
        self->nextthink = level.time + FRAMETIME;
        self->svflags |= SVF_MONSTER;
        //self->s.renderfx |= RF_FRAMELERP;
        self->takedamage = DAMAGE_YES;
        self->air_finished = level.time + 12;
        //self->use = monster_use;
        self->max_health = self->health;
        self->clipmask = MASK_MONSTERSOLID;

        self->s.skinnum = 0;
        self->deadflag = DEAD_NO;
        self->svflags &= ~SVF_DEADMONSTER;

        //if (!self->monsterinfo.checkattack)
        //      self->monsterinfo.checkattack = M_CheckAttack;
        VectorCopy (self->s.origin, self->s.old_origin);

        // randomize what frame they start on
        //if (self->monsterinfo.currentmove)
        //      self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

        return true;
}

void monster_start_go (edict_t *self)
{
        if (self->health <= 0)
                return;

        self->monsterinfo.pausetime = 100000000;
        self->monsterinfo.stand (self);

        self->think = monster_think;
        self->nextthink = level.time + FRAMETIME;
}


void walkmonster_start_go (edict_t *self)
{
        if (level.time < 1)
                M_droptofloor (self);
        
        if (!self->yaw_speed)
                self->yaw_speed = 20;
        self->viewheight = 25;

        monster_start_go (self);

        //if (self->spawnflags & 2)
        //      monster_triggered_start (self);
}

void walkmonster_start (edict_t *self)
{
        self->think = walkmonster_start_go;
        monster_start (self);
}


int c_yes, c_no;

qboolean M_CheckBottom (edict_t *ent)
{
        vec3_t  mins, maxs, start, stop;
        trace_t trace;
        int             x, y;
        float   mid, bottom;
        
        VectorAdd (ent->s.origin, ent->mins, mins);
        VectorAdd (ent->s.origin, ent->maxs, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
        start[2] = mins[2] - 1;
        for     (x=0 ; x<=1 ; x++)
                for     (y=0 ; y<=1 ; y++)
                {
                        start[0] = x ? maxs[0] : mins[0];
                        start[1] = y ? maxs[1] : mins[1];
                        if (gi.pointcontents (start) != CONTENTS_SOLID)
                                goto realcheck;
                }

        c_yes++;
        return true;            // we got out easy

realcheck:
        c_no++;
//
// check it for real...
//
        start[2] = mins[2];
        
// the midpoint must be within 16 of the bottom
        start[0] = stop[0] = (mins[0] + maxs[0])*0.5f;
        start[1] = stop[1] = (mins[1] + maxs[1])*0.5f;
        stop[2] = start[2] - 36;
        trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

        if (trace.fraction == 1.0)
                return false;
        mid = bottom = trace.endpos[2];
        
// the corners must be within 16 of the midpoint        
        for     (x=0 ; x<=1 ; x++)
                for     (y=0 ; y<=1 ; y++)
                {
                        start[0] = stop[0] = x ? maxs[0] : mins[0];
                        start[1] = stop[1] = y ? maxs[1] : mins[1];
                        
                        trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);
                        
                        if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
                                bottom = trace.endpos[2];
                        if (trace.fraction == 1.0 || mid - trace.endpos[2] > 18)
                                return false;
                }

        c_yes++;
        return true;
}
