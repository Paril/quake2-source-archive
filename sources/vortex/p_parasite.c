#include "g_local.h"

#define PARASITE_MAXFRAMES	20
#define PARASITE_RANGE		128
#define PARASITE_DELAY		2
#define PARASITE_COST		50

void parasite_endattack (edict_t *ent)
{
	gi.sound (ent, CHAN_AUTO, gi.soundindex("parasite/paratck4.wav"), 1, ATTN_NORM, 0);
	ent->sucking = false;
	ent->parasite_frames = 0;
}

void ParasiteAttack (edict_t *ent)
{
	int		damage, kick;
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;

	if (debuginfo->value)
		gi.dprintf("%s just called ParasiteAttack()\n", ent->client->pers.userinfo);

	// terminate attack
	if (!G_EntIsAlive(ent) || (ent->parasite_frames > PARASITE_MAXFRAMES)
		|| que_typeexists(ent->curses, CURSE_FROZEN))
	{
		parasite_endattack(ent);
		return;
	}

	// calculate starting point
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	// do we already have a valid target?
	if (G_ValidTarget(ent, ent->parasite_target, true) 
		&& (entdist(ent, ent->parasite_target) <= PARASITE_RANGE)
		&& infov(ent, ent->parasite_target, 90))
	{
		VectorSubtract(ent->parasite_target->s.origin, start, forward);
		VectorNormalize(forward);
	}

	VectorMA(start, PARASITE_RANGE, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	// did we hit something?
	if (G_EntIsAlive(tr.ent) && !OnSameTeam(ent, tr.ent))
	{
		if (ent->parasite_frames == 0)
		{
			ent->client->pers.inventory[power_cube_index] -= PARASITE_COST;
			gi.sound (ent, CHAN_AUTO, gi.soundindex("parasite/paratck3.wav"), 1, ATTN_NORM, 0);
			ent->client->ability_delay = level.time + PARASITE_DELAY;
		}

		ent->parasite_target = tr.ent;
		damage = 2*ent->myskills.abilities[BLOOD_SUCKER].current_level;
		if (tr.ent->groundentity)
			kick = -100;
		else
			kick = -50;
		
		T_Damage(tr.ent, ent, ent, forward, tr.endpos, 
			tr.plane.normal, damage, kick, DAMAGE_NO_ABILITIES, MOD_PARASITE);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_PARASITE_ATTACK);
		gi.WriteShort(ent-g_edicts);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(ent->s.origin, MULTICAST_PVS);

		ent->parasite_frames++;
	}
	else if (ent->parasite_frames)
		parasite_endattack(ent);

}





