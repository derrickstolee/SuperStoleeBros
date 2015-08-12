/*
 * SmartHero.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: stolee
 */

#include "dstolee_SmartHero.hpp"
#include "FloydWarshall.hpp"
#include "BFS.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <queue>

using namespace dstolee;

SmartHero::SmartHero() :
		Actor(ACTOR_HERO)
{
	this->use_fw = true;
	this->size_children = 1000;
	this->num_children = 0;
	this->children = (SmartHero**) malloc(this->size_children * sizeof(SmartHero*));
	this->child_verts = (int*) malloc(this->size_children * sizeof(int));
	this->child_v_targets = (int*) malloc(this->size_children * sizeof(int));

	this->num_verts = 0;
	this->avoid_dists = 0;
	this->search_dists = 0;
	this->search_prev = 0;
	this->child_to_visit = 0;

	this->do_improve = true;
	this->next_neighbor = 0;
	this->cur_pos = -1;
	this->cur_x = -1;
	this->cur_y = -1;

	this->cur_turn = -1;
	this->base = this;
}

SmartHero::~SmartHero()
{
	free(this->child_verts);
	free(this->child_v_targets);

	free(this->children);

	if ( this->num_verts > 0 )
	{
		free(this->search_dists);
		free(this->search_prev);
		free(this->avoid_dists);
		free(this->child_to_visit);
	}
}

/**
 * Create a new copy of this actor, in the right inherited type!
 */
Actor* SmartHero::duplicate()
{
	SmartHero* a = new SmartHero();

	// If you are calling duplicate on me, then I want a direct link to the children!
	a->base = this;
	this->children[this->num_children] = a;
	this->child_verts[this->num_children] = -1;

	(this->num_children)++;

	return a;
}

/**
 * Report your netid through your code.
 *
 * Useful for later, secret purposes.
 */
const char* SmartHero::getNetId()
{
	return "dstolee";
}

/**
 * Report the name of the actor
 */
const char* SmartHero::getActorId()
{
	return "smarthero";
}

/**
 * This is the most important method to implement!
 *
 * Return the index of the neighbor within the list of neighbors.
 */
int SmartHero::selectNeighbor( GraphMap* map, int x, int y )
{
//	wmove(stdscr, y, x);
//	waddch(stdscr, '!');
//	refresh();

	if ( map->getNumNeighbors(x, y) <= 1 )
	{
		return 0;
	}

	int avoid_type = ACTOR_ENEMY;
	int pursue_type = ACTOR_EATABLE;

	int n = map->getNumVertices();
	int* avoid_dist = (int*) malloc(n * sizeof(int));

	int null_dist = n * n * map->getDelayEnemy();
	for ( int i = 0; i < n; i++ )
	{
		avoid_dist[i] = null_dist;
	}
	std::queue<int> q;

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		if ( (map->getActorType(i) & (avoid_type | ACTOR_EATABLE | ACTOR_DEAD)) == avoid_type )
		{
			int a, b, v;
			map->getActorPosition(i, a, b);
			v = map->getVertex(a, b);

			avoid_dist[v] = 1 - map->getDelayEnemy();
			q.push(v);
		}
	}

	while ( q.size() > 0 )
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

			if ( avoid_dist[u] == null_dist )
			{
				// Mark as visited, mark reverse edge, and push!
				avoid_dist[u] = avoid_dist[v] + map->getDelayEnemy();
				q.push(u);
			}
		}
	}

// Otherwise, move TOWARDS the type we are looking for! (Whatever that means!)
	int cur_vert = map->getVertex(x, y);

	int goal_vert = -1;

	int* revverts = (int*) malloc(n * sizeof(int));
	int* dists = (int*) malloc(n * sizeof(int));
	bool* visited = (bool*) malloc(n * sizeof(bool));

	null_dist = n * n * map->getDelayHero();
	for ( int i = 0; i < n; i++ )
	{
		dists[i] = null_dist;
		revverts[i] = -1;
		visited[i] = false;
	}

	int goal_target = pursue_type;

	q.push(cur_vert);
	revverts[cur_vert] = cur_vert;
	visited[cur_vert] = true;

	dists[cur_vert] = 1 - map->getDelayHero();

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

			if ( visited[u] == false && dists[v] + map->getDelayHero() + 1 < avoid_dist[u] )
			{
				// Mark as visited, mark reverse edge, and push!
				visited[u] = true;
				revverts[u] = v;
				dists[u] = dists[v] + map->getDelayHero();
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

	int result = 0;

	if ( !found_hero )
	{
		int best_dist = 0;
		// look for farthest thing from enemies that we can get to!
		for ( int i = 0; i < n; i++ )
		{
			if ( avoid_dist[i] > best_dist && visited[i] )
			{
				best_dist = avoid_dist[i];
				int z = i;
				while ( revverts[z] != cur_vert )
				{
					z = revverts[z];
				}

				goal_vert = z;
			}
		}
	}

	for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
	{
		int a, b;
		map->getNeighbor(x, y, i, a, b);

		if ( map->getVertex(a, b) == goal_vert )
		{
			result = i;
			break;
		}
	}

	free(avoid_dist);
	free(revverts);
	free(visited);
	free(dists);

	return result;
}

