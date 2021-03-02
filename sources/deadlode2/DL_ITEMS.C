#include "g_local.h"

vec3_t grappledir[] =
{
	{-1, -1, -1},
	{-1, -1, 1},
	{-1, 1, -1},
	{1, -1, -1},
	{1, 1, 1},
	{1, 1, -1},
	{1, -1, 1},
	{-1, 1, 1},
	{0, 0, 0}
};

field_t dl_supflds[] =
{
	{"name",			SUPOFS(name),		F_LSTRING},
	{"type",			SUPOFS(type),		F_INT},
	{"flags",			SUPOFS(flags),		F_INT},
	{"tool",			SUPOFS(tool),		F_INT},
	{"weight",			SUPOFS(weight),		F_FLOAT},
	{"model",			SUPOFS(model),		F_LSTRING},
	{"skin",			SUPOFS(skinnum),	F_INT},
	{"frame",			SUPOFS(frame),		F_INT},
	{"range",			SUPOFS(range),		F_INT},
	{"fov",				SUPOFS(range),		F_INT},
	{"damage",			SUPOFS(damage),		F_INT},
	{"health",			SUPOFS(health),		F_FLOAT},
	{"duration",		SUPOFS(duration),	F_FLOAT},
	{"delay",			SUPOFS(delay),		F_FLOAT},
	{"impact_prot",		SUPOFS(impact_prot),F_FLOAT},
	{"lightlevel",		SUPOFS(impact_prot),F_FLOAT},
	{"pierce_prot",		SUPOFS(pierce_prot),F_FLOAT},
	{"blast_prot",		SUPOFS(blast_prot),	F_FLOAT},
	{"energy_prot",		SUPOFS(energy_prot),F_FLOAT},
	{"effect",			SUPOFS(effect),		F_LSTRING},
	{"s_use",			SUPOFS(s_use),		F_LSTRING},
	{"offset",			SUPOFS(offset),		F_VECTOR},
	{"inventory_pic",	SUPOFS(pic_s),		F_LSTRING},
	{"statusbar_pic",	SUPOFS(pic_b),		F_LSTRING},
	{"category",		SUPOFS(category),	F_LSTRING},
	{"defaultcmd",		SUPOFS(defaultcmd),	F_LSTRING},

	{NULL, 0, F_INT}
};

supply_t supplies[MAX_SUPPLIES];

supply_t	*GetSupplyByName(char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_SUPPLIES; i++) {
			if (!supplies[i].name)
				continue;
			if (!DL_strcmp(supplies[i].name, name, -1, false))
				return &supplies[i];
		}
	}
	return NULL;
}

// ========================================================================
// Rules for Objects
// - an object can have one parent, and any number of children
// - an entity can only have one object per team
// - several teams can have objects linked to the same entity (ex: shared cameras)
// - When an object's parent is destroyed, it is no longer valid and must
//   be destroyed as well along with its own children (handled by entity code)

void AddToTeamObjects (team_t *team, int type, edict_t *ent, edict_t *owner, char *name, char *type_name, qboolean visible) {
object_t	*obj, *parent;

	obj = gi.TagMalloc(sizeof(object_t), TAG_LEVEL);
	memset(obj, 0, sizeof(*obj));
	obj->type = type;
	obj->entity = ent;
	obj->owner = owner;
	obj->team = team;
	obj->name = ED_NewString(name);
	obj->type_name = ED_NewString(type_name);
	obj->team_visible = visible;

	switch (type)
	{
	case BLDG_CAMERA:
		break;
	case BLDG_MOUNT:
		break;
	case BLDG_GUN:
		break;
	default:
		break;
	}
	
	parent = FindTeamObject(&team->objects, TMOFS(entity), (int)ent->family.parent);
	if (parent)
	{
		AddChild(&parent->family, obj);
		obj->family.parent = parent;
	}
	else
	{
		AddChild(&team->objects, obj);
		obj->family.parent = NULL;
	}
}

void RemoveFromTeamObjects (object_t *obj) {
object_t	*parent;

	if (!obj)
		return;

	if (parent = obj->family.parent)
		RemoveChild(&parent->family, obj);
	else
		RemoveChild(&obj->team->objects, obj);

	if (obj->family.children)
 		gi.TagFree(obj->family.children);
	gi.TagFree(obj->name);
	gi.TagFree(obj->type_name);
	gi.TagFree(obj);
}

object_t *FindTeamObject(family_t *family, int fieldofs, int match)
{
	object_t	*obj;
	int			i;

	if (!family || !family->children)
		return NULL;
	
	for (i = 0; obj = family->children[i]; i++)
	{
		if(*(int *)((byte *)obj+fieldofs) == match)
			return obj;
		else if (obj->family.children)
		{
			if (obj = FindTeamObject(&obj->family, fieldofs, match))
				return obj;
		}
	}
	return NULL;
}

// Generic function to add a child to a family
void AddChild(family_t *family, void *child)
{
	int		i;

	if (!family)
		return;

	if (!family->children)
		i = 0;
	else
	{
		for (i = 0; family->children[i]; i++)
			;
	}
	family->children = TagReAlloc(family->children, (i+2)*sizeof(void *), TAG_LEVEL);
	family->children[i] = child;
	family->children[i+1] = NULL;
}

// Generic function to remove a child from a family
void RemoveChild(family_t *family, void *child)
{
	int		i;

	if (!family)
		return;

	if (family->children)
	{
		for (i = 0; family->children[i] && family->children[i] != child; i++)
			;
		for (; family->children[i]; i++)
			family->children[i] = family->children[i+1];
		if (!family->children[0])
		{
			gi.TagFree(family->children);
			family->children = NULL;
		}
	}
}

//====================================================================================
// The Deadlode Flashlight
//
// Use the "flashlight" command, which acts as a toggle.  The source simply projects
// an entity in front of the player, whose s.effects state flags are set causing the
// projected entity to glow.
//====================================================================================

