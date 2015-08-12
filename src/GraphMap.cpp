/*
 * GraphMap.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "GraphMap.hpp"

void GraphMap::initGraph( int w, int h )
{
	this->w = w;
	this->h = h;
	this->num_vertices = w * h;
	this->vertex_x = (int*) malloc(this->num_vertices * sizeof(int));
	this->vertex_y = (int*) malloc(this->num_vertices * sizeof(int));

	this->xy_vertex = (int**) malloc(this->w * sizeof(int*));
	int cur_vert = 0;
	for ( int i = 0; i < w; i++ )
	{
		this->xy_vertex[i] = (int*) malloc(this->h * sizeof(int));

		for ( int j = 0; j < h; j++ )
		{
			this->xy_vertex[i][j] = cur_vert;
			this->vertex_x[cur_vert] = i;
			this->vertex_y[cur_vert] = j;
			cur_vert++;
		}
	}

	this->out_degrees = (int*) malloc(this->num_vertices * sizeof(int));
	bzero(this->out_degrees, this->num_vertices * sizeof(int));

	this->out_edges = (int**) malloc(this->num_vertices * sizeof(int*));
	for ( int i = 0; i < this->num_vertices; i++ )
	{
		this->out_edges[i] = (int*) malloc(this->num_vertices * sizeof(int*));
	}

	this->special_vertices = (int*) malloc(this->num_vertices * sizeof(int));
	this->special_types = (int*) malloc(this->num_vertices * sizeof(int));
	this->actor_positions = (int*) malloc(this->num_vertices * sizeof(int));
	this->actor_types = (int*) malloc(this->num_vertices * sizeof(int));
	this->size_actors = this->num_vertices;
	this->num_actors = 0;
	this->size_special = this->num_vertices;
	this->num_special = 0;
}

int GraphMap::getMapChar( int x, int y )
{
	return this->map_chars[x][y];
}

void GraphMap::addEdge( int x, int y, int a, int b )
{
	if ( this->toroidal )
	{
		a = (a + this->w) % this->w;
		b = (b + this->h) % this->h;
	}
	else
	{
		if ( a < 0 || a >= this->w || b < 0 || b >= this->h )
		{
			return;
		}
	}

	if ( this->map_chars[a][b] == '#' )
	{
		/* skip! */
		return;
	}

	int v = this->xy_vertex[x][y];
	int u = this->xy_vertex[a][b];

	this->out_edges[v][this->out_degrees[v]] = u;
	this->out_degrees[v] = this->out_degrees[v] + 1;
}

int GraphMap::addActor( int type, int x, int y )
{
	if ( this->num_actors >= this->size_actors )
	{
		this->size_actors = this->size_actors * 2;

		this->actor_types = (int*) realloc(this->actor_types, this->size_actors * sizeof(int));
		this->actor_positions = (int*) realloc(this->actor_positions, this->size_actors * sizeof(int));
	}

	this->actor_types[this->num_actors] = type;
	this->actor_positions[this->num_actors] = this->xy_vertex[x][y];

	this->num_actors = this->num_actors + 1;

	return this->num_actors - 1;
}

bool GraphMap::moveActor( int actor, int x, int y, bool force )
{
	int new_pos = -1;
	if ( x >= 0 && x < this->w && y >= 0 && y < this->h )
	{
		new_pos = this->xy_vertex[x][y];
	}

	if ( force )
	{
		this->actor_positions[actor] = new_pos;
		return true;
	}

	int old_pos = this->actor_positions[actor];

	bool found_neighbor = false;
	for ( int i = 0; !found_neighbor && i < this->out_degrees[old_pos]; i++ )
	{
		if ( new_pos == this->out_edges[old_pos][i] )
		{
			found_neighbor = true;
			this->actor_positions[actor] = new_pos;
		}
	}

	return found_neighbor;
}

int GraphMap::setActorType( int i, int type )
{
	this->actor_types[i] = type;

	return type;
}

/**
 * Load a graph from a file, using the game-board format.
 */
