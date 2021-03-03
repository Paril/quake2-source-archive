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

// gl_script.c - scripted texture rendering - MrG

#include "gl_local.h"

void CIN_FreeCin (int texnum);

extern float	r_turbsin[];

#define		TURBSCALE (256.0 / (2 * M_PI))

#define		TOK_DELIMINATORS "\r\n\t "

float		rs_realtime = 0;
rscript_t	*rs_rootscript = NULL;

void RS_ListScripts (void)
{
	rscript_t	*rs;
	int			i;

	for (i = 0, rs = rs_rootscript; rs; rs = rs->next, i++) {
		ri.Con_Printf (PRINT_ALL, ">> %s\n", rs->name);
	}
}

int RS_Animate (rs_stage_t *stage)
{
	anim_stage_t	*anim = stage->last_anim;
	float			time = rs_realtime * 1000 - 
		(stage->last_anim_time + stage->anim_delay);

	while (stage->last_anim_time < rs_realtime) {
		anim = anim->next;
		if (!anim)
			anim = stage->anim_stage;
		stage->last_anim_time += stage->anim_delay;
	}

	stage->last_anim = anim;

	return anim->texture->texnum;
}

image_t *RS_Animate_image (rs_stage_t *stage) // jitrscript
{
	anim_stage_t *anim;
	float time;
	
	anim = stage->last_anim;
	time = rs_realtime * 1000 - (stage->last_anim_time + stage->anim_delay);

	
	while (stage->last_anim_time < rs_realtime)
	{
		anim = anim->next;
	
		if (!anim)
			anim = stage->anim_stage;

		stage->last_anim_time += stage->anim_delay;
	}

	stage->last_anim = anim;

	return anim->texture;
}

void RS_ResetScript (rscript_t *rs)
{
	rs_stage_t		*stage = rs->stage, *tmp_stage;
	anim_stage_t	*anim, *tmp_anim;
	
	if (rs->img_ptr) // jitrscript
		rs->img_ptr->rscript = NULL;

	rs->name[0] = 0;
	
	while (stage != NULL)
	{
		if (stage->anim_count)
		{
			anim = stage->anim_stage;
			while (anim != NULL)
			{
				tmp_anim = anim;

				if (anim->texture)
					if (anim->texture->is_cin)
						CIN_FreeCin(anim->texture->texnum);

				anim = anim->next;
				free(tmp_anim);
			}
		}

		tmp_stage = stage;
		stage = stage->next;
		free(tmp_stage);
	}

	/*rs->mirror = false;
	rs->stage = NULL;
	rs->dontflush = false;
	rs->subdivide = 0;
	rs->warpdist = 0.0f;
	rs->warpsmooth = 0.0f;
	rs->ready = false;*/
	memset(rs, 0, sizeof(rscript_t)); // jitrscript -- make sure we clear everything out.
}


rscript_t *RS_NewScript (char *name)
{
	rscript_t	*rs;

	if (!rs_rootscript)
	{
		rs_rootscript = (rscript_t *)malloc(sizeof(rscript_t));
		rs = rs_rootscript;
	}
	else
	{
		rs = rs_rootscript;

		while (rs->next != NULL)
			rs = rs->next;

		rs->next = (rscript_t *)malloc(sizeof(rscript_t));
		rs = rs->next;
	}

	memset(rs, 0, sizeof(rscript_t)); // jitrscript
	Q_strncpyz(rs->name, name, sizeof(rs->name));
/*
	rs->stage = NULL;
	rs->next = NULL;
	rs->dontflush = false;
	rs->subdivide = 0;
	rs->warpdist = 0.0f;
	rs->warpsmooth = 0.0f;
	rs->ready = false;
	rs->mirror = false;
	rs->img_ptr = NULL; // jitrscript
	rs->width = rs->height = 0; // jitrscript
*/
	return rs;
}

rs_stage_t *RS_NewStage (rscript_t *rs)
{
	rs_stage_t	*stage;

	if (rs->stage == NULL) {
		rs->stage = (rs_stage_t *)malloc(sizeof(rs_stage_t));
		stage = rs->stage;
	} else {
		stage = rs->stage;
		while (stage->next != NULL)
			stage = stage->next;
		stage->next = (rs_stage_t *)malloc(sizeof(rs_stage_t));
		stage = stage->next;
	}

	///*jit stage->anim_stage = NULL;
	//stage->next = NULL;
	//stage->last_anim = NULL;

	//RS_ClearStage (stage); 
	//*/

	// jitrscript:
	memset(stage, 0, sizeof(rs_stage_t)); // clear EVERYTHING
	stage->lightmap = true;
	Q_strncpyz(stage->name, "pics/noimage", sizeof(stage->name));

	return stage;
}

void RS_FreeAllScripts (void)
{
	rscript_t	*rs = rs_rootscript, *tmp_rs;

	while (rs != NULL)
	{
		tmp_rs = rs->next;
		RS_ResetScript(rs);
		free(rs);
		rs = tmp_rs;
	}

	rs_rootscript = NULL; // jitrscript
}

void RS_FreeScript(rscript_t *rs)
{
	rscript_t	*tmp_rs;

	if (!rs)
		return;

	if (rs_rootscript == rs)
	{
		rs_rootscript = rs_rootscript->next;
		RS_ResetScript(rs);
		free(rs);
		return;
	}

	tmp_rs = rs_rootscript;

	while (tmp_rs->next != rs)
		tmp_rs = tmp_rs->next;

	tmp_rs->next = rs->next;

	RS_ResetScript(rs);
	free(rs);
}

void RS_FreeUnmarked (void)
{
	rscript_t	*rs = rs_rootscript, *tmp_rs;

	while (rs != NULL) {
		tmp_rs = rs->next;

		//if (!rs->dontflush)
		if (!rs->dontflush && !rs->img_ptr) // jitrscript
			RS_FreeScript(rs);

		rs = tmp_rs;
	}
}

