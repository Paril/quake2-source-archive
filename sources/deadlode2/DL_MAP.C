#include "g_local.h"

extern void SP_trigger_multiple(edict_t *ent);
extern void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

float	PlayersRangeFromSpot (edict_t *spot);
edict_t *SelectRandomDeathmatchSpawnPoint (void);

/*QUAKED info_player_team1 (1 0 0) (-16 -16 24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_dl_player_start(edict_t *self)
{
}

edict_t *SelectDLTeamSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

//	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	if (!ent->client->resp.team)
		return SelectRandomDeathmatchSpawnPoint();

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "dl_player_start")) != NULL)
	{
		// filter out non-team spawn-points
		if (!spot->team_mask)
			continue;

		if ((spot->team_mask-1) != (ent->client->resp.team - &dlsys.teams[0]))
			continue;

		count++;
		// Place players at the team spawnpoint farthest from any players
		// This also helps prevent telefragging
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
	}

	if (!count) {
		gi.dprintf("WARNING: SPAWN: No suitable team-spawnpoints found.\n  Using deathmatch spawnpoints.\n");
		return SelectRandomDeathmatchSpawnPoint();
	}

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "dl_player_start");
		if (!spot) {
			gi.dprintf("ERROR: SPAWN: Team spawns found, but unable to use.\n  Using deathmatch spawnpoints.\n");
			return SelectRandomDeathmatchSpawnPoint();
		}
		if (!spot->team_mask)
			selection++;
		else if ((spot->team_mask-1) != (ent->client->resp.team - &dlsys.teams[0]))
			selection++;
		else if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

extern void droptofloor (edict_t *ent);

void ReturnGoalItem(edict_t *ent) {
	//setup goalitem to respawn at origin
	VectorCopy(ent->pos1, ent->s.origin);

	// release player from goalitem and vice-versa
	if (ent->goalitem && ent->goalitem->goalitem == ent)
		ent->goalitem->goalitem = NULL;
	ent->goalitem = NULL;
	ent->flags = GOAL_ATBASE;

	ent->svflags &= ~SVF_NOCLIENT;			// make visible to clients
	ent->spawnflags &= ~ITEM_TARGETS_USED;	// let returned goal refire targets
	ent->nextthink = level.time + FRAMETIME;
	ent->think = droptofloor;
	gi.linkentity(ent);

	ent->s.event = EV_ITEM_RESPAWN;
}

void DropGoalItem(edict_t *player) {
edict_t *goalitem;

	if (!player->goalitem) // must be dropped by a player
		return;

	// Remove the item from the players inventory
	RemoveInventoryItem(player, FindItemByName(player, player->goalitem->supply->name, ITEM_SUPPLY), 1);

	// Free the goal item from the player
	goalitem = player->goalitem;
	player->goalitem = NULL;
	goalitem->goalitem = NULL;
	goalitem->flags = GOAL_DROPPED;

	// This player cannot pickup the goalitem again for 2 seconds.
	goalitem->owner = player;
	goalitem->pain_debounce_time = level.time + 1.0;

	// Now drop the goal item at the players location
	VectorCopy(player->s.origin, goalitem->s.origin);
	goalitem->spawnflags &= ~ITEM_TARGETS_USED;	// let dropped goal refire targets
	goalitem->svflags &= ~SVF_NOCLIENT;
	goalitem->touch_debounce_time = level.time + goalitem->wait; // time before goal item returns
	goalitem->nextthink = level.time + FRAMETIME;
	goalitem->think = droptofloor;

	gi.linkentity(goalitem);
}

void Think_Supply(edict_t *self) {
	if (self->frame_last != self->frame_first) {
		self->s.frame++;
		if (self->s.frame > self->frame_last)
			self->s.frame = self->frame_first;
	}

	if ((self->flags == GOAL_DROPPED) && (self->touch_debounce_time < level.time))
		ReturnGoalItem(self);

	self->nextthink = level.time + FRAMETIME;
}

void Touch_Supply (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->supply)
		return;		// not a supply?

	if (ent->owner && ent->pain_debounce_time < level.time)
		ent->owner = NULL;

	// don't let the owner pickup this item
	if (ent->owner && ent->owner == other)
		return;
		

	// can the owning team return it?
	if (ent->team_mask && (ent->team_mask-1 != other->client->resp.team - &dlsys.teams[0])) {
		if (dlsys.playmode & PM_DEFENSERETURN)
			if (!VectorCompare(ent->pos1, ent->s.origin))
				ReturnGoalItem(ent);
		return;
	}

	// players can only carry one goal item at a time
	if ((ent->spawnflags & 0x04) && (other->goalitem)) {
		if (other->msg_time < level.time) {
			if (other->goalitem != ent)
				gi.cprintf(other, PRINT_HIGH, "You are already carrying a goal item.\n");
			other->msg_time = level.time + 1.0;
		}
		return;
	}

	// flash the screen
	other->client->bonus_alpha = 0.25;	
	gi.sound(other, CHAN_ITEM, gi.soundindex("items/pkup.wav"), 1, ATTN_NORM, 0);

	// add the item to the players inventory
	AddToInventory(other, ITEM_SUPPLY, ent->supply, 1, ent->count, 0, NULL, NULL);

	// Fire targets
	if (!(ent->spawnflags & ITEM_TARGETS_USED))	{
		ent->spawnflags |= ITEM_TARGETS_USED;
		G_UseTargets (ent, other);
	}

	// Special handling for goal items
	if (ent->spawnflags & 0x04) {
		VectorCopy(ent->pos1, ent->s.origin);
		other->goalitem = ent;	// link goalitem to carrier
		ent->goalitem = other;	// and vice-versa
		ent->flags = GOAL_CARRIED;

		ent->svflags |= SVF_NOCLIENT;
		ent->touch = NULL;
		gi.linkentity(ent);

		// undisguise spies
		if (other->team != other->client->resp.team) {
			other->team = other->client->resp.team;
			DL_AssignSkin(other, Info_ValueForKey(other->client->pers.userinfo, "skin"));
		}
		return;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
}

void use_target_award(edict_t *self, edict_t *other, edict_t *activator);

void SP_dl_ctfflag1(edict_t *ent) {
supply_t *sup;
edict_t *goal, *score;
static char flag[] = "*Red Flag";
int i, frame_s, frame_e;

	// find an empty supply, or one with same name
	for (i = MAX_INI_SUPPLIES; i < MAX_SUPPLIES; i++) {
		if (!supplies[i].name || !DL_strcmp(supplies[i].name, st.name, -1, false)) {
			sup = &supplies[i];
			break;
		}
	}
/*
	if (!sup) return;
	sup = &supplies[MAX_SUPPLIES];
	while (1) {
		if (sup > &supplies[MAX_SUPPLIES+MAX_MAP_SUPPLIES]) return;
		if (!sup->name)	break;
		if (!DL_strcmp(sup->name, flag, -1, false))	break;
		sup++;
	}
*/
	sup->name = flag;
	sup->type = SUP_GOAL;
	sup->model = dlsys.teams[0].ctfflag;
	if (sup->model) {
		if (GetMD2FrameSet(sup->model, "idle", &frame_s, &frame_e)) {
			sup->frame = frame_s;
			ent->frame_first = frame_s;
			ent->frame_last = frame_e;
		}
		gi.setmodel(ent, sup->model);
		ent->s.frame = sup->frame;
		ent->s.skinnum = 0;
	}


	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;
	VectorCopy(ent->s.origin, ent->pos1);	// Save spawn location, in case we need it for goalitems
	VectorSet(ent->mins, -16, -16, -16);
	VectorSet(ent->maxs, 16, 16, 16);

	ent->classname = "dl_item_supply";
	ent->message = "You have the enemy flag!";
	ent->message1 = "%P HAS THE ENEMY FLAG!";
	ent->message2 = "%P HAS STOLEN YOUR FLAG!";
	ent->spawnflags = 0x04|0x08;
	ent->team_mask = 2;
	ent->team_own = 1;
	ent->wait = 30;

	ent->supply = sup;
	ent->nextthink = level.time + 2 * FRAMETIME;	// items start after other solids
	ent->think = droptofloor;
	ent->s.effects = EF_FLAG1;

	gi.linkentity(ent);

	// For CTF flags we also need to spawn a goal trigger at the same location
	goal = G_Spawn();
	VectorCopy(ent->s.origin, goal->s.origin);
	VectorCopy(ent->s.angles, goal->s.angles);
	goal->spawnflags = 16;
	goal->radio_msg = dlsys.teams[0].capwav;
	goal->model = "models/objects/flagpad.md2";
	st.skinnum = 0;
	goal->team_mask = 1;
	goal->team_own = 1;
	goal->target = "red_scores";
	st.item = NULL;
	goal->item_name = "*Blue Flag";

	SP_dl_trigger_model(goal);
	goal->wait = 0; // Override default wait of 0.2sec

	// Finally, we need an award trigger to give points to the capturing team
	score = G_Spawn();
	score->classname = "dl_target_award";
	VectorCopy(ent->s.origin, score->s.origin);
	VectorCopy(ent->s.angles, score->s.angles);
	score->targetname = "red_scores";
	score->team_mask = 1;
	score->team_own = 1;
	score->count = 1;	// 1 point to the team;
	score->style = 5;	// 5 points to the player
	score->use = use_target_award;
	score->message = "You captured the enemy flag!";
	score->message1 = "%P CAPTURED THE ENEMY FLAG!";
	score->message2 = "%P CAPTURED YOUR FLAG!";
}

