// g_turret.c

#include "g_local.h"
/*-----/ PM /-----/ NEW:  Include new header files. /-----*/
#include "w_fire.h"
/*--------------------------------------------------------*/


void AnglesNormalize(vec3_t vec)
{
	while(vec[0] > 360)
		vec[0] -= 360;
	while(vec[0] < 0)
		vec[0] += 360;
	while(vec[1] > 360)
		vec[1] -= 360;
	while(vec[1] < 0)
		vec[1] += 360;
}

float SnapToEights(float x)
{
	x *= 8.0;
	if (x > 0.0)
		x += 0.5;
	else
		x -= 0.5;
	return 0.125 * (int)x;
}


void turret_blocked(edict_t *self, edict_t *other)
{
	edict_t	*attacker;

	if (other->takedamage)
	{
		if (self->teammaster->owner)
			attacker = self->teammaster->owner;
		else
			attacker = self->teammaster;
		T_Damage (other, self, attacker, vec3_origin, other->s.origin, vec3_origin, self->teammaster->dmg, 10, 0, MOD_CRUSH);
	}
}

/*QUAKED turret_breach (0 0 0) ?
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

"speed"		default 50
"dmg"		default 10
"angle"		point this forward
"target"	point this at an info_notnull at the muzzle tip
"minpitch"	min acceptable pitch angle : default -30
"maxpitch"	max acceptable pitch angle : default 30
"minyaw"	min acceptable yaw angle   : default 0
"maxyaw"	max acceptable yaw angle   : default 360
*/
//Wyrm: defined in W_weapon.c
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, vec3_t up, vec3_t right, int damage, int speed);

void turret_breach_fire (edict_t *self)
{
	vec3_t	f, r, u;
	vec3_t	start;
	int		damage;
	int		speed;

	AngleVectors (self->s.angles, f, r, u);
	VectorMA (self->s.origin, self->move_origin[0], f, start);
	VectorMA (start, self->move_origin[1], r, start);
	VectorMA (start, self->move_origin[2], u, start);

	damage = 100 + random() * 50;
	speed = 550 + 50 * skill->value;
        if (self->teammaster->count == TURRET_ROCKET)
        {
                fire_rocket (self->teammaster->owner, start, f, self->teammaster->health, self->teammaster->random, self->teammaster->health + 40, self->teammaster->health);
                gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
        }
        else if (self->teammaster->count == TURRET_CHAINGUN)
        {
                //grey particles
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_SPLASH);
                gi.WriteByte (8);
                gi.WritePosition (start);
                gi.WriteDir (f);
                gi.WriteByte (7);
                gi.multicast (start, MULTICAST_PVS);

                fire_bullet (self->teammaster->owner, start, f, self->teammaster->health, 0,  DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
                gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/machgf1b.wav"), 1, ATTN_NORM, 0);
        }
        else if (self->teammaster->count == TURRET_BLASTER)
        {
                fire_blaster (self->teammaster->owner, start, f, self->teammaster->health, self->teammaster->random, EF_HYPERBLASTER, true);
                gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);
        }
        else if (self->teammaster->count == TURRET_RAILGUN)
        {
                fire_rail (self->teammaster->owner, start, f, self->teammaster->health, 0);
                gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/railgf1a.wav"), 1, ATTN_NORM, 0);
        }
        else if (self->teammaster->count == TURRET_PLASMA)
        {
                fire_plasma (self->teammaster->owner, start, f, u, r, self->teammaster->health, self->teammaster->random);
                gi.positioned_sound (start, self, CHAN_WEAPON, gi.soundindex("weapons/plasma.wav"), 1, ATTN_NORM, 0);
        }
}

