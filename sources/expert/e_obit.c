/*
	e_obit.c

	Implementation of Expert Obituary Functions ansilary functions. e.g. Init, Parsing

	Credit is due to "SteQve" for ServObits 1.1 
		Some of the functionality within this module was modeled after the 
		code in ServObits.c code. Originally, I was going to use most of 
		ServObits as it stood for the Expert code. Unfortunately (for me), 
		the structure just wouldn't fit quite right into the Expert mod due 
		to the complexities of the "Context" system that was devised. Some of 
		the "helper" functions were slightly modified and included.
			String replacement functions.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "g_local.h"

unsigned int gMemAllocated; // Total bytes allocated for Obituaries.
unsigned int gMsgCount;	// Total number of messages loaded.

// List of causes of death as specified in obituary.txt file
#define 	NUM_CAUSES		35

char o_Causes[NUM_CAUSES][20] = 
{
CAUSE_UNDEFINED,
"blaster",
"shotgun",
"sshotgun",
"machinegun",
"chaingun",
"grenade",
"grenade splash",
"rocket",
"rocket splash",
"hyperblaster",
"railgun",
"bfg laser",
"bfg blast",
"bfg effect",
"handgrenade",
"handgrenade splash",
"water",
"slime",
"lava",
"crush",
"telefrag",
"falling",
"suicide",
"held grenade",
"explosive",
"barrel",
"bomb",
"exit",
"splash",
"target laser",
"trigger hurt",
"hit",
"target blaster",
"grapple"
};

// Table from names of contexts as used in the obituary.txt file to
// context flags (by bit number, ie 4 = 2^4)
#define 	NUM_CONTEXTS	29

char o_Contexts[NUM_CONTEXTS][20] = 
{
"victim female",
"victim male",
"attacker female",
"attacker male",
"attacker has quad",
"attacker has invuln",
"victim has quad",
"victim has invuln",
"victim short life",
"victim long life",
"attacker long life",
"leg hit",
"torso hit",
"head hit",
"front hit",
"side hit",
"back hit",
"gibbed",
"pointblank range",
"extreme range",
"mercy kill",
"victim on ground",
"attacker on ground",
"victim airborne",
"attacker airborne",
"victim above",
"attacker above",
"friendly fire",
"kill self"
};

int GetEntityGender(edict_t *ent);
void PrintRandObitMsg(char *aName, char *vName, int aGender, int vGender, obits_t *ob);
unsigned int DiscoverContexts(edict_t *vict, edict_t *infl, edict_t *attk, vec3_t point);
int StrBeginsWith (char *s1, char *s2);
int InsertValue(char *mess, char *format_str, char *field, char *value, int max_size);
void DisplayBestObituaryMessage(int cause, unsigned int context, int aGender, int vGender, char *aName, char *vName);
obits_t* messagesForContext(unsigned int cause, unsigned int cFlag);
qboolean ReTagObitData();
obits_t** TagMallocObituary(obitContainer_t *obitCont);
char** TagMallocMessages(obits_t *obitEntry);
unsigned int getCauseNumber(char *szCause);
int getContextNumber(char *szContext);
unsigned int getContextBit(char *szContext);
obitContainer_t **LoadMessageTree(const char *szFilename);
void ConsoleKill (edict_t *self);
void DumpChars(void);
void MacroAddAll(unsigned int cFlag, const char* message);

// debug
void printContext(unsigned int context);

//
// Main entry point for Expert Obituaries
//
void ExpertClientObituary (edict_t *victim, edict_t *inflictor, edict_t *attacker, int cod, vec3_t point)
{

	char			*aName = NULL;
	char			*vName = NULL;

	char			*szCause = NULL;
	unsigned int	context = 0;		// Context of death. 0 is nothing special.
	int 			cause = 0;
	int				aGender = 0;
	int				vGender = 0;

	// cod is the value contained in the methodOfDeath variable. 
	// note "victim" is always a player but attacker can validly
	// be NULL (console kill) or not a player (world)

	if (cod & MOD_FRIENDLY_FIRE)
		context |= CON_FRIENDLY_FIRE;
	
	cause = cod & ~MOD_FRIENDLY_FIRE;
	
	// scoring for the kill

	// killed self
	if (attacker == victim)
	{
		victim->client->resp.score--;
		victim->enemy = NULL;
	} else {
		// killed another player
		victim->enemy = attacker;

		// killed teammate
		if (context & CON_FRIENDLY_FIRE)
		{
			attacker->client->resp.score--;
		} else {
			// if attacker was a player, frag for the attacker
			if (attacker && attacker->client)
			{
				attacker->client->resp.score++;
				attacker->client->killSpree++;
			} else {
				// attacker was world (falling, lava), exploding box,
				// anything else that's not a player.  Lose a frag.
				victim->client->resp.score--;
			}
		}
	}
	
	context = DiscoverContexts(victim, inflictor, attacker, point);

	//gi.dprintf("Expert obit; cause is %d, context is %d\n", cause, context);

	if (utilflags & EXPERT_ENABLE_GIBSTAT_LOGGING)
	{
		if (context & CON_KILL_SELF)
			gsLogKillSelf(victim, cod);
		else
			gsLogFrag(victim, attacker, cod);
	}

	// Get genders and names.
	aGender = GetEntityGender(attacker);
	vGender = GetEntityGender(victim);

	if (attacker == NULL || attacker->client == NULL)
	{
		DisplayBestObituaryMessage(cause, context, aGender, vGender, 
									NULL, victim->client->pers.netname);
	} else {
		DisplayBestObituaryMessage(cause, context, aGender, vGender, 
									attacker->client->pers.netname, 
									victim->client->pers.netname);
	}
}



void DisplayBestObituaryMessage(int cause, unsigned int context,
								int aGender, int vGender,
								char *aName, char *vName)
{
	obits_t 		*ob = NULL; 	//Store the returned obituary

	unsigned int	i, j;
	unsigned int	mask = 0;
	unsigned int	contextFlags = context;

	int 			bitsSet;

	//gi.dprintf("Cause is %s[%d], contexts are ", o_Causes[cause], cause);
	//printContext(context);

	// Use the exact match context search.
	ob = messagesForContext(cause, contextFlags);

	if (ob != NULL) 			// Exact match was found
	{
		//gi.dprintf("Full match on context\n");
		PrintRandObitMsg(aName, vName, aGender, vGender, ob);
		return;
	}

	// algorithm: Since there were no messages defined for this exact set of context flags
	// we'll look for messages that match some subset of the flags that were set.
	// We prefer primarily to match as many of the flags as possible, and given the same 
	// number of flags, prefer to match the higher-order flags.
	bitsSet = numberOfBitsSet(contextFlags);
	//gi.dprintf("%d bits set\n", bitsSet);
	for (i = 0; i < bitsSet; i++)
	{
		// mask off each flag in turn
		for (j = 0; j < (sizeof(unsigned int) * 8); j++)
		{
			mask = (unsigned int)1 << j;	// Set bit mask
			if (mask & contextFlags) { // mask matches a bit set in contextFlags
				//gi.dprintf("Masking flag %s\n", o_Contexts[log2(mask)]);
				ob = messagesForContext(cause, contextFlags & ~mask);
				if (ob != NULL) {
					//gi.dprintf("Found near match on context: dropped %d flag(s)\nFinal context was ",
					//		bitsSet - numberOfBitsSet(contextFlags & ~mask));
					//printContext(contextFlags & ~mask);
					PrintRandObitMsg(aName, vName, aGender, vGender, ob);
					return;
				}
			}
		}
		// no match for any combination of bitsSet - 1 flags.  
		// Remove the lowest order flag and try again
		//gi.dprintf("Dropping flag %s\n", o_Contexts[log2(lowestOrderBit(contextFlags))]);
		contextFlags &= ~lowestOrderBit(contextFlags); 
	}

	//gi.dprintf("No messages defined for cause\n");
	ob = messagesForContext(0, 0);
	PrintRandObitMsg(aName, vName, aGender, vGender, ob);

}

void PrintRandObitMsg(char *aName, char *vName, 
						int aGender, int vGender, obits_t *ob)
{
	int iRnd = 0;
	char *szObit;
	char *szTemp;
	char *atkName;
	char *vicName;

	int vicSeparator = 156;
	char *pronouns[3][2] =	{	{OBIT_HE, OBIT_SHE},
								{OBIT_HIM, OBIT_HER},
								{OBIT_HIS, OBIT_HER}
							};

	if (ob == NULL)
	{	// No message passed in.
		gi.bprintf (PRINT_MEDIUM,"%s died and nobody wrote an obituary.\n", vName);
		return;
	}

	szTemp = calloc(sizeof(char), 512);
	szObit = calloc(sizeof(char), 512);

	atkName= calloc(sizeof(char), 100);
	vicName= calloc(sizeof(char), 100);
	// Make the attacker's name green if name ! null
	if (aName != NULL)
		greenCopy(atkName, aName);

	// Place dot separators around the victims name
	//strcpy(vicName, &(char)vicSeparator);
	//strcat(vicName, vName);
	//strcat(vicName, &(char)vicSeparator);

	strcpy(vicName, vName);

	if (ob->msgCount > 1 )
	{
		srand(time(NULL));		// Make sure it's random.
		iRnd = rand() % ob->msgCount;
		strcpy(szObit, ob->messages[iRnd]);
	} else {
		strcpy(szObit, ob->messages[0]);
	}

	// substitute the proper strings for the gender pronoun placeholders
	insertValue(szTemp, szObit, OTOK_ATTACKER_NAME, atkName, 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_ATTACKER_HE, pronouns[0][aGender], 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_ATTACKER_HIM, pronouns[1][aGender], 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_ATTACKER_HIS, pronouns[2][aGender], 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_VICTIM_NAME, vicName, 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_VICTIM_HE, pronouns[0][vGender], 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_VICTIM_HIM, pronouns[1][vGender], 512);
	strcpy(szObit, szTemp);
	insertValue(szTemp, szObit, OTOK_VICTIM_HIS, pronouns[2][vGender], 512);
	strcpy(szObit, szTemp);

	gi.bprintf (PRINT_MEDIUM,"%s\n",szObit);
	free(szTemp);
	free(szObit);
}

void DisplayObituaryInfo(edict_t *ent)
{

	if (gCauseTable == NULL)
	{
		if (utilflags & EXPERT_DISABLE_CLIENT_OBITUARIES )
			gi.cprintf (ent, PRINT_MEDIUM, "Expert Client Obituary Disabled.\n");
		else
			gi.cprintf (ent, PRINT_HIGH, "Expert Client Obituary failed to load.\n");
	} else {
		gi.cprintf (ent, PRINT_MEDIUM, "\nExpert Client Obituary Memory Stats\n");
		gi.cprintf (ent, PRINT_MEDIUM, "-----------------------------------\n");
		gi.cprintf (ent, PRINT_MEDIUM, "Total Number of Messages: %i\n",
							gMsgCount);
		gi.cprintf (ent, PRINT_MEDIUM, "Memory Used in bytes:     %i\n", 
							gMemAllocated);
		gi.cprintf (ent, PRINT_MEDIUM, "\n");
	}
}

// This routine interrogates the victim, inflictor and attacker for context flags
// and returns any found. The routine also tries to do the minimum work possible;
// i.e. it won't run through attacker contexts if it's a self kill.
unsigned int DiscoverContexts(edict_t *vict, edict_t *infl, edict_t *attk, vec3_t point)
{
	unsigned int c = 0;
	int			 gibVal = -40;
	float		 distance = 0.0;	// Distance between attk/vict
	vec3_t vec, forward;
	float dot;

	// First, compute the health at which gibbing happens
	gibVal *= sv_lethality->value;

	// Check the Gender. We'll assume that most genders are male.
	if (GetEntityGender(vict) == OBIT_GENDER_FEMALE) {
		c |= CON_VICTIM_FEMALE;
	} else {
		c |= CON_VICTIM_MALE;
	}

	// Check if victim is on ground or airborne.
	if (vict->groundentity)
		c |= CON_VICTIM_ONGROUND;
	else if ( !nearToGround(vict) )
		c |= CON_VICTIM_AIRBORNE;

	// Does the victim have the Quad Damage item?
	if (vict->client->quad_framenum > level.framenum)
		c |= CON_VICTIM_QUAD;

	// Does the victim have the Invunlerability item?
	if (vict->client->invincible_framenum > level.framenum)
		c |= CON_VICTIM_INVULN;

	// Is the victim's ping HORRIBLE?
	if (vict->client->ping >= EXP_PING_MERCY_MIN)
		c |= CON_MERCY_KILL;

	// Was the victim gibbed
	if (vict->health <= gibVal) 
		c |= CON_GIBBED;

	// Hit location contexts

	// Legs/Torso/Head
	// "point" is the location of the hit.  Note that for radius
	// damage, point is the origin of the inflictor.
	// the player's origin is roughly at his waist
	if (point[2] < vict->s.origin[2]) {
		c |= CON_LEG_HIT;
	} else if (point[2] < vict->s.origin[2] + 24) {
		c |= CON_TORSO_HIT;
	} else {
		c |= CON_HEAD_HIT;
	}

	// get the player's facing direction and the vector
	// between the point of the hit and the player's center
	AngleVectors (vict->s.angles, forward, NULL, NULL);
	VectorSubtract (point, vict->s.origin, vec);
	VectorNormalize (vec);
	// dot product measures the extent to which
	// two unit vectors are in the same direction
	// same direction: 1
	// perpendicular: 0
	// reverse directions: -1
	dot = DotProduct (vec, forward);
	if (dot > 0.4) {
		c |= CON_FRONT_HIT;
	} else if (dot < -0.4) {
		c |= CON_BACK_HIT;
	} else {
		c |= CON_SIDE_HIT;
	}

	// Was the victim killed soon after spawning?
	if (level.time - vict->client->respawn_time < EXP_SHORT_LIFE)
	{
		c |= CON_VICTIM_SHORT_LIFE;
	}

	// Has the victim lived for a very long time?
	if (level.time - vict->client->respawn_time > EXP_LONG_LIFE)
	{
		c |= CON_VICTIM_LONG_LIFE;
	}

	// stop discovering contexts if
	// attacker null (console kill)
	// victim and attack same (killed self)
	// attacker classname != player (kill by world, eg fell)

	if (attk == NULL || vict == attk ||
		Q_stricmp(attk->classname, "player"))
		return (c | CON_KILL_SELF);

	if (attk->client == NULL)
	{	
		// Player was killed by a non-client, ie a door
		return (c);
	}

	// Contexts that only apply when the attacker is another player (as opposed to
	// the same player or world).  Some of the information checked here is gathered elsewhere.
	
	// Attacker gender
	if (GetEntityGender(attk) == OBIT_GENDER_FEMALE) {
		c |= CON_ATTACKER_FEMALE;
	} else {
		c |= CON_ATTACKER_MALE;
	}

	// Has the attacker lived for a very long time?
	if (level.time - attk->client->respawn_time > EXP_LONG_LIFE)
	{
		c |= CON_ATTACKER_LONG_LIFE;
	}

	// Attacker or victim significantly higher than the other player
	distance = attk->s.origin[2] - vict->s.origin[2];
	if (distance > EXP_ABOVE_DISTANCE) {
		c |= CON_ATTACKER_ABOVE;
	} else if (distance < -EXP_ABOVE_DISTANCE) {
		c |= CON_VICTIM_ABOVE;
	}

	// Check if attacker is standing on the ground
	if (attk->groundentity)
		c |= CON_ATTACKER_ONGROUND;

	// Check if attacker was airborne at the time of shooting
	if (Q_stricmp(infl->classname, "player") == 0)
	{	// It's an instant effect weapon.
		if (!CON_ATTACKER_ONGROUND && !nearToGround(attk))
			c |= CON_ATTACKER_AIRBORNE;
	} else {
		if (infl->firedFromAir)
			c |= CON_ATTACKER_AIRBORNE;
	}

	// Does the attacker have the Quad Damage item?
	if (attk->client->quad_framenum > level.framenum)
		c |= CON_ATTACKER_QUAD;

	// Does the attacker have the Invunlerability item?
	if (attk->client->invincible_framenum > level.framenum)
		c |= CON_ATTACKER_INVULN;

	// Check range
	distance = playerDistance(attk, vict);
	//gi.dprintf("Range to target is %1.f\n", distance);
	if (distance < EXP_RANGE_POINTBLANK)
	{	// Point Blank Range...
		c |= CON_POINTBLANK_RANGE;
	} else if (distance > EXP_RANGE_EXTREME) {
		// Extreme range...
		c |= CON_EXTREME_RANGE;
	}

	return (c);
}

//
// Obituary Helper Functions
//

obits_t* messagesForContext(unsigned int cause, unsigned int context)
{
	unsigned int i;
	obits_t** searchIn;

	if (cause > NUM_CAUSES) {
		gi.dprintf("Error: bogus cause number passed to messagesForContext: %d\n", cause);
		return NULL;
	}
	
	// Search for under the cause .. 
	searchIn = gCauseTable[cause]->obituary;

	if (searchIn == NULL)	// No entries as of yet...
		return NULL;

	//gi.dprintf("Searching for context : %s\n", stringForBitField(context));

	// .. for messages matching the context 
	for (i = 0 ; i < gCauseTable[cause]->entryCount ; i++)
	{
		if (searchIn[i]->context == context) {
			//gi.dprintf("Matched context       : %s\n\n", stringForBitField(searchIn[i]->context));
			return (searchIn[i]);
		} else {
			//gi.dprintf("No match with context : %s\n", stringForBitField(searchIn[i]->context));
		}
	}
	//gi.dprintf("Found no match for    : %s\n\n", stringForBitField(context));

	return (NULL);
}

unsigned int getCauseNumber(char *szCause)
{
	unsigned int i;

	if (szCause == NULL)
		return (0);

	if (Q_stricmp(szCause, OBIT_ALL_CAUSE_MACRO) == 0)
		return (OBIT_ALL_CAUSES);

	for (i = (NUM_CAUSES - 1); i != 0 ; i--)
	{
		if (Q_stricmp(szCause, o_Causes[i]) == 0)
			return (i);
	}

	return (0);

}

//
// OBITUARY INITIALIZATION CODE
//

void InitExpertObituary(void)
{
	int i = 0;

	// global info on number of obits and memory
	gMemAllocated = 0;
	gMsgCount = 0;

	gi.dprintf(OBIT_SEP);

	gCauseTable = LoadMessageTree(E_OBIT_OBITUARYFILE);

	if (gCauseTable == NULL)
	{
		gi.dprintf(ERR_OBIT_SEP);
		gi.dprintf(ERR_OBIT_MALLOC);
		gi.dprintf(ERR_OBIT_SEP);
		return;
	}

	if (ReTagObitData(gCauseTable) == false)
	{	
		gi.dprintf(ERR_OBIT_SEP);
		gi.dprintf(ERR_OBIT_MALLOC);
		gi.dprintf(ERR_OBIT_SEP);
		return;
	}

	gi.dprintf(ERR_OBIT_SUCCESS);
	gi.dprintf(OBIT_SEP);
}

void addMessageToCause(char *message, int causeInt, int cFlag)
{
	obits_t *curObit;
	unsigned int mallocSize;
	unsigned int lIndex;
	char *newMessage;

	// See if some messages already exist for this context
	curObit = messagesForContext(causeInt, cFlag);
	if (curObit == NULL)
	{	
		// This is the first message under this context
		// Create a container for messages of this context
		if (gCauseTable[causeInt]->entryCount == 0) {
			gCauseTable[causeInt]->obituary = malloc(sizeof(obits_t*));
		} else {

			mallocSize = sizeof(obits_t*) * 
					(gCauseTable[causeInt]->entryCount + 1);
			gCauseTable[causeInt]->obituary = 
					realloc(gCauseTable[causeInt]->obituary, mallocSize);
		}
		gCauseTable[causeInt]->entryCount++;

		// Now, tagmalloc a new obits_t
		mallocSize = sizeof(obits_t);
		curObit = gi.TagMalloc(mallocSize, TAG_LEVEL);
		gMemAllocated += mallocSize;

		lIndex = (gCauseTable[causeInt]->entryCount) - 1;

		gCauseTable[causeInt]->obituary[lIndex] = curObit;
	} 

	// Now we are guaranteed a container for this message
	if (curObit->msgCount == 0)
	{		
		curObit->context = cFlag;
		curObit->msgCount++;
		curObit->messages = malloc(sizeof(char*));
	} else {
		// The message store exists, realloc it.
		curObit->msgCount++;
		curObit->messages = realloc(curObit->messages,
				sizeof(char*) * (curObit->msgCount));
	}

	// Now, allocate the string buffer.
	mallocSize = ( sizeof(char *) * (strlen(message)) ) + 2;
	newMessage = gi.TagMalloc( mallocSize, TAG_LEVEL);
	strcpy(newMessage, message);
	gMemAllocated += mallocSize;
	gMsgCount++;
	curObit->messages[curObit->msgCount - 1] = newMessage;

	/*
	gi.dprintf("Message: %s\nPlaced under cause %s with contexts ",
			newMessage, o_Causes[causeInt]);
	printContext(cFlag);
	gi.dprintf("\n");
	*/
}

