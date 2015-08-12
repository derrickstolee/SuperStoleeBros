/*
 * ssbros.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */

#include "GraphMap.hpp"
#include "GameManager.hpp"
#include "Actor.hpp"
#include "OtherActors.hpp"
#include <stdlib.h>
#include "dstolee_SmartHero.cpp"
#include "dstolee_SmartEnemy.cpp"
#include "dstolee_SmartPowerup.cpp"


int main( int argc, char** argv )
{
	Actor** actors = (Actor**) malloc(10 * sizeof(Actor*));

	int num_actors = 3;

	actors[0]=new dstolee::SmartPowerup();
	actors[1]=new dstolee::SmartEnemy();
	actors[2]=new dstolee::SmartHero();

	GameManager* manager = new GameManager(argc, argv, actors, num_actors);
	manager->play();

	delete manager;
	manager = 0;

	for ( int i = 0; i < num_actors; i++ )
	{
		delete actors[i];
		actors[i] = 0;
	}
	free(actors);
	actors = 0;

	return 0;
}