void FlashLight_Use(edict_t *owner, supply_t *type);
void FlashLight_Think(edict_t *self) {
vec3_t		start, end, angles;
vec3_t		forward, right, up;
vec3_t		size;
float		accuracy;
trace_t		t_result;
edict_t		*owner;
item_t		*cells;

	if (!self->owner->flashlight) {
		G_FreeEdict(self);
		return;
	}

	owner = self->owner;
	VectorCopy(owner->s.origin, start);
	start[2] += owner->viewheight;

	// Determine flashlight bob/stability
	if (self->owner->client) {	// people shake!
		// Build angle set using kick values, etc.
		VectorMA (self->owner->client->v_angle, 1.5, self->owner->client->oldkick, angles);
		// Accuracy modifiers for movement, stance, if mounted, etc
		if (self->supply->flags & SF_MOUNTED)
			accuracy = VectorLength(self->owner->velocity) / 100;
		else
			accuracy = VectorLength(self->owner->velocity) / 40;
		if (self->owner->client->ps.pmove.pm_flags & PMF_DUCKED)
			accuracy /= 2.0;
	} else {
		VectorCopy(self->owner->s.angles, angles);
		accuracy = 0.0;
	}

	// Apply bob effect
	angles[0] += (random() + 0.5) * accuracy;
	angles[1] += crandom() * accuracy;

	AngleVectors(angles, forward, right, up);
	VectorMA(start, self->supply->range, forward, end);
	
	VectorClear(size);

	t_result = gi.trace(start, size, size, end, owner, MASK_PLAYERSOLID);
	VectorCopy(t_result.endpos, self->s.origin);
	vectoangles(t_result.plane.normal, self->s.angles);

	if (level.time > self->timestamp) {
		cells = FindInventoryItem(self->owner, self->supply);
		if (!cells) {
			FlashLight_Use(self->owner, self->supply);
			return;
		}
		RemoveInventoryItem(self->owner, cells, 1); 
		self->timestamp = level.time + self->supply->duration;
	}

	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void FlashLight_Use(edict_t *owner, supply_t *type) {
edict_t		*self;

	if (owner->flashlight) {
		if (type->s_use)
			gi.sound(owner, CHAN_AUTO, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
		owner->flashlight->nextthink = level.time + FRAMETIME;
		owner->flashlight = NULL;
		return;
	}

	if (type->s_use)
		gi.sound(owner, CHAN_AUTO, gi.soundindex(type->s_use), 1, ATTN_STATIC, 0);

	//========== flashlight stuff
	self = G_Spawn();
	self->owner = owner;
	self->s.effects = type->frame;
	self->s.renderfx = type->tool;
	self->s.modelindex = gi.modelindex (type->model);
	self->s.skinnum = type->skinnum;
	self->supply = type;
	self->timestamp = level.time + type->duration;
	self->think = FlashLight_Think;
	self->nextthink = level.time + FRAMETIME;

	owner->flashlight = self;
}

//====================================================================================
// The Deadlode Grappling Hook
//
// Use the "use grapple" command to throw the grapple hook.  Once it attaches, a
// "laser-line" is created between the anchor and the player, with a fixed length. If
// the player jumps or crouches, the length of the line is increased or decreased.
// Use the "use grapple" command again to release the player from the anchor.
//====================================================================================

void Grapple_Anchor (edict_t *grapple, edict_t *other, cplane_t *plane, csurface_t *surf) {
vec3_t		len, vec;

	if (!grapple->owner)
		return;

	if (!grapple->owner->grapple) {
		grapple->owner = NULL;
		return;
	}

	if (other == grapple->owner || other == grapple->master)
		return;
	
	ProjectClient(vec, grapple->owner, true);
	VectorCopy(grapple->s.origin, grapple->pos2);

	VectorSubtract(grapple->s.origin, vec, len);
	grapple->count = grapple->length = VectorNormalize(len);
	
	VectorClear(grapple->target_plane);
	if (plane)
		VectorCopy(plane->normal, grapple->target_plane);

	// don't grapple to sky or slick surfaces...
	if (surf && (surf->flags & (SURF_SKY|SURF_SLICK))) {
		grapple->owner->grapple = NULL;
		grapple->owner = NULL;
		G_FreeEdict(grapple);
		return;
	}

	VectorSubtract(grapple->s.origin, other->s.origin, grapple->offset);
	VectorCopy(other->velocity, grapple->velocity);
	grapple->enemy = other;

	grapple->solid = SOLID_BBOX;
	// grapple->s.renderfx = RF_BEAM | grapple->supply->flags;
	// black:0xe0e1e2e3, red:0xf2f2f0f0, green:0xd0d1d2d3, blue:0xf3f3f1f1, yellow:0xdcdddedf
	// grapple->s.skinnum = grapple->supply->skinnum;
	// grapple->s.frame = 2;  // beam size

	gi.linkentity(grapple);
}

void Grapple_Release(edict_t *grapple) {
edict_t		*next_anchor;

	if (!grapple)
		return;

	RemoveInventoryItem(grapple->owner, FindInventoryItem(grapple->owner, grapple->supply), 1); 

	gi.sound(grapple->owner, CHAN_AUTO, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);

	// release the player from the closest anchor
	grapple->owner->grapple = NULL;
	// remove all anchors, except the original
	while (grapple->grapple) {
		next_anchor = grapple->grapple;
		G_FreeEdict(grapple);
		grapple = next_anchor;
	}
	grapple->owner = NULL;

	// Turn off the beam
	grapple->s.renderfx = 0;;
	grapple->s.skinnum = 0;
	grapple->s.frame = 0;

	G_FreeEdict(grapple);
	return;
}

void Grapple_Physics(edict_t *ent) {
	vec3_t start, chainvec;	// chain's vector
	float  chainlen;		// length of extended chain
	vec3_t velpart;			// player's velocity component moving to or away from hook
	float f1, f2, fs;			// restrainment forces
	
	// Keep the hook relative to its contact point.
	// Ie, if it hits a players foot, it will stay attached
	// to his foot, no matter where he goes...
	// Also apply the targets velocity to the hook as well.
	// This reduces hook "jump" when the anchor moves
	VectorAdd(ent->enemy->s.origin, ent->offset, ent->s.origin);
	VectorCopy(ent->enemy->velocity, ent->velocity);

	// derive start point of chain
	ProjectClient(start, ent->owner, true);
	
	// get info about chain
	_VectorSubtract (ent->s.origin, start, chainvec);
	chainlen = VectorLength (chainvec);
	
	// if player's location is beyond the chain's reach
	if (chainlen > ent->length) {	 
		// determine player's velocity component of chain vector
		VectorScale (chainvec, _DotProduct (ent->owner->velocity, chainvec) / _DotProduct (chainvec, chainvec), velpart);
		
		// restrainment default force 
		f2 = (chainlen - ent->length) * 7;
		
		// if player's velocity heading is away from the hook
		if (_DotProduct (ent->owner->velocity, chainvec) < 0) {
			// if chain has streched for 25 units
			if (chainlen > ent->length + 25) {
				// remove player's velocity component moving away from hook
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
				VectorCopy(ent->owner->velocity, ent->owner->client->oldvelocity);
			}
			
			f1 = f2;
		} else {
			// if player's velocity heading is towards the hook
			if (VectorLength(velpart) < f2)
				f1 = f2 - VectorLength (velpart);
			else		
				f1 = 0;
		}
	} else
		f1 = 0;
	
	// applys chain restrainment 
	fs = (ent->length / 100.0) + 0.25;
	if (fs < 1.0) f1 *= fs;

	VectorNormalize (chainvec);
	if(ent->enemy->client) {
		if(ent->owner->groundentity)
			VectorMA (ent->enemy->velocity, -f1, chainvec, ent->enemy->velocity);
		else {
			f1 /= 2.0;
			VectorMA (ent->enemy->velocity, -f1, chainvec, ent->enemy->velocity);
			VectorMA (ent->owner->velocity,  f1, chainvec, ent->owner->velocity);
		}
	} else
		VectorMA (ent->owner->velocity, f1, chainvec, ent->owner->velocity);
}
  
void Grapple_Think (edict_t *grapple) {
vec3_t		olen, vlen;
vec3_t		start, end;
vec3_t		normal, point, temp, p1, p2, p3;
edict_t		*ignore, *anchor;
trace_t		tr;
float		numer, denom, dist;
int			i, clipmask;
	
	// remove rogue anchors
	if (grapple->owner->grapple != grapple) {
		Grapple_Release(grapple);
		return;
	}

	clipmask = grapple->clipmask;

	// Remove extra anchors
	// Basically, check the LOS between the two anchors adjacent to the current anchor
	// If the LOS is clear (nothing in between), then remove the current anchor
	ProjectClient(start, grapple->owner, true);
	if (grapple->grapple) {
		VectorCopy(grapple->grapple->s.origin, end);
		tr = gi.trace(start, NULL, NULL, end, grapple, clipmask);
		if (tr.fraction == 1.0) {
			grapple->owner->grapple = grapple->grapple;
			grapple->grapple->owner = grapple->owner;

			VectorSubtract(end, start, vlen);
			grapple->grapple->length = VectorNormalize(vlen);

			G_FreeEdict(grapple);
			return;
		}
	}

	// Now apply swinging physics
	// check for rope length
	VectorSubtract(grapple->s.origin, start, olen);
	i = VectorNormalize(olen);
	if (i > grapple->supply->range) {
		Grapple_Release(grapple->master->grapple);
		return;
	}

	if (grapple->enemy) {
		if (grapple->enemy->inuse)
			Grapple_Physics(grapple);
		else {
			Grapple_Release(grapple->master->grapple);
			return;
		}
	}

	VectorCopy(grapple->s.origin, start);
	ProjectClient(end, grapple->owner, true);
	VectorCopy(end, point);

	// Now check for objects blocking the rope, and add new anchors as required
	ignore = grapple;
	while(1) {
		tr = gi.trace(start, NULL, NULL, end, ignore, clipmask);
		
		// No blocking...
		if (!tr.ent)
			break;
		
		if (VectorCompare(tr.endpos, end))
			break;
		else
		if ((tr.ent != grapple) && (tr.ent != grapple->owner)) {
			// if we hit something that's not a monster or player or is immune to lasers, we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) {
				anchor = G_Spawn();
				if (!anchor) {
					Grapple_Release(grapple->master->grapple);
					return;
				}

				VectorCopy (grapple->pos1, p1);
				VectorCopy (grapple->pos2, p2);
				if (!LineOfSight(p1, p2, clipmask))
				{
					gi.dprintf ("GrappleThink: Old line is not valid\n");
					G_FreeEdict(anchor);
					Grapple_Release (grapple->master->grapple);
					return;
				}
				tr = gi.trace (end, NULL, NULL, start, ignore, clipmask);
				VectorCopy (tr.plane.normal, normal);
				VectorMA (tr.endpos, 8, normal, p3);
				VectorSubtract (p3, p1, temp);
				numer = DotProduct(normal, temp);
				VectorSubtract (p2, p1, temp);
				denom = DotProduct(normal, temp);
				if (denom == 0)
				{	// something is messed up, line is parallel to plane
					gi.dprintf ("GrappleThink: no intersection between line and plane.\n");
					G_FreeEdict (anchor);
					Grapple_Release (grapple->master->grapple);
					return;
				}
				dist = numer/denom;
				if (dist >= 1)
					gi.dprintf ("Dist is >=1\n");
				VectorSubtract (p2, p1, temp);
				VectorMA (p1, dist, temp, point);
				VectorCopy (point, anchor->s.origin);
				if (!LineOfSight(point, start, clipmask) ||
					!LineOfSight(point, end, clipmask))
				{
					gi.dprintf ("GrappleThink: Anchor is not visible to both ends\n");
					G_FreeEdict (anchor);
					return;
				}
				anchor->solid = SOLID_NOT;
				anchor->svflags &= ~SVF_NOCLIENT;
				anchor->clipmask = clipmask;
				anchor->supply = grapple->supply;
				//anchor->s.modelindex = gi.modelindex(grapple->supply->model);
				//anchor->s.renderfx = RF_BEAM | grapple->supply->flags;
				//anchor->s.skinnum = grapple->supply->skinnum;
				//anchor->s.frame = 2;  // beam size

				anchor->owner = grapple->owner;
				anchor->owner->grapple = anchor;
				anchor->grapple = grapple;
				grapple->owner = anchor;
				anchor->master = grapple->master;

				// Calc anchor-to-player length;
				anchor->enemy = anchor;
				VectorClear(anchor->offset);
				VectorSubtract(anchor->s.origin, anchor->owner->s.origin, olen);
				anchor->length = VectorNormalize(olen);


				anchor->classname = "grapple_anchorpoint";
				anchor->movetype = MOVETYPE_NONE;
				anchor->think = Grapple_Think;
				anchor->nextthink = level.time + FRAMETIME;
				VectorCopy (anchor->s.origin, anchor->pos2);
				VectorCopy (grapple->s.origin, anchor->pos1);

				gi.linkentity(anchor);

				break;
			}
		}
		ignore = tr.ent;
		VectorCopy(tr.endpos, start);
	}
	if (LineOfSight(point, start, clipmask))
	{
		VectorCopy (point, grapple->pos1);
		VectorCopy (start, grapple->pos2);
	}

	MCastTempEnt(grapple->supply->flags, grapple->owner, NULL, grapple->pos1, grapple->pos2, NULL, 0, 0, 0, MULTICAST_PVS);


	VectorCopy(tr.endpos, grapple->s.old_origin);
	grapple->nextthink = level.time + FRAMETIME;
	gi.linkentity(grapple);
}

void Grapple_DoMovement(edict_t *ent, pmove_t *pm) {
vec3_t vec;

	// Turn on prediction by default
	pm->s.pm_flags -= (pm->s.pm_flags & PMF_NO_PREDICTION);
	if (!ent->grapplethunk) {
		if (ent->grapple) {
			if (!ent->grapple->supply) {
				ent->grapple = NULL;
				return;
			}
			if ((ent->grapple->length >= 0) && (ent->grapple->length <= ent->grapple->supply->range)) {
				if (pm->cmd.upmove > 0) ent->grapple->length -= 30;
				if (pm->cmd.upmove < 0) ent->grapple->length += 30;
			}
			if (ent->grapple->length < 0) ent->grapple->length = 0;
			if (ent->grapple->length > ent->grapple->supply->range) ent->grapple->length = ent->grapple->supply->range;
			// disable client prediction when not on ground
			if (!ent->groundentity) {
				pm->s.pm_flags |= PMF_NO_PREDICTION;
			}
			pm->cmd.upmove = 0;

			ProjectClient (vec, ent, true);
			if (LineOfSight (vec, ent->grapple->s.origin, ent->grapple->clipmask))
				VectorCopy (vec, ent->grapple->pos1);
		}
		ent->grapplethunk = true;
	}
}

void Grapple_Use (edict_t *self, supply_t *type) {
edict_t		*anchor;
vec3_t		forward, right, up;
	
	if (self->deadflag)
		return;
	
	if (self->grapple) {
		Grapple_Release(self->grapple);
		return;
	}

	anchor = G_Spawn();
	anchor->solid = SOLID_BBOX;
	anchor->clipmask = MASK_SOLID;
	anchor->svflags &= ~SVF_NOCLIENT;
	if (type->model) {
		anchor->s.modelindex = gi.modelindex(type->model);
		anchor->s.skinnum = type->skinnum;
	}

	// Off-hand grapple shoots from the other hand
	AngleVectors (self->client->v_angle, forward, right, up);
	ProjectClient (anchor->s.origin, self, true);
	VectorScale(forward, 1500, anchor->velocity);

	vectoangles(forward, anchor->s.angles);
	VectorSet(anchor->mins, -8, -8, -8);
	VectorSet(anchor->maxs, 8, 8, 8);
	
	anchor->owner = self;
	anchor->classname = "grapple_hook";
	anchor->movetype = MOVETYPE_FLY;
	anchor->touch = Grapple_Anchor;
	anchor->think = Grapple_Think;
	anchor->supply = type;
	anchor->nextthink = level.time + FRAMETIME;
	anchor->master = self;

	self->grapple = anchor;
	gi.linkentity(anchor);

	// send muzzle flash
	if (type->s_use)
		gi.sound(self, CHAN_AUTO, gi.soundindex(type->s_use), 1, ATTN_NORM, 0);
}


// DL_Triggers

void Supply_Die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
	edict_t	*parent;

	parent = ent->family.parent;
	if (parent)
		RemoveChild(&parent->family, ent);
	parent = ent->sensors.parent;
	if (parent)
		RemoveChild(&parent->sensors, ent);

	G_FreeEdict(ent);
}

