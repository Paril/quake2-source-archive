/*==========================================================================
//  x_motd.h -- by Patrick Martin               Last updated:  1-12-1999
//--------------------------------------------------------------------------
//  This is the header file for x_motd.c.
//========================================================================*/

/***************/
/*  CONSTANTS  */
/***************/

/* Max. number of characters allowed in MOTD. */
#define MAX_MOTD        500


/****************/
/*  PROTOTYPES  */
/****************/

/*
// x_motd.c
*/
qboolean PBM_LegalChar (char c);
void PBM_GetMOTD (void);
void PBM_ShowMOTD (edict_t *ent);

void PBM_ConfigInit (void);


/*===========================/  END OF FILE  /===========================*/
