#include "g_local.h"
#include "m_player.h"

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);


mframe_t decoy_frames_stand1 [] =
{
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL
};
mmove_t decoy_move_stand = {FRAME_stand01, FRAME_stand40, decoy_frames_stand1, NULL};

void decoy_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &decoy_move_stand;
}
mframe_t decoy_frames_run [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_run = {FRAME_run1, FRAME_run6, decoy_frames_run, NULL};

void actor_run (edict_t *self);

mframe_t decoy_frames_runandtaunt [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_runandtaunt = {FRAME_taunt01, FRAME_taunt17, decoy_frames_runandtaunt, actor_run};

mframe_t decoy_frames_runandflipoff [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_runandflipoff = {FRAME_flip01, FRAME_flip12, decoy_frames_runandflipoff, actor_run};

void actor_run (edict_t *self)
{
//	gi.dprintf("actor_run()\n");
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		decoy_stand(self);
	else
		self->monsterinfo.currentmove = &decoy_move_run;
}

char *player_msg[] =
{
	"Watch it",
	"#$@*& you",
	"Get off me",
	"OMG you're a n00b"
};

char *other_msg[] =
{
	"Gay",
	"WTF",
	"OMG",
	"T_T"
};

void drone_pain (edict_t *self, edict_t *other, float kick, int damage);
void actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	char	*s;

//	gi.dprintf("actor_pain()\n");

	//if (self->health < (self->max_health / 2))
	//	self->s.skinnum = 1;

	if ((level.time < self->pain_debounce_time) && (random() > 0.5))
		return;

	self->pain_debounce_time = level.time + GetRandom(3, 10);
//	gi.sound (self, CHAN_VOICE, actor.sound_pain, 1, ATTN_NORM, 0);
	/*
	if (other->client)
		s = HiPrint(va("%s's decoy: %s %s", self->activator->client->pers.netname, 
			player_msg[GetRandom(0, 3)], other->client->pers.netname));
	else
		s = HiPrint(va("%s's decoy: %s %s", self->activator->client->pers.netname, 
			other_msg[GetRandom(0, 3)], other->client->pers.netname));
	gi.bprintf(PRINT_HIGH, "%s\n", s);
	*/

	if (random() < 0.5)
		self->monsterinfo.currentmove = &decoy_move_runandflipoff;
	else
		self->monsterinfo.currentmove = &decoy_move_runandtaunt;
	drone_pain(self, other, kick, damage);
}


void decoy_rocket (edict_t *self)
{
	int		damage, speed;
	vec3_t	forward, start;

	speed = 650 + 30*self->activator->myskills.level;
	if (speed > 950)
		speed = 950;

	//if (random() <= 0.1)
		damage = 45 + 10*self->activator->myskills.level;	
	//else
    //damage = 1;

	MonsterAim(self, damage, speed, true, 0, forward, start);
	monster_fire_rocket (self, start, forward, damage, speed, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


void actor_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t actor_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,  NULL,
};
mmove_t actor_move_death1 = {FRAME_death101, FRAME_death106, actor_frames_death1, actor_dead};

mframe_t actor_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
};
mmove_t actor_move_death2 = {FRAME_death201, FRAME_death206, actor_frames_death2, actor_dead};

void decoy_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

#ifdef OLD_NOLAG_STYLE
	if (nolag->value)
	{
		M_Remove(self, false, true);
		return;
	}
#endif

	// check for gib
	if (self->health <= self->gib_health)
	{
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		//self->deadflag = DEAD_DEAD;
#ifdef OLD_NOLAG_STYLE
		M_Remove(self, false, false);
#else
		if (nolag->value)
			M_Remove(self, false, true);
		else
			M_Remove(self, false, false);
#endif
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;
	self->deadflag = DEAD_DEAD;

	//Talent: Exploding Decoy
	/*
	if(damage > 0 && self->activator && getTalentLevel(self->activator, TALENT_EXPLODING_DECOY) != -1)
	{
		int talentLevel = getTalentLevel(self->activator, TALENT_EXPLODING_DECOY);
		int decoyDamage = 100 + talentLevel * 50;
		int decoyRadius = 100 + talentLevel * 25;

		T_RadiusDamage(self, self->activator, decoyDamage, self, decoyRadius, MOD_EXPLODING_DECOY);

		self->takedamage = DAMAGE_NO;
		self->think = BecomeExplosion1;
		self->nextthink = level.time + FRAMETIME;
		return;
	}*/

	// regular death
	self->takedamage = DAMAGE_YES;
	self->s.modelindex2 = 0;

	n = rand() % 2;
	if (n == 0)		self->monsterinfo.currentmove = &actor_move_death1;
	else			self->monsterinfo.currentmove = &actor_move_death2;	
}

mframe_t actor_frames_attack [] =
{
	drone_ai_run, 30,  decoy_rocket,
	drone_ai_run, 30,  NULL,
	drone_ai_run, 30,   NULL,
	drone_ai_run, 30,   NULL,
	drone_ai_run, 30,  NULL,
	drone_ai_run, 30,  NULL,
	drone_ai_run, 30,   NULL,
	drone_ai_run, 30,   NULL
};
mmove_t actor_move_attack = {FRAME_attack1, FRAME_attack8, actor_frames_attack, actor_run};

qboolean CanAttack(edict_t *self)
{
	if (G_EntIsAlive(self->enemy) && !OnSameTeam(self, self->enemy) && visible(self, self->enemy))
		return true;
	else return false;
}
void actor_attack(edict_t *self)
{
	CanAttack(self);
//	int		n;

//	gi.dprintf("actor_attack()\n");


	self->monsterinfo.currentmove = &actor_move_attack;
	self->monsterinfo.attack_finished = level.time + 0.9;
//	n = (rand() & 15) + 3 + 7;
//	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

/*QUAKED misc_actor (1 .5 0) (-16 -16 -24) (16 16 32)
*/

void decoy_copy (edict_t *self)
{
	edict_t *target = self->activator;

	if (self->mtype != M_DECOY)
		return;

	if (PM_PlayerHasMonster(self->activator))
		target = self->activator->owner;

	// copy everything from our owner
	self->model = target->model;
	self->s.skinnum = target->s.skinnum;
	self->s.modelindex = target->s.modelindex;
	self->s.modelindex2 = target->s.modelindex2;
	self->s.effects = target->s.effects;
	self->s.renderfx = target->s.renderfx;

	// try to copy target's bounding box if there's room
	if (!VectorCompare(self->mins, target->mins) || !VectorCompare(self->maxs, target->maxs))
	{
		trace_t tr = gi.trace(self->s.origin, target->mins, target->maxs, self->s.origin, self, MASK_SHOT);
		if (tr.fraction == 1.0)
		{
			VectorCopy(target->mins, self->mins);
			VectorCopy(target->maxs, self->maxs);
			gi.linkentity(self);
		}
	}
}

void init_drone_decoy (edict_t *self)
{
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	decoy_copy(self);

	self->health = 350 + 100 * self->activator->myskills.level;
	self->model = "models/monsters/soldier/tris.md2";
	gi.setmodel(self, self->model);

	//Limit decoy health to 2000
	if(self->health > 3500)		self->health = 3500;

	self->max_health = self->health;
	self->gib_health = -150;
	self->mass = 200;
	self->mtype = M_DECOY;

	self->pain = actor_pain;
	self->die = decoy_die;

	self->monsterinfo.stand = decoy_stand;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.attack = actor_attack;
	self->monsterinfo.control_cost = 15;
	self->monsterinfo.cost = 12;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;

	//K03 Begin
	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	self->monsterinfo.power_armor_power = 0;
	//K03 End
/*
	if (random() > 0.5)
	{
		self->monsterinfo.melee = 1;
		self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	}
*/

//	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &decoy_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
}

qboolean MirroredEntitiesExist (edict_t *ent);

void Cmd_Decoy_f (edict_t *ent)
{
	edict_t *ret;
	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Decoy_f()\n", ent->client->pers.netname);

	if (!V_CanUseAbilities(ent, DECOY, M_DEFAULT_COST, true))
		return;

	if (MirroredEntitiesExist(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "You already have decoys out!\n");
		return;
	}

	ret = SpawnDrone(ent, 20, false);
	if (ret)
		ret->monsterinfo.level = ent->myskills.abilities[DECOY].current_level;
}
