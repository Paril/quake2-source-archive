ServObit 1.4 Technical Notes
---------------------------------
by SteQve (steqve@shore.net), 4/12/98.

This TechNotes document is for people who want to integrate ServObit with
other modifications or who want to examine its code in detail.  It assumes
that the reader has a working knowledge of computer programming.

This code is written to be easily readable and understandable.  As a result,
it's not particularly elegant and in some cases it is downright inefficient.
You have been warned.


Table of contents
-----------------
1. Changes to original source code
2. Other mods integrated into this one
3. ServObit files
4. Main Functions
5. Known limitations and inefficiencies
6. Special notes to weapons, team play, and bot modifiers
7. Obituary Tester, and My Apologies
8. Credits


1. Changes to Original Source Code
----------------------------------

All source code changes are marked with a // ServObit comment.  ServObit was
written to minimize impact on the original source code, so changes are minimal.

p_client.c - ClientObituary, ClientConnect, ClientDisconnect

g_save.c - InitGame

g_cmds.c - ClientCommand

ServObit also uses GenderMod.  See the TechNote in the GenderMod/ directory.


2. Other mods integrated into this one
--------------------------------------

GenderMod 0.1.  See the TechNote in the GenderMod/ directory.  I encourage
other mod authors to make up these TechNotes too.  If everybody did this,
wouldn't that be helpful?

3. ServObit files
-----------------

SrvObAPI.[ch] defines the API to the rest of the game.  The API is separated
from the rest of the ServObit code so that the tester program doesn't need to
have to know about edicts/etc.  It's not as pretty as that, but that was the
intention anyway.

ServObits.h defines a lot of constants which help to mnemonically and reliably
refer to object "types."  The primary ServObit "types" are defined here - 
body state, gender, weapon, etc.  It also defines raw limits on the size of
some data structures.  It also defines some macros to help with debugging
and keystroke saving.

ServObits.c defines the bulk of the ServObit capability.

ModUtils.[ch] is a set of various utility functions.  Note that not all of
of them are used by ServObit, but hey, I have to deal with multiple mods, ya
know?

motd.[ch] handles the message of the day.  Yes, I know it's ugly, so you'll
want to be sure to edit this first ;-)

gender.c and gender.h are part of GenderMod.  See the GenderMod TechNote for
details.

4. Main Functions
-----------------

The main functions are: ServObitInitGame, ReadServerObits,
ServObitPrintWelcome, ServObitAnnounceConnect, ServObitAnnounceDisconnect,
DisplayObituary, and ServObitClientObituary.  Hopefully these names are
self evident.

5. Known limitations and inefficiencies
---------------------------------------

a) If you don't think -80 is a low enough health for a MegaGib (you can get 
to like -500; but the console only goes to -99 ;-) then set SO_MEGAGIB_MINIMUM
or if you're absolutely nuts you might want to allow a user to set
ServObit.MegaGib.  Don't set it to less than -41 though, since -40 is the
normal gib amount.  I think -80 might be too little; maybe -100 or -120 would
work better.  You want mega-gibs to happen rarely enough that people on the
server say "wow!" when it happens.

b) Most functions requiring search use linear search.  In some cases this is
necessary; e.g., a linear search is performed when looking for matching
selectors, but I saw no immediately obvious way of setting up a search tree
that guaranteed the order of evaluation (since first in Obits.txt to match is
used).  A later release will address this issue.  Currently, the
MatchObituary() function has been optimized in terms of which
aspects of the situation it looks at first.  One place where search
optimization would be effective is in substitutions.

c) The maximum number of selectors and messages per selector is settable by
the user, but it is fixed once the user has set the number.  I.e., for
EVERY selector there is the same number of messages; ServObit creates the
user-specified number of selectors regardless of whether there are that many
or not.  The same applies for the number of substitution strings per
substitution.  Obviously a good amount of memory is wasted because of this.
A later release will make it more efficient and waste no space.

d) Obituary substitutions and normal text substitutions should be treated as
separate entities.  Currently, when searching for a text substitution (e.g.
when creating a death message), ServObit looks through all the obituary
substitutions too, even though it shouldn't.  This will be addressed in a
later release.

