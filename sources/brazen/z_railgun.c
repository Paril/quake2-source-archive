#include "g_local.h"
#include "m_player.h"

void RailgunFire(edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	int i;

	damage = 150;
	kick = 250;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == II_SLUGS)
        	{
			ent->client->pers.item_quantities[i]--;
			if (ent->client->pers.item_quantities[i] < 1)
				RemoveItem(ent, i);
			break;
		}
	}
}

/*
=================
Think_Railgun

=================
*/

void Think_Railgun(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->ps.stats[STAT_RIGHT_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        int shots;
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 57;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 19;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_FIRING;
			RailgunFire(ent);
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_ALT_FIRING;
			RailgunFire(ent);
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 20;
                        newState = W_IDLE;
                }
                else
                        newFrame = 19;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 57;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 19;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 19;
                        Think_Railgun(ent);
                        return;
                }
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 3: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 19;
		ent->client->weapon_sound = gi.soundindex("weapons/rg_hum.wav");
                Think_Railgun(ent);
                return;
                break;

	case 18:
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 19;
                break;

	case 19:
		ent->client->weapon_sound = gi.soundindex("weapons/rg_hum.wav");
		break;
                
        case 56:
                if (random() < 0.5)
                        newFrame = 20;
                else
                {
                        newFrame = 19;
                        newState = W_READY;
                }
                break;

        case 61:
		ent->client->weapon_sound = 0;
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        default:
                if (newFrame == -1)
                        ent->client->ps.gunframe++;
                break;
        }

        if (newFrame != -1)
                ent->client->ps.gunframe = newFrame;

        if (newState != -1)
                ent->client->weaponstate = newState;
}
