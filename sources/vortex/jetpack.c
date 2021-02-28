#include "g_local.h"

#define JETPACK_DRAIN	1 //every x frames drain JETPACK_AMMO
#define JETPACK_AMMO	2 // Less cube cost.

void ApplyThrust (edict_t *ent)
{
	int talentLevel, cost = JETPACK_AMMO;
    vec3_t forward, right;
    vec3_t pack_pos, jet_vector;

	if(ent->myskills.abilities[JETPACK].disable && level.time > pregame_time->value)
		return;

	//Talent: Flight
	if ((talentLevel = getTalentLevel(ent, TALENT_FLIGHT)) > 0)
	{
		int num;
		
		num = 0.4 * talentLevel;
		if (num < 1)
			num = 1;
		cost -= num;
	}

	//4.0 better jetpack check.
	if ((level.time > pregame_time->value) && !trading->value)  // allow jetpack in pregame and trading
		if (!G_CanUseAbilities (ent, ent->myskills.abilities[JETPACK].current_level, cost) )
			return;
	//can't use abilities (spawning sentry gun/drone/etc...)
	if (ent->holdtime > level.time)
		return;
	//4.07 can't use jetpack while being hurt
	if (ent->lasthurt+DAMAGE_ESCAPE_DELAY > level.time)
		return;
	//amnesia disables jetpack
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this ability while carrying the flag!\n");
		return;
	}

	if (ent->client->snipertime >= level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't use jetpack while trying to snipe!\n");
		return;
	}

	if (ent->client->pers.inventory[power_cube_index] >= cost || level.time < pregame_time->value) // pregame.
	{
		ent->client->thrustdrain ++;
		if (ent->client->thrustdrain == JETPACK_DRAIN)
		{
			if (level.time > pregame_time->value) // not pregame
				ent->client->pers.inventory[power_cube_index] -= cost;
			ent->client->thrustdrain = 0;
		}
	}
	else
	{
		ent->client->thrusting=0;
		return;
	}

	if (ent->velocity[2] < 350)
	{
		if (ent->groundentity)
			ent->velocity[2] = 150;
		ent->velocity[2] += 150;
	}

    AngleVectors(ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, -7, pack_pos);
    VectorAdd (pack_pos, ent->s.origin, pack_pos);
    pack_pos[2] += (ent->viewheight);

    VectorScale (forward, -50, jet_vector);

    if (ent->client->next_thrust_sound < level.time)
    {
		// wow this check is stupid.
		/*if (ent->client) */
            gi.sound (ent, CHAN_BODY, gi.soundindex("weapons/rockfly.wav"), 1, ATTN_NORM, 0);
            ent->client->next_thrust_sound=level.time+1.0;
    }

	ent->lastsound = level.framenum;
}
