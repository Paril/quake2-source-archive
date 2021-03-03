// g_actor.c

// CYBERSLASH
// This file has been heavily modified

#include "g_local.h"
#include "m_player.h"

static int      actor_chaingun_loop;
static int      actor_chaingun_winddown;
static int      actor_hyperb_loop;
static int      actor_blaster_fire;

#define MAX_ACTOR_NAMES         4
char *actor_names_male[MAX_ACTOR_NAMES] =
{
        "Bitterman",
        "Howitzer",
        "Rambear",
	"Disruptor",
};

char *actor_names_female[MAX_ACTOR_NAMES] =
{
        "Lotus",
        "Athena",
        "Voodoo",
        "Jezebel"
};

char *actor_names_cyborg[MAX_ACTOR_NAMES-1] =
{
        "ONI911",
        "PS9000",
        "TYR574"
};


/*
===============
CheckSeenKiller

This checks to see if any other actors saw a client
killing the actor killed, and if so they attack the client
===============
*/

void CheckSeenKiller (edict_t *victim, edict_t *killer)
{
        int     j;
        edict_t *other;

        for (j = 1; j <= game.maxentities; j++)
	{
		other = &g_edicts[j];
                if (!other->inuse)
			continue;
                if (!(other->monsterinfo.aiflags & AI_GOOD_GUY))
                        continue;
                if (other->enemy)
                        continue;
                if (visible(other, killer) && (random() < 0.5))
                {
                        other->enemy = killer;
                        FoundTarget(other);
                }
	}
}        


mframe_t actor_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,

	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t actor_move_stand = {FRAME_stand01, FRAME_stand40, actor_frames_stand, NULL};

void actor_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_stand;

	// randomize on startup
	if (level.time < 1.0)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));
}


mframe_t actor_frames_walk [] =
{
        ai_walk, 30,  NULL,
        ai_walk, 30,  NULL,
        ai_walk, 30,  NULL,
        ai_walk, 30,  NULL,
        ai_walk, 30,  NULL,
        ai_walk, 30,  NULL
};
mmove_t actor_move_walk = {FRAME_run1, FRAME_run6, actor_frames_walk, NULL};

void actor_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &actor_move_walk;
}


mframe_t actor_frames_run [] =
{
        ai_run, 35, NULL,
        ai_run, 35, NULL,
        ai_run, 35, NULL,
        ai_run, 35, NULL,
        ai_run, 35, NULL,
        ai_run, 35, NULL
};
mmove_t actor_move_run = {FRAME_run1, FRAME_run6, actor_frames_run, NULL};

void actor_run (edict_t *self)
{
	if ((level.time < self->pain_debounce_time) && (!self->enemy))
	{
		if (self->movetarget)
			actor_walk(self);
		else
			actor_stand(self);
		return;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		actor_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &actor_move_run;
}


mframe_t actor_frames_pain1 [] =
{
	ai_move, -5, NULL,
	ai_move, 4,  NULL,
        ai_move, 1,  NULL,
        ai_move, 1,  NULL
};
mmove_t actor_move_pain1 = {FRAME_pain101, FRAME_pain104, actor_frames_pain1, actor_run};

mframe_t actor_frames_pain2 [] =
{
	ai_move, -4, NULL,
	ai_move, 4,  NULL,
        ai_move, 0,  NULL,
        ai_move, 0,  NULL
};
mmove_t actor_move_pain2 = {FRAME_pain201, FRAME_pain204, actor_frames_pain2, actor_run};

mframe_t actor_frames_pain3 [] =
{
	ai_move, -1, NULL,
	ai_move, 1,  NULL,
        ai_move, 0,  NULL,
        ai_move, 0,  NULL
};
mmove_t actor_move_pain3 = {FRAME_pain301, FRAME_pain304, actor_frames_pain3, actor_run};

mframe_t actor_frames_flipoff [] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,

	ai_turn, 0,  NULL,
        ai_turn, 0,  NULL
};
mmove_t actor_move_flipoff = {FRAME_flip01, FRAME_flip12, actor_frames_flipoff, actor_run};

mframe_t actor_frames_taunt [] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,

	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL
};
mmove_t actor_move_taunt = {FRAME_taunt01, FRAME_taunt17, actor_frames_taunt, actor_run};

char *messages[] =
{
	"Watch it",
        "#$@*& you",
        "Idiot,",
        "Check your targets"
};

void actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

