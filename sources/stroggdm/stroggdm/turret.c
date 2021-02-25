#include "g_local.h"
// ******************************************************
// * StroggDM 8.0
// * Auto Sentry
// ******************************************************
void Create_Sentry (edict_t *ent)
{
	edict_t *sentry;           // define sentry as an entity
	edict_t *base;             // define base as an entity
	vec3_t forward;            // define foward as vector
	
	if (ent->client->sentry == 1)    // if sentry still exists, don't create again
	{
		return;
	}
	else                             // otherwise....
	{
		ent->client->sentry = 1;
	}
	
	// *********** Sentry Base ****************
	base = G_Spawn();
	base->s.modelindex         = gi.modelindex("models/objects/minelite/light1/tris.md2");
	base->s.modelindex2        = 0;
	base->owner                = ent;              // who's your dady
	base->takedamage           = DAMAGE_YES;        // can't damage it
	base->movetype             = MOVETYPE_TOSS;    // throw this base and it falls to ground
	base->solid                = SOLID_BBOX;       // the base is solid
	//base->think                = Base_Explode;     //
	//base->nextthink            = level.time + 20;
	base->s.effects            = 0;          // StroggDM Note: Rotating weapon?
	base->health				= 100;
	base->die					= Base_Die;
	
	VectorClear(base->s.angles);                   // clear sentry angles
	VectorCopy(ent->s.origin, base->s.origin);
//	VectorMA(ent->s.origin, 100, forward, base->s.origin); // Put the model in front of us
	
	base->s.origin[2] += 64;                       // a little higher please
	
	// This should set the bounderies for our sentry base
	VectorSet(base->mins,-12,-12,-24);               // Size of BBOX - a, b, c
	VectorSet(base->maxs, 12, 12, 24);               // Size of BBOX - e, f, g
	
	// *********** Sentry Weapon ****************
	
	sentry = G_Spawn();
	sentry->classname          = "sentry";
	base->sentry_weapon        = sentry;           // base's link to sentry
	sentry->sentry_base        = base;             // sentry's link to base
	sentry->s.modelindex       = gi.modelindex("models/weapons/g_blast/tris.md2");
	sentry->s.modelindex2      = 0;
	sentry->owner              = ent;              // who's your dady
	sentry->takedamage         = DAMAGE_NO;        // can damage it
	sentry->dmg                = 5;                // Amount of Damage it will Inflict
	sentry->movetype           = MOVETYPE_TOSS;    // Rests on body
	sentry->solid              = SOLID_BBOX;       // the Sentry is solid
	sentry->think              = Sentry_Seek;
	sentry->nextthink          = level.time + 0.1;
	sentry->s.effects          = 0;
	
	VectorCopy(base->s.origin, sentry->s.origin);
	
	sentry->s.origin[2] -= 8;                      // How High
	
	// Here for reference only
	//sentry->s.origin[1] += 12;                   // How far to side
	//sentry->s.origin[0] -= 22;                   // How much to back
	
	// This should set the bounderies for our sentry's weapon
	VectorSet(sentry->mins,-12,-12,-24);             // Size of BBOX for Touch()
	VectorSet(sentry->maxs, 12, 12, 36);             // Size of BBOX for Touch()
	
	gi.linkentity (sentry);
	gi.linkentity (base);

	VectorCopy(sentry->sentry_base->s.origin, sentry->s.origin);
	
	sentry->s.origin[2] -= 8;        // How High
	
	ent->client->sentry_sent = sentry;
	ent->client->sentry_base = base;
}

// ************************************************
// * Sentry_FaceEnemy
// *************************************************

void Sentry_FaceEnemy(edict_t *sentry)
{	
	vec3_t v;	
	vec3_t dir;	
	vec3_t forward;	
	vec3_t right;	
	vec3_t up;
	
	
	//VectorCopy(sentry->sentry_base->s.origin, sentry->s.origin);
	//sentry->s.origin[2] -= 8;        // How High
	
	// Calculate enemy's direction
	VectorSubtract(sentry->enemy->s.origin, sentry->s.origin, dir);
	
	// Set the second dir array to point in the same direction as the first dir
	vectoangles (dir, dir);
	
	// Copy the direction to the sentry's s.angle and movedir
	VectorCopy (dir, sentry->s.angles);
	VectorCopy (dir, sentry->movedir);
	
	VectorNormalize(dir);	
}


// *************************************************
// * Sentry_Seek
// *************************************************

