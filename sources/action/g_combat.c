// g_combat.c

#include "g_local.h"
#include "cgf_sfx_glass.h"

void Add_TeamWound( edict_t *attacker, edict_t *victim, int mod);

/*
============
CanDamage

  Returns true if the inflictor can directly damage the target.  Used for
  explosions and melee attacks.
  ============
*/
qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
        vec3_t  dest;
        trace_t trace;
        
        // bmodels need special checking because their origin is 0,0,0
//GLASS FX
        if ((targ->movetype == MOVETYPE_PUSH)
            ||
            ((targ->movetype == MOVETYPE_FLYMISSILE)
             &&
             (0 == Q_stricmp("func_explosive", targ->classname))
            )
           )
//GLASS FX
        {
                VectorAdd (targ->absmin, targ->absmax, dest);
                VectorScale (dest, 0.5, dest);
                PRETRACE();
                trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
                POSTTRACE();
                if (trace.fraction == 1.0)
                        return true;
                if (trace.ent == targ)
                        return true;
                return false;
        }
        
        PRETRACE();
        trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
        POSTTRACE();
        if (trace.fraction == 1.0)
                return true;
        
        VectorCopy (targ->s.origin, dest);
        dest[0] += 15.0;
        dest[1] += 15.0;
        PRETRACE();
        trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        POSTTRACE();
        if (trace.fraction == 1.0)
                return true;
        
        VectorCopy (targ->s.origin, dest);
        dest[0] += 15.0;
        dest[1] -= 15.0;
        PRETRACE();
        trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        POSTTRACE();
        if (trace.fraction == 1.0)
                return true;
        
        VectorCopy (targ->s.origin, dest);
        dest[0] -= 15.0;
        dest[1] += 15.0;
        PRETRACE();
        trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        POSTTRACE();
        if (trace.fraction == 1.0)
                return true;
        
        VectorCopy (targ->s.origin, dest);
        dest[0] -= 15.0;
        dest[1] -= 15.0;
        PRETRACE();
        trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        POSTTRACE();
        if (trace.fraction == 1.0)
                return true;
        
        
        return false;
}


/*
============
Killed
============
*/
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        if (targ->health < -999)
                targ->health = -999;
        
        if ( targ->client )
        {
                targ->client->bleeding = 0;
                //targ->client->bleedcount = 0;
                targ->client->bleed_remain = 0;
        }
        
        targ->enemy = attacker;
        
        if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
        {
                //              targ->svflags |= SVF_DEADMONSTER;       // now treat as a different content type
                if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
                {
                        level.killed_monsters++;
                        if (coop->value && attacker->client)
                                attacker->client->resp.score++;
                        // medics won't heal monsters that they kill themselves
                        if (strcmp(attacker->classname, "monster_medic") == 0)
                                targ->owner = attacker;
                }
        }
        
        if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
        {       // doors, triggers, etc
                targ->die (targ, inflictor, attacker, damage, point);
                return;
        }
        
        if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
        {
                targ->touch = NULL;
                monster_death_use (targ);
        }
        
        targ->die (targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
        if (damage > 255)
                damage = 255;
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (type);
        //      gi.WriteByte (damage);
        gi.WritePosition (origin);
        gi.WriteDir (normal);
        gi.multicast (origin, MULTICAST_PVS);
}


