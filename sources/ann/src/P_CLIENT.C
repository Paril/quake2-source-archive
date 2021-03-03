#include "g_local.h"
#include "m_player.h"
#include "g_brrr.h"

qboolean Jet_Active(edict_t *ent);
void Jet_BecomeExplosion(edict_t *ent, int damage);
void Jet_ApplyJet(edict_t *ent, usercmd_t *ucmd);
qboolean Jet_AvoidGround(edict_t *ent);

void Think_Airstrike(edict_t *ent);

/* *** X-RADIO *** */
extern  cvar_t  *radio_power;
/* *** */

/* *** ANN TURRET *** */
#if 0
void annturret_left(edict_t *ent);
void annturret_right(edict_t *ent);
void annturret_fire(edict_t *ent);
#endif
/* *** */

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void BleedNow(edict_t *self);

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	 edict_t *spot;
	 vec3_t  d;

	 spot = NULL;

	 while(1)
	 {
		  spot = G_Find(spot, FOFS(classname), "info_player_start");
		  if (!spot)
				return;
		  if (!spot->targetname)
				continue;
		  VectorSubtract(self->s.origin, spot->s.origin, d);
		  if (VectorLength(d) < 384)
		  {
				if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
				{
//              gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
					 self->targetname = spot->targetname;
				}
				return;
		  }
	 }
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	 edict_t *spot;

	 if(stricmp(level.mapname, "security") == 0)
	 {
		  spot = G_Spawn();
		  spot->classname = "info_player_coop";
		  spot->s.origin[0] = 188 - 64;
		  spot->s.origin[1] = -164;
		  spot->s.origin[2] = 80;
		  spot->targetname = "jail3";
		  spot->s.angles[1] = 90;

		  spot = G_Spawn();
		  spot->classname = "info_player_coop";
		  spot->s.origin[0] = 188 + 64;
		  spot->s.origin[1] = -164;
		  spot->s.origin[2] = 80;
		  spot->targetname = "jail3";
		  spot->s.angles[1] = 90;

		  spot = G_Spawn();
		  spot->classname = "info_player_coop";
		  spot->s.origin[0] = 188 + 128;
		  spot->s.origin[1] = -164;
		  spot->s.origin[2] = 80;
		  spot->targetname = "jail3";
		  spot->s.angles[1] = 90;

		  return;
	 }
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	 if (!coop->value)
		  return;
	 if(stricmp(level.mapname, "security") == 0)
	 {
		  // invoke one of our gross, ugly, disgusting hacks
		  self->think = SP_CreateCoopSpots;
		  self->nextthink = level.time + FRAMETIME;
	 }
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	 if (!deathmatch->value)
	 {
		  G_FreeEdict (self);
		  return;
	 }
	 SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	 if (!coop->value)
	 {
		  G_FreeEdict (self);
		  return;
	 }

	 if((stricmp(level.mapname, "jail2") == 0)   ||
		 (stricmp(level.mapname, "jail4") == 0)   ||
		 (stricmp(level.mapname, "mine1") == 0)   ||
		  (stricmp(level.mapname, "mine2") == 0)   ||
		 (stricmp(level.mapname, "mine3") == 0)   ||
		 (stricmp(level.mapname, "mine4") == 0)   ||
		 (stricmp(level.mapname, "lab") == 0)     ||
		 (stricmp(level.mapname, "boss1") == 0)   ||
		 (stricmp(level.mapname, "fact3") == 0)   ||
		 (stricmp(level.mapname, "biggun") == 0)  ||
		 (stricmp(level.mapname, "space") == 0)   ||
		 (stricmp(level.mapname, "command") == 0) ||
		 (stricmp(level.mapname, "power2") == 0) ||
		 (stricmp(level.mapname, "strike") == 0))
	 {
		  // invoke one of our gross, ugly, disgusting hacks
		  self->think = SP_FixCoopSpots;
		  self->nextthink = level.time + FRAMETIME;
	 }
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	 // player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	 char        *info;

	 if (!ent->client)
		  return false;

	 info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	 if (info[0] == 'f' || info[0] == 'F')
		  return true;
	 return false;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	 int         mod;
	 char        *message;
	 char        *message2;
	 qboolean    ff;

	 if (coop->value && (int) attacker->client)
		  meansOfDeath |= MOD_FRIENDLY_FIRE;

	 if (deathmatch->value || coop->value)
	 {
		  ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		  mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		  message = NULL;
		  message2 = "";

		  switch (mod)
		  {
		  case MOD_SUICIDE:
				message = "suicides";
				break;
		  case MOD_FALLING:
				message = "cratered";
				break;
		  case MOD_CRUSH:
				message = "was squished";
				break;
		  case MOD_WATER:
				message = "sank like a rock";
				break;
		  case MOD_SLIME:
				message = "melted";
				break;
		  case MOD_LAVA:
				message = "does a back flip into the lava";
				break;
		  case MOD_EXPLOSIVE:
		  case MOD_BARREL:
				message = "blew up";
				break;
		  case MOD_EXIT:
				message = "found a way out";
				break;
		  case MOD_TARGET_LASER:
				message = "saw the light";
				break;
		  case MOD_TARGET_BLASTER:
				message = "got blasted";
				break;
		  case MOD_BOMB:
		  case MOD_SPLASH:
		  case MOD_TRIGGER_HURT:
				message = "was in the wrong place";
				break;
		  case MOD_JETPACK_EXPLODE:   /* *** JETPACK *** */
			if (IsFemale(self))
					 message = "took her jetpack for a swim";
			else
				message = "took his jetpack for a swim";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
				case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
                message = "should have used a smaller gun";
                break;
            case MOD_KAMIKAZE:  /* *** KAMIKAZE MODE *** */
				if (IsFemale(self))
                    message = "saw her back";
                else
                    message = "saw his back";
					 break;
            case MOD_LASERYAYA: /* *** DEFENSE LASER *** */
                if (IsFemale(self))
                    message = "tripped over her own laser";
                else
                    message = "tripped over his own laser";
                break;
            case MOD_PIPEBOMB:  /* *** PIPEBOMB *** */
                if (IsFemale(self))
                    message = "forgot about her pipebomb";
                else
                    message = "forgot about his pipebomb";
                break;
            case MOD_PROXIMITY: /* *** PROXIMITY MINE *** */
                if (IsFemale(self))
                    message = "found her proximity mine";
                else
                    message = "found his proximity mine";
                break;
            case MOD_HOMING:    /* *** HOMING MISSILE *** */
                if (IsFemale(self))
                    message = "targeted herself";
                else
                    message = "targeted himself";
					 break;
            case MOD_NUKE:      /* *** NUKE *** */
                message = "tried to rocket-jump with a nuke";
                break;
            case MOD_AIRSTRIKE: /* *** AIRSTRIKE *** */
                message = "didn't know what an airstrike entailed";
                break;
            case MOD_BOUNCE:    /* *** BOUNCE ROCKET *** */
                if (IsFemale(self))
                    message = "played ping-pong with herself";
				else
                    message = "played ping-pong with himself";
                break;
            case MOD_GUIDED:    /* *** GUIDED MISSILE *** */
					 if (IsFemale(self))
                    message = "should have kept an eye on her missile";
                else
                    message = "should have kept an eye on his missile";
                break;
            case MOD_LASERDRONE:/* *** LASERDRONE *** */
                if (IsFemale(self))
                    message = "forgot about her LaserDrone";
                else
                    message = "forgot about his LaserDrone";
					 break;
            case MOD_CHAMBER:   /* *** CHAMBER *** */
                if (IsFemale(self))
                    message = "tortured herself. Ooh-err";
                else
                    message = "tortured himself. Ooh-err";
                break;
			case MOD_CLUSTER:   /* *** CLUSTER GRENADES *** */
				if (IsFemale(self))
					message = "Clustered herself";
				else
					message = "Clustered himself";
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
                break;
            }
        }
        if (message)
        {
            gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
            if (deathmatch->value)
				self->client->resp.score--;
            self->enemy = NULL;
            return;
        }

        self->enemy = attacker;
        if (attacker && attacker->client)
        {
            switch (mod)
            {
            case MOD_BLASTER:
                message = "was blasted by";
                break;
            case MOD_SHOTGUN:
                message = "was gunned down by";
                break;
            case MOD_SSHOTGUN:
                message = "was blown away by";
                message2 = "'s super shotgun";
                break;
            case MOD_MACHINEGUN:
                message = "was machinegunned by";
                break;
            case MOD_CHAINGUN:
					 message = "was cut in half by";
                message2 = "'s chaingun";
                break;
            case MOD_GRENADE:
                message = "was popped by";
                message2 = "'s grenade";
                break;
            case MOD_G_SPLASH:
                message = "was shredded by";
                message2 = "'s shrapnel";
                break;
            case MOD_ROCKET:
                message = "ate";
                message2 = "'s rocket";
				break;
            case MOD_R_SPLASH:
                message = "almost dodged";
                message2 = "'s rocket";
					 break;
            case MOD_HYPERBLASTER:
                message = "was melted by";
                message2 = "'s hyperblaster";
                break;
            case MOD_RAILGUN:
					 message = "was railed by";
                break;
            case MOD_BFG_LASER:
                message = "saw the pretty lights from";
                message2 = "'s BFG";
                break;
            case MOD_BFG_BLAST:
                message = "was disintegrated by";
                message2 = "'s BFG blast";
                break;
            case MOD_BFG_EFFECT:
                message = "couldn't hide from";
                message2 = "'s BFG";
                break;
            case MOD_HANDGRENADE:
                message = "caught";
                message2 = "'s handgrenade";
                break;
            case MOD_HG_SPLASH:
                message = "didn't see";
                message2 = "'s handgrenade";
                break;
            case MOD_HELD_GRENADE:
                message = "feels";
					 message2 = "'s pain";
                break;
            case MOD_TELEFRAG:
                message = "tried to invade";
				message2 = "'s personal space";
                break;
//ZOID
            case MOD_GRAPPLE:
					 message = "was caught by";
                message2 = "'s grapple";
                break;
//ZOID
            case MOD_KAMIKAZE:      /* *** KAMIKAZE MODE *** */
                message = "didn't see the nuke on";
                message2 = "'s back";
                break;
            case MOD_LASERBLASTER:  /* *** LASERBLASTER *** */
                message = "got an extra good look at";
                message2 = "'s LaserBlaster";
                break;
            case MOD_LASERYAYA: /* *** DEFENSE LASER *** */
                message = "tripped over";
                message2 = "'s laser";
                break;
				case MOD_PIPEBOMB:  /* *** PIPEBOMB *** */
                message = "discovered";
                message2 = "'s pipebomb";
                break;
            case MOD_PROXIMITY: /* *** PROXIMITY *** */
                message = "didn't see";
                message2 = "'s proximity mine";
                break;
            case MOD_HOMING:    /* *** HOMING MISSILE *** */
                message = "tried to escape";
                message2 = "'s homing missile";
                break;
            case MOD_NUKE:      /* *** NUKE *** */
                message = "got in the way of";
                message2 = "'s nuke";
                break;
            case MOD_AIRSTRIKE: /* *** AIRSTRIKE *** */
                message = "didn't notice that it was raining";
				message2 = "'s rockets";
                break;
            case MOD_BFG_SUIT:  /* *** BFG ARMOUR *** */
                message = "dry-cleaned";
					 message2 = "'s BFG Suit";
                break;
				case MOD_SNIPER:    /* *** SNIPER RIFLE *** */
                message = "was sniped by";
                break;
            case MOD_BOUNCE:    /* *** BOUNCE ROCKET *** */
                message = "was blown up, and down, and up by";
                message2 = "'s bounce rocket";
                break;
            case MOD_GUIDED:    /* *** GUIDED MISSILE *** */
				message = "tried to run from";
				message2 = "'s guided missile";
				break;
			case MOD_PHASER:    /* *** PHASER *** */
				message = "was pulled out of phase by";
				break;
			case MOD_LASERDRONE:/* *** LASERDRONE *** */
				message = "was zapped by";
				message2 = "'s LaserDrone";
				break;
			case MOD_CHAMBER:   /* *** CHAMBER *** */
				message = "was tortured by";
				message2=". Ooh-err";
				break;
			case MOD_AUTOSHOT:  /* *** AUTO SHOTGUN *** */
				message = "chewed on";
				message2="'s boomstick";
				break;
			case MOD_CLUSTER:   /* *** CLUSTER GRENADES *** */
				message = "was clustered by";
				break;
			case MOD_DISRUPTOR: /* *** DISRUPTOR RIFLE *** */
				message = "was disrupted by";
				break;
			case MOD_SABER:		/* *** LIGHT SABER *** */
				message = "felt the force of";
				message2 = "'s Light Saber";
				break;
			default:
				message = "was killed by";
				break;
			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
					 if (deathmatch->value)
				{
				if (ff)
					attacker->client->resp.score--;
				else
					attacker->client->resp.score++;
			}
			return;
		}
	}
	}

	 gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

