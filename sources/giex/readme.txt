About
Introduction
Long time ago, I felt the urge to improve upon the depth of the cooperative game mode of Quake2. I also enjoyed playing role playing games of various kinds. After some thinking, the ancestor of Giex - Chacoop - was born.
Chacoop spun off of such games as Diablo and mods like KOTS and Vortex, only it was set in the Quake2 cooperative game mode instead of by the gates of Hell or in Quake2 Deathmatch. Players would fight the Strogg and in doing so gained experience, which in it's turn led to new, improved skills and upgraded weapons.

Giex aims to become the first MMORPG mod for Quake2, by taking the fundamental ideas from Chacoop and enhancing them. With it's - for Quake2 - totally unique class, item and powerup systems, Giex is getting as close to MMORPG as you can possibly get in Quake2.

Features
5 classes - Pick the Soldier if you prefer dealing damage with insanely powerful weapons, or choose the Tech if you feel the need to survive nuclear wars. If you're feeling helpful and like playing supportive roles in a team, or if you simply like being able to keep yourself alive with quite potent healing abilities while ending your foes with shotguns and handgrenades, go for Cleric. If you would prefer to wreak carnage with some quite unique spells, the Mage is just the thing for you. The Vampire is fast and nimble, excellent for hit and run tactics.
Multi-classing - Of course, you are not limited to only these five classes. If you feel you would prefer playing a mix between Tech and Mage, or maybe a combination of all five classes, go ahead; nothing is stopping you!
More than 70 unique abilities - Everything from increasing your blaster damage to learning how to sling fireballs, or protect your team with regenerative auras.
More than 200 items - You'll find old favourites like the Rocket Launcher and Chaingun in Giex. You'll also find completely new weapons, armors and even helmets, like the the Mortar, Gattling cannon, BioArmor and Vampire skull.
5 game modes - Cooperative, Free for all, Team Deathmatch, Domination and Players vs. monsters.
2 sub game modes - Craze, where monsters really are everywhere and Hunt, where you have to team up to take out a single, very powerful, monster.
Heavily improved monster AI - As you gain levels and power, monsters becomes progressively tougher. You'll also learn to hate the new monster abilities, like kamikaze flyers and Jorg's Blackhole spell. Monsters also mix it up quite nicely in classical "players only" game modes, like Free for all.
Persistant character information - Of course, your character will be kept intact, complete with abilities and inventory, between your playing sessions.
The team
Chamooze	Lead development/concepts
R1CH	Co-development/Generic good guy
Zheele	Hud programming/ideas
Virgo	Website logic



Changes
(Version history)
v0.5
New stuff
Added client-side graphics used for inferno, storm, flashlight and a few other places. Giex is now officially no longer server-side only.
New class: Vampire.
New spells: Laser beam, Drone, Storm, Corpse burst, Blaze and Swarm.
New items: A whole bunch of new weapons, armors and other equipment, 214 items now! Thanks to Uhrak and Redeem.
New monster abilities: Stealth and Silenced.
Most grenade launchers and handgrenades now grants the "spell" Detpipes, that instantly explodes all your grenades within range.
Player class Tank renamed to Technician (or Tech, for short).
Tech now gets a slight level-based max armor bonus.
Tech gets level based damage bonuses to Laser Grid, Laser Beam and Drone, twice as high as Mage bonus. (Note that these spells have quite low base damages)
Armor affinity tuned down slightly.
Mage no longer gets bonus to Laser Grid.
Identify shows some new stuff.
Lieutenant monsters in Coop.
Silencer now lowers the chance that monsters will turn agressive when you attack one of their own kind.
On maps with many monster spawn points, the limit to number of monsters are automatically increased making for more monsters on bigger maps.
Whole bunch of other, small stuff.
 
