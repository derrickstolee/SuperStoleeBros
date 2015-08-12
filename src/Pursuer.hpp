/*
 * Pursuer.hpp
 *
 *  Created on: Mar 25, 2014
 *      Author: dstolee
 */

#ifndef PURSUER_HPP_
#define PURSUER_HPP_


#include "Actor.hpp"

class Pursuer : public Actor
{
	private:
		int pursue_type;

	public:
		Pursuer(int type, int pursue_type);
		virtual ~Pursuer();
		virtual int selectNeighbor(GraphMap* map, int x, int y);
		virtual Actor* duplicate();
		virtual const char* getActorId();
};




#endif /* PURSUER_HPP_ */
