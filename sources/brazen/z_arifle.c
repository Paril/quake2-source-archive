#include "g_local.h"
#include "m_player.h"

qboolean CanReloadGLauncher(edict_t *ent)
{
	int i;

	if (ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO4)
		return false;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == II_FRAG_GRENADES)
			return true;
	}

	return false;
}

void ReloadGLauncher(edict_t *ent)
{
	int i;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == II_FRAG_GRENADES)
        	{
			ent->client->pers.item_quantities[i]--;

			if (!(ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO1))
				ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] |= SHF_EXTRA_AMMO1;
			else if (!(ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO2))
				ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] |= SHF_EXTRA_AMMO2;
			else if (!(ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO3))
				ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] |= SHF_EXTRA_AMMO3;
			else if (!(ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO4))
				ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] |= SHF_EXTRA_AMMO4;

			if (ent->client->pers.item_quantities[i] < 1)
				RemoveItem(ent, i);
			break;
		}
	}
}

void ARGLauncherFire(edict_t *ent)
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

	fire_cgrenade (ent, start, forward, damage, 900, 2.5, radius, II_FRAG_GRENADES);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO4)
		ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] &= ~SHF_EXTRA_AMMO4;
	else if (ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO3)
		ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] &= ~SHF_EXTRA_AMMO3;
	else if (ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO2)
		ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] &= ~SHF_EXTRA_AMMO2;
	else if (ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] & SHF_EXTRA_AMMO1)
		ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] &= ~SHF_EXTRA_AMMO1;
}


void FireAssaultRifle(edict_t *ent)
{
        vec3_t forward, right, offset, start;
	VectorSet(offset, 0, 2, ent->viewheight-2);
        ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        
	ent->client->weapon_halfspeed = true;
        fire_bullet (ent, start, forward, 13, 30, 100, 100, MOD_ARIFLE);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_HYPERBLASTER);
	//gi.WriteByte (MZ_MACHINEGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}
}

/*
=================
Think_AssaultRifle

Single Pistol
=================
*/

void Think_AssaultRifle(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
        int ammoGrenade = ent->client->ps.stats[STAT_LEFT_EXTRA_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 31;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 11;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ammoRight < 1)
                        {
                                if (CanRightReload(ent))
                                {
                                        newFrame = 35;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else
                                        newFrame = 11;
                        }
                        else
                        {
				ent->client->ps.gunframe = 6;
                                newState = W_FIRING;
                        }
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        if (ammoGrenade < 1)
                        {
				if (CanReloadGLauncher(ent))
                                {
                                        newFrame = 69;
                                        newState = W_LEFT_RELOADING;
                                }
                                else
                                        newFrame = 11;
                        }
                        else
                        {
				ent->client->ps.gunframe = 58;
				newState = W_ALT_FIRING;
                        }
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 12;
                        newState = W_IDLE;
                }
                else
                        newFrame = 11;
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 35;
                newState = W_RIGHT_RELOADING;
                break;

        case W_FIRING:
        	if ((!bAttack) || (ent->client->ps.gunframe == 11) || (ent->client->ps.gunframe == 7))
                {
                        ent->client->ps.gunframe = 11;
                        if ((ammoRight < 1) && bAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_AssaultRifle(ent);
                        return;
                }
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 31;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 11;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 11;
                        Think_AssaultRifle(ent);
                        return;
                }
                break;

        case W_ALT_FIRING:
		if (ent->client->ps.gunframe == 60)
                {
                	if (bAttack && ammoGrenade > 0)
                	{
				ARGLauncherFire(ent);
				ent->client->ps.gunframe = 61;
				return;
			}
			else if (!bAltAttack)
			{
                        	ent->client->weaponstate = W_READY;
				ent->client->ps.gunframe = 11;
			}
			else
				return;
                }

		if (ent->client->ps.gunframe == 68)
		{
			ent->client->ps.gunframe = 60;
			return;
		}
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 5: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 11;
                Think_AssaultRifle(ent);
                return;
                break;

        case 6:
		if (ammoRight > 0)
		{
			FireAssaultRifle(ent);
                	ent->client->ps.gunframe = 10;
		}
		else
			ent->client->ps.gunframe = 11;
		break;

        case 10:
		if (ammoRight > 0)
		{
			FireAssaultRifle(ent);
                	ent->client->ps.gunframe = 6;
		}
		else
			ent->client->ps.gunframe = 11;
                break;


        case 11: // do nothing... we're ready, not idle
                break;
                
        case 30:
                if (random() < 0.5)
                        newFrame = 12;
                else
                {
                        newFrame = 11;
                        newState = W_READY;
                }
                break;

        case 34:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        case 41:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/arifle_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 52:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/arifle_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 57:
                ent->client->weaponstate = W_READY;
		ent->client->ps.gunframe = 11;
                break;

        case 66:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotcock.wav"), 1, ATTN_NORM, 0);
                ent->client->ps.gunframe++;
                break;

        case 74:
		ReloadGLauncher(ent);
                ent->client->ps.gunframe++;
                break;

        case 75:
		if (bAltAttack && CanReloadGLauncher(ent))
		{
                        newFrame = 69;
			newState = W_LEFT_RELOADING;
                }
                else
                {
			newFrame = 11;
			newState = W_READY;
		}
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
