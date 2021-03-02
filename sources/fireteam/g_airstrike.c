#include "g_local.h"

void Grenade_Explode (edict_t *ent);
void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//==========================================
//======= AIRSTRIKE BOMB FLAGS =============
//==========================================
#define ROCKET_BOMBS 1 // airstrike1
#define CLUSTER_BOMBS 2 // airstrike2

//==========================================================
// Create 8 grenades and toss them out in random directions.
//==========================================================
void Cluster_Grenades(edict_t *cluster) {
	trace_t tr;
	gitem_t *item=NULL;
	edict_t *grenade[10];
	vec3_t offset={0,0,0};
	int i;
	vec3_t spray[] = { { 25, 00, 40 },
	{ 17, -17, 40 },
	{ 00, -25, 40 },
	{ -17, -17, 40 },
	{ -25, 00, 40 },
	{ -17, -17, 40 },
	{ -25, 00, 40 },
	{ -17, 17, 40 },
	{ 00, 25, 40 },
	{ 17, 17, 40 }};
	vec3_t forward, right, up, dir;

	for (i=0; i<=9; i++) {
		grenade[i]=G_Spawn();
		grenade[i]->owner=cluster->activator;
		VectorSet(offset, spray[i][0], spray[i][1], spray[i][2]);
		offset[0] += ((crandom()*16.0)-8.0);
		offset[1] += ((crandom()*16.0)-8.0);
		offset[2] += ((crandom()*16.0)-8.0);
		vectoangles(offset, dir);
		AngleVectors(dir, forward, right, up);
		VectorSet(grenade[i]->mins,-8,-8,-8);
		VectorSet(grenade[i]->maxs, 8, 8, 8);
		G_ProjectSource(cluster->s.origin, offset, forward, right, grenade[i]->s.origin);
		tr=gi.trace(cluster->s.origin, grenade[i]->mins, grenade[i]->maxs, grenade[i]->s.origin, cluster, CONTENTS_SOLID);
		VectorCopy(tr.endpos, grenade[i]->s.origin);
		VectorScale(offset, crandom()*10.0, grenade[i]->velocity);
		grenade[i]->velocity[2]=150;
		grenade[i]->movetype=MOVETYPE_BOUNCE;
		grenade[i]->spawnflags=4; // Tell Grenade_Explode() which obits to use..
		grenade[i]->classname="grenade";
		item = item_grenades;
		grenade[i]->clipmask=MASK_SHOT;
		grenade[i]->solid=SOLID_BBOX;
		grenade[i]->s.effects |= EF_GRENADE;
		grenade[i]->dmg=40;
		grenade[i]->radius_dmg=60;
		grenade[i]->dmg_radius=120; // same as regular grenade..
//		grenade[i]->model = "models/objects/grenade/tris.md2";
//		grenade[i]->s.modelindex = gi.modelindex (grenade[i]->model);
		grenade[i]->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
		grenade[i]->touch=Grenade_Touch;
		grenade[i]->think=Grenade_Explode;
		grenade[i]->nextthink = level.time + 3 + (random()*5);
		gi.linkentity(grenade[i]);
	} // end for
}

//======================================================
// Explode into 8 randomly tossed grenades at impact pt.
//======================================================
void clusterbomb_touch(edict_t *cluster, edict_t *other, cplane_t *plane, csurface_t *surf) {

	// Make a rocket-type explosion w/sounds at impact..
	G_Spawn_Explosion(TE_ROCKET_EXPLOSION, cluster->s.origin, cluster->s.origin);

	// Signal Monsters that you've made a big noise!!
	PlayerNoise(cluster->activator, cluster->s.origin, PNOISE_IMPACT);

	// Damage the direct hit other, if exists.
	T_Damage(other, cluster, cluster->activator, cluster->velocity, cluster->s.origin, plane->normal, cluster->dmg, 0, 0, MOD_CLUSTER_BOMBS);

	// Do radius blast damage to all within dmg_radius units
	T_RadiusDamage(cluster, cluster->activator, cluster->radius_dmg, cluster->owner, cluster->dmg_radius, MOD_CLUSTER_BOMBS);

	// Spew out 8 random grenades..
	Cluster_Grenades(cluster);

	G_FreeEdict(cluster);
}

