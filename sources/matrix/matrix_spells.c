#include "g_local.h"
static int	quad_drop_timeout_hack;
void MatrixEffects(edict_t *ent);

edict_t *MatrixHighestLevel (edict_t *ent)
{
	int levels, i, top=0;
	edict_t *cl_ent;	
	edict_t *chosen;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
			continue;
		if (cl_ent == ent)
			continue;
		if (ent->deadflag || ent->health < 0)
			continue;
		levels = cl_ent->healthlevel + cl_ent->staminalevel + cl_ent->damagelevel;
		if (levels >= top)
		{
			chosen = cl_ent;
			top = levels;
		}
	}

	gi.centerprintf (chosen,"%s Is trying to posses You.\n Hunt Him Down!\n", ent->client->pers.netname);
	gi.centerprintf (ent,"You must stay still during the countdown.\nYou Will be possesing %s\n", chosen->client->pers.netname);

	return chosen;
}

void MatrixCancelSwap (edict_t *ent)
{
// play stop sound
//	gi.dprintf("cancel Swap\n");
	ent->swapent = NULL;
	ent->swaptime = 0;
}

void SwapCounters (edict_t *ent)
{
	int		i;
	edict_t *player;

	for(i=1; i <= maxclients->value; i++)
		if ((player=&g_edicts[i]) && player->inuse)
			player->someswaptime = level.time + 10;

	gi.dprintf("DEBUG: Starting swap Counters\n");
}

void MatrixStartSwap (edict_t *ent)
{
//	Start a sound on ent->swapent and ent
	if(possesban->value)
	{
		gi.cprintf(ent, PRINT_HIGH,"Posses is banned on this server. Type possesban 0 to fix.\n");
		return;
	}
	if (ent->stamina< 250)
	{
		gi.cprintf(ent, PRINT_HIGH,"You dont have enough stamina\n");
		return;
	}
	MatrixEffects(ent);

	SwapCounters(ent);
	ent->swapent = MatrixHighestLevel(ent);
	ent->stamina -= 250;
	gi.sound (ent, CHAN_AUTO, gi.soundindex ("world/10_0.wav"), 1, ATTN_NONE, 0);
	gi.sound (ent->swapent, CHAN_AUTO, gi.soundindex ("world/10_0.wav"), 1, ATTN_NONE, 0);
	ent->swaptime = level.time + 10;
}

