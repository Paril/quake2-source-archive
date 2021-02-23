#include "g_local.h"
#include "m_player.h"
#include "z_gq.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

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
	edict_t	*spot;
	vec3_t	d;

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
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
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
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
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
	if(Q_stricmp(level.mapname, "security") == 0)
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
	number_of_spawns++;
//	SP_misc_teleporter_dest (self);
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

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
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
	char		*info;

	if (!ent->client)
		return false;
	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	char		*poss;
	qboolean	ff=false;
	qboolean	headshot=false;
	int			index;
	int			streak=self->client->resp.standard[GSTAT_KILLS]-self->client->resp.lastkills;

	if (streak > self->client->resp.standard[GSTAT_STREAK])
		self->client->resp.standard[GSTAT_STREAK]=streak;
	self->client->resp.standard[GSTAT_DEATHS]++;

	if ((meansOfDeath & MOD_FRIENDLY_FIRE) || (coop->value && attacker->client) || (deathmatch->value && OnSameTeam(self, attacker))) {
		ff = true;
		meansOfDeath &= ~MOD_FRIENDLY_FIRE;
//		debugmsg("Friendly fire\n");
	}

	if (meansOfDeath & MOD_HEAD_SHOT) {
		if (self->client && attacker->client && (self!=attacker)) {	// MONSTER'S DON'T HAVE CLIENTS
			if (announce_deaths->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
				gi_bprintf(PRINT_MEDIUM, "%s makes a fatal head shot against %s!\n", attacker->client->pers.netname, self->client->pers.netname);
			else
				gi_sprintf(PRINT_MEDIUM, "%s makes a fatal head shot against %s!\n", attacker->client->pers.netname, self->client->pers.netname);
		}
		meansOfDeath &= ~MOD_HEAD_SHOT;
		headshot=true;
	}

	if (deathmatch->value || coop->value)
	{
		mod = meansOfDeath;
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
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "forgot to throw";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "drinks down its own molotov cocktail";
				else if (IsFemale(self))
					message = "drinks down her own molotov cocktail";
				else
					message = "drinks down his own molotov cocktail";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_CANNON_BLAST:
				message = "should have used a smaller... uh... cannon";
				break;
			default:
				if (IsNeutral(self))
					message = "killed itself";
				else if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			if (announce_deaths->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
				gi_bprintf (PRINT_MEDIUM, "%s %s (streak: %i).\n", self->client->pers.netname, message, streak);
			else
				gi_sprintf (PRINT_MEDIUM, "%s %s (streak: %i).\n", self->client->pers.netname, message, streak);
			self->client->resp.standard[GSTAT_SUICIDES]++;
			if (deathmatch->value) {
				self->client->resp.score--;
				if (teamplay->value) {
//					debugmsg("Teamplay??\n");
					teamdata[self->client->team-1].score--;
				}
			}
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_MELEE:
				message = "was gutted by";
				break;
			case MOD_HIT:
				message = "was gutted by";
				break;
			case MOD_REVOLVER:
				message = "was filled with lead by";
				break;
			case MOD_DREVOLVER:
				message = "was given a double-dose of lead by";
				break;
			case MOD_SHOTGUN:
				message = "was perforated by";
				break;
			case MOD_SSHOTGUN:
				message = "got both barrels from";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_HATCHET:
				message = "got a facefull of";
				message2 = "'s hatchet";
				break;
			case MOD_WINCHESTER_RIFLE:
				message = "was plugged by";
				message2 = "'s Winchester rifle";
				break;
			case MOD_SHARPS_RIFLE:
				message = "was nailed by";
				message2 = ", the sharpshooter";
				break;
			case MOD_CANNON_BLAST:
				message = "was disintegrated by";
				message2 = "'s cannonball";
				break;
			case MOD_CANNON_EFFECT:
				message = "couldn't hide from";
				message2 = "'s cannon";
				break;
			case MOD_HANDGRENADE:
				message = "drank down";
				message2 = "'s molotov cocktail";
				break;
			case MOD_HG_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_FIRE:
				message = "is left a crispy critter by";
				break;
			case MOD_POISON:
				message = "keels over because of";
				message2 = "'s toxic nature";
				break;
			case MOD_BFG_LASER:
				message = "gets zapped by";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			}
			if (message)
			{
				index=ITEM_INDEX(FindItem("Money Bag"));
				if (ff) {
//					debugmsg("Team death.\n");
					if (IsNeutral(self))
						poss = "its";
					else if (IsFemale(self))
						poss = "her";
					else
						poss = "his";
					if (announce_deaths->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
						gi_bprintf (PRINT_MEDIUM,"%s %s %s teammate %s%s (streak: %i)\n", self->client->pers.netname, message, poss, attacker->client->pers.netname, message2, streak);
					else
						gi_sprintf (PRINT_MEDIUM,"%s %s %s teammate %s%s (streak: %i)\n", self->client->pers.netname, message, poss, attacker->client->pers.netname, message2, streak);
				} else {
					if (announce_deaths->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
						gi_bprintf (PRINT_MEDIUM,"%s %s %s%s (streak: %i)\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, streak);
					else
						gi_sprintf (PRINT_MEDIUM,"%s %s %s%s (streak: %i)\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, streak);
				}
				if (deathmatch->value)
				{
					int score=1;
					if (ff) {
						if (((int)playmode->value==PM_BIG_HEIST) && (self->client->pers.inventory[index])) {
							gi_bprintf (PRINT_MEDIUM,"%s killed a bag-carrying teammate!\n", attacker->client->pers.netname);
							score-=4;
						}
						attacker->client->resp.score-=score;
						self->client->resp.standard[GSTAT_TEAMKILLS]++;
						if (teamplay->value) {
//							debugmsg("Teamplay??\n");
							teamdata[attacker->client->team-1].score-=score;
						}
					} else {
						if (headshot)
							score++;
						if (((int)playmode->value == PM_BIG_HEIST) && self->client->pers.inventory[index]) {
							gi_bprintf (PRINT_MEDIUM,"%s killed a bag-carrying enemy!\n", attacker->client->pers.netname);
							attacker->client->resp.special[GSTAT_BAGKILLS]++;
							score++;
						}
						attacker->client->resp.score+=score;
						attacker->client->resp.standard[GSTAT_KILLS]++;

						if (teamplay->value) {
//							debugmsg("Teamplay??\n");
							teamdata[attacker->client->team-1].score+=score;
						}
						if ((int)playmode->value == PM_BADGE_WARS) {
							if (attacker->client->team==2)
								attacker->client->resp.special[GSTAT_OUTLAWKILLS]++;
							if (attacker->client->team==1) {
								attacker->client->resp.special[GSTAT_SHERIFFKILLS]++;
								attacker->client->resp.score+=score;
							}
						}
					}
				}
				return;
			}
		}
	}

	if (announce_deaths->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
		gi_bprintf (PRINT_MEDIUM,"%s died (streak: 0).\n", self->client->pers.netname, streak);
	else
		gi_bprintf (PRINT_MEDIUM,"%s died (streak: 0).\n", self->client->pers.netname, streak);
/*	if (deathmatch->value) {
		self->client->resp.score--;
		if (teamplay->value) {
			teamdata[self->client->team-1].score--;
		}
	}
*/
}

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Bowie Knife") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP) && ((int)playmode->value != PM_BADGE_WARS))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 24;
	else
		spread = 0.0;

	if (drop_all_on_death->value || weapons_unique->value) {
		int index,i;
		spread = 24;
		item=FindItem("Double Revolver");
		index=ITEM_INDEX(item);
		for (i=0;i<14;i++, index++, item++) {
			if (self->client->pers.inventory[index]>0) {
//				debugmsg("Dropping %s\n",item->pickup_name);
				self->client->v_angle[YAW] -= i*spread;
				drop = Drop_Item (self, item);
				self->client->v_angle[YAW] += i*spread;
				drop->spawnflags = DROPPED_PLAYER_ITEM;
			}
		}
	} else {
		if (item)
		{
			self->client->v_angle[YAW] -= spread;
			drop = Drop_Item (self, item);
			self->client->v_angle[YAW] += spread;
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
	}

	if (quad)
	{
//		debugmsg("Dropping quad.\n");
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		if (deathmatch->value && ((int)playmode->value == PM_BADGE_WARS)) {
			drop->s.effects |= EF_HYPERBLASTER;
			drop->nextthink = level.time + SPECIAL_RESPAWN_TIME + (rand() % SPECIAL_RESPAWN_RANGE);
			drop->think = GQ_RespawnSpecial;
		} else {
			drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
			drop->think = G_FreeEdict;
		}
	}
	if (self->client->artifact) {
//		debugmsg("Okay, throwing %s\n",self->client->artifact->pickup_name);
		drop = Drop_Item(self, self->client->artifact);
		drop->s.angles[0] = rand() % 360;
		drop->think = GQ_FinishSpecialSpawn;
		VectorSet(drop->velocity, 200-(rand()%100), 200-(rand()%100), (rand()%100)+100);
		VectorCopy (self->s.origin, drop->s.origin);
		drop->nextthink = level.time + .2;
		drop->movetype = MOVETYPE_BOUNCE;
		drop->s.renderfx = self->client->artifact->quantity;
		self->client->artifact=NULL;
	}
	item=FindItem("Money Bag");
	if (((int)playmode->value == PM_BIG_HEIST) && (self->client->pers.inventory[ITEM_INDEX(item)]>0)) {
		debugmsg("Dropping money bag!!!!\n");
		GQ_DropMoneyBag (self, item);
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

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

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
	

}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
	int		mod;

	debugmsg("Func: player_die\n");
	VectorClear (self->avelocity);

	self->s.effects = 0;
	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove badge or flag
	self->s.modelindex4 = 0;	// remove team indicator
	
	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	GQ_ClearSatellite(self);
	GQ_UnzoomPlayer(self);
	if (teamplay->value)
		GQ_CountTeamPlayers();
	RPS_MenuClose(self);
	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		TossClientWeapon (self);
		if (deathmatch->value) {
			Cmd_Help_f (self);		// show scores
		}
		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}
		CalcWeight(self);
	}

	if (deathmatch->value && ((int)playmode->value == PM_BADGE_WARS) && (self->client->team==2)) {
//		debugmsg("Setting ex-sheriff's team to 1\n");
		self->client->team=1;
	}
	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;
	self->flags &= ~FL_LANTERN;
	self->client->alt_fire_on = false;

	// Kills can't gib... unless killed by an explosion
	mod=meansOfDeath;
	mod&=~MOD_FRIENDLY_FIRE;
	mod&=~MOD_HEAD_SHOT;
	debugmsg("Mod: %i\n",mod);
	if ((self->health < -40) && ((mod == MOD_HANDGRENADE) || (mod == MOD_CANNON_BLAST) || (mod == MOD_CANNON_EFFECT)))
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
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
	self->client->resp.lastkills=self->client->resp.standard[GSTAT_KILLS];

	// Let's see if the game is over now
	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING) && (GQ_TeamsLeft()<2) && !gameoverframe) {
		gameoverframe=level.framenum+50;
		gi_bprintf(PRINT_HIGH, "All rivals have been eliminated\n");
	}
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
	gitem_t		*item;

	debugmsg("InitClientPersistant\n");
	memset (&client->pers, 0, sizeof(client->pers));

	// Added by Stone
	item = FindItem("Bowie Knife");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	item = FindItem("Revolver");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	client->pers.carried[ITEM_INDEX(item)] = item->capacity;
	client->pers.weapon = item;
	if ((int)playmode->value == PM_LAST_MAN_STANDING) {
		client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 20;
		client->pers.cash=(int)starting_cash->value;
		debugmsg("Setting cash\n");
		client->pers.max_bullets45LC= 200;
		client->pers.max_shells		= 30;
		client->pers.max_hatchets	= 20;
		client->pers.max_cocktails	= 20;
		client->pers.max_bullets3030= 30;
		client->pers.max_bullets4570= 20;
		client->pers.max_cannonballs= 8;
		item = FindItem("Lantern");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	} else {
		client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 24;
		client->pers.max_bullets45LC= 50;
		client->pers.max_shells		= 25;
		client->pers.max_hatchets	= 5;
		client->pers.max_cocktails	= 5;
		client->pers.max_bullets3030= 30;
		client->pers.max_bullets4570= 20;
		client->pers.max_cannonballs= 8;
	}
	
	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
	client->alt_fire_on = false;
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
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	debugmsg("FetchClientEntData\n");
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
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
float	PlayersRangeFromSpot (edict_t *spot, edict_t *ent)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		if (player == ent)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

//		debugmsg("%s range: %f\n", player->client->pers.netname, playerdistance);
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
edict_t *SelectRandomDeathmatchSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
//		debugmsg("Searching...\n");
		count++;
		range = PlayersRangeFromSpot(spot, ent);
//		debugmsg("Range: %f\n", range);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
//			debugmsg("Spot1\n");
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
//			debugmsg("Spot2\n");
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
//		debugmsg("Finding info_player_deathmatch\n");
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
edict_t *SelectFarthestDeathmatchSpawnPoint (edict_t *ent)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot, ent);

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

