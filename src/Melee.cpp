/*
 * Melee.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: dstolee
 */
#include <stdio.h>

#include <ncurses.h>

#include <stdlib.h>
#include <sys/select.h>

#include <string.h>
#include "GameManager.hpp"
#include "Actor.hpp"
#include "OtherActors.hpp"
#include <math.h>
#include <time.h>
#include <queue>
#include "Melee.hpp"

#define UNSELECTED 251
#define SELECTED 252
#define EDIBLE_GAP 20

const int INFO_COLOR = 1;
const int MSGS_COLOR = 2;
const int PLAY_COLOR = 3;
const int MAZE_COLOR = 4;
const int DOTS_COLOR = 5;

const int EDIBLE_CHASER = 6;
const int EYES_CHASER = 7;
const int CHASER_ONE = 8;
const int CHASER_TWO = 9;
const int CHASER_THREE = 10;
const int CHASER_FOUR = 11;
const int HERO_COLOR = 12;

Melee::Melee( int argc, char** argv )
{
	this->factory = new ActorFactory();

	score_for_eatable = 1;
	score_for_powerup = 10;
	score_for_hero = 100;
	score_for_enemy = 100;

	window = 0;
	current_move = 0;
	total_moves = 1000;

	// Map list is given as a file in the arguments.
	this->size_maps = 1000;
	this->num_maps = 0;
	this->map_list = (char**) malloc(this->size_maps * sizeof(char*));

	this->num_teams = 0;
	this->team_colors = 0;
	this->eatable_enemy_color = 0;
	this->team_representatives = 0;
	this->current_teams = 0;

	this->move_when_enemies_are_not_eatable = -1;
	this->eatable_actor = 0;
	this->hero_teams = 0;
	this->enemy_teams = 0;
	this->powerup_teams = 0;

	this->current_map = 0;

	this->num_current_actors = 0;
	this->current_actors = 0;
	this->current_scores = 0;
	this->move_interval = 0;
	this->move_offset = 0;

	this->delay_hero = 1;
	this->delay_eatable = 1;
	this->delay_enemy = 1;
	this->delay_powerup = 1;

	this->delay_in_clocks = 100000;

	// TODO: Read the command-line arguments
	for ( int i = 0; i < argc; i++ )
	{
		// check the argument!
		if ( i < argc - 1 )
		{
			if ( strcmp(argv[i], "--maps") == 0 )
			{
				FILE* mapfile = fopen(argv[i + 1], "r");

				// read lines from mapfile
				unsigned long int buflen = 1000;
				char* buffer = (char*) malloc(buflen);
				while ( getline(&buffer, &buflen, mapfile) > 0 )
				{
					int len = strlen(buffer);
					this->map_list[this->num_maps] = (char*) malloc(len + 1);
					strcpy(this->map_list[this->num_maps], buffer);
					this->map_list[this->num_maps][len - 1] = 0; // remove newline

					(this->num_maps)++;
				}

				free(buffer);
				fclose(mapfile);
			}
			else if ( strcmp(argv[i], "--delay") == 0 )
			{
				this->delay_in_clocks = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--moves") == 0 )
			{
				this->total_moves = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--delay-hero") == 0 )
			{
				this->delay_hero = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--delay-enemy") == 0 )
			{
				this->delay_enemy = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--delay-eatable") == 0 )
			{
				this->delay_eatable = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--delay-powerup") == 0 )
			{
				this->delay_powerup = atoi(argv[i + 1]);
			}
			else if ( strcmp(argv[i], "--window") == 0 )
			{
				this->window = atoi(argv[i + 1]);
			}
		}
	}

	initscr(); /* Start curses mode 		  */
	start_color();

	init_pair(MAZE_COLOR, COLOR_BLUE, COLOR_BLACK);
	init_pair(DOTS_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(PLAY_COLOR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(MSGS_COLOR, COLOR_RED, COLOR_BLACK);

	init_pair(EDIBLE_CHASER, COLOR_BLUE, COLOR_BLACK);
	init_pair(EYES_CHASER, COLOR_WHITE, COLOR_BLACK);
	init_pair(CHASER_ONE, COLOR_RED, COLOR_BLACK);
	init_pair(CHASER_TWO, COLOR_GREEN, COLOR_BLACK);
	init_pair(CHASER_THREE, COLOR_CYAN, COLOR_BLACK);
	init_pair(CHASER_FOUR, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(HERO_COLOR, COLOR_YELLOW, COLOR_BLACK);

	init_pair(UNSELECTED, COLOR_WHITE, COLOR_BLACK);
	init_pair(SELECTED, COLOR_WHITE, COLOR_BLUE);

	noecho();
	cbreak();
	keypad(stdscr, 1);
	clear();
}

Melee::~Melee()
{
	// TODO: clear out things!

	endwin(); /* End curses mode		  */
}

void Melee::clearGame()
{
	delete this->current_map;

	for ( int i = 0; i < this->num_teams; i++ )
	{
		delete this->team_representatives[i];
	}
	free(this->team_representatives);
	free(this->team_colors);

	free(this->hero_teams);
	free(this->enemy_teams);
	free(this->powerup_teams);
	delete this->eatable_actor;

	for ( int i = 0; i < this->num_current_actors; i++ )
	{
		delete this->current_actors[i];
	}
	free(current_actors);
	free(current_scores);
	free(move_interval);
	free(move_offset);
}

void Melee::play()
{
	makeMapChoice();

	makeActorChoices();

	// Fill in initialization stuff?

	while ( !inFinishState() )
	{
		this->render(stdscr);

		this->playOneRound();

		this->render(stdscr);

		if ( this->delay_in_clocks > 0 )
		{
			timeval to;
			to.tv_sec = 0;
			to.tv_usec = this->delay_in_clocks;
			select(0, 0, 0, 0, &to);
		}
	}

	// Tally Final Scores
	for ( int i = 0; i < this->num_current_actors; i++ )
	{
		if ( this->current_map->getActorType(i) & ACTOR_DEAD )
		{
		
		}
		else
		{
			if ( this->current_map->getActorType(i) & ACTOR_HERO )
			{
				this->current_scores[this->current_teams[i]] += this->score_for_hero;
			}
		}
	}

	this->render(stdscr);
	this->renderFinalScores(stdscr);
	this->waitForEnter();

	this->clearGame();
}

void Melee::makeMapChoice()
{
	int max_width = 30;
	bool selected = false;
	int cur_selection = 0;

	while ( !selected )
	{
		wclear(stdscr);
		wmove(stdscr, 0, 0);
		wprintw(stdscr, "Select your map:");

		for ( int i = 0; i < this->num_maps; i++ )
		{
			int len = strlen(map_list[i]);

			if ( len > max_width )
			{
				max_width = len + 1;
			}

			if ( i == cur_selection )
			{
				wmove(stdscr, i + 1, 0);
				waddch(stdscr, '>');
				this->drawStringWithColor(this->map_list[i], 0, SELECTED);
			}
			else
			{
				wmove(stdscr, i + 1, 1);

				this->drawStringWithColor(this->map_list[i], 0, UNSELECTED);
			}
		}

		// Render all map choices
		// Render Selected map
		char* cur_map_name = this->map_list[cur_selection];
		this->renderMap(stdscr, cur_map_name, 0, max_width);

		refresh();

		// read some input
		int c = getch();

		switch ( c )
		{
			case 'j':
			case 'J':
			case '2':
			case KEY_DOWN:
				cur_selection = (cur_selection + 1) % this->num_maps;
				break;

			case 'k':
			case 'K':
			case '8':
			case KEY_UP:
				cur_selection = (cur_selection + this->num_maps - 1) % this->num_maps;
				break;

			case '\n':
				// Selected!
				selected = true;

		};  // switch
	}

	cbreak();
	FILE* f = fopen(this->map_list[cur_selection], "r");
	this->current_map = new GraphMap(f);
	fclose(f);

}

void Melee::makeActorChoices()
{
	// for all actor types and teams
	int max_hero_team = -1;
	int max_enemy_team = -1;
	int max_powerup_team = -1;

	for ( int x = 0; x < this->current_map->w; x++ )
	{
		for ( int y = 0; y < this->current_map->h; y++ )
		{
			Actor* a;
			int val = this->current_map->map_chars[x][y];
			int team = val >> 8;

			switch ( val & 0xFF )
			{
				case '*':
					this->current_map->addActor(ACTOR_EATABLE, x, y);
					break;

				case 'P':
					this->current_map->addActor(ACTOR_EATABLE | ACTOR_POWERUP, x, y);
					if ( team > max_powerup_team )
					{
						max_powerup_team = team;
					}
					break;

				case 'e':
					this->current_map->addActor(ACTOR_ENEMY, x, y);
					if ( team > max_enemy_team )
					{
						max_enemy_team = team;
					}
					break;

				case 'h':
					this->current_map->addActor(ACTOR_HERO, x, y);
					if ( team > max_hero_team )
					{
						max_hero_team = team;
					}
					break;
			}
		}
	}

	this->num_teams = (max_hero_team + max_enemy_team + max_powerup_team) + 3;

	this->num_current_actors = this->current_map->getNumActors();

	this->team_representatives = (Actor**) malloc(this->num_teams * sizeof(Actor*));
	this->team_colors = (int*) malloc(this->num_teams * sizeof(int));

	for ( int i = 0; i < this->num_teams; i++ )
	{
		this->team_colors[i] = 50 + i;
	}

	this->num_teams = 0;

	if ( max_hero_team >= 0 )
	{
		const char* actorid = "smarthero";
		int hero_backgrounds[6] = { COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_WHITE, COLOR_MAGENTA };

		this->hero_teams = (Actor**) malloc((max_hero_team + 1) * sizeof(Actor*));

		// There are heroes! Let's select a hero
		int num_heroes = 0;
		const char** heroes = this->factory->getAllNetIdsForActor(actorid, num_heroes);

		int num_actor_ids;
		const char** actorids = this->factory->getAllActorIds(num_actor_ids);

		for ( int cur_team = 0; cur_team <= max_hero_team; cur_team++ )
		{
			init_pair(this->team_colors[this->num_teams], COLOR_YELLOW, hero_backgrounds[cur_team % 5]);

			bool selected = false;
			int cur_selection = 0;
			int max_width = 20;

			while ( !selected )
			{
				wclear(stdscr);
				wmove(stdscr, 0, 0);
				wprintw(stdscr, "Select your %s for Team %d", actorid, cur_team);

				for ( int i = 0; i < num_heroes; i++ )
				{
					int len = strlen(heroes[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i == cur_selection )
					{
						wmove(stdscr, i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(heroes[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, i + 1, 1);

						this->drawStringWithColor(heroes[i], 0, this->team_colors[this->num_teams]);
					}
				}
				for ( int i = 0; i < num_actor_ids; i++ )
				{
					int len = strlen(actorids[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i + num_heroes == cur_selection )
					{
						wmove(stdscr, num_heroes + i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(actorids[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, num_heroes + i + 1, 1);

						this->drawStringWithColor(actorids[i], 0, this->team_colors[this->num_teams]);
					}
				}

				refresh();

				// read some input
				int c = getch();

				switch ( c )
				{
					case 'j':
					case 'J':
					case '2':
					case KEY_DOWN:
						cur_selection = (cur_selection + 1) % (num_actor_ids + num_heroes);
						break;

					case 'k':
					case 'K':
					case '8':
					case KEY_UP:
						cur_selection = (cur_selection + (num_actor_ids + num_heroes) - 1)
						        % (num_actor_ids + num_heroes);
						break;

					case '\n':
						// Selected!
						selected = true;

				};  // switch
			}

			if ( cur_selection < num_heroes )
			{
				this->hero_teams[cur_team] = this->factory->getActor(heroes[cur_selection], actorid)->duplicate();
			}
			else
			{
				this->hero_teams[cur_team] =
				        this->factory->getActor(0, actorids[cur_selection - num_heroes])->duplicate();
			}

			this->team_representatives[(this->num_teams)++] = this->hero_teams[cur_team];

		}
	}

	if ( max_enemy_team >= 0 )
	{
		const char* actorid = "smartenemy";

		int enemy_backgrounds[6] = { COLOR_BLACK, COLOR_GREEN, COLOR_CYAN, COLOR_MAGENTA,
		COLOR_WHITE,
		                                      COLOR_YELLOW };
		this->enemy_teams = (Actor**) malloc((max_enemy_team + 1) * sizeof(Actor*));

		// There are heroes! Let's select a hero
		int num_enemies = 0;
		const char** enemies = this->factory->getAllNetIdsForActor(actorid, num_enemies);

		int num_actor_ids;
		const char** actorids = this->factory->getAllActorIds(num_actor_ids);

		for ( int cur_team = 0; cur_team <= max_enemy_team; cur_team++ )
		{
			init_pair(this->team_colors[this->num_teams], COLOR_RED, enemy_backgrounds[cur_team % 6]);

			init_pair(short(this->team_colors[this->num_teams] + EDIBLE_GAP), COLOR_CYAN,
			          enemy_backgrounds[cur_team % 6]);

			bool selected = false;
			int cur_selection = 0;
			int max_width = 20;

			while ( !selected )
			{
				wclear(stdscr);
				wmove(stdscr, 0, 0);
				wprintw(stdscr, "Select your %s for Team %d", actorid, cur_team);

				for ( int i = 0; i < num_enemies; i++ )
				{
					int len = strlen(enemies[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i == cur_selection )
					{
						wmove(stdscr, i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(enemies[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, i + 1, 1);

						this->drawStringWithColor(enemies[i], 0, this->team_colors[this->num_teams]);
					}
				}

				for ( int i = 0; i < num_actor_ids; i++ )
				{
					int len = strlen(actorids[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i + num_enemies == cur_selection )
					{
						wmove(stdscr, num_enemies + i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(actorids[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, num_enemies + i + 1, 1);

						this->drawStringWithColor(actorids[i], 0, this->team_colors[this->num_teams]);
					}
				}
				refresh();

				// read some input
				int c = getch();

				switch ( c )
				{
					case 'j':
					case 'J':
					case '2':
					case KEY_DOWN:
						cur_selection = (cur_selection + 1) % (num_actor_ids + num_enemies);
						break;

					case 'k':
					case 'K':
					case '8':
					case KEY_UP:
						cur_selection = (cur_selection + num_actor_ids + num_enemies - 1)
						        % (num_actor_ids + num_enemies);
						break;

					case '\n':
						// Selected!
						selected = true;

				};  // switch
			}

			if ( cur_selection < num_enemies )
			{
				this->enemy_teams[cur_team] = this->factory->getActor(enemies[cur_selection], actorid)->duplicate();
			}
			else
			{
				this->enemy_teams[cur_team] =
				        this->factory->getActor(0, actorids[cur_selection - num_enemies])->duplicate();
			}
			this->team_representatives[(this->num_teams)++] = this->enemy_teams[cur_team];
		}
	}

	if ( max_powerup_team >= 0 )
	{
		const char* actorid = "smartpowerup";

		int powerup_backgrounds[6] =  { COLOR_BLACK, COLOR_RED, COLOR_MAGENTA,
		COLOR_WHITE,
		                                        COLOR_YELLOW };
		this->powerup_teams = (Actor**) malloc((max_powerup_team + 1) * sizeof(Actor*));

		// There are heroes! Let's select a hero
		int num_powerup = 0;
		const char** powerups = this->factory->getAllNetIdsForActor(actorid, num_powerup);

		int num_actor_ids;
		const char** actorids = this->factory->getAllActorIds(num_actor_ids);

		for ( int cur_team = 0; cur_team <= max_powerup_team; cur_team++ )
		{
			init_pair(this->team_colors[this->num_teams], COLOR_GREEN, powerup_backgrounds[cur_team % 5]);

			bool selected = false;
			int cur_selection = 0;
			int max_width = 20;

			while ( !selected )
			{
				wclear(stdscr);
				wmove(stdscr, 0, 0);
				wprintw(stdscr, "Select your %s for Team %d", actorid, cur_team);

				for ( int i = 0; i < num_powerup; i++ )
				{
					int len = strlen(powerups[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i == cur_selection )
					{
						wmove(stdscr, i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(powerups[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, i + 1, 1);

						this->drawStringWithColor(powerups[i], 0, this->team_colors[this->num_teams]);
					}
				}

				for ( int i = 0; i < num_actor_ids; i++ )
				{
					int len = strlen(actorids[i]);

					if ( len > max_width )
					{
						max_width = len + 1;
					}

					if ( i + num_powerup == cur_selection )
					{
						wmove(stdscr, num_powerup + i + 1, 0);
						waddch(stdscr, '>');
						this->drawStringWithColor(actorids[i], A_BOLD, this->team_colors[this->num_teams]);
					}
					else
					{
						wmove(stdscr, num_powerup + i + 1, 1);

						this->drawStringWithColor(actorids[i], 0, this->team_colors[this->num_teams]);
					}
				}
				refresh();

				// read some input
				int c = getch();

				switch ( c )
				{
					case 'j':
					case 'J':
					case '2':
					case KEY_DOWN:
						cur_selection = (cur_selection + 1) % (num_powerup + num_actor_ids);
						break;

					case 'k':
					case 'K':
					case '8':
					case KEY_UP:
						cur_selection = (cur_selection + (num_powerup + num_actor_ids) - 1)
						        % (num_powerup + num_actor_ids);
						break;

					case '\n':
						// Selected!
						selected = true;

				};  // switch
			}

			if ( cur_selection < num_powerup )
			{
				this->powerup_teams[cur_team] = this->factory->getActor(powerups[cur_selection], actorid)->duplicate();
			}
			else
			{
				this->powerup_teams[cur_team] =
				        this->factory->getActor(0, actorids[cur_selection - num_powerup])->duplicate();
			}

			this->team_representatives[(this->num_teams)++] = this->powerup_teams[cur_team];
		}
	}

	{
		// There are heroes! Let's select a hero
		int num_actorids = 0;
		const char** actorids = this->factory->getAllActorIds(num_actorids);

		bool selected = false;
		int cur_selection = 0;
		int max_width = 20;

		while ( !selected )
		{
			wclear(stdscr);
			wmove(stdscr, 0, 0);
			wprintw(stdscr, "Select your Eatable:");

			for ( int i = 0; i < num_actorids; i++ )
			{
				int len = strlen(actorids[i]);

				if ( len > max_width )
				{
					max_width = len + 1;
				}

				if ( i == cur_selection )
				{
					wmove(stdscr, i + 1, 0);
					waddch(stdscr, '>');
					this->drawStringWithColor(actorids[i], 0, SELECTED);
				}
				else
				{
					wmove(stdscr, i + 1, 1);

					this->drawStringWithColor(actorids[i], 0, UNSELECTED);
				}
			}

			refresh();

			// read some input
			int c = getch();

			switch ( c )
			{
				case 'j':
				case 'J':
				case '2':
				case KEY_DOWN:
					cur_selection = (cur_selection + 1) % num_actorids;
					break;

				case 'k':
				case 'K':
				case '8':
				case KEY_UP:
					cur_selection = (cur_selection + num_actorids - 1) % num_actorids;
					break;

				case '\n':
					// Selected!
					selected = true;

			};  // switch
		}

		this->eatable_actor = this->factory->getActor(0, actorids[cur_selection])->duplicate();
	}

	// Now that the actors have been chosen, populate our data structures!
	this->current_actors = (Actor**) malloc(this->num_current_actors * sizeof(Actor*));
	this->current_teams = (int*) malloc(this->num_current_actors * sizeof(int));
	this->move_interval = (int*) malloc(this->num_current_actors * sizeof(int));
	this->move_offset = (int*) malloc(this->num_current_actors * sizeof(int));

	this->num_current_actors = 0;

	for ( int x = 0; x < this->current_map->w; x++ )
	{
		for ( int y = 0; y < this->current_map->h; y++ )
		{
			int full_c = this->current_map->map_chars[x][y];

			char c = (char) full_c & 0xFF;
			int team = (full_c >> 8) & 0xFF;

			switch ( c )
			{
				case 'e':
					this->current_actors[this->num_current_actors] = this->enemy_teams[team]->duplicate();
					this->current_teams[this->num_current_actors] = team + max_hero_team + 1;
					this->move_interval[this->num_current_actors] = this->delay_enemy;
					this->move_offset[this->num_current_actors] = this->num_current_actors % this->delay_enemy;

					this->current_map->setActorType(this->num_current_actors, ACTOR_ENEMY);
					this->current_actors[this->num_current_actors]->setType(ACTOR_ENEMY);

					(this->num_current_actors)++;

					break;

				case 'h':
					this->current_actors[this->num_current_actors] = this->hero_teams[team]->duplicate();
					this->current_teams[this->num_current_actors] = team;
					this->move_interval[this->num_current_actors] = this->delay_hero;
					this->move_offset[this->num_current_actors] = this->num_current_actors % this->delay_hero;

					this->current_map->setActorType(this->num_current_actors, ACTOR_HERO);
					this->current_actors[this->num_current_actors]->setType(ACTOR_HERO);

					(this->num_current_actors)++;
					break;

				case 'P':
					this->current_actors[this->num_current_actors] = this->powerup_teams[team]->duplicate();
					this->current_teams[this->num_current_actors] = team + max_hero_team + max_enemy_team + 2;
					this->move_interval[this->num_current_actors] = this->delay_powerup;
					this->move_offset[this->num_current_actors] = this->num_current_actors % this->delay_powerup;
					this->current_map->setActorType(this->num_current_actors, ACTOR_POWERUP | ACTOR_EATABLE);
					this->current_actors[this->num_current_actors]->setType(ACTOR_POWERUP | ACTOR_EATABLE);
					(this->num_current_actors)++;
					break;

				case '*':
					this->current_actors[this->num_current_actors] = this->eatable_actor->duplicate();

					this->move_interval[this->num_current_actors] = this->delay_eatable;
					this->move_offset[this->num_current_actors] = this->num_current_actors % this->delay_eatable;

					this->current_map->setActorType(this->num_current_actors, ACTOR_EATABLE);
					this->current_actors[this->num_current_actors]->setType(ACTOR_EATABLE);

					this->current_teams[this->num_current_actors] = -1;
					(this->num_current_actors)++;

					break;
			}
		}
	}

	this->current_scores = (int*) malloc(this->num_teams * sizeof(int));
	bzero(this->current_scores, this->num_teams * sizeof(int));

	this->current_move = 0;
}

bool Melee::inFinishState()
{
	if ( this->current_move >= this->total_moves )
	{
		return true;
	}

	int num_heroes = 0;
	int num_enemies = 0;
	int num_eatables = 0;

	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		int type = this->current_map->getActorType(i);

		if ( (type & ACTOR_DEAD) == 0 )
		{
			if ( type & ACTOR_EATABLE )
			{
				num_eatables++;
			}
			if ( type & ACTOR_HERO )
			{
				num_heroes++;
			}
			if ( type & ACTOR_ENEMY )
			{
				num_enemies++;
			}
		}
	}
//
	if ( num_heroes == 0 || num_eatables == 0 )
	{
		return true;
	}
//
//	wmove(stdscr, this->current_map->getHeight(), 0);
//	wprintw(stdscr, "Heroes: %d Enemies: %d Eatables: %d\n", num_heroes, num_enemies, num_eatables);
//	refresh();

	return false;
}

void Melee::playOneRound()
{
	int* prev_pos = (int*) malloc(this->current_map->getNumActors() * sizeof(int));
	int* next_pos = (int*) malloc(this->current_map->getNumActors() * sizeof(int));

	if ( this->current_move == this->move_when_enemies_are_not_eatable )
	{
		for ( int i = 0; i < this->current_map->getNumActors(); i++ )
		{
			if ( this->current_map->getActorType(i) & ACTOR_ENEMY )
			{
				this->current_map->setActorType(i, this->current_map->getActorType(i) & (~ACTOR_EATABLE));
				this->current_actors[i]->setType(this->current_map->getActorType(i));
			}
		}
	}

	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		if ( this->current_map->getActorType(i) & ACTOR_DEAD )
		{
			prev_pos[i] = -1;
			next_pos[i] = -1;
		}
		else
		{
			int x, y, v;
			this->current_map->getActorPosition(i, x, y);
			v = this->current_map->getVertex(x, y);

			prev_pos[i] = v;
			next_pos[i] = v;
		}
	}

	int cur_i = 0;
	// Fill in prev/next choices for each actor.
	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		if ( this->current_map->getActorType(i) & ACTOR_DEAD )
		{
			continue;
		}

		if ( (this->current_move % this->move_interval[i]) == 0 )
		{
			int x, y, v;
			this->current_map->getActorPosition(i, x, y);

			time_t start = time(NULL);
			int n = this->current_actors[i]->selectNeighbor(this->current_map, x, y);
			time_t end = time(NULL);

			int a, b;
			this->current_map->getNeighbor(x, y, n, a, b);

			v = this->current_map->getVertex(a, b);

			if ( (end - start) > 2 || v < 0 )
			{
				// mark this actor as dead and at -1,-1
				int type = this->current_actors[i]->getType() | ACTOR_DEAD;
				this->current_actors[i]->setType(type);
				this->current_map->setActorType(i, type);
				this->current_map->moveActor(i, -1, -1, true);
				next_pos[i] = -1;
//				wmove(stdscr, this->current_map->h + i, 0);
//				wprintw(stdscr, "Removing actor %d from the board!\n", i);
			}
			else
			{
				next_pos[i] = v;

//				if ( this->current_map->getActorType(i) != ACTOR_EATABLE )
//				{
//					wmove(stdscr, this->current_map->h + cur_i, 0);
//					wprintw(stdscr, "Actor %d moving from %d to %d\n", i, prev_pos[i], next_pos[i]);
//					cur_i++;
//				}
			}
		}
	}

	// Determine which moves are possible.
	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		for ( int j = i + 1; j < this->current_map->getNumActors(); j++ )
		{
			bool collision = false;

			collision = next_pos[i] >= 0
			        && ((next_pos[i] == next_pos[j]) || (next_pos[i] == prev_pos[j] && prev_pos[i] == next_pos[j]));

			if ( collision && this->current_map->getActorType(i) == this->current_map->getActorType(j)
			        && this->current_teams[i] != this->current_teams[j] )
			{
				// Moves are nullified!
				next_pos[i] = prev_pos[i];
				next_pos[j] = prev_pos[j];
			}
		}
	}

	// Determine if any actors should be ejected
	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		if ( (this->current_map->getActorType(i) & (ACTOR_ENEMY | ACTOR_EATABLE)) == ACTOR_ENEMY )
		{
			for ( int j = 0; j < this->num_current_actors; j++ )
			{
				bool collision = false;

				collision = next_pos[i] >= 0
				        && ((next_pos[i] == next_pos[j]) || (next_pos[i] == prev_pos[j] && prev_pos[i] == next_pos[j]));

				if ( collision && (this->current_map->getActorType(j) == ACTOR_HERO) )
				{
					// this enemy eats this hero!
					(this->current_scores[this->current_teams[i]]) += this->score_for_hero;

					this->current_actors[j]->setType(this->current_map->getActorType(j) | ACTOR_DEAD);
					this->current_map->setActorType(j, this->current_map->getActorType(j) | ACTOR_DEAD);
					next_pos[j] = -1;

					this->current_map->moveActor(j, -1, -1, true);
				}
			}
		}
	}

	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		if ( (this->current_map->getActorType(i) & (ACTOR_HERO | ACTOR_DEAD)) == ACTOR_HERO )
		{
			for ( int j = 0; j < this->current_map->getNumActors(); j++ )
			{
				bool collision = false;

				collision = (i != j) && next_pos[i] >= 0
				        && ((next_pos[i] == next_pos[j]) || (next_pos[i] == prev_pos[j] && prev_pos[i] == next_pos[j]));

				if ( collision
				        && ((this->current_map->getActorType(j) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE) )
				{
					// this enemy eats this hero!
					if ( this->current_map->getActorType(j) & ACTOR_ENEMY )
					{
						this->current_scores[this->current_teams[i]] += this->score_for_enemy;
					}
					else if ( this->current_map->getActorType(j) & ACTOR_POWERUP )
					{
						this->current_scores[this->current_teams[i]] += this->score_for_powerup;

						this->move_when_enemies_are_not_eatable = this->current_move + this->window;
						for ( int i = 0; i < this->current_map->getNumActors(); i++ )
						{
							if ( this->current_map->getActorType(i) & ACTOR_ENEMY )
							{
								this->current_map->setActorType(i,
								                                this->current_map->getActorType(i) | (ACTOR_EATABLE));
								this->current_actors[i]->setType(this->current_map->getActorType(i));
							}
						}

					}
					else if ( this->current_map->getActorType(j) & ACTOR_EATABLE )
					{
						this->current_scores[this->current_teams[i]] += this->score_for_eatable;
					}
					else
					{
						continue;
					}

					this->current_actors[j]->setType(this->current_map->getActorType(j) | ACTOR_DEAD);
					next_pos[j] = -1;
					this->current_map->setActorType(j, this->current_map->getActorType(j) | ACTOR_DEAD);
					this->current_map->moveActor(j, -1, -1, true);
				}
			}
		}
	}

	// Now, actually make the moves!
	for ( int i = 0; i < this->current_map->getNumActors(); i++ )
	{
		if ( prev_pos[i] != next_pos[i] && next_pos[i] >= 0 )
		{
			int a, b;
			this->current_map->getPosition(next_pos[i], a, b);

			this->current_map->moveActor(i, a, b, true);
		}
	}

	(this->current_move)++;
}

void Melee::drawPos( WINDOW* w, int x, int y )
{
	GraphMap* map = this->current_map;

	int fullc = map->getMapChar(x, y);
	char c = (char) fullc;
	int team = fullc >> 8;

	int color = 0;

// TODO: modify output letter based on geography
	switch ( c )
	{
		case '#':
			color = MAZE_COLOR;
			break;

		case '>':
		case '<':
		case 'v':
		case '^':
			color = CHASER_TWO;
			break;

		case '*':
		case 'P':
		case 'h':
		case 'e':
			c = ' ';
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			color = CHASER_FOUR;
			c = ACS_S9;
			break;

		default:
			color = INFO_COLOR;
	}

	char oldc = c;
	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = map->getActorType(i);

		if ( a == x && b == y && (t & ACTOR_EATABLE) )
		{
			char d = this->current_actors[i]->getImage();

			if ( t & ACTOR_POWERUP )
			{
				// this color would already be set
				color = this->team_colors[this->current_teams[i]];
				c = d;
			}
			else if ( c == oldc )
			{
				// this color would already be set
				color = DOTS_COLOR;
				c = d;
			}
		}
	}

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = this->current_actors[i]->getType();

		if ( a == x && b == y && (t & ACTOR_HERO) )
		{
			char d = this->current_actors[i]->getImage();

			c = d;
			color = this->team_colors[this->current_teams[i]];
		}
	}

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = this->current_actors[i]->getType();

		if ( a == x && b == y && (t & ACTOR_ENEMY) )
		{
			char d = this->current_actors[i]->getImage();

			if ( t & ACTOR_EATABLE )
			{
				color = this->team_colors[this->current_teams[i]] + EDIBLE_GAP;
			}
			else
			{
				color = this->team_colors[this->current_teams[i]];
			}

			c = d;
		}
	}

	waddch(w, c | A_BOLD | COLOR_PAIR(color));
}

void Melee::render( WINDOW* w )
{
	GraphMap* map = this->current_map;

	wclear(w);

	for ( int y = 0; y < map->h; y++ )
	{
		wmove(w, y, 0);
		for ( int x = 0; x < map->w; x++ )
		{
			this->drawPos(w, x, y);
		}
	}
	wmove(w, map->h, 0);
	wprintw(w, "Moves Left: %4d", this->total_moves - this->current_move);

	refresh();
}

void Melee::renderMap( WINDOW* w, char* map_filename, int cur_h, int cur_w )
{
	FILE* f = fopen(map_filename, "r");
	GraphMap* map = new GraphMap(f);
	fclose(f);

	int enemy_backgrounds[6] = { COLOR_BLACK, COLOR_GREEN, COLOR_CYAN, COLOR_MAGENTA,
	COLOR_WHITE,
	                                      COLOR_YELLOW };

	int powerup_backgrounds[6] = { COLOR_BLACK, COLOR_RED, COLOR_MAGENTA, COLOR_WHITE,
	COLOR_YELLOW };

	int hero_backgrounds[6] = { COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_MAGENTA, COLOR_WHITE };

//don't clear!
// wclear(w);

	for ( int y = 0; y < map->h; y++ )
	{
		wmove(w, cur_h + y, cur_w);
		for ( int x = 0; x < map->w; x++ )
		{
			int full_c = map->getMapChar(x, y);

			char c = (char) full_c;
			int team = full_c >> 8;

			int color = 0;

			// TODO: modify output letter based on geography
			switch ( c )
			{
				case '#':
					color = MAZE_COLOR;
					break;

				case '>':
				case '<':
				case 'v':
				case '^':
					color = CHASER_TWO;
					break;

				case '*':
					color = DOTS_COLOR;
					break;

				case 'P':
					color = 100 + team;
					init_pair(color, COLOR_GREEN, powerup_backgrounds[team % 5]);

					// TODO: team color!
					break;

				case 'h':
					color = 150 + team;
					init_pair(color, COLOR_YELLOW, hero_backgrounds[team % 5]);
					// TODO: team color!
					break;

				case 'e':
					color = 200 + team;
					init_pair(color, COLOR_RED, enemy_backgrounds[team % 6]);
					// TODO: team color!
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					color = CHASER_FOUR;
					c = ACS_S9;
					break;

				default:
					color = INFO_COLOR;
			}

			waddch(w, c | A_BOLD | COLOR_PAIR(color));
		}
	}
	refresh();


	delete map;
}

void Melee::renderFinalScores( WINDOW* w )
{
	wclear(w);
	int cur_h = this->current_map->getHeight();

	wmove(w, cur_h++, 0);
	waddstr(w, "GAME OVER");

	for ( int i = 0; i < this->num_teams; i++ )
	{
		char buffer[1024];

		sprintf(buffer, "Team %d: %15s %20s - %5d points\n", i, this->team_representatives[i]->getNetId(),
		        this->team_representatives[i]->getActorId(), this->current_scores[i]);

		wmove(w, cur_h++, 0);
		drawStringWithColor(buffer, 0, this->team_colors[i]);
	}

	refresh();
}

void Melee::waitForEnter()
{
	{
		refresh();
		nocbreak();
		wgetch(stdscr);
		cbreak();
	}
}

void Melee::drawStringWithColor( const char* str, int style, int color )
{
	char* c = (char*) str;

	while ( *c != 0 )
	{
		waddch(stdscr, *c | style | COLOR_PAIR(color));
		c++;
	}
}
