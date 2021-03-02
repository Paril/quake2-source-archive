// e_util.c
//
// Misc. functions that should be dependant only on the original game src.

#include "g_local.h"

#define LOG_FILENAME				"debuglog.txt"

FILE *logfile = NULL;

// StuffCmd: Sends arbitrary console commands to a client.
// Mucho thanks to James Abbatiello for this code!
void StuffCmd(edict_t *player, char *cmd)
{
	gi.WriteByte(11);
	gi.WriteString(cmd);
	gi.unicast (player, true);
}

/**
 * Send a sound to one player
 * Didn't bother to support 'timeofs'
 * Another version might be made to support positioned sounds
 */
#define svc_sound 9

#define MASK_VOLUME 			1
#define MASK_ATTENUATION 		2
#define MASK_POSITION 			4
#define MASK_ENTITY_CHANNEL 	8
#define MASK_TIMEOFS 			16	

void unicastSound(edict_t *player, int soundIndex, float volume)
{

	int mask = MASK_ENTITY_CHANNEL;

	if (volume != 1.0) {
		mask |= MASK_VOLUME;
	}
	gi.WriteByte(svc_sound);
	gi.WriteByte((byte)mask);
	gi.WriteByte((byte)soundIndex);
	if (mask & MASK_VOLUME) {
//		gi.dprintf("Volume is %.1f, writing volume as %d\n",
//	           	   volume,
//		           (byte)(volume * 255));
		gi.WriteByte((byte)(volume * 255));
	}
	//gi.dprintf("player - g_edicts is %d\n",
	//     	   player - g_edicts - 1);
	gi.WriteShort(
		((player - g_edicts - 1) << 3) +
		CHAN_NO_PHS_ADD);

	gi.unicast (player, true);
}

// A recreation of the parsing code for sounds in the quake2 client 
/*
          long mix;

          mask = ReadByte;
          soundindex = ReadByte;
          vol = (mask & MASK_VOLUME) ? ((float)ReadByte / 255.0) : (1.0);
          attenuation = (mask & MASK_ATTENUATION) ? ((float)ReadByte / 64.0) : (1.0);
          timeofs = (mask & MASK_TIMEOFS) ? ((float)ReadByte * 0.001) : (0.0); 
          if (mask & MASK_ENTITY_CHANNEL) {
            mix = ReadShort;
            entity = (mix >> 3);
            if (entity > MAX_EDICTS) {
              error("CL_ParseStartSoundPacket: ent = %i", entity);
            }
            channel = mix & 0x07;
          } else {
            entity = 0;
            channel = 0;
          }
          if (mask & MASK_POSITION) {
            ReadPosition(origin);
          }
*/

int log2(unsigned int num)
{
	int i = 0;
	while (num > 1)
	{
		i++;
		num = num >> 1;
	}
	return i;
}

int lowestOrderBit(unsigned int bitField)
{
	unsigned int mask, i;
	for (i = 0; i < (sizeof(unsigned int) * 8); i++)
	{
		mask = 1 << i;
		if (bitField & mask)
		{
			return mask; 
		}
	}
    return 0;

}

int numberOfBitsSet(unsigned int bitField)
{
	unsigned int mask;
    int i, count = 0;
	for (i = 0; i < (sizeof(unsigned int) * 8); i++)
	{
		mask = 1 << i;
		if (bitField & mask) count++;
	}
	return count;

}

// Note this won't work on platforms where 1 char != 8 bits
char *stringForBitField(unsigned int field)
{
	int i;
	static char buf[sizeof(unsigned int) * 8 + 1];

	buf[sizeof(int) * 8] = '\0';
	for (i = 0; i < (sizeof(unsigned int) * 8); i++)
	{
		if (field & (1 << i)) 
			buf[i] = '1'; 
		else
			buf[i] = '0';
	}
	return buf;
}

