#include "g_local.h"
#include "p_laser.h"

void LaserGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			te_type;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_DEFENSE_LASER);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (ent->waterlevel) {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION_WATER;
		else
			te_type=TE_ROCKET_EXPLOSION_WATER; }
	else {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION;
		else
			te_type=TE_ROCKET_EXPLOSION; }

	G_Spawn_Explosion(te_type, origin, ent->s.origin);

	G_FreeEdict (ent->owner);
	G_FreeEdict (ent);
}

void LaserGrenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
    self->nextthink = level.time + .1;
    self->think = LaserGrenade_Explode;
}

void    PlaceLaser (edict_t *ent)
{
	edict_t         *laser, *grenade;
    vec3_t          forward, wallp;
    trace_t         tr;

    // valid ent ?
    if ((!ent->client) || (ent->health<=0))
	    return;

	if (ent->client->resp.class != ENGINEER)
		return;

    // cells for laser ?
    if (ent->client->pers.inventory[ITEM_INDEX(item_cells)] < CELLS_FOR_LASER)
    {
        gi.cprintf(ent, PRINT_HIGH, "Not enough cells for laser.\n");
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
	    gi.cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
        return;
    }

    // Hit sky ?
    if (tr.surface)
    	if (tr.surface->flags & SURF_SKY)
	        return;

    // Ok, lets stick one on then ...
    gi.cprintf (ent, PRINT_HIGH, "Laser attached.\n");

    ent->client->pers.inventory[ITEM_INDEX(item_cells)] -= CELLS_FOR_LASER;

    // -----------
    // Setup laser
    // -----------
    laser = G_Spawn();

    laser -> movetype                = MOVETYPE_NONE;
    laser -> solid                   = SOLID_NOT;
    laser -> s.renderfx              = RF_BEAM|RF_TRANSLUCENT;
    laser -> s.modelindex    		= 1;                    // must be non-zero
    laser -> s.sound                 = gi.soundindex("world/laser.wav");
    laser -> classname               = "laser_yaya";
    laser -> s.frame                 = 2;    // beam diameter
    laser -> owner                   = laser;

	if (deathmatch->value)
	{
		if (ctf->value)
		{
			if (ent->client->resp.ctf_team == 1)
				laser->s.skinnum = Laser_Red; // red
			else if (ent->client->resp.ctf_team == 2)
				laser->s.skinnum = Laser_Blue; // blue
			else
				laser->s.skinnum = Laser_Green; // no team
		}
		else if (team_dm->value)
		{
			if (ent->client->resp.team == 1)
				laser->s.skinnum = Laser_Red; // red
			else if (ent->client->resp.team == 2)
				laser->s.skinnum = Laser_Blue; // blue
			else if (ent->client->resp.team == 3)
				laser->s.skinnum = Laser_Green; // green
			else if (ent->client->resp.team == 4)
				laser->s.skinnum = Laser_Mix; // as close as we can get to white
			else	
				laser->s.skinnum = Laser_DkPurple;
		}
		else
			laser->s.skinnum = Laser_Red; // red
	}
	else
		laser->s.skinnum = Laser_Red; // red

    laser -> dmg                     = LASER_DAMAGE;
    laser -> think                   = pre_target_laser_think;
    laser -> delay                   = level.time + LASER_TIME;

    // Set orgin of laser to point of contact with wall
    VectorCopy(tr.endpos,laser->s.origin);

    // convert normal at point of contact to laser angles
    vectoangles(tr.plane.normal,laser -> s.angles);

    // setup laser movedir (projection of laser)
    G_SetMovedir (laser->s.angles, laser->movedir);

    VectorSet (laser->mins, -8, -8, -8);
    VectorSet (laser->maxs, 8, 8, 8);

    // link to world
    gi.linkentity (laser);

    // start off ...
    target_laser_off (laser);

    // ... but make automatically come on
    laser -> nextthink = level.time + 2;

	grenade = G_Spawn();

	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);

	grenade -> movetype		= MOVETYPE_NONE;
	grenade -> clipmask		= MASK_SHOT;
	grenade -> solid		= SOLID_BBOX;
	grenade -> s.modelindex	= gi.modelindex (HGRENADE_MODEL);
	grenade -> owner		= laser;
	grenade -> activator	= laser;
	grenade -> nextthink	= level.time + LASER_TIME;
	grenade -> think		= LaserGrenade_Explode;
	grenade -> dmg 			= 120;
	grenade -> dmg_radius 	= 160;

	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);

	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = LaserGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;
    VectorCopy (tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal,grenade -> s.angles);

    gi.linkentity (grenade);
}

void    pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);

    self->think = target_laser_think;
}

