#include "g_local.h"


void heavybreathing (edict_t *ent)
{
	if(ent->deadflag)
		return;
	ent->breathdelay ++;
	if (ent->breathdelay >10)
		ent->breathdelay = 0;
	
	if (ent->breathdelay % 10 == 5)
	{
		return;	
		// Fixed Fear :)		
		if (ent->stamina > 60)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("breath3.wav"), 1, ATTN_NORM, 0);
		else if (ent->stamina > 30)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("breath2.wav"), 1, ATTN_NORM, 0);
		else if (ent->stamina > 0)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("breath1.wav"), 1, ATTN_NORM, 0);
	}
	
}
void MatrixStamina (edict_t *ent)
{

	if (ent->client->resp.tank)
	{
		ent->stamina += 1;
		if (ent->stamina > 250)
			ent->stamina = 250;
	}
	
	if (ent->staminalevel && (ent->max_stamina != (ent->staminalevel * 50) + 200))
		ent->max_stamina = (ent->staminalevel * 50) + 200;
	
	

	if (ent->deadflag || ent->health < 0)
		return;

	if (ent->client->pers.spectator)
		return;


	
		ent->blocking = false;

	
	ent->stamina += 1 * ((ent->staminalevel /2) + 1);

	if (ent->stamina > ent->max_stamina)
	{
		ent->stamina = ent->stamina - .99;
	}
	
	if (ent->stamina < 0)
		ent->stamina = 0;

	if (ent->healthlevel && (ent->max_health != (ent->healthlevel * 50) + 100))
	{
		ent->max_health = (ent->healthlevel * 50) + 100;
		ent->client->pers.max_health = (ent->healthlevel * 50) + 100;
	}
	
	// Stamina

	if(!deathmatch->value && ent->client->cloak_framenum> level.framenum)
		ent->flags |= FL_NOTARGET;
	else
		ent->flags &=~ FL_NOTARGET;
	
}