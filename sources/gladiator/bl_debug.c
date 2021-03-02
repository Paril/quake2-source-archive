//===========================================================================
//
// Name:				bl_debug.c
// Function:		debug functions
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-10
// Tab Size:		3
//===========================================================================

#include "g_local.h"

#ifdef BOT

#include "bl_main.h"

//===========================================================================
// shows a line edict at the given position and with the given color
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ShowLine(edict_t *line, vec3_t start, vec3_t end, int color)
{
	//if no color make the line invisible for clients
	if (color == LINECOLOR_NONE)
	{
		line->svflags |= SVF_NOCLIENT;
		return;
	} //end if
	//line is visible to clients
	line->svflags &= ~SVF_NOCLIENT;
	//set the color
	line->s.skinnum = color;
	//start point of the line
	VectorCopy(start, line->s.origin);
	//end point of the line
	VectorCopy(end, line->s.old_origin);
	//relink edict after moving
	gi.linkentity(line);
} //end of the function ShowLine
//===========================================================================
// creates a line edict
//
// Parameter:				-
// Returns:					the line edict
// Changes Globals:		-
//===========================================================================
edict_t *CreateLine(void)
{
	edict_t *line;
	line = G_Spawn();
	//classname
	line->classname = "debugline";
	//not moving, position only changed by setting the origin
	line->movetype = MOVETYPE_NONE;
	//don't set the old_origin in G_RunFrame
	line->flags |= FL_OLDORGNOTSET;
	//not solid
	line->solid = SOLID_NOT;
	//translucent beam
	line->s.renderfx |= RF_BEAM | RF_TRANSLUCENT;
	//must be non-zero
	line->s.modelindex = 1;
	//diameter of beam (2 seems to be smallest)
	line->s.frame = 2;
	//set the default color (red)
	line->s.skinnum = 0xf2f2f0f0;
	//size of the beam??
	VectorSet(line->mins, -8, -8, -8);
	VectorSet(line->maxs, 8, 8, 8);
	//link the edict
	gi.linkentity(line);
	return line;
} //end of the function CreateLine
//===========================================================================
// creates a debug line
//
// Parameter:				-
// Returns:					handle to the debug line
// Changes Globals:		-
//===========================================================================
int DebugLineCreate(void)
{
	edict_t *line;
	line = CreateLine();
	return DF_ENTNUMBER(line);
} //end of the function DebugLineCreate;
//===========================================================================
// deletes a debug line
//
// Parameter:				line : handle to the debug line
// Returns:					-
// Changes Globals:		-
//===========================================================================
void DebugLineDelete(int line)
{
	edict_t *l;

	if (line < 0 || line >= game.maxentities)
	{
		gi.dprintf("DebugLineDelete: invalid line entity\n");
		return;
	} //end if
	l = DF_NUMBERENT(line);
	if (!l->inuse || Q_strcasecmp(l->classname, "debugline"))
	{
		gi.dprintf("DebugLineDelete: not a line entity\n");
		return;
	} //end if
	G_FreeEdict(l);
} //end of the function DebugLineDelete
//===========================================================================
// shows a debug line with a specific color
//
// Parameter:				line  : handle to the debug line
//								start : start of the line
//								end	: end of the line
//								color : color of the line
// Returns:					-
// Changes Globals:		-
//===========================================================================
void DebugLineShow(int line, vec3_t start, vec3_t end, int color)
{
	edict_t *l;

	if (line < 0 || line >= game.maxentities)
	{
		gi.dprintf("DebugLineShow: invalid line entity\n");
		return;
	} //end if
	l = DF_NUMBERENT(line);
	if (!l->inuse || Q_strcasecmp(l->classname, "debugline"))
	{
		gi.dprintf("DebugLineShow: not a line entity\n");
		return;
	} //end if
	ShowLine(l, start, end, color);
} //end of the function DebugLineShow
//===========================================================================
// creates a visible bounding box
//
// Parameter:				box : bounding box to create
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CreateVisibleBoundingBox(visiblebbox_t *box)
{
	int i;

	if (box->created) return;
	for (i = 0; i < BBOX_LINES; i++)
	{
		box->lines[i] = CreateLine();
	} //end for
	box->created = true;
} //end of the function CreateVisibleBoundingBox
//===========================================================================
// destroys a visible bounding box
//
// Parameter:				box : bounding box to delete
// Returns:					-
// Changes Globals:		-
//===========================================================================
void DestroyVisibleBoundingBox(visiblebbox_t *box)
{
	int i;

	if (!box->created) return;
	for (i = 0; i < BBOX_LINES; i++)
	{
		G_FreeEdict(box->lines[i]);
		box->lines[i] = NULL;
	} //end for
	box->created = false;
} //end of the function DestroyVisibleBoundingBox
//===========================================================================
// toggles a visible bounding box of a specific entity
//
// Parameter:				ent : entity to show bounding box of
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ToggleVisibleBoundingBox(edict_t *ent)
{
#ifdef OBSERVER
	edict_t *cam;

	if (ent->flags & FL_OBSERVER)
	{
		cam = ent->client->camera.ent;
		if (cam != ent)
		{
			if (cam->box.created) DestroyVisibleBoundingBox(&cam->box);
			else CreateVisibleBoundingBox(&cam->box);
			return;
		} //end if
	} //end if
#endif
	if (ent->box.created) DestroyVisibleBoundingBox(&ent->box);
	else CreateVisibleBoundingBox(&ent->box);
} //end of the function ToggleVisibleBoundingBox
//===========================================================================
// sets the bouding box of the given ent
//
// Parameter:				box : bounding box used for visualization
//								ent : entity to visualize the bounding box of
// Returns:					-
// Changes Globals:		-
//===========================================================================
void SetVisibleBoundingBox(visiblebbox_t *box, edict_t *ent)
{
	vec3_t bboxcorners[8], start, end;
	int i;

	if (!box->created) gi.error("SetVisibleBoundingBox: box not created");
	//check if the box has line edicts
	for (i = 0; i < BBOX_LINES; i++)
	{
		if (!box->lines[i]) gi.error("SetVisibleBoundingBox: missing box line");
	} //end for
	//upper corners
	bboxcorners[0][0] = ent->s.origin[0] + ent->maxs[0];
	bboxcorners[0][1] = ent->s.origin[1] + ent->maxs[1];
	bboxcorners[0][2] = ent->s.origin[2] + ent->maxs[2];
	//
	bboxcorners[1][0] = ent->s.origin[0] + ent->mins[0];
	bboxcorners[1][1] = ent->s.origin[1] + ent->maxs[1];
	bboxcorners[1][2] = ent->s.origin[2] + ent->maxs[2];
	//
	bboxcorners[2][0] = ent->s.origin[0] + ent->mins[0];
	bboxcorners[2][1] = ent->s.origin[1] + ent->mins[1];
	bboxcorners[2][2] = ent->s.origin[2] + ent->maxs[2];
	//
	bboxcorners[3][0] = ent->s.origin[0] + ent->maxs[0];
	bboxcorners[3][1] = ent->s.origin[1] + ent->mins[1];
	bboxcorners[3][2] = ent->s.origin[2] + ent->maxs[2];
	//lower corners
	memcpy(bboxcorners[4], bboxcorners[0], sizeof(vec3_t) * 4);
	for (i = 0; i < 4; i++) bboxcorners[4 + i][2] = ent->s.origin[2] + ent->mins[2];
	//draw bounding box
	for (i = 0; i < 4; i++)
	{
		//top plane
		ShowLine(box->lines[i], bboxcorners[i], bboxcorners[(i+1)&3], LINECOLOR_RED);
		//bottom plane
		ShowLine(box->lines[4+i], bboxcorners[4+i], bboxcorners[4+((i+1)&3)], LINECOLOR_RED);
		//vertical lines
		ShowLine(box->lines[8+i], bboxcorners[i], bboxcorners[4+i], LINECOLOR_RED);
	} //end for
	//mark origin with cross
	for (i = 0; i < 2; i++)
	{
		VectorCopy(ent->s.origin, start);
		start[i] += ent->maxs[i];
		VectorCopy(ent->s.origin, end);
		end[i] += ent->mins[i];
		ShowLine(box->lines[12+i], start, end, LINECOLOR_BLUE);
	} //end for
} //end of the function SetVisibleBoundingBox

#endif //BOT
