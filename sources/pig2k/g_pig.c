
#include "g_local.h"


// remove_target: remove a monster from the level completely
void remove_target (edict_t *ent, edict_t *targ, int mod)
{
        targ->s.modelindex = 0;
        targ->s.effects = 0;

        if (targ->client)
        {
                targ->client->pigged = true;
                // have to use NO_PROTECTION _and_ NO_ARMOR in here cos NO_PROTECTION doesn't work right
                T_Damage (targ, ent, ent, ent->client->v_angle, targ->s.origin, vec3_origin, (targ->health+5), 0, DAMAGE_NO_ARMOR|DAMAGE_NO_PROTECTION, mod);
        }
        else
        {
                monster_death_use (targ);
                targ->nextthink = 0;
                gi.linkentity(targ);

                // 'officially' remove monster from level (unless a sheep)
                if (Q_stricmp(targ->classname, "misc_sheep") != 0)
                {
                        level.killed_monsters++;
                        if (coop->value)
                                ent->client->resp.score++;
                }

                G_FreeEdict(targ);
        }
}

// checksplat: checks to see if an inverted ent has hit the ceiling
void pig_checksplat (edict_t *ent)
{
        vec3_t  v1, v2, v3;

        VectorCopy(ent->s.origin, v1);
        v1[2] += ent->maxs[2] + 2;

        VectorCopy(v1, v2);
        VectorCopy(v1, v3);

        v2[0] += ent->maxs[0];
        v2[1] += ent->maxs[1];

        v3[0] += ent->mins[0];
        v3[1] += ent->mins[1];

        if ((gi.pointcontents(v1) & MASK_ALL)
                || (gi.pointcontents(v2) & MASK_ALL)
                || (gi.pointcontents(v3) & MASK_ALL))
        {
                ent->gravity *= -1;
                ent->s.angles[ROLL] -= 180;
                ent->s.effects = 0;
                ent->s.renderfx = 0;
                T_Damage (ent, ent, ent, ent->s.angles, ent->s.origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_PIG_GRAVITY);
                if (ent->client)
                {
                        ent->gravity = 1;
                        ent->velocity[2] = 0;
                        ent->s.origin[2] -= 48; // pull them out of the ceiling
                        if (ent->client->ps.pmove.pm_flags & PMF_NO_PREDICTION)
                                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                }
        }
}


void pig_gib (edict_t *ent, edict_t *targ)
{
        // green blast
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
        gi.WritePosition (targ->s.origin);
        gi.multicast (targ->s.origin, MULTICAST_PVS);

        // gib em
        T_Damage (targ, ent, ent, ent->client->v_angle, targ->s.origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_PIG_GIB);
}

void remove_groundshake_think (edict_t *self)
{
        edict_t *e;
        int     i;

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
                if (e->client)
                        e->velocity[2] = 300 * (100.0 / e->mass);
                else if (e->monsterinfo.stand)
                        e->velocity[2] = 50 * (100.0 / e->mass);                        
	}

        if (level.time < self->timestamp)
                self->nextthink = level.time + FRAMETIME;
}        

void pig_remove (edict_t *ent, edict_t *targ)
{
        edict_t *s;
                        
        // blue shockwave
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
        gi.WritePosition (targ->s.origin);
        gi.multicast (targ->s.origin, MULTICAST_ALL);

        // the sound
        gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/bigtele.wav"), 1, ATTN_NONE, 0);

        // bright flash
        ent->client->bonus_alpha = 0.5;

        // start the ground shaking
        s = G_Spawn();
        s->timestamp = level.time + 3;
        s->think = remove_groundshake_think;
        remove_groundshake_think(s);

        remove_target (ent, targ, MOD_PIG_REMOVE);
}

void SP_item_health_large (edict_t *self);
void SP_misc_explobox (edict_t *self);
void sheep_create (edict_t *self);
void pig_transform (edict_t *ent, edict_t *targ)
{
        edict_t *new;
        int     r;

        new = G_Spawn();
        VectorCopy (targ->s.origin, new->s.origin);
        new->s.origin[2] += 16; // make sure new thingy is well off ground

        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (targ-g_edicts);
        gi.WriteByte (MZ_LOGIN);
        gi.multicast (targ->s.origin, MULTICAST_PVS);

        remove_target (ent, targ, MOD_PIG_TRANSFORM);

        // every now and then make em into a sheep
        if (random() < 0.2)
        {
                sheep_create(new);
                return;
        }

        r = random()*4;

        switch(r)
        {
        case 0:
        default:
                SP_item_health_large(new);
                break;
        case 1:
                SpawnItem(new, FindItem("Combat Armor"));
                break;
        case 2:
                SpawnItem(new, FindItem("Quad Damage"));
                break;
        case 3:
                SP_misc_explobox(new);
                break;
        }

}

