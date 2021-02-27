#include "g_local.h"
#include "m_player.h"

// Skid Added
#include "m_genqguy.h"
#include "g_genutil.h"
#include "g_genmidi.h"
#include "g_gensock.h"
// End Skid

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//
// this function is an ugly as hell hack to fix some map flaws
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
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
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
            {
//                              gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
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
           (Q_stricmp(level.mapname, "power2") == 0)  ||
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
void SP_info_player_intermission(edict_t *self)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        // player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
        char            *info;

        if (!ent->client)
                return false;

        info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
        if (info[0] == 'f' || info[0] == 'F')
                return true;
        return false;
}

qboolean IsNeutral (edict_t *ent)
{
        char            *info;

        if (!ent->client)
                return false;

        info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
        if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
                return true;
        return false;
}


/*
==============
OBITUARY

modified for Doom/Quake  messages -Skid
==============
*/

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
        int                     mod;
        char            *message;
        char            *message2;
        qboolean        ff;

        int                     player_class;
        player_class =self->client->resp.player_class;

        if (coop->value && attacker->client)
                meansOfDeath |= MOD_FRIENDLY_FIRE;

// Skid added
        
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
// Skid added                   
                        {
                                switch(player_class)
                                {
                                        case CLASS_Q1:
                                        {
                                                if(IsFemale(self))
                                                        message = "fell to her death"; 
                                                else
                                                        message = "fell to his death";
                                                break;
                                        }
                                        case CLASS_Q2:
                                        default:
                                                message = "cratered";
                                                break;
                                }
                                break;
                        }
// End Skid
                case MOD_CRUSH:
                        message = "was squished";
                        break;
                case MOD_WATER:
// Skid added                   
                        {
                                switch(player_class)
                                {
                                        case CLASS_Q1:
                                        {
                                                if(random() >0.5)
                                                        message = "sleeps with the fishes";
                                                else
                                                        message = "sucks it down";
                                                break;
                                        }
                                        case CLASS_Q2:
                                        default:
                                                message = "sank like a rock";
                                                break;
                                }
                                break;
                        }
                case MOD_SLIME:
                        {
                                switch(player_class)
                                {
										case CLASS_DOOM:
											{
												message = "goes slimey dipping";
												break;
											}
                                        case CLASS_Q1:
                                        {
                                                if(random() >0.5)
                                                        message = "gulped a load of slime";
                                                else
                                                        message = "can't exist on slime alone";
                                                break;
                                        }
                                        case CLASS_Q2:
                                        default:
                                                message = "melted";
                                                break;
                                }
                                break;
                        }
                case MOD_LAVA:
                        {
                                switch(player_class)
                                {
										case CLASS_DOOM:
										{
											message = "melts in the lava";
											break;
										}
                                        case CLASS_Q1:
                                        {
                                                if(self->health < -25)
                                                        message = "burst into flames";
                                                else if(random() >0.5)
                                                        message = "turned into hot slag";
                                                else
                                                        message = "visits the Volcano God";
                                                break;
                                        }
                                        case CLASS_Q2:
                                        default:
                                                message = "does a back flip into the lava";
                                                break;
                                }
                                break;
                        }
// end Skid             
                case MOD_EXPLOSIVE:
                case MOD_BARREL:
                        message = "blew up";
                        break;
                case MOD_EXIT:
// Skid added                   
                        {
                                switch(player_class)
                                {
                                        case CLASS_Q1:
                                                message = "tried to leave";
                                                break;
                                        case CLASS_Q2:
                                        default:
                                                message = "found a way out";
                                                break;
                                }
                                break;
                        }
// end Skid             
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
                                message = "tried to put the pin back in";
                                break;
                        case MOD_HG_SPLASH:
                        case MOD_G_SPLASH:
                                if (IsNeutral(self))
                                        message = "tripped on its own grenade";
                                else if (IsFemale(self))
                                        message = "tripped on her own grenade";
                                else
                                        message = "tripped on his own grenade";
                                break;
                        case MOD_R_SPLASH:
                                if (IsNeutral(self))
                                        message = "blew itself up";
                                else    if (IsFemale(self))
                                        message = "blew herself up";
                                else
                                        message = "blew himself up";
                                break;
                        case MOD_BFG_BLAST:
                                message = "should have used a smaller gun";
                                break;

// Start Skid - new MODs
                        case MOD_Q1_LG_SPLASH:
                                message = "discharges into the water";
                                break;

                        case MOD_Q1_GL_SPLASH:
                                message = "tries to put the pin back in";
                                break;

                        case MOD_Q1_RL_SPLASH:
                                message = "becomes bored with life";
                                break;
                        
                        case MOD_DOOM_RL_SPLASH:
                                message = "hasn't played Doom for a while";
                                break;

                        case MOD_WOLF_RL_SPLASH:
								if (IsFemale(self))
                                        message = "gibbed herself";
                                else
                                        message = "gibbed himself";
								break;
						case MOD_DOOM_BARREL:
								message = "realizes that toxic barrels can hurt";
								break;
