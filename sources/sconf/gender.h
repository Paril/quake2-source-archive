/* gender.h - constants and function prototypes */

#define GENDERMOD_VERSION "GenderMod Support"

#define GENDER_ERROR 0
#define GENDER_MALE 1
#define GENDER_FEMALE 2
#define GENDER_NEUTER 3
#define GENDER_ANY 4

struct ModelGenderMapEntry
{
	char *model;
	int gender;
};

void GenderModInitGame ();
int UserSetPlayerGender (edict_t *ent);
int GetPlayerGender (edict_t *ent);
void Cmd_Gender_f (edict_t *ent);

int ConvertGenderSpec (char *str);

char * GenderToName (int gender);
char * GenderToSheHe (int gender);
char * GenderToHimHer (int gender);
char * GenderToHisHer (int gender);
char * GenderToHimselfHerself (int gender);
char * GenderToBoyGirl (int gender);
char * GenderToManWoman (int gender);
