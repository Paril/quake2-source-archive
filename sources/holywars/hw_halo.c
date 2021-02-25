/*
==================================================================================
HOLY WARS - Halo
==================================================================================
The functions which directly deal with the halo and the Saint can be found here.
*/

#include "g_local.h"


// ***************
// Spawns the Halo
// ***************
void HW_SpawnHalo(edict_t *spot)
{
	gitem_t *haloitem;
	edict_t	*halo;
	edict_t	*nextplayer = NULL;
	edict_t *halosearch = NULL;

	if ((halosearch = G_Find (halosearch, FOFS(classname), "halo")) != NULL)
		return;   // there is already an halo

	if ((haloitem = FindItemByClassname("halo")) == NULL)
		return;   // something went wrong

	halo = G_Spawn();                               // The halo was born
	halo->classname = haloitem->classname;          // Give it a name
	halo->item = haloitem;                          // Link it with its "item"
	halo->spawnflags = DROPPED_ITEM;                // It's free to take
	halo->s.effects = haloitem->world_model_flags | EF_BLASTER;  // Yellow light and golden particles
	halo->s.renderfx = RF_GLOW;                     // Bright
	halo->s.event = EV_ITEM_RESPAWN;                // Respawn effect
	VectorSet(halo->mins, -10, -10, -10);           // Set bounding box
	VectorSet(halo->maxs, 10, 10, 10);              //
	gi.setmodel (halo, halo->item->world_model);    // Set model
	if (hw_shoothalo->value)
	{
		halo->solid = SOLID_BBOX;					// allow halo to be hit
		halo->takedamage = DAMAGE_YES;				// damage causes bouncing
	}
	else
	{
		halo->solid = SOLID_TRIGGER;				// don't allow halo to be hit
		halo->takedamage = DAMAGE_NO;				// can't be damaged
	}
	halo->movetype = MOVETYPE_BOUNCE;               // Bouncing
	halo->touch = Touch_Item;                       // Invoke this when touched
	halo->owner = halo;								// Nobody owns it
	hw.halostatus = BASE;                           // It's in base now
	VectorCopy (spot->s.origin, halo->s.origin);    // Phisically put it in base
	halo->velocity[2] = 1;                          // Tip it off
	gi.linkentity (halo);                           // Good... Link it!

	// Monks sound:
	gi.sound(spot, CHAN_VOICE, gi.soundindex ("hw/hw_spawn.wav"), 1, ATTN_NONE, 0);
	// Reports the halo spawning to players:
	while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
		gi.centerprintf (nextplayer, "The Halo spawned!\n");
	// Sets the next report time:
	hw.nextreport = level.time + hw_report->value;
}



// ********************
// Halo pickup function
// ********************
qboolean HW_PickupHalo(edict_t *ent, edict_t *other)
{
	gitem_t *halo;
	edict_t *visiblehalo;

// HWv2.1
	// llamas can't become saints
	if (other->client->resp.score < hw_llama->value)
		return false;
// end HW2.1

	if ((halo = FindItemByClassname("halo")) == NULL)
		return false;
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;  // add the halo to the inventory

	HW_BecomeSaint(other);

	ent->owner = other;      // The halo is now the Saint's personal property
	hw.halostatus = OWNED;
	hw.teleported = 0;
	hw.nextreport = level.time + hw_report->value;  // Resets next report time.

	// Thunder sound:
	gi.sound(ent, CHAN_BODY, gi.soundindex("hw/hw_thund.wav"), 0.7, ATTN_NONE, 0);

	// -------------------------------------------------------------
	// spawns an incorporeal visible halo over the head of the Saint
	// -------------------------------------------------------------
	visiblehalo = G_Spawn();
	visiblehalo->classname = "visiblehalo";
	gi.setmodel (visiblehalo, "models/halo/tris.md2");
	visiblehalo->s.renderfx = RF_GLOW;
	visiblehalo->movetype = MOVETYPE_NOCLIP;
	visiblehalo->solid = SOLID_NOT;
	visiblehalo->s.effects |= EF_BLASTER;
	visiblehalo->hwowner = other;
	VectorCopy (ent->s.origin, visiblehalo->s.origin);  // places it where the true halo was
	visiblehalo->s.angles[PITCH] = 9;
	visiblehalo->think = HW_VisibleHaloThink;
	visiblehalo->nextthink = level.time + 0.1;
	gi.linkentity(visiblehalo);

	return true;
}



