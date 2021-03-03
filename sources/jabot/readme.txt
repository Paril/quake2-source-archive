 JABot Q2 is a Quake 2 AI. Originally based in ACEbot is almost a total rewrite of it, while still keeps the ACEbot states machine design. JABot more flashy features are: 

- A* pathing in realtime 
- MoveType based pathing: The AI will only find paths using movements its ai class can perform 
- AI support for BOTs and MONSTERs. Through the use of ai classes. 
- weighted goal system. 
- Jump movement/movetype support for Bots class. 
- Bot roams support (using a item_botroam entity, or dropping it from console). 
- Many other improvements in every aspect of the code. 

Unsupported: 
- Bot chat and bot messages have been removed. 

JABotQ2, while usable for players as a standard DM bot, is written for Q2 modders to be used as base for new monsters/bots/actors, allowing to create a modern AI for their mods. JABot is released under GPL licese.  
 

These are only a few. Many other improvements were also done.

Usage commands:
sv makenodes - Start creating a navigation file from scratch
sv editnodes - Modify the loaded navigation file
sv savenodes - Save nodes file
sv addbotroam - Drop a bot roam node
sv addbot <team> <name> <skin> - Spawn a bot (consult ACEbot readme, it's just the same thing)
sv removebot <name> - remove a bot ("all" removes every bot in the map)
sv addmonster - Spawns a monster. It's a testing function. Spawning from map files is not present in the code.

I hope you enjoy it.