void SP_dl_ctfflag2(edict_t *ent) {
supply_t *sup;
edict_t *goal, *score;
static char flag[] = "*Blue Flag";
int i, frame_s, frame_e;

	// find an empty supply, or one with same name
	for (i = MAX_INI_SUPPLIES; i < MAX_SUPPLIES; i++) {
		if (!supplies[i].name || !DL_strcmp(supplies[i].name, st.name, -1, false)) {
			sup = &supplies[i];
			break;
		}
	}
/*
	if (!sup) return;
	sup = &supplies[MAX_SUPPLIES];
	while (1) {
		if (sup > &supplies[MAX_SUPPLIES+MAX_MAP_SUPPLIES]) return;
		if (!sup->name)	break;
		if (!DL_strcmp(sup->name, flag, -1, false))	break;
		sup++;
	}
*/
	sup->name = flag;
	sup->type = SUP_GOAL;
	sup->model = dlsys.teams[1].ctfflag;
	if (sup->model) {
		if (GetMD2FrameSet(sup->model, "idle", &frame_s, &frame_e)) {
			sup->frame = frame_s;
			ent->frame_first = frame_s;
			ent->frame_last = frame_e;
		}
		ent->s.frame = sup->frame;
		gi.setmodel(ent, sup->model);
		ent->s.skinnum = 1;
	}


	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;
	VectorCopy(ent->s.origin, ent->pos1);	// Save spawn location, in case we need it for goalitems
	VectorSet(ent->mins, -16, -16, -16);
	VectorSet(ent->maxs, 16, 16, 16);

	ent->classname = "dl_item_supply";
	ent->message = "You have the enemy flag!";
	ent->message1 = "%P HAS THE ENEMY FLAG!";
	ent->message2 = "%P HAS STOLEN YOUR FLAG!";
	ent->spawnflags = 0x04|0x08;
	ent->team_mask = 1;
	ent->team_own = 2;
	ent->wait = 30;

	ent->supply = sup;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = EF_FLAG2;

	gi.linkentity(ent);

	// For CTF flags we also need to spawn a goal trigger at the same location
	goal = G_Spawn();
	goal->classname = "dl_trigger_model";
	VectorCopy(ent->s.origin, goal->s.origin);
	VectorCopy(ent->s.angles, goal->s.angles);
	goal->spawnflags = 16;
	goal->radio_msg = dlsys.teams[1].capwav;
	goal->model = "models/objects/flagpad.md2";
	st.skinnum = 1;
	goal->team_mask = 2;
	goal->team_own = 2;
	goal->target = "blue_scores";
	st.item = NULL;
	goal->item_name = "*Red Flag";

	SP_dl_trigger_model(goal);
	goal->wait = 0; // Override default wait of 0.2sec

	// Finally, we need an award trigger to give points to the capturing team
	score = G_Spawn();
	score->classname = "dl_target_award";
	VectorCopy(ent->s.origin, score->s.origin);
	VectorCopy(ent->s.angles, score->s.angles);
	score->targetname = "blue_scores";
	score->team_mask = 2;
	score->team_own = 2;
	score->count = 1;	// 1 point to the team;
	score->style = 5;	// 5 points to the player
	score->use = use_target_award;
	score->message = "You captured the enemy flag!";
	score->message1 = "%P CAPTURED THE ENEMY FLAG!";
	score->message2 = "%P CAPTURED YOUR FLAG!";
}

