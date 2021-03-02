/****************************************************************************/
/* Expert Quake Overlay Code												*/
/* Michael Buttrey, mbuttrey@bigfoot.com, March 98							*/
/****************************************************************************/

#include "g_local.h"
#include "e_iovr.h"

// Conventions: Pure Block Emulation, Pure Hungarian
// "ed" is an edict. "sz" is a zero terminated string.

//testing
static int testvar;

//radar
static char blip[MAX_HORIZ+1][MAX_VERT+1];
static qboolean IsColumn[MAX_HORIZ+1];
static qboolean IsRow[MAX_VERT+1];

//current player & settings
static edict_t *mpedCur;
static unsigned short curMode = LEARNON, curScreen;
static unsigned short hpos, vpos;
static unsigned short hscale, vscale;

//arrays
static sightEnt_t* mpaSightings = NULL;
static plotEnt_t* mpaFixtures = NULL;
static short mcFixtures;

// =============================================================================
//  Overlay Special Routines
// =============================================================================
void OverlayPrint(edict_t* pedTarget, byte flags, const char* pszEntry, byte cUpdates)
{
	char szAddition[TIMEDENTRY_CHARS+1];
	qboolean Success = false;
	qboolean Expired = false;
	assert(pedTarget != NULL && cUpdates > 0);
	
	if (pedTarget->client->cUpdatesLeft == 0) {
		Expired = true;
	}

	if (!Expired && flags & PRINT_APPEND) {
		Com_sprintf(szAddition, sizeof(szAddition),
			"xv %d yv %d string \"%s\" ",
			160 - 4 * strlen(pszEntry),	
			120 - 4 * numchr(pedTarget->client->szTimedEntry, '\"'),
			pszEntry);

		if (strlen(pedTarget->client->szTimedEntry) +
			strlen(szAddition) <= TIMEDENTRY_CHARS) {

			strcat(pedTarget->client->szTimedEntry, szAddition);
			pedTarget->client->cUpdatesLeft = cUpdates;
			Success = true;
		}
	} 

	if (!Success && (Expired || flags & PRINT_FORCE)) {
		Com_sprintf(szAddition, sizeof(szAddition), 
			"xv %d yv 120 string \"%s\" ", 
			160 - 4 * strlen(pszEntry),	pszEntry);
		strncpy(pedTarget->client->szTimedEntry, szAddition, TIMEDENTRY_CHARS);
		pedTarget->client->cUpdatesLeft = cUpdates;
		Success = true;
	}

	if (Success && flags & UPDATE_NOW) {
		OverlayUpdate(pedTarget);
	}
}

void RecordSighting(edict_t *pedViewer, edict_t *pedTarget)
{
	byte iSight;

	assert(expflags & EXPERT_RADAR && pedViewer->client->resp.team != pedTarget->client->resp.team);

	//either add the enemy to an empty spot, or update
	//the existing enemy's expiry, whichever happens first.
	for (iSight = 0; iSight < game.maxclients; iSight++) {
		if (mpaSightings[iSight].expiryframe < level.framenum) {
			mpaSightings[iSight].pedTarget = pedTarget;
			mpaSightings[iSight].expiryframe = level.framenum + EXPERT_UPDATE_FRAMES * 2;
			mpaSightings[iSight].teamSeenBy = (byte)pedViewer->client->resp.team; 
			return;

		} else if (mpaSightings[iSight].teamSeenBy == pedViewer->client->resp.team &&
			mpaSightings[iSight].pedTarget == pedTarget) {
			mpaSightings[iSight].expiryframe = level.framenum + EXPERT_UPDATE_FRAMES * 2;
			return;
		}
	}

	gi.dprintf("Expert Overlay Warning: Sightings Overflow.\n");
}

