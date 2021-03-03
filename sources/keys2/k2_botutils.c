#include "g_local.h"


//Returns the value of the "desired" key, if it's gretaer than the one
//the bot is carrying.
int K2_botCompareKey(edict_t *ent, int key)
{
	//gi.dprintf("Bot compared keys\n");
	if ( key > ent->client->key)
		return key;
	else
		return 0;
}

qboolean K2_botCheckKeyTimer(edict_t *self)
{
	int	timeremaining = 0;
	
	switch(self->client->key)
	{
	case KEY_ANTIKEY:
		timeremaining = ((antitime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_REGENERATION:
		timeremaining = ((regentime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_FUTILITY:
		timeremaining = ((futilitytime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_INFLICTION:
		timeremaining = ((inflictiontime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_HASTE:
		timeremaining = ((hastetime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_STEALTH:
		timeremaining = ((stealthtime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_HOMING:
		timeremaining = ((homingtime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	case KEY_BFK:
		timeremaining = ((bfktime->value/2) - ((self->client->k2_key_framenum - level.framenum)/10));
		break;
	}

	if (timeremaining > 0)
	{
		//gi.dprintf("Bot has no time remaining (%i)\n",timeremaining);
		return true;
	}
	//gi.dprintf("Bot has %i time remaining\n",timeremaining);
	return false;


}


void K2_HomingInformDanger(edict_t *self)
{
	int	i;
	edict_t	*bot;
	vec3_t	org2, vec;
	float	dist;

	VectorMA(self->s.origin, 0.25, self->velocity, org2);

	// scan for bot's to inform
	for (i=0; i<num_players; i++)
	{
		bot = players[i];

		if (bot->bot_client && (bot->bot_stats->combat < 4))
			continue;

		if ((fabs(bot->s.origin[0] - self->s.origin[0]) > 300) ||
			(fabs(bot->s.origin[1] - self->s.origin[1]) > 300))
			continue;

		// make sure rocket is heading somewhat towards this bot
		VectorSubtract(org2, bot->s.origin, vec);
		if (((dist=entdist(bot, self)) - VectorLength(vec)) > 75)
		{	// yup, inform bot of danger
			bot->avoid_ent = self;
		}
	}

}

void K2_botBFKInformDanger(edict_t *self)
{

	int	i;
	edict_t	*bot;

	if(!K2_IsBFK(self))
		return;
	
	// scan for bot's to inform
	for (i=0; i<num_players; i++)
	{
		bot = players[i];

		if (bot->bot_client && (bot->bot_stats->combat < 4))
			continue;

		//if ((fabs(bot->s.origin[0] - self->s.origin[0]) > 300) ||
		//	(fabs(bot->s.origin[1] - self->s.origin[1]) > 300))
		//	continue;

		//If the other bot has BFK, he doesn't care
		if(!K2_IsBFK(bot))
			bot->avoid_ent = self;
		
	}
}

qboolean K2_botCanSeeStealth(edict_t *other)
{

	//Returns true if the player is not stealth, or the player is stealth, but is visible

	if(!K2_IsStealth(other))
		return true;

	if( (other->client->anim_priority == ANIM_PAIN) || (other->client->anim_priority == ANIM_ATTACK))
		return true;

	return false;

}