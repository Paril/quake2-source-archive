/*

	QMENU - Quake 2 Menuing System
   ------------------------------
   (c) 1998, Online Gaming Technologies.

   This mod. allows mod creators to add a very nice menuing system to
   their mods.  It utilizes the inventory window to allow for easy
   access to options or menus.

   You may merge QMENU in to any of your custom mods, but please
   note that you are using it.  Any questions can be forward to:
                        qmenu@ogt.net

   This mod and the source code may not be included with any time
   of commerical product nor may it be used on a commerical of any kind
   without the written concent of Online Gaming Technologies.

*/

#include "g_local.h"

/*
   ==========================================================

   Menu_Hlp - Displays a help text about how to use the
   menuing system.

   ==========================================================
*/

void Menu_Hlp(edict_t *ent)
{

   gi.cprintf(ent,PRINT_HIGH,"\n");
//                         1234567890123456789012345678901234567890
   gi.cprintf(ent,PRINT_HIGH,"This sever is running a mod that makes\n");
   gi.cprintf(ent,PRINT_HIGH,"use of the QMENU in game menuing system.\n");
   gi.cprintf(ent,PRINT_HIGH,"\n");
   gi.cprintf(ent,PRINT_HIGH,"QMENU makes selecting options as simple\n");
   gi.cprintf(ent,PRINT_HIGH,"as using the inventory system built in\n");
   gi.cprintf(ent,PRINT_HIGH,"to quake it'self.\n");
   gi.cprintf(ent,PRINT_HIGH,"\n");
   gi.cprintf(ent,PRINT_HIGH,"When in a QMENU, use the same keys\n");
   gi.cprintf(ent,PRINT_HIGH,"you would use to move through Quake 2's\n");
   gi.cprintf(ent,PRINT_HIGH,"inventory (ie: Next_Inventory and\n");
   gi.cprintf(ent,PRINT_HIGH,"Prev_Inventory).  When you wish to\n");
   gi.cprintf(ent,PRINT_HIGH,"select an option, press your use key!\n");
   gi.cprintf(ent,PRINT_HIGH,"\n");
   gi.cprintf(ent,PRINT_HIGH,"Press ESC will abort any menu.\n");
   gi.cprintf(ent,PRINT_HIGH,"\n");
}



/*
   ==========================================================

   Menu_Msg - Displays a static message in a box.  You have
   14 lines x 27 cols.  This menu can be closed by pressing
   ESC.
   ==========================================================
*/
void Menu_Msg(edict_t *ent,char *message)
{

	char final[1024];
   char work[80];
   char tmp[80];
   int i,j,k,l;


	if (ent->client->showinventory ||  ent->client->showmenu ||
       (ent->client->showscores && !ent->client->showmsg))
     return;


   if (ent->client->showmsg)
   {

      Menu_Close(ent);
      return;

   }

   sprintf (final, "xv 32 yv 8 picn inventory ");

   // Break the message down in to up to 14 different strings.

   i=j=k=0;
   l = 24;
	while (k<14)
   {

      if (strlen(final)==0)
      {
        gi.bprintf(PRINT_HIGH,"Turned zero at char: %i[%c]\n",i,message[i]);
      }

      if (message[i]=='\0')
        break;

      else if (message[i]=='\n')
      {

        work[j]='\0';

        if (strlen(work)>0)
        {
          sprintf(tmp,"xv 50 yv %i string2 \"%s\" ",l,work);
          strcat(final,tmp);
        }
        l+=12;
        j=0; work[j]='\0';
        k++;


        i++;

        if (k==14)
          break;

      }
      else
      {

         work[j] = message[i];
         j++;
         if (j==28)
         {
           work[j]='\0';

           sprintf(tmp,"xv 50 yv %i string2 \"%s\" ",l,work);
           strcat(final,tmp);

           l+=12;
           j=0;work[j]='\0';
           k++;

           // Swallow the rest of the line until a \0 or \N is reached;

           while (message[i]!='\0' || message[i]!='\n')
             i++;

           if (k==14)
             break;

         }

      }

      i++;

   }

   if (j>0 && k<14)
   {
     work[j]='\0';
     sprintf(tmp,"xv 50 yv %i string2 \"%s\" ",l,work);
     strcat(final,tmp);
   }


   ent->client->showscores = true;
   ent->client->showmsg = true;

   ent->client->showmenu = false;
	ent->client->showinventory = false;

 	gi.WriteByte (svc_layout);
	gi.WriteString (final);
	gi.unicast (ent, true);



} // Menu_Msg

/*
   ==========================================================

   Menu_Init - Sets the whole menuing system up for each
   client that connects.

   ==========================================================
*/

void Menu_Init(edict_t *ent)
{

  int i;

	for (i=0;i<11;i++)
	  ent->client->menu_items[i]=NULL;

	ent->client->menu_title = NULL;

	ent->client->menu_item_count=0;


} // Menu_Init


/*
   ==========================================================

   Menu_Clear - This function needs to be called before you
   begin adding items to the menu.  It will blank out the
   menu space for a given player

   ==========================================================
*/
void Menu_Clear(edict_t *ent)
{
   int i;


   if (!ent->client->showmenu)
     return;

	for (i=0;i<11;i++)
     if (ent->client->menu_items[i] != NULL)
  	    free(ent->client->menu_items[i]);

   if (ent->client->menu_title!=NULL)
   	free(ent->client->menu_title);

   ent->client->menu_item_count=0;
   ent->client->menu_choice=0;

} // Menu_Clear

