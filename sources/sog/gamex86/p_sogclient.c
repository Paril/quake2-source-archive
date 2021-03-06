#include "g_local.h"
#include "g_sogutil.h"

//Knightmare added
#include "g_sogcvar.h"

//======================================================================
// Called after a client first selects class
//======================================================================

void GenPutClientInGame(edict_t *ent)
{
	if(ent->client->resp.player_next_class <= NO_CLASS)
		return;
	
	if(sog_ctf->value || 
	  (sog_team->value && ((int)sogflags->value & SOG_REDBLUE_TEAMS)))
	{
		if(ent->client->resp.team <= CTF_NOTEAM)
		return;
	}
	
	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);
	
	// add a teleportation effect
	GenClientRespawnFX(ent);
	
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	ent->client->respawn_time = level.time;
}



//======================================================================
// common routine used by following InitPersistant funcs
//======================================================================

static void MaxClassAmmo(gclient_t *client)
{
	// Ammo
	client->pers.inventory[AMMO_SHELLS_INDEX] = client->pers.max_shells;
	client->pers.inventory[AMMO_BULLETS_INDEX] = client->pers.max_bullets;
	client->pers.inventory[AMMO_CELLS_INDEX] = client->pers.max_cells;
	client->pers.inventory[AMMO_ROCKETS_INDEX] = client->pers.max_rockets;
}


//======================================================================

gitem_t * InitQ2Persistant(gclient_t *client)
{
	gitem_t *item;
	//Knightmare- maximum ammo cvars
	client->pers.max_rockets	= idg4_max_rockets->value;
	client->pers.max_slugs		= idg4_max_slugs->value;
	client->pers.max_grenades	= idg4_max_grenades->value;
	client->pers.max_cells		= idg4_max_cells->value;

	if(sog_ctf->value)
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] =1;

	//give them all weapons if its a Fully loaded game -Skid
	if((int)sogflags->value & SOG_FULLYLOADED)
	{
		// Armor
		client->pers.inventory[Q2_BODY] = 200;
		// Ammo
		MaxClassAmmo(client);
		client->pers.inventory[AMMO_SLUGS_INDEX] = client->pers.max_slugs;
		client->pers.inventory[AMMO_GRENADES_INDEX] = client->pers.max_grenades;
		// Weapons
		client->pers.inventory[Q2_SG] = 1;
		client->pers.inventory[Q2_SSG] = 1;
		client->pers.inventory[Q2_MG] = 1;
		client->pers.inventory[Q2_CG] = 1;
		client->pers.inventory[Q2_GL] = 1;
		client->pers.inventory[Q2_HB] = 1;
		client->pers.inventory[Q2_RG] = 1;
		client->pers.inventory[Q2_BFG] = 1;
		client->pers.inventory[Q2_RL] = 1;
		item = GetItemByIndex(Q2_RL);
	}
	else
		item = FindItem("Blaster");

	return item;
}

//======================================================================

gitem_t * InitQ1Persistant(gclient_t *client)
{
	gitem_t *item;

	//Knightmare- maximum ammo cvars
	client->pers.max_rockets	= idg3_max_rockets->value;
	client->pers.max_cells		= idg3_max_cells->value;
	client->pers.max_slugs		= idg3_max_slugs->value;   //Can't pickup Slugs and Grenades
	client->pers.max_grenades	= idg3_max_grenades->value;

	if(sog_ctf->value)
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] =1;
		
	item = FindItem("Boomstick");	
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	
// give some ammo for the shotgun -Skid
	if ((int)dmflags->value & DF_INFINITE_AMMO )
		client->pers.inventory[AMMO_SHELLS_INDEX] = 1000;
	else
		client->pers.inventory[AMMO_SHELLS_INDEX] = 25;
	
	if((int)sogflags->value & SOG_FULLYLOADED)
	{
		// Armor
		client->pers.inventory[Q1_RED] = 200;
		//Ammo
		MaxClassAmmo(client);
		// Weapons
		client->pers.inventory[Q1_SSG] = 1;
		client->pers.inventory[Q1_NG] = 1;
		client->pers.inventory[Q1_SNG] = 1;
		client->pers.inventory[Q1_GL] = 1;
		client->pers.inventory[Q1_LG] = 1;
		client->pers.inventory[Q1_RL] = 1;
		item =GetItemByIndex(Q1_RL);
	}
	return item;
}

//======================================================================