e) Obituary substitutions are "spliced" into each selector that uses it.  A
more efficient use of memory would be to refer to the substitution itself,
instead of just splicing in all the obit substitution's messages into each
selector.

f) I need to implement a better random number generator.  rand() just doesn't
cut it. I suppose I could have used random() and cast it to an int, but I
remember reading somewhere that your random distribution is still screwed up
if you just take the mod of the result.

g) There is a bit of duplication in some code.  It could be cleaned up by
defining new functions or using more "elegant" C coding.  But, at least it's
a little cleaner than the last version.

6. Special notes to weapons, team play, and bot modifiers
---------------------------------------------------------

All modifiers - if you define new "types" of objects, you may need
to change the ServObit parser and define these new "types" in ServObit.h.
Also, if you want to do some selection based on some other characteristics
besides gender/weapontype/etc., you will have to change the selector
parser functions.

All modifiers - if you change the ServObit code, you may have to change
the tester code too.

Most changes you make to the ServObit code will probably be "busy work"
and won't be difficult.  The tester code is pretty much undocumented, but you
will have to change that, too.  See the paragraph below, and I apologize
in advance.

Weapon modifiers - If you define new weapons, you will have to define new
constants.  Make sure they fall within the _END and _START ranges (you'll
see those in ServObits.h).  You'll also have to change the parser functions.
Also, if you define new weapon powerups besides Quad, you may have to define
new PowerTypes.  Especially pay attention to WeaponType things.  Sorry to
dump all this on you.

Team play modifiers - if you want to define new selectors based on whether
someone kills a team member or not, you will have to change the parser
functions and define a new KillerType.

Bot modifiers - You may want to define a new KillerType, and you can use the
neuter gender if you wish, or create a new one if you dare.

7. Obituary Tester, and My Apologies
------------------------------------

This documentation section sucks.  I know.  Sorry.

The main thing to know about the Obituary Tester code is that it includes
STRIPPED DOWN VERSIONS of some ServObit source code such as ModUtils and
GenderMod.  The main reason was to get rid of all references to edict_t
and other ubiquitous structures.  So if you change ServObit in some fashion,
you may have to re-strip and change the tester code too.  Sorry.

Oh, yeah.  This kinda blows for you, but I had to change some ServObit
functions to make them work properly for the tester, e.g. to have the
obituaries formatted and printed to the log file, and to spit out extra
complaints if ServObit reached a point it shouldn't have.  You can take small
comfort that I think I made a comment in the places when I changed the
Servobit code.  You would think that I'd have used a // SOTest comment
or somesuch, but you'd be wrong, although I do try to use a SERV_OBIT_TESTER
preprocessor directive, albeit not particularly effectively.  Sorry.

SrvObTest.c is the main file.  All it basically takes care of is generating
the permutations, parsing command line options, and making pretty looking 
output that's readable to humans.  I can't think of anything to apologize
for in this paragraph.  Sorry.

Limitations of the tester - it's stupid about suicide situations.  It
generates incorrect cases where the victim gender is different than the killer
gender, and it may confuse some server ops.  Sucks, eh?  I'll fix it in
the next release.  Sorry.

Oh yeah, and I have it try an "any enemy death" situation which is what
it would spit out if there wasn't a better matching death, but it will
probably confuse some server ops when they see the generic death messages.
Sorry.

8. Credits
----------

Thanks to idsoftware for Quake - natch.

I'm an Emacs man myself so I don't like their editor, but Microsoft's Visual
C++ 4.0 debugger was very useful.  It pains me to say that but it's true.

Thanks to NewtonD (newton@moongates.com) for his Death Message mod which gave
me ideas and encouragement for extending ServObit.  The text substitutions
were originally in his death message mod.  Too bad he's a good C programmer;
I couldn't understand his code before I ran out of patience ;-) although
his binary search routines will come in handy for the next version.

Derek Westcott (!Eradicator!) gave me some very detailed feedback that helped
me fix one bug, and he found and fixed a second one, for ServObit 1.2. 

Sati and Hawkeye, TagYerIt server admins deluxe, have provided feedback and
some funny death messages.  Sati's questions about having to add the same
obits to multiple situations prompted the obit groups capability.

EAVY and I had some good email conversations about addressing the gender/model
issue.  He went the simpler and less autocratic route; to each their own ;-)
