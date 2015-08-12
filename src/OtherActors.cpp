/*
 * OtherActors.cpp
 *
 *  Created on: Mar 10, 2014
 *      Author: stolee
 */

#include "OtherActors.hpp"
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <stdlib.h>

RandomActor::RandomActor( int type ) :
		Actor(type)
{
	this->num_to_avoid = 5;
	this->avoid_list = (int*) malloc(this->num_to_avoid * sizeof(int));
	for ( int i = 0; i < this->num_to_avoid; i++ )
	{
		this->avoid_list[i] = -1;
	}
}

RandomActor::~RandomActor()
{
	free(this->avoid_list);
}

int RandomActor::selectNeighbor( GraphMap* map, int x, int y )
{
	int d = map->getNumNeighbors(x, y);

	if ( d <= 1 )
	{
		int p = -1;
		int a, b;
		map->getNeighbor(x, y, 0, a, b);
		p = map->getVertex(a, b);

		for ( int i = this->num_to_avoid - 1; i > 0; i-- )
		{
			this->avoid_list[i] = this->avoid_list[i - 1];
		}
		this->avoid_list[0] = p;

		return 0;
	}

	int choice = -1;
	bool choice_in_avoid_list = false;
	int count = 0;
	int p = -1;

	do
	{
		choice = (rand() % d);

		choice_in_avoid_list = false;
		for ( int i = 0; i < this->num_to_avoid; i++ )
		{
			int a, b;
			map->getNeighbor(x, y, choice, a, b);

			p = map->getVertex(a, b);

			if ( this->avoid_list[i] == p )
			{
				choice_in_avoid_list = true;
			}
		}
		count++;
	}
	while ( count < 5 && choice_in_avoid_list );

	for ( int i = this->num_to_avoid - 1; i > 0; i-- )
	{
		this->avoid_list[i] = this->avoid_list[i - 1];
	}
	this->avoid_list[0] = p;

	return choice;
}

KeyboardActor::KeyboardActor( int type ) :
		Actor(type)
{

}

KeyboardActor::~KeyboardActor()
{

}



int KeyboardActor::selectNeighbor( GraphMap* map, int x, int y )
{
	int d = map->getNumNeighbors(x, y);

	if ( d <= 1 )
	{
		return 0;
	}

	timeout(10);

	int c = getch();

	int a = x;
	int b = y;

	switch ( c )
	{
		case 'h':
		case 'H':
		case '4':
		case KEY_LEFT:
			a = (x + map->getWidth() - 1) % map->getWidth();
			break;

		case 'j':
		case 'J':
		case '2':
		case KEY_DOWN:
			b = (y + 1) % map->getHeight();
			break;

		case 'k':
		case 'K':
		case '8':
		case KEY_UP:
			b = (y + map->getHeight() - 1) % map->getHeight();
			break;

		case 'l':
		case 'L':
		case '6':
		case KEY_RIGHT:
			a = (x + 1) % map->getWidth();
			break;

		case ' ':
			return d - 1; // the teleport!
	};  // switch

	for ( int i = 0; i < d; i++ )
	{
		int p, q;
		map->getNeighbor(x, y, i, p, q);

		if ( p == a && q == b )
		{
			return i;
		}
	}

	return 0;
}

Actor* RandomActor::duplicate()
{
	return new RandomActor(this->getType());
}

Actor* KeyboardActor::duplicate()
{
	return new KeyboardActor(this->getType());
}


const char* KeyboardActor::getActorId()
{
	return "keyboard";
}


const char* RandomActor::getActorId()
{
	return "random";
}




