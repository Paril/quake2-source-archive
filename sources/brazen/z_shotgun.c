#include "g_local.h"
#include "m_player.h"

void FireShotgun(edict_t *ent, qboolean left)
{
        vec3_t forward, right, offset, start;
        int type = 0;

        if (left)
        {
                VectorSet(offset, 0, -2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]--;
                if (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] == II_EXP_SHOTGUN_CLIP)
                	type = 1;
                else if (ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE] == II_SOLID_SHOTGUN_CLIP)
                	type = 2;
        }
        else
        {
                VectorSet(offset, 0, 2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;
                if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] == II_EXP_SHOTGUN_CLIP)
                	type = 1;
                else if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE] == II_SOLID_SHOTGUN_CLIP)
                	type = 2;
        }

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        
	if (type == 1)
	        fire_bullet (ent, start, forward, 80, 30, 100, 100, MOD_EXP_SHOTGUN);
	else if (type == 2)
	        fire_bullet (ent, start, forward, 80, 30, 100, 100, MOD_SOLID_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, 6, 12, 1000, 500, 15, MOD_SHOTGUN);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
        // GRIM 26/06/2001 3:25PM - FIX ME - silencer??
        //if ((ent->client->pers.specials & SI_SILENCER) && (!ent->client->pers.weapon2))
		//gi.WriteByte (MZ_HYPERBLASTER);
        //else
		gi.WriteByte (MZ_SSHOTGUN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

/*
=================
Think_Shotgun

Single Shotgun
=================
*/

void Think_Shotgun(edict_t *ent)
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
                        newFrame = 62;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 8;
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
                                        newFrame = 65;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else
                                        newFrame = 8;
                        }
                        else
                        {
				newFrame = 4;
                                newState = W_FIRING;
                                FireShotgun(ent, false);
                        }
                }
                else if (bAltAttack && (ammoGrenade > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        newFrame = 82;
			newState = W_ALT_FIRING;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 9;
                        newState = W_IDLE;
                }
                else
                        newFrame = 8;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 62;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 8;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 8;
                        Think_Shotgun(ent);
                        return;
                }
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 65;
                newState = W_RIGHT_RELOADING;
                break;

        case W_FIRING:
                if (ent->client->ps.gunframe == 8)
                {
                        if ((ammoRight < 1) && bAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_Shotgun(ent);
                        return;
                }
                break;

        case W_ALT_FIRING:
		if (ent->client->ps.gunframe >= 91)
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
		ent->client->ps.gunframe = 8;
                Think_Shotgun(ent);
                return;
                break;

        case 8: // do nothing... we're ready, not idle
                break;
                
        case 61:
                if (random() < 0.5)
                        newFrame = 9;
                else
                {
                        newFrame = 8;
                        newState = W_READY;
                }
                break;

        case 64:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        case 68:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 74:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipin.wav"), 1, ATTN_NORM, 0);
                ent->client->ps.gunframe++;
                break;

        case 77:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotcock.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 81:
                newState = W_READY;
                newFrame = 8;
                break;

        case 85:
                if (bAltAttack)
                        return;
                ent->client->ps.gunframe++;
                break;

        case 86:
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
Think_TwinShotgun

Shotgun in each hand
=================
*/

void Think_TwinShotgun(edict_t *ent)
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
                        ent->client->ps.gunframe = 13;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 31;
                        newState = W_LEFT_DROPPING;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ammoRight < 1)
                        {
				ent->client->ps.gunframe = 13;
                                if (CanRightReload(ent))
                                {
                                        newFrame = 37;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else if (ammoLeft > 0)
                                {
					ent->client->ps.gunframe = 61;
	                                newState = W_ALT_FIRING;
				}
                        }
                        else if (ammoLeft > 0)
			{
				ent->client->ps.gunframe = 91;
	                	newState = W_FIRING;
			}
                        else
                        {
                                ent->client->ps.gunframe = 7;
                                newState = W_ALT_FIRING;
                        }
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        if (ammoLeft < 1)
                        {
				ent->client->ps.gunframe = 13;
                                if (CanLeftReload(ent))
                                {
                                        newFrame = 67;
                                        newState = W_LEFT_RELOADING;
                                }
                                else if (ammoRight > 0)
                                {
					ent->client->ps.gunframe = 7;
	                                newState = W_ALT_FIRING;
				}
                        }
                        else
                        {
                        	if (ammoRight > ammoLeft)
					ent->client->ps.gunframe = 7;
				else
	                                ent->client->ps.gunframe = 61;
				newState = W_ALT_FIRING;
                        }
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 14;
                        newState = W_IDLE;
                }
                else
                        newFrame = 13;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {       // no anim for change right, so just switch
                        ent->client->ps.gunframe = 13;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 31;
                        newState = W_LEFT_DROPPING;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
                        if (ammoRight <= ammoLeft)
                                ent->client->ps.gunframe = 61;
                        else
                                ent->client->ps.gunframe = 7;
                        Think_TwinShotgun(ent);
                        return;
                }
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 37;
                newState = W_RIGHT_RELOADING;
                break;

        case W_FIRING:
                if ((ent->client->ps.gunframe == 97) || (ent->client->ps.gunframe == 13))
                {
                	if (!bAttack)
                	{
				newFrame = 13;
		                newState = W_READY;
			}
                        else if (ammoRight < 1)
                        {
                        	if (ammoLeft > 0)
					ent->client->ps.gunframe = 61;
				else
				{
					ent->client->ps.gunframe = 13;
					if (bAttack)
						return;
		                        ent->client->weaponstate = W_READY;
			                Think_TwinShotgun(ent);
                		        return;
				}
			}
			else if (ammoLeft < 1)
			{
				ent->client->ps.gunframe = 7;
				newState = W_ALT_FIRING;
			}
			else
				ent->client->ps.gunframe = 91;
                }
                break;

        case W_ALT_FIRING:
                switch(ent->client->ps.gunframe)
                {
                case 11:
                case 12:
                	if (!bAltAttack)
                	{
                		if (ent->client->ps.gunframe == 12)
                		{
                	                newState = W_READY;
					newFrame = 13;
				}
			}
			else if ((ammoLeft < 1) && (ammoRight < 1))
			{
                		if (ent->client->ps.gunframe == 12)
                		{
					ent->client->ps.gunframe = 13;
					if (bAltAttack)
						return;
			                ent->client->weaponstate = W_READY;
			                Think_TwinShotgun(ent);
			                return;
				}
			}
			else if (ammoLeft > 0)
				ent->client->ps.gunframe = 61;
			else if ((ammoRight > 0) && (ent->client->ps.gunframe == 12))
				ent->client->ps.gunframe = 7;
                        break;
		
		case 13:
			if (bAltAttack)
				return;
			ent->client->weaponstate = W_READY;
			Think_TwinShotgun(ent);
			return;
			break;                

                case 65:
                case 66:
                	if (!bAltAttack)
                	{
                		if (ent->client->ps.gunframe == 66)
                		{
                	                newState = W_READY;
					newFrame = 13;
				}
			}
			else if ((ammoLeft < 1) && (ammoRight < 1))
			{
                		if (ent->client->ps.gunframe == 66)
                		{
					ent->client->ps.gunframe = 13;
					if (bAltAttack)
						return;
			                ent->client->weaponstate = W_READY;
			                Think_TwinShotgun(ent);
			                return;
				}
			}
			else if (ammoRight > 0)
				ent->client->ps.gunframe = 7;
			else if ((ammoLeft > 0) && (ent->client->ps.gunframe == 66))
				ent->client->ps.gunframe = 61;
                        break;

                default:
                        break;
                }
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 6: // ready now, so think again straight away in case we want to fire fast
        case 12:
        case 66:
                ent->client->weaponstate = W_READY;
                ent->client->ps.gunframe = 13;
                Think_TwinShotgun(ent);
                return;
                break;

        case 7:
		FireShotgun(ent, false);
		ent->client->ps.gunframe++;
		break;		

        case 13: // do nothing... we're ready, not idle
                break;
                
        case 30:
                if (random() < 0.5)
                        newFrame = 14;
                else
                {
                        newFrame = 13;
                        newState = W_READY;
                }
                break;

        case 36:
                ChangeLeftWeapon(ent);
		ent->client->ps.gunframe = 8;
                break;

        case 44:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 50:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipin.wav"), 1, ATTN_NORM, 0);
                ent->client->ps.gunframe++;
                break;

        case 53:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotcock.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 61:
		FireShotgun(ent, true);
		ent->client->ps.gunframe++;
		break;		

        case 74:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 80:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgun_clipin.wav"), 1, ATTN_NORM, 0);
                ent->client->ps.gunframe++;
                break;

        case 83:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotcock.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_LEFTHAND);
                ent->client->ps.gunframe++;
                break;

        case 60:
        case 90:
                newState = W_READY;
                newFrame = 13;
                break;

        case 91:
		FireShotgun(ent, false);
		FireShotgun(ent, true);
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
