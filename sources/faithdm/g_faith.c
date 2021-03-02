//g_faith.c
#include "g_local.h"
#include "g_faith.h"

pmenu_t joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Deathmatch",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Satanic Belief",		PMENU_ALIGN_LEFT, NULL, JoinSatan },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Christian Belief",		PMENU_ALIGN_LEFT, NULL, JoinGod },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
};

void OpenTeamMenu (edict_t *ent){
	PMenu_Open(ent, joinmenu, 0, sizeof(joinmenu) / sizeof(pmenu_t));
}

void JoinGod (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	gi.bprintf(PRINT_HIGH, "%s joined the legions of God\n", ent->client->pers.netname);
	ent->client->resp.team = CHRISTIAN;
	ent->client->resp.holylevel = 0;
	ent->client->resp.mana = 1;
}

void JoinSatan (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	gi.bprintf(PRINT_HIGH, "%s joined the minions of Satan\n", ent->client->pers.netname);
	ent->client->resp.team = SATANIST;
	ent->client->resp.holylevel = 0;
	ent->client->resp.mana = 1;
}

void holylevel (edict_t *ent){
	char new_level[16];
	if ((ent->client->resp.score % 5 == 0) && (ent->client->resp.score < 31)){
		if (ent->client->resp.team == SATANIST){
			if (ent->client->resp.score == 5){
				strcpy(new_level, "DEMON");
				ent->client->resp.holylevel =1;
			}
			if (ent->client->resp.score == 10){
				strcpy(new_level, "DEVIL");
				ent->client->resp.holylevel =2;
			}
			if (ent->client->resp.score == 15){
				strcpy(new_level, "HELL FIEND");
				ent->client->resp.holylevel = 3;
			}
			if (ent->client->resp.score == 20){
				strcpy(new_level, "SUCCUBUS");
				ent->client->resp.holylevel = 4;
			}
			if (ent->client->resp.score == 25){
				strcpy(new_level, "INCUBUS");
				ent->client->resp.holylevel = 5;
			}
			if (ent->client->resp.score == 30){
				strcpy(new_level, "LORD OF HELL");
				ent->client->resp.holylevel = 6;
			}

		}
		if (ent->client->resp.team == CHRISTIAN) {
			if (ent->client->resp.score == 5){
				strcpy(new_level, "ARCHANGEL");
				ent->client->resp.holylevel = 1;
			}
			if (ent->client->resp.score == 10){
				strcpy(new_level, "VIRTUE");
				ent->client->resp.holylevel = 2;
			}
			if (ent->client->resp.score == 15){
				strcpy(new_level, "PRINCIPALITY");
				ent->client->resp.holylevel = 3;
			}
			if (ent->client->resp.score == 20){
				strcpy(new_level, "DOMINION");
				ent->client->resp.holylevel = 4;
			}
			if (ent->client->resp.score == 25){
				strcpy(new_level, "CHERUBUM");
				ent->client->resp.holylevel = 5;
			}
			if (ent->client->resp.score == 30){
				strcpy(new_level, "SERAPHIM");
				ent->client->resp.holylevel = 6;
			}
		}
		gi.centerprintf(ent, "You are now a %s\n", new_level);
	}
	ent->client->resp.mana++;
}

pmenu_t RedMenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Deathmatch",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Cast Dark Energy",		PMENU_ALIGN_LEFT, NULL, SpellOne },
	{ "Cast Unholy Darkness",		PMENU_ALIGN_LEFT, NULL, SpellTwo },
	{ "Cast Shockwave",		PMENU_ALIGN_LEFT, NULL, SpellThree },
	{ "Cast Vampiric Nature",		PMENU_ALIGN_LEFT, NULL, SpellFour },
	{ "Cast Wrath of Satan",		PMENU_ALIGN_LEFT, NULL, SpellFive },
	{ "Cast Summon Hellspawn",		PMENU_ALIGN_LEFT, NULL, SpellSix },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
};

