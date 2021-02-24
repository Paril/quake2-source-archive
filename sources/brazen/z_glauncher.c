#include "g_local.h"
#include "m_player.h"

void GrenadelauncherFire (edict_t *ent, qboolean alt)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int i;
	int		damage = 120;
	float	radius;

	radius = damage + 40;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (alt)
		fire_cgrenade (ent, start, forward, damage, 900, 2.5, radius, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);
	else
		fire_grenade (ent, start, forward, damage, 900, 2.5, radius, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE])
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
Think_GrenadeLauncher

Chaingun - the ultimate spray n' pray weapon
=================
*/

void Think_GrenadeLauncher(edict_t *ent)
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
                        newFrame = 61;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 18;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 7;
			newState = W_FIRING;
			GrenadelauncherFire(ent, false);
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 7;
			newState = W_ALT_FIRING;
			GrenadelauncherFire(ent, true);
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 19;
                        newState = W_IDLE;
                }
                else
                        newFrame = 18;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 61;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 18;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 18;
                        Think_GrenadeLauncher(ent);
                        return;
                }
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 61;
                newState = W_RIGHT_RELOADING;
                break;

        case W_RIGHT_RELOADING:
        	if (ent->client->ps.gunframe == 65)
        	{
        	        gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotcock.wav"), 1, ATTN_NORM, 0);
			ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
			ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] = ent->client->newRightAmmoType;
			ent->client->newRightAmmoType = 0;
        		ent->client->ps.gunframe = 0;
        		return;
		}
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 6: // ready now, so think again straight away in case we want to fire fast
	case 17:
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 18;
                Think_GrenadeLauncher(ent);
                return;
                break;

	case 18:
		break;
                
        case 60:
                if (random() < 0.5)
                        newFrame = 19;
                else
                {
                        newFrame = 18;
                        newState = W_READY;
                }
                break;

        case 65:
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
