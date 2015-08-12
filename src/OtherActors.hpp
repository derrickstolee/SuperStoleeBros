/*
 * OtherActors.hpp
 *
 *  Created on: Mar 10, 2014
 *      Author: stolee
 */

#ifndef OTHERACTORS_HPP_
#define OTHERACTORS_HPP_

#include "Actor.hpp"
#include "GraphMap.hpp"

/**
 * The RandomActor is not purely random.
 *
 * They keep track of a few previous positions
 * that it will try to avoid. But otherwise it
 * will pick a random neighbor that is not in
 * that list. If it fails to pick a neighbor not
 * in that list within 10 attempts, it will just
 * select a random neighbor.
 */
class RandomActor : public Actor
{
	protected:
		int num_to_avoid;
		int* avoid_list;

	public:
		RandomActor(int type);
		virtual ~RandomActor();
		virtual int selectNeighbor(GraphMap* map, int x, int y);
		virtual Actor* duplicate();
		virtual const char* getActorId();
};


class KeyboardActor : public Actor
{
	public:
		KeyboardActor(int type);
		virtual ~KeyboardActor();
		virtual int selectNeighbor(GraphMap* map, int x, int y);
		virtual Actor* duplicate();
		virtual const char* getActorId();
};


#endif /* OTHERACTORS_HPP_ */
