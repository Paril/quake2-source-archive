/* SrvObAPI.h
*/

int GetPlayerBodyState (edict_t *ent);
int PlayerHasQuadDamage (edict_t *entity);
void ServObitPrintWelcome (edict_t *ent);
void ServObitAnnounceConnect (edict_t *ent);
void ServObitAnnounceDisconnect (edict_t *ent);
int ConvertMeansOfDeath (int mod);
int StripMeansOfDeath (int mod);
int MeansOfDeathToServObitDeath (int mod);
void InitServObitMeansOfDeathMap();
int ServObitClientObituary (edict_t *self, edict_t *inflictor, 
							edict_t *attacker);
void ServObit_Cmd_Help_f (edict_t *ent);
void ServObit_Cmd_Score_f (edict_t *ent);
void ServObit_Cmd_Inven_f (edict_t *ent);
void ServObitInitGame ();