void MatrixPlayerSwap (edict_t *ent, edict_t *other)
{
	vec3_t	tempvec;
	int		temp1, n;
	float	tempfloat;
	int		tempitems[MAX_ITEMS];
	gitem_t	*tempitem;

	if (!other->client)
		return;
	if (other == ent)
		return;

	gi.bprintf(PRINT_HIGH, "%s swapped with %s", ent->client->pers.netname, other->client->pers.netname);
	gi.unlinkentity (other);
	gi.unlinkentity (ent);

	for ( n = 0; n < game.num_items; n++ )
	{
		tempitems[n] = ent->client->pers.inventory[n];
		ent->client->pers.inventory[n] = other->client->pers.inventory[n];
		other->client->pers.inventory[n] = tempitems[n];
	}
	temp1 = ent->health;
	ent->health = other->health;
	other->health = temp1;

	tempitem = ent->client->pers.weapon;
	ent->client->pers.weapon = other->client->pers.weapon;
	other->client->pers.weapon = tempitem;
	ent->client->newweapon = ent->client->pers.weapon;
	other->client->newweapon = other->client->pers.weapon;

	temp1 = ent->healthlevel;
	ent->healthlevel = other->healthlevel;
	other->healthlevel = temp1;

	temp1 = ent->staminalevel;
	ent->staminalevel = other->staminalevel;
	other->staminalevel = temp1;

	temp1 = ent->damagelevel;
	ent->damagelevel = other->damagelevel;
	other->damagelevel = temp1;

	temp1 = ent->max_stamina;
	ent->max_stamina = other->max_stamina;
	other->max_stamina = temp1;

	temp1 = ent->max_health;
	ent->max_health = other->max_health;
	other->max_health = temp1;

	tempfloat = ent->client->speed_framenum;
	ent->client->speed_framenum = other->client->speed_framenum;
	other->client->speed_framenum = tempfloat;

	tempfloat = ent->bullet_framenum;
	ent->bullet_framenum = other->bullet_framenum;
	other->bullet_framenum = tempfloat;

	tempfloat = ent->client->ir_framenum;
	ent->client->ir_framenum = other->client->ir_framenum;
	other->client->ir_framenum = tempfloat;

	ChangeWeapon (ent);
	ChangeWeapon (other);

	VectorCopy(ent->s.origin, tempvec);
	VectorCopy(other->s.origin, ent->s.origin);
	VectorCopy(tempvec, other->s.origin);
	gi.linkentity (ent);
	gi.linkentity (other);

	gi.sound (ent, CHAN_VOICE, gi.soundindex ("teleport.wav"), 1, ATTN_NORM, 0);
	gi.sound (other, CHAN_VOICE, gi.soundindex ("teleport.wav"), 1, ATTN_NORM, 0);
}
void MatrixSwapThink (edict_t *ent)
{
	if (ent->swaptime < level.time)
		return;

	if (( VectorLength(ent->velocity) ) || ( !ent->swapent ))
		MatrixCancelSwap (ent);

	if ((ent->swaptime == level.time) && (!ent->swapent->deadflag))
		MatrixPlayerSwap(ent, ent->swapent);
}
void MatrixPlayerChange (edict_t *ent) //AHH! telefrag spell! NOOOO!!
{
	int n;
	edict_t *other;

	other = MatrixHighestLevel(ent);
	if (!other)
		return;
	if (other == ent)
		return;
	if (ent->stamina< 250)
		return;

	ent->stamina -= 250;

	for ( n = 0; n < game.num_items; n++ )
	{
		ent->client->pers.inventory[n] = 0;
		ent->client->pers.inventory[n] = other->client->pers.inventory[n];
	}
	ent->health = other->health;
	VectorCopy(other->s.origin, ent->s.origin);
	ent->client->pers.weapon = other->client->pers.weapon;
	ent->client->newweapon = ent->client->pers.weapon;

	ent->healthlevel = other->healthlevel;
	ent->staminalevel = other->staminalevel;
	ent->damagelevel = other->damagelevel;
	ent->max_stamina = other->max_stamina;
	ent->max_health = other->max_health;

	KillBox (ent);
	ChangeWeapon (ent);
	gi.linkentity (ent);
}

void MatrixStopBullets (edict_t *ent)
{
	edict_t *bullet = NULL;

	if (ent->bullet_framenum < level.framenum)
		return;

	while ((bullet = findradius(bullet, ent->s.origin, 135)) != NULL)
	{
		if (bullet->owner == ent)
			continue;
		if ((!Q_stricmp(bullet->classname, "bolt") == 0)		&&
			(!Q_stricmp(bullet->classname, "grenade") == 0)		&&
			(!Q_stricmp(bullet->classname, "hgrenade") == 0)	&&
			(!Q_stricmp(bullet->classname, "rocket") == 0)		&&
			(!Q_stricmp(bullet->classname, "bullet") == 0)		&&
			(!Q_stricmp(bullet->classname, "bfg blast") == 0))
			continue;

		// Clear horizontal components of velocity
		
		if (bullet->velocity[0] || bullet->velocity[1]) 
			VectorClear(bullet->velocity);
	
			
		
		bullet->velocity[2] -= sv_gravity->value * FRAMETIME;
		
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("buletstp.wav"), 1, ATTN_NORM, 0);
	
	}
}