rscript_t *RS_FindScript(const char *name)
{
	rscript_t	*rs = rs_rootscript;

	while (rs != NULL)
	{
		if (Q_strcaseeq(rs->name, name))
			return rs;

		rs = rs->next;
	}

	return NULL;
}

void RS_ReadyScript (rscript_t *rs)
{
	rs_stage_t		*stage;
	anim_stage_t	*anim;
	char			mode;

	if (rs->ready)
		return;

#ifdef DEBUG
	{
		int err;

		err = qgl.GetError();
		assert(err == GL_NO_ERROR);
	}
#endif

	stage = rs->stage;
	mode = stage->sharp ? it_sharppic : (rs->dontflush) ? it_pic : it_wall;

	while (stage != NULL)
	{
		anim = stage->anim_stage;

		while (anim != NULL)
		{
			anim->texture = GL_FindImage(anim->name, mode);

			if (!anim->texture)
				anim->texture = r_notexture;

			anim = anim->next;
		}

		if (stage->name[0])
		{
			stage->texture = GL_FindImage(stage->name, mode);

			if (!stage->texture)
				stage->texture = r_notexture;
		}

		stage = stage->next;
	}

	rs->ready = true;
}

void RS_UpdateRegistration (void)
{
	rscript_t		*rs = rs_rootscript;
	rs_stage_t		*stage;
	anim_stage_t	*anim;
	int				mode;

	while (rs && (stage = rs->stage)) // jitrscript, make sure stage exists
	{
		mode = stage->sharp ? it_sharppic : (rs->dontflush) ? it_pic : it_wall; // jitrscript

		while (stage != NULL)
		{
			anim = stage->anim_stage;

			while (anim != NULL)
			{
				anim->texture = GL_FindImage(anim->name, mode);

				if (!anim->texture)
					anim->texture = r_notexture;

				anim = anim->next;
			}

			if (stage->texture)
			{
				stage->texture = GL_FindImage(stage->name, mode);

				if (!stage->texture)
					stage->texture = r_notexture;
			}

			stage = stage->next;
		}

		rs = rs->next;
	}
}

static int RS_BlendID (char *blend)
{
	if (!blend[0])
		return 0;
	if (Q_strcaseeq(blend, "GL_ZERO"))
		return GL_ZERO;
	if (Q_strcaseeq(blend, "GL_ONE"))
		return GL_ONE;
	if (Q_strcaseeq(blend, "GL_DST_COLOR"))
		return GL_DST_COLOR;
	if (Q_strcaseeq(blend, "GL_ONE_MINUS_DST_COLOR"))
		return GL_ONE_MINUS_DST_COLOR;
	if (Q_strcaseeq(blend, "GL_SRC_ALPHA"))
		return GL_SRC_ALPHA;
	if (Q_strcaseeq(blend, "GL_ONE_MINUS_SRC_ALPHA"))
		return GL_ONE_MINUS_SRC_ALPHA;
	if (Q_strcaseeq(blend, "GL_DST_ALPHA"))
		return GL_DST_ALPHA;
	if (Q_strcaseeq(blend, "GL_ONE_MINUS_DST_ALPHA"))
		return GL_ONE_MINUS_DST_ALPHA;
	if (Q_strcaseeq(blend, "GL_SRC_ALPHA_SATURATE"))
		return GL_SRC_ALPHA_SATURATE;
	if (Q_strcaseeq(blend, "GL_SRC_COLOR"))
		return GL_SRC_COLOR;
	if (Q_strcaseeq(blend, "GL_ONE_MINUS_SRC_COLOR"))
		return GL_ONE_MINUS_SRC_COLOR;

	return 0;
}

static int RS_FuncName (char *text)
{
	if (Q_strcaseeq(text, "static"))			// static
		return RSCRIPT_STATIC; // jitrscript (defines)
	else if (Q_strcaseeq(text, "sine"))		// sine wave
		return RSCRIPT_SINE;
	else if (Q_strcaseeq(text, "cosine"))	// cosine wave
		return RSCRIPT_COSINE;
	else if (Q_strcaseeq(text, "sinabs"))		// sine wave, only positive - jitrscript
		return RSCRIPT_SINABS;
	else if (Q_strcaseeq(text, "cosabs"))		// cosine wave, only positive - jitrscript
		return RSCRIPT_COSABS;

	return 0;
}

static char *RS_ParseString (char **ptr) // jitrscript - from qfusion
{
	char *token;

	if (!ptr || !(*ptr))
		return "";

	if (!**ptr || **ptr == '}')
		return "";

	//token = COM_ParseExt(ptr, false);
	token = strtok(NULL, TOK_DELIMINATORS);
	strlwr(token);

	return token;
}

static float RS_ParseFloat (char **ptr) // jitrscript -- from qfusion
{
	if (!ptr || !(*ptr))
	{
		return 0.0f;
	}

	if (!**ptr || **ptr == '}')
	{
		return 0.0f;
	}

	//return atof(COM_ParseExt(ptr, false));
	return atof(strtok(NULL, TOK_DELIMINATORS));
}

static void RS_ParseVector4 (char **ptr, vec4_t v) // jitrscript - from qfusion
{
	char *token;
	qboolean bracket;

	token = RS_ParseString(ptr);
	if (Q_strcaseeq(token, "("))
	{
		bracket = true;
		token = RS_ParseString(ptr);
	}
	else if (token[0] == '(')
	{
		bracket = true;
		token = &token[1];
	}
	else
	{
		bracket = false;
	}

	v[0] = atof(token);
	v[1] = RS_ParseFloat(ptr);
	v[2] = RS_ParseFloat(ptr);
	v[3] = 0;

	/*token = RS_ParseString(ptr);

	if (!token[0])
	{
		v[3] = 0;
	}
	else if (token[strlen(token)-1] == ')')
	{
		token[strlen(token)-1] = 0;
		v[3] = atof(token);
	}
	else
	{
		v[3] = atof(token);

		if (bracket) 
		{
			RS_ParseString(ptr);
		}
	}*/
}