void turret_client_check (edict_t *self)
{
        edict_t *ent;

        vec3_t  target, forward;

        AnglesNormalize(self->move_angles);

        // FIXME: do a tracebox from up and behind towards the turret, to try and keep them from
        // getting stuck inside the rotating turret
        // x & y
        AngleVectors(self->s.angles, forward, NULL, NULL);
        VectorScale(forward, 40, forward);
        VectorSubtract(self->s.origin, forward, target);

        VectorCopy(self->s.origin, target);

                        VectorSubtract(target, forward, target);
        //Wyrm--> stuck into the turret...
        VectorClear(self->owner->velocity);
        VectorCopy(target, self->owner->s.origin);

        // has the player abondoned the turret?
        // use button disables turret

        if (self->owner->client->onturret > 2)
        {
                // level the gun
                self->move_angles[0] = 0;

                ent = self->owner;

                // throw them back from turret
                AngleVectors(self->s.angles, forward, NULL, NULL);
                VectorScale(forward, -300, forward);

                forward[2] = 150;
                VectorCopy(forward, ent->velocity);
                        
                ent->s.origin[2] += 5;
                ent->movetype = MOVETYPE_WALK;
                ent->gravity = 1;

                if (!ent->client->chasetoggle && !ent->client->missile)
                {
                        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
                        ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                }

                gi.linkentity (ent);

                self->owner->client->onturret = 0;

                self->owner = NULL;
        }
}

void turret_breach_think (edict_t *self)
{
	edict_t	*ent;
	vec3_t	current_angles;
	vec3_t	delta;
        int i;

	VectorCopy (self->s.angles, current_angles);
	AnglesNormalize(current_angles);

	AnglesNormalize(self->move_angles);
	if (self->move_angles[PITCH] > 180)
		self->move_angles[PITCH] -= 360;

	// clamp angles to mins & maxs
	if (self->move_angles[PITCH] > self->pos1[PITCH])
		self->move_angles[PITCH] = self->pos1[PITCH];
	else if (self->move_angles[PITCH] < self->pos2[PITCH])
		self->move_angles[PITCH] = self->pos2[PITCH];

	if ((self->move_angles[YAW] < self->pos1[YAW]) || (self->move_angles[YAW] > self->pos2[YAW]))
	{
		float	dmin, dmax;

		dmin = fabs(self->pos1[YAW] - self->move_angles[YAW]);
		if (dmin < -180)
			dmin += 360;
		else if (dmin > 180)
			dmin -= 360;
		dmax = fabs(self->pos2[YAW] - self->move_angles[YAW]);
		if (dmax < -180)
			dmax += 360;
		else if (dmax > 180)
			dmax -= 360;
		if (fabs(dmin) < fabs(dmax))
			self->move_angles[YAW] = self->pos1[YAW];
		else
			self->move_angles[YAW] = self->pos2[YAW];
	}

	VectorSubtract (self->move_angles, current_angles, delta);
	if (delta[0] < -180)
		delta[0] += 360;
	else if (delta[0] > 180)
		delta[0] -= 360;
	if (delta[1] < -180)
		delta[1] += 360;
	else if (delta[1] > 180)
		delta[1] -= 360;
	delta[2] = 0;

	if (delta[0] > self->speed * FRAMETIME)
		delta[0] = self->speed * FRAMETIME;
	if (delta[0] < -1 * self->speed * FRAMETIME)
		delta[0] = -1 * self->speed * FRAMETIME;
	if (delta[1] > self->speed * FRAMETIME)
		delta[1] = self->speed * FRAMETIME;
	if (delta[1] < -1 * self->speed * FRAMETIME)
		delta[1] = -1 * self->speed * FRAMETIME;

	VectorScale (delta, 1.0/FRAMETIME, self->avelocity);

	self->nextthink = level.time + FRAMETIME;

	for (ent = self->teammaster; ent; ent = ent->teamchain)
		ent->avelocity[1] = self->avelocity[1];

	// if we have adriver, adjust his velocities
	if (self->owner)
	{
                if (self->owner->client) //Wyrm: a player is mounting the turret
                {
			for (i=0; i<3; i++)
				self->move_angles[i] = self->owner->client->v_angle[i];

			// should the turret shoot now?
                        if ((self->owner->client->buttons & BUTTON_ATTACK) && (self->delay < level.time))  // 3 second break between shots
			{
				turret_breach_fire (self);
                                //Wyrm modified cadency
                                self->delay = level.time + self->wait;
			}
                }
                else
                {
                        float   angle;
                        float   target_z;
                        float   diff;
                        vec3_t  target;
                        vec3_t  dir;

                        // angular is easy, just copy ours
                        self->owner->avelocity[0] = self->avelocity[0];
                        self->owner->avelocity[1] = self->avelocity[1];

                        // x & y
                        angle = self->s.angles[1] + self->owner->move_origin[1];
                        angle *= (M_PI*2 / 360);
                        target[0] = SnapToEights(self->s.origin[0] + cos(angle) * self->owner->move_origin[0]);
                        target[1] = SnapToEights(self->s.origin[1] + sin(angle) * self->owner->move_origin[0]);
                        target[2] = self->owner->s.origin[2];

                        VectorSubtract (target, self->owner->s.origin, dir);
                        self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
                        self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;

                        // z
                        angle = self->s.angles[PITCH] * (M_PI*2 / 360);
                        target_z = SnapToEights(self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2]);

                        diff = target_z - self->owner->s.origin[2];
                        self->owner->velocity[2] = diff * 1.0 / FRAMETIME;

                        if (self->spawnflags & 65536)
                        {
                                turret_breach_fire (self);
                                self->spawnflags &= ~65536;
                        }
                }
	}
        else
        {  // Wyrm: new! check if a player has mounted the turret

		// find a player
		int		i;
		edict_t	*ent;
		vec3_t	target, forward;
		vec3_t	dir;

		ent = &g_edicts[0];
		ent++;
		for (i=0 ; i<maxclients->value ; i++, ent++)
		{

			if (!ent->inuse)
				continue;

                        //not observers or gibs!
                        if (ent->client->ps.pmove.pm_type != PM_NORMAL)
                                continue;

			// determine distance from turret seat location

			// x & y
			AngleVectors(self->s.angles, forward, NULL, NULL);
			VectorScale(forward, 32, forward);
			VectorSubtract(self->s.origin, forward, target);

			VectorSubtract (target, ent->s.origin, dir);
			if (fabs(dir[2]) < 64)
				dir[2] = 0;

                        if (VectorLength(dir) < 16)
			{	
				self->owner = ent;

                                //disable jet!
                                ent->client->Jet_framenum = 0;

                                ent->client->turret=self;

                                ent->movetype = MOVETYPE_PUSH;

                                VectorClear(ent->velocity);
                                ent->gravity = 1;

                                ent->client->weaponstate = WEAPON_READY;
                                ent->client->weapon_sound = 0;

                                ent->client->ps.gunindex = 0;

                                ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION; // this turns off Quake2's inclination to predict where the camera is going,

				// turn off client side prediction for this player
				//gi.WriteByte(11);				// 11 = svc_stufftext
				//gi.WriteString("cl_predict 0\n");
				//gi.unicast(ent, 1);

				gi.linkentity(ent);

                                gi.centerprintf(ent, "Turret mounted\n\nPress FIRE to fire, JUMP to abandon");

				ent->client->onturret = 1;

			}

		}
	}
}