//        if (self->health < (self->max_health / 2))
//                self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if ((other->client) && (random() < 0.4))
	{
		vec3_t	v;

		VectorSubtract (other->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw (v);
		if (random() < 0.5)
			self->monsterinfo.currentmove = &actor_move_flipoff;
		else
			self->monsterinfo.currentmove = &actor_move_taunt;
                gi.cprintf (other, PRINT_CHAT, "%s: %s %s!\n", self->message, messages[rand()%3], other->client->pers.netname);
		return;
	}

	n = rand() % 3;
	if (n == 0)
        {
		self->monsterinfo.currentmove = &actor_move_pain1;
                gi.sound (self, CHAN_VOICE, gi.soundindex(va("../players/%s/pain100_1.wav", self->model)), 1, ATTN_NORM, 0);
        }
	else if (n == 1)
        {
		self->monsterinfo.currentmove = &actor_move_pain2;
                gi.sound (self, CHAN_VOICE, gi.soundindex(va("../players/%s/pain100_2.wav", self->model)), 1, ATTN_NORM, 0);
        }
	else
        {
		self->monsterinfo.currentmove = &actor_move_pain3;
                gi.sound (self, CHAN_VOICE, gi.soundindex(va("../players/%s/pain75_1.wav", self->model)), 1, ATTN_NORM, 0);
        }
}

// standard monster_fire_*'s use MZ2's, I wanna use MZ's
void actorMuzzleflash (edict_t *self, vec3_t start, int flashtype)
{
        gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (flashtype);
	gi.multicast (start, MULTICAST_PVS);
}

void actorBlaster (edict_t *self, vec3_t start, vec3_t forward, qboolean hyper)
{
        if (hyper)
        {
                self->s.sound = actor_hyperb_loop;
                actorMuzzleflash(self, start, MZ_HYPERBLASTER);
                fire_blaster (self, start, forward, 15, 1000, EF_HYPERBLASTER, false);
        }
        else
        {
//                gi.sound(self, CHAN_AUTO, actor_blaster_fire, 1, ATTN_NORM, 0);
                actorMuzzleflash(self, start, MZ_BLASTER);
                fire_blaster (self, start, forward, 10, 1000, EF_BLASTER, false);
        }
}

void actorShotgun (edict_t *self, vec3_t start, vec3_t forward, qboolean super)
{
        vec3_t  v;

        if (super)
        {
                actorMuzzleflash(self, start, MZ_SSHOTGUN);
                v[PITCH] = self->s.angles[PITCH];
                v[YAW]   = self->s.angles[YAW] - 5;
                v[ROLL]  = self->s.angles[ROLL];
                AngleVectors (v, forward, NULL, NULL);
                fire_shotgun (self, start, forward, 6, 12, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_UNKNOWN);
                v[YAW]   = self->s.angles[YAW] + 5;
                AngleVectors (v, forward, NULL, NULL);
                fire_shotgun (self, start, forward, 6, 12, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_UNKNOWN);
        }
        else
        {
                actorMuzzleflash(self, start, MZ_SHOTGUN);
                fire_shotgun (self, start, forward, 4, 8, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_UNKNOWN);
        }
}

void actorMachinegun (edict_t *self, vec3_t start, vec3_t forward, qboolean chaingun)
{
        int     shots, i;

        if (chaingun)
                switch(self->s.frame)
                {
                default:
                case FRAME_attack1:
                        self->s.sound = actor_chaingun_loop;
                        shots = 3;
                        break;
                case FRAME_attack2:
                        self->s.sound = actor_chaingun_loop;
                        shots = 2;
                        break;
                case FRAME_attack3:
                        gi.sound(self, CHAN_AUTO, actor_chaingun_winddown, 1, ATTN_NORM, 0);
                        shots = 1;
                        break;
                }

        if (!chaingun)
        {
                actorMuzzleflash(self, start, MZ_MACHINEGUN);
                fire_bullet (self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_UNKNOWN);
        }
        else
                for (i=0 ; i<shots ; i++)
                {
                        actorMuzzleflash(self, start, MZ_CHAINGUN1+(self->s.frame-FRAME_attack1));
                        fire_bullet (self, start, forward, 5, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_UNKNOWN);
                }
}

void actorRailgun (edict_t *self, vec3_t start, vec3_t forward)
{
        actorMuzzleflash(self, start, MZ_RAILGUN);
        fire_rail (self, start, forward, 50, 100);
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
	ai_move, -13, NULL,
	ai_move, 14,  NULL,
	ai_move, 3,   NULL,
        ai_move, -2,  NULL
};
mmove_t actor_move_death1 = {FRAME_death101, FRAME_death106, actor_frames_death1, actor_dead};

