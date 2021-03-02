#include "../g_local.h"
#include "../m_player.h"

#define AGM_RANGE_DELTA				50
#define AGM_RANGE_MIN				50
#define AGM_RANGE_MAX				8000
#define AGM_MOVE_SCALE				10.0

/*
======================================================================
Anti-Grav Manipulator subroutines
======================================================================
*/
void Move_AGM(edict_t *self, vec3_t start, vec3_t aimdir)
{
	gclient_t	*cl = self->client;
	trace_t		tr;
	vec3_t		end;
	vec3_t		dir;
	float		speed;

	if (cl->agm_target == NULL)
	{
		gi.dprintf("BUG: Move_AGM() called with null target\n");
		return;
	}

//	Move our target towards the beam endpoint (make the delta-v proportional to target's 
//	distance from the endpoint).
//	Note that we want to do it here, rather than after the checks for beam-breakage, to make
//	it easier for players to throw their target into a hard surface (this way, it gives the 
//	target a last velocity boost before shutting off the beam).

	VectorMA(start, cl->agm_range, aimdir, end);
	VectorSubtract(end, cl->agm_target->s.origin, dir);
	speed = AGM_MOVE_SCALE * VectorLength(dir);
	VectorNormalize(dir);
	VectorScale(dir, speed, cl->agm_target->velocity);
	gi.linkentity(cl->agm_target);

//	Trace a line from the player to the fixed end of the AGM beam. If the trace meets something that isn't
//	the intended client, disconnect the beam.

	cl->agm_target->flung_by_agm = false;
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.ent && (tr.ent != cl->agm_target) && Q_stricmp(tr.ent->classname, "worldspawn"))
	{
		cl->agm_target->held_by_agm = false;
		cl->agm_target->thrown_by_agm = true;
		cl->agm_target = NULL;
		return;
	}

	if ((tr.fraction < 1.0) && !Q_stricmp(tr.ent->classname, "worldspawn"))
	{
		cl->agm_target->held_by_agm = false;
		cl->agm_target->thrown_by_agm = true;
		cl->agm_target = NULL;
		return;
	}

	cl->agm_target->held_by_agm = true;
	cl->agm_target->thrown_by_agm = false;

//	If the target is being held against a wall, make sure we can't screw up the range calcs by
//	extending the beam endpoint into the wall.

	tr = gi.trace(cl->agm_target->s.origin, NULL, NULL, end, cl->agm_target, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorSubtract(tr.endpos, self->s.origin, dir);
		self->client->agm_range = VectorLength(dir);
	}
}
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);

