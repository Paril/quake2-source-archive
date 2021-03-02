/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_tent.c -- client side temporary entities

#include "client.h"

typedef enum
{
	ex_free, ex_explosion, ex_misc, ex_flash, ex_mflash, ex_poly, ex_poly2
} exptype_t;

typedef struct
{
	exptype_t	type;
	entity_t	ent;

	int			frames;
	float		light;
	vec3_t		lightcolor;
	float		start;
	int			baseframe;
} explosion_t;

#define	MAX_EXPLOSIONS	512 // was 64
explosion_t	cl_explosions[MAX_EXPLOSIONS];

#define	MAX_BEAMS	32
typedef struct
{
	int		entity;
	int		dest_entity;
	struct model_s	*model;
	int		endtime;
	vec3_t	offset;
	vec3_t	start, end;
} beam_t;

beam_t		cl_beams[MAX_BEAMS];


#define	MAX_LASERS	32
typedef struct
{
	entity_t	ent;
	int			endtime;
} laser_t;

laser_t		cl_lasers[MAX_LASERS];

//ROGUE
cl_sustain_t	cl_sustains[MAX_SUSTAINS];
//ROGUE

//PGM
extern void CL_TeleportParticles (vec3_t org);
//PGM

void CL_BlasterParticles (vec3_t org, vec3_t dir);
void CL_ExplosionParticles (vec3_t org);
void CL_BFGExplosionParticles (vec3_t org);
// RAFAEL
void CL_BlueBlasterParticles (vec3_t org, vec3_t dir);
// jit <!--
#ifdef QUAKE2
struct sfx_s	*cl_sfx_ric1;
struct sfx_s	*cl_sfx_ric2;
struct sfx_s	*cl_sfx_ric3;
struct sfx_s	*cl_sfx_lashit;
struct sfx_s	*cl_sfx_spark5;
struct sfx_s	*cl_sfx_spark6;
struct sfx_s	*cl_sfx_spark7;
struct sfx_s	*cl_sfx_railg;
struct sfx_s	*cl_sfx_rockexp;
struct sfx_s	*cl_sfx_grenexp;
struct sfx_s	*cl_sfx_watrexp;
// RAFAEL
struct sfx_s	*cl_sfx_plasexp;
struct sfx_s	*cl_sfx_footsteps[4];
struct sfx_s	*cl_sfx_footsteps_snow[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_grass[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_asphalt[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_wood_plank[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_metal[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_sand[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_metalt[4]; // jitsound
struct sfx_s	*cl_sfx_footsteps_barrel[4]; // jitsound

struct model_s	*cl_mod_explode;
struct model_s	*cl_mod_smoke;
struct model_s	*cl_mod_flash;
struct model_s	*cl_mod_parasite_segment;
struct model_s	*cl_mod_grapple_cable;
struct model_s	*cl_mod_parasite_tip;
struct model_s	*cl_mod_explo4;
struct model_s	*cl_mod_bfg_explo;
struct model_s	*cl_mod_powerscreen;
// RAFAEL
struct model_s	*cl_mod_plasmaexplo;

//ROGUE
struct sfx_s	*cl_sfx_lightning;
struct sfx_s	*cl_sfx_disrexp;
struct model_s	*cl_mod_lightning;
struct model_s	*cl_mod_heatbeam;
struct model_s	*cl_mod_monster_heatbeam;
struct model_s	*cl_mod_explo4_big;
#else
struct sfx_s	*cl_sfx_splat[2];
struct sfx_s	*cl_sfx_splat_barrel[2];
struct sfx_s	*cl_sfx_splat_grass[2];
struct sfx_s	*cl_sfx_splat_metal_thick[2];
struct sfx_s	*cl_sfx_splat_metal_thin[2];
struct sfx_s	*cl_sfx_splat_brick[2];
struct sfx_s	*cl_sfx_splat_dirt[2];
struct sfx_s	*cl_sfx_splat_wood_solid[2];
struct sfx_s	*cl_sfx_splat_wood_plank[2];
struct sfx_s	*cl_sfx_grensplat1;
struct sfx_s	*cl_sfx_grensplat2;
struct sfx_s	*cl_sfx_grensplat3;
struct sfx_s	*cl_sfx_paintfly[3];
struct sfx_s	*cl_sfx_footsteps[MAX_STEP_VARIATIONS];
struct sfx_s	*cl_sfx_footsteps_snow[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_grass[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_asphalt[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_wood_plank[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_metal[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_sand[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_metalt[MAX_STEP_VARIATIONS]; // jitsound
struct sfx_s	*cl_sfx_footsteps_barrel[MAX_STEP_VARIATIONS]; // jitsound
struct model_s	*cl_mod_smoke;
struct model_s	*cl_mod_splat;
struct model_s	*cl_mod_paintball;
#endif

/*
=================
CL_RegisterTEntSounds
=================
*/
void CL_RegisterTEntSounds (void) // jit, cleaned up and adjusted for paintball
{
#ifdef QUAKE2
#else
	cl_sfx_splat[0] = S_RegisterSound("splat/splat1.wav");
	cl_sfx_splat[1] = S_RegisterSound("splat/splat2.wav");
	cl_sfx_splat_barrel[0] = S_RegisterSound("splat/splat_barrel1.wav");
	cl_sfx_splat_barrel[1] = S_RegisterSound("splat/splat_barrel2.wav");
	cl_sfx_splat_grass[0] = S_RegisterSound("splat/splat_grass1.wav");
	cl_sfx_splat_grass[1] = S_RegisterSound("splat/splat_grass2.wav");
	cl_sfx_splat_metal_thick[0] = S_RegisterSound("splat/splat_metal_thick1.wav");
	cl_sfx_splat_metal_thick[1] = S_RegisterSound("splat/splat_metal_thick2.wav");
	cl_sfx_splat_metal_thin[0] = S_RegisterSound("splat/splat_metal_thin1.wav");
	cl_sfx_splat_metal_thin[1] = S_RegisterSound("splat/splat_metal_thin2.wav");
	cl_sfx_splat_brick[0] = S_RegisterSound("splat/splat_brick1.wav");
	cl_sfx_splat_brick[1] = S_RegisterSound("splat/splat_brick2.wav");
	cl_sfx_splat_dirt[0] = S_RegisterSound("splat/splat_dirt1.wav");
	cl_sfx_splat_dirt[1] = S_RegisterSound("splat/splat_dirt2.wav");
	cl_sfx_splat_wood_solid[0] = S_RegisterSound("splat/splat_wood_solid1.wav");
	cl_sfx_splat_wood_solid[1] = S_RegisterSound("splat/splat_wood_solid2.wav");
	cl_sfx_splat_wood_plank[0] = S_RegisterSound("splat/splat_wood_plank1.wav");
	cl_sfx_splat_wood_plank[1] = S_RegisterSound("splat/splat_wood_plank2.wav");
	cl_sfx_grensplat1 = S_RegisterSound("splat/grensplat1.wav");
	cl_sfx_grensplat2 = S_RegisterSound("splat/grensplat2.wav");
	cl_sfx_grensplat3 = S_RegisterSound("splat/grensplat3.wav");
	/*cl_sfx_paintfly[0] = S_RegisterSound("paint/whiz1.wav");
	cl_sfx_paintfly[1] = S_RegisterSound("paint/whiz2.wav");
	cl_sfx_paintfly[2] = S_RegisterSound("paint/whiz3.wav");*/
#endif

	S_RegisterSound("player/land1.wav");
	cl_sfx_footsteps[0] = S_RegisterSound("player/step0.wav");
	cl_sfx_footsteps[1] = S_RegisterSound("player/step1.wav");
	cl_sfx_footsteps[2] = S_RegisterSound("player/step2.wav");
	cl_sfx_footsteps[3] = S_RegisterSound("player/step3.wav");
	cl_sfx_footsteps_snow[0] = S_RegisterSound("player/step_snow0.wav");
	cl_sfx_footsteps_snow[1] = S_RegisterSound("player/step_snow1.wav");
	cl_sfx_footsteps_snow[2] = S_RegisterSound("player/step_snow2.wav");
	cl_sfx_footsteps_snow[3] = S_RegisterSound("player/step_snow3.wav");
	cl_sfx_footsteps_grass[0] = S_RegisterSound("player/step_grass0.wav");
	cl_sfx_footsteps_grass[1] = S_RegisterSound("player/step_grass1.wav");
	cl_sfx_footsteps_grass[2] = S_RegisterSound("player/step_grass2.wav");
	cl_sfx_footsteps_grass[3] = S_RegisterSound("player/step_grass3.wav");
	cl_sfx_footsteps_asphalt[0] = S_RegisterSound("player/step_asphalt0.wav");
	cl_sfx_footsteps_asphalt[1] = S_RegisterSound("player/step_asphalt1.wav");
	cl_sfx_footsteps_asphalt[2] = S_RegisterSound("player/step_asphalt2.wav");
	cl_sfx_footsteps_asphalt[3] = S_RegisterSound("player/step_asphalt3.wav");
	cl_sfx_footsteps_wood_plank[0] = S_RegisterSound("player/step_wood_plank0.wav");
	cl_sfx_footsteps_wood_plank[1] = S_RegisterSound("player/step_wood_plank1.wav");
	cl_sfx_footsteps_wood_plank[2] = S_RegisterSound("player/step_wood_plank2.wav");
	cl_sfx_footsteps_wood_plank[3] = S_RegisterSound("player/step_wood_plank3.wav");
	cl_sfx_footsteps_metal[0] = S_RegisterSound("player/step_metal0.wav");
	cl_sfx_footsteps_metal[1] = S_RegisterSound("player/step_metal1.wav");
	cl_sfx_footsteps_metal[2] = S_RegisterSound("player/step_metal2.wav");
	cl_sfx_footsteps_metal[3] = S_RegisterSound("player/step_metal3.wav");
	cl_sfx_footsteps_sand[0] = S_RegisterSound("player/step_sand0.wav");
	cl_sfx_footsteps_sand[1] = S_RegisterSound("player/step_sand1.wav");
	cl_sfx_footsteps_sand[2] = S_RegisterSound("player/step_sand2.wav");
	cl_sfx_footsteps_sand[3] = S_RegisterSound("player/step_sand3.wav");
	cl_sfx_footsteps_metalt[0] = S_RegisterSound("player/step_metalt0.wav");
	cl_sfx_footsteps_metalt[1] = S_RegisterSound("player/step_metalt1.wav");
	cl_sfx_footsteps_metalt[2] = S_RegisterSound("player/step_metalt2.wav");
	cl_sfx_footsteps_metalt[3] = S_RegisterSound("player/step_metalt3.wav");
	cl_sfx_footsteps_barrel[0] = S_RegisterSound("player/step_barrel0.wav");
	cl_sfx_footsteps_barrel[1] = S_RegisterSound("player/step_barrel1.wav");
	cl_sfx_footsteps_barrel[2] = S_RegisterSound("player/step_barrel2.wav");
	cl_sfx_footsteps_barrel[3] = S_RegisterSound("player/step_barrel3.wav");
}	

/*
=================
CL_RegisterTEntModels
=================
*/
void CL_RegisterTEntModels (void) // jit: changed to only load paintball stuff:
{
#ifdef QUAKE2
	cl_mod_explode = re.RegisterModel("models/objects/explode/tris.md2");
	cl_mod_smoke = re.RegisterModel("models/objects/smoke/tris.md2");
	cl_mod_flash = re.RegisterModel("models/objects/flash/tris.md2");
	cl_mod_parasite_segment = re.RegisterModel("models/monsters/parasite/segment/tris.md2");
	cl_mod_grapple_cable = re.RegisterModel("models/ctf/segment/tris.md2");
	cl_mod_parasite_tip = re.RegisterModel("models/monsters/parasite/tip/tris.md2");
	cl_mod_explo4 = re.RegisterModel("models/objects/r_explode/tris.md2");
	cl_mod_bfg_explo = re.RegisterModel("sprites/s_bfg2.sp2");
	cl_mod_powerscreen = re.RegisterModel("models/items/armor/effect/tris.md2");

	re.RegisterModel("models/objects/laser/tris.md2");
	re.RegisterModel("models/objects/grenade2/tris.md2");
	re.RegisterModel("models/weapons/v_machn/tris.md2");
	re.RegisterModel("models/weapons/v_handgr/tris.md2");
	re.RegisterModel("models/weapons/v_shotg2/tris.md2");
	re.RegisterModel("models/objects/gibs/bone/tris.md2");
	re.RegisterModel("models/objects/gibs/sm_meat/tris.md2");
	re.RegisterModel("models/objects/gibs/bone2/tris.md2");

	re.RegisterPic("w_machinegun");
	re.RegisterPic("a_bullets");
	re.RegisterPic("i_health");
	re.RegisterPic("a_grenades");

	//ROGUE
	cl_mod_explo4_big = re.RegisterModel("models/objects/r_explode2/tris.md2");
	cl_mod_lightning = re.RegisterModel("models/proj/lightning/tris.md2");
	cl_mod_heatbeam = re.RegisterModel("models/proj/beam/tris.md2");
	cl_mod_monster_heatbeam = re.RegisterModel("models/proj/widowbeam/tris.md2");
	//ROGUE
#else
	cl_mod_smoke = re.RegisterModel("sprites/smoke.sp2");
	cl_mod_splat = re.RegisterModel("models/paint/splat2.md2");
	cl_mod_paintball = re.RegisterModel("models/paint/ball.md2");
#endif
}

// jit -->

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
	memset(cl_beams, 0, sizeof(cl_beams));
	memset(cl_explosions, 0, sizeof(cl_explosions));
	memset(cl_lasers, 0, sizeof(cl_lasers));
	memset(cl_sustains, 0, sizeof(cl_sustains));
}

/*
=================
CL_AllocExplosion
=================
*/
explosion_t *CL_AllocExplosion (void)
{
	int		i;
	int		time;
	int		index;
	
	for (i = 0; i < MAX_EXPLOSIONS; i++)
	{
		if (cl_explosions[i].type == ex_free)
		{
			memset(&cl_explosions[i], 0, sizeof (cl_explosions[i]));
			return &cl_explosions[i];
		}
	}

	// find the oldest explosion
	time = cl.time;
	index = 0;

	for (i = 0; i < MAX_EXPLOSIONS; i++)
	{
		if (cl_explosions[i].start < time)
		{
			time = cl_explosions[i].start;
			index = i;
		}
	}

	memset(&cl_explosions[index], 0, sizeof (cl_explosions[index]));
	return &cl_explosions[index];
}

/*
=================
CL_SmokeAndFlash
=================
*/
void CL_SmokeAndFlash(vec3_t origin)
{
	explosion_t	*ex;

	ex = CL_AllocExplosion();
	VectorCopy(origin, ex->ent.origin);
	ex->type = ex_misc;
	ex->frames = 4;
	ex->ent.flags = RF_TRANSLUCENT;
	ex->start = cl.frame.servertime - 100;
	ex->ent.model = cl_mod_smoke;

	//ex = CL_AllocExplosion();
	//VectorCopy(origin, ex->ent.origin);
	//ex->type = ex_flash;
	//ex->ent.flags = RF_FULLBRIGHT;
	//ex->frames = 2;
	//ex->start = cl.frame.servertime - 100;
	//ex->ent.model = cl_mod_flash;
}

/*
=================
CL_ParseParticles
=================
*/
void CL_ParseParticles (void)
{
	int		color, count;
	vec3_t	pos, dir;

	MSG_ReadPos (&net_message, pos);
	MSG_ReadDir (&net_message, dir);

	color = MSG_ReadByte(&net_message);

	count = MSG_ReadByte(&net_message);

	CL_ParticleEffect (pos, dir, color, count);
}

/*
=================
CL_ParseBeam
=================
*/
int CL_ParseBeam (struct model_s *model)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort (&net_message);
	
	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return ent;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return ent;
		}
	}
	Com_Printf ("Beam list overflow!\n");	
	return ent;
}

/*
=================
CL_ParseBeam2
=================
*/
int CL_ParseBeam2 (struct model_s *model)
{
	int		ent;
	vec3_t	start, end, offset;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort (&net_message);
	
	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);
	MSG_ReadPos (&net_message, offset);

//	Com_Printf ("end- %f %f %f\n", end[0], end[1], end[2]);

// override any beam with the same entity

	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = model;
			b->endtime = cl.time + 200;	
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorCopy (offset, b->offset);
			return ent;
		}
	}
	Com_Printf ("Beam list overflow!\n");	
	return ent;
}


/*
=================
CL_ParseLightning
=================
*/
int CL_ParseLightning (struct model_s *model)
{
	int		srcEnt, destEnt;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	srcEnt = MSG_ReadShort (&net_message);
	destEnt = MSG_ReadShort (&net_message);

	MSG_ReadPos (&net_message, start);
	MSG_ReadPos (&net_message, end);

// override any beam with the same source AND destination entities
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == srcEnt && b->dest_entity == destEnt)
		{
//			Com_Printf("%d: OVERRIDE  %d -> %d\n", cl.time, srcEnt, destEnt);
			b->entity = srcEnt;
			b->dest_entity = destEnt;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return srcEnt;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
//			Com_Printf("%d: NORMAL  %d -> %d\n", cl.time, srcEnt, destEnt);
			b->entity = srcEnt;
			b->dest_entity = destEnt;
			b->model = model;
			b->endtime = cl.time + 200;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			VectorClear (b->offset);
			return srcEnt;
		}
	}
	Com_Printf ("Beam list overflow!\n");	
	return srcEnt;
}

/*
=================
CL_ParseLaser
=================
*/
void CL_ParseLaser (int colors)
{
	vec3_t	start;
	vec3_t	end;
	laser_t	*l;
	int		i;

	MSG_ReadPos(&net_message, start);
	MSG_ReadPos(&net_message, end);

	for (i = 0, l = cl_lasers; i < MAX_LASERS; ++i, ++l)
	{
		if (l->endtime < cl.time)
		{
			l->ent.flags = RF_TRANSLUCENT | RF_BEAM;
			VectorCopy(start, l->ent.origin);
			VectorCopy(end, l->ent.oldorigin);
			l->ent.alpha = 0.30f;
			l->ent.skinnum = (colors >> ((rand() % 4) * 8)) & 0xff;
			l->ent.model = NULL;
			l->ent.frame = 4;
			l->endtime = cl.time + 100;
			return;
		}
	}
}

//=============
//ROGUE
void CL_ParseSteam (void)
{
	vec3_t	pos, dir;
	int		id, i;
	int		r;
	int		cnt;
	int		color;
	int		magnitude;
	cl_sustain_t	*s, *free_sustain;

	id = MSG_ReadShort (&net_message);		// an id of -1 is an instant effect
	if (id != -1) // sustains
	{
//			Com_Printf ("Sustain effect id %d\n", id);
		free_sustain = NULL;
		for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
		{
			if (s->id == 0)
			{
				free_sustain = s;
				break;
			}
		}
		if (free_sustain)
		{
			s->id = id;
			s->count = MSG_ReadByte(&net_message);
			MSG_ReadPos (&net_message, s->org);
			MSG_ReadDir (&net_message, s->dir);
			r = MSG_ReadByte(&net_message);
			s->color = r & 0xff;
			s->magnitude = MSG_ReadShort (&net_message);
			s->endtime = cl.time + MSG_ReadLong (&net_message);
			s->think = CL_ParticleSteamEffect2;
			s->thinkinterval = 100;
			s->nextthink = cl.time;
		}
		else
		{
//				Com_Printf ("No free sustains!\n");
			// FIXME - read the stuff anyway
			cnt = MSG_ReadByte(&net_message);
			MSG_ReadPos (&net_message, pos);
			MSG_ReadDir (&net_message, dir);
			r = MSG_ReadByte(&net_message);
			magnitude = MSG_ReadShort (&net_message);
			magnitude = MSG_ReadLong (&net_message); // really interval
		}
	}
	else // instant
	{
		cnt = MSG_ReadByte(&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = MSG_ReadByte(&net_message);
		magnitude = MSG_ReadShort (&net_message);
		color = r & 0xff;
		CL_ParticleSteamEffect (pos, dir, color, cnt, magnitude);
//		S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
	}
}

void CL_ParseWidow (void)
{
	vec3_t	pos;
	int		id, i;
	cl_sustain_t	*s, *free_sustain;

	id = MSG_ReadShort (&net_message);

	free_sustain = NULL;
	for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
	{
		if (s->id == 0)
		{
			free_sustain = s;
			break;
		}
	}
	if (free_sustain)
	{
		s->id = id;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 2100;
		s->think = CL_Widowbeamout;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

void CL_ParseNuke (void)
{
	vec3_t	pos;
	int		i;
	cl_sustain_t	*s, *free_sustain;

	free_sustain = NULL;
	for (i=0, s=cl_sustains; i<MAX_SUSTAINS; i++, s++)
	{
		if (s->id == 0)
		{
			free_sustain = s;
			break;
		}
	}
	if (free_sustain)
	{
		s->id = 21000;
		MSG_ReadPos (&net_message, s->org);
		s->endtime = cl.time + 1000;
		s->think = CL_Nukeblast;
		s->thinkinterval = 1;
		s->nextthink = cl.time;
	}
	else // no free sustains
	{
		// FIXME - read the stuff anyway
		MSG_ReadPos (&net_message, pos);
	}
}

//ROGUE
//=============


/*
=================
CL_ParseTEnt
=================
*/
static byte splash_color[] = {0x00, 0xe0, 0xb0, 0x50, 0xd0, 0xe0, 0xe8};
extern cvar_t *splattime;
void CL_ParseTEnt (void)
{
	int		type;
	vec3_t	pos, pos2, dir;
	explosion_t	*ex;
	int		cnt;
	int		color;
	int		r;
	int		ent;
//	int		magnitude;
	vec3_t	rgbcolour;
#ifndef QUAKE2
	int		texnum; // splat texture index
#endif 

	type = MSG_ReadByte(&net_message);

	switch (type)
	{
	case TE_LASER_SPARKS: // jit - Paintball 2 paint splatters!
		cnt = MSG_ReadByte(&net_message);
		MSG_ReadPos(&net_message, pos);
		MSG_ReadDir(&net_message, dir);
		color = MSG_ReadByte(&net_message);
#ifdef QUAKE2
#else
		CL_ParticleEffect2(pos, dir, color, cnt);

		// ===
		// jit -- paint stains / paint gren splats
		if (color > 63 && color < 80)
		{	// red
			texnum = 0;
			rgbcolour[0] = 1.0f;
			rgbcolour[1] = 0.68f;
			rgbcolour[2] = 0.65f;
		}
		else if (color > 143 && color < 160)
		{	// purple
			texnum = 2;
			rgbcolour[0] = 0.94f;
			rgbcolour[1] = 0.71f;
			rgbcolour[2] = 1.0f;
		}
		else if (color > 191 && color < 208)
		{	// yellow
			texnum = 3;
			rgbcolour[0] = 1.0f;
			rgbcolour[1] = 1.0f;
			rgbcolour[2] = 0.21f;
		}
		else
		{	// blue
			texnum = 1;
			rgbcolour[0] = 0.75f;
			rgbcolour[1] = 0.78f;
			rgbcolour[2] = 1.00f;
		}
		
		V_AddStain(pos, rgbcolour, cnt / 3); // stain size depends on cnt
		
		if (cnt != 16 && cnt != 15) // grenade splatter
		{
			cnt = rand() & 7; // don't do it for every splatter!

			if (cnt == 0)
				S_StartSound(pos, 0, 0, cl_sfx_grensplat1, 1, ATTN_IDLE, 0);
			else if (cnt == 1)
				S_StartSound(pos, 0, 0, cl_sfx_grensplat2, 1, ATTN_IDLE, 0);
			else if (cnt == 2)
				S_StartSound(pos, 0, 0, cl_sfx_grensplat3, 1, ATTN_IDLE, 0);
		}
		else
		{
			struct sfx_s *sound;
			int n;
			trace_t tr;
			vec3_t end, start, back, forward, entpos;
			surface_sound_type_t surface_sound = SURFACE_SOUND_UNKNOWN;
			qboolean make_splat = (cnt == 16);

			// check the closest wall for an exact angle
			VectorScale(dir, 1.0f, back);
			VectorScale(dir, -8.0f, forward);
			VectorAdd(pos, back, start);
			VectorAdd(pos, forward, end);
			tr = CM_BoxTrace(start, end, vec3_origin, vec3_origin, 0, MASK_SOLID);

			if (tr.fraction < 1.0f)
			{
				VectorCopy(tr.plane.normal, dir);
				surface_sound = tr.surface->surface_sound;

				// Set the entity pos to be 0.1 units away from the surface (to avoid any potential floating point issues/z fighting, etc.
				VectorScale(dir, 0.1f, back);
				VectorAdd(tr.endpos, back, entpos);
			}
			else
			{
				VectorCopy(pos, entpos);
			}

			// Generate splat model
			if (make_splat)
			{
				ex = CL_AllocExplosion();
				VectorCopy(entpos, ex->ent.origin);
				VectorCopy(entpos, ex->ent.startorigin);

				// Convert vector to angles
				ex->ent.angles[0] = (float)acos(dir[2]) / (float)M_PI * 180.0f;

				if (dir[0])
					ex->ent.angles[1] = atan2(dir[1], dir[0]) / (float)M_PI * 180.0f;
				else if (dir[1] > 0)
					ex->ent.angles[1] = 90.0f;
				else if (dir[1] < 0)
					ex->ent.angles[1] = 270.0f;
				else
					ex->ent.angles[1] = 0.0f;

				ex->type = ex_misc;
				ex->ent.flags = RF_TRANSLUCENT;
				ex->ent.alpha = 1.0f;
				ex->ent.alphavel = -1.0f / (splattime->value + frand()*.5);
				ex->start = (float)cl.frame.servertime - 100.0f;
				ex->ent.skinnum = texnum;
				ex->ent.scale = ex->ent.startscale = (0.7f + frand() * 0.6f);
				ex->frames = 1; // doesn't matter
				ex->ent.model = cl_mod_splat;
			}

			// randomly choose 1 of 2 splat sounds:
			if (frand() > 0.5f)
				n = 1;
			else
				n = 0;

			switch (surface_sound)
			{
			case SURFACE_SOUND_METAL_BARREL:
				sound = cl_sfx_splat_barrel[n];
				break;
			case SURFACE_SOUND_GRASS:
				sound = cl_sfx_splat_grass[n];
				break;
			case SURFACE_SOUND_METAL_THIN:
				sound = cl_sfx_splat_metal_thin[n];
				break;
			case SURFACE_SOUND_METAL_THICK:
			case SURFACE_SOUND_METAL_OTHER:
				sound = cl_sfx_splat_metal_thick[n];
				break;
			case SURFACE_SOUND_BRICK:
			case SURFACE_SOUND_GLASS:
				sound = cl_sfx_splat_brick[n];
				break;
			case SURFACE_SOUND_DIRT:
			case SURFACE_SOUND_SAND:
			case SURFACE_SOUND_SNOW:
				sound = cl_sfx_splat_dirt[n];
				break;
			case SURFACE_SOUND_WOOD_PLANK:
			case SURFACE_SOUND_WOOD_OTHER:
				sound = cl_sfx_splat_wood_plank[n];
				break;
			case SURFACE_SOUND_WOOD_SOLID:
			case SURFACE_SOUND_TREE:
				sound = cl_sfx_splat_wood_solid[n];
				break;
			case SURFACE_SOUND_DEFAULT:
			case SURFACE_SOUND_UNKNOWN:
			case SURFACE_SOUND_ROCK:
			case SURFACE_SOUND_CEMENT:
			default:
				sound = cl_sfx_splat[n];
			}

			S_StartSound(pos, 0, 0, sound, 1, ATTN_NORM, 0);
		}
#endif
		// jit
		// ===
		break;

	case TE_CHAINFIST_SMOKE: // jit -- paintball2 smoke grenade puffs!
		MSG_ReadPos(&net_message, pos);

		for(r = 0; r < 3; r++)
		{
			ex = CL_AllocExplosion();
			VectorCopy(pos, ex->ent.origin);
			VectorCopy(pos, ex->ent.startorigin);
			ex->ent.origin[2] += 10;
			ex->type = ex_poly;
			ex->ent.flags = RF_TRANSLUCENT;
			ex->start = cl.frame.servertime - 100;
			ex->light = 0;
			ex->lightcolor[0] = 0.0;
			ex->lightcolor[1] = 0.0;
			ex->lightcolor[2] = 0.0;
			ex->ent.alpha = 2.0;
			ex->ent.alphavel = -0.3 / (0.8 + frand()*0.19);
			ex->ent.scale = ex->ent.startscale = 0.5;
			ex->ent.scalevel = 0.3 / (0.5 + frand()*0.3);
			ex->ent.vel[0] = frand()*150.0f - 75.0f;
			ex->ent.vel[1] = frand()*150.0f - 75.0f;
			ex->ent.vel[2] = frand()*20.0f+10.0f;
			ex->ent.accel[0] = 0.0f - ex->ent.vel[0]*0.16f;
			ex->ent.accel[1] = 0.0f - ex->ent.vel[1]*0.16f;
			ex->frames = 1;
			ex->ent.model = cl_mod_smoke;
		}

		break;

	// old crap we can probably get rid of:

	case TE_BLOOD:			// bullet hitting flesh
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		CL_ParticleEffect (pos, dir, 0xe8, 60);

		rgbcolour[0] = 0.97f;
		rgbcolour[1] = 0.51f;
		rgbcolour[2] = 0.51f;
		V_AddStain(pos, rgbcolour, 30);
		break;

	case TE_GUNSHOT:			// bullet hitting wall
	case TE_SPARKS:
	case TE_BULLET_SPARKS:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		rgbcolour[0] = 0.89;
		rgbcolour[1] = 0.89;
		rgbcolour[2] = 0.89;
		V_AddStain(pos, rgbcolour, 5);

		if (type == TE_GUNSHOT)
			CL_ParticleEffect (pos, dir, 0, 40);
		else
			CL_ParticleEffect (pos, dir, 0xe0, 6);

		break;
		
	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		if (type == TE_SCREEN_SPARKS)
			CL_ParticleEffect (pos, dir, 0xd0, 40);
		else
			CL_ParticleEffect (pos, dir, 0xb0, 40);
		break;
		
	case TE_SHOTGUN:			// bullet hitting wall
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);

		rgbcolour[0] = 0.89;
		rgbcolour[1] = 0.89;
		rgbcolour[2] = 0.89;
		V_AddStain(pos, rgbcolour, 5);

		CL_ParticleEffect (pos, dir, 0, 20);
		CL_SmokeAndFlash(pos);
		break;

	case TE_SPLASH:			// bullet hitting water
		cnt = MSG_ReadByte(&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		r = MSG_ReadByte(&net_message);
		if (r > 6)
			color = 0x00;
		else
			color = splash_color[r];
		CL_ParticleEffect (pos, dir, color, cnt);

		break;

	case TE_BLASTER:			// blaster hitting wall
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		CL_BlasterParticles (pos, dir);

		rgbcolour[0] = 1.1;
		rgbcolour[1] = 1.1;
		rgbcolour[2] = 0;
		V_AddStain(pos, rgbcolour, 10);

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->ent.angles[0] = acos(dir[2])/M_PI*180;
	// PMM - fixed to correct for pitch of 0
		if (dir[0])
			ex->ent.angles[1] = atan2(dir[1], dir[0])/M_PI*180;
		else if (dir[1] > 0)
			ex->ent.angles[1] = 90;
		else if (dir[1] < 0)
			ex->ent.angles[1] = 270;
		else
			ex->ent.angles[1] = 0;

		ex->type = ex_misc;
		ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 150;
		ex->lightcolor[0] = 1;
		ex->lightcolor[1] = 1;
		ex->ent.model = cl_mod_smoke;
		ex->frames = 4;
		break;
		
	case TE_RAILTRAIL:			// railgun effect
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_RailTrail (pos, pos2);
		break;

	case TE_EXPLOSION2:
	case TE_GRENADE_EXPLOSION:
	case TE_GRENADE_EXPLOSION_WATER:
		MSG_ReadPos (&net_message, pos);

		rgbcolour[0] = 0.8;
		rgbcolour[1] = 0.8;
		rgbcolour[2] = 0.8;
		V_AddStain(pos, rgbcolour, 35);

		ex = CL_AllocExplosion();
		VectorCopy(pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.model = cl_mod_smoke;
		ex->frames = 19;
		ex->baseframe = 30;
		ex->ent.angles[1] = rand() % 360;
		CL_ExplosionParticles(pos);

#ifdef QUAKE2
		ex->ent.angles[1] = rand() % 360;

		if (type == TE_GRENADE_EXPLOSION_WATER)
			S_StartSound(pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
		else
			S_StartSound(pos, 0, 0, cl_sfx_grenexp, 1, ATTN_NORM, 0);
#endif
		break;

	// RAFAEL

	
	case TE_EXPLOSION1:
	case TE_EXPLOSION1_BIG:						// PMM
	case TE_ROCKET_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
	case TE_EXPLOSION1_NP:						// PMM
		MSG_ReadPos (&net_message, pos);
		rgbcolour[0] = 0.8;
		rgbcolour[1] = 0.8;
		rgbcolour[2] = 0.8;
		V_AddStain(pos, rgbcolour, 35);
		ex = CL_AllocExplosion();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.angles[1] = rand() % 360;
		ex->ent.model = cl_mod_smoke;

		if (frand() < 0.5)
			ex->baseframe = 15;

		ex->frames = 15;

		if ((type != TE_EXPLOSION1_BIG) && (type != TE_EXPLOSION1_NP))		// PMM
			CL_ExplosionParticles(pos);									// PMM

#ifdef QUAKE2
		if (type != TE_EXPLOSION1_BIG)				// PMM
			ex->ent.model = cl_mod_explo4;			// PMM
		else
			ex->ent.model = cl_mod_explo4_big;

		if (type == TE_ROCKET_EXPLOSION_WATER)
			S_StartSound (pos, 0, 0, cl_sfx_watrexp, 1, ATTN_NORM, 0);
		else
			S_StartSound (pos, 0, 0, cl_sfx_rockexp, 1, ATTN_NORM, 0);
#endif
		break;

	case TE_BFG_EXPLOSION:
		MSG_ReadPos (&net_message, pos);
		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 0.0;
		ex->lightcolor[1] = 1.0;
		ex->lightcolor[2] = 0.0;
		ex->ent.model = cl_mod_smoke;
		ex->ent.flags |= RF_TRANSLUCENT;
		ex->ent.alpha = 0.30;
		ex->frames = 4;
		break;

	case TE_BFG_BIGEXPLOSION:
		MSG_ReadPos (&net_message, pos);
		CL_BFGExplosionParticles (pos);
		break;

	case TE_BFG_LASER:
		CL_ParseLaser (0xd0d1d2d3);
		break;

	case TE_BUBBLETRAIL:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_BubbleTrail (pos, pos2);
		break;

	case TE_MEDIC_CABLE_ATTACK:
//		ent = CL_ParseBeam (cl_mod_parasite_segment);
		break;

	// RAFAEL
	case TE_WELDING_SPARKS:
		cnt = MSG_ReadByte(&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		color = MSG_ReadByte(&net_message);
		CL_ParticleEffect2 (pos, dir, color, cnt);

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_flash;
		// note to self
		// we need a better no draw flag
		ex->ent.flags = RF_BEAM;
		ex->start = cl.frame.servertime - 0.1;
		ex->light = 100 + (rand()%75);
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 1.0;
		ex->lightcolor[2] = 0.3;
		ex->ent.model = cl_mod_smoke;
		ex->frames = 2;
		break;

	// RAFAEL
	case TE_TUNNEL_SPARKS:
		cnt = MSG_ReadByte(&net_message);
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		color = MSG_ReadByte(&net_message);
		CL_ParticleEffect3 (pos, dir, color, cnt);
		break;

//=============
//PGM
		// PMM -following code integrated for flechette (different color)
	case TE_BLASTER2:			// green blaster hitting wall
	case TE_FLECHETTE:			// flechette
		MSG_ReadPos (&net_message, pos);
		MSG_ReadDir (&net_message, dir);
		
		// PMM
		if (type == TE_BLASTER2)
			CL_BlasterParticles2 (pos, dir, 0xd0);
		else
			CL_BlasterParticles2 (pos, dir, 0x6f); // 75

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->ent.angles[0] = acos(dir[2])/M_PI*180;
	// PMM - fixed to correct for pitch of 0
		if (dir[0])
			ex->ent.angles[1] = atan2(dir[1], dir[0])/M_PI*180;
		else if (dir[1] > 0)
			ex->ent.angles[1] = 90;
		else if (dir[1] < 0)
			ex->ent.angles[1] = 270;
		else
			ex->ent.angles[1] = 0;

		ex->type = ex_misc;
		ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;

		// PMM
		if (type == TE_BLASTER2)
			ex->ent.skinnum = 1;
		else // flechette
			ex->ent.skinnum = 2;

		ex->start = cl.frame.servertime - 100;
		ex->light = 150;
		// PMM
		if (type == TE_BLASTER2)
			ex->lightcolor[1] = 1;
		else // flechette
		{
			ex->lightcolor[0] = 0.19;
			ex->lightcolor[1] = 0.41;
			ex->lightcolor[2] = 0.75;
		}
		ex->ent.model = cl_mod_smoke;
		ex->frames = 4;
		break;

	case TE_DEBUGTRAIL:
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_DebugTrail (pos, pos2);
		break;

	case TE_PLAIN_EXPLOSION:
		MSG_ReadPos (&net_message, pos);

		rgbcolour[0] = 0.8;
		rgbcolour[1] = 0.8;
		rgbcolour[2] = 0.8;
		V_AddStain(pos, rgbcolour, 35);

		ex = CL_AllocExplosion ();
		VectorCopy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = cl.frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.angles[1] = rand() % 360;
		ex->ent.model = cl_mod_smoke;
		if (frand() < 0.5)
			ex->baseframe = 15;
		ex->frames = 15;
		break;

	case TE_FLASHLIGHT:
		MSG_ReadPos(&net_message, pos);
		ent = MSG_ReadShort(&net_message);
		CL_Flashlight(ent, pos);
		break;

	case TE_FORCEWALL:
		MSG_ReadPos(&net_message, pos);
		MSG_ReadPos(&net_message, pos2);
		color = MSG_ReadByte(&net_message);
		CL_ForceWall(pos, pos2, color);
		break;

	case TE_STEAM:
		CL_ParseSteam();
		break;

	case TE_BUBBLETRAIL2:
		cnt = 8;
		MSG_ReadPos (&net_message, pos);
		MSG_ReadPos (&net_message, pos2);
		CL_BubbleTrail2 (pos, pos2, cnt);
		break;

	case TE_TELEPORT_EFFECT:
	case TE_DBALL_GOAL:
		MSG_ReadPos (&net_message, pos);
		CL_TeleportParticles (pos);
		break;

	case TE_WIDOWBEAMOUT:
		CL_ParseWidow ();
		break;

	case TE_NUKEBLAST:
		CL_ParseNuke ();
		break;

	case TE_WIDOWSPLASH:
		MSG_ReadPos (&net_message, pos);
		CL_WidowSplash (pos);
		break;
//PGM
//==============

	default:
		Com_Error (ERR_DROP, "CL_ParseTEnt: bad type");
	}
}

/*
=================
CL_AddBeams
=================
*/
void CL_AddBeams (void)
{
	int			i,j;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	ent;
	float		yaw, pitch;
	float		forward;
	float		len, steps;
	float		model_length;
	
// update beams
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

		// if coming from the player, update the start position
		if (b->entity == cl.playernum+1)	// entity 0 is the world
		{
			VectorCopy (cl.refdef.vieworg, b->start);
			b->start[2] -= 22;	// adjust for view height
		}
		VectorAdd (b->start, b->offset, org);

	// calculate pitch and yaw
		VectorSubtract (b->end, org, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
	// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0;
		}

	// add new entities for the beams
		d = VectorNormalizeRetLen(dist);
		memset(&ent, 0, sizeof(ent));
/*		if (b->model == cl_mod_lightning)
		{
			model_length = 35.0;
			d-= 20.0;  // correction so it doesn't end in middle of tesla
		}
		else*/
		{
			model_length = 30.0;
		}
		steps = ceil(d/model_length);
		len = (d-model_length)/(steps-1);

		// PMM - special case for lightning model .. if the real length is shorter than the model,
		// flip it around & draw it from the end to the start.  This prevents the model from going
		// through the tesla mine (instead it goes through the target)
/*		if ((b->model == cl_mod_lightning) && (d <= model_length))
		{
//			Com_Printf ("special case\n");
			VectorCopy (b->end, ent.origin);
			// offset to push beam outside of tesla model (negative because dist is from end to start
			// for this beam)
//			for (j=0 ; j<3 ; j++)
//				ent.origin[j] -= dist[j]*10.0;
			ent.model = b->model;
			ent.flags = RF_FULLBRIGHT;
			ent.angles[0] = pitch;
			ent.angles[1] = yaw;
			ent.angles[2] = rand()%360;
			V_AddEntity (&ent);			
			return;
		}*/
		while (d > 0)
		{
			VectorCopy (org, ent.origin);
			ent.model = b->model;
/*			if (b->model == cl_mod_lightning)
			{
				ent.flags = RF_FULLBRIGHT;
				ent.angles[0] = -pitch;
				ent.angles[1] = yaw + 180.0;
				ent.angles[2] = rand()%360;
			}
			else*/
			{
				ent.angles[0] = pitch;
				ent.angles[1] = yaw;
				ent.angles[2] = rand()%360;
			}
			
//			Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
			V_AddEntity (&ent);

			for (j=0 ; j<3 ; j++)
				org[j] += dist[j]*len;
			d -= model_length;
		}
	}
}


/*
=================
CL_AddExplosions
=================
*/
void CL_AddExplosions (void) // jitsmoke
{
	explosion_t	*ex;
	entity_t	*ent;
	float		time;
	int			i;
	int			j;

	for (i = 0, ex = cl_explosions; i < MAX_EXPLOSIONS; ++i, ++ex)
	{
		time = (cl.time - ex->start) * 0.001f;
		ent = &ex->ent;

		if (ex->type != ex_free)
		{
			if (ent->alpha <= 0.0f)
			{
				ex->type = ex_free;
				continue;
			}
			else
			{
				ent->alpha = 1.0f + time * ent->alphavel;
				ent->scale = ent->startscale + time*ent->scalevel;

				for (j = 0; j < 3; ++j)
					ent->origin[j] = ent->startorigin[j] + (time * ent->vel[j]) + (time * time * ent->accel[j]);

				VectorCopy(ent->origin, ent->oldorigin);
			}

			V_AddEntity(ent);
		}
	}
}


/*
=================
CL_AddLasers
=================
*/
void CL_AddLasers (void)
{
	laser_t		*l;
	int			i;

	for (i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++)
	{
		if (l->endtime >= cl.time)
			V_AddEntity (&l->ent);
	}
}

/* PMM - CL_Sustains */
void CL_ProcessSustain ()
{
	cl_sustain_t	*s;
	int				i;

	for (i=0, s=cl_sustains; i< MAX_SUSTAINS; i++, s++)
	{
		if (s->id)
			if ((s->endtime >= cl.time) && (cl.time >= s->nextthink))
			{
//				Com_Printf ("think %d %d %d\n", cl.time, s->nextthink, s->thinkinterval);
				s->think (s);
			}
			else if (s->endtime < cl.time)
				s->id = 0;
	}
}

/*
=================
CL_AddTEnts
=================
*/
void CL_AddTEnts (void)
{
	CL_AddBeams();
	CL_AddExplosions(); // jit, actually they're "smoke explosions" now :)
	CL_AddLasers();
	// PMM - set up sustain
	CL_ProcessSustain();
}