Bug fixes
Identify has been tweaked in various ways, which should result in less traffic (and fewer overflows).
Fixed bug with Armor affinity that was severely limiting the effectiveness of Tech armor.
Fixed bug where you could gain higher levels in a class than you were supposed to, resulting in quite nasty things. Max level for all classes is 250.
Fixed bug with Gib rain, where your magic started to regen while still casting it.
Certain of the latest added items couldn't spawn due to mis-numbering.
Hopefully fixed so you can't crater team-mates with powerful grenade launchers.
Fixed so Medics correctly revives monsters at their own skill level, instead of the Medic's.
HUD in Coop now contains same things as in DM, including identify.
Fixed crash bug with tank commander body in Coop.
"cmd help" now toggles between Help computer and Score board in Coop.
Whole bunch of other, small fixes.
 
Tweaks
All curves for class-level based bonuses have been flattened out, resulting in slower advancement (ie. much weaker high level players).
Further reduced the Tech's bonus for weapons.
Mage gets slightly more magic per level.
Decreased the magic cost for spells above level 40 to double the normal cost-increase per level.
Changed "protective behaviour" of monsters. All monsters (regardless of being aggressive or not) will attack you, if they see you attack one of their own kin.
Monster health and damage values retweaked roughly according to the new, "flatter" bonus curves.
Changed monster skill level formula, so you have to be much higher level for monsters to max out at skill 30.
Capped exp loss multiplier you receive from your kills vs. deaths ratio.
Laser Grid damage increased, but capped max amount of damage it can deal.
Increased damage of Plague bomb, added a cap to total amount of damage it can deal.
Max Magic lowered from 15 to 10 extra magic per level.
Reduced magic cost of Gib rain.
Changed Corpse spores to track the target under the caster's reticle, if no target they fall dead on the floor.
Healing teammates now give exp to your Cleric-level, even if the "patient" has high health.
Increased drops in Coop.
Whole bunch of other, small tweaks.
v0.452
New stuff
Added a bunch of new items, thanks to Uhrak. :)
New submode: Monster hunt. Similar to craze, you can "vote hunt" at most every 6 maps. Monster hunt is kind of the opposite to craze though... Have fun! :)
Exp penalty reduced to a tenth in craze and removed completely in monster hunt.
All monsters are aggressive in craze (but will still not attack players that are too low level).
All players get Identify. Special items can increase amount of information displayed. This feature is experimental and may cause overflows, I'll disable it if the problems become too great.
New monster fighting and "strolling" code.
The "base" lieutenant monster enhancements have been lowered, instead they may get random special abilities. The abilities increases exp gained from killing that monster and also show up on players' HUDs encoded as follows:
H = More health
D = More damage
S = Faster movement
Ex = Explosive weapon resistance (Not BFGs)
Im = Impact weapon resistance (all bullet and pellet weapons)
En = Energy weapon resistance (All blasters, also Rails and BFGs)
Fm = Fire magic resistance
Bm = Blood magic resistance
Lm = Lightning magic resistance
Ra = Regeneration aura
Sa = Shard armor
So for instance, a monster marked as "DSEx" would deal more damage, have faster movement and be more resistant to explosive weapons than normal.
Tank gets damage bonus per class level to "heavy" weapons, such as slow firing RGs, all BFGs and some CGs. This damage bonus is roughly half of what a Soldier gets.
Cleric gets damage bonus per class level to all MGs, SGs and HGs. This damage bonus is a little more than half of that which a Soldier gets.
Mage gets damage bonus per class level to all Wands and Staves.
Some new stuff with Berserker.
Changed how "idle" monsters behave, they now tend to run around the map, instead of just standing about.
dmflags disabled, using hardcoded defaults.
 
Bug fixes
Finally fixed the bug causing problems in equipping items when logging in.
Fixed so HUD displays health, ammo and armor up to 4 digits.
Fixed BFGs all starting at 0 damage.
Fixed players spawning items when killed by teammates.
Fixed bug where wielding a weapon with Armor pierce would make your spells piercing as well.
Fixed so correct skill level should be chosen in Coop.
Fixed player corpses not gibbing when casting corpse spells on un-respawned players? Player corpses are still acting weird with corpse spells.
Fixed player corpses having too low health.
 
