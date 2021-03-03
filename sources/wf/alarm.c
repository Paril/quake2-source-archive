#include "g_local.h"
//void Cmd_WFPlayTeam (edict_t *self, char *wavename);

void alarm_remove(edict_t *self)
{
	if (self == NULL)
	{
		return;
	}
	if (self->owner && self->owner->client)
	{
//		--self->owner->client->pers.active_special[ITEM_SPECIAL_ALARMS];
		self->owner->alarm1 = NULL;
	}
	G_FreeEdict (self);
}

void alarm_die (edict_t *self, edict_t * inflictor, edict_t * attacker, int damage, vec3_t point)
{
	self->takedamage=DAMAGE_NO;
	//T_RadiusDamage (self, self->owner, 20, NULL, 10,0);
	// BANG !
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	G_FreeEdict (self);
//	--self->owner->client->pers.active_special[ITEM_SPECIAL_ALARMS];
}

void alarmthink(edict_t *ent)
{
	edict_t *blip;

	if (level.time > ent->delay)	//don't exploce on contact
	{
		alarm_remove (ent);
        return;
	}

	blip = NULL;
	while ((blip = findradius (blip, ent->s.origin, 256)) != NULL)
	{
		if (!blip->client)
			continue;
		// Don't sound alarm for team mates
		if (blip->wf_team == ent->wf_team)
			continue;
		// Don't sound alarm for dead enemies
		if (blip->health <= 0)
			continue;
		// Don't sound alarm for folks we can't see
		if (!visible(ent, blip))
			continue;
		// Don't sound alarm for spies 95% of the time
		if ((blip->disguised) && (random() < 0.95))
			continue;
		// Sound alarm for everyone else
		break;

	}

	if (blip != NULL)
	{
		if ( ent->wf_team == CTF_TEAM1)
		{
			// play Team 1's sound
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("alarm.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			// play Team 2's sound
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("ctf/tech1.wav"), 1, ATTN_NORM, 0);
		}
		// let the player know their alarm was triggered
		safe_cprintf (ent->owner, PRINT_HIGH, "Alarm triggered by %s\n", blip->client->pers.netname);
		// don't check for another second because we are playing the sound for one second
		ent->nextthink = level.time + 1.0;
		//If this is an enemy spy, tell the team!
		//			if (ent->owner->client->player_special & SPECIAL_DISGUISE)
		//			{
		//				Cmd_WFPlayTeam(ent->owner, "radio/d_spy.wav");
		//			}

	}
	else
	{
		// check next frmae because we are idle
		ent->nextthink = level.time + 0.1;
	}
}