//======================================================
// Handle the rocket when it finally touches something.
//======================================================
void rocketbomb_touch(edict_t *rocket, edict_t *other, cplane_t *plane, csurface_t *surf) {

	// Make a rocket-type explosion w/sounds at impact..
	G_Spawn_Explosion(TE_ROCKET_EXPLOSION, rocket->s.origin, rocket->s.origin);

	// Signal Monsters that you've made a big noise!!
	PlayerNoise(rocket->activator, rocket->s.origin, PNOISE_IMPACT);

	// Damage the direct hit other, if exists.
	T_Damage(other, rocket, rocket->activator, rocket->velocity, rocket->s.origin, plane->normal, rocket->dmg, 0, 0, MOD_ROCKET_BOMBS);

	// Do radius blast damage to all within dmg_radius units
	T_RadiusDamage(rocket, rocket->activator, rocket->radius_dmg, rocket->owner, rocket->dmg_radius, MOD_ROCKET_BOMBS);

	G_FreeEdict(rocket);
}

//======================================================
// Spawn 6 rocket models.
//======================================================
void drop_rocketbomb(edict_t *craft){
	int i;
	vec3_t start;
	vec3_t down={0,0,-1};
	edict_t *rocket[6];

	for (i=0;i<=5;i++) {
		rocket[i]=G_Spawn();
		rocket[i]->owner=craft;
		rocket[i]->activator=craft->activator;// Link back to owner.
		rocket[i]->movetype=MOVETYPE_FLY;
		rocket[i]->clipmask=MASK_SHOT;
		rocket[i]->solid=SOLID_BBOX;
		rocket[i]->s.effects=EF_ROTATE;
		VectorCopy(craft->s.origin, start);
		start[0]+=30*random();
		start[1]+=30*random();
		VectorCopy(start, rocket[i]->s.origin);
		VectorMA(rocket[i]->s.origin, 20*random(), down, rocket[i]->s.origin);
		VectorCopy(down, rocket[i]->movedir);
		vectoangles(down, rocket[i]->s.angles);
		VectorClear(rocket[i]->velocity);
		rocket[i]->velocity[2]=-250-(200*random());
		VectorClear(rocket[i]->mins);
		VectorClear(rocket[i]->maxs);
		rocket[i]->s.modelindex= gi.modelindex (ROCKET_MODEL);
		rocket[i]->s.sound=ROCKET_FLY_SOUND;
		rocket[i]->dmg=100 + (int)(random()*100);
		rocket[i]->radius_dmg=240;
		rocket[i]->dmg_radius=300;
		rocket[i]->touch=rocketbomb_touch;
		rocket[i]->think=NULL;
		rocket[i]->nextthink=0;
		gi.linkentity(rocket[i]); }
}

//======================================================
// Spawn the 'bomb' model
//======================================================
void drop_clusterbomb(edict_t *craft) {
	edict_t *cluster;
	vec3_t down={0,0,-1};

	cluster=G_Spawn();
	cluster->owner=craft;
	cluster->activator=craft->activator; // Activator gets the frags..
	VectorCopy(craft->s.origin, cluster->s.origin);
	VectorCopy(down, cluster->movedir);
	vectoangles(down, cluster->s.angles);
	VectorScale(down, 250, cluster->velocity);
	cluster->movetype=MOVETYPE_FLY;
	cluster->clipmask=MASK_SHOT;
	cluster->solid=SOLID_BBOX;
	cluster->s.effects=EF_ROTATE|EF_GRENADE;
	VectorClear(cluster->mins);
	VectorClear(cluster->maxs);
	cluster->s.modelindex= gi.modelindex (BOMB_MODEL);
	cluster->s.sound=ROCKET_FLY_SOUND;
	cluster->dmg=120;
	cluster->radius_dmg=160;
	cluster->dmg_radius=200;
	cluster->touch=clusterbomb_touch;
	gi.linkentity(cluster);
}

