//===========================================================================
//
// Name:         bl_debug.h
// Function:     debug functions
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1999-02-10
// Tab Size:     3
//===========================================================================
/* already defined in botlib.h
#define LINECOLOR_NONE			-1
#define LINECOLOR_RED			0xf2f2f0f0
#define LINECOLOR_GREEN			0xd0d1d2d3
#define LINECOLOR_BLUE			0xf3f3f1f1
#define LINECOLOR_YELLOW		0xdcdddedf
#define LINECOLOR_ORANGE		0xe0e1e2e3
*/
#define BBOX_LINES				14

typedef struct visiblebbox_s
{
	//true if line edicts are created
	qboolean created;
	//bounding box composed of BBOX_LINES lines
	edict_t *lines[BBOX_LINES];
} visiblebbox_t;

//functions do deal with debug lines
int DebugLineCreate(void);
void DebugLineDelete(int line);
void DebugLineShow(int line, vec3_t start, vec3_t end, int color);
//visualizes the bouding box of the given entity
void SetVisibleBoundingBox(visiblebbox_t *box, edict_t *ent);
//toggles a visible bounding box
void ToggleVisibleBoundingBox(edict_t *ent);
