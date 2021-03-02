/*
* Copyright (C) 2006 by QwazyWabbit and ClanWOS.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* You may freely use and alter this code so long as this banner
* remains and credit is given for its source.
*/
//
// for maplist.c
//
// Declarations for maplist rotation code
// Use #include "maplist.h" in files where the maplist
// functions are called:

// g_save.c
// Call Maplist_InitVars from your mod's InitGame function

// g_main.c
// Call Maplist_Next in EndDMLevel to make it do the rotations.

// Set the cvars in your server.cfg the way you want them.
// The default values are defined in Maplist_InitVars.

#ifndef MAPLIST_H
#define MAPLIST_H

// basic maplist handling
extern cvar_t *maplist;      //line pointer
extern cvar_t *maplistfile;   // the file name when not load sensing
extern cvar_t *gamedir;      // our mod dir
extern cvar_t *basedir;      // our root dir
extern cvar_t *mymaplistfile;   // the working maplist

// for maplist selection varies with player load
extern cvar_t *maplist1;   // line pointer low load
extern cvar_t *maplist2;   // line pointer med load
extern cvar_t *maplist3;   // line pointer high load
extern cvar_t *maplistvaries;   // set 1 if you want variation
extern cvar_t *maplistfile1;   // file name of low-load list
extern cvar_t *maplistfile2;   // name of medium load list
extern cvar_t *maplistfile3;   // name of high load list
extern cvar_t *lowcount;      // upper bound of low load
extern cvar_t *medcount;      // upper bound of medium

//public
qboolean   Maplist_Next (void);
void   Maplist_InitVars(void);
qboolean Maplist_CheckStockmaps(char *thismap);
qboolean Maplist_CheckFileExists(char *mapname);

//private
static int Maplist_CountPlayers(void);

#endif 