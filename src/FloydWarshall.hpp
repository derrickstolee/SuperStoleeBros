/*
 * FloydWarshall.hpp
 *
 *  Created on: Apr 4, 2014
 *      Author: dstolee
 */

#ifndef FLOYDWARSHALL_HPP_
#define FLOYDWARSHALL_HPP_

#include "GraphMap.hpp"

namespace dstolee
{
class FloydWarshall
{
	protected:
		int N;
		int* distances;

		int* avoid_set;
		int num_avoid;

		GraphMap* map;

		int* avoid_distances;

	public:
		FloydWarshall( GraphMap* map );
		virtual ~FloydWarshall();

		int getDistance( int i, int j );

		void recomputeAvoidDistances( int* avoid, int num_avoid);
		int getDistance( int i, int j, int* avoid, int num_avoid);

		double getAverageDistanceTo( int i, int* others, int num_others );
		double getAverageDistanceFrom( int i, int* others, int num_others );
};
}

#endif /* FLOYDWARSHALL_HPP_ */