Tweaks
Retweaked almost all items.
More than trippled the exp given from fighting Jorgs, Makrons, Tanks and Tank commanders.
Increased timeout for monster corpses slightly.
Lowered kickback from SSG slightly.
Lowered how long time items and powerups stay in the map before they disappear, to help with lag issues in large games.
Armor piercing now gives a slight damage bonus to monsters, to simulate piercing Strogg armor.
Slightly increased overall chances to drop items at higher skill levels.
Increased item drops for Tanks and Gladiators.
Soldier
Reduced weapon damage bonus per level.
Tank
Reduced armour toughness bonus per level.
Cleric
Reduced spell bonus per level.
Mage
Reduced spell bonus per level.
Spells
Further increased magic cost of auras, increased extra magic cost for high level auras.
Reduced all lightning spells' armor piercing to 75% instead of bypassing armor completely.
Reduced Life drain's armor piercing to 100% instead of bypassing armor completely (tank armor can have > 100% protection).
Increased Corpse drain's draining speed by about 50%.
Increased Corpse explosion's base damage, decreased damage gain from corpse health.
v0.42
New stuff
Hurting a player on your team will give you an experience penalty. Note that by default you cannot harm team mates in any of the DM modes, only in Coop.
Healing team members that are low on health now gives you a small Cleric exp reward (no matter what your current class is).
 
Bug fixes
Fixed Spark crashing Windows servers.
Fixed some direct-hit spells passing through windows.
Lightning staff can actually drop now.
 
Tweaks
Slightly decreased the overall exp high level players gain.
Slightly decreased the exp penalty for being killed by a monster.
Decreased the amount of MP gained by each Max magic level to 15.
Cleric
Decreased magic regen gain per level a bunch.
Mage
Decreased magic regen gain per level a bunch.
Spells
Decreased level req per spell level for all spells.
Inferno: Slight increase in damage, slight increase in radius.
Fireball: Decreased blast radius, increased blast damage.
Lightning: Decreased magic cost, decreased damage to monsters.
Spark: Slightly lowered damage per lvl, slight increase in range.
Bolt: Lowered damage to monsters, slightly raised damage per lvl, lowered charge damage bonus alot, lowered charge magic cost alot.
Corpse drain: Changed gradually "suck out" health from corpses instead of instantly sucking out everything.
v0.41
Added some more helmets and a few class-specific, very rare items, we're now up to 91 items! :)
Slightly decreased the base exp cost of all classes.
Increased the portion of exp that goes towards class' exp.
New spells: Spark and Bolt.
Slightly decreased the portion of exp that goes towards spells' exp.
Restored Cleric level requirements for SSGs and lowered them for MGs.
Lowered amount of health you get for each max health level to 8.
Lowered Mage and Cleric's MP gain per level, increased their MP regen gain per level.
Gave Tanks and Tank commanders the ability to pound the ground to cause a damaging, short-range shockwave.
Fixed bug where "wield" would allow you to equip any item.
Fixed bug where item level requirements were calculated incorrectly.
Fixed bug with powerups socketed in items that would sometimes give you much higher skill levels than you should have.
Fixed bug where you would regen health/armor/ammo/magic even when dead.
Fixed baseq2 "feature" of Makron sounds playing over entire level.
Fixed Vampire skull, now compatible with Vampire instead of Ammo regen.
Added delay to weapon and spell bonus sounds, avoiding evil noises.
Slightly increased Handcannon damage.
Fixed Inventory displaying items outside of box.
Fixed so you can't get more than one aura active by quickly switching back and forth between them.
Fixed getting negative exp for healing with medkit.
Fixed bug where Spell affinity would give bonuses to Cleric spells/auras.
Fixed skill levels not updating after unplugging powerup in Memory.
Finally fixed bug with powerups going into items other than the one you chose.
Increased magic cost for Sanctuary and Regeneration auras by about 30%.
Increased magic cost for all spells and auras at high levels.
Changed score board to display player's Team instead of Time.
Fixed so Jorg correctly spawns Makron of same level.
Fixed so Lieutenant monsters correctly has increased chance of dropping higher tier items.
Changed so monsters become non-clipping already while playing death animation.
Fixed so powerups in an item will stop leveling up, if it will cause the item's level requirement to outgrow the player (untested).
v0.4
Total revamp of almost everything! Welcome to the new Giex!
Very dynamic class system, complete with support for multi-classing
Fresh, new item system.
Menus! Giex has never been easier to play. :)