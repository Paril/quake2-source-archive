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
#include "c4.h"
//void droptofloor (edict_t *ent);

/*void C4_Explode_Medium (edict_t *self);
void C4_Explode_Medium_Smoke (edict_t* self)
{
        self->nextthink = level.time + FRAMETIME;

        //if (self->s.modelindex == gi.modelindex("models/objects/explode/tris.md2")) {
        if (self->think == C4_Explode_Medium) {
                self->think = C4_Explode_Medium_Smoke;

                if(rand() & 1)
                        self->s.modelindex = firea_index;
                else
                        self->s.modelindex = fireb_index;
                //self->s.modelindex = gi.modelindex("sprites/s_bfg3.sp2");
                self->s.effects = EF_SPHERETRANS | EF_HYPERBLASTER; // EF_FLAG1
                self->s.renderfx |= RF_TRANSLUCENT;
                self->s.frame = 0;
                gi.linkentity(self);
        } else {
                if (!level.intermissiontime && visible (self->owner, self) && self->owner && self->owner->owner)
                        T_RadiusDamage (self, self->owner->owner, 240, self, 500, MOD_C4, DAMAGE_NO_KNOCKBACK);
                if (++self->s.frame > 8)
                        G_FreeEdict(self);
        }
}*/

void C4_Explode_Medium (edict_t* self)
{
        self->nextthink = level.time + FRAMETIME;

        //r1: c4 premature removal somehow fix
        if (!self->owner->inuse) 
        {
                G_FreeEdict (self);
                return;
        }

        if (!self->s.modelindex)
        {
                self->think = C4_Explode_Medium;

                self->classname = "medium ex";
                self->s.effects = EF_SPHERETRANS | EF_HYPERBLASTER; // EF_FLAG1;

                self->s.modelindex = gi.modelindex("models/objects/explode/tris.md2");
                self->s.frame = FRAME_medium_explodeB1;
                self->s.renderfx |= RF_FULLBRIGHT;
                gi.linkentity(self);
        }
        else
        {
                if (++self->s.frame > FRAME_medium_explodeB12)
                {
                        //C4_Explode_Medium_Smoke(self);
                        G_FreeEdict(self);
                }
                else
                {
//                      self->s.skinnum = (self->s.frame - FRAME_big_explodec1) / ((FRAME_big_explodec18 - FRAME_big_explodec1) / 5);
                        if (!level.intermissiontime && CanDamage (self, self->owner))
                                T_RadiusDamage (self, self->owner->owner, 280, self, 260, MOD_C4, DAMAGE_NO_KNOCKBACK);
                        //gi.linkentity(self);
                }
        }
}

void flame_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        G_FreeEdict (self);
}

void fire_chunk (edict_t *self)
{
        self->s.frame++;

        if(self->s.frame >= 8)
        {
                self->think = G_FreeEdict;
        }

        self->nextthink = level.time + FRAMETIME*2;
}

void ThrowFlames (vec3_t origin)
{
        edict_t *chunk;
        //vec3_t        v;

        chunk = G_Spawn();
        VectorCopy (origin, chunk->s.origin);
        //gi.setmodel (chunk, "models/objects/debris2/tris.md2");

        if (randomMT() & 1)
                chunk->s.modelindex = firea_index;
        else
                chunk->s.modelindex = fireb_index;

        /*v[0] = 700 * crandom();
        v[1] = 700 * crandom();
        v[2] = 200 + 100 * crandom();
        VectorScale (v, 2 + crandom(), v);
        VectorCopy(v,chunk->velocity);*/

        chunk->velocity[0] = 700 * crandom();
        chunk->velocity[1] = 700 * crandom();
        chunk->velocity[2] = 200 + 100 * crandom();
        VectorScale (chunk->velocity, 2 + crandom(), chunk->velocity);

        chunk->movetype = MOVETYPE_TOSS;
        //chunk->solid = SOLID_NOT;
        chunk->avelocity[0] = random()*600;
        chunk->avelocity[1] = random()*600;
        chunk->avelocity[2] = random()*600;
        //chunk->think = G_FreeEdict;
        //chunk->nextthink = level.time + 3 + random()*2;
        chunk->think = fire_chunk;
        chunk->nextthink = level.time + FRAMETIME*2;
        //chunk->s.frame = 0;
        //chunk->s.skinnum = 0;
        //chunk->s.renderfx = 0;
        chunk->s.effects = EF_SPHERETRANS | EF_ROCKET;
        //chunk->flags = 0;
        chunk->classname = "flaming debris";
        //chunk->takedamage = DAMAGE_NO;
        chunk->die = flame_die;
        gi.linkentity (chunk);
}

