/*
 * Actor.hpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */

#ifndef ACTOR_HPP_
#define ACTOR_HPP_

#include "GraphMap.hpp"

// These actor types can be combined to form bitmasks.
#define ACTOR_HERO     1
#define ACTOR_ENEMY    2
#define ACTOR_EATABLE  4
#define ACTOR_POWERUP  8
#define ACTOR_DEAD    16

class GameManager;
class Melee;

class Actor
{
		friend class GameManager;
		friend class Melee;
	private:
		int type;
		int iteration;

		/**
		 * This will possibly change the type, so you can modify your behavior!
		 */
		void setType( int type );


	public:
		Actor( int type );

		/**
		 * Destructor
		 */
		virtual ~Actor();

		/**
		 * This is the most important method to implement!
		 *
		 * Return the index of the neighbor within the list of neighbors.
		 */
		virtual int selectNeighbor( GraphMap* map, int cur_x, int cur_y );

		/**
		 * What is the character to use to indicate this actor?
		 */
		virtual unsigned char getImage();

		/**
		 * Create a new copy of this actor, in the right inherited type!
		 */
		virtual Actor* duplicate();


		int getType();

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

#endif /* ACTOR_HPP_ */
