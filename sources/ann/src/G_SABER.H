/*
 * saber variable definitions
 */
#define SABER_NORMAL_DAMAGE		120
#define SABER_DEATHMATCH_DAMAGE	150
#define SABER_KICK				 75
#define SABER_RANGE				 48

/*
=============
fire_saber

edict_t *self - entity producing it, yourself
vec3_t start - The place you are
vec3_t aimdir - Where you are looking at in this case
int damage - the damage the saber inflicts
int kick - how much you want that bitch to be thrown back
=============
*/

void fire_saber ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	trace_t tr; //detect whats in front of you up to range "vec3_t end"
	char temp[40];

	vec3_t end;

	// Figure out what we hit, if anything:

	VectorMA (start, SABER_RANGE, aimdir, end);  //calculates the range vector

	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
						// figuers out what in front of the player up till "end"

	sprintf(temp,"weapons/lightsaber/swing%i.wav", (rand()%3)+1);
	gi.sound (self, CHAN_AUTO, gi.soundindex(temp) , 1, ATTN_NORM, 0);

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				//This tells us to damage the thing that in our path...hehe
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY, MOD_SABER);
				sprintf(temp,"weapons/lightsaber/enthit%i.wav", (rand()%4)+1);
				gi.sound (self, CHAN_AUTO, gi.soundindex(temp) , 1, ATTN_NORM, 0);
			}
			else
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_SPARKS);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);

				gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/lightsaber/wallhit.wav") , 1, ATTN_NORM, 0);
			}
		}
	}
	return;
}

void saber_attack (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t  forward, right;
	vec3_t  start;
	vec3_t  offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_saber (ent, start, forward, damage, SABER_KICK );
}

void Weapon_Saber_Fire (edict_t *ent)
{
	int damage;
	if (deathmatch->value)
		damage = SABER_DEATHMATCH_DAMAGE;
	else
		damage = SABER_NORMAL_DAMAGE;
	saber_attack (ent, vec3_origin, damage);
	ent->client->ps.gunframe++;
}

void Weapon_Saber (edict_t *ent)
{
	static int      pause_frames[]  = {19, 32, 0};
	static int      fire_frames[]   = {5, 0};

	Weapon_Generic (ent, 3, 8, 58, 61, pause_frames, fire_frames, Weapon_Saber_Fire);
}

