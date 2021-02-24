#include "g_local.h"
#include "m_player.h"

void InfWeaponFire(edict_t *ent)
{
	vec3_t		start, forward, right, offset;
	float	rotation;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	ent->client->machinegun_shots++;
	if (ent->client->machinegun_shots > 6)
		ent->client->machinegun_shots = 1;
	
	VectorSet(offset, 0, 7,  ent->viewheight-8);

	rotation = ent->client->machinegun_shots * 2 * M_PI / 6;
	offset[0] += -4 * sin(rotation);
	offset[2] += 4 * cos(rotation);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_bullet (ent, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex ("infantry/infatck1.wav"), 1, ATTN_NORM, 0);

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
Think_InfWeapon

=================
*/

void Think_InfWeapon(edict_t *ent)
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
                        newFrame = 16;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 6;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_FIRING;
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 4;
			newState = W_FIRING;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 7;
                        newState = W_IDLE;
                }
                else
                        newFrame = 6;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 16;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 6;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 6;
                        Think_InfWeapon(ent);
                        return;
                }
                break;

        case W_FIRING:
		if ((!bAttack) || (ent->client->ps.gunframe == 6))
        	{
                	ent->client->ps.gunframe = 6;
			if ((ammoRight < 1) && bAttack)
	                	return;
			ent->client->weaponstate = W_READY;
                	ent->last_fire = level.time;
                        Think_InfWeapon(ent);
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
		ent->client->ps.gunframe = 6;
                Think_InfWeapon(ent);
                return;
                break;

        case 4:
		if (ammoRight > 0)
		{
			InfWeaponFire(ent);
                	ent->client->ps.gunframe = 5;
		}
		else
			ent->client->ps.gunframe = 6;
		break;

        case 5:
		if (ammoRight > 0)
		{
			InfWeaponFire(ent);
                	ent->client->ps.gunframe = 4;
		}
		else
			ent->client->ps.gunframe = 6;
                break;

	case 6:
		break;

        case 10:
        case 14:
                if (random() < 0.8)
                	return;
		ent->client->ps.gunframe++;
		break;
                
        case 15:
                if (random() < 0.5)
                        newFrame = 7;
                else
                {
                        newFrame = 6;
                        newState = W_READY;
                }
                break;

        case 19:
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
