#include "g_local.h"
#include "m_player.h"

int CountHealthItems(edict_t *ent, int type)
{
        int count, i;

	count = 0;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == type)
        		count++;
	}
	
	if ((ent->client->pers.cstats[CSTAT_RIGHTHAND] == type) && (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] > 0))
		count++;

	if ((ent->client->pers.cstats[CSTAT_LEFTHAND] == type) && (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] > 0))
		count++;
	
	return count;
}

/*
=================
UseHealth
=================
*/
void UseHealth(edict_t *ent, int type)
{
	int 	i;

	if (ent->health >= ent->max_health)
		return;

	switch(type)
	{
	case II_HEALTH:
		ent->health += 10;
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
		break;

	case II_HEALTH_LARGE:
		ent->health += 30;
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
		break;

	default:
		break;
	}

	if (ent->health > ent->max_health)
		ent->health = ent->max_health;
	
	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == type)
        	{
			RemoveItem(ent, i);
			ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 1;
			break;
		}
	}
}

/*
=================
Think_Health

=================
*/

void Think_Health(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammo = CountHealthItems(ent, II_HEALTH);
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                else if (ent->client->newLeftWeapon > -1)
                        ChangeLeftWeapon(ent);
		else if ((ammo > 0) && (ent->client->latched_buttons & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			UseHealth(ent, II_HEALTH);
		}

        	if (ammo < 1)
		{        		
                        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
                        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
		        SetupItemModels(ent);
			AutoSwitchWeapon(ent, 0, 0);
                	return;
		}
                break;

        default:
                newState = W_READY;
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        default:
                break;
        }

        if (newFrame != -1)
                ent->client->ps.gunframe = newFrame;

        if (newState != -1)
                ent->client->weaponstate = newState;
}

/*
=================
Think_HealthLarge

=================
*/

void Think_HealthLarge(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammo = CountHealthItems(ent, II_HEALTH_LARGE);
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                else if (ent->client->newLeftWeapon > -1)
                        ChangeLeftWeapon(ent);
		else if ((ammo > 0) && (ent->client->latched_buttons & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			UseHealth(ent, II_HEALTH_LARGE);
		}

        	if (ammo < 1)
		{        		
                        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
                        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
		        SetupItemModels(ent);
			AutoSwitchWeapon(ent, 0, 0);
                	return;
		}
                break;

        default:
                newState = W_READY;
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        default:
                break;
        }

        if (newFrame != -1)
                ent->client->ps.gunframe = newFrame;

        if (newState != -1)
                ent->client->weaponstate = newState;
}
