#include "g_local.h"

extern qboolean techspawn; // = false;

char *tnames[] = 
{
	"item_tech1", 
	"item_tech2", 
	"item_tech3", 
	"item_tech4",
	NULL
};


/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 4) 
	{
		safe_centerprintf(who, "You already have a TECH powerup.");
		gi.sound (who, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech(edict_t *ent)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) 
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) 
		{
			return tech;
		}
		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{
//Skid added	
	int		index;

	if(other->flags & FL_RUNE_MASK)
	{
		CTFHasTech(other);
		return false;
	}

	index = ITEM_INDEX(ent->item);
	if(Q_stricmp(ent->classname,"item_tech1")==0)
	{
		other->flags |= FL_RUNE_RESIST;
	}
	else if(Q_stricmp(ent->classname,"item_tech2")==0)
	{
		other->flags |= FL_RUNE_STRENGTH;
	}
	else if(Q_stricmp(ent->classname,"item_tech3")==0)
	{
		other->flags |= FL_RUNE_HASTE;
	}
	else if(Q_stricmp(ent->classname,"item_tech4")==0)
	{
		other->flags |= FL_RUNE_REGEN;
	}
//End Skid

	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	return true;
}



static void SpawnTech(gitem_t *item, edict_t *spot);

static edict_t *FindTechSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
	{
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	} 
	else
	{
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + CTF_TECH_TIMEOUT;
	tech->think = TechThink;
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;

//Skid added
	ent->flags &= ~FL_RUNE_MASK;
//end Skid
}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

	i = 0;
	while (tnames[i]) 
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) 
		{
			dropped = Drop_Item(ent, tech);
			// hack the velocity to make it bounce random
			dropped->velocity[0] = (rand() % 600) - 300;
			dropped->velocity[1] = (rand() % 600) - 300;
			dropped->nextthink = level.time + CTF_TECH_TIMEOUT;
			dropped->think = TechThink;
			dropped->owner = NULL;
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
		}
		i++;
	}
//Skid added
	ent->flags &= ~FL_RUNE_MASK;
//end Skid
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;

//gi.dprintf("SpawnTech : %s\n", item->classname);

	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	gi.setmodel (ent, ent->item->world_model); // return new tech model
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;

	gi.linkentity (ent);
}

static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i=0;
	
	while (tnames[i]) 
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			(spot = FindTechSpawn()) != NULL)
			SpawnTech(tech, spot);
		i++;
	}
}

// frees the passed edict!
void CTFRespawnTech(edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);
	G_FreeEdict(ent);
}

void CTFSetupTechSpawn(void)
{
	edict_t *ent;

	if (techspawn || ((int)dmflags->value & DF_CTF_NO_TECH))
		return;

	ent = G_Spawn();
	ent->nextthink = level.time + 2;
	ent->think = SpawnTechs;
	techspawn = true;
}

//======================================================================
//======================================================================
//======================================================================
//======================================================================

int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech1");
	}
	
	if (dmg && tech && ent->client &&
//Skid added
		ent->flags & FL_RUNE_RESIST)
//end Skid
	{
		// make noise
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return dmg / 2;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech2");
	}

	if (dmg && tech && ent->client && 
//Skid added
		ent->flags & FL_RUNE_STRENGTH)
//end Skid
	{
		return dmg * 2;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech2");
	}
	
	if (tech && ent->client &&
//Skid added
		ent->flags & FL_RUNE_STRENGTH)
//end Skid
	{
		if (ent->client->ctf_techsndtime < level.time) 
		{
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech3");
	}

	if (tech && ent->client &&
//Skid added
		ent->flags & FL_RUNE_HASTE)
//end Skid
		return true;
	return false;
}

void CTFApplyHasteSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech3");
	}

	if (tech && ent->client &&
//Skid added
		ent->flags & FL_RUNE_HASTE &&
//end Skid
		ent->client->ctf_techsndtime < level.time) 
	{
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech4");
	}
	
	if (tech &&
//Skid added
		ent->flags & FL_RUNE_REGEN)
//end Skid
	{
		if (client->ctf_regentime < level.time) 
		{
			client->ctf_regentime = level.time;
			if (ent->health < 150)
			{
				ent->health += 5;
				if (ent->health > 150)
					ent->health = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
			index = ArmorIndex (ent);
			if (index && client->pers.inventory[index] < 150) 
			{
				client->pers.inventory[index] += 5;
				if (client->pers.inventory[index] > 150)
					client->pers.inventory[index] = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
		}
		if (noise && ent->client->ctf_techsndtime < level.time)
		{
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
}


qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
	{
		tech = FindItemByClassname("item_tech4");
	}
	
	if (tech && ent->client &&
//Skid added
		ent->flags & FL_RUNE_REGEN)
//end Skid
		return true;
	return false;
}