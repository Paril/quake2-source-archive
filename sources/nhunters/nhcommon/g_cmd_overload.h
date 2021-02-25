//============================================================================
// g_cmd_overload.h
//
// Predator rocket overload command toggle.
//
// Original code by: DingBat
//
// Copyright(c) 1999, The BatCave, All Rights Reserved.
//============================================================================

#define ALLOW_PRED_OVERLOAD_DEFAULT "1"
#define PRED_OVERLOAD_COST_DEFAULT "2"

void Cmd_Overload_f(edict_t *ent) ;
void ClearOverload(edict_t *ent) ;

void validatePredatorOverloadCost();
int getPredatorOverloadCost();