void barrel_explode (edict_t *self);
void pig_drop (edict_t *ent, edict_t *targ)
{
        edict_t *thingy;
        trace_t tr;
        vec3_t  start, up, dir, end, mins, maxs;

        thingy = G_Spawn();

        VectorSet (thingy->mins, -16, -16, 0);
        VectorSet (thingy->maxs, 1, 16, 40);

        VectorSet(up, -90, 0, 0);
        AngleVectors(up, dir, NULL, NULL);
        VectorCopy(targ->s.origin, start);
        start[2] += targ->maxs[2] + 2;
        VectorMA(start, 8192, dir, end);
        tr = gi.trace(targ->s.origin, mins, maxs, end, targ, MASK_ALL);
        VectorCopy (tr.endpos, thingy->s.origin);
        thingy->s.origin[2] -= 42; // lower barrel so it isn't in the ceiling

        thingy->solid = SOLID_BBOX;
        thingy->movetype = MOVETYPE_STEP;

        thingy->model = "models/objects/barrels/tris.md2";
        thingy->s.modelindex = gi.modelindex (thingy->model);
        VectorSet (thingy->mins, -16, -16, 0);
        VectorSet (thingy->maxs, 16, 16, 40);

        thingy->mass = 400;
        thingy->health = 10;
        thingy->dmg = 80;

        thingy->activator = ent;
        thingy->die = barrel_explode;
        thingy->touch = barrel_explode;
        thingy->takedamage = DAMAGE_NO;

        VectorSet(thingy->velocity, 0, 0, -50);

        gi.linkentity (thingy);
}

void pig_swap (edict_t *ent, edict_t *targ)
{
        vec3_t  temp;

        VectorCopy(ent->s.origin, temp);
        VectorCopy(targ->s.origin, ent->s.origin);
        VectorCopy(temp, targ->s.origin);

        // make sure neither ents are in ground
        ent->s.origin[2] += ent->mins[2] - targ->mins[2] + 2;
        targ->s.origin[2] += targ->mins[2] - ent->mins[2] + 2;
        M_droptofloor(targ);

        // bright flash
        ent->client->bonus_alpha = 0.5;
        if (targ->client)
                targ->client->bonus_alpha = 0.5;

        // send movement effect to client
        gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (ent-g_edicts);
        gi.WriteByte (MZ_LOGOUT);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
        if (targ->client)
        {
                gi.WriteByte (svc_muzzleflash);
                gi.WriteShort (targ-g_edicts);
                gi.WriteByte (MZ_LOGOUT);
                gi.multicast (targ->s.origin, MULTICAST_PVS);
        }

        // hold in place briefly
        ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        ent->client->ps.pmove.pm_time = 14;
        if (targ->client)
        {
                targ->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
                targ->client->ps.pmove.pm_time = 14;
        }
}


void pig_gravity (edict_t *ent, edict_t *targ)
{
        int     d;

        gi.dprintf("Inverting gravity...\n");

        if (targ->client)
        {
                targ->gravity = -1;
//                targ->s.origin[2] += 4; // get em off the floor
                targ->velocity[2] = 500;
        }
        else
        {
                targ->gravity *= -1;
                targ->velocity[2] = -5 * targ->gravity;
                targ->s.angles[ROLL] += 180;
        }

// flip their bounding box
        d = targ->maxs[2] - (targ->mins[2] * -1);
        targ->s.origin[2] += d + 2;
        targ->maxs[2] -= d;
        targ->mins[2] -= d;
}

void fire_pig (edict_t *ent, edict_t *targ)
{
        int     r;

        r = random()*10;

        switch(r)
        {
        case 0:             
        default:
                pig_gib (ent, targ);
                break;
        case 1:
        case 2:
                pig_remove (ent, targ);
                break;
        case 3:
        case 4:
        case 5:
                pig_transform (ent, targ);
                break;
        case 6:
        case 7:
                pig_drop (ent, targ);
                break;
        case 8:
                pig_swap (ent, targ);
                break;
        case 9:
                pig_gravity (ent, targ);
                break;
        }
}

