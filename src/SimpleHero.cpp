/*
 * SimpleHero.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: dstolee
 */

#include <queue>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "SimpleHero.hpp"
#include "BFS.hpp"
#include <ncurses.h>

using namespace dstolee;

SimpleHero::SimpleHero() :
		Actor(ACTOR_HERO)
{
	srand(time(NULL));
	this->base = this;
	this->fw = 0;
}

SimpleHero::~SimpleHero()
{
	if ( this->fw != 0 )
	{
		delete this->fw;
	}

	freeBFS();
}

/**
 * This is the most important method to implement!
 *
 * Return the index of the neighbor within the list of neighbors.
 */
int SimpleHero::selectNeighbor( GraphMap* map, int cur_x, int cur_y )
{
	if ( map->getNumNeighbors(cur_x, cur_y) <= 1 )
	{
		return 0;
	}

	if ( map->getNumVertices() < 2000 )
	{
		if ( this->base->fw == 0 )
		{
			// just the once!
			this->base->fw = new FloydWarshall(map);
		}

		int cur_v = map->getVertex(cur_x, cur_y);

		// Select the target!
		int target = -1;
		int target_i = -1;
		double best_dist = map->getNumVertices();

		for ( int i = 0; i < map->getNumActors(); i++ )
		{
			if ( (map->getActorType(i) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE )
			{
				// can this be a target?
				int a, b, u;
				map->getActorPosition(i, a, b);
				u = map->getVertex(a, b);

				if ( this->base->fw->getDistance(cur_v, u) > map->getNumVertices() )
				{
					// Can't reach it, skip it!
					continue;
				}

				bool can_reach = true;
				for ( int j = 0; can_reach && j < map->getNumActors(); j++ )
				{
					if ( i == j )
					{
						continue;
					}

					int y;
					map->getActorPosition(j, a, b);
					y = map->getVertex(a, b);

					if ( (map->getActorType(j) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE
					        && this->base->fw->getDistance(cur_v, y) < map->getNumVertices()
					        && this->base->fw->getDistance(u, y) >= map->getNumVertices() )
					{
						// Can't reach it, skip it!
						can_reach = false;
					}
				}

				if ( can_reach && this->base->fw->getDistance(cur_v, u) < best_dist )
				{
					best_dist = this->base->fw->getDistance(cur_v, u);
					target = u;
					target_i = i;
				}
			}
		}

//		if ( target_i < 0 )
//		{
//			timeval to;
//			to.tv_sec = 10;
//			to.tv_usec = 0;
//			select(0, 0, 0, 0, &to);
//		}

		int* others = (int*) malloc(100 * sizeof(int));
//		int* dists = (int*) malloc(100 * sizeof(int));
		int num_others = 1;
		others[0] = target;

		std::queue<int> q;
		q.push(target);

		// build a weighted collection of possible places!
		while ( q.size() > 0 && num_others < 15 )
		{
			int y = q.front();
			q.pop();

			int a, b;
			map->getPosition(y, a, b);

			for ( int i = 0; i < map->getNumNeighbors(a, b); i++ )
			{
				int aa, bb;
				map->getNeighbor(a, b, i, aa, bb);

				int z = map->getVertex(aa, bb);

				q.push(z);

				others[num_others] = z;
				num_others++;
			}
		}

		int best_i = 0;
		int best_x, best_y;
		best_dist = map->getNumVertices();
		double best_avg_dist = best_dist;

		for ( int i = 0; i < map->getNumNeighbors(cur_x, cur_y); i++ )
		{
			int a, b;
			map->getNeighbor(cur_x, cur_y, i, a, b);
			int u = map->getVertex(a, b);

			double d = this->base->fw->getAverageDistanceTo(u, others, num_others);

			if ( d < best_avg_dist )
			{
				best_i = i;
				best_x = a;
				best_y = b;
				best_avg_dist = d;
				best_dist = this->base->fw->getDistance(u, target);
			}
			else if ( d < best_avg_dist + 0.5 && this->base->fw->getDistance(u, target) < best_dist )
			{
				best_i = i;
				best_x = a;
				best_y = b;
				best_avg_dist = d;
				best_dist = this->base->fw->getDistance(u, target);
			}
		}

//		char buffer[1000];
//		wmove(stdscr, map->getHeight() + 1, 0);
//		sprintf(buffer, "Targeting Eatable %d, Selecting Neighbor %d:(%d,%d), BestDist:%d, BestAvgDist:%lf, %d others.\n", target_i,
//		        best_i, best_x, best_y, best_dist, best_avg_dist, num_others);
//		waddstr(stdscr, buffer);

		free(others);
//		free(dists);
		return best_i;
	}
	else
	{
		// Select the target!
		static int target_i = -1;

		if ( target_i >= 0 && (map->getActorType(target_i) & ACTOR_DEAD) == 0 )
		{
			int choice = -1;
			int a, b;
			map->getActorPosition(target_i, a, b);

			BFS(map, cur_x, cur_y, a, b, choice);

			for ( int j = 0; j < map->getNumNeighbors(cur_x, cur_y); j++ )
			{
				int a, b, y;
				map->getNeighbor(cur_x, cur_y, j, a, b);
				y = map->getVertex(a, b);

				if ( y == choice )
				{
					return j;
				}
			}

			return 0;
		}

		int cur_v = map->getVertex(cur_x, cur_y);

		int num_actors = map->getNumActors();
		int* actor_dists = (int*) malloc(num_actors * sizeof(int));
		int* actor_neighs = (int*) malloc(num_actors * sizeof(int));

		double best_dist = map->getNumVertices();

		for ( int i = 0; i < map->getNumActors(); i++ )
		{
			// can this be a target?
			int a, b, u;
			map->getActorPosition(i, a, b);
			u = map->getVertex(a, b);

			if ( a >= 0 && b >= 0 )
			{
				actor_dists[i] = BFS(map, cur_x, cur_y, a, b, actor_neighs[i]);
			}
			else
			{
				actor_dists[i] = map->getNumVertices();
			}
		}

		for ( int i = 0; i < map->getNumActors(); i++ )
		{
			if ( (map->getActorType(i) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE )
			{
				if ( actor_dists[i] >= best_dist )
				{
					// skip!
					continue;
				}

				int a, b;
				map->getActorPosition(i, a, b);

				bool can_reach = true;
				for ( int j = 0; can_reach && j < map->getNumActors(); j++ )
				{
					if ( i == j )
					{
						continue;
					}

					int c, d;
					map->getActorPosition(j, c, d);
					int n;

					if ( (map->getActorType(j) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE
					        && actor_dists[j] < map->getNumVertices()
					        && BFS(map, a, b, c, d, n) >= map->getNumVertices() )
					{
						// Can't reach it, skip it!
						can_reach = false;
					}
				}

				if ( can_reach && actor_dists[i] < best_dist )
				{
					best_dist = actor_dists[i];
					target_i = i;
				}
			}
		}

		int choice = actor_neighs[target_i];

		free(actor_dists);
		free(actor_neighs);

		for ( int j = 0; j < map->getNumNeighbors(cur_x, cur_y); j++ )
		{
			int a, b, y;
			map->getNeighbor(cur_x, cur_y, j, a, b);
			y = map->getVertex(a, b);

			if ( y == choice )
			{
				return j;
			}
		}

		return 0;
	}
}

/**
 * Create a new copy of this actor, in the right inherited type!
 */
Actor* SimpleHero::duplicate()
{
	SimpleHero* n = new SimpleHero();
	n->base = this->base;

	return n;
}

/**
 * Report your netid through your code.
 *
 * Useful for later, secret purposes.
 */
const char* SimpleHero::getNetId()
{
	return "dstolee";
}

/**
 * Report the name of the actor
 */
const char* SimpleHero::getActorId()
{
	return "simplehero";
}