edict_t *TossClientWeapon (edict_t *self)
{
	gitem_t     *item;
	edict_t     *drop;
	qboolean    quad;
	float       spread;

//	if (!deathmatch->value)
//		return NULL;
// Tom: No, let's :)

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	 /* *** TOM: BUGFIX (FAKE DEATH) *** */
	if (item && (strcmp (item->pickup_name, "Grapple") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Light Saber") == 0))
		item = NULL;
	/* *** */

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	 {
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}

	return drop;
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
    vec3_t      dir;

    if (attacker && attacker != world && attacker != self)
    {
		  VectorSubtract (attacker->s.origin, self->s.origin, dir);
    }
    else if (inflictor && inflictor != world && inflictor != self)
    {
        VectorSubtract (inflictor->s.origin, self->s.origin, dir);
    }
    else
    {
        self->client->killer_yaw = self->s.angles[YAW];
        return;
    }

    if (dir[0]!=0 || dir[1]!=0) /* *** TOM - STOP "atan2: domain error" messages *** */
        self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int     n;

	/* *** GUIDED MISSILES *** */
    if (self->client->missile)
    {
        self->client->missile->takedamage=DAMAGE_NO;
        self->client->rocketview=false;
    	self->client->missile=NULL;
    }
	/* *** */

	/* *** KAMIKAZE *** */
    Kamikaze_Cancel(self);
    /* *** */

    /* *** AIRSTRIKE *** */
    self->client->airstrike_called = 0;

    /* *** CHAMBER *** */
    self->client->chamber=0;
    self->client->inchamber=0;
	/* *** */

    /* *** Stop cloaking */
    self->svflags &= ~SVF_NOCLIENT;
    self->client->cloakable = false;
    self->client->cloaking = 0;
    /* *** */

    /* *** FAKE DEATH *** */
    if (self->client->flg_faking)
    {
        self->deadflag=0;
        self->client->flg_faking=0;
    }
    /* *** */

    VectorClear (self->avelocity);

    self->takedamage = DAMAGE_YES;
	 self->movetype = MOVETYPE_TOSS;

    self->s.modelindex2 = 0;    // remove linked weapon model
//ZOID
    self->s.modelindex3 = 0;    // remove linked ctf flag
//ZOID

	self->s.angles[0] = 0;
    self->s.angles[2] = 0;

    self->s.sound = 0;
    self->client->weapon_sound = 0;

    self->maxs[2] = -8;

//  self->solid = SOLID_NOT;
    self->svflags |= SVF_DEADMONSTER;

	 if (!self->deadflag)
    {
        self->client->respawn_time = level.time + 1.0;
        LookAtKiller (self, inflictor, attacker);
		  self->client->ps.pmove.pm_type = PM_DEAD;
        /* *** DEFENSE LASER *** */
        if (Q_stricmp(inflictor -> classname,"laser_yaya") == 0)
        {
            ClientObituary (self, inflictor, inflictor->parent);
        }
        else
        {
            ClientObituary (self, inflictor, attacker);
        }
//ZOID
        CTFFragBonuses(self, inflictor, attacker);
//ZOID
        TossClientWeapon (self);
//ZOID
        CTFPlayerResetGrapple(self);
        CTFDeadDropFlag(self);
        CTFDeadDropTech(self);
//ZOID
        if (deathmatch->value && !self->client->showscores)
            Cmd_Help_f (self);      // show scores
    }

	// remove powerups
	 self->client->quad_framenum = 0;
    self->client->invincible_framenum = 0;
    self->client->breather_framenum = 0;
    self->client->enviro_framenum = 0;

    // clear inventory
    memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

    /* *** JETPACK *** */
    if ( Jet_Active(self) )
	 {
        Jet_BecomeExplosion( self, damage );
        /*stop jetting when dead*/
        self->client->Jet_framenum = 0;
    }
    else
    /* *** */
    if (self->health < -40)
    {   // gib
        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n= 0; n < 4; n++)
            ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead (self, damage);
//ZOID
		  self->client->anim_priority = ANIM_DEATH;
        self->client->anim_end = 0;
//ZOID
        self->takedamage = DAMAGE_NO;
    }
    else
    {   // normal death
        if (!self->deadflag)
        {
            static int i;

            i = (i+1)%3;
            // start a death animation
			self->client->anim_priority = ANIM_DEATH;
            if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
            {
                self->s.frame = FRAME_crdeath1-1;
                self->client->anim_end = FRAME_crdeath5;
            }
            else switch (i)
            {
            case 0:
                self->s.frame = FRAME_death101-1;
                self->client->anim_end = FRAME_death106;
					 break;
            case 1:
                self->s.frame = FRAME_death201-1;
                self->client->anim_end = FRAME_death206;
                break;
            case 2:
                self->s.frame = FRAME_death301-1;
                self->client->anim_end = FRAME_death308;
                break;
            }
            gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
        }
    }

    self->deadflag = DEAD_DEAD;

    gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
    gitem_t     *item;

    memset (&client->pers, 0, sizeof(client->pers));

    item = FindItem("Blaster");
    client->pers.selected_item = ITEM_INDEX(item);
    client->pers.inventory[client->pers.selected_item] = 1;

	/* *** LIGHT SABER *** */
	client->pers.inventory[ITEM_INDEX(FindItem("Light Saber"))] = 1;
    /* *** */

	client->pers.weapon = item;
//ZOID
    client->pers.lastweapon = item;
//ZOID

//ZOID
    item = FindItem("Grapple");
    client->pers.inventory[ITEM_INDEX(item)] = 1;
//ZOID

    client->pers.health         = 100;
    client->pers.max_health     = 100;

    client->pers.max_bullets    = 200;
    client->pers.max_shells     = 100;
    client->pers.max_rockets    = 50;
    client->pers.max_grenades   = 50;
    client->pers.max_cells      = 200;
    client->pers.max_slugs      = 50;

    /* TOM: Erm, I think these are unnecessary - client is memset'd above */

    /* *** PORTABLE TELEPORTER *** */
    client->teleport_stored = false;

    /* *** LASERDRONE *** */
	client->laserdrone=0;

	/* *** CHAMBER *** */
	client->chamber=0;
	client->inchamber=0;

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
//ZOID
    int ctf_team = client->resp.ctf_team;
//ZOID

    memset (&client->resp, 0, sizeof(client->resp));

//ZOID
    client->resp.ctf_team = ctf_team;
//ZOID

    client->resp.enterframe = level.framenum;
    client->resp.coop_respawn = client->pers;

//ZOID
    if (ctf->value && client->resp.ctf_team < CTF_TEAM1)
        CTFAssignTeam(client);
//ZOID
}

/*
==================
SaveClientData

Some information that should be persistant, like health,
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
    int     i;
    edict_t *ent;

    for (i=0 ; i<game.maxclients ; i++)
    {
        ent = &g_edicts[1+i];
        if (!ent->inuse)
				continue;
        game.clients[i].pers.health = ent->health;
        game.clients[i].pers.max_health = ent->max_health;
        game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
        if (coop->value)
            game.clients[i].pers.score = ent->client->resp.score;
	 }
}

void FetchClientEntData (edict_t *ent)
{
    ent->health = ent->client->pers.health;
    ent->max_health = ent->client->pers.max_health;
    if (ent->client->pers.powerArmorActive)
        ent->flags |= FL_POWER_ARMOR;
    if (coop->value)
        ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float   PlayersRangeFromSpot (edict_t *spot)
{
	 edict_t *player;
    float   bestplayerdistance;
    vec3_t  v;
    int     n;
    float   playerdistance;


    bestplayerdistance = 9999999;

    for (n = 1; n <= maxclients->value; n++)
    {
        player = &g_edicts[n];

        if (!player->inuse)
            continue;

		  if (player->health <= 0)
            continue;

        VectorSubtract (spot->s.origin, player->s.origin, v);
        playerdistance = VectorLength (v);

        if (playerdistance < bestplayerdistance)
            bestplayerdistance = playerdistance;
    }

    return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
    edict_t *spot, *spot1, *spot2;
	 int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
    {
        count++;
        range = PlayersRangeFromSpot(spot);
        if (range < range1)
        {
            range1 = range;
            spot1 = spot;
        }
        else if (range < range2)
        {
            range2 = range;
            spot2 = spot;
        }
    }

	 if (!count)
		return NULL;

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
        spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
        if (spot == spot1 || spot == spot2)
            selection++;
    } while(selection--);

    return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
    edict_t *bestspot;
    float   bestdistance, bestplayerdistance;
    edict_t *spot;


    spot = NULL;
    bestspot = NULL;
    bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
    {
        bestplayerdistance = PlayersRangeFromSpot (spot);

        if (bestplayerdistance > bestdistance)
        {
            bestspot = spot;
            bestdistance = bestplayerdistance;
        }
	 }

	 if (bestspot)
    {
        return bestspot;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

    return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
    if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
        return SelectFarthestDeathmatchSpawnPoint ();
    else
        return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
    int     index;
    edict_t *spot = NULL;
    char    *target;

	index = ent->client - game.clients;

    // player 0 starts in normal player spawn point
    if (!index)
        return NULL;

    spot = NULL;

    // assume there are four coop spots at each spawnpoint
    while (1)
    {
		  spot = G_Find (spot, FOFS(classname), "info_player_coop");
        if (!spot)
            return NULL;    // we didn't have enough...

        target = spot->targetname;
        if (!target)
            target = "";
        if ( Q_stricmp(game.spawnpoint, target) == 0 )
		  {   // this is a coop spawn point for one of the clients here
            index--;
            if (!index)
                return spot;        // this is it
        }
    }
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void    SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
    edict_t *spot = NULL;

    if (deathmatch->value)
//ZOID
        if (ctf->value)
            spot = SelectCTFSpawnPoint(ent);
		  else
//ZOID
            spot = SelectDeathmatchSpawnPoint ();
    else if (coop->value)
        spot = SelectCoopSpawnPoint (ent);

	 // find a single player start spot
    if (!spot)
    {
        while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
        {
            if (!game.spawnpoint[0] && !spot->targetname)
                break;

            if (!game.spawnpoint[0] || !spot->targetname)
                continue;

            if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
                break;
        }

        if (!spot)
        {
            if (!game.spawnpoint[0])
				{   // there wasn't a spawnpoint without a target, so use any
                spot = G_Find (spot, FOFS(classname), "info_player_start");
            }
            if (!spot)
                gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
        }
    }

    VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
    VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
    int     i;
    edict_t *ent;

    level.body_que = 0;
    for (i=0; i<BODY_QUEUE_SIZE ; i++)
    {
		  ent = G_Spawn();
        ent->classname = "bodyque";
    }
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int n;

    if (self->health < -40)
    {
        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n= 0; n < 4; n++)
            ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->s.origin[2] -= 48;
        ThrowClientHead (self, damage);
        self->takedamage = DAMAGE_NO;
    }
}

void CopyToBodyQue (edict_t *ent)
{
    edict_t     *body;

	 // grab a body que and cycle to the next one
    body = &g_edicts[(int)maxclients->value + level.body_que + 1];
    level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

    // FIXME: send an effect on the removed body

    gi.unlinkentity (ent);

    gi.unlinkentity (body);
    body->s = ent->s;
	 body->s.number = body - g_edicts;

    body->svflags = ent->svflags;
    VectorCopy (ent->mins, body->mins);
    VectorCopy (ent->maxs, body->maxs);
    VectorCopy (ent->absmin, body->absmin);
    VectorCopy (ent->absmax, body->absmax);
    VectorCopy (ent->size, body->size);
    body->solid = ent->solid;
    body->clipmask = ent->clipmask;
    body->owner = ent->owner;
    body->movetype = ent->movetype;

    body->die = body_die;
	 body->takedamage = DAMAGE_YES;

    gi.linkentity (body);
}


void respawn (edict_t *self)
{
    /* *** FAKE DEATH *** */
    if (self->client && self->client->flg_faking)
        return;
	/* *** */

	/* *** PIPEBOMBS *** */
	self->pipetagowner=NULL;
    /* *** */

    /* *** WELCOME *** */
//    brrr_welcome(self);
    /* *** */

	if (deathmatch->value || coop->value)
    {
        CopyToBodyQue (self);
        PutClientInServer (self);

        // add a teleportation effect
        self->s.event = EV_PLAYER_TELEPORT;

        // hold in place briefly
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		/* *** RESPAWN PROTECTION *** */
		if (respawn_protect->value>0)
			self->client->invincible_framenum = (int)level.framenum + respawn_protect->value;
		/* *** */

		self->client->respawn_time = level.time;

		return;
    }

    // restart the entire server
    gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
/* *** VWEP *** */
void ShowGun(edict_t *ent);
/* *** */
void PutClientInServer (edict_t *ent)
{
    vec3_t  mins = {-16, -16, -24};
    vec3_t  maxs = {16, 16, 32};
    int     index;
    vec3_t  spawn_origin, spawn_angles;
    gclient_t   *client;
    int     i;
    client_persistant_t saved;
    client_respawn_t    resp;

    /* *** WELCOME *** */
    brrr_welcome(ent);
    /* *** */

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
	 SelectSpawnPoint (ent, spawn_origin, spawn_angles);

    index = ent-g_edicts-1;
    client = ent->client;

    // deathmatch wipes most client data every spawn
    if (deathmatch->value)
    {
        char        userinfo[MAX_INFO_STRING];

		  resp = client->resp;
        memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant (client);
        ClientUserinfoChanged (ent, userinfo);
    }
    else if (coop->value)
    {
        int         n;
        char        userinfo[MAX_INFO_STRING];

        resp = client->resp;
        memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < MAX_ITEMS; n++)
        {
            if (itemlist[n].flags & IT_KEY)
                resp.coop_respawn.inventory[n] = client->pers.inventory[n];
        }
        client->pers = resp.coop_respawn;
        ClientUserinfoChanged (ent, userinfo);
        if (resp.score > client->pers.score)
            client->pers.score = resp.score;
    }
    else
	 {
        memset (&resp, 0, sizeof(resp));
    }

	 // clear everything but the persistant data
    saved = client->pers;
    memset (client, 0, sizeof(*client));
    client->pers = saved;
    if (client->pers.health <= 0)
        InitClientPersistant(client);
    client->resp = resp;

    // copy some data from the client to the entity
    FetchClientEntData (ent);

    // clear entity values
    ent->groundentity = NULL;
    ent->client = &game.clients[index];
    ent->takedamage = DAMAGE_AIM;
    ent->movetype = MOVETYPE_WALK;
    ent->viewheight = 22;
    ent->inuse = true;
    ent->classname = "player";
    ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	/* *** GUIDED MISSILES *** */
	if (!client->rocketview)
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
    {
        client->ps.fov = 90;
    }
    else
    {
        client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
        if (client->ps.fov < 1)
            client->ps.fov = 90;
        else if (client->ps.fov > 160)
            client->ps.fov = 160;
    }

    client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

    // clear entity state values
    ent->s.effects = 0;
    ent->s.skinnum = ent - g_edicts - 1;
    ent->s.modelindex = 255;        // will use the skin specified model
    /* *** VWEP *** */
    //ent->s.modelindex2 = 255;     // custom gun model
    ShowGun(ent);
	 /* *** */
    ent->s.frame = 0;
    VectorCopy (spawn_origin, ent->s.origin);
    ent->s.origin[2] += 1;  // make sure off ground
    VectorCopy (ent->s.origin, ent->s.old_origin);

    // set the delta angle
    for (i=0 ; i<3 ; i++)
		  client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

    ent->s.angles[PITCH] = 0;
    ent->s.angles[YAW] = spawn_angles[YAW];
    ent->s.angles[ROLL] = 0;
    VectorCopy (ent->s.angles, client->ps.viewangles);
    VectorCopy (ent->s.angles, client->v_angle);

    /* *** X-RADIO *** */
    if ((Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "1") == 0) || (Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "on") == 0))
        ent->client->resp.radio_power = 1;
    else if ((Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "2") == 0) || (Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "team") == 0))
        ent->client->resp.radio_power = 2;
    else
        ent->client->resp.radio_power = 0;
    /* *** */

