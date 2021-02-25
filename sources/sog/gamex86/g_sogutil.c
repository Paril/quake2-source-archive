#include "g_local.h"
#include "g_sogutil.h"


int GetRandomClass()
{
	float f= random();
	if(f < 0.25)
	{
		return CLASS_WOLF;
	}
	else if(f < 0.5)
	{
		return CLASS_DOOM;
	}
	else if(f < 0.75)
	{
		return CLASS_Q1;
	}
	else
	{
		return CLASS_Q2;
	}

}


//======================================================================
// ITEM Substitution Arrays 
//======================================================================
//
// Weapons Array
//
// Gavino modified


int weapon_subst[12][4]=
{		
	W_GG,		D_CG,		Q1_SNG,		Q2_CG,	
	W_MG,		D_CG,		Q1_NG,		Q2_MG,
	W_MG,		D_SG,		Q1_SG,		Q2_SG, 
	W_GG,		D_SSG,		Q1_SSG,		Q2_SSG,
	W_RL,		D_RL,		Q1_RL,		Q2_RL,
	W_RL,		D_RL,		Q1_GL,		Q2_GL,
	W_FT,		D_PG,		Q1_LG,		Q2_HB,
	W_GG2,		D_SSG,		Q1_LG,		Q2_RG,
	W_GG2,		D_BFG,		Q1_LG,		Q2_BFG,
	W_MG,		D_CSAW,		Q1_NG,		Q2_MG,   // now it should work
	INDEX_W_RL,	INDEX_D_RS,	INDEX_Q1_RG,	AMMO_GRENADES_INDEX,
	K_RAYGUN,	K_RAYGUN,	K_RAYGUN,	K_RAYGUN,
};


int GiveNewWeapon(int oldindex, int item_class, int player_class)
{
	int newindex=0;
	int i;

	// add Gavino, this should work.

	if (player_class == 1)
	{
		if (oldindex==W_MG)
			return D_SG;
		else if (oldindex==Q1_NG)
			return D_SG;
		else if (oldindex==Q2_MG)
			return D_SG;
	}

	for(i=0;i<12;i++)
	{
		if(oldindex == weapon_subst[i][item_class])
		{
			newindex=weapon_subst[i][player_class];
			break;
		}
	}
	return newindex;
}

//======================================================================
// powerups

int powerup_subst[][4]=
{
	W_GOD,		D_GOD,		Q1_PENT,		Q2_GOD,
	D_INVIS,		D_INVIS,		Q1_RING,		Q2_SILENCER,
	Q2_BACKPACK,	D_BACKPACK,		Q1_PACK,		Q2_BACKPACK,
	Q2_QUAD,		D_BERSERK,		Q1_QUAD,		Q2_QUAD,	
	Q2_HEALTH_MEGA,	D_SOULSPHERE,	Q1_HEALTH_MEGA, 	Q2_HEALTH_MEGA, // fixme
	Q2_ENVIROSUIT,	D_RADSUIT,		Q1_SUIT,		Q2_ENVIROSUIT,
	Q2_REBREATHER,	D_RADSUIT,		Q1_SUIT,		Q2_REBREATHER,
	Q2_ADERNALINE,	D_BERSERK,		Q2_ADERNALINE,	Q2_ADERNALINE
};

int GiveNewPowerup(int oldindex, int item_class , int player_class)
{
	int newindex=0;
	int i;

	for(i=0;i<8;i++)
	{
		if(oldindex == powerup_subst[i][item_class])
		{
			newindex=powerup_subst[i][player_class];
			break;
		}
	}

	return newindex;
}

//======================================================================
// armor

int armor_subst[][4]=
{
	W_GREEN,	D_GREEN,	Q1_GREEN,	Q2_JACKET,
	W_GREEN,	D_GREEN,	Q1_YELLOW,	Q2_COMBAT,
	W_BLUE,		D_BLUE,		Q1_RED,		Q2_BODY,
	W_CROSS,	D_HELMET,	Q1_SHARD,	Q2_SHARD,
	W_CHALICE,	D_HELMET,	Q1_SHARD,	Q2_SHARD,
	W_CHEST,	D_HELMET,	Q1_SHARD,	Q2_SHARD,
	W_CROWN,	D_HELMET,	Q1_SHARD,	Q2_SHARD,
	W_BLUE,		D_BLUE,		Q1_RED,		Q2_POWERSCREEN,
	W_BLUE,		D_BLUE,		Q1_RED,		Q2_POWERSHIELD
};

int GiveNewArmor(int oldindex, int item_class ,int player_class)
{
	int newindex=0;
	int i;

	for(i=0;i<9;i++)
	{
		if(oldindex == armor_subst[i][item_class])
		{
			newindex=armor_subst[i][player_class];
			break;
		}
	}
	return newindex;
}

//======================================================================
// ammo  
// Gavino modified
//
// Hey Arno, do you know how to add multiple ammo to classes?  
// Doom/Q1/Q2 should get both bullets plus shells from Wolf ammo.
// Or else they are screwed! :)  (But the main ammo should still be bullets)

