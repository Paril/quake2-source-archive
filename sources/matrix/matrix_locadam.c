#include "g_local.h"

qboolean pointinback (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot < -0.3)
		return true;
	return false;
}
qboolean pointinfront (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot > 0.3)
		return true;
	return false;
}
qboolean pointabove (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	up;

	AngleVectors (self->s.angles, NULL, NULL, up);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, up);
	
	if (dot > 0.3)
		return true;
	return false;
}
qboolean pointbelow (edict_t *self, vec3_t point)
{
	vec3_t	vec;
	float	dot;
	vec3_t	up;

	AngleVectors (self->s.angles, NULL, NULL, up);
	VectorSubtract (point, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, up);
	
	if (dot < -0.3)
		return true;
	return false;
}

// checks if behind (not sides)
qboolean inback (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
	if (dot < -0.3)
		return true;
	return false;
}
int MatrixApplyLocationDamage (edict_t *targ, vec3_t point, int mod, int damage)
{
	vec3_t hitpos;
	
	VectorSubtract(point, targ->s.origin, hitpos);

	if (!( mod == MOD_BLASTER	   || mod == MOD_SHOTGUN  || mod == MOD_SSHOTGUN
		|| mod == MOD_HYPERBLASTER || mod == MOD_CHAINGUN || mod == MOD_MACHINEGUN
		|| mod == MOD_ROCKET	   || mod == MOD_GRENADE  || mod == MOD_HANDGRENADE 
		|| mod == MOD_RAILGUN	   || mod == MOD_KICK	  || mod == MOD_SWEEP
		|| mod == MOD_HOVER		   || mod == MOD_SNIPER	  || mod == MOD_AK_MK23	  
		|| mod == MOD_HOOK		  ||  mod == MOD_JAB	  ||  mod == MOD_UNKNOWN))
		return LOC_NULL;

	if (fabs((targ->s.origin[2] + targ->viewheight) - point[2]) <= 4)
	{
		damage *= 2;
		if (pointinfront(targ, point))
			return LOC_FACE;
		else
			return LOC_HEAD;
	}
	else if (((targ->s.origin[2] + targ->viewheight) - point[2]) < 20)// if (hitpos[2] > 0)
	{
		if (pointinfront(targ, point))
			return LOC_CHEST;
		else if (pointinback(targ, point))
			return LOC_BACK;
		else if (hitpos[1] > 0)
			return LOC_RIGHTARM;
		else
			return LOC_LEFTARM;
	}
	else
	{
		damage *= 0.8;
		if (hitpos[1] > 0)
			return LOC_RIGHTLEG;
		else
			return LOC_LEFTLEG;
	}
}