void Fire_AGM(edict_t *self, vec3_t start, vec3_t aimdir)
{
	trace_t	tr;
	vec3_t	end;
	vec3_t	vbeam;
//	float	dist;
	int		damage;
//	int		kick;

//	Trace a line to try and find something that can be manipulated.

	self->client->agm_target = NULL;
	VectorMA(start, 2000, aimdir, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.startsolid && !tr.ent->client)
		return;

//	As long as we're not firing point-blank at something, draw the tracer beam.

	/*if (!tr.startsolid)
	{
		edict_t	*beam = G_Spawn();

		beam->movetype = MOVETYPE_NOCLIP;
		beam->solid = SOLID_NOT;
		beam->s.renderfx |= RF_BEAM; 
		beam->s.modelindex = 1;
		beam->s.frame = 2;
		beam->s.skinnum = 0xf1f3f1f1;
		beam->think = G_FreeEdict;
		beam->nextthink = level.time + (1.1 * FRAMETIME);		// give it enough time to be seen

		VectorCopy(start, beam->s.origin);
        VectorSubtract(tr.endpos, start, vbeam);
		dist = 10.0 * 2000;
		VectorScale(aimdir, dist, beam->velocity);

		gi.linkentity(beam);
	}*/

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (start, MULTICAST_PHS);

//	A living player is a valid target.

	if (tr.ent->client && tr.ent->inuse && (tr.ent->health > 0))	
	{
		VectorSubtract(tr.endpos, start, vbeam);

//		If the target is currently using an AGM on us, cut our AGM beam and get damaged.

		if (tr.ent->client->agm_target && (tr.ent->client->agm_target == self))
		{
			damage = 20;
			if (self->client->quad_framenum > level.framenum)
				damage *= 4;
			T_Damage(self, tr.ent, tr.ent, vbeam, start, vec3_origin, damage, -damage, 0, MOD_AGM_FEEDBACK);
			self->client->agm_target = NULL;
			self->client->agm_charge = 0;
			gi.sound(self, CHAN_ITEM, SoundIndex("weapons/brain/Brnatck2.wav"), 1, ATTN_NORM, 0);
		}

//		The beam also cuts out if our target is currently being held by someone else's AGM, 
//		and we get damaged if the AGM wielder is not on our team.

		else if (tr.ent->held_by_agm)
		{
			if (!CheckTeamDamage(tr.ent->agm_enemy, self))
			{
				damage = 20;
				if (self->client->quad_framenum > level.framenum)
					damage *= 4;
				T_Damage(self, tr.ent->agm_enemy, tr.ent->agm_enemy, vbeam, start, vec3_origin, damage, -damage, 0, MOD_AGM_FEEDBACK);
			}
			self->client->agm_target = NULL;
			self->client->agm_charge = 0;
			gi.sound(self, CHAN_ITEM, SoundIndex("weapons/brain/Brnatck2.wav"), 1, ATTN_NORM, 0);
		}

		//			As long as they don't have the Invulnerability, or are on our team, they're ours to play with
		//			like a cheap toy.
		
		if (tr.ent->client->invincible_framenum > level.framenum)
			return;
		
		if (CheckTeamDamage(tr.ent, self))
		{
			self->client->agm_target = NULL;
			return;
		}
		
		self->client->agm_target = tr.ent;
		self->client->agm_range = VectorLength(vbeam);
		if ((self->client->agm_range < AGM_RANGE_MIN) || tr.startsolid)
			self->client->agm_range = AGM_RANGE_MIN;
		
		tr.ent->agm_enemy = self;
		tr.ent->held_by_agm = true;
		tr.ent->flung_by_agm = false;
		tr.ent->thrown_by_agm = false;
		
		gi.sound(self, CHAN_WEAPON, SoundIndex("brain/melee3.wav"), 1, ATTN_NORM, 0);
		gi.sound(tr.ent, CHAN_BODY, SoundIndex("brain/melee3.wav"), 1, ATTN_NORM, 0);
		safe_centerprintf(tr.ent, "You're being manipulated by %s\n", self->client->pers.netname);
	}
}


/*
======================================================================
ANTI-GRAV MANIPULATOR
======================================================================
*/
void AGM_Reset(edict_t *self)
{
	self->client->agm_on = false;
	self->client->agm_push = false;
	self->client->agm_pull = false;

	if (self->client->agm_target != NULL)
	{
		self->client->agm_target->held_by_agm = false;
		self->client->agm_target->flung_by_agm = false;
		self->client->agm_target->thrown_by_agm = true;
		self->client->agm_target = NULL;
	}
}

