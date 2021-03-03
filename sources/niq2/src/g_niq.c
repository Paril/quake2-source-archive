/////////////////////////////////////////////////////////////////////////////
// g_niq.c:
//
// Main file for implementing the NIQ mods for Quake II. Its getting a bit
// large, but everything is in one place...
//
/////////////////////////////////////////////////////////////////////////////
// History
//
/////////////////////
// 02/09/98 NIQ 1.95:
//
// Fix for hook crash if released then refired quickly.
//
/////////////////////
// 02/09/98 NIQ 1.94:
//
// Got newer hook code from James A. (author of Vanilla CTF).
//	- hook does less damage
//  - misc tweaks
// Hooking sky is now optional (default=disabled).
//
/////////////////////
// 02/09/98 NIQ 1.93:
//
// Made "Logging enabled" information come up even if not enabled.
// Added tractor beam style hook. Bind a key to +hook to have this
// key fire/release the tractor beam.
//
/////////////////////
// 02/09/98 NIQ 1.92:
//
// Added a logging status entry to help screen #4. Thanks Louis for doing
// a great job on adding the Logging support.
// 
// Fixed the problem (I think) with the #$%*&%+ scoreboard staying up for
// too long after you get killed and respawn so you can't see anything for
// a while.
//
/////////////////////
// 02/09/98 NIQ 1.91:
//
// Additions for version 1.91 were made by Louis de Carufel (aka Mr.Bungle).
// All new code is marked with 'BUNGLE' for easy removal :)
//
// The version strings are now 4 characters long.  That way it's possible
// to use "1.91" as the current version.
//
// Added support for Standard Logging.  The generated logfiles are placed
// in the Quake2 root directory and can be parsed by programs such as
// GibStats to compile statistics about players, levels, etc. Lots of fun!
//
// The code for the logfile generation was originally written by Mark Davies.
// The files gslog.c, gslog.h, stdlog.c and stdlog.h are taken from his
// Logging Toolkit 1.0 found on the GSLogMod web site.
//
// To enable logging with NIQ, add the following lines in NIQ.CFG:
//
//      set stdlogfile 1                <- Logging master switch
//                                         (default is 0)
//      set stdlogname "NAME.LOG"       <- Name of the logfile
//                                         (default is "StdLog.log")
//
// The logfile is appended to any existing file, so delete the old
// logfile if you don't want this to happen.  I don't know if GibStats
// handle multiple games per logfile well.
// 
// mfox comments: 
//	Logging doesn't seem to work with bots.
//  Logging does not currently take into account the NIQ scoring system.
//
////////////////////
// 02/06/98 NIQ 1.9:
//
// Fix for clients spawning outside map etc.
// 
// niq_blk1=1 will disable the no telefrag while observing fix in case this
// causes problems.
//
// niq_blk2=1 will disable putting clients into observer mode when joining
// the server.
//
////////////////////
// There was no NIQ 1.8, I think.
//
////////////////////
// 02/06/98 NIQ 1.6/1.7:
//
// First cut at integrating NIQ and Eraser Bots code. Seems to work pretty
// well so far.
// 
// Sorting was screwed up. If score A <= score B, A could still go ahead
// of B on the scoreboard if his df was less.
// 
// Added current df for each client in addition to average df, because
// these can be quite different.
//
// When client selects 512 or 640 scoreboard, shows msg indicating resolution
// needed.
//
// Added code to take NIQ_DEF_VERSION and automatically stick in all titles 
// etc.
//
// Redid all scoreboards. Added PPH (Points Per Hour). Cleanup. There are 
// now 9 scoreboards to choose from (2 of these are unavailable if handicap
// is disabled, 1 requires 512x384 or greater, 1 requires 640x400 or greater
// and is only used for debugging).
//
// Note that the 640x480 debug scoreboard shows the current weapon, health
// and ammo amount for all clients, including bots. Also, if a client is
// actively attacking (weapon is currently firing), a '*' will appear 
// beside the weapon.
//
// The '1' key will now toggle the HUD if the scoreboard isn't up, and will
// continue to toggle the scoreboard when it is up.
//
// Reorganized all HUDs. For example, the health, ammo and weapon information
// is now spread evenly across the bottom of the screen.
//
// Now show selected item icon in sp/coop if this is not the same as the
// current weapon.
//
// Turned all cvars into non-serverinfo, non-latched vars so these can
// still be changed immediately but they just won't show up in serverinfo.
// This is how I should have done this in the first place, I had assumed
// that I *had* to use a CVAR flag like CVAR_LATCHED or CVAR_SERVERINFO.
// Also putting back easier to understand variables and getting rid of
// combined flags variables. This will break old .cfg files (again).
//
// Help menu is now N menus, Esc/nhelp clears them, jump gives the next
// one. Can have much more detail in these now, such as what clients 
// can do, explain Esc problem etc.
// 
// Added the sv ninfo command so the server operator can easily see all
// NIQ settings.
//
// Added player id. Works in all modes, including deathmatch, non-ctf mode.
// Not showing word "Viewing" in front of name (no need). Works in observer 
// mode except the client can't be clipped while IDing a player/bot. If
// a scoreboard or menu is up, player id is disabled temporarily.
//
// Bots have a green * appended to their name to identify them as bots.
//
// Got rid of almost all eraser bot serverinfo vars for now.
//
// Move to Eraser Bots 0.992 source.
//
// Can have the q2ctf grappling hook in regular games, just set grapple to
// 1. Bots will use it too. Needs the q2ctf pak0.pak for the graphics and
// sounds. It will actually work without these but its not pretty. This
// is currently an 'on-line' hook so you can't fire your weapon while 
// using it. I may make this optionally off-line using standard laser
// beam graphics and sounds so clients don't need the q2ctf pak.
//
// Added a small console background bitmap. You need to have the NIQ
// pak5.pak file in your quake2/niq directory to see this (its the NIQ
// logo -- upside down Q2 logo with an extra spike) and if you don't
// have this, no problem -- you will simply see the regular background.
// Thanks to Joseph Davila for this.
//
// Supports any of "use grapple", "use hook", "use grappling hook" to 
// try to be compatible with some of the other mods that use hooks.
// 
// Now simply show xx instead of clients=xx in the NIQ scoreboards
// since it was easy to see this before and think someone was joining.
// 
// The 640 column debugging scoreboard is now available in release
// builds if niq_sbdebug is set to 1. Careful as this uses a lot of
// data and is updated every 1/10th of a second.
//
// Server can disable wide scoreboards (>=512 columns needed) if these
// cause any problems (e.g. lag, crashes). No known problems though.
//
// Servers can disable the mini-SB on the HUD for the same reason if
// necessary.
//
// Server can control wait between levels before next map is automatically
// loaded. Normally, after 5 seconds, some client has to hit a button/key
// to have this happen. Note: there is possibly a bug, probably only when
// using bots, that can cause the server to lock up when a client hits a
// key between levels at the 'wrong time'. The server will actually continue
// to run with the bots, but clients will see it time out. If this happens,
// try setting niq_inctime to 1 to 5 seconds or even -1 to force the next
// level to load immediately as this seems to fix the problem in some cases.
//
// Killed messages now show how much health the attacker had left so you can
// so how close you came (or didn't) to killing him yourself.
// 
// The console variable niq_enable can be set to 0 to disable NIQ so that
// the server will run a regular game (with items). This might be nice if
// you want to play a regular (e.g. against bots) without having to have
// a separate mod installed (e.g. with route tables).
//
// Clients are not put into observer mode initially so they can read the
// help screens and set things up (e.g. select the scoreboard to use) 
// without getting fragged.
//
// MOTD text in motd file is now read once and saved. The number of lines
// is now limited to 4.
// 
// Note that it may be possible to win the level with a score which is less 
// than the fraglimit if you are killed just after killing someone
// (usually because their rocket hits you just after you hit them because
// once you hit the fraglimit, we don't take into account the fact that 
// you could lose points shortly after for getting killed). This should
// never be a problem if niq_kildpts is < 0.5.
//
// Almost all settings that anyone could possibly be interested in are
// now shown in the info screens. No support for now for dedicated servers
// to see these (too much trouble and can't scroll console anyway).
// 
// Made handicapping a bit more obscure. Its off by default and unless its
// on, the scoreboards with DFs in them don't even show up.
//
// Added a new scoreboard which shows time, ping, PPH, rank, score and full 
// 15-char names.
//
// Weapon switch and switch warning sounds are now played on CHAN_ITEM so
// jumping, running, firing won't override these, and the health increment
// sound is played on CHAN_HLTH (=5) so these use their own channel too.
// I guess if the sound hardware doesn't support this many channels, some
// of these could overlap.
//
// Moved to a modified version of the MapMod map cycling code. If niq_allmaps
// is set along with mapmod_random, all maps in the list are used and no map
// is used 2x in a row (unless there is only 1 map in maps.txt...).
//
// Fixed a problem which I think is in the q2ctf code as well. If you are in
// observer mode, you can get telefragged (at least by a bot) if you are too
// close to a teleport. Added a check for the 'target' being in observer mode
// in which case it is not telefragged.
//
// Choice of 5 HUD mini-SB layouts (combinations of rank, name, pph and score)
// and mini-SB shows team color in CTF mode, or first letter of teamname in
// teamplay mode.
//
// Added the "sv status" command for showing all clients, including bots on
// the server.
//
////////////////////
// 01/06/98 NIQ 1.5:
//	
// Fixed niq_wflags not being used correctly so weapons were never random.
//
////////////////////
// 31/05/98 NIQ 1.4:
//
// Fixed niq_kildpts not used problem.
//
////////////////////
// 26/05/98 NIQ 1.3:
//	
// The df which is shown in the scoreboard is now the average df over the
// time that each client is connected. This prevents clients from cheating
// by playing with a high df then switching to a low one just before the
// end of the level (so they look like they were playing with a low df all
// the time).
//
// If/when handicapping changes to disabled (niq_flags & xxx), make sure we
// force all clients to df=1.0. If client's df was being averaged before
// (tics != 0), we will continue to average it until the map changes.
//
// Optimized timer code a bit. There is now a single 1 second timer used 
// for incrementing health, incrementing ammo and averaging out the df.
// When the current weapon switches, some clients might start getting 
// ammo right away and others might have to wait up to a second depending
// on where their timers were, but I don't see this as a problem at all.
//
// Added map cycling. Users can specify the maps that they want to use 
// in the file	niqmaps.txt and these will be cycled (once some initial
// map is specified -- starting right off the bat with the first map in
// the file isn't supported yet).
//
////////////////////
// 25/05/98 NIQ 1.2:
//
// Mainly fixes for still more serverinfo problems.
//
////////////////////
// 18/05/98 NIQ 1.1:
//
// Cleaned up a few problems. Fixed serverinfo problem.
//
////////////////////
// 18/03/98 NIQ 1.0:
//
// Initial release with 90% of NIQ's basic idea implemented.
//
// ###
/////////////////////////////////////////////////////////////////////////////
// BOT NOTES:
//
// For now, the only changes which have been made to the Eraser Bot code are
// those which are needed to ensure that all bots are using the current weapon
// and have their health and ammo incremented automatically. Bots, like human
// clients, can not have their health increase while they are fighting (or
// drowning/burning/sliming). The definition of 'bot is fighting' is whether
// the weapon fire time (see niq_weap_fire_times) has expired since the bot
// last fired his weapon. This is more or less identical to the rule used for
// humans, since we look at weather the weapon is still firing flag is set 
// and this is controlled by similar logic. The only difference is that if
// a human player is pressing the attack key, his health also can't increase,
// whereas this test obviously doesn't apply to bots.
//
// I have modified the bot AI slightly so that when they are out of ammo 
// they will try to avoid aquiring a target. In actual play, this seems
// to work -- the bots definitely don't seem follow you like they did before
// when they are out of ammo and don't seem to acquire new targets.
//
// I also disabled (from what I can tell) some of the code related to selecting
// an item to go after, with the exception of the code for going after flags
// in CTF mode. This seems to have resulted in a slight speedup of the game
// (9 bots + me on a P200MMX, voodoo2, 64MB is a bit smoother than before and
// quite playable). The bots also seem quite a bit smarter now, since previously
// they would have assumed that certain items were available and gone after
// them instead of attacking. The bots are definitely more aggressive now.
// Its possible that redoing the route tables using the NIQ'd maps (i.e. with
// no items in them) would yield a further improvement, but a game with the
// bots at skill 1 seems very, very much like a game against real players on
// the internet, at least from my perspective.
//
// Bots may need added code to handle being close/far to target since some
// weapons can damage the bot when close and some weapons are ineffective
// from far away and there may be no other weapon (e.g. blaster) to switch
// to. Depends on whether the code for using each weapon is smart enough 
// already, e.g. GL code would have to know if health < x, and close, should 
// not fire or might kill self, or does the GL code assume that the earlier 
// call to the "choose best weapon" code will have switched to another weapon,
// e.g. the blaster if this is the case. Update: the current AI seems fine.
//
// Bots aren't going to be 'aware' that their health and ammo will increment
// automatically and aren't 'aware' that their health can only increase while
// they are not fighting. Anyway, its not obvious to me what the best strategy
// is here. The only clear case where a bot should run away is if its obvious
// a player has more health and is at least as skilled, but this is not 
// something one generally knows (without cheating), unless you actually see
// someone spawn.
//
// Bots obviously won't change their behaviour depending on the scoring being
// used since this feature isn't even in the regular game. e.g. if 
// niq_kildpts is set high, bots should try to not be killed, rather than 
// trying to get kills, or if niq_suicpts is high, should be careful about not
// blowing themselves up or jumping in lava etc.
//
// I *could* consider having bots which don't have a target as the current
// weapon is about to switch fire off a few random shots in the hope that
// they'll get a lucky frag, but this would be very difficult to do without
// having the bot frag itself with a grenade, or the BFG. Probably won't do
// and would rarely make a difference anyway.
// 
// For the time being (until I find a better solution, or if Ridah changes
// this), bots use the skill 1 hyperblaster ability at skill 2. Without
// this change, bot are much, much too strong with the HB at skill 2, and
// maybe even skill 1 (with a high accuracy rating).
// 
// Note that Eraser Bots 'cheat' in at least one way -- they use the internal
// knowledge of each client's health to pick the best target and to decide
// whether to run away (I think). This is information which a human client
// would not have -- he would have to guess as to the health of an opponent
// and would often have no clue.
//
////////////////////////////////
// Future Plans:
//
// Support some client-side commands (+password) for:
//	setting the number of bots
//  adding a bot by name (this is now possible)
//  dropping a bot by name (this might be possibe?)
//  changing the map
//  setting the fraglimit/timelimit/capturelimit/dmflags
//
// GENERAL:
//
// Look at Q2CTF+ code & figure out how to support CTF in this way on
// all maps by modifying the current code.
// 
// A fun option might be to support a damage factor for all weapons in
// niqweaps.txt. Probably not a lot of work. This would make it possible
// to play with a Q1-like rocket launcher, in terms of power. While I'm
// at it, it might be fun to change the firing delay as well, but I don't
// know if this can simply be changed without also changing the frames
// for the graphics? Also, add support in niqweaps.txt for specifying damage 
// and blast radius for all weapons.
//
// I tried to have NIQ play a unique sound so clients can hear when *they* 
// get a kill but I have yet to find a way to play a sound to one client 
// only. The best built-in sound that I found was gladiator/gldsrch1.wav
// but I think that if clients put any .wav in the file gldsrch1.wav in 
// baseq2\sound\gladiator, it would be used instead on the client's 
// machine.
///////////////////////////////
// DM/CTF:
//
// Add optional automatic handicapping (now that voluntary handicapping is 
// available). Each client's damage factor would be adjusted up or down 
// depending on his ping and/or skill. The tricky part would be to do this in
// a way that keeps as many people happy as possible. At this point I'm 
// leaning towards not implementing this as I don't think people will want
// to connect to a server which handicaps them. Also, ping times simply 
// aren't a reliable indicator of how difficult it will be to play on a 
// server -- I've had smoother connections at 300ms than at 100ms in some
// cases -- and pings can vary a lot for a given client.
//
// Any way to handle fact that clients who get a very negative score (likely
// if niq_kildpts is high) can just reconnect to start out at 0 again? This
// can happen in regular Quake II as well, e.g. if a client suicides several
// times, he will have a -ve score. Probably do nothing.
//
// Have the value of niq_game (serverinfo) indicate the type of game being
// played? (e.g. 1: NIQ dm, 2: NIQ ctf, 3: NIQ teamplay? 4: NIQ dm+bots, 
// 5: NIQ ctf+bots, 6: NIQ teamplay + bots, 11: regular dm, 12: regular
// CTF, 13: regular teamplay, 14: regular dm+bots, 15: regular CTF+bots, 
// 16: regular teamplay+bots). The last 6 values depend on whether I 
// support a switch for enabling/disabling NIQ.
//
// Add support for setting the (max) df for all clients to some value so
// all clients can be made more or less powerful. Probably useless if we
// add support for df for weapons themselves.
//
// Add a safety mode for a few seconds after respawning like the Lithium
// mod does? Less important in NIQ since you have a weapon but it would
// be especially good with bots since these seem to frag you the second
// you appear (they would need to 'understand' safety mode though).
//
///////////////////////////////
// BOTS:
//
// Eraser bots, at least in the code I have, are *way* to strong with the HB 
// or CG even at skill 2. Most other weapons aren't too bad, even at skill 3. 
// This may have been tweaked in a recent release. Maybe at skill 3, its OK 
// for the bots to slaughter any human players with the same weapon, but at 
// skill 2 they should play with combat skills that are no better than the 
// best human players. Currently, I can 'hit' a skill 2 Eraser Bot with the HB
// for maybe 0.5 secs and the bots can almost always turn around and kill me 
// instantly. I've changed this for now -- see above.
//
// Server can control whether certain bot commands can be given by clients. 
// This only supports botname at present (why not bot_name to be consistent
// with the server-side commands???). Might be nice to support more of these
// (bot_num) and to add (client and server) support for bot_num n taking 
// effect right away if n < old value (i.e. kill off old value - n bots).
// 
// Some way to specify that bots+clients=N would be nice. 
// 
// Change code to report # clients as bots+clients, but have to be careful not
// to show server as full if there are still client spots (show #bots+#clients
// /#bots+max?). I'm pretty sure this can't be changed through the gamex86 dll
// -- it probably only tracks clients, not server-side bots (could I 'fake'
// having clients e.g. add the same number of clients as bots but leave them
// suspended and drop/add one each time someone joins/exits. Sounds like a 
// lot of trouble -- servers can always put 'BOTS' in the server name and clients
// can always look at bot_num (add?) in the serverinfo information.
//
// It might be worth going over the bot code once more to look for other 
// areas of optimization.
// 
// Should make sure the following are disabled (so they aren't eating up
// CPU cycles) if necessary: quad freak, camper, favorite weapon.
// 
// Make green '*' after bots optional so server can be set up so that clients
// won't automatically know which other 'players' are really bots? This is a
// bit of a tricky issue -- I think that I personally would always rather 
// know that I'm up against a Bot -- which is why I'm leaving this out for 
// now.
//
////////////////////////////////
// SP/COOP:
//
// Have any POW killed by a monster cause client to lose points if points -ve.
//
// Have clients lose points for gibbing any POW corpses (accidently or if a 
// monster does this)? Might add a bit to the game. Careful about levels where
// POWs are being processed though since there is nothing clients can do about
// this.
//
// Scrap all secret areas since pretty hard to handle these (e.g. some are 
// triggered by walking into an area where there used to be an item). Just set
// secrets to 0/0 for all maps and open all triggered secret areas?
//
// When client dies in sp/coop show how/what killed him?
//
// Have points for killing a monster depend on what weapon was used. I doubt
// I'll do this.
/////////////////////////////////////////////////////////////////////////////
// Known Bugs:
// 
// Can't find pic (4x or more) with every weapon switch??? Lag-related? May
// only happen the 1st time a weapon is used? I've seen this with most other
// mods and regular Q2 as well.
//
// Can still get msg overflowed (not NIQ bug?). Can screw up console.
//
// Deathmatch (including CTF):
//
// Sometimes when you join the server seems to show more clients when connecting
// than there really are?
//
// CTF:
// 
// Singleplayer/coop:
// 
// Can still end up with -x in inventory?
//  
// jail5: floor under HB -- only one side drops?
// cool1: only 36/39 kills
// ware1: hidden berserks only triggered when client steps into area where 
//        there used to be an item (not obvious). Trigger automatically?
// mine4: screwed up drill bit graphics -- tunnel of mirrors effect / doesn't 
//        move save/restore only works some of the time.
//
// Save/restore still doesn't always work.
// 
/////////////////////////////////////////////////////////////////////////////
// Code notes:
// 
// Make sure you #define NIQ to compile NIQ stuff in all files. Also,
// #define EBOTS and link with the appropriate navlib library to have
// Eraser Bots support included. (Update: if you really want to build
// without NIQ code, and I'm not sure why you would want to do this, I
// just realized that a lot of the #ifdef/#ifndef NIQ code was replaced 
// with if(niq_enable->value)/if(!niq_enable->value) ... so you will need
// to search for niq_enable and enclose this in #ifdef NIQ/#ifndef NIQ as
// necessary.
// 
// This mod supports (assuming NIQ and EBOTS are defined), deathmatch,
// CTF and teamplay (2 kinds: Eraser teamplay and ID skin/model teamplay)
// with or without (NIQ) items in the game and with or without Eraser
// Bots. There is no support for singleplayer/coop in this mod, at present
// the last version which supports this is 1.5 and this will have to be
// modified to use the new g_niq files. Also the grappling hook and player
// id are (theoretically) available in all modes and the visible weapons
// patch is supported.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// NIQ code starts here:

#include "g_local.h"
#include "m_player.h"
#include "g_map_mod.h"
#ifdef EBOTS
#include "bot_procs.h"
#endif

#ifdef OHOOK
#include "hook.h"
#endif//OHOOK

#include "g_niq.h"

/////////////////////////////////////////////////////////////////////////////

// BUNGLE Changed to 4 characters-long version strings
#define NIQ_VERSION_STRING              "1.95"  // Be sure this is ALWAYS 4 characters long!
                                                // You can use "2.1 " for example...

#define MAXNIQWEAPONS                    10

#define	CHAN_HLTH						 5 // seems to work fine (can go up to 7 for sound channels?)

#define BOTCHARG						 ((char)(128+'*'))
#define BOTCHARW						 ('*')

#define NIQ_HUD_0						 0
#define NIQ_HUD_1						 1
#define NIQ_HUD_2						 2
#define NIQ_HUD_3						 3
#define NIQ_HUD_4						 4
#define NIQ_MAXHUD						 4

#define NIQ_SCOREBOARD_YOFFSET          20
#define NIQ_SCOREBOARD_FIRST_LINE		 4
#define NIQ_SB_HELP_OFFSET				60

#define NIQ_SCOREBOARD_LINESIZE         10
#define NIQ_SCOREBOARD_CHAR_XSIZE        4
#define NIQ_DEFMAXSCORELINES            12
#define NIQ_SBCHARS320                  40
#define NIQ_HUD_YOFFSET					40
#define NUM_SBHUDNAMECHARS				4
#define NIQ_DEFINFOSTARTLINE			4
#define NIQ_DEFINFOENDLINE			    20

