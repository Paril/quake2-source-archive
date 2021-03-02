#include "g_local.h"

//======================================================
//============= HELPER FUNCTIONS HERE ==================
//======================================================

/* Don't know where (if anyplace) these go!
TE_FLAME,
TE_LIGHTNING,
TE_FLASHLIGHT,
TE_FORCEWALL,
TE_HEATBEAM,
TE_MONSTER_HEATBEAM,
TE_STEAM, //PSY: i have a tut on this, but its of no use here
TE_HEATBEAM_STEAM,
TE_TELEPORT_EFFECT,
TE_DBALL_GOAL,
TE_WIDOWBEAMOUT,
TE_FLECHETTE
*/

//======================================================
//========== Spawn Temp Entity Functions ===============
//======================================================
/*
Spawns (type) Splash with {count} particles of (color) at (start) moving
in (direction) and Broadcasts to all in Potentially Visible Set from
vector (origin)

TE_LASER_SPARKS - Splash particles obey gravity
TE_WELDING_SPARKS - Splash particles with flash of light at {origin}
TE_SPLASH - Randomly shaded shower of particles
TE_WIDOWSPLASH - New to v3.20
*/
//======================================================
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) {
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
void G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin ) {
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
Spawns a trail of (type) from (start) to (end) and Broadcasts to all
in Potentially Visible Set from vector (origin)

TE_BFG_LASER - Spawns a green laser
TE_BUBBLETRAIL - Spawns a trail of bubbles
TE_BUBBLETRAIL2 - NOT IMPLEMENTED IN ENGINE
TE_PLASMATRAIL - NOT IMPLEMENTED IN ENGINE
TE_RAILTRAIL - Spawns a blue spiral trail filled with white smoke
TE_RAILTRAIL2 - NOT IMPLEMENTED IN ENGINE
TE_DEBUGTRAIL - New to v3.20
*/
//======================================================
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin ) {
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
Spawns sparks of (type) from (start) in direction of (movdir) and
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
TE_BLASTER2 - New to v3.20
TE_MOREBLOOD - New to v3.20
TE_CHAINFIST_SMOKE - New to v3.20
TE_TUNNEL_SPARKS - New to v3.20
TE_ELECTRIC_SPARKS - New to v3.20
TE_HEATBEAM_SPARKS - New to v3.20
TE_BLUEHYPERBLASTER - New to v3.20
*/
//======================================================
void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin ) {
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
Spawns a (type) explosion at (start} and Broadcasts to all in the
Potentially Hearable set from vector (origin)

TE_BFG_BIGEXPLOSION - Spawns a BFG particle explosion
TE_BFG_EXPLOSION - Spawns a BFG explosion sprite
TE_BOSSTPORT - Spawns a mushroom-cloud particle effect
TE_EXPLOSION1 - Spawns a mid-air-style explosion
TE_EXPLOSION2 - Spawns a nuclear-style explosion
TE_GRENADE_EXPLOSION - Spawns a grenade explosion
TE_GRENADE_EXPLOSION_WATER - Spawns an underwater grenade explosion
TE_ROCKET_EXPLOSION - Spawns a rocket explosion
TE_ROCKET_EXPLOSION_WATER - Spawns an underwater rocket explosion
TE_NUKEBLAST - New to v3.20
TE_EXPLOSION1_BIG - New to v3.20
TE_EXPLOSION1_NP - New to v3.20
TE_PLAIN_EXPLOSION - New to v3.20
TE_PLASMA_EXPLOSION - New to v3.20
TE_TRACKER_EXPLOSION - New to v3.20
*/
//======================================================
void G_Spawn_Explosion(int type, vec3_t start, vec3_t origin ) {
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.multicast(origin, MULTICAST_PHS);
}

//======================================================
// Display Muzzleflash of type 'flashtype' at vector start.
//======================================================
void G_MuzzleFlash(short rec_no, vec3_t start, int flashtype ) {
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(rec_no);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

//======================================================
// Muzzleflash2 of type flashtype (Monster Fire weapons)
//======================================================
void G_MuzzleFlash2(short rec_no, vec3_t start, int flashtype ) {

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(rec_no);
	gi.WriteByte(flashtype);
	gi.multicast(start, MULTICAST_PVS);
}

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

//==========================================================
// True if ent has amount of item in inventory (then deduct)
//==========================================================
qboolean G_Deduct_Item(edict_t *ent, gitem_t *item, int amount) {
	int index=ITEM_INDEX(item);
	if (ent->client->pers.inventory[index] >= amount) {
		ent->client->pers.inventory[index] -= amount;
		return true; }
	return false;
}

//==========================================================
// This is the same as function P_ProjectSource in the source
// except it projects in reverse...
//==========================================================
void P_ProjectSource_Reverse(gclient_t *client,
vec3_t point,
vec3_t distance,
vec3_t forward,
vec3_t right,
vec3_t result)
{
	vec3_t dist;

	VectorCopy(distance, dist);
	if (client->pers.hand == RIGHT_HANDED)
		dist[1] *= -1; // Left Hand already defaulted
	else if (client->pers.hand == CENTER_HANDED)
		dist[1]= 0;
	G_ProjectSource(point, dist, forward, right, result);
}

//==============================================================
// This won't be used in the bot source but I included it anyway.
// I use this in my source in place of the KillBox() function
// because this is what this function really does.. Telefrag All.
//==============================================================
void Telefrag_All(edict_t *ent) {
	KillBox(ent);
}

//======================================================
//============== MISC UTILITY ROUTINES =================
//======================================================

//=======================================================
// Returns the distance between two entities..
//=======================================================
float G_Distance(edict_t *ent1, edict_t *ent2) {
	vec3_t result;
	VectorSubtract(ent1->s.origin, ent2->s.origin, result);
	return VectorLength(result);
}

//=========================================================
// Returns the next available edict_t record.
//=========================================================
int G_GetFreeEdict(void) {
	edict_t *ent=NULL;
	int i;

	// Get next available edict record..
	for(i=0; i < maxclients->value; i++) {
		ent=g_edicts+i+1;
		if (!ent->inuse)
			return i; }

	// Otherwise, No Free Edicts!

	return -1; // Signal to Refuse Connection!!
}

//======================================================
// True if Ent is valid Monster Entity.
//======================================================
qboolean G_IsMonster(edict_t *ent) {
	return ((ent->svflags & SVF_MONSTER)
	&& (ent->deadflag != DEAD_DEAD)
	&& (ent->takedamage)
	&& ((ent->movetype==MOVETYPE_WALK)
	|| (ent->movetype==MOVETYPE_STEP)
	|| (ent->movetype==MOVETYPE_FLY)));
}

//======================================================
// Returns 1 if clear path from spot1 to spot2
//======================================================
qboolean G_Clear_Path(vec3_t spot1, vec3_t spot2) {
	return (gi.trace(spot1, NULL, NULL, spot2, NULL, MASK_OPAQUE).fraction == 1.0);
}
