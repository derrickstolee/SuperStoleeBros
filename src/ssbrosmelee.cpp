/*
 * ssbrosmelee.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: dstolee
 */

#include "Melee.hpp"

int main( int argc, char** argv )
{
	Melee* m = new Melee(argc, argv);

	while ( true )
	{
		m->play();
	}

	delete m;
	return 0;
}



