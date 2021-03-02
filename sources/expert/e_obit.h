/*
  e_obit.h

  Definitions for Expert Obituary Functions

*/

//
// Constants
//
#define OBIT_BUFF_SIZE			512

#define OBIT_COMMENT			"#;\'"
#define OBIT_DELIMITER			",\n\r"
#define CAUSE_DELIMITER			"|\n\r"
#define OBIT_NULL_WEAPON		"*"
#define OBIT_CONSOLE_KILL		"Console Kill"
#define OBIT_ALL_CAUSE_MACRO	"all causes"
#define OBIT_SEP				"\n  ================================\n\n"

#define OBIT_GENDER_OTHER		0				// Treat other as male for subst.
#define OBIT_GENDER_MALE		0
#define OBIT_GENDER_FEMALE		1

#define OBIT_ALL_CAUSES			0xFFFFFFFF 		// Little hack for All Cause macro.
//
// Gender Strings
//

#define OBIT_HE					"he"
#define OBIT_HIM				"him"
#define OBIT_HIS				"his"

#define OBIT_SHE				"she"
#define OBIT_HER				"her"
#define OBIT_HER				"her"

/* ERRORS */
#define ERR_OBIT_FILEOPEN		"   ERROR: Couldn't read/open\n          \"%s\"."
#define ERR_OBIT_NOCAUSEMAP 	"   ERROR: Couldn't initialize Obit\n          Cause Map. Default message\n          will be used.\n"
#define ERR_OBIT_MALLOC 		"   ERROR: Couldn't allocate enough\n	       memory for obituary. Default\n	       messages will be used.\n"

#define ERR_OBIT_SUCCESS		"   Expert Obituary Initialized!\n"

#define ERR_OBIT_SEP			"********************\n"

/* Files */
#define E_OBIT_CAUSEFILE		"causes.txt"
#define E_OBIT_CONTEXTFILE		"contexts.txt"
#define E_OBIT_OBITUARYFILE 	"obituary.txt"

//
// Constants for determining contexts
//

#define EXP_PING_MERCY_MIN		900
#define EXP_RANGE_POINTBLANK	100	
#define EXP_RANGE_EXTREME		1000
#define EXP_ABOVE_DISTANCE		32 // 2/3 of a player's height
#define EXP_SHORT_LIFE			5 // 5 seconds
#define EXP_LONG_LIFE			300 // 5 minutes

//
// Contexts
//

#define CON_VICTIM_FEMALE		(1 << 0)			// 1
#define CON_VICTIM_MALE			(1 << 1)			// 2	
#define CON_ATTACKER_FEMALE 	(1 << 2)			// 4
#define CON_ATTACKER_MALE		(1 << 3)			// 8
#define CON_ATTACKER_QUAD		(1 << 4)			// 16		
#define CON_ATTACKER_INVULN 	(1 << 5)			// 32
#define CON_VICTIM_QUAD 		(1 << 6)			// 64
#define CON_VICTIM_INVULN		(1 << 7)			// 128	
#define CON_VICTIM_SHORT_LIFE	(1 << 8)			// 256
#define CON_VICTIM_LONG_LIFE	(1 << 9)			// 512	
#define	CON_ATTACKER_LONG_LIFE	(1 << 10)			// 1024
#define CON_LEG_HIT				(1 << 11)			// 2048
#define CON_TORSO_HIT			(1 << 12)			// 4096
#define CON_HEAD_HIT			(1 << 13)			// 8192
#define CON_FRONT_HIT			(1 << 14)			// 16384
#define CON_SIDE_HIT			(1 << 15)			// 32768	
#define CON_BACK_HIT			(1 << 16)			// 65536
#define CON_GIBBED				(1 << 17)			// 131072
#define	CON_POINTBLANK_RANGE	(1 << 18)			// 262144
#define CON_EXTREME_RANGE		(1 << 19)			// 524288
#define CON_MERCY_KILL			(1 << 20)			// 1048576	
#define CON_VICTIM_ONGROUND		(1 << 21)			// 2097152
#define CON_ATTACKER_ONGROUND	(1 << 22)			// 4194304
#define CON_VICTIM_AIRBORNE 	(1 << 23)			// 8388608	
#define CON_ATTACKER_AIRBORNE	(1 << 24)			// 16777216
#define CON_VICTIM_ABOVE		(1 << 25)			// 33554432
#define CON_ATTACKER_ABOVE		(1 << 26)			// 67108864
#define CON_FRIENDLY_FIRE		(1 << 27)			// 134217728
#define CON_KILL_SELF			(1 << 28)			// 268435456

/*
(1 << 29)			// 536870912
(1 << 30)			// 1073741824
*/
	
//
// Tokens
//

#define OTOK_VICTIM_NAME		"$VName"
#define OTOK_VICTIM_HE			"$VHe"
#define OTOK_VICTIM_HIM			"$VHim"
#define OTOK_VICTIM_HIS			"$VHis"

#define OTOK_ATTACKER_NAME		"$AName"
#define OTOK_ATTACKER_HE		"$AHe"
#define OTOK_ATTACKER_HIM		"$AHim"
#define OTOK_ATTACKER_HIS		"$AHis"

#define CAUSE_UNDEFINED			"*"

//
// Structures
//

typedef struct obits_s {
	unsigned int	context;
	unsigned int	msgCount;
	char			**messages;
} obits_t;

typedef struct obitContainer_s {
	unsigned int entryCount;
	obits_t **obituary;
} obitContainer_t;

obitContainer_t **gCauseTable;

//
// e_obit.c
//
void InitExpertObituary(void);
void ExpertClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker, int cod, vec3_t point);
