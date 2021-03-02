#include "g_local.h"

#define	sound_sight1 gi.soundindex("float/fltsrch1.wav")
#define rocketlauncher_flash MZ2_TANK_ROCKET_1

static int sound_idle;
int MeanOfDeath;

void sentry_fire_inflictor (edict_t *self, vec3_t v, vec3_t targetdir);
void sentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point); 
void sentry_run (edict_t *sentry);
void sentry_attack (edict_t *sentry);
void sentry_noise (edict_t *ent);

void ShowGun (edict_t *nt);
qboolean FindTarget (edict_t *self);

mmove_t sentry_move_attack2;
mmove_t sentry_move_stand1;

#define W_LASER    1
#define W_CHAINGUN 2

#define W_GREANDES 3
#define W_ROCKETL  4
//#define W_HROCKETL 5
#define W_RAILGUN  5

qboolean CheckArea (edict_t *obj, edict_t *builder)
{
	vec3_t src, end;
	int pos;
	trace_t tr;

	// Check the origin
	pos = gi.pointcontents(obj->s.origin); 
	if (pos == 1 || pos == 2) 
		return false;

	// Check the surrounding area
	VectorSet (src, obj->s.origin[0] + obj->maxs[0] + 16, obj->s.origin[1] + obj->maxs[1] + 16, obj->s.origin[2] + obj->maxs[2] + 16);

	pos = gi.pointcontents(src); 

	if (pos == 1 || pos == 2) 
		return false;

	// Check the surrounding area
	VectorSet (end, obj->s.origin[0] + obj->mins[0] - 16, obj->s.origin[1] + obj->mins[1] - 16, obj->s.origin[2] + obj->mins[2] - 16);

	tr = gi.trace (src, NULL, NULL, end, obj, (MASK_SOLID | MASK_WATER | MASK_MONSTERSOLID | MASK_PLAYERSOLID));

	if (tr.fraction != 1)
		return false;

	pos = gi.pointcontents(end); 

	if (pos == 1 || pos == 2) 
		return false;

	// extend the size a little
	VectorSet (src, obj->s.origin[0] + obj->mins[0] - 16, obj->s.origin[1] + obj->maxs[1] + 16, obj->s.origin[2] + obj->maxs[2] + 16);

	pos = gi.pointcontents(src); 
	if (pos == 1 || pos == 2) 
		return false;

	VectorSet (end, obj->s.origin[0] + obj->maxs[0] + 16, obj->s.origin[1] + obj->mins[1] - 16, obj->s.origin[2] + obj->mins[2] - 16);

	tr = gi.trace (src, NULL, NULL, end, obj, (MASK_SOLID | MASK_WATER | MASK_MONSTERSOLID | MASK_PLAYERSOLID));

	if (tr.fraction != 1)
		return false;

	pos = gi.pointcontents(end); 
	if (pos == 1 || pos == 2) 
		return false;

	// Trace a line from the player to the object too
	tr = gi.trace (builder->s.origin, NULL, NULL, obj->s.origin, builder, (MASK_SOLID | MASK_WATER | MASK_MONSTERSOLID | MASK_PLAYERSOLID));

	if (tr.fraction != 1)
		return false;

	// may add in more checks later

	return true;
}

/*
=============================
Sentry attack functions
=============================
*/

void Sentry_ChangeWeapon (edict_t *self)
{
	if (self->slugs)
	{
		self->speed = W_RAILGUN;
		self->bounces = self->slugs;
		self->s.modelindex2 = gi.modelindex ("models/weapons/s_rail/tris.md2");
	}
	else if (self->cells)
	{
		self->speed = W_LASER;
		self->bounces = self->cells;
		self->s.modelindex2 = gi.modelindex ("models/weapons/s_rocket/tris.md2");
	}
	else if (self->rockets)
	{
		self->speed = W_ROCKETL;
		self->bounces = self->rockets;
		self->s.modelindex2 = gi.modelindex ("models/weapons/s_rocket/tris.md2");
	}
	else if (self->bullets)
	{
		self->speed = W_CHAINGUN;
		self->bounces = self->bullets;
		self->s.modelindex2 = gi.modelindex ("models/weapons/s_chain/tris.md2");
	}
}