/*
scriptname
{
	subdivide <size>
	vertexwarp <speed> <distance> <smoothness>
	safe
	{
		map <texturename>
		scroll <xtype> <xspeed> <ytype> <yspeed>
		blendfunc <source> <dest>
		alphashift <speed> <min> <max>
		anim <delay> <tex1> <tex2> <tex3> ... end
		envmap
		nolightmap
		alphamask
	}
}
*/

static void rs_stage_map (rs_stage_t *stage, char **token)
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	Q_strncpyz(stage->name, *token, sizeof(stage->name));
}

static void rs_stage_scroll (rs_stage_t *stage, char **token)
{
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scroll.typeX = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scroll.speedX = atof(*token);
	
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scroll.typeY = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scroll.speedY = atof(*token);
}

static void rs_stage_blendfunc (rs_stage_t *stage, char **token)
{
	stage->blendfunc.blend = true;

	*token = strtok (NULL, TOK_DELIMINATORS);

	if (Q_strcaseeq(*token, "add"))
	{
		stage->blendfunc.source = GL_ONE;
		stage->blendfunc.dest = GL_ONE;
	}
	else if (Q_strcaseeq(*token, "blend"))
	{
		stage->blendfunc.source = GL_SRC_ALPHA;
		stage->blendfunc.dest = GL_ONE_MINUS_SRC_ALPHA;
	}
	else if (Q_strcaseeq(*token, "filter"))
	{
		stage->blendfunc.source = GL_ZERO;
		stage->blendfunc.dest = GL_SRC_COLOR;
	}
	else
	{
		stage->blendfunc.source = RS_BlendID (*token);

		*token = strtok (NULL, TOK_DELIMINATORS);
		stage->blendfunc.dest = RS_BlendID (*token);
	}
}

static void rs_stage_alphashift (rs_stage_t *stage, char **token)
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	stage->alphashift.speed = (float)atof(*token);

	*token = strtok(NULL, TOK_DELIMINATORS);
	stage->alphashift.min = (float)atof(*token);

	*token = strtok(NULL, TOK_DELIMINATORS);
	stage->alphashift.max = (float)atof(*token);
}

static void rs_stage_anim (rs_stage_t *stage, char **token)
{
	anim_stage_t	*anim = (anim_stage_t *)malloc(sizeof(anim_stage_t));

	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->anim_delay = (float)atof(*token);

	stage->anim_stage = anim;
	stage->last_anim = anim;

	*token = strtok(NULL, TOK_DELIMINATORS);
	
	while (Q_strcasecmp(*token, "end"))
	{
		stage->anim_count++;
		Q_strncpyz(anim->name, *token, sizeof(anim->name));
		anim->texture = NULL;
		*token = strtok(NULL, TOK_DELIMINATORS);

		if (Q_strcaseeq(*token, "end"))
		{
			anim->next = NULL;
			break;
		}

		anim->next = (anim_stage_t *)malloc(sizeof(anim_stage_t));
		anim = anim->next;
	}
}

static void rs_stage_envmap (rs_stage_t *stage, char **token)
{
	stage->tcGen = TC_GEN_ENVIRONMENT;
}

static void rs_stage_nolightmap (rs_stage_t *stage, char **token)
{
	stage->lightmap = false;
}

static void rs_stage_alphamask (rs_stage_t *stage, char **token)
{
	stage->alphamask = true;
}

static void rs_stage_rotate (rs_stage_t *stage, char **token)
{
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->rot_speed = (float)atof(*token);
}

// scaleadd adds to the normal scale offering more dynamic options to scaling.
static void rs_stage_scaleadd (rs_stage_t *stage, char **token) // jitrscript
{
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scaleadd.typeX = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scaleadd.scaleX = atof(*token);
	
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scaleadd.typeY = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scaleadd.scaleY = atof(*token);
}

static void rs_stage_scale (rs_stage_t *stage, char **token)
{
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scale.typeX = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scale.scaleX = atof(*token);
	
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scale.typeY = RS_FuncName(*token);
	*token = strtok (NULL, TOK_DELIMINATORS);
	stage->scale.scaleY = atof(*token);
}

static void rs_stage_tcGen (rs_stage_t *stage, char **token) // jitrscript
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	
	// jitrscript -- make compatible with q3 shaders - jitodo, tcMod
	if (Q_strcaseeq(*token, "environment"))
		rs_stage_envmap(stage, token);

	if (Q_strcaseeq(*token, "vector"))
	{
		//*token = strtok(NULL, TOK_DELIMINATORS);
		stage->tcGen = TC_GEN_VECTOR;
		RS_ParseVector4(token, stage->tcGenVec[0]);
		RS_ParseVector4(token, stage->tcGenVec[1]);
	}
}

static void rs_stage_offset (rs_stage_t *stage, char **token) // jitrscript
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	stage->offset.offsetX = atof(*token);
	
	*token = strtok(NULL, TOK_DELIMINATORS);
	stage->offset.offsetY = atof(*token);
}

static void rs_stage_sharp (rs_stage_t *stage, char **token) // jitrscript
{
	stage->sharp = true;
}

static rs_stagekey_t rs_stagekeys[] = 
{
	{	"map",			&rs_stage_map			},
	{	"scroll",		&rs_stage_scroll		},
	{	"blendfunc",	&rs_stage_blendfunc		},
	{	"alphashift",	&rs_stage_alphashift	},
	{	"anim",			&rs_stage_anim			},
	{	"envmap",		&rs_stage_envmap		},
	{	"nolightmap",	&rs_stage_nolightmap	},
	{	"alphamask",	&rs_stage_alphamask		},
	{	"rotate",		&rs_stage_rotate		},
	{	"scale",		&rs_stage_scale			},
	{	"scaleadd",		&rs_stage_scaleadd		}, // jitrscript
	{	"offset",		&rs_stage_offset		}, // jitrscript
	{	"tcgen",		&rs_stage_tcGen			}, // jitrscript
	{	"sharp",		&rs_stage_sharp			}, // jitrscript

	{	NULL,			NULL					}
};