//ZOID
    if (CTFStartClient(ent))
        return;
//ZOID

    if (!KillBox (ent))
    {   // could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	/* *** X-RADIO *** */
	if ((Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "1") == 0) || (Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "on") == 0))
		ent->client->resp.radio_power = 1;
	else if ((Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "2") == 0) || (Q_stricmp (Info_ValueForKey(ent->client->pers.userinfo, "radio_power"), "team") == 0))
		ent->client->resp.radio_power = 2;
	else
		ent->client->resp.radio_power = 0;
	/* *** */
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int     i;
	edict_t	*from;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inuse == true)
    {
        // the client has cleared the client side viewangles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (i=0 ; i<3 ; i++)
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
    }
    else
    {
		  // a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		  // ClientConnect() time
        G_InitEdict (ent);
        ent->classname = "player";
        InitClientResp (ent->client);
        PutClientInServer (ent);
    }

	if (level.intermissiontime)
    {
        MoveClientToIntermission (ent);
    }
    else
    {
        // send effect if in a multiplayer game
        if (game.maxclients > 1)
        {
            gi.WriteByte (svc_muzzleflash);
            gi.WriteShort (ent-g_edicts);
            gi.WriteByte (MZ_LOGIN);
            gi.multicast (ent->s.origin, MULTICAST_PVS);

            gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		  }
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	/* *** IR GOGGLES *** */
	if (!deathmatch->value)
	{
		for (from=g_edicts;from<&g_edicts[globals.num_edicts];from++)
		{
			if (!from->inuse)
				continue;

            if (from->svflags & SVF_MONSTER)
            {
				from->s.renderfx |= RF_IR_VISIBLE;
            }
		}
	}
	/* *** */
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
    char    *s;
    int     playernum;
    int     n;

    // check for malformed or illegal info strings
	 if (!Info_Validate(userinfo))
    {
        strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey (userinfo, "name");
    strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

    /* *** TOM *** */
    for (n=0;n<strlen(ent->client->pers.netname);n++)
    {
        if (ent->client->pers.netname[n]=='$')
            ent->client->pers.netname[n]='_';
	}

    if (strlen(ent->client->pers.netname)==0)
        strncpy(ent->client->pers.netname,"badinfo",sizeof(ent->client->pers.netname)-1);
    /* *** */

    // set skin
    s = Info_ValueForKey (userinfo, "skin");

    playernum = ent-g_edicts-1;

    // combine name and skin into a configstring
//ZOID
    if (ctf->value)
        CTFAssignSkin(ent, s);
    else
//ZOID
        gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

    // fov
    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
    {
        ent->client->ps.fov = 90;
    }
    else
    {
        ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
        if (ent->client->ps.fov < 1)
            ent->client->ps.fov = 90;
        else if (ent->client->ps.fov > 160)
            ent->client->ps.fov = 160;
    }

    // handedness
	 s = Info_ValueForKey (userinfo, "hand");
    if (strlen(s))
    {
        ent->client->pers.hand = atoi(s);
	}

    // save off the userinfo in case we want to check something later
    strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

    /* *** X-RADIO *** */
    if ((Q_stricmp (Info_ValueForKey(userinfo, "radio_power"), "1") == 0) || (Q_stricmp (Info_ValueForKey(userinfo, "radio_power"), "on") == 0))
        ent->client->resp.radio_power = 1;
    else if ((Q_stricmp (Info_ValueForKey(userinfo, "radio_power"), "2") == 0) || (Q_stricmp (Info_ValueForKey(userinfo, "radio_power"), "team") == 0))
        ent->client->resp.radio_power = 2;
    else
        ent->client->resp.radio_power = 0;
    /* *** */

    /* *** VWEP *** */
    ShowGun(ent);
    /* *** */
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char    *value;

	// check to see if they are on the banned IP list
	 value = Info_ValueForKey (userinfo, "ip");

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (strcmp(password->string, value) != 0)
		return false;

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	 // if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
//ZOID
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s @ %s connected\n", ent->client->pers.netname,Info_ValueForKey(ent->client->pers.userinfo, "ip"));

	ent->client->pers.connected = true;

	/* *** WELCOME *** */
//	ent->client->welcomecount=0;
	/* *** */

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int     playernum;

	if (!ent->client || !ent->inuse)
    	return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

	// send effect
	gi.WriteByte (svc_muzzleflash);
    gi.WriteShort (ent-g_edicts);
    gi.WriteByte (MZ_LOGOUT);
    gi.multicast (ent->s.origin, MULTICAST_PVS);

    gi.unlinkentity (ent);
    ent->s.modelindex = 0;
    ent->solid = SOLID_NOT;
    ent->inuse = false;
    ent->classname = "disconnected";
	ent->client->pers.connected = false;

	 playernum = ent-g_edicts-1;
    gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
    if (pm_passent->health > 0)
		  return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
    else
        return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
    int v,i;
    v = 0;
    for (i=0 ; i<c ; i++)
		  v+= ((byte *)b)[i];
    return v;
}
void PrintPmove (pmove_t *pm)
{
    unsigned    c1, c2;

    c1 = CheckBlock (&pm->s, sizeof(pm->s));
    c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
    Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t   *client;
	edict_t *other;
	 int     i, j;
	pmove_t pm;

	static int delay=10;

	/* *** MEMOSERV *** */
	if (ent->client->resp.welcomecount<WELCOME_TIME)
    {
		ent->client->resp.welcomecount++;
    }
    else if (ent->client->resp.welcomecount==WELCOME_TIME)
    {
        brrr_memos(ent);
		ent->client->resp.welcomecount++;
    }
	/* *** */

	/* *** DM PAUSE *** */
	if (dmpaused)
	{
		if (delay>=10)
		{
			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			return;
		}
		else
		{
			delay++;
		}
	}
	/* *** */

	 level.current_entity = ent;
	 client = ent->client;

	/* *** GUIDED MISSILES *** */
	if (client->was_guided && !client->rocketview) // back to normal
	{
		ent->client->was_guided = 0;
		//ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model); -- Seems to cause MAJOR crashes
	}
	/* *** */

	 if (level.intermissiontime)
	 {
		  client->ps.pmove.pm_type = PM_FREEZE;
		  // can exit intermission after five seconds
		  if (level.time > level.intermissiontime + 5.0
				&& (ucmd->buttons & BUTTON_ANY) )
				level.exitintermission = true;
		  return;
	}

	/* *** ANTI-ZBOT *** */
	if (botdetection)
	   BotDetection(ent, ucmd);
	/* *** */

	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

        /* *** IMPROVED CHASECAM *** */
        if(ucmd->forwardmove < 0)
        {
            ent->speed--;
            if(ent->speed < 0)
                ent->speed = 0;
        }
        else if(ucmd->forwardmove > 0)
		  {
            ent->speed++;
        }
        /* *** */

        return;
    }
//ZOID

    /* *** FREEZER *** */
    if (ent->client->frozen>0)
    {
        ent->client->ps.pmove.pm_time=100;
        ent->client->ps.pmove.pm_flags=PMF_TIME_TELEPORT;
        ent->client->frozen-=1;
        return;
    }
    /* *** */

    // set up for pmove
    memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
        client->ps.pmove.pm_type = PM_SPECTATOR;
    else if (ent->s.modelindex != 255)
        client->ps.pmove.pm_type = PM_GIB;
    else if (ent->deadflag)
        client->ps.pmove.pm_type = PM_DEAD;
    else
        client->ps.pmove.pm_type = PM_NORMAL;

    client->ps.pmove.gravity = sv_gravity->value;
    /* *** BOOTS *** */
    if (ent->flags & FL_BOOTS)
        client->ps.pmove.gravity = sv_gravity->value * 0.15;
    /* *** */
    /* *** JETPACK *** */
    if ( Jet_Active(ent) )
		  Jet_ApplyJet( ent, ucmd );
    /* *** */
    pm.s = client->ps.pmove;

    for (i=0 ; i<3 ; i++)
    {
        pm.s.origin[i] = ent->s.origin[i]*8;
        pm.s.velocity[i] = ent->velocity[i]*8;
    }

    if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
    {
        pm.snapinitial = true;
//      gi.dprintf ("pmove changed!\n");
    }

    pm.cmd = *ucmd;

    pm.trace = PM_trace;    // adds default parms
    pm.pointcontents = gi.pointcontents;

    // perform a pmove
	gi.Pmove (&pm);

    // save results of pmove
    client->ps.pmove = pm.s;
    client->old_pmove = pm.s;

    for (i=0 ; i<3 ; i++)
    {
        ent->s.origin[i] = pm.s.origin[i]*0.125;
        /* *** JETPACK *** */
        if ( !Jet_Active(ent) || (Jet_Active(ent)&&(fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )
        /* *** */
            ent->velocity[i] = pm.s.velocity[i]*0.125;
    }

    VectorCopy (pm.mins, ent->mins);
	 VectorCopy (pm.maxs, ent->maxs);

    client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
    client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
    client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

    /* *** JETPACK *** */
    if ( Jet_Active(ent) )
        if( pm.groundentity )           /*are we on ground?*/
            if ( Jet_AvoidGround(ent) ) /*then lift us if possible*/
                pm.groundentity = NULL; /*now we are no longer on ground*/
    /* *** */

    if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
    {
        gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
        PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

        /* *** BOOTS *** */
        if (ent->flags & FL_BOOTS)
        {
            if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]<2*BOOTS_ROCKETS)
			{
                ent->flags -= FL_BOOTS;
                gi.centerprintf (ent, "Anti-gravity boots disabled!\nOut of rockets!\n");
            }

            ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]-=BOOTS_ROCKETS;
        }
        /* *** */
    }

    ent->viewheight = pm.viewheight;
    ent->waterlevel = pm.waterlevel;
    ent->watertype = pm.watertype;
    ent->groundentity = pm.groundentity;
    if (pm.groundentity)
        ent->groundentity_linkcount = pm.groundentity->linkcount;

    if (ent->deadflag)
    {
        client->ps.viewangles[ROLL] = 40;
        client->ps.viewangles[PITCH] = -15;
        client->ps.viewangles[YAW] = client->killer_yaw;
    }
    else
    {
        VectorCopy (pm.viewangles, client->v_angle);
        VectorCopy (pm.viewangles, client->ps.viewangles);
	}

