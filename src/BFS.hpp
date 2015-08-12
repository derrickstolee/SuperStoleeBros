
#include "GraphMap.hpp"

void freeBFS();

/**
 * Returns minimum-length path.
 */
int BFS( GraphMap* map, int x, int y, int a, int b, int& first_neighbor );

/**
 * Returns minimum-length path given avoiding certain positions
 */
int BFS( GraphMap* map, int x, int y, int a, int b, int& first_neighbor, int* avoid_pos, int num_avoid );

/**
 * Add a list of positions that are reachable from x,y to the given list.
 *
 * May require reallocing the avoid_pos array.
 */
void getRange(GraphMap* map, int x, int y, int radius, int*& avoid_pos, int& num_avoid, int& size_avoid);


