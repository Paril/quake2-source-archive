#include "g_local.h"

void Building_die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void Building_Use (edict_t *ent, edict_t *other, edict_t *activator);
void Building_think (edict_t *ent);
void Building_donebuild (edict_t *ent);

field_t dl_bldflds[] =
{
	{"name",				BLDOFS(name),				F_LSTRING},
	{"type",				BLDOFS(type),				F_INT},
	{"subtype",				BLDOFS(subtype),			F_INT},
	{"flags",				BLDOFS(flags),				F_INT},
	{"use_count",			BLDOFS(use_count),			F_INT},
	{"weight",				BLDOFS(weight),				F_FLOAT},
	{"build_time",			BLDOFS(build_time),			F_FLOAT},
	{"req_tools",			BLDOFS(req_tools),			F_INT},
	{"min_points",			BLDOFS(min_points),			F_INT},
	{"max_points",			BLDOFS(max_points),			F_INT},
	{"duration",			BLDOFS(duration),			F_FLOAT},
	{"wait",				BLDOFS(duration),			F_FLOAT},
	{"min_dist",			BLDOFS(min_dist),			F_INT},
	{"max_dist",			BLDOFS(max_dist),			F_INT},
	{"health",				BLDOFS(health),				F_INT},
	{"range",				BLDOFS(range),				F_INT},
	{"fov",					BLDOFS(fov),				F_INT},
	{"damage",				BLDOFS(damage),				F_INT},
	{"model",				BLDOFS(model),				F_LSTRING},
	{"mins",				BLDOFS(mins),				F_VECTOR},
	{"maxs",				BLDOFS(maxs),				F_VECTOR},
	{"skin",				BLDOFS(skin),				F_INT},
	{"frame_build",			BLDOFS(frame_build),		F_INT},
	{"frame_build_num",		BLDOFS(frame_build_num),	F_INT},
	{"frame_idle",			BLDOFS(frame_idle),			F_INT},
	{"frame_idle_num",		BLDOFS(frame_idle_num),		F_INT},
	{"frame_action",		BLDOFS(frame_action),		F_INT},
	{"frame_action_num",	BLDOFS(frame_action_num),	F_INT},
	{"frame_die",			BLDOFS(frame_die),			F_INT},
	{"frame_die_num",		BLDOFS(frame_die_num),		F_INT},
	{"weapon",				BLDOFS(weapon[0].name),		F_LSTRING,		0,	MAX_BLDG_WEAPONS,	sizeof(buildings[0].weapon[0])},
	{"weapon_offset",		BLDOFS(weapon[0].offset),	F_VECTOR,		0,	MAX_BLDG_WEAPONS,	sizeof(buildings[0].weapon[0])},
	{"weapon_ammo",			BLDOFS(weapon[0].ammo),		F_LSTRING,		0,	MAX_BLDG_WEAPONS,	sizeof(buildings[0].weapon[0])},
	{"attach",				BLDOFS(attach[0].type),		F_INT,			0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_offset",		BLDOFS(attach[0].offset),	F_VECTOR,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_angles",		BLDOFS(attach[0].angles),	F_ANGLE,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_rotate_min",	BLDOFS(attach[0].min),		F_ANGLE,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_rotate_max",	BLDOFS(attach[0].max),		F_ANGLE,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_rotate_speed",	BLDOFS(attach[0].speed),	F_VECTOR,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"attach_track_speed",	BLDOFS(attach[0].trackspeed),F_VECTOR,		0,	MAX_BLDG_ATTACH,	sizeof(buildings[0].attach[0])},
	{"trigger",				BLDOFS(trigger),	F_LSTRING},
	{"trigger_offset",		BLDOFS(trigger_offset),		F_VECTOR},
	{"trigger_angles",		BLDOFS(trigger_angles),		F_ANGLE},
	{"s_start",				BLDOFS(s_start),	F_LSTRING},
	{"s_done",				BLDOFS(s_done),		F_LSTRING},
	{"s_idle",				BLDOFS(s_idle),		F_LSTRING},
	{"s_idle_time",			BLDOFS(s_idle_time),F_FLOAT},
	{"s_die",				BLDOFS(s_die),		F_LSTRING},
	{"s_use",				BLDOFS(s_use),		F_LSTRING},
	{"effect",				BLDOFS(effect),		F_LSTRING},
	{"upgrade",				BLDOFS(upgrade),	F_LSTRING},
	{"inventory_pic",		BLDOFS(pic_s),		F_LSTRING},
	{"statusbar_pic",		BLDOFS(pic_b),		F_LSTRING},
	{"category",			BLDOFS(category),	F_LSTRING},
	{NULL, 0, F_INT}
};
building_t buildings[MAX_BUILDINGS];

building_t	*GetBuildingByName(const char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_BUILDINGS; i++) {
			if (buildings[i].name)
				if (!DL_strcmp(buildings[i].name, name, -1, false))
					return &buildings[i];
		}
	}
	return NULL;
}


//
// Laser-Gate - Basically a device, when used in pairs, creates upto
// 4 laser-beams at each weaponX_offset point - activating toggles on/off
// Laser-Gates will generate lasers between ALL other laser-gates of the
// same type, within the current Laser-Gates field of view and range, 
// IF the Laser-Gate is ALSO within the field-of-view of the target Laser-Gate.
//
// ent->state is the current state, ent->spawnflags is the default state
//

void RemoveBeam(edict_t *self) {
edict_t *g;

	// remove self, and still retain beam-list	
	if (self->owner) {
		if (self->owner->chain_next == self) {
			self->owner->chain_next = self->chain_next;
		} else {
			for (g = self->owner->chain_next; g && (g->chain_next != self); g = g->chain_next);
			if (g)
				g->chain_next = self->chain_next;
		}
	}
	G_FreeEdict(self);
}
	

void LaserGate_Off(edict_t *ent, float time);
void LaserGate_use(edict_t *ent, edict_t *other, edict_t *activator);

void LaserGate_LaserThink(edict_t *self) {
edict_t	*ignore;
vec3_t	start, end, offs;
trace_t	tr;

	if (!self->owner || !self->goalitem || !self->owner->building || !self->goalitem->building) {
		if (self->owner && self->owner->building)
			LaserGate_Off(self->owner, 0.0);
		if (self->goalitem && self->goalitem->building)
			LaserGate_Off(self->goalitem, 0.0);
		RemoveBeam(self);
		return;
	}

	if ((self->owner->state != BLDG_STATE_ACTIVATED) || (self->goalitem->state != BLDG_STATE_ACTIVATED)) {
		RemoveBeam(self);
		return;
	}

	VectorRotate(self->pos1, self->owner->s.angles, offs);
	VectorAdd(self->owner->s.origin, offs, self->s.origin);

	VectorRotate(self->pos2, self->goalitem->s.angles, offs);
	VectorAdd(self->goalitem->s.origin, offs, self->offset);

	ignore = self;
	VectorCopy (self->s.origin, start);
	VectorCopy (self->offset, end);
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		if (tr.ent == self->goalitem)
			break;

		// If a teammate, shut off lasers
		if (tr.ent->team == self->team) {	// use tr.ent->team so that spies can go through as well.
			VectorCopy (tr.endpos, self->s.old_origin);
			self->nextthink = level.time + FRAMETIME;

			LaserGate_use(self->owner, self, tr.ent);
			return;
		}

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
			T_Damage (tr.ent, self, self, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER, NULL);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			break;

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}

	VectorCopy (tr.endpos, self->s.old_origin);
	self->nextthink = level.time + FRAMETIME;
}