//=====================================================
//=========== VIPER AIRCRAFT ROUTINES =================
//=====================================================

//=====================================================
void AirCraft_Touch(edict_t *craft, edict_t *other, cplane_t *plane, csurface_t *surf){

	if (Q_strcasecmp(other->classname, "worldspawn")==0) {
		G_FreeEdict(craft);
		return; }
}

//=====================================================
void AirCraft_Die(edict_t *craft, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
	vec3_t forward, down={0,0,-1}, torigin;

	// Blow up the craft into a big fireball.
	G_Spawn_Explosion(TE_EXPLOSION2, craft->s.origin, craft->s.origin);

	//
	// Spawn pair of Icarus 'Hover' Monsters
	//
	// Lower first Icarus from exposion fireball.
	AngleVectors(craft->s.angles, forward, NULL, NULL);
	VectorMA(craft->s.origin, 220, down, torigin);
	G_Spawn_Monster(craft, torigin, M_HOVER, 180);

	// Slightly offset second Icarus
	VectorMA(craft->s.origin, 300, forward, torigin);
	G_Spawn_Monster(craft, torigin, M_HOVER, 180);

	// Rain down a bunch of debris!!
	ThrowDebris(craft, DEBRIS3_MODEL, 3.75, craft->s.origin);
	ThrowDebris(craft, DEBRIS3_MODEL, 2.50, craft->s.origin);
	ThrowDebris(craft, DEBRIS2_MODEL, 4.60, craft->s.origin);
	ThrowDebris(craft, DEBRIS2_MODEL, 1.50, craft->s.origin);
	ThrowDebris(craft, DEBRIS3_MODEL, 2.30, craft->s.origin);
	ThrowDebris(craft, DEBRIS3_MODEL, 4.50, craft->s.origin);

	// Award frags for killing the Strogg Viper Ship
	if (G_EntExists(attacker) && (attacker!=craft->activator)) {
		gi.centerprintf(attacker,"3 Frags for killing Viper!\n");
		attacker->client->resp.score += 5; }

	// Clear out the airstrike vars.
	VectorClear(craft->activator->airstrike_start);
	VectorClear(craft->activator->airstrike_target);
	craft->activator->airstrike_called=false;

	G_FreeEdict(craft);
}

//======================================================
void drop_payload(edict_t *craft) {
	vec3_t up={0,0,1};

	// Payload dropped from Craft's position..
	switch (craft->activator->airstrike_type) {
		case CLUSTER_BOMBS: drop_clusterbomb(craft); break;
		case ROCKET_BOMBS: drop_rocketbomb(craft); break;
	} // end switch

	// Aircraft speeds up and away
	VectorClear(craft->velocity);
	VectorMA(craft->velocity, 80, craft->movedir, craft->velocity);
	VectorMA(craft->velocity, 220, up, craft->velocity);
	craft->movetype=MOVETYPE_FLY; // start moving again..
	craft->touch=AirCraft_Touch; // Disappear when hit sky...
	craft->think=G_FreeEdict;
	craft->nextthink=level.time + 3.0;
}

//======================================================
void AirCraft_Lower(edict_t *craft) {
	vec3_t end, down={0,0,-1};

	VectorClear(end);
	VectorMA(craft->s.origin, 500, down, end);

	// Does 500 units downward hit ground??
	if (gi.pointcontents(end) & MASK_SHOT) {
		VectorClear(craft->velocity); // Stop craft!!
		craft->think=drop_payload; // Drop payload in 1 sec.
		craft->nextthink=level.time + 1.0; }

	craft->nextthink=level.time + 0.1;
}