static int num_stagekeys = sizeof (rs_stagekeys) / sizeof(rs_stagekeys[0]) - 1;

// =====================================================

void rs_script_safe (rscript_t *rs, char **token)
{
	rs->dontflush = true;
}

void rs_script_width (rscript_t *rs, char **token) // jitrscript
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	rs->width = atoi(*token);
}

void rs_script_height (rscript_t *rs, char **token) // jitrscript
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	rs->height = atoi(*token);
}

void rs_script_subdivide (rscript_t *rs, char **token)
{
	int divsize, p2divsize;

	*token = strtok (NULL, TOK_DELIMINATORS);
	divsize = atoi (*token);
 
	// cap max & min subdivide sizes
	if (divsize > 128)
		divsize = 128;
	else if (divsize <= 8)
		divsize = 8;

	// find the next smallest valid ^2 size, if not already one
	for (p2divsize = 2; p2divsize <= divsize ; p2divsize <<= 1 );

	p2divsize >>= 1;

	rs->subdivide = (char)p2divsize;
}

void rs_script_vertexwarp (rscript_t *rs, char **token)
{
	*token = strtok(NULL, TOK_DELIMINATORS);
	rs->warpspeed = atof (*token);
	*token = strtok(NULL, TOK_DELIMINATORS);
	rs->warpdist = atof (*token);
	*token = strtok(NULL, TOK_DELIMINATORS);
	rs->warpsmooth = atof (*token);

	if (rs->warpsmooth < 0.001f)
		rs->warpsmooth = 0.001f;
	else if (rs->warpsmooth > 1.0f)
		rs->warpsmooth = 1.0f;
}

void rs_script_mirror (rscript_t *rs, char **token)
{
	// TODO
	rs->mirror = true;
}

static rs_scriptkey_t rs_scriptkeys[] = 
{
	{	"safe",			&rs_script_safe			},
	{	"width",		&rs_script_width		},
	{	"height",		&rs_script_height		},
	{	"subdivide",	&rs_script_subdivide	},
	{	"vertexwarp",	&rs_script_vertexwarp	},
	{	"mirror",		&rs_script_mirror		},

	{	NULL,			NULL					}
};

static int num_scriptkeys = sizeof (rs_scriptkeys) / sizeof(rs_scriptkeys[0]) - 1;

// =====================================================

void RS_LoadScript (char *script)
{
	qboolean		inscript = false, instage = false;
	char			ignored = 0;
	char			*token, *fbuf, *buf;
	rscript_t		*rs = NULL;
	rs_stage_t		*stage;
	unsigned char	tcmod = 0;
	unsigned int	len, i;

	len = ri.FS_LoadFile(script, (void **)&fbuf);

	if (!fbuf || len < 16) 
	{
		// jit -- removed -- don't want to confuse the newbies :)
		// T3RR0R15T: enabled for developer (changed PRINT_ALL to PRINT_DEVELOPER)
		ri.Con_Printf(PRINT_DEVELOPER,"Could not load script %s.\n",script);
		return;
	}

	buf = (char*)malloc(len+1);
	memcpy(buf, fbuf, len);
	buf[len] = 0;

	ri.FS_FreeFile(fbuf);

	token = strtok(buf, TOK_DELIMINATORS);

	while (token != NULL) 
	{
		if (Q_streq(token, "/*") || Q_streq(token, "["))
		{
			ignored++;
		}
		//else if (Q_strcaseeq(token, "*/") || Q_strcaseeq(token, "]"))
		//{
		//	ignored--;
		//	token = strtok(NULL, TOK_DELIMINATORS); // jitrscript (don't make rscripts named "*/")
		//}

		if (!inscript && !ignored) 
		{
			if (Q_strcaseeq(token, "{"))
			{
				inscript = true;
			}
			else
			{
				rs = RS_FindScript(token);

				if (rs)
				{
					image_t *image;

					// jitrscript: if we have a texture pointing to this, update the pointer!
					image = rs->img_ptr;

					RS_FreeScript(rs);
					rs = RS_NewScript(token);
					
					if (image) // jit
					{
						rs->img_ptr = image;
						image->rscript = rs;
					}
				}
				else
				{
					rs = RS_NewScript(token);
				}
			}
		}
		else if (inscript && !ignored)
		{
			if (Q_strcaseeq(token, "}"))
			{
				if (instage)
				{
					instage = false;
				}
				else
				{
					inscript = false;
				}
			}
			else if (Q_strcaseeq(token, "{"))
			{
				if (!instage)
				{
					instage = true;
					stage = RS_NewStage(rs);
				}
			}
			else
			{
				if (instage && !ignored)
				{
					for (i = 0; i < num_stagekeys; i++)
					{
						if (Q_strcaseeq(rs_stagekeys[i].stage, token))
						{
							rs_stagekeys[i].func(stage, &token);
							break;
						}
					}
				}
				else
				{
					for (i = 0; i < num_scriptkeys; i++)
					{
						if (Q_strcaseeq(rs_scriptkeys[i].script, token))
						{
							rs_scriptkeys[i].func(rs, &token);
							break;
						}
					}
				}
			}			
		}

		// jitrscript, moved: don't make rscripts named "*/"
		if (Q_streq(token, "*/") || Q_streq(token, "]"))
		{
			ignored--;
		}

		token = strtok (NULL, TOK_DELIMINATORS);
	}

	free(buf);
}