void LaserGate_LaserSpawn(edict_t *owner, edict_t *targ, int cnt, int skin, float time) {
edict_t *beam = NULL;
vec3_t	offs1, offs2, pos;
trace_t	tr;

	if (!owner->building || !targ->building)
		return;

	if (owner->state < BLDG_STATE_ONLINE)
		return;

	// Look for en existing beam between these points
	while (beam = G_Find(beam, FOFS(classname), "gate_beam")) {
		if ( ( ((beam->owner == owner) && (beam->goalitem == targ)) ||
			   ((beam->goalitem == owner) && (beam->owner == targ)) ) &&
			   (beam->count == cnt)) {

			// Setup state for master/slaves
			owner->state = BLDG_STATE_ACTIVATED;
			owner->s.sound = targ->s.sound = gi.soundindex(owner->building->s_use);
			if (!time) owner->spawnflags = targ->spawnflags = BLDG_STATE_ACTIVATED;
			else owner->timestamp = targ->timestamp = level.time + time;
			// =============================

			return;
		}
	}

	beam = G_Spawn();

	VectorCopy(owner->building->weapon[cnt].offset, pos);
	VectorCopy(pos, beam->pos1);
	VectorRotate(pos, owner->s.angles, offs1);
	VectorAdd(owner->s.origin, offs1, beam->s.origin);

	VectorCopy(pos, beam->pos2);
	VectorRotate(pos, targ->s.angles, offs2);
	VectorAdd(targ->s.origin, offs2, beam->offset);

	VectorSet (beam->mins, -8, -8, -8);
	VectorSet (beam->maxs,  8,  8,  8);

	tr = gi.trace(beam->s.origin, NULL, NULL, beam->offset, owner, MASK_OPAQUE);
	if ((tr.fraction != 1.0) && (tr.ent != targ)) {
		G_FreeEdict(beam);
		return;
	}

	beam->classname =		"gate_beam";
	beam->owner =			owner;
	beam->goalitem =		targ;
	beam->team =			owner->team;
	beam->count =			cnt;
	beam->svflags &=		~SVF_NOCLIENT;
	beam->movetype =		MOVETYPE_NONE;
	beam->solid	=			SOLID_NOT;
	beam->s.renderfx |=		RF_BEAM|RF_TRANSLUCENT;
	beam->s.modelindex =	1;			// must be non-zero
	beam->s.frame =			4;			// small diameter
	beam->s.skinnum =		skin;		// set the color
	beam->dmg = 1;
	if (owner->building->damage)
		beam->dmg = owner->building->damage;

	beam->think = LaserGate_LaserThink;
	beam->nextthink = level.time + FRAMETIME;

	gi.linkentity (beam);

	// Setup state for master/slaves
	owner->state = BLDG_STATE_ACTIVATED;
	owner->s.sound = targ->s.sound = gi.soundindex(owner->building->s_use);
	if (!time) owner->spawnflags = targ->spawnflags = BLDG_STATE_ACTIVATED;
	else owner->timestamp = targ->timestamp = level.time + time;	
	// =============================
}

void LaserGate_Off(edict_t *ent, float time) {
edict_t *g=NULL;
	
	// remove all beams which are owned or targets by this gate
	while (g = G_Find(g, FOFS(classname), "gate_beam")) {
		if ( (g->owner == ent) || (g->goalitem == ent) ) {

			// Shutdown any other gates which link to this one
			g->owner->state = g->goalitem->state = BLDG_STATE_ONLINE;
			g->owner->s.sound = g->goalitem->s.sound = 0;
			if (!time) g->owner->spawnflags = g->goalitem->spawnflags = BLDG_STATE_ONLINE;
			else g->owner->timestamp = g->goalitem->timestamp = level.time + time;
			G_FreeEdict(g);
		}
	}
}

void LaserGate_On(edict_t *ent, edict_t *slave, float time) {
	int	i;

	for (i = 0; i < MAX_BLDG_WEAPONS; i++)
		if (ent->building->weapon[i].offset[0] || ent->building->weapon[i].offset[1] || ent->building->weapon[i].offset[2])
			LaserGate_LaserSpawn(ent, slave, i, ent->building->subtype, time);
}

void LaserGate_SyncVisibles(edict_t *p, int state, float time) {
edict_t *g=NULL;
vec3_t	diff;
float	dist;
int rangeToChild, rangeToParent, parentRange, childRange, numSynced=0;


	if (p->state < BLDG_STATE_ONLINE)
		return;

	p->state = state;	
	while (g = G_Find(g, FOFS(classname), "laser_gate")) {
		if (g->building != p->building)
			continue;

		if (g == p)
			continue;

		if (g->state == state) {
			numSynced++;
			continue;
		}

		VectorSubtract(p->s.origin, g->s.origin, diff);
		dist = VectorLength(diff);

		parentRange = p->building->range ? p->building->range : ((4500/p->building->fov)*50) / 4.0;
		childRange = g->building->range ? g->building->range : ((4500/g->building->fov)*50) / 4.0;

		rangeToChild = InFOV(p->s.origin, p->s.angles, g->s.origin, p->building->fov, false);
		rangeToParent = InFOV(g->s.origin, g->s.angles, p->s.origin, g->building->fov, false);

		if ((rangeToChild < parentRange) && (rangeToParent < childRange)) { 
			if (g->state != state) {
				if (state == BLDG_STATE_ACTIVATED) LaserGate_On(p, g, time);
				else LaserGate_Off(g, time);
				p->spawnflags = state;
			}
			LaserGate_SyncVisibles(g, state, time);
			numSynced++;
		}
	}
	// If its unable to sync with any others, don't keep trying.
	if (!numSynced)
		p->state = p->spawnflags = BLDG_STATE_ONLINE;
}

void LaserGate_think(edict_t *ent) {
edict_t *g = NULL;
int		active = false;
float	pHealth;

	while (g = G_Find(g, FOFS(classname), "gate_beam")) {
		if ((g->owner == ent) || (g->goalitem == ent)) {
			g->svflags &= ~SVF_NOCLIENT;

			pHealth = ((float)ent->health / (float)ent->building->min_points / 5.0) + 0.8;
			if (random() > pHealth)
				g->svflags |= SVF_NOCLIENT;
			active = true;
		}
	}
	if (!active && (ent->state == BLDG_STATE_ACTIVATED))
		LaserGate_SyncVisibles(ent, BLDG_STATE_ONLINE, ent->building->duration);
	else if ((ent->state != ent->spawnflags) && (level.time > ent->timestamp))
		LaserGate_SyncVisibles(ent, ent->spawnflags, 0.0);
	
	Building_think(ent);
	ent->nextthink = level.time + FRAMETIME;
}

void LaserGate_use(edict_t *ent, edict_t *other, edict_t *activator) {
float duration;
int state;

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	Building_Use(ent, other, activator);

	// Other state changes/rules?  What if a state goes from ACTIVATED to OFFLINE, or from OFFLINE to ACTIVATED?
	if (activator == other) {
		if (ent->spawnflags == BLDG_STATE_ONLINE) ent->spawnflags = BLDG_STATE_ACTIVATED;
		else if (ent->spawnflags == BLDG_STATE_ACTIVATED) ent->spawnflags = BLDG_STATE_ONLINE;
		state = ent->spawnflags;
		duration = 0.0;
	} else {
		if (ent->spawnflags == BLDG_STATE_ONLINE) ent->state = BLDG_STATE_ACTIVATED;
		else if (ent->spawnflags == BLDG_STATE_ACTIVATED) ent->state = BLDG_STATE_ONLINE;
		state = ent->state;
		duration = ent->building->duration;
	}
	LaserGate_SyncVisibles(ent, state, duration);
}

void LaserGate_create(edict_t *ent) {
char name[128];

	ent->classname = "laser_gate";
	ent->use = LaserGate_use;
	ent->die = Building_die;
	ent->think = LaserGate_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->spawnflags = BLDG_STATE_ONLINE;

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = DL_strcpy(NULL, name, -1);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_LASERGATE, ent, ent->master, name, ent->building->name, true);
}

//
// EfxBox - simple device that activates an effect when used.
// EfxBox must be attached, but can bet attached to almost anything.
//
void EfxBox_use(edict_t *ent, edict_t *other, edict_t *activator) {
vec3_t	forward, pos;
edict_t *user;

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	Building_Use(ent, other, activator);

	if (ent->timestamp > level.time)
		return;

	if (ent->building->s_use)
		gi.sound(ent, CHAN_VOICE, gi.soundindex(ent->building->s_use), 1.0, ATTN_NORM, 0);

	ent->takedamage = DAMAGE_NO;
	AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorRotate(ent->building->weapon[0].offset, ent->s.angles, pos);
	VectorAdd(ent->s.origin, pos, pos);

	if (other && other->client && (other == activator))
		user = other;
	else
		user = ent->master;

	apply_effects(ent->building->effect, activator, ent, user, pos, activator->s.origin, forward, forward, ent->building->range, ent->health * 3, NULL);

	if (!ent->building)	// building was destroyed by effect
		return;

	ent->timestamp = level.time + ent->building->duration;

	if (ent->building->use_count) {
		if (!(--ent->count)) {
			ent->use = NULL;
			if (ent->die)
				ent->die (ent, activator, activator, ent->health, ent->s.origin);
		}
	}
}

