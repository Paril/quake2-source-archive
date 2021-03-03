#include "g_local.h"
/*
===============
Plasma Bomb
===============
*/
void DefusePlasmabomb(edict_t *ent, pmenu_t *p)
{
	float dist;
	vec3_t distance;
	distance[0]=ent->s.origin[0]-ent->selectedsentry->s.origin[0];
	distance[1]=ent->s.origin[1]-ent->selectedsentry->s.origin[1];
	distance[2]=ent->s.origin[2]-ent->selectedsentry->s.origin[2];
	dist=VectorLength(distance);
	if(dist>100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Plasmabomb too far away.\n");
		PMenu_Close(ent);
		return;
	}
	//Set the time for defusing
	ent->selectedsentry->sentrydelay= level.time + 0.4;
	ent->selectedsentry->selectedsentry=ent;
	ent->cantmove = 1;
	VectorCopy(ent->s.origin,ent->LockedPosition);
	PMenu_Close(ent);
}

void Plasma_Near (edict_t *ent, edict_t *other)
{
	//Skip if they already have a menu up
	if ((other->client) && (other->client->menu))
		return;

	if(other->sentrydelay>level.time)
		return;

	//Only show menu a max of every 1 second

	//set up menu
//	if (other->client && (other->wf_team != ent->wf_team) && (other->client->player_special & SPECIAL_DEFUSEPLASMABOMB))
    if(!other->bot_client)
	if (other->client && (other->wf_team != ent->wf_team) && (other->client->player_special & SPECIAL_PLASMA_BOMB))
	{
		PMenu_Close(other);

		sprintf(other->client->wfsentrystr[0], "*%s's Plasmabomb", ent->owner->client->pers.netname);
		other->client->sentrymenu[0].text = other->client->wfsentrystr[0];
		other->client->sentrymenu[0].SelectFunc = NULL;
		other->client->sentrymenu[0].align = PMENU_ALIGN_CENTER;
		other->client->sentrymenu[0].arg = 0;
		other->client->sentrymenu[1].text = "1. Defuse";
		other->client->sentrymenu[1].SelectFunc = DefusePlasmabomb;
		other->client->sentrymenu[1].align = PMENU_ALIGN_LEFT;
		other->client->sentrymenu[1].arg = 0;

		other->selectedsentry = ent;//Actually selected plasmabomb

		PMenu_Open(other, other->client->sentrymenu, -1, sizeof(other->client->sentrymenu) / sizeof(pmenu_t), true, false);
		other->sentrydelay = level.time + 1.5;

		//Set timeout for menu (4 seconds)
		if (other->client->menu) other->client->menu->MenuTimeout = level.time + 4;
	}
}