/*
============
T_Damage

  targ            entity that is being damaged
  inflictor       entity that is causing the damage
  attacker        entity that caused the inflictor to damage targ
  example: targ=monster, inflictor=rocket, attacker=player
  
        dir                     direction of the attack
        point           point at which the damage is being inflicted
        normal          normal vector from that point
        damage          amount of damage being inflicted
        knockback       force to be applied against targ as a result of the damage
        
          dflags          these flags are used to control how T_Damage works
          DAMAGE_RADIUS                   damage was indirect (from a nearby explosion)
          DAMAGE_NO_ARMOR                 armor does not protect from this damage
          DAMAGE_ENERGY                   damage is from an energy based weapon
          DAMAGE_NO_KNOCKBACK             do not affect velocity, just view angles
          DAMAGE_BULLET                   damage is from a bullet (used for ricochets)
          DAMAGE_NO_PROTECTION    kills godmode, armor, everything
          ============
*/
static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
        gclient_t       *client;
        int                     save;
        int                     power_armor_type;
        int                     index;
        int                     damagePerCell;
        int                     pa_te_type;
        int                     power;
        int                     power_used;
        
        if (!damage)
                return 0;
        
        client = ent->client;
        
        if (dflags & DAMAGE_NO_ARMOR)
                return 0;
        
        if (client)
        {
                power_armor_type = PowerArmorType (ent);
                if (power_armor_type != POWER_ARMOR_NONE)
                {
                        index = ITEM_INDEX(FindItem("Cells"));
                        power = client->pers.inventory[index];
                }
        }
        else if (ent->svflags & SVF_MONSTER)
        {
                power_armor_type = ent->monsterinfo.power_armor_type;
                power = ent->monsterinfo.power_armor_power;
        }
        else
                return 0;
        
        if (power_armor_type == POWER_ARMOR_NONE)
                return 0;
        if (!power)
                return 0;
        
        if (power_armor_type == POWER_ARMOR_SCREEN)
        {
                vec3_t          vec;
                float           dot;
                vec3_t          forward;
                
                // only works if damage point is in front
                AngleVectors (ent->s.angles, forward, NULL, NULL);
                VectorSubtract (point, ent->s.origin, vec);
                VectorNormalize (vec);
                dot = DotProduct (vec, forward);
                if (dot <= 0.3)
                        return 0;
                
                damagePerCell = 1;
                pa_te_type = TE_SCREEN_SPARKS;
                damage = damage / 3;
        }
        else
        {
                damagePerCell = 2;
                pa_te_type = TE_SHIELD_SPARKS;
                damage = (2 * damage) / 3;
        }
        
        save = power * damagePerCell;
        if (!save)
                return 0;
        if (save > damage)
                save = damage;
        
        SpawnDamage (pa_te_type, point, normal, save);
        ent->powerarmor_time = level.time + 0.2;
        
        power_used = save / damagePerCell;
        
        if (client)
                client->pers.inventory[index] -= power_used;
        else
                ent->monsterinfo.power_armor_power -= power_used;
        return save;
}

static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
        gclient_t       *client;
        int                     save;
        int                     index;
        gitem_t         *armor;
        
        if (!damage)
                return 0;
        
        client = ent->client;
        
        if (!client)
                return 0;
        
        if (dflags & DAMAGE_NO_ARMOR)
                return 0;
        
        index = ArmorIndex (ent);
        if (!index)
                return 0;
        
        armor = GetItemByIndex (index);
        
        if (dflags & DAMAGE_ENERGY)
                save = ceil(((gitem_armor_t *)armor->info)->energy_protection*damage);
        else
                save = ceil(((gitem_armor_t *)armor->info)->normal_protection*damage);
        if (save >= client->pers.inventory[index])
                save = client->pers.inventory[index];
        
        if (!save)
                return 0;
        
        client->pers.inventory[index] -= save;
        SpawnDamage (te_sparks, point, normal, save);
        
        return save;
}

void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
        if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
                return;
        
        if (attacker == targ || attacker == targ->enemy)
                return;
        
        // if we are a good guy monster and our attacker is a player
        // or another good guy, do not get mad at them
        if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
        {
                if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
                        return;
        }
        
        // we now know that we are not both good guys
        
        // if attacker is a client, get mad at them because he's good and we're not
        if (attacker->client)
        {
                // this can only happen in coop (both new and old enemies are clients)
                // only switch if can't see the current enemy
                if (targ->enemy && targ->enemy->client)
                {
                        if (visible(targ, targ->enemy))
                        {
                                targ->oldenemy = attacker;
                                return;
                        }
                        targ->oldenemy = targ->enemy;
                }
                targ->enemy = attacker;
                if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                        FoundTarget (targ);
                return;
        }
        
        // it's the same base (walk/swim/fly) type and a different classname and it's not a tank
        // (they spray too much), get mad at them
        if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
                (strcmp (targ->classname, attacker->classname) != 0) &&
                (strcmp(attacker->classname, "monster_tank") != 0) &&
                (strcmp(attacker->classname, "monster_supertank") != 0) &&
                (strcmp(attacker->classname, "monster_makron") != 0) &&
                (strcmp(attacker->classname, "monster_jorg") != 0) )
        {
                if (targ->enemy)
                        if (targ->enemy->client)
                                targ->oldenemy = targ->enemy;
                        targ->enemy = attacker;
                        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                                FoundTarget (targ);
        }
        else
        // otherwise get mad at whoever they are mad at (help our buddy)
        {
                if (targ->enemy)
                        if (targ->enemy->client)
                                targ->oldenemy = targ->enemy;
                        targ->enemy = attacker->enemy;
                        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
                                FoundTarget (targ);
        }
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
        //FIXME make the next line real and uncomment this block
        // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
        return false;
}