#define NIQ_SB_320A                      0 
#define NIQ_SB_320B                      1 
#define NIQ_SB_320C                      2 
#define NIQ_SB_320D                      3 
#define NIQ_SB_320E                      4 
#define NIQ_SB_512A                      5 
#define NIQ_SB_640A                      6 
#define NIQ_SB_DM                        7 
#define NIQ_SB_CTF                       8
#define NIQ_MAXSCOREBOARD                8

#define MAXLINESIZE                     256
#define NIQMAXTRIGGERS                  100
#define MAXTARGETLEN                    32
#define NIQ_MAXSBCHARS					1400
#define NIQ_MAXHUDCHARS					1024
#define NIQMAXHELPSIZE                  1400

#define NIQ_MINDAMAGE                   0.1
#define NIQ_MAXDAMAGE                  10.0

#define NIQ_DEFAULTPATHNIQ              ".\\niq\\"
#define NIQ_DEFAULTPATHCTF              ".\\ctf\\"

#define NIQ_WEAPFILE                    "niqweaps.txt"
#define NIQ_AMMOFILE                    "niqammo.txt"
#define NIQ_MONSTFILE                   "niqmonst.txt"
#define NIQ_MOTDFILE                    "niqmotd.txt"

#define NIQ_MAXMOTDLINES				4

char motdlines[NIQ_MAXMOTDLINES][NIQ_SBCHARS320+1] =
	{ "", "", "", "" };

// defaults for user-configurable settings
#define NIQ_DEF_VERSION                 NIQ_VERSION_STRING

#define NIQ_DEF_ENABLE				    "1"		// set to 0 to disable NIQ'ing the level
#define NIQ_DEF_EBOTS 				    "0"		// set to 1 to enable adding Eraser Bots
#define NIQ_DEF_HANDICAP                "0"
#define NIQ_DEF_ALLMAPS                 "1"
#define NIQ_DEF_TRACTOR					"1"     
#define NIQ_DEF_HOOKSKY					"0"

#define NIQ_DEF_WEAPRAND                "1"
#define NIQ_DEF_WEAPALL                 "1"

#define NIQ_DEF_SNDHLTH				    "0"
#define NIQ_DEF_SNDWARN                 "1"
#define NIQ_DEF_SNDSWITCH               "1"
#define NIQ_DEF_MSGSWITCH               "0"

#define NIQ_DEF_SBHP                    "999"
#define NIQ_DEF_SBLINES					"12"  

#define NIQ_DEF_WEAPSECS                "60"  
#define NIQ_DEF_HLTHINC                 "10"  
#define NIQ_DEF_HLTHMAX                 "100" 

#define NIQ_DEF_KILLPTS                 "1.0" 
#define NIQ_DEF_KILDPTS                 "0.2"
#define NIQ_DEF_SUICPTS                 "1.0"

#define NIQ_DEF_AUTO                    "1"   
#define NIQ_DEF_WEAPKILLS               "5"   
#define NIQ_DEF_GRAPPLE                 "0"

// BUNGLE Standard Logging Support
#define NIQ_DEF_LOGFILE                 "0"     // No logging by default
// BUNGLE

#ifdef _DEBUG
#define NIQ_DEF_SBDEBUG					"1"
#else
#define NIQ_DEF_SBDEBUG					"0"
#endif
#define NIQ_DEF_SBWIDE					"1"
#define NIQ_DEF_SBMINI					"1"
#define NIQ_DEF_INTTIME					"10.0"	// after this time server goes to next level even if no client hits a key
#define NIQ_DEF_PLAYERID				"1"

static int	 g_nSBLineNum				= 0;
static char* g_helpptr					= NULL;
static char  g_helpstr[NIQMAXHELPSIZE];

// these will all get reset when a new game is started or a saved game is loaded
float niq_prevwsecs = -1.0;

#ifdef NIQSP
// vars for holding trigger information
int   niq_nNumTriggers = 0;
char  niq_nTriggers[NIQMAXTRIGGERS][MAXTARGETLEN];
#endif

// latched
cvar_t *niq_enable;
cvar_t *niq_ebots;
cvar_t *niq_handicap;
cvar_t *niq_allmaps;
cvar_t *niq_tractor;
cvar_t *niq_hooksky;

cvar_t *niq_blk1;
cvar_t *niq_blk2;

// BUNGLE Standard Logging Support
cvar_t *niq_logfile;
// BUNGLE

// write-protected
cvar_t *niq_version;
//TBD: cvar_t *niq_game;

// these show up with serverinfo -- may make latched
cvar_t *niq_sbhp;
cvar_t *niq_sblines;
cvar_t *niq_weapsecs;
cvar_t *niq_hlthinc;
cvar_t *niq_hlthmax;
cvar_t *niq_killpts;
cvar_t *niq_kildpts;
cvar_t *niq_suicpts;
cvar_t *niq_auto;
cvar_t *niq_weapkills;
cvar_t *niq_sbdebug;
cvar_t *niq_sbwide;
cvar_t *niq_sbmini;
cvar_t *niq_inttime;
cvar_t *niq_playerid;
cvar_t *niq_weaprand;
cvar_t *niq_weapall;

cvar_t *niq_sndhlth;
cvar_t *niq_sndwarn;
cvar_t *niq_sndswitch;
cvar_t *niq_msgswitch;

// strings containing version number which are modified on startup
// BUNGLE Changed x.x for x.xx
									 //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
char 	szWelcomeStrNIQ[]			=      "Welcome to NIQ x.xx for Quake II";
char 	szWelcomeStr1NIQ[]			=   "This is NIQ Version x.xx for Quake II";
char 	szWelcomeStr2NIQ[]			=    "NIQ code: Mike (Artful Dodger) Fox";
char 	szWelcomeStr3NIQ[]			=      "Bot code: Ryan (Ridah) Feltrin";
char 	szWelcomeStr4NIQ[]			=       "Other: See the NIQ web site.";

char	szNIQCTFMENUStr1[]			= "*Quake II (NIQ x.xx)";
char	szNIQCTFMENUStr2[]			= "*(NIQ x.xx by Mike Fox)";

char    szNIQTitle_NIQ[]			= "NIQ x.xx";

char	szContact1[]				= "Check out the official NIQ home page at:";
char	szContact2[]				=        "www.planetquake.com/niq";
char	szContact3[]				=     "Send Comments/Suggestions/Bugs to:";
char	szContact4[]				=            "mfox@legendent.com";

char	szPrompt0[]					=   "You are currently in observer mode.";
char	szPrompt1[]					= "Use [ or ] to change screens, Esc exits.";
char	szBlank1[]					=              "(observing)";
char	szBlank2[]					=              "Use [ ] Esc";

/////////////////////////////////////////////////////////////////////////////
// Titles for various scoreboards. If in NIQCTF and ctf=1, we start out with
// the usual CTF scoreboard by default, otherwise, we start out with one of 
// the customized NIQ scoreboards.

// 320xXXX scoreboards                 1234567890123456789012345678901234567890
char    szNIQTitle_320A[]			= "Name          KLS KLD SUI PING PPH SCORE";
//									   1234567890123 xxx xxx xxx xxxx xxx xxx.x

char    szNIQTitle_320B[]			= "Name     KLS KLD SUI TIME PING PPH SCORE";
//									   12345678 xxx xxx xxx xxxx xxxx xxx xxx.x

char    szNIQTitle_320C[]			= "Name            TIME PING PPH RANK SCORE";
//									   123456789012345 xxxx xxxx xxx xxxx xxx.x

char    szNIQTitle_320D[]			= "Name          DFAVG DFCUR PING PPH SCORE";
//									   1234567890123 xxx.x xxx.x xxxx xxx xxx.x

char    szNIQTitle_320E[]			= "Name    DFAV DFCU KLS KLD SUI PING SCORE";
//									   1234567 xx.x xx.x xxx xxx xxx xxxx xxx.x

// 512xXXX scoreboards                 1234567890123456789012345678901234567890123456789012345678901234
char    szNIQTitle_512A[]			= "Name            DFAVG DFCUR KLS KLD SUI TIME PING PPH RANK SCORE";
//									   123456789012345 xxx.x xxx.x xxx xxx xxx xxxx xxxx xxx xxxx xxx.x

// 640xXXX: mostly used for debugging  12345678901234567890123456789012345678901234567890123456789012345678901234567890
char    szNIQTitle_640A[]			= "Name            WEAP HLTH AMMO  DFAVG DFCUR KLS KLD SUI TIME PING PPH RANK SCORE";
//									   123456789012345 xxxx xxxx xxxx  xxx.x xxx.x xxx xxx xxx xxxx xxxx xxx xxxx xxx.x

char    szSBHelp[]					= "(type nhelp for help)";

char	szHUDLabelStr[]				= "xr -42 yt 2 string2 \"SCORE\" xr -26 yt 42 string2 \"PPH\" xr -34 yt 82 string2 \"RANK\"";

// needed prototypes:
char*		ED_ParseEdict (char *data, edict_t *ent);
void		Cmd_Inven_f   (edict_t *ent);
void		Cmd_PutAway_f (edict_t *ent);
void		Cmd_Use_f     (edict_t *ent);
void		CTFSetIDView(edict_t *ent);

#ifdef EBOTS
qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
int			ClosestNodeToEnt(edict_t *self, int check_fullbox, int check_all_nodes);
void		botSetWant(edict_t	*self, int dist_divide);
float		PathToEnt(edict_t *self, edict_t *target, int check_fullbox, int check_all_nodes);
#endif

int			num_view_weapons = 0;
char		view_weapon_models[64][64];

/////////////////////////////////////////////////////////////////////////////

typedef struct niqitem_s
{
	char*		szName;                     // weapon name                    
    char*       szAmmo;                     // weapon ammo                    

    int         nAmmoInitial;               // amount of ammo on switch       
    int         nAmmoIncrement;             // amount of ammo given per second   
    int         nAmmoSecs;                  // secs to wait before adding ammo
    int         nAmmoMax;                   // max amount of ammo             

    int         nListIndexWeapon;           // itemlist index for weapon (set in NIQInit)
    int         nListIndexAmmo;             // itemlist index for ammo (set in NIQInit)

    char*       szMessage;                  // switch message
} niqitem_t;

/////////////////////////////////////////////////////////////////////////////
// Note that the current weapon index (1-10) always corresponds to the 
// standard weapons in order (blaster through bfg10k). This is still the case 
// even if the user bans certain weapons (e.g. the bfg) and/or specifies a 
// specific order in which case the NIQ-determined current weapon is converted
// to the standard weapon index 1-10.

#define NIQLISTITEMS (MAXNIQWEAPONS+2)
niqitem_t niqlist[NIQLISTITEMS] = 
{
	{
		NULL
	},	// leave index 0 alone

	{//1
		"blaster",                  // doesn't use any ammo!
        "",                
        0,
        0,
        0,
        0,
        999,                     
        999,                     
        "switching to super blaster"        
	},

	{//2
		"shotgun",              
        "shells",               
        10,                     
        1,                     
        2,
        50,                    
        999,                     
        999,                     
        "switching to shotgun"        
	},

	{//3
		"super shotgun",         
        "shells",               
        12,                     
        1,                     
        1,
        50,                    
        999,                     
        999,                     
        "switching to super shotgun"        
	},

	{//4
		"machinegun",           
        "bullets",              
        50,                     
        5,                     
        1,
        200,                    
        999,                     
        999,                     
        "switching to machinegun"        
	},

	{//5
		"chaingun",             
        "bullets",              
        100,                     
        10,
        1,                     
        200,                    
        999,                     
        999,                     
        "switching to chaingun"        
	},

	{//6
		"grenade launcher",     
        "grenades",             
        10,                     
        1,                     
        2,
        50,                    
        999,                     
        999,                     
        "switching to grenade launcher"        
	},

	{//7
		"rocket launcher",      
        "rockets",              
        10,                     
        1,                     
        2,
        50,                    
        999,                     
        999,                     
        "switching to rocket launcher"        
	},

	{//8
		"hyperblaster",         
        "cells",                
        100,                     
        4,                     
        1,
        200,                    
        999,                     
        999,                     
        "switching to hyperblaster"        
	},

	{//9
		"railgun",              
        "slugs",                
        10,                     
        1,                     
        3,
        25,                    
        999,                     
        999,                     
        "switching to railgun"        
	},

	{//10
		"bfg10k",     
        "cells",      
        100,           
        5,           
        1,
        200,          
        999,           
        999,                     
        "switching to bfg10k"        
	},

	// end of list marker (TBD)
	{NULL}
};

// move into g_niq.h?
// time that weapon remains in firing state after being fired (seconds) (applies to bots)
float niq_weap_fire_times[MAXNIQWEAPONS] = 
	{
	0.8, // BLASTER			
	1.8, // SHOTGUN			
	1.7, // SSHOTGUN			
	0.5, // MACHINEGUN		
	3.1, // CHAINGUN			
	1.6, // GRENADELAUNCHER	
	1.2, // ROCKETLAUNCHER	
	2.0, // HYPERBLASTER		
	1.8, // RAILGUN			
	3.2, // BFG				
	};

#ifdef NIQSP

typedef struct niqmonst_s
{
	char*		szName;         // monster name
    int         nHealthInc;     // health to give
} niqmonst_t;

// note that the insane POWs are included here in case someone wants to give/take away health for killing them
niqmonst_t niq_monsters[] =
{
    {"",                       -999},
    {"misc_insane",               0}, // crazy POW 
    {"monster_berserk",           5}, // tall dude with sword 
    {"monster_boss2",            40}, // big bug like thing
    {"monster_boss3_stand",     999}, // ??? is this in the game?
    {"monster_brain",            30}, // wimpy monsters with hooks, shields
    {"monster_chick",            10}, // rocket-toting chick
    {"monster_flipper",           2}, // wimpy pirahna 
    {"monster_floater",          10}, // floating jar
    {"monster_flyer",            10}, // flying boomerang
    {"monster_gladiator",        15}, // railgun dude
    {"monster_gunner",            5}, // headless machinegun dude
    {"monster_hover",            10}, // flying gear/cross
    {"monster_infantry",         10}, // tall dude with GL and MG
    {"monster_jorg",             40}, // strogg leader part I
    {"monster_medic",             5}, // wimpy medic
    {"monster_mutant",            5}, // big dog-like thing
    {"monster_parasite",         20}, // thing with real long tongue
    {"monster_soldier_light",     2}, // soldier with wimpy blaster
    {"monster_soldier_ss",        5}, // SS soldier
    {"monster_soldier",           5}, // soldier with machine gun (keep soldiers in this order)
    {"monster_supertank",        30}, // huge rolling tank
    {"monster_tank_commander",   20}, // big dude with many weapons
    {"monster_tank",            999}, // ??? is this in the game?
    {"turret_driver",            20}, // turret driver
    {"",                       -999},
};
#endif

#ifdef NIQSP
char *niq_single_statusbar = 
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor (niq: current weapon icon and time remaining or kills needed for switch)
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
"	xv	262 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
;
#endif

char *niq_dm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	124 " // 100
"	anum "
"	xv	174 " // 150
"	pic 2 "
"endif "

// armor (niq: current weapon icon and time remaining or kills needed for switch)
"if 4 "
"	xv	244 " // 200
"	rnum "
"	xv	294 " // 250
"	pic 4 "
"endif "

// id view state (niq: dropped "Viewing " and added to all modes)
"if 27 "
  "xv 0 "
  "yb -58 "
  "stat_string 27 "
"endif "

//  frags
"if 31 "
"xr	-50 "
"yt 12 "
"num 3 14"
"endif "

// niq: PPH
"if 31 "
"xr	-50 "
"yt 52 "
"num 3 29"
"endif "

// niq: rank
"if 31 "
"xr	-50 "
"yt 92 "
"num 3 30"
"endif "
;

char *niq_ctf_statusbar =

"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	124 " // 100
"	anum "
"	xv	174 " // 150
"	pic 2 "
"endif "

// armor (niq: current weapon icon and time remaining or kills needed for switch)
"if 4 "
"	xv	244 " // 200
"	rnum "
"	xv	294 " // 250
"	pic 4 "
"endif "

// have flag graph (niq: put just below team status info)
"if 21 "
  "yb -50 " // -24
  "xr -26 "
  "pic 21 "
"endif "

"yb	-50 "

// picked up item (niq: needed in ctf to show flag briefly when picked up)
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// red team
"yb -102 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "

//joined overlay
"if 22 "
  "yb -104 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -75 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "
"if 23 "
  "yb -77 "
  "xr -28 "
  "pic 23 "
"endif "

// id view state (niq: dropped "Viewing ")
"if 27 "
  "xv 0 "
  "yb -58 "
  "stat_string 27 "
"endif "

//  frags
"if 31 "
"xr	-50 "
"yt 12 "
"num 3 14"
"endif "

// niq: PPH
"if 31 "
"xr	-50 "
"yt 52 "
"num 3 29"
"endif "

// niq: rank
"if 31 "
"xr	-50 "
"yt 92 "
"num 3 30"
"endif "
;

/////////////////////////////////////////////////////////////////////////////

void niq_patchversionstring(char* szStr)
{
    char* pVersion;

	pVersion = strstr(szStr, "x.xx");   // BUNGLE Changed x.x for x.xx
	if(!pVersion)
		gi.dprintf("x.xx not found in version string\n");

#ifdef _DEBUG
	if(strlen(NIQ_VERSION_STRING) != 4)
		gi.dprintf("NIQ: version string is wrong length!\n");
#endif

	// assumes version string is 3 chars long
	strncpy(pVersion, NIQ_VERSION_STRING, 4);
} // niq_patchversionstring

/////////////////////////////////////////////////////////////////////////////
// Quick hack for now: there *is* a better way.

void niq_patchversionstrings()
{
	niq_patchversionstring(szNIQTitle_NIQ);
	niq_patchversionstring(szWelcomeStrNIQ);
	niq_patchversionstring(szWelcomeStr1NIQ);
	niq_patchversionstring(szNIQCTFMENUStr1);
	niq_patchversionstring(szNIQCTFMENUStr2);
} // niq_patchversionstrings

/////////////////////////////////////////////////////////////////////////////
// niq_showmessagetoallclients
//
// There *must* be a function to do this somewhere but I haven't found it yet.

void niq_showmessagetoallclients(char* szMsg, qboolean bHigh)
{
    int i;
	for (i=1; i<=game.maxclients; i++)
        {
    	edict_t* ent = g_edicts + i;
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;
		if(ent->client->pers.niq_isbot)
			continue;
		if(ent->svflags & SVF_NOCLIENT)
			continue;	// don't show switch messages if in observer mode
		if(ent->client->resp.niq_helpmode)
			continue;	// don't show switch messages if client has a menu up (for now do show them if a scoreboard is up)

        if(bHigh)
    		gi.cprintf (ent, PRINT_HIGH, "%s\n", szMsg);
        else
            gi.centerprintf(ent, "%s\n", szMsg);
    	}
} // niq_showmessagetoallclients

/////////////////////////////////////////////////////////////////////////////
// niq_selectcurrentweapon:
//
// In random mode, we normally make sure we don't select the same weapon 2x
// in a row, unless there is only 1 weapon available or the weapon list file
// specified at least one weapon 2 or more times.

void niq_selectcurrentweapon(qboolean bInit)
{
    if(game.niq_numweapons==1)
        {
        // only 1 weapon to choose from
    	game.niq_curweapon = game.niq_weaponsindx[1];
        }
    else if(niq_weaprand->value)
        {
        int nNewWeapon;

        if(niq_weapall->value)
            {
            // use all weapons in list once randomly (could get same one consecutively if in list more than once)
            qboolean bAnyUnused = false;
            int ii;

            // first see if we need to restart
            for(ii=1; ii<=game.niq_numweapons && !bAnyUnused; ii++)
                if(game.niq_weaponsused[ii] == 0)
                    bAnyUnused = true;

            if(!bAnyUnused)
                {
                // clear the used flag for all weapons the pick a new index at random
                for(ii=1; ii<=game.niq_numweapons; ii++)
                    game.niq_weaponsused[ii] = 0;

                do
                    {
                    nNewWeapon = (rand()%game.niq_numweapons) + 1;
                    }
                while(nNewWeapon == game.niq_wlindex);
                }
            else
                {            
                // pick any weapon at random
                nNewWeapon = (rand()%game.niq_numweapons) + 1;

                // scan until weapon is unused (we know there is at least one)
                while(game.niq_weaponsused[nNewWeapon] != 0)
                    {
                    nNewWeapon++;
                    if(nNewWeapon > game.niq_numweapons)
                        nNewWeapon = 1;
                    }
                }

            game.niq_weaponsused[nNewWeapon] = 1;
            }
        else
            {
            // pick any new index randomly
            do
                {
                nNewWeapon = (rand()%game.niq_numweapons) + 1;
                }
            while(nNewWeapon == game.niq_wlindex);
            }

        // we could still select the same weapon 2x if some weapons are in the list 2x or more
        game.niq_curweapon = game.niq_weaponsindx[nNewWeapon];
        game.niq_wlindex   = nNewWeapon;
        }
    else if(bInit)
        {
        // take first weapon in list initially
    	game.niq_curweapon = game.niq_weaponsindx[1];
        game.niq_wlindex   = 1;
        }
    else
        {
        game.niq_wlindex++;
    	if(game.niq_wlindex > game.niq_numweapons)
    	    game.niq_wlindex = 1;

       	game.niq_curweapon = game.niq_weaponsindx[game.niq_wlindex];
        }
} // niq_selectcurrentweapon

/////////////////////////////////////////////////////////////////////////////
// niq_setweapontime:
//
// Sets the actual timer and keeps track of the last niq_wsecs value
// used so that we can reset the weapon timer if this changes.

void niq_setweapontime(float fTime)
{
    game.niq_wsecs = fTime;
    niq_prevwsecs  = niq_weapsecs->value;
} // niq_setweapontime

/////////////////////////////////////////////////////////////////////////////

void niq_setweapontimer()
{
    if(game.niq_wsecs == -1)
        {
        niq_setweapontime(-1);
        return; // weapons change when map changes
        }

    // reset weapon switch timer but keep current time remaining if any
    if(game.niq_wsecs > level.time)
        {
        niq_setweapontime(game.niq_wsecs - level.time);

        // idiot check against having timer set to high for some reason
        if(game.niq_wsecs > niq_weapsecs->value)
            niq_setweapontime(niq_weapsecs->value);
        }
    else
        niq_setweapontime(niq_weapsecs->value);

    // the level time will be cleared rigth after calling this so we don't need to add level.time
} // niq_setweapontimer

/////////////////////////////////////////////////////////////////////////////

gitem_t* niq_getcurrentweapon()
{
	return FindItem(niqlist[game.niq_curweapon].szName);
} // niq_getcurrentweapon

/////////////////////////////////////////////////////////////////////////////
// niq_incrementhealth:
//
// Each client maintains his own health timer?