void PlasmaBombExplode(edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;
	int		j;
	vec3_t Vec1, Vec2, Vec3, Vec4;
	vec3_t Vec5, Vec6;

	VectorSet(Vec1,28,28,0);
	VectorSet(Vec2,28,-28,0);
	VectorSet(Vec3,-28,-28,0);
	VectorSet(Vec4,-28,28,0);
	VectorSet(Vec5,0,0,28);
	VectorSet(Vec6,0,0,-28);
	VectorAdd(Vec1,self->s.origin,Vec1);
	VectorAdd(Vec2,self->s.origin,Vec2);
	VectorAdd(Vec3,self->s.origin,Vec3);
	VectorAdd(Vec4,self->s.origin,Vec4);
	VectorAdd(Vec5,self->s.origin,Vec5);
	VectorAdd(Vec6,self->s.origin,Vec6);
	if (self->s.frame == 0)
	{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (Vec1);
			gi.multicast (self->s.origin, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (Vec2);
			gi.multicast (self->s.origin, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (Vec3);
			gi.multicast (self->s.origin, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (Vec4);
			gi.multicast (self->s.origin, MULTICAST_PHS);

	}
	if (self->s.frame == 1)
	{
		gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec1);
			gi.WritePosition (Vec5);
			gi.multicast (Vec1, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec2);
			gi.WritePosition (Vec5);
			gi.multicast (Vec2, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec3);
			gi.WritePosition (Vec5);
			gi.multicast (Vec3, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec4);
			gi.WritePosition (Vec5);
			gi.multicast (Vec4, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec1);
			gi.WritePosition (Vec6);
			gi.multicast (Vec1, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec2);
			gi.WritePosition (Vec6);
			gi.multicast (Vec2, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec3);
			gi.WritePosition (Vec6);
			gi.multicast (Vec3, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec4);
			gi.WritePosition (Vec6);
			gi.multicast (Vec4, MULTICAST_PHS);
	}
	if (self->s.frame == 2)
	{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec5);
			gi.WritePosition (self->s.origin);
			gi.multicast (Vec5, MULTICAST_PHS);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (Vec6);
			gi.WritePosition (self->s.origin);
			gi.multicast (Vec6, MULTICAST_PHS);


	}

	if (self->s.frame == 4)
	{
/*
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius+500)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (!CanDamage (ent, self))
				continue;
			//The next 2 lines force person to be in the room when it explodes
			//if (!CanDamage (ent, self->owner))
			//	continue;

			//G.A.R.  - only damage players (new)
			if (!ent->client)
				continue;

			//Calculate distance
			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);

			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;
			if (!ent->client)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_PLASMABOMB);
		}
*/
// ++TeT	inline the findradius func so we can do some short cut checking
//			and keep the resulting distance calculation

		ent = g_edicts;
		for ( ; ent < &g_edicts[globals.num_edicts]; ent++)
		{
			if (!ent->inuse)
				continue;
			if (ent->solid == SOLID_NOT)
				continue;
			if (!ent->takedamage)
				continue;

			// Do we want to damage other things like sentries?
			//if (!ent->client)
			//	continue;
			if (!CanDamage (ent, self))
				continue;

			// now we check how far it is from us
			for (j = 0; j < 3; j++)
				v[j] = self->s.origin[j] - (ent->s.origin[j] + (ent->mins[j] + ent->maxs[j])*0.5);
			// inlined vectorLength func which returned sqrt of vecLength,
			// instead we compare it to the square of dmg_radius
			dist = 0;
			for (j = 0; j < 3; j++)
				dist += v[j] * v[j];
			if (dist > (self->dmg_radius * self->dmg_radius))
				continue;
			// now that we have eliminated the junk, we do the sqrts here
			// and make it a ratio
			dist = sqrt(sqrt(dist)/self->dmg_radius);
			points = self->radius_dmg * (1.0 - dist);
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_PLASMABOMB);

		}
// --TeT
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void plasmabomb_think(edict_t *self)
{
//	vec3_t	origin;
	edict_t *target;
	target = NULL;
	while ((target = findradius(target, self->s.origin,75)) != NULL)
	{
		//Don't go through walls
		if (!visible(self, target))
	             continue;
		//Check for plasma defusing
		Plasma_Near(self,target);
	}
	if (self->delay < level.time)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
		self->solid = SOLID_NOT;
		self->touch = NULL;
		VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
		VectorClear (self->velocity);
		self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
		self->s.frame = 0;
		self->s.sound = 0;
		self->s.effects &= ~EF_ANIM_ALLFAST;
		self->think = PlasmaBombExplode;
		self->nextthink = level.time + FRAMETIME;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_BIGEXPLOSION);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
	if (self->PlasmaDelay < level.time)
	{
		self->owner->cantmove = 0;
	}
	if((self->sentrydelay<level.time) && (self->selectedsentry))
	{
		if(random()<0.08)
		{
			safe_cprintf(self->selectedsentry, PRINT_HIGH, "Mistake defusing Plasmabomb.\n");
			gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
			self->solid = SOLID_NOT;
			self->touch = NULL;
			VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
			VectorClear (self->velocity);
			self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
			self->s.frame = 0;
			self->s.sound = 0;
			self->s.effects &= ~EF_ANIM_ALLFAST;
			self->think = PlasmaBombExplode;
			self->nextthink = level.time + FRAMETIME;
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_BIGEXPLOSION);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			return;
		}
		self->selectedsentry->cantmove = 0;

		safe_cprintf(self->selectedsentry, PRINT_HIGH, "Plasmabomb defused.\n");

		G_FreeEdict (self);
		return;
	}
	self->nextthink = level.time + 0.1;
}

void DropPlasmaBomb (edict_t *self, int time)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	bfg->wf_team = self->wf_team;
	VectorCopy (self->s.origin, bfg->s.origin);
	bfg->movetype = MOVETYPE_NONE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	if(self->wf_team == CTF_TEAM1)  //Shelton Red team sprite
		bfg->s.modelindex = gi.modelindex ("sprites/r_bfg1.sp2");
	else if(self->wf_team == CTF_TEAM2)  //Shelton Blue team sprite
		bfg->s.modelindex = gi.modelindex ("sprites/b_bfg1.sp2");
	else
		bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");

	bfg->owner = self;
	bfg->radius_dmg = 300;	//was 3000 // TeT was 2000 pts of damage - wow
	bfg->dmg_radius = 2000;	//was 4000
	bfg->classname = "plasma blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	bfg->think = plasmabomb_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->delay = level.time + time;
	bfg->PlasmaDelay =level.time + 2;
	self->cantmove = 1;
	VectorCopy(self->s.origin,self->LockedPosition);
	self->PlasmaDelay = level.time + time + 10;
	gi.linkentity (bfg);
}

