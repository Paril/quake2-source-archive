/*
====================
HOLY WARS: Main file
====================
*/


// Declare that we are in hw_main.c (for hw_main.h)
#define IN_HW_MAIN
#include "g_local.h"

// HWv2.1
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies

// declations of extern functions:
qboolean IsFemale (edict_t *ent);

// end HWv2.1


// *****************
// global variables:
// *****************
hw_game_t hw;           // game globals
cvar_t  *hw_edit;       // console variable: toggle "editing mode"
cvar_t  *hw_debug;      // console variable: toggle "debug mode"

cvar_t *hw_sinkill;			// points awarded for sinner killing sinner
cvar_t *hw_holykill;		// points awarded for saint killing sinner
cvar_t *hw_killheretic;		// points awarded for killing heretic
cvar_t *hw_killsaint;		// points awarded for killing saint
cvar_t *hw_takehalo;		// points awarded for picking up halo
cvar_t *hw_heretic;			// points awarded awarded for becoming a heretic
cvar_t *hw_llama;			// points needed to become a llama
cvar_t *hw_spawn;			// seconds until first halo appears
cvar_t *hw_remove;			// seconds until fallen halo disappears
cvar_t *hw_respawn;			// seconds until removed halo reappears
cvar_t *hw_heresy;			// heresy threshold parameter
cvar_t *hw_sinhurt;			// allow sinners to damage each other
cvar_t *hw_melee;			// melee range heresy calculation
cvar_t *hw_near;			// near range heresy calculation
cvar_t *hw_far;				// far range heresy calculation
cvar_t *hw_hbonus;			// halo health bonus per enemy
cvar_t *hw_abonus;			// halo armor bonus per enemy
cvar_t *hw_maxbonus;		// max halo bonus
cvar_t *hw_report;			// saint report interval (seconds)
cvar_t *hw_intermission;	// number of seconds for intermission
cvar_t *hw_saintsound;		// flag for "oooh" sound when saint attacks
cvar_t *hw_usebfg;			// allow use of BFG (or turn it into hyperblaster)
cvar_t *hw_shoothalo;		// halo can be shot and bounced
cvar_t *hw_inertia;			// halo moves around saint
cvar_t *hw_height;			// halo z-position relative to player
cvar_t *hw_tolerance;		// halo movement tolerance
cvar_t *hw_maxspeed;		// halo maxium velocity
cvar_t *hw_accelx;			// halo acceleration
cvar_t *hw_accely;
cvar_t *hw_accelz;
cvar_t *hw_brakex;			// halo braking
cvar_t *hw_brakey;
cvar_t *hw_brakez;
cvar_t *hw_cycles;			// number of halo physics calculations per frame
cvar_t *hw_halofall;		// speed of falling halo

cvar_t *hw_scoreboard;		// scoreboard mode (detailed or standard)
cvar_t *use_player_id;		// persistent storage of player ID usage