void niq_incrementhealth(edict_t *ent, gclient_t* client)
{
	qboolean noise = false;
    qboolean bDrowning;
    qboolean bInLava;
    qboolean bInSlime;

    // health can't increase while drowning etc.
    bDrowning = (ent->waterlevel >= 3 && ent->air_finished < level.time);
    bInLava   = (ent->watertype & CONTENTS_LAVA);
    bInSlime  = (ent->watertype & CONTENTS_SLIME);

	// in case hlthmax is reduced during a game
	if (ent->health > niq_hlthmax->value)
		ent->health = niq_hlthmax->value;

#if 0
	// test for bots
    if(bDrowning || bInLava || bInSlime)
		{
		if(client->pers.niq_isbot)
			gi.dprintf("Bot is drowning/burning/sliming\n");
		else
			gi.dprintf("Hum is drowning/burning/sliming\n");
        return;
		}
#else
    if(bDrowning || bInLava || bInSlime)
        return;
#endif

    if (ent->health < niq_hlthmax->value)
        {
		noise = (niq_sndhlth->value);

        if(deathmatch->value)
            {
            // 28/03/98 niq: client health can't improve while fighting (firing weapon) in DM???
            qboolean bFighting = false;

			// could I use this test for all clients?
			if(client->pers.niq_isbot)
				{
				// test if bot player is actively fighting
				// we define 'bot is fighting' if weapon is still firing
				if((ent->last_fire + niq_weap_fire_times[game.niq_curweapon-1]) >= level.time)
					bFighting = true;
				}
			else
				{
				// test if human player is actively fighting -- defined as weapon is firing or client trying to fire weapon
				if(client->weaponstate == WEAPON_FIRING || ((client->latched_buttons|client->buttons) & BUTTON_ATTACK))
					bFighting = true;
				}

            if(!bFighting)
    			ent->health += niq_hlthinc->value;
            else
#if 0
				{
				// test for bots
				if(client->pers.niq_isbot)
					gi.dprintf("Bot is fighting -- health can't increase\n");
				else
					gi.dprintf("Hum is fighting -- health can't increase\n");

                noise = false;
				}
#else
                noise = false;
#endif
            }
        else
    		ent->health += niq_hlthinc->value;

		if (ent->health > niq_hlthmax->value)
			ent->health = niq_hlthmax->value;
		}

    // do health increment sound?
	if (noise)
		gi.sound(ent, CHAN_HLTH, gi.soundindex("items/s_health.wav"), 1.0, ATTN_NORM, 0);
} // niq_incrementhealth

/////////////////////////////////////////////////////////////////////////////

qboolean niq_has_enough_ammo(edict_t* ent)
{
	int nAmmo;

	if (!ent->client)
		return false;

	nAmmo = ent->client->pers.inventory[niqlist[game.niq_curweapon].nListIndexAmmo];

	if(game.niq_curweapon == 1)
		return true;			// blaster: doesn't use ammo
	if(game.niq_curweapon == 3)
		return (nAmmo >= 2);	// SSG: 2 shells
	if(game.niq_curweapon == 10)
		return (nAmmo >= 50);	// BFG: 50 cells

	// all other weapons only require 1 unit of ammo
	return (nAmmo >= 1);
} // niq_has_enough_ammo

/////////////////////////////////////////////////////////////////////////////
// niq_set_ammo:
//
// TBD: change this and Add_Ammo to use maximums given above (which might
// differ for different weapons).

static void niq_set_ammo(gclient_t* cl, int count, int nWeap)
{
	if (!cl) 
	{ 
#ifdef _DEBUG 
		gi.dprintf("niq_set_ammo: null client\n"); 
#endif
		return; 
	}

	// long-time bug fix?: if count is 0, we are clearing the ammo which we always have to do
    // if user has cheated, don't reset ammo until back down to normal levels
	if((count != 0) && (cl->pers.inventory[niqlist[nWeap].nListIndexAmmo] > niqlist[nWeap].nAmmoMax))
        return;

	if (count > niqlist[nWeap].nAmmoMax)
		count = niqlist[nWeap].nAmmoMax;

	cl->pers.inventory[niqlist[nWeap].nListIndexAmmo] = count;
} // niq_set_ammo

/////////////////////////////////////////////////////////////////////////////
// niq_add_ammo:
//
// TBD: change this and Add_Ammo to use maximums given above (which might
// differ for different weapons).

qboolean niq_add_ammo(gclient_t* client, int count, int nWeap)
{
    // if user has cheated, don't reset ammo until back down to normal levels
 	if(client->pers.inventory[niqlist[nWeap].nListIndexAmmo] >= niqlist[nWeap].nAmmoMax)
        return true;

	count += client->pers.inventory[niqlist[nWeap].nListIndexAmmo];

	if (count > niqlist[nWeap].nAmmoMax)
		count = niqlist[nWeap].nAmmoMax;

	client->pers.inventory[niqlist[nWeap].nListIndexAmmo] = count;

	return true;
} // niq_add_ammo

/////////////////////////////////////////////////////////////////////////////
// niq_incrementammo:
//
// Each client maitains its own ammo timer?

void niq_incrementammo(gclient_t* client)
{
    client->niq_ammotics++;

    if (client->niq_ammotics >= niqlist[game.niq_curweapon].nAmmoSecs) 
        {
        client->niq_ammotics = 0;

        // nothing to do for blaster, doesn't use any ammo
        if(game.niq_curweapon != 1)
            {
           	int nAmmoIncrease   = niqlist[game.niq_curweapon].nAmmoIncrement;
        	int nAmmoIndex      = niqlist[game.niq_curweapon].nListIndexAmmo;

        	if(nAmmoIncrease <= 0)
		    	{
#ifdef _DEBUG
	    	    gi.dprintf ("NIQ: invalid ammo increase amount!\n");
#endif
    	    	return;							
			    }

        	if(nAmmoIndex <= 0)
		    	{
#ifdef _DEBUG
	    	    gi.dprintf ("NIQ: invalid ammo index!\n");
#endif
    	    	return;							
			    }

    	    if(nAmmoIndex != 999)
                {
        		// goose up ammo (niq_add_ammo enforces limits)
   	        	niq_add_ammo(client, nAmmoIncrease, game.niq_curweapon);
                }
            else
				{
#ifdef _DEBUG
    		    gi.dprintf ("NIQ: invalid ammo index!\n"); // only the super blaster has no ammo
#endif
				}
            }
        }
} // niq_incrementammo

/////////////////////////////////////////////////////////////////////////////
// niq_trackdfactor:
//
// niq1.3: the dfactor which we show in the scoreboard is now averaged out
// over the active connected time of each client. This prevents cheating by
// playing with a high df then switching to a low one just before the end of
// the level so that you look better than you are.

void niq_trackdfactor(gclient_t* client)
{
	client->resp.niq_fDamageTotal += client->pers.niq_fDamage;
	client->resp.niq_fDamageTics++;
} // niq_trackdfactor

/////////////////////////////////////////////////////////////////////////////

void niq_settimers(gclient_t* client)
{
    if(!client)
        return;

	client->niq_sectimer		= level.time + NIQ_ONESECOND;
	client->niq_ammotics		= 0;                               
} // niq_settimers

/////////////////////////////////////////////////////////////////////////////

void niq_CTFScoreboardMessage(edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];

	int		total[2];

	float	sortedscores[2][MAX_CLIENTS];
	float	score, totalscore[2];
	int		iScore[2];

	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	// sort the clients by team and score
	total[0] = total[1] = 0;
	last[0] = last[1] = 0;
	totalscore[0] = totalscore[1] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			team = 0;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			team = 1;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		sortedscores[team][j] = score;
		totalscore[team] += score;
		total[team]++;
	}

	// print level name and exit rules
	// add the clients in sorted order
	*string = 0;
	len = 0;

	if(totalscore[0] >= 0.0)
		iScore[0] = (int)(totalscore[0] + 0.5);
	else
		iScore[0] = (int)(totalscore[0] - 0.5);

	if(totalscore[1] >= 0.0)
		iScore[1] = (int)(totalscore[1] + 0.5);
	else
		iScore[1] = (int)(totalscore[1] - 0.5);

	sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
		"xv 40 yv 28 string \"%4d/%-3d\" "
		"xv 98 yv 12 num 2 18 "
		"if 25 xv 168 yv 8 pic 25 endif "
		"xv 200 yv 28 string \"%4d/%-3d\" "
		"xv 256 yv 12 num 2 20 ",
		iScore[0], total[0],
		iScore[1], total[1]);

	len = strlen(string);

	for (i=0 ; i<16 ; i++)
	{
		if (i >= total[0] && i >= total[1])
			break; // we're done

		*entry = 0;

		// left side
		if (i < total[0]) {
			cl = &game.clients[sorted[0][i]];
			cl_ent = g_edicts + 1 + sorted[0][i];

			if(cl->resp.score >= 0.0)
				iScore[0] = (int)(cl->resp.score + 0.5);
			else
				iScore[0] = (int)(cl->resp.score - 0.5);

			sprintf(entry+strlen(entry),
				"ctf 0 %d %d %d %d ",
				42 + i * 8,
				sorted[0][i],
				iScore[0],
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
				sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",
					42 + i * 8);

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[0] = i;
			}
		}

		// right side
		if (i < total[1]) {
			cl = &game.clients[sorted[1][i]];
			cl_ent = g_edicts + 1 + sorted[1][i];

			if(cl->resp.score >= 0.0)
				iScore[0] = (int)(cl->resp.score + 0.5);
			else
				iScore[0] = (int)(cl->resp.score - 0.5);

			sprintf(entry+strlen(entry),
				"ctf 160 %d %d %d %d ",
				42 + i * 8,
				sorted[1][i],
				iScore[0],
				cl->ping > 999 ? 999 : cl->ping);

			if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
				sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
				last[1] = i;
			}
		}
	}

	// put in spectators if we have enough room
	if (last[0] > last[1])
		j = last[0];
	else
		j = last[1];
	j = (j + 2) * 8 + 42;

	k = n = 0;
	if (maxsize - len > 50) {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			cl = &game.clients[i];
			if (!cl_ent->inuse ||
				cl_ent->solid != SOLID_NOT ||
				cl_ent->client->resp.ctf_team != CTF_NOTEAM)
				continue;

			if (!k) {
				k = 1;
				sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
				strcat(string, entry);
				len = strlen(string);
				j += 8;
			}

			if(cl->resp.score >= 0.0)
				iScore[0] = (int)(cl->resp.score + 0.5);
			else
				iScore[0] = (int)(cl->resp.score - 0.5);

			sprintf(entry+strlen(entry),
				"ctf %d %d %d %d %d ",
				(n & 1) ? 160 : 0, // x
				j, // y
				i, // playernum
				iScore[0],
				cl->ping > 999 ? 999 : cl->ping);

			if (maxsize - len > strlen(entry)) {
				strcat(string, entry);
				len = strlen(string);
			}
			
			if (n & 1)
				j += 8;
			n++;
		}
	}

	if (total[0] - last[0] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
	if (total[1] - last[1] > 1) // couldn't fit everyone
		sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
} // niq_CTFScoreboardMessage

/////////////////////////////////////////////////////////////////////////////

static void niq_addcenteredsbline(char* strdst, char* strsrc, int nLine)
{
	sprintf(strdst, "xv %d yv %d string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-strlen(strsrc)), nLine*NIQ_SCOREBOARD_LINESIZE, strsrc);
}

/////////////////////////////////////////////////////////////////////////////

void niq_sb_addmaintitles(char* szStrDst, int nMode)
{
    char	szTemp1[200] = "";
    char	szTemp2[200] = "";
	int		nClients = niq_getnumclients();

//	sprintf(szTemp1, "%s (%d)", szNIQTitle_NIQ, nClients);

//	niq_addcenteredsbline(szTemp2, szTemp1, NIQ_SCOREBOARD_FIRST_LINE);

	if     (nMode == NIQ_SB_320A)
		niq_addcenteredsbline(szTemp1, szNIQTitle_320A, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_320B)
		niq_addcenteredsbline(szTemp1, szNIQTitle_320B, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_320C)
		niq_addcenteredsbline(szTemp1, szNIQTitle_320C, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_320D)
		niq_addcenteredsbline(szTemp1, szNIQTitle_320D, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_320E)
		niq_addcenteredsbline(szTemp1, szNIQTitle_320E, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_512A)
		niq_addcenteredsbline(szTemp1, szNIQTitle_512A, NIQ_SCOREBOARD_FIRST_LINE);
	else if(nMode == NIQ_SB_640A)
		niq_addcenteredsbline(szTemp1, szNIQTitle_640A, NIQ_SCOREBOARD_FIRST_LINE);
	else
		gi.dprintf("NIQ: invalid scoreboard setting.\n");

	strcat(szTemp2,  szTemp1);
	strcat(szStrDst, szTemp2);
} // niq_sb_addmaintitles

/////////////////////////////////////////////////////////////////////////////

float niq_getaverageDF(gclient_t* cl)
{
	if(cl->resp.niq_fDamageTics)
		return (cl->resp.niq_fDamageTotal / cl->resp.niq_fDamageTics);
	else
		return cl->pers.niq_fDamage;
} // niq_getaverageDF

/////////////////////////////////////////////////////////////////////////////

int niq_getPPH(gclient_t* cl)
{
	float fCLTimeSecs = (level.framenum - cl->resp.enterframe)/10.0;
	float fPPH;

	if(fCLTimeSecs > 0.0)
		fPPH = (cl->resp.score * 3600.0/fCLTimeSecs);
	else
		fPPH = cl->resp.score;

	// we never show an FPH of > 999 or < -99 for now (3 character field)
	if(fPPH >= 999.0)
		return 999;
	if(fPPH <= -99.0)
		return -99;

	if(fPPH >= 0.0)
		return (int)(fPPH + 0.5);
	else
		return (int)(fPPH - 0.5);
} // niq_getaverageDF

/////////////////////////////////////////////////////////////////////////////

void niq_getcurweapname(gclient_t* cl, char* pszDst)
{
	if ( cl->pers.inventory[ITEM_INDEX(item_bfg10k)] )
		strcpy(pszDst,"BFG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_railgun)] )
		strcpy(pszDst, "RG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_hyperblaster)] )
		strcpy(pszDst, "HB");
	else if ( cl->pers.inventory[ITEM_INDEX(item_rocketlauncher)] )
		strcpy(pszDst, "RL");
	else if ( cl->pers.inventory[ITEM_INDEX(item_grenadelauncher)] )
		strcpy(pszDst, "GL");
	else if ( cl->pers.inventory[ITEM_INDEX(item_chaingun)] )
		strcpy(pszDst, "CG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_machinegun)] )
		strcpy(pszDst, "MG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_supershotgun)] )
		strcpy(pszDst, "SSG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_shotgun)] )
		strcpy(pszDst, "SG");
	else if ( cl->pers.inventory[ITEM_INDEX(item_blaster)] )
		strcpy(pszDst, "SB");
	else
		strcpy(pszDst, "??");
} // niq_getcurweapname

/////////////////////////////////////////////////////////////////////////////

static void niq_getname(int nLenMax, gclient_t* cl, char* pszName, qboolean bGreen)
{
	strncpy(pszName, cl->pers.netname, 15);
	pszName[nLenMax] = '\0';

	// append a green letter to the end of Bot names so these can be identified
	if(cl->pers.niq_isbot)
		{
		int nLen = strlen(pszName);
		if(nLen < nLenMax)
			{
			// room to append BOTCHAR
			if(bGreen)
				pszName[nLen]	= (char)BOTCHARG;
			else
				pszName[nLen]	= (char)BOTCHARW;
			pszName[nLen+1]	= '\0';
			}
		else
			{
			// replace last char with BOTCHAR
			if(bGreen)
				pszName[nLen-1] = (char)BOTCHARG; 
			else
				pszName[nLen-1] = (char)BOTCHARW; 
			}
		}
} // niq_getname

/////////////////////////////////////////////////////////////////////////////

int niq_sb_addcliententry(int nCurLen, char* szSBStr, gclient_t* cl, int nMode, int nRank, int nOffY, qboolean bSelf)
{
    char szTemp1[200] = ""; 
    char szTemp2[200] = ""; 
	int  nOffX = 0;		// column offset for 320xXXX screen
	int  nLenNew;
	int  nLenMax;
	int  j;

	// make sure name is 10 or 15 chars max
	char szName[33];

	switch(nMode)
		{
		case NIQ_SB_320A:
			nLenMax = 13;
			break;

		case NIQ_SB_320B:
			nLenMax = 8;
			break;

		case NIQ_SB_320C:
			nLenMax = 15;
			break;

		case NIQ_SB_320D:
			nLenMax = 13;
			break;

		case NIQ_SB_320E:
			nLenMax = 7;
			break;

		default:
			nLenMax = 15;
		}

	niq_getname(nLenMax, cl, szName, true);

	if      (nMode == NIQ_SB_320A)
		{
		// Name          KLS KLD SUI PING PPH SCORE
		// 1234567890123 xxx xxx xxx xxxx xxx xxx.x
        sprintf(szTemp1, "%-13s %3d %3d %3d %4d %3d %5.1f", szName, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, cl->ping, niq_getPPH(cl), cl->resp.score);
		}
	else if (nMode == NIQ_SB_320B)
		{
		// Name     KLS KLD SUI TIME PING PPH SCORE
		// 12345678 xxx xxx xxx xxxx xxxx xxx xxx.x
	    sprintf(szTemp1, "%-8s %3d %3d %3d %4d %4d %3d %5.1f", szName, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, (level.framenum - cl->resp.enterframe)/600, cl->ping, niq_getPPH(cl), cl->resp.score);
		}
	else if (nMode == NIQ_SB_320C)
		{
		// Name            TIME PING PPH RANK SCORE
		// 123456789012345 xxxx xxxx xxx xxxx xxx.x
	    sprintf(szTemp1, "%-15s %4d %4d %3d %4d %5.1f", szName, (level.framenum - cl->resp.enterframe)/600, cl->ping, niq_getPPH(cl), nRank, cl->resp.score);
		}
	else if (nMode == NIQ_SB_320D)
		{
		// Name          DFAVG DFCUR PING PPH SCORE
		// 1234567890123 xxx.x xxx.x xxxx xxx xxx.x
	    sprintf(szTemp1, "%-13s %5.1f %5.1f %4d %3d %5.1f", szName, niq_getaverageDF(cl), cl->pers.niq_fDamage, cl->ping, niq_getPPH(cl), cl->resp.score);
		}
	else if (nMode == NIQ_SB_320E)
		{
		// Name    DFAV DFCU KLS KLD SUI PING SCORE
		// 1234567 xx.x xx.x xxx xxx xxx xxxx xxx.x
	    sprintf(szTemp1, "%-7s %4.1f %4.1f %3d %3d %3d %4d %5.1f", szName, niq_getaverageDF(cl), cl->pers.niq_fDamage, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, cl->ping, cl->resp.score);
		}
	else if (nMode == NIQ_SB_512A)
		{
		// Name            DFAVG DFCUR KLS KLD SUI TIME PING PPH RANK SCORE
		// 123456789012345 xxx.x xxx.x xxx xxx xxx xxxx xxxx xxx xxxx xxx.x
        sprintf(szTemp1, "%-15s %5.1f %5.1f %3d %3d %3d %4d %4d %3d %4d %5.1f", szName, niq_getaverageDF(cl), cl->pers.niq_fDamage, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, (level.framenum - cl->resp.enterframe)/600, cl->ping, niq_getPPH(cl), nRank, cl->resp.score);

		nOffX = -96;
		}
	else if (nMode == NIQ_SB_640A)
		{
		// Name            WEAP HLTH AMMO  DFAVG DFCUR KLS KLD SUI TIME PING PPH RANK SCORE
		// 123456789012345 xxxx xxxx xxxx  xxx.x xxx.x xxx xxx xxx xxxx xxxx xxx xxxx xxx.x
		char pszWeap[5];
		int  nHealth;

		niq_getcurweapname(cl, pszWeap);

		// find the ent which corresponds to this client
		for (j=0; j<game.maxclients; j++)
	    	{
			edict_t* cl_ent = g_edicts + 1 + j;
			if (!cl_ent->inuse)
				continue;

	        if(cl_ent->client == cl)
				{
				// found him
				nHealth = cl_ent->health;

				// show clients that are actively fighting
				if(cl->pers.niq_isbot)
					{
					if((cl_ent->last_fire + niq_weap_fire_times[game.niq_curweapon-1]) >= level.time)
						strcat(pszWeap, "*");
					}
				else 
					{
					if(cl->weaponstate == WEAPON_FIRING || ((cl->latched_buttons|cl->buttons) & BUTTON_ATTACK))
						strcat(pszWeap, "*");
					}

				break;
				}
			}

        sprintf(szTemp1, "%-15s %4s %4d %4d  %5.1f %5.1f %3d %3d %3d %4d %4d %3d %4d %5.1f", szName, pszWeap, nHealth, cl->pers.inventory[niqlist[game.niq_curweapon].nListIndexAmmo], niq_getaverageDF(cl), cl->pers.niq_fDamage, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, (level.framenum - cl->resp.enterframe)/600, cl->ping, niq_getPPH(cl), nRank, cl->resp.score);

		nOffX = -160;
		}

	if(bSelf)
        // draw self in green
		sprintf(szTemp2, "xv %d yv %d string2 \"%s\"", nOffX, nOffY, szTemp1);
	else
		sprintf(szTemp2, "xv %d yv %d string \"%s\"",  nOffX, nOffY, szTemp1);

	nLenNew = strlen(szTemp2);
	if((nCurLen + nLenNew) >= NIQ_MAXSBCHARS)
		return 0;

	// enough room to add string
	strcat(&szSBStr[nCurLen], szTemp2);
	return nLenNew;	
} // niq_sb_addcliententry

/////////////////////////////////////////////////////////////////////////////
// niq_sortclients:
//
// Sorts by best score first then lowest df second. Returns an array of sorted
// client indices, the total number of clients in the array, and the index of
// the given client into the sorted list

void niq_sortclients(int sorted[MAX_CLIENTS], edict_t* ent, int* total, int* myIndex)
{
	int		i, j, k;
	float	sortedscores[MAX_CLIENTS];
	float	sorteddfacts[MAX_CLIENTS];
	float	score;
	float	dFact;
	int		nTotal;

	nTotal = 0;
	for (i=0 ; i<game.maxclients ; i++)
    	{
		edict_t* cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

        if(cl_ent == ent)
            *myIndex = i;                                                              

		score = game.clients[i].resp.score;
		if(!game.clients[i].resp.niq_fDamageTics)
			dFact = game.clients[i].pers.niq_fDamage;
		else
			dFact = game.clients[i].resp.niq_fDamageTotal / game.clients[i].resp.niq_fDamageTics;

		for (j=0 ; j<nTotal ; j++)
	    	{
			// sort by score first
			if (score > sortedscores[j])
				break;

			// and by df second
			if (score == sortedscores[j] && dFact < sorteddfacts[j])
				break;
		    }

		for (k=nTotal ; k>j ; k--)
    		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
			sorteddfacts[k] = sorteddfacts[k-1];
	    	}

		sorted[j] = i;
		sortedscores[j] = score;
		sorteddfacts[j] = dFact;
		nTotal++;
    	}

#ifdef _DEBUG
	if(*myIndex >= game.maxclients)
		{
		// idiot check
	    gi.dprintf("NIQ: invalid index for self\n");
		}
#endif

	// convert myIndex from index into game.clients into index into sorted list
	for(i=0; i<nTotal; i++)
		{
		if(sorted[i] == *myIndex)
			{
			*myIndex = i;
			break;
			}
		}

	*total = nTotal;
} // niq_sortclients

/////////////////////////////////////////////////////////////////////////////
// niq_deathmatchscoreboard:

static void niq_deathmatchscoreboard(edict_t* ent)
{
	char	szSBStr[NIQ_MAXSBCHARS];
	int		nSBStrLen;
	int     nNewLen = 0;
	int		nRank;
	int		total;
	int		sorted[MAX_CLIENTS];
	int		myIndex = -1;
	float   fLastScore;
	int		nMode;
	int i;
	gclient_t* cl;
	int nMaxSBClients;
	int g_nScoreBoard_YOffset;

	niq_sortclients(sorted, ent, &total, &myIndex);

	nMode = ent->client->pers.niq_sbmode;

	// ugly hack
	g_nScoreBoard_YOffset = NIQ_SCOREBOARD_YOFFSET;

	nMaxSBClients = NIQ_DEFMAXSCORELINES;
	if(nMode == NIQ_SB_512A)
		{
		nMaxSBClients = NIQ_DEFMAXSCORELINES-6;	// can only show 6 clients max + me
//		g_nScoreBoard_YOffset += 40;
		}
	else if(nMode == NIQ_SB_640A)
		{
		nMaxSBClients = NIQ_DEFMAXSCORELINES-6;	// can only show 6 clients max + me
//		g_nScoreBoard_YOffset += 20;
		}

    // user can control scoreboard size up to MAXSCORELINES (good for testing)
    if(total > niq_sblines->value)
	    total = niq_sblines->value;

	if (total <= 0)
		total = nMaxSBClients;

	if (total > nMaxSBClients)
		total = nMaxSBClients;

	// print level name and exit rules
	szSBStr[0]	= 0;
	nSBStrLen	= 0;

	// we can assume that adding the main titles won't exceed the string limit
	niq_sb_addmaintitles(szSBStr, nMode);
	nSBStrLen = strlen(szSBStr);

	// add the clients in sorted order (and show the rank of each)
	nRank		= 0;
	fLastScore  = 9999999.0;

	for (i=0; i<total; i++)
    	{
		cl = &game.clients[sorted[i]];

		// each time the score goes down, so does the rank (i.e. increase its value)
		if(cl->resp.score < fLastScore)
			{
			nRank++;
			fLastScore = cl->resp.score;
			}

		nNewLen = niq_sb_addcliententry(nSBStrLen, szSBStr, cl, nMode, nRank, g_nScoreBoard_YOffset + 40 + 10 * i, i == myIndex);
		if(!nNewLen)
			break;

		nSBStrLen += nNewLen;
    	}

    // code to add ent's score if not in top MAXSCORELINES (so clients always see their own score)
    if(nNewLen && myIndex >= total)
        {
		int nOffY;

		// scan from total to myIndex, increasing rank as score decreases
		for(i=total; i<=myIndex; i++)
			{
			cl = &game.clients[sorted[i]];
			if(cl->resp.score < fLastScore)
				{
				nRank++;
				fLastScore = cl->resp.score;
				}
			}

		if(myIndex == total)
			nOffY = g_nScoreBoard_YOffset + 40 + 10 * total;		// no gap
		else
			nOffY = g_nScoreBoard_YOffset + 40 + 10 * (total+1);	// 1 line gap
			
		nSBStrLen += niq_sb_addcliententry(nSBStrLen, szSBStr, cl, nMode, nRank, nOffY, true);
        }

	if(nSBStrLen < (NIQ_MAXSBCHARS - 100))
		{
	    // add a line to explain the scoring being used
//		sprintf(szTemp1, "(KILL:%+4.1f KILLED:%+4.1f SUICIDE:%+4.1f)", niq_killpts->value, -niq_kildpts->value, -niq_suicpts->value);

	    // center the text
//		sprintf(szTemp2, "xv %d yv %d string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-strlen(szTemp1)), g_nScoreBoard_YOffset + 70 + 10 * NIQ_DEFMAXSCORELINES, szTemp1);

//		strcat(&szSBStr[nSBStrLen], szTemp2);

//		nSBStrLen+=strlen(szTemp2);

	    // and add a centered help line
		sprintf(&szSBStr[nSBStrLen], "xv %d yv %d string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-strlen(szSBHelp)), g_nScoreBoard_YOffset + NIQ_SB_HELP_OFFSET + 10 * NIQ_DEFMAXSCORELINES, szSBHelp);
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (szSBStr);
} // niq_deathmatchscoreboard

/////////////////////////////////////////////////////////////////////////////
// niq_deathmatchscoreboardmessage
// 
// TBD: show client in different color so easy to find? Could also maybe put
// a * in front.

void niq_deathmatchscoreboardmessage(edict_t *ent, edict_t *killer, qboolean reliable)
{
	// idiot checks
	if(!ent || !ent->inuse || !ent->client)
		return;

	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_deathmatchscoreboardmessage called for a bot!\n");
#endif
		return;
		}

	// make sure we didn't switch to non-ctf mode with ctf sb up
	if(!ctf->value && (ent->client->pers.niq_sbmode == NIQ_SB_CTF))
		{
	    if(niq_handicap->value)
			ent->client->pers.niq_sbmode	= NIQ_SB_320D;
	    else
			ent->client->pers.niq_sbmode	= NIQ_SB_320A;	// default to 320x240 scoreboard
		}

	// make sure we didn't turn off niq_sbdebug with the 640 SB up
	if(!niq_sbdebug->value && ent->client->pers.niq_sbmode == NIQ_SB_640A)
		ent->client->pers.niq_sbmode++;

	// make sure we didn't turn off wide scoreboards with wide SB up
	if(!niq_sbwide->value && ent->client->pers.niq_sbmode >= NIQ_SB_512A && ent->client->pers.niq_sbmode <= NIQ_SB_640A)
		ent->client->pers.niq_sbmode = NIQ_SB_DM;

	if(ent->client->pers.niq_sbmode >= NIQ_SB_DM)
		{
		// client wants regular deathmatch or CTF scoreboard
		DeathmatchScoreboardMessage(ent, killer);
		}		
	else
		{
		// client wants one of the NIQ scoreboards
		niq_deathmatchscoreboard(ent);
		}

	gi.unicast(ent, reliable);
} // niq_deathmatchscoreboardmessage

/////////////////////////////////////////////////////////////////////////////

void niq_cleardfforallclients()
{
	// looks like handicapping was just disabled, make sure all clients have df=1.0
	int i;
	for (i=1; i<=game.maxclients; i++)
	    {
		edict_t* ent = g_edicts + i;
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		if(ent->client->pers.niq_fDamage != 1.0)
			{
			ent->client->pers.niq_fDamage = 1.0;

			if(ent->client->showscores)
				niq_deathmatchscoreboardmessage (ent, NULL, false);
			}
		}
} // niq_cleardfforallclients

/////////////////////////////////////////////////////////////////////////////

void niq_checktimers(edict_t* ent)
{
	static qboolean bDFEnabled = false;
	gclient_t* client;

  	if (!ent->inuse)
    	return;

    // don't do anything after client dies
	if (ent->health < 1)
        return;

	client = ent->client;
	if (!client)
		return;

#ifdef _DEBUG
	if(client->pers.niq_isbot && !ent->bot_client || !client->pers.niq_isbot && ent->bot_client)
		gi.dprintf("NIQ: isbot != bot_client in niq_checktimers!\n");
#endif
				
	// nothing to do if client is observing
	if(ent->svflags & SVF_NOCLIENT)
		return;

    // did he join a team yet?
    if(ctf->value && ent->client->resp.ctf_team == CTF_NOTEAM)
        return;

	if (client->niq_sectimer < level.time)
		{
	    // increment health unless niq_auto=0 (means you have to kill monsters/clients to get health)
		if(niq_auto->value)
			niq_incrementhealth(ent, client);

	    // increment ammo unless infinite ammo is enabled
		if ( !((int)dmflags->value & DF_INFINITE_AMMO) )
	   		niq_incrementammo(client);

		// niq1.3: average out df over time connected
		if(niq_handicap->value)
			{
			niq_trackdfactor(client);
			bDFEnabled = true;
			}
		else
			{
			if(bDFEnabled)
				{
				niq_cleardfforallclients();
				bDFEnabled = false;
				}

			// don't forget to let clients df converge back towards 1 if necessary
			if(client->resp.niq_fDamageTics)
				niq_trackdfactor(client);
			}

		// reset timer
		client->niq_sectimer = level.time + NIQ_ONESECOND;
		}
} // niq_checktimers

#ifdef EBOTS

/////////////////////////////////////////////////////////////////////////////

void niq_botsetstuff(edict_t *self)
{
	gclient_t	*client;

	client = self->client;

	// niq: if BFG is current weapon, have to use it (quad does not apply)
	if (client->pers.inventory[ITEM_INDEX(item_bfg10k)] )
	{
		self->bot_fire = botBFG;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_BFG;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_railgun)] )
	{
		self->bot_fire = botRailgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_RAILGUN;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_chaingun)] )
	{
		self->bot_fire = botChaingun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_CHAINGUN;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_hyperblaster)] )
	{
		self->bot_fire = botHyperblaster;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] )
	{
		self->bot_fire = botRocketLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_machinegun)] )
	{
		self->bot_fire = botMachineGun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_MACHINEGUN;

		goto found;
	}

	if (client->pers.inventory[ITEM_INDEX(item_supershotgun)] )
	{
		self->bot_fire = botSuperShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SSHOTGUN;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] )
	{
		self->bot_fire = botGrenadeLauncher;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;

		goto found;
	}
	if (client->pers.inventory[ITEM_INDEX(item_shotgun)] )
	{
		self->bot_fire = botShotgun;
		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;

		goto found;
	}

