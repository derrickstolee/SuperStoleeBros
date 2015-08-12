/*
 * Pursuer.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: dstolee
 */

#include "Pursuer.hpp"

#include <queue>
#include <stdlib.h>

const char* Pursuer::getActorId()
{
	return "pursuer";
}

Actor* Pursuer::duplicate()
{
	return new Pursuer(this->getType(), this->pursue_type);
}

Pursuer::Pursuer( int type, int pursue_type ) :
		Actor(type)
{
	this->pursue_type = pursue_type;
}

Pursuer::~Pursuer()
{

}

int Pursuer::selectNeighbor( GraphMap* map, int x, int y )
{
	if ( map->getNumNeighbors(x, y) <= 1 )
	{
		return 0;
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
						int z = u;
						while ( revverts[z] != cur_vert )
						{
							z = revverts[z];
						}

						goal_vert = z;
					}
				}
			}
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