void BloodSprayThink (edict_t *self)
{
        
        
/*  if ( self->dmg > 0 )
{
self->dmg -= 10;
//              SpawnDamage (TE_BLOOD, self->s.origin, self->movedir, self->dmg);
gi.WriteByte (svc_temp_entity);
gi.WriteByte (TE_SPLASH);
gi.WriteByte (6);
gi.WritePosition (self->s.origin);
gi.WriteDir (self->movedir);
gi.WriteByte (6);       //blood
gi.multicast (self->s.origin, MULTICAST_PVS);

  }
  else
  {
  self->think = G_FreeEdict;
  }
  
        self->nextthink = level.time + 0.1;
        gi.linkentity (self);
        */
        
        G_FreeEdict(self);
        
}

void blood_spray_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == ent->owner)
                return;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 0.1;
}



void spray_blood (edict_t *self, vec3_t start, vec3_t dir, int damage, int mod )
{
        edict_t *blood;
        int speed;
        
        switch(mod) 
        { 
        case MOD_MK23:
                speed = 1800;
                break;
        case MOD_MP5:
                speed = 1500;
                break;  
        case MOD_M4:
                speed = 2400;
                break;
        case MOD_KNIFE:
                speed = 0;
                break;
        case MOD_KNIFE_THROWN:
                speed = 0;
                break;
        case MOD_DUAL:
                speed = 1800;
                break;
        case MOD_SNIPER:
                speed = 4000;
                break;
        default:
                speed = 1800;
        }
        
        
        
        
        blood = G_Spawn();
        VectorNormalize(dir);
        VectorCopy (start, blood->s.origin);
        VectorCopy (dir, blood->movedir);
        vectoangles (dir, blood->s.angles);
        VectorScale (dir, speed, blood->velocity);
        blood->movetype = MOVETYPE_BLOOD;
        blood->clipmask = MASK_SHOT;
        blood->solid = SOLID_BBOX;
        blood->s.effects |= EF_GIB; 
        VectorClear (blood->mins);
        VectorClear (blood->maxs);
        blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
        blood->owner = self;
        blood->nextthink = level.time + speed/1000;//3.2;
        blood->touch = blood_spray_touch;
        blood->think = BloodSprayThink;
        blood->dmg = damage; 
        blood->classname = "blood_spray";
        
        gi.linkentity (blood);
}


// zucc based on some code in Action Quake
void spray_sniper_blood(  edict_t *self, vec3_t start, vec3_t dir )
{
        vec3_t forward;
        int mod = MOD_SNIPER;
        
        VectorCopy (dir, forward);
        
        forward[2] += .03;
        
        spray_blood(self, start, forward, 0, mod);
        
        
        VectorCopy (dir, forward);      
        forward[2] -= .03;
        spray_blood(self, start, forward, 0, mod);
        
        
        VectorCopy (dir, forward);
        if ( (forward[0] > 0)  && (forward[1] > 0) )
        {
                forward[0] -= .03;
                forward[1] += .03;
        }
        if ( (forward[0] > 0)  && (forward[1] < 0) )
        {
                forward[0] += .03;
                forward[1] += .03;
        }
        if ( (forward[0] < 0)  && (forward[1] > 0) )
        {
                forward[0] -= .03;
                forward[1] -= .03;
        }
        if ( (forward[0] < 0)  && (forward[1] < 0) )
        {
                forward[0] += .03;
                forward[1] -= .03;
        }
        spray_blood(self, start, forward, 0, mod);
        
        
        VectorCopy (dir, forward);
        if ( (forward[0] > 0)  && (forward[1] > 0) )
        {
                forward[0] += .03;
                forward[1] -= .03;
        }
        if ( (forward[0] > 0)  && (forward[1] < 0) )
        {
                forward[0] -= .03;
                forward[1] -= .03;
        }
        if ( (forward[0] < 0)  && (forward[1] > 0) )
        {
                forward[0] += .03;
                forward[1] += .03;
        }
        if ( (forward[0] < 0)  && (forward[1] < 0) )
        {
                forward[0] -= .03;
                forward[1] += .03;
        }
        spray_blood(self, start, forward, 0, mod);
        
        VectorCopy (dir, forward);
        spray_blood(self, start, forward, 0, mod);
                                
}