#ifdef _DEBUG
	if (!client->pers.inventory[ITEM_INDEX(item_blaster)] )
		gi.dprintf("NIQ: blaster not found in inventory\n");
#endif

	self->bot_fire = botBlaster;
	self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;
	self->fire_interval = FIRE_INTERVAL_BLASTER;

found:
	client->pers.niq_isbot = true;
} // niq_botsetstuff

#endif

/////////////////////////////////////////////////////////////////////////////
// niq_removeallweapons: 
//
// Called when a player dies or connects to remove all weapons and ammo for 
// that player.
// If and when he decides to join back in, the server will allocate the 
// current weapon and some ammo to him.

void niq_removeallweapons(edict_t* ent)
{
    // make sure the client has no ammo and no weapon initially (depending
    // on timing, if the server switches weapons while the client isn't
    // connected, the next weapon/ammo will be allocated without removing
    // the old one.
    int nWeap;
    for(nWeap=1; nWeap<=MAXNIQWEAPONS; nWeap++)
        {
    	ent->client->pers.inventory[niqlist[nWeap].nListIndexWeapon] = 0;
  	    niq_set_ammo(ent->client, 0, nWeap);
        }

	ent->client->ammo_index = 0;
} // niq_removeallweapons

/////////////////////////////////////////////////////////////////////////////
// niq_changeclientweapon:
//
// I'm almost positive that this should be hooked into the server timer
// (ServerThink?) to ensure that all clients are switched at the same time.
// TBD: delay allowing any weapons to fire for N seconds after a switch.
// Depends on how smoothly the switch can be done. Might want a delay before
// the switch too to let all projectiles reach their destination, explode
// etc. e.g. freeze all firing for 2 seconds, switch the weapon, freeze all
// firing for 2 seconds, continue.

static void niq_changeclientweapon(edict_t *ent, int nOldWeapon, int nNewWeapon)
{
    int nOldWeaponIndex;
    int nOldAmmoIndex;
    int nNewWeaponIndex;
    int nNewAmmoIndex;
    int nNewAmmoAmount;
	int oldselected;

	gclient_t* client;

	client = ent->client;
	if (!client)
		return;

    // is he dead?
	if (ent->health < 1)
       return;

// switch even if not on a team so what is shown while observing is correct
    // did he join a team yet?
//    if(ctf->value && client->resp.ctf_team == CTF_NOTEAM)
//        return;

    // save the old weapon information
    nOldWeaponIndex     = niqlist[nOldWeapon].nListIndexWeapon;
    nOldAmmoIndex       = niqlist[nOldWeapon].nListIndexAmmo;
    nNewWeaponIndex     = niqlist[nNewWeapon].nListIndexWeapon;
    nNewAmmoIndex       = niqlist[nNewWeapon].nListIndexAmmo;
    nNewAmmoAmount      = niqlist[nNewWeapon].nAmmoInitial;

    if(nOldWeaponIndex  <= 0 ||
       nOldAmmoIndex    <= 0 ||
       nNewWeaponIndex  <= 0 ||
       nNewAmmoIndex    <= 0 ||
       nNewAmmoAmount   <  0)
        {
#ifdef _DEBUG
		gi.dprintf ("NIQ: error in NIQChangeClientWeapon!\n");
#endif
        return;
        }

    // allocate new ammo:
	if(nNewAmmoIndex != 999)
    	niq_set_ammo(ent->client, nNewAmmoAmount, nNewWeapon);

	ent->client->ammo_index = nNewAmmoIndex;

    // allocate new weapon
	client->pers.inventory[nNewWeaponIndex] = 1;

	if(ctf->value || grapple->value)
		{
	    // niq: only switch to new weapon if grapple is not in use
		if(client->pers.weapon && client->pers.weapon->pickup_name && (stricmp(client->pers.weapon->pickup_name, "Grapple") || client->newweapon))
	        client->newweapon = &itemlist[nNewWeaponIndex];
		}
	else
	    client->newweapon = &itemlist[nNewWeaponIndex];
        
    // remove old weapon if not same as new one
    if(nOldWeaponIndex != nNewWeaponIndex)
    	client->pers.inventory[nOldWeaponIndex] = 0;

	// take away the old ammo if not same as new ammo
    if((nOldAmmoIndex != 999) && (nOldAmmoIndex != nNewAmmoIndex))
	    niq_set_ammo (ent->client, 0, nOldWeapon);

#ifdef _DEBUG
	// long time bug: sometimes end up with old ammo staying???
	{
	int count = 0;
	if(ent->client->pers.inventory[ITEM_INDEX(item_shells)] != 0)
		count++;
	if(ent->client->pers.inventory[ITEM_INDEX(item_bullets)] != 0)
		count++;
	if(ent->client->pers.inventory[ITEM_INDEX(item_grenades)] != 0)
		count++;
	if(ent->client->pers.inventory[ITEM_INDEX(item_cells)] != 0)
		count++;
	if(ent->client->pers.inventory[ITEM_INDEX(item_rockets)] != 0)
		count++;
	if(ent->client->pers.inventory[ITEM_INDEX(item_slugs)] != 0)
		count++;

	if(nNewWeapon == 1 && count != 0)
		gi.dprintf("NIQ: ammo problem!\n");

	if(nNewWeapon != 1 && count != 1)
		gi.dprintf("NIQ: ammo problem!\n");
	}
#endif

	oldselected = client->pers.selected_item;

	if(ctf->value || grapple->value)
		{
		// niq: only switch to new weapon if grapple is not in use
	    if(client->pers.weapon && client->pers.weapon->pickup_name && (stricmp(client->pers.weapon->pickup_name, "Grapple") || client->newweapon))
	        client->pers.selected_item = nNewWeaponIndex;
		}
	else
		client->pers.selected_item = nNewWeaponIndex;

  	client->weapon_sound = 0;

    // reset client ammo tics (niq1.3: ammo timer stays where it is so some clients might get 
	// more ammo right away but this is at most a 1 second difference -- not worth keeping)
	client->niq_ammotics = 0;

#ifdef EBOTS
	if (ent->bot_client)
		niq_botsetstuff(ent);
#endif

    if(oldselected != client->pers.selected_item)
		ChangeWeapon(ent);

	// play pickup sound when switching weapon
    if (niq_sndswitch->value) 
    	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/w_pkup.wav"), 1.0, ATTN_NORM, 0);
} // niqchangeclientweapon

/////////////////////////////////////////////////////////////////////////////

static void niq_changeweapon()
{
   	edict_t	*ent;
    int i;
    int niq_oldweapon = game.niq_curweapon;

    niq_selectcurrentweapon(false);

	// tell all clients about the impending weapon change
    if(niq_msgswitch->value)
        niq_showmessagetoallclients(niqlist[game.niq_curweapon].szMessage, false);

    // get all current clients to change to new weapon
	for (i=1; i<=maxclients->value; i++)
        {
		ent = g_edicts + i;
    	if (!ent->inuse)
	    	continue;

        niq_changeclientweapon(ent, niq_oldweapon, game.niq_curweapon);
    	}
} // niq_changeweapon

/////////////////////////////////////////////////////////////////////////////

void niq_checkiftimetochangeweapon()
{    
    static float    fSavedTime          = -1.0;
    static qboolean bDidIntermission    = false;

    // niq 15/05/98: don't use switching code at all if only 1 weapon available
    if(game.niq_numweapons == 1)
        return;

    // in SP/COOP you normally have to kill N monsters to have the weapon change
    if(!deathmatch->value)
        {
        // not in deathmatch
        if(!niq_auto->value)
            {
            // weapons don't auto switch
            if(level.intermissiontime)
                {
                // in intermission (level change)
                if(niq_weapkills->value == -1 && !bDidIntermission)
                    {
                    // switchkills is -1 ==> change weapon on level change
                    niq_changeweapon();

                    bDidIntermission = true;
                    }
                }
            else
                bDidIntermission = false;

            return;
            }
        }

	if (level.intermissiontime)
        {
        if(game.niq_wsecs == -1 && !bDidIntermission)
    		{
            // yes -- change it (no need to reset timer)
            niq_changeweapon();

            bDidIntermission = true;
            return;
    		}

        // don't allow time to move forward while switching levels
        if(fSavedTime == -1.0)
            {
  		    fSavedTime = (game.niq_wsecs - level.time);
            if(fSavedTime < 0.0)
                fSavedTime = 0.0;
            }

        return;
        }

    bDidIntermission = false;

    // not in intermission at this point
    if(fSavedTime != -1.0)
        { 
        // pick up the timer where we left off
        if(game.niq_wsecs != -1)
            game.niq_wsecs = level.time + fSavedTime;

        fSavedTime = -1.0;
        }
    
    // if user changes setting for this, take it if 
    if(niq_weapsecs->value == -1)
        {
        // is set to -1 (switch on map change)
  		niq_setweapontime(-1);
        }
    else if(niq_prevwsecs == -1 && niq_weapsecs->value != -1)
        {
        // if it was set to -1
  		niq_setweapontime(level.time + niq_weapsecs->value);
        }
    else if(niq_weapsecs->value > niq_prevwsecs)
        {
        // it is more than the old one
  		niq_setweapontime(level.time + niq_weapsecs->value);
        }
    else if(level.time + niq_weapsecs->value < game.niq_wsecs)
        {
        // or less than the remaining time
        niq_setweapontime(level.time + niq_weapsecs->value);
        }

    if(game.niq_wsecs == -1)
        return; // weapon switches when map changes

    // time to change weapon?
	if (game.niq_wsecs < level.time) 
		{
        // yes -- change it
        niq_changeweapon();

        // reset switch timer
  		niq_setweapontime(level.time + niq_weapsecs->value);
		}

    // but is it *almost* time to change weapon?
	else if(niq_sndwarn->value)
        {
        float fTimeLeft = game.niq_wsecs - level.time;

		if (fTimeLeft == 3.0)
            {
        	// warning sound before switching weapon
            int i;
            edict_t* ent;

            // warn all clients that the weapon is about to change even if they are dead
    	    for (i=1; i<=maxclients->value; i++)
                {
	    	    ent = g_edicts + i;
            	if (!ent->inuse)
	            	continue;

            	if (!ent->client)
	            	continue;

                // did he join a team yet?
                if(ctf->value && ent->client->resp.ctf_team == CTF_NOTEAM)
                    return;

    			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
                }
            }
        }
} // niq_checkiftimetochangeweapon

/////////////////////////////////////////////////////////////////////////////
// niq_getclientrank

int niq_getclientrank(edict_t *ent)
{
    return -1;
} // niq_getclientrank

/////////////////////////////////////////////////////////////////////////////

int niq_getnumclients()
{
	int nCount = 0;
	int i;

	for (i=1; i<=game.maxclients; i++)
        {
    	edict_t* ent = g_edicts + i;
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		nCount++;
		}

	return nCount;
} // niq_getnumclients

/////////////////////////////////////////////////////////////////////////////

//#define _FILEIODEBUG 0
static FILE* niq_fopen(const char* szFile, qboolean bNotFoundOK)
{
    FILE* fret;
    char szNIQFile[128] = "";
    cvar_t *gamedir = gi.cvar ("gamedir",   "",    CVAR_SERVERINFO);

#ifdef _FILEIODEBUG
    gi.dprintf("niq_fopen: gamedir is '%s' \n", gamedir->string);
#endif
	
    if(gamedir && gamedir->string[0] != '\0')
        {
        // try finding files in the gamedir
        strcpy(szNIQFile, gamedir->string);
        strcat(szNIQFile, "\\");
        }
    else
        {
        // invalid gamedir -- try guessing
		if(ctf->value)
			{
			// should be in ctf dir
	        strcpy(szNIQFile, NIQ_DEFAULTPATHCTF);
			}
		else
			{
			// try niq, but if ctf stuff being used (grapple, maps) not good for clients
	        strcpy(szNIQFile, NIQ_DEFAULTPATHNIQ);
			}
        }

    strcat(szNIQFile, szFile);

#ifdef _FILEIODEBUG
    gi.dprintf("niq_fopen: trying to open '%s' \n", szNIQFile);
#endif

    // try to open the file
    fret = fopen(szNIQFile, "r");

    if(!fret && !bNotFoundOK)
	{
        gi.dprintf("NIQ: file '%s' not found in gamedir or in default dir -- using defaults.\n", szFile);
    }
    
    return fret;
} // niq_fopen

/////////////////////////////////////////////////////////////////////////////

void niq_loadmotdfile()
{
    FILE *motd_file;
    char line[MAXLINESIZE];
    int  nLine = 0;

    // handle custom motd: 
    motd_file = niq_fopen(NIQ_MOTDFILE, true);
    if(motd_file)
        {
        while(fgets(line, 80, motd_file))
            {
			int nEnd;

			if(nLine >= NIQ_MAXMOTDLINES)
				{
                gi.dprintf("NIQ: more than %d motd lines, extra lines ignored.\n", NIQ_MAXMOTDLINES);
                break;
				}

			// strip trailing CR, if any
			nEnd = strlen(line)-1;
			if(line[nEnd] == '\n')
				line[nEnd] = '\0';

            // make sure no line is more than NIQ_SBCHARS320 characters
            line[NIQ_SBCHARS320] = '\0';

            strcpy(motdlines[nLine], line);

			nLine++;
            }

        fclose(motd_file);
        }
} // niq_loadmotdfile

/////////////////////////////////////////////////////////////////////////////

