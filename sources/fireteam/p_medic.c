#include "g_local.h"

void Set_Heal_Health(edict_t *ent, edict_t *other)
{
	other->client->pers.heal_health += ent->count;

    if (!(ent->spawnflags & DROPPED_ITEM))
	    SetRespawn (ent, 30);
}

void Player_Heal(edict_t *ent, edict_t *other)
{
	int heal_needed;        //How much healing our patient needs
    int heal_left;          //How much our medic has to give

    if( (heal_needed = other->max_health - other->health) <=0 || ent->health <=0)
	    return;

    if( (heal_left = ent->client->pers.heal_health) <=0)
    {       //No healing points. Exit quietly...
        return;
    }

    //OK, now we figure out whether we have enough points to completely heal the pt.
    if(heal_left >= heal_needed)
    {       //Enough to completely heal the patient
	    other->health += heal_needed;   //Add back the points we need to be healed
	    heal_left -= heal_needed;               //...and subtract them from the medic's points
        //Play a sound for the player.
        gi.sound(ent, CHAN_VOICE, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
    }
    else
    {       //Not enough. Use what we have.
        other->health += heal_left;             //Take all the points remaining
        heal_left = 0;
        //Play a sound for the player.
        gi.sound(ent, CHAN_VOICE, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
    }

    //Now, set the variables for the medic based on what we actually used...
    ent->client->pers.heal_health = heal_left;
}