GraphMap::GraphMap( FILE* f )
{
// Load map from file!
	char geometry[100];
	int w, h;

	fscanf(f, "%s %d %d", geometry, &w, &h);

	if ( strcmp(geometry, "TORUS") == 0 )
	{
		this->toroidal = true;
	}
	else
	{
		this->toroidal = false;
	}

	this->initGraph(w, h);

	int* teleport_a = (int*) malloc(22 * sizeof(int));
	int* teleport_b = (int*) malloc(22 * sizeof(int));
	for ( int i = 0; i < 22; i++ )
	{
		teleport_a[i] = -1;
		teleport_b[i] = -1;
	}

	this->map_chars = (int**) malloc(w * sizeof(int*));
	for ( int x = 0; x < w; x++ )
	{
		this->map_chars[x] = (int*) malloc(h * sizeof(int));
	}

	for ( int y = 0; y < h; y++ )
	{
		for ( int x = 0; x < w; x++ )
		{
			char c = fgetc(f);

			while ( c == '\n' )
			{
				c = fgetc(f);
			}

			this->map_chars[x][y] = c;

			int teleport = -1;
			if ( '0' <= c && c <= '9' )
			{
				teleport = c - '0';
			}
			else if ( 'A' <= c && c <= 'F' )
			{
				teleport = (c - 'A') + 10;
			}

			if ( teleport >= 0 )
			{
				if ( teleport_a[teleport] < 0 )
				{
					teleport_a[teleport] = this->xy_vertex[x][y];
				}
				else
				{
					teleport_b[teleport] = this->xy_vertex[x][y];
				}
			}

			if ( c == 'h' || c == 'e' || c == 'P' )
			{
				// Get a team number... 0...9
				c = fgetc(f);

				if ( c >= '0' && c <= '9' )
				{
					// do something!
					int team = c - '0';

					// shift the char over for a team value!
					this->map_chars[x][y] = (this->map_chars[x][y] | (team << 8));
				}
				else
				{
					// put it back!
					ungetc(c, f);
				}
			}
		}
	}

	for ( int y = 0; y < h; y++ )
	{
		for ( int x = 0; x < w; x++ )
		{
			char c = this->map_chars[x][y];

			// only care about the _letter_
			switch ( c & 0xFF )
			{
				case '#':
					// do nothing!
					break;

				case '>':
					this->addEdge(x, y, x + 1, y);
					break;
				case '<':
					this->addEdge(x, y, x - 1, y);
					break;

				case 'v':
					this->addEdge(x, y, x, y + 1);
					break;

				case '^':
					this->addEdge(x, y, x, y - 1);
					break;

				case '*': // Eatable
				case 'h': // hero spawn point
				case 'e': // enemy spawn point
				case 'P': // powerup spawn point
				case ' ': // empty point
				default: // This includes teleporters! Others?
					// This is a position that can be occupied
					this->addEdge(x, y, x, y);
					this->addEdge(x, y, x - 1, y);
					this->addEdge(x, y, x + 1, y);
					this->addEdge(x, y, x, y - 1);
					this->addEdge(x, y, x, y + 1);
					break;
			}

			int teleport = -1;
			if ( '0' <= c && c <= '9' )
			{
				teleport = c - '0';
			}
			else if ( 'A' <= c && c <= 'F' )
			{
				teleport = (c - 'A') + 10;
			}

			if ( teleport >= 0 )
			{
				if ( teleport_a[teleport] == this->xy_vertex[x][y] )
				{
					int a = this->vertex_x[teleport_b[teleport]];
					int b = this->vertex_y[teleport_b[teleport]];
					this->addEdge(x, y, a, b);
				}
				else
				{
					int a = this->vertex_x[teleport_a[teleport]];
					int b = this->vertex_y[teleport_a[teleport]];
					this->addEdge(x, y, a, b);
				}
			}

			// Special Vertices!
			switch ( c )
			{
				case '*': // Eatable
					this->special_vertices[this->num_special] = this->xy_vertex[x][y];
					this->special_types[this->num_special] = SPECIAL_EATABLE_SPAWN;
					this->num_special = this->num_special + 1;
					break;
				case 'h': // hero spawn point
					this->special_vertices[this->num_special] = this->xy_vertex[x][y];
					this->special_types[this->num_special] = SPECIAL_HERO_SPAWN;
					this->num_special = this->num_special + 1;
					break;
				case 'e': // enemy spawn point
					this->special_vertices[this->num_special] = this->xy_vertex[x][y];
					this->special_types[this->num_special] = SPECIAL_ENEMY_SPAWN;
					this->num_special = this->num_special + 1;
					break;
				case 'P': // powerup spawn point
					this->special_vertices[this->num_special] = this->xy_vertex[x][y];
					this->special_types[this->num_special] = SPECIAL_POWERUP_SPAWN;
					this->num_special = this->num_special + 1;
					break;
			}
		}
	}

	free(teleport_a);
	teleport_a = 0;
	free(teleport_b);
	teleport_b = 0;
}