gitem_t * InitDoomPersistant(gclient_t *client)
{
	gitem_t *item;

	//Knightmare- maximum ammo cvars
	client->pers.max_shells		= idg2_max_shells->value;
	client->pers.max_bullets	= idg2_max_bullets->value;
	client->pers.max_rockets	= idg2_max_rockets->value;
	client->pers.max_cells		= idg2_max_cells->value;
	client->pers.max_slugs		= idg2_max_slugs->value;
	client->pers.max_grenades	= idg2_max_grenades->value;

	if(sog_ctf->value)
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] =1;
		
	item = FindItem("Pistol");	
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	if ((int)dmflags->value & DF_INFINITE_AMMO )
		client->pers.inventory[AMMO_BULLETS_INDEX] = 1000;
	else
		client->pers.inventory[AMMO_BULLETS_INDEX] = 50;
		
	if((int)sogflags->value & SOG_FULLYLOADED)
	{
		//Armor
		client->pers.inventory[D_BLUE] = 200;
		//Ammo
		MaxClassAmmo(client);
		//Weapons
		client->pers.inventory[D_CSAW] = 1;
		client->pers.inventory[D_SG] = 1;
		client->pers.inventory[D_CG] = 1;
		client->pers.inventory[D_RL] = 1;
		client->pers.inventory[D_PG] = 1;
		client->pers.inventory[D_BFG] = 1;
		client->pers.inventory[D_SSG] = 1;
		item = GetItemByIndex(D_RL);
	}
	return item;
}

//======================================================================

gitem_t * InitWolfPersistant(gclient_t *client)
{
	gitem_t *item;

	//Knightmare- maximum ammo cvars
	client->pers.max_rockets	= idg1_max_rockets->value;
	client->pers.max_bullets	= idg1_max_bullets->value;
	client->pers.max_slugs		= idg1_max_slugs->value;
	client->pers.max_grenades	= idg1_max_grenades->value;
	client->pers.max_cells		= idg1_max_cells->value;

	if(sog_ctf->value)
		client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] =1;
	
	item = FindItem("Walter P-38");	
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	if ( (int)dmflags->value & DF_INFINITE_AMMO )
		client->pers.inventory[AMMO_BULLETS_INDEX] = 1000;
	else
		client->pers.inventory[AMMO_BULLETS_INDEX] = 20;
		
	if((int)sogflags->value & SOG_FULLYLOADED)
	{
		client->pers.inventory[D_BLUE] = 200;
		//Ammo
		MaxClassAmmo(client);
		//Weapons
		client->pers.inventory[W_MG] = 1;
		client->pers.inventory[W_GG] = 1;
		client->pers.inventory[W_GG2] = 1;
		client->pers.inventory[W_RL] = 1;
		client->pers.inventory[W_FT] = 1;
		item = GetItemByIndex(W_FT);
	}
	return item;
}

void Q1_DropBackPack(edict_t *ent)
{
	edict_t	*q1backpack;

	q1backpack = Drop_Item(ent, FindItemByClassname("item_idg3_backpack"));
	q1backpack->touch = Touch_Item;
	q1backpack->owner = ent;
	q1backpack->nextthink = level.time + 120;
	q1backpack->think = G_FreeEdict;
}

//======================================================================
/*
================================================================================
Death View
================================================================================
by Steffen "The Addict" Itterheim
E-Mail: 110213.1772@compuserve.com
or      The_Addict@compuserve.com

keeps looking at killer
called from Client_Think
================================================================================
================================================================================
*/

