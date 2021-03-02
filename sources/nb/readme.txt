About Neuralbot
   ---------------

Neuralbot is an automated Quake2 deathmatch opponent(bot) that uses an artificial neural network to control its actions and a genetic algorithm to train its neural network.(NN)
    The bot is basically totally autonomous; that is no pre-programmed behaviours are included in its AI code.
    The only vaguely non-autonomous aspect to the bot is its find_target function - the bot is hard-coded to select the nearest visible possible-target in it's field of view. This aspect of the bot AI has been programmed in by me because the processing power is not really there to provide the bot with a sense of where the enemy is using just ray-traces.(sight simulation)
    This bot is unique (I think) in this aspect as the only bot that learns all its behaviour. So how does it learn? As mentioned above, the bot uses an artificial neural-network (ANN) to choose what to do next. A genetic algorithm modifies the neural-network so that the bot gets more frags - the bot learns.
    In programming this bot I am not attempting to create the best bot yet for Quake2 or anything like that. This bot is first and foremost an experiment in artificial intelligence.

    Neural Networks
    ---------------

Check out

   'Neural Networks with Java'

for a slightly more indepth explanation, otherwise read on.

Naturally occuring Neural networks
----------------------------------
Neural networks are collections of interacting brain cells called neurons. Each neuron is a fantastically complex information processor. A Neuron can hold a certain electric charge before it 'fires' a signal to the other neurons it is connected to. This signal charges up the neurons it is connected to.
    Neurons connect themselves with other neurons with long growths off the nucleus of the cell called axons. These split up into many sub-branches which terminate in little nubs called synapses. These synapses can latch on to parts of other neurons, establishing an electrical/chemical link between the the neurons. In this way each neuron may be connected to up to around 10000 other neurons.
    Some Neurons can also communicate by other means such as emitting gas that can effect other neurons they are not connected to by synapses. So
    each neuron is influenced by many other neurons, meaning there is no central seat of power or center of influence in the brain.
    The complex thought processes of a brain result from the many interactions of these cells. The human brain has about 100 billion neurons with about 100 - 1000 trillion synapses(connections). The nematode worm Caenorhabditis elegans has only 302 neurons but can move, react to stimuli and even cary out simple pattern recognition to find food.(ta New Scientist).

Artificial Neural networks
----------------------------------
The goal of true artificial intelligence has been around for a long time. And what better way to create the infrastructure of intelligence than to emulate a design that has been proven to work and be versitile by evolution. Artifical neural networks are (usually greatly simplified)models of naturally occuring neural nets that seek to utilise the power and capacity for learning of neural nets. An artificial neural net implemented on a computer usually consists of a grid of stylised neurons and synapses.

 a typical artificial neural network

Note that this artificial neural network is arranged in layers, as opposed to the arrangement of the real thing. This is to simulate the parallelism of neural nets. When the passage of electric charge is being simulated, each layer is proccesed in turn, acting only on the layers below it. This means that each neuron in a layer is no more or less important than any other neuron in the layer because of when the CPU simulated it's activity.
    In my bots' neural networks, every neuron is connected by synapses to every other neuron on the layers directly above and below it. Thus the total number of synapses in the nerual net in the picture following this synapse convention would be 2*4*4 = 32 synapses. My bot has 2-5 neuron layers with (currently) up to 60 neurons in each layer, so its NN has up to 14400 synapses.
    The neural network operates in discrete time-steps; this is what happens in run through of the NN:

The input layer neurons are charged up by the bot's senses(bot sees a person in front of it etc..). The arrows at the top of the picture represent this.
the charge is dissapated through the neural network until the charge reaches the output layer.
if an ouput layer neuron is charged up enough, a corresponding output action occurs - the bot does something. (moves forwards/fires it's weapon etc..) The arrows at the bottom of the picture represent this/
This happens 10 times every second for my bot.
    The behaviour of the NN I use, and hence the bot, is determined by the weights(the strengths) of all the synapses. So you might expect the synapse weights between the 'foot in lava' input neuron, and the 'jump' output neuron to be large, while the synapse weights between the 'wall in front' input neuron, and the 'fire rocket launcher' output neuron to be negative.
    So a new behaviour is learnt when the pattern of synapse weights is changed. Each bot has its own neural-network with a unique set of synapse weights.
    Man i hope that was coherent..
    Here's a visual representation of what a typical bot neural network looks like:
net screenshot

Also here's a quick list of the inputs and outputs that my bot has right now

input/output list

    Genetic Algorithms
    ------------------

Genetic algorithms use ideas from the natural process of evolution to mould populations into a form which is well suited to their environment. This is how the GA I use operates on the Neuralbot's NNs. This GA is called a steady-state genetic algorithm because of it's constant population size.

Roughly every 1-6 minutes, The GA is run on the bots:

The fitness (or success) of each bot is evaluated. The fitness of a bot goes something like this: fitness = 'number of frags since last run of the GA' * 30.
Two parents are chosen from all the bots. The parents are chosen using one of three selection mechanisms. Generally the better the bot, the more chance it has of being chosen as a parent.
All the weights(connection strengths) of the two parent bot's NNs are then encoded into a chromosome, or string of 'DNA' for each parent.

the encoding process

These two pieces of dna are sliced at certain random points and swapped around to create two new pieces of dna from the parent dna.

the crossover process

Two children bots are chosen - criteria: low fitness. The oven-fresh child dna is then converted back into synapse weights with a reverse-encoding process and used to overwrite the children bots' NN weights. At this point mutation can occur: I have coded this as random changes in the weight of a certain number of synapses.(say 500)
So what happens is that every now and then a random change is made in a bot's NN; if the change produces benefits then the change is passed on to other bots; if the change has negative results(as the majority of changes do), the benefits are not likely to be passed on to other bots.

I use a GA as a learning algorithm as they are versatile, robust and very cool :) As Philip D. Wasserman puts it:

