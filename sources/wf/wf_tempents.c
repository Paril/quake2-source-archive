#include "g_local.h"


//Temp Entity handling from Maj.Bitch

//======================================================
//========== Spawn Temp Entity Functions ===============
//======================================================



/*
Spawns (type) Splash with {count} particles of {color} at {start} moving
in {direction} and Broadcasts to all in Potentially Visible Set from
vector (origin)

TE_LASER_SPARKS - Splash particles obey gravity
TE_WELDING_SPARKS - Splash particles with flash of light at {origin}
TE_SPLASH - Randomly shaded shower of particles

colors:
1 - red/gold - blaster type sparks
2 - blue/white - blue
3 - brown - brown
4 - green/white - slime green
5 - red/orange - lava red
6 - red - blood red
All others are grey
*/
//======================================================
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WriteByte(count);
	gi.WritePosition(start);
	gi.WriteDir(movdir);
	gi.WriteByte(color);
	gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
/*
Spawns a string of successive (type) models of from record (rec_no)
from (start) to (endpos) which are offset by vector (offset) and
Broadcasts to all in Potentially Visible Set from vector (origin)

Type:
TE_GRAPPLE_CABLE - The grappling hook cable
TE_MEDIC_CABLE_ATTACK - NOT IMPLEMENTED IN ENGINE
TE_PARASITE_ATTACK - NOT IMPLEMENTED IN ENGINE
*/
//======================================================
void G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin ) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WriteShort(rec_no);
	gi.WritePosition(start);
	gi.WritePosition(endpos);
	gi.WritePosition(offset);
	gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
/*
Spawns a trail of (type) from {start} to {end} and Broadcasts to all
in Potentially Visible Set from vector (origin)

TE_BFG_LASER - Spawns a green laser
TE_BUBBLETRAIL - Spawns a trail of bubbles
TE_RAILTRAIL - Spawns a blue spiral trail filled with white smoke
*/
//======================================================
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin ) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.WritePosition(endpos);
	gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
/*
Spawns sparks of (type) from {start} in direction of {movdir} and
Broadcasts to all in Potentially Visible Set from vector (origin)

TE_BLASTER - Spawns blaster sparks
TE_BLOOD - Spawns spurt of red blood
TE_BULLET_SPARKS - Same as TE_SPARKS, with a bullet puff and richochet sound
TE_GREENBLOOD - NOT IMPLEMENTED - Spawns a spurt of green blood
TE_GUNSHOT - Spawns a grey splash of particles, with a bullet puff
TE_SCREEN_SPARKS - Spawns a large green/white splash of sparks
TE_SHIELD_SPARKS - Spawns a large blue/violet splash of sparks
TE_SHOTGUN - Spawns a small grey splash of spark particles, with a bullet puff
TE_SPARKS - Spawns a red/gold splash of spark particles
*/
//======================================================
void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin ) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.WriteDir(movdir);
	gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
/*
Spawns a (type) explosion at (start} and Broadcasts to all Potentially
Visible Sets from {origin}

TE_BFG_BIGEXPLOSION - Spawns a BFG particle explosion
TE_BFG_EXPLOSION - Spawns a BFG explosion sprite
TE_BOSSTPORT - Spawns a mushroom-cloud particle effect
TE_EXPLOSION1 - Spawns a mid-air-style explosion
TE_EXPLOSION2 - Spawns a nuclear-style explosion
TE_GRENADE_EXPLOSION - Spawns a grenade explosion
TE_GRENADE_EXPLOSION_WATER - Spawns an underwater grenade explosion
TE_ROCKET_EXPLOSION - Spawns a rocket explosion
TE_ROCKET_EXPLOSION_WATER - Spawns an underwater rocket explosion

Note: The last four EXPLOSION entries overlap to some degree.
TE_GRENADE_EXPLOSION is the same as TE_EXPLOSION2,
TE_ROCKET_EXPLOSION is the same as TE_EXPLOSION1,
and both of the EXPLOSION_WATER entries are the same, visually.
*/
//======================================================
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin ) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.multicast(origin, MULTICAST_PVS);
}

//======================================================
/*
Spawns flames of (type) from {start} in direction of {movdir} and
Broadcasts to all in Potentially Visible Set from vector (origin)

TE_FLAME - Spawns a flame
*/
//======================================================

void G_Spawn_Flame(int type, short rec_no, int count, vec3_t start, vec3_t origin, vec3_t pos1,vec3_t pos2,vec3_t pos3, vec3_t pos4 )
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	gi.WriteShort(rec_no);
	gi.WriteShort(count);
	gi.WritePosition (start);
	gi.WritePosition (origin);
	gi.WritePosition (pos1);
	gi.WritePosition (pos2);
	gi.WritePosition (pos3);
	gi.WritePosition (pos4);
	gi.multicast (origin, MULTICAST_PVS);

}

