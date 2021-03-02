//
// e_matrix.c
//
void UpdateMatrixScores (edict_t *pedTarget, edict_t *pedKiller);
void ExpandMatrix (edict_t *pedJoining);
void ContractMatrix (edict_t *pedLeaving);
void DrawMatrix (edict_t* pedViewer, char* pszLayout, byte flags);

//DrawMatrix flags
#define MXLEARN 1