#include "g_local.h"
#include "m_player.h"

/*
=================
Think_Hands

Punching, neck snapping etc
=================
*/

void Think_Hands(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                else if (ent->client->newLeftWeapon > -1)
                        ChangeLeftWeapon(ent);
                break;

        case W_FIRING:
                break;

        case W_ALT_FIRING:
                break;

        case W_RIGHT_RELOADING:
                break;

        case W_RIGHT_RAISING:
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