// End Skid - new MODs
                        
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
                        gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
                        if (deathmatch->value)
                        {
                                self->client->resp.score--;
//SKID ADDED
                                if(gen_team->value)
                                {
                                        if((int)genflags->value & GEN_REDBLUE_TEAMS)
                                                RemoveFromTeamScore(self->client->resp.team,1);
                                        else
                                                RemoveFromTeamScore(self->client->resp.player_class,1);
                                }
//END SKID
                        }

                        self->enemy = NULL;
                        return;
                }

        if (deathmatch->value || coop->value )
        {               
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
// Start Skid - new MODs
// Quake
                        case MOD_Q1_AXE:
                                message = "was axe-murdered by";
                                break;
                        case MOD_Q1_SG:
                                message = "chewed on";
                                message2 = "'s boomstick";
                                break;
                        case MOD_Q1_SSG:
                                message = "ate 2 loads of";
                                message2 = "'s buckshot";
                                break;
                        case MOD_Q1_NG:
                                message = "was nailed by";
                                break;
                        case MOD_Q1_SNG:
                                message = "was punctured by";
                                break;
                        case MOD_Q1_GL:
                                message = "eats";
                                message2 = "'s pineapple";
                                break;
                        case MOD_Q1_RL:
                                message = "rides";
                                message2 = "'s rocket";
                                break;
                        case MOD_Q1_LG:
                                message = "accepts";
                                message2 = "'s shaft";
                                break;
                        case MOD_Q1_RL_SPLASH:
                                message = "was splattered by";
                                message2 = "'s rocket";
                                break;
                        case MOD_Q1_GL_SPLASH:
                                message = "tripped on";
                                message2 = "'s grenade";
                                break;
                        case MOD_Q1_LG_SPLASH:
                                message = "accepts";
                                message2 = "'s discharge";
                                break;
// Doom                 
                        case MOD_DOOM_FISTS:
                                message = "was knocked out by";
                                message2= "'s Mighty Punch";
                                break;
						case MOD_DOOM_PISTOL:
								message = "was laid to rest by";
								message2 = "'s little pistol";
								break;
                        case MOD_DOOM_SAW:
                                message = "was sawed in half by";
                                break;
                        case MOD_DOOM_SG:
                                message = "ate lead from";
                                message2 = "'s doomstick";
                                break;
                        case MOD_DOOM_SSG:
                                message = "ate  truckloads of";
                                message2 = "'s buckshot";
                                break;
                        case MOD_DOOM_CG:
                                message = "was punctured by";
                                message2= "'s chaingun";
                                break;
                        case MOD_DOOM_PG:
                                message = "was plasmatized by";
                                break;
                        case MOD_DOOM_RL:
                                message = "was Doomed by";
                                message2= "'s rocket";
                                break;
                        case MOD_DOOM_RL_SPLASH:
                                message = "tried avoiding";
                                message2 = "'s rocket";
                                break;
                        case MOD_DOOM_BFG_LASER:
                                message = "saw the pretty lights from";
                                message2 = "'s BFG";
                                break;
                        case MOD_DOOM_BFG_BLAST:
                                message = "was disintegrated by";
                                message2 = "'s BFG blast";
                                break;
                        case MOD_DOOM_BFG_EFFECT:
                                message = "couldn't hide from";
                                message2 = "'s BFG";
                                break;
// Wolf
                        case MOD_WOLF_KNIFE:
                                message = "was ripped apart by";
                                message2 ="'s Knife";
                                break;
                        case MOD_WOLF_PISTOL:
                                message = "was brought down by";
                                message2="'s Pea Shooter";
                                break;
                        case MOD_WOLF_MACHINEGUN:
                                message = "was tickled to death by";
                                message2= "'s Machinegun";
                                break;
                        case MOD_WOLF_GG:
                                message= "was cut in half by";
                                message2="'s Gatling Gun";
                                break;
						case MOD_WOLF_GG2:
                                message= "was cut to pieces by";
                                message2="'s Dual Gatling Guns";
                                break;
                        case MOD_WOLF_RL:
                                message= "was blown apart by";
                                message2="'s rusty old rocket";
                                break;
                        case MOD_WOLF_RL_SPLASH:
                                message= "tried dodging";
                                message2="'s rocket";
                                break;
                        case MOD_WOLF_FLAME:
                                message= "was burnt to a crisp by";
                                message2="'s gib toaster";
                                break;
                        case MOD_WOLF_FLAMEBURN:
                                message= "died a slow death from";
                                message2="'s painful flames";
                                break;
                        case MOD_GRAPPLE:
                                message = "was grappled to death by";
                                break;
						case MOD_DOOM_BARREL:
								message = "walks into";
								message2="'s Barrel'O'Fun";
								break;
// End Skid - new MODs
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
                        }
                        if (message)
                        {
                                gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
                                if (deathmatch->value)
                                {
                                        if (ff)
                                        {
                                                attacker->client->resp.score--;
//SKID ADDED
                                                if(gen_team->value)
                                                {
                                                        if((int)genflags->value & GEN_REDBLUE_TEAMS)
                                                                RemoveFromTeamScore(self->client->resp.team,1);
                                                        else
                                                                RemoveFromTeamScore(self->client->resp.player_class,1);
                                                }
                                        }
//END SKID
                                        else
                                        {
                                                attacker->client->resp.score++;
//SKID ADDED
                                                if(gen_team->value)
                                                {
                                                        if((int)genflags->value & GEN_REDBLUE_TEAMS)
                                                                AddToTeamScore(self->client->resp.team,1);
                                                        else
                                                                AddToTeamScore(self->client->resp.player_class,1);
                                                }
                                        }
//END SKID

                                }
                                return;
                        }
                }
        }
// Skid added 
// MONSTER MESSAGES !
        else if((gen_sp->value || coop->value) && attacker && (attacker->svflags & SVF_MONSTER))
        {
                if(Q_stricmp(attacker->classname,"q1_monster_fiend")==0)
                        message ="was eviscerated by a Fiend";
                else if(Q_stricmp(attacker->classname,"q1_monster_soldier")==0)
                        message = "was shot by a Grunt";
                else if(Q_stricmp(attacker->classname,"q1_monster_ogre")==0)
                        message = "was destroyed by an Ogre";
                else if(Q_stricmp(attacker->classname,"q1_monster_knight")==0)
                        message = "was slashed by a Knight";
                else if(Q_stricmp(attacker->classname,"q1_monster_scrag")==0)
                        message = "was scragged by a Scrag";
                else if(Q_stricmp(attacker->classname,"q1_monster_zombie")==0)
                        message = "joins the Zombies";
                else if(Q_stricmp(attacker->classname,"q1_monster_shambler")==0)
                        message = "was smashed by a Shambler";
                else if(Q_stricmp(attacker->classname,"q1_monster_dog")==0)
                        message = "was mauled by a Rottweiler";
                else if(Q_stricmp(attacker->classname,"q1_monster_chton")==0)
                        message = "couldn't face Chton";
				else if(Q_stricmp(attacker->classname,"d_monster_cyberdemon")==0)
				{
						message = "bows down before the Cyberdemon";
						cyberdemon_salute (attacker);
				}
				else if(Q_stricmp(attacker->classname,"d_monster_imp")==0)
                        message = "couldn't handle an imp";
				else if(Q_stricmp(attacker->classname,"d_monster_baron")==0)
                        message = "kneels before the Baron of Hell";
                if(message)
                {
                        gi.bprintf(PRINT_MEDIUM,"%s %s.\n", self->client->pers.netname,message);
                        return;
                }
        }
// End MONSTER MESSAGES !

        gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
        if (deathmatch->value)
        {
                self->client->resp.score--;
//SKID ADDED
                if(gen_team->value)
                {
                        if((int)genflags->value & GEN_REDBLUE_TEAMS)
                                RemoveFromTeamScore(self->client->resp.team,1);
                        else
                                RemoveFromTeamScore(self->client->resp.player_class,1);
                }
        }
//END SKID
}



void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
        gitem_t         *item;
        edict_t         *drop;
        qboolean        quad;
        float           spread;

        if (!deathmatch->value)
                return;

        item = self->client->pers.weapon;
        if (! self->client->pers.inventory[self->client->ammo_index] )
                item = NULL;
        if (item && ((strcmp (item->pickup_name, "Blaster") == 0) ||
                         (strcmp (item->pickup_name, "Grapple") == 0)))
                item = NULL;

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
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
        vec3_t          dir;

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

void ThrowDoomguy (edict_t *self);

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
     int             n;

// new vars     
        qboolean        axe;
        int                     player_class;

        player_class=self->client->resp.player_class;
        
//Get rid of the chasecam crap  shouldnt be needed - Skid
        if(self->client->chasetoggle && !(self->client->resp.spectator))
                self->svflags &= ~SVF_NOCLIENT;

//Death View
        if(player_class != CLASS_DOOM)
                VectorClear (self->avelocity);

        self->takedamage = DAMAGE_YES;
        self->movetype = MOVETYPE_TOSS;

        self->s.modelindex2 = 0;        // remove linked weapon model

        self->s.angles[0] = 0;
        self->s.angles[2] = 0;

        self->s.sound = 0;
        self->client->weapon_sound = 0;