obitContainer_t **LoadMessageTree(const char *szFilename)
{
	FILE *f;

	char line[OBIT_BUFF_SIZE];
	char *message;
	char *cause;
	char *causeList;
	char *context;

	unsigned int causeInt;
	unsigned int cFlag;
	int i = 0, mallocSize;

	char *filename;

	// Create the filename.
	filename = malloc(strlen(gamedir->string) + strlen(szFilename) + 2);
	strcpy(filename, gamedir->string);
	strcat(filename, "/");
	strcat(filename, szFilename);

	gi.dprintf("   Loading Messages ...\n");

	// First, let's open the file.
	if ( (f=fopen(filename, "r")) == NULL)
	{	// Error opening the file.
		free(filename);
		gi.dprintf(ERR_OBIT_FILEOPEN, szFilename);
		return (NULL);
	}

	// Create the container in obits
	mallocSize = sizeof(obitContainer_t *) * (NUM_CAUSES);
	gCauseTable = gi.TagMalloc(mallocSize, TAG_LEVEL);
	gMemAllocated += mallocSize;

	if (gCauseTable == NULL)
	{	// malloc error...
		free(filename);
		gi.dprintf(ERR_OBIT_MALLOC);
		return (NULL);
	}

	// It also prevents GPF's if you allocate the actual space, instead of just the **
	for (i = 0 ; i < NUM_CAUSES ; i++)
	{
		mallocSize = sizeof(obitContainer_t);
		gCauseTable[i] = gi.TagMalloc(mallocSize, TAG_LEVEL);
		if (gCauseTable[i] == NULL)
		{	// malloc error...
			gi.dprintf(ERR_OBIT_MALLOC);
			return (NULL);
		}
		gMemAllocated += mallocSize;
	}

	// We'll use an array of obits_t (tempObits) to store the pointers to the
	// parsed information. When all information has been parsed, we'll copy the
	// tempObits to the permanent member of gCauseTable.

	while ( fgets(line, OBIT_BUFF_SIZE, f) )
	{
		// Not a comment or blank line.
		if ( (strlen(line) > 2 ) && (strcspn(line, OBIT_COMMENT) != 0) )
		{
			int contextFlag;
			qboolean validCause;

			// Re-initialize all temporary vars...
			message = NULL;
			causeList = NULL;
			cause = NULL;
			context = NULL;
			causeInt = 0;
			cFlag = 0;

			// Load the message parts.
			message = strtok(line, OBIT_DELIMITER);
			causeList = strtok(NULL, OBIT_DELIMITER);
			trimWhitespace(cause);

			// Translate the contexts for this obit into a bitfield
			cFlag = 0;
			context = strtok(NULL, OBIT_DELIMITER);
			while (context != NULL)
			{
				trimWhitespace(context);
				contextFlag = getContextBit(context);
				//gi.dprintf("Context is %s, number of context is %d[%s]\n", context,
				//		getContextNumber(context), o_Contexts[getContextNumber(context)]); 
				if (contextFlag == 0) {
					gi.dprintf("Error: \"%s\" is not a valid context.  Context discarded\n", context);
				} else {
					cFlag |= contextFlag;
				}
				context = strtok(NULL, OBIT_DELIMITER);
			}

			validCause = false;
			cause = strtok(causeList, CAUSE_DELIMITER);
			while (cause != NULL)
			{
				// Translate the cause into an ordinal into gCauseTable.
				causeInt = getCauseNumber(cause);
				if (causeInt == 0 && strcmp(cause, CAUSE_UNDEFINED) != 0) 
				{
					// The obituary message did not specify the undefined cause, but 
					// the cause number returned was 0 - this indicates a bogus cause 
					gi.dprintf("Error: \"%s\" is not a valid cause-of-death\n", cause);
					cause = strtok(NULL, CAUSE_DELIMITER);
					continue;
				}
				validCause = true;
				addMessageToCause(message, causeInt, cFlag);

				cause = strtok(NULL, CAUSE_DELIMITER);
			}

			// No valid causes were provided
			if (!validCause)
			{
				gi.dprintf("No valid causes provided for obituary \"%s\"\n"
						"The obituary was discarded\n", line);
			}
/*
			switch (causeInt)
			{
			case OBIT_ALL_CAUSES:
				MacroAddAll(cFlag, message);	// Add this message and context all.
				break;

			default:					// Normal handeling
*/
					

			
		}
	}

	fclose(f);
	return (gCauseTable);
}

