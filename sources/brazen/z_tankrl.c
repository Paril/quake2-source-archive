#include "g_local.h"
#include "m_player.h"

void TankWeaponFire(edict_t *ent, vec3_t offset)
{
	vec3_t start, forward, right;
	int rocketSpeed;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	rocketSpeed = 500 + (100 * skill->value);	// PGM rock & roll.... :)
	fire_rocket (ent, start, forward, 50, rocketSpeed, 70, 50);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("tank/tnkatck1.wav"), 1, ATTN_NORM, 0);

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
Think_TankWeapon

=================
*/

void Think_TankWeapon(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        vec3_t offset;
        int shots;
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 27;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 17;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if ((bAttack || bAltAttack) && (ammoRight > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			VectorSet(offset, 0, 9,  ent->viewheight-8);
			TankWeaponFire(ent, offset);
			ent->client->ps.gunframe = 4;
	                ent->client->weaponstate = W_FIRING;
	                return;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 18;
                        newState = W_IDLE;
                }
                else
                        newFrame = 17;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 27;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 17;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 17;
                        Think_TankWeapon(ent);
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
		ent->client->ps.gunframe = 17;
                Think_TankWeapon(ent);
                return;
                break;

	case 6:
		VectorSet(offset, 0, 6,  ent->viewheight-8);
		TankWeaponFire(ent, offset);
		ent->client->ps.gunframe++;
		break;

	case 8:
		VectorSet(offset, 0, 3,  ent->viewheight-8);
		TankWeaponFire(ent, offset);
		ent->client->ps.gunframe++;
		break;

	case 16:
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		break;

	case 17:
		break;

        case 21:
                if (random() < 0.8)
                	return;
		ent->client->ps.gunframe++;
		break;
                
        case 26:
                if (random() < 0.5)
                        newFrame = 18;
                else
                {
                        newFrame = 17;
                        newState = W_READY;
                }
                break;

        case 30:
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