/**
 * Destructor
 */
GraphMap::~GraphMap()
{
	if ( this->vertex_x != 0 )
	{
		free(this->vertex_x);
		this->vertex_x = 0;
	}

	if ( this->vertex_y != 0 )
	{
		free(this->vertex_y);
		this->vertex_y = 0;
	}

	if ( this->xy_vertex != 0 )
	{
		for ( int i = 0; i < this->w; i++ )
		{
			free(this->xy_vertex[i]);
			this->xy_vertex[i] = 0;
		}
		free(this->xy_vertex);
		this->xy_vertex = 0;
	}

	if ( this->out_degrees != 0 )
	{
		free(this->out_degrees);
		this->out_degrees = 0;
	}
	if ( this->out_edges != 0 )
	{
		for ( int i = 0; i < this->num_vertices; i++ )
		{
			free(this->out_edges[i]);
			this->out_edges[i] = 0;
		}
		free(this->out_edges);
		this->out_edges = 0;
	}

	if ( this->special_types != 0 )
	{
		free(this->special_types);
		this->special_types = 0;
	}

	if ( this->special_vertices != 0 )
	{
		free(this->special_vertices);
		this->special_vertices = 0;
	}

	if ( this->actor_positions != 0 )
	{
		free(this->actor_positions);
		this->actor_positions = 0;
	}

	if ( this->actor_types != 0 )
	{
		free(this->actor_types);
		this->actor_types = 0;
	}

	if ( this->map_chars != 0 )
	{
		for ( int x = 0; x < w; x++ )
		{
			free(map_chars[x]);
		}
		free(map_chars);
	}

	this->num_vertices = 0;
	this->w = 0;
	this->h = 0;
	this->size_special = 0;
	this->num_special = 0;
	this->size_actors = 0;
	this->num_actors = 0;
}

/**
 * Returns the number of vertices in the graph.
 */
int GraphMap::getNumVertices()
{
	return this->num_vertices;
}

/**
 * Returns the number of out-neighbors for the position (x,y)
 */
int GraphMap::getNumNeighbors( int x, int y )
{
	return this->out_degrees[this->xy_vertex[x][y]];
}

/**
 * Assigns the value of the ith neighbor of (x,y) to (a,b).
 */
void GraphMap::getNeighbor( int x, int y, int i, int& a, int& b )
{
	if ( x < 0 || x >= this->w || y < 0 || y >= this->h )
	{
		a = -1;
		b = -1;
		return;
	}

	int v = this->xy_vertex[x][y];

	if ( i < 0 || i >= this->out_degrees[v] )
	{
		a = -1;
		b = -1;
		return;
	}

	a = this->vertex_x[this->out_edges[v][i]];
	b = this->vertex_y[this->out_edges[v][i]];
}

int GraphMap::getNumActors()
{
	return this->num_actors;
}

int GraphMap::getActorType( int i )
{
	return this->actor_types[i];
}

void GraphMap::getActorPosition( int i, int& x, int& y )
{
	int p = this->actor_positions[i];

	if ( p < 0 )
	{
		x = -1;
		y = -1;
		return;
	}

	x = this->vertex_x[p];
	y = this->vertex_y[p];
}

