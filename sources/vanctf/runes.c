#include "g_local.h"
#include "runes.h"
#define RF_SHELL_YELLOW 65536
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
char *rune_namefornum[] = {NULL, RUNE_STRENGTH_NAME, RUNE_RESIST_NAME, RUNE_HASTE_NAME, RUNE_REGEN_NAME};
char *rune_iconfornum[] = {NULL, RUNE_STRENGTH_ICON, RUNE_RESIST_ICON, RUNE_HASTE_ICON, RUNE_REGEN_ICON};
//int	rune_renderfx[] = {RF_SHELL_RED, RF_SHELL_RED | RF_SHELL_BLUE, RF_SHELL_RED | RF_SHELL_GREEN, RF_SHELL_GREEN};
int	rune_renderfx[] = {0, RF_SHELL_GREEN, RF_SHELL_BLUE, RF_SHELL_YELLOW/*RF_SHELL_RED | RF_SHELL_GREEN*/, RF_SHELL_RED};

// true if a person has a specific rune
qboolean rune_has_rune(edict_t *ent, int type) {
	if (!ent->client) return (false);
	return (ent->client->pers.inventory[ITEM_INDEX(FindItem(rune_namefornum[type]))]);
}

// true if the person has a rune
int rune_has_a_rune(edict_t *ent) {
	int	i;
	if (!ent->client)
		return (0);	// only people can have runes
	for (i=RUNE_FIRST; i<=RUNE_LAST; i++)
		if (rune_has_rune(ent, i))
			return (i);
	return(0);
}

// a live client has touched a rune
qboolean rune_pickup(edict_t *self, edict_t *other) {
	// can only carry one rune
	if (rune_has_a_rune(other))
		return (false);
	// give them this rune
	other->client->pers.inventory[ITEM_INDEX(self->item)] = 1;
	return(true);
}

// moves the rune to a teleporter pad
void rune_move (edict_t *self) {
	rune_select_spawn_point(self->s.origin);
	if (VectorCompare(self->s.origin, vec3_origin)) {
		G_FreeEdict(self);
		return;
	}
	self->touch = Touch_Item;
	self->nextthink = level.time + 120;
	self->think = rune_move;
	gi.linkentity(self);
}

// makes the rune touchable again after being droppped
void rune_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->nextthink = level.time + 120;
	ent->think = rune_move;
}

// call rune_drop from here?
void rune_use (edict_t *ent, gitem_t *item) {
	// do nothing
}

void rune_drop_dying (edict_t *ent, gitem_t *item)
{
	edict_t	*rune;
	rune = Drop_Item(ent, item);
	rune->think = rune_make_touchable;
	rune->s.renderfx |= rune_renderfx[item - FindItem(rune_namefornum[RUNE_FIRST]) + RUNE_FIRST];//RF_SHELL_RED | RF_SHELL_GREEN;

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ValidateSelectedItem(ent);
}
		
// drops a rune
void rune_drop (edict_t *ent, gitem_t *item) {
	edict_t	*rune;
//	rune = G_Spawn();
//	VectorCopy(ent->s.origin, rune->s.origin);
//	rune_spawn(rune, item->pickup_name);
	rune = Drop_Item(ent, item);
	rune->nextthink = level.time + 120;
	rune->think = rune_move;
	rune->s.renderfx |= rune_renderfx[item - FindItem(rune_namefornum[RUNE_FIRST]) + RUNE_FIRST];//RF_SHELL_RED | RF_SHELL_GREEN;
//	rune->noblock = true;

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ValidateSelectedItem(ent);
}

void runes_drop_dying (edict_t *ent)
{
	int i;
	if (i = rune_has_a_rune(ent))
		rune_drop_dying (ent, FindItem(rune_namefornum[i]));
}

// drops any rune a person might have
void runes_drop (edict_t *ent) {
	int	i;
	if (i = rune_has_a_rune(ent))
		rune_drop (ent, FindItem(rune_namefornum[i]));
}

// finds a place to put a rune
// or vec3_origin if it can't find one
void rune_select_spawn_point(vec3_t origin) {
// ZOID
	edict_t *rune_spawn_point = NULL;
	int i = rand() & 15;
	
	while (i--)
		rune_spawn_point = G_Find(rune_spawn_point, FOFS(classname), "info_player_deathmatch");
	if (!rune_spawn_point)
		rune_spawn_point = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	if (!rune_spawn_point) { 
		gi.dprintf ("Couldn't find spawn point for rune\n");
		VectorClear(origin);
	} else {
		VectorCopy(rune_spawn_point->s.origin, origin);
	}
}

// spawns a rune
void rune_spawn(edict_t *rune, gitem_t *item) {
	rune->item = item;
	rune->classname = rune->item->classname;
	SpawnItem(rune, rune->item);
	rune->s.renderfx |= rune_renderfx[rune->item - FindItem(rune_namefornum[RUNE_FIRST]) + RUNE_FIRST];//RF_SHELL_RED | RF_SHELL_GREEN;
//	rune->noblock = true;
}

// spawns all the runes
void runes_spawn(edict_t *self) {
	edict_t	*rune;
	int	i, j;

	for (i=0; i<(int)runes->value; i++) {	// runes number of each rune
		for (j=RUNE_FIRST; j<=RUNE_LAST; j++) {	// run thru all runes
			rune = G_Spawn();
			rune_select_spawn_point(rune->s.origin);
			if (VectorCompare(rune->s.origin, vec3_origin)) {
				G_FreeEdict(rune);
			} else {
				rune_spawn(rune, FindItem(rune_namefornum[j]));
			}
		}
	}
	G_FreeEdict(self);
}

// so we can spawn the runes after the level starts
void runes_spawn_start() {
	edict_t	*temp;
	if (((int)dmflags->value & DF_CTF_NO_TECH) || !runes->value)
		return;
	temp = G_Spawn();
	temp->think = runes_spawn;
	temp->nextthink = level.time + 2 * FRAMETIME;
}

void rune_set_effects(edict_t *ent)
{
	if (ent->client->rune_time > level.time) {
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
//		ent->s.effects |= EF_PENT | EF_QUAD;
	}
}

void rune_apply_regen(edict_t *ent)
{
// ZOID
	gclient_t *client = ent->client;

	if (!client)
		return;
	
	if (client->regen_time <= level.time)
	{
		client->regen_time = level.time;
		if (ent->health < 150) {
			ent->health += RUNE_REGEN_PER_SEC / 2;
			if (ent->health > 150)
				ent->health = 150;
			if (ent->pain_debounce_time < level.time)
			{
				if (client->silencer_shots)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.2, ATTN_NORM, 0);
				else
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1.0, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			client->regen_time += 0.5;
		}                       

		if (ArmorIndex(ent) && client->pers.inventory[ArmorIndex(ent)] < 150) {
			client->pers.inventory[ArmorIndex(ent)] += RUNE_REGEN_PER_SEC / 2;
			if (client->pers.inventory[ArmorIndex(ent)] > 150)
				client->pers.inventory[ArmorIndex(ent)] = 150;
			if (ent->pain_debounce_time < level.time)
			{
				if (client->silencer_shots)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.2, ATTN_NORM, 0);
				else
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1.0, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			client->regen_time += 0.5;
		}
	}
}
