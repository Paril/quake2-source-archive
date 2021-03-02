/****************************************************************************/
/* Expert Quake Matrix Code													*/
/* Michael Buttrey, mbuttrey@bigfoot.com, March 98							*/
/****************************************************************************/
#include "g_local.h"

#ifdef _WIN32
#pragma warning(default : 4100 4127 4701 4054)
#pragma warning(default : 4136 4051 4018 4305 4244)
#endif

//matrix
#define MATRIXROW 7
#define MATRIXTOP 10
#define MATRIXLEFT 3
#define LEARNROWS 9
#define MATRIXROWS 13
#define ENTRYCHARS 256

byte gcPlayers;
static byte* mpaTranslateRank = NULL;

// =============================================================================
//  Overlay Matrix Routines
// =============================================================================

int safediv (int numer, int denom, int zeroval)
{
	if (denom == 0) {
		return zeroval;
	}
	return numer / denom;
}

void DrawMatrix (edict_t* pedViewer, char* pszLayout, byte flags)
{
	byte	iRank;
	byte	iRank2;
	byte	iFirstRank;
	byte	iMiddle;
	byte	cLength;
	char	szEntry[ENTRYCHARS+1];
	char	szSubEntry[64];
	int		skill;
	byte	misc;
	byte	ypos;
	gclient_t*	pclClient;

	if (flags & MXLEARN) {
		cLength = LEARNROWS;
		iMiddle = LEARNROWS/2;
	} else {
		cLength = MATRIXROWS;
		iMiddle = MATRIXROWS/2;
	}
	
	if (cLength >= gcPlayers) {
		iFirstRank = 0;
		cLength = (byte)gcPlayers;
	} else if (pedViewer->client->resp.iRank <= iMiddle) {
		iFirstRank = 0;
	} else if (pedViewer->client->resp.iRank + iMiddle > gcPlayers) {
		iFirstRank = (byte)(gcPlayers - cLength);
	} else {
		iFirstRank = (byte)(pedViewer->client->resp.iRank - iMiddle);
	}

	//begin the drawing
	ypos = MATRIXTOP;
	strcpy (pszLayout, va("xv %d ", MATRIXLEFT));

	//print the first legend
	if (flags & MXLEARN) {
		strcat(pszLayout, va("yv %d string2 \"%s\" ", ypos, "Name Frags Ping Rank Eff FPM Skill"));
		ypos += MATRIXROW;
	}

	//print the statistics
	for (iRank = iFirstRank; iRank < iFirstRank+cLength; iRank++) {
		pclClient = (game.clients + mpaTranslateRank[iRank]);

		assert(strlen(pclClient->pers.netname) != 0);
		assert(pclClient->resp.iRank == iRank);

		skill = misc = 0;
		for (iRank2 = 0; iRank2 < gcPlayers; iRank2++) {
			if (pclClient->resp.paKillsVersus[iRank2]) {
				misc++;
				skill += pclClient->resp.paKillsVersus[iRank2] * game.clients[mpaTranslateRank[iRank2]].resp.score;
			}
		}

		strcpy(szSubEntry, va("%-12.12s %3i %3i %3i %2i %2i %3i"
			,pclClient->pers.netname
			,pclClient->resp.score
			,((pclClient->ping / 10) * 10)
			,pclClient->resp.score - pclClient->resp.cDeaths
			,safediv(pclClient->resp.score * 100, pclClient->resp.score + pclClient->resp.cDeaths, 0)
			,safediv(pclClient->resp.score, (level.framenum - pclClient->resp.enterframe)/600, pclClient->resp.score)
			,safediv(skill, misc, 0)
			));

		if (iRank == pedViewer->client->resp.iRank) {
			strcpy(szEntry, va("yv %d string2 \"%s\" ", ypos, szSubEntry));
		} else {
			strcpy(szEntry, va("yv %d string \"%s\" ", ypos, szSubEntry));
		}

		if (strlen(szEntry) + strlen(pszLayout) > 1399) 
			return;
		strcat (pszLayout, szEntry);
		ypos += MATRIXROW;
	}

	//add a empty line
	ypos += MATRIXROW;

	//print the second legend
	if (flags & MXLEARN) {
		strcpy(szSubEntry, va("%s,", game.clients[mpaTranslateRank[iFirstRank]].pers.netname));
		if (gcPlayers > 1) {
			strcat(szSubEntry, va(" %s,", game.clients[mpaTranslateRank[iFirstRank+1]].pers.netname));
		}

		strcpy(szEntry, va("yv %d string2 \"Kills v. %s etc.\" ", ypos, szSubEntry));
		if (strlen(szEntry) + strlen(pszLayout) > 1399) 
			return;
		strcat (pszLayout, szEntry);
		ypos += MATRIXROW;
	}

	//print the matrix
	for (iRank = iFirstRank; iRank < iFirstRank+cLength; iRank++) {

		if (flags & MXLEARN) {
			strcpy(szSubEntry, va("%-8.8s ", pedViewer->client->pers.netname));
		} else {
			szSubEntry[0] = '\0';
		}

		for (iRank2 = iFirstRank; iRank2 < iFirstRank+cLength; iRank2++) {
			strcat(szSubEntry, va("%2i", game.clients[mpaTranslateRank[iRank]].resp.paKillsVersus[mpaTranslateRank[iRank2]]));
		}

		if (iRank == pedViewer->client->resp.iRank) {
			strncpy(szEntry, va("yv %d string2 \"%s\" ", ypos, szSubEntry), ENTRYCHARS);
		} else {
			strncpy(szEntry, va("yv %d string \"%s\" ", ypos, szSubEntry), ENTRYCHARS);
		}

		if (strlen(szEntry) + strlen(pszLayout) > 1399) 
			return;
		strcat (pszLayout, szEntry);
		ypos += MATRIXROW;
	}
}