/**
 * Special positions, used by controller.
 */
int GraphMap::getNumSpecial()
{
	return this->num_special;
}

int GraphMap::getSpecialType( int i )
{
	return this->special_types[i];
}

void GraphMap::print()
{
	for ( int i = 0; i < this->num_vertices; i++ )
	{

		printf("%d (%d, %d) %d : ", i, this->vertex_x[i], this->vertex_y[i], this->out_degrees[i]);

		for ( int j = 0; j < this->out_degrees[i]; j++ )
		{
			printf("%d ", this->out_edges[i][j]);
		}
		printf("\n");
	}
	fflush(stdout);
}

int GraphMap::getVertex( int x, int y )
{
	if ( this->toroidal )
	{
		x = (x + this->w) % this->w;
		y = (y + this->h) % this->h;
	}
	else
	{
		if ( x < 0 || x >= this->w || y < 0 || y >= this->h )
		{
			return -1;
		}
	}

	return this->xy_vertex[x][y];
}

void GraphMap::getPosition( int v, int& x, int& y )
{
	if ( v < 0 || v >= this->num_vertices )
	{
		x = -1;
		y = -1;
	}

	x = this->vertex_x[v];
	y = this->vertex_y[v];
}

int GraphMap::getDelayHero()
{
	return this->delay_hero;
}

int GraphMap::getDelayEnemy()
{
	return this->delay_enemy;
}

int GraphMap::getDelayEatable()
{
	return this->delay_eatable;
}

int GraphMap::getDelayPowerup()
{
	return this->delay_powerup;
}

GraphMap::GraphMap( const GraphMap& map )
{
	this->num_vertices = map.num_vertices;
	this->w = map.w;
	this->h = map.h;

	this->vertex_x = (int*) malloc(this->num_vertices * sizeof(int));
	this->vertex_y = (int*) malloc(this->num_vertices * sizeof(int));
	this->out_degrees = (int*) malloc(this->num_vertices * sizeof(int));
	this->out_edges = (int**) malloc(this->num_vertices * sizeof(int*));

	for ( int i = 0; i < this->num_vertices; i++ )
	{
		this->vertex_x[i] = map.vertex_x[i];
		this->vertex_y[i] = map.vertex_y[i];

		this->out_degrees[i] = map.out_degrees[i];

		this->out_edges[i] = (int*) malloc(map.num_vertices * sizeof(int));

		for ( int j = 0; j < this->out_degrees[i]; j++ )
		{
			this->out_edges[i][j] = map.out_edges[i][j];
		}
	}

	this->xy_vertex = (int**) malloc(this->w * sizeof(int*));
	this->map_chars = (int**) malloc(this->w * sizeof(char*));

	for ( int x = 0; x < this->w; x++ )
	{
		this->xy_vertex[x] = (int*) malloc(this->h * sizeof(int));
		this->map_chars[x] = (int*) malloc(this->h * sizeof(int));

		for ( int y = 0; y < this->h; y++ )
		{
			this->xy_vertex[x][y] = map.xy_vertex[x][y];
			this->map_chars[x][y] = map.map_chars[x][y];
		}
	}

	this->toroidal = map.toroidal;

	// specials don't matter
	this->size_special = 0;
	this->num_special = 0;
	this->special_vertices = 0;
	this->special_types = 0;

	this->size_actors = map.size_actors;
	this->num_actors = map.num_actors;
	this->actor_positions = (int*) malloc(map.size_actors * sizeof(int));
	this->actor_types = (int*) malloc(map.size_actors * sizeof(int));

	for ( int i = 0; i < map.num_actors; i++ )
	{
		this->actor_positions[i] = map.actor_positions[i];
		this->actor_types[i] = map.actor_types[i];
	}

	this->delay_enemy = map.delay_enemy;
	this->delay_eatable = map.delay_eatable;
	this->delay_hero = map.delay_hero;
	this->delay_powerup = map.delay_powerup;
}