void EfxBox_create(edict_t *ent) {
char name[128];

	ent->classname = "efx_box";
	ent->use = EfxBox_use;
	ent->die = Building_die;
	ent->think = Building_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->timestamp = 0;

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_EFXBOX, ent, ent->master, name, ent->building->name, false);
}

//
// Spotter - simple building which activates a flashlight/laser sight similar device.
// Spotter must be attached, but are most often attached to turrets or guns
//
void Spotter_think(edict_t *self) {
vec3_t		start, end, forward;
trace_t		tr;
edict_t		*owner;


	if (!self->owner->flashlight) {
		self->owner->state = BLDG_STATE_ONLINE;
		self->owner->flashlight = NULL;
		G_FreeEdict(self);
		return;
	}
	if (!self->spawnflags && (level.time > self->timestamp)) {
		self->owner->state = BLDG_STATE_ONLINE;
		self->owner->flashlight = NULL;
		G_FreeEdict(self);
		return;
	}

	if (self->owner->state < BLDG_STATE_ONLINE) {
		self->owner->flashlight = NULL;
		G_FreeEdict(self);
		return;
	}
	self->owner->state = BLDG_STATE_ACTIVATED;

	owner = self->owner;
	VectorCopy(owner->s.origin, start);
	start[2] += owner->viewheight;

	AngleVectors(owner->s.angles, forward, NULL, NULL);
	VectorMA(start, 24, forward, start);
	VectorMA(start, owner->building->range, forward, end);
	
	tr = gi.trace(start, NULL, NULL, end, owner, MASK_PLAYERSOLID);
	VectorCopy(tr.endpos, self->s.origin);
	vectoangles(tr.plane.normal, self->s.angles);

	gi.linkentity(self);
	self->nextthink = level.time + FRAMETIME;
}

void Spotter_use(edict_t *ent, edict_t *other, edict_t *activator) {

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	Building_Use(ent, other, activator);

	if (ent->building->s_use)
		gi.sound(ent, CHAN_VOICE, gi.soundindex(ent->building->s_use), 1.0, ATTN_NORM, 0);

	if (other && other->client && (other == activator) && (ent->flashlight)) {
		ent->flashlight->spawnflags = !ent->spawnflags;
		return;
	}

	ent->state = BLDG_STATE_ACTIVATED;
	if (!ent->flashlight) {
	edict_t *spot;
		//========== flashlight stuff
		spot = G_Spawn();
		spot->owner = ent;
		spot->s.effects = ent->building->subtype;
		spot->s.renderfx = ent->building->use_count;
		spot->s.modelindex = gi.modelindex (ent->building->effect);
		spot->s.skinnum = ent->building->skin;
		spot->timestamp = level.time + ent->building->duration;
		spot->think = Spotter_think;
		spot->nextthink = level.time + FRAMETIME;

		ent->flashlight = spot;
	} else
		ent->flashlight->timestamp = level.time + ent->building->duration;
}

void Spotter_create(edict_t *ent) {
char name[128];

	ent->classname = "spotter";
	ent->use = Spotter_use;
	ent->die = Building_die;
	ent->think = Building_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->timestamp = 0;

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_SPOTTER, ent, ent->master, name, ent->building->name, false);
}

//
// Decoder - building which intercepts non-jammed, enemy radio messages, and
// replays them to the owning team.  Decoders are activated when they detect
// an enemy transmition, and cannot be manually activated.
void MsgDecoder_think(edict_t *ent) {
	if (ent->radio_que)
		Radio_CheckQue(ent);

	Building_think(ent);
	ent->nextthink = level.time + FRAMETIME;
}

void MsgDecoder_use(edict_t *ent, edict_t *other, edict_t *activator) {

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	Building_Use(ent, other, activator);

	ent->state = BLDG_STATE_ACTIVATED;
	if (ent->message1)
		Radio_Chat(ent, ent->message1);

	if (ent->radio_msg) {
		// Radio_Use(ent, "misc/lasrgate.wav");
		Radio_Use(ent, ent->radio_msg);
	}
}

void MsgDecoder_create(edict_t *ent) {
char name[128];

	ent->classname = "msg_decoder";
	ent->use = MsgDecoder_use;
	ent->die = Building_die;
	ent->think = MsgDecoder_think;
	ent->nextthink = level.time + FRAMETIME;
	ent->timestamp = 0;
	ent->radio_channel = ent->master->radio_channel;

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_MSGDECODER, ent, ent->master, name, ent->building->name, false);
}

// Gun Mount - basic device that is the basis for sentry guns
// Mounts serve no purpose but to provide a mountint point for
// turrets.  Weapons may then be mounted on turrets.  
void Mount_think (edict_t *ent){
int i, j, on_target=0;
edict_t *pilot;
vec3_t	aDif;
char	file[32];
qboolean moving = false;

	ent->nextthink = level.time + FRAMETIME; // before we return and it never gets set
	if (ent->state < BLDG_STATE_ONLINE)
		return;

	ent->s.sound = 0;
	pilot = ent->use_this;
	// mount piloted?
	if (pilot && pilot->use_this == ent) {
		if (pilot->client) {
			for (i = 0; i < MAX_BLDG_ATTACH; i++) {
				if (!ent->attach[i])
					continue;

				if (!VectorCompare(ent->building->attach[i].speed, vec3_origin)) {
					if (ent->building->s_use) {
						if (!VectorCompare(ent->attach[i]->angle_offset, ent->attach[i]->ideal_angles)) {
							j = sub_count(ent->building->s_use);
							ent->s.sound = gi.soundindex(sub_string(ent->building->s_use, file, random()*i));
						}
					}
					VectorSubtract(pilot->client->v_angle, ent->s.angles, aDif);
					VectorSubtract(aDif, ent->building->attach[i].angles, ent->attach[i]->ideal_angles);
					AnglesMod(ent->attach[i]->ideal_angles, ent->attach[i]->ideal_angles);
				}

				if ( ((pilot->client->latched_buttons|pilot->client->buttons) & BUTTON_ATTACK) ) {
					if (ent->attach[i]->entity->use)
						ent->attach[i]->entity->use(ent->attach[i]->entity, pilot, pilot);
					if (!ent->building)
						return;
				}
			}
		}
	} else {
		if (ent->activator) {
			for (i = 0; i < MAX_BLDG_ATTACH; i++) {
				if (!ent->attach[i])
					continue;
				AnglesMod(ent->attach[i]->ideal_angles, ent->attach[i]->ideal_angles);
				if (VectorCompare(ent->attach[i]->angle_offset, ent->attach[i]->ideal_angles) ||
					VectorCompare(ent->building->attach[i].trackspeed, vec3_origin)) {
					ent->timestamp = level.time + ent->building->duration;
					if (ent->attach[i]->entity->use)
						ent->attach[i]->entity->use(ent->attach[i]->entity, ent, ent->activator);

					if (!ent->building)
						return;
/*
					// Release the lock, and return to normal scanning
					for (i = 0; i < MAX_BLDG_ATTACH; i++)
						if (ent->attach[i])
							VectorCopy(ent->building->attach[i].max, ent->attach[i]->ideal_angles);

					ent->use_this = NULL;
					ent->state = BLDG_STATE_ONLINE;
*/
					ent->activator = NULL;
				}
			}
		} else if (ent->timestamp < level.time) {
			ent->use_this = NULL;
			ent->activator = NULL;
			ent->state = BLDG_STATE_ONLINE;
		}
	}
	Building_think(ent);
}

void Mount_use (edict_t *ent, edict_t *other, edict_t *activator) {
vec3_t	dir, angles, pivot, targ_pos;
edict_t *g = NULL;
int		i;

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	ent->state = BLDG_STATE_ACTIVATED;

	if (!other->client) {
		Building_Use(ent, other, activator);

		if (activator) {
			VectorCopy(activator->s.origin, targ_pos);
			if (activator->maxs[2] < 16)
				targ_pos[2] -= 32;

			ent->activator = activator;
		} else {
			VectorCopy(other->pos3, targ_pos);
			ent->activator = &g_edicts[0];
		}
			
		for (i = 0; i < MAX_BLDG_ATTACH; i++) {
			if (!ent->attach[i])
				continue;

			if (VectorCompare(ent->building->attach[i].trackspeed, vec3_origin))
				continue;

			VectorRotate(ent->building->attach[i].offset, ent->s.angles, pivot);
			VectorAdd(ent->s.origin, pivot, pivot);
			VectorSubtract(targ_pos, pivot, dir);
			VectorNormalize(dir);
			vectoangles(dir, angles);

			VectorSubtract(angles, ent->s.angles, ent->attach[i]->ideal_angles);
			VectorSubtract(ent->attach[i]->ideal_angles, ent->building->attach[i].angles, ent->attach[i]->ideal_angles);
		}
	} else {
		// Release the previously used item
		if (other->use_this)
			other->use_this->use_this = NULL;

		if (ent->use_this && (ent->use_this->use_this == ent))
			return;

		ent->activator = other;
		ent->use_this = other;
		other->use_this = ent;
	}
}