//=======================================================
//============= ELECTRIC FENCE ROUTINES =================
//=======================================================
void abs_vector(vec3_t vec1, vec3_t vec2, vec3_t result)
{
	int j;

    for (j=0; j<3; j++)
	    result[j]=abs(vec1[j]-vec2[j]);
}

//=======================================================
void min_vector(vec3_t start, vec3_t end1, vec3_t end2, vec3_t result)
{
	vec3_t eorg1={0,0,0};
    vec3_t eorg2={0,0,0};

    abs_vector(start, end1, eorg1);
    abs_vector(start, end2, eorg2);
    if (VectorLength(eorg1) < VectorLength(eorg2))
	    VectorCopy(end1, result);
    else
        VectorCopy(end2, result);
}

//=======================================================
void beam_laser_think(edict_t *beam)
{
	vec3_t end;
    vec3_t zvec={0,0,0};
    trace_t tr;

    // Has the time expired?
    if (beam->wait < level.time) {
	    G_FreeEdict(beam);
        return; }

    VectorMA(beam->s.origin, beam->spawnflags, beam->movedir, end);
    tr=gi.trace(beam->s.origin, NULL, NULL, end, beam, MASK_ALL);

    if ((tr.ent) && (tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
	    T_Damage(tr.ent, beam, beam->activator, beam->movedir, tr.endpos, zvec, beam->dmg, 1, DAMAGE_ENERGY, MOD_LASERFENCE);
    VectorCopy(tr.endpos, beam->s.old_origin);
    beam->nextthink = level.time + 0.1;
}

//=======================================================
// Spawns a 2 post fence with 4 electric laser rails.
//=======================================================
void Spawn_LaserFence(edict_t *ent)
{
	edict_t *beam[6];
    int i;
    vec3_t post1, post2;
    vec3_t forward, up, right, end, torigin, torigin2, len;
    trace_t tr;

    VectorCopy(ent->s.origin, torigin);
    AngleVectors(ent->s.angles, forward, right, up);

    // Move torigin forward 50 units to end.
    VectorMA(torigin, 50, forward, end);

    // Trace this path to see what we hit.
    tr = gi.trace(torigin, NULL, NULL, end, NULL, MASK_SOLID);

    // torigin2 is the shorter of these vectors.
    min_vector(torigin, tr.endpos, end, torigin2);
    VectorClear(end);

    // Move left from torigin2 1/2 fence length to end.
    VectorMA(torigin2, -FENCE_LENGTH/2, right, end);

    // Trace this path to see what we hit.
    tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);

    // Left post1 is the shorter of these vectors.
    min_vector(torigin2, tr.endpos, end, post1);
    VectorClear(end);

    // Move right from post1 entire length to end.
    VectorMA(post1, FENCE_LENGTH, right, end);

    // Trace this path to see what we hit.
    tr = gi.trace(torigin2, NULL, NULL, end, NULL, MASK_SHOT);

    // Right post2 is the shorter of these vectors.
    min_vector(torigin2, tr.endpos, end, post2);

    // Get length of fence.
    abs_vector(post1, post2, len);

    // Okay, let's make the damn beam already!!
    for (i=0; i<=5; i++)
	{
	    beam[i] = G_Spawn();
        beam[i]->owner = beam[i];
        beam[i]->classname= "LaserFence";
        beam[i]->activator = ent; // Must set for frags!!
        beam[i]->movetype = MOVETYPE_NONE;
        beam[i]->solid = SOLID_NOT;
        beam[i]->svflags &= ~SVF_NOCLIENT;
        beam[i]->s.renderfx = RF_BEAM;
        beam[i]->s.modelindex = 1;
        beam[i]->spawnflags = VectorLength(len); // Store Length here..

		if (deathmatch->value)
		{
			if (ctf->value)
			{
				if (ent->client->resp.ctf_team == 1)
					beam[i]->s.skinnum = Laser_Red;
				else if (ent->client->resp.ctf_team == 2)
					beam[i]->s.skinnum = Laser_Blue;
				else
					beam[i]->s.skinnum = Laser_DkPurple;
			}
			else if (team_dm->value)
			{
				if (ent->client->resp.team == 1)
					beam[i]->s.skinnum = Laser_Red;
				else if (ent->client->resp.team == 2)
					beam[i]->s.skinnum = Laser_Blue;
				else if (ent->client->resp.team == 3)
					beam[i]->s.skinnum = Laser_Green;
				else if (ent->client->resp.team == 4)
					beam[i]->s.skinnum = Laser_Mix;
				else
					beam[i]->s.skinnum = Laser_DkPurple;
			}
			else
				beam[i]->s.skinnum = Laser_Red;
		}
		else
			beam[i]->s.skinnum = Laser_Red;

        if (i==0) // Only need sound for first one..
	        beam[i]->s.sound = gi.soundindex("world/laser.wav");
        else
            beam[i]->s.sound=0;
        beam[i]->s.frame=2;
        VectorSet(beam[i]->mins,-8,-8,-8);
        VectorSet(beam[i]->maxs, 8, 8, 8);
        beam[i]->dmg = 20; // Each Rail..
        VectorClear(beam[i]->s.angles);
        VectorCopy(right, beam[i]->movedir);
        VectorCopy(post1, beam[i]->s.origin);
        VectorCopy(post1, beam[i]->s.old_origin);
        switch (i) {
	        case 0: // Bottom Fence Rail
	            VectorMA(beam[i]->s.origin,-15, up, beam[i]->s.origin);
	            break;
            case 1: // Mid-Lower Fence Rail
                break;
            case 2: // Mid-Upper Fence Rail
                VectorMA(beam[i]->s.origin, 15, up, beam[i]->s.origin);
                break;
            case 3: // Top Fence Rail
                VectorMA(beam[i]->s.origin, 30, up, beam[i]->s.origin);
                break;
            case 4: // Make 1st Fence Post.
                beam[i]->spawnflags = 55;
                beam[i]->s.frame *= 4;
                VectorCopy(post1, beam[i]->s.origin);
                VectorMA(beam[i]->s.origin, -20, up, beam[i]->s.origin);
                VectorCopy(up, beam[i]->movedir);
                break;
            case 5: // Make 2nd Fence Post.
                beam[i]->spawnflags = 55;
                beam[i]->s.frame *= 4;
                VectorCopy(post2, beam[i]->s.origin);
                VectorMA(beam[i]->s.origin, -20, up, beam[i]->s.origin);
                VectorCopy(up, beam[i]->movedir);
                break;
		} // end switch
        beam[i]->wait = level.time + 60.0; // 1 Minute to beam Destruct.
        beam[i]->think = beam_laser_think;
        beam[i]->nextthink = level.time + 1.0;
        gi.linkentity(beam[i]);
	} // end for
}

//===========================================
void Cmd_LaserFence_f(edict_t *ent)
{
	int index;

	if (ent->client->resp.class != ENGINEER)
		return;

	if (!ent->groundentity)
		return; // in air

	if (ent->waterlevel)
		return; // under water

    index = ITEM_INDEX(item_cells);
    if (ITEM_IN_ENTS_INVENTORY < 20)
	    gi.cprintf(ent, PRINT_HIGH, "Fence requires 20 Cells.\n");
    else {
        ITEM_IN_ENTS_INVENTORY -= 20;
        Spawn_LaserFence(ent); }
}

/*----------------------------------------
SP_LaserSight

Create/remove the laser sight entity
-----------------------------------------*/

#define lss self->lasersight

void SP_LaserSight(edict_t *self) {

	vec3_t  start,forward,right,end;

    if ( lss ) {
	    G_FreeEdict(lss);
        lss = NULL;
        gi.cprintf(self, PRINT_HIGH, "Lasersight off.\n");
        return;
    }

    gi.cprintf(self, PRINT_HIGH, "Lasersight on.\n");

    AngleVectors (self->client->v_angle, forward, right, NULL);

    VectorSet(end,100 , 0, 0);
    G_ProjectSource (self->s.origin, end, forward, right, start);

    lss = G_Spawn ();
    lss->owner = self;
    lss->movetype = MOVETYPE_NOCLIP;
    lss->solid = SOLID_NOT;
    lss->classname = "lasersight";
    lss->s.modelindex = gi.modelindex ("models/objects/lsight/tris.md2");
    lss->s.skinnum = 0;

    lss->s.renderfx |= RF_FULLBRIGHT;

    lss->think = LaserSightThink;
    lss->nextthink = level.time + 0.1;
}

/*---------------------------------------------
LaserSightThink

Updates the sights position, angle, and shape
self is the lasersight entity
---------------------------------------------*/
void LaserSightThink (edict_t *self)
{
	vec3_t start,end,endp,offset;
    vec3_t forward,right,up;
    trace_t tr;

    AngleVectors (self->owner->client->v_angle, forward, right, up);

    VectorSet(offset,24 , 6, self->owner->viewheight-7);
    G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
    VectorMA(start,8192,forward,end);

    tr = gi.trace (start,NULL,NULL, end,self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

    if (tr.fraction != 1) {
	    VectorMA(tr.endpos,-4,forward,endp);
        VectorCopy(endp,tr.endpos);
    }

    vectoangles(tr.plane.normal,self->s.angles);
    VectorCopy(tr.endpos,self->s.origin);

    gi.linkentity (self);
    self->nextthink = level.time + 0.1;
}
