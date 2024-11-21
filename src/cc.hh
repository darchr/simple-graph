#ifndef CC_HH_
#define CC_HH_

#include <iostream>
#include <math.h>

#include "graph.hh"
#include "dfs.hh"

namespace simple {
class CC {
    protected:
        void printCC(int* component, size_t size);
    public:
        CC(Graph *G);
};
}
#endif