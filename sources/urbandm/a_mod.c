//a_mod.c

#include "g_local.h"
#include "m_player.h"
#define JUMPER_MODEL gi.modelindex ("models/mod/jumper.md2")

extern qboolean is_quad;
extern byte is_silenced;

//=================
// Jumping plat
//
// Throw it, then walk on it,
// and grab some stars !
//
//=================

static void Jumping_Plat_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	ent->movetype = MOVETYPE_NONE;
	if (other != world)
	{
	vec3_t forward, up;
	vec3_t new_velo;
	AngleVectors (other->s.angles, forward, NULL, up);
	VectorCopy (other->velocity, new_velo);
	new_velo[2] += 400;
	new_velo[0] += 20;
	VectorAdd (other->velocity, new_velo, other->velocity);
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
	return;
	}
}

//self is the player
void Place_Jumping_Plat (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
{
	edict_t	*plat;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	plat = G_Spawn();
	gi.setmodel (plat, "models/objects/dmspot/tris.md2");
	VectorCopy (start, plat->s.origin);
	VectorScale (aimdir, speed/3, plat->velocity);
	VectorMA (plat->velocity, 40 + crandom() * 10.0, up, plat->velocity);
	plat->movetype = MOVETYPE_BOUNCE;
	//plat->clipmask = MASK_PLAYERSOLID;
	plat->solid = SOLID_TRIGGER;
	plat->s.effects |= EF_GRENADE;
	VectorClear (plat->mins);
	VectorClear (plat->maxs);
	VectorSet (plat->mins, -32, -32, -24);
	VectorSet (plat->maxs, 32, 32, -16);

	//plat->s.modelindex = JUMPER_MODEL;
	plat->owner = self;
	plat->touch = Jumping_Plat_Touch;
	plat->nextthink = level.time + 60;
	plat->think = G_FreeEdict;
	plat->classname = "jumping_plat";

	gi.linkentity (plat);
}

extern void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

void Fire_Jumping_Plat (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;

	/*if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] < RAILGUN_TURRET_COST)
	{
		gi.cprintf (ent, PRINT_HIGH, "You need at least %d cells to fire your special\n", RAILGUN_TURRET_COST);
		return;
	}
	else
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= RAILGUN_TURRET_COST;
	*/
	
	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	Place_Jumping_Plat (ent, start, forward, 600);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


//==============================================
// HEAD !!
//==============================================

//Flags
#define HEAD_ON_GROUND	0x00000001	//the owner can't pick his head up.
#define HEAD_ON_PLAYER	0x00000002	//player is carring it
#define HEAD_DROPPED	0x00000004 //can be grabbed by both the owner and a player

//ent is the head picked up
static void Head_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//gi.dprintf ("Touch !\n");

	if ((ent->head_flag & HEAD_ON_GROUND) || (ent->head_flag & HEAD_DROPPED))
	{
		if (other->client)
		{
			if (other->head_num >= 5)
				return;

			if (!other->deadflag)
			{
				if (ent->owner)
				{
					if (ent->owner->client)
					{
						if (ent->head_flag & HEAD_ON_GROUND)
							if (other == ent->owner)
								return;
					
						gi.setmodel (ent, "models/mod/head/tris.md2");
						ent->head_flag = HEAD_ON_PLAYER;
						ent->touch = NULL;
						ent->die = NULL;
						ent->carrier = other;
						other->head_num++;
						other->cap_head[other->head_num] = ent;
						gi.bprintf (PRINT_CHAT, "%s found %s's head !\n", other->client->pers.netname, ent->owner->client->pers.netname);
						gi.sound (ent, CHAN_VOICE, gi.soundindex ("world/flesh1.wav"), 1, ATTN_NORM, 0);
					}//else gi.dprintf ("Owner isn't a client\n");
				}//else gi.dprintf ("No owner\n");
			}//else gi.dprintf ("Other has deadflag\n");
		} //else gi.dprintf ("Not a client\n");
	}//else gi.dprintf ("Not on ground flag\n");
		

}

void CopyHead (edict_t *ent)
{
	edict_t *new_head = NULL;

	gi.unlinkentity (ent);
	new_head = G_Spawn();
	new_head->s = ent->s;
	VectorCopy (ent->mins, new_head->mins);
	VectorCopy (ent->maxs, new_head->maxs);
	VectorCopy (ent->absmin, new_head->absmin);
	VectorCopy (ent->absmax, new_head->absmax);
	VectorCopy (ent->size, new_head->size);
	VectorCopy (ent->s.origin, new_head->s.origin);
	new_head->classname = "pickup_head";
	new_head->owner = ent;
	new_head->solid = ent->solid;
	new_head->clipmask = ent->clipmask;
	new_head->movetype = ent->movetype;
	new_head->touch = Head_Touch;
	new_head->head_flag = HEAD_ON_GROUND;

	gi.linkentity (new_head);
}

#define HEAD_DIST	-15

void DropPlayerHeads (edict_t *ent)
{
	int i;
	edict_t *head = NULL;

	if (ent->head_num < 1)
		return;

	for (i = 1; i < MAX_HEAD; i++)
	{
		if (ent->cap_head[i] == NULL)
		{
			//gi.dprintf ("Warning : drop player heads bad head num %d\n", i);
			continue;
		}
		else
		{
			vec3_t speed;
			head = ent->cap_head[i];
			head->head_flag = HEAD_DROPPED;
			head->solid = SOLID_TRIGGER;
			head->movetype = MOVETYPE_BOUNCE;
			head->owner = ent->cap_head[i]->owner;

			head->clipmask = ent->cap_head[i]->clipmask;
			head->movetype = ent->cap_head[i]->movetype;

			head->touch = Head_Touch;

			head->nextthink = level.time + 30;
			head->think = G_FreeEdict;

			head->s.effects |= EF_GIB;
			VectorSet (head->mins, -4, -4, -4);
			VectorSet (head->maxs, 4, 4, 4);
			
			gi.linkentity (head);

			VectorNormalize (head->velocity);
			VectorScale (head->velocity, 100, speed);
			speed[2] += 40;
			VectorAdd (head->velocity, speed, head->velocity);
			
		}
	}
}


void CapturedHeadThink (edict_t *ent, edict_t *head)
{
	if (head->head_flag & HEAD_ON_PLAYER)
	{
		vec3_t dir, forward;

		VectorCopy (ent->s.angles, dir);
		AngleVectors (dir, forward, NULL, NULL);
		VectorScale (forward, HEAD_DIST, forward);
		forward [2] += (ent->viewheight / 2);
		VectorAdd (forward, ent->s.origin, forward);
		//VectorCopy (ent->s.origin, head->s.origin);
		VectorCopy (forward, head->s.origin);
		head->s.angles[0] += rand()%2;
		head->s.angles[1] += rand()%2;
		head->s.angles[2] += rand()%2;
		head->solid = SOLID_NOT;
		head->s.effects |= EF_GIB;
		head->head_flag = HEAD_ON_PLAYER;
		gi.linkentity (head);
	}
}



//====================================================== 
//========== Spawn Temp Entity Functions =============== 
//====================================================== 
/* 
Spawns (type) Splash with {count} particles of {color} at {start} moving 
in {direction} and Broadcasts to all in Potentially Visible Set from 
vector (origin) 

TE_LASER_SPARKS - Splash particles obey gravity 
TE_WELDING_SPARKS - Splash particles with flash of light at {origin} 
TE_SPLASH - Randomly shaded shower of particles 

colors: 
1 - red/gold - blaster type sparks 
2 - blue/white - blue 
3 - brown - brown 
4 - green/white - slime green 
5 - red/orange - lava red 
6 - red - blood red 
All others are grey 
*/ 
//====================================================== 
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) 
{ 
gi.WriteByte(svc_temp_entity); 
gi.WriteByte(type); 
gi.WriteByte(count); 
gi.WritePosition(start); 
gi.WriteDir(movdir); 
gi.WriteByte(color); 
gi.multicast(origin, MULTICAST_PVS); 
} 