//Reset effects - Skid added 
        self->s.effects =0; 
        self->s.renderfx=0; 
        
        self->maxs[2] = -8;

//      self->solid = SOLID_NOT;
        self->svflags |= SVF_DEADMONSTER;

        if (!self->deadflag)
        {
                self->client->respawn_time = level.time + 1.0;
// Death View
                if(player_class == CLASS_DOOM || player_class == CLASS_WOLF)
                        WhoKilledDoomguy(self,attacker); 
                else
                        LookAtKiller (self, inflictor, attacker);

// Check for axe
			if((ITEM_INDEX(self->client->pers.weapon) == Q1_AXE) &&
				self->client->resp.player_next_class == CLASS_Q1 &&
                player_class == CLASS_Q1)
                axe=true;
                
				self->client->ps.pmove.pm_type = PM_DEAD;
                ClientObituary (self, inflictor, attacker);

// CTF stuff -Skid
                if(gen_ctf->value)
                {
                        CTFFragBonuses(self, inflictor, attacker);
                        CTFPlayerResetGrapple(self);
                        CTFDeadDropFlag(self);
                        CTFDeadDropTech(self);
                }
                
                if(deathmatch->value)
                {
// Dont toss weapon in Pickup games      -Skid
                        if (!((int)genflags->value & GEN_FULLYLOADED) 
                                && self->client->resp.state == PLAYING_GEN)
                        {
// to Drop or not to drop                               
                                switch(player_class) 
                                {
                                        case CLASS_Q1:
                                                Q1_DropBackPack(self);
                                                break;
                                        case CLASS_Q2:
                                                TossClientWeapon (self);
                                                break;
                                        case CLASS_DOOM:
                                        case CLASS_WOLF:
                                        default:
                                                break;
                                }
                        }
                        // show scores
                        if(!self->client->showscores)
						{
							Cmd_Help_f (self);   
						}
                }               
                
// clear inventory
// this is kind of ugly, but it's how we want to handle keys in coop
                for (n = 0; n < game.num_items; n++)
                {
                        if (coop->value && itemlist[n].flags & IT_KEY)
						{
							self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
							self->client->pers.inventory[n] = 0;
						}
                }
        }

        // remove powerups
        self->client->quad_framenum = 0;
        self->client->invincible_framenum = 0;
        self->client->breather_framenum = 0;
        self->client->enviro_framenum = 0;
        self->flags &= ~FL_POWER_ARMOR;
		self->flags &= ~FL_D_BERSERK;
		self->flags &= ~FL_D_GIBBED;

//Invis & Touch - Skid
        self->flags &= ~FL_INVIS;
        self->client->invis_framenum = 0;
        self->client->oldweapon = 0.0;
		self->client->super_damage_finished = 0.0;
        self->client->dlastsound = 0.0;
		self->client->dlastfallsound = 0.0;
		self->client->lastfiretime = 0.0;
		self->client->dlasttouchsound=0.0;
		self->touch_debounce_time =0;


		// For corpse gibbin - Skid  
        switch(player_class)
        {
                case CLASS_WOLF:
                        self->style = 1;
                        break;
                case CLASS_DOOM:
                        self->style = 2;
                        break;
                case CLASS_Q1:
                        self->style = 3;
                        break;
                case CLASS_Q2:
                default:
                        self->style = 0;
                        break;
        }
// Skid

	if (self->health < -40)
    {
// Old gibs- Skid
		switch(player_class)
		{
		case CLASS_WOLF:
			{
				gi.sound (self, CHAN_BODY, gi.soundindex ("doomguy/dgib.wav"), 1, ATTN_NORM, 0);
				ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
				ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
				ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
				ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
				ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
				ThrowClientHead (self, damage);
				break;
			}
		case CLASS_DOOM:
			{
//GIB MODEL IF STANDING
				gi.sound (self, CHAN_BODY, gi.soundindex ("doomguy/dgib.wav"), 1, ATTN_NORM, 0);

				if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					for (n= 0; n < 8; n++)
						ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage/2, GIB_ORGANIC);
					ThrowClientHead (self, damage);
				}
				else
				{
					gi.sound (self, CHAN_VOICE, gi.soundindex("doomguy/death1.wav"), 1, ATTN_NORM, 0);
					ThrowDoomguy (self);
				}
                break;
			}
		case CLASS_Q1:
			{
				if(damage >= 100000)
					gi.sound (self, CHAN_BODY, gi.soundindex ("q1guy/teledth1.wav"), 1, ATTN_NORM, 0);
				else
                {       
					if(random() > 0.5)
                       gi.sound (self, CHAN_BODY, gi.soundindex ("q1guy/gib.wav"), 1, ATTN_NORM, 0);
                    else
                       gi.sound (self, CHAN_BODY, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
                }
				ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", damage, GIB_ORGANIC);
                for (n= 0; n < 4; n++)
                {
                  if ((n % 2) == 0) 
                     ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
                  else
                     ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
                }
				ThrowClientHead (self, damage);
                break;
			}
		case CLASS_Q2:
			{
				gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                        for (n= 0; n < 4; n++)
                           ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
				ThrowClientHead (self, damage);
                break;
			}
		}
// End modifications
		       self->client->anim_priority = ANIM_DEATH;
               self->client->anim_end = 0;
               self->takedamage = DAMAGE_NO;
        }
        else
        {       // normal death
                if (!self->deadflag)
                {

// Quake1 player death with AXE                 
                        if (axe==true)
                        {
                                if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
                                {
                                        self->s.frame = FRAME_axe_crdeath1-1;
                                        self->client->anim_end = FRAME_axe_crdeath8;
                                }
                                else
                                {
                                        self->s.frame = FRAME_axe_death101-1;
                                        self->client->anim_end = FRAME_axe_death109;
                                }
                        }
                        else
                        {
// End Skid     
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
						default:
                                self->s.frame = FRAME_death301-1;
                                self->client->anim_end = FRAME_death308;
                                break;
                        }
                        }
                        
						switch(player_class)
						{
						case CLASS_WOLF:
							{
								gi.sound (self, CHAN_VOICE, gi.soundindex("wolfguy/death.wav"), 1, ATTN_NORM, 0);
								break;
							}
						case CLASS_DOOM:
							{
								if(self->health < -15)
									gi.sound (self, CHAN_VOICE, gi.soundindex("doomguy/death1.wav"), 1, ATTN_NORM, 0);
								else
									gi.sound (self, CHAN_VOICE, gi.soundindex("doomguy/death2.wav"), 1, ATTN_NORM, 0);
								break;
							}
						case CLASS_Q1:
						case CLASS_Q2:
						default:
							{
								gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
								break;
							}
						}
                }
        }
        
// DEAD_GIBBED for Doom/Wolf Death View 
        if((player_class <=CLASS_DOOM) && !(self->client->chasetoggle))
        {
                if(self->health < -40)
                        self->deadflag = DEAD_GIBBED;
                else
                        self->deadflag = DEAD_DYING;
        }
        else
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
        gitem_t         *item;