void place_alarm (int number,edict_t *ent)
{

	vec3_t		forward,
				wallp;

	trace_t		tr;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	// cells for laser ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 25)
	{
 		safe_cprintf(ent, PRINT_HIGH, "Not enough cells for an Alarm.\n");
		return;
	}

	//Are there too many alarms now?
/*	if (ent->client->pers.active_special[ITEM_SPECIAL_ALARMS] >= MAX_SPECIAL_ALARMS)
	{
 		safe_cprintf(ent, PRINT_HIGH, "You can only have %d active Alarms.\n",MAX_SPECIAL_ALARMS );
		return;
	}
*/

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;

	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
	 	safe_cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 25;

//	++ent->client->pers.active_special[ITEM_SPECIAL_ALARMS];


	if (number == 1)
	{
		if (ent->alarm1)
		{
			alarm_remove (ent->alarm1);
			safe_cprintf(ent, PRINT_HIGH, "Alarm off.\n");
			return;
		}
		safe_cprintf(ent, PRINT_HIGH, "Alarm on.\n");
		ent->alarm1 = G_Spawn();
		VectorClear (ent->alarm1->mins);
		VectorClear (ent->alarm1->maxs);
		VectorCopy (tr.endpos, ent->alarm1->s.origin);
		vectoangles(tr.plane.normal,ent->alarm1 -> s.angles);
		ent->alarm1 -> movetype		= MOVETYPE_NONE;
		ent->alarm1 -> clipmask		= MASK_SHOT;
		//grenade -> solid		= SOLID_NOT;
		ent->alarm1->solid = SOLID_BBOX;
		VectorSet(ent->alarm1->mins, -3, -3, 0);
		VectorSet(ent->alarm1->maxs, 3, 3, 6);
//		ent->alarm1->takedamage=DAMAGE_YES;
		ent->alarm1->takedamage=DAMAGE_NO;
	    ent->alarm1->mass = 2;
		ent->alarm1 -> s.modelindex	= gi.modelindex (GRNORMAL_MODEL);
		ent->alarm1 -> owner		= ent;
//		ent->alarm1->die = alarm_die;
		ent->alarm1 -> nextthink	= level.time + 0.2;
		ent->alarm1 -> think		= alarmthink;
		ent->alarm1->classname = "Alarm";
		ent->alarm1->health= 60;
		ent->alarm1->max_health =60;
		ent->alarm1->wf_team = ent->wf_team;
		ent->alarm1->delay = level.time + 120.0;
		gi.linkentity (ent->alarm1);
	}
	else if (number == 2)
	{
		if (ent->alarm2)
		{
			G_FreeEdict(ent->alarm2);
			ent->alarm2 = NULL;
			gi.bprintf (PRINT_HIGH, "Alarm 2 off.\n");
			return;
		}
		gi.bprintf (PRINT_HIGH, "Alarm 2 on.\n");
		ent->alarm2 = G_Spawn();
		VectorClear (ent->alarm2->mins);
		VectorClear (ent->alarm2->maxs);
		VectorCopy (tr.endpos, ent->alarm2->s.origin);
		vectoangles(tr.plane.normal,ent->alarm2 -> s.angles);
		ent->alarm2 -> movetype		= MOVETYPE_NONE;
		ent->alarm2 -> clipmask		= MASK_SHOT;
		//grenade -> solid		= SOLID_NOT;
		ent->alarm2->solid = SOLID_BBOX;
		VectorSet(ent->alarm2->mins, -3, -3, 0);
		VectorSet(ent->alarm2->maxs, 3, 3, 6);
		ent->alarm2->takedamage=DAMAGE_YES;
	    ent->alarm2->mass = 2;
		ent->alarm2 -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
		ent->alarm2 -> owner		= ent;
		ent->alarm2->die = alarm_die;
		ent->alarm2 -> nextthink	= level.time + 0.1;
		ent->alarm2 -> think		= alarmthink;
		ent->alarm2->health= 60;
		ent->alarm2->max_health =60;
		ent->alarm2->classname = "Alarm";
		ent->alarm2->wf_team = ent->wf_team;
		gi.linkentity (ent->alarm2);
	}
	else if (number == 3)
	{
		if (ent->alarm3)
		{
			G_FreeEdict(ent->alarm3);
			ent->alarm3 = NULL;
			gi.bprintf (PRINT_HIGH, "Alarm 3 off.\n");
			return;
		}
		gi.bprintf (PRINT_HIGH, "Alarm 3 on.\n");
		ent->alarm1 = G_Spawn();
		VectorClear (ent->alarm3->mins);
		VectorClear (ent->alarm3->maxs);
		VectorCopy (tr.endpos, ent->alarm3->s.origin);
		vectoangles(tr.plane.normal,ent->alarm3 -> s.angles);
		ent->alarm3 -> movetype		= MOVETYPE_NONE;
		ent->alarm3 -> clipmask		= MASK_SHOT;
		//grenade -> solid		= SOLID_NOT;
		ent->alarm3->solid = SOLID_BBOX;
		VectorSet(ent->alarm3->mins, -3, -3, 0);
		VectorSet(ent->alarm3->maxs, 3, 3, 6);
		ent->alarm3->takedamage=DAMAGE_YES;
	    ent->alarm3->mass = 2;
		ent->alarm3->die = alarm_die;
		ent->alarm3 -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
		ent->alarm3 -> owner		= ent;
		ent->alarm3 -> nextthink	= level.time + 0.1;
		ent->alarm3 -> think		= alarmthink;
		ent->alarm3->health= 60;
		ent->alarm3->max_health =60;
		ent->alarm3->classname = "Alarm";
		ent->alarm3->wf_team = ent->wf_team;
		gi.linkentity (ent->alarm3);
	}
}
void cmd_Alarm(edict_t *ent)
{
	char    *string;

	string = gi.args();

	if (!ent->client) return;

	//argument = "on", "off"
	if (Q_stricmp ( string, "on") == 0)
	{
		if (!ent->alarm1)
		{
			alarm_remove(ent->alarm1);
		}
	}
	else if (Q_stricmp ( string, "off") == 0)
	{
		if (ent->alarm1)
		{
			alarm_remove(ent->alarm1);
		}

	}
	else
	{
		if (ent->alarm1)
		{
			alarm_remove(ent->alarm1);
		}
		else
		{
			place_alarm(1, ent);
		}

	}
}