// ***********************
// Inits the HolyWars game
// ***********************
void HW_InitGame(void)
{
	// console variables
	hw_edit = gi.cvar("hw_edit", "0", 0);
	hw_debug = gi.cvar("hw_debug", "0", 0);
	hw_sinkill = gi.cvar("hw_sinkill", "0", CVAR_LATCH);
	hw_holykill = gi.cvar("hw_holykill", "1", CVAR_LATCH);
	hw_killheretic = gi.cvar("hw_killheretic", "3", CVAR_LATCH);
	hw_killsaint = gi.cvar("hw_killsaint", "3", CVAR_LATCH);
	hw_takehalo = gi.cvar("hw_takehalo", "2", CVAR_LATCH);
	hw_heretic = gi.cvar("hw_heretic", "-5", CVAR_LATCH);
	hw_llama = gi.cvar("hw_llama", "-14", CVAR_LATCH);
	hw_heresy = gi.cvar("hw_heresy", "1.3", CVAR_LATCH);
	hw_sinhurt = gi.cvar("hw_sinhurt", "1", CVAR_LATCH);
	hw_melee = gi.cvar("hw_melee", "100", CVAR_LATCH);
	hw_near = gi.cvar("hw_near", "400", CVAR_LATCH);
	hw_far = gi.cvar("hw_far", "1000", CVAR_LATCH);
	hw_spawn = gi.cvar("hw_spawn", "15", CVAR_LATCH);
	hw_remove = gi.cvar("hw_remove", "10", CVAR_LATCH);
	hw_respawn = gi.cvar("hw_respawn", "10", CVAR_LATCH);
	hw_hbonus = gi.cvar("hw_hbonus", "30", CVAR_LATCH);
	hw_abonus = gi.cvar("hw_abonus", "30", CVAR_LATCH);
	hw_maxbonus = gi.cvar("hw_maxbonus", "200", CVAR_LATCH);
	hw_report = gi.cvar("hw_report", "40", 0);
	hw_intermission = gi.cvar("hw_intermission", "25", 0);
	hw_saintsound = gi.cvar("hw_saintsound", "0", CVAR_LATCH);
	hw_usebfg = gi.cvar("hw_usebfg", "0", CVAR_LATCH);
	hw_shoothalo = gi.cvar("hw_shoothalo", "1", CVAR_LATCH);
	hw_inertia = gi.cvar("hw_inertia", "0", 0);
	hw_height = gi.cvar("hw_height", "10", 0);
	hw_tolerance = gi.cvar("hw_tolerance", "7", 0);
	hw_maxspeed = gi.cvar("hw_maxspeed", "800", 0);
	hw_accelx = gi.cvar("hw_accelx", "0.7", 0);
	hw_accely = gi.cvar("hw_accely", "0.7", 0);
	hw_accelz = gi.cvar("hw_accelz", "0.6", 0);
	hw_brakex = gi.cvar("hw_brakex", "6", 0);
	hw_brakey = gi.cvar("hw_brakey", "6", 0);
	hw_brakez = gi.cvar("hw_brakez", "5", 0);
	hw_cycles = gi.cvar("hw_cycles", "5", 0);
	hw_halofall = gi.cvar("hw_halofall", "9", 0);

	hw_scoreboard = gi.cvar("hw_scoreboard", "1", CVAR_ARCHIVE);
	use_player_id = gi.cvar("use_player_id", "0", CVAR_ARCHIVE);
}


// HWv2.1 - The following function is new to this version

// *********************
// Creates the Halo base
// *********************
void HW_CreateHaloBase(void)
{
	if (hw.halobase != NULL)
		return;		// the base was already created

	// creates the halo base, places it into the right place
	// and uses it to generate the Halo at the right time.
	hw.halobase = G_Spawn();
	HW_BaseInLevel(hw.halobase);
	hw.halobase->classname = "halobase";
	hw.halobase->movetype = MOVETYPE_NOCLIP;
	hw.halobase->solid = SOLID_NOT;
	hw.halobase->svflags |= SVF_NOCLIENT;
	hw.halobase->nextthink = level.time + hw_spawn->value;
	hw.halobase->think = HW_SpawnHalo;
	gi.linkentity(hw.halobase);
}


