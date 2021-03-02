#include "g_local.h"

void Weapon_HolyFire_Fire (edict_t *ent);
void holyfire_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_holyfire (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void SpellFive (edict_t *ent);
void SpellThree (edict_t *ent);
void FindRedFlag (edict_t *ent);
void FindBlueFlag (edict_t *ent);
void FindRandomFlag (edict_t *ent);
void Shockwave (edict_t *ent);

static qboolean	is_quad;

void holylevel (edict_t *ent)
{
	char	new_level[16];
	
	if ((ent->client->resp.captures > 0) && (ent->client->resp.captures < 7)){
		if (ent->client->resp.ctf_team == CTF_TEAM1){
			if (ent->client->resp.captures == 1)
				strcpy(new_level, "DEMON");
			if (ent->client->resp.captures == 2)
				strcpy(new_level, "DEVIL");
			if (ent->client->resp.captures == 3)
				strcpy(new_level, "HELL FIEND");
			if (ent->client->resp.captures == 4)
				strcpy(new_level, "SUCCUBUS");
			if (ent->client->resp.captures == 5)
				strcpy(new_level, "INCUBUS");
			if (ent->client->resp.captures == 6)
				strcpy(new_level, "LORD OF HELL");
		}
		if (ent->client->resp.ctf_team == CTF_TEAM2) {
			if (ent->client->resp.captures == 1)
				strcpy(new_level, "ARCHANGEL");
			if (ent->client->resp.captures == 2)
				strcpy(new_level, "VIRTUE");
			if (ent->client->resp.captures == 3)
				strcpy(new_level, "PRINCIPALITY");
			if (ent->client->resp.captures == 4)
				strcpy(new_level, "DOMINION");
			if (ent->client->resp.captures == 5)
				strcpy(new_level, "CHERUBUM");
			if (ent->client->resp.captures == 6)
				strcpy(new_level, "SERAPHIM");
		}
		ent->client->resp.holylevel++;
		ent->client->pers.max_health += 15;
		ent->max_health += 15;
		ent->client->pers.max_bullets += 30;
		ent->client->pers.max_shells += 25;
		ent->client->pers.max_rockets += 15;
		ent->client->pers.max_grenades += 15;
		ent->client->pers.max_cells += 30;
		ent->client->pers.max_slugs += 25;
		
		gi.centerprintf(ent, "You are now a %s", new_level);
	} 
}


void Yell (edict_t *ent)
{
	char *t;
	char sound[20];

	t = gi.args();
	strcpy(sound, "yell/");


	if(!*t)
		gi.cprintf(ent, PRINT_HIGH, "Must have a sound\n");
	else {
		strcat(sound, t);
		strcat(sound, ".wav");
		gi.sound(ent, CHAN_VOICE, gi.soundindex(sound), 1, ATTN_NORM, 0);
	}
		
}


void SP_item_blue_statue (edict_t *ent)
{
	gi.setmodel (ent, "models/statue/bluestatue.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->s.frame = 0;
	ent->s.effects = EF_FLAG2;
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 40);
	gi.linkentity (ent);
}
void SP_item_red_statue (edict_t *ent)
{
	gi.setmodel (ent, "models/statue/redstatue.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->s.frame = 0;
	ent->s.effects = EF_FLAG1;
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 40);
	gi.linkentity (ent);
}

void misc_torch_think (edict_t *self)
{
	if (++self->s.frame < 6)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_torch (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FULLBRIGHT;
	VectorSet (ent->mins, -4, -4, 0);
	VectorSet (ent->maxs, 4, 4, 26);
	ent->s.modelindex = gi.modelindex ("models/flame/tris.md2");
	ent->think = misc_torch_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

void misc_fire_think (edict_t *self)
{
	if (++self->s.frame < 11)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_fire (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FULLBRIGHT;
	VectorSet (ent->mins, -4, -4, 0);
	VectorSet (ent->maxs, 4, 4, 26);
	ent->s.modelindex = gi.modelindex ("models/fire/tris.md2");
	ent->think = misc_fire_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

void item_angel_statue_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	vec3_t	eyes;
	int		dmg;
	trace_t	tr;

	
	dmg = 3;


	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self->owner)
			continue;

		if (ent == self)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		VectorCopy (self->s.origin, eyes);
		eyes[2] = eyes[2] + 20;

		VectorMA (ent->absmin, 0.5, ent->size, point);
		
		VectorSubtract (point, eyes, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (eyes, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (eyes);
		gi.WritePosition (tr.endpos);
		gi.multicast (eyes, MULTICAST_PHS);
	}
	self->nextthink = level.time + 2*FRAMETIME;
}

void SP_item_angel_statue (edict_t *ent)
{
	gi.setmodel (ent, "models/statue/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->s.frame = 2;
	ent->s.renderfx = RF_GLOW;
	ent->owner = ent;
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 64);
	ent->think = item_angel_statue_think;
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity (ent);
}

void target_gravity_use (edict_t *self)
{
	float i;
	while(1){
		i = random();
		if ((i < 0.5) && (i > -0.5))
			break;
	}
	sv_gravity->value = sv_gravity->value * i;
}

void SP_target_gravity (edict_t *ent)
{
	ent->use = target_gravity_use;
}

void SpellOne (edict_t *ent)
{
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 1){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
		
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		SuperJump(ent);
	if (ent->client->resp.ctf_team == CTF_TEAM2)
		Heal(ent);
}

void SuperJump (edict_t *ent)
{
	ent->client->resp.mana = 0;
	ent->client->resp.jump = 1;
}

void Heal (edict_t *ent)
{
	ent->health = ent->max_health;
	ent->client->resp.mana = 0;
}

#define         FLASH_RADIUS                    200
#define         BLIND_FLASH                     50      // Time of blindness in FRAMES
             
void Flash_Explode (edict_t *ent)
{
	vec3_t   offset, origin;
	edict_t *target;



	target = NULL;
	while ((target = findradius(target, ent->s.origin, FLASH_RADIUS)) != NULL)
	{
		
		if (!target->client)
			continue;       // It's not a player
		if (!visible(ent, target))
			continue;       // The grenade can't see it
		if (!infront(target, ent))
			continue;       // It's not facing it

		// Increment the blindness counter
		if (ent->client->resp.ctf_team == CTF_TEAM1)
			target->client->Dark = 1;
		else
			target->client->Dark = 0;
		target->client->blindTime += BLIND_FLASH * 1.5;
		target->client->blindBase = BLIND_FLASH;

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

void SpellFour (edict_t *ent)
{
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 3){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
		
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1){
		ent->client->resp.mana = 0;
		Spell_earthquake(ent);
	}
	if (ent->client->resp.ctf_team == CTF_TEAM2){
		ent->client->resp.mana = 0;
		SP_Decoy(ent, 0);
	}
}
void SpellTwo (edict_t *ent)
{
	PMenu_Close(ent);
	if (ent->client->resp.holylevel < 2){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
		
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1){
		ent->client->resp.mana = 0;
		Flash_Explode(ent);
	}
	if (ent->client->resp.ctf_team == CTF_TEAM2){
		ent->client->resp.mana = 0;
		Flash_Explode(ent);
	}
}
void SpellSix (edict_t *ent)
{
	PMenu_Close(ent);

	if (ent->client->resp.holylevel < 4){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1){
		ent->client->resp.mana = 0;
		SP_monster_mutant(ent);
	}
	if (ent->client->resp.ctf_team == CTF_TEAM2){
		ent->client->resp.mana = 0;
		Divine(ent);
	}
}
void divine_think (edict_t *self)
{
	edict_t *target;

	target = NULL;
	while ((target = findradius(target, self->s.origin, 100)) != NULL)
	{
		if (!target->client)
			continue;       // It's not a player
		if (target == self->owner){
			target->client->resp.mana = 1;
			target->health = target->max_health;
			break;
		}
	}
	self->nextthink = level.time + 2*FRAMETIME;
}
void Divine (edict_t *ent)
{
	edict_t *divine;
	vec3_t v;

	if (ent->divine){
		G_FreeEdict(ent->divine);
		ent->divine = NULL;
	}
	VectorCopy (ent->s.origin, v);
	v[2] = v[2] + 10;
	divine = G_Spawn();
	divine->owner = ent;
	ent->divine = divine;
	strcpy(divine->classname, "info_divine");
	VectorCopy(v, divine->s.origin);
	divine->s.modelindex = gi.modelindex ("models/objects/gibs/head/tris.md2");
	divine->s.sound = gi.soundindex("az/hum1.wav");
	divine->s.angles[2] = -90;
	divine->s.effects = EF_QUAD|EF_FLAG2;
	divine->think = divine_think;
	divine->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(divine);
}

void CheckforDivine(edict_t *ent)
{
	edict_t *target;

	target = NULL;
	while ((target = findradius(target, ent->s.origin, 200)) != NULL)
	{
		if (target->classname != "info_divine")
			continue;       // It's not a divine head
		if (target->owner == ent){
			ent->client->resp.score++;
			break;
		}
	}

}

pmenu_t RedMenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Cast Dark Energy",		PMENU_ALIGN_LEFT, NULL, SpellOne },
	{ "Cast Unholy Darkness",		PMENU_ALIGN_LEFT, NULL, SpellTwo },
	{ "Cast Shockwave",		PMENU_ALIGN_LEFT, NULL, SpellThree },
	{ "Cast Earthquake",		PMENU_ALIGN_LEFT, NULL, SpellFour },
	{ "Cast Unclean Spirit",		PMENU_ALIGN_LEFT, NULL, SpellFive },
	{ "Cast Summon Hellspawn",		PMENU_ALIGN_LEFT, NULL, SpellSix },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

pmenu_t BlueMenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Cast Prayer Heal",		PMENU_ALIGN_LEFT, NULL, SpellOne },
	{ "Cast Light of Faith",		PMENU_ALIGN_LEFT, NULL, SpellTwo },
	{ "Cast Chaos Seek",		PMENU_ALIGN_LEFT, NULL, SpellThree },
	{ "Cast Illusion",		PMENU_ALIGN_LEFT, NULL, SpellFour },
	{ "Cast Flag Seek",		PMENU_ALIGN_LEFT, NULL, SpellFive },
	{ "Cast Divine Summoning",		PMENU_ALIGN_LEFT, NULL, SpellSix },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

void SpellMenu (edict_t *ent)
{
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		PMenu_Open(ent, RedMenu, 0, sizeof(RedMenu) / sizeof(pmenu_t));
	else
		PMenu_Open(ent, BlueMenu, 0, sizeof(BlueMenu) / sizeof(pmenu_t));
}

void Weapon_HolyFire (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, Weapon_HolyFire_Fire);
}

void Weapon_HolyFire_Fire (edict_t *ent)
{
	int i;
	vec3_t	start, forward, right, angles;
	int		damage = 15;
	int 		kick = 30;
	vec3_t	offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}

	if(level.time < ent->client->next_fire){
		return;
	}
	else {
		ent->client->next_fire = level.time + 1;
	}

	if (is_quad)
	{
		damage *=4;
		kick *=4;
	}
	
	for(i=1; i<3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet (offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	if(ent->client->resp.ctf_team == CTF_TEAM1)
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("az/redspel.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("az/bluspel.wav"), 1, ATTN_NORM, 0);	
	fire_holyfire (ent, start, forward, damage, 550, 60, 60);
}

void holyfire_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);


	G_FreeEdict (ent);
}

void fire_holyfire (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if (self->client->resp.ctf_team == CTF_TEAM2)
		rocket->s.effects |=EF_FLAG2;
	else
		rocket->s.effects |=EF_FLAG1;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	if (self->client->resp.ctf_team == CTF_TEAM2)
		rocket->s.modelindex = gi.modelindex ("models/weapons/holyfire/tris.md2");
	else
		rocket->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	rocket->owner = self;
	rocket->touch = holyfire_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "holy_fire";



	gi.linkentity (rocket);
}



void CheckForBody(edict_t *ent){

	edict_t *target;
	target = NULL;
	while ((target = findradius(target, ent->s.origin, 200)) != NULL)
	{
		if (Q_stricmp(target->classname,"body") == 0){
			SP_Decoy(ent, 1);
			G_FreeEdict(target);
			}
	}
}
void SendtoFlag(edict_t *dest, edict_t *ent)
{
	int			i;
	vec3_t		forward;

	if (!ent->client)
		return;

//ZOID
	CTFPlayerResetGrapple(ent);
//ZOID

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (ent);

	VectorCopy (dest->s.origin, ent->s.origin);
	VectorCopy (dest->s.origin, ent->s.old_origin);
//	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);
	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	ent->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - ent->client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = dest->s.angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (dest->s.angles, ent->client->ps.viewangles);
	VectorCopy (dest->s.angles, ent->client->v_angle);

	// give a little forward velocity
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 200, ent->velocity);

	// kill anything at the destination
	if (!KillBox (ent))
	{
	}

	gi.linkentity (ent);
}

pmenu_t FlagMenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Faith Capture the Flag",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Find Red Flag",		PMENU_ALIGN_LEFT, NULL, FindRedFlag },
	{ "Find Blue Flag",		PMENU_ALIGN_LEFT, NULL, FindBlueFlag },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

void FindRedFlag (edict_t *ent)
{
	int i;
	edict_t *e;
	PMenu_Close(ent);
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (Q_stricmp(e->classname,"item_flag_team1") != 0)
			continue;
		SendtoFlag(e, ent);
	}

}

void FindFlag (edict_t *ent)
{
	PMenu_Open(ent, FlagMenu, 0, sizeof(FlagMenu) / sizeof(pmenu_t));

}
void FindBlueFlag (edict_t *ent)
{
	int i;
	edict_t *e;
	PMenu_Close(ent);
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (Q_stricmp(e->classname,"item_flag_team2") != 0)
			continue;
		SendtoFlag(e, ent);
	}

}
void SpellFive (edict_t *ent)
{
	PMenu_Close(ent);

	if (ent->client->resp.holylevel < 5){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1){
		ent->client->resp.mana = 0;
		CheckForBody(ent);
	}
	if (ent->client->resp.ctf_team == CTF_TEAM2){
		ent->client->resp.mana = 0;
		FindFlag(ent);
	}

}
void FindRandomFlag (edict_t *ent)
{
	float i;
	i = random();
	if (i < 0.5)
		FindBlueFlag(ent);
	if (i > 0.75)
		FindRedFlag(ent);

}
void SpellThree (edict_t *ent)
{
	PMenu_Close(ent);

	if (ent->client->resp.holylevel < 3){
		gi.centerprintf(ent, "You don't know this spell level");
		return;
	}
	if (ent->client->resp.mana != 1){
		gi.centerprintf(ent, "You do not have the mana to cast a spell");
		return;
	}
	if (ent->client->resp.ctf_team == CTF_TEAM1){
		ent->client->resp.mana = 0;
		Shockwave(ent);
	}
	if (ent->client->resp.ctf_team == CTF_TEAM2){
		ent->client->resp.mana = 0;
		FindRandomFlag(ent);
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