void TriggerThink_Timer(edict_t *trig) {
float tminus;
vec3_t	dir;

	if (!trig->supply) {
		gi.dprintf("ERROR: TriggerThink_Timer: Entity is not a supply!\n");
		G_FreeEdict(trig);
		return;
	}

	if (!trig->chain) {
		if (trig->target_ent || trig->supply->effect) {
			trig->chain = trig->target_ent;
			if (trig->supply->delay > 0.0)
				trig->timestamp = level.time + trig->supply->delay;
			else
				trig->timestamp = level.time + trig->delay;
		}
		trig->nextthink = level.time + FRAMETIME;
		return;
	}

	if (level.time > trig->pain_debounce_time) {
		gi.sound(trig, CHAN_AUTO, gi.soundindex("misc/beep.wav"), 1, ATTN_NORM, 0);
		tminus = trig->timestamp - level.time;
		if (tminus > 10.0)
			trig->pain_debounce_time = level.time + 5.0;
		else
			trig->pain_debounce_time = level.time + (tminus / 2.0);

		trig->master->client->ps.stats[STAT_TRIGGER] = gi.imageindex(trig->supply->pic_b);
		trig->master->client->trigger_time = trig->pain_debounce_time - 0.2;
	}

	if (level.time > trig->timestamp) {
		if (trig->target_ent && trig->target_ent->building && trig->target_ent->use)
			trig->target_ent->use(trig->target_ent, trig, trig); 

		if (!trig->inuse || !trig->supply && !trig->supply->effect)
			return;

		AngleVectors(trig->s.angles, dir, NULL, NULL);
		apply_effects(trig->supply->effect, NULL, trig, trig->master, trig->s.origin, trig->s.origin, dir, dir, (float)trig->supply->range, (float)trig->supply->damage, NULL);

		G_FreeEdict(trig);
		return;
	}
	trig->nextthink = level.time + FRAMETIME;
}

void TriggerThink_Trip(edict_t *trig) {
vec3_t	start, end, diff, dir;
edict_t *g=NULL;
float	len;
trace_t	tr;
int		i;

	if (!trig->supply) {
		gi.dprintf("ERROR: TriggerThink_Trip: Entity is not a supply!\n");
		G_FreeEdict(trig);
		return;
	}

	VectorAdd(trig->s.origin, trig->pos3, start);
	VectorCopy(trig->pos2, end);

	// only make wire visible to players in range of the tripwire
	if (trig->supply && trig->supply->flags) {
		if (random() > trig->supply->duration) {
			VectorSubtract(end, start, diff);
			len = VectorLength(diff);
			VectorNormalize(diff);
			VectorMA(start, len / 2, diff, diff);
			while (g = findradius(g, diff, len))
				if (g->client && g->client->pers.connected)
					UCastTempEnt(trig->supply->flags, trig, NULL, start, end, NULL, 0, 0, 0, g);
		}
	}

	tr = gi.trace(start, NULL, NULL, end, trig, MASK_SHOT);
	if ((tr.fraction < 1.0) && (tr.ent != trig->enemy)) {
		if (trig->target_ent && trig->target_ent->building && trig->target_ent->use)
			trig->target_ent->use(trig->target_ent, trig, tr.ent); 

		if (!trig->inuse || !trig->supply && !trig->supply->effect)
			return;

		AngleVectors(trig->s.angles, dir, NULL, NULL);
		apply_effects(trig->supply->effect, tr.ent, trig, trig->master, trig->s.origin, tr.ent->s.origin, dir, dir, (float)trig->supply->range, (float)trig->supply->damage, NULL);

		trig->touch_debounce_time = level.time + 0.5;

		trig->master->client->ps.stats[STAT_TRIGGER] = gi.imageindex(trig->supply->pic_b);
		trig->master->client->trigger_time = level.time + 1.0;

		// Can be retriggered?
		if (trig->supply->delay > 0.0) {
			trig->nextthink = level.time + trig->supply->delay;
			return;
		}

		G_FreeEdict(trig);
		return;
	}

	trig->enemy = tr.ent;
	trig->nextthink = level.time + FRAMETIME;

	// don't let the player get stuck inside the trigger
	if (trig->owner) {
		for (i = 0; i < 3; i++)	{
			if ((trig->master->absmin[i] > trig->absmax[i]) || (trig->master->absmax[i] < trig->absmin[i]))	{
				trig->owner = NULL;
				break;
			}
		}
	}
}