#define PROTECT_CELLS 4
#define PROTECT_TIME  3.5
#define CANT_PROTECT_TIME PROTECT_TIME * 1.5

void ApplyProtect (edict_t *ent)
{

	if (ent->misc_time[3] > level.time)
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < PROTECT_CELLS)
	{
		gi.cprintf (ent, PRINT_HIGH, "You need at least %d cells\n", PROTECT_CELLS);
		return;
	}
	else
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= PROTECT_CELLS;

	ent->misc_time[2] = level.time + PROTECT_TIME;
	ent->misc_time[3] = level.time + CANT_PROTECT_TIME;
}

//=====================
//WEAPONS
//=====================
static void mod_drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void mod_drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *Mod_Drop_Item (edict_t *ent, gitem_t *item, int forward_distance, int up_distance)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = mod_drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;
		vec3_t temp_vec_angle;
		int r;

		VectorCopy (ent->client->v_angle, temp_vec_angle);

		if ((r = random()*10) <= 5)
			temp_vec_angle[YAW] += rand()%180 * r;
		else
			temp_vec_angle[YAW] -= rand()%180 * r;
		
		AngleVectors (temp_vec_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	if (!forward_distance)
		forward_distance = 100;
	if (!up_distance)
		up_distance = 300;

	VectorScale (forward, forward_distance, dropped->velocity);
	
	dropped->velocity[2] = up_distance;

	dropped->think = mod_drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Mod_Drop_General (edict_t *ent, gitem_t *item, int forward_distance, int up_distance)
{
	Mod_Drop_Item (ent, item, forward_distance, up_distance);
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ValidateSelectedItem (ent);
}

/*
=================
fire_rail
=================
*/
void fire_winder (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	vec3_t forward;
	
	gitem_t *item;
	int item_count;
	// to make it easier to add items to the list 
	char *item_list[] = 
	{
		"Shotgun", 
		"Super Shotgun", 
		"Machinegun",
		"Chaingun",
		"Grenade Launcher", 
		"Rocket Launcher", 
		"HyperBlaster", 
		"Railgun", 
		"BFG10K"
	};
	
	int i;

	item_count = 9;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
			{
				if (tr.ent->client)
				{		
					// switch to blaster
					tr.ent->client->newweapon = FindItem ("Blaster");
					ChangeWeapon (tr.ent);
					// drop items
					for (i = 0; i < item_count; i++) 
					{
						if (tr.ent->client->pers.inventory[ITEM_INDEX(FindItem(item_list[i]))])
						{
							if ((item = FindItem (item_list[i])) != NULL)
							{
								int f_dist, up_dist;	//forward and up distances for the drop

								f_dist = ceil(100 + (random() * 100));
								up_dist = ceil(300 + (random() * 100));

								Mod_Drop_General(tr.ent,item, f_dist, up_dist);
								//tr.ent->client->pers.inventory[ITEM_INDEX(FindItem(item_list[i]))] = 0;
							}
						}
					}	
				}	

				VectorCopy (aimdir, forward);
				VectorNormalize (forward);
				VectorScale(forward, 1000, forward);
				VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
			}

			VectorCopy (tr.endpos, from);
		}
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void weapon_winder_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	damage = 0;
	kick = 450;

	if (is_quad)
	{
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 8,  ent->viewheight-4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_winder (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Winder (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_winder_fire);
}


//===============================================================
//			TURRET CODE
//===============================================================
#define TURRET_FIND_RADIUS	600
#define TURRET_COST 250	//200 cells

//FUNCTIONS/////////////
void Mod_Turret_FaceEnemy(edict_t *self)
{
	vec3_t start, end, dir;

	VectorCopy (self->enemy->s.origin, start);
	VectorCopy (self->s.origin, end);
	VectorSubtract (start, end, dir);
	vectoangles (dir, self->s.angles);
}

void Mod_Turret_Fire (edict_t *self)
{
//self = turret's canon
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int effect;

	effect = EF_HYPERBLASTER;

	if (self->delay > level.time)
	{
		return;
	}

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(0,0,0), forward, right, start);

	start[2] += 4;

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	//fire_grenade (self, start, forward, 120, 600, 2.5, 120);
	fire_bullet (self, start, forward, 5, 35, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	self->delay = level.time + 0.2;	//next rockets in 1 second...

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}


void Mod_Turret_Hunt_Enemy (edict_t *self)
{
	//self = turret
	//turn the turret canon toward its enemy
	self->turret_canon->enemy = self->enemy;
	Mod_Turret_FaceEnemy (self->turret_canon);

	if (infront(self->turret_canon, self->turret_canon->enemy))
	{
		//gi.dprintf ("Firing rocket !\n");
	//	Enginer_Rocket_Turret_Fire (self->turret_canon);
		Mod_Turret_Fire (self->turret_canon);
	}

}

qboolean Mod_Turret_Search_Enemy (edict_t *self)
{
	//self = turret
	edict_t *ent;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!visible (self, ent))
			continue;

		if (!ent->takedamage)
			continue;

		if (!ent->health)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;
	
		//ok, now just hunt our enemy
		self->enemy = ent;
		Mod_Turret_Hunt_Enemy (self);
		return true;
	}

	return false;

}

