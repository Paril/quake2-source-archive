#include "g_local.h"

/*
==============
ToastThatFucker - This next bit is for removing things blocking plats!
                (Corpse or other)
==============
*/
void RPI_Think (edict_t *ent);
void ToastThatFucker (edict_t *self)
{
        teams_t *team;

	//flags are important
        if ((!strcmp(self->classname, "item_flag_team1")) || (!strcmp(self->classname, "item_flag_team2"))
          || (!strcmp(self->classname, "item_flag_team3")) || (!strcmp(self->classname, "item_flag_team4")))
        {
                team = GetTeamByIndex(self->item->quantity);                
                CTFResetFlag(team); // this will free self!
                gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n", team->name);
		return;
	}

//ZOID
	// techs are important too
        // Dirty
        if (self->item && (self->item->flags & IT_RAND))
        {
                RPI_Think(self);
		return;
	}
        // Dirty
//ZOID
        self->movetype = MOVETYPE_NOCLIP;
        self->solid = SOLID_NOT;
        self->svflags |= SVF_NOCLIENT;
        self->think = G_FreeEdict;
        self->nextthink = level.time + FRAMETIME;
        gi.linkentity (self);
}

/*
================================================
HealthCalc

Check for bleed or healing
================================================
*/
qboolean BindWounds (edict_t *ent, int count)
{
        int     goods, i;

        if (count < 1)
                return false;

        if (!ent->client)
                return false;

        if (!Wounded(ent))
                return false;

        goods = count;

        i = 0;
        while (i < 6)
        {
                if (ent->client->pers.wounds[i] > 0)
                {
                        ent->client->pers.wounds[i] -= goods;
                        goods = (ent->client->pers.wounds[i] * -1);
                }
                if (goods < 1)
                        break;
                i++;
        }

        SetWoundsHud (ent);

        return true;
}

qboolean ApplyHealth (edict_t *ent, int count)
{
        // Stim packs just add health and slows bleeding...
        if (count < 0)
        {
                ent->health += (count * -1);
                if ((ent->client->bleed_time > level.time) && (random() < 0.5))
                        ent->client->bleed_time = ent->client->bleed_time + 0.5;
                return true;
        }

        // Dirty
        if (!Wounded(ent))
                return false;

        /*
        if (!Wounded(ent))
        {
                if ((int)realflags->value & RF_NO_HEAL)
                        return false;
                else if ((ent->client->pers.heal_rate > 8) || (ent->health >= ent->max_health))
                        return false;
        }
        */
        // Dirty

        if (BindWounds (ent, count))
                return true;
        ent->client->pers.heal_rate += (count/3);

        return true;
}

int LegWounds(edict_t *ent)
{
        int     w;

        if (!ent->client)
                return 0;

        w = 0;

        if (ent->client->pers.wounds[W_LLEG] > 0)
                w += ent->client->pers.wounds[W_LLEG];
        if (ent->client->pers.wounds[W_RLEG] > 0)
                w += ent->client->pers.wounds[W_RLEG];

        return w;
}

int ArmWounds(edict_t *ent)
{
        int     w;

        if (!ent->client)
                return 0;

        w = 0;

        if (ent->client->pers.wounds[W_LARM] > 0)
                w += ent->client->pers.wounds[W_LARM];
        if (ent->client->pers.wounds[W_RARM] > 0)
                w += ent->client->pers.wounds[W_RARM];

        return w;
}

int Wounded (edict_t *ent)
{
        int i, w;

        if (!ent->client)
                return 0;

        i = 0;
        w = 0;
        while (i < 6)
        {
                if (ent->client->pers.wounds[i] > 0)
                        w += ent->client->pers.wounds[i];
                i++;
        }

        if (w > 0)
                return w;

        return 0;
}