/*
====================================
sentry_fire_inflictor

Sentry shoots bullets, slugs or rockets
dependable on its class and ammo.
If it can't see enemy, it doesn't shoot.
Should add more weapons?
=====================================
*/

void sentry_fire_inflictor (edict_t *self, vec3_t v, vec3_t targetdir)
{
	if (!visible(self->enemy, self))
		return;

	if (self->bounces == 0)
	{
		Sentry_ChangeWeapon (self);
		return;
	}

	if (self->speed == W_CHAINGUN)
	{
		sentry_noise (self);
       	monster_fire_bullet (self, v, targetdir, 10, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ_BOOMERGUN);
		MeanOfDeath = 1;
      	self->bounces--;
		self->bullets--;
	}
    else if (self->speed == W_ROCKETL)
	{
		monster_fire_rocket (self, v, targetdir, 40, 800, MZ_BOOMERGUN);
		MeanOfDeath = 2;
      	self->bounces--;
		self->rockets--;
	}
	else if (self->speed == W_RAILGUN)
	{
		monster_fire_railgun (self, v, targetdir, 60, 10, MZ_BOOMERGUN);
		MeanOfDeath = 3;
      	self->bounces--;
		self->slugs--;
	}
}

/*
===================================
sentry_attack2

Used for aiming. Code a-la monster.
===================================
*/

void sentry_attack2 (edict_t *self)
{
    vec3_t start;
    vec3_t forward, right;
	vec3_t target, offset;

	VectorSet (offset, 15, 18, 33);
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, offset, forward, right, start);

	if (self->enemy)
	{
		VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
		target[2] += self->enemy->viewheight - 8;
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	else
		AngleVectors (self->s.angles, forward, right, NULL);

    sentry_fire_inflictor (self, start, forward);
}

/*
=================================
ClearSentryAmmo

Clears sentry's ammo. Only shells
are still remaining.
=================================
*/

void ClearSentryAmmo (edict_t *ent)
{
	ent->bullets = ent->creator->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))];
	ent->rockets = 0;
	ent->slugs = 0;
	ent->cells = 0;
}

/*
=================================
sentry_run

Hack. Imagine sentry running. Urghh...
Gimme fresh water.
Actually replaces monster running with
firing.
=================================
*/

void sentry_run (edict_t *sentry)
{
    if (sentry->enemy->deadflag == DEAD_DEAD)
	{
		sentry->monsterinfo.currentmove = &sentry_move_stand1;
        FindTarget (sentry);
		return;
	}

	if (sentry->enemy)
		if (sentry->enemy->client)
			if (sentry->enemy->client->resp.s_team == sentry->ripstate)
			{
				sentry->monsterinfo.currentmove = &sentry_move_stand1;
                FindTarget (sentry);
				return;
			}
	
	sentry->monsterinfo.attack(sentry);
}

/*
======================================
sentry_noise

Weapon noise.
======================================
*/

void sentry_noise (edict_t *ent)
{
	if (ent->s.modelindex2 == gi.modelindex ("models/weapons/s_chain/tris.md2"))
	    gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/chngnl1a.wav"), 1, ATTN_NORM, 0);
}

// actually nothing
void sentry_noise_rotate (edict_t *ent)
{
	gi.sound (ent, CHAN_VOICE, sound_sight1, 1, ATTN_NORM, 0);
}

void sentry_stand (edict_t *sentry);

mframe_t sentry_stand_frames[]=
{
    ai_stand, 0, sentry_noise_rotate,
	ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, sentry_noise_rotate,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL,
    ai_stand, 0, NULL
};
mmove_t sentry_move_stand1 = {0, 15, sentry_stand_frames, sentry_stand};

void sentry_stand (edict_t *sentry)
{
	if (sentry->enemy != NULL || sentry->goalentity != NULL)
	{
		if (visible (sentry->enemy, sentry))
			sentry->monsterinfo.attack(sentry);
		return;
	}

	sentry->monsterinfo.currentmove = &sentry_move_stand1;
}

