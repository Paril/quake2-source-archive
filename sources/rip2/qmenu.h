/*

	QMENU - Quake 2 Menuing System
   ------------------------------
   (c) 1998, Online Gaming Technologies.

   This mod. allows mod creators to add a very nice menuing system to
   their mods.  It utilizes the inventory window to allow for easy
   access to options or menus.

   You may merge QMENU in to any of your custom mods, but please
   note that you are using it.  Any questions can be forward to:
                        qrank@ogt.net

   This mod and the source code may not be included with any time
   of commerical product nor may it be used on a commerical of any kind
   without the written concent of Online Gaming Technologies.

*/

void Menu_Hlp(edict_t *ent);
void Menu_Msg(edict_t *ent,char *message);
void Menu_Init(edict_t *ent);
void Menu_Clear(edict_t *ent);
void Menu_Add(edict_t *ent, char *text);
void Menu_Title(edict_t *ent, char *text);
void Menu_Open(edict_t *ent);
void Menu_Close(edict_t *ent);
void Menu_Up(edict_t *ent);
void Menu_Dn(edict_t *ent);
void Menu_Sel(edict_t *ent);