// *****************************************
// One frame of the halo flying animation.
// Also reports the situation to the players
// every hw_report->value seconds.
// *****************************************
void HW_VisibleHaloThink(edict_t *self)
{
	vec3_t targetpos;
	char *info;

	// Paranoid checks:
	// (theoretically, samothing bad can happen if a client reconnects
	// soon after connecting and the net drops its "disconnect" packet.
	// For example, a reconnecting player could still be identified by
	// the halo as its hwowner, although he's not a Saint anymore. As
	// somebody reported problems with this, here are some paranoid
	// checks to keep things in shape - a pretty hacked solution, but
	// it should work fine).
	if (!(self->hwowner))
		HW_HaloReset();
    if (!(self->hwowner->client))
		HW_HaloReset();
	if (self->hwowner->client->plstatus != SAINT)
		HW_HaloReset();
	if (!self->hwowner->client->pers.connected)
		HW_HaloReset();

	VectorCopy(self->hwowner->s.origin, targetpos);  // The Saint is the Halo's target.
	targetpos[2] += self->hwowner->viewheight;       // Raise up to eyes.

	// report to players
	if (level.time > hw.nextreport)   // Time to report...
	{
		HW_ReportSaint(self->hwowner);
		hw.nextreport += hw_report->value;
	}

	self->svflags &= ~SVF_NOCLIENT;  // Part of the "teleport trails" hack (see below).

    // Checks if the owner has just been teleported:
	if (hw.teleported)
	{
		hw.teleported = 0;
		VectorCopy (targetpos, self->s.origin);   // Teleport on Saint's head
		self->s.origin[2] += 20;                  // Raise a bit;
		VectorClear (self->velocity);             // Resets halo velocity
		self->s.event = EV_PLAYER_TELEPORT;       // Teleport effect
		self->svflags |= SVF_NOCLIENT;            // Horrible hack to avoid "teleport trails".
	}

	if (!hw_inertia->value)    // "Locked" halo behaviour
	{
		// -----------------------------------------------------------------------------
		// Instead of linking the Halo model to the player model, we calculate the right
		// position and then we manually move it there. We don't worry about messing
		// with movement prediction either. This gives us more flexibility
		// and a nice "lazy movement" effect too.
		// -----------------------------------------------------------------------------
		// Calculate a standard targetpos:
		targetpos[2] += hw_height->value;
		// adjust if the Saint is crouching:
		if (self->hwowner->client->ps.pmove.pm_flags & PMF_DUCKED)
			targetpos[2] += 10;
		else
		{
			// Crackwhore hack:
			// if the player is using the Crackwhore model, raise the halo:
			info = Info_ValueForKey (self->hwowner->client->pers.userinfo, "skin");
    		if(	(info[0] == 'c' || info[0] == 'C') &&
    	    	(info[1] == 'r' || info[1] == 'R') &&
    	    	(info[2] == 'a' || info[2] == 'A') &&
    	    	(info[3] == 'k' || info[3] == 'K') &&
    	    	(info[4] == 'h' || info[4] == 'H') &&
    	    	(info[5] == 'o' || info[5] == 'O') &&
    	    	(info[6] == 'r' || info[6] == 'R') )
    			targetpos[2] += 6;
		}

		// Set halo on Saint's head in a single frame:
		VectorCopy (targetpos, self->s.origin);   // Put on Saint's head
		// Rotation:
		self->s.angles[YAW] += 12;
	}
	else
	{
		// --------------------------------
		// Use inertia effect for the halo.
		// --------------------------------
		// Just to have shortest variable names...
		vec3_t halopos, haloacc, halobrk;
		int calc;
		int i;

		VectorSet(haloacc, hw_accelx->value, hw_accely->value, hw_accelz->value);
		VectorSet(halobrk, hw_brakex->value, hw_brakey->value, hw_brakez->value);

		VectorCopy(self->s.origin, halopos);
		// If the Halo is visible from the player's eyes, try to force it to an higher
		// position on the player's head (this way, the halo will stay down on the head
		// of the player only in particular situations like very low ceilings, etc.)
		if (visible(self->hwowner, self))
		{
			targetpos[2] += hw_height->value;         // Raise over head.
		}

		// Main physics cycle: The main calc for the position is executed more
		// than one time so that the halo reaches a decent speed - this can't be done
		// by simply multiplying acceleration, as this would cause an incorrect
		// behaviour in some cases (ever seen an halo taking orbits around a Saint?).
		for (calc = 1; calc <= hw_cycles->value; calc++)
		{
			for (i = 0; i <= 2; i++)  // Repeat for x, y and z
			{
				// Accelerates towards the right spot:
				if ((halopos[i] + hw_tolerance->value) < targetpos[i])
					self->velocity[i] += haloacc[i];
				else if ((halopos[i] - hw_tolerance->value) > targetpos[i])
					self->velocity[i] -= haloacc[i];

				// Brakes if going the wrong way:
		        if (((halopos[i] - targetpos[i]) > 0) && (self->velocity[i] > halobrk[i]))
		        	self->velocity[i] -= halobrk[i];
		        else if (((halopos[i] - targetpos[i]) < 0) && (self->velocity[i] < -halobrk[i]))
		        	self->velocity[i] += halobrk[i];

				// Clips velocity to its maximum value:
				if (self->velocity[i] > 800)
					self->velocity[i] = 800;
				else if (self->velocity[i] < -800)
					self->velocity[i] = -800;

				// Updates the position according to the velocity:
				halopos[i] += self->velocity[i];
			}
		} // end of (calc = 1; ...) cycle

		// Angles:
		// the halo yaw is equal to the yaw of its direction;
		vectoangles(self->velocity, self->s.angles);
		// the halo pitch is proportional to its velocity (but always less than 70);
		self->s.angles[PITCH] = (int)(VectorLength(self->velocity) * 4) % 70;
		// the halo roll is always 0.
		self->s.angles[ROLL] = 0;

		VectorCopy(halopos, self->s.origin);
	}

	// Prepare next frame:
	self->think = HW_VisibleHaloThink;
	self->nextthink = level.time + 0.1;
	// Update halo state:
	gi.linkentity(self);
}