void MatrixTankDropItem (edict_t *ent,int type)
{
	gitem_t	*it;

	if (!ent->client->resp.tank)
		return;
	// Probably not the best way of doing it but i cant figure a while loop out
start:
	// Pick a random item
	it = &itemlist[(int)rand()%game.num_items];

	//	If its not the right type pick another 
	if ((it->flags & type) && (it->world_model)/*Stops it from picking blaster/health*/)
	{
		Drop_Item(ent, it);
		return;
	}
	else 
		goto start;
}
void Effectthink(edict_t *ent)
{
	vec3_t dest;

	if (ent->owner->swaptime < level.time)
	{
		G_FreeEdict(ent);
		return;
	}

	VectorSubtract(ent->owner->s.origin, ent->s.origin, dest);

	ent->velocity[0]+=crandom()*15;
	ent->velocity[1]+=crandom()*15;
	ent->velocity[2]+=crandom()*15;

	VectorNormalize(dest);
	VectorNormalize(ent->velocity);
	VectorScale(dest, 0.5, dest);
	VectorAdd(ent->velocity, dest, ent->velocity);
	VectorNormalize(ent->velocity);
	VectorScale(ent->velocity, 300, ent->velocity);

	VectorMA (ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles);
	VectorMA (ent->s.origin, FRAMETIME, ent->velocity, ent->s.origin);

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = Effectthink;
}

void MatrixEffects(edict_t *ent)
{
	int i;
	for (i=0; i<3; i++)
	{
		vec3_t vec, up, right, forward;
		edict_t	*best;
		best = G_Spawn();
// Change me
	//	best->s.modelindex = gi.modelindex ("models/objects/shell1/tris.md2");	
		best->s.modelindex = 0; //rather have nothing than a fairy or something.
// Change me

		forward[0] = crandom()*2 - 1;
		forward[1] = crandom()*2 - 1;
		forward[2] = crandom()*2 - 1;

		VectorNormalize(forward);
		VectorScale(forward, 100, best->velocity);
		VectorCopy(ent->s.origin,best->s.origin);		
		VectorCopy (ent->s.angles, best->s.angles);

		best->solid = SOLID_NOT;
		if (i==0)
			best->s.effects |= EF_TAGTRAIL;
		else if (i==1)
			best->s.effects |= EF_FLAG1;
		else
			best->s.effects |= EF_FLAG2;

		best->owner = ent;

		best->movetype = MOVETYPE_NOCLIP;
		best->svflags |= SVF_MONSTER;
  
		VectorClear (best->mins);
		VectorClear (best->maxs);

		best->nextthink = level.time + FRAMETIME;
		best->think = Effectthink;
		gi.linkentity (best);
	}


}


void dodgebullets (edict_t *ent)
{
	
	edict_t *bullet = NULL;

	if (ent->dodge_framenum < level.framenum)
		return;

	if(ent->dodgethis && ent->stamina > 0)
		{

			ent->stamina -= 3; ///this is a test, it was originally 75...
		
			//if(ent->stamina < 0)
			//	ent->stamina = 0;

	while ((bullet = findradius(bullet, ent->s.origin, 256)) != NULL)
	{
		
		if (bullet->owner == ent)
			continue;
		if ((!Q_stricmp(bullet->classname, "bolt") == 0)		&&
			(!Q_stricmp(bullet->classname, "grenade") == 0)		&&
			(!Q_stricmp(bullet->classname, "hgrenade") == 0)	&&
			(!Q_stricmp(bullet->classname, "rocket") == 0)		&&
			(!Q_stricmp(bullet->classname, "bullet") == 0)		&&
			(!Q_stricmp(bullet->classname, "bfg blast") == 0))
			continue;

		if(bullet->velocity)
		{
				bullet->velocity[0] -= bullet->velocity[0]/5;
			
				bullet->velocity[1] -= bullet->velocity[1]/5;
			
				bullet->velocity[2] -= bullet->velocity[2]/5;
		}

		if(bullet->velocity && bullet->classname == "bullet")
		{
				bullet->velocity[0] -= bullet->velocity[0]/1.27;
			
				bullet->velocity[1] -= bullet->velocity[1]/1.27;
			
				bullet->velocity[2] -= bullet->velocity[2]/1.27;
		}

		
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("whatever.wav"), 1, ATTN_NORM, 0);
	  
	}
	}
	
}