//ZOID
    if (client->ctf_grapple)
        CTFGrapplePull(client->ctf_grapple);
//ZOID

/* *** CLOAKING *** */
    if (ent->client->cloakable==1)
    {
        if (ucmd->forwardmove != 0 || ucmd->sidemove != 0)
		{
            ent->svflags &= ~SVF_NOCLIENT;
            ent->client->cloaking = 0;
        }
        else
        {
            if (!(ent->svflags & SVF_NOCLIENT))
            {
                if (ent->client->cloaking==1)
                {
					if (level.time > ent->client->cloaktime)
                    {
                        ent->svflags |= SVF_NOCLIENT;
                        ent->client->cloakdrain = 0;
                    }
                }
                else
                {
                    ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
                    ent->client->cloaking = 1;
                }
			}
        }
    }
    else
    {
        if (!ent->client->stealth_on)
            ent->svflags &= ~SVF_NOCLIENT;
    }
/* *** */

/* *** CLOAKING JAMMER * *** */
    // Reset visible cloaking flags

    if (ent->client->cloaking==2)
    {
        ent->client->cloaking=1;
        ent->client->cloakable=1;
		  ent->svflags &= SVF_NOCLIENT;
    }
/* *** */

/* *** IR GOGGLES *** */
	draingoggles(ent);
