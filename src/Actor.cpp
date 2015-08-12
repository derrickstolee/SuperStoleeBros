/*
 * Actor.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */

#include "Actor.hpp"

Actor::Actor( int type )
{
	this->type = type;
	this->iteration = 0;
}

/**
 * Destructor
 */
Actor::~Actor()
{
	// do nothing...
}

/**
 * This is the most important method to implement!
 */
int Actor::selectNeighbor( GraphMap* map, int cur_x, int cur_y )
{
	// this actor is dumb.
	// It just picks the first neighbor, which is probably a loop
	// Thus no move unless required to move!
	return 0;
}

/**
 * This will possibly change the type, so you can modify your behavior!
 */
void Actor::setType( int type )
{
	this->type = type;
}

int Actor::getType()
{
	return this->type;
}

/**
 * What is the character to use to indicate this actor?
 */
unsigned char Actor::getImage()
{
	iteration++;

	if ( this->type & ACTOR_HERO )
	{
		if ( iteration % 2 == 0 )
		{
			return 'C';
		}
		else
		{
			return 'O';
		}
	}
	else if ( this->type == ACTOR_EATABLE )
	{
		return '*';
	}
	else if ( this->type & ACTOR_POWERUP )
	{
		return '@';
	}
	else if ( this->type & ACTOR_ENEMY )
	{
		if ( iteration % 2 == 0 )
		{
			return 'E';
		}
		else
		{
			return 'B';
		}
	}

	return '_';
}

/**
 * Create a new copy of this actor, in the right inherited type!
 */
Actor* Actor::duplicate()
{
	return new Actor(this->type);
}

/**
 * Report your netid through your code.
 *
 * Useful for later, secret purposes.
 */
const char* Actor::getNetId()
{
	return "dstolee";
}

const char* Actor::getActorId()
{
	return "actor";
}

