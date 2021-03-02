// Definitions
#define PWORDER_CHAR		0
#define PWORDER_BYTE		1
#define PWORDER_SHORT		2
#define PWORDER_LONG		3
#define PWORDER_FLOAT		4
#define PWORDER_STRING		5
#define PWORDER_POSITION	6
#define PWORDER_DIR			7
#define PWORDER_ANGLE		8

#define PWORDER_SLOT1 0
#define PWORDER_SLOT2 1
#define PWORDER_SLOT3 2
#define PWORDER_SLOT4 3
#define PWORDER_SLOT5 4
#define PWORDER_SLOT6 5
#define PWORDER_SLOT7 6
#define PWORDER_SLOT8 7
#define PWORDER_SLOT9 8
#define PWORDER_SLOT10 9
#define PWORDER_MAXSLOTS 10

/*
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
	void	(*WriteString) (char *s);
	void	(*WritePosition) (vec3_t pos);	// some fractional bits
	void	(*WriteDir) (vec3_t pos);		// single byte encoded, very coarse
	void	(*WriteAngle) (float f);
*/

// All variables here are cleared on each multicast/unicast call for use in the next generation (WooOoooOOoooOoooOOOOOoo)
extern int pwri_callorders[PWORDER_MAXSLOTS]; // Array of 10 integers. Stores the order of which the writes are called in.

struct pworder_s
{
	int byte1;
	int byte2;
	int byte3;
	int byte4;
	int char1;
	int char2;
	int shorts[MAX_ITEMS]; // Damn inventory.
	int long1;
	int long2;
	float float1;
	float float2;
	char *string1;
	char *string2;
	vec3_t pos1;
	vec3_t pos2;
	vec3_t dir1;
	vec3_t dir2;
	float angle1;
	float angle2;
};

void Clear_Structure ();

void WriteAngle (float byte);
void WriteDir (vec3_t dir);
void WritePosition (vec3_t position);
void WriteString (char *string);
void WriteFloat (float byte);
void WriteLong (int byte);
void WriteShort (int byte);
void WriteByte (int byte);
void WriteChar (int byte);
void Unicast (edict_t *ent, qboolean reliable);
void Multicast (vec3_t origin, multicast_t to);
void ParilWrite_Initialize ();

struct required_te_innerts
{
	int needs_pos;
	int needs_pos2;
	int needs_dir;
	int needs_short;
	int needs_byte3;
};

// Required shit for a temporary entity
struct required_te_s
{
	// We assume it already has svc_temp_entity...
	int type;
	struct required_te_innerts required;
};

typedef struct pworder_s pworder_t;
typedef struct required_te_s required_te_t;

extern required_te_t required_list[];
