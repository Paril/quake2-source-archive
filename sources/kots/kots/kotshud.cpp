//*************************************************************************************
//*************************************************************************************
// File: kotshud.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kotshud.h"

#include "user.h"
#include "shared.h"

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "superdef.h"
#include "p_menu.h"
}
#include "menu.h"
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSWorldSpawn
//*************************************************************************************
//*************************************************************************************

void KOTSWorldSpawn()
{
	gi.configstring( CS_STATUSBAR, kots_statusbar );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSVote
//*************************************************************************************
//*************************************************************************************

static void KOTSVote( edict_t *ent, SMenu *menu )
{
	PMenu_Close( ent );

	if ( !ent || !ent->client )
		return;
		
	ent->kots_vote = menu->arg;			

	gi.cprintf( ent, PRINT_HIGH, "your vote has been registered.\n" );
}

//*************************************************************************************
//*************************************************************************************
// Function: votemenu
//*************************************************************************************
//*************************************************************************************

SMenu votemenu[] = 
{
	{ "*Norb & Mother's",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*King of the Servers",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Vote for the next map", PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "1. ",	PMENU_ALIGN_LEFT, 1, KOTSVote },
	{ "2. ",	PMENU_ALIGN_LEFT, 2, KOTSVote },
	{ "3. ",	PMENU_ALIGN_LEFT, 3, KOTSVote },
	{ "4. ",	PMENU_ALIGN_LEFT, 4, KOTSVote },
	{ "5. ",	PMENU_ALIGN_LEFT, 5, KOTSVote },
	{ "6. ",	PMENU_ALIGN_LEFT, 6, KOTSVote },
	{ "7. ",	PMENU_ALIGN_LEFT, 7, KOTSVote },
	{ "8. ",	PMENU_ALIGN_LEFT, 8, KOTSVote },
	{ "9. ",	PMENU_ALIGN_LEFT, 9, KOTSVote },
	{ "10. ",	PMENU_ALIGN_LEFT, 10, KOTSVote }
};

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Vote_f
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Vote_f( edict_t *ent )
{
	int   x = 5;
	char  *s, *t;
	CUser *user = KOTSGetUser( ent );

	static const char *seps = " ,\n\r";

	if ( !user )
		return;

	if ( ent->client->kots_menu )
	{
		PMenu_Close( ent );
		return;
	}

	if ( strlen( sv_maplist->string ) < 2 )
	{
		gi.cprintf( ent, PRINT_HIGH, "no maps to vote on.\n" );
		return;
	}

	if ( gi.argc() == 2 )
	{
		ent->kots_vote = atoi( gi.argv( 1 ) );

		gi.cprintf( ent, PRINT_HIGH, "your vote has been registered.\n" );

		return;
	}
	s = strdup( sv_maplist->string );
	t = strtok( s, seps );

	while ( t != NULL && x < 15 ) 
	{
		sprintf( votemenu[x].text, "%d: %s", x - 4, t );

		x++;

		t = strtok( NULL, seps );
	}
	free(s);

	PMenu_Open( ent, votemenu, 0, x );
}

//*************************************************************************************
//*************************************************************************************
// Function: infomenu
//*************************************************************************************
//*************************************************************************************

SMenu infomenu[] = 
{
	{ "*Norb & Mother's",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*King of the Servers",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Name",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Level",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Rank",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Score",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Points",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Hole",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Frags",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Shots",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Hits",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Accuracy",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Spree",		PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					  PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
};

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Info_f
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Info_f( edict_t *ent )
{
	int   i;
	int   x;
	int   rank  = 1;
	int   total = 1;
	char  sname[ 50 ];
	float ratio;
	CUser *user = KOTSGetUser( ent );

	edict_t	*cl_ent;

	if ( !user )
		return;

	if ( ent->client->kots_menu )
	{
		PMenu_Close( ent );
		return;
	}
	x = 3;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		if ( cl_ent == ent )
			continue;

		if ( ent->client->resp.score < game.clients[i].resp.score )
			rank++;

		total++;
	}
	sprintf( sname, "Name    : %s", user->m_name );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Level   : %d", user->Level() );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Rank    : %d/%d", rank, total );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Score   : %d", user->Score() );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Points  : %d", user->m_points );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Hole    : %d", user->m_hole );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Frags   : %d", ent->kots_kills );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Shots   : %d", ent->kots_shots );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Hits    : %d", ent->kots_hits );
	strcpy( infomenu[x++].text, sname );

	if ( ent->kots_shots > 0 )
	{
		ratio = (float)ent->kots_hits / (float)ent->kots_shots;
	
		ratio *= 100;
	}
	else
		ratio = 0.0;

	sprintf( sname, "Accuracy: %3.2f%%", ratio );
	strcpy( infomenu[x++].text, sname );

	sprintf( sname, "Spree   : %d", ent->client->pers.kots_streak_count );
	strcpy( infomenu[x++].text, sname );

	PMenu_Open( ent, infomenu, 0, sizeof infomenu / sizeof(SMenu));
}

