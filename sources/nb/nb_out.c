/*========================================================================
	Neuralbot v0.6						23/November/99
	--------------
		
Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
	If you want to use any of the code for any of your projects, feel free.
If you release this project, drop me a line as I would like to see what 
this code has been used for.
	If you wish to use any of this code in a commercial release, you must gain 
my written consent first.

The area of bot AI is large, and way more experimentation needs to be done
with neural-networks and g.a.s in bots than I could ever do.  So I encourage 
you all to use this code as a base or inspiration or whatever and see if you 
can squeeze some learning out of a bot.

  Nick Chapman/Ono-Sendai		nickamy@paradise.net.nz		
								onosendai@botepidemic.com
								icq no# 19054613

==========================================================================*/
#include "g_local.h"
/*==========================================================================
nb_out
------
Output functions
==========================================================================*/


void Bot_Cmd_Use_f (edict_t *ent, char* weapon);

//NOTE: not currently used
void fire_railgun (edict_t* self)
{
	self->client->botinfo.ucmd.buttons |= BUTTON_ATTACK;	

}


void fire_weapon (edict_t* self)
{
	self->client->botinfo.ucmd.buttons |= BUTTON_ATTACK;
}



void select_blaster(edict_t* self)
{
	Bot_Cmd_Use_f(self, "blaster");
}

void select_shotgun(edict_t* self)
{
	Bot_Cmd_Use_f(self, "shotgun");	
}

void select_sshotgun(edict_t* self)
{						
	Bot_Cmd_Use_f(self, "super shotgun");
}

void select_mgun(edict_t* self)
{
	Bot_Cmd_Use_f(self, "machinegun");						
}
void select_chaingun(edict_t* self)
{
	Bot_Cmd_Use_f(self, "chaingun");
}

void select_grenadel(edict_t* self)
{
	Bot_Cmd_Use_f(self, "grenade launcher");
}

void select_rocketl(edict_t* self)
{
	Bot_Cmd_Use_f(self, "rocket launcher");						
}

void select_hblaster(edict_t* self)
{
	Bot_Cmd_Use_f(self, "hyperblaster");	
}

void select_railgun(edict_t* self)
{
	Bot_Cmd_Use_f(self, "railgun");	
}

void select_bfg10k(edict_t* self)
{
	Bot_Cmd_Use_f(self, "bfg10k");	
}

void bot_jump (edict_t* self)
{
	self->client->botinfo.ucmd.upmove += sv_maxvelocity->value;	//of course upmove needs to be set to 0 every movement	
}

void bot_duck (edict_t* self)
{
	if(thegame->allowcrouch)
		self->client->botinfo.ucmd.upmove -= sv_maxvelocity->value;	
}












void go_forwards (edict_t* self)
{
	self->client->botinfo.forwardvotes++;
}


void go_back (edict_t* self)
{
	self->client->botinfo.forwardvotes--;
}



void go_right (edict_t* self)
{
	self->client->botinfo.rightvotes++;
}

void go_left (edict_t* self)
{
	self->client->botinfo.rightvotes--;
}

void bot_run (edict_t* self)
{
	self->client->botinfo.runvote = true;//bot will run if it moves now
}













/*

look functions ----- 

  max pitch is 90 (straight down) and min is -90 (straight up)
*/


void look_right (edict_t* self)
{
	// NOTE: check this is the right way
	self->client->v_angle[YAW] -= TURN_MEDIUM;

	if ( self->client->v_angle[YAW] < 0 )
		self->client->v_angle[YAW] += 360; 


}
/*====================================================================
look_left
----------------------------------------------------------------------
====================================================================*/
void look_left (edict_t* self)
{
	
	
	self->client->v_angle[YAW] += TURN_MEDIUM;

	if ( self->client->v_angle[YAW] > 360 )
		self->client->v_angle[YAW] -= 360; 
	

}
/*====================================================================
lookjust_left/right
----------------------------------------------------------------------
look just 2 degrees to the left or the right.
hopefully the bots will find this useful for those accurate rail snipings.
====================================================================*/