void niq_loadweaponlist()
{
    FILE *weap_file;
    char line[MAXLINESIZE];
    int  nLine = 0;
    int  ii;
    qboolean bFound;

    // try niq subdirectory first
    game.niq_numweapons = 0;
    weap_file = niq_fopen(NIQ_WEAPFILE, false);
    if(weap_file)
        {
        while(fgets(line, 80, weap_file))
            {
            nLine++;

            // ignore lines starting with / or # or a blank or a CR
            if(line[0] == '/' || line[0] == '#' || line[0] == ' ' || line[0] == '\n')
                continue;
            
            // ignore empty lines
            if(strlen(line) < 2)
                continue;

            // hack: we use 'superblaster' in weapon list, but blaster internally
            if(!strncmp(line, "superblaster", 12))
                strcpy(line, "blaster");

            // identify weapon and its order in the list
            ii=1;
            bFound = false;
            while(ii<=MAXNIQWEAPONS && !bFound)
                {
                if(!strncmp(line, niqlist[ii].szName, strlen(niqlist[ii].szName)))
                    bFound = true;
                else
                    ii++;
                }

            if(!bFound)
                {
                gi.dprintf("NIQ: invalid weapon list item at line %d, resetting default list.\n", nLine);
                game.niq_numweapons = 0;
                break;
                }

            if(game.niq_numweapons >= NIQMAXLISTWEAPONS)
                {
                gi.dprintf("NIQ: more than %d weapons specified, no more weapons accepted.\n", NIQMAXLISTWEAPONS);
                break;
                }

            game.niq_numweapons++;
            game.niq_weaponsindx[game.niq_numweapons] = ii;
            }

        fclose(weap_file);
        }

    // on any error, just restore the default list
    if(game.niq_numweapons == 0)
        {
        for(ii=1; ii<=MAXNIQWEAPONS; ii++)
            game.niq_weaponsindx[ii] = ii;
        game.niq_numweapons = MAXNIQWEAPONS;
        }

    // clear the used flags for all weapons
    for(ii=1; ii<=game.niq_numweapons; ii++)
        game.niq_weaponsused[ii] = 0;
} // niq_loadweaponlist

/////////////////////////////////////////////////////////////////////////////

void niq_loadammolist()
{
    FILE *ammo_file;
    char line[MAXLINESIZE];
    int  nLine = 0;
    int  ii;
    int  ammoinitial, ammoincrement, ammosecs, ammomax;

    // try niq subdirectory first
    ammo_file = niq_fopen(NIQ_AMMOFILE, false);
    if(ammo_file)
        {
        while(fgets(line, 80, ammo_file))
            {
            nLine++;

            // ignore lines starting with / or # or a blank or a CR
            if(line[0] == '/' || line[0] == '#' || line[0] == ' ' || line[0] == '\n')
                continue;
            
            // ignore empty lines
            if(strlen(line) < 2)
                continue;

            // identify weapon and its order in the list
            ii=1;
            while(ii<=MAXNIQWEAPONS)
                {
                if(!strncmp(line, niqlist[ii].szName, strlen(niqlist[ii].szName)))
                    {
                    // set the parameters for this weapon from the file
                    sscanf(&line[strlen(niqlist[ii].szName)], "%d %d %d %d\n", &ammoinitial, &ammoincrement, &ammosecs, &ammomax);

                    niqlist[ii].nAmmoInitial    = ammoinitial;
                    niqlist[ii].nAmmoIncrement  = ammoincrement;
                    niqlist[ii].nAmmoSecs       = ammosecs;    
                    niqlist[ii].nAmmoMax        = ammomax;     
                    break;
                    }
                else
                    ii++;
                }

            if(ii>MAXNIQWEAPONS)
                gi.dprintf("NIQ: invalid ammo list item at line %d.\n", nLine);
            }

        fclose(ammo_file);
        }
} // niq_loadammolist

/////////////////////////////////////////////////////////////////////////////

#ifdef NIQSP
void niq_loadmonsterlist()
{
    FILE *monst_file;
    char line[MAXLINESIZE];
    char szMonsterName[80];
    int  nLine = 0;
    int  ii;

    // try niq subdirectory first
    monst_file = niq_fopen(NIQ_MONSTFILE, false);
    if(monst_file)
        {
        while(fgets(line, 80, monst_file))
            {
            int healthinc;

            nLine++;

            // ignore lines starting with / or # or a blank or a CR
            if(line[0] == '/' || line[0] == '#' || line[0] == ' ' || line[0] == '\n')
                continue;
            
            // ignore empty lines
            if(strlen(line) < 2)
                continue;

            // everything up to first whitespace is part of monstername
            ii=0;
            while((line[ii] >= 'a' && line[ii] <= 'z') || (line[ii] >= 'A' && line[ii] <= 'Z') || (line[ii] >= '0' && line[ii] <= '9') || line[ii] == '_')
                {
                szMonsterName[ii] = tolower(line[ii]);
                ii++;
                }
            szMonsterName[ii] = '\0';

            // read health increment for this monster
            sscanf(&line[strlen(szMonsterName)], "%d\n", &healthinc);

            // identify monster and set its health increment
            ii=1;
            while(niq_monsters[ii].nHealthInc != -999)
                {
                if( !strcmp(szMonsterName, &niq_monsters[ii].szName[strlen("monster_")]) ||
                    !strcmp(szMonsterName, &niq_monsters[ii].szName[strlen("misc_")]) ||
                    !strcmp(szMonsterName, niq_monsters[ii].szName) )
                    {
                    // set the health increment for this monster from the file
                    niq_monsters[ii].nHealthInc = healthinc;
                    break;
                    }
                else
                    ii++;
                }

            if(niq_monsters[ii].nHealthInc == -999)
                gi.dprintf("NIQ: invalid monster list item at line %d.\n", nLine);
            }

        fclose(monst_file);
        }
} // niq_loadmonsterlist

/////////////////////////////////////////////////////////////////////////////

int niq_getkillsneededforswitch(edict_t *ent)
{
    // in sp/coop someone has to have N kills to cause weapon to switch    
    int killsneeded;

    // idiot check
    if(niq_weapkills->value == 0)
        niq_weapkills->value = 1;

    killsneeded = (int)niq_weapkills->value;

/*  Not sure why I had this. Why should the kills needed to 
    switch to the next weapon change with the number of clients?
    if(coop->value)
        killsneeded *= niq_getnumclients();
*/

    return killsneeded - game.niq_monsterkills;
} // niq_getkillsneededforswitch

#endif

/////////////////////////////////////////////////////////////////////////////

void niq_clientkill(edict_t* attacker)
{
	int nOrigHealth;
	gclient_t* client;

    // only applies to deathmatch
    if(!deathmatch->value)
        return;

    // if we are autoincrementing nothing to do
    if(niq_auto->value)
        return;
        
    // don't increment health after client dies!
  	if (!attacker->inuse)
    	return;

	client = attacker->client;
	if (!client)
		return;

    // is he dead?
	if (attacker->health < 1)
        return;

    nOrigHealth = attacker->health;

    attacker->health += niq_hlthinc->value;

    // make sure we didn't exceed the maximum allowed health
    if(attacker->health > niq_hlthmax->value)
        attacker->health = niq_hlthmax->value;

    // do health increment sound?
    if((niq_sndhlth->value) && attacker->health > nOrigHealth)
		gi.sound(attacker, CHAN_HLTH, gi.soundindex("items/s_health.wav"), 1.0, ATTN_NORM, 0);
} // niq_clientkill

#ifdef NIQSP

/////////////////////////////////////////////////////////////////////////////

void niq_catmonstername(char* szMsg, char* szMonster)
{
    if(!strncmp(szMonster, "monster_", 8))
        {
        if(strncmp(&szMonster[8], "infantry", 8))
            strcat(szMsg, "a ");
        else
            strcat(szMsg, "an ");

        strcat(szMsg, &szMonster[8]);
        }
    else if(!strncmp(szMonster, "misc_insane", 11))
        strcat(szMsg, "a POW");
    else if(!strncmp(szMonster, "turret_driver", 13))
        strcat(szMsg, "a Turret Driver");
    else if(!strncmp(szMonster, "noclass", 7)) // Strogg Leader Part II?
        strcat(szMsg, "the Strogg Leader");
} // niq_catmonstername

/////////////////////////////////////////////////////////////////////////////
// niq_monsterdeath:
//
// In NIQ, when a monster dies (in SP/COOP) the attacker gets some health
// depending on the type of monster killed.
//
void niq_monsterdeath(edict_t *monster, edict_t *attacker)
{
    int nOrigHealth;
    int ii;
	gclient_t *client;
    char szMsg[128] = "";

    if(deathmatch->value)
        return;

    if(!monster || !attacker)
        return;

    // show what monster was killed to all clients
    if(attacker->client)
		{
		// attacker was a client -- show what he killed
        strcpy(szMsg, attacker->client->pers.netname);  
	    strcat(szMsg, " killed ");
		niq_catmonstername(szMsg, monster->classname);
		}
    else
		{
		// attacker was a monster -- show that it died
        niq_catmonstername(szMsg, monster->classname);  
	    strcat(szMsg, " died");
        }

    niq_showmessagetoallclients(szMsg, true);

    // if autoinc is set, nothing to do here
    if(niq_auto->value)
        return;

	client = attacker->client;

    // if the attacker isn't a client (e.g. another monster), return
	if (!client)
		return;

    // 08/05/98 mfox: don't increment counter if only 1 weapon (no need to switch)
    if(game.niq_numweapons != 1 && niq_weapkills->value != -1)
        {
        // don't increment count for killing the insane POWs
        if(strncmp(monster->classname, "misc_insane", strlen("misc_insane")))
            {
            game.niq_monsterkills++;

            if(niq_getkillsneededforswitch(attacker) == 0)
                {
                niq_changeweapon();
                game.niq_monsterkills = 0;
                }
            }
        }

    // don't increment health after client dies!
  	if (!attacker->inuse)
    	return;

    // is he dead?
	if (attacker->health < 1)
        return;

    nOrigHealth = attacker->health;

    // see if monster class is one of the ones we handle
    ii=1;
    while(niq_monsters[ii].nHealthInc != -999)
        {
        if(!strncmp(monster->classname, niq_monsters[ii].szName, strlen(niq_monsters[ii].szName)))
            {
            // if the health is still 999, we haven't set it yet
            if(niq_monsters[ii].nHealthInc == 999)
                {
                // for some reason the Strogg leader comes in as "noclass"
                if(strncmp(monster->classname, "noclass", 7))
                    {
                    gi.dprintf("NIQ: health not set for '%s'!\n", niq_monsters[ii].szName);

                    // give the attacker some health anyway
                    attacker->health += 10;
                    }
                }
            else
                attacker->health += niq_monsters[ii].nHealthInc;

            break;
            }
        else
            ii++;
        }
    
    if(niq_monsters[ii].nHealthInc == -999)
        {
        // monster not found -- this shouldn't happen
        gi.dprintf("NIQ: killed monster not found!\n");
    
        // but give the attacker some health anyway
        attacker->health += 10;
        }

    // make sure we didn't exceed the maximum allowed health
    if(attacker->health > niq_hlthmax->value)
        attacker->health = niq_hlthmax->value;

    // users can enter -ve health for fun, but don't let it go below 1 since
    // this screws things up because quake doesn't expect you to die this way
    if(attacker->health < 1)
        attacker->health = 1;

    // do health increment sound?
    if((niq_sndhlth->value) && attacker->health > nOrigHealth)
		gi.sound(attacker, CHAN_HLTH, gi.soundindex("items/s_health.wav"), 1.0, ATTN_NORM, 0);
} // niq_monsterdeath
#endif

/////////////////////////////////////////////////////////////////////////////
// niq_drop_item:
//
// Most items can not be dropped in NIQ except for keys etc. which are needed
// in SP/COOP to unlock doors. For NIQCTF, we will also need to allow dropping 
// flags. Also allowing rebreather and environment suit for SP/COOP.

void niq_drop_item(edict_t *self, gitem_t *item)
{
    if(!item)
        return;

    if(deathmatch->value)
        return;

    if(!strncmp(item->classname, "key_", 4))
        Drop_Item (self, item);

    else if(!strncmp(item->classname, "item_breather", strlen("item_breather")))
        Drop_Item (self, item);

    else if(!strncmp(item->classname, "item_enviro",   strlen("item_enviro")))
        Drop_Item (self, item);
} // niq_drop_item

/////////////////////////////////////////////////////////////////////////////

static qboolean niq_commandisblocked(edict_t *ent, char* cmd)
{
	if ( !Q_stricmp (cmd, "drop")       ||
	     !Q_stricmp (cmd, "invnextp")   ||
	     !Q_stricmp (cmd, "invprevp")   ||
	     !Q_stricmp (cmd, "invdrop") )
        {
		gi.cprintf (ent, PRINT_HIGH, "NIQ: command is unavailable.\n");
        return true;
        }

	if ( !ctf->value && !grapple->value &&

	     (!Q_stricmp (cmd, "invnextw")   ||
	      !Q_stricmp (cmd, "invprevw")   ||
	      !Q_stricmp (cmd, "weapprev")   ||
	      !Q_stricmp (cmd, "weapnext")   ||
	      !Q_stricmp (cmd, "weaplast")) )
        {
		gi.cprintf (ent, PRINT_HIGH, "NIQ: command is currently unavailable.\n");
        return true;
        }

	return false;
} // niq_commandisblocked

/////////////////////////////////////////////////////////////////////////////

static void niq_zapitems()
{
	int		ii;
	gitem_t	*it;

	it = itemlist;
	for (ii=0 ; ii<game.num_items ; ii++, it++)
	{
        if(!it->classname)
            continue;

		if(ctf->value)
			{
	        // leave flags as is
			if (!strncmp(it->classname, "item_flag", 9))
			    continue;
			}

        // leave the grapple as is
		if (!strncmp(it->classname, "weapon_grapple", 14))
            continue;

		if(!deathmatch->value)
			{
	        // unless the item is a key or flag item, we mark it as can't be picked up and can't be dropped
			if (!strncmp(it->classname, "key_", 4))
				continue;

	        // we also allow the rebreather and environment suit for sp/coop
			if (!strncmp(it->classname, "item_breather", 13))
				continue;

			if (!strncmp(it->classname, "item_enviro", 11))
				continue;
			}

        // anything but weapons can't be used (for CTF, have to allow selection)
		if (strncmp(it->classname, "weapon_", 7))
            it->use = NULL;

        // anything but weapons can't be used (for CTF, have to allow selection)
		if (!strncmp(it->classname, "ammo_", 5))
            {
            it->use = NULL;
            it->weaponthink = NULL;
            }
        
        // anything else can't be picked up or dropped
        it->drop = NULL;
        it->pickup = NULL;
	}
} // niq_zapitems

/////////////////////////////////////////////////////////////////////////////
// niq_initcvars:
//
// The default cvar behaviour is to have changes take effect immediately and
// the variables don't show up in serverinfo or userinfo. For now, the only
// variable which the NIQ mods will add to serverinfo is niq_version because
// (especially with NIQBOTS), it is too easy to have too many characters in
// the serverinfo string which will prevent the server from being able to
// register itself with a master server (the famous "Info string length
// exceeded" error message). This is fine because I will add the ninfo command
// so that client's can see the values of the following, and the "sv ninfo"
// command for the server to see all of these.
//
// See q_shared.h for a list of CVAR flags for changing this behaviour.

void niq_initcvars()
{
	// can't be changed from the console (really botjust there for GameSpy Tab)
    niq_version         = gi.cvar ("niq_version",       NIQ_DEF_VERSION,        CVAR_SERVERINFO | CVAR_NOSET);
//TBD:     niq_game            = gi.cvar ("niq_game",          -1,                     CVAR_SERVERINFO | CVAR_NOSET);

	// latched vars do not take effect immediately -- game has to be restarted
    niq_enable          = gi.cvar ("niq_enable",        NIQ_DEF_ENABLE,         CVAR_LATCH);
    niq_ebots           = gi.cvar ("niq_ebots",         NIQ_DEF_EBOTS,          CVAR_LATCH);
    niq_handicap        = gi.cvar ("niq_handicap",      NIQ_DEF_HANDICAP,       CVAR_LATCH);
    niq_allmaps         = gi.cvar ("niq_allmaps",       NIQ_DEF_ALLMAPS,        CVAR_LATCH);
    niq_tractor         = gi.cvar ("niq_tractor",       NIQ_DEF_TRACTOR,        CVAR_LATCH);
    niq_hooksky         = gi.cvar ("niq_hooksky",       NIQ_DEF_HOOKSKY,        CVAR_LATCH);

    niq_blk1            = gi.cvar ("niq_blk1",          "0",                    CVAR_LATCH);
    niq_blk2            = gi.cvar ("niq_blk2",          "0",                    CVAR_LATCH);

// BUNGLE Standard Logging Support
    niq_logfile         = gi.cvar ("stdlogfile",        NIQ_DEF_LOGFILE,        CVAR_LATCH);
// BUNGLE
    
    // All of the following can be changed at any time and the new value
	// will be used right away but they don't show up with serverinfo.
    niq_sbhp            = gi.cvar ("niq_sbhp",          NIQ_DEF_SBHP,           0);

    niq_auto            = gi.cvar ("niq_auto",          NIQ_DEF_AUTO,           0);

    niq_weapsecs        = gi.cvar ("niq_weapsecs",      NIQ_DEF_WEAPSECS,       0);
    niq_weapkills       = gi.cvar ("niq_weapkills",     NIQ_DEF_WEAPKILLS,      0);
    niq_weaprand        = gi.cvar ("niq_weaprand",      NIQ_DEF_WEAPRAND,       0);
    niq_weapall         = gi.cvar ("niq_weapall",       NIQ_DEF_WEAPALL,        0);

    niq_hlthinc         = gi.cvar ("niq_hlthinc",       NIQ_DEF_HLTHINC,        0);
    niq_hlthmax         = gi.cvar ("niq_hlthmax",       NIQ_DEF_HLTHMAX,        0);
    niq_killpts         = gi.cvar ("niq_killpts",       NIQ_DEF_KILLPTS,        0);
    niq_kildpts         = gi.cvar ("niq_kildpts",       NIQ_DEF_KILDPTS,        0);
    niq_suicpts         = gi.cvar ("niq_suicpts",       NIQ_DEF_SUICPTS,        0);

    niq_sndhlth         = gi.cvar ("niq_sndhlth",       NIQ_DEF_SNDHLTH,        0);
    niq_sndwarn         = gi.cvar ("niq_sndwarn",       NIQ_DEF_SNDWARN,        0);
    niq_sndswitch       = gi.cvar ("niq_sndswitch",     NIQ_DEF_SNDSWITCH,      0);
    niq_msgswitch       = gi.cvar ("niq_msgswitch",     NIQ_DEF_MSGSWITCH,      0);

    niq_sblines			= gi.cvar ("niq_slines",		NIQ_DEF_SBLINES,		0);
    niq_sbdebug         = gi.cvar ("niq_sbdebug",       NIQ_DEF_SBDEBUG,        0);
    niq_sbwide          = gi.cvar ("niq_sbwide",        NIQ_DEF_SBWIDE,         0);
    niq_sbmini          = gi.cvar ("niq_sbmini",        NIQ_DEF_SBMINI,         0);
    niq_inttime         = gi.cvar ("niq_inttime",       NIQ_DEF_INTTIME,        0);

    niq_playerid        = gi.cvar ("niq_playerid",      NIQ_DEF_PLAYERID,       0);
} // niq_initcvars

/////////////////////////////////////////////////////////////////////////////

int niq_getcurrentweaponindex()
{
    return niqlist[game.niq_curweapon].nListIndexWeapon;
}

/////////////////////////////////////////////////////////////////////////////
// niq_initall:
//
// Reinitializes all of NIQ. This is done each time a new game is started
// (i.e. if the map is changed or a save game is loaded, but not when going
// to the next level). 
//
// Fetches the itemlist index for each weapon in the niqlist and stores it
// in niqlist for quicker access. Then initializes the current weapon either
// superblaster / random / of first in given list.
//
// As clients connect, they always start out with the current NIQ weapon. When
// the weapon timer runs out, the current weapon is changed and all clients
// are changed at this time. Any time a client spawns after dying or when joining
// the server, he starts out with nAmmoOnSpawn units of ammo. When the weapon is
// changed, clients start out with nAmmoInitial units of ammo which is usually
// much more. This is to prevent clients from killing themselves or reconnecting
// on purpose in order to get more ammo (they will also be discouraged from this
// by losing 2 frags for killing themselves and 1 frag for getting killed).

void niq_initall()
{
    int nWeap;

#ifdef _DEBUG
    gi.dprintf("\nIn NIQInitAll\n");
#endif

    niq_initcvars();

    game.niq_curweapon     =    0;
    game.niq_monsterkills  =    0;
    game.niq_wlindex       =    0;
    game.niq_wsecs		   =  0.0;
    game.niq_numweapons    = MAXNIQWEAPONS;

	niq_patchversionstrings();

	// load motd file if any
	niq_loadmotdfile();

	if(!niq_enable->value)
		return;

	// anything after this line is NOT executed if niq_enable=0

    niq_zapitems();

    // load weapon list if any
    niq_loadweaponlist();

    // load ammo list if any
    niq_loadammolist();

#ifdef NIQSP
    // load monster health list if any
    if(!deathmatch->value)
        niq_loadmonsterlist();
#endif

    // always reseed the random number generator so things aren't always in the same order
    srand( (unsigned)time( NULL ) );

    for(nWeap=1; nWeap<=MAXNIQWEAPONS; nWeap++)
        {
        niqlist[nWeap].nListIndexWeapon = ITEM_INDEX(FindItem(niqlist[nWeap].szName));

        if(niqlist[nWeap].szAmmo[0] != '\0')
            niqlist[nWeap].nListIndexAmmo   = ITEM_INDEX(FindItem(niqlist[nWeap].szAmmo));
        }

    // initialize current weapon -- just sets game.niq_curweapon since no clients at present
    niq_selectcurrentweapon(true);

    // initialize weapon switch timer to -1 (will be set in SpawnEntities)
	game.niq_wsecs = 0.0;
} // niq_initall

/////////////////////////////////////////////////////////////////////////////

void niq_initdefaults(edict_t* ent)
{
	gclient_t *cl;
	cl = ent->client;
		
    // niq: initialize things once here
    cl->pers.niq_fDamage		= 1.0;
    cl->resp.niq_fDamageTotal	= 0.0;
    cl->resp.niq_fDamageTics	= 0;
//	cl->resp.niq_helpmode		= 0;				// no help --> get motd at each level change

#ifdef _DEBUG
	// nice way to break on bot being setup here
	if(ent->bot_client)
		cl->pers.niq_isbot		= 1;
	else
		cl->pers.niq_isbot		= 0;
#else
	cl->pers.niq_isbot			= ent->bot_client;
#endif

	cl->pers.niq_hudmode		= NIQ_HUD_0;		// no mini-SB
	cl->pers.niq_sbmode			= NIQ_SB_DM;	    // default to DM scoreboard

	if(cl->pers.niq_isbot)
		return;

//	cl->resp.niq_helpmode		= 1;				// force the MOTD up initially (for non-bots)

	if(deathmatch->value)
		{
		cl->pers.niq_sbmode		= NIQ_SB_320A;		// default to first NIQ scoreboard

		// set up default HUD, scoreboard for deathmatch
		if(ctf->value)
			{
			cl->pers.niq_hudmode= NIQ_HUD_4;		// default mini-SB (rank, name, score)

			cl->pers.niq_sbmode	= NIQ_SB_CTF;		// default to regular CTF scoreboard
			}
		else
			{
			cl->pers.niq_hudmode= NIQ_HUD_2;		// default mini-SB (rank, pph, score)

			if(niq_handicap->value)
				cl->pers.niq_sbmode	= NIQ_SB_320D;	// default to 320x240 DF scoreboard
			}
		}
} // niq_initdefaults