void Mod_Turret_Think (edict_t *self)
{
	if ((Mod_Turret_Search_Enemy(self))==true)	//if we've found an enemy
	{	//just return cuz the next function is handeled by the Search_Enemy function
	}
	//self->turret_canon->s.angles[YAW]++;

	if (self->misc_time[0] < level.time)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("gunner/gunidle.wav"), 1, ATTN_NORM, 0);
		self->misc_time[0] = level.time + (8 + rand()%4);
	}

	self->nextthink = level.time + 0.1;
}

//ent is the grenade
static void Mod_Turret_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
	return;
}

void Mod_Turret_First_Think (edict_t *ent)
{
	edict_t *canon;
	vec3_t temp_vec;
	//remove s.modelindex2, and spawn a "real" entity as the turret's canon, so it can turn
	//around -
	ent->s.modelindex2 = 0;
	
	canon = G_Spawn();
	VectorCopy (ent->s.origin, temp_vec);
	temp_vec[2] += 40;
	VectorCopy (temp_vec, canon->s.origin);
	canon->s.modelindex = gi.modelindex ("models/turret/cannon/tris.md2");//"models/weapons/g_rocket/tris.md2");
	//canon->s.effects |= EF_QUAD;
	ent->turret_canon = canon;
	canon->owner = ent;
	canon->solid = SOLID_BBOX;
	gi.linkentity (canon);
	gi.linkentity (ent);

	ent->think = Mod_Turret_Think;
	ent->nextthink = level.time + FRAMETIME;
}