void Sentry_Seek(edict_t *self)
{
	edict_t *target;
	
	if (self->owner->client->sentry == 0)
	{
		G_FreeEdict(self);
		return;
	}		
	self->enemy = NULL;
			
	// look for a target
	target = findradius(NULL, self->s.origin, 800);
	while(target)
	{
		if (visible(self, target))              // is target visible?
		{
			if (((strcmp(target->classname, "player") == 0 || strcmp(target->classname, "bot") == 0)) && target != self->owner)   // is target a player ?
			{
				if (ctf->value)
				{
					if (!(target->client->resp.ctf_team == self->owner->client->resp.ctf_team))
						self->enemy = target;
				}
				else
					self->enemy = target;
			}
		}
		
		// next target
		target = findradius(target, self->s.origin, 800);
	}
	
	if (self->enemy != NULL)  // did we find an enemy?
	{
		Sentry_FaceEnemy(self);                        // face the enemy
		self->think         = sentry_fire;             // set think to fire
		self->nextthink     = level.time + 0.1;        // on next frame
	}
	
	self->nextthink = level.time + 0.1;
}

// ************************************************
// * Base_Explode & Base_Die
// *************************************************

void Base_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 1 * FRAMETIME;
	self->think = Base_Explode;
}

void Base_Explode(edict_t *base)
{
	base->owner->client->sentry = 0;

	// Destroy the base in a big fireball.
	G_Spawn_Explosion(TE_EXPLOSION2, base->s.origin, base->s.origin);
	
	
	
	// Throw base debris all over the place...
	ThrowDebris(base, "models/objects/debris2/tris.md2", 3.50, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 2.50, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 1.50, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 4.50, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 3.75, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 2.30, base->s.origin, NULL);
	ThrowDebris(base, "models/objects/debris2/tris.md2", 1.00, base->s.origin, NULL);
	
	// Assign any Radius Damage frags to the base's owner..
	T_RadiusDamage(base, base->owner, 40, NULL, 400, MOD_SPLASH);
	
	// Free the base entity & weapon entity
	// Make sure the sentry's owner's variables for sentry get NULLed
	base->owner->client->sentry_sent = NULL;
	base->owner->client->sentry_base = NULL;
	G_FreeEdict(base->sentry_weapon);
	G_FreeEdict(base);
}



// ************************************************
// * Sentry Fire
// *************************************************
void sentry_fire(edict_t *sentry)