// numchr: Find out how many of a given character are in a given string.
// Useful for finding out how many returns to pad for centerprinting..
int numchr(char *str, int c)
{
	int number = 0;
	char *left = str;

	if (str == NULL)
		return 0;
	
	while ((left = strchr(left, c)) != NULL)
	{
		number++;
		left++;
	}

	return number;
}

// BootPlayer: Boots the given player with the given (1st) error message.
// 2nd message is for global messages.
void BootPlayer(edict_t *player, char *error, char *global)
{
	char *buf;

	// Create command buffer and stuff it
	buf = malloc(strlen(error) + 11);
	buf = strcpy(buf, "\nerror \"");
	buf = strcat(buf, error);
	buf = strcat(buf, "\"\n");

	StuffCmd(player, buf);
	free(buf);

	// Make an announcement
	gi.bprintf(PRINT_HIGH, "%s kicked: %s\n", player->client->pers.netname, global);

	// Kick the player for good measure
	buf = malloc(7 + strlen(player->client->pers.netname));
	buf = strcpy(buf, va("kick %d\n", player - g_edicts - 1));
/*
	buf = strcat(buf, player->client->pers.netname);
	buf = strcat(buf, "\n");
*/
	gi.AddCommandString(buf);
	free(buf);
}

void E_LogAppend(char *desc, char *fmt, ...)
{
	va_list argptr;

	if (logfile == NULL)
	{
		logfile = OpenGamedirFile(gamedir->string, LOG_FILENAME, "a");

		if (logfile == NULL)
		{
			gi.dprintf("ERROR: Unable to open log file. Log message cancelled.\n");
			return;
		}

//		char filename[256];
//		strcpy(filename, gamedir->string);
//		strcat(filename, "/");
//		strcat(filename, LOG_FILENAME);
//		logfile = fopen(filename, "a");
	}

	fprintf(logfile, "Log message at time %.2f, frame %d. Description: %s\n"
					 "Message:\n", level.time, level.framenum, desc);

	va_start(argptr, fmt);
	vfprintf(logfile, fmt, argptr);
	va_end(argptr);

	fprintf(logfile, "\n----- End of message -----\n\n");
}

void E_LogClose(void)
{
	if (logfile != NULL)
	{
		fprintf(logfile, "Closing logfile at time %.2f seconds elapsed in game\n", level.time);
		fclose(logfile);
	}
	else
		gi.dprintf("ERROR in E_LogClose: Attempted to close an already-closed logfile\n");
}

qboolean nearToGround(edict_t *ent)
{
	vec3_t		startPos, traceTo, dist;
	trace_t		trace;

	if (ent->waterlevel > 0) return true;

	VectorCopy(ent->s.origin, startPos);

	dist[0] = 0;
	dist[1] = 0;
	dist[2] = EXP_AIRBORNE_THRESH;

	VectorAdd(startPos, dist, traceTo);
	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, traceTo, ent, MASK_PLAYERSOLID|MASK_WATER);

	if (trace.fraction == 1.0)
		return (false);

	return (true);
}

void trimWhitespace(char *szString)
{
	char *temp;
	int	 len = 0;
	int  pos = 0;
	int  i = 0;
	int  curChar = 0;

	if (szString == NULL)
		return;

	len = strlen(szString);

	if (len == 0)
		return;

	temp = calloc(len + 1, sizeof(char));
	strcpy(temp, szString);

	// Start from the tail of the string and replace spaces with null.
	for (i = (len - 1); i > 0 ; i--)
	{
		curChar = *(temp + i);
		if ((curChar == ' ') ||		// space
		    (curChar == '\t') ||	// tab
		    (curChar == '\n') ||	// CR
		    (curChar == '\r'))		// LF
			temp[i] = 0;		// Change to null
		else
			break;
	}

	// Start from the front of the string and move the pointer up one.
	for (i = 0 ; i < len ; i++)
	{
		curChar = *(temp + i);
		if ((curChar == ' ') ||		
		    (curChar == '\t') ||
		    (curChar == '\n') ||
		    (curChar == '\r'))
			pos++;
		else
			break;
	}

	// Copy the string and exit
	strcpy(szString, temp + pos);
	free(temp);

}

