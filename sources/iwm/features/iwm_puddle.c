#include "../g_local.h"

#define TELEPUDDLE_MODEL "models/other/puddle/tris.md2"

#define TP_THROWN_TELE_NOT_LAYED	1
#define TP_THROWN_TELE_LAYED		2
#define P_THROWN_TELE_NOT_LAYED		4
#define P_THROWN_TELE_LAYED			8

void TelePuddle_Frames (edict_t *ent)
{
	ent->s.frame++;

	if (ent->s.frame == 8)
		ent->s.frame = 0;

	ent->nextthink = level.time + .1;
}

void TelePuddle_LayDestination (edict_t *ent)
{
	vec3_t start, forward;
	vec3_t maxs = {12, 12, 2};
	vec3_t mins = {-12, -12, -2};
	edict_t *dest = ent->client->resp.puddle_dest;

	dest = G_Spawn();

	AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorCopy(ent->s.origin, start);
	VectorMA(start, 50, forward, start);

	VectorCopy (start, dest->s.origin);
	dest->movetype = MOVETYPE_TOSS;
	dest->solid = SOLID_TRIGGER;
	VectorCopy (mins, dest->mins);
	VectorCopy (maxs, dest->maxs);
	dest->s.modelindex = ModelIndex(TELEPUDDLE_MODEL);
	dest->owner = ent;
	dest->think = TelePuddle_Frames;
	dest->nextthink = level.time + .1;
	dest->s.sound = SoundIndex ("misc/puddle_amb.wav");
	
	gi.linkentity(dest);

	ent->client->resp.puddle_dest = dest;
	ent->client->resp.layed_tele = 1;
}

void SinkIntoGround (edict_t *ent)
{
	vec3_t forward, right, up, or;
	//ent->s.origin[2] -= 5;
	//VectorCopy (ent->client->resp.puddle_tele->s.angles, or);
	//VectorInverse (or);
	AngleVectors (ent->client->resp.puddle_tele->s.angles, forward, right, up);

	VectorInverse(forward);

	VectorScale (forward, 12, or);

	VectorAdd (ent->s.origin, or, ent->s.origin);

	//VectorScale (ent->s.origin, 0.0001, ent->s.origin);
	//gi.dprintf ("SinkIntoGround: %s %s %s\n", vtos(forward), vtos(right), vtos(up));

	gi.linkentity(ent);

	ent->nextthink4 = level.time + .1;
}

void MoveUpFromGround (edict_t *ent)
{
	vec3_t forward, right, up, or;
	//ent->s.origin[2] -= 5;
	//VectorCopy (ent->client->resp.puddle_tele->s.angles, or);
	//VectorInverse (or);
	AngleVectors (ent->client->resp.puddle_dest->s.angles, forward, right, up);

	//VectorInverse(forward);

	VectorScale (forward, 15, or);

	VectorAdd (ent->s.origin, or, ent->s.origin);

	//VectorScale (ent->s.origin, 0.0001, ent->s.origin);
	//gi.dprintf ("SinkIntoGround: %s %s %s\n", vtos(forward), vtos(right), vtos(up));

	gi.linkentity(ent);

	ent->nextthink4 = level.time + .1;
}

void Stop_Think2 (edict_t *ent)
{
	G_FreeEdict (ent->client->resp.puddle_dest);

	ent->client->resp.puddle_dest = NULL;

	ent->client->teleing = 0;

	//ent->s.origin[2] += 24;

	ent->think4 = NULL;
	ent->nextthink4 = -1;
}

void TelePuddle_TeleFragNearby (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 40))
	{
		if (blip == ent)
			continue;
		if (!blip->takedamage)
			continue;
			// Found player, activate touch.
		T_Damage (blip, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 9999999, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
	}

	ent->nextthink2 = level.time + .1;
}