//*************************************************************************************
//*************************************************************************************
// Function: ChooseTeam
//*************************************************************************************
//*************************************************************************************

void KOTSTeam( edict_t *ent )
{
	int     i;
	int     teams[2];
	edict_t *cl_ent;

	memset( teams, 0, sizeof teams );

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		if ( !cl_ent->client )
			continue;

		if ( !cl_ent->client->pers.kotsdata )
			continue;

		if ( !cl_ent->client->resp.kots_ingame )
			continue;

		teams[ cl_ent->client->resp.kots_team ]++;
	}

	if ( teams[1] < teams[0] )
		ent->client->resp.kots_team = 1;
	else
		ent->client->resp.kots_team = 0;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSJoin
//*************************************************************************************
//*************************************************************************************

void KOTSJoin( edict_t *ent, SMenu * )
{
	char *skin;

	PMenu_Close( ent );

	if ( !KOTSClientCanEnter( ent ) )
		return;

	ent->client->resp.kots_ingame = true;

	ent->svflags &= ~SVF_NOCLIENT;

	if ( kots_teamplay->value )
	{
		KOTSTeam( ent );

		skin = KOTSAssignSkin( ent );
	
		gi.bprintf( PRINT_HIGH, "%s Joins Team %s\n", ent->client->pers.netname, skin );

		gi.centerprintf( ent, "You are on the %s Team\n", skin );
	}
	else
		gi.bprintf( PRINT_HIGH, "%s Starts Their Reign\n", ent->client->pers.netname );

	PutClientInServer( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSChaseCam
//*************************************************************************************
//*************************************************************************************

void KOTSChaseCam( edict_t *ent, SMenu * )
{
	int i;
	edict_t *e;

	if ( ent->client->chase_target ) 
	{
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for ( i = 1; i <= maxclients->value; i++ ) 
	{
		e = g_edicts + i;
	
		if (e->inuse && e->solid != SOLID_NOT) 
		{
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSBack
//*************************************************************************************
//*************************************************************************************

void KOTSBack( edict_t *ent, SMenu * )
{
	PMenu_Close( ent );

	KOTSOpenJoinMenu( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: Help
//*************************************************************************************
//*************************************************************************************

SMenu helpmenu[] = {
	{ "*King of the Server",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "At the console type:",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "set kots_password yp u",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Replace yp with",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "your own unique password.",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "type it twice.",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Select Start Reign.",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Back",		PMENU_ALIGN_LEFT, NULL, KOTSBack },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
};

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHelp
//*************************************************************************************
//*************************************************************************************

void KOTSHelp( edict_t *ent, SMenu * )
{
	PMenu_Close( ent );

	PMenu_Open( ent, helpmenu, 12, sizeof helpmenu / sizeof(SMenu) );
}

//*************************************************************************************
//*************************************************************************************
// Function:
//*************************************************************************************
//*************************************************************************************

SMenu creditmenu[] = {
	{ "*King of the Server",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Code:    norb",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Code:    mother",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Artwork: bork",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Website: bork",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Testing: soul",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Back",		PMENU_ALIGN_LEFT, NULL, KOTSBack },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" GDS_PASSWORD,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCredits
//*************************************************************************************
//*************************************************************************************

void KOTSCredits( edict_t *ent, SMenu * )
{
	PMenu_Close( ent );

	PMenu_Open( ent, creditmenu, 8, sizeof creditmenu / sizeof(SMenu) );
}

//*************************************************************************************
//*************************************************************************************
// Function: SMenu
//*************************************************************************************
//*************************************************************************************

SMenu joinmenu[] = {
	{ "*Quake II",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Norb & Mother's",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*King of the Server",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Start your Reign",		PMENU_ALIGN_LEFT, NULL, KOTSJoin },
	{ "Help",		      PMENU_ALIGN_LEFT, NULL, KOTSHelp },
	{ "Chase Camera",	PMENU_ALIGN_LEFT, NULL, KOTSChaseCam },
	{ "Credits",		  PMENU_ALIGN_LEFT, NULL, KOTSCredits },
	{ NULL,					  PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v" GDS_PASSWORD,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

//*************************************************************************************
//*************************************************************************************
// Function: UpdateJoinMenu
//*************************************************************************************
//*************************************************************************************

void KOTSUpdateJoinMenu( edict_t *ent )
{
	static char levelname[ 32 ];

	if ( ent->client->chase_target )
		strcpy( joinmenu[7].text, "Leave Chase Camera" );
	else
		strcpy( joinmenu[7].text, "Chase Camera" );

	levelname[0] = '*';

	if ( kots_teamplay->value )
		strcpy( joinmenu[5].text, "Join KOTS Teamplay" );

	if ( g_edicts[0].message )
		strncpy( levelname+1, g_edicts[0].message, sizeof(levelname) - 2 );
	else
		strncpy( levelname+1, level.mapname, sizeof(levelname) - 2 );

	levelname[sizeof(levelname) - 1] = 0;

	strcpy( joinmenu[3].text, levelname );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSOpenJoinMenu
//*************************************************************************************
//*************************************************************************************

void KOTSOpenJoinMenu( edict_t *ent )
{
	int team;

	KOTSUpdateJoinMenu( ent );

	if ( ent->client->chase_target )
		team = 7;
	else
		team = 5;

	PMenu_Open( ent, joinmenu, team, sizeof joinmenu / sizeof (SMenu) );
}

//*************************************************************************************
//*************************************************************************************
// Function: loc_buildboxpoints
//*************************************************************************************
//*************************************************************************************

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

//*************************************************************************************
//*************************************************************************************
// Function: loc_CanSee
//*************************************************************************************
//*************************************************************************************

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) 
	{
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}
	return false;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSetIDView
//*************************************************************************************
//*************************************************************************************

static void KOTSSetIDView( edict_t *ent )
{
	int     i;
	float	  bd = 0, d;
	CUser   *user;
	CUser   *tuser;
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;

	user = KOTSGetUser( ent );

	if ( !user )
		return;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	VectorScale(forward, 1024, forward);

	VectorAdd(ent->s.origin, forward, forward);

	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);

	if (tr.fraction < 1 && tr.ent && tr.ent->client) 
	{
		tuser = KOTSGetUser( tr.ent );

		if ( !tuser )
			return;

		if ( tr.ent->health < 1 )
			return;

		ent->client->ps.stats[STAT_KOTS_IDLEVEL] = tuser->Level();

		ent->client->ps.stats[STAT_KOTS_IDVIEW] = 
			CS_PLAYERSKINS + (tr.ent - g_edicts - 1);

		if ( user->Level() >= KOTS_KARMA_HEALTHID )
			ent->client->ps.stats[ STAT_KOTS_IDHEALTH ] = tr.ent->health;

		return;
	}
	AngleVectors(ent->client->v_angle, forward, 0, 0);
	
	best = NULL;
	
	for (i = 1; i <= maxclients->value; i++) 
	{
		who = g_edicts + i;
		if (!who->inuse)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	
	if (bd > 0.90)
	{
		tuser = KOTSGetUser( best );

		if ( !tuser )
			return;

		if ( best->health < 1 )
			return;
		
		if ( user->Level() >= KOTS_KARMA_HEALTHID )
			ent->client->ps.stats[ STAT_KOTS_IDHEALTH ] = best->health;

		ent->client->ps.stats[STAT_KOTS_IDLEVEL] = tuser->Level();

		ent->client->ps.stats[STAT_KOTS_IDVIEW] = 
			CS_PLAYERSKINS + (best - g_edicts - 1);
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSShowHUD
//*************************************************************************************
//*************************************************************************************

void KOTSShowHUD( edict_t *ent )
{
	int     i;
	int     rank  = 1;
	int     index;
	CUser   *user = KOTSGetUser( ent );
	edict_t	*cl_ent;

	ent->client->ps.stats[ STAT_KOTS_IDHEALTH ] = 0;
	ent->client->ps.stats[ STAT_KOTS_IDVIEW   ] = 0;
	ent->client->ps.stats[ STAT_KOTS_IDLEVEL  ] = 0;
	ent->client->ps.stats[ STAT_KOTS_RANK     ] = 0;
	ent->client->ps.stats[ STAT_KOTS_LEVEL    ] = 0;
	ent->client->ps.stats[ STAT_KOTS_ITEM     ] = 0;

	ent->client->ps.stats[ STAT_KOTS_STREAK     ] = 0;

	if ( !user )
		return;

	ent->client->ps.stats[ STAT_KOTS_ITEM ] = 1;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		if ( cl_ent == ent )
			continue;

		if ( ent->client->resp.score < game.clients[i].resp.score )
			rank++;
	}
	index = ent->client->pers.selected_item;
	index = ent->client->pers.inventory[ index ];

	ent->client->ps.stats[ STAT_KOTS_RANK  ] = rank;
	ent->client->ps.stats[ STAT_KOTS_LEVEL ] = user->Level();
	ent->client->ps.stats[ STAT_KOTS_ITEM  ] = index;

	ent->client->ps.stats[ STAT_KOTS_STREAK ] = 
			ent->client->pers.kots_streak_count;

	if ( user->Level() >= KOTS_KARMA_ID || user->Level() >= KOTS_KARMA_HEALTHID )
		KOTSSetIDView( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSTeamplayScoreboard
//*************************************************************************************
//*************************************************************************************

void KOTSTeamplayScoreboard( edict_t *ent, int bEnd )
{
	char name  [20];
	char entry [1024];
	char string[1400];
	int  iteam;
	int  ilevel;
	int	 len;
	int  team[2];
	int	 i, j, k;
	int	 total = 0;
	int	 sorted[MAX_CLIENTS];
	int	 sortedscores[MAX_CLIENTS];
	int	 score;
	int  maxsize = 1024;

	CUser *user;

	edict_t		*cl_ent;
	gclient_t	*cl;

	memset( team, 0, sizeof team );

	PMenu_Close( ent );

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		score = game.clients[i].resp.score;

		for ( j = 0; j < total; j++ )
		{
			if ( score > sortedscores[ j ] )
				break;
		}
		team[ cl_ent->client->resp.kots_team ] += score;

		for ( k = total; k > j; k-- )
		{
			sorted      [k] = sorted      [k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j]        = i;
		sortedscores[j]  = score;

		total++;
	}

	if ( bEnd ) // award winner of the level
	{
		if ( total < 2 )
			return;

	 	for ( i = 0; i < game.maxclients; i++ )
		{
			cl_ent = g_edicts + 1 + i;

			if ( !cl_ent->inuse )
				continue;

			if ( team[ cl_ent->client->resp.kots_team ] <= team[ !cl_ent->client->resp.kots_team ] )
				continue;

			if ( ( user = KOTSGetUser( cl_ent ) ) == NULL )
				continue;
		}
		return;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len     = 0;

	Com_sprintf( string, sizeof string, 
	       "xv 5 yv 7 string2 \"Team 0: %d\" "
	       "xv 133 yv 7 string2 \"Team 1: %d\" "
	       "xv 5 yv 15 string2 \"Player\" "
	       "xv 133 yv 15 string2 \"Game\" "	
	       "xv 173 yv 15 string2 \"Tm\" "	
	       "xv 197 yv 15 string2 \"Lvl\" "	
	       "xv 229 yv 15 string2 \"Png\" ", 
				 team[0],
				 team[1]
			 );

	len = strlen( string );

	for ( i = 0; i < total; i++ )
	{
		*entry = 0;

		cl     = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		user = KOTSGetUser( cl_ent );

		memset( name, 0, sizeof name );

		if ( !user )
		{
			strncpy( name, cl->pers.netname, 15 );
			ilevel = -1;
		}
		else
		{
			strncpy( name, user->m_name, 15 );

			ilevel = user->Level();
		}
		iteam = -1;

		if ( cl->resp.kots_ingame )
			iteam = cl->resp.kots_team;

		Com_sprintf( entry, sizeof entry,
	         "xv 5 yv %d string2 \"%s\" "
					 "xv 133 yv %i string \"%4i %2i %3i %3i\" ", 
		       25 + i * 8,
					 name,
		       25 + i * 8,
		       cl->resp.score,
					 iteam,
		       ilevel,
		       cl->ping > 999 ? 999 : cl->ping );

		if ( len + strlen( entry ) > maxsize ) 
			break;

		strcat( string, entry );
		len = strlen(string);
	}
	gi.WriteByte  ( svc_layout );
	gi.WriteString( string );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSScoreboardMessage
//*************************************************************************************
//*************************************************************************************

void KOTSScoreboardMessage( edict_t *ent, edict_t *, int bEnd )
{
	char name  [20];
	char entry [1024];
	char string[1400];
	int	 len;
	int  ilevel;
	int  itotal;
	int	 i, j, k;
	int	 total = 0;
	int	 sorted[MAX_CLIENTS];
	int	 sortedscores[MAX_CLIENTS];
	int	 score;
	int  maxsize = 1024;

	CUser *user;

	edict_t		*cl_ent;
	gclient_t	*cl;

	if ( kots_teamplay->value )
	{
		KOTSTeamplayScoreboard( ent, bEnd );
		return;
	}
	PMenu_Close( ent );

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		if ( kots_lives->value )
			score = cl_ent->kots_lives;
		else
			score = game.clients[i].resp.score;

		for ( j = 0; j < total; j++ )
		{
			if ( score > sortedscores[ j ] )
				break;
		}

		for ( k = total; k > j; k-- )
		{
			sorted      [k] = sorted      [k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j]        = i;
		sortedscores[j]  = score;

		total++;
	}

	if ( bEnd ) // award winner of the level
	{
		if ( total < 2 )
			return;

		cl_ent = g_edicts + 1 + sorted[0];

		if ( cl_ent->kots_lives < 1 )
			return;

		if ( ( user = KOTSGetUser( cl_ent ) ) == NULL )
			return;

		itotal = total;

		if ( kots_lives->value )
			itotal *= 5;
		else if ( itotal >= 8 )
			itotal += KOTS_LEVEL_BONUS;

		user->ModScore( itotal, USER_LEVELWIN );

		return;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len     = 0;

	if ( kots_lives->value )
	{
		Com_sprintf( string, sizeof string, 
	         "xv 5 yv 15 string2 \"Player\" "
	         "xv 128 yv 15 string2 \"Game\" "	
	         "xv 168 yv 15 string2 \"Lvl\" "	
	         "xv 200 yv 15 string2 \"Lives\" "	
	         "xv 248 yv 15 string2 \"Ping\" " 
				 );
	}
	else
	{
		Com_sprintf( string, sizeof string, 
	         "xv 5 yv 15 string2 \"Player\" "
	         "xv 133 yv 15 string2 \"Game\" "	
	         "xv 173 yv 15 string2 \"Lv\" "	
	         "xv 197 yv 15 string2 \"Frg\" "	
	         "xv 229 yv 15 string2 \"Png\" " 
				 );
	}
	len = strlen( string );

	for ( i = 0; i < total; i++ )
	{
		*entry = 0;

		cl     = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		user = KOTSGetUser( cl_ent );

		memset( name, 0, sizeof name );

		if ( !user )
		{
			strncpy( name, cl->pers.netname, 15 );
			ilevel = 0;
		}
		else
		{
			strncpy( name, user->m_name, 15 );

			ilevel = user->Level();
		}

		if ( kots_lives->value )
		{
			Com_sprintf( entry, sizeof entry,
	           "xv 5 yv %d string2 \"%s\" "
						 "xv 128 yv %i string \"%4i %3i %5i %4i\" ", 
			       25 + i * 8,
						 name,
			       25 + i * 8,
			       cl->resp.score,
						 ilevel,
			       cl_ent->kots_lives,
			       cl->ping > 999 ? 999 : cl->ping );
		}
		else
		{
			Com_sprintf( entry, sizeof entry,
	           "xv 5 yv %d string2 \"%s\" "
						 "xv 133 yv %i string \"%4i %2i %3i %3i\" ", 
			       25 + i * 8,
						 name,
			       25 + i * 8,
			       cl->resp.score,
						 ilevel,
			       cl_ent->kots_kills,
			       cl->ping > 999 ? 999 : cl->ping );
		}
		if ( len + strlen( entry ) > maxsize ) 
			break;

		strcat( string, entry );
		len = strlen(string);
	}
	gi.WriteByte  ( svc_layout );
	gi.WriteString( string );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSStatScoreboard
//*************************************************************************************
//*************************************************************************************

void KOTSStatScoreboard( edict_t *ent )
{
	char  name  [20];
	char  entry [1024];
	char  string[1400];
	int	  len;
	int	  i, j, k;
	int	  total = 0;
	int	  sorted[MAX_CLIENTS];
	int   maxsize = 1024;
	float score;
	float sortedscores[MAX_CLIENTS];

	CUser *user;

	edict_t		*cl_ent;
	gclient_t	*cl;

	PMenu_Close( ent );

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse )
			continue;

		if ( cl_ent->kots_shots > 0 )
		{
			score  = (float)cl_ent->kots_hits / (float)cl_ent->kots_shots;
			score *= 100;
		}
		else
			score = 0;

		for ( j = 0; j < total; j++ )
		{
			if ( score > sortedscores[ j ] )
				break;
		}

		for ( k = total; k > j; k-- )
		{
			sorted      [k] = sorted      [k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j]        = i;
		sortedscores[j]  = score;

		total++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len     = 0;

	Com_sprintf( string, sizeof string, 
	         "xv 5 yv 15 string2 \"Player\" "
	         "xv 133 yv 15 string2 \"Shots\" "	
	         "xv 181 yv 15 string2 \"Hits\" "	
	         "xv 221 yv 15 string2 \"Accuracy\" "	
				 );

	len = strlen( string );

	for ( i = 0; i < total; i++ )
	{
		*entry = 0;

		cl     = &game.clients[sorted[i]];
		
		cl_ent = g_edicts + 1 + sorted[i];
	
		user = KOTSGetUser( cl_ent );

		memset( name, 0, sizeof name );

		if ( !user )
			strncpy( name, cl->pers.netname, 15 );
		else
			strncpy( name, user->m_name, 15 );

		Com_sprintf( entry, sizeof entry,
	           "xv 5 yv %d string2 \"%s\" "
						 "xv 133 yv %i string \"%5i %4i %3.3f%%\" ", 
			       25 + i * 8,
						 name,
			       25 + i * 8,
						 cl_ent->kots_shots,
			       cl_ent->kots_hits,
			       sortedscores[i] );

		if ( len + strlen( entry ) > maxsize ) 
			break;

		strcat( string, entry );
		len = strlen(string);
	}
	gi.WriteByte  ( svc_layout );
	gi.WriteString( string );
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

