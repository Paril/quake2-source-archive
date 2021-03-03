#include "g_local.h"
void T_Radius2Damage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int kickback, int mod);

//ent = the remote camera
void remote_remove(edict_t *ent)
{
	if (!ent) return;

	ent->takedamage = DAMAGE_NO;

	//Should it blow up?
	if (ent->dmg)
	{
		T_Radius2Damage(ent, ent->owner, ent->dmg, NULL, 200, 30, MOD_CAMERA);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

	}

	if ((ent->owner) && (ent->owner->client))
	{
		ent->owner->client->remotetoggle = 0;
		ent->owner->remotecam = NULL;
	}

	//Remove camera
	G_FreeEdict (ent);
}

void remote_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

if (wfdebug) gi.dprintf("remote_die\n");
//	T_RadiusDamage (self, self->owner, 20, NULL, 10,0);

	// BANG !
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = remote_remove;
}

void remotecam_think(edict_t *ent)
{
	if(ent->owner->client->remotetoggle)
	{
		/* Remove cell requirements for now - GAR
		if (ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 1)
		{
 			safe_cprintf(ent->owner, PRINT_HIGH, "Not enough cells for the Remote Camera to stay\n");

			T_RadiusDamage (ent, ent->owner, 10, NULL, 10,0);

			// BANG !
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition(ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			remote_remove(ent);
			return;
		}
		*/
	}
	/* Remove cell requirements for now - GAR
	if(ent->owner->client->remotetoggle)
		ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 1;
		*/
	ent->nextthink = level.time + 0.5;
}

void place_remotecam (edict_t *ent)
{
	
	vec3_t		forward,
				wallp, offset;

	trace_t		tr;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	//If there already is a camera, remove it
	if (ent->remotecam) 
	{  
		//If damage is set to zero, just remove it
		if (ent->remotecam->dmg == 0)
		{
			remote_remove(ent->remotecam);
			safe_cprintf (ent, PRINT_HIGH, "Camera removed.\n");
			return;
		}

		//Otherwise, detonate it
		if (ent->remotecam->delay > level.time)
		{
			safe_cprintf (ent, PRINT_HIGH, "You can't blow up camera for another %d seconds.\n",
				(int)(ent->remotecam->delay - level.time)); 
			return;
		}
		safe_cprintf (ent, PRINT_HIGH, "Remote cam will detonate in 5 seconds.\n"); 
		ent->remotecam->dmg = 400;
		ent->remotecam->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
		ent->remotecam->think = remote_remove;
		ent->remotecam->nextthink = level.time + 5;
		//remote_remove(ent->remotecam);
		return; 
	}

	// cells for camera
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 25)
	{
 		safe_cprintf(ent, PRINT_HIGH, "You need 25 cells to place camera\n");
		return;
	}

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
	{
		if (tr.surface->flags & SURF_SKY)
			return;
	}

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 25;

	if (ent->remotecam) 
	{  
		safe_cprintf (ent, PRINT_HIGH, "Remote cam off.\n"); 
		remote_remove(ent->remotecam);
		return; 
	}

	safe_cprintf (ent, PRINT_HIGH, "Remote cam on.\n");

	ent->remotecam = G_Spawn();
	VectorClear (ent->remotecam->mins);
	VectorClear (ent->remotecam->maxs);
	VectorCopy (tr.endpos, ent->remotecam->s.origin);
	vectoangles(tr.plane.normal,ent->remotecam -> s.angles);
	ent->remotecam -> movetype		= MOVETYPE_NONE;
	ent->remotecam -> clipmask		= MASK_SHOT;
	//grenade -> solid		= SOLID_NOT;
	ent->remotecam->solid = SOLID_BBOX;
	VectorSet(ent->remotecam->mins, -3, -3, 0);
	VectorSet(ent->remotecam->maxs, 3, 3, 6);
	ent->remotecam->classname="camera";
	ent->remotecam->takedamage=DAMAGE_YES;
	ent->remotecam -> s.modelindex	= gi.modelindex ("models/objects/camera/tris.md2");
	ent->remotecam -> owner = ent;	
	ent->remotecam->think = remotecam_think;
	ent->remotecam->nextthink = level.time + 0.5;
	ent->remotecam->die = remote_die;
	ent->remotecam->health= 60;
	ent->remotecam->max_health = 60;
	ent->remotecam->mass = 2;
	ent->remotecam->delay = level.time + 10;		//can't blow up for this many seconds
	ent->remotecam->dmg = 20;
	offset[0]=forward[0]*-10;
	offset[1]=forward[1]*-10;
	offset[2]=forward[2]*-10;
	VectorAdd(offset,tr.endpos,offset);
	VectorCopy(offset,ent->remotecam->camposition);
	gi.linkentity (ent->remotecam);