pmenu_t BlueMenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Deathmatch",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Cast Prayer Heal",		PMENU_ALIGN_LEFT, NULL, SpellOne },
	{ "Cast Light of Faith",		PMENU_ALIGN_LEFT, NULL, SpellTwo },
	{ "Cast Gift of God",		PMENU_ALIGN_LEFT, NULL, SpellThree },
	{ "Cast Life Well",		PMENU_ALIGN_LEFT, NULL, SpellFour },
	{ "Cast Starburst",		PMENU_ALIGN_LEFT, NULL, SpellFive },
	{ "Cast Eyes of Justice",		PMENU_ALIGN_LEFT, NULL, SpellSix },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
};

void SpellMenu(edict_t *ent){
	if (ent->client->resp.team == SATANIST)
		PMenu_Open(ent, RedMenu, 0, sizeof(RedMenu) / sizeof(pmenu_t));
	else
		PMenu_Open(ent, BlueMenu, 0, sizeof(BlueMenu) / sizeof(pmenu_t));
}

void SpellOne (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 1){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.team == SATANIST){
		ent->client->resp.mana--;
		ent->client->jump = 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NORM, 0);
	}
	else {
		ent->client->resp.mana--;
		ent->health = ent->max_health;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
	}
}

void SpellTwo (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 2){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	Flash_Explode(ent);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/Bfg__x1b.wav"), 1, ATTN_NORM, 0);
	ent->client->resp.mana--;
}

void SpellThree (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 3){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.team == SATANIST){
		Shockwave(ent);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("tank/thud.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.mana--;
	}
	else {
		RandomPowerUp(ent);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lite_on3.wav"), 1, ATTN_NORM, 0);
		ent->client->resp.mana--;
	}
}
void SpellFour (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 4){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.team == CHRISTIAN){
		Lightning(ent);
		ent->client->resp.mana--;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lite_out.wav"), 1, ATTN_NORM, 0);
	}
	else {
		Vampire(ent);
		ent->client->resp.mana--;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/airhiss2.wav"), 1, ATTN_NORM, 0);
	}

}
void SpellFive (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 5){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	if(ent->client->resp.team == SATANIST){
		ent->client->resp.mana--;
		Spell_earthquake(ent);
	}
	else {
		ent->client->resp.mana--;
		Starburst(ent);
	}
}
void SpellSix (edict_t *ent, pmenu_t *p){
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 6){
		gi.centerprintf(ent, "You do not know this spell\nlevel %s.\n", ent->client->pers.netname);
		return;
	}
	if (ent->client->resp.mana == 0){
		gi.centerprintf(ent, "You do not have enough mana %s.\n",ent->client->pers.netname);
		return;
	}
	if(ent->client->resp.team == SATANIST){
		ent->client->resp.mana--;
		Hellspawn(ent);
	}
	else {
		ent->client->resp.mana--;
		Eyes(ent);
	}

}