void C4_Timer (edict_t *c4);
void C4_Explode (edict_t* self)
{
        edict_t*        c4medium;
        //edict_t               *target = NULL;
//      int points;
        int i = 0;
        int j;
//      vec3_t          v;

        self->nextthink = level.time + FRAMETIME;

        if (self->think == C4_Timer)
        {
                // free lights
                if (self->teamchain) {
                        G_FreeEdict(self->teamchain);
                  VectorClear(self->s.angles); //Don't orient explosions
                }

                // stop the explosion if player isn't valid (eg. left game or switched sides)
                if (!self->owner->inuse || (self->owner->client && self->owner->client->resp.team != TEAM_HUMAN && !(self->owner->client->pers.adminpermissions & PERMISSION_TOYS) && !level.intermissiontime))
                {
                        G_FreeEdict(self);
                        return;
                }

                self->s.sound = 0;
                self->think = C4_Explode;

                self->s.frame = FRAME_big_explodec1;
                //self->s.skinnum = 0;
                self->s.modelindex = gi.modelindex("models/objects/r_explode/tris.md2");
                self->s.renderfx |= RF_FULLBRIGHT;
                self->s.effects = EF_SPHERETRANS | EF_HYPERBLASTER;

                //r1ch: prevent c4 from "dying" mid explosion
                self->solid = SOLID_NOT;
                self->takedamage = DAMAGE_NO; 

                //r1ch - 8/10/2000 - beefed up c4 a bit (yeah i know i shouldn't :P)
                //original damage: 600 radius: 550
                // fuxed around by ankka
                if (!level.intermissiontime)
                  T_RadiusDamage(self, self->owner, 600, self, 750, MOD_C4, DAMAGE_NO_KNOCKBACK);
                //T_RadiusDamage(self, self->owner, 700, self, 800 * 1.2, MOD_C4, DAMAGE_IGNORE_CLIENTS|DAMAGE_NO_KNOCKBACK);
                //T_RadiusDamage(self, self->owner, 700 * 4, self, 100, MOD_C4, DAMAGE_NO_KNOCKBACK);
                //r1ch

                /* blinding should be done in t_raddamage or t_damage
                while ((target = findradius_c(target, self, 1000)) != NULL)
                {
            if (!visible(self, target))
                    continue;       // The grenade can't see it
                        VectorAdd (target->mins, target->maxs, v);
                        VectorMA (target->s.origin, 0.5, v, v);
                        VectorSubtract (self->s.origin, v, v);
                        points = 1000-VectorLength (v);
            //if (target == self->owner)
                    //continue;       // You know when to close your eyes, don't you?
                        target->client->blind_time = points/5;
                        if (target->client->blind_time > 3)
                                target->client->blind_time = 3;
                        target->client->blind_time_alpha = 0.5;
                }*/

                gi.sound (self, CHAN_AUTO, SoundIndex (weapons_c4), 1.0, ATTN_NORM, 0);

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_NUKEBLAST);
                gi.WritePosition (self->s.origin);
                gi.multicast (self->s.origin, MULTICAST_PVS);

                // Create medium explosions                             

                c4medium = G_Spawn();
                VectorCopy(self->s.origin, c4medium->s.origin);
                c4medium->s.origin[0] += 150;
                c4medium->owner = self;
                C4_Explode_Medium(c4medium);

                c4medium = G_Spawn();
                VectorCopy(self->s.origin, c4medium->s.origin);
                c4medium->s.origin[0] -= 150;
                c4medium->owner = self;
                C4_Explode_Medium(c4medium);

                c4medium = G_Spawn();
                VectorCopy(self->s.origin, c4medium->s.origin);
                c4medium->s.origin[1] += 150;
                c4medium->owner = self;
                C4_Explode_Medium(c4medium);

                c4medium = G_Spawn();
                VectorCopy(self->s.origin, c4medium->s.origin);
                c4medium->s.origin[1] -= 150;
                c4medium->owner = self;
                C4_Explode_Medium(c4medium);

                j = 3 + random() * 3;
                for (i = 0; i < j;i++) {
                        ThrowFlames (self->s.origin);
                }

        }
        else
        {
                if (++self->s.frame > FRAME_big_explodec18)
                        G_FreeEdict(self);
                else
                {
                        self->s.skinnum = (self->s.frame - FRAME_big_explodec1) / ((FRAME_big_explodec18 - FRAME_big_explodec1) / 5);

                        //r1: removed (unnecessay i think)
                        //gi.linkentity(self);
                }
        }
}