void TriggerThink_Proxy(edict_t *trig) {
edict_t	*other = NULL, *setter = NULL;
vec3_t	start, end, dir;
trace_t	tr;
team_t	*team;

	if (!trig->supply) {
		gi.dprintf("ERROR: TriggerThink_Proxy: Entity is not a supply!\n");
		G_FreeEdict(trig);
		return;
	}

	if ((!trig->target_ent && !trig->supply->effect) || (trig->flash_time > level.time)) {
		if (trig->flash_time > level.time)
			trig->nextthink = level.time + (trig->flash_time - level.time);
		else
			trig->nextthink = level.time + FRAMETIME;
		return;
	}

	AngleVectors(trig->s.angles, trig->movedir, NULL, NULL);
	VectorMA(trig->s.origin, 4.0, trig->movedir, start);

	do {
		other = findradius(other, start, trig->supply->range);

		if (!other)
			continue;

		if (!other->client && DL_strcmp(other->classname, "effect_drone", -1, false))
			continue;

		if (other == trig->enemy) {
			setter = other;
			continue;
		}

		if (!other->takedamage)
			continue;

		if (trig->target_ent)
			team = trig->target_ent->team;
		else
			team = trig->team;

		if (other->team == team)	// ignore enemy spies
			continue;

		if (other->client && (other->client->resp.team == team))	// ignore teammates
			continue;
		else if (other->team == team)
			continue;

		if (other->movetype == MOVETYPE_NOCLIP)
			continue;

		VectorCopy(other->s.origin, end);
		tr = gi.trace(start, NULL, NULL, end, trig, MASK_SHOT);

		if (tr.ent != other)
			continue;

		if (VectorLength(other->velocity) < 40.0)
			continue;

		break;

	} while(other);

	if (other) {
		trig->master->client->ps.stats[STAT_TRIGGER] = gi.imageindex(trig->supply->pic_b);
		trig->master->client->trigger_time = level.time + 1.0;

		if (trig->supply && trig->supply->s_use && trig->touch_debounce_time < level.time)
			gi.sound(trig, CHAN_AUTO, gi.soundindex(trig->supply->s_use), 1, ATTN_NORM, 0);

		if (trig->target_ent && trig->target_ent->building && trig->target_ent->use)
			trig->target_ent->use(trig->target_ent, trig, other);

		if (!trig->inuse || !trig->supply && !trig->supply->effect)
			return;

		AngleVectors(trig->s.angles, dir, NULL, NULL);
		apply_effects(trig->supply->effect, other, trig, trig->master, trig->s.origin, other->s.origin, dir, dir, (float)trig->supply->range, (float)trig->supply->damage, NULL);
		trig->touch_debounce_time = level.time + 0.5;
	}

	trig->enemy = setter;
	trig->nextthink = level.time + FRAMETIME;
}

void TriggerThink_Video(edict_t *trig) {
edict_t	*g, *setter=NULL, *targ=NULL;
float	l, targ_off=999.0;
vec3_t	start, dir, diff;
trace_t	tr;
team_t	*team;

	if (!trig->supply) {
		gi.dprintf("ERROR: TriggerThink_Video: Entity is not a supply!\n");
		G_FreeEdict(trig);
		return;
	}

	if ((!trig->target_ent && !trig->supply->effect) || (trig->flash_time > level.time)) {
		if (trig->flash_time > level.time)
			trig->nextthink = level.time + (trig->flash_time - level.time);
		else
			trig->nextthink = level.time + FRAMETIME;
		return;
	}

	AngleVectors(trig->s.angles, trig->movedir, NULL, NULL);
	VectorMA(trig->s.origin, 4.0, trig->movedir, start);

	// IR Sensors are light-sensative, and cannot be used in lit areas.
	if ((trig->supply->flags & SF_INFRARED) && (trig->light_level > trig->supply->impact_prot))
		trig->flash_time = level.time + (3.0 * ((trig->light_level / 255) * (trig->light_level / 255)));

	// Sensor is malfunctioning from a POV (Persistance of Vision) problem, usually from being blinded..
	if (trig->flash_time > level.time)
		return;

	for (g = &g_edicts[0]; g < &g_edicts[globals.num_edicts]; g++) {
		if (!g->inuse)
			continue;

		if (!g->client && !g->vid_decoy)
			continue;

		// Special handling for IR-Video Sensors
		if (trig->supply->flags & SF_INFRARED) {
			if (!(g->s.renderfx & RF_IR_VISIBLE))
				continue;	// Target is not emitting IR and so not visible
			if (g->light_level > trig->supply->impact_prot)
				return;	// Target is blinding IR and so it can't process image, safe-guards prevent the sensor for activating
		} else {
			// Normal rules for Video (Non-IR) Sensors
			if (g->light_level < trig->supply->impact_prot)
				continue;
			if (g->in_smoke)
				continue;
		}

		if (g == trig)
			continue;

		if (!g->takedamage)
			continue;

		if (g->deadflag)
			continue;

		if (g->movetype == MOVETYPE_NOCLIP)
			continue;

		tr = gi.trace(start, NULL, NULL, g->s.origin, trig, MASK_OPAQUE);
		if ((g != tr.ent) && (tr.fraction != 1.0))
			continue;

		if (InFOV(start, trig->s.angles, g->s.origin, trig->supply->range, true)) {
			if (g == trig->enemy) {
				setter = g;
				continue;
			}
			if (trig->target_ent)
				team = trig->target_ent->team;
			else
				team = trig->team;
			if (g->client) {
				if (g->team == team)	// Ignore enemy spies
					continue;
				if (g->client->resp.team == team)	// ignore teammates
					continue;
			} else if (g->team == team)	// Ignore friendly decoys
				continue;

			
			VectorSubtract(g->s.origin, start, diff);
			vectoangles(diff, dir);
			AnglesDiff(trig->s.angles, dir, diff);

			if ((l = VectorLength(diff)) < targ_off) {
				targ = g;
				targ_off = l;
			}
		}
	}

	if (targ) {
		// Activate targets
		if (trig->target_ent && trig->target_ent->building && trig->target_ent->use)
			trig->target_ent->use(trig->target_ent, trig, targ);

		if (!trig->inuse || !trig->supply && !trig->supply->effect)
			return;

		// Notify owner
		trig->master->client->ps.stats[STAT_TRIGGER] = gi.imageindex(trig->supply->pic_b);
		trig->master->client->trigger_time = level.time +  1.0;

		// Execute local effect
		AngleVectors(trig->s.angles, dir, NULL, NULL);
		apply_effects(trig->supply->effect, targ, trig, trig->master, trig->s.origin, targ->s.origin, dir, dir, (float)trig->supply->range, (float)trig->supply->damage, NULL);

		if (!trig->inuse || !trig->supply)
			return;

		// set retrigger debounce
		trig->touch_debounce_time = level.time + 0.5;
	}

	trig->enemy = setter;
	trig->nextthink = level.time + FRAMETIME;
}

