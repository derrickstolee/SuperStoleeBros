/*
 * SmartEnemy.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#include <stdlib.h>
#include <queue>
#include "dstolee_SmartEnemy.hpp"
#include "BFS.hpp"

using namespace dstolee;

void updateDistanceArraySE( GraphMap* map, int from_type, int* d )
{
	int N = map->getNumVertices();
	for ( int i = 0; i < N; i++ )
	{
		d[i] = 2 * N;
	}

	std::queue<int> q;

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		if ( map->getActorType(i) == from_type )
		{
			int x, y, v;
			map->getActorPosition(i, x, y);
			v = map->getVertex(x, y);

			q.push(v);
			d[v] = 0;
		}
	}

	while ( q.size() > 0 )
	{
		int v = q.front();
		q.pop();

		int x, y;
		map->getPosition(v, x, y);

		int deg = map->getNumNeighbors(x, y);
		int offset = rand() % deg;
		for ( int i = 0; i < deg; i++ )
		{
			int a, b, z;

			map->getNeighbor(x, y, (i + offset) % deg, a, b);
			z = map->getVertex(a, b);

			if ( d[z] > N )
			{
				d[z] = d[v] + 1;
				q.push(z);
			}
		}
	}
}

void updateDistanceArraySE( GraphMap* map, int from_x, int from_y, int* d, int* avoid_d, int* pred = 0 )
{
	int N = map->getNumVertices();
	for ( int i = 0; i < N; i++ )
	{
		d[i] = 2 * N;

		if ( pred != 0 )
		{
			pred[i] = -1;
		}
	}

	int start = map->getVertex(from_x, from_y);
	d[start] = 0;
	pred[start] = start;

	std::queue<int> q;
	q.push(start);

	while ( q.size() > 0 )
	{
		int v = q.front();
		q.pop();

		int x, y;
		map->getPosition(v, x, y);

		int deg = map->getNumNeighbors(x, y);
		int offset = rand() % deg;
		for ( int i = 0; i < deg; i++ )
		{
			int a, b, z;

			map->getNeighbor(x, y, (i + offset) % deg, a, b);
			z = map->getVertex(a, b);

			if ( d[z] > N && d[v] + 1 < avoid_d[z] )
			{
				d[z] = d[v] + 1;

				if ( pred != 0 )
				{
					pred[z] = v;
				}

				q.push(z);
			}
		}
	}
}

SmartEnemy::SmartEnemy() :
		Actor(ACTOR_ENEMY)
{
	this->size_children = 1000;
	this->num_children = 0;
	this->children = (SmartEnemy**) malloc(this->size_children * sizeof(SmartEnemy*));
	this->child_id = -1;
	this->fw = 0;
	this->base = this;
	this->pursue_type = ACTOR_HERO;
	this->avoid_type = ACTOR_HERO;


	this->avoid_type = ACTOR_HERO;
	this->avoid_array = 0;
	this->distance_array = 0;
	this->predecessor_array = 0;
}

SmartEnemy::~SmartEnemy()
{
	if ( this->base == this )
	{
		delete this->fw;
	}

	free(this->children);

	freeBFS();
}

/**
 * Create a new copy of this actor, in the right inherited type!
 */
Actor* SmartEnemy::duplicate()
{
	SmartEnemy* a = new SmartEnemy();

	a->base = this->base;
	this->children[this->num_children] = a;
	this->children[this->num_children]->child_id = this->num_children;
	(this->num_children)++;


	return a;
}

/**
 * Report your netid through your code.
 *
 * Useful for later, secret purposes.
 */
const char* SmartEnemy::getNetId()
{
	return "dstolee";
}

/**
 * Report the name of the actor
 */
const char* SmartEnemy::getActorId()
{
	return "smartenemy";
}

/**
 * This is the most important method to implement!
 *
 * Return the index of the neighbor within the list of neighbors.
 */