void InitRadar(void)
{
	edict_t *pedBlip = NULL;
	short cEntsAllocated;

	mpaSightings = gi.TagMalloc(game.maxclients*sizeof(sightEnt_t), TAG_LEVEL); 

	mpaFixtures = gi.TagMalloc(64*sizeof(plotEnt_t), TAG_LEVEL);
	cEntsAllocated = 32;

	for (pedBlip = g_edicts + game.maxclients + 1; pedBlip < g_edicts + globals.num_edicts; pedBlip++) {
		if (strncmp(pedBlip->classname, "wea", 3) == 0) {
			mpaFixtures[mcFixtures].pedTarget = pedBlip;
			mpaFixtures[mcFixtures].pic = WEAP;
			mcFixtures++;
			if (mcFixtures == cEntsAllocated) {
				ResizeLevelMemory(&mpaFixtures, 
					(cEntsAllocated*2)*sizeof(plotEnt_t), 
					cEntsAllocated*sizeof(plotEnt_t));
				cEntsAllocated *= 2;
			}

		} else if (strncmp(pedBlip->classname, "ite", 3) == 0) {
			if (strcmp(pedBlip->classname, "item_p") == 0 ||
				strcmp(pedBlip->classname, "item_inv") == 0 ||
				strcmp(pedBlip->classname, "item_qua") == 0 ||
				strcmp(pedBlip->classname, "item_ban") == 0 ||
				strcmp(pedBlip->classname, "item_health_mega") == 0 ||
				strcmp(pedBlip->classname, "item_sil") == 0 ||
				strcmp(pedBlip->classname, "item_armor_body") == 0 ||
				strcmp(pedBlip->classname, "item_armor_combat") == 0) {

				mpaFixtures[mcFixtures].pedTarget = pedBlip;
				mpaFixtures[mcFixtures].pic = ITEM;
				mcFixtures++;
				if (mcFixtures == cEntsAllocated) {
					ResizeLevelMemory(&mpaFixtures, 
						(cEntsAllocated*2)*sizeof(plotEnt_t), 
						cEntsAllocated*sizeof(plotEnt_t));
					cEntsAllocated *= 2;
				}
			}
		}
	}
}

// =============================================================================
//  Overlay Radar Routines
// =============================================================================

//Places a position and picture pair in the array for drawing
void Plot (edict_t *pedBlip, char pic, int flags)
{
	vec3_t	offset;
	vec3_t	NORMAL = {0,0,-1};
	vec3_t	final;
	int col, row, layer;

	VectorSubtract (mpedCur->s.origin, pedBlip->s.origin, offset);
	
	layer = (int)(offset[2]);
	offset[2] = 0.0;

	//FIXME: Circular boundary comparison
	//take the cLength from origin / using x and y only
	//cLength = sqrt (v[0]*v[0] + v[1]*v[1]);

	if (curMode & LOCKFIT) {
		RotatePointAroundVector(final, NORMAL, offset, 90.0);
	} else {
		//FIXME: various hacks - is the angle off by 90 degrees?
		RotatePointAroundVector(final, NORMAL, offset, mpedCur->s.angles[YAW]);
	}

	//FIXME: Inelegant
	if (curMode & ZVERT) {
		row = (int)(layer / vscale) + scrHeight[curScreen]/2;
		layer = (int)(final[1] / Y_LAYERSCALE);
	} else {
		row = (int)(final[0] / vscale) + scrHeight[curScreen]/2;
	}
	col = (int)(final[1] / hscale) + scrWidth[curScreen]/2;

	//FIXME: Add seperate arrow for above/below
//	if (flags & PLOT_LAYER) {
//		if (layer < BELOW_FEET) {
//			pic = PLOT_BELOW;
//		} else if (layer > ABOVE_FEET) {
//			pic = PLOT_ABOVE;
//		}
//	}

	//plot on edge, even if out of range
	if (flags & PLOT_EDGE) {
		col = MAX(MIN(col, scrWidth[curScreen]), 0);
		row = MAX(MIN(row, scrHeight[curScreen]), 0);
	} else if (col != MAX(MIN(col, scrWidth[curScreen]), 0) ||
		row != MAX(MIN(row, scrHeight[curScreen]), 0)) {
		return;
	}
		
	if (flags & PLOT_FORCE || blip[col][row] == NOPIC) {
		blip[col][row] = pic;
		IsColumn[col] = true;
		IsRow[row] = true;
	} 
}

