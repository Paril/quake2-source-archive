//============================================================================
// p_nhmenu.h
//
// Night Hunters menu
//
// Originally coded by DingBat
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//============================================================================

// NH menu.
extern qboolean showscores;// set layout stat

// Show the NH menu.
void ShowNHMenu (edict_t *ent); 
qboolean NHStartClient(edict_t *ent) ;
void EnterGame(edict_t *ent, pmenu_t *p) ;