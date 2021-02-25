BraZen 2.0
-----------------------------------

BraZen 2.0 is a completely re-written version of the original BraZen.
While lacking many of the originals features, it is overall more stable,
and a lot more user-friendly. Full source code is included.

The basic premise is this : to create a less rectrictive version of Quake2

To that end, you can carry and drop any item, rip weapons off dead monsters,
use two weapons and so forth.

This current version is lacking intended features (such as the ability
to climb up ledges and loot bodies), but such things are on the way.
I spent most of my time spent on coop play and making sure existing maps
could be updated with out the need to recompile the actual maps (Hence the
added .ent & .ant support).


Quick Setup
-----------

Unzip to your Quake2 directory, keeping directory structures intact.
Then create a shortcut to you quake2.exe with the command line parameters
+set game brazen. Failing that, run the included batch file (brazen.bat).

Then start a new game.


Client Commands
---------------

NOTE : keys should be configured manually (ie. edit your autoexec.cfg)
	***YOU HAVE BE WARNED!!!***

+action
	- used to pick up items
	- used to remove weapons from dead monsters
	
weapnext
	- if not up, brings up inventory
	- cycles forward through inventory items.

weapprev 
	- if not up, brings up inventory
	- cycles backwards through inventory items.

drop
	- drops item(s) in your hand(s)
	- if inventory is up, drops currently selected item
	
+attack 
	- use item(s) currently in your hand(s)
	- if inventory is up, selects item in right hand
	
+attack2
	- alt-uses item(s) currently in your hand(s)
	- if inventory is up, selects item in left hand

reload
	- reloads weapon in right hand
	
coopview
	- if playing a coop game, switch view to other player(s)

gotosecret
	- if cheats are enabled, teleport to a secret area 
	(use for editing map entity files)

give
	- if cheats are enabled, give yourself something or spawn another ent
	(ie. give all, give health 999, give weapon_shotgun, give monster_tank)


Server Commands
---------------

entz
	- if cheats are enabled, dump current maps default entity list


New cvars
---------

sv_maxdropped
	- max dropped items at any one time (deathmatch only)
	
sv_edit
	- enable edit mode. Only works from command line
	(quake2 +set game brazen +set sv_edit 1)
	
sv_equipment
	- default equipment list (deathmatch only)
	(ie. weapon_shotgun ammo_expshotgunclip ammo_expshotgunclip)


Itemlist
--------

do_not_put_me_in_map 
	- used for players hands, do no use

weapon_pistol
	- crappy pistol, shouldn't use

ammo_pistolclip

weapon_submach
	- default weapon
	
ammo_submachclip

weapon_shotgun
	- single barrel, semi-auto shotgun

ammo_shotgunclip

weapon_fraghgrenade
	- hand grenade
	
weapon_emphgrenade
	- emp hand grenade, good against machine based monsters
	 
weapon_chaingun
	- six barreled gun of best, so don't add many

ammo_chaingunpack
	- should not use!!!
	
weapon_grenadelauncher

ammo_grenades

weapon_arifle
	- assault rifle with grenade launcher that uses ammo_grenades

ammo_arifleclip

weapon_railgun

ammo_slugs

ammo_expshotgunclip
	- explosive shotgun shells, good against non-metal monsters
	(Use while shotgun is current item)

ammo_solidshotgunclip
	- good against metal monsters
	(Use while shotgun is current item)

ammo_hepgrenades
	- high explosive grenade (small radius, high damage)
	(Use while grenade launcher is current item)

ammo_empgrenades
	- emp grenade, good against machine based monsters
	(Use while grenade launcher is current item)

ammo_hvsubmachclip
	- high velocity clip, good against machine based monsters
	(Use while sub machinegun is current item)

item_armor_jacket
	- must free hands to pickup and put on

item_armor_combat
	- must free hands to pickup and put on

item_armor_body
	- must free hands to pickup and put on

item_health
	- must be current item to be used

item_health_large
	- must be current item to be used

item_bandolier
	- allows you to carry more
	(FIX ME - buggy between levels)


item_pack
	- allows you to carry A LOT more
	(FIX ME - buggy between levels)

weapon_edit
	- not a real weapon. Gives info on item pos etc

key_data_cd

key_power_cube

key_pyramid

key_data_spinner

key_pass

key_blue_key

key_red_key

key_commander_head

key_airstrike_target

weapon_stroggblaster
	- tear off a soldier, can't be reloaded

weapon_stroggshotgun
	- tear off a soldier, can't be reloaded

weapon_stroggsubmach
	- tear off a soldier, can't be reloaded

weapon_infchain
	- tear off an infantry guy, can't be reloaded

weapon_gunchain
	- tear off a gunner, can't be reloaded

weapon_medichyper
	- tear off a medic, can't be reloaded

weapon_bitchrlauncher
	- tear off a chick, can't be reloaded

weapon_tankrlauncher
	- remove from a tank, can't be reloaded


Entlist
--------
info_player_rally_point
	- placed in maps at key points
	
touchent_multiple
	- non-brush based trigger_multiple
	(used in .ent/.ant files to update maps)
	
touchent_once
	- non-brush based trigger_once
	(used in .ent/.ant files to update maps)


Coop play
---------

- If you die, you will respawn back at the last rally point that was touched 
	or back at the start of the level

- If you die, your respawn with no equipment, so be careful

- If you die in lava/slime and had a key, it will  at the last rally point that 
	was touched or back at the start of the level

- While in coopview
	- jump cycles through players
	- fire leaves coop view
	- being shot leaves coop view

- Items DO NO RESPAWN, so share weapons and other resources wisely

- most levels must be left together
	


Credits 
-------

Kris 'Mr_Grim' Rigby (mr_grim@dude.com.au)

Quake2 -> id Software for the ever changeable Quake2

EntZ 1.01 code -> Avi 'Zung!' Rozen (zungbang@telefragged.com)


And thanks to everyone who kept pesturing me for an updated version!!
Hope you like where this version is headed...