void SmartHero::computeAvoidDists( GraphMap* map )
{
	std::queue<int> q;

	for ( int i = 0; i < this->num_verts; i++ )
	{
		this->avoid_dists[i] = this->num_verts;
	}

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		if ( (map->getActorType(i) & (ACTOR_ENEMY | ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_ENEMY )
		{
			int a, b, v;
			map->getActorPosition(i, a, b);
			v = map->getVertex(a, b);

			this->avoid_dists[v] = 1 - map->getDelayEnemy();
			q.push(v);
		}
	}

	while ( q.size() > 0 )
	{
		int v = q.front();
		q.pop();

		int x, y;
		map->getPosition(v, x, y);

		for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
		{
			int a, b, u;
			map->getNeighbor(x, y, i, a, b);
			u = map->getVertex(a, b);

			if ( avoid_dists[u] < this->num_verts )
			{
				continue;
			}

			avoid_dists[u] = avoid_dists[v] + map->getDelayEnemy();
			q.push(u);
		}
	}

}

void SmartHero::computeSearchDists( GraphMap* map, int child_i )
{
	int start_v = this->children[child_i]->cur_pos;

	if ( start_v < 0 )
	{
		return;
	}

	std::queue<int> q;

	q.push(start_v);

	for ( int i = 0; i < this->num_verts; i++ )
	{
		this->search_dists[i] = this->num_verts;
		this->search_prev[i] = -1;
	}

	this->search_dists[start_v] = 0;
	this->search_prev[start_v] = start_v;

	while ( q.size() > 0 )
	{
		int v = q.front();
		q.pop();

		int x, y;
		map->getPosition(v, x, y);

		for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
		{
			int a, b, u;
			map->getNeighbor(x, y, i, a, b);
			u = map->getVertex(a, b);

			if ( search_dists[u] < this->num_verts )
			{
				continue;
			}

			if ( search_dists[v] + 1 >= this->avoid_dists[u] )
			{
				continue;
			}
//
//			if ( child_to_visit[u] < child_i )
//			{
//				continue;
//			}

			search_dists[u] = search_dists[v] + 1;
			search_prev[u] = v;

			q.push(u);
		}
	}
}

void SmartHero::selectEatingMove( GraphMap* map, int child_i )
{
	int start_v = this->child_verts[child_i];

	if ( start_v < 0 )
	{
		return;
	}

	this->computeSearchDists(map, child_i);

	int min_dist = this->num_verts;
	this->child_v_targets[child_i] = this->child_verts[child_i];
	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		if ( (map->getActorType(i) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE )
		{
			int a, b, u;

			map->getActorPosition(i, a, b);
			u = map->getVertex(a, b);

			if ( search_dists[u] < min_dist )
			{
				min_dist = search_dists[u];
				this->child_v_targets[child_i] = u;
			}
			else if ( search_dists[u] == min_dist && (map->getActorType(i) & ACTOR_ENEMY) )
			{
				min_dist = search_dists[u];
				this->child_v_targets[child_i] = u;
			}
		}
	}

	int u = this->child_v_targets[child_i];

	if ( u < 0 || this->search_prev[u] < 0 )
	{
		this->children[child_i]->next_neighbor = 0;
		return;
	}

	int count_steps = 0;
	while ( this->search_prev[u] != this->child_verts[child_i] )
	{
		u = this->search_prev[u];
		if ( u < 0 || this->search_prev[u] < 0 || count_steps > num_verts )
		{
			this->children[child_i]->next_neighbor = 0;
			return;
		}

		count_steps++;
	}
	this->child_to_visit[u] = child_i;

	int x, y;
	map->getPosition(start_v, x, y);
	for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
	{
		int a, b;
		map->getNeighbor(x, y, i, a, b);
		int y = map->getVertex(a, b);

		if ( u == y )
		{
			this->children[child_i]->next_neighbor = i;
			this->child_verts[child_i] = y;
			return;
		}
	}
}

void SmartHero::selectHidingMove( GraphMap* map, int child_i )
{
	int start_v = this->child_verts[child_i];

	if ( start_v < 0 )
	{
		return;
	}

	this->computeSearchDists(map, child_i);

	int max_dist = 0;
	int max_dist2 = 0;
	this->child_v_targets[child_i] = this->child_verts[child_i];
	for ( int u = 0; u < map->getNumVertices(); u++ )
	{
		if ( search_dists[u] < this->num_verts && avoid_dists[u] > max_dist )
		{
			max_dist = avoid_dists[u];
			max_dist2 = search_dists[u];
			this->child_v_targets[child_i] = u;
		}
		else if ( search_dists[u] < this->num_verts && avoid_dists[u] == max_dist && search_dists[u] > max_dist2 )
		{
			max_dist2 = search_dists[u];
			this->child_v_targets[child_i] = u;
		}
	}

	int u = this->child_v_targets[child_i];

	if ( u < 0 || this->search_prev[u] < 0 )
	{
		this->children[child_i]->next_neighbor = 0;
		return;
	}

	int count_steps = 0;
	while ( this->search_prev[u] != this->child_verts[child_i] )
	{
		u = this->search_prev[u];
		if ( u < 0 || this->search_prev[u] < 0 || count_steps > num_verts )
		{
			this->children[child_i]->next_neighbor = 0;
			return;
		}

		count_steps++;
	}
	this->child_to_visit[u] = child_i;

	int x, y;
	map->getPosition(start_v, x, y);
	for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
	{
		int a, b;
		map->getNeighbor(x, y, i, a, b);
		int y = map->getVertex(a, b);

		if ( u == y )
		{
			this->children[child_i]->next_neighbor = i;
			this->child_verts[child_i] = y;
			return;
		}
	}
}

void SmartHero::selectHoveringMove( GraphMap* map, int child_i )
{
	int start_v = this->child_verts[child_i];

	if ( start_v < 0 )
	{
		return;
	}

	this->computeSearchDists(map, child_i);

	int goal_hover = 4;
	int min_diff = num_verts;
	int min_dist = 0;

	this->child_v_targets[child_i] = -1;

	for ( int u = 0; u < map->getNumVertices(); u++ )
	{

		if ( search_dists[u] < this->num_verts && abs(avoid_dists[u] - goal_hover) < min_diff )
		{
			min_diff = abs(avoid_dists[u] - goal_hover);
			min_dist = search_dists[u];
			this->child_v_targets[child_i] = u;
		}
		else if ( abs(avoid_dists[u] - goal_hover) == min_diff && search_dists[u] < min_dist )
		{
			min_dist = search_dists[u];
			this->child_v_targets[child_i] = u;
		}
	}

	int u = this->child_v_targets[child_i];

	if ( u < 0 || this->search_prev[u] < 0 )
	{
		this->children[child_i]->next_neighbor = 0;
		return;
	}

	int count_steps = 0;
	while ( this->search_prev[u] != this->child_verts[child_i] )
	{
		u = this->search_prev[u];
		if ( u < 0 || this->search_prev[u] < 0 || count_steps > num_verts )
		{
			this->children[child_i]->next_neighbor = 0;
			return;
		}

		count_steps++;
	}
	this->child_to_visit[u] = child_i;

	int x, y;
	map->getPosition(start_v, x, y);
	for ( int i = 0; i < map->getNumNeighbors(x, y); i++ )
	{
		int a, b;
		map->getNeighbor(x, y, i, a, b);
		int y = map->getVertex(a, b);

		if ( u == y )
		{
			this->children[child_i]->next_neighbor = i;
			this->child_verts[child_i] = y;
			return;
		}
	}

}

/**
 * Called on the base, used for deciding what ALL of the SmartHeroes should do.
 */
void SmartHero::makeDecisions( GraphMap* map, int turn )
{
	bool force_decisions = false;
	for ( int i = 0; i < this->num_children; i++ )
	{
		if ( this->child_verts[i] < 0 && this->children[i]->cur_pos >= 0 )
		{
			this->child_verts[i] = this->children[i]->cur_pos;
			force_decisions = true;
		}
	}

	if ( !force_decisions && turn <= this->cur_turn )
	{
		// Already computed!
		return;
	}

	if ( this->num_verts <= 0 )
	{
		this->num_verts = map->getNumVertices();

		this->child_to_visit = (int*) malloc(this->num_verts * sizeof(int));
		this->avoid_dists = (int*) malloc(this->num_verts * sizeof(int));
		this->search_dists = (int*) malloc(this->num_verts * sizeof(int));
		this->search_prev = (int*) malloc(this->num_verts * sizeof(int));
	}

	this->cur_turn = turn;

	for ( int i = 0; i < this->num_verts; i++ )
	{
		this->child_to_visit[i] = this->num_verts;
		this->avoid_dists[i] = this->num_verts;
		this->search_dists[i] = this->num_verts;
		this->search_prev[i] = -1;
	}

	this->computeAvoidDists(map);

// Compute avoiding positions!
	int cur_i = 0;
	for ( int i = 0; i < this->num_children; i++ )
	{
		if ( ((this->children[i]->getType() & ACTOR_DEAD) == 0) && this->child_verts[i] >= 0 )
		{
			switch ( cur_i % 1 )
			{
				case 0:
					this->selectEatingMove(map, i);
					break;
				case 1:
					this->selectHidingMove(map, i);
					break;
				case 2:
					this->selectHoveringMove(map, i);
					break;
			}
			cur_i++;
		}
	}
}