void SP_dl_item_supply(edict_t *ent) {
int i, frame_s, frame_e;
supply_t *sup = NULL;

	// find an empty supply, or one with same name
	for (i = MAX_INI_SUPPLIES; i < MAX_SUPPLIES; i++) {
		if (!supplies[i].name || !DL_strcmp(supplies[i].name, st.name, -1, false)) {
			sup = &supplies[i];
			break;
		}
	}
	if (!sup) return;

	sup->name = st.name;
	sup->type = st.type;
	sup->model = ent->model;
	sup->flags = st.flags;
	sup->skinnum = st.skinnum;
	sup->frame = st.frame;
	sup->range = st.range;
	sup->duration = st.duration;
	sup->effect = st.effect;
	sup->s_use = st.s_use;
	sup->pic_s = st.pic_s;
	sup->pic_b = st.pic_b;

	if (!DL_strcmp(sup->model, "%f", -1, false))
		sup->model = dlsys.teams[ent->team_own-1].ctfflag;

	if (sup->model) {
		if (GetMD2FrameSet(sup->model, "idle", &frame_s, &frame_e)) {
			sup->frame = frame_s;
			ent->frame_first = frame_s;
			ent->frame_last = frame_e;
		}
		gi.setmodel(ent, sup->model);
		ent->s.frame = sup->frame;
		ent->s.skinnum = sup->skinnum;
	}

	if (ent->spawnflags & 0x04) {
		sup->type |= SUP_GOAL;
		ent->flags = GOAL_ATBASE;
	}

	sup->health = ent->health;
	sup->damage = ent->dmg;
	sup->delay = ent->delay;

	ent->model = sup->model;
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;
	VectorCopy(ent->s.origin, ent->pos1);	// Save spawn location, in case we need it for goalitems
	VectorSet(ent->mins, -16, -16, -16);
	VectorSet(ent->maxs, 16, 16, 16);

	ent->supply = sup;
	ent->nextthink = level.time + (2 * FRAMETIME);    // items start after other solids
	ent->think = droptofloor;
	ent->s.renderfx = RF_GLOW;

	gi.linkentity(ent);
}