edict_t *SelectDeathmatchSpawnPoint (edict_t *ent)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint (ent);
	else
		return SelectRandomDeathmatchSpawnPoint (ent);
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

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
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles, qboolean teamspawn)
{
	edict_t	*spot = NULL;

//	debugmsg("Selecting spawn point\n");
	if (deathmatch->value) {
		if (teamplay->value && teamspawn) {
			spot = GQ_SelectTeamSpawnPoint(ent);
			if (!spot) {
//				debugmsg("No spot found!\n");
				spot=SelectDeathmatchSpawnPoint(ent);
			}
		} else
			spot = SelectDeathmatchSpawnPoint (ent);
	} else if (coop->value)
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
			{	// there wasn't a spawnpoint without a target, so use any
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
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

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
	edict_t		*body;

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
	if (deathmatch->value || coop->value)
	{
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;
		self->client->alt_fire_on = false;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (teamplay->value)
		GQ_CountTeamPlayers();
	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			gi_cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			if (!(ent->flags & FL_ANTIBOT)) {
				gi.WriteByte (svc_stufftext);
				gi.WriteString ("spectator 0\n");
				gi.unicast(ent, true);
			}
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi_cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			if (!(ent->flags & FL_ANTIBOT)) {
				gi.WriteByte (svc_stufftext);
				gi.WriteString ("spectator 0\n");
				gi.unicast(ent, true);
			}
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			gi_cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			if (!(ent->flags & FL_ANTIBOT)) {
				gi.WriteByte (svc_stufftext);
				gi.WriteString ("spectator 1\n");
				gi.unicast(ent, true);
			}
			return;
		}
	}

	debugmsg("Clear score\n");
	// clear client on respawn
	debugmsg("Team: %i\n", ent->client->team);
	if (teamplay->value && ent->client->team) {
		teamdata[ent->client->team-1].score-=ent->client->resp.score;
	}
	debugmsg("Done clearing score\n");

	ent->client->resp.score = ent->client->pers.score = 0;

	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		ent->client->respawn_framenum = level.framenum+(RESPAWN_INVULN_TIME*10);
	}

	ent->client->respawn_time = level.time;

	if (ent->client->pers.spectator) {
		gi_bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
		ent->client->team=0;
	} else {
		if (teamplay->value) {
			gi_bprintf (PRINT_HIGH, "%s joined team %s\n", ent->client->pers.netname, teamdata[ent->client->team-1].name->string);
			teamdata[ent->client->team-1].players++;
		} else
			gi_bprintf (PRINT_HIGH, "%s has joined the game.\n", ent->client->pers.netname);
	}
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int		team;
//	float	u, z;

	debugmsg("PutClientInServer\n");
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if (teamplay->value)
		GQ_CountTeamPlayers();
	if ((int)playmode->value == PM_LAST_MAN_STANDING)
		SelectSpawnPoint (ent, spawn_origin, spawn_angles, false);
	else
		SelectSpawnPoint (ent, spawn_origin, spawn_angles, true);
	GQ_UnzoomPlayer(ent);

	index = ent-g_edicts-1;
	client = ent->client;
	team = client->team;
