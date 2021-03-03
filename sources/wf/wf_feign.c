/*****************************************************************
 
 Feign source code - by Acrid-, acridcola@hotmail.com
 
 ..............................................................
 
 This file is Copyright(c) 1999, Acrid-, All Rights Reserved.
 
 ..............................................................
 

 Should you decide to release a modified version of the Feign, you
 MUST include the following text (minus the BEGIN and END lines) in 
 the documentation for your modification, and also on all web pages 
 related to your modification, should they exist.
 
 --- BEGIN ---
 
 The Feign and related Feign code is a product of Acrid- designed 
 for Weapons Factory, and is available as part of the Weapons Factory 
 Source Code or a seperate tutorial.
 
 This program MUST NOT be sold in ANY form. If you have paid for
 this product, you should contact Acrid- at:
 acridcola@hotmail.com
 
 --- END ---
 
 have fun,
 
 Acrid-
 
 *****************************************************************/
#include "g_local.h"
//Feign code by Acrid
//
// last things, stop rapid fire weapons from continued fire after feign off,
// maybe turn off pain sounds when taking damage

void feign_on (edict_t *ent)
{
    static int i;
	i = (i+1)%3;

	//Take some damage
	T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 10, 10, DAMAGE_NO_ARMOR, MOD_FEIGN);
	
	VectorClear (ent->avelocity);
	ent->takedamage = DAMAGE_YES;
	ent->movetype = MOVETYPE_TOSS;//needed _none so pain doesnt start animation//solved elsewhere
	ent->s.modelindex2 = 0;	// remove linked weapon model
	ent->s.angles[0] = 0;
	ent->s.angles[2] = 0;
	ent->s.sound = 0;
	ent->client->weapon_sound = 0;
	ent->maxs[2] = -8;
	ent->viewheight = 0;

    /*  set knockback flag so grenades and other weapons 
	    with knockback dont move players view
	    used in T_damage code in g_combat.c */
	ent->flags |= FL_NO_KNOCKBACK;

	// stop running/footsteps
    VectorClear (ent->velocity);
	
	//newgrap 5/99
	if (Is_Grappling(ent->client)) 
	{
		CTFPlayerResetGrapple2(ent);
	}

	//dont keep firing/no model to fire/model removed from player view
	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = NULL;// needed?
	ent->client->ps.gunindex = 0;

			// start a random animation
            switch (i)
			{
			case 0:
				ent->s.frame = FRAME_death101-1;
				ent->client->anim_end = FRAME_death106;
				break;
			case 1:
				ent->s.frame = FRAME_death201-1;
				ent->client->anim_end = FRAME_death206;
				break;
			case 2:
				ent->s.frame = FRAME_death301-1;
				ent->client->anim_end = FRAME_death308;
				break;
			}
	gi.sound (ent, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
}
void feign_off(edict_t *ent)
{
	    
	    //relink weapon model
        ent->s.modelindex2 = 255;
		//clear knockback flag
        ent->flags &= ~FL_NO_KNOCKBACK;
		//bring back player view weapon
        ent->client->pers.weapon = ent->client->pers.lastweapon;

		if (ent->client->pers.weapon)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

}