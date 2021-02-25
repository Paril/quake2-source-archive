#include "g_local.h"
#include "m_player.h"
static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}
void weapon_teeth_fire (edict_t *ent)
{
	int damage,kick;
	vec3_t	offset, start;
	vec3_t	dir, right;
	vec3_t	v;

	
	ent->client->ps.gunframe++;

	if(ent->leftarm || ent->rightarm)
	{
		NoAmmoWeaponChange (ent);
		return;
	}

		damage = 50;
		kick = 400;
		VectorScale (dir, -2, ent->client->kick_origin);
		VectorSet(offset, 8, 8, ent->viewheight-16);
		P_ProjectSource (ent->client, ent->s.origin, offset, dir, right, start);
		kick_attack (ent, start, dir , damage, kick, MOD_TEETH);
	


	
}
void Weapon_Teeth (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 5, 52, 55, pause_frames, fire_frames, weapon_teeth_fire);
}
