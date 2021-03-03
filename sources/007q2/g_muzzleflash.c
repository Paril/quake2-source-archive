#include "g_local.h"

// Muzzle Flash modification
// By David Leinbach
// August 8, 2000

void MuzzleFlash_Create(vec3_t location, vec3_t dir, char filename[])
//void MuzzleFlash_Create(edict_t *self)
{
/*	edict_t *muzzleflash;
	char filepath[25];

	muzzleflash = G_Spawn();
	VectorCopy (location, muzzleflash->s.origin);
	vectoangles (dir, muzzleflash->s.angles);
//	VectorNormalize(forward);

	strcpy(filepath, "sprites/"); // Setup Filepath to
	strcat(filepath, filename);   // the appropriate muzzle
	strcat(filepath, ".tga");	  // flash sprite
//	strcat(filepath, ".sp2");	  // flash sprite
	muzzleflash->s.modelindex = gi.modelindex("models/muzzleflash/tris.md2");
//	muzzleflash->s.modelindex = gi.modelindex (filepath); // Muzzle Flash Sprite
//	ent->player->s.modelindex3 = gi.modelindex("models/muzzleflash/tris.md2");
	muzzleflash->solid = SOLID_NOT;
	muzzleflash->takedamage=DAMAGE_NO;
//	muzzleflash->s.renderfx |= RF_FLASH;

	muzzleflash->think=G_FreeEdict; // Remove flash after delay
//	muzzleflash->think=muzzleflash->s.modelindex3 = 0; // Remove flash after delay
	muzzleflash->nextthink=level.time+0.002; // Delay before removing flash (in seconds) NOTE: Any value under about 0.105 will NOT work correctly

	gi.linkentity (muzzleflash);
//
//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_MUZZLEFLASH);
//	gi.WritePosition (self->s.origin);
//	gi.multicast (self->s.origin, MULTICAST_PVS);

//	G_FreeEdict (self);
*/
}
// ion
//	VectorCopy(ent->player->s.origin, ent->muzzleflash->s.origin);
// ion

void UpdateMuzzleflash (edict_t *ent)
{
        vec3_t offset,spot,forward,right,start;
        trace_t tr;
        if (!ent->muzzleflash)
                return;
	VectorSet(offset, 20, 7, ent->viewheight-5);
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
                AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorNormalize(forward);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        VectorMA (start, 8192, forward, spot);
        tr = gi.trace (start, vec3_origin, vec3_origin, spot, ent, MASK_PLAYERSOLID);
        VectorCopy (tr.endpos, ent->crosshair->s.origin);

        gi.linkentity (ent->muzzleflash);
}

void MakeMuzzleflash (edict_t *ent)
{
//        if (!muzzle->value)
//                return;
        ent->muzzleflash = G_Spawn ();
        ent->muzzleflash->solid = SOLID_NOT;
        ent->muzzleflash->movetype = MOVETYPE_NONE;
        ent->muzzleflash->s.renderfx = RF_FULLBRIGHT;
        gi.setmodel (ent->muzzleflash, "models/muzzleflash/tris.md2");
        UpdateMuzzleflash (ent);
}

void DestroyMuzzleflash (edict_t *ent)
{
        if (!ent->muzzleflash)
                return;
        G_FreeEdict(ent->muzzleflash);
        ent->muzzleflash = NULL;
}