void sentry_attack (edict_t *sentry);

mframe_t sentry_attack2_frames[]=
{
    ai_charge, 0, NULL,
	ai_charge, 0, NULL,
    ai_charge, 0, NULL,
    ai_charge, 0, NULL,
    ai_charge, -3, sentry_attack2,
    ai_charge, -2, NULL,
    ai_charge, -1, NULL,
    ai_charge, 0, NULL,
    ai_charge, 1, NULL,
    ai_charge, 2, NULL,
    ai_charge, 3, NULL,
    ai_charge, 0, NULL,
    ai_charge, 0, NULL,
    ai_charge, 0, NULL,
    ai_charge, 0, NULL,
    ai_charge, 0, NULL
};
mmove_t sentry_move_attack2 = {16, 27, sentry_attack2_frames, sentry_run};

mframe_t sentry_attack3_frames[]=
{
    ai_charge, 0, sentry_attack2,
	ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, -3, sentry_attack2,
    ai_charge, -2, sentry_attack2,
    ai_charge, -1, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 1, sentry_attack2,
    ai_charge, 2, sentry_attack2,
    ai_charge, 3, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2,
    ai_charge, 0, sentry_attack2
};
mmove_t sentry_move_attack3 = {16, 27, sentry_attack3_frames, sentry_run};

void sentry_expl (edict_t *self)
{
	if (!self->waterlevel)
        G_PointEntity(TE_GRENADE_EXPLOSION, self->s.origin, MULTICAST_PVS);
	else
        G_PointEntity(TE_GRENADE_EXPLOSION_WATER, self->s.origin, MULTICAST_PVS);

    G_FreeEdict (self);
}

void sentry_attack (edict_t *self)
{
	if (self->enemy && self->enemy->client)
	{
		if (self->enemy->client->resp.s_team == self->ripstate)
		{
     		self->monsterinfo.currentmove = &sentry_move_stand1;
            FindTarget (self);
			return;
		}
	}

	if (self->speed <= W_CHAINGUN)
	{
       	self->monsterinfo.currentmove = &sentry_move_attack3;
		AttackFinished (self, .01);
	}
	else
	{
       	self->monsterinfo.currentmove = &sentry_move_attack2;
		AttackFinished (self, 1);
	}
}

void sentry_pain (edict_t *sentry, edict_t *other, float kick, int damage);

mframe_t sentry_pain_frames[]=
{
    ai_move, -2, NULL,
	ai_move, -1, NULL,
	ai_move, 0, NULL,
	ai_move, 2, NULL
};
mmove_t sentry_move_pain1 = {28, 31, sentry_pain_frames, sentry_run};

void sentry_pain (edict_t *sentry, edict_t *other, float kick, int damage)
{
    sentry->monsterinfo.currentmove = &sentry_move_pain1;
}

/*
==============
Cmd_Sentry_f

This creats new sentry, sets it activator, and gives it 
a playerclass that is used later at firing.
==============
*/

//  // SIGHT logic  //
void decoy_sight(edict_t *self, edict_t *other)
{
}

