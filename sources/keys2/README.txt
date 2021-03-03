Keys2 1.94 - ABSOLUTE Final Release #2
Rich Shetina
rshetina@planetquake.com

Changes from 1.93a

	- Merged Keys2 code with the 3.20 source to gain all the enhancments.
	- Removed usevwep cvar - not needed VWEP is automatic
	- Removed my lame IP Banning support...use id's instead
	- Haste now fires 3x instead of 2x
	- Infliction is 3x instead of 2x
	- Added Freeze Grenades
	- Added client-side botnum command
	- Added maxbots cvar so server admins canlimit the number of bots clients can add
	- Added single player "join" command because some people don't understand how to use
          the menu.
	- id View does not id Stealth players
	- Fixed "nohominghyperblaster" not working
	- Added client commands to quick-change to weapons:

		drunk
		firerl
		freezegl
		firegl	
		flashgl
		gib

	- Removed id Chase camera.  The Q2 Camera works better.
	  Q2 Camera Commands:

		cam on
		cam follow
		cam off
	  
	  There are other Q2 Camera Commands.  Consult the documentaion for Q2 Camera.

	- Added "connectlogging" cvar.  Setting to 1 (on) will log each player name and IP
          that conects to the server.   The file name is connect.log.

	- Added "botfraglogging" cvar.  Setting to 1 (on) will allow bot frags to be logged.
	  NOTE: Bot frags are logged only when a real player kills a bot.  If a bot kills a bot
	        or a bot kills a player, it's not logged. 

	Notes: Freezing bots works, but they can sometimes still move a little.


Changes from 1.93

	- Fixed player chat messages not appearing on dedicated console
	- Fixed Gibstats Logging so the file is written every map change
	- Fixed "sticky" hook

Changes from 1.92

	- Fixed shell color rendering issues (again) introduced by 
          Quake2 version 3.20.
        - Fixed nomegahealth not working properly.

Changes from 1.90

	- Added Zbot detection and "correction" :-)  support. 
	  When a Zbot is detected, it will be disconnected from the game.
          To use, set nozbots to 1 in keys2.cfg
	- Changed k2maps.txt file so it now has two seperate sections,
	  one for DM and one for CTF.
	- Fixed "bug" where if you dropped a key on purpose and then died,
          you were still the previous owner of the key.
	- Removed always_a_bot var and functionality.
	- Replaced all printf-type functions with bot-safe functions.
	- Added custom pak5.pak

	KNOWN BUGS
	----------
	Have seen where when playing CTF with bots, the server will stop responding
	to clients and lock up.  No error is generated.


Changes from 1.84

	- Eraser Bot Support! (Version 1.90 oly)
		This includes the Q2 Camera which can be used to view any
		player including bots. Unlike the original Q2 Camera,
		this one can be turned on and off at will.

		Keys2-specific features:
			
			Bots with Anti-Key will take other player keys
			
			When bots have the BFK, their aggression is maxed-out.
			When they drop the BFK, their aggression resets to
			their default value.
			
			When bots pickup the Homing key, they will choose
			the best homing weapon.
			
			Bots can be blinded by flash grenades.  When blind,
			they lose track of their current enemy, and can't
			find new ones.
		
			Bots run from Homing projectiles (very entertaining :-))

			Bots won't attack spawn-protected players.

			Bots don't "see" Stealth or Feigning players
			(until attacked, or if they were already chasing them)

			Bots will use their key for half of the key time, then
			will compare their key with others nearby.  If the
			nearby key is better than the one they have, they will
			drop their key and get the new one.

				Bots rank the keys as follows (ascending order)
					ANTIKEY			
					REGENERATION	
					KEY_FUTILITY	
					KEY_INFLICTION	
					KEY_HASTE		
					KEY_STEALTH		
					KEY_HOMING		
					KEY_BFK			

			Bots try to avoid players with the BFK (unless they have
			BFK as well)

			new cvar always_a_bot - Setting this to 1 will make sure 
			there is always a bot in the game.  Note that this will
			give the impression that the server has a free connection
			slot, but it really does not, because the bot is "using" it.
			There is no facility to send a message to the client to
			tell him this.  The client will just get a "Connection
			refused" message.
			
	- Updated GibStats logging
		- Now logs ping time of attacker
		- The game checks the log file for errors and fixes
		  them if needed.
	
	- Added totalstealth cvar
		Setting to 1 will make the Stealth player *totally*
		invisible unless firing or taking damage
	
	- Added new cvar, resetlevels
		Setting this to 1 will cause Keys2 to re-read k2maps.txt and
		restartlevel cycling.  This should be set to 0 at server 
		initialization.
	
	- Added IP Banning support. (Version 1.90 oly)

	- Enhanced Means of Death identifiers and messages for Homing
	  projectiles

	- Bug Fixes
	    Level cycle blank map bug
	    NULL ent->think errors don't cause a server error (crash)
	    droptofloor errors would cause a key to not respawn in the game
	    now the key respawns.

	- New Welcome Banner and more motd cvars.

	Refer to the Keys2 Web Site for instructions on using the new features.
	http://www.planetquake.com/keys2

Changes from 1.83

	- In CTF, player shells now "flash" so you can see what team a key-holding
		player is on.
	- Fixed capturelimit countdown (was not working)
	- Changed "set vwep" to "set usevwep" in keys2.cfg (VWEP was not working)
	- Changed the Stealth sound code for better effect
	- Added Player Menu which allows:
		- Observer mode
		- Chase Cam
		- Help/Information
	- Added skyhook variable in keys2.cfg to allow/disallow sky hooking.  
	- Updated GSLogMod support. New variables added to keys2.cfg

	- Summary of keys2.cfg changes:

		Added "set skyhook"
		Added "set stdlogfile"
		Added "set stdlogname"
		Changed "set gsfraglog" ***Only required for Sparc Platform

Changes from 1.82
	
	- Fixed bug where starting without swaat tunred on would crash the server

Changes from 1.81

	- Changed Stealth effect so that you "uncloak" only when firing or taking damage
	- Fixed CTF Player Shell rendering problems
	- Fixed CTF Homing crash
	- Added customizable weapon damage

Changes from 1.80

	- Correctly renders shell effects based on game version

Changes from 1.71

	- Integrated Zoids CTF
	- Removed monster code
	- Changed stealth effect:
		When you have Stealth Key, your vision gets a little darker
		You are cloaked only while standing still, or walking
		You "uncloak" when you run, take damage, or fire your weapon.

Changes from 1.69

	- Keys2.ini is no longer used, replaced by keys2.cfg
	- Maps for rotation now kept in k2maps.txt
	- All Keys2 settings are now "set" commands, most of which can be changed at any time.
	- Most Keys2 parameters can be changed on a per-map basis
	- Starting Weapos/Armor/Ammo/Health can be customized
	- Each key has an individual settable timer
	- You can specifiy how many of each individual key are spawned.
	- You can exclude weapons from the game.
	- You can exclude the Quad, Pent, and Megahealth powerups.

Starting Keys2

	The command to start a Keys2 server is now:

		quake2 +set game keys2 +exec keys2.cfg 

	You can append any other set commands you wish.

Notes

	I'm told that setting the rcon_password in the keys2.cfg file doesn't work.
	Set it on the command line or in a server.cfg file.
	
	Make sure that the gamemap or map command is the very last command in 
	your keys2.cfg file.