"There is a growing recognition that genetic algorithms have a close relationship with artificial neural networks."

I guess you could say GAs are the algorithm nature uses :)

The parallel structure evolving GA
-----------------------------------
I have a second GA running in parallel with the first that evolves an optimum structure for the NN. It does this by operating on encodings of whether a synapse 'exists' or not. The theory is that this GA will evolve away all the unneccesary synapses, and may hopefully lead to encapsualtion between different parts of the neural net. (like between aiming and dodging).

This second GA doesn't really seem to help though : )

neuralbot learning
------------------
My neuralbots(currently) do not learn the same way as humans do. The Neuralbots use genetic-algorithms to learn over many lifetimes while human learning is implemented with a chemical process that acts on synapses and neuron structure over the course of seconds to hours. The bots learn by a process analogous to the way a relatively primitive organism such as an ant 'learns' over millions of years. That is that the synapses and neural network in the bot/ants brain is not altered over its lifetime - all (or nearly all) of the synapses, and the general neural network in a small organism is 'hard-wired' form birth to death. This is instinct. The 'learning' that takes place is spread over many generations, with the weights of all the synapses and the design of the neural network encoded in the organisms' DNA.
    That is not to say that the type of learning I use on the bots cannot be sped up greatly and used to simulate human-type learning. However, human-type - chemical intra-generational - learning could be a lot cooler and I hope to implement that kind of learning soon.

   Installing the bots
   ----------------------------------

Unzip the contents of the zip file nbot_06.zip to your Quake2 directory(the directory with the file Quake2.exe in it), preserving directory structure. With winzip you can make sure directory structure is preserved by ticking the 'Use Folder Names' box.

The file nb.ini should have unzipped to your Quake2 directory. This is a text file that can be opened up with wordpad and contains values that are read in when all the bots are spawned. All the other files should have unzipped to the new 'nb' directory.

The file nb01.bsp (a map file) should have unzipped to the directory Quake2/baseq2/maps. Maps have to be in this directory if you want to use them.

   Training up a bot
   -----------------

The best way to train up some bots is to run Quake2 in dedicated server mode. With dedicated server mode, the only thing that is run is a map with the bots playing on it. This way the computer doesn't have to do player physics and graphics for a client, which means that time can be accelerated a lot more. To start a dedicated server, use the command line command
C:\Quake2\quake2.exe +set dedicated 1 +set deathmatch 1 +set maxclients 100 +set cheats 1 +set game nb +map nb01

you can do this by using the start/run option on the start menu thingey. What i do is set up a shortcut on my desktop. You can do this by right clicking on your background image and selecting new/shortcut. Then paste the command above into the command line box when it pops up.

There should now be a dedictated server ready to use. Enter the command
sv addbot

This will spawn the bots into the game. Note the sv required. This prefix is needed in front of all commands when entering them into a dedicated server.

enter the command
sv cruise

This will accelerate the passage of time in the game.

Leave the bots overnight or for some suitably long period of time.
enter the command
sv savenn somefile

This will save the bot dna to a file called somefile.dna

quit the dedicated server
Now you have some trained up dna saved to a file.
    How long does it take for the bots to learn something observable? Good question. At the least probably a couple of hundred generations, but letting them play for >1000 won't hurt.
A tip: You need to make sure that the level of interaction between the bots is sufficient for learning. Generally this can be accomplished by having more than about 10 bots in a level. If the sumfitness stays at 0 for more than a hundred or so generations, you probably need some more interaction between the bots.

   checking out the trained up bot
   -------------------------------

