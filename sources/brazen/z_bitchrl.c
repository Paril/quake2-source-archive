#include "g_local.h"
#include "m_player.h"

void BitchWeaponFire(edict_t *ent)
{
	vec3_t start, forward, right, offset;
	int rocketSpeed;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 0, 4,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	rocketSpeed = 500 + (100 * skill->value);	// PGM rock & roll.... :)
	fire_rocket (ent, start, forward, 50, rocketSpeed, 70, 50);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("chick/chkatck2.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_RAILGUN);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO]--;
}

/*
=================
Think_BitchWeapon

=================
*/

void Think_BitchWeapon(edict_t *ent)
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
                        newFrame = 19;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 10;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			newFrame = 4;
			newState = W_FIRING;
			BitchWeaponFire(ent);
                }
                else if (bAltAttack && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			newFrame = 4;
			newState = W_FIRING;
			BitchWeaponFire(ent);
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 11;
                        newState = W_IDLE;
                }
                else
                        newFrame = 10;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 19;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 10;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 10;
                        Think_BitchWeapon(ent);
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
		ent->client->ps.gunframe = 10;
                Think_BitchWeapon(ent);
                return;
                break;

	case 9:
                ent->client->weaponstate = W_READY;
		ent->client->ps.gunframe = 10;
		break;

	case 10:
		break;

        case 14:
                if (random() < 0.8)
                	return;
		ent->client->ps.gunframe++;
		break;
                
        case 18:
                if (random() < 0.5)
                        newFrame = 11;
                else
                {
                        newFrame = 10;
                        newState = W_READY;
                }
                break;

        case 22:
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
