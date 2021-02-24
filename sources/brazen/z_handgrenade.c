#include "g_local.h"
#include "m_player.h"

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

int CountOffHandGrenades(edict_t *ent, int type)
{
        int count, i;

	count = 0;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == type)
        		count += ent->client->pers.item_quantities[i];
	}
	
	return count;
}

int CountHandGrenades(edict_t *ent)
{
        int count, i;

	count = 0;
	
	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == ent->client->pers.cstats[CSTAT_RIGHTHAND])
        		count += ent->client->pers.item_quantities[i];
	}
	
	if (ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] == 1)
		count++;
	
	return count;
}

void ThrowOffHandGrenade(edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int 	i;
	int	damage = 125;
	float	timer;
	int	speed;
	float	radius;

	radius = damage + 40;

	VectorSet(offset, 8, -8, ent->viewheight - 8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = 2.7 + random() * 0.5;
	//gi.dprintf("ent->client->v_angle[PITCH] = %f\n", ent->client->v_angle[PITCH]);

	if (ent->client->v_angle[PITCH] > 60)
		speed = 128;	// aiming down quite a lot
	else
		speed = GRENADE_MAXSPEED;
		
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, ent->client->pers.hgren_type);

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == ent->client->pers.hgren_type)
        	{
			ent->client->pers.item_quantities[i]--;
			if (ent->client->pers.item_quantities[i] < 1)
				RemoveItem(ent, i);
			break;
		}
	}

	if (ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

/*
=================
ThrowHandGrenade
=================
*/
void ThrowHandGrenade(edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int 	i;
	int	damage = 125;
	float	timer;
	int	speed;
	float	radius;

	radius = damage + 40;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	if (timer < 0.3)
		timer = 0.3;

	if (ent->client->v_angle[PITCH] > 60)
		speed = 128;	// aiming down quite a lot
	else
		speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
		
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE]);

	ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 0;

	for (i = BA_LEG_ARMOUR; i < BA_MAX ; i++)
        {
        	if (ent->client->pers.item_bodyareas[i] == ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMOTYPE])
        	{
			ent->client->pers.cstats[CSTAT_RIGHTHAND_AMMO] = 1;
			ent->client->pers.item_quantities[i]--;
			if (ent->client->pers.item_quantities[i] < 1)
				RemoveItem(ent, i);
			break;
		}
	}

	if (ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

/*
=================
Think_HandGrenade

Single Hand Grenade
=================
*/

void Think_HandGrenade(edict_t *ent)
{
        int newState = -1;
        int newFrame = -1;
        int ammo = CountHandGrenades(ent);
        qboolean bAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK);
        qboolean bAltAttack = ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ALT_ATTACK);
        
        switch(ent->client->weaponstate)
        {
        case W_READY:
                if (ent->client->newRightWeapon > -1)
                {
                        ent->client->ps.gunframe = 16;
                        ChangeRightWeapon(ent);
                        return;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 16;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack && (ammo > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = 0;
			newState = W_FIRING;
                }
                else if (bAltAttack && (ammo > 0))
                {
                        ent->client->latched_buttons &= ~BUTTON_ALT_ATTACK;
			ent->client->ps.gunframe = 0;
			newState = W_ALT_FIRING;
                }
                else if (level.time - ent->last_fire > IDLE_DELAY)
                {
                        newFrame = 17;
                        newState = W_IDLE;
                }
                else
                        newFrame = 16;
                break;

        case W_IDLE:
                if (ent->client->newRightWeapon > -1)
                {
                        ent->client->ps.gunframe = 16;
                        ChangeRightWeapon(ent);
                        return;
                }                
                else if (ent->client->newLeftWeapon > -1)
                {
                        ent->client->ps.gunframe = 16;
                        ChangeLeftWeapon(ent);
                        return;
                }
                else if (bAttack || bAltAttack)
                {
                        ent->client->weaponstate = W_READY;
			ent->client->ps.gunframe = 16;
                        Think_HandGrenade(ent);
                        return;
                }
                break;

        case W_RIGHT_RAISING:
		ent->client->ps.gunframe = 16;
		newState = W_READY;
                break;

        default:
                break;
        }

        switch(ent->client->ps.gunframe)
        {
        case 3:
                ent->client->ps.gunframe = 9;
		ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
        	break;

        case 10:
                if (bAttack || bAltAttack)
                        return;
                ent->client->ps.gunframe = 13;
                break;

        case 13:
                ThrowHandGrenade(ent);
                ent->client->ps.gunframe++;
                break;

        case 15:
        	if (ammo < 1)
		{        		
                        ent->client->pers.cstats[CSTAT_RIGHTHAND] = II_HANDS;
                        ent->client->pers.cstats[CSTAT_RIGHTHAND_FLAGS] = 0;
		        SetupItemModels(ent);
			AutoSwitchWeapon(ent, 0, 0);
                        return;
		}
		else
		{
                 	ent->last_fire = level.time;
                        ent->client->ps.gunframe++;
			newState = W_READY;
		}
                break;

        case 16:
        	break;

        case 48:
                if (random() < 0.5)
                        newFrame = 17;
                else
                {
                        newFrame = 16;
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
