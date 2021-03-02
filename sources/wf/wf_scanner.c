/*==============================================================================
The Weapons Factory - 
Scanner Functions
Original code by ??
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"

void ClearScanner(gclient_t *client)
{
	client->pers.scanner_active=0;
}


void ShowScanner(edict_t *ent,char *layout)
{
//		int     i;

        edict_t *player = g_edicts;
		edict_t *target;

        char    stats[64];
//		float	len;
		int		hd;
//        int		sx, sy;
        vec3_t  v;
		vec3_t  dp;
		vec3_t  normal = {0,0,-1};

        // Main scanner graphic draw
        Com_sprintf (stats, sizeof(stats),"xv 80 yv 40 picn %s ", PIC_SCANNER_TAG);
        SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);


		target = ent->client->tracker_target;
		if (!target) return;

		//See if target is in front
		if (!infront(ent, target)) return;

		// calc player to decoy vector
		VectorSubtract (ent->s.origin, target->s.origin, v);

//gi.dprintf("v 0=%f, 1=%f, 2=%f\n",v[0], v[1], v[2]);

		// save height differential
		hd = v[2];

		// remove height component
//		v[2] = 0;

		// normal vector to enemy
		VectorNormalize(v);

		// rotate round player view angle (yaw)
		RotatePointAroundVector( dp, normal, v, ent->s.angles[1]);

		// setup arrows
		if (dp[1] < -.1)
		{
			Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
				118, 108, PIC_LEFT_TAG);
		}
		else if (dp[1] > .1)
		{
			Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
				178, 108, PIC_RIGHT_TAG);
		}

		SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);

		// clear stats
		*stats = 0;

		// set up/down arrow
		if (dp[2] < -.03)
		{
			Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
				148, 88,PIC_UP_TAG);
		}
		else if (dp[2] > .03)
		{
			Com_sprintf (stats, sizeof(stats),"xv %i yv %i picn %s ",
				148, 128,PIC_DOWN_TAG);
		}

//gi.dprintf("dp 0=%f, 1=%f, 2=%f, hd=%f\n",dp[0], dp[1],  dp[2], hd);

		// any up/down ?
		if (*stats)
			SAFE_STRCAT(layout,stats,LAYOUT_MAX_LENGTH);

}
void Toggle_Scanner(edict_t *ent)
{
	char string[1040];

	if ((!ent->client) || (ent->health <=0))
		return;

	ent->client->showinventory = 0;
	ent->client->showscores = 0;

	//toggle low on/off bit (and clear scores/inventory display if required)
	if (ent->client->pers.scanner_active)
	{
		ent->client->pers.scanner_active = 0;
	}
	else
	{

		//What do we want to track?
		//ent->client->tracker_target = ent->decoy;

		if (ent->wf_team == 1)
			ent->client->tracker_target = G_Find(NULL, FOFS(classname), "item_flag_team2");
		else if (ent->wf_team == 2)
			ent->client->tracker_target = G_Find(NULL, FOFS(classname), "item_flag_team1");
		else 
			ent->client->tracker_target = NULL;

		string[0] = 0;
		ent->client->pers.scanner_active = 1;
		ShowScanner(ent,string);
		gi.WriteByte (svc_layout);
		gi.WriteString (string);
	}
}
