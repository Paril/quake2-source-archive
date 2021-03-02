//===========================================================================
//
// Name:         g_ch.c
// Function:     Colored Hitman
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1999-02-10
// Tab Size:     3
//===========================================================================

#include "g_local.h"

#define STAT_COLOR		 30	//Colored Hitman color stat

cvar_t *ch;
cvar_t *ch_maxcolors;
cvar_t *ch_colortime;

int lastnumplayers;
float colortime;


char *ch_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14"

// Colored Hitman color
"yb -102 "
"if 30 "
  "xr -26 "
  "pic 30 "
"endif "
;

char *ch_colors[] =
{
	"red",
	"blue",
	"green",
	"white",
	"yellow",
	"orange",
	NULL
};

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void PrecacheColoredHitman(void)
{
	int i;
	char buf[144];

	for (i = 0; ch_colors[i]; i++)
	{
		gi.imageindex(ch_colors[i]);
		sprintf(buf, "models/%s/tris.md2", ch_colors[i]);
		gi.modelindex(buf);
	} //end for
	gi.soundindex("ch/flagcap.wav");
	gi.dprintf("ch precached\n");
} //end of the function PrecacheColoredHitman
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *ColorImageName(int color)
{
	int i;

	for (i = 0; i < color-1 && ch_colors[i]; i++) ;

	if (ch_colors[i]) return ch_colors[i];
	return ch_colors[0];
} //end of the function ColorImage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ColorImage(int color)
{
	int i;

	for (i = 0; i < color-1 && ch_colors[i]; i++) ;

	if (ch_colors[i]) return gi.imageindex(ch_colors[i]);
	return gi.imageindex(ch_colors[0]);
} //end of the function ColorImage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ColorModel(int color)
{
	int i;
	char buf[144];

	for (i = 0; i < color-1 && ch_colors[i]; i++) ;

	if (ch_colors[i]) sprintf(buf, "models/%s/tris.md2", ch_colors[i]);
	else sprintf(buf, "models/%s/tris.md2", ch_colors[0]);
	return gi.modelindex(buf);
} //end of the function ColorModel
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ColoredHitmanEffects(edict_t *ent)
{
	if (!ent->client) return;
	ent->s.modelindex3 = ColorModel(ent->client->chcolor);
} //end of the function ColoredHitmanModel
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ColoredHitmanStats(edict_t *ent)
{
	if (!ent->client) return;
	//set the color image in the status bar
	ent->client->ps.stats[STAT_COLOR] = ColorImage(ent->client->chcolor);
} //end of the function ColoredHitmanStats
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int NumPlayers(void)
{
	int i, numplayers;
	edict_t *cl_ent;

	numplayers = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		//if not an active client
		if (!cl_ent->inuse) continue;
		if (!cl_ent->client) continue;
#ifdef OBSERVER
		//if in observer mode
		if (cl_ent->flags & FL_OBSERVER) continue;
#endif //OBSERVER
		//
		numplayers++;
	} //end for
	return numplayers;
} //end of the function NumPlayers
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int MaxColors(void)
{
	int mc;

	mc = ch_maxcolors->value;
	if (mc <= 1) mc = 2;
	return mc;
} //end of the function MaxColors
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ResetPlayerColors(void)
{
	int numplayers, maxcolors, color, i, j, rnd;
	edict_t *cl_ent;

	maxcolors = MaxColors();
	numplayers = NumPlayers();

	//we can't use more colors than half the number of players, after all
	//every player has to have at least one opponent with the same color
	if (maxcolors > numplayers / 2) maxcolors = numplayers / 2;
	if (maxcolors < 1) maxcolors = 1;
	//remove all the client colors
	for (j = 0; j < maxclients->value; j++)
	{
		cl_ent = g_edicts + 1 + j;
		//if not an active client
		if (!cl_ent->inuse) continue;
		if (!cl_ent->client) continue;
#ifdef OBSERVER
		//if in observer mode
		if (cl_ent->flags & FL_OBSERVER) continue;
#endif //OBSERVER
		cl_ent->client->chcolor = 0;
	} //end for
	//start with the first color
	color = 1;
	//add every player to one of the colors
	for (i = 0; i < numplayers; i++)
	{
		//select a player randomly
		rnd = floor((numplayers-i) * random());
		for (j = 0; j < maxclients->value && rnd >= 0; j++)
		{
			cl_ent = g_edicts + 1 + j;
			//if not an active client
			if (!cl_ent->inuse) continue;
			if (!cl_ent->client) continue;
#ifdef OBSERVER
			//if in observer mode
			if (cl_ent->flags & FL_OBSERVER) continue;
#endif //OBSERVER
			if (cl_ent->client->chcolor) continue;
			//
			rnd--;
		} //end for
		cl_ent->client->chcolor = color;
		//next color
		if (++color > maxcolors) color = 1;
	} //end for
} //end of the function ResetPlayerColors
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int InvalidColors(void)
{
	int i;
	edict_t *cl_ent;

	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		//if not an active client
		if (!cl_ent->inuse) continue;
		if (!cl_ent->client) continue;
#ifdef OBSERVER
		//if in observer mode
		if (cl_ent->flags & FL_OBSERVER) continue;
#endif //OBSERVER
		//
		if (!cl_ent->client->chcolor)
		{
			gi.dprintf("player with invalid colors\n");
			return true;
		} //end if
	} //end for
	return false;
} //end of the function InvalidColors
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void UpdateColoredHitman(void)
{
	int numplayers;

	numplayers = NumPlayers();
	//if the number of players changed or someone has an invalid color
	//or it's time to change colors
	if (numplayers != lastnumplayers || InvalidColors() ||
			level.time > colortime + ch_colortime->value)
	{
		lastnumplayers = numplayers;
		colortime = level.time;
		//reset the player colors
		ResetPlayerColors();
		//play sound to attent everyone about the color change
		gi.sound(world, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ch/flagcap.wav"), 1, ATTN_NONE, 0);
		gi.dprintf("ch changed colors\n");
	} //end if
} //end of the function UpdateColoredHitman
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void InitColoredHitman(void)
{
	ch = gi.cvar("ch", "0", CVAR_SERVERINFO);
	ch_maxcolors = gi.cvar("ch_maxcolors", "6", 0);
	ch_colortime = gi.cvar("ch_colortime", "30", 0);
	lastnumplayers = 0;
	gi.dprintf("ch initialized\n");
} //end of the function InitColoredHitman