void SP_dl_trigger_model(edict_t *ent) {
vec3_t	ang;

	if (!ent->model) {	// a model MUST be specified
		G_FreeEdict(ent);
		return;
	}

	if (!DL_strcmp(ent->model, "%b", -1, false)) {
		ent->model = dlsys.teams[ent->team_own-1].banner;
		st.skinnum = 0;
	} else if (!DL_strcmp(ent->model, "%s", -1, false)) {
		ent->model = dlsys.teams[ent->team_own-1].sbanner;
		st.skinnum = 0;
	} else {
		VectorSet(ent->mins, -16, -16, -16);
		VectorSet(ent->maxs, 16, 16, 16);
	}
	// because we are using a real model, we need to preserve the angles
	// SP_trigger_multiple trashes them...
	VectorCopy(ent->s.angles, ang);
	SP_trigger_multiple(ent);
	VectorCopy(ang, ent->s.angles);

	ent->s.skinnum = st.skinnum;

	ent->svflags -= (SVF_NOCLIENT & ent->svflags);
	gi.linkentity(ent);
}

void Use_Target_Resupply(edict_t *self, edict_t *other, edict_t *activator) {
int i;

	if (!activator || !activator->client)
		return;

	if (activator->client->resp.profile[0] != -1)
		LoadSystemProfile(activator, activator->client->resp.profile, self->spawnflags, self->delay);
	else
		LoadProfile(activator, activator->client->prefs.IHnd, self->spawnflags, self->delay);

	if ((self->health > 0) && (activator->health < activator->max_health)) {
		i = activator->health + self->health;
		if (i < activator->max_health)
			activator->health = i;
		else
			activator->health = activator->max_health;
	}
}


void SP_dl_target_resupply(edict_t *ent) {
	ent->use = Use_Target_Resupply;
	if (ent->delay > 1.0)
		ent->delay /= 100.0;
	if (ent->delay <= 0.0)
		ent->delay = 1.0;
}

void Use_Target_Teleport(edict_t *self, edict_t *other, edict_t *activator) {
	teleporter_touch(self, activator, NULL, NULL);
}

void SP_dl_target_teleport(edict_t *ent) {
	ent->use = Use_Target_Teleport;
}

// bubble spawner
void think_bubblespawn(edict_t *self) {
vec3_t	org;
int		i;

	if (!self->s.modelindex) {
		self->s.modelindex = gi.modelindex("models/objects/flash/tris.md2");
		self->s.renderfx = RF_TRANSLUCENT;
		gi.linkentity(self);
	}

	if (!(gi.pointcontents(self->s.origin) & MASK_WATER) || (VectorLength(self->velocity) == 0.0)) {
		if (!(self->owner->spawnflags & 0x01)) {
			G_FreeEdict(self);
			return;
		}
		VectorCopy(self->owner->s.origin, org);
		for (i=0; i<3; i++)
			org[i] += crandom() * self->owner->size[i];
		VectorCopy(org, self->s.origin);
		VectorSet(self->velocity, 0, 0, 15+(random()*35));
		self->s.modelindex = 0;
		gi.linkentity(self);
	}
	self->nextthink = level.time + FRAMETIME;
}