void Flash_Explode (edict_t *ent)
{
	edict_t *target;



	target = NULL;
	while ((target = findradius(target, ent->s.origin, 200)) != NULL)
	{

		if (!target->client)
			continue;       // It's not a player
		if (!visible(ent, target))
			continue;       // The grenade can't see it
		if (!infront(target, ent))
			continue;       // It's not facing it

		// Increment the blindness counter
		if (ent->client->resp.team == SATANIST)
			target->client->Dark = 1;
		else
			target->client->Dark = 0;
		target->client->blindTime += 50 * 1.5;
		target->client->blindBase = 50;

		// Let the player know what just happened
		// (It's just as well, he won't see the message immediately!)
		gi.cprintf(target, PRINT_HIGH,
			"You are blinded by a spell!!!\n");

		// Let the owner of the grenade know it worked
		gi.cprintf(ent->owner, PRINT_HIGH,
			"%s is blinded by your spell!\n",
		target->client->pers.netname);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

}

void RandomPowerUp (edict_t *ent){

	gitem_t *item;
	float i;
	i = random();
	if ( i < 0.33){
		item = FindItem("Quad Damage");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		ent->client->pers.selected_item = ITEM_INDEX(item);
	}
	else if (i < 0.66){
		item = FindItem("Invulnerability");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		ent->client->pers.selected_item = ITEM_INDEX(item);
	}
	else {
		item = FindItem("BFG10K");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		ent->client->pers.selected_item = ITEM_INDEX(item);
	}
}

void ShockwaveExplode (edict_t *self)
{
	edict_t *target;
	vec3_t dir, point;
	float a, b, c;
	int count;
	for (count = 0; count < 6; count++){
		a = random();
		b = random();
		c = random();
		a *= 100;
		b *= 100;
		c *= 100;
		if (count % 2 == 0){
			a *= -1;
			b *= -1;
			c *= -1;
		}
		a = a + self->s.origin[0];
		b = b + self->s.origin[1];
		c = c + self->s.origin[2];
		VectorSet(point, a, b, c);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (point);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}
	target = NULL;
	while ((target = findradius(target, self->s.origin, 300)) != NULL)
	{
		if (!target->takedamage)
			continue;
		VectorSubtract(self->s.origin, target->s.origin, dir);
		T_Damage (target, self, self, dir, target->s.origin, vec3_origin, 40, 1, DAMAGE_ENERGY, MOD_UNKNOWN);
	}
}

void ShockwaveThink(edict_t *ent)
{
	if (level.time > ent->timestamp + ent->delay){
		ShockwaveExplode(ent);
		G_FreeEdict(ent);
	}
	else
		ent->nextthink = level.time + FRAMETIME;
}

void Shockwave (edict_t *ent)
{
	edict_t *bomb;
	bomb = G_Spawn();
	VectorCopy(ent->s.origin, bomb->s.origin);
	bomb->s.origin[2] += 30;
	bomb->delay = 3;
	bomb->timestamp = level.time;
	bomb->think = ShockwaveThink;
	bomb->nextthink = level.time + FRAMETIME;
	bomb->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.linkentity(bomb);
}

void Lightning (edict_t *ent)
{
	edict_t *bomb;
	bomb = G_Spawn();
	VectorCopy(ent->s.origin, bomb->s.origin);
	bomb->s.origin[2] += 30;
	bomb->delay = 5;
	bomb->s.effects |= EF_SPINNINGLIGHTS | EF_FLAG2;
	bomb->timestamp = level.time;
	bomb->think = LightningThink;
	bomb->nextthink = level.time + FRAMETIME;
	bomb->s.modelindex = gi.modelindex ("models/objects/gibs/sm_metal/tris.md2");
	gi.linkentity(bomb);
}

void LightningThink(edict_t *ent)
{
	if (level.time > ent->timestamp + ent->delay){
		LightningExplode(ent);
		G_FreeEdict(ent);
	}
	else
		ent->nextthink = level.time + FRAMETIME;
}

void LightningExplode (edict_t *self)
{
	edict_t *target;
	vec3_t dir;
	target = NULL;
	while ((target = findradius(target, self->s.origin, 300)) != NULL)
	{
		if (!target->takedamage)
			continue;
		if (!CanDamage (self, target))
				continue;
		VectorSubtract(self->s.origin, target->s.origin, dir);
		T_Damage (target, self, self, dir, target->s.origin, vec3_origin, 40, 1, DAMAGE_ENERGY, MOD_UNKNOWN);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
		gi.WriteShort (target - g_edicts);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (target->s.origin);
		gi.multicast (target->s.origin, MULTICAST_PVS);

	}
}

void Vampire (edict_t *self){
	edict_t *target;
	vec3_t dir;
	target = NULL;
	while ((target = findradius(target, self->s.origin, 300)) != NULL)
	{
		if (target == self)
			continue;
		if (!target->takedamage)
			continue;
		if (!CanDamage (self, target))
				continue;
		VectorSubtract(self->s.origin, target->s.origin, dir);
		T_Damage (target, self, self, dir, target->s.origin, vec3_origin, 30, 1, DAMAGE_ENERGY, MOD_UNKNOWN);
		if (self->health > (self->max_health -20))
			self->health = self->max_health;
		else
			self->health +=20;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
		gi.WriteShort (target - g_edicts);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (target->s.origin);
		gi.multicast (target->s.origin, MULTICAST_PVS);

	}
}

void Spell_earthquake (edict_t *ent)
{
	edict_t *self;

	self = G_Spawn();

	self->count = 5;
	self->speed = 600;
	self->svflags |= SVF_NOCLIENT;
	self->think = Spell_earthquake_think;
	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->last_move_time = 0;


	self->noise_index = gi.soundindex ("world/quake.wav");
}

void Spell_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict (self);
}