/* This subroutine takes a look at each of the obit_t structures contained in the
pointer passed in and converts them to TagMalloc'd strings and 
records the memory usage.

The routine then frees the "temporary" string pointers that were originally
allocated.
*/
qboolean ReTagObitData()
{
	unsigned int ulCause;
	
	// The causes member of the obitInfo structure should be the correct size.
	
	for (ulCause = 0 ; ulCause < NUM_CAUSES ; ulCause++)
	{	// We'll loop through all of the entries in gCauseTable
		// And retag the obituary. The obituary will then retag all of the messages.
		
		if (TagMallocObituary(gCauseTable[ulCause]) == NULL)
		{	// Malloc error...
			return (false);
		}
	}
	
	return (true);
	
}

obits_t** TagMallocObituary(obitContainer_t *obitCont)
{
	unsigned int i;
	unsigned int mallocSize;
	obits_t 	**newObits;
	
	// allocate new space
	mallocSize = sizeof(obits_t*) * (obitCont->entryCount);
	newObits = gi.TagMalloc(mallocSize, TAG_LEVEL);
	if (newObits == NULL)
	{	// Malloc error...
		return (NULL);
	}
	gMemAllocated += mallocSize;
	
	// Copy entries from old space.
	for (i = 0 ; i < obitCont->entryCount ; i++)
	{
		newObits[i] = obitCont->obituary[i];
		if (TagMallocMessages(obitCont->obituary[i]) == NULL)
		{	// MALLOC Problem.
			return (NULL);
		}
	}

	free(obitCont->obituary);
	obitCont->obituary = newObits;
	
	return (newObits);
}