void lookjust_right(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)//NOTE: put this in a better place
		return;

	self->client->v_angle[YAW] -= TURN_JUST * activation;

	if ( self->client->v_angle[YAW] < 0 )
		self->client->v_angle[YAW] += 360; 

}
void lookjust_left(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;

	self->client->v_angle[YAW] += TURN_JUST * activation;

	if ( self->client->v_angle[YAW] > 360 )
		self->client->v_angle[YAW] -= 360; 

}
//Currently used://
void looktiny_right(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;

	self->client->v_angle[YAW] -= activation * TURN_TINY;

	if ( self->client->v_angle[YAW] < 0 )
		self->client->v_angle[YAW] += 360; 


}
void looktiny_left(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;
	
	self->client->v_angle[YAW] += activation * TURN_TINY;

	if ( self->client->v_angle[YAW] > 360 )
		self->client->v_angle[YAW] -= 360; 

}

void look_up (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] -= LOOKUP * activation;

	if ( self->client->v_angle[PITCH] < -90 )
		self->client->v_angle[PITCH] = -90;
	
}


void look_down (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] += LOOKUP * activation;


	if ( self->client->v_angle[PITCH] > 90 )
		self->client->v_angle[PITCH] = 90;
}


void looklots_right (edict_t* self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;
	
	self->client->v_angle[YAW] -= TURN_LOTS * activation;

	if ( self->client->v_angle[YAW] < 0 )
		self->client->v_angle[YAW] += 360;

}
void looklots_left (edict_t* self, float activation)
{	
	if (self->deadflag != DEAD_NO)
		return;

	self->client->v_angle[YAW] += TURN_LOTS * activation;

	if ( self->client->v_angle[YAW] > 360 )
		self->client->v_angle[YAW] -= 360; 

}



void lookheaps_right(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;
	
	self->client->v_angle[YAW] -= TURN_HEAPS * activation;

	if ( self->client->v_angle[YAW] < 0 )
		self->client->v_angle[YAW] += 360;

}
void lookheaps_left(edict_t*self, float activation)
{
	if (self->deadflag != DEAD_NO)
		return;
	
	self->client->v_angle[YAW] += TURN_HEAPS * activation;

	if ( self->client->v_angle[YAW] > 360 )
		self->client->v_angle[YAW] -= 360; 

}
//end currently used//

void lookjust_up (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] -= LOOKUP_JUST * activation;


	if ( self->client->v_angle[PITCH] < -90 )
		self->client->v_angle[PITCH] = -90;
}


void lookjust_down (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] += LOOKUP_JUST * activation;

	if ( self->client->v_angle[PITCH] > 90 )
		self->client->v_angle[PITCH] = 90;

}

void looklots_up (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] -= LOOKUP_LOTS * activation;


	if ( self->client->v_angle[PITCH] < -90 )
		self->client->v_angle[PITCH] = -90;
}


void looklots_down (edict_t* self, float activation)
{
	if(!thegame->lookupanddown)
		return;

	self->client->v_angle[PITCH] += LOOKUP_LOTS * activation;

	if ( self->client->v_angle[PITCH] > 90 )
		self->client->v_angle[PITCH] = 90;

}


void look_straight (edict_t* self)//center view - view pitch := 0
{
	if(thegame->allowcenterview)
		self->client->v_angle[PITCH] = 0;
}

/*======================================================================
Bot_Cmd_Use_f
-------------
an example:

Bot_Cmd_Use_f(railgun);

does the same thing as if the bot typed 'use railgun' into the console
(changes to railgun)
======================================================================*/

void Bot_Cmd_Use_f (edict_t *ent, char* s)
{
	int			index;
	gitem_t		*it;


	it = FindItem (s);
	if (!it)
	{

		return;
	}
	if (!it->use)
	{
		
		return;
	}
	index = ITEM_INDEX(it);

	//IMITATION NEWCODE: set change weapon output flags
	//if(imitationOn(ent))
	//	getChangeWeaponOutput(ent, it);

	if (!ent->client->pers.inventory[index])
	{
		//NEWCODE: don't cprintf to a bot, it crashes the game!
		//if(ent->client->botinfo.isbot == false)
		//	gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);

	ent->client->botinfo.last_changeweapon_time = level.time;//NOTE: take this out
}