void Starburst (edict_t *ent){

	vec3_t dir1, dir2, dir3, dir4, dir5, dir6, dir7, dir8;

	vec3_t start1, start2, start3, start4, start5, start6, start7, start8;
	vec3_t a,b,c,d,e,f,g,h;

	VectorSet(dir1, 10, 0, 0);
	VectorSet(dir2, -10, 0, 0);
	VectorSet(dir3, 0, 10, 0);
	VectorSet(dir4, 0, -10, 0);
	VectorSet(dir5, 10, 10, 0);
	VectorSet(dir6, -10, -10, 0);
	VectorSet(dir7, -10, 10, 0);
	VectorSet(dir8, 10, -10, 0);
	VectorSet(start1, 8, 0, 10);
	VectorSet(start2, -8, 0, 10);
	VectorSet(start3, 0, 8, 10);
	VectorSet(start4, 0, -8, 10);
	VectorSet(start5, 8, 8, 10);
	VectorSet(start6, -8, -8, 10);
	VectorSet(start7, -8, 8, 10);
	VectorSet(start8, 8, -8, 10);
	VectorAdd(start1, ent->s.origin, a);
	VectorAdd(start2, ent->s.origin, b);
	VectorAdd(start3, ent->s.origin, c);
	VectorAdd(start4, ent->s.origin, d);
	VectorAdd(start5, ent->s.origin, e);
	VectorAdd(start6, ent->s.origin, f);
	VectorAdd(start7, ent->s.origin, g);
	VectorAdd(start8, ent->s.origin, h);
	fire_starburst(ent, a, dir1, 50, 700);
	fire_starburst(ent, b, dir2, 50, 700);
	fire_starburst(ent, c, dir3, 50, 700);
	fire_starburst(ent, d, dir4, 50, 700);
	fire_starburst(ent, e, dir5, 50, 700);
	fire_starburst(ent, f, dir6, 50, 700);
	fire_starburst(ent, g, dir7, 50, 700);
	fire_starburst(ent, h, dir8, 50, 700);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/laser2.wav"), 1, ATTN_NORM, 0);


}

void fire_starburst (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_IONRIPPER;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = starburst_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";

	gi.linkentity (bolt);


	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}

void starburst_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void Hellspawn (edict_t *ent){

	vec3_t forward;
	edict_t *self;
	if(ent->monster){
		G_FreeEdict(ent->monster);
		return;
	}
	self = G_Spawn();
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(ent->s.origin, 100, forward, self->s.origin);
	self->classname = "monster_mutant";
	self->creator = ent;
	ent->monster = self;
	ED_CallSpawn(self);
	gi.unlinkentity(self);
	KillBox(self);
	gi.linkentity(self);
}

void Eyes (edict_t *ent){

	edict_t *self;
	vec3_t v;

	if (ent->monster){
		G_FreeEdict(ent->monster);
		return;
	}
	self = G_Spawn();
	self->creator = ent;
	ent->monster = self;
	self->classname = "monster_eyes";
	VectorCopy(ent->s.origin, v);
	v[2] += 10;
	VectorCopy(v, self->s.origin);
	self->s.modelindex = gi.modelindex("models/objects/gibs/head/tris.md2");
	self->s.angles[2] = -90;
	self->s.effects = EF_QUAD|EF_FLAG2;
	self->think = Eyes_think;
	self->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(self);
}

void Eyes_think (edict_t *self){
	float i;
	i = random() * 10;
	i = i / 2;
	Starburst(self);
	self->nextthink = level.time + i;
}



