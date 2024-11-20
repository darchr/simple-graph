#ifndef DFS_hH_
#define DFS_HH_
// Simple implementation of dfs
//
#include <iostream>
#include "graph.hh"

namespace simple {
class DFS  {
    private:
    public:
        DFS(Graph *G, uint64_t starting_node);
        void dfs(Graph *G, uint64_t node, int *visited);
};
}
#endif