float playerDistance(edict_t *plyr1, edict_t *plyr2)
{
	vec3_t	v;
	float	distance = 0;

	VectorSubtract (plyr1->s.origin, plyr2->s.origin, v);
	
	// fabs function is HORRIBLY slow...

	distance = VectorLength(v);

	if (distance >= 0)
		return (distance);

	return -(distance);
}

/**
 * Takes a string and bitshifts all characters that Quake2 can
 * render as green so that the string will render as green when
 * printed.
 *
 * For greenCopy(), szDest must be at least as large as szSrc.
 * For convenience szDest is returned.
 */
char *greenText(char *s)
{
	return greenCopy(s, s);
}

char *greenCopy(char *szDest, const char *szSrc)
{

	int curChar = 0;
	int len = 0;
	int i = 0;

	len = strlen(szSrc);

	if (len == 0)
		return NULL;

	for (i = 0 ; i < len ; i++)
	{
		curChar = *(szSrc + i);
		if (curChar > 32 && curChar < 128) {
			szDest[i] = curChar | 128;
		} else {
			szDest[i] = curChar;
		}
	}
	szDest[i] = 0;

	return szDest;
}

int strBeginsWith (char *prefix, char *s2)
{
	int 	c1, c2, max = 999999;
	
	do
	{
		c1 = *prefix++;
		c2 = *s2++;
		if (!c1) return(1); // Reached end of search string
		if (!max--)
			return 1;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return 0;		// strings not equal
		}
	} while (c1);
	
	return 1;		// strings are equal
}

int insertValue(char *mess, char *format_str, char *field, char *value, int max_size)
{
	int x=0, y=0, matches = 0;
	mess[0] = 0;
	while (format_str[x] && (x < max_size))
	{
		if (strBeginsWith(field, format_str + x))
		{
			if (x + strlen(value) < max_size)
			{
				//PRINT2("Found match on '%s' for field '%s\n", format_str, field);
				matches++;
				strcpy(mess+y, value);
				y = y + strlen(value);
				x = x + strlen(field);
			}
			else {
				// Buffer overflow would occur.
				return(-1);
			}
		}
		else {
			mess[y++] = format_str[x++];
		}
	}
	if (y < max_size)
		mess[y] = 0;
	else mess[max_size - 1] = 0;
	return(matches);
}

int StrToInt(char *pszFrom, int Default)
{
	char *psz = pszFrom; //for incrementing through the string
	signed char sign = 1;
	int value = 0;

	while(*psz == ' ' || *psz == '\t' || *psz == '\n')
		psz++;

	if (*psz == '+')
		psz++;
	else if (*psz == '-') {
		psz++;
		sign = -1;
	}

	if (*psz < '0' || *psz > '9')
		return Default;

	while (*psz >= '0' && *psz <= '9') {
		value = (10 * value) + *psz - '0';
		psz++;
	}

	return (value * sign);
}

void ResizeLevelMemory(void** ppMem, size_t sizeNew, size_t sizeOld)
{
	byte* pOld = (byte *)*ppMem;
	byte* pNew = gi.TagMalloc(sizeNew, TAG_LEVEL);

	assert(ppMem != NULL && sizeNew > 0 && sizeOld > 0);
	if (sizeNew == sizeOld) {
		return;
	}

	memcpy(pNew, pOld, sizeOld);

	#ifdef _DEBUG
	{
		if (sizeNew > sizeOld) {
			memset(pNew+sizeNew, 0xCC, sizeNew - sizeOld);
		} else if (sizeOld > sizeNew) {
			memset(pOld+sizeNew, 0xCC, sizeOld - sizeNew);
		}
	}
	#endif

	gi.TagFree(pOld);
	*ppMem = (void *)pNew;
}