edict_t *SetTrigger(edict_t *owner, edict_t *anchor, vec3_t pos, vec3_t dir, edict_t *target, vec3_t endpos, supply_t *type) {
edict_t *trig;


	if (owner->client && (RemoveInventoryItem(owner, FindInventoryItem(owner, type), 1) == NULL))
		return NULL;

	trig = G_Spawn();

	// Trigger attached to a building?
	if (target && anchor->building) {
		if (anchor == target) {	// Find top-most target
			while (target->family.parent && ((edict_t *)target->family.parent)->building)
				target = target->family.parent;
		}
		VectorRotate(anchor->building->trigger_offset, anchor->s.angles, trig->pos2);
		VectorAdd(anchor->s.origin, trig->pos2, trig->s.origin);
		VectorAdd(anchor->s.angles, anchor->building->trigger_angles, trig->s.angles);
		AngleVectors(trig->s.angles, trig->movedir, NULL, NULL);
	} else {
		if (!pos || !dir)
			return NULL;
		VectorCopy(pos, trig->s.origin);
		vectoangles(dir, trig->s.angles);
		VectorCopy(dir, trig->movedir);
	}

	VectorSet(trig->mins, -8, -8, 0);
	VectorSet(trig->maxs, 8, 8, 4);

	trig->master = owner;
	trig->enemy = owner; // Ignore this person when first setting trigger
	trig->owner = owner; // Don't let player get stuck on the trigger
	trig->target_ent = target;
	trig->classname = "trigger";
	trig->movetype = MOVETYPE_NONE;
	trig->solid = SOLID_BBOX;
	trig->team = owner->client->resp.team;
	trig->supply = type;
	trig->s.renderfx = RF_IR_VISIBLE;

	if (target)
	{
		trig->sensors.parent = target;
		AddChild(&target->sensors, trig);
	}
	if (type->model) {
		trig->s.modelindex = gi.modelindex(type->model);
		trig->s.skinnum = type->skinnum;
		trig->s.frame = type->frame;
	}
	if (type->health) {
		trig->takedamage = DAMAGE_YES;
		trig->health = type->health;
		trig->die = Supply_Die;
	}

	switch (type->type) {
	case SUP_TIMER:
		trig->pain_debounce_time = 0.0;
		trig->delay = atoi(gi.argv(2));
		trig->nextthink = level.time + FRAMETIME;
		trig->think = TriggerThink_Timer;
		break;
	case SUP_TRIPWIRE:
		VectorCopy(endpos, trig->pos2);
		trig->nextthink = level.time + FRAMETIME;
		trig->think = TriggerThink_Trip;
		VectorRotate(type->offset, trig->s.angles, trig->pos3);
		break;
	case SUP_PROXY:
		trig->nextthink = level.time + FRAMETIME;
		trig->think = TriggerThink_Proxy;
		break;
	case SUP_VIDSENSOR:
		trig->nextthink = level.time + FRAMETIME;
		trig->think = TriggerThink_Video;
		break;
	default:
		G_FreeEdict(trig);
		return NULL;
	}

	if (anchor) {
		stick(trig, anchor);
		AddChild(&anchor->family, trig);
		trig->family.parent = anchor;
	}
	gi.linkentity(trig);

	return trig;
}

void Timer_Use(edict_t *ent, supply_t *type) {
edict_t *trig;
vec3_t	start, end, forward;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 48, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if (!(tr.fraction < 1.0) || (tr.surface->flags & (SURF_SKY|SURF_SLICK))) {
		gi.cprintf(ent, PRINT_HIGH, "Nothing to attach timer too.\n");
		return;
	}
	if (tr.ent && tr.ent->client) {
		gi.cprintf(ent, PRINT_HIGH, "Can not attach timer to players.\n");
		return;
	}

	if (tr.ent && tr.ent->use && tr.ent->building) {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, tr.ent, NULL, type);
		if (tr.ent->building)
			gi.cprintf(ent, PRINT_HIGH, "%s set to trigger \"%s\"\n", trig->supply->name, tr.ent->message);
	} else {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, NULL, NULL, type);
		if (!type->effect)
			PMenu_Open(ent, ID_SELECTOBJECTS, 0, (void *)trig);
	}

	if (ent->client)
		ent->client->svc_time = level.time + 2.0;
}

void TripWire_Think(edict_t *ent) {
vec3_t	start, end;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	VectorCopy(ent->master->s.origin, ent->s.old_origin);
	VectorCopy(ent->s.old_origin, end);

	tr = gi.trace(start, NULL, NULL, end, ent->master, MASK_SHOT);
	if (tr.fraction < 1.0) {
		G_FreeEdict(ent);
		return;
	}

	if (ent->supply && ent->supply->flags)
		MCastTempEnt(ent->supply->flags, ent, NULL, start, end, NULL, 0, 0, 0, MULTICAST_PVS);
	ent->nextthink = level.time + FRAMETIME;
}

void TripWire_Use (edict_t *ent, supply_t *type) {
edict_t *trip = NULL, *trig;
vec3_t	start, end, forward;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 48, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.ent && tr.ent->client) {
		gi.cprintf(ent, PRINT_HIGH, "Can not attach Tripwire to players.\n");
		return;
	}

	if (!(tr.fraction < 1.0) || (tr.surface->flags & (SURF_SKY|SURF_SLICK))) {
		gi.cprintf(ent, PRINT_HIGH, "Nothing to attach tripwire too.\n");
		do {
			trip = G_Find(trip, FOFS(classname), "tripwire");
			if (trip && trip->master == ent) {
				G_FreeEdict(trip);
				return;
			}
		} while (trip);
	} else {
		do {
			trip = G_Find(trip, FOFS(classname), "tripwire");
			if (trip && trip->master == ent) {
				// Set the trigger and attach the trigger to the box
				if (trip->target_ent && trip->target_ent->use && trip->target_ent->building) {
					trig = SetTrigger(ent, trip->target_ent, trip->s.origin, trip->movedir, trip->target_ent, tr.endpos, type);
					if (tr.ent->building)
					gi.cprintf(ent, PRINT_HIGH, "%s set to trigger \"%s\"\n", trig->supply->name, tr.ent->message);
				} else {
					trig = SetTrigger(ent, trip->target_ent, trip->s.origin, trip->movedir, NULL, tr.endpos, type);
					if (!type->effect)
						PMenu_Open(ent, ID_SELECTOBJECTS, 0, (void *)trig);
				}

				G_FreeEdict(trip);
				return;
			}
		} while (trip);

		trip = G_Spawn();
		VectorCopy(tr.endpos, trip->s.origin);
		VectorCopy(tr.plane.normal, trip->movedir);
		vectoangles(tr.plane.normal, trip->s.angles);

		trip->master = ent;
		trip->svflags &= ~SVF_NOCLIENT;
		trip->classname = "tripwire";
		trip->movetype = MOVETYPE_NONE;
		trip->solid = SOLID_NOT;
		trip->target_ent = tr.ent;
		trip->supply = type;
		if (type->model) {
			trip->s.modelindex = gi.modelindex(type->model);
			trip->s.skinnum = type->skinnum;
			trip->s.frame = type->frame;  // beam size
		}

		trip->nextthink = level.time + FRAMETIME;
		trip->think = TripWire_Think;


		gi.linkentity(trip);
	}
}

void Proxy_Use(edict_t *ent, supply_t *type) {
edict_t *trig;
vec3_t	start, end, forward;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 48, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if (!(tr.fraction < 1.0) || (tr.surface->flags & (SURF_SKY|SURF_SLICK))) {
		gi.cprintf(ent, PRINT_HIGH, "Nothing to attach motion sensor too.\n");
		return;
	}
	if (tr.ent && tr.ent->client) {
		gi.cprintf(ent, PRINT_HIGH, "Can not attach motion sensor to players.\n");
		return;
	}

	if (tr.ent && tr.ent->use && tr.ent->building) {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, tr.ent, NULL, type);
		if (tr.ent->building)
			gi.cprintf(ent, PRINT_HIGH, "%s set to trigger \"%s\"\n", trig->supply->name, tr.ent->message);
	} else {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, NULL, NULL, type);
		if (!type->effect)
			PMenu_Open(ent, ID_SELECTOBJECTS, 0, (void *)trig);
	}
}

void VidSensor_Use(edict_t *ent, supply_t *type) {
edict_t *trig;
vec3_t	start, end, forward;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 48, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if (!(tr.fraction < 1.0) || (tr.surface->flags & (SURF_SKY|SURF_SLICK))) {
		gi.cprintf(ent, PRINT_HIGH, "Nothing to attach video sensor too.\n");
		return;
	}
	if (tr.ent && tr.ent->client) {
		gi.cprintf(ent, PRINT_HIGH, "Can not attach video sensor to players.\n");
		return;
	}

	if (tr.ent && tr.ent->use && tr.ent->building) {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, tr.ent, NULL, type);
		if (tr.ent->building)
			gi.cprintf(ent, PRINT_HIGH, "%s set to trigger \"%s\"\n", trig->supply->name, tr.ent->message);
	} else {
		trig = SetTrigger(ent, tr.ent, tr.endpos, tr.plane.normal, NULL, NULL, type);
		if (!type->effect)
			PMenu_Open(ent, ID_SELECTOBJECTS, 0, (void *)trig);
	}
}

void ReclaimTrigger(edict_t *player, edict_t *trig) {
	if (!trig->supply || !(trig->supply->type & (SUP_TIMER|SUP_TRIPWIRE|SUP_PROXY|SUP_VIDSENSOR)) ) {
		gi.cprintf(player, PRINT_HIGH, "Item is not a trigger.\n");
		return;
	}
	if (trig->master == player || (trig->target_ent && (trig->target_ent->master == player)) ) {
		AddToInventory(player, ITEM_SUPPLY, trig->supply, 0, 1, 0, NULL, NULL);
		G_FreeEdict(trig);
		return;
	}
	gi.cprintf(player, PRINT_HIGH, "That trigger does not belong to you.\n");
}

