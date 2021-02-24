#include "g_local.h"
#include "m_player.h"

void StroggWeaponFire(edict_t *ent)
{
	vec3_t		start, forward, right, offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	switch(ent->client->pers.cstats[CSTAT_RIGHTHAND])
	{
	case II_STROGG_BLASTER:
		fire_blaster (ent, start, forward, 5, 600, EF_BLASTER, false);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("soldier/solatck2.wav"), 1, ATTN_NORM, 0);
		break;

	case II_STROGG_SHOTGUN:
		fire_shotgun (ent, start, forward, 2, 1, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("soldier/solatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case II_STROGG_SUBMACH:
		fire_bullet (ent, start, forward, 2, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("soldier/solatck3.wav"), 1, ATTN_NORM, 0);
		break;
	}

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
Think_StroggSoldierWeapon

=================
*/

void Think_StroggSoldierWeapon(edict_t *ent)
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
                        newFrame = 29;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 9;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.cstats[CSTAT_RIGHTHAND] != II_STROGG_SUBMACH)
			{
				newFrame = 4;
				StroggWeaponFire(ent);
			}
			else
				ent->client->ps.gunframe = 4;
			newState = W_FIRING;
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			if (ent->client->pers.cstats[CSTAT_RIGHTHAND] != II_STROGG_SUBMACH)
			{
				newFrame = 4;
				StroggWeaponFire(ent);
			}
			else
				ent->client->ps.gunframe = 4;
			newState = W_FIRING;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 10;
                        newState = W_IDLE;
                }
                else
                        newFrame = 9;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 29;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 9;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 9;
                        Think_StroggSoldierWeapon(ent);
                        return;
                }
                break;

        case W_FIRING:
		if (ent->client->pers.cstats[CSTAT_RIGHTHAND] == II_STROGG_SUBMACH)
		{
	        	if ((!bAttack) || (ent->client->ps.gunframe == 9))
        	        {
                	        ent->client->ps.gunframe = 9;
                        	if ((ammoRight < 1) && bAttack)
	                                return;
        	                ent->client->weaponstate = W_READY;
                	        ent->last_fire = level.time;
                        	Think_StroggSoldierWeapon(ent);
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
		ent->client->ps.gunframe = 9;
                Think_StroggSoldierWeapon(ent);
                return;
                break;

        case 4:
		if (ent->client->pers.cstats[CSTAT_RIGHTHAND] == II_STROGG_SUBMACH)
		{
			if (ammoRight > 0)
			{
				StroggWeaponFire(ent);
                		ent->client->ps.gunframe = 8;
			}
			else
				ent->client->ps.gunframe = 9;
		}
		else
			ent->client->ps.gunframe++;
		break;

        case 5:
		if (ent->client->pers.cstats[CSTAT_RIGHTHAND] == II_STROGG_BLASTER)
			ent->client->ps.gunframe = 7;
		else
			ent->client->ps.gunframe++;
		break;

        case 8:
		if (ent->client->pers.cstats[CSTAT_RIGHTHAND] == II_STROGG_SUBMACH)
		{
			if (ammoRight > 0)
			{
				StroggWeaponFire(ent);
                		ent->client->ps.gunframe = 4;
			}
			else
			{
				ent->client->ps.gunframe = 9;
		                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
			}
		}
		else
		{
			ent->client->ps.gunframe = 9;
		        ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		}
                break;

	case 9:
		break;

        case 14:
        case 21:
        case 26:
                if (random() < 0.8)
                	return;
		ent->client->ps.gunframe++;
		break;
                
        case 28:
                if (random() < 0.5)
                        newFrame = 10;
                else
                {
                        newFrame = 9;
                        newState = W_READY;
                }
                break;

        case 32:
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
