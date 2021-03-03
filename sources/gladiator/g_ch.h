//===========================================================================
//
// Name:         g_ch.h
// Function:     Colored Hitman
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1999-02-10
// Tab Size:     3
//===========================================================================

extern cvar_t *ch;
extern char *ch_statusbar;

char *ColorImageName(int color);
void InitColoredHitman(void);
void PrecacheColoredHitman(void);
void UpdateColoredHitman(void);
void ColoredHitmanStats(edict_t *ent);
void ColoredHitmanEffects(edict_t *ent);
