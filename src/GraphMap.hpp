/*
 * GraphMap.hpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */

#ifndef GRAPHMAP_HPP_
#define GRAPHMAP_HPP_

#include <stdio.h>

#define SPECIAL_HERO_SPAWN 1
#define SPECIAL_ENEMY_SPAWN 2
#define SPECIAL_EATABLE_SPAWN 4
#define SPECIAL_POWERUP_SPAWN 8

class GameManager;
class Melee;

/**
 * The Graph class contains a simple data structure for reading from a (directed) graph.
 *
 * It is limited to only storing out-edges and out-degree information.
 *
 * This graph is also used to store the 2D map.
 */
class GraphMap
{
		/**
		 * Give GameManager special privileges to access protected members of GraphMap.
		 */
		friend class GameManager;
		friend class Melee;
	protected:
		int num_vertices;
		int* vertex_x;
		int* vertex_y;
		int** xy_vertex;
		int* out_degrees;
		int** out_edges;
		bool toroidal;

		void initGraph( int w, int h );
		void addEdge( int x, int y, int a, int b );

		int w;
		int h;

		int size_special;
		int num_special;
		int* special_vertices;
		int* special_types;

		int size_actors;
		int num_actors;
		int* actor_positions;
		int* actor_types;

		int delay_hero;
		int delay_eatable;
		int delay_powerup;
		int delay_enemy;

		/**
		 * Modify the actors, used by GameManager
		 */
		int addActor( int type, int x, int y );
		bool moveActor( int actor, int x, int y, bool force );
		int setActorType( int i, int type );

		/**
		 * Special positions, used by GameManager.
		 */
		int getNumSpecial();
		int getSpecialType( int i );

		int** map_chars;
		int getMapChar( int x, int y );


	public:

		GraphMap(const GraphMap& map);

		/**
		 * Load a graph from a file, using the game-board format.
		 */
		GraphMap( FILE* f );

		/**
		 * Destructor
		 */
		virtual ~GraphMap();

		/**
		 * Returns the number of vertices in the graph.
		 */
		int getNumVertices();

		/**
		 * Returns the number of out-neighbors for the position (x,y)
		 */
		int getNumNeighbors( int x, int y );

		/**
		 * Assigns the value of the ith neighbor of (x,y) to (a,b).
		 */
		void getNeighbor( int x, int y, int i, int& a, int& b );

		/**
		 * Returns the number of actors in the map.
		 *
		 * All actors are stored in an array, and this gives the length of the array.
		 */
		int getNumActors();

		/**
		 * Returns the type int for the ith actor.
		 */
		int getActorType( int i );

		/**
		 * Given an actor index i, assigns (x,y) to be the position.
		 */
		void getActorPosition( int i, int& x, int& y );

		inline int getWidth()
		{
			return this->w;
		}
		inline int getHeight()
		{
			return this->h;
		}
		int getVertex( int x, int y );
		void getPosition( int v, int& x, int& y );

		int getDelayHero();
		int getDelayEnemy();
		int getDelayEatable();
		int getDelayPowerup();

		void print();
};

#endif /* GRAPH_HPP_ */
