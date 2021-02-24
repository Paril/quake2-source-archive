#include "g_local.h"
#include "m_player.h"

void MedicWeaponFire(edict_t *ent, int type)
{
	vec3_t		start, forward, right, offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_blaster (ent, start, forward, 2, 1000, type, type == EF_BLASTER ? false : true);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("medic/medatck1.wav"), 1, ATTN_NORM, 0);

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
Think_MedicWeapon

=================
*/

void Think_MedicWeapon(edict_t *ent)
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
                        newFrame = 22;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 20;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			newFrame = 4;
			newState = W_FIRING;
			MedicWeaponFire(ent, EF_BLASTER);
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			newFrame = 10;
			newState = W_ALT_FIRING;
                }
                else
                        newFrame = 20;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 22;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 20;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 20;
                        Think_MedicWeapon(ent);
                        return;
                }
                break;

        case W_FIRING:
		if (((ent->client->ps.gunframe == 14) || (ent->client->ps.gunframe == 15)) && (!bAttack))
			ent->client->ps.gunframe = 13;

		if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
		{
			if (!bAttack)
			{
                        	ent->client->weaponstate = W_READY;
				ent->client->ps.gunframe = 20;
				return;
			}
		}

		if (ent->client->ps.gunframe == 13)
		{
			if (!bAttack)
			{
				ent->client->ps.gunframe = 16;
				return;
			}
		}
                break;

	case W_ALT_FIRING:
		if (((ent->client->ps.gunframe == 14) || (ent->client->ps.gunframe == 15)) && (!bAttack))
			ent->client->ps.gunframe = 13;

		if (ent->client->ps.gunframe == 13)
		{
			if (bAttack)
				ent->client->ps.gunframe = 14;
			else
			{
				if (!bAltAttack)
					ent->client->ps.gunframe = 16;
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
		ent->client->ps.gunframe = 20;
                Think_MedicWeapon(ent);
                return;
                break;

        case 7:
		MedicWeaponFire(ent, EF_BLASTER);
                ent->client->ps.gunframe++;
		break;
		
        case 14:
		if (ammoRight > 0)
		{
			MedicWeaponFire(ent, EF_HYPERBLASTER);
                	ent->client->ps.gunframe = 15;
		}
		else
			ent->client->ps.gunframe = 16;
		break;

        case 15:
		if (ammoRight > 0)
		{
			MedicWeaponFire(ent, EF_HYPERBLASTER);
                	ent->client->ps.gunframe = 14;
		}
		else
			ent->client->ps.gunframe = 16;
		break;

        case 19:
		ent->client->ps.gunframe = 20;
		ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
                break;

	case 20:
	case 21:
		break;

        case 25:
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
