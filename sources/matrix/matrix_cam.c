#include "g_local.h"

void camera_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	VectorCopy(plane->normal, self->s.angles);
	VectorClear(self->velocity);
	self->movetype = MOVETYPE_NONE;
}
void camera_think(edict_t *self)
{

}

void Cmd_ViewCam_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (!(ent->camon) )
	{
		msg = "camera ON\n";
		ent->camon = true;
		ent->client->ps.gunindex = 0;

	}
	else
	{
		msg = "camera OFF\n";
		ent->camon = false;
		
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		if (ent->client->akimbo)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}
void Cmd_ThrowCam_f (edict_t *self)
{
	edict_t	*camera;
	vec3_t	dir;
	vec3_t	forward, right, up;

	if (self->cam)
		G_FreeEdict(self->cam);

	AngleVectors (self->client->v_angle, forward, right, up);

	camera = G_Spawn();
	VectorCopy (self->s.origin, camera->s.origin);
	VectorScale (forward, 500, camera->velocity);
	VectorMA (camera->velocity, 200 + crandom() * 10.0, up, camera->velocity);
	VectorMA (camera->velocity, crandom() * 10.0, right, camera->velocity);
	camera->movetype = MOVETYPE_BOUNCE;
	camera->clipmask = MASK_SHOT;
	camera->solid = SOLID_BBOX;
	VectorClear (camera->mins);
	VectorClear (camera->maxs);
	camera->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	camera->owner = self;
	camera->touch = camera_touch;
	camera->nextthink = level.time + FRAMETIME;
	camera->think = camera_think;
	camera->classname = "camera";

	gi.linkentity (camera);

	self->cam = camera;
}