void Sentry_AddAmmo (edict_t *ent, int mod, int quantity)
{
	int add;

	if (mod > ent->sentry->playerclass)
		return;

	if (mod == 0)
	{
		if (quantity > ent->health)
			quantity = ent->health - 1;

		if (ent->sentry->health > ent->sentry->max_health)
			return;

		if (ent->sentry->health + quantity > 200)
		{
			add = 200 - ent->sentry->health;

			if (quantity == ent->health)
				if (quantity < add)
					add = quantity;
		}
		else
			add = quantity;

        ent->health -= add; // substract health before counting burnout

        if (ent->burnout)
        {
			if ((rand() % 100) < add)
			{
				add -= ent->burnout;
				ent->burnout = 0;
			}
            else
                ent->burnout -= add;
        }

		ent->sentry->health += add;
		if (add != 0)
			gi.cprintf (ent, PRINT_HIGH, "Sentry repaired.\n");
		return;
	}
	else if (mod == 3)
	{
		if (quantity > ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))])
			quantity = ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))];

		if (ent->sentry->slugs >= 20)
			return;

		if (ent->sentry->slugs + quantity > 20)
		{
			add = 20 - ent->sentry->slugs;

			if (quantity == ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))])
				if (quantity < add)
					add = quantity;
		}
		else
			add = quantity;

		ent->sentry->slugs += add;
		gi.cprintf (ent, PRINT_HIGH, "Slugs added.\n");
        ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] -= add;
	}
	else if (mod == 2)
	{
		if (quantity > ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))])
			quantity = ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))];

		if (ent->sentry->rockets >= 50)
			return;

		if (ent->sentry->rockets + quantity > 50)
		{
			add = 50 - ent->sentry->rockets;

			if (quantity == ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))])
				if (quantity < add)
					add = quantity;
		}
		else
			add = quantity;

		ent->sentry->rockets += add;
		gi.cprintf (ent, PRINT_HIGH, "Rockets added.\n");
        ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] -= add;
	}
	else if (mod == 1)
	{
		if (quantity > ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))])
			quantity = ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))];

		if (ent->sentry->bullets >= 100)
			return;

		if (ent->sentry->bullets + quantity > 100)
		{
			add = 100 - ent->sentry->bullets;

			if (quantity == ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))])
				if (quantity < add)
					add = quantity;
		}
		else
			add = quantity;

		ent->sentry->bullets += add;
		gi.cprintf (ent, PRINT_HIGH, "Bullets added.\n");
        ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= add;
	}
	Sentry_ChangeWeapon (ent->sentry);
}

void Sentry_Sel (edict_t *ent, int choice) 
{
    if (choice == 0)
	    Sentry_AddAmmo (ent, 1, 20);
	else if (choice == 1)
        Sentry_AddAmmo (ent, 2, 4);
	else if (choice == 2)
        Sentry_AddAmmo (ent, 4, 4);
	else if (choice == 3)
        Sentry_AddAmmo (ent, 0, 15);
	else if (choice == 4)
	{
		if (ent->sentry->playerclass > 3)
			return;

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < 70)
		{
			gi.cprintf (ent, PRINT_HIGH, "Not enough energy\n");
			return;
		}

		gi.cprintf (ent, PRINT_HIGH, "Sentry upgraded.\n");
        ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] = 0;
		ent->sentry->playerclass++;

		ent->sentry->max_health = 100 * ent->sentry->playerclass;

		Sentry_ChangeWeapon (ent->sentry);
	}
    else if (choice == 5)
	{
		ent->sentry->ideal_yaw = ent->sentry->s.angles[YAW] += 45;
		gi.cprintf (ent, PRINT_HIGH, "Rotated 45 degrees to the left.\n");
	}
	else if (choice == 6)
        sentry_expl (ent->sentry);
} // MyWave_Sel

void Cmd_SentryM_f (edict_t *ent)
{
	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   if (ent->deadflag == DEAD_DEAD)
	   return;

   // send the layout
   
   Menu_Title(ent,"Sentry");
   Menu_Add(ent,"Add Bullets  ");
   Menu_Add(ent,"Add Rockets  ");
   Menu_Add(ent,"Add Slugs    ");
   Menu_Add(ent,"Repair       ");
   Menu_Add(ent,"Upgrade      ");
   Menu_Add(ent,"Rotate       ");
   Menu_Add(ent,"Dismantle    ");

    // Setup the User Selection Handler

   ent->client->usr_menu_sel = Sentry_Sel;
   Menu_Open(ent);
}

void Sentry_Menu (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	if (!G_ClientExists(other))
		return;

	if (other->client->resp.s_team != self->ripstate)
		return;

	if (other->playerclass != 6)
		return;

	Cmd_SentryM_f (other);
}