int ammo_subst[][4]=
{
	INDEX_W_BM,	INDEX_D_BL,	INDEX_Q1_NL,	AMMO_BULLETS_INDEX,
	INDEX_W_BL,	INDEX_D_BL,	INDEX_Q1_NL,	AMMO_BULLETS_INDEX,
	INDEX_W_BS,	INDEX_D_BS,	INDEX_Q1_NS,	AMMO_BULLETS_INDEX,
	INDEX_W_BM,	INDEX_D_SL,	INDEX_Q1_SL,	AMMO_SHELLS_INDEX,
	INDEX_W_BS,	INDEX_D_SS,	INDEX_Q1_SS,	AMMO_SHELLS_INDEX,
	INDEX_W_PS,	INDEX_D_CS,	INDEX_Q1_CS,	AMMO_CELLS_INDEX,
	INDEX_W_PL,	INDEX_D_CL,	INDEX_Q1_CL,	AMMO_CELLS_INDEX,
	INDEX_W_RS,	INDEX_D_RS, INDEX_Q1_RS,	AMMO_ROCKETS_INDEX,
	INDEX_W_RL,	INDEX_D_RL, INDEX_Q1_RL,	AMMO_ROCKETS_INDEX,
	INDEX_W_RS,	INDEX_D_RS,	INDEX_Q1_RG,	AMMO_GRENADES_INDEX,
	INDEX_W_BM,	INDEX_D_SL,	INDEX_Q1_NL,	AMMO_SLUGS_INDEX,	
};


int GiveNewAmmo(int oldindex, int item_class , int player_class)
{
	int newindex=0;
	int i;

	//Doom guys always get Small cells in other maps since 100 cells is a LOT
	//Gavino - removed references to level type, since maps can be mixed or unidentified
	if(player_class == 1)
	{
		if(oldindex == INDEX_Q1_CL)
			return INDEX_D_CS;
		else if(oldindex == INDEX_Q1_RS)
			return INDEX_D_RL;

		if(oldindex == INDEX_W_BL)	// Gavino add
			return INDEX_D_SL; 	// Give Doomy shells
	
	}
	else if(player_class == 3)
	{
		if(oldindex == INDEX_Q1_CS)
			return AMMO_SLUGS_INDEX;
	}
	
	if(oldindex == INDEX_W_RS)
	{
		if(player_class == 2)
			return INDEX_Q1_RG;
		if(player_class == 3)
			return INDEX_Q2_RG;
	}

	if(oldindex == INDEX_D_RS)
	{
		if(player_class == 2)
			return INDEX_Q1_RG;
		if(player_class == 3)
			return INDEX_Q2_RG;
	}

	if(oldindex == INDEX_D_CS)
		if(player_class == 3)
			return AMMO_SLUGS_INDEX;
	
	for(i=0;i<11;i++)
	{
		if(oldindex == ammo_subst[i][item_class])
		{
			newindex=ammo_subst[i][player_class];
			break;
		}
	}
	return newindex;
}

//======================================================================

gitem_t * GiveNewItem(gitem_t *olditem, int player_class)
{
	gitem_t *	newitem = NULL;
	int			oldindex=0,
				newindex=0,
				item_class =0;
		
	if(!(olditem->flags))
		return NULL;
	else
		oldindex=ITEM_INDEX(olditem);
	
	if(olditem->flags & IT_Q1)
		item_class = CLASS_Q1;
	else if(olditem->flags & IT_DOOM)
		item_class = CLASS_DOOM;
	else if(olditem->flags & IT_WOLF)
		item_class = CLASS_WOLF;
	else
		item_class = CLASS_Q2;

	item_class--;
	player_class--;

	if(olditem->flags & IT_WEAPON)
	{
		newindex=GiveNewWeapon(oldindex,item_class,player_class);
	}
	else if(olditem->flags & IT_AMMO)
	{
		newindex=GiveNewAmmo(oldindex,item_class,player_class);
	}
	else if(olditem->flags & IT_ARMOR)
	{
		newindex=GiveNewArmor(oldindex,item_class,player_class);
	}
	else if(olditem->flags & IT_POWERUP)
	{
		newindex=GiveNewPowerup(oldindex,item_class,player_class);
	}
	else
	{
		return NULL;
	}
	
	if(newindex)
	{
		newitem = GetItemByIndex(newindex);
		return newitem;
	}
	return NULL;
}

//======================================================================
// Utility Function - gives weapon and ammo (if specified) 
// called from Touch_item

void GiveWeapon(edict_t *ent,int index,qboolean ammo)
{
	//Give weapon
	ent->client->pers.inventory[index]++;

	//Give ammo
	if(ammo == true)
	{
		gitem_t		*ammo;
		gitem_t		*item;

		item = GetItemByIndex(index);
		ammo = FindItem (item->ammo);
		
		if(index==Q2_RG)
		{
//gi.dprintf("Adding Railgun slugs\n");
			Add_Ammo (ent, ammo, 10);
		}
		else
			Add_Ammo (ent, ammo, ammo->quantity);
	}
}

//======================================================================

