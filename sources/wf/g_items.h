/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin, Added to by Acrid-

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin/Acrid-, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin
	-Acrid-

 *****************************************************************/
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
qboolean	Pickup_Health (edict_t *ent, edict_t *other);
qboolean	Pickup_Ammo (edict_t *ent, edict_t *other);
qboolean	Pickup_Armor (edict_t *ent, edict_t *other);
qboolean	Pickup_Pack (edict_t *ent, edict_t *other);
qboolean	Place_Special (edict_t *ent, edict_t *other);//42 Acrid

edict_t	*AddToItemList(edict_t *ent, edict_t	*head);

extern int	jacket_armor_index;
extern int	combat_armor_index;
extern int	body_armor_index;
extern int	power_screen_index;
extern int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2
