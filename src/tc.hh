#ifndef TC_HH_
#define TC_HH_

#include <iostream>
#include "graph.hh"

namespace simple {
class TC  {
    private:
        uint64_t triangle_count;
    protected:
        uint64_t min(uint64_t a, uint64_t b);
        uint64_t intersectCount(Graph *G, uint64_t start, uint64_t end, size_t len1, size_t len2);
    public:
        TC(Graph *G);
};
}
#endif