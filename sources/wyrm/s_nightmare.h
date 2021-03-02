/* _________ *\
** ¯¯¯¯¯¯¯¯¯ **
** Nightmare **
** _________ **
\* ¯¯¯¯¯¯¯¯¯ */
#define	NIGHTMARE_THINK	0.1
void	nightmareModeSetup (void);
void	nightmareResetFlashlight (edict_t *self);
void	flashlight_think (edict_t *self);
void    Use_Flashlight (edict_t *player, gitem_t *item);
void	nightmareCheckRules (void);
void	nightmareModeToggle (qboolean cmd);
void	nightmarePlayerResetFlashlight (edict_t *player);
void	nightmareClientGiveFlashlight (gclient_t *client);
void	nightmareEffects (edict_t *player);
extern	cvar_t	*nightmare;
extern	cvar_t	*nighttime;
extern	cvar_t	*lighttime;
extern	float	nightmareToggleTime;
extern	qboolean	nightmareModeState;
//Nightmare
