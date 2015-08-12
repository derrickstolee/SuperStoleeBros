Super Stolee Bros is a completely original video game*. The goal is for the heroe(s) to eat all of the “eatable” items in the map, but to not be eaten by any of the enemies! Right now, the game is
built for the user to control the heroe(s) by the keyboard, but your job is to create AI2 to replace the heroes, enemies, and even the powerups! Each instance of the game takes place on a 2D map that is placed on a grid. Certain locations
are marked as walls, and you cannot walk through walls. Otherwise, you can typically walk left, right, up, down, or even stay put, but in certain places there are “teleporters” that allow a jump to
another place in the map. There are also “treadmills” that force movement in one direction. The map is stored in a data structure called a GraphMap that stores lists of the possible moves from
each location. Each map is loaded to the game from a text file. 

Here are some basic rules:
• When a hero overlaps an eatable item, that eatable item is removed.
• When a hero overlaps with an enemy (that is not eatable), then the hero is removed.
• The heroes win if all eatable items are removed.
• The enemies win if all heroes are removed, or if time runs out.

Your goal is to create actors that perform certain actions:
• simplehero : This hero will eat all eatables, given sufficient time, no enemies, and no “fast”
powerups. (This is the only strategy required to be done by Part A.)
• smarthero : This hero will eat all eatables, given sufficient time, “slow” enemies, and no
“fast” powerups.
• smartenemy : This enemy will have a coordinated strategy that will guarantee the enemies
win (by eating the hero, or by making time run out).
• smartpowerup : This powerup will try to make the enemies win by avoiding the hero and
“hiding” near the enemies.
