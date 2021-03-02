#include "g_local.h"

//======================================================
// True if start and end are within radius distance.
//======================================================
qboolean G_Within_Radius(vec3_t start, vec3_t end, float rad) {
	vec3_t eorg={0,0,0};
	int j;
	for (j=0; j<3; j++)
		eorg[j]=abs(start[j]-end[j]);
	return (VectorLength(eorg) < rad);
}

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
qboolean G_EntExists(edict_t *ent) {
	return ((ent) && (ent->client) && (ent->inuse));
}

//======================================================
// True if ent is not DEAD or DEAD or DEAD (and BURIED!)
//======================================================
qboolean G_ClientNotDead(edict_t *ent) {
	qboolean buried=true;
	qboolean b1=ent->client->ps.pmove.pm_type!=PM_DEAD;
	qboolean b2=ent->deadflag != DEAD_DEAD;
	qboolean b3=ent->health > 0;
	return (b3 || b2 || b1) && (buried);
}

//======================================================
// True if ent is not DEAD and not just did a Respawn.
//======================================================
qboolean G_ClientInGame(edict_t *ent) {
	if (!G_EntExists(ent)) return false;
	if (!G_ClientNotDead(ent)) return false;
	return (ent->client->respawn_time + 5.0 < level.time);
}

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
	
	  /* Color Codes
	  0xf2f2f0f0,//red
	  0xd0d1d2d3,//green
	  0xf3f3f1f1,//blue
	  0xdcdddedf,//yellow
	  0xe0e1e2e3,//yellow strobe
	  0x80818283,//dark purple
	  0x70717273,//light blue
	  0x90919293,//different green
	  0xb0b1b2b3,//purple
	  0x40414243,//different red
	  0xe2e5e3e6,//orange
	  0xd0f1d3f3,//mixture
	  0xf2f3f0f1,//inner = red, outer = blue
	  0xf3f2f1f0,//inner = blue, outer = red
	  0xdad0dcd2,//inner = green, outer = yellow
	  0xd0dad2dc //inner = yellow, outer = green
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
//======================================================
//======================================================
/*
Spawns a string of successive (type) models of from record (rec_no)
from (start) to (endpos) which are offset by vector (offset) and
Broadcasts to all in Potentially Visible Set from vector (origin)

  Type:
  TE_GRAPPLE_CABLE - The grappling hook cable (NOT IN 3.19!)
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
//======================================================
//======================================================
/*
Spawns a trail of (type) from {start} to {end} and Broadcasts to all
in Potentially Visible Set from vector (origin)

  TE_BFG_LASER - Spawns a green laser
  TE_BUBBLETRAIL - Spawns a trail of bubbles
  TE_PLASMATRAIL - NOT IMPLEMENTED IN ENGINE
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
//======================================================
//======================================================
/*
Spawns sparks of (type) from {start} in direction of {movdir} and
Broadcasts to all in Potentially Visible Set from vector (origin)

  TE_BLASTER - Spawns a blaster sparks
  TE_BLOOD - Spawns a spurt of red blood
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
//======================================================
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

void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin) 
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.multicast(origin, MULTICAST_PVS);
}