/////////////////////////////////////////////////////////////////////////////

int niq_strncmp(char* str1, char* str2)
{
    return strncmp(str1, str2, strlen(str2));
}

/////////////////////////////////////////////////////////////////////////////

char* niq_hud_addmaintitles(int nMode, char* pszDst)
{
    char  szTemp1[200];
    char  szTemp2[200];
	int   nOffX=0;
	int   nOffY=NIQ_HUD_YOFFSET;
	int   nClients = niq_getnumclients();

	if(!pszDst)
		return NULL;

	if(nMode == NIQ_HUD_1)
		sprintf(szTemp1, "%2d Score", nClients);
	else if(nMode == NIQ_HUD_2)
		sprintf(szTemp1, "%2d PPH Score", nClients);
	else if(nMode == NIQ_HUD_3)
		sprintf(szTemp1, "%2d Name PPH Score", nClients);
	else if(nMode == NIQ_HUD_4)
		sprintf(szTemp1, "%2d Name Score", nClients);

	sprintf(szTemp2, "xl %d yt %d string \"%s\"",  nOffX, nOffY, szTemp1);
	strcat(pszDst, szTemp2);

	return &pszDst[strlen(szTemp2)];
} // niq_hud_addmaintitles

/////////////////////////////////////////////////////////////////////////////
// 512		nOffX = -96;
// 640		nOffX = -160

char* niq_addplayerstats(gclient_t* cl, qboolean bGreen, int nLine, int nMode, int nRank, char* pszDst)
{
    char szTemp1[200] = ""; 
    char szTemp2[200] = ""; 
	int nOffX=0;
	int nOffY=NIQ_HUD_YOFFSET + 10*(nLine+2); // line 0 is the title

	if(!pszDst)
		return NULL;

	if(nMode == NIQ_HUD_1)
		// rank, score
	    sprintf(szTemp1, "%2d %5.1f", nRank, cl->resp.score);
	else if(nMode == NIQ_HUD_2)
		// rank, pph, score
	    sprintf(szTemp1, "%2d %3d %5.1f", nRank, niq_getPPH(cl), cl->resp.score);
	else if(nMode == NIQ_HUD_3)
		{
		// rank, name, pph, score
		char szName[33];
		niq_getname(NUM_SBHUDNAMECHARS, cl, szName, true);

	    sprintf(szTemp1, "%2d %-*s %3d %5.1f", nRank, NUM_SBHUDNAMECHARS, szName, niq_getPPH(cl), cl->resp.score);
		}
	else if(nMode == NIQ_HUD_4)
		{
		// rank, name, score
		char szName[33];
		niq_getname(NUM_SBHUDNAMECHARS, cl, szName, true);

	    sprintf(szTemp1, "%2d %-*s %5.1f", nRank, NUM_SBHUDNAMECHARS, szName, cl->resp.score);
		}

	if(ctf->value)
		{
		// show team color letter after score on mini-SB
		if(cl->resp.ctf_team == CTF_TEAM1)
			strcat(szTemp1, " R");
		else if(cl->resp.ctf_team == CTF_TEAM2)
			strcat(szTemp1, " B");
		}

#ifdef EBOTS
	//  !! there are other ways of having teamplay:
	//	EB uses this, but you can also just set teamplay by model/skin
	//  which I've never even played, let alone tested.
	if(niq_ebots->value)
	{
		if(teamplay->value && cl->team)
			{
			// stick first letter of team name after score on mini-SB
			strncat(szTemp1, cl->team->teamname, 1);
			}
	}
#endif

	// we don't need xl's here since the entire menu is at xl=0
	if(bGreen)
        // draw self in green
		sprintf(szTemp2, "yt %d string2 \"%s\"", nOffY, szTemp1);
	else
		sprintf(szTemp2, "yt %d string \"%s\"", nOffY, szTemp1);

	strcat(pszDst, szTemp2);

	return &pszDst[strlen(szTemp2)];
} // niq_addplayerstats

/////////////////////////////////////////////////////////////////////////////

void niq_showhudtext(edict_t* ent)
{
	if(!deathmatch->value)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: showhudtext called when in sp/coop\n");
#endif
		return;
		}

	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_showhudtext called for a bot!\n");
#endif
		return;
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (szHUDLabelStr);
	gi.unicast(ent, false);
} // niq_showhudtext

/////////////////////////////////////////////////////////////////////////////
// niq_showhudsb:
//
// Sorts the players then displays the first-place player, the players above
// and below the given client, and the last place player, using scoreboard-
// style output. The given client's stats are printed in green,

void niq_showhudsb(edict_t* ent)
{
	char	szHUDStr[NIQ_MAXHUDCHARS];
	char*   pszEnd;
	int		sorted[MAX_CLIENTS];
    int		myIndex = -1;    
	int		total;
	int		hudmode;
	int		nClient;
	int		nRank;
	float	fLastScore;
	int		nPrevClient;
	int		nLine;
	gclient_t* cl;

#ifdef _DEBUG
	if(!deathmatch->value)
		{
		gi.dprintf("NIQ: showhudsb called when in sp/coop\n");
		return;
		}
#endif

	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_showhudsb called for a bot!\n");
#endif
		return;
		}

	// check for server turning of mini-SB
	if(!niq_sbmini->value && ent->client->pers.niq_hudmode)
		{
		ent->client->pers.niq_hudmode = NIQ_HUD_0;
		niq_showhudtext(ent);
		return;
		}

	if(ent->client->showhelp || ent->client->showscores || ent->client->resp.niq_helpmode)
		return;

	szHUDStr[0]	= 0;

	hudmode = ent->client->pers.niq_hudmode;

	niq_sortclients(sorted, ent, &total, &myIndex);

	pszEnd = niq_hud_addmaintitles(hudmode, szHUDStr);

	// ugly hacks
	nLine		= 0;
	nPrevClient	= 0;
	nRank		= 0;
	fLastScore	= 9999999.0;

	// go through list of sorted clients, displaying any applicable ones on the HUD
	for(nClient=0; nClient<total; nClient++)
		{
		int nIndex = sorted[nClient];

#ifdef _DEBUG

#ifdef EBOTS
		// bots get added at maxclients-1, maxclients-2...
		if( ( niq_ebots->value && (nIndex < 0 || nIndex >= game.maxclients)) ||
		    (!niq_ebots->value && (nIndex < 0 || nIndex >= niq_getnumclients())) )
#else
		if(nIndex < 0 || nIndex >= niq_getnumclients())
#endif
			{
			gi.dprintf("NIQ: bad index in niq_showhudsb\n");
			continue;
			}
#endif

		// find the given client
		cl = &game.clients[nIndex];
		if(!cl)
			continue;

		if(cl->resp.score < fLastScore)
			{
			nRank++;
			fLastScore = cl->resp.score;
			}

		if( (total   <= 5)							||	// only 5 clients ==> everyone fits
		    (nClient == 0)							|| 	// #1 is always shown
		    (nClient == (myIndex-1))				||  // show client above me
		    (nClient == myIndex)					||	// show me
		    (nClient == (myIndex+1))				||	// show client below me
		    (nClient == 1 && myIndex==(total-1))	||  // show #2 if I'm last
		    (nClient == 2 && myIndex==(total-1))	||  // show #3 if I'm last
		    (nClient == 1 && myIndex==(total-2))	||  // show #2 if I'm 2nd last
		    (nClient == 2 && myIndex==0)			||  // show #3 if I'm 1st
		    (nClient == 3 && myIndex<=1)			||  // show #4 if I'm 1st or 2nd
		    (nClient == (total-1)) )					// show last score
			{
			if(nClient > (nPrevClient+1))
				nLine++;	// space out 1st and last from rest of group?

			pszEnd = niq_addplayerstats(cl, (nClient==myIndex), nLine, hudmode, nRank, pszEnd);
			nPrevClient = nClient;
			nLine++;
			}
		}

	// add label text for RHS HUD bar
	strcat(szHUDStr, szHUDLabelStr);

	gi.WriteByte (svc_layout);
	gi.WriteString (szHUDStr);
	gi.unicast(ent, false);
} // niq_showhudsb

/////////////////////////////////////////////////////////////////////////////
// niq_updatescreen:
//
// Call this function to immediately update current scoreboard/help menu or
// the HUD, when these are changed.

void niq_updatescreen(edict_t* ent)
{
	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_updatescreen called for a bot!\n");
#endif
		return;
		}

 	if(ent->client->showhelp)
		return;

	if(ent->client->showscores)
		{
		if (ent->client->menu) 
			{
			PMenu_Update(ent);

			gi.unicast (ent, false);
			} 
		else
			niq_deathmatchscoreboardmessage (ent, NULL, false);

		return;
		}

	if(ent->client->resp.niq_helpmode)
		{
		niq_help(ent);
		return;
		}

	if(niq_enable->value)
		{
		// only show these if NIQ is enabled
		if(ent->client->pers.niq_hudmode)
			niq_showhudsb(ent);				// show mini-SB + RHS HUD bar
		else
			niq_showhudtext(ent);			// show RHS HUD bar 
		}
} // niq_updatescreen
	
/////////////////////////////////////////////////////////////////////////////

void niq_setstats(edict_t *ent)
{
	gitem_t		*item;
	int			weapindex;

 	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_setstats called for a bot!\n");
#endif
		return;
		}

	// health
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH]		= ent->health;

	if (!ent->client->ammo_index || (ent->svflags & SVF_NOCLIENT))
    	{
		ent->client->ps.stats[STAT_AMMO_ICON]	= 0;
		ent->client->ps.stats[STAT_AMMO]		= 0;
	    }
	else
    	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);

        // 08/05/98 niq: just show 999 when infinite ammo set
        if ( (int)dmflags->value & DF_INFINITE_AMMO )
    		ent->client->ps.stats[STAT_AMMO] = 999;
        else
    		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	    }

	// pickup message
	// (niq): this is set to picked up item in the Touch_Item function (needed in sp/coop for breather/envsuit, ctf for flags)
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	weapindex = -1;
	if (ent->client->pers.weapon)
		{
		weapindex = gi.imageindex (ent->client->pers.weapon->icon);

	    // niq: always show current weapon icon where armor icon normally goes
		ent->client->ps.stats[STAT_ARMOR_ICON] = weapindex;
		}

#ifdef NIQSP
	// niq: not needed for deathmatch (nothing to select except the current
	// weapon). In sp/ccop just show if not same as current weapon (e.g. 
	// breather/env.suit when available).
    if(!deathmatch->value)
        {
	    // selected item
	    ent->client->ps.stats[STAT_SELECTED_ICON] = 0;

	    if (ent->client->pers.selected_item != -1)
			{
			int itemindex = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);
			if(weapindex != itemindex)
		    	ent->client->ps.stats[STAT_SELECTED_ICON] = itemindex;
			}

    	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

		// timers for breather/envsuit
		if (ent->client->enviro_framenum > level.framenum)
			{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
			}
		else if (ent->client->breather_framenum > level.framenum)
			{
			ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
			ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
			}
		else
			{
			ent->client->ps.stats[STAT_TIMER_ICON] = 0;
			ent->client->ps.stats[STAT_TIMER] = 0;
			}
        }

    if(!deathmatch->value && !niq_auto->value)
        {
        // in sp/coop, with autoinc not set, clients need xx kills before next switch will happen
        // 08/05/98: only show if at least 2 weapons (otherwise will always 'switch' to same weapon)
        if(game.niq_numweapons > 1 && niq_weapkills->value >=0)
            ent->client->ps.stats[STAT_ARMOR] = niq_getkillsneededforswitch(ent);
        else
            ent->client->ps.stats[STAT_ARMOR] = 0;
        }
    else
#endif
        {        
        // niq: show weapon countdown timer beside weapon icon (only items which can be selected)
        // 08/05/98: only show if AT LEAST 2 WEAPONS
	    if (game.niq_numweapons > 1 && game.niq_wsecs > level.time)
            ent->client->ps.stats[STAT_ARMOR] = (int)(game.niq_wsecs - level.time + 0.5);
        else
            ent->client->ps.stats[STAT_ARMOR] = 0;
        }

    if(deathmatch->value)
        {
        // niq: show current player rank in lower-right corner

        // we don't need to sort the scores, just figure out how many scores
        // are greater than the client's score, which is less expensive
        
        int i;
        int nRank = 1;
        float fMyScore = ent->client->resp.score;
    	edict_t	*cl_ent;

    	for (i=0 ; i<game.maxclients ; i++)
        	{
    		cl_ent = g_edicts + 1 + i;
	    	if (!cl_ent->inuse)
		    	continue;

            if(cl_ent != ent && game.clients[i].resp.score > fMyScore)
                nRank++;
            }

		// using sb style output to the HUD means blocking Esc key to get to options
		// if client asks for more info on 'HUD' and nothing else up, show it (every 8/10ths of a second?)
		if(	!ent->client->showhelp && !ent->client->showscores && !ent->client->resp.niq_helpmode )
			{
			// update mini sb every 8/10ths
			if(ent->client->pers.niq_hudmode)
				{
				if(!(level.framenum & 7))
					niq_showhudsb(ent);
				}
			else
				{
				// update labels every 32/10ths
				if(!(level.framenum & 31))
					niq_showhudtext(ent);
				}
			}

		if(!ent->client->showscores && !ent->client->resp.niq_helpmode)
			{
			// dummy stat which controls showing frags, pph and rank
	    	ent->client->ps.stats[STAT_DUMMY] = 1;

			// show frags (niq: score is float, HUD is int so round it)
		    if(ent->client->resp.score < 0.0)
		    	ent->client->ps.stats[STAT_FRAGS]	= (int)(ent->client->resp.score-0.5); 
		    else
		    	ent->client->ps.stats[STAT_FRAGS]	= (int)(ent->client->resp.score+0.5);

			// show client's PPH (Points Per Hour)
			ent->client->ps.stats[STAT_PPH]			= niq_getPPH(ent->client);

			// show client's current rank
			ent->client->ps.stats[STAT_RANK]		= nRank;
			}
		else
			{
	    	ent->client->ps.stats[STAT_DUMMY]		= 0;
			}
        }

	// layouts
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	// anything which is put on the screen 'scoreboard-style' has to control these flags
	if (deathmatch->value)
	    {
		// with NIQ, there is always some SB stuff on-screen
		ent->client->ps.stats[STAT_LAYOUTS]		|= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    	}
#ifdef NIQSP
	else
	    {
		if (ent->client->showscores || ent->client->showhelp || ent->client->resp.niq_helpmode)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    	}
#endif

#ifdef NIQSP
	// niq: STAT_HELPICON only used in sp/coop to show help computer
	if(!deathmatch->value)
		{
		// help icon / current weapon if not shown
		if (ent->client->resp.helpchanged && (level.framenum&8) )
			ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
		else
			ent->client->ps.stats[STAT_HELPICON] = 0;
		}
#endif

	if(!ctf->value)
		{
		// If i use 0, name is the name of the map, so CTF must clear something 
		// somewhere. 255 seems to work OK and we should never have this many clients?
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 255;

		// niq: don't allow player id if a scoreboard/menu etc. is up
		if(ent->client->resp.id_state && !ent->client->showhelp && !ent->client->showscores && (!ent->client->resp.niq_helpmode || (ent->client->resp.niq_helpmode == NIQ_MAXHELPSCREEN)))
			CTFSetIDView(ent);
		}
	else
		SetCTFStats(ent);
} // niq_setstats

/////////////////////////////////////////////////////////////////////////////

#ifdef NIQSP

qboolean niq_isremoveditem(edict_t* ent)
{
    int i;
	gitem_t	*item;

    if(deathmatch->value)
        return false;

	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;

        // these are the *only* items which will stay in a sp/coop game
		if (!strcmp(item->classname, ent->classname))
            {
   		    if(!niq_strncmp(ent->classname, "key_")              ||
    		   !niq_strncmp(ent->classname, "item_breather")     ||
    		   !niq_strncmp(ent->classname, "item_enviro"))
                return false;   // these items are kept in the game
            else
                return true;
            }
	}

    return false;
}

/////////////////////////////////////////////////////////////////////////////
// niq_findtriggers:
//
// Makes a list of all the targets in the given list of entities which belong
// to items which will be removed from the game, so we will know what targets
// (monsters, doors, secrets) to trigger immediately.

void niq_findtriggers(char *entities)
{
	char*   com_token;
	edict_t	ent;

    niq_nNumTriggers = 0;
    if(deathmatch->value)
        return;

    // parse ents without changing anything
	while (1)
    	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;

		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

        memset(&ent, 0, sizeof(ent));
        G_InitEdict(&ent);

		entities = ED_ParseEdict (entities, &ent);

        if(ent.target)
            {
            // make a list of all removed items which trigger events

    		if(niq_isremoveditem(&ent))
                {
                // item is one which will be removed

                if(niq_nNumTriggers >= NIQMAXTRIGGERS)
                    gi.dprintf("NIQ: too many triggers found!\n");
                else if(strlen(ent.target) >= MAXTARGETLEN)
                    gi.dprintf("NIQ: target is too long to store!\n");
                else
                    {
#ifdef _DEBUG
                    gi.dprintf("NIQ: item %s has target %s\n", ent.classname, ent.target);
#endif
                    strcpy(niq_nTriggers[niq_nNumTriggers++], ent.target);
                    }
                }
            }
	    }	

#ifdef _DEBUG
    gi.dprintf("targets: %d\n", niq_nNumTriggers);
#endif

} // niq_findtriggers

/////////////////////////////////////////////////////////////////////////////
// niq_istriggered:
//
// Called after niq_findtriggers has been executed. Returns true if the given
// item is normally triggered by an item which will no longer be in the game,
// e.g. the 2 soldiers on base1 who only appear when the shotgun is picked up
// or the doors on base2 which only open when the supershotgun is picked up.

qboolean niq_istriggered(edict_t* ent)
{
    if(niq_nNumTriggers < 1)
        return false;

    if(ent->targetname)
        {
        qboolean bMatch = false;
        int i=0;

        while(!bMatch && i<niq_nNumTriggers)
            {
            if(!strcmp(niq_nTriggers[i], ent->targetname))
                bMatch = true;
            else
               i++;
            }

        if(bMatch)
            {
#ifdef _DEBUG
            gi.dprintf("ent %s is triggered by removed item %s\n", ent->classname, ent->targetname);
#endif
            return true;
            }
        }

    return false;
} // niq_istriggered

/////////////////////////////////////////////////////////////////////////////

qboolean niq_free_triggered_entity(edict_t* ent)
{
    if(niq_istriggered(ent))
        {
        // !! should anything else go here?
        // !! should anything not go here -- try more maps...
        if(!strcmp(ent->classname, "func_door") || !strcmp(ent->classname, "target_secret") || !strcmp(ent->classname, "func_explosive"))
            return true;
#ifdef _DEBUG
        else
            gi.dprintf("  not freeing triggered item %s\n", ent->classname);
#endif
        }
        
    // item stays
    return false;
} // niq_free_triggered_entity
#endif

/////////////////////////////////////////////////////////////////////////////
// niq_adjustdamage:
//
// Have to make sure the overhead of calling this isn't too high. If it is,
// use a macro, or do inline.

void niq_adjustdamage(edict_t *targ, edict_t *attacker, int* damage)
{
	// niq: in handicap mode?
	if(!niq_handicap->value)
		return;

	if((targ->client || (targ->svflags & SVF_MONSTER)) && attacker->client && (targ != attacker))
		{
		// niq: target is a client, attacker is a client, and they aren't the same?
		if(attacker->client->pers.niq_fDamage != 1.0)
			{
			// niq: attacker is using a handicap, so apply it
			*damage = (int)((*damage * attacker->client->pers.niq_fDamage) + 0.5);

		    // make sure damage is at least 1
		    if(*damage <= 0)
		        *damage = 1;
			}
		}
} // niq_adjustdamage

/////////////////////////////////////////////////////////////////////////////

void niq_increase_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(!niq_handicap->value)
        {
 		gi.cprintf (ent, PRINT_HIGH, "NIQ handicapping is disabled.\n");
        return;
        }

    if(ent->client)
        {
        // clients can only increase the damage that they do past 1.0 if cheats are enabled
	    if ((ent->client->pers.niq_fDamage >= 0.999) && (deathmatch->value || coop->value) && !sv_cheats->value)
    	    {
    		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to increase damage past 1.0.\n");
	    	return;
	        }

        ent->client->pers.niq_fDamage += 0.1;

        // make sure we don't go over maximum
        if(ent->client->pers.niq_fDamage > (NIQ_MAXDAMAGE-0.001))
            ent->client->pers.niq_fDamage = NIQ_MAXDAMAGE;

  		gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor set to: %0.1f\n", ent->client->pers.niq_fDamage);

		if(ent->client->showscores)
			niq_deathmatchscoreboardmessage (ent, NULL, false);
        }
}

/////////////////////////////////////////////////////////////////////////////

void niq_decrease_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(!niq_handicap->value)
        {
 		gi.cprintf (ent, PRINT_HIGH, "NIQ handicapping is disabled.\n");
        return;
        }

    // clients can always decrease the damage that they do
    if(ent->client)
        {
        ent->client->pers.niq_fDamage -= 0.1;

        // make sure we don't go under minimum
        if(ent->client->pers.niq_fDamage < (NIQ_MINDAMAGE+0.001))
            ent->client->pers.niq_fDamage = NIQ_MINDAMAGE;

  		gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor set to: %0.1f\n", ent->client->pers.niq_fDamage);

		if(ent->client->showscores)
			niq_deathmatchscoreboardmessage (ent, NULL, false);
        }
}

/////////////////////////////////////////////////////////////////////////////

void niq_max_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(!niq_handicap->value)
        {
 		gi.cprintf (ent, PRINT_HIGH, "NIQ handicapping is disabled.\n");
        return;
        }

    // clients can only increase the damage that they do if cheats are enabled
	if ((deathmatch->value || coop->value) && !sv_cheats->value)
    	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to select maximum damage.\n");
		return;
	    }

    if(ent->client)
        {
        ent->client->pers.niq_fDamage = NIQ_MAXDAMAGE;

    	gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor set to: %0.1f\n", ent->client->pers.niq_fDamage);

		if(ent->client->showscores)
			niq_deathmatchscoreboardmessage (ent, NULL, false);
        }
}

/////////////////////////////////////////////////////////////////////////////