void LookAtKiller2 (edict_t *ent)
{
	gclient_t	*client;
	float factor, distance;
	float fovdiv=14.00;

	vec3_t vec1, vec2;
	vec_t anglevec;
	float temp1, temp2;

	//Skid added
	int		contents;
	vec3_t	eyes;

	client = ent->client;

	if(!ent->goalentity)
	{
		return;
	}

	// monsters walk around, players run, so we need a higher factor for players
	if (ent->goalentity->classname == "player")
		factor = 0.33;	
	else
		factor = 0.05;
	
	if (ent->deadflag == DEAD_DYING)
	{
		//misuse ideal_yaw for later adjustment of pitch angle,
		//for the case that the player killed him/herself
		//we need a static pitch angle, so it's been put to ideal_yaw for now
		//also randomize the ROLL angle

		ent->ideal_yaw = crandom() * 30 - 20;
		client->ps.viewangles[ROLL] = crandom() * 50;
		if (client->ps.viewangles[ROLL] > 0 && client->ps.viewangles[ROLL] < 5)
			client->ps.viewangles[ROLL] = 5;
		else if (client->ps.viewangles[ROLL] < 0 && client->ps.viewangles[ROLL] > -5)
			client->ps.viewangles[ROLL] = -5;

		ent->deadflag = DEAD_DEAD;
	}

	if (strcmp (ent->goalentity->classname, "worldspawn") == 0 ||
		VectorCompare (ent->goalentity->s.origin, ent->s.origin))	//killed by world or self kill
	{
		ent->goalentity = ent;

		//assume self/world kill -> simply perform full rotations
		client->ps.viewangles[PITCH] = ent->ideal_yaw;
		client->ps.viewangles[YAW] += 1;
		if (ent->deadflag == DEAD_GIBBED)
			ent->viewheight = 22;
		else if (ent->viewheight > -2)
			ent->viewheight -= 1;
		if (client->ps.viewangles[YAW] >= 180)
			client->ps.viewangles[YAW] -= 360;
		return;
	}


//PITCH
	//create two vectors, one from player origin + viewheight to killer origin + viewheight
	//the other from player origin + viewheight to killer origin on the Z axis plane (IOW, ignoring the height)
	//that way we're able to calculate the pitch angle

	vec1[0] = ent->goalentity->s.origin[0] - ent->s.origin[0];
	vec1[1] = ent->goalentity->s.origin[1] - ent->s.origin[1];
	vec1[2] = (ent->goalentity->s.origin[2] + ent->goalentity->viewheight) - (ent->s.origin[2] + ent->viewheight);

	VectorCopy (vec1, vec2);
	vec2[2] = 0;

	//calculating the pitch angle
	anglevec = DotProduct(vec1, vec2);
	anglevec = anglevec / VectorLength(vec1) / VectorLength(vec2);

	// player viewpoint higher than killer's viewpoint?
	if (vec1[2] > 0)  
		anglevec = acos(anglevec) * -57.2957795131 + 90;  // 180 / M_PI = 57.295...
	else
		anglevec = acos(anglevec) * 57.2957795131 + 90;  

	//add 90 to get rid of a possible sign, only to make calculations more convenient
	client->ps.viewangles[PITCH] += 90; 
	client->ps.viewangles[PITCH] += (anglevec - client->ps.viewangles[PITCH]) * factor;
	//subtract the 90 degrees added above
	client->ps.viewangles[PITCH] -= 90;  

//PITCH

//FOV/VIEWHEIGHT	
	//go down slowly, then adjust FOV if viewheight < -2
	//DEAD_GIBBED is set when player dies in lava
	//if the player dies in lava and the viewheight is below 22 the player's view will be distorted


VectorCopy(ent->s.origin,eyes);
eyes[2] += ent->viewheight;
contents = gi.pointcontents (eyes);
	
	if(ent->viewheight < 6 && contents & (CONTENTS_SOLID))
	{
//		gi.dprintf("View in solid - adjusting\n");
		ent->viewheight++;
	}
	else if (ent->viewheight > -2 && ent->deadflag != DEAD_GIBBED)
		ent->viewheight -= 1;
	else
	{
		// get the distance from the killer
		distance = VectorLength(vec1);
		//if (fovdiv->value == 0)
		//	gi.cvar_set ("fovdiv", "1");
		//distance /= fovdiv->value;
		distance /= fovdiv;

		if (distance > 80)
			distance = 80;
		distance = 90 - distance;
		distance -= client->ps.fov;
		//adjust FOV
		client->ps.fov += distance * factor;
	}
//FOV/VIEWHEIGHT	


//YAW
	//add 180 to YAW angle to get rid of any possible signs
	client->ps.viewangles[YAW] += 180;

	//get the angle towards killer
	vectoangles(vec2, vec1);
	vec1[YAW] += 180;
	if (vec1[YAW] > 360)
		vec1[YAW] -= 360;

 //is current viewing angle greater than the angle towards the killer?
	if (client->ps.viewangles[YAW] > vec1[YAW])
	{
		//check which direction (left/right) is nearer
		temp1 = client->ps.viewangles[YAW] - vec1[YAW];
		//since 350 is higher than 5, but the difference is still only 15, check if that is the case
		temp2 = vec1[YAW] + 360 - client->ps.viewangles[YAW];

		//depending on which of the above two is smaller, use it to add/subtract to the YAW angle
		//or do nothing if both are equal (there's an extremely small chance that if the killer is standing still
		//and the player is looking exactly in the opposite direction, then nothing will happen)
		//i don't think this will ever become an issue though...
		if (temp1 < temp2)
		{
			client->ps.viewangles[YAW] -= temp1 * factor;
			if (client->ps.viewangles[YAW] < 0)
				client->ps.viewangles[YAW] += 360;
		}
		else if (temp1 > temp2)
		{
			client->ps.viewangles[YAW] += temp2 * factor;
			if (client->ps.viewangles[YAW] > 360)
				client->ps.viewangles[YAW] -= 360;
		}
	}
	else
	{
		temp1 = vec1[YAW] - client->ps.viewangles[YAW];
		temp2 = client->ps.viewangles[YAW] + 360 - vec1[YAW];

		if (temp1 < temp2)
		{
			client->ps.viewangles[YAW] += temp1 * factor;
			if (client->ps.viewangles[YAW] < 0)
				client->ps.viewangles[YAW] += 360;
		}
		else if (temp1 > temp2)
		{
			client->ps.viewangles[YAW] -= temp2 * factor;
			if (client->ps.viewangles[YAW] > 360)
				client->ps.viewangles[YAW] -= 360;
		}
	}

	client->ps.viewangles[YAW] -= 180;
//YAW
}
//SI_END