void turret_breach_finish_init (edict_t *self)
{
	// get and save info for muzzle location
	if (!self->target)
	{
		gi.dprintf("%s at %s needs a target\n", self->classname, vtos(self->s.origin));
	}
	else
	{
		self->target_ent = G_PickTarget (self->target);
		VectorSubtract (self->target_ent->s.origin, self->s.origin, self->move_origin);
		G_FreeEdict(self->target_ent);
	}

	self->teammaster->dmg = self->dmg;
	self->think = turret_breach_think;
	self->think (self);
}

void SP_turret_breach (edict_t *self)
{
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (!self->speed)
		self->speed = 50;
	if (!self->dmg)
		self->dmg = 10;

	if (!st.minpitch)
		st.minpitch = -30;
	if (!st.maxpitch)
		st.maxpitch = 30;
	if (!st.maxyaw)
		st.maxyaw = 360;

//Wyrm: modified cadency
        if (!self->wait)
                self->wait = 1;
//Wyrm: missile damage
        if (!self->health) self->health = 110;
//Wyrm: missile speed
        if (!self->random) self->random = 800;

//Wyrm : We won't check the count field

	self->pos1[PITCH] = -1 * st.minpitch;
	self->pos1[YAW]   = st.minyaw;
	self->pos2[PITCH] = -1 * st.maxpitch;
	self->pos2[YAW]   = st.maxyaw;

	self->ideal_yaw = self->s.angles[YAW];
	self->move_angles[YAW] = self->ideal_yaw;

	self->blocked = turret_blocked;

	self->think = turret_breach_finish_init;
	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);
}


/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.
*/

void SP_turret_base (edict_t *self)
{
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	self->blocked = turret_blocked;
	gi.linkentity (self);
}


/*QUAKED turret_driver (1 .5 0) (-16 -16 -24) (16 16 32)
Must NOT be on the team with the rest of the turret parts.
Instead it must target the turret_breach.
*/