void PlotFixtures(void)
{
	short i;

	for (i = 0; i < mcFixtures; i++) {
		switch (mpaFixtures[i].pic) {
			case WEAP:
			if (curMode & SHOWWEAP) {
				Plot(mpaFixtures[i].pedTarget, mpaFixtures[i].pic, 0);
			}

			break; case ITEM:
			if (curMode & SHOWPOWER) {
				Plot(mpaFixtures[i].pedTarget, mpaFixtures[i].pic, 0);
			}

			break; default:
				Plot(mpaFixtures[i].pedTarget, mpaFixtures[i].pic, 0);
		}
		
	}
}

void PlotPlayers(void)
{
	edict_t* pedPlayer;
	byte iSight;
	byte iClient;

	//go through every client in the game
	for (iClient = 0; iClient < game.maxclients; iClient++) {
		pedPlayer = g_edicts + iClient + 1;
		if (pedPlayer->inuse && mpedCur->client->resp.team == pedPlayer->client->resp.team) {
			Plot(pedPlayer, FRIEND, 0);
		}
	}

	for (iSight = 0; iSight < game.maxclients; iSight++) {
		if (mpaSightings[iSight].expiryframe > level.framenum &&
			mpaSightings[iSight].teamSeenBy == mpedCur->client->resp.team) {
			Plot(mpaSightings[iSight].pedTarget, ENEMY, 0);
		}
	}
}

//Appends all plots, backdrop and stuff to string
void AppendRadar (char *pszLayout)
{
	char	szEntry[20];
	short	col, row;
	short	cRows, cCols;

	//empty it out
	for (col = 0; col <= scrWidth[curScreen]; col++) {
		IsColumn[col] = false;
		for (row = 0;row <= scrHeight[curScreen]; row++) {
			IsRow[row] = false;
			blip[col][row] = NOPIC;
		}
	}

	//master plotting systems
	if (gametype == GAME_DM) {
#ifdef PIDTEST
		PlotPlayers();
#endif
		PlotFixtures();
	} else {
		PlotPlayers();
		PlotFixtures();
	}
	
	//backdrop
	if (curMode & SHOWBACK) {
		blip[0][0] = scrPic[curScreen];
		IsColumn[0] = true;
		IsRow[0] = true;
	}

	cCols = cRows = 0;
	for (col = 0; col <= scrWidth[curScreen]; col++) {
		if (IsColumn[col]) 
			cCols++;
	}

	for (row = 0; row <= scrHeight[curScreen]; row++) {
		if (IsRow[row]) 
			cRows++;
	}

	//two versions of blip drawing
	//one for column by column, other for row by row
	if (cRows >= cCols) {
		for (col = 0; col <= scrWidth[curScreen]; col++) {
			if (!IsColumn[col]) 
				continue; 
			strcpy(szEntry, va("xl %d ", hpos+col));
			if (strlen(szEntry) + strlen(pszLayout) > 1399) 
				return; 
			strcat(pszLayout, szEntry);

			for (row = 0; row <= scrHeight[curScreen]; row++) {
				if (blip[col][row] == NOPIC) 
					continue; 
				strcpy(szEntry, va("yt %d picn %c ", vpos+row, blip[col][row]));
				if (strlen(szEntry) + strlen(pszLayout) > 1399) 
					return; 
				strcat(pszLayout, szEntry);
			}
		}	
	} else {
		for (row = 0; row <= scrHeight[curScreen]; row++) {
			if (!IsRow[row]) 
				continue; 

			strcpy (szEntry, va("yt %d ", vpos+row));
			if (strlen(szEntry) + strlen(pszLayout) > 1399) 
				return; 

			strcat(pszLayout, szEntry);
			for (col = 0; col <= scrWidth[curScreen]; col++) {	
				if (blip[col][row] == NOPIC) 
					continue; 

				strcpy(szEntry, va("xl %d picn %c ", hpos+col, blip[col][row]));
				if (strlen(szEntry) + strlen(pszLayout) > 1399) 
					return; 

				strcat(pszLayout, szEntry);
			}
		}
	}
}

// =============================================================================
//  Overlay Control Routines
// =============================================================================

void SetIntKey(char sKey[32], int value)
{
	StuffCmd(mpedCur, va("set %s %i u \n", sKey, value));

	if (strlen(Info_ValueForKey(mpedCur->client->pers.userinfo, sKey)) == 0) {
		StuffCmd(mpedCur, va("set %s %i u \n", sKey, value));
	}
}