void C4_Timer (edict_t *c4)
{
        c4->nextthink = level.time + 1;

        // don't play beep sound on explosion frame
        if (c4->count-- <= 0)
                C4_Explode(c4);
        else
                gi.sound (c4, CHAN_AUTO, SoundIndex (misc_comp_up),  1.0, ATTN_IDLE, 0);

}

static void c4_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (plane) { //Update orientation
	  vec3_t rn = {-plane->normal[0], -plane->normal[1], -plane->normal[2]};
	  vec3_t fw;
          float a = ent->s.angles[2];
	  AngleVectors(ent->s.angles, fw, NULL, NULL);
	  if (fabs(DotProduct(fw, rn)) < 0.42)
            a = (int)(a+ent->s.angles[1])%360;
	  vectoangles(rn, ent->s.angles);
	  ent->s.angles[2] = a;
	}
        VectorCopy(ent->s.origin, ent->teamchain->s.origin);
        gi.linkentity(ent->teamchain);
        gi.linkentity(ent);
}

edict_t  *C4_Arm (edict_t* self)
{
        edict_t* c4;
        vec3_t  min={-8, -8, -6},max={8, 8, 8};
        //float*        v;
        edict_t *ent = NULL;
        //trace_t       tr;

        c4 = G_Spawn();

        VectorCopy(self->s.origin, c4->s.origin);
        c4->s.angles[1] = 90;
        c4->s.angles[2] = self->s.angles[1];

        c4->owner = self;
        //c4->model = "models/objects/c4/tris.md2";
        c4->s.modelindex = gi.modelindex("models/objects/c4/tris.md2");
//      droptofloor(c4);                                
        c4->flags |= FL_CLIPPING;
        c4->touch = c4_touch;
        c4->movetype=MOVETYPE_STEP;
        c4->solid=SOLID_BBOX;
        VectorCopy(min,c4->mins);
        VectorCopy(max,c4->maxs);
        //      gi.sound (c4, CHAN_AUTO, SoundIndex (weapons_hgrent1a), 1, ATTN_NORM, 0);
        //gi.sound (c4, CHAN_AUTO, SoundIndex (world_10_0), 1, ATTN_NORM, 0);
        // sounds awkward, removed
        //c4->s.sound = SoundIndex (weapons_hgrenc1b);
        c4->think = C4_Timer;
        c4->count = 10;
        c4->nextthink = level.time + FRAMETIME;
        c4->s.renderfx |= RF_IR_VISIBLE;
        c4->classname = "c4";
        //c4->dmg_radius = 800;
        //c4->radius_dmg = 700;
        c4->takedamage = DAMAGE_NO;

        c4->teamchain = G_Spawn ();
        VectorCopy(c4->s.origin, c4->teamchain->s.origin);
        c4->teamchain->owner=self;
        c4->teamchain->touch=c4_touch;
        c4->teamchain->movetype = MOVETYPE_NOCLIP;
        c4->teamchain->solid = SOLID_NOT;
        VectorCopy(min,c4->teamchain->mins);
        VectorCopy(max,c4->teamchain->maxs);
        c4->teamchain->classname = "spinninglights";
        c4->teamchain->s.modelindex = shiny_index;      // HEY KIDDYS NOTE THIS
        //c4->teamchain->s.skinnum = 0;
        c4->teamchain->s.effects |= EF_SPINNINGLIGHTS;

        //r1ch - 8/10/2000 - fix lights on c4 premature death
        c4->teamchain->think = G_FreeEdict;
        c4->teamchain->nextthink = level.time + 11;

        c4->enttype = ENT_C4;
        //r1ch

        // ?
        //v = tv(0,0,-128);
        //VectorAdd (c4->s.origin, v, dest);
        //tr = gi.trace (self->s.origin, c4->mins, c4->maxs, dest, self, MASK_SOLID);

        gi.linkentity(c4);
        gi.linkentity(c4->teamchain);

        while ((ent=findradius_c(ent,c4,1000))!=NULL){
                gi.cprintf(ent,PRINT_HIGH,"C4 Explosive Armed!\n");
        }

        return c4;
}

