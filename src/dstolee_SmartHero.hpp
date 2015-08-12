/*
 * SmartHero.hpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#ifndef SMARTHERO_HPP_
#define SMARTHERO_HPP_

#include "Actor.hpp"
#include "GraphMap.hpp"

namespace dstolee
{
class SmartHero: public Actor
{
	protected:
		SmartHero* base;
		SmartHero** children;
		int num_children;
		int size_children;
		int* child_verts;

		bool do_improve;
		bool use_fw;
		int cur_turn;
		int cur_pos;
		int cur_x;
		int cur_y;
		int next_neighbor; // Set by base

		int num_verts;
		int* avoid_dists;
		int* search_dists;
		int* search_prev;
		int* child_to_visit;
		int* child_v_targets;

		/**
		 * Called on the base, used for deciding what ALL of the SmartHeroes should do.
		 */
		void makeDecisions( GraphMap* map, int turn );

		void computeAvoidDists( GraphMap* map );
		void computeSearchDists(GraphMap* map, int child_i);
		void selectEatingMove( GraphMap* map, int child_i );
		void selectHidingMove( GraphMap* map, int child_i );
		void selectHoveringMove( GraphMap* map, int child_i );

	public:
		SmartHero();
		virtual ~SmartHero();

		/**
		 * This is the most important method to implement!
		 *
		 * Return the index of the neighbor within the list of neighbors.
		 */
		virtual int selectNeighbor( GraphMap* map, int cur_x, int cur_y );

		/**
		 * Create a new copy of this actor, in the right inherited type!
		 */
		virtual Actor* duplicate();

		/**
		 * Report your netid through your code.
		 *
		 * Useful for later, secret purposes.
		 */
		virtual const char* getNetId();

		/**
		 * Report the name of the actor
		 */
		virtual const char* getActorId();

};
}

#endif /* SMARTHERO_HPP_ */
