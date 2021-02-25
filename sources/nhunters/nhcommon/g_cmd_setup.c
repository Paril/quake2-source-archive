//============================================================================
// g_cmd_setup.c
//
// Player control setup
//
// Copyright(c) 1999, The BatCave, All Rights Reserved.
//============================================================================ 

#include "g_local.h"
#include "g_cmd_setup.h"

void Cmd_Setup_f(edict_t *ent) {

  // NH changes: Night Hunters default bindings
  stuffcmd(ent, "bind g flashlight\n");
  stuffcmd(ent, "bind m menu\n");
  stuffcmd(ent, "bind v gunscope\n");
  stuffcmd(ent, "bind o overload\n");
  stuffcmd(ent, "bind e anchor\n");
  stuffcmd(ent, "bind r recall\n");
  stuffcmd(ent, "bind f flare\n");
  stuffcmd(ent, "bind q report\n") ;
  stuffcmd(ent, "bind [ invprev\n");
  stuffcmd(ent, "bind ] invnext\n");
  stuffcmd(ent, "bind enter invuse\n");
  stuffcmd(ent, "bind tab inven\n");
  gi.cprintf(ent, PRINT_HIGH, "\nCreating default bindings for game:\n\n");

  gi.cprintf(ent, PRINT_HIGH, "bind g flashlight\n");
  gi.cprintf(ent, PRINT_HIGH, "bind m menu\n");
  gi.cprintf(ent, PRINT_HIGH, "bind v gunscope\n");
  gi.cprintf(ent, PRINT_HIGH, "bind o overload\n");
  gi.cprintf(ent, PRINT_HIGH, "bind e anchor\n");
  gi.cprintf(ent, PRINT_HIGH, "bind r recall\n");
  gi.cprintf(ent, PRINT_HIGH, "bind f flare\n");
  gi.cprintf(ent, PRINT_HIGH, "bind q report\n") ;
  gi.cprintf(ent, PRINT_HIGH, "\nCreating default bindings for menu:\n\n");

  gi.cprintf(ent, PRINT_HIGH, "bind [ invprev\n");
  gi.cprintf(ent, PRINT_HIGH, "bind ] invnext\n");
  gi.cprintf(ent, PRINT_HIGH, "bind enter invuse\n");
  gi.cprintf(ent, PRINT_HIGH, "bind tab inven\n\n");
  gi.cprintf(ent, PRINT_HIGH,
	     "Press your FLASHLIGHT key to join the game!!!\n\n");

} 
