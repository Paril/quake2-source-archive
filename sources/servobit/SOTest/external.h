/* External definitions that were part of the game, but
haven't been cleanly separated from the rest of the ServObit
code */


/* From gender.h */
#define GENDERMOD_VERSION "GenderMod 0.1 4/5/98"

#define GENDER_ERROR 0
#define GENDER_MALE 1
#define GENDER_FEMALE 2
#define GENDER_NEUTER 3
#define GENDER_ANY 4

struct external_edict
{
	int whatever;
} edict_t;

int Q_stricmp (char *str1, char *str2);

#define MAX_STRING_CHARS 1024