void Mount_create(edict_t *ent) {
char name[128];

	ent->state = BLDG_STATE_DISABLED;

	ent->classname = "gun_mount";
	ent->die = Building_die;
	ent->use = Mount_use;
	ent->think = Mount_think;
	ent->nextthink = level.time + FRAMETIME;

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_MOUNT, ent, ent->master, name, ent->building->name, true);
}

void Gun_Use(edict_t *ent, edict_t *other, edict_t *activator)
{
	vec3_t	dir, pos, fire_pos;
	weap_t	*weapon;
	ammo_t	*ammo;
	edict_t	*old_owner, *user, *g;
	char	*weap_name = NULL, *weap_ammo = NULL;
	float	pHealth;
	int		i;

	if (!ent->inuse || !ent->building)
		return;

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	Building_Use(ent, other, activator);

	ent->state = BLDG_STATE_ACTIVATED;

	pHealth = ((float)ent->health / (float)ent->building->min_points / 5.0) + 0.8;
	if (random() > pHealth)
		return;

	VectorSubtract(other->s.origin, ent->s.origin, dir);
	VectorNormalize(dir);

	ent->enemy = other;
	ent->activator = activator;

	old_owner = ent->owner;
	ent->owner = ent->family.parent;

	do {
		int i = 0;
		while (!weap_name && (i < MAX_BLDG_WEAPONS)) {
			ent->count++, i++;
			ent->count %= MAX_BLDG_WEAPONS;
			if (ent->nextfire[ent->count] <= level.time) {
				weap_name = ent->building->weapon[ent->count].name;
				weap_ammo = ent->building->weapon[ent->count].ammo;
				VectorCopy (ent->building->weapon[ent->count].offset, fire_pos);
			}
		}

		weapon = GetWeaponByName(weap_name);
		if (!weapon)
			continue;
	
		if ((weapon->flags == WT_SEMI) && (ent->timestamp > level.time))
			continue;

		VectorRotate(fire_pos, ent->s.angles, pos);
		VectorAdd(pos, ent->s.origin, pos);

		ammo = GetAmmoByName(weap_ammo);
		if (!ammo)
			ammo = GetCompatibleAmmo(weapon->base_ammo);
		if (!ammo)
			gi.dprintf("No ammo for %s\n", weapon->name);

		if (other->client && (other == activator))
			user = activator;
		else
			user = ent->master;

		GenericMountedWeapon(ent, user, pos, ent->s.angles, weapon, ammo);
		ent->nextfire[ent->count] = level.time + weapon->cyclic_rate - FRAMETIME;

		// Weapon may have killed itself while firing
		if (!ent->inuse)
			return;
		
		// Continue to fire weapon after use function is called (for upto ->duration seconds)
		if (other != activator)
			if (ent->timestamp < level.time)
				ent->timestamp = level.time + ent->building->duration;

		/*
		ammo[i] = (ammo_t *)FindPreferedCompatibleAmmo(ent, weapon[i]->base_ammo)->item;
		if (!ammo[i])
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			continue;
		}
		*/

	} while (0);

	ent->owner = old_owner;

	for (i = 0; ent->family.children && (g = ent->family.children[i]); )
	{
		if (g->inuse && g->building && g->use)
			g->use(g, ent, ent->activator);
//		if (g->inuse)
			i++;
		if (!ent->inuse)	// a building's child can kill it
			return;
	}
}

void Gun_think(edict_t *ent)
{
	edict_t *target;

	ent->nextthink = level.time + FRAMETIME; // before we return and it never gets set

	target = ent->family.parent;
	if (!target || !target->inuse)
		return;

/*	if (target->building && target->building->type == BLDG_MOUNT)
		VectorCopy(target->move_angles, ent->s.angles);
	else if (target != world)
		VectorCopy(target->s.angles, ent->s.angles);
*/
	if (ent->state < BLDG_STATE_ONLINE)
		return;

	if (ent->state == BLDG_STATE_ONLINE) //{
		if (ent->building->frame_idle_num > 1)
			ent->s.frame = ent->building->frame_idle + (ent->s.frame % ent->building->frame_idle_num);
//		if (ent->s.frame > ent->building->frame_idle + ent->building->frame_idle_num)
//			ent->s.frame = ent->building->frame_idle;
//	}

	// Continue to fire weapon until timestamp expires
	if (ent->activator) {
		if (ent->timestamp > level.time)
			Gun_Use(ent, ent->enemy, ent->activator);
		else {
			ent->activator = NULL;
			ent->timestamp = 0.0;
			ent->state = BLDG_STATE_ONLINE;
		}
		// Weapon may have destroyed itself when firing
		if (!ent->inuse)
			return;
	}

	Building_think(ent);
}

	
void Gun_create(edict_t *ent) {
char	name[128];
int		i;

	ent->classname = "gun";
	ent->die = Building_die;
	ent->use = Gun_Use;
	ent->think = Gun_think;
	ent->nextthink = level.time + FRAMETIME;
	for (i = 0; i < MAX_BLDG_WEAPONS; i++)
	{
		if (ent->building->weapon[i].name)
			AddToInventory(ent, ITEM_WEAPON, GetWeaponByName(ent->building->weapon[i].name), i+1, 1, 1, NULL, NULL);
	}
	
	ent->state = BLDG_STATE_DISABLED;
	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_GUN, ent, ent->master, name, ent->building->name, true);
}

//MacG -- Begin 6/24/98
//====================================================================================
// Security camera
//
// Attaches to the wall the player is facing and rotates. Allows 
// other players to view through camera. 
// Properties:
// owner = player who built camera
// move_origin = point player looks through (must be outside model)
// random = fov to restrict player's view (0 doesn't restrict)
// health = camera's health (<= 0 makes it invincible)
// chain = next camera in team's camera list
// team_master = previous camera in team's camera list
//====================================================================================

// dummy entity, used for camera (possibly for chasecam)
// currently limited to one dummy per player
void Dummy_destroy (edict_t *ent)
{
	if (!ent->client->dummy)
		return;
	ent->svflags &= ~SVF_NOCLIENT;
	G_FreeEdict (ent->client->dummy);
	ent->client->dummy = NULL;
}

void Dummy_think (edict_t *ent)
{
	int vw_index	= ent->s.modelindex2;
	ent->s			= ent->owner->s;
	ent->s.modelindex2 = vw_index;
/*
	if ((ent->owner->client->weaponstate == WEAPON_READY) || (ent->owner->client->weaponstate == WEAPON_FIRING)) {
		if (ent->owner->client->vw_index) ent->s.modelindex2 = ent->owner->client->vw_index;
		else ent->s.modelindex2 = 255; // default vweap index
	} else
		ent->s.modelindex2 = 0;
*/	
	ent->nextthink	= level.time + FRAMETIME;
	gi.linkentity (ent);
}

void Dummy_create (edict_t *ent)
{
	edict_t *dummy;

	// don't make multiple dummies
	if (ent->client->dummy)
		return;

	dummy			= G_Spawn();
	if (!dummy)
		return;
	dummy->movetype	= MOVETYPE_NONE;
	dummy->solid	= SOLID_NOT;
	dummy->owner	= ent;
	dummy->s		= ent->s;
	dummy->think	= Dummy_think;
	dummy->nextthink= level.time + FRAMETIME;
	gi.linkentity (dummy);

	ent->client->dummy = dummy;
	ent->svflags |= SVF_NOCLIENT;	
}

// called every server frame
void Camera_update (edict_t *ent)
{
	gclient_t	*client = ent->client;
	edict_t		*camera = client->camera;

	if (!camera)
		return;

	// move player's eyes to camera
	client->ps.pmove.origin[0] = camera->move_origin[0]*8;
	client->ps.pmove.origin[1] = camera->move_origin[1]*8;
	client->ps.pmove.origin[2] = camera->move_origin[2]*8;

	VectorClear(client->ps.pmove.velocity);
	VectorClear(client->ps.viewoffset);
	VectorClear(client->ps.kick_angles);
	if (camera->random)
		client->ps.fov = camera->random;

	VectorCopy(camera->s.angles, client->ps.viewangles);
	client->ps.gunindex = 0;
}