Go to the start menu, select run, and enter the command:

C:\quake2\quake2 +set game nb +set cheats 1 +set deathmatch 1 +map nb01 +addbot

Once you are in the game, you will notice all the bots blundering around the level. Load up the trained dna by entering the command
loadnn somefile

Where somefile.dna is the dna file to load up.

   Neuralbot console commands
   --------------------------

You can access the console by pressing the tilde key (~) near the top left of your keyboard. Most of these commands can also be entered directly into a dedicated server box. When doing this, you have to include a sv prefix before the command. For example, to spawn some bots into a dedicated server, you need to use the command sv addbot

addbot	spawns some bots into the level.

evolve	manually runs the genetic algorithm.

sevolve	manually runs the NN structure genetic algorithm.

purge	randomize the weights of all the synapses of all the bots. Use with caution

zap	sets the weights of all the synapses of all the bots to 0. Use with caution.

longer	increases the time between runs of the genetic algorithm by 5 seconds.

shorter	decreases the time between runs of the genetic algorithm by 5 seconds.

botinfo	prints out heaps of useful information, including the generation of bot. (the generation of bot is the number of times the genetic algorithm has been run)

freeweapon	use this command by entering
freeweapon X

where X is the number of the weapon to be free, as you select it on the keyboard as usual. For example, to have the rocket launcher free, enter the command 'freeweapon 7'. When a weapon is free, all the bots get one when they spawn, and firing that weapon doesn't deplete ammo. Freeweapon 10 is all weapons free, freeweapon -1 is no weapons free.


faster	increases the speed time flows in the game. This is a bit of a hack. You will notice that your movement physics aren't affected. This is because the human physics code is not part of the code you can change. Sure I could decrease your velocity each server think etc.. but the accelerated time works for the bots and that's what it was for in the first place. The time-acceleration is limited only by the processing power of the computer. My p100 can only get about a two-fold acceleration in practice. My pIII 450 can get about a 10-15 times increase :).

slower	decreases the speed time flows in the game. This uses the same hack as above, and as a consequence the interpolation between positions is all messed up when the acceleration factor drops below 1 - so no super smooth 'Matrix' Quake2. Perhaps someday i'll code this in.

lotsfaster	increases the speed time flows by 10 times.

lotsslower	decreases the speed time flows by 10 times. You can decrease the time accleration factor below 1. (slow down time)

normal	set the speed time flows back to normal.

cruise	sets the time acceleration factor to the value of cruise_speed as specified in nb.ini

obits	Obituaries. Toggles bot death-messages on and off. Also toggles g.a.(genetic algorithm) status reports.

itemtrail	Toggles a bfg trail from the bot to the item they are 'locked on to'.

enemytrail	Toggles a bfg trail from the bot to the enemy they are 'locked on to'.

savenn xxxxx	Save the weights of all the bots' neural-networks to the file xxxxx.dna in your Quake2 directory.

loadnn xxxxx	Load up the file xxxxx.dna and put all the weights into the bots' n.n.s. If thereare more n.n.s saved than bots, the bots' brains will be filled up in order until there are no more unfilled bots. If there are more bots than saved n.n.s, some bots' neural-network weights will not be overwritten.

invisible	toggles invisible mode: when on, the bots can't lock on to you, so they basically can't see you

quickevolve	toggles quickevolve

addrefbot	use the command
addrefbot somefile

where somefile.dna is the dna you want your reference bot to use. See the section on reference bots for more info on them.


removerefbots	removes all reference bots from the game.

evolution	toggles whether the main GA should be run or not.

cam on	turn yourself into a auto-camera thingey. by the way, you can't change back :)

cam follow	if you are a auto-cam, enter this command to become a chasecam.

There's also a built-in console command that you can enter to become a spectator. Spectator mode comes with a chasecam as well :). Type spectator 1 to enter spectator mode and spectator 0 to get back into the deathmatch.

   Saving and loading NN weights
   -----------------

I'm quite proud of this feature. With it you can train up a bot every night for a week, compare bots trained with various different parameters, send the dna files to other people, and perhaps if I ever get around to it view the color-coded neural net of one of your bots(nearly there on this one).
    What the savenn function does is to save all the weights (floating point values) of all the bots into a file. Thus the size of the file is proportional to the number of bots that were running when you save the game. As I said above, If you load more sets of NN weights than you currently have bots running, not all of the saved data can be used. If you load less sets of NN weights than you currently have bots running, only as many bots as there are data for will be loaded up with new weights.
    When you load up some saved dna, the variables HIDDEN_NEURONS and SYNAPSE_LAYERS will be loaded in as well. The new loaded in varibles will overwrite what was specified in nb.ini. So if you load in some dna that uses 4 synapse layers, and then start training up some bots without quiting Q2, the bots will have 4 synapse layers as well.

   Autosave
   -----------------