void use_bubblespawn(edict_t *self, edict_t *other, edict_t *activator) {
vec3_t	org;
int		i, j;
edict_t	*b;

	if (self->spawnflags & 0x01) {
		self->spawnflags -= (self->spawnflags & 0x01);
	} else {
		self->spawnflags |= 0x01;
		for (j=0; j<self->count; j++) {
			b = G_Spawn();
			b->owner = self;
			b->classname = "bubble";
			b->movetype = MOVETYPE_FLYMISSILE;
			b->solid = SOLID_NOT;
			b->flags |= FL_NO_WATER_TRANS;
			b->think = think_bubblespawn;
			b->nextthink = level.time + FRAMETIME;

			VectorCopy(self->s.origin, org);
			for (i=0; i<3; i++)
				org[i] += crandom() * self->size[i];
			VectorCopy(org, b->s.origin);
			VectorSet(b->velocity, 0, 0, 15+(random()*35));
			gi.linkentity(b);
		}
	}
}

void SP_dl_target_bubblespawn(edict_t *ent) {
	VectorSet(ent->size, 24, 24, 0);
	ent->use = use_bubblespawn;
	use_bubblespawn(ent, NULL, NULL);
}

/*
Okay, then:  dl_target_award will hurt/award health, rather than dl_resupply...

dl_target will have a team_mask which determines what team the award is given
too, reguardless of who activates.  If no team is specified, the dl_award will
be given to the activating team/player.  If "All Teams" (-1) is specified,
the award is given to all players.
*/

void use_target_award(edict_t *self, edict_t *other, edict_t *activator) {
edict_t	*g;
int		i;
team_t	*team;

	if (!activator->client)
		return;

	if (level.intermissiontime)
		return;

	if (self->team_mask == -1)
		team = activator->client->resp.team;
	else if (self->team_mask > 0)
		team = &dlsys.teams[self->team_mask-1];
	else
		return;

	if (self->team_own <= 0)
		self->team_own = self->team_mask;

	if (self->spawnflags & 0x01) {
		for (i=0; i<MAX_TEAMS; i++) {
			if (team != &dlsys.teams[i])
				dlsys.teams[i].score += self->count;
		}
	} else
		team->score += self->count;

	for (g = &g_edicts[1]; g <= &g_edicts[(int)maxclients->value]; g++) {
		if (!g->client)
			continue;
		if (!g->client->pers.connected)
			continue;
		if (self->spawnflags & 0x01) {
			if (team && (g->client->resp.team == team))
				continue;	// apply results to specific team only
		} else {
			if (team && (g->client->resp.team != team))
				continue;	// apply results to specific team only
		}


		// Award/Remove Health from targets
		if (!g->deadflag) {
			if (self->health) {
				if (self->health < g->health)
					T_Damage (g, world, world, vec3_origin, g->s.origin, vec3_origin, g->health - self->health, 0, DAMAGE_NO_ARMOR|DAMAGE_NO_HITLOC, MOD_UNKNOWN, NULL);
				else
					g->health = self->health;
			}

			if (self->dmg) {
				if (self->dmg > 0)
					T_Damage (g, world, world, vec3_origin, g->s.origin, vec3_origin, self->dmg, 0, DAMAGE_NO_ARMOR|DAMAGE_NO_HITLOC, MOD_UNKNOWN, NULL);
				else {
					i = g->health - self->dmg;
					if (i < g->max_health) g->health = i;
					else g->health = g->max_health;
				}
			}
		}
		// Adjust scores for targets
		g->client->resp.score += self->style;
	}
//
// print the messages
//
	if (activator->client && (self->message || self->noise_index)) {
		// send messages
		for (g = &g_edicts[1]; g <= &g_edicts[(int)maxclients->value]; g++) {
			if (!g->client)
				continue;
			if (!g->client->pers.connected)
				continue;

			if ((g == activator) && (self->message || self->noise_index)) {
				// send activator message and sound
				if (self->message) {
					gi.centerprintf (g, "%s", PicNParseMsg(self->message, activator->client->pers.netname, activator->client->resp.team->team_name, dlsys.teams[self->team_own-1].team_name));
					if (self->noise_index)
						unicastSound(g, self->noise_index, 1.0);
					else
						unicastSound(g, gi.soundindex("misc/talk1.wav"), 1.0);
				} else if (self->noise_index)
						unicastSound(g, self->noise_index, 1.0);
			} else if ((g->client->resp.team == activator->client->resp.team) && self->message1)
				// send team message
				gi.centerprintf (g, "%s", PicNParseMsg(self->message1, activator->client->pers.netname, activator->client->resp.team->team_name, dlsys.teams[self->team_own-1].team_name));
			else if (self->message2)
				// send global message
				gi.centerprintf (g, "%s", PicNParseMsg(self->message2, activator->client->pers.netname, activator->client->resp.team->team_name, dlsys.teams[self->team_own-1].team_name));
		}
	}
}