/*db	if(ent->client->remotetoggle)
	{
		ent->client->oldplayer = G_Spawn(); 
		ent->client->oldplayer->s.frame = ent->s.frame;
		VectorCopy (ent->s.origin, ent->client->oldplayer->s.origin);
		VectorCopy (ent->velocity, ent->client->oldplayer->velocity);
		VectorCopy (ent->s.angles, ent->client->oldplayer->s.angles);
		ent->client->oldplayer->s.modelindex = ent->s.modelindex;
		ent->client->oldplayer->s.modelindex2 = ent->s.modelindex2;
		gi.linkentity (ent->client->oldplayer);
	}*/
}

//Remote Camera Commands
void cmd_CameraPlace(edict_t *ent)
{
	place_remotecam (ent);
}

void cmd_CameraToggle(edict_t *ent)
{
	if (ent->remotecam == NULL)
	{
		safe_cprintf (ent, PRINT_HIGH, "Remote camera does not exist!\n"); 
		return;
	}

	if (ent->client->remotetoggle)
	{
		ent->client->remotetoggle = 0;
//db		G_FreeEdict(ent->client->oldplayer);
	}
	else
	{
		ent->client->remotetoggle =1;
/*db		ent->client->oldplayer = G_Spawn(); 
		ent->client->oldplayer->s.frame = ent->s.frame;
		VectorCopy (ent->s.origin, ent->client->oldplayer->s.origin);
		VectorCopy (ent->velocity, ent->client->oldplayer->velocity);
		VectorCopy (ent->s.angles, ent->client->oldplayer->s.angles);
		ent->client->oldplayer->s.modelindex = ent->s.modelindex;
		ent->client->oldplayer->s.modelindex2 = ent->s.modelindex2;
		gi.linkentity (ent->client->oldplayer);*/
	}
}

// Command line handling for camera
void cmd_Camera(edict_t *ent)
{
	char    *string;
	int 	time = 0;

	string = gi.args();

	if (!ent->client) return;

	//argument = "build", "detonate", "remove", "toggle"
	if (Q_stricmp ( string, "build") == 0)
	{
		if (ent->remotecam) 
			safe_cprintf(ent, PRINT_HIGH, "You already have an active camera\n");
		else
			place_remotecam (ent);
	}
	else if (Q_stricmp ( string, "toggle") == 0)
	{
		if (!ent->remotecam) 
			safe_cprintf(ent, PRINT_HIGH, "You don't have an active camera!\n");
		else
			cmd_CameraToggle(ent);
	}
	else if (Q_stricmp ( string, "detonate") == 0)
	{
		if (!ent->remotecam) 
			safe_cprintf(ent, PRINT_HIGH, "You don't have an active camera!\n");
		else
			place_remotecam (ent);
	}
	else if (Q_stricmp ( string, "remove") == 0)
	{
		if (!ent->remotecam) 
			safe_cprintf(ent, PRINT_HIGH, "You don't have an active camera!\n");
		else
		{
			ent->remotecam->dmg = 0;
			place_remotecam (ent);
		}
	}

	else
	{
		safe_cprintf(ent->owner, PRINT_HIGH, "Incomplete command: add BUILD, DETONATE, REMOVE or TOGGLE\n");
	}

}