void Camera_clientupdate (edict_t *ent)
{
	edict_t *camera = ent->client->camera;
	gclient_t *client = ent->client;

	// needed with PM_FREEZE?
	client->ps.pmove.pm_type |= PMF_NO_PREDICTION;
	// PM_FREEZE turns off prediction, but still sends movement/angles
	client->ps.pmove.pm_type = PM_FREEZE;
}

edict_t *Camera_next (edict_t *ent) {
edict_t *camera;
team_t	*team;

	camera = ent->client->camera;

	team = ent->client->resp.team;

	while (camera = G_Find(camera, FOFS(classname), "camera"))
	{
		if (camera->team == team)
			return camera;
	}
	camera = NULL;
	while (camera = G_Find(camera, FOFS(classname), "camera"))
	{
		if (camera->team == team)
			return camera;
	}
	return NULL;
}

void Camera_toggle (edict_t *ent) {

	if (!ent)
		return;

	if (ent->client->viewcam)
		Camera_select(ent, NULL);
	else
		Camera_select(ent, Camera_next(ent));
}

void Camera_die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		i;
	edict_t	*other;

	for (i = 1; i <= maxclients->value; i++)
	{
		other = g_edicts + i;
		if (other->inuse && other->client->camera == ent)
			Camera_select (other, NULL);
	}
	
	Building_die (ent, inflictor, attacker, damage, point);
}

void Camera_select (edict_t *ent, edict_t *camera) {

	if (!ent)
		return;

	if (camera) {
		// Make ent view camera
		ent->client->camera = camera;
		ent->client->viewcam = true;
		Dummy_create (ent);
	} else {
		// No camera specified, so make ent exit camera view
		ent->client->viewcam = false;
		if (!ent->client->camera)
			return;
		Dummy_destroy (ent);
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	}
}

void Camera_use (edict_t *ent, edict_t *other, edict_t *activator) {
edict_t	*viewer;

	if (!other)
		return;

	if (ent->oldenemy == activator && ent->oldenemy_time > level.time)
		return;

	if (ent->state < BLDG_STATE_ONLINE)
		return;

	ent->oldenemy = activator;
	ent->oldenemy_time = level.time + 3.0;

	if (other->client)
		viewer = other;
	else if (other->master->client)
		viewer = other->master;
	else
		return;

	Building_Use(ent, other, activator);

	// Don't ever activate the camera, unless the player is expliciately doing so
	if (viewer->use_this != ent) {
		// Instead, setup the camera to be the first one viewed with "viewcam"
		viewer->client->camera = ent;
		return;
	}

	Camera_select(viewer, ent);

	if (viewer->use_this == ent) {

		if (ent->use_this && (ent->use_this->use_this == ent))
			return;

		ent->activator = activator;
		ent->use_this = viewer;
		viewer->use_this = ent;
	}
}

void Camera_think (edict_t *ent) {
vec3_t	dir;

/*	target = ent->family.parent;
	if (target) {
		if (target->building && target->building->type == BLDG_MOUNT)
			VectorCopy(target->move_angles, ent->s.angles);
		else if (target != world)
			VectorCopy(target->s.angles, ent->s.angles);
	}
*/
	// Are we being used?
	if (ent->use_this) {
		// Does the user no he is using us?
		if (ent->use_this->use_this == ent) {
			VectorSubtract(ent->s.origin, ent->use_this->s.origin, dir);
			if (VectorLength(dir) > 64) {// to far away to use, so release the user and the camera
				if (ent->use_this->client->camera = ent)
					Camera_toggle(ent->use_this);
				ent->use_this->use_this = NULL;
				ent->use_this = NULL;
			}
		} else
			ent->use_this = NULL;
	}
		
	AngleVectors(ent->s.angles, dir, NULL, NULL);
	VectorMA(ent->s.origin, 8, dir, ent->move_origin);

	Building_think (ent);
	ent->nextthink = level.time + FRAMETIME;
}

void Camera_create (edict_t *ent) {
char name[128];

	if (ent->takedamage)
		ent->takedamage	= DAMAGE_YES;
	ent->die			= Camera_die;
	ent->use			= Camera_use;
	ent->think			= Camera_think;
	ent->nextthink		= level.time + FRAMETIME;
	ent->classname		= "camera";
	ent->random			= ent->building->fov;

	ent->angle			= ent->s.angles[YAW];

	sprintf (name, "%s %2d", ent->master->client->pers.netname, ent->master->client->resp.building_count[ent->building - buildings]);
	ent->message = ED_NewString(name);
	AddToTeamObjects (ent->master->client->resp.team, BLDG_CAMERA, ent, ent->master, name, ent->building->name, true);
}






void ReleaseBuildings(edict_t *ent) {
edict_t	*bldg;
struct gclient_s *cl;
int		tpts = 0;
int		i, j, k;

	if (!ent)
		return;

	cl = ent->client;
	for (k=0; k<MAX_BLDG; k++) {
		bldg = cl->resp.bldgs[k];
		if (!bldg)
			continue;

		// Is the player a maintainer for this building?
		for (i=0; i<4; i++) {
			if (cl == bldg->builders[i]) {

				// free player from building
				cl->resp.bldg_pts[bldg->bldr_index[i]] = 0;
				cl->resp.bldgs[bldg->bldr_index[i]] = NULL;

				// free building from player
				bldg->builders[i] = NULL;
				bldg->bldr_index[i] = 0;

				// recalc building health from remaining points
				for (j=0; j<4; j++) {
					if (bldg->builders[i] && bldg->builders[i]->resp.bldgs[bldg->bldr_index[i]])
						tpts += bldg->builders[i]->resp.bldg_pts[bldg->bldr_index[i]];

					bldg->health = tpts;
				}
				if (!bldg->health)
					Building_die(bldg, NULL, ent, 0, bldg->s.origin);
			}
		}
	}
}

int CheckBuildingPoints(edict_t *ent) {
int i, pts_inuse = 0, pts = 0, new_pts = 0, diff;
item_t *item;
float f;

	// Count players total points
	for (item = ent->inventory, pts = 0; item; item = item->next)
		if ((item->itemtype == ITEM_SUPPLY) && (ITEMSUPPLY(item)->type == SUP_TOOL))
			pts += item->quantity;
	ent->client->bldg_pts = pts;

	// Count points in use by buildings
	for (i = 0, pts_inuse = 0; i < MAX_BLDG; i++)
		pts_inuse += ent->client->resp.bldg_pts[i];
	new_pts = pts_inuse;

	// Redistribute if inuse is more then available
	if (pts_inuse > ent->client->bldg_pts) {
		new_pts = 0;
		for (i = 0; i < MAX_BLDG; i++) {
			if (!ent->client->resp.bldgs[i]) {	// Invalid building reference
				ent->client->resp.bldg_pts[i] = 0;
				continue;
			}
			f = (float)ent->client->resp.bldg_pts[i] / (float)pts_inuse;  // Get factor of difference in points
			pts = (float)ent->client->bldg_pts * f;	// Calculate new points from factor
			diff = ent->client->resp.bldg_pts[i] - pts;
			Building_pain(ent->client->resp.bldgs[i], ent, 0.0, diff);	// Damage the building by the difference
			new_pts += ent->client->resp.bldg_pts[i];	// Calc new points
		}
	}
	return new_pts;
}

void Building_pain(edict_t *self, edict_t *other, float kick, int damage) {
struct gclient_s *cl;
int		pts[4], tpts = 0;
int		i, j;

	// distribute points lost back to players
	// calculate percentage distributive
	for (i=0; i<4; i++) {
		cl = self->builders[i];
		if (cl && cl->pers.connected) {
			pts[i] = (cl->resp.bldg_pts[self->bldr_index[i]] / (self->health + damage)) * damage;
			tpts += pts[i];
		}
	}
	// cleanup after rounding errors, dump fractional points
	j = 0;
	while ((tpts != damage) && (j<4)) {
		i++, j++;
		i %= 4;
		cl = self->builders[i];
		if (cl && cl->pers.connected && (pts[i] != cl->resp.bldg_pts[self->bldr_index[i]])) {
			if (tpts > damage) {
				pts[i]--;
				tpts--;
			} else {
				pts[i]++;
				tpts++;
			}
			j=0;
		}
	}
	if (j >= 4)
		gi.dprintf("Resource Point Distribution mismatch for: %s\n", self->building->name);

	// apply points back to players
	for (i=0; i<4; i++) {
		cl = self->builders[i];
		if (cl)
			cl->resp.bldg_pts[self->bldr_index[i]] -= pts[i];
	}
}