//======================================================
void AirCraft_Think(edict_t *craft) {
	vec3_t dist;

	// How close above crosshair's target are we yet?
	VectorSubtract(craft->activator->airstrike_target, craft->s.origin, dist);

	// If within 50 units then close enough!
	if ((int)VectorLength(dist) < 50) {
		VectorClear(craft->velocity);
		craft->velocity[2] += -50; // Slow decent..
		craft->think=AirCraft_Lower; }

	craft->nextthink=level.time + 0.1;
}

//======================================================
void spawn_aircraft(edict_t *ent) {
	vec3_t start, dir, forward, tangles;
	edict_t *craft=NULL;

	// Set this ASAP else ClientThink() repeats on next frame!
	ent->airstrike_called=false;

	VectorCopy(ent->airstrike_start, start);

	craft=G_Spawn();
	craft->classname="XViper";
	craft->owner=ent;
	craft->activator=ent;
	craft->health=200;
	craft->max_health=200;
	craft->takedamage=DAMAGE_AIM;
	craft->clipmask = MASK_SHOT;
	craft->movetype=MOVETYPE_FLY;
	craft->solid=SOLID_BBOX; // Craft Body Box
	VectorSubtract(ent->airstrike_target, ent->airstrike_start, dir);
	vectoangles(dir, tangles);
	AngleVectors(tangles, forward, NULL, NULL);
	VectorCopy(ent->airstrike_start, craft->s.origin);
	VectorCopy(forward, craft->movedir); // Craft Move direction
	vectoangles(forward, craft->s.angles); // Vector angle of direction
	VectorScale(forward, 100, craft->velocity);
	VectorSet(craft->mins, -30, -30, -10);
	VectorSet(craft->maxs, 30, 30, 10);
	craft->s.modelindex = gi.modelindex (STROGG_SHIP_MODEL);
	craft->s.modelindex2=0;

	craft->s.sound=FLYBY1_SOUND; // Continuous play mode!
	craft->die=AirCraft_Die;
	craft->touch=NULL; // Don't activate this yet!!
	craft->think=AirCraft_Think;
	craft->nextthink=level.time + 0.1;

	gi.linkentity(craft);
}