void Sentry_Think (edict_t *self)
{
    ClearSentryAmmo (self);

    self->movetype = MOVETYPE_NONE;
    self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
    self->s.effects = 0;
    self->s.frame = 0;
	self->touch = Sentry_Menu;
    self->model = "models/weapons/s_base/tris.md2";
    self->classname = "sentry";
    self->max_health = 100;
	self->playerclass = 1;
    self->ripstate = self->creator->client->resp.s_team;
    self->mass = 200;
    self->pain = sentry_pain;
    self->die = sentry_die;
    self->fireflags = 18 | FIREFLAG_DOWN | FIREFLAG_DELTA_VIEW | FIREFLAG_IGNITE;
	self->style = 1;

    self->monsterinfo.stand = sentry_stand;
    self->monsterinfo.walk = NULL;
    self->monsterinfo.run = sentry_run;
    self->monsterinfo.dodge = NULL;
    self->monsterinfo.attack = sentry_attack;
    self->monsterinfo.melee = NULL;
    self->monsterinfo.sight = decoy_sight;

    self->monsterinfo.aiflags |= AI_NOSTEP;

    self->health = 100;
    self->gib_health = -30;

    gi.linkentity (self);

	M_droptofloor (self);

    // First animation sequence
    self->monsterinfo.stand (self);

    //Let monster code control this decoy
    walkmonster_start (self);

	self->creator->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] -= 70;
    self->creator->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] = 0;
	self->creator->teamstate &= ~STATE_BUILDING;
   	self->creator->client->ps.gunindex = gi.modelindex(self->creator->client->pers.weapon->view_model);
	Sentry_ChangeWeapon (self);
}

void Sentry_health (edict_t *ent)
{
	if (ent->health == 100)
	{
		ent->nextthink = level.time + .1;
		ent->think = Sentry_Think;
		return;
	}

	if (rand() & 1)
		if (rand() & 1)
            G_ImpactEntity (TE_BLASTER, ent->s.origin, vec3_origin, MULTICAST_PVS);

	ent->health += 2;
	ent->nextthink = level.time + .1;
}

void Cmd_Build_f (edict_t *owner)
{
    edict_t *self;
    vec3_t forward;

	if (owner->sentry != NULL && owner->sentry->inuse && owner->sentry->classname && owner->sentry->health > 0)
	{
		gi.cprintf (owner, PRINT_HIGH, "You already have a sentrygun\n");
		return;
	}

	if (owner->playerclass != 6)
	{
		gi.cprintf (owner, PRINT_HIGH, "Only scientist can build a sentry gun\n");
		return;
	}

	if (owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 70)
	{
		gi.cprintf (owner, PRINT_HIGH, "Not enough energy cells\n");
		return;
	}

    self = G_Spawn();

    // Place sentry 100 units forward of our position
    AngleVectors(owner->client->v_angle, forward, NULL, NULL);
    VectorMA(owner->s.origin, 100, forward, self->s.origin);

	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 25);

	if (!CheckArea (self, owner))
	{
		gi.cprintf (owner, PRINT_HIGH, "Not enough space to build sentrygun\n");
		G_FreeEdict (self);
		return;
	}

    //Link two entities together
    owner->sentry = self;	//for the owner, this is a pointer to the decoy
    self->creator = owner;	//for the decoy, this is a pointer to the owner
	owner->teamstate |= STATE_BUILDING;
	self->s.modelindex = gi.modelindex ("models/weapons/s_base/tris.md2");
	self->nextthink = level.time + .1;
	self->takedamage = DAMAGE_AIM;
	self->die = sentry_die;
	self->think = Sentry_health;
	M_droptofloor (self);
	gi.linkentity (self);
}

void sentry_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	gi.linkentity (self);
}

mframe_t sentry_death_frames[]=
{
    ai_move, 0, NULL,
	ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL,
    ai_move, 0, NULL
};
mmove_t sentry_death_move = {32, 43, sentry_death_frames, sentry_dead};

void sentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (attacker != self->creator)
		gi.cprintf (self->creator, PRINT_HIGH, "Your sentrygun was destroyed by %s\n", attacker->client->pers.netname);

	sentry_expl (self);
}