//Skid added
//spawning weapon
        char     first_weap[16]; 
		int		level;

		if(coop->value)
			level = client->pers.levels;

        memset (&client->pers, 0, sizeof(client->pers));
        
        client->pers.max_bullets        = 200;
        client->pers.max_shells         = 100;
        client->pers.health             = 100;
        client->pers.max_health         = 100;

//Skid added
//======================================================================
//have to copy the weapon name and then add it to the players inventory
//would crash otherwise 
        
        switch(client->resp.player_class)
        {
                case CLASS_WOLF:
                        strcpy(first_weap, "Knife");
                        break;
                case CLASS_DOOM:
                        strcpy(first_weap, "Fists");         // FIXME -CHANGE TO FISTS
                        break;
                case CLASS_Q1:
                        strcpy(first_weap, "Axe");
                        break;
                case CLASS_Q2:
                default:
                        strcpy(first_weap, "Blaster");
                        break;
        }
        item = FindItem(first_weap);    
        client->pers.inventory[ITEM_INDEX(item)] = 1;
        client->pers.inventory[client->pers.selected_item] = 1;

// different stuff for different classes
        switch(client->resp.player_class)
        {
                //Returns the weapon to be assigned as current
                case CLASS_WOLF:
                        item = InitWolfPersistant(client);
                        break;
                case CLASS_DOOM:
                        item = InitDoomPersistant(client);
                        break;
                case CLASS_Q1:
                        item = InitQ1Persistant(client);
                        break;
                case CLASS_Q2:
                        item = InitQ2Persistant(client);
                        break;
        }
//======================================================================
//End Skid
                
        client->pers.weapon = item;
        client->pers.connected = true;


		if(coop->value)
			client->pers.levels = 	level;
}


