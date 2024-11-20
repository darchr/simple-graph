#ifndef SSSP_HH
#define SSSP_HH_
// Simple implementation of sssp
//
#include <iostream>

#include <stdlib.h>
#include <limits.h>

#include "graph.hh"

namespace simple {
class SSSP  {
    private:
    public:
        SSSP(Graph *G, uint64_t source);
        void dijkstra(Graph *G, uint64_t source);
};
}
#endif