/*
 * GameManager.cpp
 *
 *  Created on: Mar 7, 2014
 *      Author: stolee
 */

#include <ncurses.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include "GameManager.hpp"
#include "Actor.hpp"
#include "OtherActors.hpp"
#include <math.h>
#include <time.h>
#include <queue>

const int INFO_COLOR = 1;
const int MSGS_COLOR = 2;
const int PLAY_COLOR = 3;
const int MAZE_COLOR = 4;
const int DOTS_COLOR = 5;

const int EDIBLE_CHASER = 6;
const int EYES_CHASER = 7;
const int CHASER_ONE = 8;
const int CHASER_TWO = 9;
const int CHASER_THREE = 10;
const int CHASER_FOUR = 11;
const int HERO_COLOR = 12;

namespace dstolee
{
void updateDistanceArray( GraphMap* map, int from_type, int* d )
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

void updateDistanceArray( GraphMap* map, int from_x, int from_y, int* d, int* avoid_d, int* pred = 0 )
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

/**
 * A class that is helpful for things
 */
class Avoider: public Actor
{
	private:
		int avoid_type;
		int* avoid_array;
		int* distance_array;
		int* predecessor_array;

	public:
		Avoider( int type, int avoid_type ) :
				Actor(type)
		{
			this->avoid_type = avoid_type;
			this->avoid_array = 0;
			this->distance_array = 0;
			this->predecessor_array = 0;
		}

		virtual ~Avoider()
		{
			if ( this->distance_array != 0 )
			{
				free(this->avoid_array); // = (int*) malloc(map->getNumVertices() * sizeof(int));
				free(this->distance_array); // = (int*) malloc(map->getNumVertices() * sizeof(int));
				free(this->predecessor_array); // = (int*) malloc(map->getNumVertices() * sizeof(int));
			}
		}

		virtual int selectNeighbor( GraphMap* map, int cur_x, int cur_y )
		{
			if ( this->distance_array == 0 )
			{
				this->avoid_array = (int*) malloc(map->getNumVertices() * sizeof(int));
				this->distance_array = (int*) malloc(map->getNumVertices() * sizeof(int));
				this->predecessor_array = (int*) malloc(map->getNumVertices() * sizeof(int));
			}

			updateDistanceArray(map, this->avoid_type, this->avoid_array);

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

			updateDistanceArray(map, cur_x, cur_y, this->distance_array, this->avoid_array, this->predecessor_array);

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
				int me = map->getVertex(cur_x, cur_y);
				int v = best_v;
				while ( this->predecessor_array[v] != me )
				{
					v = this->predecessor_array[v];
				}

				for ( int i = 0; i < map->getNumNeighbors(cur_x, cur_y); i++ )
				{
					int a, b, u;
					map->getNeighbor(cur_x, cur_y, i, a, b);
					u = map->getVertex(a, b);

					if ( u == v )
					{
						return i;
					}
				}
			}

			return 0;
		}

		virtual const char* getActorId()
		{
			return "avoider";
		}

		virtual Actor* duplicate()
		{
			return new Avoider(this->getType(), this->avoid_type);
		}
};

class HeroAvoider: public Avoider
{
	public:
		HeroAvoider( int type ) :
				Avoider(type, ACTOR_HERO)
		{
		}

		virtual ~HeroAvoider()
		{

		}

		virtual const char* getActorId()
		{
			return "heroavoider";
		}
};

class EnemyAvoider: public Avoider
{
	public:
		EnemyAvoider( int type ) :
				Avoider(type, ACTOR_ENEMY)
		{
		}

		virtual ~EnemyAvoider()
		{

		}

		virtual const char* getActorId()
		{
			return "enemyavoider";
		}
};

class Pursuer: public Actor
{
	private:
		int pursue_type;
		int* prev_verts;
		int num_prev_verts;
		int size_prev_verts;

	protected:
		double laziness;

	public:

		virtual const char* getActorId()
		{
			return "pursuer";
		}

		virtual Actor* duplicate()
		{
			return new Pursuer(this->getType(), this->pursue_type, this->laziness);
		}

		Pursuer( int type, int pursue_type, double laziness ) :
				Actor(type)
		{
			this->pursue_type = pursue_type;
			this->laziness = laziness;

			this->num_prev_verts = 0;
			this->size_prev_verts = 100;
			this->prev_verts = (int*) malloc(this->size_prev_verts * sizeof(int));
		}

		virtual ~Pursuer()
		{
			free(this->prev_verts);
		}