void RS_ScanPathForScripts (char *dir)
{
	char	script[MAX_OSPATH];
	char	dirstring[1024], *c;
	char	**script_list;
	int	script_count, i;

	Com_sprintf(dirstring, sizeof(dirstring), "%s/scripts/*.txt", dir);
	script_list = ri.FS_ListFiles(dirstring, &script_count, 0, 0, true);

	if (!script_list)
		return;

	for (i = 0; i < script_count-1; i++)
	{
		c = COM_SkipPath(script_list[i]);
		Com_sprintf(script, MAX_OSPATH, "scripts/%s", c);
		RS_LoadScript(script);
	}

	ri.FS_FreeFileList(script_list, script_count);
}

_inline void RS_RotateST (float *os, float *ot, float degrees, msurface_t *fa)
{
	float cost = cos(degrees), sint = sin(degrees);
	float is = *os, it = *ot;

	*os = cost * (is - fa->c_s) + sint * (fa->c_t - it) + fa->c_s;
	*ot = cost * (it - fa->c_t) + sint * (is - fa->c_s) + fa->c_t;
}


_inline static void RS_SetEnvmap (vec3_t v, float *os, float *ot, vec_t *normals) // jit - enhanced to work properly
{
	//vec3_t vec, dir;
	//float t;


	///*if (!AxisCompare(currententity->axis, axisDefault)) // jitodo -- what is this -- is it needed? Maybe for viewweapons?
	//{
	//	VectorSubtract(r_refDef.viewOrigin, currententity->origin, dir);
	//	VectorRotate(dir, currententity->axis, vec);
	//}
	//else*/
	//{
	//	VectorSubtract(r_newrefdef.vieworg, currententity->origin, vec);
	//}

	//VectorSubtract(vec, v, dir);

	////VectorSubtract(r_newrefdef.vieworg, v, dir);
	//VectorNormalize(dir); // todo - needed?

	//t = Q_rsqrt(DotProduct(dir, dir));

	//if(!surf)
	//{
	//	*os = dir[0]*t - 1.0f; // jitodo -- not sure if this is right?
	//	*ot = dir[1]*t; // -0;
	//}
	//else
	//{
	//	*os = dir[0]*t - surf->plane->normal[0]; //normalArray[i][0];
	//	*ot = dir[1]*t - surf->plane->normal[1]; //normalArray[i][1];
	//}


	// code from qfusion
	vec3_t transform, t, n;
	mat3_t inverse_axis;

	if (!currentmodel)
	{
		VectorSubtract(vec3_origin, currententity->origin, transform);
		Matrix3_Transpose(currententity->axis, inverse_axis);
	}
	else if (currentmodel == r_worldmodel)
	{
		VectorSubtract(vec3_origin, r_origin, transform);
	}
	else if (currentmodel->type == mod_brush)
	{
		VectorNegate(currententity->origin, t);
		VectorSubtract(t, r_origin, transform);
		Matrix3_Transpose(currententity->axis, inverse_axis);
	}
	else
	{
		VectorSubtract(vec3_origin, currententity->origin, transform);
		Matrix3_Transpose(currententity->axis, inverse_axis);
	}

	
	VectorAdd(v, transform, t);

	// project vector
	if (currentmodel && (currentmodel == r_worldmodel))
	{
		n[0] = normals[0];
		n[1] = normals[1];
		n[2] = Q_rsqrt(DotProduct(t,t));
	}
	else
	{
		n[0] = DotProduct(normals, inverse_axis[0]);
		n[1] = DotProduct(normals, inverse_axis[1]);
		n[2] = Q_rsqrt(DotProduct(t,t));
	}

	*os = t[0]*n[2] - n[0];
	*ot = t[1]*n[2] - n[1];
}

