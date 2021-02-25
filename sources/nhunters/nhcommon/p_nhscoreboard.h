//============================================================================
// p_nhscoreboard.h
//
// Night Hunters scoreboard
//
// Originally coded by DingBat
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//============================================================================

#define SCOREBOARDSIZE 1024
#define USE_NH_SCOREBOARD_DEFAULT "1"

// NH changes: Scoreboard.
void NHScoreboardMessage(edict_t *ent, edict_t *killer) ;
void NHScoreboard(edict_t *ent) ;
void Cmd_NHScore_f(edict_t *ent) ;

