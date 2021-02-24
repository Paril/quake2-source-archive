#include "g_local.h"
#include "m_player.h"

void FireSubMach(edict_t *ent, qboolean left)
{
        vec3_t forward, right, offset, start;
        int type = 0;

        if (left)
        {
                VectorSet(offset, 0, -2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]--;
                if (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] == II_HV_SUBMACH_CLIP)
                	type = 1;
        }
        else
        {
                VectorSet(offset, 0, 2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;
                if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] == II_HV_SUBMACH_CLIP)
                	type = 1;
        }

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

        if (type == 1)
        {
        	fire_bullet (ent, start, forward, 30, 30, 100, 100, MOD_HV_SUBMACH);
		ent->client->weapon_halfspeed = true;
	}
	else
	{
        	fire_bullet (ent, start, forward, 7, 30, 100, 100, MOD_MACHINEGUN);
	}
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((type == 1) ? MZ_BLASTER : MZ_MACHINEGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

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
Think_SubMach

Single SubMach
=================
*/

void Think_SubMach(edict_t *ent)
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
                        newFrame = 86;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 6;
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
                                        newFrame = 90;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else
                                        newFrame = 6;
                        }
                        else
                        {
				ent->client->ps.gunframe = 5;
                                newState = W_FIRING;
                        }
                }
                else if (bAltAttack && (ammoGrenade > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        newFrame = 102;
			newState = W_ALT_FIRING;
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
                        newFrame = 86;
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
                        Think_SubMach(ent);
                        return;
                }
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 90;
                newState = W_RIGHT_RELOADING;
                break;

        case W_FIRING:
        	if ((!bAttack) || (ent->client->ps.gunframe == 6))
                {
                        ent->client->ps.gunframe = 6;
                        if ((ammoRight < 1) && bAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_SubMach(ent);
                        return;
                }
                break;

        case W_ALT_FIRING:
		if (ent->client->ps.gunframe >= 111)
                {
                        newState = W_READY;
			ent->client->ps.gunframe = 8;
                }
                break;

        case W_RIGHT_RELOADING:
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 3: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 6;
                Think_SubMach(ent);
                return;
                break;

        case 4:
		if (ammoRight > 0)
		{
			FireSubMach(ent, false);
                	ent->client->ps.gunframe = 5;
		}
		else
			ent->client->ps.gunframe = 6;
		break;

        case 5:
		if (ammoRight > 0)
		{
			FireSubMach(ent, false);
                	ent->client->ps.gunframe = 4;
		}
		else
			ent->client->ps.gunframe = 6;
                break;

        case 6: // do nothing... we're ready, not idle
                break;
                
        case 85:
                if (random() < 0.5)
                        newFrame = 7;
                else
                {
                        newFrame = 6;
                        newState = W_READY;
                }
                break;

        case 89:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        case 93:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;


        case 97:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 101:
                newState = W_READY;
                newFrame = 6;
                break;

        case 105:
                if (bAltAttack)
                        return;
                ent->client->ps.gunframe++;
                break;

        case 106:
                ThrowOffHandGrenade(ent);
                ent->client->ps.gunframe++;
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

/*
=================
Think_TwinSubMach

SubMach in each hand
=================
*/

void Think_TwinSubMach(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
        int ammoLeft = ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);

        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {       // no anim for change right, so just switch
                        ent->client->ps.gunframe = 8;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 48;
                        newState = W_LEFT_DROPPING;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ammoRight < 1)
                        {
                                if (CanRightReload(ent))
                                {
                                        newFrame = 53;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else if (ammoLeft > 0)
                                {
					ent->client->ps.gunframe = 70;
	                                newState = W_FIRING;
				}
                        }
                        else if (ammoLeft > 0)
			{
				ent->client->ps.gunframe = 88;
	                	newState = W_FIRING;
			}
                        else
                        {
                                ent->client->ps.gunframe = 7;
                                newState = W_FIRING;
                        }
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        if (ammoLeft < 1)
                        {
                                if (CanLeftReload(ent))
                                {
                                        newFrame = 71;
                                        newState = W_LEFT_RELOADING;
                                }
                                else if (ammoRight > 0)
                                {
					ent->client->ps.gunframe = 7;
	                                newState = W_FIRING;
				}
                        }
                        else if (ammoRight > 0)
			{
				ent->client->ps.gunframe = 88;
	                	newState = W_FIRING;
			}
                        else
                        {
                                ent->client->ps.gunframe = 70;
                                newState = W_FIRING;
                        }
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 9;
                        newState = W_IDLE;
                }
                else
                        newFrame = 8;
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 37;
                newState = W_RIGHT_RELOADING;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {       // no anim for change right, so just switch
                        ent->client->ps.gunframe = 8;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 48;
                        newState = W_LEFT_DROPPING;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
                        /*
                        if (ammoRight <= ammoLeft)
                                ent->client->ps.gunframe = 53;
                        else
                                ent->client->ps.gunframe = 7;
			*/
			ent->client->ps.gunframe = 8;
                        Think_TwinSubMach(ent);
                        return;
                }
                break;

	case W_FIRING:
		if ((ammoRight < 1) && (ammoLeft < 1))
		{
			ent->client->ps.gunframe = 8;

			if (bAttack || bAltAttack)
				return;

	                newState = W_READY;
		}
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 5: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;
                ent->client->ps.gunframe = 8;
                Think_TwinSubMach(ent);
                return;
                break;

        case 6:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if (ammoRight > 0)
		{
			FireSubMach(ent, false);
	                ent->client->ps.gunframe = 7;
                }
                break;

        case 7:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if (ammoRight > 0)
		{
			FireSubMach(ent, false);
                	ent->client->ps.gunframe = 6;
                }
                break;

        case 8: // do nothing... we're ready, not idle
                break;

        case 47:
                if (random() < 0.5)
                        newFrame = 9;
                else
                {
                        newFrame = 8;
                        newState = W_READY;
                }
                break;

        case 52:
                ChangeLeftWeapon(ent);
		ent->client->ps.gunframe = 6;
                break;

        case 58:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 63:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 77:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 80:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/sub_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_LEFTHAND);
                ent->client->ps.gunframe++;
                break;


        case 69:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if (ammoLeft > 0)
		{
			FireSubMach(ent, true);
                	ent->client->ps.gunframe = 70;
                }
                break;

        case 70:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if (ammoLeft > 0)
		{
			FireSubMach(ent, true);
                	ent->client->ps.gunframe = 69;
                }
                break;

        case 87:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if ((ammoRight > 0) && (ammoLeft > 0))
		{
			FireSubMach(ent, false);
			FireSubMach(ent, true);
			ent->client->ps.gunframe = 88;
		}
		else if (ammoRight > 0)
		{
			FireSubMach(ent, false);
			ent->client->ps.gunframe = 7;
		}
		else if (ammoLeft > 0)
		{
			FireSubMach(ent, true);
			ent->client->ps.gunframe = 70;
		}
                break;

        case 88:
        	if (!bAttack && !bAltAttack)
		{
	                ent->client->ps.gunframe = 8;
                        newState = W_READY;
                }
		else if ((ammoRight > 0) && (ammoLeft > 0))
		{
			FireSubMach(ent, false);
			FireSubMach(ent, true);
			ent->client->ps.gunframe = 87;
		}
		else if (ammoRight > 0)
		{
			FireSubMach(ent, false);
			ent->client->ps.gunframe = 6;
		}
		else if (ammoLeft > 0)
		{
			FireSubMach(ent, true);
			ent->client->ps.gunframe = 69;
		}
                break;

        case 68:
        case 86:
                newState = W_READY;
                newFrame = 8;
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