_inline static void RS_SetTexcoords (rs_stage_t *stage, float *os, float *ot, msurface_t *fa)
{
	// scale
	if (stage->scale.scaleX)
	{
		switch (stage->scale.typeX)
		{
		case RSCRIPT_STATIC:	// static
			*os *= stage->scale.scaleX;
			break;
		case RSCRIPT_SINE:	// sine
			*os *= stage->scale.scaleX*sin(rs_realtime*0.05);
			break;
		case RSCRIPT_COSINE:	// cosine
			*os *= stage->scale.scaleX*cos(rs_realtime*0.05);
			break;
		}
	}

	if (stage->scale.scaleY)
	{
		switch (stage->scale.typeY)
		{
		case RSCRIPT_STATIC:	// static
			*ot *= stage->scale.scaleY;
			break;
		case RSCRIPT_SINE:	// sine
			*ot *= stage->scale.scaleY*sin(rs_realtime*0.05);
			break;
		case RSCRIPT_COSINE:	// cosine
			*ot *= stage->scale.scaleY*cos(rs_realtime*0.05);
			break;
		}
	}




	// rotate
	if (stage->rot_speed)
		RS_RotateST (os, ot, -stage->rot_speed * rs_realtime * 0.0087266388888888888888888888888889, fa);
}
//
//
///*
// =================
// RB_CalcTextureCoords
// Taken from Quake2Evolved (probably taken from qfusion)
// Modified by jitspoe
// =================
//*/
//static void RS_SetTexcoords (rs_stage_t *stage, float *os, float *ot, msurface_t *fa)
//{
//	tcGen_t		*tcGen;
//	tcMod_t		*tcMod;
//	int			i, j;
//	vec3_t		vec, dir;
//	waveTable_t	*table;
//	float		now, f, t;
//	float		rad, s, c;
//	vec2_t		st;
//
//	tcGen = &stage->tcGen;
//	tcMod = stage->tcMod;
//
//	switch (tcGen->type)
//	{
//	case TCGEN_BASE:
//		// os and ot stay the same?
//
//		break;
//	case TCGEN_LIGHTMAP:
//		for (i = 0; i < numVertex; i++)
//		{
//			texCoordArray[unit][i][0] = inLmCoordArray[i][0];
//			texCoordArray[unit][i][1] = inLmCoordArray[i][1];
//		}
//		
//		break;
//	case TCGEN_ENVIRONMENT:
//		if (!AxisCompare(rb_entity->axis, axisDefault))
//		{
//			VectorSubtract(r_refDef.viewOrigin, rb_entity->origin, dir);
//			VectorRotate(dir, rb_entity->axis, vec);
//		}
//		else
//			VectorSubtract(r_refDef.viewOrigin, rb_entity->origin, vec);
//
//		for (i = 0; i < numVertex; i++)
//		{
//			VectorSubtract(vec, vertexArray[i], dir);
//			t = Q_rsqrt(DotProduct(dir, dir));
//
//			texCoordArray[unit][i][0] = dir[0]*t - normalArray[i][0];
//			texCoordArray[unit][i][1] = dir[1]*t - normalArray[i][1];
//		}
//	
//		break;
//	case TCGEN_VECTOR:
//		for (i = 0; i < numVertex; i++)
//		{
//			texCoordArray[unit][i][0] = DotProduct(vertexArray[i], tcGen->params+0);
//			texCoordArray[unit][i][1] = DotProduct(vertexArray[i], tcGen->params+3);
//		}
//		
//		break;
//	default:
//		Com_Error(ERR_FATAL, "RB_CalcTextureCoords: unknown tcGen type %d in shader '%s'", tcGen->type, rb_shader->name);
//	}
//	
//	for (i = 0; i < tcModNum; i++, tcMod++){
//		switch (tcMod->type){
//		case TCMOD_TRANSLATE:
//			for (j = 0; j < numVertex; j++)
//			{
//				texCoordArray[unit][j][0] += tcMod->params[0];
//				texCoordArray[unit][j][1] += tcMod->params[1];
//			}
//
//			break;
//		case TCMOD_SCALE:
//			for (j = 0; j < numVertex; j++)
//			{
//				texCoordArray[unit][j][0] *= tcMod->params[0];
//				texCoordArray[unit][j][1] *= tcMod->params[1];
//			}
//
//			break;
//		case TCMOD_SCROLL:
//			st[0] = tcMod->params[0] * rb_shaderTime;
//			st[0] -= floor(st[0]);
//
//			st[1] = tcMod->params[1] * rb_shaderTime;
//			st[1] -= floor(st[1]);
//
//			for (j = 0; j < numVertex; j++)
//			{
//				texCoordArray[unit][j][0] += st[0];
//				texCoordArray[unit][j][1] += st[1];
//			}
//			
//			break;
//		case TCMOD_ROTATE:
//			rad = -DEG2RAD(tcMod->params[0] * rb_shaderTime);
//			s = sin(rad);
//			c = cos(rad);
//
//			for (j = 0; j < numVertex; j++)
//			{
//				st[0] = texCoordArray[unit][j][0];
//				st[1] = texCoordArray[unit][j][1];
//				texCoordArray[unit][j][0] = c * (st[0] - 0.5) + s * (0.5 - st[1]) + 0.5;
//				texCoordArray[unit][j][1] = c * (st[1] - 0.5) + s * (st[0] - 0.5) + 0.5;
//			}
//			
//			break;
//		case TCMOD_TURB:
//			table = RB_TableForFunc(&tcMod->func);
//			now = tcMod->func.params[2] + tcMod->func.params[3] * rb_shaderTime;
//
//			for (j = 0; j < numVertex; j++)
//			{
//				texCoordArray[unit][j][0] += (table->table[((int)(((vertexArray[j][0] + vertexArray[j][2]) * 1.0/128 * 0.125 + now) * table->size)) % table->size] * tcMod->func.params[1] + tcMod->func.params[0]);
//				texCoordArray[unit][j][1] += (table->table[((int)(((vertexArray[j][1]) * 1.0/128 * 0.125 + now) * table->size)) % table->size] * tcMod->func.params[1] + tcMod->func.params[0]);
//			}
//			
//			break;
//		case TCMOD_WARP:
//			now = tcMod->func.params[2] + tcMod->func.params[3] * rb_shaderTime;
//
//			for (j = 0; j < numVertex; j++)
//			{
//				st[0] = texCoordArray[unit][j][0] * 64.0;
//				st[1] = texCoordArray[unit][j][1] * 64.0;
//				texCoordArray[unit][j][0] += ((rb_warpSinTable[((int)((st[1] * 0.125 + now) * (256.0/M_PI2))) & 255] * (1.0/64)) * tcMod->func.params[1] + tcMod->func.params[0]);
//				texCoordArray[unit][j][1] += ((rb_warpSinTable[((int)((st[0] * 0.125 + now) * (256.0/M_PI2))) & 255] * (1.0/64)) * tcMod->func.params[1] + tcMod->func.params[0]);
//			}
//
//			break;
//		case TCMOD_STRETCH:
//			table = RB_TableForFunc(&tcMod->func);
//			now = tcMod->func.params[2] + tcMod->func.params[3] * rb_shaderTime;
//			f = table->table[((int)(now * table->size)) % table->size] * tcMod->func.params[1] + tcMod->func.params[0];
//			
//			f = (f) ? 1.0 / f : 1.0;
//			t = 0.5 - 0.5 * f;
//
//			for (j = 0; j < numVertex; j++)
//			{
//				texCoordArray[unit][j][0] = texCoordArray[unit][j][0] * f + t;
//				texCoordArray[unit][j][1] = texCoordArray[unit][j][1] * f + t;
//			}
//			
//			break;
//		case TCMOD_TRANSFORM:
//			for (j = 0; j < numVertex; j++)
//			{
//				st[0] = texCoordArray[unit][j][0];
//				st[1] = texCoordArray[unit][j][1];
//				texCoordArray[unit][j][0] = st[0] * tcMod->params[0] + st[1] * tcMod->params[2] + tcMod->params[4];
//				texCoordArray[unit][j][1] = st[1] * tcMod->params[1] + st[0] * tcMod->params[3] + tcMod->params[5];
//			}
//			
//			break;
//		default:
//			Com_Error(ERR_FATAL, "RB_CalcTextureCoords: unknown tcMod type %d in shader '%s'", tcMod->type, rb_shader->name);
//		}
//	}
//}