void Weapon_AGM_Fire(edict_t *self)
{
	vec3_t		start;
	vec3_t		forward;
	vec3_t		right;
	vec3_t		offset;

	if (!(self->client->buttons & BUTTON_ATTACK))
	{
		self->client->weaponstate = WEAPON_READY;
		return;
	}

//	Once the AGM has tripped off, it can't be fired again until it has recharged to 100.

	if (self->client->agm_tripped && (self->client->agm_target == NULL))
	{
		gi.sound(self, CHAN_WEAPON, SoundIndex("floater/Fltatck1.wav"), 1, ATTN_STATIC, 0);
		return;
	}

//	Wrap round animation to the start of the firing set if the fire button is still being pressed.

	self->client->ps.gunframe++;
	if ((self->client->ps.gunframe == 21) && self->client->pers.inventory[self->client->ammo_index])
	{
		if (self->client->buttons & BUTTON_ATTACK)
			self->client->ps.gunframe = 6;
		else
			return;
	}

//	Check if there's enough ammo; swap weapons if we're dry.

	if (self->client->pers.inventory[self->client->ammo_index] < 1)
	{
		self->client->ps.gunframe = 21;
		if (level.time >= self->pain_debounce_time)
		{
			gi.sound(self, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			self->pain_debounce_time = level.time + 1.0;
		}
		AGM_Reset(self);
		NoAmmoWeaponChange(self);
		return;
	}

//	Set beam start position.
	
	AngleVectors(self->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24.0, 7.0, self->viewheight-6.0);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start, 0);

//	Fire!

	if (self->client->agm_target != NULL)
		Move_AGM(self, start, forward);
	else
	{
		if (!self->client->agm_on)
		{
			self->client->agm_on = true;
			if ((!((int)dmflags->value & DF_INFINITE_AMMO)))
				self->client->pers.inventory[self->client->ammo_index] -= 10;
		}
		Fire_AGM(self, start, forward);
		gi.sound(self, CHAN_WEAPON, SoundIndex("medic/medatck1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(self, start, PNOISE_WEAPON);
	}

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		self->client->pers.inventory[self->client->ammo_index] -= 1;

//		The Haste powerup/Tech effect needs to be applied separately due to the way that
//		the secondary mode is implemented.

		if (CTFApplyHaste(self))
		{
			self->client->pers.inventory[self->client->ammo_index] -= 1;
			CTFApplyHasteSound(self);
		}

		if (self->client->pers.inventory[self->client->ammo_index] < 0)
			self->client->pers.inventory[self->client->ammo_index] = 0;
	}

	if (self->client->pers.inventory[self->client->ammo_index] == 0)
		return;

//	Spend extra ammo if we're manipulating a player who has the Invulnerability.

	if ((self->client->agm_target != NULL) && (self->client->agm_target->client->invincible_framenum > level.framenum))
	{
		self->client->pers.inventory[self->client->ammo_index] -= 1;
		if (self->client->pers.inventory[self->client->ammo_index] < 0)
			self->client->pers.inventory[self->client->ammo_index] = 0;
	}

//	Decrease AGM charge if there's no target.

	if ((self->client->agm_target == NULL))
	{
		self->client->agm_charge -= (is_quad?2:1) * 10;
		if (self->client->agm_charge <= 0)
		{
			self->client->agm_charge = 0;
			if (!self->client->agm_tripped)
			{
				self->client->agm_tripped = true;
				AGM_Reset(self);
			}
		}
	}
}

void Weapon_AGM(edict_t *self)
{
	static int	pause_frames[]	= {21, 43, 49, 0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};
	static gitem_t *tech = NULL;

	if (!self->client->agm_showcharge)
		self->client->agm_showcharge = true;

//	Implement the effects of the '+push' and '+pull' commands.

	//if (self->client->agm_pull)
	//{
		self->client->agm_range -= 100;
		if (self->client->agm_range < AGM_RANGE_MIN)
			self->client->agm_range = AGM_RANGE_MIN;
	//}
	else if (self->client->agm_push)
	{
		self->client->agm_range += AGM_RANGE_DELTA;
		if (self->client->agm_range > AGM_RANGE_MAX)
			self->client->agm_range = AGM_RANGE_MAX;
	}

//	If the player has stopped firing and they had an AGM target, flag that target as being thrown.

	if (self->client->agm_on && !(self->client->buttons & BUTTON_ATTACK))
	{
		if (self->client->agm_target)
			Cmd_AGMFling_f (self);
		AGM_Reset(self);
		self->client->ps.gunframe = 21;

//		Play Quad or Tech sounds, if appropriate.

		if (!tech)
			tech = FindItemByClassname("item_tech2");

		if (tech && self->client && self->client->pers.inventory[ITEM_INDEX(tech)])
		{
			if (self->client->ctf_techsndtime < level.time)
			{
				self->client->ctf_techsndtime = level.time + 1.0;
				if (self->client->quad_framenum > level.framenum)
					gi.sound(self, CHAN_VOICE, SoundIndex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound(self, CHAN_VOICE, SoundIndex("ctf/tech2.wav"), 1, ATTN_NORM, 0);
			}
		}
		else if (self->client->quad_framenum > level.framenum)
			gi.sound(self, CHAN_ITEM, SoundIndex("items/damage3.wav"), 1, ATTN_NORM, 0);
	}

//	Sanity check: if we have an AGM target and they've just died, but for some reason our agm_target
//	pointer hasn't been reset, then do it manually here.

	if ((self->client->agm_target != NULL) && (self->client->agm_target->health < 1))
	{
		self->client->agm_on = false;
		self->client->agm_target->flung_by_agm = false;
		self->client->agm_target->thrown_by_agm = false;
		self->client->agm_target = NULL;
	}

//	Charge up the AGM if it's not firing.

	if (!self->client->agm_on && (self->client->agm_charge < 100))
	{
		self->client->agm_charge += 5;
		if (self->client->agm_charge >= 100)
		{
			self->client->agm_charge = 100;
			if (self->client->agm_tripped)
			{
				self->client->agm_tripped = false;
				gi.sound(self, CHAN_VOICE, SoundIndex("world/fusein.wav"), 1, ATTN_STATIC, 0);
			}
		}
	}

//	Weapon actions.

	Weapon_Generic(self, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_AGM_Fire);
}