//===============
// Q1 Weapon ammo
//===============

struct
{
	int  q1weapindex;
	int  count;
} q1_weap_ammo[] = 
{
	{ Q1_SG,	15	},
	{ Q1_SSG,	5	},
	{ Q1_NG,	30	},
	{ Q1_SNG,	30	},
	{ Q1_GL,	5	},
	{ Q1_RL,	5	},
	{ Q1_LG,	15	}
};

int GiveQ1WeapAmmo(int weap_index)
{
	int i;
	for(i=0;i<=Q1_LG;i++)
	{
		if(weap_index == q1_weap_ammo[i].q1weapindex)
			break;
	}
	return q1_weap_ammo[i].count;
}


//===============
// Doom Weapon ammo
//===============

struct
{
	int  dweapindex;
	int  count;
} d_weap_ammo[] = 
{
	{ D_SG,		8	},
	{ D_SSG,	8	},
	{ D_CG,		40	},
	{ D_RL,		5	},
	{ D_PG,		40	},
	{ D_BFG,	40	}
};

int GiveDoomWeapAmmo(int weap_index)
{
	int i;
	for(i=0;i<=D_BFG;i++)
	{
		if(weap_index == d_weap_ammo[i].dweapindex)
			break;
	}
	return d_weap_ammo[i].count;
}

/*
================
End Substitution Rountines
================
*/

//======================================================================

int Q1WeaponRank(int index)
{
	if(index == Q1_LG)
		return 1;
	if(index == Q1_RL)
		return 2;
	if(index == Q1_SNG)
		return 3;
	if(index == Q1_GL)
		return 4;
	if(index == Q1_SSG)
		return 5;
	if(index == Q1_NG)
		return 6;
	return 7;
}

qboolean Is_new_weapon_better (int index, gitem_t *old_weap , int player_class)
{
	int old_index;
	old_index= ITEM_INDEX(old_weap);

	switch(player_class)
	{
		case CLASS_DOOM:
			{
				if(index > old_index)  
					return true;
				break;
			}
//FIX ME
		case CLASS_WOLF:
			{
				if(index > old_index)
					return true;
				break;
			}
		case CLASS_Q1:
			{
				if(Q1WeaponRank(index) < Q1WeaponRank(old_index))
					return true;
				break;
			}
	}
	return false;
}	

//======================================================================

//=================
// TELEPORTER
//================

static void d_telethink (edict_t *self)
{

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 9)
		self->think = G_FreeEdict;
}

void DoomTeleporterSplash(vec3_t spot, vec3_t offset)
{
	edict_t	*telesplash;
	vec3_t forward,right,up;
	vec3_t newspot;
	
	VectorCopy(spot, newspot);
	
	AngleVectors(offset,forward,right,up);
	VectorMA(newspot,36,forward,newspot);
	VectorMA(newspot,24,up,newspot);

	telesplash = G_Spawn();
	VectorCopy (newspot, telesplash->s.origin);
	VectorCopy (newspot,telesplash->s.old_origin);
	telesplash->movetype = MOVETYPE_NOCLIP;
	telesplash->clipmask = MASK_ALL;
	telesplash->solid = SOLID_NOT;
	telesplash->s.effects = EF_SPHERETRANS; 
	
	VectorClear (telesplash->velocity); 
	VectorClear (telesplash->mins);
	VectorClear (telesplash->maxs);
	telesplash->s.modelindex = gi.modelindex ("sprites/Dtele.sp2"); 
	telesplash->s.frame = 0;
	telesplash->s.sound = 0; 
	telesplash->touch = NULL;
	telesplash->nextthink = level.time + FRAMETIME;
	telesplash->think = d_telethink;
	telesplash->dmg = 0;
	telesplash->classname = "telesplash";
	
	gi.linkentity (telesplash);

	gi.sound(telesplash,CHAN_NO_PHS_ADD+CHAN_VOICE,gi.soundindex ("idg2world/teleport.wav"),1,ATTN_NORM,0);
}

//======================================================================
//======================================================================
//======================================================================

void GenQ1TeleportSounds(edict_t *ent)
{
	gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("idg3world/teleport.wav"), 1, ATTN_NORM, 0);
}

void GenQ1TeleportSounds2(edict_t *ent1, edict_t *ent2)
{
	gi.sound (ent1, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("idg3world/teleport.wav"), 1, ATTN_NORM, 0);
	gi.sound (ent2, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("idg3world/teleport.wav"), 1, ATTN_NORM, 0);
}


void GenClientRespawnFX(edict_t *ent)
{
	switch(ent->client->resp.player_class)
	{
		case CLASS_DOOM:
				DoomTeleporterSplash(ent->s.origin,ent->s.angles);
				break;
		case CLASS_Q1:
				ent->s.event = EV_PLAYER_TELEPORT;
				GenQ1TeleportSounds(ent);
				break;
		case CLASS_Q2:
		case CLASS_WOLF:
		default:
				ent->s.event = EV_PLAYER_TELEPORT;
				gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
				break;
	}
}



				
/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
//static 
edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

//static 
qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}