char** TagMallocMessages(obits_t *obitEntry)
{
	unsigned int mallocSize;
	unsigned int i;
	char **newMsgArray;
	
	
	// Allocate the new space.
	mallocSize = sizeof(char*) * (obitEntry->msgCount);
	newMsgArray = gi.TagMalloc(mallocSize, TAG_LEVEL);
	if (newMsgArray == NULL)
	{	// Malloc error
		return (NULL);
	}
	gMemAllocated += mallocSize;
	
	// Copy entries from old space.
	for (i = 0 ; i < obitEntry->msgCount ; i++)
		newMsgArray[i] = obitEntry->messages[i];
	
	free(obitEntry->messages);
	obitEntry->messages = newMsgArray;
	
	return (newMsgArray);
}

int getContextNumber(char *szContext)
{
	unsigned int l;
	
	for (l = 0; l < NUM_CONTEXTS; l++)
	{
		if (Q_stricmp(szContext, o_Contexts[l]) == 0)
			return l;
	}
	return (-1);
}

unsigned int getContextBit(char *szContext)
{
	int contextNumber = getContextNumber(szContext);
	if (contextNumber == -1) { // context not found
		return 0;
	} else {
		return (1 << getContextNumber(szContext));
	}
}

int GetEntityGender(edict_t *ent)
{

	char		*info;

	if (!ent->client)
		return OBIT_GENDER_MALE;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return OBIT_GENDER_FEMALE;

	return OBIT_GENDER_MALE;

}

