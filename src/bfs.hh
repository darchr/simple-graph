#ifndef BFS_HH_
#define BFS_HH_
// Simple implementation of bfs
//
#include <iostream>
#include "graph.hh"

namespace simple {
class BFS  {
    private:
    public:
        BFS(Graph *G, uint64_t starting_node);
};
}
#endif