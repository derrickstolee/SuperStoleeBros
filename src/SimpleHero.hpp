/*
 * SimpleHero.hpp
 *
 *  Created on: Apr 4, 2014
 *      Author: dstolee
 */

#ifndef SIMPLEHERO_HPP_
#define SIMPLEHERO_HPP_

#include "Actor.hpp"
#include "GraphMap.hpp"
#include "FloydWarshall.hpp"

namespace dstolee
{
class SimpleHero: public Actor
{
	protected:
		SimpleHero* base;
		FloydWarshall* fw;

	public:
		SimpleHero();
		virtual ~SimpleHero();

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

#endif /* SIMPLEHERO_HPP_ */
