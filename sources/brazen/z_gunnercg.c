#include "g_local.h"
#include "m_player.h"

void GunnerWeaponFire(edict_t *ent)
{
	vec3_t		start, forward, right, offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_bullet (ent, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("gunner/gunatck2.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_RAILGUN);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	//ent->client->ps.gunframe++;
	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;
}

/*
=================
Think_GunnerWeapon

=================
*/

void Think_GunnerWeapon(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        int shots;
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 17;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 16;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_FIRING;
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_ALT_FIRING;
                }
                else
                        newFrame = 16;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 17;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 16;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 16;
                        Think_GunnerWeapon(ent);
                        return;
                }
                break;

        case W_FIRING:
		if ((ent->client->ps.gunframe > 6) && (ent->client->ps.gunframe < 12))
		{		
			if (ammoRight > 0)
			{
				GunnerWeaponFire(ent);
			}

			if (ent->client->ps.gunframe == 11 && (bAttack || bAltAttack))
			{
				ent->client->ps.gunframe = 7;
				if (!bAttack)
					ent->client->weaponstate = W_ALT_FIRING;
				return;
			}
		}
                break;

        case W_ALT_FIRING:
		if ((ent->client->ps.gunframe > 6) && (ent->client->ps.gunframe < 12))
		{		
			if (bAttack)
				ent->client->weaponstate = W_FIRING;

			if (ent->client->ps.gunframe == 11 && (bAttack || bAltAttack))
			{
				ent->client->ps.gunframe = 7;
				return;
			}
		}
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 3: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 16;
                Think_GunnerWeapon(ent);
                return;
                break;

        case 4:
        case 12:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("gunner/gunatck1.wav"), 1, ATTN_IDLE, 0);
		ent->client->ps.gunframe++;
		break;

        case 15: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 16;
		break;

	case 16:
		break;

        case 20:
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