void VerifyHeadShot( vec3_t point, vec3_t dir, float height, vec3_t newpoint)
{
        vec3_t normdir;
        vec3_t normdir2;
        
        
        VectorNormalize2(dir, normdir);
        VectorScale( normdir, height, normdir2 );
        VectorAdd( point, normdir2, newpoint );
}



// zucc adding location hit code
// location hit code based off ideas by pyromage and shockman

#define LEG_DAMAGE (height/2.2) - abs(targ->mins[2]) - 3 
#define STOMACH_DAMAGE (height/1.8) - abs(targ->mins[2]) 
#define CHEST_DAMAGE (height/1.4) - abs(targ->mins[2]) 

#define HEAD_HEIGHT 12.0
qboolean IsFemale (edict_t *ent);


void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
        gclient_t       *client;
        int                     take;
        int                     save;
        int                     asave;
        int                     psave;
        int                     te_sparks;
        int                                             do_sparks = 0;
        int                     damage_type = 0; // used for MOD later
        int                     bleeding = 0; // damage causes bleeding
        int                     head_success = 0;
        int                                             instant_dam = 1;
        
        float           z_rel; 
        int                     height; 
    
        gitem_t*        item;
        
        float           from_top;
        
        vec3_t                  line;
        vec_t                   dist;
        float           targ_maxs2;  //FB 6/1/99
        
        // do this before teamplay check
        if (!targ->takedamage)
                return;
        
        //FIREBLADE
        if (teamplay->value && mod != MOD_TELEFRAG)
        {
                if (lights_camera_action)
                        return;
                
                if (targ != attacker && targ->client && attacker->client &&
                        (targ->client->resp.team == attacker->client->resp.team &&
                            ((int)(dmflags->value) & (DF_NO_FRIENDLY_FIRE))))
                        return;
        }
        //FIREBLADE
        
        
        // damage reduction for shotgun
        // if far away, reduce it to original action levels
        if ( mod == MOD_M3 )
        {
                VectorSubtract(targ->s.origin, inflictor->s.origin, line );
                dist = VectorLength( line );
                if ( dist > 450.0 )
                {
                        damage = damage - 2;
                }
                
        }
        
        item = FindItem(KEV_NAME);

        targ_maxs2 = targ->maxs[2];
        if (targ_maxs2 == 4)
                targ_maxs2 = CROUCHING_MAXS2; //FB 6/1/99
        
        height = abs(targ->mins[2]) + targ_maxs2; 
        
        // locational damage code
        // base damage is head shot damage, so all the scaling is downwards
        if (targ->client)
        { 
                if (!((targ != attacker) && 
                      ((deathmatch->value && ((int)(dmflags->value) 
                        & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value) && 
                      (attacker && attacker->client && 
                      OnSameTeam (targ, attacker) && 
                      ((int)(dmflags->value) & (DF_NO_FRIENDLY_FIRE)))))
                {
                        
                        if ((mod == MOD_MK23) || 
                                (mod == MOD_MP5) || 
                                (mod == MOD_M4) || 
                                (mod == MOD_SNIPER) || 
                                (mod == MOD_DUAL) || 
                                (mod == MOD_KNIFE) || 
                                (mod == MOD_KNIFE_THROWN))
                        { 
                                z_rel = point[2] - targ->s.origin[2]; 
                                from_top = targ_maxs2 - z_rel;
                                if (from_top < 0.0)      //FB 6/1/99
                                        from_top = 0.0;  //Slightly negative values were being handled wrong
                                bleeding = 1;   
                                instant_dam = 0;
                                
                                // damage reduction for longer range pistol shots
                                if ( mod == MOD_MK23 || mod == MOD_DUAL )
                                {
                                        VectorSubtract(targ->s.origin, inflictor->s.origin, line );
                                        dist = VectorLength( line );
                                        if ( dist > 600.0 && dist < 1400.0 )
                                        {
                                                damage = (int)(damage*2/3);
                                        }
                                        else if ( dist > 1400.0 )
                                                damage = (int)(damage*1/2);
                                }
                                
                                
                                //gi.cprintf(targ, PRINT_HIGH, "z_rel is %f\n leg: %f stomach: %f chest: %f\n", z_rel, LEG_DAMAGE, STOMACH_DAMAGE, CHEST_DAMAGE );
                                //gi.cprintf(targ, PRINT_HIGH, "point[2]: %f targ->s.origin[2]: %f height: %d\n", point[2], targ->s.origin[2], height );
                                //gi.cprintf(targ, PRINT_HIGH, "abs(trag->min[2]): %d targ_max[2] %d\n", (int)abs(targ->mins[2]), (int)targ_maxs2);
                                //gi.cprintf(attacker, PRINT_HIGH, "abs(trag->min[2]): %d targ_max[2] %d\n", (int)abs(targ->mins[2]), (int)targ_maxs2); 
                                //gi.cprintf(attacker, PRINT_HIGH, "abs(trag->min[0]): %d targ_max[0] %d\n", (int)abs(targ->mins[0]), (int)targ->maxs[0]); 
                                //gi.cprintf(attacker, PRINT_HIGH, "abs(trag->min[1]): %d targ_max[1] %d\n", (int)abs(targ->mins[1]), (int)targ->maxs[1]); 
                                
                                
                                if ( from_top < 2*HEAD_HEIGHT )
                                {
                                        vec3_t new_point;
                                        VerifyHeadShot( point, dir, HEAD_HEIGHT, new_point );
                                        VectorSubtract( new_point, targ->s.origin, new_point );
                                        //gi.cprintf(attacker, PRINT_HIGH, "z: %d y: %d x: %d\n", (int)(targ_maxs2 - new_point[2]),(int)(new_point[1]) , (int)(new_point[0]) );
                                        
                                        if ( (targ_maxs2 - new_point[2]) < HEAD_HEIGHT 
                                                && (abs(new_point[1])) < HEAD_HEIGHT*.8 
                                                && (abs(new_point[0])) < HEAD_HEIGHT*.8 )
                                                
                                        {
                                                head_success = 1;
                                        }
                                }
                                
                                if ( head_success )
                                {
                                        
                                        damage = damage*1.8 + 1;
                                        gi.cprintf(targ, PRINT_HIGH, "Head damage\n"); 
                                        if (attacker->client) 
                                                gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the head\n", targ->client->pers.netname); 
                                        damage_type = LOC_HDAM;
                                        if ( mod != MOD_KNIFE && mod != MOD_KNIFE_THROWN )
                                                gi.sound(targ, CHAN_VOICE, gi.soundindex("misc/headshot.wav"), 1, ATTN_NORM, 0);                
                                        //else
                                        //      gi.sound(targ, CHAN_VOICE, gi.soundindex("misc/glurp.wav"), 1, ATTN_NORM, 0);                
                                }
                
                                else if (z_rel < LEG_DAMAGE)
                                { 
                                        damage = damage * .25; 
                                        gi.cprintf(targ, PRINT_HIGH, "Leg damage\n"); 
                                        if (attacker->client) 
                                                gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the legs\n", targ->client->pers.netname); 
                                        damage_type = LOC_LDAM;
                                        targ->client->leg_damage = 1;
                                        targ->client->leghits++;
                                } 
                                else if (z_rel < STOMACH_DAMAGE)
                                { 
                                        damage = damage * .4; 
                                        gi.cprintf(targ, PRINT_HIGH, "Stomach damage\n"); 
                                        if (attacker->client) 
                                                gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the stomach\n", targ->client->pers.netname); 
                                        damage_type = LOC_SDAM;
                                } 
                                else //(z_rel < CHEST_DAMAGE)
                                {   
                                        if ( (targ->client->pers.inventory[ITEM_INDEX(item)])
                                                && mod != MOD_KNIFE
                                                && mod != MOD_KNIFE_THROWN
                                                && mod != MOD_SNIPER )
                                        {
                                                if (attacker->client)
                                                {
                                                        gi.cprintf(attacker, PRINT_HIGH, "%s has a Kevlar Vest - AIM FOR THE HEAD!\n",
                                                                targ->client->pers.netname);
                                                        gi.cprintf(targ, PRINT_HIGH, "Kevlar Vest absorbed most of %s's shot\n", 
                                                                attacker->client->pers.netname);
                                                                /*
                                                                if (IsFemale(targ))
                                                                gi.cprintf(attacker, PRINT_HIGH, "You bruised %s through her Kevlar Vest\n", targ->client->pers.netname);
                                                                else
                                                                gi.cprintf(attacker, PRINT_HIGH, "You bruised %s through his Kevlar Vest\n", targ->client->pers.netname);
                                                        */
                                                }
                                                gi.sound(targ, CHAN_ITEM, gi.soundindex("misc/vest.wav"), 1, ATTN_NORM, 0);
                                                damage = (int)(damage/10);
                                                damage_type = LOC_CDAM;
                                                bleeding = 0;
                                                instant_dam = 1;
                                                stopAP = 1;                                                                                             do_sparks = 1;
                                        }
                                        else if ( (targ->client->pers.inventory[ITEM_INDEX(item)])
                                                && mod == MOD_SNIPER )
                                        {
                                                if ( attacker->client )
                                                {
                                                        gi.cprintf(attacker, PRINT_HIGH, "%s has a Kevlar Vest, too bad you have AP rounds...\n",
                                                                targ->client->pers.netname);
                                                        gi.cprintf(targ, PRINT_HIGH, "Kevlar Vest absorbed some of %s's AP sniper round\n",
                                                                attacker->client->pers.netname);
                                                }
                                                damage = damage * .325;
                                                damage_type = LOC_CDAM;
                                        }
                                        else
                                        {
                                                damage = damage * .65; 
                                                gi.cprintf(targ, PRINT_HIGH, "Chest damage\n"); 
                                                if (attacker->client) 
                                                        gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the chest\n", targ->client->pers.netname); 
                                                damage_type = LOC_CDAM;
                                        }
                                        
                                } 
                                /*else
                                {   
                                
                                  // no mod to damage 
                                  gi.cprintf(targ, PRINT_HIGH, "Head damage\n"); 
                                  if (attacker->client) 
                                  gi.cprintf(attacker, PRINT_HIGH, "You hit %s in the head\n", targ->client->pers.netname); 
                                  damage_type = LOC_HDAM;
                                  gi.sound(targ, CHAN_VOICE, gi.soundindex("misc/headshot.wav"), 1, ATTN_NORM, 0);
                        } */
               } 
				if (team_round_going && attacker->client && targ != attacker && OnSameTeam(targ, attacker))
				{
					Add_TeamWound(attacker, targ, mod);
				}
            }   
        }
                
                
        if ( damage_type && !instant_dam) // bullets but not vest hits
        {
                        vec3_t temp;
                        vec3_t temporig;
                        //vec3_t forward;
                        VectorMA (targ->s.origin, 50, dir, temp);
                        //AngleVectors (attacker->client->v_angle, forward, NULL, NULL);
                        VectorScale( dir, 20, temp);
                        VectorAdd( point, temp, temporig );
                        if ( mod != MOD_SNIPER )
                                spray_blood (targ, temporig, dir, damage, mod );
                        else
                        {
                                spray_sniper_blood( targ, temporig, dir );
                        }
        }
                
        if ( mod == MOD_FALLING && !(targ->flags & FL_GODMODE))
        {
                        if ( targ->client && targ->health > 0)
                        {
                                gi.cprintf(targ, PRINT_HIGH, "Leg damage\n"); 
                                targ->client->leg_damage = 1;
                                targ->client->leghits++;
                //      bleeding = 1; for testing
                        }
        }
                
                
        // friendly fire avoidance
        // if enabled you can't hurt teammates (but you can hurt yourself)
        // knockback still occurs
        if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value))
        {
                        if (OnSameTeam (targ, attacker))
                        {
                                if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
                                        damage = 0;
                                else
                                        mod |= MOD_FRIENDLY_FIRE;
                       }
        }

        meansOfDeath = mod;
        locOfDeath = damage_type; // location
        
        client = targ->client;
        
        if (dflags & DAMAGE_BULLET)
                        te_sparks = TE_BULLET_SPARKS;
        else
                        te_sparks = TE_SPARKS;
        
        VectorNormalize(dir);
        
        // bonus damage for suprising a monster
        //      if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
        //              damage *= 2;
        
        if (targ->flags & FL_NO_KNOCKBACK)
                        knockback = 0;
        
        // figure momentum add
        if (!(dflags & DAMAGE_NO_KNOCKBACK))
        {
                        if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
                        {
                                vec3_t  kvel, flydir;
                                float   mass;
                                
                                if ( mod != MOD_FALLING )
                                {
                                        VectorCopy(dir, flydir);
                                        flydir[2] += 0.4;
                                }
                                
                                if (targ->mass < 50)
                                        mass = 50;
                                else
                                        mass = targ->mass;
                                
                                if (targ->client  && attacker == targ)
                                        VectorScale (flydir, 1600.0 * (float)knockback / mass, kvel);      // the rocket jump hack...
                                else
                                        VectorScale (flydir, 500.0 * (float)knockback / mass, kvel);
                                
                                // FB
                                //if (mod == MOD_KICK )
                                //{
                                //        kvel[2] = 0;
                                //}
                                
                                VectorAdd (targ->velocity, kvel, targ->velocity);
                        }
        }
        
        take = damage;
        save = 0;
        
        // check for godmode
        if ( (targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) )
        {
                        take = 0;
                        save = damage;
                        SpawnDamage (te_sparks, point, normal, save);
        }
        
        // zucc don't need this stuff, but to remove it need to change how damagefeedback works with colors
        
        // check for invincibility
        if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
        {
                        if (targ->pain_debounce_time < level.time)
                        {
                                gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
                                targ->pain_debounce_time = level.time + 2;
                        }
                        take = 0;
                        save = damage;
        }
        
        psave = CheckPowerArmor (targ, point, normal, take, dflags);
        take -= psave;
        
        asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
        take -= asave;
        
                //treat cheat/powerup savings the same as armor
                asave += save;
                
                // team damage avoidance
                if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
                        return;
        if (    (mod == MOD_M3) 
             || (mod == MOD_HC) 
             || (mod == MOD_HELD_GRENADE) 
             || (mod == MOD_HG_SPLASH) 
             || (mod == MOD_G_SPLASH) 
             || (mod == MOD_BREAKINGGLASS)
           )
        {
//FB 6/3/99 - shotgun damage report stuff
                                int playernum = targ - g_edicts;
				playernum--;
				if (playernum >= 0 &&
					playernum <= game.maxclients - 1)
					*(took_damage + playernum) = 1;
//FB 6/3/99

                        bleeding = 1;
                        instant_dam = 0;
        }
                
                /*        if ( (mod == MOD_M3) || (mod == MOD_HC) )
                {
                instant_dam = 1;            
                remain = take % 2;
                take = (int)(take/2); // balances out difference in how action and axshun handle damage/bleeding
                
                  }
                */              
                // do the damage
        if (take)
                {               
                        // zucc added check for stopAP, if it hit a vest we want sparks
                        if (((targ->svflags & SVF_MONSTER) || (client)) && !do_sparks )
                                SpawnDamage (TE_BLOOD, point, normal, take);
                        else
                                SpawnDamage (te_sparks, point, normal, take);
                        
                        // all things that have at least some instantaneous damage, i.e. bruising/falling
                        if ( instant_dam ) 
                                targ->health = targ->health - take;
                        
                        if (targ->health <= 0)
                        {
                                if ((targ->svflags & SVF_MONSTER) || (client))
                                        targ->flags |= FL_NO_KNOCKBACK;
                                Killed (targ, inflictor, attacker, take, point);
                                return;
                        }
        }
        
        if (targ->svflags & SVF_MONSTER)
        {
                        M_ReactToDamage (targ, attacker);
                        if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
                        {
                                targ->pain (targ, attacker, knockback, take);
                                // nightmare mode monsters don't go into pain frames often
                                if (skill->value == 3)
                                        targ->pain_debounce_time = level.time + 5;
                        }
        }
        else if (client)
        {
                        if (!(targ->flags & FL_GODMODE) && (take))
                                targ->pain (targ, attacker, knockback, take);
        }
        else if (take)
        {
                        if (targ->pain)
                                targ->pain (targ, attacker, knockback, take);
        }
        
        // add to the damage inflicted on a player this frame
        // the total will be turned into screen blends and view angle kicks
        // at the end of the frame
        if (client)
        {
                        client->damage_parmor += psave;
                        client->damage_armor += asave;
                        client->damage_blood += take;
                        client->damage_knockback += knockback;
                        //zucc handle adding bleeding here
                        if ( damage_type && bleeding ) // one of the hit location weapons
                        {
                        /* zucc add in partial bleeding, changed
                        if ( client->bleeding < 4*damage*BLEED_TIME )
                        {
                        client->bleeding = 4*damage*BLEED_TIME + client->bleeding/2;
                        
                          }
                          else
                          {
                          client->bleeding += damage*BLEED_TIME*2;
                          
                        }*/
                                client->bleeding += damage*BLEED_TIME;
                                VectorSubtract (point, targ->absmax, targ->client->bleedloc_offset);
                                //VectorSubtract(point, targ->s.origin,  client->bleedloc_offset);
                                
                        }
                        else if ( bleeding )
                        {
                        /*
                        if ( client->bleeding < damage*BLEED_TIME )
                        {
                        client->bleeding = damage*BLEED_TIME;
                        //client->bleedcount = 0;
                        }*/
                                client->bleeding += damage*BLEED_TIME;
                                VectorSubtract (point, targ->absmax, targ->client->bleedloc_offset);
                                //VectorSubtract(point, targ->s.origin,  client->bleedloc_offset);
                                
                        }
                        if ( attacker->client )
                        {
                                attacker->client->resp.damage_dealt += damage; 
                                client->attacker = attacker;
                                client->attacker_mod = mod;
                                client->attacker_loc = damage_type;
                                client->push_timeout = 50;
                                //VectorCopy(dir, client->bleeddir );
                                //VectorCopy(point, client->bleedpoint );
                                //VectorCopy(normal, client->bleednormal);
                                
                        }
                        
                        VectorCopy (point, client->damage_from);
        }
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
        float   points;
        edict_t *ent = NULL;
        vec3_t  v;
        vec3_t  dir;

        while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
        {
                if (ent == ignore)
                        continue;
                if (!ent->takedamage)
                        continue;
                
                VectorAdd (ent->mins, ent->maxs, v);
                VectorMA (ent->s.origin, 0.5, v, v);
                VectorSubtract (inflictor->s.origin, v, v);
                points = damage - 0.5 * VectorLength (v);
                //zucc reduce damage for crouching, max is 32 when standing
                if (ent->maxs[2] < 20 ) 
                {
                       points = points * 0.5; // hefty reduction in damage
                }
                //if (ent == attacker)
                //points = points * 0.5; 
                if (points > 0)
                {
                  #ifdef _DEBUG
                  if (0 == Q_stricmp(ent->classname, "func_explosive"))
                    {
                      CGF_SFX_ShootBreakableGlass(ent, inflictor, 0, mod);
                    }
                  else
                  #endif
                        if (CanDamage (ent, inflictor))
                        {
                                VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
                                // zucc scaled up knockback(kick) of grenades
                                T_Damage (ent, inflictor, attacker, dir, ent->s.origin, vec3_origin, (int)(points*.75), (int)(points*.75), DAMAGE_RADIUS, mod);
                        }
                }
        }
}