void InitClientResp (gclient_t *client)
{
//Skid - Find current team/Class and save them
        int ctf_team=0;
        int next_class=0;
        int current_class=0;

        current_class=client->resp.player_class;
        next_class=client->resp.player_next_class;
                
        if (gen_ctf->value)
                ctf_team=client->resp.team;
//End Skid

       memset (&client->resp, 0, sizeof(client->resp));
        
//Skid - Assign them to thier old teams and class 
        if (gen_ctf->value)
                client->resp.team=ctf_team;
        client->resp.player_class = current_class;
        client->resp.player_next_class = next_class;
//End Skid

		client->resp.enterframe = level.framenum;
        client->resp.coop_respawn = client->pers;
		client->pers.levels ++;
        

//Skid added - Support ForceJoins - assign NO_CLASS, NO_TEAM to client vars
        if((client->resp.team < CTF_TEAM1) && 
          (gen_ctf->value || (gen_team->value && (int)genflags->value & GEN_REDBLUE_TEAMS)))
                CTFAssignTeam(client);

        if(client->resp.player_class < CLASS_WOLF) 
                GenAssignClass(client);
//End Skid
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
        int             i;
        edict_t *ent;

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
float   PlayersRangeFromSpot (edict_t *spot)
{
        edict_t *player;
        float   bestplayerdistance;
        vec3_t  v;
        int             n;
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
        int             count = 0;
        int             selection;
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
        int             index;
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
                {       // this is a coop spawn point for one of the clients here
                        index--;
                        if (!index)
                                return spot;            // this is it
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
void    SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
        edict_t *spot = NULL;

// Setup state, first base respawn - Skid
        if(gen_ctf->value)
                spot = SelectGenSpawnPoint (ent);
        else if (deathmatch->value)
                spot = SelectDeathmatchSpawnPoint();
        else if (coop->value)
                spot = SelectCoopSpawnPoint (ent);

// to support co-op, SPQ -Skid  
        ent->client->resp.state = PLAYING_GEN;
// end - Skid

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
                        {       // there wasn't a spawnpoint without a target, so use any
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
        int             i;
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
        int     n;

        if (self->health < -40)  
        {
// Corpse Gibs -Skid // added, more logical gibbin
/*                
					if( damage < 20 && 
                   (attacker && attacker != world) &&      
                   (inflictor && inflictor != world ))     
                        return;
*/
// fix me - add Wolf, Doom gibs when they're done
               if(self->style)
                {
				   if(self->style == 1)
				   {
						gi.sound (self, CHAN_BODY, gi.soundindex ("doomguy/dgib.wav"), 1, ATTN_NORM, 0);
						ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
						ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
						ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
						ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
						ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
					}
                    else if(self->style==2)
                    {
						for (n= 0; n < 8; n++)
							ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage/2, GIB_ORGANIC);
						gi.sound (self, CHAN_BODY, gi.soundindex ("doomguy/dgib.wav"), 1, ATTN_NORM, 0);
		            }
                    else if(self->style == 3)
                    {
                        if(random() > 0.5)
				             gi.sound (self, CHAN_BODY, gi.soundindex ("q1guy/gib.wav"), 1, ATTN_NORM, 0);
                        else
                            gi.sound (self, CHAN_BODY, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
						ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", damage, GIB_ORGANIC);
						for (n= 0; n < 4; n++)
						{
							if ((n % 2) == 0) 
                              ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
							else
                              ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
                        }
					}
	           }
                else
                {
// Corpse Gibs -Skid
                        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                        for (n= 0; n < 4; n++)
                                ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                }
                self->s.origin[2] -= 48;
                ThrowClientHead (self, damage);
                self->takedamage = DAMAGE_NO;
        }
}

void CopyToBodyQue (edict_t *ent)
{
        edict_t         *body;
// For diff gibs -Skid
        int         corpse_style=0; 
        
// Save Gib style
        if(ent->style)
                corpse_style = ent->style;

        // grab a body que and cycle to the next one
        body = &g_edicts[(int)maxclients->value + level.body_que + 1];
        level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

        // FIXME: send an effect on the removed body

        gi.unlinkentity (ent);

        gi.unlinkentity (body);
        body->s = ent->s;
        body->s.number = body - g_edicts;
//
		body->s.modelindex = ent->s.modelindex;
		body->s.skinnum = ent->s.skinnum;
		
        body->svflags = ent->svflags;
        VectorCopy (ent->mins, body->mins);
        VectorCopy (ent->maxs, body->maxs);
        VectorCopy (ent->absmin, body->absmin);
        VectorCopy (ent->absmax, body->absmax);
        VectorCopy (ent->size, body->size);

 // set body to a Gib Style -Skid
        body->style = corpse_style;

        body->solid = ent->solid;
        body->clipmask = ent->clipmask;
		body->owner = ent->owner;
        body->movetype = ent->movetype;

        body->die = body_die;
        body->takedamage = DAMAGE_YES;

        gi.linkentity (body);
}

//Skid
void ChasecamStart (edict_t *ent);

void respawn (edict_t *self)
{
//CHASECAM
		qboolean chase=false;
		
		if(self->client->chasetoggle)
		{
			chase = true;
			if (self->client->oldplayer != NULL)
			{
				free(self->client->oldplayer->client);
				G_FreeEdict(self->client->oldplayer);
			}
			if (self->client->chasecam != NULL)
				G_FreeEdict(self->client->chasecam);
		}
//CHASECAM

        if (deathmatch->value || coop->value)
        {
                // spectator's don't leave bodies
                if (self->movetype != MOVETYPE_NOCLIP)
                        CopyToBodyQue (self);

//RANDOM change class
                if((int)genflags->value & GEN_RANDOM_CLASS)
                {
					float r= random();
                        if(r > 0.25)
							self->client->resp.player_next_class = CLASS_WOLF;
						else if(r > 0.5)	
							self->client->resp.player_next_class = CLASS_DOOM;
						else if(r > 0.75)
							self->client->resp.player_next_class = CLASS_Q1;
						else
                           self->client->resp.player_next_class = CLASS_Q2;
	             }
//end Skid

                
				self->svflags &= ~SVF_NOCLIENT;
                
				PutClientInServer (self);

                // add a teleportation effect
                GenClientRespawnFX(self);
                //self->s.event = EV_PLAYER_TELEPORT;

                // hold in place briefly
                self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
                self->client->ps.pmove.pm_time = 14;

                self->client->respawn_time = level.time;

				if(chase == true)
					ChasecamStart(self);
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

        if (ent->client->pers.spectator) 
		{
                char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
                if (*spectator_password->string && 
                        strcmp(spectator_password->string, "none") && 
                        strcmp(spectator_password->string, value)) 
				{
                        gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
                        ent->client->pers.spectator = false;
                        gi.WriteByte (svc_stufftext);
                        gi.WriteString ("spectator 0\n");
                        gi.unicast(ent, true);
                        return;
                }

                // count spectators
                for (i = 1, numspec = 0; i <= maxclients->value; i++)
                        if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
                                numspec++;

                if (numspec >= maxspectators->value) 
				{
                        gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
                        ent->client->pers.spectator = false;
                        // reset his spectator var
                        gi.WriteByte (svc_stufftext);
                        gi.WriteString ("spectator 0\n");
                        gi.unicast(ent, true);
                        return;
                }
        } 
		else 
		{
                // he was a spectator and wants to join the game
                // he must have the right password
                char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
                if (*password->string && strcmp(password->string, "none") && 
                        strcmp(password->string, value)) 
				{
                        gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
                        ent->client->pers.spectator = true;
                        gi.WriteByte (svc_stufftext);
                        gi.WriteString ("spectator 1\n");
                        gi.unicast(ent, true);
                        return;
                }
        }

        // clear client on respawn
        ent->client->resp.score = ent->client->pers.score = 0;

        if(!(ent->client->chasetoggle) && (ent->client->resp.player_class > NO_CLASS))
			ent->svflags &= ~SVF_NOCLIENT;
        
		PutClientInServer (ent);

        // add a teleportation effect
        if (!ent->client->pers.spectator)  
		{
                // send effect
                gi.WriteByte (svc_muzzleflash);
                gi.WriteShort (ent-g_edicts);
                gi.WriteByte (MZ_LOGIN);
                gi.multicast (ent->s.origin, MULTICAST_PVS);

                // hold in place briefly
                ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
                ent->client->ps.pmove.pm_time = 14;
        }

        ent->client->respawn_time = level.time;

        if (ent->client->pers.spectator) 
                gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
        else
                gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
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
        vec3_t  mins = {-16, -16, -24};
        vec3_t  maxs = {16, 16, 32};
        int             index;
        vec3_t  spawn_origin, spawn_angles;
        gclient_t       *client;
        int             i;
        client_persistant_t     saved;
        client_respawn_t        resp;

#ifdef GEN_SOCK
		//Always saved
		client_sock_t			socksave;
#endif

//Skid added
//Ugly - check if player has changed class
        qboolean classchange=false;
		qboolean randomclass=false;

		if(ent->client->random_class==true)
		{
			ent->client->resp.player_next_class = GetRandomClass();
			randomclass = true;
		}
        
// set new class
        if(ent->client->resp.player_class != ent->client->resp.player_next_class)
        {
                ent->flags &= ~FL_VALID_MODEL;
                classchange=true;

				if(deathmatch->value && !gen_ctf->value)
				{
					if((ent->client->resp.player_next_class == CLASS_DOOM) ||
					   (ent->client->resp.player_next_class == CLASS_WOLF))
					{
						if(deathmatch->value)
							SendStatusBar(ent,wolfdm);
						else
							SendStatusBar(ent,wolfsp);
					}
					else if((ent->client->resp.player_next_class == CLASS_Q1) ||
							(ent->client->resp.player_next_class == CLASS_Q2))
						SendStatusBar(ent,dm_statusbar);
				}
				
				switch(ent->client->resp.player_next_class)
				{

				case CLASS_WOLF:
					{
						stuffcmd(ent,"exec gwolf.cfg\n");
						break;
					}
				case CLASS_DOOM:
					{
						stuffcmd(ent,"exec gdoom.cfg\n");
						break;
					}
				case CLASS_Q1:
					{
						stuffcmd(ent,"exec gq1.cfg\n");
						break;
					}
				case CLASS_Q2:
					{
						stuffcmd(ent,"exec gq2.cfg\n");
						break;
					}
				}

//manage team scores            
                if(gen_team->value)
                {
                        if(!((int)genflags->value & GEN_REDBLUE_TEAMS))
                        {       
                                AddToTeamScore(ent->client->resp.player_next_class, ent->client->resp.score);
                                RemoveFromTeamScore(ent->client->resp.player_class, ent->client->resp.score);
                        }
                }
	     }
        ent->client->resp.player_class = ent->client->resp.player_next_class;
// end -Skid

        // find a spawn point
        // do it before setting health back up, so farthest
        // ranging doesn't count this client
        SelectSpawnPoint (ent, spawn_origin, spawn_angles);

        index = ent-g_edicts-1;
        client = ent->client;

        // deathmatch wipes most client data every spawn
        if (deathmatch->value)
        {
                char            userinfo[MAX_INFO_STRING];

                resp = client->resp;
                memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
                InitClientPersistant (client);
                ClientUserinfoChanged (ent, userinfo);
        }
        else if (gen_sp->value)
        {
                char            userinfo[MAX_INFO_STRING];

                resp = client->resp;
                memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

                if(coop->value)
                {
					int level = client->pers.levels;
					int n;

					resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
                    resp.coop_respawn.helpchanged = client->pers.helpchanged;

                      if(classchange==true || (level <= 1))
					  {
							for (n = 0; n < game.num_items; n++)
							{
								if (coop->value && itemlist[n].flags & IT_KEY)
								{
									resp.coop_respawn.inventory[n] = client->pers.inventory[n];
									client->pers.inventory[n] = 0;
								}
							}
							InitClientPersistant (client);
							resp.coop_respawn = client->pers;
	                  }
					  else
					  {
						  client->pers = resp.coop_respawn;
						  client->pers.levels = level;
						  ClientUserinfoChanged (ent, userinfo);
					  }
                        if (resp.score > client->pers.score)
                                client->pers.score = resp.score;
                }
                //inits both co-op and sp player classes
                else if(classchange)
                {
                        InitClientPersistant (client);
                }
                
                ClientUserinfoChanged (ent, userinfo);
        }
        else
        {
                memset (&resp, 0, sizeof(resp));
        }

        // clear everything but the persistant data
        saved = client->pers;
//Skid	
#ifdef GEN_SOCK
		socksave= client->sock;
#endif

        memset (client, 0, sizeof(*client));
        
		client->pers = saved;
//Skid
#ifdef GEN_SOCK
		client->sock = socksave;
#endif
        
		if (client->pers.health <= 0)
                InitClientPersistant(client);
        client->resp = resp;

        // copy some data from the client to the entity
        FetchClientEntData (ent);

//hacking
		if(randomclass)
			client->random_class=true;

        // clear entity values
        ent->groundentity = NULL;
        ent->client = &game.clients[index];
        ent->takedamage = DAMAGE_AIM;
        ent->movetype = MOVETYPE_WALK;

        
        //ent->viewheight = 22;
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

        //Skid
        ent->touch_debounce_time = 0.0;
		ent->flags &= ~FL_D_GIBBED;
        if(gen_ctf->value)
                ent->flags &= ~FL_RUNE_MASK;


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

        // clear entity state values
        ent->s.effects = 0;
        ent->s.modelindex = 255;                // will use the skin specified model
        ent->s.modelindex2 = 255;               // custom gun model
        
        // sknum is player num and weapon number
        // weapon number will be added in changeweapon
        ent->s.skinnum = ent - g_edicts - 1;

        ent->s.frame = 0;
        VectorCopy (spawn_origin, ent->s.origin);
        ent->s.origin[2] += 1;  // make sure off ground
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

        // spawn a spectator
        if ((client->pers.spectator)  
//Skid added
/*			(client->resp.player_class < NO_CLASS) ||
			((gen_ctf->value || 
			 (gen_team->value && ((int)gen_team->value & GEN_REDBLUE_TEAMS))) && 
			 client->resp.team < CTF_NOTEAM)
*/
		)
//End Skid
        {
                client->chase_target = NULL;
                client->resp.spectator = true;
                ent->movetype = MOVETYPE_NOCLIP;
                ent->solid = SOLID_NOT;
                ent->svflags |= SVF_NOCLIENT;
                ent->client->ps.gunindex = 0;
                gi.linkentity (ent);
                return;
        } 
        else
                client->resp.spectator = false;

// Check if current class/team info is valid - Skid
        if (GenStartClient(ent))
                return;
// end Skid

        if (!KillBox (ent))
        {       // could't spawn in?
        }

        gi.linkentity (ent);

        // force the current weapon up
        client->newweapon = client->pers.weapon;
        ChangeWeapon (ent);
}


//======================================================================

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

#ifdef GEN_SOCK
// Skid added
// Select Effect based on map 
	if(level.miditime)
	{
		//try to connect to client if not already connected
		//and send over the command to play	
		//or play locally on listen server
		if((!dedicated->value) || 
			((ent->client->sock.sconnected) || GenConnect(ent->client)))
			GenMidiCmd(ent,MCMD_PLAY);
	}
	else if((!dedicated->value) || (ent->client->sock.sconnected))
			GenMidiCmd(ent,MCMD_STOP);
#endif

         if((level.game == CLASS_DOOM) || 
           (ent->client->resp.player_class == CLASS_DOOM))
        {
              DoomTeleporterSplash(ent->s.origin,ent->s.angles);
        }
        else if((level.game == CLASS_Q1) ||
              (ent->client->resp.player_class == CLASS_Q1))
        {
             ent->s.event = EV_PLAYER_TELEPORT;
             gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("q1monstr/scrag/wsight.wav"), 1, ATTN_NORM, 0);
        }
        else 
        {
//End Skid
             // send effect
             gi.WriteByte (svc_muzzleflash);
             gi.WriteShort (ent-g_edicts);
             gi.WriteByte (MZ_LOGIN);
             gi.multicast (ent->s.origin, MULTICAST_PVS);
        }
    }
    gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);



// make sure all view stuff is valid
    ClientEndServerFrame (ent);
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
	int i;

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
#ifdef GEN_SOCK
// Skid added
// Select Effect based on map 
				//try to connect to client if not already connected
				//and send over the command to play	
				//or play locally on listen server
				if(level.miditime)
				{
					if((!dedicated->value) || 
						((ent->client->sock.sconnected) || GenConnect(ent->client)))
						GenMidiCmd(ent,MCMD_PLAY);
				}
				else if((!dedicated->value) || (ent->client->sock.sconnected))
					GenMidiCmd(ent,MCMD_STOP);
#endif

                // send effect if in a multiplayer game
                if (game.maxclients > 1)
                {
					if(level.game == CLASS_DOOM || (ent->client->resp.player_class == CLASS_DOOM))
                    {
                            DoomTeleporterSplash(ent->s.origin,ent->s.angles);
                    }
                    else if(level.game == CLASS_Q1 || (ent->client->resp.player_class == CLASS_Q1))
                    {
							ent->s.event = EV_PLAYER_TELEPORT;
                            gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("q1monstr/scrag/wsight.wav"), 1, ATTN_NORM, 0);
                    }
                    else 
                    {
							gi.WriteByte (svc_muzzleflash);
							gi.WriteShort (ent-g_edicts);
							gi.WriteByte (MZ_LOGIN);
							gi.multicast (ent->s.origin, MULTICAST_PVS);
                    }
                    gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
                }
        }

        // make sure all view stuff is valid
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
void ShowGun(edict_t *ent);
qboolean GenValidateModel(int pclass, const char *s);

