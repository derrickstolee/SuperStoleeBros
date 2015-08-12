/*
 * ActorFactory.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: dstolee
 */

#ifndef ACTORFACTORY_HPP_
#define ACTORFACTORY_HPP_

#include "Actor.hpp"

class ActorFactory
{
	protected:
		Actor** actors;
		int num_actors;
		int size_actors;

		int list_count;
		int size_list;
		char** actor_id_for_list;
		char*** lists;
		int* num_in_list;

		int size_actor_ids;
		int num_actor_ids;
		char** actor_ids;


	public:
		ActorFactory();
		virtual ~ActorFactory();

		const char** getAllNetIdsForActor( const char* actorid, int& num_netids );
		Actor* getActor( const char* netid, const char* actorid );

		const char** getAllActorIds(int& num_actorids);
};

#endif /* ACTORFACTORY_HPP_ */