void SP_dl_target_award(edict_t *ent) {
	ent->use = use_target_award;
	if (!ent->team_mask) {
		gi.error("ERROR: dl_target_award must specify a team number.\n");
		return;
	}
}

void use_force_lights(edict_t *self, edict_t *other, edict_t *activator) {
edict_t *ent = NULL;

	while(ent = G_Find (ent, FOFS(targetname), self->targetname)) {
		if (DL_strcmp(ent->classname, "light", -1, false))
			continue;
		ent->spawnflags = !self->spawnflags;
		ent->use(ent, self, activator);
	}
}

void SP_dl_force_lights(edict_t *ent) {
	ent->use = use_force_lights;
}

void use_target_radio(edict_t *self, edict_t *other, edict_t *activator) {

	if (!self->radio_msg)
		return;

	Radio_Use(self, self->radio_msg);
	self->nextthink = level.time + FRAMETIME;
}

void SP_dl_target_radio(edict_t *ent) {
	if(!st.noise) {
		gi.dprintf("target_radio with no noise set at %s\n", vtos(ent->s.origin));
		return;
	}
	ent->radio_msg = st.noise;

	if (!ent->volume)
		ent->volume = 1.0;

	if (ent->team_mask)
		ent->team = &dlsys.teams[ent->team_mask-1];
	ent->use = use_target_radio;
	ent->think = Radio_CheckQue;
}

// A few notes about target_menu's
// - Menu_ID is the 100 + edict index
// - menu items fire target
// - 

void use_target_menu(edict_t *self, edict_t *other, edict_t *activator) {
int id;
	if (!activator->client)
		return;

	self->msg_time = level.time + self->wait;
	if (self->menu_id) id = self->menu_id + 2000;
	else id = ID_MAPMENU+(self-g_edicts);

	PMenu_Open(activator, id, 0, self);
}

void ShowTargetMenu(edict_t *ent, pmenuhnd_t *hnd) {
int num, i, j;
edict_t	 *menu;

	menu = (edict_t *)hnd->data;
	if (menu->wait && (menu->msg_time < level.time)) {
//		ent->client->menu[ent->client->curmenu] = NULL;
//		hnd->entries = NULL;
		return;
	}

	i = menu->num_mitem;
	if (ent->client->curmenu) num = i + 3;
	else num = i + 2;
/*
	if (menu->old_enemy == ent)
		hnd->cur = menu->count;
	menu->count = hnd->cur;
	menu->old_enemy = ent;
*/
	hnd->entries = gi.TagMalloc(sizeof(*hnd->entries)*num, TAG_LEVEL);
	memcpy(hnd->entries, menu->mitems, sizeof(*hnd->entries)*i);
	for (j = 0; j < i; j++) {
		hnd->entries[j].text = ED_NewString(menu->mitems[j].text);
		hnd->entries[j].pic_s = ED_NewString(menu->mitems[j].pic_s);
		hnd->entries[j].pic_b = ED_NewString(menu->mitems[j].pic_b);
		hnd->entries[j].target = ED_NewString(menu->mitems[j].target);
	}

	hnd->entries[i++] = MakeMenuItem(NULL);
	if (ent->client->curmenu)
		hnd->entries[i++] = MakeMenuItem("TF", "Go back", PreviousMenu);
	hnd->entries[i++] = MakeMenuItem("TF", "Exit", ExitMenu);
	hnd->num = i;
}

void SP_dl_target_menu(edict_t *ent) {
int i, j, k;
char opts[8], text[64], target[64], pic[64];

	// first setup pmenu_t structure
	// start by counting the number of menu elements
	for (ent->num_mitem=i=0; i<32; i++)
		if (st.menuitem[i])
			ent->num_mitem++;

	if (!ent->num_mitem) {
		G_FreeEdict(ent);
		return;
	}

	// Allocate and fill the pmenu_t array for this ent
	ent->mitems = (pmenu_t *)gi.TagMalloc(sizeof(pmenu_t) * ent->num_mitem, TAG_LEVEL);
	if (!ent->mitems) {
		G_FreeEdict(ent);
		return;
	}
	// text|flags|target|pic
	for (j=i=0; i<32 && j<ent->num_mitem; i++) {
		if (st.menuitem[i]) {
			memset(opts, 0, 8);
			memset(text, 0, 64);
			memset(target, 0, 64);
			memset(pic, 0, 64);
			for (k=0; k<sub_count(st.menuitem[i]); k++) {
				switch(k) {
				case 0:	sub_string(st.menuitem[i], opts, k); break;
				case 1:	sub_string(st.menuitem[i], text, k); break;
				case 2:	sub_string(st.menuitem[i], target, k); break;
				case 3:	sub_string(st.menuitem[i], pic, k); break;
				}
			}
			ent->mitems[j++] = MakeMenuItem(opts, text, target, pic);
		}
	}
	ent->classname = "target_menu";
	ent->use = use_target_menu;
}