void RankPlayerUp (gclient_t* pclPlayer)
{
	int			myScore;
	byte		oldIndex;
	gclient_t*	pclOther;
	
	//while this player is better than the next player on the list
	for (myScore = pclPlayer->resp.score; pclPlayer->resp.iRank != 0; ) {
		pclOther = game.clients + mpaTranslateRank[pclPlayer->resp.iRank-1];

		if (pclOther->resp.score > myScore) {
			return;
		}

		//swap their translation table positions
		oldIndex = mpaTranslateRank[pclPlayer->resp.iRank];
		mpaTranslateRank[pclPlayer->resp.iRank] = mpaTranslateRank[pclOther->resp.iRank];
		mpaTranslateRank[pclOther->resp.iRank] = oldIndex;

		//swap their rank indexes
		pclPlayer->resp.iRank--;
		pclOther->resp.iRank++;
	} 
}

void RankPlayerDown (gclient_t* pclPlayer)
{
	int			myScore;
	byte		oldIndex;
	gclient_t*	pclOther;
	
	//while this player is better than the next player on the list
	for (myScore = pclPlayer->resp.score; pclPlayer->resp.iRank != gcPlayers-1; ) {
		pclOther = game.clients + mpaTranslateRank[pclPlayer->resp.iRank+1];

		if (myScore > pclOther->resp.score) {
			return;
		}

		//swap their translation table positions
		oldIndex = mpaTranslateRank[pclPlayer->resp.iRank];
		mpaTranslateRank[pclPlayer->resp.iRank] = mpaTranslateRank[pclOther->resp.iRank];
		mpaTranslateRank[pclOther->resp.iRank] = oldIndex;

		//swap their rank indexes
		pclPlayer->resp.iRank++;
		pclOther->resp.iRank--;
	} 
}

//updates the scores for the target and killer when the killer kills the target
void UpdateMatrixScores (edict_t* pedTarget, edict_t *pedKiller)
{
	//increment target's death counter
	pedTarget->client->resp.cDeaths++;

	//if the killer is a player, increment killer's KillsVersus against target and update killer's ranking
	if (pedKiller->client) {
		pedKiller->client->resp.paKillsVersus[mpaTranslateRank[pedTarget->client->resp.iRank]]++;
		RankPlayerUp(pedKiller->client);
	} else {
		//otherwise, it was suicide
		pedTarget->client->resp.paKillsVersus[mpaTranslateRank[pedTarget->client->resp.iRank]]++;
		RankPlayerDown(pedTarget->client);
	}
}

//prepares all arrays for a new player, expanding if necessary
void ExpandMatrix (edict_t* pedJoining)
{
	byte iNewRank;
	byte iClient; //for looping through all the clients
	edict_t *pedPlayer;
	
	if (mpaTranslateRank == NULL) {
		mpaTranslateRank = gi.TagMalloc(game.maxclients, TAG_LEVEL);
		memset(mpaTranslateRank, 0xCC, game.maxclients);
	}
	gcPlayers++;

/*	for (iClient = 0; iClient < game.maxclients; iClient++) {
		pedPlayer = g_edicts + iClient + 1;
		//if the names match, it's him
		if (pedPlayer->inuse) {
			mcPlayers++;
		}
	}*/

	//zero data for client
	pedJoining->client->resp.cDeaths = 0;

	//allocate and zero the client's new KillsVersus array
	//assumption: size of the array is one byte per score
	pedJoining->client->resp.paKillsVersus = gi.TagMalloc(game.maxclients, TAG_LEVEL);
	memset(pedJoining->client->resp.paKillsVersus, 0, game.maxclients);
	assert(sizeof(pedJoining->client->resp.paKillsVersus[0]) == 1);

	//place the client at the end of the rankings
	iNewRank = (byte)(gcPlayers - 1);
	pedJoining->client->resp.iRank = iNewRank;

	//find his client array position
	for (iClient = 0; iClient < game.maxclients; iClient++) {
		pedPlayer = g_edicts + iClient + 1;
		//if the names match, it's him
		if (pedPlayer->inuse && StrMatch(pedJoining->client->pers.netname, pedPlayer->client->pers.netname)) {
			mpaTranslateRank[iNewRank] = iClient; 
			break;
		}
	}

	//call RankPlayer to correctly rank the client
	RankPlayerUp(pedJoining->client);
}

void ContractMatrix (edict_t* pedLeaving)
{
	gcPlayers--;

	//move the player to the end of the scores
	pedLeaving->client->resp.score = -999;
	//rank the player, so they'll be at the end of everything
	RankPlayerDown(pedLeaving->client);
}