short ReadShortKey(char *psKey, short min, short max, short def)
{
	int value;
	value = StrToInt(Info_ValueForKey(mpedCur->client->pers.userinfo, psKey), def);

	if (value < min || value > max) 
		return def;
	return (short)value;
}

void ResetConfig(void)
{	
	SetIntKey("hpos", INIT_HPOS);
	SetIntKey("vpos", INIT_VPOS);
	SetIntKey("hscale", INIT_HSCALE);
	SetIntKey("vscale", INIT_VSCALE);
	SetIntKey("screen", INIT_SCREEN);

	//change to an invalid value, so LEARN_MODE will not always be on or off
	SetIntKey("mode", -1);
}

void SetCurrentPlayer(void)
{
	hpos = ReadShortKey("hpos", 0, 1600, INIT_HPOS);
	vpos = ReadShortKey("vpos", 0, 1600, INIT_VPOS);
	hscale = ReadShortKey("hscale", 2, 1000, INIT_HSCALE);
	vscale = ReadShortKey("vscale", 2, 1000, INIT_VSCALE);
	curScreen = ReadShortKey("screen", 0, SCREENS-1, INIT_SCREEN);
	if (gametype == GAME_DM) {
		curMode = ReadShortKey("mode",  0, 32767, INIT_MODE | LEARN_MODE);
	} else {
		curMode = ReadShortKey("mode",  0, 32767, INIT_MODE);
	}
}

void SetProperMode (short bit)
{
	char* pszParam = gi.argv(1);
	short oldMode = curMode;

	//if on/yes/1, turn on
	if (StrMatch(pszParam, "on") || 
		StrMatch(pszParam, "yes") || 
		StrToInt(pszParam, -1) == 1)
		curMode |= bit;
	//otherwise if off/no/0, turn off
	else if (StrMatch(pszParam, "off") || 
		StrMatch(pszParam, "no") || 
		StrToInt(pszParam, -1) == 0)
		curMode &= ~bit;
	//otherwise, toggle
	else curMode ^= bit;

	if (oldMode != curMode) {
		SetIntKey("mode", curMode);
	}
}

void PrintProperMessage (char *pszType, int value)
{
	int parameter = StrToInt(gi.argv(1), -999);

	if (parameter == -999)
		gi.cprintf(mpedCur, PRINT_HIGH, "Current %s setting: %i. \n", pszType, value);
	else
		gi.cprintf(mpedCur, PRINT_HIGH, "%i is an unusable %s setting. Please try a different value.\n", parameter, pszType);
}