void HealthCalc (edict_t *ent)
{
        int     wound_level, i;
        float   chance;
        //vec3_t  vec;

        if ((ent->health < 1) || (ent->deadflag))
                return;

        if (ent->movetype == MOVETYPE_NOCLIP)
                return;

        // Dirty
        /*
        if (ent->client->pers.heal_rate < 0)
                ent->client->pers.heal_rate = 0;
        if (ent->client->pers.heal_rate > 9)
                ent->client->pers.heal_rate = 9;
        */
        // Dirty

        i = 0;
        wound_level = 0;
        while (i < 6)
        {
                if (ent->client->pers.wounds[i] > 9)
                        ent->client->pers.wounds[i] = 9;

                if (ent->client->pers.wounds[i] > 0)
                        wound_level += ent->client->pers.wounds[i];
                else
                        ent->client->pers.wounds[i] = 0;
                i++;
        }

        if (wound_level > 15)
                wound_level = 15;

        // Dirty
        /*
        if ((!((int)realflags->value & RF_NO_HEAL)) && (ent->client->heal_time < level.time) && (wound_level == 0))
        {
                if (ent->health >= ent->max_health)
                {
                        if (ent->client->pers.heal_rate > 3)
                                ent->client->pers.heal_rate -= 1;
                        ent->client->heal_time = level.time + ent->client->pers.heal_rate;
                }
                else if (level.time - ent->last_damage > 5)
                {
                        ent->client->heal_time = level.time + (10 - ent->client->pers.heal_rate);
                        ent->health += 1;
                }
        }
        */
        // Dirty

        if ((ent->client->bleed_time < level.time) && wound_level > 0)
        {
                chance = 0.3 + ((15 - wound_level)/15);
                
                ent->client->bleed_time = level.time + chance;

                if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                        chance += 0.1;
                if (ent->velocity)
                        chance += 0.1;

                // Paranoid temp - FIX ME LATER (fire_blood part)
                //VectorSubtract(ent->s.origin, ent->s.old_origin, vec);
                //fire_blood (ent, ent->s.old_origin, vec, (wound_level * 10));
                //SpawnDamage (TE_BLOOD, ent->s.origin, vec, wound_level);
                // Paranoid temp

                if (random() < chance)
                {
                        ent->health--;
                        
                        // Dirty
                        if (wound_level > 6 && (random() < 0.2))
                                ent->client->damage_blood += 1;
                        // Dirty

                        if ((ent->health <= 0) && (ent->deadflag == DEAD_NO))
                        {
                                meansOfDeath = ent->client->last_means;
                                HitLocation = ent->client->last_hitloc;

                                if ((level.time - ent->last_combat > 10) && (!((int)bflags->value & BF_NO_BLEED_DEATH)))
                                {
                                        ent->enemy = ent;
                                        meansOfDeath = MOD_BLED;
                                }

                                TypeOfDamage = 0;
                                player_die (ent, ent, ent->enemy, 100000, vec3_origin);
                        }
                }
        }
}


/*
================

ACTION KEY SECTION

================
*/

/*
================
Heal_Their_Ass
================
*/
void ApplyHealthKit (edict_t *ent, gitem_t *item, gitem_t *itemagain);

void Heal_Their_Ass (edict_t *ent, edict_t *other)
{
        gitem_t *item = NULL;
        int     i = 0;

        if (ent->delay_time > level.time)
                return;

        if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Hands"))
        {
                gi.centerprintf (ent, "Put away your weapon first.\n");
                return;                      
        }

        i = Wounded (other);

        if (i > 0)
        {
                // Dirty
                if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Bandages"))] > 0)
                        item = FindItem("Bandages");
                else
                        gi.centerprintf (ent, "You have no bandages!\n");
                // Dirty
                        
                if (item)
                {
                        ent->corpse = other;
                        Use_Health (ent, item);
                }
        }
        else
                gi.centerprintf (ent, "They're not wounded!\n");
}


