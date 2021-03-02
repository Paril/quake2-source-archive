/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/u_entmgr.h,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:50 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
 
 /*
  u_entmgr.h

  vjj

  This file declares the functions that a user would have at their disposal to
  manipulate items and entities.
*/

//returns a pointer to the gitem_t allocated in the itemlist, null if failed

gitem_t *InsertItem(gitem_t *it,spawn_t *spawnInfo);

//returns a pointer to the spawn_t structure in the spawns[], null if failed

spawn_t *InsertEntity(spawn_t *spawnInfo);


//finds and removes the entity. returns the old index, -1 if not found

int RemoveEntity(char *name);
