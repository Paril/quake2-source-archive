// g_ctf.h

/*--------------------------------------------------------------------------*/
// ID code borrowed from ZOID's CTF
/*--------------------------------------------------------------------------*/

#define STAT_CTF_ID_VIEW			27

void CTFID_f (edict_t *ent);
void CTFSetIDView(edict_t *ent);

void OpenMainMenu(edict_t *ent);
void OpenMOTDMenu(edict_t *ent, pmenu_t *p);
