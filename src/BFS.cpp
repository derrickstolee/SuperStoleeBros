/*
 * BFS.cpp
 *
 *  Created on: Apr 5, 2014
 *      Author: stolee
 */

#include "BFS.hpp"
#include "GraphMap.hpp"
#include <queue>
#include <stdlib.h>
#include <string.h>

int* bfs_S = 0;
int bfs_S_len = 0;
int* bfs_p = 0;
int bfs_p_len = 0;
int bfs_last_x = -1;
int bfs_last_y = -1;
int* bfs_avoid = 0;
int bfs_num_avoid = 0;
int* avoid_S = 0;
int* avoid_d = 0;

void freeBFS()
{
	if ( bfs_S != 0 )
	{
		free(bfs_S);
		bfs_S = 0;
	}
	if ( bfs_p != 0 )
	{
		free(bfs_p);
		bfs_p = 0;
	}

	if ( bfs_avoid != 0 )
	{
		free(bfs_avoid);
		bfs_avoid = 0;
	}
	if ( avoid_S != 0 )
	{
		free(avoid_S);
		avoid_S = 0;
	}
	if ( avoid_d != 0 )
	{
		free(avoid_d);
		avoid_d = 0;
	}
}

/**
 * Returns minimum-length path.
 */
int BFS( GraphMap* map, int x, int y, int a, int b, int& first_neighbor )
{
	if ( bfs_S_len != map->getNumVertices() )
	{
		if ( bfs_S != 0 )
		{
			free(bfs_S);
			bfs_S = 0;
		}

		bfs_S = (int*) malloc(map->getNumVertices() * sizeof(int));
		bfs_S_len = map->getNumVertices();

		bfs_p = (int*) malloc(map->getNumVertices() * sizeof(int));
		bfs_p_len = map->getNumVertices();

		bfs_last_x = -1;
		bfs_last_y = -1;
	}

	int v = map->getVertex(x, y);

	if ( bfs_last_x != x || bfs_last_y != y )
	{
		// Re-compute!
		for ( int i = 0; i < bfs_S_len; i++ )
		{
			bfs_p[i] = -1;
			bfs_S[i] = 0;
		}

		bfs_S[v] = 1;
		std::queue<int> q;
		q.push(v);

		while ( q.size() > 0 )
		{
			int u = q.front();
			q.pop();

			int a, b;
			map->getPosition(u, a, b);

			for ( int i = 0; i < map->getNumNeighbors(a, b); i++ )
			{
				int c, d;
				map->getNeighbor(a, b, i, c, d);
				int z = map->getVertex(c, d);

				if ( bfs_S[z] == 0 )
				{
					q.push(z);
					bfs_S[z] = 1;
					bfs_p[z] = u;
				}
			}
		}
	}

	int u = map->getVertex(a, b);

	if ( bfs_p[u] < 0 )
	{
		return bfs_S_len * 2;
	}

	int count = 1;
	while ( bfs_p[u] != v )
	{
		count++;
		u = bfs_p[u];
	}

	first_neighbor = u;
	return count;
}

/**
 * Returns minimum-length path given avoiding certain positions
 */
int BFS( GraphMap* map, int x, int y, int a, int b, int& first_neighbor, int* avoid_pos, int num_avoid )
{
	if ( bfs_S_len != map->getNumVertices() )
	{
		if ( bfs_S != 0 )
		{
			free(bfs_S);
			bfs_S = 0;
		}

		bfs_S = (int*) malloc(map->getNumVertices() * sizeof(int));
		bfs_S_len = map->getNumVertices();

		bfs_p = (int*) malloc(map->getNumVertices() * sizeof(int));
		bfs_p_len = map->getNumVertices();

		bfs_last_x = -1;
		bfs_last_y = -1;
	}

	int v = map->getVertex(x, y);

	bool new_avoid = (bfs_avoid == 0) || (num_avoid == bfs_num_avoid);

	if ( !new_avoid )
	{
		// compare against previous
		for ( int i = 0; !new_avoid && i < num_avoid; i++ )
		{
			new_avoid = (avoid_pos[i] != bfs_avoid[i]);
		}
	}

	if ( new_avoid )
	{
		// compare against previous
		for ( int i = 0; !new_avoid && i < num_avoid; i++ )
		{
			bfs_avoid[i] = avoid_pos[i];
		}

		// Re-compute because of avoidance!
		for ( int i = 0; i < bfs_S_len; i++ )
		{
			bfs_p[i] = -1;
			bfs_S[i] = 0;
		}

		bfs_S[v] = 1;
		std::queue<int> q;
		q.push(v);

		while ( q.size() > 0 )
		{
			int u = q.front();
			q.pop();

			int a, b;
			map->getPosition(u, a, b);

			for ( int i = 0; i < map->getNumNeighbors(a, b); i++ )
			{
				int c, d;
				map->getNeighbor(a, b, i, c, d);
				int z = map->getVertex(c, d);

				bool to_avoid = false;

				for ( int j = 0; !to_avoid && j < num_avoid; j++ )
				{
					if ( z == avoid_pos[j] )
					{
						to_avoid = true;
					}
				}

				if ( !to_avoid && bfs_S[z] == 0 )
				{
					q.push(z);
					bfs_S[z] = 1;
					bfs_p[z] = u;
				}
			}
		}
	}

	int u = map->getVertex(a, b);

	if ( bfs_p[u] < 0 )
	{
		return bfs_S_len * 2;
	}

	int count = 1;
	while ( bfs_p[u] != v )
	{
		count++;
		u = bfs_p[u];
	}

	first_neighbor = u;
	return count;
}

/**
 * Add a list of positions that are reachable from x,y to the given list.
 *
 * May require reallocing the avoid_pos array.
 */
void getRange( GraphMap* map, int x, int y, int radius, int*& avoid_pos, int& num_avoid, int& size_avoid )
{
	if ( avoid_pos == 0 )
	{
		size_avoid = 100;
		num_avoid = 0;
		avoid_pos = (int*) malloc(size_avoid * sizeof(int));
	}

	if ( avoid_S == 0 )
	{
		avoid_S = (int*) malloc(map->getNumVertices() * sizeof(int));
	}
	if ( avoid_d == 0 )
	{
		avoid_d = (int*) malloc(map->getNumVertices() * sizeof(int));
	}

	memset(avoid_S, 0, map->getNumVertices() * sizeof(int));
	memset(avoid_d, 0, map->getNumVertices() * sizeof(int));

	int v = map->getVertex(x, y);

	avoid_d[v] = 0;
	avoid_S[v] = 1;
	std::queue<int> q;
	q.push(v);

	while ( q.size() > 0 )
	{
		int u = q.front();
		q.pop();

		if ( avoid_d[u] >= radius )
		{
			continue;
		}

		int a, b;
		map->getPosition(u, a, b);

		for ( int i = 0; i < map->getNumNeighbors(a, b); i++ )
		{
			int c, d;
			map->getNeighbor(a, b, i, c, d);
			int z = map->getVertex(c, d);

			if ( avoid_S[z] == 0 )
			{
				q.push(z);
				avoid_S[z] = 1;
				avoid_d[z] = avoid_d[u] + 1;

				if ( num_avoid >= size_avoid )
				{
					size_avoid <<= 1;
					avoid_pos = (int*) realloc(avoid_pos, size_avoid * sizeof(int));
				}

				avoid_pos[num_avoid] = z;
				num_avoid++;
			}
		}
	}
}