void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
        char    *s;
//Skid added    
        char    *newskin;
#ifdef GEN_SOCK
		char	ip[16];
#endif
        
		int player_class= ent->client->resp.player_class;

        // check for malformed or illegal info strings
        if (!Info_Validate(userinfo))
        {
            switch(player_class)
			{
			case CLASS_WOLF:
				strcpy (userinfo, "\\name\\badinfo\\skin\\wolfguy/base");
				break;
			case CLASS_DOOM:
				strcpy (userinfo, "\\name\\badinfo\\skin\\doomguy/base");
				break;
			case CLASS_Q1:
				strcpy (userinfo, "\\name\\badinfo\\skin\\quakeguy/base");
				break;
			case CLASS_Q2:
			default:
				strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
				break;
			}
        }

        // set name
        s = Info_ValueForKey (userinfo, "name");
        strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

        // set spectator
        s = Info_ValueForKey (userinfo, "spectator");
        // spectators are only supported in deathmatch
        if (deathmatch->value && *s && strcmp(s, "0"))
                ent->client->pers.spectator = true;
        else
                ent->client->pers.spectator = false;

//Skid
#ifdef GEN_SOCK
		if(ParseIP(Info_ValueForKey(userinfo, "ip"),ip))	
			strcpy(ent->client->sock.ipaddr,ip);

		s = Info_ValueForKey (userinfo,"midiport");
		if(*s)
		{
			if(!sscanf(s,"%d",&ent->client->sock.port))
				ent->client->sock.port = 28000;				//default to 28000
		}
		else
			ent->client->sock.port = 28000;				//default to 28000

