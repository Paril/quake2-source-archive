#include "g_local.h"
#include "m_player.h"

//void fire_tracer (edict_t *self, vec3_t start, vec3_t dir);

void FireChaingun (edict_t *ent, int shots)
{
	vec3_t	start, forward, right, up, offset;
	int	damage, i;
	float	r, u;
	int	kick = 2;

	damage = 8;

	ent->client->weapon_halfspeed = true;

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

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	if (ent->client->ps.stats[STAT_RIGHT_AMMO] < shots)
		shots = ent->client->ps.stats[STAT_RIGHT_AMMO];

	/*
	AngleVectors (ent->client->v_angle, forward, right, up);
	VectorSet(offset, 16, 7, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GUNSHOT);
	gi.WritePosition (start);
	gi.WriteDir (forward);
	gi.multicast (start, MULTICAST_PVS);
	*/

	/*ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots == 3)
	{
		AngleVectors (ent->client->v_angle, forward, right, up);
		VectorSet(offset, 0, 7, ent->viewheight - 8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		fire_tracer (ent, start, forward);
		ent->client->machinegun_shots = 0;
	}*/		

	for (i = 0; i < shots; i++)
	{	// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom() * 4;
		u = crandom() * 4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	PlayerNoise(ent, start, PNOISE_WEAPON);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_CHAINGUN1 + shots - 1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == II_CHAINGUN_PACK)
        	{
			ent->client->pers.item_quantities[i]--;
			if (ent->client->pers.item_quantities[i] < 1)
				RemoveItem(ent, i);
			break;
		}
	}
}

/*
=================
Think_Chaingun

Chaingun - the ultimate spray n' pray weapon
=================
*/

void Think_Chaingun(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammoRight = ent->client->ps.stats[STAT_RIGHT_AMMO];
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        int shots;
        
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
                        ent->client->ps.gunframe = 33;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 5;
			newState = W_FIRING;
                }
                else if (bAltAttack)
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 5;
			newState = W_ALT_FIRING;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 33;
                        newState = W_IDLE;
                }
                else
                        newFrame = 32;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        newFrame = 62;
                        newState = W_RIGHT_DROPPING;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        newFrame = 33;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 32;
                        Think_Chaingun(ent);
                        return;
                }
                break;

        case W_FIRING:
		if (ent->client->ps.gunframe < 22)
		{		
			ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");

			if (ammoRight > 0)
			{
				if (ent->client->ps.gunframe <= 9)
					shots = 1;
				else if (ent->client->ps.gunframe <= 14)
				{	
					if (bAttack)
						shots = 2;
					else
						shots = 1;
				}
				else
					shots = 3;

				if (shots > 0)
					FireChaingun(ent, shots);
			}

			if (ent->client->ps.gunframe == 21 && (bAttack || bAltAttack))
			{
				ent->client->ps.gunframe = 15;
				if (!bAttack)
					ent->client->weaponstate = W_ALT_FIRING;
				return;
			}
		}
                break;

        case W_ALT_FIRING:
		if (ent->client->ps.gunframe < 22)
		{		
			ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
			if (bAttack)
				ent->client->weaponstate = W_FIRING;
	
			if (ent->client->ps.gunframe == 21 && (bAttack || bAltAttack))
			{
				ent->client->ps.gunframe = 15;
				return;
			}
		}
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 4: // ready now, so think again straight away in case we want to fire fast
                ent->client->weaponstate = W_READY;// do nothing... we're ready, not idle
		ent->client->ps.gunframe = 32;
                Think_Chaingun(ent);
                return;
                break;

        case 5:
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);
		ent->client->ps.gunframe++;
		break;

	case 22:
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
                ent->client->ps.gunframe++;
		break;

	case 31:
		newState = W_READY;
                ent->client->ps.gunframe++;
		break;

	case 32:
		break;
                
        case 61:
                if (random() < 0.5)
                        newFrame = 33;
                else
                {
                        newFrame = 32;
                        newState = W_READY;
                }
                break;

        case 64:
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