void Building_die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point){
edict_t *other;
object_t	*obj;
int			i;

	// Remove building from team/personal object lists
	ent->die = NULL;

	// release resource points
	for (i=0; i<4; i++) {
		if (ent->builders[i]) {
			ent->builders[i]->resp.bldgs[ent->bldr_index[i]] = NULL;
			ent->builders[i]->resp.bldg_pts[ent->bldr_index[i]] = 0;
		}

		ent->builders[i] = NULL;
		ent->bldr_index[i] = 0;
	}

	if (ent->building->flags & BLDG_USE_ON_DESTROY && ent->use)
		ent->use(ent, attacker, attacker);

	ThrowDebris (ent, "models/objects/debris1/tris.md2", 5, ent->s.origin);

	// Kill all attachments
	for (i = 0; ent->family.children && (other = ent->family.children[i]); )
	{
		// Don't increment i unless there is no die function
		// otherwise i can be incremented while the child list is shortened,
		// and eventually skip entries and/or the null terminator
		if (other->die)
			other->die (other, inflictor, attacker, damage, point);
		else
		{
			G_FreeEdict(other);
			i++;
		}
	}
	if (ent->family.children)
		gi.TagFree(ent->family.children);
	ent->family.children = NULL;

	// Kill all sensors pointing to this entity
	for (i = 0; ent->sensors.children && (other = ent->sensors.children[i]);)
	{
		if (other->die)
			other->die (other, inflictor, attacker, damage, point);
		else if (other->inuse) {
			G_FreeEdict(other);
			i++;
		} else
			i++;
	}
	if (ent->sensors.children)
		gi.TagFree(ent->sensors.children);
	ent->sensors.children = NULL;

	// Remove building from team object list
	for (i = 0; i < MAX_TEAMS; i++)
	{
		if (obj = FindTeamObject(&dlsys.teams[i].objects, TMOFS(entity), (int)ent))
		{
			// Notify owner of death
			if (obj->owner)
				gi.cprintf(obj->owner, PRINT_MEDIUM, "%s \"%s\" has been destroyed\n", obj->type_name, obj->name);
			RemoveFromTeamObjects (obj);
		}	
	}
	// building still under construction?
	if (ent->master && ent->master->client && ent->master->client->building == ent)
		ent->master->client->building = NULL;

	// Remove from parent's child list
	if (other = ent->family.parent)
	{
		RemoveChild(&other->family, ent);
		// Free up attachment slot
		if (other->building)
		{
			for (i = 0; i < MAX_BLDG_ATTACH; i++)
			{
				if (other->attach[i] && other->attach[i]->entity == ent)
				{
					gi.TagFree(other->attach[i]);
					other->attach[i] = NULL;
				}
			}
		}
	}
	gi.TagFree (ent->message);
	G_FreeEdict(ent);
}

void Building_Use (edict_t *ent, edict_t *other, edict_t *activator) {
int i;

	for (i = 0 ; i < MAX_BLDG_ATTACH; i++) {
		if (ent->attach[i] && ent->attach[i]->entity->use)
			ent->attach[i]->entity->use(ent->attach[i]->entity, ent, activator);
	}
}

void Building_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (other->client)
		gi.cprintf(other, PRINT_CHAT, "%s: Stop touching me!!\n", self->message);
}

void Building_think (edict_t *ent) {
vec3_t	spLoc, min, max;
int		i, j, pts, pts_inuse = 0;
float	pHealth;
edict_t	*parent;

	// Building construction is a factor of building points.  Health is
	// based on building points / max points.
	// As points are applied, health increases.  A building does not
	// become operational until its min points are achieved, and ceases
	// to operate (but still exist) if its points drops below min points

	// Buildings need to keep track of who contributed who many points
	// to its constuction, so as its damaged or destroyed, points are
	// reallocated back to thier original owners

	// Buildings, are automatically upgraded when thier points reaches
	// max points.  It is assumed that the new buildings max points is
	// greater than the old buildings hax points

	// Do damage/construction effects - throw sparks and flash based on health
	pHealth = (float)ent->health / (float)ent->building->min_points;
	if (random() > pHealth*0.2 + 0.8) {
		VectorCopy(ent->s.origin, spLoc);
		for (i=0; i<3; i++)
			spLoc[i] += (crandom() * ent->maxs[0]);
		CastFountain(TE_SPLASH, spLoc, tv(0,0,1), 8 + (random() * 16), 1, 0);
		gi.multicast(spLoc, MULTICAST_PVS);
		MuzzleFlash2(ent, 61, MULTICAST_PVS);

		// randomly rotate attachments
		for (i = 0; i < MAX_BLDG_ATTACH; i++)
		{
			if (!ent->attach[i])
				continue;

			ent->attach[i]->ideal_angles[YAW] += (crandom() * (1.0 - pHealth)) * 15;
			ent->attach[i]->ideal_angles[PITCH] += (crandom() * (1.0 - pHealth)) * 15;
		}
	}
	// Disable buildings with less than 1/4 thier health..
	if (pHealth < 0.25)
		ent->state = BLDG_STATE_DISABLED;

	// sometimes building don't die - cheap hack
	if (ent->health <= 0) {
		if (ent->die)
			ent->die(ent, NULL, NULL, 0, vec3_origin);
		else
			Building_die(ent, NULL, NULL, 0, vec3_origin);
		return;
	}

	// Check the buildings points
	for (i=0; i<4; i++) {
		if (ent->builders[i])
			pts_inuse += ent->builders[i]->resp.bldg_pts[ent->bldr_index[i]];
	}
	if (pts_inuse != ent->health)
		ent->health = pts_inuse;

	if (ent->health < ent->building->min_points)
		ent->s.renderfx = RF_TRANSLUCENT;
	else if (ent->state >= BLDG_STATE_ONLINE)
		ent->s.renderfx = RF_IR_VISIBLE;
	else
		ent->s.renderfx = 0;

	if (ent->think == Building_think)
		ent->nextthink = level.time + FRAMETIME;

	// rotate attachments
	if ((ent->state > BLDG_STATE_OFFLINE)) {
		for (i = 0; i < MAX_BLDG_ATTACH; i++) {
			if (!ent->attach[i])
				continue;
			
			VectorAdd(ent->s.angles, ent->building->attach[i].angles, ent->attach[i]->entity->s.angles);
			// check seperately, so activated buildings which ARE locked, don't pan idle
			if (ent->state == BLDG_STATE_ACTIVATED) {
			// tracking to ideal angles (automatically, or player controlled
				if (!VectorCompare(ent->building->attach[i].trackspeed, vec3_origin))
					// Rotates on its own
					DL_ChangeAngles(ent->attach[i]->angle_offset, ent->attach[i]->ideal_angles, ent->building->attach[i].trackspeed, ent->attach[i]->angle_offset);
				else if (ent->building->flags & BLDG_ALLOW_PLAYERROTATE)
					// Manually rotated
					DL_ChangeAngles(ent->attach[i]->angle_offset, ent->attach[i]->rotate_offset, tv(15, 15, 15), ent->attach[i]->angle_offset);
				//VectorAdd(ent->attach[i]->entity->s.angles, ent->attach[i]->angle_offset, ent->attach[i]->entity->s.angles);
			} else if (VectorCompare(ent->building->attach[i].speed, vec3_origin) && (ent->building->flags & BLDG_ALLOW_PLAYERROTATE)) {
			// rotate toward IDEAL_ANGLES for mounts and turrets that can be manually (by hand) rotated.
				DL_ChangeAngles(ent->attach[i]->angle_offset, ent->attach[i]->rotate_offset, tv(15, 15, 15), ent->attach[i]->angle_offset);
				//VectorAdd(ent->attach[i]->entity->s.angles, ent->attach[i]->angle_offset, ent->attach[i]->entity->s.angles);
			} else if (!VectorCompare(ent->building->attach[i].speed, vec3_origin)) {
			// Idle and automatically rotated
				if (pHealth >= 1.0) {
					VectorAdd(ent->building->attach[i].min, ent->attach[i]->rotate_offset, min);
					VectorAdd(ent->building->attach[i].max, ent->attach[i]->rotate_offset, max);
					for (j = 0; j < 3; j++) {
						ent->attach[i]->ideal_angles[j] = anglemod(ent->attach[i]->ideal_angles[j]);
						if (ent->attach[i]->angle_offset[j] == ent->attach[i]->ideal_angles[j]) {
							if (ent->attach[i]->ideal_angles[j] == max[j])
								ent->attach[i]->ideal_angles[j] = min[j];
							else
								ent->attach[i]->ideal_angles[j] = max[j];
						}
					}
				}

				DL_ChangeAngles(ent->attach[i]->angle_offset, ent->attach[i]->ideal_angles, ent->building->attach[i].speed, ent->attach[i]->angle_offset);
				//VectorAdd(ent->attach[i]->entity->s.angles, ent->attach[i]->angle_offset, ent->attach[i]->entity->s.angles);
			}
			VectorAdd(ent->attach[i]->entity->s.angles, ent->attach[i]->angle_offset, ent->attach[i]->entity->s.angles);
			VectorClear(ent->attach[i]->entity->mins);
			VectorClear(ent->attach[i]->entity->maxs);
			BBoxRotate(ent->attach[i]->entity->building->mins, ent->attach[i]->entity->s.angles, ent->attach[i]->entity->mins, ent->attach[i]->entity->maxs);
			BBoxRotate(ent->attach[i]->entity->building->maxs, ent->attach[i]->entity->s.angles, ent->attach[i]->entity->mins, ent->attach[i]->entity->maxs);
		}
	}
	gi.linkentity(ent);

	if ((ent->owner != ent->master) && (ent->master->client->building != ent))
		return;

	// check construction process
	if (ent->master->client->building == ent) {
		VectorSubtract(ent->s.origin, ent->master->s.origin, spLoc);
		if ((VectorLength(spLoc) > ent->building->max_dist) || (ent->health >= ent->building->min_points)) {
			Building_finish(ent);
		} else {
			pts = (ent->building->min_points / ent->building->build_time) * FRAMETIME;
			if (!pts) pts = 1;
			ent->health += pts;

			// Account for resource points
			ent->builders[0]->resp.bldg_pts[ent->bldr_index[0]] += pts;
		}
	} else if (ent->flags)
		// Something happened to our building, so go ahead and finish the building, as though he had walked away.
		Building_finish(ent);

	// check to see if the builder has left the bounding box, and if so
	// make it solid to him
	for (i = 0; i < 3; i++)	{
		if ((ent->master->deadflag) || (ent->master->absmin[i] > ent->absmax[i]) || (ent->master->absmax[i] < ent->absmin[i]))	{
			for (parent = ent; parent->family.parent; parent = parent->family.parent);
			ent->owner = parent;	// set owner to the highest level building
			break;
		}
	}

	if (ent->owner == ent->master) {
		VectorSubtract(ent->master->s.origin, ent->s.origin, spLoc);
		VectorNormalize(spLoc);
		VectorMA(ent->master->velocity, 50, spLoc, ent->master->velocity);
	}
}


