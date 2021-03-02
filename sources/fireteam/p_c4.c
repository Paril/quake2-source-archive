#include "g_local.h"
#include "p_c4.h"

qboolean G_ClientInGame(edict_t *ent);
qboolean G_Within_Radius(vec3_t start, vec3_t end, float rad);

/*
===============
C4...
This should be interesting
Some of the code (mainly wall
sticking and angle positioning)
has been taken from the Defense
Laser tutorial by Yaya up on
qdevels.

Look in the .H file for some
important #defines
===============
*/

void PlaceC4 (edict_t *ent)
{
	edict_t *c4;
	vec3_t forward, wallp;
	trace_t tr;

	// are we dead or not connected?
	if ((!ent->client) || (ent->health<=0))
		return;

	// since I have a class mod, this is the class check.
	// if this is going in a q2dm mod, feel free to remove this
	if (ent->client->resp.class != DEMOLITIONS) // my demolitions class
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(item_grenades)] < GRENADES_FOR_C4)
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough grenades to place C-4.\n");
		return;
	}

	// CODE BLOCK HERE TAKEN FROM DEFENSE LASER - if it ain't broke don't fix it

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
		gi.cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;

	// Ok, lets stick one on then ...
	gi.cprintf (ent, PRINT_HIGH, "C-4 Explosive attached. Fuse set to 25 seconds.\n");

	ent->client->pers.inventory[ITEM_INDEX(item_grenades)] -= GRENADES_FOR_C4;

	// Ok. Now we get into my code - the wonderful explosive stuff

	c4 = G_Spawn();

	c4 -> movetype = MOVETYPE_NONE;
	c4 -> clipmask = MASK_SHOT;
	c4 -> solid = SOLID_NOT;
	c4 -> s.modelindex = gi.modelindex ("models/items/c4/tris.md2"); // NOTE THIS LINE PEOPLE!!!!
	c4 -> owner = ent;
	c4 -> nextthink = level.time + C4_TIMER;
	c4 -> think = C4_Explode;

	// place it on the wall
	VectorCopy(tr.endpos,c4->s.origin);

	// orient the pack so it doesnt look stupid
	vectoangles(tr.plane.normal,c4 -> s.angles);

	// qME conveniently gives us these numbers
	VectorSet (c4->mins, -10, -9, -1);
	VectorSet (c4->maxs, 7, 8, 27);

	// link to world
	gi.linkentity (c4);
}

void C4_Explode (edict_t *ent)
{
	edict_t *ent2=NULL;
	trace_t tr;
	vec3_t zvec={0,0,0};
	vec3_t start,end;
	vec3_t kvelocity,dir;
	float radius=850;
	int i;

	// Ok. Here's what we need to do in this function:
	// 1. Make an explosion
	// 2. Throw back all players inside radius that aren't dead (thanks to Phil's sheckwave tut)
	// 3. Project damage
	// 4. Make the pack go away.

	// first things first, let's deal the damage.
	T_RadiusDamage(ent, ent->owner, C4_RADIUS_DAMAGE, NULL, C4_DAMAGE_RADIUS, MOD_C4);
	// an explanantion:
	// start at pack, owner will take 50%, deal X amount of damage, someone to ignore (NULL
	// in this case), broadcast over X radius, and tell the death MSG's that its C4.

	// next logical thing to do is handle the throwback - note: directly from phil's tutorial

	// Blow backward ALL ents within 500 units...
	for(i=0;i < game.maxclients;i++) {
		ent2=g_edicts+i+1;

		if (!G_ClientInGame(ent2))
			continue;
		if (ent2==ent)
			continue;
		if (!G_Within_Radius(ent->s.origin, ent2->s.origin, radius))
			continue;

		VectorCopy(ent->s.origin, start);
		VectorCopy(ent2->s.origin, end);
		tr=gi.trace(start, ent2->mins, ent2->maxs, end, NULL, MASK_SHOT);

		// subtract ent's origin from grenade's origin to get direction..
		VectorSubtract(tr.endpos, start, dir);
		VectorScale(dir, 10, kvelocity);
		kvelocity[2] += random()*10; // some upward component
		// Scale new velocity into ent's velocity!
		VectorMA(zvec, 10, kvelocity, ent2->velocity);
	} // end for

	// now we have blown all people back within the radius, only two things left
	// and thanks to the id programmers, we can do them both at once.
	BecomeExplosion1(ent);

	// there we go, exploded C4.
}