void Fire_Mod_Turret (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	/*
	grenade is the base of the turret
	grenade->turret_canon is the canon of the turret
	*/
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed/3, grenade->velocity);
	VectorMA (grenade->velocity, 40 + crandom() * 10.0, up, grenade->velocity);
	//VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex2 = gi.modelindex("models/turret/cannon/tris.md2");
	grenade->s.modelindex = gi.modelindex ("models/turret/base/tris.md2");
	grenade->owner = self;
	grenade->touch = Mod_Turret_Touch;
	grenade->nextthink = level.time + 4;
	grenade->think = Mod_Turret_First_Think;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "Mod_Turret";

	gi.linkentity (grenade);
}

void PlaceTurret (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 15;
	if (is_quad)
		damage *= 2;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < TURRET_COST)
	{
		gi.cprintf (ent, PRINT_HIGH, "You need at least %d cells to fire your special\n", TURRET_COST);
		return;
	}
	else
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= TURRET_COST;


	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	Fire_Mod_Turret 
	(
		ent,
		start, 
		forward, 
		damage, 
		600,
		2.5,
		TURRET_FIND_RADIUS
	);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


//*********** FEIGN DEATH *******************
// see also ClientThink
void FeignDeath (edict_t *self)
{
	//si on n'est pas en train de faire la feinte,
	//c'est que l'on veut la faire !
	if (self->deadflag)
		return;	//si on est deja mort, pas besoin de l'etre 2 fois... :)

	if (!self->feign)
	{
		static int i;

		self->feign = true;
		self->raising_up = false;

		//gi.dprintf ("Feigning death\n");

		VectorClear (self->avelocity);

		self->takedamage = DAMAGE_YES;
		self->movetype = MOVETYPE_TOSS;

		self->s.modelindex2 = 0;	// remove linked weapon model
		self->client->ps.gunindex = 0;

		self->s.angles[0] = 0;
		self->s.angles[2] = 0;

		self->s.sound = 0;
		self->client->weapon_sound = 0;

		self->maxs[2] = -8;
		self->client->ps.pmove.pm_type = PM_DEAD;
		

		i = (i+1)%3;
		// start a death animation
		self->client->anim_priority = ANIM_DEATH;
		
		if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			self->s.frame = FRAME_crdeath1-1;
			self->client->anim_end = FRAME_crdeath5;
		}
		else switch (i)
		{
		case 0:
			self->s.frame = FRAME_death101-1;
			self->client->anim_end = FRAME_death106;
			break;
		case 1:
			self->s.frame = FRAME_death201-1;
			self->client->anim_end = FRAME_death206;
			break;
		case 2:
			self->s.frame = FRAME_death301-1;
			self->client->anim_end = FRAME_death308;
			break;
		}
		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		
		gi.linkentity (self);
	}
	else	//sinon, il faut se relever
	{
		vec3_t	mins = {-16, -16, -24};
		vec3_t	maxs = {16, 16, 32};

		self->feign = false;
		self->raising_up = true;

		self->movetype = MOVETYPE_WALK;
		self->viewheight = 22;
		self->solid = SOLID_BBOX;
		self->flags &= ~FL_NO_KNOCKBACK;
		self->svflags &= ~SVF_DEADMONSTER;

		VectorCopy (mins, self->mins);
		VectorCopy (maxs, self->maxs);
		VectorClear (self->velocity);

		self->client->anim_priority = ANIM_REVERSE;
		if(self->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			
			self->s.frame = FRAME_crdeath5+1;
			self->client->anim_end = FRAME_crdeath1;
		}
		else
		{
			self->s.frame = FRAME_death106+1;
			self->client->anim_end = FRAME_death101;
			
		}

		self->raising_up = true;
		self->misc_time[7] = level.time + 0.5;

		//self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
		//ShowGun(self);					// ### Hentai ### special gun model
		//self->s.modelindex2 = 255;		// custom gun model

		gi.linkentity (self);
	}
}
