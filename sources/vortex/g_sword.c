#include "g_local.h"

extern qboolean is_quad;

#define SWORD_DEATHMATCH_DAMAGE sabre_damage_initial->value//130
#define SWORD_KICK sabre_kick->value//400
#define SWORD_RANGE sabre_range->value//60
extern byte		is_silenced;//K03

void lance_think (edict_t *self)
{
	trace_t tr;
	vec3_t forward, end;

	if (!G_EntIsAlive(self->owner) || level.time > self->delay)
	{
		G_FreeEdict(self);
		return;
	}

	vectoangles(self->velocity, self->s.angles);
	ValidateAngles(self->s.angles);
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, self->dmg_radius, forward, end);
	tr = gi.trace (self->s.origin, NULL, NULL, end, self->owner, MASK_SHOT);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (self->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	// if we hit something, damage it and burn if it's upgraded
	if (tr.ent && tr.ent->takedamage && T_Damage(tr.ent, self, self->owner, forward, tr.endpos, tr.plane.normal, self->dmg, self->dmg, DAMAGE_ENERGY, MOD_SWORD)
		&& self->owner->myskills.weapons[WEAPON_SWORD].mods[3].current_level >= 1)
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex("espada/hit1_2.wav") , 1, ATTN_NORM, 0); 
		burn_person(tr.ent, self->owner, self->radius_dmg);
	}

	// if we hit anything at all, remove ent
	if (tr.fraction < 1)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_EXPLOSION);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PVS);

		G_FreeEdict(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_lance (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int burn_damage, float speed, float length)
{
	edict_t	*lance;

	//  entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// spawn grenade entity
	lance = G_Spawn();
	VectorCopy (start, lance->s.origin);
	lance->movetype = MOVETYPE_TOSS;
	lance->s.modelindex = gi.modelindex("models/bow/w_bow.md2"); //no funcionó
	lance->svflags |= SVF_NOCLIENT;
	lance->owner = self;
	lance->think = lance_think;
	lance->dmg_radius = length;
	lance->dmg = damage;
	lance->radius_dmg = burn_damage;
	lance->classname = "lance";
	lance->delay = level.time + 10.0;
	gi.linkentity (lance);
	lance->nextthink = level.time + FRAMETIME;
	vectoangles(aimdir, lance->s.angles);

	// adjust velocity
	VectorScale (aimdir, speed, lance->velocity);
	lance->velocity[2] += 300;
}

/*
=============
fire_sword

attacks with the beloved sword of the highlander
edict_t *self - entity producing it, yourself
vec3_t start - The place you are
vec3_t aimdir - Where you are looking at in this case
int damage - the damage the sword inflicts
int kick - how much you want that bitch to be thrown back
=============
*/

void fire_sword ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;
	vec3_t begin;
	vec3_t begin_offset;
	int sword_bonus = 1;
	int swordrange;
	
	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	if (self->myskills.class_num == CLASS_PALADIN)	//doomie
		sword_bonus = 1.3;
	swordrange = SABRE_INITIAL_RANGE * sword_bonus + (SABRE_ADDON_RANGE * self->myskills.weapons[WEAPON_SWORD].mods[2].current_level * sword_bonus);

	VectorSet( begin_offset,0,0,self->viewheight-8);
	VectorAdd( self->s.origin, begin_offset, begin);

    // Figure out what we hit, if anything:

    VectorMA (start, swordrange, aimdir, end);  //calculates the range vector                      

    tr = gi.trace (begin, NULL, NULL, end, self, MASK_SHOT);
                        // figuers out what in front of the player up till "end"

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (begin);
	gi.WritePosition (tr.endpos);
	gi.multicast (begin, MULTICAST_PHS);
	
	  
	// Figure out what to do about what we hit, if anything
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
				if (self->myskills.weapons[WEAPON_SWORD].mods[4].current_level < 1)
					gi.sound (self, CHAN_WEAPON, gi.soundindex("espada/hit1_2.wav") , 1, ATTN_NORM, 0); 

				if (T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_SWORD))
				{
					if (self->myskills.weapons[WEAPON_SWORD].mods[3].current_level >= 1)
						burn_person(tr.ent, self, (int)(SABRE_ADDON_HEATDAMAGE * self->myskills.weapons[WEAPON_SWORD].mods[3].current_level * sword_bonus));
				}

            }        
            //else gi.multicast(begin,MULTICAST_PHS);
        }

		if (self->myskills.weapons[WEAPON_SWORD].mods[4].current_level < 1)
		{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (self-g_edicts);
		gi.WriteByte (MZ_IONRIPPER|MZ_SILENCED);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		}
    }
}