/*
================
ObjectAndFunction - Trace for objects to interact with.
================
*/
qboolean ObjectAndFunction (edict_t *ent) 
{
        trace_t tr;
        vec3_t  forward, point;
        int     damage;

        AngleVectors (ent->client->v_angle, forward, NULL, NULL);

        VectorMA (ent->s.origin, 48, forward, point);
        tr = gi.trace (ent->s.origin, NULL, NULL, point, ent, MASK_SHOT);

        if (tr.fraction == 1)
                return false;

        if ((ent->delay_time > level.time) || (ent->attack_time > level.time))
                return false;

        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                return false;


        if ((tr.ent->takedamage) && (!tr.ent->deadflag) && (!tr.ent->item))
        {
                if (tr.ent->client && (OnSameTeam(tr.ent, ent)))
                {
                        Heal_Their_Ass(ent, tr.ent);
                        return true;
                }

                // Dirty
                if (ent->mindmg && (ent->mindmg > 5))
                        return false;
                // Dirty

                ent->attack_time = level.time + 0.4;

                damage = 6 + (rand() %5);

                forward[2] = 0.01;
                T_Damage (tr.ent, ent, ent, forward, tr.endpos, vec3_origin, damage, 170, HIT_CHEST, TOD_NO_WOUND|TOD_NO_ARMOR, MOD_KICKED);

                if (random() > 0.5)
                        gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/kick1.wav"), 1, ATTN_NORM, 0);
                else 
                        gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/kick2.wav"), 1, ATTN_NORM, 0);

                return true;
        }

        return false;
        /*
        if (tr.ent->movetype == MOVETYPE_PUSH))
                return;

        // hit wall
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_SHOTGUN);
        gi.WritePosition (tr.endpos);
        gi.WriteDir (tr.plane.normal);
        gi.multicast (tr.endpos, MULTICAST_PVS);

        if ((tr.ent->use) && ((tr.ent->use == door_use) || (tr.ent->use == door_secret_use)))
                return false;
        else
                return true;
        */
}

/*
==============
Cmd_Action_On - Actions.  Grab, kick, climb.
==============
*/
qboolean Grab_n_Climb (edict_t *ent);

void Cmd_Action_On (edict_t *ent)
{
        if (ent->deadflag || (ent->movetype == MOVETYPE_NOCLIP))
                return;

        // If jumping/falling etc
        // Dirty - Grab bandages quick
        if ((ent->client->pers.grabbing == 2) && ent->client->showinventory)
        {
                if (ent->corpse && (!ent->corpse->client))
                {
                        if (ent->corpse->inventory[ITEM_INDEX(FindItem("Bandages"))] > 0)
                        {
                                ent->client->pers.selected_item = ITEM_INDEX(FindItem("Bandages"));
                                CorpseDropItem (ent);
                        }
                }
                //ent->client->showinventory = false;
                return;
        }
        // GRIM

         // If weapon idle...
        if (ent->client->weaponstate == WEAPON_READY)
        {
                if (ObjectAndFunction(ent))
                        return;
                // If jumping/falling etc
                // DTEMP
                if (Grab_n_Climb (ent))
                {
                        //ent->client->hanging = true;
                        return;
                }
                // DTEMP
        }
        // Dirty
        else if (!Q_stricmp(ent->client->pers.weapon->pickup_name, "M61 Fragmentation Grenade"))
        {
                if (ObjectAndFunction(ent))
                        return;
        }
        // Dirty

        ent->client->pers.grabbing = 1;  // On
}

void Cmd_Action_Off (edict_t *ent)
{
        if (ent->client->pers.grabbing != 2)
                ent->client->pers.grabbing = -1;  // Off, go figure
                                                // well, door open actually
}

