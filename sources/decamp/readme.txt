DeCamper 1.0: No Frills Source Code Release
-------------------------------------------


Mod Name: DeCamper
Version: 1.0 (12/16/98)
Author: SteQve and the rest of Two Pole Software (Rohn and X)
Contact: steqve@shore.net
URL: http://www.shore.net/~steqve/quakemods.html


There have been enough requests for DeCamper code over the months that I've
finally decided to just plain release it and be done with it all.  I do
not intend to maintain this code.  If an established mod author wishes to
maintain it, contact me and I'll give my "official" blessing, like that amounts
to a hill of beans or something.




IMPORTANT STUFF FOR YOU TO KNOW, OR FOR ME TO APOLOGIZE ABOUT
-------------------------------------------------------------

See DeCamper.h for function prototypes that are used in the game source code.

All code changes in the original game code are wrapped by the following
comments:

   // DeCamper
   // End DeCamper

I tried to minimize this, but sometimes you have to change the actual
code instead of just wrapping it.  I tried to add explanatory comments
in the places where this occurred.


The most important structs:
	VotingRegistry
	Camper
See them in DeCamper.h!


At the point in time when I coded this, I didn't want to touch the edict_t
structure (portability, the idea that new source code would be coming out
soon, etc.)  So all DeCamper-specific data structures are kept elsewhere,
and a linear search is performed to match the structures to their associated
entities.  Inefficient?  Yep.  Sorry.


The DeCamper/ folder is hard-coded into pathnames.  Sorry.


Some actions, because I thought they might take up some CPU cycles, occur
only once every 10 or more frames.  That can probably be changed.  Look for
tests like "level.framenum % 10 == 0" and stuff like that.  I'd apologize
about this, but it's not too bad and comes brutally close to making sense.


Every pickup function is modified - Pickup_Armor, Pickup_Weapon, etc.
It probably would have been easier to just modify Touch_Item, which ultimately
calls these other pickup functions.  Sorry.  I was young and naive and clueless
in January '98 ;-)


There are probably objects that don't get freed at the end of the level
like they should.  See DeCamperEndLevel, for example - I loop through a linked
list but never seem to clean up the entities in the linked list...  Sorry.
It's probably not *too* bad.  See FreeCamperVoterObjects(), which does most
of the work.



IMPORTANT FUNCTIONS
-------------------

InitializeDeCamper
  - called during a level init.

Cmd_DeCamp
  - user commands to nominate campers, vote, etc.

NominateDeCamp
  - sets a nomination in motion.  Called from Cmd_DeCamp.

SetupCamperVotingCommunity
  - sets up a vote; associated with NominateDeCamp

VoteDeCamp
  - records a client's vote

EvaluateDeCamperVote
  - all the logic that determines when a vote is complete, if a nomination
    was successful, etc.  A biggie.  Includes many error checks, e.g.
	nominated camper disconnects, etc.

DeCamperEndLevel
  - cleans up Camper structures, etc.

AddCamperInventoryItem
  - adds an item to a camper's inventory so they can't pick it up again

RenderCamperEffects
  - sets a camper's glowing, etc.


VAPOR FUNCTIONS
---------------

DeCamperAdminCommand
  - was going to allow admins with a password to cancel votes, change settings,
    etc.  



Other stuff to know
===================

Look at the item inventories for each Camper.  It's ugly, but when the
game mode automatically replaces items that a Camper picks up, then
you don't want the Camper to keep turning around to pick up the items
again and again, so the inventory is necessary.


I do my best to do error checking for things like when a client leaves
the game during a vote, etc.

There are several mechanisms to prevent or reduce abuse of the voting system.
E.g., somebody makes a nuisance of themselves by nominating everybody as a
Camper, or a Camper tries to take revenge by consistently nominating
somebody else.  Things like the voting "timeout" and Camper "immunity" exists.

This sort of coding has to be done to prevent what I call the Asswipe Factor:
too many Asswipes out there have fun abusing things (look at message flooding)
so we have to code around them.  Note that I do not mean to label Campers
as Asswipes, however.