//	u = client->unzoomsensitivity;
//	z = client->zoomsensitivity;
	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
//		for (n = 0; n < game.num_items; n++)
//		{
//			if (itemlist[n].flags & IT_KEY)
//				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
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
	client->alt_fire_on = false;
	client->idon = true;
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
	client->poison = NULL;

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model

	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	// Keep this data... in deathmatch, pers is cleared every death and resp every level
	ent->client->team=team;
//	ent->client->unzoomsensitivity=u;
//	ent->client->zoomsensitivity=z;

	GQ_SetupClient(ent, spawn_origin, spawn_angles);

	// spawn a spectator
	if (client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->client->ps.pmove.pm_type = PM_SPECTATOR;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		ent->s.modelindex3 = 0;
		ent->s.modelindex4 = 0;
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	if (ent->flags & FL_ANTIBOT)	// Antibot
		return;

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	client->resp.lastrank=0;
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

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi_bprintf (PRINT_HIGH, "%s has arrived.\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
void foo (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (ent->client && other->client)
		debugmsg("Ent %s touched by %s\n", ent->client->pers.netname, other->client->pers.netname);
}
*/

void ClientBegin (edict_t *ent)
{
	int		i;

//	gi.error("Stopped at ClientBegin\n");
//	ent->touch = foo;
	ent->client = game.clients + (ent - g_edicts - 1);

	ent->client->pers.hit_antibot = 0;	// Antibot

	// Set up the important aliases
	ent->message=RPS_AddToStringPtr(ent->message, "alias +alt_fire \"cmd +alt_fire\"\n", true);
	ent->message=RPS_AddToStringPtr(ent->message, "alias -alt_fire \"cmd -alt_fire\"\n", true);
	ent->message=RPS_AddToStringPtr(ent->message, "alias +reload \"cmd +reload\"\n", true);
	ent->message=RPS_AddToStringPtr(ent->message, "alias -reload \"cmd -reload\"\n", true);

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

			gi_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ent->client->reloadcount=-1;
	ent->client->alt_fire_on = false;
	ClientEndServerFrame (ent);
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
	char	*s;
	int		playernum, i;
	char	tempstr[50];
	char	tempname[16];
//	qboolean	zoom, unzoom;
	edict_t *player;

	debugmsg("ClientUserinfoChanged\n");
//	gi.error("Stopped at ClientUserinfoChanged\n");
	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (tempname, s, sizeof(tempname)-1);
	tempname[15]='\0';
	if (!(ent->flags & FL_ANTIBOT)) {
//		debugmsg("Userinfo: %s\n", userinfo);
		//	debugmsg("S name: %s\n", s);
//		debugmsg("tempname: %s\n", tempname);
//		debugmsg("Old name: %s\n", ent->client->pers.netname);
	}
	if (!(ent->flags & FL_ANTIBOT) && Q_strcasecmp(tempname, ent->client->pers.netname)) {
		qboolean match=false;
//		debugmsg("New name: %s\n", tempname);
		for (i = 1; i <= maxclients->value; i++) {
			player = &g_edicts[i];
			if (!player->inuse)
				continue;
			if (!Q_strcasecmp(tempname, player->client->pers.netname)) {
//				debugmsg("Matches existing player!!\n");
				if (strlen(ent->client->pers.netname)<1) {
					debugmsg("Sending 'set name Player'\n", tempstr);
					ent->message=RPS_AddToStringPtr(ent->message,"set name Player\n", true);
//					debugmsg("Just joining\n");
//					debugmsg("Setting name to Player\n");
					strcpy (ent->client->pers.netname, "Player");
				} else {
					if (ent->client->resp.enterframe+10 < level.framenum)
						gi_centerprintf (ent, "The name \"%s\" is already taken!\nYou are now known as \"Player\".", tempname);
//					debugmsg("Setting name back to whatever\n");
					sprintf(tempstr, "set name %s\n", ent->client->pers.netname); 
//					debugmsg("Sending '%s'\n", tempstr);
					ent->message=RPS_AddToStringPtr(ent->message, tempstr, true);
				}
				match=true;
			}
		}
		if (!match) {
//			debugmsg("New name okay!\n");
			strcpy (ent->client->pers.netname, tempname);
		}
	} else {
		strcpy (ent->client->pers.netname, tempname);
//		debugmsg("Setting name to %s\n", tempname);
	}

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

/*	Sensitivities are no longer passed to the server
	if (!(ent->flags & FL_ANTIBOT)) {
		//	debugmsg("%s\n",userinfo);
		//	debugmsg("Old: %f/%f\n", ent->client->zoomsensitivity, ent->client->unzoomsensitivity);
		s = Info_ValueForKey (userinfo, "unzoomsensitivity");
		if (strcmp(s,"")) {
			unzoom=true;
			ent->client->unzoomsensitivity=atof(s);
			//		debugmsg("New unzoom sensitivity: %s/%f\n", s, ent->client->unzoomsensitivity);
		}
		
		s = Info_ValueForKey (userinfo, "zoomsensitivity");
		if (strcmp(s,"")) {
			zoom=true;
			ent->client->zoomsensitivity=atof(s);
			//		debugmsg("New zoom sensitivity: %s/%f\n", s, ent->client->zoomsensitivity);
		}
		//	debugmsg("Int: %f/%f\n", ent->client->zoomsensitivity, ent->client->unzoomsensitivity);
		
		s = Info_ValueForKey (userinfo, "sensitivity");
		if (strcmp(s,"")) {
			if (zoom && ent->client->zoomed && (ent->client->zoomsensitivity!=atof(s))) {
				//			debugmsg("New zoom sensitivity: %f\n", ent->client->zoomsensitivity);
				sprintf(tempstr, "set zoomsensitivity %s\n", s);
				//			debugmsg("Sending stuffcmd\n");
				ent->message=RPS_AddToStringPtr(ent->message, tempstr, true);
			}
			if (unzoom && !ent->client->zoomed && (ent->client->unzoomsensitivity!=atof(s))) {
				
				//			debugmsg("New unzoom sensitivity: %f\n", ent->client->unzoomsensitivity);
				sprintf(tempstr, "set unzoomsensitivity %s\n", s);
				//			debugmsg("Sending stuffcmd\n");
				ent->message=RPS_AddToStringPtr(ent->message, tempstr, true);
			}
		}
		//	debugmsg("New: %f/%f\n", ent->client->zoomsensitivity, ent->client->unzoomsensitivity);
	}
*/

	// set skin
//	s = Info_ValueForKey (userinfo, "skin");

	// Antibot stuff
	if (!(ent->flags & FL_ANTIBOT))
		s = CheckModel(Info_ValueForKey (userinfo, "skin"));
	else
		s = Info_ValueForKey (userinfo, "skin");
	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	if (!(ent->flags & FL_ANTIBOT)) {
		if (teamplay->value) {
			if (use_ctf_skins->value) {
				CTFAssignSkin(ent, s);
			} else {
				GQ_AttachTriangle(ent);
				gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
			}
		} else {
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
		}
	} else
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

	// Antibot
	s = Info_ValueForKey (userinfo, "cl_not_bot");
	if (strlen(s))
		ent->client->pers.allowed_download = true;
	else
		ent->client->pers.allowed_download = false;

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
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
	char	*value;

//	gi.error("Stopped at ClientConnect\n");
	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}


	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		debugmsg ("%s connected\n", ent->client->pers.netname);

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;
	ent->client->alt_fire_on = false;
	ent->client->team = 0;

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
	int		playernum;

	if (!ent->client)
		return;

	gi_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
	if ((teamplay->value) && ent->client->team)
		teamdata[ent->client->team-1].players--;
	
	if (teamplay->value)
		GQ_CountTeamPlayers();
	GQ_DestroySatellite(ent);
	GQ_DropUniques(ent);
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


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

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
	gclient_t	*client;
	edict_t		*other;
	int			i, j, z;
	pmove_t		pm;
	int			topspeed;
	float		speed;

	// Limit player speed
	if (item_weight->value) {
		topspeed=400-((ent->client->pers.weight-15)*4);
		if (topspeed>400)
			topspeed=400;
		if (GQ_MatchItem(ent->client->artifact,"Strength of the Bear")) {
			if (topspeed<160)
				topspeed=160;
		} else {
			if (topspeed<80)
				topspeed=80;
		}
//		if (level.framenum % 15 == 0) {
//			ent->message=RPS_AddToStringPtr(ent->message, va("set cl_forwardspeed %i\nset cl_sidespeed %i\n", topspeed, topspeed), true);
//		}
//		gi_bprintf(PRINT_HIGH, tempstr);
//		sprintf(tempstr, "set cl_sidespeed %i\n", topspeed);
//		ent->message=AddToStringPtr(ent->message, tempstr);
//		gi_bprintf(PRINT_HIGH, tempstr);
		speed=sqrt((ucmd->forwardmove*ucmd->forwardmove)+(ucmd->sidemove*ucmd->sidemove));
		if (speed > topspeed) {
			ucmd->forwardmove*=(topspeed/speed);
			ucmd->sidemove*=(topspeed/speed);
		}
		if (ucmd->forwardmove>topspeed) {
			ucmd->forwardmove=topspeed;
		}
		if (ucmd->sidemove>topspeed) {
			ucmd->sidemove=topspeed;
		}
//		debugmsg("T: %i  F: %i  S: %i\n", topspeed, ucmd->forwardmove, ucmd->sidemove);
	}
	// Z-bot detection
	if (ucmd->impulse) {
		if (zk_logonly->value)
			gi.dprintf ("[ZKick]: client#%d @ %s is a bot (impulse=%d)\n",
			(ent-g_edicts)-1, Info_ValueForKey (ent->client->pers.userinfo,"ip"),
			ucmd->impulse);
		else
		{
			gi.dprintf ("[ZKick]: client#%d @ %s was kicked (impulse=%d)\n",
				(ent-g_edicts)-1, Info_ValueForKey (ent->client->pers.userinfo,"ip"),
				ucmd->impulse);
			ent->message=RPS_AddToStringPtr(ent->message, "quit\n", true);
		}
	}
	// End z-bot detection

	level.current_entity = ent;
	client = ent->client;

	if (ent->flags & FL_ANTIBOT)
		return;
	
	//remove the bot client for our game 
	if(ent->client->pers.hit_antibot >= 5){
//		stuffcmd (ent, "impulse 33\n");
		ent->message=RPS_AddToStringPtr(ent->message, "impulse 33\n", true);
		OnBotDetection(ent, "impulse 33 ");
	}

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			if (matchstats) {
				edict_t *player;
				matchstats=false;
				level.intermissiontime = level.time - 2;
				for (i=0 ; i<maxclients->value ; i++) {
					player = g_edicts + 1 + i;
					if (!player->inuse)
						continue;
					DeathmatchScoreboard(player, true);
				}

			}
			else {
				level.exitintermission = true;
			}
		return;
	}

	pm_passent = ent;
	if ((client->ps.fov<90) && !client->zoomed) {
		client->ps.fov=90;
	}

	if (ent->client->chase_target) {

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

		if (item_weight->value) {
//			gi_bprintf(PRINT_HIGH, "Velocity1: ");
//			printvector(ent->velocity);
//			if (item_weight->value && (ent->velocity[2]>topspeed+50) && (ucmd->upmove>0)) {
//				ent->velocity[2]=topspeed+50;
//			}
			i=(2*topspeed)-300;
			if ((ent->velocity[2]>i) && ((ucmd->forwardmove!=0) || (ucmd->sidemove!=0) || (ucmd->upmove!=0))) {
				ent->velocity[2]=i;
			}
			if (ent->waterlevel>2) {
				ucmd->upmove+=(2*topspeed)-800;;
			}
//			gi_bprintf(PRINT_HIGH, "Velocity2: ");
//			printvector(ent->velocity);
		}
		// set up for pmove
		memset (&pm, 0, sizeof(pm));

		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->s.modelindex != 255)
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else if (!item_weight->value && (ent->client->pers.weapon->weapmodel == WEAP_CANNON)) {
			client->ps.pmove.pm_type = PM_DEAD;
		} else
			client->ps.pmove.pm_type = PM_NORMAL;

		client->ps.pmove.gravity = sv_gravity->value;
		pm.s = client->ps.pmove;

		for (i=0 ; i<3 ; i++)
		{
			pm.s.origin[i] = ent->s.origin[i]*8;
			pm.s.velocity[i] = ent->velocity[i]*8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		{
			pm.snapinitial = true;
	//		debugmsg ("pmove changed!\n");
		}

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		// perform a pmove
		gi.Pmove (&pm);

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

//		debugmsg("Upward velocity: %f\n", (pm.s.origin[2]*0.125)-ent->s.origin[2]);
		z=(pm.s.origin[2]*0.125)-ent->s.origin[2];
		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
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
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch (other, ent, NULL, NULL);
		}

	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;
			
			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);
			
		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
	
	// Fire alternate weapon -- Added by Stone
	if (client->alt_fire_on == true)
	{
		//            gi_cprintf(ent, PRINT_HIGH, "Alt firing!");
		if (client->resp.spectator) {
 
			client->latched_buttons = 0;

			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}


	// Update lantern position
	if ((ent->flags & FL_LANTERN) && (ent->waterlevel > 2))
		ent->flags &= ~FL_LANTERN;

	GQ_UpdateSatellite(ent);
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
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

	if (level.framenum % 16 == 4)
		ent->message=RPS_AddToStringPtr(ent->message, "set gl_dynamic 1\n", true);

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator)
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
				ent->enemy=NULL;
				if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING))
					spectator_respawn(ent);
				else
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

	GQ_GameEffects (ent);
}