int Building_findslot (edict_t *parent, int type, qboolean ignore_children) {
int		i;
	
	if (!parent->building)
		return -1;

	for (i = 0; i < MAX_BLDG_ATTACH; i++)
	{
		if (parent->building->attach[i].type & type)
		{
			if (ignore_children)
				return i;

			if (!parent->attach[i])
				return i;
		}
	}
	return -1;
}

edict_t *Building_createbasic (edict_t *ent, building_t *type, vec3_t origin, vec3_t angles, edict_t *target, char *name)
{
	edict_t	*building;
	int		i, slot;

	building = G_Spawn();
	building->solid = SOLID_BBOX;
	building->movetype = MOVETYPE_NONE;
	building->clipmask = MASK_SOLID;
	building->svflags &= ~SVF_NOCLIENT;
	building->flags = 1;	// under construction
	building->health = 1;
	building->count = type->use_count;
	if (type->min_points > 0) {
		building->takedamage = DAMAGE_AIM;
		building->pain = Building_pain;
	}
	building->master = ent;
	building->owner = ent;
	building->team = ent->client->resp.team;
	building->building = type;

//	building->touch = Building_touch;

	// find open building slot, or one that references same building (later *should not* happen)
	for (i=0; i<MAX_BLDG; i++)
		if (!ent->client->resp.bldgs[i] || (ent->client->resp.bldgs[i] == building))
			break;
	if (!ent->client->resp.bldgs[i] || (ent->client->resp.bldgs[i] == building)) {
		// found a spot, so load up the values, linking the owner and this building to each other
		ent->client->resp.bldgs[i] = building;
		ent->client->resp.bldg_pts[i] = 0;

		building->bldr_index[0] = i;
		building->builders[0] = ent->client;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "You can't build any more items.\n");
		return NULL;
	}

	if (target && target != world)
	{
		target->goalentity = building;
		building->family.parent = target;
		AddChild(&target->family, building);
		if (target->building) {
			if ((slot = Building_findslot(target, type->type, false)) >= 0) {
				target->attach[slot] = gi.TagMalloc(sizeof(*target->attach[slot]), TAG_LEVEL);
				memset(target->attach[slot], 0, sizeof(*target->attach[slot]));
				target->attach[slot]->entity = building;
				VectorCopy(target->building->attach[slot].max, target->attach[slot]->ideal_angles);
			}
		}
	}
	else
		building->family.parent = NULL;

	building->message = ED_NewString(name);
	VectorCopy (origin, building->s.origin);
	VectorCopy (angles, building->s.angles);

	if (type->model)
		gi.setmodel (building, type->model);
	building->s.skinnum = type->skin;

	gi.linkentity (building);

	building->think = Building_think;
	building->nextthink = level.time + FRAMETIME;

	stick(building, target);
	return building;
}

void Building_finish(edict_t *building) {
supply_t *sup;
edict_t *parent;
int i;

	// ent could be world (made by mapmaker)
	if (building->master && building->master->client) {
		building->master->client->resp.building_count[building->building-buildings]++;

		// check to see if the builder has left the bounding box, and if so
		// make it solid to him
		for (i = 0; i < 3; i++)	{
			if ((building->master->deadflag) || (building->master->absmin[i] > building->absmax[i]) || (building->master->absmax[i] < building->absmin[i]))	{
				for (parent = building; parent->family.parent; parent = parent->family.parent);
				building->owner = parent;	// set owner to the highest level building
				break;
			}
		}
	}

	switch (building->building->type) {
	case BLDG_EFXBOX:
		EfxBox_create(building);
		break;
	case BLDG_MOUNT:
		Mount_create(building);
		break;
	case BLDG_GUN:
		Gun_create(building);
		break;
	case BLDG_CAMERA:
		Camera_create(building);
		break;
	case BLDG_REMOTE_TRIGGER:
		break;
	case BLDG_MSGDECODER:
		MsgDecoder_create(building);
		break;
	case BLDG_LASERGATE:
		LaserGate_create(building);
		break;
	case BLDG_SPOTTER:
		Spotter_create(building);
		break;
	default:
		gi.error ("Unknown building type: %d\n", building->building->type);
		break;
	}
	if (building->health < building->building->min_points)
		building->state = BLDG_STATE_DISABLED;
	else
		building->state = BLDG_STATE_ONLINE;	// Bring buildings online immediately

	building->flags = 0;	// construction complete

	if (building->master) {
		sup = GetSupplyByName(building->building->trigger);
		if (sup && (sup->type & SUP_TIMER|SUP_PROXY|SUP_VIDSENSOR))
			SetTrigger(building->master, building, NULL, NULL, building, vec3_origin, sup);

		if (building->master->client) {
			gi.cprintf(building->master, PRINT_HIGH, "Finished building.\n");
			building->master->client->building = NULL;
		}
	}
	if (building->building->s_done)
		gi.sound(building, CHAN_VOICE, gi.soundindex(building->building->s_done), 1.0, ATTN_NORM, 0);
}