{
	
	vec3_t forward, right, dir;
	vec3_t start;
	int r = rndnum (0, 4);
	char *folder;
	char *file;
	char *file2;
	float wait;
	float nextfire;
	vec3_t from={0,0,0}, end={0,0,0};
	int weaponnum = sentry->sent_weapon;

	if (!weaponnum)
	{
		file = "blastf1a";
		file2 = NULL;
		folder = "weapons";
		wait = 0;
		nextfire = 0.2;
	}
	else if (weaponnum == 1)
	{
		file = "Shotgf1b";
		file2 = "Shotgr1b";
		folder = "weapons";
		wait = 0.1;
		nextfire = 1.0;
	}
	else if (weaponnum == 2)
	{
		if (r == 0)
			file = "Machgf1b";
		else if (r == 1)
			file = "Machgf2b";
		else if (r == 2)
			file = "Machgf3b";
		else if (r == 3)
			file = "Machgf4b";
		else
			file = "Machgf5b";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 0.1;
	}
	else if (weaponnum == 3)
	{
		folder = "weapons";
		file = "GRENLF1A";
		file2 = "GRENLR1b";
		wait = 0.1;
		nextfire = 1.2;
	}
	else if (weaponnum == 4)
	{
		file = "ROCKLF1A";
		file2 = "ROCKLR1b";
		wait = 0.1;
		nextfire = 1.1;
		folder = "weapons";
	}
	else if (weaponnum == 5)
	{
		file = "Hyprbf1a";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 0.1;
	}
	else if (weaponnum == 6)
	{
		file = "RAILGF1A";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 1.3;
	}
	else if (weaponnum == 7)
	{
		file = "bfg_fire";
		folder = "makron";
		file2 = NULL;
		wait = 0;
		nextfire = 2.8;
	}
	else if (weaponnum == 8)
	{
		file = "rippfire";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 0.3;
	}
	else if (weaponnum == 9)
	{
		file = "shootplasma";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 1.0;
	}
	else if (weaponnum == 10)
	{
		file = "nail1";
		folder = "weapons";
		file2 = NULL;
		wait = 0;
		nextfire = 0.1;
	}
	
	// Calculate the direction were enemy is
	VectorSubtract(sentry->enemy->s.origin, sentry->s.origin, dir);
	
	// Take apart s.angles into “forward”, “right” and “up” (NULL in this case)
	AngleVectors (sentry->s.angles, forward, right, NULL);
	
	// Copy the sentry’s vector to variable “start”
	VectorCopy(sentry->s.origin, start);
	
	// Change Start position a little higher so it doesn’t fire from the ground
	start[2] += 26;
	
	// shoot the bastard
	/*if (sentry->owner->client->resp.class == 13)
	{
		sound (sentry, CHAN_WEAPON, "weapons", file, ATTN_NORM);
		fire_bullet (sentry, start, dir, sentry->dmg - 3, 8, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	}
	else
	{*/

	if (!weaponnum)
	{
		fire_blaster (sentry, start, forward, 9, 1000, EF_BLASTER, MOD_BLASTER);
	}
	else if (weaponnum == 1)
	{
		fire_shotgun (sentry, start, forward, 5, 10, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	}
	else if (weaponnum == 2)
	{
		fire_bullet (sentry, start, forward, 3, 5, 300, 500, MOD_MACHINEGUN);
	}
	else if (weaponnum == 3)
	{
		fire_grenade (sentry, start, forward, 45, 700, 2.5, 100);
	}
	else if (weaponnum == 4)
	{
		fire_rocket (sentry, start, forward, 90, 1000, 100, 100);
	}
	else if (weaponnum == 5)
	{
		fire_blaster (sentry, start, forward, 6, 900, EF_HYPERBLASTER, MOD_HYPERBLASTER);
	}
	else if (weaponnum == 6)
	{
		fire_rail (sentry, start, forward, 100, 50, MOD_RAILGUN);
	}
	else if (weaponnum == 7)
	{
		fire_bfg (sentry, start, forward, 200, 600, 100);
	}
	else if (weaponnum == 8)
	{
		fire_ionripper (sentry, start, forward, 8, 700, EF_IONRIPPER);
	}
	else if (weaponnum == 9)
	{
		fire_phlas (sentry, start, forward, 21, 850, 35, 35);
	}
	else if (weaponnum == 10)
	{
		fire_flechette (sentry, start, forward, 8, 900, 30);
	}


		sound (sentry, CHAN_WEAPON, folder, file, ATTN_NORM);

		if (file2)
			gi.sound (sentry, CHAN_AUTO, gi.soundindex(va("%s/%s.wav", folder, file2)), 1, ATTN_NORM, wait);
		
		//fire_blaster (sentry, start, forward, sentry->dmg + 12, 1000, EF_BLASTER, MOD_BLASTER);
	//}
	
	sentry->think       = Sentry_Seek;      // Start looking for the enemy again
	sentry->nextthink   = level.time + nextfire; // on the next frame
	
}

void Sentry_ControlWeapon (edict_t *ent)
{
			if (ent->client->sentry == 1)
			{
				edict_t *sentry_weapon = ent->client->sentry_sent;
				int weaponnum = sentry_weapon->sent_weapon;

				if (!weaponnum) // Has blaster, move to shotgun
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_shotg/tris.md2");
					sentry_weapon->sent_weapon = 1;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Shotgun.\n");
				}
				else if (weaponnum == 1) // Has shotgun, move to machinegun
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_machn/tris.md2");
					sentry_weapon->sent_weapon = 2;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Machinegun.\n");
				}
				else if (weaponnum == 2) // Has machinegun, move to grenade launcher
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_launch/tris.md2");
					sentry_weapon->sent_weapon = 3;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Grenade Launcher.\n");
				}
				else if (weaponnum == 3) // Has grenade launcher, move to rocket launcher
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_rocket/tris.md2");
					sentry_weapon->sent_weapon = 4;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Rocket Launcher.\n");
				}
				else if (weaponnum == 4) // Has rocket launcher, move to hyperblaster
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_hyperb/tris.md2");
					sentry_weapon->sent_weapon = 5;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Hyperblaster.\n");
				}
				else if (weaponnum == 5) // Has hyperblaster, move to railgun
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_rail/tris.md2");
					sentry_weapon->sent_weapon = 6;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Railgun.\n");
				}
				else if (weaponnum == 6) // Has railgun, move to BFG
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_bfg/tris.md2");
					sentry_weapon->sent_weapon = 7;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the BFG.\n");
				}
				else if (weaponnum == 7) // Has BFG, move to Ion Ripper 
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_boom/tris.md2");
					sentry_weapon->sent_weapon = 8;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Ion Ripper.\n");
				}
				else if (weaponnum == 8) // Has Ion Ripper, move to Phalanx
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_shotx/tris.md2");
					sentry_weapon->sent_weapon = 9;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Phalanx.\n");
				}
				else if (weaponnum == 9) // Has Phalanx, move to ETF Rifle
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_etf_rifle/tris.md2");
					sentry_weapon->sent_weapon = 10;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the ETF Rifle.\n");
				}
				else if (weaponnum == 10) // Has ETF Rifle, back to blaster.
				{
					sentry_weapon->s.modelindex = gi.modelindex("models/weapons/g_blast/tris.md2");
					sentry_weapon->sent_weapon = 0;
					safe_cprintf (ent, PRINT_HIGH, "Your sentry is using the Blaster.\n");
				}
			}
			else
				safe_cprintf (ent, PRINT_HIGH, "You don't have a sentry out!\n");
}