qboolean ValidOverlayCommand (edict_t *pedPlayer)
{
	char* szCommand = gi.argv(0);
	int parameter = StrToInt(gi.argv(1), -1);

	mpedCur = pedPlayer;
	if (expflags & EXPERT_RADAR) {
		SetCurrentPlayer();
	}

	if (StrMatch(szCommand, "reset")) {
		ResetConfig();

	} else if (StrMatch(szCommand, "overlay")) {
		gi.cprintf(mpedCur, PRINT_HIGH, OVERLAYVER);

	} else if (StrMatch(szCommand, "print")) {
	//	OverlayPrint(mpedCur, PRINT_APPEND | PRINT_FORCE | UPDATE_NOW, gi.args, 5);

	} else if (StrMatch(szCommand, "legend")) {
		switch(gametype) {
			case GAME_DM:
				break;
			default:
				gi.cprintf(mpedCur, PRINT_HIGH, "No legend defined for current game mode.\n");
				break;
		}
		if (curMode & SHOWWEAP || curMode & SHOWPOWER) {
				gi.cprintf(mpedCur, PRINT_HIGH, LEGEND_ITEMS);
		}

	//mode toggles		
	} else if (StrMatch(szCommand, "matrix")) {
		mpedCur->client->showOverlay ^= 2;
		OverlayUpdate(mpedCur);
	} else if (StrMatch(szCommand, "LEARN"))
		SetProperMode (LEARNON);
	else if (StrMatch(szCommand, "RADAR"))
		SetProperMode (RADARON);
	else if (StrMatch(szCommand, "backdrop")) 
		SetProperMode (SHOWBACK);
	else if (StrMatch(szCommand, "rotation")) 
		SetProperMode (LOCKFIT);
	else if (StrMatch(szCommand, "ZVERT"))
		SetProperMode (ZVERT);
	else if (StrMatch(szCommand, "SHOWWEAP"))
		SetProperMode (SHOWWEAP);
	else if (StrMatch(szCommand, "SHOWPOWER")) {
		SetProperMode (SHOWPOWER);

	//setting commands
	} else if (StrMatch(szCommand, "hpos")) {
		if (parameter > 0 && parameter < 1600)
			SetIntKey("hpos", parameter);
		else
			PrintProperMessage("horizontal position", hpos);

	} else if (StrMatch(szCommand, "testvar")) {
		if (parameter == -9999)
			gi.cprintf(mpedCur, PRINT_HIGH, "Testvar value: %i\n", testvar);
		else
			testvar = parameter;
		
	} else if (StrMatch(szCommand, "vpos")) {
		if (parameter > 0 && parameter < 1280)
			SetIntKey("vpos", parameter);
		else
			PrintProperMessage("vertical position", vpos);

	} else if (StrMatch(szCommand, "scale")) { //both
		if (parameter > 2 && parameter < 1000) {
			SetIntKey("hscale", parameter);
			SetIntKey("vscale", parameter);
		} else
			PrintProperMessage("scale", vscale);

	} else if (StrMatch(szCommand, "hscale")) {
		if (parameter > 2 && parameter < 1000)
			SetIntKey("hscale", parameter);
		else
			PrintProperMessage("horizontal scale", hscale);

	} else if (StrMatch(szCommand, "vscale")) {
		if (parameter > 2 && parameter < 1000)
			SetIntKey("vscale", parameter);
		else
			PrintProperMessage("vertical scale", vscale);

	} else if (StrMatch(szCommand, "screen")) {
		if (parameter > 0 && parameter <= SCREENS)
			SetIntKey("screen", parameter-1);
		else
			PrintProperMessage("screen", curScreen+1);

	} else if (StrMatch(szCommand, "mode")) { //direct changes
		if (parameter >= 0 && parameter < 32767)
			SetIntKey("mode", parameter);
		else
			PrintProperMessage("mode", curMode);

	} else {
		return false; //command doesn't match
	}

	return true; //command matched
}

void OverlayUpdate(edict_t *pedViewer)
{
	char szLayout[1400] = "";
	qboolean hasChanged = false;
	
	pedViewer->client->updateFrame = level.framenum + EXPERT_UPDATE_FRAMES;
	mpedCur = pedViewer;
	if (expflags & EXPERT_RADAR) {
		if (mpaSightings == NULL) {
			InitRadar();
		}
		SetCurrentPlayer();
	}
	
	if (pedViewer->client->cUpdatesLeft) {
		strcpy(szLayout, pedViewer->client->szTimedEntry); 
	}

	if (pedViewer->client->showOverlay & 2) {
		if (curMode & LEARNON)
			DrawMatrix(mpedCur, szLayout, MXLEARN);
		else 
			DrawMatrix(mpedCur, szLayout, 0);
	} else if (expflags & EXPERT_RADAR && curMode & RADARON) {
		AppendRadar(szLayout);
	} else if (pedViewer->client->cUpdatesLeft) {
		pedViewer->client->cUpdatesLeft--;
	} else {
		pedViewer->client->szOldLayout[0] = '\0';
		pedViewer->client->showOverlay = 0;
		return;
	}

	pedViewer->client->showOverlay |= 1;

	// If it's the same as last time, quit but leave showscores on
	if (strncmp(pedViewer->client->szOldLayout, szLayout, OLDLAYOUT_CHARS) == 0) {
		return;
	}
	strncpy (pedViewer->client->szOldLayout, szLayout, OLDLAYOUT_CHARS);

	//final bandwidth reduction: no need for trailing space
	szLayout[strlen(szLayout)-1] = '\0';

#ifdef PIDTEST
	gi.cprintf(mpedCur, PRINT_MEDIUM, "%i\n", strlen(szLayout));
#endif

	gi.WriteByte(svc_layout);
	gi.WriteString (szLayout);
	gi.unicast(mpedCur, false);
}