// Trigger_sequence - Specify multiple targets seperated by '|'
// Each time this entity is used, it fires the next target in the target list
// It will either reset at the end, or remove itself
void use_trigger_sequence(edict_t *self, edict_t *other, edict_t *activator) {
char	temp[64];

	self->count++;
	if (self->count >= sub_count(self->radio_msg)) {
		if (!(self->spawnflags & 0x01)) {
			G_FreeEdict(self);
			return;
		}	
		self->count = 0;
	}

	if (self->target)
		gi.TagFree(self->target);
	self->target = DL_strcpy(NULL, sub_string(self->radio_msg, temp, self->count), -1);

	G_UseTargets(self, activator);
}

void SP_dl_trigger_sequence(edict_t *ent) {
char	temp[64];

	ent->count = -1;
	ent->radio_msg = ent->target;
	ent->target = DL_strcpy(NULL, sub_string(ent->radio_msg, temp, ent->count), -1);
	ent->use = use_trigger_sequence;;
}

void use_trigger_random (edict_t *self, edict_t *other, edict_t *activator) {
char buffer[64];

	if (self->count) {
		if (--self->count == 0) {
			G_FreeEdict(self);
			return;
		}
	}
	if (self->timestamp > level.time)
		return;

	self->activator = activator;
	if (self->target)
		gi.TagFree(self->target);
	self->target = DL_strcpy(NULL, sub_string(self->radio_msg, buffer, random() * sub_count(self->radio_msg)), -1);
	G_UseTargets(self, self->activator);

	self->timestamp = level.time + self->delay;
}

void SP_dl_trigger_random (edict_t *self) {
	self->use = use_trigger_random;
	self->svflags = SVF_NOCLIENT;
	self->radio_msg = self->target;
	self->target = NULL;
}

// Target_timer counts up or down until it reaches 0 or count.
// When it reaches 0 or count, it stops and fires one of two targets.
void think_trigger_timer(edict_t *ent) {
char	temp[64];

	if (ent->spawnflags & 0x01) {
		if (ent->dmg > 0)
			ent->dmg--;
	} else {
		if (ent->dmg < ent->count)
			ent->dmg++;
	}

	if (ent->dmg >= ent->count) {
		//ent->spawnflags |= 0x01;
		ent->dmg = ent->count;
		ent->think = NULL;

		if (sub_count(ent->radio_msg) >= 1) {
			if (ent->target)
				gi.TagFree(ent->target);
			ent->target = DL_strcpy(NULL, sub_string(ent->radio_msg, temp, 0), -1);
			G_UseTargets(ent, ent->activator);
		}
		return;
	} else if (ent->dmg <= 0) {
		//ent->spawnflags &= ~0x01;
		ent->dmg = 0;
		ent->think = NULL;

		if (sub_count(ent->radio_msg) > 1) {
			if (ent->target)
				gi.TagFree(ent->target);
			ent->target = DL_strcpy(NULL, sub_string(ent->radio_msg, temp, 1), -1);
			G_UseTargets(ent, ent->activator);
		}
		return;
	}

	ent->think = think_trigger_timer;
	ent->nextthink = level.time + 1.0;
}

void use_trigger_timer(edict_t *self, edict_t *other, edict_t *activator) {
	if (self->spawnflags & 0x01)
		self->spawnflags &= ~0x01;
	else
		self->spawnflags |= 0x01;

	self->activator = activator;
	think_trigger_timer(self);
}

void SP_dl_trigger_timer(edict_t *ent) {
char	temp[64];

	if (ent->spawnflags & 0x01)
		ent->spawnflags &= ~0x01;
	else
		ent->spawnflags |= 0x01;

	ent->dmg = st.lip;
	ent->radio_msg = ent->target;
	ent->target = DL_strcpy(NULL, sub_string(ent->radio_msg, temp, 0), -1);
	ent->use = use_trigger_timer;

	if (ent->spawnflags & 0x02) { // Start level counting
		ent->think = think_trigger_timer;
		ent->nextthink = level.time + 2.0;	// A little time for players to spawn in
	}
}

