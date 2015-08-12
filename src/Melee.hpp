/*
 * Melee.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: dstolee
 */

#ifndef MELEE_HPP_
#define MELEE_HPP_

#include <ncurses.h>
#include "Actor.hpp"
#include "ActorFactory.hpp"
#include "GraphMap.hpp"

/**
 * The Melee class is the main driver for controlling how the game works for multiple actors simultaneously.
 */
class Melee
{
	protected:
		ActorFactory* factory;

		int size_maps;
		int num_maps;
		char** map_list;

		Actor* eatable_actor;
		Actor** hero_teams;
		Actor** enemy_teams;
		Actor** powerup_teams;

		GraphMap* current_map;

		int eatable_enemy_color;

		int num_teams;
		int* team_colors;
		int* current_scores;
		Actor** team_representatives;

		int num_current_actors;
		Actor** current_actors;
		int* current_teams;
		int* move_interval;
		int* move_offset;

		int delay_hero;
		int delay_eatable;
		int delay_enemy;
		int delay_powerup;
		int delay_in_clocks;

		int score_for_eatable;
		int score_for_powerup;
		int score_for_hero;
		int score_for_enemy;

		int window;
		int move_when_enemies_are_not_eatable;
		int current_move;
		int total_moves;

		void drawPos( WINDOW* w, int x, int y );
		void render( WINDOW* w );

		void renderMap( WINDOW* w, char* map_filename, int cur_h, int cur_w );

		void makeMapChoice();
		void makeActorChoices();

		bool inFinishState();
		void playOneRound();
		void clearGame();

		void renderFinalScores( WINDOW* w );
		void waitForEnter();
		void drawStringWithColor( const char* str, int style, int color );

	public:
		Melee( int argc, char** argv );
		virtual ~Melee();

		void play();

};

#endif /* MELEE_HPP_ */