void TelePort_Player (edict_t *other)
{
	edict_t *ent = other->client->resp.puddle_tele;
//	float oldor;


	if (!other->client->resp.puddle_dest)
	{
		G_FreeEdict(ent);
		return;
	}

	// Owner touched it. Woot.
	// Todo: Event Stuff
	other->client->resp.layed_tele = 0;
	other->client->resp.layed_t = 0;
	//oldor = other->s.origin[2];
	VectorCopy (other->client->resp.puddle_dest->s.origin, other->s.origin);
	//other->s.origin[2] = oldor;
	//other->s.origin[2] -= 48;
	gi.linkentity(other);
	//G_FreeEdict (other->client->resp.puddle_dest);
	G_FreeEdict (other->client->resp.puddle_tele);
	//other->client->resp.puddle_dest = NULL;
	other->client->resp.puddle_tele = NULL;
	TelePuddle_TeleFragNearby(other);
	VectorClear (other->velocity);

	other->think4 = MoveUpFromGround;
	other->nextthink4 = level.time + .1;

	other->think3 = Stop_Think2;
	other->nextthink3 = level.time + 0.6;
}

void TelePuddle_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other != ent->owner)
		return;
	if (other->client->teleing)
		return;

	other->client->teleing = 1;
	other->think4 = SinkIntoGround;
	other->nextthink4 = level.time + .1;
	other->think3 = TelePort_Player;
	other->nextthink3 = level.time + 0.3;
}

void TelePuddle_LayTeleporter (edict_t *ent)
{
	vec3_t start, forward;
	vec3_t maxs = {12, 12, 2};
	vec3_t mins = {-12, -12, -2};
	edict_t *port = ent->client->resp.puddle_tele;

	port = G_Spawn();
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorCopy(ent->s.origin, start);
	VectorMA(start, 50, forward, start);

	VectorCopy (start, port->s.origin);
	port->movetype = MOVETYPE_TOSS;
	port->solid = SOLID_TRIGGER;
	VectorCopy (mins, port->mins);
	VectorCopy (maxs, port->maxs);
	port->s.modelindex = ModelIndex(TELEPUDDLE_MODEL);
	port->s.skinnum = 1;
	port->owner = ent;
	port->touch = TelePuddle_Touch;
	port->think = TelePuddle_Frames;
	port->nextthink = level.time + .1;
	port->s.sound = SoundIndex ("misc/puddle_amb.wav");
	
	gi.linkentity(port);

	ent->client->resp.puddle_tele = port;
	ent->client->resp.layed_t = 1;

}

void Cmd_TelePuddle (edict_t *ent)
{
	if (CheckBan(feature_ban, FEATUREBAN_TELEPUDDLE))
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, TelePuddle is banned.\n");
		return;
	}

	ent->client->newweapon = FindItem("Porta-Puddle");
	return;

	if (ent->client->resp.layed_tele && !ent->client->resp.layed_t)
	{
		TelePuddle_LayTeleporter (ent);
		return;
	}
	else if (ent->client->resp.layed_tele && ent->client->resp.layed_t)
	{
		safe_cprintf (ent, PRINT_HIGH, "You already have a teleporter out!\n");
		return;
	}
	else
		TelePuddle_LayDestination (ent);
}


/*
New Puddle!
It's a weapon. Pretty.
*/
void PuddleSound (edict_t *ent, int miss);

void l_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t maxs = {12, 12, 2};
	vec3_t mins = {-12, -12, -2};

	if (other == self->owner)
		return;
/*	if (other == self->owner && self->s.skinnum == 0)
	{
		if (other->client->teleing)
			return;
		if (!other->client->resp.layed_t & P_THROWN_TELE_LAYED)
			return;

		other->client->teleing = 1;
		other->think2 = SinkIntoGround;
		other->nextthink2 = level.time + .1;
		other->think3 = TelePort_Player;
		other->nextthink3 = level.time + .3;
		return;
	}*/

	if (other && other->takedamage)
	{
		if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
			self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		else
			self->owner->client->resp.layed_t &= ~P_THROWN_TELE_NOT_LAYED;
		if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
		{
			G_FreeEdict(self);
			self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		}
		return;
	}	
	if (!plane)
	{
		if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
			self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		else
			self->owner->client->resp.layed_t &= ~P_THROWN_TELE_NOT_LAYED;
		// Die.
		// Literally shouldn't happen, but it will anyway. Shishaw.
		if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
		{
			G_FreeEdict(self);
			self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		}
		return;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
			self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		else
			self->owner->client->resp.layed_t &= ~P_THROWN_TELE_NOT_LAYED;

		G_FreeEdict (self);
		return;
	}

	vectoangles(plane->normal,self->s.angles);
	VectorScale (self->velocity, 0, self->velocity);
	self->movetype = MOVETYPE_FLYMISSILE;
	self->solid = SOLID_BBOX;

	if (self->owner->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED)
	{
		self->owner->client->resp.layed_t &= ~TP_THROWN_TELE_NOT_LAYED;
		self->owner->client->resp.layed_t |= TP_THROWN_TELE_LAYED;
	}
	else
	{
		self->owner->client->resp.layed_t &= ~P_THROWN_TELE_NOT_LAYED;
		self->owner->client->resp.layed_t |= P_THROWN_TELE_LAYED;
	}

	if (self->owner->client->resp.layed_t & P_THROWN_TELE_LAYED)
	{
		VectorCopy (mins, self->mins);
		VectorCopy (maxs, self->maxs);
		self->touch = TelePuddle_Touch;
	}
}