_inline void RS_RotateST2 (float *os, float *ot, float degrees)
{
	float cost = cos(degrees), sint = sin(degrees);
	float is = *os, it = *ot;

	*os = cost * (is - 0.5) + sint * (0.5 - it) + 0.5;
	*ot = cost * (it - 0.5) + sint * (is - 0.5) + 0.5;
}

void RS_SetTexcoords2D (rs_stage_t *stage, float *os, float *ot)
{
	*os += stage->offset.offsetX; // jitrscript
	*ot += stage->offset.offsetY; // jitrscript

	// scale
	if (stage->scale.scaleX)
	{
		switch (stage->scale.typeX)
		{
		case RSCRIPT_STATIC:	// static
			*os *= stage->scale.scaleX;
			break;
		case RSCRIPT_SINE:	// sine
			*os *= stage->scale.scaleX*sin(rs_realtime*0.05);
			break;
		case RSCRIPT_COSINE:	// cosine
			*os *= stage->scale.scaleX*cos(rs_realtime*0.05);
			break;
		case RSCRIPT_SINABS: // jitrscript
			*os *= stage->scale.scaleX*(sin(rs_realtime*0.05)+1.0);
			break;
		case RSCRIPT_COSABS: // jitrscript
			*os *= stage->scale.scaleX*(cos(rs_realtime*0.05)+1.0);
			break;
		}
	}

	if (stage->scale.scaleY)
	{
		switch (stage->scale.typeY)
		{
		case RSCRIPT_STATIC:	// static
			*ot *= stage->scale.scaleY;
			break;
		case RSCRIPT_SINE:	// sine
			*ot *= stage->scale.scaleY*sin(rs_realtime*0.05);
			break;
		case RSCRIPT_COSINE:	// cosine
			*ot *= stage->scale.scaleY*cos(rs_realtime*0.05);
			break;
		case RSCRIPT_SINABS: // jitrscript
			*ot *= stage->scale.scaleY*(sin(rs_realtime*0.05)+1.0);
			break;
		case RSCRIPT_COSABS: // jitrscript
			*ot *= stage->scale.scaleY*(cos(rs_realtime*0.05)+1.0);
			break;
		}
	}

	// scaleadd -- jitrscript
	if (stage->scaleadd.scaleX) // jitrscript
	{
		switch (stage->scaleadd.typeX) 
		{
		case 0:	// static
			if (*os > 0)
				*os += stage->scaleadd.scaleX;
			else
				*os -= stage->scaleadd.scaleX;
			break;
		case 1:	// sine (probably won't get used, but just for completeness)
			if (*os > 0)
				*os += stage->scaleadd.scaleX*sin(rs_realtime*0.05);
			else
				*os -= stage->scaleadd.scaleX*sin(rs_realtime*0.05);
			break;
		case 2:	// cosine
			if (*os > 0)
				*os += stage->scaleadd.scaleX*cos(rs_realtime*0.05);
			else
				*os -= stage->scaleadd.scaleX*cos(rs_realtime*0.05);
			break;
		}
	}

	if (stage->scaleadd.scaleY) // jitrscript
	{
		switch (stage->scaleadd.typeY)
		{
		case 0:	// static
			if (*ot > 0)
				*ot += stage->scaleadd.scaleY;
			else
				*ot -= stage->scaleadd.scaleY;
			break;
		case 1:	// sine
			if (*ot > 0)
				*ot += stage->scaleadd.scaleY*sin(rs_realtime*0.05);
			else
				*ot -= stage->scaleadd.scaleY*sin(rs_realtime*0.05);
			break;
		case 2:	// cosine
			if (*ot > 0)
				*ot += stage->scaleadd.scaleY*cos(rs_realtime*0.05);
			else
				*ot -= stage->scaleadd.scaleY*cos(rs_realtime*0.05);
			break;
		}
	}

	// rotate
	if (stage->rot_speed)
		RS_RotateST2 (os, ot, -stage->rot_speed * rs_realtime * 0.0087266388888888888888888888888889);
}


qboolean alphasurf = false; // jitrscript