// DeadLode Grenades

void Grenade_Detonate(edict_t *ent) {
vec3_t	norm, start, end;
trace_t tr;

	VectorClear(norm);
	VectorCopy(ent->s.origin, start);
	start[2] += 4.0;
	VectorCopy(start, end);
	end[2] -= 16.0;

	tr = gi.trace(ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
	if (tr.fraction < 1.0)
		VectorCopy(tr.plane.normal, norm);

	if (ent->master) {
		if (ent->master->grenade == ent) {
			ent->master->grenade = NULL;
			ent->master->client->weapon_sound = 0;
			RemoveInventoryItem(ent->master, FindInventoryItem(ent->master, ent->supply), 1); 
			AngleVectors(ent->master->s.angles, end, NULL, NULL);
			VectorMA(ent->master->s.origin, 8, end, ent->s.origin);
		}
		apply_effects(ent->supply->effect, NULL, ent, ent->master, ent->s.origin, ent->s.origin, NULL, norm, ent->supply->damage, ent->supply->range, NULL);
	}

	G_FreeEdict(ent);
	return;
}

void Grenade_Bounce (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
float rnd;
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)	{
		rnd = random();
		if (rnd < 0.33)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		else if (rnd < 0.66)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		else 
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
	}
	VectorSet(ent->avelocity, crandom() * 300, crandom() * 300, crandom() * 300);
}


// Using a grenade once, primes it, using ANY grenade while one is primed,
// will throw the primed grenade.
void Grenade_Use (edict_t *ent, supply_t *type) {
edict_t *gren;
vec3_t	forward, up;

	if (!ent)
		return;

	if (ent->grenade) {

		if (ent->client)
			ent->client->svc_time = level.time + 2.0;

		gren = ent->grenade;
		ent->grenade = NULL;
		RemoveInventoryItem(ent, FindInventoryItem(ent, gren->supply), 1); 

		if (gren->timestamp <= level.time) {
			ent->client->weapon_sound = 0;
			Grenade_Detonate(gren);
			return;
		}

		VectorCopy(ent->s.origin, gren->s.origin);
		AngleVectors(ent->client->v_angle, forward, NULL, up);
		VectorCopy(ent->velocity, gren->velocity);
		VectorMA(gren->velocity, 500, forward, gren->velocity);
		gren->velocity[2] += 300; // Give extra upward push (lob?) for more airtime.
		VectorSet(gren->avelocity, crandom() * 300, crandom() * 300, crandom() * 300);
		gren->movetype = MOVETYPE_BOUNCE;
		gren->clipmask = MASK_SHOT;
		gren->solid = SOLID_BBOX;
		gren->classname = "hand_grenade";

		ent->client->weapon_sound = 0;
		gren->s.sound = 0;

		if (gren->supply->model) {
			gren->touch = Grenade_Bounce;
			gren->s.modelindex = gi.modelindex(gren->supply->model);
			gren->s.skinnum = gren->supply->skinnum;
			gren->s.frame = gren->supply->frame;
			gren->s.renderfx |= RF_IR_VISIBLE;
		}

		gren->nextthink = gren->timestamp;

		gi.linkentity(gren);
		return;
	}

	if (!type)
		return;

	gren = G_Spawn();
	gren->owner = ent;
	gren->master = ent;
	gren->classname = "primed_grenade";
	gren->movetype = MOVETYPE_NONE;
	gren->solid = SOLID_NOT;
	if (ent->client)
		gren->team = ent->client->resp.team;
	else
		gren->team = ent->team;
	gren->supply = type;
	gren->timestamp = level.time + type->delay;
	gren->think = Grenade_Detonate;
	gren->nextthink = gren->timestamp;

	gi.cprintf(ent, PRINT_MEDIUM, "%s primed.\n", type->name);
	ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
	if (type->s_use)
		gi.positioned_sound(ent->s.origin, ent, CHAN_WEAPON, gi.soundindex(type->s_use), 1, 1, 0);
	
	ent->grenade = gren;
}

void MsgJammer_Use(edict_t *ent, supply_t *type) {
edict_t *jam;
vec3_t	start, end, forward;
trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 48, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);
	if (!(tr.fraction < 1.0)) {
		gi.cprintf(ent, PRINT_HIGH, "Nothing to attach Jammer to.\n");
		return;
	}

	jam = G_Spawn();
	VectorCopy(tr.endpos, jam->s.origin);
	VectorCopy(tr.plane.normal, jam->movedir);
	vectoangles(tr.plane.normal, jam->s.angles);
	VectorSet(jam->mins, -4, -4, 0);
	VectorSet(jam->maxs, 4, 4, 4);

	jam->master = ent;
	jam->svflags &= ~SVF_NOCLIENT;
	jam->classname = "msg_jammer";
	jam->movetype = MOVETYPE_NONE;
	jam->solid = SOLID_BBOX;
	jam->target_ent = tr.ent;
	jam->supply = type;

	if (ent->client)
		jam->team = ent->client->resp.team;
	else
		jam->team = ent->team;

	if (type->health) {
		jam->takedamage = DAMAGE_YES;
		jam->health = type->health;
		jam->die = Supply_Die;
	}

	if (type->model) {
		jam->s.modelindex = gi.modelindex(type->model);
		jam->s.skinnum = type->skinnum;
		jam->s.frame = type->frame;  // beam size
	}

	stick(jam, tr.ent);

	gi.linkentity(jam);
	RemoveInventoryItem(ent, FindInventoryItem(ent, type), 1); 
}

void Tool_Use(edict_t *ent, supply_t *type) {
edict_t *bldg;
vec3_t	start;
int		i, j, k, pts, pts_avail, pts_inuse = 0, pl_pts = 0;

	if (!type) {
		gi.cprintf(ent, PRINT_HIGH, "No tool selected.\n");
		return;
	}

	if (ent->deadflag || (ent->client->svc_time > level.time))
		return;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	bldg = FindInFOV(start, ent->client->v_angle, 90, 64);
	if (!bldg || !bldg->building) {
		PMenu_Open(ent, ID_CONSTRUCTION, 0, NULL);
		return;
	}

	if (ent->client->building) {
		gi.cprintf(ent, PRINT_HIGH, "You are currently building an %s.\n", ent->client->building->building->name);
		return;
	}


	// Check to see if this tool can service the targeted building
	if (!(type->flags & bldg->building->req_tools)) {
		gi.cprintf(ent, PRINT_HIGH, "This tool cannot service a %s\n", bldg->building->name);
		return;
	}

	if ((type->tool & TOOL_REPAIR) || (type->tool & TOOL_DISMANTLE)) {
		pts_avail = ent->client->bldg_pts - CheckBuildingPoints(ent);

		// Check to see if this building is on the service-list for this player
		for (i=0, j=-1; j<0 && i<8; i++)
			if (ent->client->resp.bldgs[i] == bldg)
				j = i;

		// If not, check to see if the player has room on thier service-list for the building
		for (i=0; j<0 && i<8; i++)
			if (!ent->client->resp.bldgs[i] || !ent->client->resp.bldgs[i]->inuse)
				j = i;

		// If not, there is no room, the player can't service the building
		if (j<0) {
			gi.cprintf(ent, PRINT_HIGH, "You can't service any more buildings.\n");
			return;
		}

		// check to see if the player is on the buildings maintainer list
		for (i=0, k=-1; k<0 && i<4; i++)
			if (bldg->builders[i] == ent->client)
				k = i;
		// if not, check to see if the building has room on its maintainer list for the player
		for (i=0; k<0 && i<8; i++)
			if (!bldg->builders[i] || !bldg->builders[i]->pers.connected)
				k = i;

		// link player and building to each other
		ent->client->resp.bldgs[j] = bldg;
		bldg->builders[k] = ent->client;
		bldg->bldr_index[k] = j;

		if (type->tool & TOOL_DISMANTLE) {
			pts = ent->client->resp.bldg_pts[j];
			if (pts <= 0) {
				gi.cprintf(ent, PRINT_HIGH, "This %s doesn't belong to you\n", bldg->building->name);
				return;
			}
			if (pts > type->health) pts = type->health;
			ent->client->resp.bldg_pts[j] -= pts;
			bldg->health -= pts;
		} else {
			if (pts_avail <= 0) {
				gi.cprintf(ent, PRINT_HIGH, "You don't have enough points to service this %s\n", bldg->building->name);
				return;
			}
			if (bldg->health >= bldg->building->max_points)
				return;

			pts = type->health;
			if (!pts) pts = 5;
			if (pts > pts_avail) pts = pts_avail;
			if (bldg->health + pts > bldg->building->max_points)
				pts = bldg->building->max_points - bldg->health;
			ent->client->resp.bldg_pts[j] += pts;
			bldg->health += pts;

			if (bldg->health >= bldg->building->min_points)
				bldg->state = BLDG_STATE_ONLINE;
		}
		ent->client->svc_time = level.time + type->duration;
		ent->client->bonus_alpha = 0.25;
		if (type->s_use) 
			gi.sound(ent, CHAN_ITEM, gi.soundindex(type->s_use), 1, ATTN_NORM, 0);
	} else {
//		ent->client->building = bldg;
//		tmp = MakeMenuItem(type->name, NULL, NULL, PMENU_ALIGN_LEFT, type, NULL, 0, 0);
//		ShowToolMenu(ent, &tmp);
	}
}

