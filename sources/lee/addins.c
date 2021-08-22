/*
ADD in functions for QUAKE 2, made by me!
=========================================
*/
float patch_ver = 1.01;
#include "g_local.h"

void zoomin(edict_t *ent)
{

	//Check for reasons not to zoom
	if (ent->health <0)
		return;

	//change FOV
	if (ent->client->ps.fov > 40)
		ent->client->ps.fov-=4;	

}

void zoomout(edict_t *ent)
{

	//Check for reasons not to zoom
	if (ent->health <0)
		return;

	//change FOV
	if (ent->client->ps.fov < 90)
		ent->client->ps.fov+=4;	
}

void copy_dead_player(edict_t *ent)
{
	ent->s.frame = ent->owner->s.frame;
	ent->nextthink = level.time + 0.1;
}

void make_dead_player(edict_t *ent)
{
	edict_t	*bolt;

	bolt = G_Spawn();
	VectorCopy (ent->s.origin, bolt->s.origin);
	VectorCopy (ent->s.angles, bolt->s.angles);

	bolt->movetype = MOVETYPE_NONE;
	bolt->clipmask = MASK_PLAYERSOLID;
	bolt->solid = SOLID_BBOX;

	
	bolt->s.frame = ent->s.frame;
	bolt->s.modelindex = ent->s.modelindex;
	bolt->s.modelindex2 = ent->s.modelindex2;
		
	bolt->owner = ent;

//	bolt->think = copy_dead_player;
//	bolt->nextthink = level.time + 0.1;

	bolt->classname = "dead_player";	
	gi.linkentity (bolt);
}

void draw_line(edict_t *ent, vec3_t spot1, vec3_t spot2)
{	
	//draw line
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (spot1);
	gi.WritePosition (spot2);
	gi.multicast (spot1, MULTICAST_PVS);

}

void draw_path(edict_t *ent)
{
	edict_t	*bolt;
	edict_t	*flag;
	trace_t	tr; 
	vec3_t  distance,angles;
	vec3_t  temp, temp2, right,forward,move,up, startpos, endpos;
	int dist, turn, test;

	test=0;

	if (!ctf->value)
		return;

	
	//find flag
	flag = G_Find (NULL, FOFS(classname), "item_flag_team1");

	if (flag == NULL)
		return;
	
	VectorCopy(ent->s.origin,startpos);
	VectorCopy(flag->s.origin,endpos);
	tr = gi.trace (startpos, NULL, NULL, endpos, ent, MASK_SOLID);

	//.dprintf ("'%s' %i\n",tr.surface->name,tr.surface );
	//if (tr.surface->name)
	if (*tr.surface->name)// == 7553152)
	{
		draw_line(ent,ent->s.origin,tr.endpos );
		return;
	}
		VectorCopy(ent->s.origin,move);
		//VectorCopy(ent->client->v_angle, angles);
		VectorSubtract(angles, ent->client->v_angle, flag->s.angles);
		angles[0]=-5;
		//angles[1]-=30;
		
test_path:
	AngleVectors (angles, forward, NULL, NULL);
		

		//VectorScale (right, 90, temp);
		VectorScale (forward, 200, temp2);		
		//VectorAdd (temp, move, move);
		VectorAdd (temp2, move, move);

	VectorCopy(ent->s.origin,startpos);
	VectorCopy(move,endpos);
	draw_line(ent,ent->s.origin,endpos );

	tr = gi.trace (startpos, NULL, NULL, endpos, ent, MASK_SOLID);

	//.dprintf ("'%s' %i\n",tr.surface->name,tr.surface );
	//if (tr.surface->name)
//	if (tr.surface->name == 7553152)
//	{
//		draw_line(ent,ent->s.origin,tr.endpos );
//		return;
//	}
	//
	//else 
	//{
		VectorSubtract ( ent->s.origin, tr.endpos, distance);
		dist = VectorLength(distance);	
		
		if (dist > 100)
			VectorCopy(tr.endpos,move);
		
		
		//
		//	angles[1]+=90;
//		else
			angles[1]+=45;

		
		//VectorScale (right, 90, temp);
//		AngleVectors (angles, forward, NULL, up);
//		VectorScale (forward, 1000, temp2);		
		//VectorAdd (temp, move, move);
//		VectorAdd (temp2, move, move);

//		draw_line(ent,startpos,tr.endpos);
		
//		VectorCopy(tr.endpos,startpos);
//		VectorCopy(move,endpos);

		test++;
		VectorCopy(tr.endpos, temp);
		tr = gi.trace (tr.endpos, NULL, NULL, flag->s.origin, ent, MASK_SOLID);
		
		if (*tr.surface->name)// == 7553152)
		{
			draw_line(ent,temp,flag->s.origin);
			gi.dprintf ("done..\n");
			return;
		}
		else if (test < 5)
			goto test_path;
	//}


}

//information about patch and help commands
void Help_on_patch (edict_t *ent)
{
	gi.cprintf (ent, PRINT_CHAT,"\n\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\n");
	gi.cprintf (ent, PRINT_CHAT,"\v Help on Lee's Mod V%1.2f \v\n",patch_ver);
	gi.cprintf (ent, PRINT_CHAT,"\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\n\n");
	gi.cprintf (ent, PRINT_MEDIUM,"Commands:\n\n");
	gi.cprintf (ent, PRINT_MEDIUM,"        sv bot_num # : Set number of bots to #\n");
	gi.cprintf (ent, PRINT_MEDIUM,"        bot_teleport : Teleport one bot near you(coop only)\n");
	gi.cprintf (ent, PRINT_MEDIUM,"        torch        : toggle torch light when in darkmatch mode\n");
	gi.cprintf (ent, PRINT_MEDIUM,"\nCreated by Lee Allen\nlee@planetquake.com\nwww.planetquake.com/lee\nCopyright 2000\n");
	gi.cprintf (ent, PRINT_CHAT,"\n\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\n\n");
}

#include <time.h>

void the_time (edict_t *ent)
{
	char tmpbuf[128];
	time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	strftime (tmpbuf,sizeof(tmpbuf),"%a, %d %b %Y %T %z",timeinfo);
  
	gi.cprintf (ent, PRINT_MEDIUM, "\nThe time in the real world is:\n%s\n\n",tmpbuf);
}