int CanBuildBuilding (edict_t *ent, building_t *type) {
vec3_t	start, end, forward, angles, mins, maxs;
trace_t tr, tr1;
int pts_avail, tools = 0, ttypes = 0;
item_t *item;
supply_t *tool;

	if (!ent || !type)
		return -1;

	VectorCopy (ent->s.origin, start);
	start[2] += ent->viewheight;
	VectorCopy(ent->client->v_angle, angles);
	AngleVectors (angles, forward, NULL, NULL);
	VectorMA (start, type->max_dist, forward, end);

	// Check the players total points and for the required tools
	pts_avail = ent->client->bldg_pts - CheckBuildingPoints(ent);

	for (item = ent->inventory; item; item = item->next) {
		tool = ITEMSUPPLY(item);
		if ((item->itemtype == ITEM_SUPPLY) && (tool->type == SUP_TOOL)) {
			tools |= tool->flags;
			ttypes |= tool->tool;
		}
	}

	// Enough points?
	if (pts_avail < type->min_points)
		return -2;

	// Right tools?
	if (((tools & type->req_tools) != type->req_tools) || ((ttypes & TOOL_BUILD) != TOOL_BUILD))
		return -3;

	// Check other building requirements
	tr1 = gi.trace(start, NULL, NULL, end, ent, MASK_PLAYERSOLID);
	if (VectorLength(tr1.plane.normal) > 0.0) {
		// Rotate Building BBox to match plane orientation
		VectorClear(mins);
		VectorClear(maxs);
		vectoangles(tr1.plane.normal, angles);
		BBoxRotate(type->mins, angles, mins, maxs);
		BBoxRotate(type->maxs, angles, mins, maxs);
	} else {
		VectorCopy(type->mins, mins);
		VectorCopy(type->maxs, maxs);
	}
	tr = gi.trace(start, mins, maxs, end, ent, MASK_PLAYERSOLID);

	if (tr.startsolid)
		return -4;
	if (tr.fraction >= 1.0 || !tr.ent || tr1.fraction >= 1.0 || !tr1.ent || tr1.ent != tr.ent)
		return -5;
	if ((tr.fraction * type->max_dist < type->min_dist) || (tr1.fraction * type->max_dist < type->min_dist))
		return -6;

	if (tr.ent->solid == SOLID_BSP && tr.plane.normal[2] >= 0.7 && !(type->flags & BLDG_ATTACH_FLOORS))
		return -7;
	if (tr.ent->solid == SOLID_BSP && tr.plane.normal[2] > -0.7 && tr.plane.normal[2] < 0.7 && !(type->flags & BLDG_ATTACH_WALLS))
		return -8;
	if (tr.ent->solid == SOLID_BSP && tr.plane.normal[2] <= -0.7 && !(type->flags & BLDG_ATTACH_CEILINGS))
		return -9;
	if (tr.ent->client && !(type->flags & BLDG_ATTACH_PLAYERS))
		return -10;
	if (type->type == BLDG_GUN && (!tr.ent->building || tr.ent->building->type != BLDG_MOUNT))
		return -11;
	if (tr.ent->building) {
		// Will this type of building ever be attachable to the other one?
		if (Building_findslot(tr.ent, type->type, true) == -1)
			return -12;
		// Are all the available attachment slots filled up?
		if (Building_findslot(tr.ent, type->type, false) == -1)
			return -13;
		if (tr.ent->state == BLDG_STATE_DISABLED)	// other building not finished yet
			return -14;
	}
	if (tr.ent->solid == SOLID_BBOX && (!tr.ent->building && !tr.ent->client))
		return -15;

	return 0;
}

qboolean Building_build (edict_t *ent, building_t *type) {
vec3_t	start, end, forward;
vec3_t	origin, angles, vec, mins, maxs;
trace_t tr;
edict_t *building;
char name[32];
int i, pts_inuse = 0, tools = 0, ttypes = 0, slot;

	if (!ent)
		return false;

	if (!type) {
		PMenu_Open(ent, ID_CONSTRUCTION, 0, NULL);
		return false;
	}

	if (ent->client->building)
	{
		i = ent->client->building->bldr_index[0];
		ent->client->resp.bldgs[i] = NULL;
		ent->client->resp.bldg_pts[i] = 0;

		gi.cprintf (ent, PRINT_HIGH, "Stopped building.\n");
		gi.TagFree (ent->client->building->message);
		G_FreeEdict (ent->client->building);
		ent->client->building = NULL;
		return false;
	}

	VectorCopy (ent->s.origin, start);
	start[2] += ent->viewheight;
	VectorCopy(ent->client->v_angle, angles);
	AngleVectors (angles, forward, NULL, NULL);
	VectorMA (start, type->max_dist, forward, end);

	tr = gi.trace(start, type->mins, type->maxs, end, ent, MASK_SHOT);
	switch(CanBuildBuilding(ent, type)) {
	case -1: return false;
	case -2: gi.cprintf (ent, PRINT_HIGH, "You don't have enough points build a %s.\n", type->name); return false;
	case -3: gi.cprintf (ent, PRINT_HIGH, "You don't have the supplies required to build a %s.\n", type->name); return false;
	case -4: gi.cprintf (ent, PRINT_HIGH, "Not enough room to build %s.\n", type->name); return false;
	case -5: gi.cprintf (ent, PRINT_HIGH, "Nothing to attach %s to.\n", type->name); return false;
	case -6: gi.cprintf (ent, PRINT_HIGH, "Not enough room to build here.\n"); return false;
	case -7: gi.cprintf (ent, PRINT_HIGH, "Cannot attach %s to floors.\n", type->name); return false;
	case -8: gi.cprintf (ent, PRINT_HIGH, "Cannot attach %s to walls.\n", type->name); return false;
	case -9: gi.cprintf (ent, PRINT_HIGH, "Cannot attach %s to ceilings.\n", type->name); return false;
	case -10: gi.cprintf (ent, PRINT_HIGH, "Cannot attach %s to players.\n", type->name); return false;
	case -11: gi.cprintf (ent, PRINT_HIGH, "You must attach a gun to a mount.\n"); return false;
	case -12: gi.cprintf (ent, PRINT_HIGH, "Cannot attach %s to %s.\n", type->name, tr.ent->building->name); return false;
	case -13: gi.cprintf (ent, PRINT_HIGH, "Building has no free attachment slots for %s", type->name); return false;
	case -14: gi.cprintf (ent, PRINT_HIGH, "%s is under construction.\n", tr.ent->building->name); return false;
	case -15: gi.cprintf (ent, PRINT_HIGH, "You cannot build here.\n"); return false;
	}

	// tr = gi.trace(start, mins, maxs, end, ent, MASK_SHOT);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if (tr.ent->building) {
		slot = Building_findslot(tr.ent, type->type, false);
		VectorAdd (tr.ent->s.angles, tr.ent->building->attach[slot].angles, angles);
		VectorRotate (tr.ent->building->attach[slot].offset, tr.ent->s.angles, vec);
		VectorAdd(tr.ent->s.origin, vec, origin);
	} else
		VectorCopy (tr.endpos, origin);


	DL_strcpy (name, ent->client->pers.netname, 16);
	sprintf (name + strlen(name), " %2d", ent->client->resp.building_count[type-buildings] + 1);

	// Determine the angle of the new building
	VectorClear (angles);
	if (type->flags & BLDG_ANGLES_SURFACE)
		vectoangles(tr.plane.normal, angles);

	if (type->flags & BLDG_YAW_FACEPLAYER) {
		VectorSubtract(ent->s.origin, tr.endpos, vec);
		vectoangles(vec, vec);
		angles[YAW] = vec[YAW];
	}
	if (type->flags & BLDG_PITCH_FULL) {
		if (type->flags & BLDG_PITCH_PARTIAL)
			angles[PITCH] = anglemod(-ent->s.angles[PITCH] * 0.5);
		else
			angles[PITCH] = anglemod(-ent->s.angles[PITCH]);
	}
	else if (type->flags & BLDG_PITCH_PARTIAL)
		angles[PITCH] = anglemod(-ent->s.angles[PITCH] / 3);


	// Build a placeholder entity until finished building
	gi.cprintf (ent, PRINT_HIGH, "Building %s \"%s\"\n", type->name, name);
	if (!(building = Building_createbasic(ent, type, origin, angles, tr.ent, name)))
		return false; // something happened during creation that prevented it

	// Rotate bounding box to match model
	VectorClear(mins);
	VectorClear(maxs);
	BBoxRotate(type->mins, angles, mins, maxs);
	BBoxRotate(type->maxs, angles, mins, maxs);

	VectorCopy(mins, building->mins);
	VectorCopy(maxs, building->maxs);
	
	building->state = BLDG_STATE_DISABLED;	// indicate not finished yet
	building->s.renderfx |= RF_TRANSLUCENT;
	ent->client->building = building;

	if (type->s_start)
		gi.sound(building, CHAN_VOICE, gi.soundindex(type->s_start), 1.0, ATTN_NORM, 0);

	return true;
}

void SelectBuilding(edict_t *ent, pmenu_t *p) {
building_t *building;

	building = (building_t *)p->arg;

	// close menu after building selection?
	PMenu_CloseAll(ent);
	Building_build (ent, building);
}