void MacroAddAll(unsigned int cFlag, const char* message)
{
	
	int				i = 0;
	unsigned int	lIndex;
	unsigned int	mallocSize = 0;
	
	char			*newMessage;

	obits_t			*curObit;	

	// First, allocate the stringspace.
	mallocSize = ( sizeof(char *) * (strlen(message)) ) + 2;
	newMessage = gi.TagMalloc( mallocSize, TAG_LEVEL);
	if (newMessage == NULL)
	{	// malloc error...
		gi.dprintf(ERR_OBIT_MALLOC);
		return;
	}
	strcpy(newMessage, message);
	gMemAllocated += mallocSize;
	gMsgCount++;

	for (i = 0 ; i < NUM_CAUSES ; i++)
	{
		// See if we can find the context...
		curObit = messagesForContext(i, cFlag);
		if (curObit == NULL)
		{	// It's not an entry in the context list yet...
			if (gCauseTable[i]->entryCount == 0)
			{
				gCauseTable[i]->obituary = malloc(sizeof(obits_t*));
			}else {

				mallocSize = sizeof(obits_t*) * 
							(gCauseTable[i]->entryCount + 1);
				gCauseTable[i]->obituary = 
					realloc(gCauseTable[i]->obituary, mallocSize);
			}
			gCauseTable[i]->entryCount++;
			/*
			tmpObitContainers = gCauseTable;
			tmpObitContainer = tmpObitContainers[cause];
			tmpObitContainer->obituary = malloc(sizeof(obits_t*));
			*/
			if (gCauseTable[i]->obituary == NULL)
			{	// malloc error...
				gi.dprintf(ERR_OBIT_MALLOC);
				return;
			}
			// Now, tagmalloc a new obits_t
			mallocSize = sizeof(obits_t);
			curObit = gi.TagMalloc(mallocSize, TAG_LEVEL);
			if (curObit == NULL)
			{	// malloc error...
				gi.dprintf(ERR_OBIT_MALLOC);
				return;
			}
			gMemAllocated += mallocSize;

			lIndex = (gCauseTable[i]->entryCount) - 1;

			gCauseTable[i]->obituary[lIndex] = curObit;
		} // If it was found, we don't do anything... Dont' want multiples.

		// Now that curObit is all set, let's add the message.
		if (curObit->msgCount == 0)
		{	// We have to create the message store.
			curObit->context = cFlag;
			curObit->msgCount++;
			curObit->messages = malloc(sizeof(char*));
			if (curObit->messages == NULL)
			{	// malloc error...
				gi.dprintf(ERR_OBIT_MALLOC);
				return;
			}
		} else {
			// The message store exists, realloc it.
			curObit->msgCount++;
			curObit->messages = realloc(curObit->messages,
								sizeof(char*) * (curObit->msgCount));
			if (curObit->messages == NULL)
			{	// malloc error...
				gi.dprintf(ERR_OBIT_MALLOC);
				return;
			}
		}

		curObit->messages[curObit->msgCount - 1] = newMessage;
		//gi.dprintf("MACRO ADD TO: (%i) %s\n", i, newMessage);
	}
			

}

void printContext(unsigned int context)
{
	unsigned int mask, i;
	i = context;
	while (i != 0)
	{
		mask = lowestOrderBit(i);
		gi.dprintf("%s[%d], ", o_Contexts[log2(mask)], log2(mask));
		i = i & ~mask;
	}
	if (context == 0) {
		gi.dprintf("none");
	}
	gi.dprintf("\n");
}