/*
======================================================
Give the dude a GOALENTITY to track - Skid
called in Player_die 
======================================================
*/
void WhoKilledDoomguy (edict_t *self, edict_t *attacker) 
{
	if(attacker && (attacker->client || (attacker->svflags & SVF_MONSTER)))
		self->goalentity = attacker; 
	else //if(inflictor && (strcmp(inflictor->classname, "worldspawn")==0))
		self->goalentity = NULL;
}



/*
======================================
Use func
playing sounds and pushing buttons in range
======================================
*/

//======================================================================
//======================================================================
// Use Button
//======================================================================
//======================================================================

//void button_fire (edict_t *self);

void button_use (edict_t *self, edict_t *other, edict_t *activator);
void OldDudesUseState(edict_t *ent)
{
	vec3_t forward,right,offset,start,end;
	trace_t tr;

	if(ent->client->resp.player_class != CLASS_DOOM &&
	   ent->client->resp.player_class != CLASS_WOLF)
		return;

	if(ent->touch_debounce_time < level.time)
	{
		VectorSet(offset, 0, 0,  ent->viewheight-8);
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		VectorMA (start, 48 ,forward, end);
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, (CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW));
	
		if(tr.fraction < 1.0)
		{
			if(tr.ent->movetype == MOVETYPE_STOP && !tr.ent->health)
			{
				if(Q_stricmp(tr.ent->classname,"func_button")==0) 
				{
					//button_fire(tr.ent);
					button_use (tr.ent, ent,ent);// edict_t *activator)
					//ent->client->dlasttouchsound = level.time + 0.8;
					ent->touch_debounce_time = level.time + 0.5;
					return;
				}
			}
			else if((tr.contents & MASK_SOLID) && 
					(tr.ent->movetype != MOVETYPE_PUSH) &&
					(ent->client->dlasttouchsound < level.time))
			{
				if(ent->client->resp.player_class == CLASS_DOOM)
					gi.sound (ent, CHAN_BODY, gi.soundindex("idg2/push.wav"), 1.0, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_BODY, gi.soundindex("idg1/push.wav"), 1.0, ATTN_NORM, 0);
				
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
				ent->client->dlasttouchsound = level.time + 0.8;
			}
		}
	}
}




void Animgib(edict_t *self)
{
	if (self->s.frame == 7)
	{
//		self->movetype = MOVETYPE_NONE;
		self->nextthink = level.time + 10 +(random()*10); 
		self->think = G_FreeEdict;
		gi.linkentity (self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}


void droptofloor (edict_t *ent);

void ThrowDoomguy (edict_t *self)
{
	edict_t *doomgib;
	
	doomgib = G_Spawn();
	VectorCopy (self->s.origin, doomgib->s.origin);
	doomgib->clipmask = MASK_SHOT;
	doomgib->s.effects = EF_GIB;
	VectorSet (doomgib->mins, -16, -16, -24);
	VectorSet (doomgib->maxs, 16, 16, 24);
	doomgib->s.modelindex = gi.modelindex ("models/objects/idg2gibs/tris.md2"); 
	doomgib->owner = 0;
	doomgib->touch = 0;
	doomgib->nextthink = level.time + FRAMETIME;
	doomgib->think = Animgib;
	doomgib->solid = SOLID_NOT;
	doomgib->s.sound = 0;
	doomgib->takedamage = DAMAGE_NO;
	doomgib->classname = "doomgib";
	doomgib->movetype = MOVETYPE_TOSS;  

	gi.linkentity(doomgib);
	
		
	self->s.modelindex2 = 0;
	self->s.frame = 0;
	self->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	self->s.skinnum = 0;
	//gi.setmodel(self,"models/objects/gibs/sm_meat/tris.md2");
	VectorSet (self->mins, -16, -16, -16);
	VectorSet (self->maxs, 16, 16, 16);
	self->s.origin[2] += 24;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->clipmask = MASK_SHOT;
	self->flags |= FL_NO_KNOCKBACK;
	self->movetype = MOVETYPE_TOSS; 
	self->nextthink = level.time + 10 +(random()*10); 
	self->think = G_FreeEdict;
	gi.linkentity (self);
}

