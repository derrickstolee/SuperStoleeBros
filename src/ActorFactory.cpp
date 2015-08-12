/*
 * ActorFactory.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: dstolee
 */

#include <stdlib.h>
#include <string.h>
#include "Actor.hpp"
#include "ActorFactory.hpp"
#include <ncurses.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include "GameManager.hpp"
#include "OtherActors.hpp"
#include <math.h>
#include <time.h>
#include <queue>


/* TODO: Insert Includes for Header Files Here. */
#include "dstolee_SmartHero.hpp"
#include "dstolee_SmartEnemy.hpp"
#include "dstolee_SmartPowerup.hpp"

#include "GraphMap.hpp"
#include "GameManager.hpp"
#include "Actor.hpp"
#include "OtherActors.hpp"
#include <stdlib.h>




namespace dstolee
{
void updateDistanceArrayAF( GraphMap* map, int from_type, int* d )
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

void updateDistanceArrayAF( GraphMap* map, int from_x, int from_y, int* d, int* avoid_d, int* pred = 0 )
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

			updateDistanceArrayAF(map, this->avoid_type, this->avoid_array);

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

			updateDistanceArrayAF(map, cur_x, cur_y, this->distance_array, this->avoid_array, this->predecessor_array);

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

class LazyHero: public PursueAvoider
{
	protected:
		int laziness;
	public:
		LazyHero( int type ) :
				PursueAvoider(type, ACTOR_EATABLE, ACTOR_ENEMY)
		{
			this->laziness = 10;
		}

		virtual ~LazyHero()
		{

		}

		virtual const char* getActorId()
		{
			return "lazyhero";
		}

		virtual int selectNeighbor( GraphMap* map, int x, int y )
		{
			if ( map->getNumNeighbors(x, y) <= 1 )
			{
				return 0;
			}

			if ( random() % 100 >= this->laziness )
			{
				return PursueAvoider::selectNeighbor(map, x, y);
			}

			// pick a random moving neighbor!
			return (random() % (map->getNumNeighbors(x, y) - 1)) + 1;
		}

};

}

ActorFactory::ActorFactory()
{
	this->size_actors = 10000;
	this->num_actors = 0;

	this->actors = (Actor**) malloc(this->size_actors * sizeof(Actor*));

	this->size_list = 10;
	this->list_count = 0;
	this->actor_id_for_list = (char**) malloc(this->size_list * sizeof(char*));
	this->lists = (char***) malloc(this->size_list * sizeof(char**));
	this->num_in_list = (int*) malloc(this->size_list * sizeof(int));

	this->actors[num_actors++] = new Actor(ACTOR_EATABLE);
	this->actors[num_actors++] = new RandomActor(ACTOR_EATABLE | ACTOR_POWERUP);
	this->actors[num_actors++] = new KeyboardActor(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::Pursuer(ACTOR_ENEMY, ACTOR_HERO, 0);
	this->actors[num_actors++] = new dstolee::HeroPursuer(ACTOR_ENEMY);
	this->actors[num_actors++] = new dstolee::PowerupPursuer(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::EatablePursuer(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::EnemyPursuer(ACTOR_POWERUP | ACTOR_EATABLE);
	this->actors[num_actors++] = new dstolee::HeroAvoider(ACTOR_POWERUP | ACTOR_EATABLE);
	this->actors[num_actors++] = new dstolee::EnemyAvoider(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::EnemyHeroPA(ACTOR_POWERUP | ACTOR_EATABLE);
	this->actors[num_actors++] = new dstolee::EatableEnemyPA(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::PowerupEnemyPA(ACTOR_HERO);
	this->actors[num_actors++] = new dstolee::LazyHeroPursuer(ACTOR_ENEMY);
	this->actors[num_actors++] = new dstolee::LazyHero(ACTOR_HERO);

	this->actors[num_actors++] = new dstolee::SmartHero();
	this->actors[num_actors++] = new dstolee::SmartEnemy();
	this->actors[num_actors++] = new dstolee::SmartPowerup();

	this->size_actor_ids = this->num_actors;
	this->num_actor_ids = 0;
	this->actor_ids = (char**) malloc(this->size_actor_ids * sizeof(char*));

	for ( int i = 0; i < this->num_actors; i++ )
	{
		bool found = false;

		const char* c = this->actors[i]->getActorId();

		for ( int j = 0; !found && j < this->num_actor_ids; j++ )
		{
			if ( strcmp(this->actor_ids[j], c) == 0 )
			{
				found = true;
			}
		}

		if ( !found )
		{
			this->actor_ids[this->num_actor_ids] = (char*) malloc(strlen(c) + 1);
			strcpy(this->actor_ids[this->num_actor_ids], c);
			(this->num_actor_ids)++;
		}
	}
}

ActorFactory::~ActorFactory()
{
	for ( int i = 0; i < this->num_actors; i++ )
	{
		delete this->actors[i];
	}

	free(this->actors);
	this->actors = 0;
	this->num_actors = 0;

	for ( int i = 0; i < this->list_count; i++ )
	{
		free(this->actor_id_for_list[i]);
		for ( int j = 0; j < this->num_in_list[i]; j++ )
		{
			free(this->lists[i][j]);
		}
		free(this->lists[i]);
	}
	free(this->lists);
	free(this->actor_id_for_list);
	free(this->num_in_list);

	for ( int i = 0; i < this->num_actor_ids; i++ )
	{
		free(this->actor_ids[i]);
	}
	free(this->actor_ids);
}

const char** ActorFactory::getAllNetIdsForActor( const char* actorid, int& num_netids )
{
	int found_index = -1;

	for ( int i = 0; i < this->list_count; i++ )
	{
		if ( strcmp(this->actor_id_for_list[i], actorid) == 0 )
		{
			found_index = i;
			break;
		}
	}

	if ( found_index < 0 )
	{
		// make a new one
		found_index = this->list_count;

		this->actor_id_for_list[found_index] = (char*) malloc(strlen(actorid) + 1);
		strcpy(this->actor_id_for_list[found_index], actorid);

		this->lists[found_index] = (char**) malloc(this->num_actors * sizeof(char*));
		this->num_in_list[found_index] = 0;

		for ( int i = 0; i < this->num_actors; i++ )
		{
			if ( strcmp(this->actors[i]->getActorId(), actorid) == 0 )
			{
				const char* c = this->actors[i]->getNetId();
				this->lists[found_index][this->num_in_list[found_index]] = (char*) malloc(strlen(c) + 1);
				strcpy(this->lists[found_index][this->num_in_list[found_index]], c);
				(this->num_in_list[found_index])++;
			}
		}

		(this->list_count)++;
	}

	num_netids = this->num_in_list[found_index];
	return (const char**) this->lists[found_index];
}

Actor* ActorFactory::getActor( const char* netid, const char* actorid )
{
	for ( int i = 0; i < this->num_actors; i++ )
	{
		// if netid is blank, then just pick the first one!
		if ( (netid == 0 || strcmp(this->actors[i]->getNetId(), netid) == 0)
		        && strcmp(this->actors[i]->getActorId(), actorid) == 0 )
		{
			return this->actors[i];
		}
	}

	return 0;
}

const char** ActorFactory::getAllActorIds( int& num_actorids )
{
	num_actorids = this->num_actor_ids;

	return (const char**) this->actor_ids;
}