mframe_t actor_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 7,   NULL,
	ai_move, -6,  NULL,
	ai_move, -5,  NULL,
	ai_move, 1,   NULL,
        ai_move, 0,   NULL
};
mmove_t actor_move_death2 = {FRAME_death201, FRAME_death206, actor_frames_death2, actor_dead};

void actor_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

        if (attacker)
                CheckSeenKiller(self, attacker);

// remove weapon model
        self->s.modelindex2 = 0;

// check for gib
	if (self->health <= -80)
	{
//		gi.sound (self, CHAN_VOICE, actor.sound_gib, 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand() % 2;
	if (n == 0)
        {
		self->monsterinfo.currentmove = &actor_move_death1;
                gi.sound (self, CHAN_VOICE, gi.soundindex(va("../players/%s/death1.wav", self->model)), 1, ATTN_NORM, 0);
        }
	else
        {
		self->monsterinfo.currentmove = &actor_move_death2;
                gi.sound (self, CHAN_VOICE, gi.soundindex(va("../players/%s/death2.wav", self->model)), 1, ATTN_NORM, 0);
        }
}


void actor_attacktarget (edict_t *self, edict_t *targ)
{
	vec3_t	start, target;
	vec3_t	forward, right;
        vec3_t  offset;
//        int     flash_index;

        self->s.sound = 0;

        if ((self->s.frame != FRAME_attack1) && (self->style != 5))
                return; // other attack frames are purely for chaingun

	AngleVectors (self->s.angles, forward, right, NULL);
        VectorSet(offset, 24, 8, self->viewheight-8);
        G_ProjectSource (self->s.origin, offset, forward, right, start);
        if (targ)
	{
                if (targ->health > 0)
		{
                        VectorMA (targ->s.origin, -0.2, targ->velocity, target);
                        target[2] += targ->viewheight;
		}
		else
		{
                        VectorCopy (targ->absmin, target);
                        target[2] += (targ->size[2] / 2);
		}
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	else
	{
		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

        switch(self->style)
        {
        case 1:
                actorBlaster(self, start, forward, false);
                break;
        case 2:
                actorShotgun(self, start, forward, false);
                break;
        case 3:
                actorShotgun(self, start, forward, true);
                break;
        case 4:
                actorMachinegun(self, start, forward, false);
                if (level.time >= self->monsterinfo.pausetime)
                        self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
                else
                        self->monsterinfo.aiflags |= AI_HOLD_FRAME;
                break;
        case 5:
                actorMachinegun(self, start, forward, true);
                if (level.time >= self->monsterinfo.pausetime)
                        self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
                else
                        self->monsterinfo.aiflags |= AI_HOLD_FRAME;
                break;
        case 8:
                actorBlaster(self, start, forward, true);
                if (level.time >= self->monsterinfo.pausetime)
                        self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
                else
                        self->monsterinfo.aiflags |= AI_HOLD_FRAME;
                break;
        case 9:
                actorRailgun(self, start, forward);
                break;
        }
}

void actor_fire (edict_t *self)
{
        if (self->enemy)
                actor_attacktarget(self, self->enemy);
        else
                actor_attacktarget(self, NULL);
}

void actor_reload (edict_t *self)
{
        if (self->style <= 4)
                self->s.frame = FRAME_attack8;
}

mframe_t actor_frames_attack [] =
{
	ai_charge, -2,  actor_fire,
        ai_charge, -2,  actor_fire,     // extra shots are for chaingun
        ai_charge, 3,   actor_fire,
        ai_charge, 2,   NULL,
        ai_charge, 1,   actor_reload,   // some weapon reload quicker than others
        ai_charge, 0,   NULL,
        ai_charge, 0,   NULL,
        ai_charge, 0,   NULL
};
mmove_t actor_move_attack = {FRAME_attack1, FRAME_attack8, actor_frames_attack, actor_run};

void actor_attack(edict_t *self)
{
        int             n;

	self->monsterinfo.currentmove = &actor_move_attack;
        n = rand()&15+self->style;
        self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}


void actor_use (edict_t *self, edict_t *other, edict_t *activator)
{
	vec3_t		v;

        if (!self->target)
                return;

	self->goalentity = self->movetarget = G_PickTarget(self->target);
	if ((!self->movetarget) || (strcmp(self->movetarget->classname, "target_actor") != 0))
	{
		gi.dprintf ("%s has bad target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
		self->target = NULL;
//                self->monsterinfo.pausetime = 100000000;
		self->monsterinfo.stand (self);
		return;
	}

	VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
	self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
	self->monsterinfo.walk (self);
	self->target = NULL;
}


/*QUAKED misc_actor (1 .5 0) (-16 -16 -24) (16 16 32)

Properties:
style   Weapon to use
model   Name of model in models/actor to use (without .md2)
count   Skin to use
message Actor name

Spawnflags:
8       No VWEP (will use plain weapon.md2)
16      Don't attack until first target_actor

*/

void SP_misc_actor (edict_t *self)
{
        int     n=0;

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

        // NOTE: we don't need to precache the actor sounds, they're already precached in worldspawn
        actor_chaingun_loop = gi.soundindex("weapons/chngnl1a.wav");
        actor_chaingun_winddown = gi.soundindex("weapons/chngnd1a.wav");
        actor_hyperb_loop = gi.soundindex("weapons/hyperbl1a.wav");
        actor_blaster_fire = gi.soundindex("weapons/blastf1a.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

// model & skin
        if (!self->model)
        {
                n = random()*3;
                switch(n)
                {
                default:
                case 0:
                        self->model = "male";
                        self->count = random()*4;
                        if (!self->message)
                                self->message = actor_names_male[self->count];
                        break;
                case 1:
                        self->model = "female";
                        self->count = random()*4;
                        if (!self->message)
                                self->message = actor_names_female[self->count];
                        break;
                case 2:
                        self->model = "cyborg";
                        self->count = random()*3;
                        if (!self->message)
                                self->message = actor_names_cyborg[self->count];
                        break;
                }
                n = 0;
        }

        self->s.modelindex = gi.modelindex(va("models/actor/%s.md2", self->model));

        if (!self->style)
        {
                self->style = random()*7+1;     // make sure actor has a weapon
                if (self->style > 5)
                        self->style += 2;
        }

// weapon model
setweap:
        if (!(self->spawnflags & 8))
                switch(self->style)
                {
                case 1:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_blaster.md2", self->model));
                        break;
                case 2:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_shotgun.md2", self->model));
                        break;
                case 3:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_sshotgun.md2", self->model));
                        break;
                case 4:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_machinegun.md2", self->model));
                        break;
                case 5:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_chaingun.md2", self->model));
                        break;
                case 8:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_hyperblaster.md2", self->model));
                        break;
                case 9:
                        self->s.modelindex2 = gi.modelindex(va("players/%s/w_railgun.md2", self->model));
                        break;
                default:
                        if ((self->style == 6)
                                || (self->style == 7)
                                || (self->style >= 10) )
                                gi.dprintf("QuakeActor Error 201 at %s: see errors.txt for details.\n", vtos(self->s.origin));
                        self->style = random()*5+1;     // make sure actor has a weapon
                        goto setweap;
                        break;
                }
        else
                self->s.modelindex2 = gi.modelindex(va("players/%s/weapon.md2", self->model));

	if (!self->health)
		self->health = 100;
	self->mass = 200;

	self->pain = actor_pain;
	self->die = actor_die;

	self->monsterinfo.stand = actor_stand;
	self->monsterinfo.walk = actor_walk;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.attack = actor_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;

	self->monsterinfo.aiflags |= AI_GOOD_GUY;

        gi.linkentity(self);

	self->monsterinfo.currentmove = &actor_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
        self->s.skinnum = self->count; // monster_start resets skin

	self->use = actor_use;
}


/*QUAKED target_actor (.5 .3 0) (-8 -8 -8) (8 8 8) JUMP SHOOT ATTACK x HOLD BRUTAL
Spawnflags:
1               jump in set direction upon reaching this target
2               take a single shot at the pathtarget
4               attack pathtarget until it or actor is dead
8               don't attack until next target_actor
16              trigger pathtarget

for ATTACK only:
32              hold position
64              be brutal

"target"        next target_actor
"pathtarget"    target of any action to be taken at this point
"wait"          amount of time actor should pause at this point
"message"       actor will "say" this to the player

for JUMP only:
"speed"         speed thrown forward (default 200)
"height"        speed thrown upwards (default 200)
*/

void target_actor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t  v, v2;
        edict_t *targ;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	other->goalentity = other->movetarget = NULL;

	if (self->message)
	{
		int		n;
		edict_t	*ent;

		for (n = 1; n <= game.maxclients; n++)
		{
			ent = &g_edicts[n];
			if (!ent->inuse)
				continue;
                        gi.cprintf (ent, PRINT_CHAT, "%s: %s\n", other->message, self->message);
		}
	}

        if (self->wait)
               other->monsterinfo.wait = level.time + self->wait;

	if (self->spawnflags & 1)		//jump
	{
		other->velocity[0] = self->movedir[0] * self->speed;
		other->velocity[1] = self->movedir[1] * self->speed;
		
		if (other->groundentity)
		{
			other->groundentity = NULL;
			other->velocity[2] = self->movedir[2];
			gi.sound(other, CHAN_VOICE, gi.soundindex("player/male/jump1.wav"), 1, ATTN_NORM, 0);
		}
	}


        if (self->spawnflags & 2)       //shoot
                if (self->pathtarget)
                {
                        targ = G_PickTarget(self->pathtarget);
                        other->monsterinfo.pausetime = level.time;
                        actor_attacktarget(other, targ);
                        if (targ->use)
                                targ->use(targ, other, other);
                }
                else
                        gi.dprintf("QuakeActor Error 101 at %s: see errors.txt for details.\n", vtos(self->s.origin));

	else if (self->spawnflags & 4)	//attack
	{
                if (self->pathtarget)
                {
                        other->enemy = G_PickTarget(self->pathtarget);
                        if (other->enemy)
                        {
                                other->goalentity = other->enemy;
                                if (self->spawnflags & 64)
                                        other->monsterinfo.aiflags |= AI_BRUTAL;
                                if (self->spawnflags & 32)
                                {
                                        other->monsterinfo.aiflags |= AI_STAND_GROUND;
                                        actor_stand (other);
                                }
                                else
                                {
                                        actor_run (other);
                                }
                        }
                }
                else
                        gi.dprintf("QuakeActor Error 102 at %s: see errors.txt for details.\n", vtos(self->s.origin));
	}

        // adjust No Attack value
        if ((self->spawnflags & 8) && !(other->spawnflags & 16))
                other->spawnflags |= 16;
        else if (!(self->spawnflags & 8) && (other->spawnflags & 16))
                other->spawnflags &= ~16;


        // trigger pathtarget
        if ((self->spawnflags & 16) && self->pathtarget)
        {
                targ = G_PickTarget(self->pathtarget);
                if (targ->use)
                        targ->use(targ, other, other);
                else
                        gi.dprintf("QuakeActor Error 104 at %s: see errors.txt for details.\n", vtos(self->s.origin));
        }
        else if (self->spawnflags & 16)
                        gi.dprintf("QuakeActor Error 103 at %s: see errors.txt for details.\n", vtos(self->s.origin));                   

        if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

        if (self->target)
                other->movetarget = G_PickTarget(self->target);
        else
        {
                other->monsterinfo.pausetime = level.time + 100000000;
                other->monsterinfo.stand(other);
                return;
        }

        if (!other->goalentity)
                other->goalentity = other->movetarget;

	if (!other->movetarget && !other->enemy)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else if (other->movetarget == other->goalentity)
	{
		VectorSubtract (other->movetarget->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_target_actor (edict_t *self)
{
	if (!self->targetname)
		gi.dprintf ("%s with no targetname at %s\n", self->classname, vtos(self->s.origin));

	self->solid = SOLID_TRIGGER;
	self->touch = target_actor_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags = SVF_NOCLIENT;

	if (self->spawnflags & 1)
	{
		if (!self->speed)
			self->speed = 200;
		if (!st.height)
			st.height = 200;
		if (self->s.angles[YAW] == 0)
			self->s.angles[YAW] = 360;
		G_SetMovedir (self->s.angles, self->movedir);
		self->movedir[2] = st.height;
	}

	gi.linkentity (self);
}

void Cmd_CreateActor_f (edict_t *ent)
{
        edict_t *new;
        vec3_t  org, ang;
        int     spot;
       
        new = G_Spawn();
        new->classname = "misc_actor";

        SelectSpawnPoint (new, org, ang);

        spot = gi.pointcontents(org);
        if (spot & MASK_PLAYERSOLID)
        {
                gi.dprintf("Unable to create actor\n");
                G_FreeEdict(new);
                return;
        }

        VectorCopy(org, new->s.origin);
        VectorCopy(ang, new->s.angles);

        SP_misc_actor(new);
}
