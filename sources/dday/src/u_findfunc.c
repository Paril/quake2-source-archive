/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/u_findfunc.c,v $
 *   $Revision: 1.6 $
 *   $Date: 2002/07/23 21:11:37 $
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
  u_findfunc.c

  vjj  03/16/98


  This file implements a binary search algorithm to rapidly find a function
  in the GlobalGameFunctionArray that is declared in the u_findfunc.h file.

  */

#include "g_local.h"
#include "g_cmds.h"
#include "u_findfunc.h"
#include "allfuncs.h"
#include "funcarry.h"  //this defines structures.

void (*FindGameFunction(char *t))
{
    int mid, hi, lo;
    int found;
    int compres;
    
    found = -1;
    lo = 0;
    hi = FUNARRAYLEN;
    mid = (hi + lo)/2;

    while(lo <= hi)
    {
        compres = strcmp(t,GlobalGameFunctionArray[mid].name);
        if(!compres)
        {
            found = mid;
            break;
        }
        else
        {
            if(compres < 0)
                hi = mid - 1;
            else
                lo = mid + 1;
        }
        mid = (hi + lo)/2;
    }

    if (found != -1)
        return GlobalGameFunctionArray[found].func;
    else
        return NULL;
}