// OpenGamedirFile: Opens a filename from a given basedir.
// Returns a file handle if successful, NULL otherwise.
// NOTE: No checking is done on the mode.
FILE *OpenGamedirFile(const char *basedir, const char *filename, char *mode)
{
	char tempname[MAX_QPATH] = {0};
	FILE *file;

	strcpy(tempname, basedir);
	strcat(tempname, "/");
	strcat(tempname, filename);

	file = fopen(tempname, mode);

	if (file == NULL)
	{
		gi.dprintf("Unable to load file %s\n",
			tempname);
	}

	return file;
}

// Utils for changing settings with the "sv set" command.

// Given a string name of a setting, return the bit that corresponds
// to that setting
unsigned int getSettingBit(char *setting)
{
	int settingNum = getSettingNumber(setting);
	if (settingNum == -1) return -1; // error
	return (1 << settingNum);
}

// Given a string name of a setting, look in the list of 
// settings and return the index where the string "setting" was found
int getSettingNumber(char *setting)
{
	unsigned int l;
	char *token;
	char buffer[100], settingName[100];
	
	for (l = 0; l < NUM_SETTINGS; l++)
	{
		if (Q_stricmp(setting, e_bits[l]) == 0)
			return l;
	}
	for (l = 0; l < NUM_SETTINGS; l++)
	{
		buffer[0] = '\0';
		strcpy(settingName, e_bits[l]);
		token = strtok(settingName, " ");
		while (token != NULL) {
			strcat(buffer, token);
			token = strtok(NULL, " ");
		}
		if (Q_stricmp(setting, buffer) == 0)
			return l;
	}
	return (-1);
}

void addFloodSample(edict_t* ent) {

	ent->client->resp.lastSayPos += 1;
	if (ent->client->resp.lastSayPos > FLOODPROT_SAYS - 1) {
		ent->client->resp.lastSayPos = 0;
	}
	ent->client->resp.lastSays[ent->client->resp.lastSayPos] = level.time;
}

// set all samples to a negative value to clear them
// assumes time is always positive
void clearFloodSamples(edict_t *player) {
	int i;

	for (i = 0; i < FLOODPROT_SAYS; i++) 
	{
		player->client->resp.lastSays[i] = -99;
	}
	player->client->resp.lastSayPos = 0;
}

// check if "number" samples exist over the last "floodTime" seconds
qboolean checkFlood(edict_t* ent, int number, float floodTime) {
	int oldestSample;
//	int i;
	
	// can't detect floods of more samples than the 
	// number of samples we've stored.
	if (number > FLOODPROT_SAYS)
		return false;

	// go "number" samples back into the array
	oldestSample = ent->client->resp.lastSayPos - (number - 1);
	// wrap if fell off end of array
	if (oldestSample < 0) {
		oldestSample = FLOODPROT_SAYS + oldestSample;
	}

/*
// debugging
	gi.dprintf("Floodprot: time %f, number:%d, lastSay: %d, position is %d\n",
				level.time, number, ent->client->resp.lastSayPos, oldestSample);
	for (i = 0; i < FLOODPROT_SAYS; i++) 
	{
		gi.dprintf("Sample is %f\n", ent->client->resp.lastSays[i]);
	}
*/

	// "number" samples have occured in less time than "floodTime"
	if (ent->client->resp.lastSays[ent->client->resp.lastSayPos] - 
		ent->client->resp.lastSays[oldestSample] < floodTime) {
		return true;
	}
	
	return false;
}

// returns whether player is silenced
qboolean floodProt(edict_t *ent) {

	addFloodSample(ent);
	// 5 says in 3 seconds, or 15 says in 12 seconds, activate floodprot
	if (checkFlood(ent, 5, 3.0) ||
	    checkFlood(ent, 10, 12.0) ||
	    checkFlood(ent, 20, 18.0)) {
		ent->client->resp.nextSay = level.time + 20;
	}
	
	if (ent->client->resp.nextSay > level.time) {
		gi.cprintf(ent, PRINT_HIGH, "Floodprot activated.  No spamming.\n");
		return true;
	}

	return false;
}

