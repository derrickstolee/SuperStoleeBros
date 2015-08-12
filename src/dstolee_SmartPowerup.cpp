/*
 * SmartPowerup.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#include <stdlib.h>
#include "dstolee_SmartPowerup.hpp"
#include "BFS.hpp"

using namespace dstolee;

SmartPowerup::SmartPowerup() :
		Actor(ACTOR_HERO)
{
	this->size_children = 1000;
	this->num_children = 0;
	this->children = (SmartPowerup**) malloc(this->size_children * sizeof(SmartPowerup*));

	this->fw = 0;
	this->base = this;
}

SmartPowerup::~SmartPowerup()
{
	if ( this->base == this )
	{
		delete this->fw;
	}

	free(this->children);
}

/**
 * Create a new copy of this actor, in the right inherited type!
 */
Actor* SmartPowerup::duplicate()
{
	SmartPowerup* a = new SmartPowerup();

	a->base = this->base;
	this->children[this->num_children] = a;
	(this->num_children)++;

	return a;
}

/**
 * Report your netid through your code.
 *
 * Useful for later, secret purposes.
 */
const char* SmartPowerup::getNetId()
{
	return "dstolee";
}

/**
 * Report the name of the actor
 */
const char* SmartPowerup::getActorId()
{
	return "smartpowerup";
}

/**
 * This is the most important method to implement!
 *
 * Return the index of the neighbor within the list of neighbors.
 */
int SmartPowerup::selectNeighbor( GraphMap* map, int cur_x, int cur_y )
{
	if ( 1 )
	{
		return random() % map->getNumNeighbors(cur_x, cur_y);
	}


	// Want to get as far as possible from the heroes!
	// First, find the closest one!
	int closest = map->getNumVertices();
	int best_j = 0;

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int type = map->getActorType(i);

		if ( (type & (ACTOR_HERO | ACTOR_DEAD)) == ACTOR_HERO )
		{
			int a, b;
			map->getActorPosition(i, a, b);

			int neigh;
			int dist = BFS(map, a, b, cur_x, cur_y, neigh);
			if ( dist < closest )
			{
				int farthest_neigh_dist = 0;
				for ( int j = 0; j < map->getNumNeighbors(cur_x, cur_y); j++ )
				{
					int aa, bb;

					map->getNeighbor(cur_x, cur_y, j, aa, bb);
					int d = BFS(map, a, b, aa, bb, neigh);
					if ( d > farthest_neigh_dist )
					{
						farthest_neigh_dist = d;
						best_j = j;
					}
				}
			}
		}
	}

	// can we just avoid heroes, or can we also hide behind enemies?
	return best_j;
}