void SmartScope_Think(edict_t *ent) {
int pnum, dist, maxFocus, range; 
trace_t tr; 
vec3_t diff, start, end; 
	
	ent->client->ps.stats[STAT_ID_NAME] = 0; 
	ent->client->ps.stats[STAT_ID_TEAM] = 0; 
	ent->client->ps.stats[STAT_ID_RANGE] = 0; 
	ent->client->ps.stats[STAT_ID_HITLOC] = gi.imageindex("sights/x_idle");
	ent->client->ps.stats[STAT_ID_WEAPON] = 0; 

	ent->client->ps.fov = ent->scope->range;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, end, NULL, NULL);
	VectorMA(start, 2000, end, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT); 
	if ((tr.ent->client) && (tr.ent->light_level > 16)) { 
		pnum = tr.ent-g_edicts-1;
		VectorSubtract(start, tr.endpos, diff); 
		dist = VectorLength(diff) / 30;
		maxFocus = (((7500.0/ent->scope->range) * 50.0) / 4.0) / 30.0;
		range = ((float)maxFocus / (float)dist) * 100;

		if (tr.ent->light_level > 32) {
			if ((ent->scope->flags & SF_ID_WEAPON) && ent->client->curweap && (range < 35))
				ent->client->ps.stats[STAT_ID_WEAPON] = CS_WEAPONS + WEAP_INDEX(ITEMWEAP(tr.ent->client->curweap));

			if ((ent->scope->flags & SF_ID_TEAM) && (range < 65))
				ent->client->ps.stats[STAT_ID_TEAM] = gi.imageindex(tr.ent->team->pic_b); // ID reports spies as undercover

			if (ent->scope->flags & SF_ID_IFF) {
				if (tr.ent->team == ent->team)
					ent->client->ps.stats[STAT_ID_HITLOC] = gi.imageindex("sights/x_no"); 
				else if (tr.ent->team != ent->team)
					ent->client->ps.stats[STAT_ID_HITLOC] = gi.imageindex("sights/x_ok1"); 
			}
		}

		if ((ent->scope->flags & SF_ID_NAME) && (dist < 30)) {
			ent->oldenemy = tr.ent;
			ent->oldenemy_time = level.time + 1.0;
			ent->client->ps.stats[STAT_ID_NAME] = CS_PLAYER_NAMES + pnum; 
		}

		if (ent->scope->flags & SF_ID_RANGE)
			ent->client->ps.stats[STAT_ID_RANGE] = dist; 

	} else if (ent->oldenemy && (ent->oldenemy_time > level.time)) {
		pnum = ent->oldenemy-g_edicts-1;
		ent->client->ps.stats[STAT_ID_NAME] = CS_PLAYER_NAMES + pnum; 
	}

	// Better night-vision alternate..
	if (ent->scope->flags & SF_ID_STARLIGHT)
		ent->client->night_vision = true;
} 

void SmartScope_Use(edict_t *ent, supply_t *type) {
	if (ent->scope)
		ent->scope = NULL;
	else
		ent->scope = type;
}


