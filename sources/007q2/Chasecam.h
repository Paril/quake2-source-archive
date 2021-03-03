//SKULL
  cvar_t  *tpp;
  cvar_t  *crossh;
void CheckChasecam_Viewent (edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void ChasecamRemove (edict_t *ent, int opt);
void ChasecamStart (edict_t *ent);
void Cmd_ToggleHud (void);
char *single_statusbar;
/*void MakeFakeCrosshair (edict_t *ent);
void UpdateFakeCrosshair (edict_t *ent);
void DestroyFakeCrosshair (edict_t *ent);
*/#define OPTION_OFF        0
#define OPTION_BACKGROUND 1
//END
               