// ***************************************************************
// Given a killed player and a killer player, updates the score
// of the killer.
// ***************************************************************
// HWv2.1 - many parts of this function has been modified
void HW_Frag(edict_t *killed, edict_t *killer, int mod, char *weapon)
{
  if (killer == killed)
        return;         // Let ClientObituary manage suicides

  switch(killer->client->plstatus)
        {
  case SAINT:   // SAINT kills .....

        if (killed->client->plstatus == HERETIC)
		{
			// ---------------------
			// Saint killing heretic
			// ---------------------

            // Bonus for Saint killing Heretic:
			killer->client->resp.score += hw_holykill->value + hw_killheretic->value;

			// StdLog - log score for Saint killing Heretic
			sl_LogScore( &gi,
				 killer->client->pers.netname,
				 killed->client->pers.netname,
				 "Saint vs. Heretic",
				 weapon,
				 hw_holykill->value + hw_killheretic->value,
				 level.time );
		}
		else
		{
			// --------------------
			// Saint killing sinner
			// --------------------

		    killer->client->resp.score += hw_holykill->value;

			// StdLog - log score for Saint killing Sinner
			sl_LogScore( &gi,
			 killer->client->pers.netname,
			 killed->client->pers.netname,
			 "Saint vs. Sinner",
			 weapon,
			 hw_holykill->value,
			 level.time );
		}

		break;
  case HERETIC: // Heretics don't need points!
        break;
  default:      // SINNER
        switch(killed->client->plstatus)
                {
        case SAINT:  // sinner killing Saint
                killer->client->resp.score += hw_killsaint->value;

				// StdLog - log score
				sl_LogScore( &gi,
					 killer->client->pers.netname,
					 killed->client->pers.netname,
					 "Sinner vs. Saint",
					 weapon,
					 hw_killsaint->value,
					 level.time );

                killer->client->resp.killedsaints++;
                HW_Heresy_SaintKilled(killer); // if he kills the saint .. heresy bonus!
                break;
        case HERETIC:   // Sinner killing Heretic
                killer->client->resp.score += hw_killheretic->value;

				// StdLog - log score
				sl_LogScore( &gi,
					 killer->client->pers.netname,
					 killed->client->pers.netname,
					 "Sinner vs. Heretic",
					 weapon,
					 hw_killheretic->value,
					 level.time );

                break;
        default:  // The killed must be a Sinner: Sinner killing Sinner
                // "normal kill" score:
                killer->client->resp.score += hw_sinkill->value;

				// StdLog - log score
				sl_LogScore( &gi,
					 killer->client->pers.netname,
					 killed->client->pers.netname,
					 "Sinner vs. Sinner",
					 weapon,
					 hw_sinkill->value,
					 level.time );

                // There is a saint: see if it's time for punishment...
                if (hw.halostatus == OWNED)
                        HW_Heresy_MainCalc(killed, killer, mod);
                }
        }
}