//======================================================
/*
Spawns lightning of (type) from {start} in direction of {movdir} and
Broadcasts to all in Potentially Visible Set from vector (origin)

TE_LIGHTNING - Spawns lightning
*/
//======================================================
void G_Spawn_Lightning(int type, short rec_no, short rec_no2, vec3_t start, vec3_t origin)
{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (type);
		gi.WriteShort (rec_no);			// destination entity
		gi.WriteShort (rec_no2);		// source entity
		gi.WritePosition (origin);
		gi.WritePosition (start);
		gi.multicast (start, MULTICAST_PVS);
}

/*
Spawns a debug trail from [start] to [endpos]
Type:
TE_DEBUG - The debug trail
*/
//======================================================
void G_Spawn_DBTrail(int type, vec3_t start, vec3_t endpos ) 
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	gi.WritePosition (start);
	gi.WritePosition (endpos);
	gi.multicast (start, MULTICAST_ALL);	
}

//======================================================
/*
Spawns a trail of (type) from {start} to {end} and Broadcasts to all
in Potentially Visible Set from vector (origin)

TE_FLASHLIGHT - Spawns a green laser
*/
//======================================================
void G_Spawn_Light(int type, short rec_no, vec3_t origin ) 
{

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.WriteShort (rec_no);
	gi.multicast (origin, MULTICAST_PVS);
}


void show_Temp_Ent(int type, short rec_no, short rec_no2, int count, int color, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t movdir, vec3_t origin, vec3_t pos1,vec3_t pos2,vec3_t pos3, vec3_t pos4  ) 
{
	switch (type)
	{
	case TE_LASER_SPARKS:
	case TE_WELDING_SPARKS:
	case TE_SPLASH:
	case TE_TUNNEL_SPARKS:
		G_Spawn_Splash(type, count, color, start, movdir, origin);
		return;

	case TE_GRAPPLE_CABLE:
	case TE_MEDIC_CABLE_ATTACK:
	case TE_PARASITE_ATTACK:
		G_Spawn_Models(type, rec_no, start, endpos, offset, origin );
		return;

	case TE_BFG_LASER:
	case TE_BUBBLETRAIL:
	case TE_RAILTRAIL:
	case TE_RAILTRAIL2:
	case TE_BUBBLETRAIL2:
		G_Spawn_Trails(type, start, endpos, origin );
		return;

	case TE_BLASTER:
	case TE_BLOOD:
	case TE_BULLET_SPARKS:
	case TE_GREENBLOOD:
	case TE_GUNSHOT:
	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
	case TE_SHOTGUN:
	case TE_SPARKS:
	case TE_BLUEHYPERBLASTER:
	case TE_BLASTER2:
		G_Spawn_Sparks(type, start, movdir, origin );
		return;

	case TE_BFG_BIGEXPLOSION:
	case TE_BFG_EXPLOSION:
	case TE_BOSSTPORT:
	case TE_EXPLOSION1:
	case TE_EXPLOSION2:
	case TE_GRENADE_EXPLOSION:
	case TE_GRENADE_EXPLOSION_WATER:
	case TE_ROCKET_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
	case TE_PLASMA_EXPLOSION:
	case TE_PLAIN_EXPLOSION:
	case TE_EXPLOSION1_BIG:
	case TE_EXPLOSION1_NP:
		G_Spawn_Explosion( type, start, origin );
		return;

	case TE_FLAME:
		G_Spawn_Flame(type, rec_no, count, start, origin,  pos1, pos2, pos3, pos4 );
		return;

	case TE_LIGHTNING:
		G_Spawn_Lightning(type, rec_no, rec_no2, start, origin);
		return;

	case TE_DEBUGTRAIL:
		G_Spawn_DBTrail(type, start, endpos );
		return;

	case TE_FLASHLIGHT:
		G_Spawn_Light(type, rec_no, origin );
		return;

	case TE_FORCEWALL:
	case TE_HEATBEAM:
	case TE_MONSTER_HEATBEAM:
	case TE_STEAM:
	case TE_MOREBLOOD:
	case TE_HEATBEAM_SPARKS:
	case TE_HEATBEAM_STEAM:
	case TE_CHAINFIST_SMOKE:
	case TE_ELECTRIC_SPARKS:
	case TE_TRACKER_EXPLOSION:
	case TE_TELEPORT_EFFECT:
	case TE_DBALL_GOAL:
	case TE_WIDOWBEAMOUT:
	case TE_NUKEBLAST:
	case TE_WIDOWSPLASH:
	case TE_FLECHETTE:
		return;
	}
}