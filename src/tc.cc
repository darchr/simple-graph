#include "tc.hh"

namespace simple {
TC::TC(Graph *G) {
    triangle_count = 0;
    for (size_t u = 0; u < G->getV(); u++) {
        uint64_t row_start_u = G->getRowPointerAt(u);
        uint64_t row_end_u = G->getRowPointerAt(u + 1);

        for (size_t idx = row_start_u; idx < row_end_u; idx++) {
            uint64_t v = G->getColIndexAt(idx);

            if (u < v) {
                uint64_t row_start_v = G->getRowPointerAt(v);
                uint64_t row_end_v = G->getRowPointerAt(v + 1);

                triangle_count += intersectCount(G, row_start_u, row_start_v,
                            row_end_u - row_start_u, row_end_v - row_start_v);
            }
        }
    }
    std::cout << "  Triange Count = " << triangle_count;
    std::cout << std::endl;
}

uint64_t TC::min(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}

uint64_t TC::intersectCount(
        Graph *G, uint64_t start1, uint64_t start2, size_t len1, size_t len2) {
    uint64_t i = 0, j = 0, count = 0;
    while (i < len1 && j < len2) {
        if (G->getColIndexAt(start1 + i) < G->getColIndexAt(start2 + j)) {
            i++;
        }
        else if (G->getColIndexAt(start1 + i) > G->getColIndexAt(start2 + j)) {
            j++;
        }
        else {
            count++;
            i++;
            j++;
        }
    }
    return count;
} 
}