/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_script.h
  Description: DM/SP script opcode 

\**********************************************************/

#ifndef SCRIPT_H
#define SCRIPT_H

#define SCR_GOTO           0           // Done
#define SCR_WAIT           2           // Done
#define SCR_SAY            18          // Done
#define SCR_WALK           19          // Done
#define SCR_RESERVED       20          // Done
#define SCR_RUN            21          // Done
#define SCR_DIE            23          // Done
#define SCR_DROP           24          // Done
#define SCR_ROTATE_SMOOTH  25          // Done
#define SCR_REMOVE         26          // Done
#define SCR_HIDE           27          // Done  
#define SCR_CHASE          28          // Done
#define SCR_ON_BLOCK       29          // Done
#define SCR_ON_USE         30          // Done
#define SCR_ACTION_TARGET  31          // Done
#define SCR_ATTACK         33          // Done
#define SCR_SPAWN          34          // Done
#define SCR_ON_TOUCH       36          // Done
#define SCR_ON_SEE         37          // Done
#define SCR_ON_PAIN        38          // Done
#define SCR_RETURN         40          // Done
#define SCR_LOOK_AT        41          // Done
#define SCR_ACTION         42          // Done
#define SCR_STAND          43          // Done
#define SCR_USE            44          // Done
#define SCR_STOP           46          // Done
#define SCR_CAMERA         47          // Done
#define SCR_SCOPE          48          // Done
#define SCR_RUN_SLAVE      49          // Done
#define SCR_TARGET_CAMERA  50          // Done
#define SCR_ROTATE_CAMERA  51          // Done
#define SCR_ROTATE         52          // Done
#define SCR_ROTATE_SLAVE   53          // Done
#define SCR_LOOK_AT_SLAVE  54          // Done
#define SCR_ACTION_SLAVE   55          // Done
#define SCR_STAND_SLAVE    56          // Done
#define SCR_SAY_SLAVE      57          // Done
#define SCR_LOAD_MAP       58          // Done
#define SCR_STUFFCMD       59          // Done
#define SCR_LISTEN			60          // Done
#define SCR_CREDITS			61				// Done
#define SCR_MULTI_USE		62				// Done
#define SCR_ZOOM				63				// Done
#define SCR_SND_LOOP			64				// Done
#define SCR_SND_LOOP_SLAVE	65				// Done
#define SCR_MOVE				66				// Done
#define SCR_MOVE_SLAVE		67				// Done

#define SCR_CRITICAL			80				// Done
#define SCR_FREE				81				// Done


#endif