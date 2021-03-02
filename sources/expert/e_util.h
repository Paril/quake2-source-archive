/**
 * e_util.h
 *
 * Prototypes for functions that should only be 
 * dependant on the main game source itself
 */

// Ground checking distance for NearToGround
#define EXP_AIRBORNE_THRESH		-96.0	// Two player heights.

// strings

//cuts through 3 layers of calls
#define StrMatch(left, right) (Q_strncasecmp((left), (right), 99999) == 0)

int StrToInt(char *pszFrom, int Default);
int numchr(char *str, int c);
char *greenText(char *s);
char *greenCopy(char *szDest, const char *szSrc);
int strBeginsWith (char *prefix, char *s2);
int insertValue(char *mess, char *format_str, char *field, char *value, int max_size);
void trimWhitespace(char *szString);

char *stringForBitField(unsigned int field);
int numberOfBitsSet(unsigned int bitField);
int lowestOrderBit(unsigned int bitField);
int log2(unsigned int);

// network
void StuffCmd(edict_t *player, char *cmd);
void unicastSound(edict_t *player, int soundIndex, float volume);
void BootPlayer(edict_t *player, char *error, char *global);

qboolean nearToGround(edict_t *ent);
float playerDistance(edict_t *plyr1, edict_t *plyr2);

// logging
void E_LogAppend(char *desc, char *fmt, ...);
void E_LogClose(void);

// misc
void ResizeLevelMemory(void** ppMem, size_t sizeNew, size_t sizeOld);
FILE *OpenGamedirFile(const char *basedir, const char *filename, char *mode);

// settings
unsigned int getSettingBit(char *setting);
int getSettingNumber(char *setting);

// floodprot
void addFloodSample(edict_t* ent);
void clearFloodSamples(edict_t* ent);
qboolean checkFlood(edict_t* ent, int number, float floodTime);
qboolean floodProt(edict_t* ent);