void infantry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void infantry_stand (edict_t *self);
void monster_use (edict_t *self, edict_t *other, edict_t *activator);

void turret_driver_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t	*ent;

	// level the gun
	self->target_ent->move_angles[0] = 0;

	// remove the driver from the end of them team chain
	for (ent = self->target_ent->teammaster; ent->teamchain != self; ent = ent->teamchain)
		;
	ent->teamchain = NULL;
	self->teammaster = NULL;
	self->flags &= ~FL_TEAMSLAVE;

	self->target_ent->owner = NULL;
	self->target_ent->teammaster->owner = NULL;

	infantry_die (self, inflictor, attacker, damage, point);
}

qboolean FindTarget (edict_t *self);

void turret_driver_think (edict_t *self)
{
	vec3_t	target;
	vec3_t	dir;

	self->nextthink = level.time + FRAMETIME;

	if (self->enemy && (!self->enemy->inuse || self->enemy->health <= 0))
		self->enemy = NULL;

	if (!self->enemy)
	{
		if (!FindTarget (self))
			return;
		self->monsterinfo.trail_time = level.time;
		self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
	}
	else
	{
		if (visible (self, self->enemy))
		{
			if (self->monsterinfo.aiflags & AI_LOST_SIGHT)
			{
				self->monsterinfo.trail_time = level.time;
				self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
			}
		}
		else
		{
			self->monsterinfo.aiflags |= AI_LOST_SIGHT;
			return;
		}
	}

	// let the turret know where we want it to aim
	VectorCopy (self->enemy->s.origin, target);
	target[2] += self->enemy->viewheight;
	VectorSubtract (target, self->target_ent->s.origin, dir);
	vectoangles (dir, self->target_ent->move_angles);

	// decide if we should shoot
	if (level.time < self->monsterinfo.attack_finished)
		return;

        //Wyrm-->in sp, use as normal...
        self->monsterinfo.attack_finished = level.time + self->target_ent->teammaster->wait;

	//FIXME how do we really want to pass this along?
	self->target_ent->spawnflags |= 65536;
}

void turret_driver_link (edict_t *self)
{
	vec3_t	vec;
	edict_t	*ent;

	self->think = turret_driver_think;
	self->nextthink = level.time + FRAMETIME;

	self->target_ent = G_PickTarget (self->target);
	self->target_ent->owner = self;
	self->target_ent->teammaster->owner = self;
	VectorCopy (self->target_ent->s.angles, self->s.angles);

	vec[0] = self->target_ent->s.origin[0] - self->s.origin[0];
	vec[1] = self->target_ent->s.origin[1] - self->s.origin[1];
	vec[2] = 0;
	self->move_origin[0] = VectorLength(vec);

	VectorSubtract (self->s.origin, self->target_ent->s.origin, vec);
	vectoangles (vec, vec);
	AnglesNormalize(vec);
	self->move_origin[1] = vec[1];

	self->move_origin[2] = self->s.origin[2] - self->target_ent->s.origin[2];

	// add the driver to the end of them team chain
	for (ent = self->target_ent->teammaster; ent->teamchain; ent = ent->teamchain)
		;
	ent->teamchain = self;
	self->teammaster = self->target_ent->teammaster;
	self->flags |= FL_TEAMSLAVE;
}

void SP_turret_driver (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	self->health = 100;
	self->gib_health = 0;
	self->mass = 200;
	self->viewheight = 24;
/*-----/ PM /-----/ NEW:  Set fireflags. /-----*/
        self->fireflags = FIREFLAG_DELTA_BASE | FIREFLAG_IGNITE;
/*---------------------------------------------*/

	self->die = turret_driver_die;
	self->monsterinfo.stand = infantry_stand;

	self->flags |= FL_NO_KNOCKBACK;

	level.total_monsters++;

	self->svflags |= SVF_MONSTER;
        self->monsterinfo.spawn = NULL;

	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->use = monster_use;
	self->clipmask = MASK_MONSTERSOLID;
	VectorCopy (self->s.origin, self->s.old_origin);
	self->monsterinfo.aiflags |= AI_STAND_GROUND|AI_DUCKED;

	if (st.item)
	{
		self->item = FindItemByClassname (st.item);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	self->think = turret_driver_link;
	self->nextthink = level.time + FRAMETIME;

	gi.linkentity (self);
}