//======================================================
// Acquire targeting position and set up for airstrike!
//======================================================
void Cmd_Airstrike_f(edict_t *ent) {
	vec3_t start, forward, up, end, torigin;
	trace_t tr, trN, trS, trE, trW;
	vec3_t start1, start2;
	vec3_t north, south, east, west;
	float fN,fS,fE,fW;

	if (!G_ClientInGame(ent)) return;

	// Deduct proper ammo amounts.
	if (!ALLOW_INFINITE_AMMO)
		switch (ent->airstrike_type) {
			case CLUSTER_BOMBS:
				if (!G_Deduct_Item(ent, item_grenades, 10)) {
					gi.cprintf(ent, PRINT_HIGH, "Cluster strike requires 10 Grenades!!\n");
					return; }
				break;
			case ROCKET_BOMBS:
				if (!G_Deduct_Item(ent, item_rockets, 6)) {
					gi.cprintf(ent, PRINT_HIGH, "Rocket strike requires 6 Rockets!!\n");
					return; }
				break;
		} // end switch

	// Zero out the airstrike vectors.
	VectorClear(ent->airstrike_start);
	VectorClear(ent->airstrike_target);

	// cancel airstrike if it's already been called
	if (ent->airstrike_called) {
		ent->airstrike_called=false;
		gi.cprintf(ent, PRINT_HIGH, "Airstrike has been called off!!\n");
		gi.sound(ent, CHAN_VOICE, PILOT1_SOUND, 0.4, ATTN_NORM, 0);
		return; }

	// What is the crosshair pointed at??
	VectorClear(end);
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr=gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|MASK_WATER);

	// Pointing at something but not sky! See if there is sky above it!
	if (tr.surface && !(tr.surface->flags & SURF_SKY)) {
		VectorCopy(tr.endpos,start);
		VectorSet(up, 0,0,1);
		VectorMA(start, 8192, up, end);
		tr=gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|MASK_WATER);
		if (tr.surface && !(tr.surface->flags & SURF_SKY)) {
			gi.cprintf(ent, PRINT_HIGH, "No direct airstrike path to target!\n");
			gi.sound(ent, CHAN_VOICE, PILOT1_SOUND, 0.4, ATTN_NORM, 0);
			return; }
		// Not pointing at sky and sky directly above crosshairs!!
		// NOTE: Vector torigin is directly above crosshair marker!!
		VectorCopy(tr.endpos, torigin);
		// Let's lower vector down from sky a bit...
		torigin[2] += -10;
		VectorCopy(torigin, ent->airstrike_target);
		// Okay.. Set some directional vectors..
		VectorSet(north,0, 1,0);
	    VectorSet(east, 1,0,0);
		VectorSet(south,0,-1,0);
		VectorSet(west,-1,0,0);
		// trace north.
		VectorClear(end);
		VectorMA(torigin, 8192, north, end);
		trN=gi.trace(torigin, NULL, NULL, end, NULL, MASK_ALL);
		// trace south.
		VectorClear(end);
		VectorMA(torigin, 8192, south, end);
		trS=gi.trace(torigin, NULL, NULL, end, NULL, MASK_ALL);
		// trace east.
		VectorClear(end);
		VectorMA(torigin, 8192, east, end);
		trE=gi.trace(torigin, NULL, NULL, end, NULL, MASK_ALL);
		// trace west.
		VectorClear(end);
		VectorMA(torigin, 8192, west, end);
		trW=gi.trace(torigin, NULL, NULL, end, NULL, MASK_ALL);

		//------------------------------------------------------
		// Now we want to find the farthest vector from start!!
		// So, Viper can be flying in the air the longest time!!
		//------------------------------------------------------

		//--------------------------------------
		// Get longest in the N/S direction
		//--------------------------------------
		VectorSubtract(trN.endpos,torigin,end);
		fN=VectorLength(end);
		VectorSubtract(trS.endpos,torigin,end);
		fS=VectorLength(end);
		if (fN>fS)
			VectorCopy(trN.endpos,start1);
		else
			VectorCopy(trS.endpos,start1);

		//--------------------------------------
		// Get longest in the E/W direction
		//--------------------------------------
		VectorSubtract(trE.endpos,torigin,end);
		fE=VectorLength(end);
		VectorSubtract(trW.endpos,torigin,end);
		fW=VectorLength(end);
		if (fE>fW)
			VectorCopy(trE.endpos,start2);
		else
			VectorCopy(trW.endpos,start2);

		//--------------------------------------
		// Now, which is longest of these two?
		//--------------------------------------
		VectorSubtract(start1, torigin, end);
		fN=VectorLength(end); // temp storage
		VectorSubtract(start2, torigin, end);
		fW=VectorLength(end); // temp storage
		if (fN>fW)
			VectorCopy(start1, start);
		else
			VectorCopy(start2, start);

		//--------------------------------------
		// Everything set!! Airstrike in progress!!
		//--------------------------------------
		VectorCopy(start, ent->airstrike_start);
		ent->airstrike_called=true;
		ent->airstrike_time=level.time + 10; // Time to Airstrike
		gi.cprintf(ent, PRINT_HIGH, "Target Locked! ETA 10 secs.\n");
		gi.sound(ent, CHAN_VOICE, PILOT2_SOUND, 0.8, ATTN_NORM, 0); }
	else
		gi.cprintf(ent, PRINT_HIGH, "Target not acquired!! Retarget...\n");
}