void think_target_shooter (edict_t *self) {
vec3_t	dir;

	if (level.time > self->timestamp) {
		self->think = NULL;
		if (self->flags & 0x01) // remove when finished firing
			G_FreeEdict(self);
		return;
	}		

	AngleVectors(self->s.angles, dir, NULL, NULL);
	if (self->ammotype->ftype == FT_PELLET)
		fire_pellet(self, self, self->s.origin, dir, self->ammotype->range, self->ammotype->damage, self->ammotype);
	else
		fire_projectile(self, self, self->s.origin, dir, self->ammotype->range, self->ammotype->damage, self->ammotype, NULL);

	if (self->style)
		MuzzleFlash2(self, self->style, MULTICAST_PVS);

	self->think = think_target_shooter;
	self->nextthink = level.time + self->delay;
}

void use_target_shooter (edict_t *self, edict_t *other, edict_t *activator) {
	if (self->count) {
		if (--self->count == 0) {
			self->flags |= 0x01; // Count has been reached, mark edict for removal
			return;
		}
	}

	if (self->flags & 0x01)	// Edict has been marked for removal - don't increase firetime
		return;

	self->timestamp = level.time + self->wait;
	if (!self->think)	// fire only if its not already firing, instead, just inc its fire time.
		think_target_shooter(self);
}

void SP_dl_target_shooter (edict_t *self)
{
	self->use = use_target_shooter;
	self->svflags = SVF_NOCLIENT;
	self->ammotype = GetAmmoByName(st.effect);
	if (!self->ammotype) {
		gi.dprintf("dl_target_shooter: Invalid or no munition specified.\n");
		G_FreeEdict(self);
		return;
	}
	if (!self->delay)
		self->delay = FRAMETIME;
}

void use_target_effect (edict_t *self, edict_t *other, edict_t *activator) {
	if (self->count) {
		if (--self->count == 0) {
			G_FreeEdict(self);
			return;
		}
	}
	if (self->timestamp > level.time)
		return;

	apply_effects(self->radio_msg, activator, self, other, self->s.origin, activator->s.origin, self->movedir, self->movedir, self->dmg_radius, self->last_dmg, NULL);
	self->timestamp = level.time + self->delay;
}

void SP_dl_target_effect (edict_t *self) {
	self->use = use_target_effect;
	self->svflags = SVF_NOCLIENT;
	self->radio_msg = st.effect;
	AngleVectors(self->s.angles, self->movedir, NULL, NULL);
}

void SP_dl_item_effect (edict_t *self) {
effect_t *efx;

	// find an empty supply, or one with same name
	efx = &effects[MAX_EFFECTS];
	while (1) {
		if (efx > &effects[MAX_EFFECTS]) {
			G_FreeEdict(self);
			return;
		}
		if (!efx->name) break;
		if (!DL_strcmp(efx->name, st.name, -1, false)) break;
		efx++;
	}

	efx->name = st.name;
	efx->type = st.type;
	efx->damage = st.dmg_str;
	efx->range = st.range;
	VectorCopy(self->offset, efx->offset);
	VectorCopy(st.spread, efx->spread);
	efx->duration = st.duration;
	efx->delay = self->delay;
	efx->speed = self->speed;
	efx->count = self->count;
	efx->health = self->health;
	efx->flags = st.flags;
	efx->dmgflags = st.dmgflags;
	efx->model = self->model;
	efx->skinnum = st.skinnum;
	efx->frame = st.frame;
	efx->num_frames = st.num_frames;
	efx->sound = st.s_use;
	efx->effect = st.effect;

	G_FreeEdict(self);
}

// Gives the activating player the item specified
void use_target_giveitem (edict_t *self, edict_t *other, edict_t *activator) {
void *item;

	switch (self->flags) {
	case ITEM_WEAPON:	item = (void *)GetWeaponByName(self->message);
	case ITEM_AMMO:		item = (void *)GetAmmoByName(self->message);
	case ITEM_SUPPLY:	item = (void *)GetSupplyByName(self->message);
	}
	AddToInventory(activator, self->flags, item, 0, self->count, 0, NULL, self->message1);
}

void SP_dl_target_giveitem (edict_t *self) {
	self->message = st.item;
	self->message1 = st.effect;
	self->flags = st.type;
	self->use = use_target_giveitem;
	self->svflags = SVF_NOCLIENT;
}