#include "dfs.hh"

namespace simple {
DFS::DFS(Graph *G, uint64_t starting_node) {
    // Need to do something? 
    // The algo user shoud not be doing addressing.
    // volatile char* starting_addr = G->getStartingAddr();
    // XXX: can overflow
    int *visited = (int *)calloc(G->getV(), sizeof(int)); // new uint64_t(5); // G->getV());
    for (size_t i = 0 ; i < G->getV() ; i++) {
        if (!visited[i])
            dfs(G, i, visited); // , pointer_to_row_ptr, pointer_to_col_idx);
    }
    std::cout << std::endl;
}

void DFS::dfs(Graph *G, uint64_t node, int *visited) {
    visited[node] = 1;
    std::cout << node << " ";

    for (size_t i = G->getRowPointerAt(node); i < G->getRowPointerAt(node + 1); i++) {
        uint64_t neighbor = G->getColIndexAt(i);
        if (!visited[neighbor])
            dfs(G, neighbor, visited);
    }
}
}
