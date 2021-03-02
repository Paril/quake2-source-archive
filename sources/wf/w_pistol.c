#include "g_local.h"

/*
======================================================================

PISTOL Gurza
by Fireball

======================================================================
*/


void Weapon_Pistol_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = wf_game.weapon_damage[WEAPON_PISTOL];
	int			kick = 2;
	vec3_t		offset;

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

	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/pistol.wav"), 1, ATTN_NORM, 0);

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

//   if (ent->client->machinegun_shots == 1)
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD / 2, DEFAULT_BULLET_VSPREAD / 2, MOD_PISTOL);

//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteShort (ent-g_edicts);
//	gi.WriteByte (0);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	ent->client->ps.gunframe++;

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_Pistol (edict_t *ent)
{
	static int	pause_frames[]	= {12, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 6, 15, 19, pause_frames, fire_frames, Weapon_Pistol_Fire);
}