void DropPlasmaShort(edict_t *ent, pmenu_t *p)
{
	int time;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] <50)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells (50) for Plasma Bomb.\n");
		PMenu_Close(ent);
		return;
	}
//	time= (((int)(random()*1000)%10))+10;
	time= 10.0;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] =ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] - 50;
	DropPlasmaBomb(ent,time);
	PMenu_Close(ent);
}
void DropPlasmaMedium(edict_t *ent, pmenu_t *p)
{
	int time;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] <50)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells (50) for Plasma Bomb.\n");
		PMenu_Close(ent);
		return;
	}
//	time= (((int)(random()*1000)%14))+23;
	time= 25.0;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] =ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] - 50;
	DropPlasmaBomb(ent,time);
	PMenu_Close(ent);
}
void DropPlasmaLong(edict_t *ent, pmenu_t *p)
{
	int time;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] <50)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells (50) for Plasma Bomb.\n");
		PMenu_Close(ent);
		return;
	}
//	time= (((int)(random()*1000)%20))+50;
	time= 50.0;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] =ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] - 50;
	DropPlasmaBomb(ent,time);
	PMenu_Close(ent);
}
void PlasmaBombMenu(edict_t *ent)
{
	PMenu_Close(ent);
if (!ent->bot_client)
	sprintf(ent->client->wfsentrystr[0], "Plasmabomb Det Time");//acrid added s
	ent->client->sentrymenu[0].text = ent->client->wfsentrystr[0];
	ent->client->sentrymenu[0].SelectFunc = NULL;
	ent->client->sentrymenu[0].align = PMENU_ALIGN_CENTER;
	ent->client->sentrymenu[0].arg = 0;
	ent->client->sentrymenu[1].text = "1. Short";
	ent->client->sentrymenu[1].SelectFunc = DropPlasmaShort;
	ent->client->sentrymenu[1].align = PMENU_ALIGN_LEFT;
	ent->client->sentrymenu[1].arg = 0;
	ent->client->sentrymenu[2].text = "2. Medium";
	ent->client->sentrymenu[2].SelectFunc = DropPlasmaMedium;
	ent->client->sentrymenu[2].align = PMENU_ALIGN_LEFT;
	ent->client->sentrymenu[2].arg = 0;
	ent->client->sentrymenu[3].text = "3. Long";
	ent->client->sentrymenu[3].SelectFunc = DropPlasmaLong;
	ent->client->sentrymenu[3].align = PMENU_ALIGN_LEFT;
	ent->client->sentrymenu[3].arg = 0;
	PMenu_Open(ent, ent->client->sentrymenu, -1, sizeof(ent->client->sentrymenu) / sizeof(pmenu_t), true, false);
}

void cmd_PlasmaBombMenu(edict_t *ent)
{
	if (!ent->client)
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] <50)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells (50) for Plasma Bomb.\n");
		return;
	}
	else
	{
		if (ent->PlasmaDelay < level.time)
		{
			PlasmaBombMenu(ent);
			//ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] =ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] - 50;
			//DropPlasmaBomb(ent,15);		// Explode in 15 seconds
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Not enough time has passed since last Plasma Bomb.\n");
		}
	}
}
// TeT fixed command to take arguments
void cmd_PlasmaBomb(edict_t *ent)
{
	char    *string;
	int 	time = 0;

	string = gi.args();

	if (!ent->client) return;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] <50)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells (50) for Plasma Bomb.\n");
		return;
	}
	if (ent->PlasmaDelay > level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough time has passed since last Plasma Bomb.\n");
		return;
	}

	//argument = "Short", "Medium", "Long"
	if (Q_stricmp ( string, "Short") == 0)
	{
		time = 10;
	}
	else if (Q_stricmp ( string, "Medium") == 0)
	{
		time = 25;
	}
	else if (Q_stricmp ( string, "Long") == 0)
	{
		time = 50;
	}
	else
	{
		//Otherwise pop up menu
		cmd_PlasmaBombMenu(ent);
		return;
	}
	if ( time > 0)
	{
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 50;
		DropPlasmaBomb(ent,time);
	}
}