/*
   ==========================================================

   Menu_Add - This function is used to add menu items to
   a menu list.  Currently there is an 11 item limit to the
   menu.  If someone wants to implement page up/down be my
   guest.
   ==========================================================
*/

void Menu_Add(edict_t *ent, char *text)
{

  if (ent->client->menu_item_count>11)
  {
    gi.dprintf("ERROR: Too many items in menu [%i]\n",ent->client->menu_item_count);
    return;
  }

	ent->client->menu_items[ent->client->menu_item_count] = malloc(22);

   if (ent->client->menu_items[ent->client->menu_item_count] == NULL)
     return;

   sprintf(ent->client->menu_items[ent->client->menu_item_count],"%-21s",text);
	ent->client->menu_item_count++;

} // Menu_ADD


/*
   ==========================================================

   Menu_Title - This function sets the title of the menu

   ==========================================================
*/

void Menu_Title(edict_t *ent, char *text)
{
  ent->client->menu_title = malloc(28);

  if (ent->client->menu_title==NULL)
    return;

  strncpy(ent->client->menu_title,text,27);

  // Make sure it's null-terminated

  ent->client->menu_title[27]='\0';

} // Menu_Title

void Menu_Title1(edict_t *ent, char *text, char *s)
{
  ent->client->menu_title = malloc(28);

  if (ent->client->menu_title==NULL)
    return;

  strncpy(ent->client->menu_title,text,27);

  // Make sure it's null-terminated

  ent->client->menu_title[27]='\0';

} // Menu_Title
  /*
   ==========================================================

   Menu_Show - This function builds the menu and displays it
   to the screen.  If it's already visible, it will update it.

   ==========================================================
*/

void Menu_Open(edict_t *ent)
{
 	int i,j;
   char tmp[80];
   char tmp2[80];
   char final[1024];
	// First, we are using the help menu, but using the inventory background

	sprintf (final, "xv 32 yv 8 picn inventory ");

   // Build the menu, starting with the title.  If the title is null, then
   // we add our own title.

   if (ent->client->menu_title==NULL)
     sprintf(tmp,"xv 50 yv 36 string2 \"  QRANK - Select option\" ");
   else
     sprintf(tmp,"xv 52 yv 36 string2 \"  %s\" ",ent->client->menu_title);

   strcat(final,tmp);

   sprintf(tmp,"xv 52 yv 45 string2 \"  -------------------------\" ");
   strcat(final,tmp);


	// now, add all the menu items

   j=57;
   for (i=0;i<ent->client->menu_item_count;i++)
   {
	  if (i == ent->client->menu_choice)
      {
         strcpy(tmp2,ent->client->menu_items[i]);

        // To make the selected string come out in WHITE instead of
        // Green, set the byte to the upper character set.

/*      for (k=0; k<strlen(tmp2) ; k++)
           if ((byte)tmp2[k] <= 127)
              tmp2[k]=(byte)tmp2[k]+128;
*/
      sprintf(tmp,"xv 52 yv %i string \" %s   \" ",j,tmp2); // ent->client->menu_items[i]);
      }
      else
    		sprintf(tmp,"xv 52 yv %i string2 \"  %s   \" ",j,ent->client->menu_items[i]);


	   strcat(final,tmp);

      j+=8;

   }

   ent->client->showscores = true;
   ent->client->showmenu = true;

   ent->client->showmsg = false;
	ent->client->showinventory = false;

	gi.WriteByte (svc_layout);
	gi.WriteString (final);
	gi.unicast (ent, true);


} // Menu_Open

/*
   ==========================================================

   Menu_Close - This function closes the windows and frees up
   all resources.

   ==========================================================

*/

void Menu_Close(edict_t *ent)
{

   Menu_Clear(ent);

   ent->client->showscores = false;
   ent->client->showmenu = false;
   ent->client->showmsg = false;
   ent->client->showinventory = false;
} // Menu_Close

/*
   ==========================================================

   Menu_Up - Moves the select box up 1 on the menu.  If it
   hits the top, it wraps around.

   ==========================================================

*/
void Menu_Up(edict_t *ent)
{
  if (ent->client->menu_item_count==0)
    return;


  if (ent->client->menu_choice==0)
    ent->client->menu_choice = ent->client->menu_item_count-1;
  else
    ent->client->menu_choice--;

  Menu_Open(ent);

} // Menu_Up;

/*
   ==========================================================

   Menu_Dn - Moves the select box down 1 on the menu.  If it
   hits the top, it wraps around.

   ==========================================================

*/
void Menu_Dn(edict_t *ent)
{

  if (ent->client->menu_item_count==0)
    return;


  if (ent->client->menu_choice==ent->client->menu_item_count-1)
    ent->client->menu_choice = 0;
  else
    ent->client->menu_choice++;

  Menu_Open(ent);

} // Menu_Up;

/*
   ==========================================================

   Menu_Sel - Closes the current menu (if it's active) and
   then makes a call out to the user Menu Handler

   ==========================================================

*/

void Menu_Sel(edict_t *ent)
{
  int sel;

  sel = ent->client->menu_choice;

  Menu_Close(ent);

  ent->client->usr_menu_sel(ent,sel);  // Make the call
  ent->client->usr_menu_sel = NULL;
} // Menu_Sel


