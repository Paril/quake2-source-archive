
 +++++++++++++++++++++++++++++++
 +                             +
 +      SLIGHT MECHANICAL      +
 +         DESTRUCTION         +
 +                             +
 +++++++++++++++++++++++++++++++

Game    : Quake 2 + modified Lazarus DLL
Type    : Single-player unit
Version : 1.0.0
Date    : 27 OCT 2013

Author  : Chris "Musashi" Walker
Web     : http://musashi.backshooters.com
Email   : musashi.planetquake@gmail.com


=====================================================================
 INSTALLATION & PLAY
=====================================================================

1) Extract to your Quake2 directory, using folder names. This will create a folder called smd with all the required files in it.

2) In your config file, bind a key to "+use". This is needed to operate certain computer terminals (a message will pop-up letting you know when you're near them), and can also be used to move any smaller crates you come across.

3) Run the batch file for your skill level of choice (easy/medium/hard).


=====================================================================
 BACKGROUND
=====================================================================

Standing at attention before Major Duhamel, your eyes flick round the drab green briefing room as he paces back and forth. It's just the two of you here – so that means it's top secret, and probably bad news to boot. He stops and turns to regard you gravely.
 
"Before we can execute the big assault on Stroggos, we need to disrupt their supply lines. The less these bastards have to defend that cursed planet with, the better for us."
 
The Major spits noisily before continuing. "There's a strategically vital factory on Delta Pavonis which manufactures weapons, armour, energy cells, spare parts - you name it, it's made there. If we can take it out, it'll be big frowny face time for the Stroggs.

"Trouble is, it's protected by a powerful defence shield - there's no way we can get through it. So we gotta take it down, fast. A large scale assault would tip our hand and cost too many lives. But a single marine on a covert mission? Well, that's a different story..."
 
You get that familiar feeling that you know exactly what's coming.
 
"The defence shield is maintained by a huge Power Core located in an underground facility," says the Major. "The primary access route into this facility is heavily guarded, but there's a back way in through some caverns that lead right to the surface.

"If the Power Core can be overloaded, the explosion will destroy it and take down the shield. And the way to overload the Core is to remove a small Regulator Cube from the main control room - then get the hell outta there. Captain Shaffer and his boys will pick you up once you reach the surface again."

Piece of cake, you think. Yeah, right...

The Major grimaces and sighs. "Just under a week ago, Lieutenant Druckenmiller was sent in to complete this mission. We lost all contact with him three days ago, and have to assume the worst given that the shield is still up."

Major Duhamel pauses, and has the good grace to look slightly apologetic. "You did a great job getting us out of trouble back on Xarkonia IV, and I'm hoping you'll do the same again here. We know that Druckenmiller established a secret equipment cache in the caverns, but no further intel was received from him before it all went silent."

"Method of covert insertion, sir?" you ask.

"Ah, you'll be using the good ol' Mark 9A Drop Pod. Uncomfortable, but it hasn't failed us yet."

Easy for him to say - he won't be the one crammed into it and tossed onto an enemy planet on hisown. Still, that's what they pay you for.

"Any other questions?" asks the Major.

"None, sir".

"Good. You'll be armed with a sub-machinegun and a standard rechargeable energy sidearm. We've also got the latest toy fresh out of prototype stage for you to play with - the Hyperblaster. It's an energy chaingun with no spin-up delay. Not much spare ammo, though, so go easy on the trigger. Good luck."

You give the Major a quick salute, already looking forward to getting your hands on this new gun. He returns your salute, then turns back to his viewscreen. Not one for sentimentality or wasting time; suits you fine.

Out at the spaceport you head to your designated drop ship, where the pilot is already waiting for you. He turns out to be Lt. Venhuis - an old classmate from boot camp. Probably the last friendly face you'll be seeing for a while...


=====================================================================
 MISSION SUMMARY
=====================================================================

* Find the equipment cache left by Lt. Druckenmiller.

* Locate the entrance to the Power Core facility.

* Determine your escape route back to the surface.

* Find and remove the Regulator Cube to overload the Power Core.

* Get out of there - quickly!


=====================================================================
 CREDITS
=====================================================================

* David Hyde and Tony Ferrara, for the fantastic Lazarus mod.
  This unit wouldn't have been possible without it.

* The artists who produced the Lazarus models, skins, textures and sounds used.

* John Fitzgibbons, for the awesome Oblivion texture set which this unit uses heavily.

* Lunaran, OgrO_FiX, the DoD team, U8Poo, Than, and various unknown authors of the other textures and sound files used.

* GrOWL (a.k.a. Koldone), for the cool sky and epic bug-hunting.

* GrOWL, Wiz, Maric, Buzz and Sherm for all the beta-testing.

* Geoffrey DeWan, for his qbsp3 and qvis3 compilers.                 	

* Tim Wright, for his ArghRad compiler.

* Backshooters, for kindly hosting me.

* The late great Iain M. Banks, for the name of this unit.

* My very lovely and patient wife :)


=====================================================================
 CONSTRUCTION
=====================================================================

Base       : New levels from scratch
Editor     : QERadiant (build 147)
Compilers  : qbsp3 and qvis3 (v1.02) by Geoffrey DeWan
             ArghRad (v2.01) by Tim Wright

 +-------+----------+-----------+
 |  Map: | Brushes: | Entities: |
 +-------+----------+-----------+
 |  SMD1 |    3010  |     629   |
 |  SMD2 |    8915  |    2045   |
 |  SMD3 |    5193  |    1075   |
 |  SMD4 |    5034  |    1358   |
 |  SMD5 |    2561  |     392   |
 |  SMD6 |    7822  |    1998   |
 +-------+----------+-----------+
 | TOTAL |   32535  |    7497   |
 +-------+----------+-----------+


=====================================================================
 NOTES
=====================================================================

1) I've only tested this unit on the vanilla Q2 engine; I may not work properly with other engines. And it's not meant for software mode.

2) In the final battle chamber (smd6.bsp), occasionally the boss monster will fall through the floor. I have no idea why, and haven't been able to fix it...!

3) You'll get an Index: overflow error if you use the 'give all' command on the larger maps (due to the number of sound files loaded). So don't cheat ;)


=====================================================================
 COPYRIGHT STUFF
=====================================================================

You may distribute this .zip in any electronic format; it must be free of charge. Please include all files intact in the original archive.

I've included the DLL source - it's a modified version of the Lazarus v2.105 code. Feel free to use it (with the appropriate credit).

If you want to use any of these maps as a base for other levels, send me an email. I'll be releasing the source files at some point...