//		gi.dprintf("%s from %s at %d\n",ent->client->pers.netname,ent->client->pers.ipaddr,ent->client->pers.port);
#endif

		s = Info_ValueForKey (userinfo, "skin");

		switch(ent->client->resp.player_class)
		{
		case CLASS_WOLF:
				newskin= Info_ValueForKey(userinfo, "wskin");
				break;
		case CLASS_DOOM:
				newskin= Info_ValueForKey(userinfo, "dskin");
				break;
		case CLASS_Q1:
				newskin= Info_ValueForKey(userinfo, "q1skin");
				break;
		case CLASS_Q2:
		default:
				newskin = Info_ValueForKey (userinfo, "skin");
				break;
		}

		if((!GenValidateModel(ent->client->resp.player_class,s)) &&
			(*newskin))
		{		
			if(!GenAssignSkin(ent,newskin))
				GenAssignModel(ent, s);
		}
		else
			GenAssignModel(ent, s);
       

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
        //strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
        
		// save off the userinfo in case we want to check something later
        memcpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo));
		
		ShowGun(ent);
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
//Skid	
#ifdef GEN_SOCK
		char	ip[16];
		char	*sport;
#endif

        // check to see if they are on the banned IP list
        value = Info_ValueForKey (userinfo, "ip");
        if (SV_FilterPacket(value)) 
		{
                Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
                return false;
        }


        // check for a spectator
        value = Info_ValueForKey (userinfo, "spectator");
        if (deathmatch->value && *value && strcmp(value, "0")) 
		{
                int i, numspec;

                if (*spectator_password->string && 
                        strcmp(spectator_password->string, "none") && 
                        strcmp(spectator_password->string, value)) 
				{
                        Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
                        return false;
                }

                // count spectators
                for (i = numspec = 0; i < maxclients->value; i++)
                        if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
                                numspec++;

                if (numspec >= maxspectators->value)
				{
                        Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
                        return false;
                }
        } 
		else 
		{
                // check for a password
                value = Info_ValueForKey (userinfo, "password");
                if (*password->string && strcmp(password->string, "none") && 
                        strcmp(password->string, value))
				{
                        Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
                        return false;
                }
        }

        // they can connect
        ent->client = game.clients + (ent - g_edicts - 1);

#ifdef GEN_SOCK
		if(dedicated->value)
		{
			if(ParseIP(Info_ValueForKey(userinfo, "ip"),ip))	
			{
				strcpy(ent->client->sock.ipaddr,ip);
			}
		
			value = Info_ValueForKey(userinfo,"midiport");
			if(*value)
			{
				if(!sscanf(value,"%d",&ent->client->sock.port))
					ent->client->sock.port = 28000;				//default to 28000
			}
			else
				ent->client->sock.port = 28000;				//default to 28000
//			gi.dprintf("CC: %s on midi port %d\n",ent->client->pers.netname,ent->client->sock.port);

			if(GenInitClientSock(ent->client))
			{
				GenConnect(ent->client);
			}
			else
				gi.dprintf("ClientConnect:%s unable to create socket\n",ent->client->pers.netname);
		}
#endif

        // if there is already a body waiting for us (a loadgame), just
        // take it, otherwise spawn one from scratch
        if (ent->inuse == false)
        {
//Added default values -Skid
                ent->client->resp.player_class = -1;
                ent->client->resp.player_next_class = -1;
                ent->client->resp.team = -1;
//end
				
                // clear the respawning variables
                InitClientResp (ent->client);
                if (!game.autosaved || !ent->client->pers.weapon)
                        InitClientPersistant (ent->client);
        }

        ClientUserinfoChanged (ent, userinfo);

        if (game.maxclients > 1)
                gi.dprintf ("%s connected\n", ent->client->pers.netname);

        ent->svflags = 0; // make sure we start with known default
        ent->client->pers.connected = true;
//Skid		
		ent->client->pers.levels = 0;
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
        int             playernum;

        if (!ent->client)
                return;

        gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

#ifdef GEN_SOCK
		//try to connect to client if not already connected
		//and send over the command to play	
		//or play locally on listen server
		if(level.miditime)
		if((dedicated->value) && ent->client->sock.sconnected)
				GenMidiCmd(ent,MCMD_STOP);

		GenCloseClientSocket(ent->client);
#endif

//Skid added
//Drop Flag and stuff   
        if(gen_ctf->value)
        {
                CTFDeadDropFlag(ent);
                CTFDeadDropTech(ent);
        }

//Select Effect based on map
        if((level.game == CLASS_Q1) || 
           (ent->client->resp.player_class == CLASS_Q1))
                gi.sound (ent, CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("q1guy/tornoff2.wav"), 1, ATTN_NORM, 0);
        else
        {

        // send effect
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGOUT);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
        
        }

//RemoveFromClassCount(ent->client->resp.player_class);
        if(gen_team->value && 
          (!(int)genflags->value & GEN_REDBLUE_TEAMS))
                RemoveFromTeamScore(ent->client->resp.player_class, ent->client->resp.score);
