/*
 * FloydWarshall.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: dstolee
 */

#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include "FloydWarshall.hpp"

using namespace dstolee;

FloydWarshall::FloydWarshall( GraphMap* map )
{
	this->map = map;
	this->N = map->getNumVertices();

	this->distances = (int*) malloc(this->N * this->N * sizeof(int));
	this->avoid_distances = (int*) malloc(this->N * this->N * sizeof(int));
	this->avoid_set = 0;
	this->num_avoid = 0;

	for ( int i = 0; i < N * N; i++ )
	{
		this->distances[i] = N * N;
		this->avoid_distances[i] = N * N;
	}

	for ( int i = 0; i < N; i++ )
	{
		int x, y;
		map->getPosition(i, x, y);

		for ( int j = 0; j < map->getNumNeighbors(x, y); j++ )
		{
			int a, b;
			map->getNeighbor(x, y, j, a, b);
			int vj = map->getVertex(a, b);

			this->distances[N * i + vj] = 1;
		}

		this->distances[N * i + i] = 0;
	}

	for ( int k = 0; k < N; k++ )
	{
//		char buffer[100];
//		sprintf(buffer, "k = %10d\n", k);
//		wmove(stdscr, map->getHeight() + 1, 0);
//		waddstr(stdscr, buffer);
//		refresh();

		int a, b;
		map->getPosition(k, a, b);
		if ( map->getNumNeighbors(a, b) == 0 )
		{
			continue;
		}

		for ( int i = 0; i < N; i++ )
		{
			if ( distances[N * i + k] < N )
			{
				for ( int j = 0; j < N; j++ )
				{
					if ( distances[N * i + j] > distances[N * i + k] + distances[N * k + j] )
					{
						distances[N * i + j] = distances[N * i + k] + distances[N * k + j];
					}
				}
			}
		}
	}

	// copy over
	for ( int i = 0; i < N; i++ )
	{
		for ( int j = 0; j < N; j++ )
		{
			this->avoid_distances[N * i + j] = this->distances[N * i + j];
		}
	}
}

FloydWarshall::~FloydWarshall()
{
	free(this->distances);
	free(this->avoid_distances);

	if ( this->avoid_set != 0 )
	{
		free(this->avoid_set);
	}
}

int FloydWarshall::getDistance( int i, int j )
{
	if ( i < 0 || j < 0 )
	{
		return this->N * this->N;
	}

	return this->distances[this->N * i + j];
}

void FloydWarshall::recomputeAvoidDistances( int* avoid, int num_avoid )
{
	bool recompute = false;
	if ( num_avoid != this->num_avoid )
	{
		recompute = true;
	}
	else
	{
		for ( int i = 0; !recompute && i < num_avoid; i++ )
		{
			if ( avoid[i] != this->avoid_set[i] )
			{
				recompute = true;
			}
		}
	}

	if ( !recompute )
	{
		return;
	}

	this->num_avoid = num_avoid;
	this->avoid_set = (int*) realloc(this->avoid_set, this->num_avoid * sizeof(int));

	for ( int i = 0; i < num_avoid; i++ )
	{
		this->avoid_set[i] = avoid[i];
	}

	for ( int i = 0; i < N * N; i++ )
	{
		this->avoid_distances[i] = N * N;
	}

	for ( int i = 0; i < N; i++ )
	{
		int x, y;
		map->getPosition(i, x, y);

		for ( int j = 0; j < map->getNumNeighbors(x, y); j++ )
		{
			int a, b;
			map->getNeighbor(x, y, j, a, b);
			int vj = map->getVertex(a, b);

			bool avoid_j = false;
			for ( int t = 0; t < this->num_avoid; t++ )
			{
				if ( this->avoid_set[t] == vj )
				{
					avoid_j = true;
				}
			}

			if ( avoid_j )
			{
				continue;
			}

			this->avoid_distances[N * i + vj] = 1;
		}

		this->avoid_distances[N * i + i] = 0;
	}

	for ( int k = 0; k < N; k++ )
	{
		int a, b;
		map->getPosition(k, a, b);
		if ( map->getNumNeighbors(a, b) == 0 )
		{
			continue;
		}

		for ( int i = 0; i < N; i++ )
		{
			if ( avoid_distances[N * i + k] < N )
			{
				for ( int j = 0; j < N; j++ )
				{
					if ( avoid_distances[N * i + j] > avoid_distances[N * i + k] + avoid_distances[N * k + j] )
					{
						avoid_distances[N * i + j] = avoid_distances[N * i + k] + avoid_distances[N * k + j];
					}
				}
			}
		}
	}
}

int FloydWarshall::getDistance( int i, int j, int* avoid, int num_avoid )
{
	if ( i < 0 || j < 0 )
	{
		return this->N * this->N;
	}

	this->recomputeAvoidDistances(avoid, num_avoid);

	return this->avoid_distances[this->N * i + j];
}

double FloydWarshall::getAverageDistanceTo( int i, int* others, int num_others )
{
	double total = 0;

	for ( int j = 0; j < num_others; j++ )
	{
		double d = this->getDistance(i, others[j]);

		if ( d >= N )
		{
			return pow(N, 3);
		}

		// far things are even farther!
		total += pow(d, 2);
	}

	return sqrt(total) / double(num_others);
}

double FloydWarshall::getAverageDistanceFrom( int i, int* others, int num_others )
{
	double total = 0;
	for ( int j = 0; j < num_others; j++ )
	{
		double d = this->getDistance(others[j], i);

		if ( d > N )
		{
			return pow(N, 3);
		}

		// far things are even farther!
		total += pow(d, 2);
	}

	return sqrt(total) / double(num_others);
}