// ***********************************
// The Halo falls from the dying Saint
// ***********************************
void HW_DropHalo(edict_t *player)
{
	gitem_t *halo;
	edict_t *dropped;

	if ((halo = FindItemByClassname("halo")) != NULL && player->client->pers.inventory[ITEM_INDEX(halo)])
	{
		dropped = Drop_Item(player, halo);

		hw.halostatus = FREE;
		player->client->pers.inventory[ITEM_INDEX(halo)] = 0;

		// override "standard" dropped item properties
		VectorSet(dropped->mins, -10, -10, -10);
		VectorSet(dropped->maxs, 10, 10, 10);
		if (hw_shoothalo->value)
		{
			dropped->solid = SOLID_BBOX;		// dropped halo can be hit
			dropped->takedamage = DAMAGE_YES;	// damage causes bouncing
		}
		else
		{
			dropped->solid = SOLID_TRIGGER;		// dropped halo can't be hit
			dropped->takedamage = DAMAGE_NO;	// can't be damaged
		}
		dropped->movetype = MOVETYPE_BOUNCE;
		dropped->s.effects |= EF_BLASTER;
		dropped->owner = NULL;
		dropped->nextthink = level.time + hw_remove->value;
		dropped->think = HW_HaloDisappear;

		// hack the velocity and the movetype to make it bounce around like crazy:
		dropped->velocity[0] = 100 + (rand() % 50) * hw_halofall->value * (-player->health);
		dropped->velocity[1] = 100 + (rand() % 50) * hw_halofall->value * (-player->health);
		dropped->velocity[2] = 100 + (rand() % 50) * hw_halofall->value * (-player->health);
		if ((rand() % 2) == 1)
			dropped->velocity[0] = -(dropped->velocity[0]);
		if ((rand() % 2) == 1)
			dropped->velocity[1] = -(dropped->velocity[1]);
	}
}


// ************************
// A fallen halo disappears
// ************************
void HW_HaloDisappear(edict_t *ent)
{
	edict_t *nextplayer = NULL;

	// -----------------
	// "teleport" effect
	// -----------------
	edict_t *teleffect;
	teleffect = G_Spawn();
	VectorCopy(ent->s.origin, teleffect->s.origin);
	teleffect->s.event = EV_ITEM_RESPAWN;
	gi.sound(teleffect, CHAN_BODY, gi.soundindex("hw/hw_disap.wav"), 1, ATTN_NONE, 0);
	teleffect->nextthink = level.time + 1;
	teleffect->think = G_FreeEdict;
	gi.linkentity(teleffect);

	// Report to players:
	while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
	{
		gi.centerprintf (nextplayer, "The Halo disappeared...\n");
	}

	// Make the Halo physically disappear:
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->think = HW_HaloRespawn;
	ent->nextthink = level.time + hw_respawn->value;
	gi.linkentity(ent);
	hw.halostatus = HEAVEN;
}


// *************************
// The halo respawns in base
// *************************
void HW_HaloRespawn(edict_t *ent)
{
	edict_t *nextplayer = NULL;

	// Destroy the old Halo:
	G_FreeEdict(ent);

	// Create a new one:
	HW_SpawnHalo(hw.halobase);

	// Report to players:
	while ((nextplayer = G_Find (nextplayer, FOFS(classname), "player")) != NULL)
	{
		gi.centerprintf (nextplayer, "The Halo respawned!\n");
	}

}



// ************************************************
// Destroy the current halo and create another one.
// ************************************************
void HW_HaloReset(void)
{
	edict_t *ent;
	gitem_t *halo;

	gi.dprintf("****************************\nWARNING: Destroying Halo!\n****************************\n");

	// Find and destroy current halo
	if ((ent = G_Find (NULL, FOFS(classname), "halo")) != NULL)
		G_FreeEdict(ent);

	// Destroy visible halo if existing:
    if ((ent = G_Find (NULL, FOFS(classname), "visiblehalo")) != NULL)
    	G_FreeEdict(ent);

	ent = NULL;
	if ((halo = FindItemByClassname("halo")) != NULL)
	    while ((ent = G_Find (ent, FOFS(classname), "player")) != NULL)
		{
			// Remove all traces of the halo from all player's inventories:
			ent->client->pers.inventory[ITEM_INDEX(halo)] = 0;
			// No player is a Saint anymore:
			if(ent->client->plstatus == SAINT)
				ent->client->plstatus = SINNER;
		}

	// Make another halo
	hw.halostatus = HEAVEN;
	hw.halobase->nextthink = level.time + 0.1;
	hw.halobase->think = HW_SpawnHalo;
	gi.linkentity(hw.halobase);
}