void sword_attack (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t  forward, right;
	vec3_t  start;
	vec3_t  offset;

	int swordrange = SABRE_INITIAL_RANGE + SABRE_ADDON_RANGE * ent->myskills.weapons[WEAPON_SWORD].mods[2].current_level;

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	//VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorSet(offset, 24, 0, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

    fire_sword (ent, start, forward, damage, SABRE_INITIAL_KICK + SABRE_ADDON_KICK * ent->myskills.weapons[WEAPON_SWORD].mods[0].current_level);
}

void Weapon_Sword_Fire (edict_t *ent)
{
	int sword_bonus = 1;
	int damage;
	float temp;
	
	// special rules; flag carrier can't use weapons
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		return;

	if (ent->myskills.class_num == CLASS_PALADIN)
		sword_bonus = 1.5;
	damage = SABRE_INITIAL_DAMAGE + (SABRE_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_SWORD].mods[0].current_level * sword_bonus);

	// sword forging reduces the per-frame damage penalty
	temp = 0.8 + 0.007 * ent->myskills.weapons[WEAPON_SWORD].mods[1].current_level;

	 if ((temp < 1.0) && (ent->client->ps.gunframe - 5 > 0))
		damage *= pow(temp, ent->client->ps.gunframe - 5);
	
	 //gi.dprintf("damage=%d\n", damage);

	 if ((ent->client->ps.gunframe == 5) && (ent->myskills.weapons[WEAPON_SWORD].mods[4].current_level < 1))
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("espada/arm.wav") , 1, ATTN_NORM, 0);

     if ( ent->client->buttons & BUTTON_ATTACK )
		sword_attack (ent, vec3_origin, damage);
     ent->client->ps.gunframe++;
}

void Weapon_Lance_Fire (edict_t *ent)
{
	int sword_bonus = 1.0;
	int damage, burn_damage;
	float speed;
	vec3_t  forward, start;
	
	// special rules; flag carrier can't use weapons
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		return;

	// calculate knight bonus
	if (ent->myskills.class_num == CLASS_PALADIN)
		sword_bonus = 1.5;

	damage = SABRE_INITIAL_DAMAGE + (SABRE_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_SWORD].mods[0].current_level * sword_bonus);
	burn_damage = SABRE_ADDON_HEATDAMAGE * ent->myskills.weapons[WEAPON_SWORD].mods[3].current_level * sword_bonus;
	speed = 700 + (15 * ent->myskills.weapons[WEAPON_SWORD].mods[2].current_level * sword_bonus);

	// lance modifier
	damage *= 2;

	 if (ent->myskills.weapons[WEAPON_SWORD].mods[4].current_level < 1)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("b/bowfire1.wav") , 1, ATTN_NORM, 0);

	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	G_EntMidPoint(ent, start);

	// apply view kick
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// fire lance
	fire_lance(ent, start, forward, damage, burn_damage, speed, 64);

	// show muzzle flare
	if (ent->myskills.weapons[WEAPON_SWORD].mods[4].current_level < 1)
	{
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent-g_edicts);
		gi.WriteByte(MZ_IONRIPPER|MZ_SILENCED);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}

	// advance weapon frame
     ent->client->ps.gunframe++;
}

void Weapon_Sword (edict_t *ent)
{
	static int      pause_frames[] = {19, 32, 0};
	static int      sword_frames[] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 0};
	static int		lance_frames[] = {5, 0};
	
	if (ent->client->weapon_mode)
		Weapon_Generic (ent, 4, 20, 52, 55, pause_frames, lance_frames, Weapon_Lance_Fire);
	else
		Weapon_Generic (ent, 4, 20, 52, 55, pause_frames, sword_frames, Weapon_Sword_Fire);
}