//===========================================================================
//
// Name:				p_botmenu.h
// Function:		menu
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

//create the bot menu
void CreateBotMenu(void);
//close the bot menu for the given client
void CloseBotMenu(edict_t *ent);
//open the bot menu for the given client
void OpenBotMenu(edict_t *ent);
//toggle the bot menu for the given client
void ToggleBotMenu(edict_t *ent);