qboolean Grab_n_Climb (edict_t *ent)
{
        vec3_t  forward, start, end;
        int     check = 0;
        int     plusup = 0;
        trace_t tr;

        if (ent->deadflag || (ent->movetype == MOVETYPE_NOCLIP))
                return false;

        if (ent->groundentity)
                return false;

        AngleVectors (ent->s.angles, forward, NULL, NULL);
        //forward[2] = 0;
        VectorNormalize (forward);

        VectorCopy (ent->s.origin, start);
        // Space above?
        start[2] += 56;
        tr = gi.trace (ent->s.origin, NULL, NULL, start, ent, MASK_SHOT);
        if (tr.fraction < 1.0)
                return false;

        VectorCopy (ent->s.origin, start);

        while (check < 64)
        {
                start[2] += 1;
                VectorMA (start, 40, forward, end); // Increased from 24
                tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);
                if (tr.fraction < 1.0)
                        check = 64;
                else
                        check++;
        }

        plusup = ent->weight - 110;

        if (plusup < 0)
                plusup = 0;

        if (plusup > 175)
                plusup = 175;

        if (tr.fraction < 1.0)
        {
                VectorCopy (ent->s.origin, start);
                start[2] += 84;
                VectorMA (start, 40, forward, end);  // Increased from 24
                tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

                if (!(tr.fraction < 1.0))
                {
                        VectorCopy (end, ent->client->hang_point);
                        check = (260 - plusup);
                        if (ent->velocity[2] < 0)
                                ent->velocity[2] = 0;
                        ent->velocity[2] += check;
                        if (ent->velocity[2] > 400)
                                ent->velocity[2] = 400;

                        if (ent->weight > 100)
                                gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
                        //gi.dprintf ("Grab_n_Climb - pass\n");
                        return true;
                }
        }
        //gi.dprintf ("Grab_n_Climb - fail\n");
        return false;
}

qboolean HangingCheck (edict_t *ent)
{
        vec3_t  forward, start, end;
        int     check = 0;
        trace_t tr;

        if (ent->groundentity)
        {
                ent->client->hanging = false;
                return false;
        }

        AngleVectors (ent->s.angles, forward, NULL, NULL);
        //forward[2] = 0;
        VectorNormalize (forward);

        VectorCopy (ent->s.origin, start);
        // Space above?
        start[2] += 56;
        tr = gi.trace (ent->s.origin, NULL, NULL, start, ent, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                ent->client->hanging = false;
                return false;
        }
        VectorCopy (ent->s.origin, start);
        start[2] -= 32;

        while (check < 88)
        {
                start[2] += 1;
                VectorMA (start, 40, forward, end); // Increased from 24
                tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);
                if (tr.fraction < 1.0)
                        check = 88;
                else
                        check++;
        }

        if (tr.fraction < 1.0)
        {
                VectorCopy (ent->s.origin, start);
                start[2] = ent->client->hang_point[2];
                VectorMA (start, 40, forward, end);  // Increased from 24
                tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

                if (!(tr.fraction < 1.0))
                {
                        ent->client->hanging = true;
                        if (ent->velocity[2] > 400)
                                ent->velocity[2] = 400;
                        return true;
                }
        }
        ent->client->hanging = false;
        return false;
}


qboolean CheckHang (edict_t *ent)
{
        vec3_t  forward, start, end;
        int     check;
        trace_t tr;

        //gi.dprintf ("CheckHang - ");
        if (ent->groundentity)
        {
                //gi.dprintf ("on ground, fail\n");
                return false;
        }
        AngleVectors (ent->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorNormalize (forward);

        VectorCopy (ent->s.origin, start);

        start[2] += 52;

        // Space above?
        tr = gi.trace (ent->s.origin, NULL, NULL, start, ent, MASK_SHOT);
        if (tr.fraction < 1.0)
        {
                //gi.dprintf ("something above, fail\n");
                return false;
        }

        check = 0;
        while (check < 12)
        {
                VectorMA (start, 24, forward, end);
                tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);
                if (tr.fraction < 1.0)
                        check = 12;
                else
                {
                        start[2] += 1;
                        check++;
                }
        }

        if (tr.fraction < 1.0)
        {
                //gi.dprintf ("fp pass, ");
                check = 0;
                while (check < 12)
                {
                        VectorMA (start, 24, forward, end);
                        tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);

                        if (!(tr.fraction < 1.0))
                        {
                                ent->client->hanging = true;
                                VectorCopy (tr.endpos, ent->client->hang_point);
                                //gi.dprintf ("full pass\n");
                                return true;
                        }
                        else 
                                check++;
                        start[2] += 1;
                }
        }

        //gi.dprintf ("fail\n");
        ent->client->hanging = false;
        return false;
}