/* not in use ? 
void Weapon_C4 (edict_t *ent)
{
        if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
        {
                ChangeWeapon (ent);
                return;
        }

        if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
        {
                ent->client->latched_buttons &= ~BUTTON_ATTACK;
                if (ent->client->resp.inventory[ent->client->ammo_index])
                {
                        ent->client->resp.inventory[ent->client->ammo_index]--;
                        C4_Arm(ent);
                }
        }

        ent->client->weaponstate = WEAPON_READY;
}*/

void Cmd_Debug_test (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
        edict_t *c4;
        vec3_t  min={-8,-8,-8},max={8,8,8};
        //float*        v;
        //trace_t       tr;
        vec3_t  dir;
        vec3_t  forward, right, up;

        c4 = G_Spawn();
        VectorCopy(self->s.origin, c4->s.origin);
        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        VectorCopy (start, c4->s.origin);
        VectorScale (aimdir, speed, c4->velocity);
        VectorMA (c4->velocity, 200 + crandom() * 10.0, up, c4->velocity);
        VectorMA (c4->velocity, crandom() * 10.0, right, c4->velocity);
        VectorSet (c4->avelocity, 300, 300, 300);

        c4->owner = self;
        c4->s.modelindex = gi.modelindex("models/objects/c4/tris.md2");
//      droptofloor(c4);                                
        c4->touch = c4_touch;
        c4->dmg_radius = 1000;
        c4->radius_dmg = 1000;
        c4->movetype=MOVETYPE_BOUNCE;
        c4->solid=SOLID_BBOX;
        VectorCopy(min,c4->mins);
        VectorCopy(max,c4->maxs);
        //      gi.sound (c4, CHAN_AUTO, SoundIndex (weapons_hgrent1a), 1, ATTN_NORM, 0);
        
        //c4->s.sound = SoundIndex (weapons_hgrenc1b);
        c4->think = C4_Timer;
        c4->count = 10;
        c4->enttype = ENT_C4;
        c4->nextthink = level.time + 1;
        c4->s.renderfx |= RF_IR_VISIBLE;
        c4->classname = "c4";

        c4->teamchain = G_Spawn ();
        VectorCopy(c4->s.origin, c4->teamchain->s.origin);
        c4->teamchain->owner=self;
        //c4->teamchain->touch=NULL;
        c4->teamchain->movetype = MOVETYPE_NOCLIP;
        //c4->teamchain->solid = SOLID_NOT;
        VectorCopy(min,c4->teamchain->mins);
        VectorCopy(max,c4->teamchain->maxs);
        c4->teamchain->classname = "spinninglights";
        c4->teamchain->s.modelindex = shiny_index;      // HEY KIDDYS NOTE THIS
        //c4->teamchain->s.skinnum = 0;
        c4->teamchain->s.effects |= EF_SPINNINGLIGHTS;

        //r1ch - 8/10/2000 - fix lights on c4 premature death
        c4->teamchain->think = G_FreeEdict;
        c4->teamchain->nextthink = level.time + 11;
        //r1ch

        // ?
        //v = tv(0,0,-128);
        //VectorAdd (c4->s.origin, v, dest);
        //tr = gi.trace (self->s.origin, c4->mins, c4->maxs, dest, self, MASK_SOLID);

        gi.linkentity(c4);
        gi.linkentity(c4->teamchain);
}