// *****************************
// The player becomes an heretic
// *****************************
void HW_BecomeHeretic(edict_t *p)
{
        edict_t *swarm;
        gitem_t *it;

        p->client->plstatus = HERETIC;
        p->client->resp.llamaness++;    // HWv2.1

// HWv2.1
	    // scream:
        if (IsFemale(p))  // female player
                gi.sound(p, CHAN_VOICE, gi.soundindex("/players/female/pain100_1.wav"), 1, ATTN_NORM, 0);
        else  // male player
        {
                int r;
                r = rand() % 3;
                //Play a random male scream:
                if (r == 1)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_her1.wav"), 1, ATTN_NORM, 0);
                else if (r == 2)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_her2.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_her3.wav"), 1, ATTN_NORM, 0);
        }

        // take out all armor, cells, slugs, grenades and rockets
		// (version 2.0 only took out cells)
        if (it = FindItem("Jacket Armor"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Combat Armor"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Body Armor"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Cells"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Slugs"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Grenades"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;
        if (it = FindItem("Rockets"))
        	p->client->pers.inventory[ITEM_INDEX(it)] = 0;

		// Reduce health to 50 max
        if (p->health > 50)
        	p->health = 50;

		// Loose frags:
		p->client->resp.score += hw_heretic->value;

		// StdLog - log score
		sl_LogScore( &gi,
			 p->client->pers.netname,
			 NULL,
			 "Become Heretic",
			 NULL,
			 hw_heretic->value,
			 level.time );

// end HWv2.1

        // Shame on you!
        HW_ReportNewHeretic(p);

        // Create a swarm of flies
        swarm = G_Spawn();
        swarm->classname = "swarm";
        gi.setmodel (swarm, "models/invis/tris.md2");    // HWv2.1
        swarm->s.sound = gi.soundindex ("infantry/inflies1.wav");
        swarm->movetype = MOVETYPE_NOCLIP;
        swarm->solid = SOLID_NOT;
        swarm->s.effects |= EF_FLIES;
        swarm->hwowner = p;
        VectorCopy (p->s.origin, swarm->s.origin);
        swarm->think = HW_SwarmThink;
        swarm->nextthink = level.time + 0.1;
        gi.linkentity(swarm);
}


// **************************
// The player becomes a Saint
// **************************
void HW_BecomeSaint(edict_t *p)
{
        int hbonus;       // Health bonus
        int abonus;       // Armor bonus
        edict_t *swarm;
        edict_t *rightswarm;

		if (p->client->plstatus == HERETIC)  // Forgive this Heretic
		{
			// find and kill the flies
			// HWFIXME: make a function for this (it's also done in HW_PlayerDie)
			swarm = NULL;
			rightswarm = NULL;
			while ((swarm = G_Find (swarm, FOFS(classname), "swarm")) != NULL)
			{
				if (swarm->hwowner == p)
					rightswarm = swarm;
			}
			if (rightswarm != NULL)
			{
				G_FreeEdict(rightswarm);
			}
		}

		p->client->plstatus = SAINT;                         // there's a new Saint in town!
// HWv2.1
        p->client->resp.lasttakentime = level.time;          // memorize the current time (used for final stats)
        p->client->resp.takenhalos++;                        // increase number of halos taken
// end HWv2.1
        p->client->heresy = 0;                               // sanctity forgives all sins!
        p->client->resp.score += hw_takehalo->value;       // increase picker's frag count.
// HWv2.1
		// StdLog - log score
		sl_LogScore( &gi,
			 p->client->pers.netname,
			 NULL,
			 "Become Saint",
			 NULL,
			 hw_takehalo->value,
			 level.time );
// end HWv2.1

        // Laughter sound:
        if (IsFemale(p))  // female player
                gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_fpick.wav"), 1, ATTN_NORM, 0);
        else  // male player
        {
                int r;
                r = rand() % 5;
                //Play a random laugh:
                if (r == 1)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_pick1.wav"), 1, ATTN_NORM, 0);
                else if (r == 2)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_pick2.wav"), 1, ATTN_NORM, 0);
                else if (r == 3)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_pick3.wav"), 1, ATTN_NORM, 0);
                else if (r == 4)
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_pick4.wav"), 1, ATTN_NORM, 0);
                else
                        gi.sound(p, CHAN_VOICE, gi.soundindex("hw/hw_pick5.wav"), 1, ATTN_NORM, 0);
        }

        // Reports the event to everybody:
        HW_ReportNewSaint(p);

        // -------
        // Bonuses
        // -------
        // Calc bonus for health:
        hbonus = HW_CountEnemies(p) * hw_hbonus->value;
        if (hbonus > hw_maxbonus->value)
                hbonus = hw_maxbonus->value;
        if (hbonus < 0)      // Negative bonuses can happen in teamplay.
                hbonus = 0;
        // Add energy to the player:
        p->health += hbonus;
        // Calc bonus for armor:
        abonus = HW_CountEnemies(p) * hw_abonus->value;
        if (abonus > hw_maxbonus->value)
                abonus = hw_maxbonus->value;
        if (abonus < 0)
                abonus = 0;
        // Add armor to the player:
        if (!ArmorIndex(p))
                p->client->pers.inventory[ITEM_INDEX(FindItem("Jacket Armor"))] = abonus;
        else
                p->client->pers.inventory[ArmorIndex(p)] += abonus;
}



// **************************
// Invoked when a player dies
// **************************
void HW_PlayerDie(edict_t *ent)
{
        edict_t *visiblehalo;
        edict_t *swarm;
        edict_t *rightswarm;

        // -----------------
        // forgive heretics:
        // -----------------
        ent->client->heresy = 0;
        if (ent->client->plstatus == HERETIC)
        {
                ent->client->plstatus = SINNER;

                // find and kill the flies
                swarm = NULL;
                rightswarm = NULL;
                while ((swarm = G_Find (swarm, FOFS(classname), "swarm")) != NULL)
                {
                        if (swarm->hwowner == ent)
                                rightswarm = swarm;
                }
                if (rightswarm != NULL)
                {
                        G_FreeEdict(rightswarm);
                }
        }

        // takes away all the effects from the body
        ent->s.effects &= ~(EF_FLAG1 | EF_FLAG2 | EF_FLIES);
        ent->s.renderfx &= ~RF_GLOW;

        if (ent->client->plstatus != SAINT)
                return;

        // --------------------------------------------------------
        // The following is only executed if the player was a saint
        // --------------------------------------------------------

        HW_DropHalo(ent);
        ent->client->plstatus = SINNER;
        ent->client->resp.sanctitytime += (level.time - ent->client->resp.lasttakentime);  // HWv2.1
        gi.sound(ent, CHAN_VOICE, gi.soundindex("hw/hw_death.wav"), 1, ATTN_NONE, 0);
        HW_ReportMartyr(ent);                                   // Tell everybody

        // find and destroy the visible halo thing
        if ((visiblehalo = G_Find (NULL, FOFS(classname), "visiblehalo")) != NULL)
        {
                G_FreeEdict(visiblehalo);
        }
}


// **********************
// A swarm of flies moves
// **********************
void HW_SwarmThink(edict_t *ent)
{
// HWFIXME: kill flies when underwater?

        VectorCopy(ent->hwowner->s.origin, ent->s.origin);
        ent->think = HW_SwarmThink;
        ent->nextthink = level.time + 0.1;
                gi.linkentity(ent);
}


// ***********************
// Sets effects for player
// ***********************
void HW_SetClientEffects(edict_t *player)
{
        if (player->client->plstatus == SAINT)
        {
                player->s.effects |= EF_FLAG2;
                player->s.renderfx |= RF_GLOW;
        }
        else if (player->client->plstatus == HERETIC)
        {
                // add some more flies (only visible by other players):
                player->s.effects |= EF_FLAG1;
                player->s.renderfx |= RF_GLOW;
        }
}


// -----------------------
// Sound of Saint shooting
// -----------------------
void HW_SaintShootSound(edict_t *ent)
{
        float volume;

        if (!ent->client)
                return;

        if (ent->client->silencer_shots)
                volume = 0.3;
        else
                volume = 0.7;

        if (level.time - ent->client->saintsoundtime > 1.3)
        {
                gi.sound(ent, CHAN_AUTO, gi.soundindex("hw/hw_saint.wav"), volume, ATTN_NORM, 0);
                ent->client->saintsoundtime = level.time;
        }
}


// ------------------
// End of level stuff
// ------------------
void HW_EndOfLevel(void)
{
        edict_t         *ent;

        // Destroy the visible halo:
        if ((ent = G_Find (NULL, FOFS(classname), "visiblehalo")) != NULL)
        {
                G_FreeEdict(ent);
        }

        // The halo disappears:
        if ((ent = G_Find (NULL, FOFS(classname), "halo")) != NULL)
        {
                G_FreeEdict(ent);
        }
        hw.halostatus = HEAVEN;

        // destroy all swarms:
        ent = NULL;
        while ((ent = G_Find (ent, FOFS(classname), "swarm")) != NULL)
        {
                G_FreeEdict(ent);
        }

        // scan players:
        ent = NULL;
        while ((ent = G_Find (ent, FOFS(classname), "player")) != NULL)
        {
                if (ent->client != NULL)
                {
                        // adjusts stats for the Saint:
                        if(ent->client->plstatus == SAINT)
                                ent->client->resp.sanctitytime += (level.time - ent->client->resp.lasttakentime);  // HWv2.1

                        ent->client->plstatus = SINNER;
                }
        }

}

