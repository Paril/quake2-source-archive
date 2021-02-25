#include "g_local.h"
#include "m_player.h"

extern char *leper_statusbar =
"yb	-24 "

// health number
"xv	0 "
"hnum "

"yb	-32 " //height

"xv	50 "
"pic 22 "//head

"xv	66 " 
"pic 18 " //leftarm

"yb	-28 " //height

"xv	50 "
"pic 19 "//rightarm

"xv	60 "
"pic 23 "//body

"yb	-18 "//height

"xv	50 "
"pic 21 "//rightleg

"xv	62 "
"pic 20 "//leftleg


"yb	-24 "
// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "
;