/* *** */

    gi.linkentity (ent);

    if (ent->movetype != MOVETYPE_NOCLIP)
        G_TouchTriggers (ent);

    // touch other objects
    for (i=0 ; i<pm.numtouch ; i++)
    {
        other = pm.touchents[i];
        for (j=0 ; j<i ; j++)
            if (pm.touchents[j] == other)
                break;
        if (j != i)
            continue;   // duplicated
		  if (!other->touch)
            continue;
        other->touch (other, ent, NULL, NULL);
    }


    client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
    client->latched_buttons |= client->buttons & ~client->oldbuttons;

    // save light level the player is standing on for
    // monster sighting AI
    ent->light_level = ucmd->lightlevel;

    // fire weapon from final position if needed
    if (client->latched_buttons & BUTTON_ATTACK
//ZOID
        && ent->movetype != MOVETYPE_NOCLIP
//ZOID
        )
    {
        if (!client->weapon_thunk)
        {
            client->weapon_thunk = true;
            Think_Weapon (ent);
        }
    }

//ZOID
//regen tech
    CTFApplyRegeneration(ent);
//ZOID

    /* *** KAMAKAZI MODE *** */
    if ((ent->client->kamikaze_framenum <= level.framenum) && (ent->client->kamikaze_mode & 1))
        Kamikaze_Explode(ent);
    /* *** */

