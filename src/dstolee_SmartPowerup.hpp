/*
 * SmartPowerup.hpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#ifndef SMARTPOWERUP_HPP_
#define SMARTPOWERUP_HPP_

#include "Actor.hpp"
#include "GraphMap.hpp"
#include "FloydWarshall.hpp"

namespace dstolee
{
class SmartPowerup: public Actor
{
	protected:
		SmartPowerup* base;
		SmartPowerup** children;
		int num_children;
		int size_children;

		FloydWarshall* fw;

	public:
		SmartPowerup();
		virtual ~SmartPowerup();

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

#endif /* SMARTPOWERUP_HPP_ */