void TelePuddle_FindPlayer (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 36))
	{
		if (!blip->takedamage)
			continue;
		if (!blip->client)
			continue;
		if (ent->owner == blip)
		{
			//gi.dprintf ("Found a player..?\n");
			// Found player, activate touch.
			ent->touch (ent, blip, NULL, NULL);
		}
	}

	ent->nextthink2 = level.time + .1;
}

void PuddleSound (edict_t *ent, int miss)
{
	if (miss)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/puddlemiss.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/puddlefire.wav"), 1, ATTN_NORM, 0);
}

void fire_puddle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *bolt;
	trace_t	tr;
	qboolean red = false;
	vec3_t end, forward;

	speed = 8000;
	
	//VectorCopy(ent->s.origin, start);
	AngleVectors(self->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.surface && (tr.surface->flags & SURF_SKY))
	{
		PuddleSound (self, 1);
		return;
	}

	if (tr.ent->solid != SOLID_BSP)
	{
		PuddleSound (self, 1);
		return;
	}

	if (!self->client)
		return;

	bolt = self->client->resp.puddle_tele;

	if (self->client->teleing)
	{
		PuddleSound (self, 1);
		return;
	}
	if (self->client->resp.layed_t & TP_THROWN_TELE_NOT_LAYED || self->client->resp.layed_t & P_THROWN_TELE_NOT_LAYED)
	{
		PuddleSound (self, 1);
		return;
	}
	if (self->client->resp.layed_t & P_THROWN_TELE_LAYED)
	{
		PuddleSound (self, 1);
		return;
	}
	if (self->client->resp.layed_t & TP_THROWN_TELE_LAYED)
		red = true;

	if (!red)
		bolt = self->client->resp.puddle_dest;

	if (!red)
		self->client->resp.layed_t |= TP_THROWN_TELE_NOT_LAYED;
	else
		self->client->resp.layed_t |= P_THROWN_TELE_NOT_LAYED;

	VectorNormalize (dir);
	PuddleSound (self, 0);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	//VectorCopy (tr.endpos, start);
	vectoangles (dir, bolt->s.angles);
	//VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	//VectorCopy (mins, bolt->mins);
	//VectorCopy (maxs, bolt->maxs);	
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex(TELEPUDDLE_MODEL);
	if (red)
		bolt->s.skinnum++;
	bolt->s.sound = SoundIndex ("misc/puddle_amb.wav");
	bolt->owner = self;
	bolt->touch = l_touch;
	bolt->nextthink = level.time + .1;
	bolt->think = TelePuddle_Frames;
	bolt->nextthink2 = level.time + .1;
	bolt->think2 = TelePuddle_FindPlayer;
	//bolt->s.renderfx |= RF_FULLBRIGHT;
	bolt->s.effects |= EF_FLAG2;
	if (red)
	{
		bolt->s.effects |= EF_FLAG1;
		bolt->s.effects &= ~EF_FLAG2;
	}
	if (red)
		bolt->classname = "puddle_t";
	else
		bolt->classname = "puddle";

	if (red)
		self->client->resp.puddle_tele = bolt;
	else
		self->client->resp.puddle_dest = bolt;

	VectorCopy (tr.endpos, bolt->s.origin);
	gi.linkentity (bolt);
	bolt->touch (bolt, NULL, &tr.plane, tr.surface);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
		G_FreeEdict(bolt);
}	