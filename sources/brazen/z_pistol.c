#include "g_local.h"
#include "m_player.h"

void FirePistol(edict_t *ent, qboolean left)
{
        vec3_t forward, right, offset, start;

        if (left)
        {
                VectorSet(offset, 0, -2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]--;
        }
        else
        {
                VectorSet(offset, 0, 2, ent->viewheight-2);
                ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;
        }

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        
        fire_bullet (ent, start, forward, 15, 30, 100, 100, (ent->client->pers.cstats[CSTAT_LEFTHAND]== II_PISTOL) ? MOD_TWIN_PISTOL : MOD_PISTOL);

        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        // GRIM 26/06/2001 3:25PM - FIX ME - silencer??
        //if ((ent->client->pers.specials & SI_SILENCER) && (!ent->client->pers.weapon2))
		//gi.WriteByte (MZ_HYPERBLASTER);
        //else
                gi.WriteByte (MZ_BLASTER);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
}

/*
=================
Think_Pistol

Single Pistol
=================
*/

void Think_Pistol(edict_t *ent)
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
                        newFrame = 58;
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
                                        newFrame = 62;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else
                                        newFrame = 8;
                        }
                        else
                        {
				ent->client->ps.gunframe = 5;
				//newFrame = 5;
                                newState = W_FIRING;
                                FirePistol(ent, false);
                        }
                }
                else if (bAltAttack && (ammoGrenade > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        newFrame = 75;
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
                        newFrame = 58;
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
                        Think_Pistol(ent);
                        return;
                }
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 62;
                newState = W_RIGHT_RELOADING;
                break;

        case W_FIRING:
                if (ent->client->ps.gunframe == 8)
                {
                	ent->client->ps.gunframe++;
                	return;
		}
		
                if (ent->client->ps.gunframe == 10)
                {
                        if ((ammoRight < 1) && bAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_Pistol(ent);
                        return;
                }
                break;

        case W_ALT_FIRING:
		if (ent->client->ps.gunframe >= 83)
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
        case 4: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 8;
                Think_Pistol(ent);
                return;
                break;

        case 6:
        case 7:
		if (ammoRight > 0)
			FirePistol(ent, false);
                ent->client->ps.gunframe++;
                break;

        case 8: // do nothing... we're ready, not idle
                break;
                
        case 57:
                if (random() < 0.5)
                        newFrame = 9;
                else
                {
                        newFrame = 8;
                        newState = W_READY;
                }
                break;

        case 61:
                if (ent->client->newRightWeapon > -1)
                        ChangeRightWeapon(ent);
                break;

        case 64:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;


        case 71:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 74:
                newState = W_READY;
                newFrame = 13;
                break;

        case 78:
                if (bAltAttack)
                        return;
                ent->client->ps.gunframe++;
                break;

        case 79:
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
Think_TwinPistol

Pistol in each hand
=================
*/

void Think_TwinPistol(edict_t *ent)
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
                        ent->client->ps.gunframe = 10;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 30;
                        newState = W_LEFT_DROPPING;
                }
                else if (bAttack && bAltAttack && (ammoLeft > 0) && (ammoRight > 0))
                {
			ent->client->ps.gunframe = 75;
        	        newState = W_BOTH_FIRING;
			FirePistol(ent, true);
                        FirePistol(ent, false);
			/*
                        if ((ent->client->ps.gunframe == 10) && (ammoLeft > 0))
                        {
                                newState = W_BOTH_FIRING;
                                ent->client->ps.gunframe = 53;
                                //newFrame = 61;
                                FirePistol(ent, true);
                        }
                        else if (ammoRight > 0)
                        {
                                newState = W_BOTH_FIRING;
                                ent->client->ps.gunframe = 7;
                                //newFrame = 9;
                                FirePistol(ent, false);
                        }
                        */
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        if (ammoRight < 1)
                        {
                                if (CanRightReload(ent))
                                {
                                        newFrame = 37;
                                        newState = W_RIGHT_RELOADING;
                                }
                                else
                                        newFrame = 10;
                        }
                        else
                        {
                                ent->client->ps.gunframe = 7;
                                //newFrame = 9;
                                newState = W_FIRING;
                                FirePistol(ent, false);
                        }
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
                        if (ammoLeft < 1)
                        {
                                if (CanLeftReload(ent))
                                {
                                        newFrame = 59;
                                        newState = W_LEFT_RELOADING;
                                }
                                else
                                        newFrame = 10;
                        }
                        else
                        {
                                ent->client->ps.gunframe = 53;
                                //newFrame = 61;
                                newState = W_ALT_FIRING;
                                FirePistol(ent, true);
                        }
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 11;
                        newState = W_IDLE;
                }
                else
                        newFrame = 10;
                break;

        case W_START_RIGHT_RELOAD:
		newFrame = 37;
                newState = W_RIGHT_RELOADING;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {       // no anim for change right, so just switch
                        ent->client->ps.gunframe = 10;
                        ChangeRightWeapon(ent);
                        return;
                }
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 30;
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
			ent->client->ps.gunframe = 10;
                        Think_TwinPistol(ent);
                        return;
                }
                break;

        case W_BOTH_FIRING:
        	/*
                switch(ent->client->ps.gunframe)
                {
                case 8:
                case 9:
                        if (bAttack && bAltAttack && (ammoLeft > 0))
                        {
                                ent->client->ps.gunframe = 53;
                                FirePistol(ent, true);
                                ent->client->ps.gunframe++;
                                ent->last_fire = level.time;
                                return;
                        }
                        else if (ent->client->ps.gunframe == 9)
                        {
                                ent->client->weaponstate = W_READY;
                                ent->client->ps.gunframe = 10;
                                Think_TwinPistol(ent);
                                return;
                        }
                        break;
                
                case 54:
                case 55:
                        if (bAttack && bAltAttack && (ammoRight > 0))
                        {
                                ent->client->ps.gunframe = 7;
                                ent->client->ps.gunframe++;
                                ent->last_fire = level.time;
                                FirePistol(ent, false);
                                return;
                        }
                        else if (ent->client->ps.gunframe == 55)
                        {
                                ent->client->weaponstate = W_READY;
                                ent->client->ps.gunframe = 10;
                                Think_TwinPistol(ent);
                                return;
                        }
                        break;
                default:
                        break;
                }
                */
                if (ent->client->ps.gunframe == 79)
                {
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_TwinPistol(ent);
                        return;
                }
                break;

        case W_FIRING:
                if (ent->client->ps.gunframe == 10)
                {
                	ent->client->ps.gunframe++;
        		return;
                }
                
                if ((ent->client->ps.gunframe == 11) && bAltAttack && (ammoLeft > 0))
                {
	                ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 53;
                	newState = W_ALT_FIRING;
			FirePistol(ent, true);
		}
		
                if (ent->client->ps.gunframe == 12)
                {
                        if ((ammoRight < 1) && bAttack && !bAltAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_TwinPistol(ent);
                        return;
                }
                break;

        case W_ALT_FIRING:
                if ((ent->client->ps.gunframe == 57) && bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 7;
                        newState = W_FIRING;
                        FirePistol(ent, false);
		}

                if (ent->client->ps.gunframe == 58)
                {
                        if ((ammoLeft < 1) && bAltAttack && !bAttack)
                                return;
                        ent->client->weaponstate = W_READY;
                        ent->last_fire = level.time;
                        Think_TwinPistol(ent);
                        return;
                }
                break;

        case W_RIGHT_RELOADING:
        case W_LEFT_RELOADING:
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 6: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;
                ent->client->ps.gunframe = 10;
                Think_TwinPistol(ent);
                return;
                break;

        case 8:
        case 9:
		if (ammoRight > 0)
			FirePistol(ent, false);
                ent->client->ps.gunframe++;
                break;

        case 54:
        case 55:
		if (ammoLeft > 0)
			FirePistol(ent, true);
                ent->client->ps.gunframe++;
                break;

        case 76:
        case 77:
		if (ammoRight > 0)
			FirePistol(ent, false);
		if (ammoLeft > 0)
			FirePistol(ent, true);
                ent->client->ps.gunframe++;
                break;

        case 10: // do nothing... we're ready, not idle
                break;
                
        case 29:
                if (random() < 0.5)
                        newFrame = 11;
                else
                {
                        newFrame = 10;
                        newState = W_READY;
                }
                break;

        case 36:
                ChangeLeftWeapon(ent);
                break;

        case 40:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 46:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_RIGHTHAND);
                ent->client->ps.gunframe++;
                break;

        case 63:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipout.wav"), 1, ATTN_NORM, 0);
		DropClip(ent, ent->client->pers.cstats[CSTAT_LEFTHAND_AMMOTYPE], ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO]);
		ent->client->pers.cstats[CSTAT_LEFTHAND_AMMO] = 0;
                ent->client->ps.gunframe++;
                break;

        case 69:
                gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol_clipin.wav"), 1, ATTN_NORM, 0);
                ReloadHand(ent, CSTAT_LEFTHAND);
                ent->client->ps.gunframe++;
                break;

        case 52:
        case 74:
                newState = W_READY;
                newFrame = 12;
                break;

	case 79:
                ent->client->ps.gunframe = 10;
		ent->client->weaponstate = W_READY;
                ent->last_fire = level.time;
                if ((bAttack && (ammoRight > 0)) || (bAltAttack && (ammoLeft> 0)))
                {
	                Think_TwinPistol(ent);
			return;
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
