//
// e_overlay.c
//

void RecordSighting(edict_t *pedViewer, edict_t *pedTarget);
qboolean ValidOverlayCommand (edict_t *pedPlayer);
void OverlayUpdate (edict_t *pedViewer);
void OverlayPrint(edict_t* pedTarget, byte flags, const char* pszEntry, byte cUpdates);

//OverlayPrint flags
#define PRINT_APPEND	1
#define PRINT_FORCE		2
#define UPDATE_NOW		4