There's a paramenter called autosave_period in the nb.ini file. The bots will automatically be saved to a file called autosave.dna if the current generation is a multiple of autosave_period.

    nb.ini
   ----------------------

This is an initialization text file that is loaded up when a Botgame is started. Open it up and have a read. The parameters are described in the file.

If this file is no found, some default values will be used.

NOTE: This file must be in your Quake2 directory for the code to see it. (Not in the Quake2/nb directory)

    reference bots
   ----------------------

One of the problems in training up bots is that they tend to learn themselves into a corner, on account of their opponents (the other bots) being so similar to themselves. For example, the bots tend to end up circle-strafing in one direction. Act like a bot and circle-strafe in the same direction and the bots will waste you in narry a second. Circle the other way however, and the bots will probably be quite ineffectual in killing you.

To counter this, I though it would be a good idea to mix up the bot opponents a bit. So I have coded in reference bots. These bots use neural networks just like the other bots, but are not included in the main population, and are not operated on by the GA. They are free to use weights from a dna file different to that of all the main bots in the GA population. Well that's the point of them really. Each reference bot can use a different dna file, which you specify when you create it. By adding to the diversity of bot behaviour in the deathmatch, hopefully the reference bots will lead to better learning from the normal bots.

Again, the only way reference bots effect the normal bots is through providing some different opponents to play against. The reference bot dna will stay the same for as long as they exist, and will not be exchanged in any way with normal bot dna.

To add a reference bot, use the command

addrefbot somefile

where somefile.dna is the dna file you wish to use for the reference bot.

Also you can remove all the reference bots with the command

removerefbots

Having to load up reference bots manually yourself isn't exactly the greatest way to do it. A cooler way to do it would be for reference bots to be loaded up automatically if it appeared that the bots were learning themselves into a corner - if their behaviour is getting too specialised for fighting against themselves.

   Future possibilities
   --------------------

Who knows, perhaps neuralbot might oneday have evolved into a serious deathmatch opponent. One day far far in the future we might see NN contolled bots the standard in terms of non-human deathmatch opponents. NNs or not, I think bots that learn are definitely going to be standard in a few years.

   Meanwhile, here is some stuff that I might get round to doing:
    -------------------------------------------

realtime intragenerational learning algorithms
An external program that would allow you to view the NN. (nearly done :) )
A Quake3Arena port.
setting up a .dna file repository on the web
a bot tournament!
Basically the possibilities are infinite.
Feel free to email or icq me if you have any questions or suggestions (or bugs).


    Nick Chapman / Ono-Sendai
    nickamy@paradise.net.nz
    onosendai@botepidemic.com
    icq no# 19054613

23/November/99

   Credits:
   --------

thanks to iD software for an excellent and excellently alterable game;

Thanks to pondy @ VUW for some excellent advice + conversations.
The bot would undoubtably be a lot worse without you

Thanks to all those coders and non-coders out there I've had conversations with, especially William.

Thanks to the author of the Tangential tutorials, which enabled me to get some proper client-emulation going.

thanks to 'theFatal', the author of Famkebot for releasing your source; I used to have quite a bit of your code in there somewhere, and your code got me started.

thanks to Ryan Feltrin, author of the Eraserbot - even if I didn't end up using your code (I think), your bot is still something to aspire to.

thanks to Blackmane (Andrew Griffin) and the crew at inside3d for the excellent tutorial on using .ini files - that's where I got my code from. Coders Check out inside3d at www.inside3d.com

Thanks to Paul Jordan for the rocking chasecam code

thanks to my friends (and family) for enduring my obsession with NNs and AI.

thanks to mango for my copy of Quake2( can i say that?) and my copy of MSVC++.(and can I say that?)

Also thanks to mango for these lovely quotes:

    On making a neuralbot - "You look to the sky before you can walk"

    On bots being able to learn - "That's impossible"

Shout out to clan dC also.

Thanks to botepidemic (www.botepidemic.com)for hosting the website, posting news about my bot and being a kick-ass source of bot-news.

/*=========================================
    Neuralbot v0.x
    --------------

Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
   If you want to use any of the code for any of your projects, feel free.
   If you release this project, drop me a line as I would like to see what this code has been used for.
   If you wish to use any of this code in a commercial release, you must gain my written consent first.
   The area of bot AI is large, and way more experimentation needs to be done with neural-networks and g.a.s in bots than I could ever do. So I encourage you all to use this code as a base or inspiration or whatever and see if you can squeeze some learning out of a bot.

    Nick Chapman nickamy@paradise.net.nz icq no# 19054613

=========================================*/

back to main page

