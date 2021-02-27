#include "g_local.h"
#include "scanner.h"

void ClearScanner(gclient_t *client)
{
	client->pers.scanner_active = 0;
}

void Toggle_Scanner (edict_t *ent)
{
	if (!ent->client || (ent->health <= 0))
		return;

	if (!V_CanUseAbilities(ent, SCANNER, 0, true))
		return;

	// toggle low on/off bit (and clear scores/inventory display if required)
	if ((ent->client->pers.scanner_active ^= 1) & 1)
	{
		ent -> client -> showinventory	= 0;
		ent -> client -> showscores		= 0;
	}
	else
	{
		ent->client->pers.scanner_active = 0;
		return;
	}

	// set "just changed" bit
	ent->client->pers.scanner_active |= 2;
}

void ShowAllyInfo (edict_t *ent)
{
	int		i, hp, armor;
	int		x=0, y=40;
	char	string[1400];
	char	stats[64], *tag;
	edict_t *e;

//	gi.dprintf("ShowAllyInfo()\n");

	memset(string, 0, sizeof(string));
	memset(stats, 0, sizeof(stats));

	for (i = 0; i < game.maxclients ; i++)
	{
		e = g_edicts + 1 + i;

		if (!e->inuse || !IsAlly(ent, e) || ent == e)
			continue;
	//	if (e != ent)
	//		continue;
		
		// set output
		Com_sprintf (stats, sizeof(stats),"xl %i yv %i string \"%s\" ", x, y, e->client->pers.netname);
		// don't overflow the buffer
		SAFE_STRCAT(string, stats, 1400);

		y += 8;
		
		hp = 5 * (int)((100 * ((float)e->health / e->max_health)) / 5);
		if (hp > 100)
			hp = 100;
		if (hp < 0)
			hp = 0;
		tag = va("status/bh%d", hp);

		// set output
		Com_sprintf (stats, sizeof(stats),"xl %i yv %i picn %s ", x, y, tag);

		// don't overflow the buffer
		SAFE_STRCAT(string, stats, 1400);

		y += 5;

		armor = 5 * (int)((100 * ((float)e->client->pers.inventory[body_armor_index] / MAX_ARMOR(e))) / 5);
		if (armor > 100)
			armor = 100;
		if (armor < 0)
			armor = 0;
		tag = va("status/ba%d", armor);

		// set output
		Com_sprintf (stats, sizeof(stats),"xl %i yv %i picn %s ", x, y, tag);

		// don't overflow the buffer
		SAFE_STRCAT(string, stats, 1400);

		y += 8;
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);

//	gi.dprintf("%s:%s\n", ent->client->pers.netname, string);
}

void ShowScanner (edict_t *ent, char *layout)
{
	edict_t	*e = NULL;
	char	stats[64], *tag;

	// update complete
	ent->client->pers.scanner_active &= ~2;

	// Main scanner graphic draw
	Com_sprintf (stats, sizeof(stats),"xl 0 yv 40 picn %s ", PIC_SCANNER_TAG);
	SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);

	while ((e = findradius(e, ent->s.origin, SCANNER_RANGE)) != NULL)
	{
		int		sx, sy;
		vec3_t	v, dp;
		vec3_t	normal = {0,0,-1};
		float dist, heightDelta;

		// sanity check
		if (!e || (e && !e->inuse))
			continue;
		// don't target self
		if (e == ent)
			continue;
		// target can't be hurt, is dead, or non-solid
		if (!e->takedamage || (e->health < 1) || (e->deadflag == DEAD_DEAD) || (e->solid == SOLID_NOT))
			continue;
		// target is cloaked or hidden
		if (e->svflags & SVF_NOCLIENT)
			continue;
		// ignore spawn points
		if (e->mtype == INVASION_PLAYERSPAWN || e->mtype == CTF_PLAYERSPAWN)
			continue;

		// target is invalid
		if ((level.framenum & 8) && ((e->flags & FL_CHATPROTECT) || (e->flags & FL_GODMODE) || (e->client && e->client->respawn_time>level.time) 
			|| (e->client && e->client->invincible_framenum>level.framenum) || (e->monsterinfo.inv_framenum>level.framenum)))
			tag = PIC_QUADDOT_TAG; // blue dot
		// target is on our team
		else if (OnSameTeam(ent, e))
			tag = PIC_DOT_TAG; // green dot 
		// target is enemy
		else
			tag = PIC_INVDOT_TAG; // red dot
		
		// calculate vector to enemy
		VectorSubtract(ent->s.origin, e->s.origin, v);

		// calculate height differential
		heightDelta = ent->absmin[2] - e->absmin[2];//v[2];

		// remove height component
		v[2] = 0;
		// calculate distance to target
		dist = VectorLength(v);

		VectorNormalize(v);

		// rotate round player view angle (yaw)
		RotatePointAroundVector(dp, normal, v, ent->s.angles[YAW]);

		// scale to fit scanner range (80 = pixel range of scanner)
		// scanner is 160x160 pixels, so 80 is the radius
		VectorScale(dp,dist*80/SCANNER_RANGE,dp);

		// calc screen (x,y) (2 = half dot width)
		sx = (80 + dp[1]) - 2;
		sy = (120 + dp[0]) - 2;

		// set output
		Com_sprintf (stats, sizeof(stats),"xl %i yv %i picn %s ", sx, sy, tag);

		// don't overflow the buffer
		SAFE_STRCAT(layout, stats, LAYOUT_MAX_LENGTH);

		// clear stats
		*stats = 0;

		// set up/down arrow
		if (heightDelta + 18 < 0)
			Com_sprintf (stats, sizeof(stats),"yv %i picn %s ", sy - 5, PIC_UP_TAG);
		else if (heightDelta - 18 > 0)
			Com_sprintf (stats, sizeof(stats),"yv %i picn %s ", sy + 5, PIC_DOWN_TAG);

		// any up/down ?
		if (*stats)
			SAFE_STRCAT(layout, stats, LAYOUT_MAX_LENGTH);
	}
}