//End Skid

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
        int     v,i;
        v = 0;
        for (i=0 ; i<c ; i++)
                v+= ((byte *)b)[i];
        return v;
}
void PrintPmove (pmove_t *pm)
{
        unsigned        c1, c2;

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
        gclient_t       *client;
        edict_t *other;
        int             i, j;
        pmove_t pm;

//Skid added
        int             player_class=0;

        level.current_entity = ent;
        client = ent->client;

//Skid added
        player_class = client->resp.player_class;

		if(gen_sp->value && !player_class)
		{
			client->ps.pmove.pm_type = PM_FREEZE;
			return;
		}
//end Skid

        if (level.intermissiontime)
        {
                client->ps.pmove.pm_type = PM_FREEZE;
                // can exit intermission after five seconds
                if (level.time > level.intermissiontime + 5.0 
                        && (ucmd->buttons & BUTTON_ANY) )
                        level.exitintermission = true;
                return;
        }

        pm_passent = ent;

        if (ent->client->chase_target) 
        {

                client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
                client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
                client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

// ### Hentai ### BEGIN - this is improved chasecam stuff
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
// ### Hentai ### END
        } 
        else 
        {
                // set up for pmove
                memset (&pm, 0, sizeof(pm));

                if (ent->movetype == MOVETYPE_NOCLIP)
                        client->ps.pmove.pm_type = PM_SPECTATOR;
                else if ((ent->s.modelindex != 255)
//Skid - EYES
                &&  !(ent->flags & FL_INVIS)) 
//end
                        client->ps.pmove.pm_type = PM_GIB;
                else if (ent->deadflag)
                        client->ps.pmove.pm_type = PM_DEAD;
                else
                        client->ps.pmove.pm_type = PM_NORMAL;

//SPEED 
				//if(player_class == CLASS_DOOM && (lan->value || gen_sp->value))
				if(player_class == CLASS_DOOM && (lan->value ))//|| gen_sp->value))
				{
					if(ucmd->forwardmove == 400 ||
					   ucmd->sidemove == 400)
					   client->ps.pmove.gravity = sv_gravity->value * 0.9;
					else
						client->ps.pmove.gravity = sv_gravity->value * 1.0;
				}
				else
					client->ps.pmove.gravity = sv_gravity->value;

//Smoother Chasecam ?
                if(client->chasetoggle || client->chase_target)
                {
                        if(ent->groundentity)
                                client->ps.pmove.gravity = 0;
                }
                pm.s = client->ps.pmove;

                for (i=0 ; i<3 ; i++)
                {
                        pm.s.origin[i] = ent->s.origin[i]*8;
                        pm.s.velocity[i] = ent->velocity[i]*8;
                }

                if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
                {
                        pm.snapinitial = true;
        //              gi.dprintf ("pmove changed!\n");
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
                        ent->velocity[i] = pm.s.velocity[i]*0.125;
                }

                VectorCopy (pm.mins, ent->mins);
                VectorCopy (pm.maxs, ent->maxs);

                client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
                client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
                client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

//SPEEDS ===================================================

if(player_class == CLASS_DOOM && (lan->value ))//|| gen_sp->value))
if((ucmd->forwardmove == 400 || ucmd->forwardmove == -400) ||
   (ucmd->sidemove == 400 || ucmd->sidemove == -400))
{

	ucmd->forwardmove *= 0.1;
    ucmd->sidemove *= 0.1;
	
     memset (&pm, 0, sizeof(pm));
        
        if (ent->movetype == MOVETYPE_NOCLIP)
                client->ps.pmove.pm_type = PM_SPECTATOR;
        else if ((ent->s.modelindex != 255 )
//Skid - EYES
                &&  !(ent->flags & FL_INVIS)) 
//end
                client->ps.pmove.pm_type = PM_GIB;
        else if (ent->deadflag)
                client->ps.pmove.pm_type = PM_DEAD;
        else
                client->ps.pmove.pm_type = PM_NORMAL;

        client->ps.pmove.gravity = sv_gravity->value * 0.7;

        
        pm.s = client->ps.pmove;
        for (i=0 ; i<3 ; i++)
        {
                pm.s.origin[i] = ent->s.origin[i]*8;
                pm.s.velocity[i] = ent->velocity[i]*8;
        }

        if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
        {
                pm.snapinitial = true;
//              gi.dprintf ("pmove changed!\n");
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
                ent->velocity[i] = pm.s.velocity[i]*0.125;
        }

        VectorCopy (pm.mins, ent->mins);
        VectorCopy (pm.maxs, ent->maxs);

        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
}

//SPEEDS ===================================================
		
		//Skid - no jump sounds for Doomguy, only Landings
		if(player_class == CLASS_DOOM)
		{
			if((!ent->groundentity) && 
			   (pm.groundentity) && 
               (pm.waterlevel == 0) &&
			   (client->dlastsound > level.time) &&
			   (!ent->deadflag) &&
			   (!pm.cmd.upmove))
			{
		        //Falling sound
                gi.sound (ent, CHAN_VOICE, gi.soundindex("doomguy/noway.wav"), 1, ATTN_NORM, 0);
                PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
                client->dlastsound = 0.0;//level.time + 0.7;
			}
		}
		
		if ((ent->groundentity) && 
		    (!pm.groundentity) && 
			(pm.cmd.upmove >= 10) && 
			(pm.waterlevel == 0))
		{
                switch(player_class)
				{
				case CLASS_DOOM:
					   ent->client->dlastsound = level.time + 1.0;
					   break;
				case CLASS_Q1:
				       gi.sound(ent, CHAN_AUTO, gi.soundindex("q1guy/jump.wav"), 1, ATTN_NORM, 0);
					   break;
				case CLASS_WOLF:
                       gi.sound(ent, CHAN_VOICE, gi.soundindex("wolfguy/jump.wav"), 1, ATTN_NORM, 0);
					   break;
				default:
					   gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
					   break;
				}
                PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		}
                
/*
Death View
==========
by Steffen "The Addict" Itterheim
E-Mail: 110213.1772@compuserve.com
or      The_Addict@compuserve.com
*/      
        if(player_class <= CLASS_DOOM && ent->deadflag && !(ent->client->chasetoggle))
                LookAtKiller2(ent);
        else
        {
                ent->viewheight = pm.viewheight;
                ent->waterlevel = pm.waterlevel;
                ent->watertype = pm.watertype;
                ent->groundentity = pm.groundentity;
                if (pm.groundentity)
                        ent->groundentity_linkcount = pm.groundentity->linkcount;

                if(ent->deadflag)
                {
                        if(player_class == CLASS_Q1)
                        {                               
                                client->ps.viewangles[ROLL] = 40;
                                client->ps.viewangles[PITCH] = pm.viewangles[PITCH];
                                client->ps.viewangles[YAW] = pm.viewangles[YAW];
                        }
                        else
                        {
                                client->ps.viewangles[ROLL] = 40;
                                client->ps.viewangles[PITCH] = -15;
                                client->ps.viewangles[YAW] = client->killer_yaw;
                        }
                }
                else
                {
                        VectorCopy (pm.viewangles, client->v_angle);
                        VectorCopy (pm.viewangles, client->ps.viewangles);
                }
        }       

//ZOID
        if (client->ctf_grapple)
        {
                if (player_class == CLASS_Q1)
                      Q1GrapplePull(client->ctf_grapple);
				else //if(player_class == CLASS_Q2)
                        CTFGrapplePull(client->ctf_grapple);
                
        }
//ZOID
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
                                continue;       // duplicated
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
        if (client->latched_buttons & BUTTON_ATTACK//)
//ZOID
              && ent->movetype != MOVETYPE_NOCLIP)
//ZOID
        {
                if (client->resp.spectator) 
                {
                        client->latched_buttons = 0;

                        if (client->chase_target) 
                        {
                                client->chase_target = NULL;
                                client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                        } 
                        else
                                GetChaseTarget(ent);
                } 
                else if (!client->weapon_thunk) 
                {
                        client->weapon_thunk = true;
                        Think_Weapon (ent);
                }
        }

        if (client->resp.spectator) 
        {
                if (ucmd->upmove >= 10) 
                {
                        if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) 
                        {
                                client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                                if (client->chase_target)
                                        ChaseNext(ent);
                                else
                                        GetChaseTarget(ent);
                        }
                } 
                else
                        client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
        }

        if(gen_ctf->value)
//ZOID
        CTFApplyRegeneration(ent);
//ZOID

        // update chase cam if being followed
        for (i = 1; i <= maxclients->value; i++)
        {
                other = g_edicts + i;
                if (other->inuse && other->client->chase_target == ent)
                        UpdateChaseCam(other);
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
        gclient_t       *client;
        int             buttonMask;

        if (level.intermissiontime)
                return;

        client = ent->client;

        if (deathmatch->value &&
            client->pers.spectator != client->resp.spectator &&
            (level.time - client->respawn_time) >= 5)
		{
                spectator_respawn(ent);
                return;
        }

        // run weapon animations if it hasn't been done by a ucmd_t
        if (!client->weapon_thunk && !client->resp.spectator//)
//ZOID
              && ent->movetype != MOVETYPE_NOCLIP)
//ZOID
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