//ZOID
    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;
        if (other->inuse && other->client->chase_target == ent)
            UpdateChaseCam(other);
    }
//ZOID

    /* *** BLEEDING *** */
    /* Make the player bleed if <40% health and not dead */
    if ((ent->health<40) && (ent->health>0))
    {
        BleedNow(ent);
    }

    /* *** AIRSTRIKE *** */
    if (client->airstrike_called && level.time > client->airstrike_time)
    {
		client->airstrike_called = 0;
        Think_Airstrike (ent);
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t   *client;
	int         buttonMask;

/*	static float x1=0.0f,y1=0.0f,z1=0.0f,x2=0.0f,y2=0.0f,z2=0.0f;

	if (ent->client->aktiv)
	{
	if (ent->s.origin[0]<x1 || x1==0.0f)
		x1=ent->s.origin[0];
	if (ent->s.origin[1]<y1 || y1==0.0f)
		y1=ent->s.origin[1];
	if (ent->s.origin[2]<z1 || z1==0.0f)
		z1=ent->s.origin[2];
	if (ent->s.origin[0]>x2 || x2==0.0f)
		x2=ent->s.origin[0];
	if (ent->s.origin[1]>y2 || y2==0.0f)
		y2=ent->s.origin[1];
	if (ent->s.origin[2]>z2 || z2==0.0f)
		z2=ent->s.origin[2];

	gi.dprintf("%.0f %.0f %.0f - %.0f %.0f %.0f\n",x1,y1,z1,x2,y2,z2);
	}*/

	if (level.intermissiontime)
		  return;

	client = ent->client;

	/* *** CONVERTING *** */
    convert(ent);
	/* *** */

    /* *** REGEN CELLS *** */
    if (((unsigned long)ann_allow2->value&(unsigned long)ANN_ALLOW2_REGENCELLS) && ent->solid != SOLID_NOT) // not spectator
    {
        ent->client->regenframe++;
        if (ent->client->regenframe==REGENCELLS_FRAMES)
        {
            client->regenframe=0;
            if (client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]+1<=client->pers.max_cells)
                client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]++;
        }
    }
    /* *** */

    /* *** CLOAKING *** */
    if (ent->client->cloaking)
    {
        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= CLOAK_AMMO)
        {
            ent->client->cloakdrain ++;
            if (ent->client->cloakdrain == CLOAK_DRAIN)
            {
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CLOAK_AMMO;
                ent->client->cloakdrain = 0;
            }
        }
        else
        {
            ent->svflags &= ~SVF_NOCLIENT;
            ent->client->cloaking = 0;
        }
    }
    /* *** */

    /* *** CLOAKING JAMMER *** */
    if (ent->client->jammer_power)
    {
        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= JAMMER_AMMO)
        {
            jammerthink(ent);

            ent->client->jammerdrain ++;
            if (ent->client->jammerdrain == JAMMER_DRAIN)
			{
                ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= JAMMER_AMMO;
                ent->client->jammerdrain = 0;
            }
        }
        else
        {
            ent->client->jammer_power=0;
            gi.centerprintf (ent, "Cloaking jammer disabled!\nOut of cells!\n");
        }
    }
    /* *** */

	/* *** ANN TURRET *** */
#if 0
	if (ent->client->at_left)
		annturret_left(ent);
	else if (ent->client->at_right)
		annturret_right(ent);
	else if (ent->client->at_fire)
		annturret_fire(ent);
#endif
	/* *** */

	// run weapon animations if it hasn't been done by a ucmd_t
    if (!client->weapon_thunk
//ZOID
        && ent->movetype != MOVETYPE_NOCLIP
//ZOID
        )
        Think_Weapon (ent);
    else
        client->weapon_thunk = false;

	if (ent->deadflag)
    {
        // wait for any button just going down
        if ( level.time > client->respawn_time)
        {
            // in deathmatch, only wait for attack button
            if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            else
                buttonMask = -1;

            if ( ( client->latched_buttons & buttonMask ) ||
                (deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
            {
                respawn(ent);
                client->latched_buttons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    if (!deathmatch->value)
        if (!visible (ent, PlayerTrail_LastSpot() ) )
            PlayerTrail_Add (ent->s.old_origin);

    client->latched_buttons = 0;
}