int SmartEnemy::selectNeighbor( GraphMap* map, int x, int y )
{
	if ( map->getNumNeighbors(x, y) <= 1 )
	{
		return 0;
	}

	if ( this->getType() & ACTOR_EATABLE )
	{
		// AVOID HEROES!
		if ( this->distance_array == 0 )
		{
			this->avoid_array = (int*) malloc(map->getNumVertices() * sizeof(int));
			this->distance_array = (int*) malloc(map->getNumVertices() * sizeof(int));
			this->predecessor_array = (int*) malloc(map->getNumVertices() * sizeof(int));
		}

		updateDistanceArraySE(map, this->avoid_type, this->avoid_array);

//			for ( int y = 0; y < map->getHeight(); y++ )
//			{
//				for ( int x = 0; x < map->getWidth(); x++ )
//				{
//					wmove(stdscr, map->getHeight() + y, 4 * x);
//					char buffer[10];
//					sprintf(buffer, "%4d", this->avoid_array[map->getVertex(x, y)]);
//					waddstr(stdscr, buffer);
//				}
//			}

		updateDistanceArraySE(map, x, y, this->distance_array, this->avoid_array, this->predecessor_array);

		int best_d = 0;
		int best_v = -1;

		for ( int i = 0; i < map->getNumVertices(); i++ )
		{
			if ( this->avoid_array[i] < map->getNumVertices() && this->distance_array[i] < map->getNumVertices()
			        && (this->avoid_array[i] > best_d || (this->avoid_array[i] == best_d && (rand() & 1))) )
			{
				best_d = this->avoid_array[i];
				best_v = i;
			}
		}

		if ( best_v >= 0 )
		{
			int me = map->getVertex(x, y);
			int v = best_v;
			while ( this->predecessor_array[v] != me )
			{
				v = this->predecessor_array[v];
			}

			for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
			{
				int a, b, u;
				map->getNeighbor(x, y, i, a, b);
				u = map->getVertex(a, b);

				if ( u == v )
				{
					return i;
				}
			}
		}

		return 0;
	}

	if ( random() % 100 < 20 )
	{
		return (random() % (map->getNumNeighbors(x, y) - 1)) + 1;
	}
	// Otherwise, move TOWARDS the type we are looking for! (Whatever that means!)
	int cur_vert = map->getVertex(x, y);

	int goal_vert = -1;
	int n = map->getNumVertices();
	int* revverts = (int*) malloc(n * sizeof(int));
	bool* visited = (bool*) malloc(n * sizeof(bool));

	for ( int i = 0; i < n; i++ )
	{
		revverts[i] = -1;
		visited[i] = false;
	}
	std::queue<int> q;

	int goal_target = this->pursue_type;

	q.push(cur_vert);
	revverts[cur_vert] = cur_vert;
	visited[cur_vert] = true;

	bool found_hero = false;
	int found_dist = 0;

	while ( !found_hero && q.size() > 0 )
	{
		int v = q.front();
		q.pop();

		int a, b;
		map->getPosition(v, a, b);
		int d = map->getNumNeighbors(a, b);

		for ( int i = 0; i < d; i++ )
		{
			int u, r, s;
			map->getNeighbor(a, b, i, r, s);
			u = map->getVertex(r, s);

			if ( visited[u] == false )
			{
				// Mark as visited, mark reverse edge, and push!
				visited[u] = true;
				revverts[u] = v;
				q.push(u);

				for ( int j = 0; j < map->getNumActors(); j++ )
				{
					int t = map->getActorType(j);
					int l, k;
					map->getActorPosition(j, l, k);

					if ( l == r && k == s && (t & goal_target) )
					{
						found_hero = true;
						found_dist = 1;
						int z = u;
						while ( revverts[z] != cur_vert )
						{
							found_dist++;
							z = revverts[z];
						}

						goal_vert = z;
					}
				}
			}
		}
	}

	while ( q.size() > 0 )
	{
		q.pop();
	}

	if ( goal_vert < 0 || goal_vert >= n )
	{
		free(visited);
		free(revverts);
		return 0;
	}

	if ( true )
	{
		int old_goal = goal_vert;
		int old_dist = found_dist;

		for ( int i = 0; i < n; i++ )
		{
			revverts[i] = -1;
			visited[i] = false;
		}

		q.push(cur_vert);

		revverts[cur_vert] = cur_vert;
		visited[cur_vert] = true;
		visited[goal_vert] = true;

		goal_vert = -1;

		bool found_hero = false;

		while ( !found_hero && q.size() > 0 )
		{
			int v = q.front();
			q.pop();

			int a, b;
			map->getPosition(v, a, b);
			int d = map->getNumNeighbors(a, b);

			for ( int i = 0; i < d; i++ )
			{
				int u, r, s;
				map->getNeighbor(a, b, i, r, s);
				u = map->getVertex(r, s);

				if ( visited[u] == false )
				{
					// Mark as visited, mark reverse edge, and push!
					visited[u] = true;
					revverts[u] = v;
					q.push(u);

					for ( int j = 0; j < map->getNumActors(); j++ )
					{
						int t = map->getActorType(j);
						int l, k;
						map->getActorPosition(j, l, k);

						if ( l == r && k == s && (t & goal_target) )
						{
							found_hero = true;
							found_dist = 1;
							int z = u;
							while ( revverts[z] != cur_vert )
							{
								found_dist++;
								z = revverts[z];
							}

							goal_vert = z;
						}
					}
				}
			}
		}

		if ( goal_vert < 0 || found_dist > old_dist + 1 )
		{
			goal_vert = old_goal;
		}
	}

	free(revverts);
	free(visited);

	for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
	{
		int a, b;
		map->getNeighbor(x, y, i, a, b);

		if ( map->getVertex(a, b) == goal_vert )
		{
			return i;
		}
	}

	return 0;
}

