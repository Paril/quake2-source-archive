#include "g_local.h"

#define DEFAULT_AK47_HSPREAD	200	//narrower spread than machine gun
#define DEFAULT_AK47_VSPREAD	300

/*
======================================================================

MACHINEGUN AK-47
by Fireball

======================================================================
*/

void AK47_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = wf_game.weapon_damage[WEAPON_AK47];	// AK-47 is better than Q2's machinegun :)
	int			kick = 3;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}
// Cardinal
	/*
	if ((ent->client->pers.inventory[ent->client->ammo_index] - 1) % 30 == 0 && ent->client->pers.inventory[ent->client->ammo_index] > 1)
	{
		ent->client->weaponstate = WEAPON_MACHINEGUNREARMING;
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe = 6;
	}
	*/
// Cardinal
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// don't raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_AK47_HSPREAD, DEFAULT_AK47_VSPREAD, MOD_AK47);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (MZ_MACHINEGUN| is_silenced);
	gi.WriteByte (MZ_ETF_RIFLE| is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_AK47 (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, AK47_Fire);
}