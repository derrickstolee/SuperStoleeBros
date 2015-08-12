/*
 * SmartEnemy.hpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#ifndef SMARTENEMY_HPP_
#define SMARTENEMY_HPP_


#include "Actor.hpp"
#include "GraphMap.hpp"
#include "FloydWarshall.hpp"

namespace dstolee
{
class SmartEnemy : public Actor
{
	protected:
		SmartEnemy* base;
		SmartEnemy** children;
		int num_children;
		int child_id;
		int size_children;

		int pursue_type;
		int avoid_type;
		int* avoid_array;
		int* distance_array;
		int* predecessor_array;

		FloydWarshall* fw;

	public:
		SmartEnemy();
		virtual ~SmartEnemy();

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


#endif /* SMARTENEMY_HPP_ */