/*
=======================
Identification Stuff
=======================
*/
void Cmd_IdState_f (edict_t *ent)
{
	if (ent->client->resp.id_state)
        {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	}
        else
        {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

void EnemySpotted (edict_t *ent, edict_t *enemy)
{
        teams_t *team;
        teams_t *nme_team;
        teams_t *old_nme_team;
        int     i;

        team = ent->client->resp.team;
        nme_team = enemy->client->resp.team;
        old_nme_team = ent->spotted->client->resp.team;

        if ((nme_team != old_nme_team) || (ent->spotted_time < level.time))
        {
                i = range(ent, enemy);
                
                if (i < RANGE_NEAR)
                        NewEvent (ent, ENEMY_HERE);
                else
                        NewEvent (ent, ENEMY_SPOTTED);
                ent->spotted = enemy;
                ent->spotted_time = level.time + 20;
        }
}

void SetIDView (edict_t *ent)
{
        vec3_t  forward, right, offset, end, start;
        vec3_t  mins = {-12, -12, -12};
        vec3_t  maxs = {12, 12, 12};
        int     mask;
	trace_t	tr;

        if (ent->health < 1)
                return;

        mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_WATER;

        AngleVectors(ent->client->v_angle, forward, right, NULL);
        VectorSet (offset, 0, 0, ent->viewheight);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        // Dirty
        if (ent->client->ps.fov == 90)
                VectorMA (start, 1024, forward, end);
        else if (ent->client->ps.fov == 40)
                VectorMA (start, 2048, forward, end);
        else if (ent->client->ps.fov == 20)
                VectorMA (start, 4096, forward, end);
        else if (ent->client->ps.fov == 10)
                VectorMA (start, 8192, forward, end);
        // Dirty

        tr = gi.trace(start, mins, maxs, end, ent, mask);

        if (tr.fraction == 1)
		return;

        if (tr.ent && (!tr.ent->client))
        {
                if ((tr.fraction * 1024) < 64) // retrace...with no MINS/MAXS
                        tr = gi.trace(start, vec3_origin, vec3_origin, end, ent, mask);
                else
                        return;
        }

        if (tr.fraction == 1)
		return;

        if (tr.ent && (!tr.ent->client))
                return;

        if (tr.ent->deadflag == DEAD_DEAD)
		return;

        // If invisible :)
        if (tr.ent->s.effects & EF_SPHERETRANS)
		return;

        if ((tr.ent->waterlevel > 2) && (ent->waterlevel < 3))
		return;

        if ((ent->waterlevel > 2) && (!tr.ent->waterlevel))
		return;

        if (tr.ent->light_level < 7)
		return;

        ent->client->ps.stats[STAT_ID_VIEW] = CS_PLAYERNAMES + (tr.ent - g_edicts - 1);

        // Dirty
        if ((!OnSameTeam(ent, tr.ent)) && (!AllyCheck (tr.ent, ent)))
        {
                ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("enemy");
                EnemySpotted(ent, tr.ent);
        }
        else if (ent->last_player_heard && (ent->last_player_heard == tr.ent))
        {
                ent->req = false;
                ent->audio = false;
                ent->last_player_heard = NULL;
                ent->client->ps.stats[STAT_GOAL2] = gi.imageindex ("g2none");
        }
        else if (ent->last_player_request && (ent->last_player_request == tr.ent))
        {
                ent->client->ps.stats[STAT_GOAL1] = gi.imageindex ("g1none");
                ent->last_player_request = NULL;
                ent->goal = false;
        }
        // Dirty
}

// GRIM - No monster code
/*
=============
range

returns the range catagorization of an entity reletive to self
0	melee range, will become hostile even if back is turned
1	visibility and infront, or visibility and show hostile
2	infront and show hostile
3	only triggered by damage
=============
*/
int range (edict_t *self, edict_t *other)
{
	vec3_t	v;
	float	len;

	VectorSubtract (self->s.origin, other->s.origin, v);
	len = VectorLength (v);
	if (len < MELEE_DISTANCE)
		return RANGE_MELEE;
	if (len < 500)
		return RANGE_NEAR;
	if (len < 1000)
		return RANGE_MID;
	return RANGE_FAR;
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible (edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy (other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);
	
	if (trace.fraction == 1.0)
		return true;
	return false;
}


/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;
	return false;
}

// GRIM - 24/01/99
/*
====================
point_infront - more specific then just infront
====================
*/
qboolean point_infront (edict_t *self, vec3_t point)
{
        vec3_t  vec, forward;
	float	dot;

        if (!self->client)
                return false;
	
        AngleVectors (self->client->v_angle, forward, NULL, NULL);
        VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;

	return false;
}
// GRIM - 24/01/99

// GRIM - No monster code

void SetGoal1 (edict_t *ent)
{
        vec3_t  forward, right, vec;
        float   dot_f, dot_r;
        char    *goal_arrow;
        int     dist;

        if ((!ent->goal) || (ent->movetype == MOVETYPE_NOCLIP))
        {
                ent->client->ps.stats[STAT_GOAL1] = 0;
                return;
        }

        goal_arrow = NULL;

        AngleVectors (ent->s.angles, forward, right, NULL);
        VectorSubtract (ent->goal_pos, ent->s.origin, vec);

        dist = VectorLength (vec);

        VectorNormalize (vec);
        forward[2] = 0;
        right[2] = 0;

        dot_f = DotProduct (vec, forward);
        dot_r = DotProduct(vec, right);

        if (dist < 128)
        {
                goal_arrow = "g1none";
                ent->goal = false;
        }
        else if (dot_f > 0.5)
        {
                if (dot_r > 0.5)
                        goal_arrow = "g1ne";
                else if (dot_r < -0.5)
                        goal_arrow = "g1nw";
                else
                        goal_arrow = "g1n";
        }
        else if (dot_f < -0.5)
        {
                if (dot_r > 0.5)
                        goal_arrow = "g1se";
                else if (dot_r < -0.5)
                        goal_arrow = "g1sw";
                else
                        goal_arrow = "g1s";
        }
        else if (dot_r > 0.5)
                goal_arrow = "g1e";
        else if (dot_r < -0.5)
                goal_arrow = "g1w";
        else
                goal_arrow = "g1none";

        if (goal_arrow)
                ent->client->ps.stats[STAT_GOAL1] = gi.imageindex (goal_arrow);
        else
                ent->client->ps.stats[STAT_GOAL1] = 0;
}

void SetGoal2 (edict_t *ent)
{
        vec3_t  forward, right, vec;
        float   dot_f, dot_r;
        char    *goal_arrow;
        int     dist;

        if ((!ent->req && !ent->audio) || (ent->movetype == MOVETYPE_NOCLIP))
        {
                ent->client->ps.stats[STAT_GOAL2] = 0;
                return;
        }

        goal_arrow = NULL;

        AngleVectors (ent->s.angles, forward, right, NULL);
        
        if (ent->req)
                VectorSubtract (ent->req_pos, ent->s.origin, vec);
        else
        {
                VectorSubtract (ent->audio_pos, ent->s.origin, vec);
                if (level.time - ent->last_heard > 15)
                        ent->audio = false;
        }

        dist = VectorLength (vec);

        VectorNormalize (vec);
        forward[2] = 0;
        right[2] = 0;

        dot_f = DotProduct (vec, forward);
        dot_r = DotProduct(vec, right);

        if (dist < 128)
        {
                if (ent->req)
                        ent->req = false;
                else
                        ent->audio = false;
                goal_arrow = "g2none";
        }
        else if (dot_f > 0.5)
        {
                if (dot_r > 0.5)
                        goal_arrow = "g2ne";
                else if (dot_r < -0.5)
                        goal_arrow = "g2nw";
                else
                        goal_arrow = "g2n";
        }
        else if (dot_f < -0.5)
        {
                if (dot_r > 0.5)
                        goal_arrow = "g2se";
                else if (dot_r < -0.5)
                        goal_arrow = "g2sw";
                else
                        goal_arrow = "g2s";
        }
        else if (dot_r > 0.5)
                goal_arrow = "g2e";
        else if (dot_r < -0.5)
                goal_arrow = "g2w";
        else
                goal_arrow = "g2none";

        if (goal_arrow)
                ent->client->ps.stats[STAT_GOAL2] = gi.imageindex (goal_arrow);
        else
                ent->client->ps.stats[STAT_GOAL2] = 0;
}


/*
====================
strtostr2 - Converts regular text to the green kind.

====================
*/
char *strtostr2 (char *s)
{
        // FUCKING WELL FUCKING FIX ME ALREADY!
        /* DTEMP - Causing problems with funny characters??
        char *p;

        p = s;
        while (*p)
        {
                if ((*p >= 0x1b && *p <= 0x79) || (*p >= 0x0a && *p <= 0x11))
                        *p += (char) 0x80;
                p++;
        }
        */
        // FUCKING WELL FUCKING FIX ME ALREADY!

        return s;
}


/*
================
Show progress
================
*/
void ShowProg (edict_t *ent, int current, int max)
{
        char    *pic;
        float   i;
        int     f;

        //ent->sealing = level.time + 0.3;

        if (current >= max)
                ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("p_full");
        else if (current <= 0)
                ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("p_00");
        else
        {
                i = ( ( (float)current / (float)max ) * 100);
                f = ((((int)i / 2) + 0.5) * 2) - 1;
                //gi.dprintf ("%i\n", f);
                if (f < 10)
                        pic = va("p_0%i", f);
                else
                        pic = va("p_%i", f);
                ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (pic);
        }
}


// Dirty
void trace_free (edict_t *ent)
{
        if (ent->owner && (ent->owner->trace == ent))
                ent->owner->trace = NULL;
        G_FreeEdict (ent);
}

void TracenPlace (edict_t *ent, int y, int z)
{
        vec3_t  forward, right, offset, end, start, dist;
        int     mask, mindex, distance;
	trace_t	tr;

        if ((!(ent->client->pers.specials & SI_FLASH_LIGHT))
          && (!(ent->client->pers.specials & SI_LASER_SIGHT)))
                return;

        mask = MASK_SHOT | MASK_WATER;

        VectorSet (offset, 0, y-1, z+1);
        AngleVectors(ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        VectorMA (start, 4096, forward, end);

        tr = gi.trace(start, NULL, NULL, end, ent, mask);

        if (tr.fraction == 1)
		return;

        if (ent->client->pers.specials & SI_LASER_SIGHT)
        {
                mindex = gi.modelindex ("sprites/lsight.sp2");
                VectorSubtract (tr.endpos, start, dist);
                distance = VectorLength (dist);
                if (distance < 1024)
                        ent->client->pers.specials |= SI_LASER_SIGHT_HELP;
                else
                        ent->client->pers.specials &= ~SI_LASER_SIGHT_HELP;
        }
        else
                mindex = gi.modelindex ("sprites/null.sp2");

        if (ent->trace == NULL)
        {
                ent->trace = G_Spawn();
                ent->trace->owner = ent;
                ent->trace->movetype = MOVETYPE_NOCLIP;
                ent->trace->solid = SOLID_NOT;
                ent->trace->classname = "trace";
        }

        if (ent->client->pers.specials & SI_FLASH_LIGHT)
                ent->trace->s.effects |= EF_HYPERBLASTER;
        else
                ent->trace->s.effects &= ~EF_HYPERBLASTER;

        if (ent->client->pers.specials & SI_LASER_SIGHT)
                ent->trace->s.renderfx |= RF_TRANSLUCENT;
        else
                ent->trace->s.renderfx &= ~RF_TRANSLUCENT;

        if (mindex && (ent->trace->s.modelindex != mindex))
                ent->trace->s.modelindex = mindex;

        VectorMA(tr.endpos, -4, forward, ent->trace->s.origin);
        ent->trace->nextthink = level.time + 0.1;
        ent->trace->think = trace_free;

        gi.linkentity (ent->trace);
}
// Dirty