		virtual int selectNeighbor( GraphMap* map, int x, int y )
		{
			if ( this->num_prev_verts < this->size_prev_verts )
			{
				this->prev_verts[this->num_prev_verts] = map->getVertex(x, y);
				(this->num_prev_verts)++;
			}
			else
			{
				for ( int i = 0; i < this->size_prev_verts - 1; i++ )
				{
					this->prev_verts[i] = this->prev_verts[i + 1];
				}
				this->prev_verts[this->size_prev_verts - 1] = map->getVertex(x, y);
			}

			if ( map->getNumNeighbors(x, y) <= 1 )
			{
				return 0;
			}

			if ( (double(random() % 100) / 100.0) < this->laziness )
			{
				return (random() % (map->getNumNeighbors(x, y)));
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

			bool goal_is_dup = false;
			for ( int i = 0; i < this->num_prev_verts; i++ )
			{
				if ( this->prev_verts[i] == goal_vert )
				{
					goal_is_dup = true;
				}
			}

			if ( goal_is_dup )
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
};

/**
 * A class that is helpful for things
 */
class EnemyPursuer: public Pursuer
{

	public:
		EnemyPursuer( int type ) :
				Pursuer(type, ACTOR_ENEMY, 0)
		{

		}

		virtual ~EnemyPursuer()
		{

		}

		virtual const char* getActorId()
		{
			return "enemypursuer";
		}
};

class PowerupPursuer: public Pursuer
{

	public:
		PowerupPursuer( int type ) :
				Pursuer(type, ACTOR_POWERUP, 0)
		{

		}

		virtual ~PowerupPursuer()
		{

		}

		virtual const char* getActorId()
		{
			return "poweruppursuer";
		}
};

class EatablePursuer: public Pursuer
{

	public:
		EatablePursuer( int type ) :
				Pursuer(type, ACTOR_EATABLE, 0)
		{

		}

		virtual ~EatablePursuer()
		{

		}

		virtual const char* getActorId()
		{
			return "eatablepursuer";
		}
};

class HeroPursuer: public Pursuer
{

	public:
		HeroPursuer( int type ) :
				Pursuer(type, ACTOR_HERO, 0)
		{

		}

		virtual ~HeroPursuer()
		{

		}

		virtual const char* getActorId()
		{
			return "heropursuer";
		}
};

class LazyHeroPursuer: public Pursuer
{

	public:
		LazyHeroPursuer( int type ) :
				Pursuer(type, ACTOR_HERO, 0.25)
		{
		}

		virtual ~LazyHeroPursuer()
		{

		}

		virtual const char* getActorId()
		{
			return "lazypursuer";
		}
};

class PursueAvoider: public Actor
{
	private:
		int pursue_type;
		int avoid_type;

	public:

		virtual const char* getActorId()
		{
			return "pursueavoider";
		}

		virtual Actor* duplicate()
		{
			return new PursueAvoider(this->getType(), this->pursue_type, this->avoid_type);
		}

		PursueAvoider( int type, int pursue_type, int avoid_type ) :
				Actor(type)
		{
			this->pursue_type = pursue_type;
			this->avoid_type = avoid_type;
		}

		virtual ~PursueAvoider()
		{

		}

		virtual int selectNeighbor( GraphMap* map, int x, int y )
		{
			if ( map->getNumNeighbors(x, y) <= 1 )
			{
				return 0;
			}

			int n = map->getNumVertices();
			int* avoid_dist = (int*) malloc(n * sizeof(int));

			for ( int i = 0; i < n; i++ )
			{
				avoid_dist[i] = n * n;
			}
			std::queue<int> q;

			for ( int i = 0; i < map->getNumActors(); i++ )
			{
				if ( (map->getActorType(i) & (this->avoid_type | ACTOR_DEAD)) == this->avoid_type )
				{
					int a, b, v;
					map->getActorPosition(i, a, b);
					v = map->getVertex(a, b);

					avoid_dist[v] = 0;
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

					if ( avoid_dist[u] > n )
					{
						// Mark as visited, mark reverse edge, and push!
						avoid_dist[u] = avoid_dist[v] + 1;
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

			for ( int i = 0; i < n; i++ )
			{
				dists[i] = n * n;
				revverts[i] = -1;
				visited[i] = false;
			}

			int goal_target = this->pursue_type;

			q.push(cur_vert);
			revverts[cur_vert] = cur_vert;
			visited[cur_vert] = true;
			dists[cur_vert] = 0;

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

					if ( visited[u] == false && dists[v] + 2 < avoid_dist[u] )
					{
						// Mark as visited, mark reverse edge, and push!
						visited[u] = true;
						revverts[u] = v;
						dists[u] = dists[v] + 1;
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
};

class EnemyHeroPA: public PursueAvoider
{

	public:
		EnemyHeroPA( int type ) :
				PursueAvoider(type, ACTOR_ENEMY, ACTOR_HERO)
		{

		}

		virtual ~EnemyHeroPA()
		{

		}

		virtual const char* getActorId()
		{
			return "enemyheropa";
		}
};

class EatableEnemyPA: public PursueAvoider
{

	public:
		EatableEnemyPA( int type ) :
				PursueAvoider(type, ACTOR_EATABLE, ACTOR_ENEMY)
		{

		}

		virtual ~EatableEnemyPA()
		{

		}

		virtual const char* getActorId()
		{
			return "eatableenemypa";
		}
};

class PowerupEnemyPA: public PursueAvoider
{

	public:
		PowerupEnemyPA( int type ) :
				PursueAvoider(type, ACTOR_POWERUP, ACTOR_ENEMY)
		{

		}

		virtual ~PowerupEnemyPA()
		{

		}

		virtual const char* getActorId()
		{
			return "powerupenemypa";
		}
};

}

/**
 * A helper method...
 */
void GameManager::drawPos( GraphMap* map, WINDOW* w, int x, int y )
{
	int c = map->getMapChar(x, y);
	int color = 0;

	// TODO: modify output letter based on geography
	switch ( c & 0xFF )
	{
		case '#':
			color = MAZE_COLOR;
			break;

		case '>':
		case '<':
		case 'v':
		case '^':
			color = CHASER_TWO;
			break;

		case '*':
		case 'P':
		case 'h':
		case 'e':
			c = ' ';
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			color = CHASER_FOUR;
			c = ACS_S9;
			break;

		default:
			color = INFO_COLOR;
	}

	char oldc = c;
	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = this->getActor(i)->getType();

		if ( a == x && b == y && (t & ACTOR_EATABLE) )
		{
			char d = this->getActor(i)->getImage();

			if ( t & ACTOR_POWERUP )
			{
				// this color would already be set
				color = MSGS_COLOR;
				c = d;
			}
			else if ( c == oldc )
			{
				// this color would already be set
				color = DOTS_COLOR;
				c = d;
			}
		}
	}

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = this->getActor(i)->getType();

		if ( a == x && b == y && (t & ACTOR_HERO) )
		{
			char d = this->getActor(i)->getImage();

			c = d;
			color = HERO_COLOR;
		}
	}

	for ( int i = 0; i < map->getNumActors(); i++ )
	{
		int a, b;
		map->getActorPosition(i, a, b);
		int t = this->getActor(i)->getType();

		if ( a == x && b == y && (t & ACTOR_ENEMY) )
		{
			char d = this->getActor(i)->getImage();

			if ( t & ACTOR_EATABLE )
			{
				color = CHASER_TWO;
			}
			else
			{
				color = CHASER_ONE;
			}

			c = d;
		}
	}

	waddch(w, c | A_BOLD | COLOR_PAIR(color));
}

/**
 * Render the map, using the current set of actors
 * and positions
 */
void GameManager::render( WINDOW* w, GraphMap* map )
{
	for ( int y = 0; y < map->h; y++ )
	{
		wmove(w, y, 0);
		for ( int x = 0; x < map->w; x++ )
		{
			this->drawPos(map, w, x, y);
		}
	}
	refresh();
}

/**
 * Construct a game manager using command-line arguments.
 */
GameManager::GameManager( int argc, char** argv, Actor**& actors, int& num_actors )
{
	actors = (Actor**) realloc(actors, (num_actors + 100) * sizeof(Actor*));
	actors[num_actors++] = new KeyboardActor(ACTOR_HERO);
	actors[num_actors++] = new dstolee::Pursuer(ACTOR_ENEMY, ACTOR_HERO, 0);
	actors[num_actors++] = new RandomActor(ACTOR_EATABLE | ACTOR_POWERUP);
	actors[num_actors++] = new Actor(ACTOR_EATABLE);
	actors[num_actors++] = new dstolee::HeroPursuer(ACTOR_ENEMY);
	actors[num_actors++] = new dstolee::PowerupPursuer(ACTOR_HERO);
	actors[num_actors++] = new dstolee::EatablePursuer(ACTOR_HERO);
	actors[num_actors++] = new dstolee::EnemyPursuer(ACTOR_POWERUP | ACTOR_EATABLE);
	actors[num_actors++] = new dstolee::HeroAvoider(ACTOR_POWERUP | ACTOR_EATABLE);
	actors[num_actors++] = new dstolee::EnemyAvoider(ACTOR_HERO);
	actors[num_actors++] = new dstolee::EnemyHeroPA(ACTOR_POWERUP | ACTOR_EATABLE);
	actors[num_actors++] = new dstolee::EatableEnemyPA(ACTOR_HERO);
	actors[num_actors++] = new dstolee::PowerupEnemyPA(ACTOR_HERO);
	actors[num_actors++] = new dstolee::LazyHeroPursuer(ACTOR_ENEMY);

	srand(time(NULL));
	this->test_name = 0;
	this->message_buffer = 0;
	this->goal_mode = GOAL_MODE_HEROWINS;
	this->run_points = 2;
	this->finish_points = 8;

	this->actor_time = 0;
	this->actor_clock = 0;
	this->start_time = time(NULL);
	this->max_time = time(NULL) + 120;

	FILE* f = fopen(argv[1], "r");
	this->graph = new GraphMap(f);
	fclose(f);

	this->enemies_eatable_until = -1;
	this->enemy_eatable_window = 0;

	this->hero = 0;
	this->enemy = 0;
	this->eatable = 0;
	this->powerup = 0;

	this->num_heroes = 0;
	this->num_eatable = 0;
	this->num_powerups = 0;
	this->num_enemies = 0;
	this->num_actors = 0;
	this->start_num_eatables = 0;

	this->delay_hero = 1;
	this->delay_enemy = 1;
	this->delay_eatable = 1;
	this->enemies_enabled = true;
	this->delay_powerup = 1;

	this->size_actors = 400;
	this->actors = (Actor**) malloc(this->size_actors * sizeof(Actor*));

	this->num_repeats = 1;
	this->render_all = true;
	this->delay_in_ms = 50000;
	this->max_num_moves = 1000;
	this->cur_move = 0;
	/* See http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/keys.html for more info */

	for ( int i = 0; i < argc; i++ )
	{
		if ( strcmp(argv[i], "--disable-enemies") == 0 )
		{
			this->enemies_enabled = false;
		}
		if ( strcmp(argv[i], "--render-off") == 0 )
		{
			this->render_all = false;
		}
		else if ( strcmp(argv[i], "--delay") == 0 && i < argc - 1 )
		{
			this->delay_in_ms = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--moves") == 0 && i < argc - 1 )
		{
			this->max_num_moves = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--delay-hero") == 0 && i < argc - 1 )
		{
			this->delay_hero = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--delay-enemy") == 0 && i < argc - 1 )
		{
			this->delay_enemy = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--delay-eatable") == 0 && i < argc - 1 )
		{
			this->delay_eatable = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--delay-powerup") == 0 && i < argc - 1 )
		{
			this->delay_powerup = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--window") == 0 && i < argc - 1 )
		{
			this->enemy_eatable_window = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--time") == 0 && i < argc - 1 )
		{
			this->max_time = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--test") == 0 && i < argc - 1 )
		{
			this->test_name = (char*) malloc(strlen(argv[i + 1]) + 1);
			strcpy(this->test_name, argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--runpts") == 0 && i < argc - 1 )
		{
			this->run_points = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--goalpts") == 0 && i < argc - 1 )
		{
			this->finish_points = atoi(argv[i + 1]);
		}
		else if ( strcmp(argv[i], "--goal") == 0 && i < argc - 1 )
		{
			if ( strcmp(argv[i + 1], "hero") == 0 )
			{
				this->goal_mode = GOAL_MODE_HEROWINS;
			}
			if ( strcmp(argv[i + 1], "enemy") == 0 )
			{
				this->goal_mode = GOAL_MODE_ENEMYWINS;
			}
			if ( strcmp(argv[i + 1], "powerup") == 0 )
			{
				this->goal_mode = GOAL_MODE_POWERUP_SURVIVES;
			}
		}
		else if ( strcmp(argv[i], "--hero") == 0 && i < argc - 1 )
		{
			for ( int j = 0; j < num_actors; j++ )
			{
				if ( strcmp(argv[i + 1], actors[j]->getActorId()) == 0 )
				{
					this->hero = actors[j]->duplicate();
					this->hero->setType(ACTOR_HERO);
				}
			}
		}
		else if ( strcmp(argv[i], "--enemy") == 0 && i < argc - 1 )
		{
			for ( int j = 0; j < num_actors; j++ )
			{
				if ( strcmp(argv[i + 1], actors[j]->getActorId()) == 0 )
				{
					this->enemy = actors[j]->duplicate();
					this->enemy->setType(ACTOR_ENEMY);
				}
			}
		}
		else if ( strcmp(argv[i], "--eatable") == 0 && i < argc - 1 )
		{
			for ( int j = 0; j < num_actors; j++ )
			{
				if ( strcmp(argv[i + 1], actors[j]->getActorId()) == 0 )
				{
					this->eatable = actors[j]->duplicate();
					this->eatable->setType(ACTOR_EATABLE);
				}
			}
		}
		else if ( strcmp(argv[i], "--powerup") == 0 && i < argc - 1 )
		{
			for ( int j = 0; j < num_actors; j++ )
			{
				if ( strcmp(argv[i + 1], actors[j]->getActorId()) == 0 )
				{
					this->powerup = actors[j]->duplicate();
					this->powerup->setType(ACTOR_EATABLE | ACTOR_POWERUP);
				}
			}
		}
		else if ( strcmp(argv[i], "--bestof") == 0 && i < argc - 1 )
		{
			this->num_repeats = atoi(argv[i + 1]);
		}
	}

	if ( this->hero == 0 )
	{
		this->hero = new KeyboardActor(ACTOR_HERO);
	}
	if ( this->enemy == 0 )
	{
		this->enemy = new dstolee::Pursuer(ACTOR_ENEMY, ACTOR_HERO, 0);
	}
	if ( this->eatable == 0 )
	{
		this->eatable = new Actor(ACTOR_EATABLE);
	}
	if ( this->powerup == 0 )
	{
		this->powerup = new RandomActor(ACTOR_EATABLE | ACTOR_POWERUP);
	}

	for ( int x = 0; x < this->graph->w; x++ )
	{
		for ( int y = 0; y < this->graph->h; y++ )
		{
			Actor* a;
			switch ( this->graph->map_chars[x][y] & 0xFF )
			{
				case '*':
					a = eatable->duplicate();
					a->setType(ACTOR_EATABLE);
					this->addActor(a);
					this->graph->addActor(ACTOR_EATABLE, x, y);
					(this->num_eatable)++;
					break;

				case 'P':
					a = powerup->duplicate();
					a->setType(ACTOR_EATABLE | ACTOR_POWERUP);
					this->addActor(a);
					this->graph->addActor(ACTOR_EATABLE | ACTOR_POWERUP, x, y);
					(this->num_eatable)++;
					(this->num_powerups)++;
					break;

				case 'e':
					if ( this->enemies_enabled )
					{
						a = enemy->duplicate();
						a->setType(ACTOR_ENEMY);
						this->addActor(a);
						this->graph->addActor(ACTOR_ENEMY, x, y);
						(this->num_enemies)++;
					}
					break;

				case 'h':
					a = hero->duplicate();
					a->setType(ACTOR_HERO);
					this->addActor(a);
					this->graph->addActor(ACTOR_HERO, x, y);
					(this->num_heroes)++;
					break;
			}
		}
	}

	this->start_num_heroes = this->num_heroes;
	this->start_num_eatables = this->num_eatable - this->num_powerups;
	this->start_num_powerups = this->num_powerups;

	this->graph->delay_eatable = this->delay_eatable;
	this->graph->delay_enemy = this->delay_enemy;
	this->graph->delay_hero = this->delay_hero;
	this->graph->delay_powerup = this->delay_powerup;

	if ( this->render_all )
	{
		initscr(); /* Start curses mode 		  */
		start_color();

		init_pair(MAZE_COLOR, COLOR_BLUE, COLOR_BLACK);
		init_pair(DOTS_COLOR, COLOR_WHITE, COLOR_BLACK);
		init_pair(PLAY_COLOR, COLOR_YELLOW, COLOR_BLACK);
		init_pair(MSGS_COLOR, COLOR_RED, COLOR_BLACK);

		init_pair(EDIBLE_CHASER, COLOR_BLUE, COLOR_BLACK);
		init_pair(EYES_CHASER, COLOR_WHITE, COLOR_BLACK);
		init_pair(CHASER_ONE, COLOR_RED, COLOR_BLACK);
		init_pair(CHASER_TWO, COLOR_GREEN, COLOR_BLACK);
		init_pair(CHASER_THREE, COLOR_CYAN, COLOR_BLACK);
		init_pair(CHASER_FOUR, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(HERO_COLOR, COLOR_YELLOW, COLOR_BLACK);

		noecho();
		cbreak();
		keypad(stdscr, 1);
		clear();
	}

	this->start_num_eatables = this->num_eatable;
}

GameManager::~GameManager()
{
	delete this->graph;
	delete this->hero;
	delete this->enemy;
	delete this->powerup;
	delete this->eatable;

	for ( int i = 0; i < this->num_actors; i++ )
	{
		delete this->actors[i];
	}
	free(this->actors);

	if ( this->render_all )
	{
		endwin(); /* End curses mode		  */
	}

	if ( this->test_name != 0 )
	{
		free(this->test_name);
		this->test_name = 0;
	}

}

/**
 * This makes a shallow copy of an actor in the list.
 * However, it takes ownership and will delete the actor when through.
 *
 * This allows you to use a line such as: manager->addActor(new Actor());
 */
void GameManager::addActor( Actor* actor )
{
	if ( this->num_actors >= this->size_actors )
	{
		(this->size_actors) *= 2;
		this->actors = (Actor**) realloc(this->actors, this->size_actors * sizeof(Actor*));
	}

	this->actors[this->num_actors] = actor;
	(this->num_actors)++;
}

int GameManager::getNumActors()
{
	return this->num_actors;
}

Actor* GameManager::getActor( int i )
{
	return this->actors[i];
}

/**
 * Call this to start the game!
 */
void GameManager::play()
{
	this->best_score = 0;
	for ( int round = 0; round < this->num_repeats; round++ )
	{
		GraphMap* map = new GraphMap(*(this->graph));

		this->num_heroes = this->start_num_heroes;
		this->num_eatable = this->start_num_eatables;
		this->num_powerups = this->start_num_powerups;

		// do a loop
		this->start_clock = clock();
		this->cur_move = 0;
		bool running = true;

		for ( int i = 0; i < this->num_actors; i++ )
		{
			this->actors[i]->setType(this->actors[i]->getType() & (~ACTOR_DEAD));
		}

		int cur_turn_hero = 0;
		int cur_turn_enemy = 0;
		int cur_turn_eatable = 0;
		int cur_turn_powerup = 0;

		int N = map->getNumActors();

		int* last_pos = (int*) malloc(N * sizeof(int));
		int* next_pos = (int*) malloc(N * sizeof(int));

		for ( int i = 0; i < N; i++ )
		{
			int x, y, v;
			map->getActorPosition(i, x, y);
			v = map->getVertex(x, y);

			last_pos[i] = v;
			next_pos[i] = v;
		}

		while ( running && this->cur_move < this->max_num_moves && time(NULL) - this->start_time < this->max_time )
		{
			cur_turn_hero++;
			cur_turn_enemy++;
			cur_turn_eatable++;
			cur_turn_powerup++;

			(this->cur_move) = this->cur_move + 1;

			if ( this->cur_move == this->enemies_eatable_until )
			{
				for ( int i = 0; i < map->getNumActors(); i++ )
				{
					if ( this->actors[i]->getType() & ACTOR_ENEMY )
					{
						this->actors[i]->setType(this->actors[i]->getType() & (~ACTOR_EATABLE));
						map->setActorType(i, this->actors[i]->getType());
					}
				}
			}

			if ( this->render_all )
			{
				this->render(stdscr, map);
			}

			for ( int i = 0; i < map->getNumActors(); i++ )
			{
				int x, y;
				map->getActorPosition(i, x, y);
				int t = this->getActor(i)->getType();
				last_pos[i] = map->getVertex(x, y);
				next_pos[i] = last_pos[i];
			}

			// ask the actors what to do!
			for ( int i = 0; running && i < map->getNumActors(); i++ )
			{
				int x, y;

				map->getActorPosition(i, x, y);

				int t = this->getActor(i)->getType();

				if ( x < 0 || y < 0 || t & ACTOR_DEAD )
				{
					// this actor has been removed!
					last_pos[i] = -1;
					next_pos[i] = -1;
					continue;
				}

				bool is_hero = (t & ACTOR_HERO);
				bool is_enemy = (t & ACTOR_ENEMY);
				bool is_powerup = (t & ACTOR_POWERUP);
				bool is_eatable = (t & ACTOR_EATABLE);
				bool is_turn = false;

				if ( is_hero )
				{
					is_turn = (cur_turn_hero % this->delay_hero) == 0;
				}
				else if ( is_enemy )
				{
					is_turn = (cur_turn_enemy % this->delay_enemy) == 0;
				}
				else if ( is_powerup )
				{
					is_turn = (cur_turn_powerup % this->delay_powerup) == 0;
				}
				else if ( is_eatable )
				{
					is_turn = (cur_turn_eatable % this->delay_eatable) == 0;
				}

				if ( !is_turn )
				{
					continue;
				}

				// select neighbor time!
				clock_t start_clock = clock();
				int n = this->getActor(i)->selectNeighbor(map, x, y);
				clock_t end_clock = clock();

				(this->actor_clock) += (end_clock - start_clock);

				int a, b;

				map->getNeighbor(x, y, n, a, b);

				(next_pos[i] = map->getVertex(a, b));
			}

			// Determine collisions!
			for ( int i = 0; running && i < N; i++ )
			{
				if ( (map->getActorType(i) & (ACTOR_HERO | ACTOR_DEAD)) == ACTOR_HERO )
				{
					bool alive = true;
					for ( int j = 0; alive && j < N; j++ )
					{
						// look for a non-hero

						if ( (map->getActorType(j) & (ACTOR_ENEMY | ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_ENEMY )
						{
							// this is an enemy.. check for a crossing
							if ( next_pos[i] == next_pos[j]
							        || (last_pos[i] == next_pos[j] && last_pos[j] == next_pos[i]) )
							{
								// crossed!
								map->setActorType(i, map->getActorType(i) | ACTOR_DEAD);
								map->moveActor(i, -1, -1, true);
								alive = false;

								next_pos[i] = -1;

								// this count is only for non-enemies
								(this->num_heroes) = this->num_heroes - 1;

								if ( this->num_heroes <= 0 )
								{
									if ( this->render_all )
									{
										this->message_buffer = (char*) malloc(2000);
										sprintf(this->message_buffer, "All heroes have been eaten!");

										this->writeEndMessageAndWait(round, map);
										// check mode and exit
										running = false;
									}
									else
									{
										fflush(stdout);
										fflush(stderr);
										fprintf(stderr, "GAME OVER.  All heroes have been eaten!  It took %d moves.\n",
										        this->cur_move);
										fflush(stdout);
										fflush(stderr);

										this->computeRoundScore(round, map);
									}

									running = false;
								}
							}
						}
					}

					if ( running && alive )
					{
						for ( int j = 0; j < N; j++ )
						{
							// look for a non-hero
							if ( (map->getActorType(j) & (ACTOR_EATABLE | ACTOR_DEAD)) == ACTOR_EATABLE )
							{
								// this is an eatable.. check for a crossing
								if ( next_pos[i] == next_pos[j]
								        || (last_pos[i] == next_pos[j] && last_pos[j] == next_pos[i]) )
								{
									// crossed!
									map->setActorType(j, map->getActorType(j) | ACTOR_DEAD);
									map->moveActor(j, -1, -1, true);

									if ( (map->getActorType(j) & ACTOR_ENEMY) == 0 )
									{
										// this count is only for non-enemies
										(this->num_eatable) = this->num_eatable - 1;

										next_pos[j] = -1;
										if ( map->getActorType(j) & ACTOR_POWERUP )
										{
											if ( this->enemy_eatable_window > 0 )
											{
												this->enemies_eatable_until = this->cur_move
												        + this->enemy_eatable_window + 1;
												for ( int i = 0; i < map->getNumActors(); i++ )
												{
													if ( this->actors[i]->getType() & ACTOR_ENEMY )
													{
														this->actors[i]->setType(
														        this->actors[i]->getType() | (ACTOR_EATABLE));
														map->setActorType(i, this->actors[i]->getType());
													}
												}
											}
											(this->num_powerups)--;
										}
									}

									if ( this->num_eatable <= 0 )
									{
										if ( this->render_all )
										{
											this->message_buffer = (char*) malloc(2000);
											sprintf(this->message_buffer, "All eatables have been eaten!");

											this->writeEndMessageAndWait(round, map);
										}
										else
										{
											fflush(stdout);
											fflush(stderr);
											fprintf(stderr, "All eatables have been eaten! It took %d moves.\n",
											        this->cur_move);
											fflush(stdout);
											fflush(stderr);
											this->computeRoundScore(round, map);
										}

										running = false;
									}
								}
							}
						}
					}
				}
			} // end check collisions

			for ( int i = 0; running && i < map->getNumActors(); i++ )
			{
				if ( next_pos[i] < 0 )
				{
					continue;
				}

				int a, b;
				map->getPosition(next_pos[i], a, b);

				bool result = map->moveActor(i, a, b, true);

//				if ( !result )
//				{
//					this->message_buffer = (char*) malloc(2000);
//					sprintf(this->message_buffer, "Invalid move from vertex %d to vertex %d (Actor %d of type %d)!!!\n",
//					        last_pos[i], next_pos[i], i, map->getActorType(i));
////					map->print();
//					this->writeEndMessageAndWait(round, map);
//					running = false;
//				}
			}

			if ( this->render_all && this->delay_in_ms > 0 )
			{
				timeval to;
				to.tv_sec = 0;
				to.tv_usec = this->delay_in_ms;
				select(0, 0, 0, 0, &to);
			}
		}

		if ( running )
		{
			if ( this->render_all )
			{
				this->message_buffer = (char*) malloc(2000);
				sprintf(this->message_buffer, "Out of moves/time!");

				this->writeEndMessageAndWait(round, map);
			}
			else
			{
				fflush(stdout);
				fflush(stderr);
				fprintf(stderr, "Run out of moves/time.\n");
				fflush(stdout);
				fflush(stderr);
				this->computeRoundScore(round, map);
			}
		}

		free(last_pos);
		last_pos = 0;
		free(next_pos);
		next_pos = 0;

		delete map;
	}

	char buffer[1000];
	int cur_h = 0;
	wclear(stdscr);
	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		sprintf(buffer, "Maximum Points for this test: %d / %d", this->best_score,
		        this->run_points + this->finish_points);
		waddstr(stdscr, buffer);
	}

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		waddstr(stdscr, "Press ENTER to continue.");

		refresh();
		nocbreak();
		wgetch(stdscr);
	}

	if ( this->test_name != 0 )
	{
		fprintf(stderr, "Maximum Points for test %s: %d / %d\n\n", this->test_name, this->best_score,
		        this->run_points + this->finish_points);
	}
	else
	{
		fprintf(stderr, "Maximum Points for test: %d / %d\n\n", this->best_score,
		        this->run_points + this->finish_points);
	}

	clear();
	endwin(); /* End curses mode		  */
	clear();
}

void GameManager::computeRoundScore( int round, GraphMap* map )
{
	int pts = this->run_points;
	if ( this->goal_mode == GOAL_MODE_HEROWINS )
	{
		int survive_pts = floor(double(this->finish_points * this->num_heroes) / double(this->start_num_heroes * 2));

		pts += survive_pts;

		int eatable_pts = floor(
		        double(this->finish_points * ((this->start_num_eatables-this->start_num_powerups) - (this->num_eatable - this->num_powerups)))
		                / double((this->start_num_eatables -this->start_num_powerups)* 2));

		pts += eatable_pts;

		int powerup_pts = floor(
		        double(this->finish_points * (this->start_num_powerups - this->num_powerups))
		                / double(this->start_num_powerups * 2));

		pts += powerup_pts;
	}

	if ( this->goal_mode == GOAL_MODE_ENEMYWINS )
	{
		int survive_pts = floor(
		        double(this->finish_points * (this->start_num_heroes - this->num_heroes))
		                / double(this->start_num_heroes * 2));

		pts += survive_pts;

		int eatable_pts = floor(
		        double(this->finish_points * ((this->num_eatable - this->num_powerups)))
		                / double((this->start_num_eatables-this->start_num_powerups) * 2));

		pts += eatable_pts;

		int powerup_pts = floor(
		        double(this->finish_points * (this->num_powerups)) / double(this->start_num_powerups * 2));

		pts += powerup_pts;
	}
	if ( this->goal_mode == GOAL_MODE_POWERUP_SURVIVES )
	{
		int survive_pts = floor(
		        double(this->finish_points * (this->start_num_heroes - this->num_heroes))
		                / double(this->start_num_heroes * 2));

		pts += survive_pts;

		int powerup_pts = floor(double(this->finish_points * (this->num_powerups)) / double(this->start_num_powerups));

		pts += powerup_pts;
	}

	if ( this->test_name != 0 )
	{
		fprintf(stderr, "Points for test %s (Round %d): %d / %d\n\n", this->test_name, round + 1, pts,
		        this->run_points + this->finish_points);
	}
	else
	{
		fprintf(stderr, "Points for test (Round %d): %d / %d\n\n", round + 1, pts,
		        this->run_points + this->finish_points);
	}

	if ( pts > this->best_score )
	{
		this->best_score = pts;
	}
}

void GameManager::writeEndMessageAndWait( int round, GraphMap* map )
{
	this->render(stdscr, map);

	int cur_h = map->h;

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		if ( this->message_buffer != 0 )
		{
			waddstr(stdscr, this->message_buffer);
			free(this->message_buffer);
			this->message_buffer = 0;
		}
	}

	char buffer[1000];

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		sprintf(buffer, "Moves: %3d / %4d", this->cur_move, this->max_num_moves);
		waddstr(stdscr, buffer);
	}

	int pts = this->run_points;

	if ( this->goal_mode == GOAL_MODE_HEROWINS )
	{
		int survive_pts = floor(double(this->finish_points * this->num_heroes) / double(this->start_num_heroes * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d heroes survived. (%d points)", this->num_heroes, this->start_num_heroes,
			        survive_pts);
			waddstr(stdscr, buffer);
		}
		pts += survive_pts;

		int eatable_pts = floor(
		        double(this->finish_points * ((this->start_num_eatables-this->start_num_powerups) - ((this->num_eatable - this->num_powerups))))
		                / double((this->start_num_eatables-this->start_num_powerups) * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d eatables survived. (%d points)", this->num_eatable - this->num_powerups,
			        this->start_num_eatables-this->start_num_powerups, eatable_pts);
			waddstr(stdscr, buffer);
		}
		pts += eatable_pts;

		int powerup_pts = floor(
		        double(this->finish_points * (this->start_num_powerups - this->num_powerups))
		                / double(this->start_num_powerups * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d powerups survived. (%d points)", this->num_powerups, this->start_num_powerups,
			        powerup_pts);
			waddstr(stdscr, buffer);
		}
		pts += powerup_pts;
	}

	if ( this->goal_mode == GOAL_MODE_ENEMYWINS )
	{
		int survive_pts = floor(
		        double(this->finish_points * (this->start_num_heroes - this->num_heroes))
		                / double(this->start_num_heroes * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d heroes survived. (%d points)", this->num_heroes, this->start_num_heroes,
			        survive_pts);
			waddstr(stdscr, buffer);
		}
		pts += survive_pts;

		int eatable_pts = floor(
		        double(this->finish_points * ( (this->num_eatable - this->num_powerups)))
		                / double((this->start_num_eatables-this->start_num_powerups) * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d eatables survived. (%d points)", this->num_eatable - this->num_powerups,
			        this->start_num_eatables-this->start_num_powerups, eatable_pts);
			waddstr(stdscr, buffer);
		}
		pts += eatable_pts;

		int powerup_pts = floor(
		        double(this->finish_points * (this->num_powerups)) / double(this->start_num_powerups * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d powerups survived. (%d points)", this->num_powerups, this->start_num_powerups,
			        powerup_pts);
			waddstr(stdscr, buffer);
		}
		pts += powerup_pts;
	}
	if ( this->goal_mode == GOAL_MODE_POWERUP_SURVIVES )
	{
		int survive_pts = floor(
		        double(this->finish_points * (this->start_num_heroes - this->num_heroes))
		                / double(this->start_num_heroes * 2));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d heroes survived. (%d points)", this->num_heroes, this->start_num_heroes,
			        survive_pts);
			waddstr(stdscr, buffer);
		}
		pts += survive_pts;

		int powerup_pts = floor(double(this->finish_points * (this->num_powerups)) / double(this->start_num_powerups));
		{
			cur_h++;
			wmove(stdscr, cur_h, 0);
			sprintf(buffer, "%d of %d powerups survived. (%d points)", this->num_powerups, this->start_num_powerups,
			        powerup_pts);
			waddstr(stdscr, buffer);
		}
		pts += powerup_pts;
	}

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);

		this->actor_time = double(clock() - this->start_clock) / double(CLOCKS_PER_SEC);

		sprintf(buffer, "Required %02.04lf seconds of cpu time.", this->actor_time);
		waddstr(stdscr, buffer);
	}

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		sprintf(buffer, "Points for this test (Round %d): %d / %d", round + 1, pts,
		        this->run_points + this->finish_points);
		waddstr(stdscr, buffer);
	}
	if ( this->test_name != 0 )
	{
		fprintf(stderr, "Points for test %s (Round %d): %d / %d\n\n", this->test_name, round + 1, pts,
		        this->run_points + this->finish_points);
	}
	else
	{
		fprintf(stderr, "Points for test (Round %d): %d / %d\n\n", round + 1, pts,
		        this->run_points + this->finish_points);
	}

	if ( pts > this->best_score )
	{
		this->best_score = pts;
	}

	{
		cur_h++;
		wmove(stdscr, cur_h, 0);
		waddstr(stdscr, "Press ENTER to continue.");

		refresh();
		nocbreak();
		wgetch(stdscr);
		cbreak();
	}

}