void niq_min_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(!niq_handicap->value)
        {
 		gi.cprintf (ent, PRINT_HIGH, "NIQ handicapping is disabled.\n");
        return;
        }

    // clients can always decrease the damage that they do
    if(ent && ent->client)
        {
        ent->client->pers.niq_fDamage = NIQ_MINDAMAGE;

  		gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor set to: %0.1f\n", ent->client->pers.niq_fDamage);

		if(ent->client->showscores)
			niq_deathmatchscoreboardmessage (ent, NULL, false);
        }
}

/////////////////////////////////////////////////////////////////////////////

void niq_reset_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(!niq_handicap->value)
        {
 		gi.cprintf (ent, PRINT_HIGH, "NIQ handicapping is disabled.\n");
        return;
        }

    // clients can always decrease the damage that they do
    if(ent->client)
        {
        ent->client->pers.niq_fDamage = 1.0;

  		gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor set to: %0.1f\n", ent->client->pers.niq_fDamage);

		if(ent->client->showscores)
			niq_deathmatchscoreboardmessage (ent, NULL, false);
        }
}

/////////////////////////////////////////////////////////////////////////////

void niq_show_damage(edict_t* ent)
{
    if(!ent)
        return;

    if(ent->client)
  		gi.cprintf (ent, PRINT_HIGH, "NIQ damage factor is: %0.1f\n", ent->client->pers.niq_fDamage);
}

/////////////////////////////////////////////////////////////////////////////