// Check to see if the player should loose his disguise.
// This function should be called anytime the player is disguided and shoots an enemy target
// Basically check to see if anyone saw him shoot the target
void CheckDisguise(edict_t *ent) {
edict_t *g = NULL;

	for (g = &g_edicts[1]; g <= &g_edicts[(int)maxclients->value]; g++) {
		if (!g->inuse || !g->client)
			continue;
		if (g->client->resp.team != ent->team)	// only check the team the player is posing as
			continue;
		if (InFOV(g->s.origin, g->client->v_angle, ent->s.origin, ent->client->ps.fov, true)) {
			ent->team = ent->client->resp.team;
			DL_AssignSkin(ent, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
			// Loose the disguise, but don't inform the player that someone saw him...
		}
	}
}

void Uniform_Use(edict_t *ent, supply_t *type) {
	if (ent->goalitem) {
		gi.centerprintf(ent, "You cannot use a uniform while\ncarrying a goal item.\n");
		return;
	}

	PMenu_Open(ent, ID_SELECTUNIFORM, 0, (void *)type);
}

void Medikit_Think(edict_t *med) {

	med->nextthink = level.time + FRAMETIME;

	if ((med->timestamp < level.time) || (!med->random)) { //  || ((int)med->angle != med->owner->health)) {
		med->owner->medikit = NULL;
		G_FreeEdict(med);
		return;
	}

	if ((int)med->angle != med->owner->health)
		med->angle = med->owner->health;

	if (VectorLength(med->owner->velocity) > 50)
		// player movement causes medikit to be 1/2 as effective.
		med->angle += (med->random / 2.0);
	else
		med->angle += med->random;
	med->owner->health = med->angle;

	if (med->owner->health >= med->owner->max_health) {
		med->owner->health = med->owner->max_health;
		med->owner->medikit = NULL;
		G_FreeEdict(med);
		return;
	}
}

void Medikit_Use(edict_t *ent, supply_t *type) {
edict_t *med;

	if (ent->medikit)	// Already using a medikit.
		return;

	RemoveInventoryItem(ent, FindInventoryItem(ent, type), 1); 

	med = G_Spawn();
	med->classname = "medikit_timer";
	med->owner = ent;
	med->supply = type;
	med->solid = SOLID_NOT;
	med->timestamp = level.time + type->duration;
	med->angle = ent->health;
	med->random = ((ent->max_health - ent->health) * type->health) / (type->duration * 10.0); // convert to frames
	med->think = Medikit_Think;

	ent->medikit = med;

	Medikit_Think(med);
}

void Supply_Use(edict_t *ent, supply_t *sup) {

	if (ent->deadflag)
		return;

	// Don't let players activate supplies too fast...
	if (ent->client && (ent->client->svc_time > level.time))
		return;

	switch (sup->type) {
	case SUP_FLASHLIGHT:
		FlashLight_Use(ent, sup);
		break;
	case SUP_GRAPPLE:
		Grapple_Use(ent, sup);
		break;
	case SUP_TIMER:
		Timer_Use(ent, sup);
		break;
	case SUP_TRIPWIRE:
		TripWire_Use(ent, sup);
		break;
	case SUP_PROXY:
		Proxy_Use(ent, sup);
		break;
	case SUP_VIDSENSOR:
		VidSensor_Use(ent, sup);
		break;
	case SUP_GRENADE:
		Grenade_Use(ent, sup);
		break;
	case SUP_MSGJAMMER:
		MsgJammer_Use(ent, sup);
		break;
	case SUP_TOOL:
		Tool_Use(ent, sup);
		break;
	case SUP_ARMOUR:
//		Armour_Use(ent, type);
		break;
	case SUP_SIGHT:
		SmartScope_Use(ent, sup);
		break;
	case SUP_UNIFORM:
		Uniform_Use(ent, sup);
		break;
	case SUP_MEDIKIT:
		Medikit_Use(ent, sup);
		break;
	default:
		gi.cprintf(ent, PRINT_HIGH, "SUPPLY: %s: Supply references unknown type.\n", sup->name);
		break;
	}
}

void UseSupply(edict_t *ent, char *name) {
item_t	 *item;

	if (!ent)
		return;

	item = FindItemByName(ent, name, ITEM_SUPPLY);
	if (!item) {
		gi.cprintf(ent, PRINT_HIGH, "You don't have any %s's in your inventory.\n", name);
		return;
	}

	Supply_Use(ent, (supply_t *)(item->item));
}

void Radio_Tune(edict_t *ent, int channel) {
int rval, snd;

	ent->radio_channel = channel;

	rval = (int)(random()*2);
	switch (rval) {
	case 0:		snd = gi.soundindex("radio/squelch1.wav"); break;
	case 1:		snd = gi.soundindex("radio/squelch2.wav"); break;
	default:	snd = gi.soundindex("radio/squelch3.wav"); break;
	}

	unicastSound(ent, snd, 1.0);
}

qboolean IsMsgJammed(edict_t *ent) {
edict_t *g = NULL;
float dist, jammed = 0.0;
vec3_t diff;
trace_t tr;

	while (g = G_Find(g, FOFS(classname), "msg_jammer")) {
		if (ent->client) {
			if (g->team == ent->client->resp.team)
				continue;
		} else {
			if (g->team == ent->team)
				continue;
		}

		VectorSubtract(ent->s.origin, g->s.origin, diff);
		dist = VectorLength(diff);
		if (dist < g->supply->range) {
			jammed = 1.5 - (dist / g->supply->range);
			tr = gi.trace(ent->s.origin, NULL, NULL, g->s.origin, ent, MASK_SOLID); // different/better mask?
			if (tr.fraction < 1.0)
				jammed *= 0.5;

			if (random() < jammed)
				return true;
		}
	}
	return false;
}

void Radio_AddToQue(edict_t *ent, char *msg) {
radio_t	*rMsg;

	rMsg = gi.TagMalloc(sizeof(radio_t), TAG_LEVEL);
	memset(rMsg, 0, sizeof(radio_t));

	if (!msg || !*msg) {
		rMsg->len = 0.3;	// null/blank is 0.3 second silence
		rMsg->msg = NULL;
	} else {
		rMsg->len = (((int)(GetWAVLength(ent, msg)*10.0)) / 10.0) - 0.1;	// round to frames
		rMsg->msg = ED_NewString(msg);
//		gi.dprintf("AddToQue: %s\n", rMsg->msg);
	}

	rMsg->prev = NULL;
	rMsg->next = ent->radio_que;
	if (ent->radio_que)
		ent->radio_que->prev = rMsg;

	ent->radio_que = rMsg;
}

void Radio_PlayQue(edict_t *ent, radio_t *rMsg) {
edict_t *g = NULL;
team_t	*dest_team = NULL;
float which, vol;
int i = 0;

	if (!rMsg)
		return;

	rMsg->playing = true;
	rMsg->expire = level.time + rMsg->len;

	if (!rMsg->msg)
		return;

//	gi.dprintf("PlayQue: %s\n", rMsg->msg);
	if (IsMsgJammed(ent)) {
		// let the sender hear his jammed message (static)
		which = random();
		if (which < 0.3) unicastSound(ent, gi.soundindex("radio/jammed1.wav"), 1.0);
		else if (which < 0.6) unicastSound(ent, gi.soundindex("radio/jammed2.wav"), 1.0);
		else unicastSound(ent, gi.soundindex("radio/jammed3.wav"), 1.0);
		return;
	}

	if (ent->client) {
		dest_team = ent->client->resp.team;
		vol = 1.0;	// team messages at full volume
	} else {
		dest_team = ent->team;
		vol = 0.5;	// Decoded messages are much quieter
	}

	for (g=&g_edicts[1]; g < &g_edicts[globals.num_edicts]; g++) {
		if (g->client && g->client->pers.connected) {
			if (!dest_team || (dest_team == g->client->resp.team)) {
				if (ent->radio_channel == g->radio_channel) {
					if (ent->client && IsMsgJammed(g)) {	// only clients trasmitions are jammed
						which = random();
						if (which < 0.3) unicastSound(ent, gi.soundindex("radio/jammed1.wav"), 1.0);
						else if (which < 0.6) unicastSound(ent, gi.soundindex("radio/jammed2.wav"), 1.0);
						else unicastSound(ent, gi.soundindex("radio/jammed3.wav"), 1.0);
					} else
						unicastSound(g, gi.soundindex(rMsg->msg), vol);
				}
			}
		// Radio messages can also be passed on to enemy decoders, assuming they were not jammed at transmittion 
		} else if (ent->client) {
			if (!DL_strcmp(g->classname, "msg_decoder", -1, false) && g->use && (g->team != dest_team)) {
				if ((ent->radio_channel == g->radio_channel) && !IsMsgJammed(g)) {
					g->radio_msg = rMsg->msg;
					g->message1 = NULL;
					g->use(g, ent, ent);
				}
			}
		}
	}
}

void Radio_CheckQue(edict_t *ent) {
radio_t *rMsg;

	if (!ent->radio_que)
		return;

	if (!ent->radio_curmsg)
		for (rMsg = ent->radio_que; rMsg; rMsg = rMsg->next)
			ent->radio_curmsg = rMsg;

	if (ent->think == Radio_CheckQue)
		ent->nextthink = level.time + FRAMETIME;

	if (ent->radio_curmsg->expire > level.time)
		return;

	if (ent->radio_curmsg->playing) {
		rMsg = ent->radio_curmsg;
		ent->radio_curmsg = ent->radio_curmsg->prev;
		if (ent->radio_curmsg)
			ent->radio_curmsg->next = NULL;

		if (rMsg->msg)
			gi.TagFree(rMsg->msg);
		gi.TagFree(rMsg);
	}

	if (ent->radio_curmsg) {
		ent->radio_curmsg->next = NULL;
		Radio_PlayQue(ent, ent->radio_curmsg);
	} else
		ent->radio_que = NULL;
}

void Radio_Use(edict_t *ent, char *msg) {
char wav[MAX_QPATH], filename[MAX_QPATH], *p, *st;

	if (!msg)
		return;

	if (!msg[0]) {
		PMenu_Open(ent, ID_RADIOMESSAGES, 0, NULL);
		return;
	}

	// starting squelch
	if (!ent->client && !(ent->spawnflags & 0x04))
		Radio_AddToQue(ent, "*radio/dl_squelch.wav");
	// parse message string for multiple filenames
	for (st=p=msg;(p=strstr(p," ")) != NULL; p++) {
		DL_strcpy(wav, st, (int)(p-st));
		st = p+1;

		if (wav[0]) {
			// Make sure it ends in ".wav"
			if (!strstr(wav, ".wav"))
				strcat(wav, ".wav");

			if ((ent->client) || (!DL_strcmp(ent->classname, "dl_target_radio", -1, false) && !(ent->spawnflags & 0x02)))
				sprintf(filename, "*radio/%s", wav);
			else DL_strcpy(filename, wav, -1);
			Radio_AddToQue(ent, filename);
		} else
			Radio_AddToQue(ent, NULL);	// Insert 0.3sec of silence
	}
	if (*st) {
		DL_strcpy(wav, st, -1);
		// Make sure it ends in ".wav"
		if (!strstr(wav, ".wav"))
			strcat(wav, ".wav");

		if ((ent->client) || (!DL_strcmp(ent->classname, "dl_target_radio", -1, false) && !(ent->spawnflags & 0x02)))
			sprintf(filename, "*radio/%s", wav);
		else DL_strcpy(filename, wav, -1);

		Radio_AddToQue(ent, filename);
	}
	// ending squelch
	if (!ent->client && !(ent->spawnflags & 0x04))
		Radio_AddToQue(ent, "*radio/dl_squelch2.wav");
}

void Radio_Chat(edict_t *ent, char *msg) {
team_t	*dest_team = NULL;
edict_t *g = NULL;
char jammed[1400], *from;
int i;


	if (!msg)
		return;

	// make a scrambled version of the original message
	for(i=0; msg[i]; i++) {
		if (msg[i] != ' ')
			jammed[i] = (random() * 96) + 32;
		else
			jammed[i] = ' ';
	}
	jammed[i] = 0;

	if (IsMsgJammed(ent)) {
		// let the sender hear his jammed message (static)
		gi.cprintf(ent, PRINT_CHAT, "(%s): %s\n", ent->client->pers.netname, jammed);
		return;
	}

	if (ent->client) {
		dest_team = ent->client->resp.team;
		from = ent->client->pers.netname;
	} else {
		// Radio Decoder or other "message generating device"
		dest_team = ent->team;
		from = ent->message;
	}

	for (g=&g_edicts[1]; g < &g_edicts[globals.num_edicts]; g++) {
		if (g->client && g->client->pers.connected) {
			if (g->client->resp.team == dest_team) {
				if (ent->radio_channel == g->radio_channel) {
					if (ent->client && IsMsgJammed(g))	// only clients trasmitions are jammed
						gi.cprintf(g, PRINT_CHAT, "(unknown): %s\n", jammed);
					else
						gi.cprintf(g, PRINT_CHAT, "(%s): %s\n", from, msg);
				}
			}
		// Radio messages can also be passed on to enemy decoders, assuming they were not jammed
		// Only player generated messages are relayed..
		} else if (ent->client) {
			if (!DL_strcmp(g->classname, "msg_decoder", -1, false) && g->use && (g->team != dest_team)) {
				if ((ent->radio_channel == g->radio_channel) && !IsMsgJammed(g)) {
					g->message1 = msg;
					g->radio_msg = NULL;
					g->use(g, ent, ent);
				}
			}
		}
	}
}

void Voice_Use(edict_t *ent, char *msg) {
char filename[64];

	if (!msg)
		return;

	if (IsFemale(ent))
		sprintf(filename, "*voice/%s.wav", msg);
	else
		sprintf(filename, "*voice/%s.wav", msg);

	gi.sound(ent, CHAN_VOICE, gi.soundindex(filename), 1, ATTN_NORM, 0);
}