void RS_DrawSurface (msurface_t *surf, qboolean lightmap, rscript_t *rs) // jitrscript
{
	glpoly_t	*p;
	float		*v;
	int			i, nv = surf->polys->numverts;
	vec3_t		wv;
	rs_stage_t	*stage;// = rs->stage;
	float		os, ot, alpha;
	float		scale, time, txm=0, tym=0;
	qboolean	firststage = true; // jitrscript

	if (!rs)
		rs = (rscript_t *)surf->texinfo->script; // jitrscript

	stage = rs->stage; // jitrscript
	time = rs_realtime * rs->warpspeed; // jitrscrpt

	do
	{
		if (stage->anim_count)
			GL_MBind(QGL_TEXTURE0, RS_Animate(stage));
		else
			GL_MBind(QGL_TEXTURE0, stage->texture->texnum);

		// sane defaults
		alpha = 1.0f;

		if (stage->blendfunc.blend && (alphasurf || !firststage))
		{
			qgl.BlendFunc(stage->blendfunc.source, stage->blendfunc.dest);
			GLSTATE_ENABLE_BLEND
		}
		else
		{
			GLSTATE_DISABLE_BLEND
		}

		if ((stage->alphashift.min || stage->alphashift.speed) &&
			(alphasurf || !firststage))
		{
			if (!stage->alphashift.speed && stage->alphashift.min > 0)
			{
				alpha = stage->alphashift.min;
			}
			else if (stage->alphashift.speed)
			{
				alpha = sin (rs_realtime * stage->alphashift.speed);
				alpha = (alpha + 1)*0.5f;

				if (alpha > stage->alphashift.max) 
					alpha = stage->alphashift.max;
				if (alpha < stage->alphashift.min) 
					alpha = stage->alphashift.min;
			}
		}
		else
			alpha = 1.0f;

		if (stage->scroll.speedX)
		{
			switch (stage->scroll.typeX)
			{
			case 0:	// static
				txm = rs_realtime*stage->scroll.speedX;
				break;
			case 1:	// sine
				txm = sin (rs_realtime*stage->scroll.speedX);
				break;
			case 2:	// cosine
				txm = cos (rs_realtime*stage->scroll.speedX);
				break;
			}
		}
		else
		{
			txm = 0;
		}

		if (stage->scroll.speedY)
		{
			switch (stage->scroll.typeY)
			{
			case 0:	// static
				tym = rs_realtime*stage->scroll.speedY;
				break;
			case 1:	// sine
				tym = sin (rs_realtime*stage->scroll.speedY);
				break;
			case 2:	// cosine
				tym = cos (rs_realtime*stage->scroll.speedY);
				break;
			}
		}
		else
		{
			tym = 0;
		}

		qgl.Color4f (1, 1, 1, alpha);

//		if (stage->envmap)
//			GL_TexEnv(GL_MODULATE);

		if (stage->alphamask && (alphasurf || !firststage))
		{
			GLSTATE_ENABLE_ALPHATEST
		}
		else
		{
			GLSTATE_DISABLE_ALPHATEST
		}

		if (rs->subdivide)
		{
			glpoly_t *bp;
			int i;

			for (bp = surf->polys; bp; bp = bp->next)
			{
				p = bp;
				qgl.Begin(GL_TRIANGLE_FAN);

				for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
				{
					switch (stage->tcGen) // jitrscript
					{
					case TC_GEN_BASE:
						os = v[3];
						ot = v[4];
						break;
					case TC_GEN_ENVIRONMENT:
						RS_SetEnvmap(v, &os, &ot, surf->plane->normal);
						break;
					case TC_GEN_VECTOR:
						// Note: unless we add support for a 4th value, stage->tcGenVec[x][3] == 0.
						// Commented out to (maybe) improve speed.
						os = DotProduct(stage->tcGenVec[0], v);// always 0: + stage->tcGenVec[0][3];
						ot = DotProduct(stage->tcGenVec[1], v);// always 0: + stage->tcGenVec[1][3];
						break;
					case TC_GEN_LIGHTMAP:
						break;
					}

					RS_SetTexcoords(stage, &os, &ot, surf);

					if (lightmap)
					{
						qgl.MultiTexCoord2fARB(QGL_TEXTURE0, os+txm, ot+tym);
						qgl.MultiTexCoord2fARB(QGL_TEXTURE1, v[5], v[6]);
					}
					else
					{
						qgl.TexCoord2f(os+txm, ot+tym); // jitrscript (added txm/tym)
					}

					if (!rs->warpsmooth)
					{
						qgl.Vertex3fv(v);
					}
					else
					{
						scale = rs->warpdist * sin(v[0]*rs->warpsmooth+time) *
							sin(v[1]*rs->warpsmooth+time) * sin(v[2]*rs->warpsmooth+time);
						VectorMA(v, scale, surf->plane->normal, wv);
						qgl.Vertex3fv (wv);
					}
				}

				qgl.End();
			}
		}
		else
		{
			for (p = surf->polys; p; p = p->chain)
			{
				qgl.Begin(GL_TRIANGLE_FAN);

				for (i = 0, v = p->verts[0]; i < nv; i++, v += VERTEXSIZE)
				{
					switch (stage->tcGen) // jitrscript
					{
					case TC_GEN_BASE:
						os = v[3];
						ot = v[4];
						break;
					case TC_GEN_ENVIRONMENT:
						RS_SetEnvmap(v, &os, &ot, surf->plane->normal);
						break;
					case TC_GEN_VECTOR:
						// Note: unless we add support for a 4th value, stage->tcGenVec[x][3] == 0.
						// Commented out to (maybe) improve speed.
						os = DotProduct(stage->tcGenVec[0], v);// always 0: + stage->tcGenVec[0][3];
						ot = DotProduct(stage->tcGenVec[1], v);// always 0: + stage->tcGenVec[1][3];
						break;
					case TC_GEN_LIGHTMAP:
						break;
					}

					RS_SetTexcoords(stage, &os, &ot, surf);

					if (lightmap && qgl.MultiTexCoord2fARB)
					{
						qgl.MultiTexCoord2fARB(QGL_TEXTURE0, os + txm, ot + tym);
						qgl.MultiTexCoord2fARB(QGL_TEXTURE1, v[5], v[6]);
					}
					else
					{
						qgl.TexCoord2f(os + txm, ot + tym); // jitrscript (added txm/tym)
					}

					if (!rs->warpsmooth)
					{
						qgl.Vertex3fv(v);
					}
					else
					{
						scale = rs->warpdist * sin(v[0] * rs->warpsmooth+time) * sin(v[1] * rs->warpsmooth + time) * sin(v[2] * rs->warpsmooth + time);
						VectorMA(v, scale, surf->plane->normal, wv);
						qgl.Vertex3fv(wv);
					}
				}

				qgl.End();
			}
		}

		//if (stage->envmap)
		//	GL_TexEnv(GL_REPLACE);

		qgl.Color4f(1, 1, 1, 1);
		qgl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLSTATE_DISABLE_BLEND
		GLSTATE_DISABLE_ALPHATEST
		GLSTATE_DISABLE_TEXGEN
		firststage = false; // jitrscript
	} while ((stage = stage->next));
}