static void niq_addsbline(char** strdst, char* stradd)
{
    int nStrLen = strlen(stradd);

#ifdef _DEBUG
	// check for menu being too long
	if((strlen(g_helpstr) + nStrLen) >= NIQMAXHELPSIZE)
		{
		gi.dprintf("niq_addsbline: menu is too long!\n");
		return;
		} 
#endif

	// check for lines which need to be made shorter
    if(nStrLen <= NIQ_SBCHARS320)
        {
        // center the text
	    sprintf(*strdst, "xv %-3d yv %-3d string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-nStrLen), g_nSBLineNum*NIQ_SCOREBOARD_LINESIZE, stradd);
        *strdst += 23+nStrLen;
        }
    else
		{
		// string is too long for 320x240, but truncate it and show it anyway
		nStrLen = NIQ_SBCHARS320;
		stradd[NIQ_SBCHARS320] = '\0';
	    sprintf(*strdst, "xv %-3d yv %-3d string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-nStrLen), g_nSBLineNum*NIQ_SCOREBOARD_LINESIZE, stradd);
        *strdst += 23+nStrLen;

        gi.dprintf("NIQ: string is too long in niq_addsbline!\n");
		}

	g_nSBLineNum++;
} // niq_addsbline

/////////////////////////////////////////////////////////////////////////////
// niq_toggle_scoreboard:

static void niq_toggle_scoreboard(edict_t* ent)
{
	if (!ent->client)
		return;

    if(ctf->value)
		{
		if(++ent->client->pers.niq_sbmode > NIQ_MAXSCOREBOARD)
			ent->client->pers.niq_sbmode = 0;
		}
	else
		{
		if(++ent->client->pers.niq_sbmode > NIQ_MAXSCOREBOARD-1)
			ent->client->pers.niq_sbmode = 0;
		}

	// 30/06/98: if handicapping is disabled, just skip over the scoreboards which show df?
	if(!niq_handicap->value && (ent->client->pers.niq_sbmode == NIQ_SB_320D || ent->client->pers.niq_sbmode == NIQ_SB_320E))
		{
		gi.cprintf (ent, PRINT_HIGH, "niq_handicap=0: skipping scoreboards!\n");
		ent->client->pers.niq_sbmode = NIQ_SB_512A;
		}

	// just in case, niq_sbwide=0 can be used to disable the 512 and 640 scoreboards
	if(!niq_sbwide->value && ent->client->pers.niq_sbmode >= NIQ_SB_512A && ent->client->pers.niq_sbmode <= NIQ_SB_640A)
		{
		gi.cprintf (ent, PRINT_HIGH, "niq_sbwide=0: skipping scoreboards!\n");
		ent->client->pers.niq_sbmode = NIQ_SB_DM;
		}

	// 640x480 scoreboard is for debug builds only -- so skip to regular SB
	if(!niq_sbdebug->value && ent->client->pers.niq_sbmode == NIQ_SB_640A)
		{
		gi.cprintf (ent, PRINT_HIGH, "niq_sbdebug=0: skipping scoreboard!\n");
		ent->client->pers.niq_sbmode++;
		}

	if(ent->client->pers.niq_sbmode == NIQ_SB_512A)
		gi.cprintf (ent, PRINT_HIGH, "Scoreboard needs at least 512x384!\n");
	else if(ent->client->pers.niq_sbmode == NIQ_SB_640A)
		gi.cprintf (ent, PRINT_HIGH, "Scoreboard needs at least 640x480!\n");

	// show change right away
	niq_deathmatchscoreboardmessage (ent, NULL, false);
} // niq_toggle_scoreboard

/////////////////////////////////////////////////////////////////////////////
// niq_toggle_hud:
//
// This controls a scoreboard-style HUD. I really want to be able to show
// each client where he stands, i.e. what his score/rank/pph is, what the
// score/rank/pph of the player above and below him are and what the s/r/p
// of the top player are. Special cases: if the client is #1, we show the next
// 3 players, if he is #2, we show #1, #3, #4, if he is last, we show the 3
// players above him. The client is shown in green, so its always easy for the
// client to identify himself.
//
// HUD numbers are simply too large to show all this information and might
// be less efficient. Also, using a scoreboard-style HUD will let us show
// each client's score in xxx.x format, which would be hard, if not impossible
// to do with the standard HUD approach (its also tricky to add strings to
// specific locations using the HUD code).
//
// There are 5? HUDS (the client's score and rank are always shown in the UR/LR):
//
//	0) Shows nothing extra.
//  1) Shows rank and score for 4 players including client on left.
//  2) Shows rank, score and PPH for 4 players including client on left.
//  3) Shows name, rank, and score for 4 players including client on left.
//  4) Shows name, rank, score and PPH for 4 players including client on left.

static void niq_toggle_hud(edict_t* ent)
{
	if (!ent->client)
		return;

	if(!niq_sbmini->value)
		ent->client->pers.niq_hudmode = NIQ_HUD_0;
	else if(++ent->client->pers.niq_hudmode > NIQ_MAXHUD)
		ent->client->pers.niq_hudmode = NIQ_HUD_0;

	// update right away?
	if(!ent->client->showhelp && !ent->client->showscores && !ent->client->resp.niq_helpmode)
		niq_updatescreen(ent);
} // niq_toggle_hud

/////////////////////////////////////////////////////////////////////////////

void niq_showinfo(char** pszDst, char* pszStr, qboolean bEnabled, qboolean bCondition)
{
	char szTemp[128];

	if(!bCondition)
		sprintf(szTemp, "%-36s %3s", pszStr,  "NA");
	else if(bEnabled)
		sprintf(szTemp, "%-36s %3s", pszStr, "YES");
	else
		sprintf(szTemp, "%-36s %3s", pszStr,  "NO");

    niq_addsbline(pszDst, szTemp);
} // show_info

/////////////////////////////////////////////////////////////////////////////

void niq_showinfoint(char** pszDst, char* pszStr, int nValue)
{
	char szTemp[128];

	sprintf(szTemp, "%-34s %5d", pszStr, nValue);
    niq_addsbline(pszDst, szTemp);
} // niq_showinfoint
	
/////////////////////////////////////////////////////////////////////////////

void niq_showinfoflt(char** pszDst, char* pszStr, float fValue)
{
	char szTemp[128];

	sprintf(szTemp, "%-34s %5.1f", pszStr, fValue);
    niq_addsbline(pszDst, szTemp);
} // niq_showinfoflt
	
/////////////////////////////////////////////////////////////////////////////
// niq_showmotd:
//
// MOTD is partly from a file, partly hard-coded here.

void niq_motd(edict_t *ent)
{
	int nLine=0;

    niq_addsbline(&g_helpptr, szWelcomeStr1NIQ);
    niq_addsbline(&g_helpptr, szWelcomeStr2NIQ);
#ifdef EBOTS
    niq_addsbline(&g_helpptr, szWelcomeStr3NIQ);
#endif
    niq_addsbline(&g_helpptr, szWelcomeStr4NIQ);
    g_nSBLineNum++;

	while(nLine<NIQ_MAXMOTDLINES && motdlines[nLine][0])
		{
		niq_addsbline(&g_helpptr, motdlines[nLine]);
		nLine++;
		}
    g_nSBLineNum++;

	if(!niq_enable->value)		
	    niq_addsbline(&g_helpptr, "#### NIQ features are disabled ####");

    niq_addsbline(&g_helpptr, szContact1);
    niq_addsbline(&g_helpptr, szContact2);
    niq_addsbline(&g_helpptr, szContact3);
    niq_addsbline(&g_helpptr, szContact4);

	if(ent->svflags & SVF_NOCLIENT)
		{
	    g_nSBLineNum = NIQ_DEFINFOENDLINE-1;
		niq_addsbline(&g_helpptr, szPrompt0);
		}

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_showmotd

/////////////////////////////////////////////////////////////////////////////

static void niq_help1(edict_t* ent)
{
    niq_addsbline(&g_helpptr, szWelcomeStrNIQ);
    g_nSBLineNum++;
                             //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_addsbline(&g_helpptr, "This server is running NIQ. In NIQ, all");
	niq_addsbline(&g_helpptr, "items are removed from the game and the");
	niq_addsbline(&g_helpptr, "server controls weapons, ammo and health");
	niq_addsbline(&g_helpptr, "for everyone. All players always have");
	niq_addsbline(&g_helpptr, "the same weapon, and ammo and health are");
	niq_addsbline(&g_helpptr, "given out at the same rate for everyone.");
	niq_addsbline(&g_helpptr, "You can only gain health if you are not");
	niq_addsbline(&g_helpptr, "attacking (firing), drowning or burning.");
    g_nSBLineNum++;
    g_nSBLineNum++;
	niq_addsbline(&g_helpptr, "Use nmotd, nhelp or ninfo to return to");
	niq_addsbline(&g_helpptr, "these menus at any time during the game.");
	if(ent->svflags & SVF_NOCLIENT)
	 niq_addsbline(&g_helpptr,   "(+attack also Exits observer mode)");
    g_nSBLineNum++;

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_help1

/////////////////////////////////////////////////////////////////////////////

static void niq_help2(edict_t* ent)
{
                             //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_addsbline(&g_helpptr, "While any scoreboard is being displayed");
	niq_addsbline(&g_helpptr, "hit '1' (use Blaster) to toggle among");
	niq_addsbline(&g_helpptr, "the available scoreboards (some of these");
	niq_addsbline(&g_helpptr, "require 512x384 or higher and some may");
	niq_addsbline(&g_helpptr, "be disabled). PPH=Points Per Hour.");
    g_nSBLineNum++;

	niq_addsbline(&g_helpptr, "While NO scoreboard or menu is up, you");
	niq_addsbline(&g_helpptr, "can also change the HUD mini-scoreboard");
	niq_addsbline(&g_helpptr, "format by hitting '1'. The mini-SB will");
	niq_addsbline(&g_helpptr, "continuously show you how you are doing");
	niq_addsbline(&g_helpptr, "relative to other players in the game.");
    g_nSBLineNum++;

	niq_addsbline(&g_helpptr, "NOTE THAT: to change your video mode");
	niq_addsbline(&g_helpptr, "and MP settings etc., first go to the");
	niq_addsbline(&g_helpptr, "console (tilde), *then* hit the Esc key.");
    g_nSBLineNum++;

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_help2

/////////////////////////////////////////////////////////////////////////////

static void niq_help3(edict_t* ent)
{
                             //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_addsbline(&g_helpptr, "Use the 'incdf' and 'decdf' commands to");
	niq_addsbline(&g_helpptr, "change your Damage Factor if voluntary");
	niq_addsbline(&g_helpptr, "handicapping is enabled. This is a");
	niq_addsbline(&g_helpptr, "voluntary system which allows you to");
	niq_addsbline(&g_helpptr, "make up for being an LPB, for example.");
	niq_addsbline(&g_helpptr, "Certain NIQ scoreboards show the average");
	niq_addsbline(&g_helpptr, "and current DFs for the current level.");
    g_nSBLineNum++;

	if(niq_tractor->value)
		{
		niq_addsbline(&g_helpptr, "The tractor beam is enabled. Bind a");
		niq_addsbline(&g_helpptr, "key to +hook (e.g. bind CTRL +hook)");
		niq_addsbline(&g_helpptr, "to be able to use it.");
		}
	else
		{
		niq_addsbline(&g_helpptr, "If the grappling hook is enabled, the");
		niq_addsbline(&g_helpptr, "best way to switch between it and the");
		niq_addsbline(&g_helpptr, "current (only available) weapon is to");
		niq_addsbline(&g_helpptr, "bind a key to 'weapnext', e.g. type");
		niq_addsbline(&g_helpptr, "'bind mouse1 weapnext' at the console.");
		}
    g_nSBLineNum++;

	niq_addsbline(&g_helpptr, "The 'id' command enables player id.");
    g_nSBLineNum++;

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_help3

/////////////////////////////////////////////////////////////////////////////

static void niq_info1(edict_t* ent)
{
	niq_addsbline(&g_helpptr,       "NIQ Settings (1/4)");
    g_nSBLineNum++;

								   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_showinfo(&g_helpptr,        "NIQ enabled:",         			niq_enable->value,		1);
	niq_showinfo(&g_helpptr,        "Bots enabled:",         			niq_ebots->value,		1);
	niq_showinfo(&g_helpptr,        "Voluntary handicapping:",			niq_handicap->value,	1);
    g_nSBLineNum++;
	niq_showinfo(&g_helpptr,        "Random weapons:",         			niq_weaprand->value,	1);
	niq_showinfo(&g_helpptr,        "Use all weapons:",					niq_weapall->value,		niq_weaprand->value);
	niq_showinfoint(&g_helpptr,     "Weapon time (secs):",				niq_weapsecs->value);
	niq_showinfo(&g_helpptr,        "Tractor beam (+hook):",			niq_tractor->value,		1);
	niq_showinfoint(&g_helpptr,     "Superblaster damage:",				niq_sbhp->value);
    g_nSBLineNum++;
	niq_showinfoflt(&g_helpptr,     "Points gained for a kill:",		niq_killpts->value);
	niq_showinfoflt(&g_helpptr,     "Points lost for being killed:",	niq_kildpts->value);
	niq_showinfoflt(&g_helpptr,     "Points lost for suiciding:",   	niq_suicpts->value);

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_info1

/////////////////////////////////////////////////////////////////////////////

static void niq_info2(edict_t* ent)
{
	niq_addsbline(&g_helpptr,       "NIQ Settings (2/4)");
    g_nSBLineNum++;

								   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_showinfoint(&g_helpptr,     "Maximum health:",					niq_hlthmax->value);
	niq_showinfoint(&g_helpptr,		"Health increment:",   				niq_hlthinc->value);
	niq_showinfo(&g_helpptr,		"Need a kill to get health:",	    !niq_auto->value,		1); 
    g_nSBLineNum++;

	niq_showinfo(&g_helpptr,        "Health increment sound:",			niq_sndhlth->value,		1);
	niq_showinfo(&g_helpptr,        "Switch warning sound:",			niq_sndwarn->value,		1);
	niq_showinfo(&g_helpptr,        "Weapon switch sound:",				niq_sndswitch->value,	1);
	niq_showinfo(&g_helpptr,        "Weapon switch messages:",			niq_msgswitch->value,	1);
    g_nSBLineNum++;

	niq_showinfoint(&g_helpptr,     "Skill setting (bots):", 			skill->value);
	niq_showinfo(&g_helpptr,        "Visible weapons (requires pak):",  view_weapons->value,	1);
	niq_showinfo(&g_helpptr,        "Grappling hook (requires pak):",   grapple->value,			1);
	niq_showinfo(&g_helpptr,        "Player ID in any mode:", 			niq_playerid->value,	1);
    g_nSBLineNum++;

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_info2

/////////////////////////////////////////////////////////////////////////////

static void niq_info3(edict_t* ent)
{
	char szMapName[128] = "";
	niq_addsbline(&g_helpptr,       "NIQ Settings (3/4)");
    g_nSBLineNum++;

	sprintf(szMapName,				"Current map: %27s",				level.mapname);
	niq_addsbline(&g_helpptr,       szMapName);

								   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_showinfo(&g_helpptr,        "Map cycling:",          			map_mod_,				1);
	niq_showinfo(&g_helpptr,        "Random maps:",          			mapmod_random->value,	map_mod_);
	niq_showinfo(&g_helpptr,        "Use all maps:",					niq_allmaps->value,		(map_mod_ && mapmod_random->value));
	niq_showinfoint(&g_helpptr,     "Intermission end delay (secs):",	niq_inttime->value);
    g_nSBLineNum++;
	niq_showinfo(&g_helpptr,        "Wide scoreboards enabled:",		niq_sbwide->value,		1);
	niq_showinfo(&g_helpptr,        "HUD mini-scoreboard enabled:",		niq_sbmini->value,		1);
	niq_showinfo(&g_helpptr,        "Debug scoreboard enabled:",		niq_sbdebug->value,		niq_sbwide->value);
	niq_showinfoint(&g_helpptr,     "Maximum scoreboard lines:",		niq_sblines->value);

    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_info3

/////////////////////////////////////////////////////////////////////////////

static void niq_info4(edict_t* ent)
{
	niq_addsbline(&g_helpptr,       "NIQ Settings (4/4)");
    g_nSBLineNum++;

								   //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
	niq_showinfo(&g_helpptr,        "CTF enabled (requires pak):",   	ctf->value,				1);
#ifdef EBOTS
	niq_showinfo(&g_helpptr,        "Teamplay enabled:",				teamplay->value,		1);
#endif
    g_nSBLineNum++;
	niq_showinfo(&g_helpptr,        "Spawn farthest:",					(int)dmflags->value & DF_SPAWN_FARTHEST,		1);
	niq_showinfo(&g_helpptr,        "Force respawn:",					(int)dmflags->value & DF_FORCE_RESPAWN,			1);
	niq_showinfo(&g_helpptr,		"Friendly fire does damage:",   	!((int)dmflags->value & DF_NO_FRIENDLY_FIRE),	1);
	niq_showinfo(&g_helpptr,		"Skin teams:",					    (int)dmflags->value & DF_SKINTEAMS,				1);
	niq_showinfo(&g_helpptr,		"Model teams:",					    (int)dmflags->value & DF_MODELTEAMS,			1);
    g_nSBLineNum++;
	niq_showinfoflt(&g_helpptr,     "Fraglimit:",						fraglimit->value);
	niq_showinfoflt(&g_helpptr,     "Timelimit:",						timelimit->value);
	if(ctf->value)
		niq_showinfoint(&g_helpptr, "Capturelimit:",					capturelimit->value);

	// 26/09/98 mfox: crap, this should come up all the time, not just if enabled
	// 26/09/98 mfox: added log file name info to help screen #4
    g_nSBLineNum++;
	niq_showinfo(&g_helpptr,    "Logging enabled (thx Mr.Bungle):", niq_logfile->value,     1);

    g_nSBLineNum++;
    g_nSBLineNum = NIQ_DEFINFOENDLINE;
    niq_addsbline(&g_helpptr, szPrompt1);
} // niq_info4

/////////////////////////////////////////////////////////////////////////////
// niq_blank: special screen for observing with the help strings right at the
// bottom of the screen (in any resolution).

static void niq_blank(edict_t* ent)
{
    int nStrLen;

    nStrLen = strlen(szBlank1);
	sprintf(g_helpstr, "xv %-3d yb -20 string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-nStrLen), szBlank1);
    g_helpptr += 23+nStrLen;

    nStrLen = strlen(szBlank2);
	sprintf(g_helpptr, "xv %-3d yb -10 string \"%s\"", NIQ_SCOREBOARD_CHAR_XSIZE*(NIQ_SBCHARS320-nStrLen), szBlank2);
} // niq_blank

/////////////////////////////////////////////////////////////////////////////
// niq_help:
//
// Shows 1 of N help menus, depending on the value of niq_helpmode for the
// given client.

void niq_help(edict_t* ent)
{
    g_helpptr		= g_helpstr;
    g_nSBLineNum	= NIQ_DEFINFOSTARTLINE;

	if (!ent->client)
		return;

	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_help called for a bot!\n");
#endif
		return;
		}

	switch(ent->client->resp.niq_helpmode)
		{
		case 1:
			niq_motd(ent);
			break;

		case 2:
			niq_help1(ent);
			break;

		case 3:
			niq_help2(ent);
			break;

		case 4:
			niq_help3(ent);
			break;

		case 5:
			niq_info1(ent);
			break;

		case 6:
			niq_info2(ent);
			break;

		case 7:
			niq_info3(ent);
			break;

		case 8:
			niq_info4(ent);
			break;

		case NIQ_MAXHELPSCREEN:
			niq_blank(ent);
			break;

		default:
			gi.dprintf("NIQ: invalid help level\n");
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (g_helpstr);

	gi.unicast (ent, false);
} // niq_help

/////////////////////////////////////////////////////////////////////////////
// niq_cmd_use_f:
//
// Same as Cmd_Use_f except certain command are blocked in NIQ/NIQCTF.

static void niq_cmd_use_f(edict_t* ent)
{	
	gitem_t		*it;
	char		*s;

	s	= gi.args();
	it  = FindItem (s);

    // niq: clients can't select weapons
	if(it && it->use && (it->flags & IT_WEAPON))
		{
		if(!ctf->value && !grapple->value)
			{
			gi.cprintf (ent, PRINT_HIGH, "NIQ selects all weapons for you.\n");
			return;
			}
		else
			{
	    	int index = ITEM_INDEX(it);
			if(index != ITEM_INDEX(FindItem("Grapple")))
				{
				// is it the current weapon?
		        if(index != niq_getcurrentweaponindex())
       				gi.cprintf (ent, PRINT_HIGH, "Only the grapple hook or the current weapon can be selected in CTF mode.\n");
				return;
                }    
			}
		}

	// pass the command on
	Cmd_Use_f (ent);
} // niq_cmd_use_f

/////////////////////////////////////////////////////////////////////////////
// niq_cmd_give_f:
//
// Pretty much a copy of Cmd_Give_f except in NIQ the client is never given
// weapons or armor and only the current ammo (if any) is given.

static void niq_cmd_give_f(edict_t* ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
		{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
		}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
		{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;

		if (!give_all)
			return;
		}

	if ((give_all || Q_stricmp(name, "ammo") == 0))
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;

            // niq: no ammo can be 'picked up' in NIQ
			if (!(it->flags & IT_AMMO))
				continue;

            // niq: has to already have this ammo to be allowed more
            if(!ent->client->pers.inventory[i])
                continue;

			Add_Ammo (ent, it, 1000);
		}

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
} // niq_cmd_give_f

/////////////////////////////////////////////////////////////////////////////

static void NIQEndObserverMode(edict_t* ent)
{
	if(!(ent->svflags & SVF_NOCLIENT))
		return; // not in observer mode

	// don't show motd again for sp/coop
	ent->client->pers.niq_didmotd = true;

	if(ctf->value)
		{
		CTFStartClient(ent);
//		ent->client->showscores	= 1;
		niq_updatescreen(ent);
		return;	 // defer to CTF menu code
		}
	
	ent->svflags &= ~SVF_NOCLIENT;

	PutClientInServer (ent);

	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;

	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

#ifdef NIQ
	gi.bprintf (PRINT_HIGH, "%s joins NIQ, %d client(s)\n", ent->client->pers.netname, niq_getnumclients());
#else							
	my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
#endif
	// update screen right away
	niq_updatescreen(ent);
} // NIQEndObserverMode

/////////////////////////////////////////////////////////////////////////////

static void niq_toggle_help(edict_t* ent, int nScreenNum)
{
    // toggle help screen
    if(ent->client->resp.niq_helpmode)
		{
        ent->client->resp.niq_helpmode	= 0;

		// update screen right away
		niq_updatescreen(ent);

		NIQEndObserverMode(ent);
		}
    else
        {
    	ent->client->showinventory		= false;
    	ent->client->showhelp			= false;
    	ent->client->showscores			= false;
		ent->client->resp.niq_helpmode	= nScreenNum;

        niq_help(ent);
        }
} // niq_toggle_help

/////////////////////////////////////////////////////////////////////////////

#if 0
void niq_setintval(int* nValToSet, int nMin, int nMax)
{
	char* cmd;
	int nVal;

	if(gi.argc() != 2)
		{
		gi.dprintf("command missing value\n");
		return;
		}

	cmd = gi.argv(1);

	nVal = atoi(cmd);

	if(nVal < nMin)
		nVal = nMin;
	else if(nVal > nMax)
		nVal = nMax;

	*nValToSet = nVal;
} // niq_setintval
#endif

/////////////////////////////////////////////////////////////////////////////

static qboolean niq_command(edict_t* ent, char* cmd)
{
	if (!Q_stricmp (cmd, "nmotd"))
		{
		niq_toggle_help(ent, 1);
		return true;
		}

	if (!Q_stricmp (cmd, "nhelp"))
		{
		niq_toggle_help(ent, 2);
		return true;
		}

	if (!Q_stricmp (cmd, "ninfo"))
		{
		niq_toggle_help(ent, 5);
		return true;
		}

	// no other commands are handled here if niq features are disabled
	if(!niq_enable->value)
		return false;

    if (!Q_stricmp (cmd, "incdf"))
		{
		niq_increase_damage(ent);
		return true;
		}
    if (!Q_stricmp (cmd, "decdf"))
		{
		niq_decrease_damage(ent);
		return true;
		}
    if (!Q_stricmp (cmd, "maxdf"))
		{
		niq_max_damage(ent);
		return true;
		}
    if (!Q_stricmp (cmd, "mindf"))
		{
		niq_min_damage(ent);
		return true;
		}
    if (!Q_stricmp (cmd, "resetdf"))
		{
		niq_reset_damage(ent);
		return true;
		}
    if (!Q_stricmp (cmd, "showdf"))
		{
    	niq_show_damage(ent);
		return true;
		}

	return false;
} // niq_command

/////////////////////////////////////////////////////////////////////////////

static qboolean	niq_toggle_cmd(edict_t* ent, char* cmd)
{
	// first see if its the toggle SB/HUD command
	if(!Q_stricmp (cmd, "use"))
		{
		char* s	= gi.args();

	    // '1' (use blaster) toggles scoreboard / HUD
		if (!Q_stricmp (s, "Blaster"))
			{
			// can't toggle SB or HUD unless in DM mode
			if(!deathmatch->value)
				return false;

			// don't let client toggle anything if scoreboard 'and' help/motd is up
			if(!ent->client->showscores && ent->client->resp.niq_helpmode)
				return true; // eat the command

			// scoreboard is up, or HUD is visible: allow toggling
			if(ent->client->showscores)
				niq_toggle_scoreboard(ent);
			else
				niq_toggle_hud(ent);

			return true;
			}
		}

	return false;
} // niq_toggle_cmd

/////////////////////////////////////////////////////////////////////////////

void niq_SelectNextMenu(edict_t* ent)
{
	if(ent->svflags & SVF_NOCLIENT)
		{
		if(++ent->client->resp.niq_helpmode > NIQ_MAXHELPSCREEN)
			ent->client->resp.niq_helpmode = 1;		// wrap back to MOTD screen
		}
	else
		{
		if(++ent->client->resp.niq_helpmode >= NIQ_MAXHELPSCREEN)
			ent->client->resp.niq_helpmode = 1;		// wrap back to MOTD screen
		}

	// update screen right away
	niq_updatescreen(ent);
} // niq_SelectNextMenu

/////////////////////////////////////////////////////////////////////////////

void niq_SelectPrevMenu(edict_t* ent)
{
	if(--ent->client->resp.niq_helpmode < 1)
		{
		if(ent->svflags & SVF_NOCLIENT)
			ent->client->resp.niq_helpmode = NIQ_MAXHELPSCREEN;		// wrap to last help screen
		else
			ent->client->resp.niq_helpmode = NIQ_MAXHELPSCREEN-1;	// wrap to last help screen
		}

	// update screen right away
	niq_updatescreen(ent);
} // niq_SelectPrevMenu

/////////////////////////////////////////////////////////////////////////////

void niq_putaway(edict_t* ent)
{
	ent->client->resp.niq_helpmode = 0;

	if(!(ent->svflags & SVF_NOCLIENT))
		{
		Cmd_PutAway_f (ent);
		}
	else
		{
		ent->client->showscores		= false;
		ent->client->showhelp		= false;
		ent->client->showinventory	= false;

		NIQEndObserverMode(ent);
		}
} // niq_putaway

/////////////////////////////////////////////////////////////////////////////

qboolean niq_blocked_intermission_cmd(edict_t* ent, char* cmd)
{
	if(!(ent->svflags & SVF_NOCLIENT))
		return false; // not in intermission

	// let user customize scoreboard?
	if(niq_toggle_cmd(ent, cmd))
		return true;

	if((!ctf->value || ent->client->resp.niq_helpmode) && !Q_stricmp (cmd, "putaway"))
		{
		niq_putaway(ent);
		return true;
		}

	// inventory command results in CTF join menu if in observer mode, so disable
	if(ctf->value && ent->client->resp.niq_helpmode && !Q_stricmp (cmd, "inven"))
		return true; // eat the command

	return false;
}
/////////////////////////////////////////////////////////////////////////////

qboolean niq_clientcommand(edict_t* ent, char* cmd)
{
	if(ent->client->pers.niq_isbot)
		{
#ifdef _DEBUG
		gi.dprintf("NIQ: niq_clientcommand called for a bot!\n");
#endif
		return true; // eat it
		}

	if(ent->client->resp.niq_helpmode && !Q_stricmp (cmd, "invnext"))
		{
		niq_SelectNextMenu(ent);
		return true;
		}
	if(ent->client->resp.niq_helpmode && !Q_stricmp (cmd, "invprev"))
		{
		niq_SelectPrevMenu (ent);
		return true;
		}

	// disable most other commands as long as player is in intermission?
	if(niq_blocked_intermission_cmd(ent, cmd))
		return true;

	if(ent->svflags & SVF_NOCLIENT)
		return false; // in intermission -- no other commands handled here

	// 1) if its an niq-specific command, do it then return true
	if(niq_command(ent, cmd))
		return true;

#ifdef OHOOK
	if (Q_stricmp (cmd, "hook") == 0)
		{
		if (niq_tractor->value)
			hook_fire (ent);
		else
			gi.cprintf(ent, PRINT_HIGH, "Tractor beam is disabled\n");
	
		return true;
		}

	if (Q_stricmp (cmd, "unhook") == 0)
		{
		if (niq_tractor->value && ent->client->hook)
			hook_reset(ent->client->hook);

		return true;
		}

#endif//OHOOK

	// no other command are handled here if NIQ features are disabled	
	if(!niq_enable->value)
		return false;

	// 2) "use Blaster" can toggle the scoreboard/HUD
	if(niq_toggle_cmd(ent, cmd))
		return true;

	// 3) if its a blocked command return true
	if(niq_commandisblocked(ent, cmd))
		return true;
	   
	// 4) some commands are handled by NIQ in a special manner
	if(!Q_stricmp (cmd, "inven"))
		{
		ent->client->resp.niq_helpmode = 0;

		Cmd_Inven_f (ent);

		NIQEndObserverMode(ent);

		return true;
		}

	if(!Q_stricmp (cmd, "putaway"))
		{
		niq_putaway(ent);

		return true;
		}

	if(!Q_stricmp (cmd, "use"))
		{
		niq_cmd_use_f(ent);

		return true;
		}

	if(!Q_stricmp (cmd, "give"))
		{
		niq_cmd_give_f(ent);

		return true;
		}

	// ???
	if(!Q_stricmp(cmd, "id"))
		{
		// allow enabling player id for any mode
		CTFID_f (ent);
		return true;
		}

	// 5) let regular ClientCommand code have a shot at the command
    return false;
} // niq_clientcommand

/////////////////////////////////////////////////////////////////////////////

#ifdef EBOTS
#ifdef NIQ
void niq_Svcmd_Status_f (void)
{
	char szName[33];
	char pszWeap[5];
	int	 i;

	gi.dprintf("\n");
	gi.dprintf("=========================\n");
	gi.dprintf("NIQ server status report:\n\n");
	gi.dprintf("Name            WEAP HLTH AMMO  DFAVG DFCUR KLS KLD SUI TIME PING PPH SCORE\n\n");
	for (i=0; i<game.maxclients; i++)
    	{
		edict_t* cl_ent = g_edicts + 1 + i;
		gclient_t* cl;

		if (!cl_ent->inuse)
			continue;

		cl = cl_ent->client;
		if(!cl)
			continue;

		niq_getname(15, cl, szName, false);

		niq_getcurweapname(cl, pszWeap);

		// show clients that are actively fighting
		if(cl->pers.niq_isbot)
			{
			if((cl_ent->last_fire + niq_weap_fire_times[game.niq_curweapon-1]) >= level.time)
				strcat(pszWeap, "*");
			}
		else 
			{
			if(cl->weaponstate == WEAPON_FIRING || ((cl->latched_buttons|cl->buttons) & BUTTON_ATTACK))
				strcat(pszWeap, "*");
			}

        gi.dprintf("%-15s %4s %4d %4d  %5.1f %5.1f %3d %3d %3d %4d %4d %3d %5.1f\n", szName, pszWeap, cl_ent->health, cl->pers.inventory[niqlist[game.niq_curweapon].nListIndexAmmo], niq_getaverageDF(cl), cl->pers.niq_fDamage, cl->resp.niq_kills, cl->resp.niq_killed, cl->resp.niq_suicides, (level.framenum - cl->resp.enterframe)/600, cl->ping, niq_getPPH(cl), cl->resp.score);
		}

	gi.dprintf("\n");
} // Svcmd_Status_f

#endif
#endif

/////////////////////////////////////////////////////////////////////////////

qboolean niq_zapitem(edict_t *ent, gitem_t *item)
{
	// 08/03/98 niq: The following removes all items of any type except if we are in
    // SP/COOP mode, we allow keys and the rebreather and environment suit
	if (deathmatch->value)
		{
		// 08/03/98 niq: don't remove flags for CTF!
		if(ctf->value && !niq_strncmp(item->classname, "item_flag"))
			return false;

		// zap it
		G_FreeEdict (ent);
		return true;
		}

#ifdef NIQSP
	// niq: keep these items so sp/coop games work
	if (item->classname &&
        niq_strncmp(item->classname, "key_") &&
        niq_strncmp(item->classname, "item_breather") &&
        niq_strncmp(item->classname, "item_enviro"))
		{
		// zap it
		G_FreeEdict (ent);
		return true;
		}
#endif

	// item not zapped
	return false;
} // niq_zapitem

/////////////////////////////////////////////////////////////////////////////

void niq_suicide(edict_t* self)
{
	self->client->resp.score -= niq_suicpts->value;
	self->client->resp.niq_suicides++;

	if (self->client->team)
	   self->client->team->score -= niq_suicpts->value;
} // niq_suicide

/////////////////////////////////////////////////////////////////////////////

void niq_kill(edict_t* self, edict_t* attacker, qboolean ff, char* message1, char* message2)
{
	// niq: show how much health attacker had left!
	my_bprintf (PRINT_MEDIUM,"%s %s %s%s (%d)\n", self->client->pers.netname, message1, attacker->client->pers.netname, message2, attacker->health);

	if (!deathmatch->value)
		return;

	if (ff)
        {
        // killed by friendly fire: attacker loses points
		attacker->client->resp.score -= niq_killpts->value;

		if (attacker->client->team)
			attacker->client->team->score -= niq_killpts->value;

        // we don't track ff kills so just decrement kills counter
    	attacker->client->resp.niq_kills--;

        // self score / counts aren't affected by FF death
        }
	else
        {
        // in NIQ you lose some points for being killed
		attacker->client->resp.score += niq_killpts->value;
    	self->client->resp.score     -= niq_kildpts->value;

		if (attacker->client->team)
			attacker->client->team->score	+= niq_killpts->value;

		if (self->client->team)
			self->client->team->score		-= niq_kildpts->value;

        // see if we should do anything else
        niq_clientkill(attacker);

    	attacker->client->resp.niq_kills++;
    	self->client->resp.niq_killed++;
        }
} // niq_kill

/////////////////////////////////////////////////////////////////////////////

void niq_die(edict_t* self)
{
	self->client->resp.score -= niq_kildpts->value;

	if (self->client->team)
		self->client->team->score -= niq_kildpts->value;
} // niq_die


/////////////////////////////////////////////////////////////////////////////

void niq_showscoreboards(edict_t *ent)
{
	// if the scoreboard is up, update it
	// update debug scoreboard every 1/10th
	if (ent->client->showscores && (!(level.framenum & 31) || (ent->client->pers.niq_sbmode == NIQ_SB_640A)) )
	{
//ZOID
		if (ent->client->menu) 
		{
			PMenu_Update(ent);

			gi.unicast (ent, false);
		} 
		else
//ZOID
	    {	
            // niq:	
            niq_deathmatchscoreboardmessage (ent, ent->enemy, false);
        }
	}

	// niq: if the help menu is up, update it (scoreboard up in intermission blocks this)
	if (!ent->client->showscores && ent->client->resp.niq_helpmode && !(level.framenum & 31) )
    {
        niq_help(ent);
    }
} // niq_showscoreboards

/////////////////////////////////////////////////////////////////////////////
// niq_setcurrentweapon:
//
// Used to set the current weapon and ammo amount for new clients. When we 
// are not in deathmatch and move to a new map because someone exited the 
// level we keep the current amount of ammo and the weapon will not change.
// In deathmatch, the weapon also won't change from what the server is
// currently using, but we will reset the ammo to the default amount.

void niq_setcurrentweapon(gclient_t* cl, qboolean bResetAmmo)
{
    int nCurWeaponIndex;
	int nAmmoIndex;

	if(!cl)
		return;

	nAmmoIndex       = niqlist[game.niq_curweapon].nListIndexAmmo;
    nCurWeaponIndex	 = niqlist[game.niq_curweapon].nListIndexWeapon;

    if(bResetAmmo && (deathmatch->value || !level.intermissiontime))
        {
        // not in intermission and/or in deathmatch allocate default ammo
        int nAmmoInitial = niqlist[game.niq_curweapon].nAmmoInitial;

    	niq_set_ammo(cl, nAmmoInitial, game.niq_curweapon);
        }

	// allocate new weapon (in case not already done)
	cl->pers.inventory[nCurWeaponIndex] = 1;
	cl->newweapon                       = &itemlist[nCurWeaponIndex];
	cl->pers.weapon                     = &itemlist[nCurWeaponIndex];
    cl->pers.selected_item              = nCurWeaponIndex;

	if(nAmmoIndex == 999)
		cl->ammo_index					= 0;
	else
		cl->ammo_index					= nAmmoIndex;
} // niq_setcurrentweapon

/////////////////////////////////////////////////////////////////////////////

void niq_InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

    // niq: no weapon at all to start with

	// niq: just set grapple to 1 to have the hook in regular games
	if (ctf->value || grapple->value)
	{
//ZOID
		item = FindItem("Grapple");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
//ZOID
	}

    // niq: clients start with health = niq_healthmax initially in dm
   	client->pers.health			= niq_hlthmax->value;
    client->pers.max_health		= niq_hlthmax->value;

    // niq: set to 999 for give all / give ammo
	client->pers.max_bullets	= 999;
	client->pers.max_shells		= 999;
	client->pers.max_rockets	= 999;
	client->pers.max_grenades	= 999;
	client->pers.max_cells		= 999;
	client->pers.max_slugs		= 999;

	client->pers.connected		= true;

    // niq: Get current NIQ weapon and spawn amount of ammo if deathmatch, 
    // or new sp/coop game but not if going to the next level in sp/coop.
	// This will also set last_fire and fire_interval as needed.
    niq_setcurrentweapon(client, true);

	client->pers.niq_isbot = false;
} // niq_InitClientPersistant

/////////////////////////////////////////////////////////////////////////////

void niq_handleclientinit(gclient_t* client)
{
	float       niq_fDamage;
    int         niq_sbmode;
    int         niq_hudmode;
    qboolean    niq_didmotd;
//    int         niq_helpmode;
    qboolean    niq_isbot;

    // niq: save some things
    niq_fDamage		= client->pers.niq_fDamage;
    niq_hudmode		= client->pers.niq_hudmode;
    niq_sbmode		= client->pers.niq_sbmode;
    niq_didmotd		= client->pers.niq_didmotd;
//    niq_helpmode	= client->resp.niq_helpmode;
    niq_isbot   	= client->pers.niq_isbot;

	InitClientPersistant (client);

    // niq: restore some things
    client->pers.niq_fDamage    = niq_fDamage;
    client->pers.niq_hudmode    = niq_hudmode;
    client->pers.niq_sbmode     = niq_sbmode;
    client->pers.niq_didmotd    = niq_didmotd;
//    client->resp.niq_helpmode   = niq_helpmode;
    client->pers.niq_isbot      = niq_isbot;
} // niq_handleclientinit

#ifdef EBOTS

/////////////////////////////////////////////////////////////////////////////
// niq_RoamFindBestItem:
//
// Similar to RoamFindBestItem, but tries to skip things which are of no use
// if the level has been NIQ'd.

int	niq_RoamFindBestItem(edict_t *self, edict_t	*list_head, int	check_paths)
{
	float	closest_dist=999999, this_dist;
	edict_t	*best=NULL, *node, *trav;
	int		si;		// node closest to self
	int		dist_divide=1;		// divide distance by this, simulates wieghts for better targets
	int		best_divide;

	if (!list_head)	// nothing to look for
		return -1;

	si = ClosestNodeToEnt(self, false, false);

	for (trav = list_head ; trav ; trav = trav->node_target)
	{
		if	(trav->solid != SOLID_TRIGGER)
			continue;

		this_dist = 0;		

		if (((trav->item->pickup != CTFPickup_Flag) && (trav->item->pickup != Pickup_Weapon))
			&& (this_dist = entdist(self, trav)) > 2000) // too far away
			continue;

		// CTF, if guarding base, don't go too far away
		if (self->target_ent && self->target_ent->item && (self->target_ent->item->pickup == CTFPickup_Flag)
			&& (this_dist > BOT_GUARDING_RANGE))
			continue;
		// CTF

		if (trav->ignore_time >= level.time)
			continue;

		if (list_head == bonus_head)
		{
			if (trav->item->pickup == CTFPickup_Flag)
			{
				dist_divide = 6;

				if (	(	(self->client->resp.ctf_team == CTF_TEAM1)
						 &&	(self->client->pers.inventory[ITEM_INDEX(flag2_item)])
						 &&	(trav == flag1_ent))
					||	(	(self->client->resp.ctf_team == CTF_TEAM2)
						 &&	(self->client->pers.inventory[ITEM_INDEX(flag1_item)])
						 &&	(trav == flag2_ent)))
				{	// we have their flag, so HEAD FOR OUR FLAG!
					dist_divide += 9999;
				}
				else if (	((self->client->resp.ctf_team == CTF_TEAM1) && (trav == flag1_ent) && (flag1_ent->solid = SOLID_TRIGGER))
						 || ((self->client->resp.ctf_team == CTF_TEAM2) && (trav == flag2_ent) && (flag2_ent->solid = SOLID_TRIGGER)))
				{	// flag is sitting at home, don't try and get it
					continue;
				}

			}
		}
		if ((this_dist < 384) && visible_box(self, trav) && CanReach(self, trav))	// go for it!
		{
			if (trav != self->save_movetarget)
				self->goalentity = trav;
			else
				self->goalentity = self->save_goalentity;

			self->movetarget = trav;

			botSetWant(self, dist_divide);

			this_dist = this_dist/dist_divide;

//			this_dist = this_dist/256;		// always grab something close by

			return this_dist;
		}

		if (!check_paths)
			continue;

		if (si == -1)		// can't see a node, so don't bother checking routes
			continue;

		// see if any of our visible nodes, has a route to one of the
		// item's visible nodes
		if (((this_dist = PathToEnt(trail[si], trav, false, false)) > -1) && ((this_dist / dist_divide) < closest_dist))
		{
			this_dist = this_dist / dist_divide;
			closest_dist = this_dist;
			best = trav;
			node = trail[si];	// go for the nearest node first
			best_divide = dist_divide;

			if (this_dist < 128)	// OPTIMIZE: go for this one!
			{
				if (node != self->save_movetarget)
					self->goalentity = node;
				else
					self->goalentity = self->save_goalentity;

				self->movetarget = best;

				botSetWant(self, dist_divide);

				return this_dist;
			}
		}

//botDebugPrint("Found item %s: %i away\n", trav->classname, (int) this_dist);
	}

	if (best)
	{
//botDebugPrint("Best item %s: %i away\n", best->classname, (int) closest_dist);
		if (node != self->save_movetarget)
			self->goalentity = node;
		else
			self->goalentity = self->save_goalentity;

		self->movetarget = best;

		botSetWant(self, best_divide);

		return closest_dist;
	}
	else
	{
		return -1;
	}
} // niq_RoamFindBestItem

#endif

/////////////////////////////////////////////////////////////////////////////
// NIQStartClient:
//
// Puts the given client into observer mode. Once the client is done with 
// the motd/help -- we simply put him into the server?
//
// Returns false if client is or should go into the server, returns true if
// the client has been placed into observer mode.

qboolean NIQStartClient(edict_t *ent)
{
	// hack to disable putting clients into observer mode if this causes problems
	if(niq_blk2->value)
		{
		ent->client->resp.niq_helpmode = 1;
		ent->client->pers.niq_didmotd  = true;

	    niq_help(ent);
		return false;
		}

	if(ent->client->pers.niq_isbot)
		{
		ent->client->resp.niq_helpmode = 0;
		ent->client->pers.niq_didmotd  = true;
		return false;
		}

	if(!ent->client->resp.niq_helpmode)
		{
		ent->client->pers.niq_didmotd = true;
		return false;
		}

	// start as 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);

	// force motd up
	ent->client->resp.niq_helpmode = 1;

    niq_help(ent);

	return true;
}

