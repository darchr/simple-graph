#include "dfs.hh"

namespace simple {
DFS::DFS() {
    // someone wants to use DFS as an algorithm.
}
DFS::DFS(Graph *G, uint64_t starting_vertex) {
    // Need to do something? 
    // The algo user shoud not be doing addressing.
    // volatile char* starting_addr = G->getStartingAddr();
    // XXX: can overflow
    int *visited = (int *)calloc(G->getV(), sizeof(int));
    
    // visited for the starting node must be filled up no?
    visited[starting_vertex] = 1;
    for (size_t i = 0 ; i < G->getV() ; i++) {
        if (!visited[i])
            dfs(G, i, visited); // , pointer_to_row_ptr, pointer_to_col_idx);
    }
    std::cout << std::endl;
}

void DFS::dfs(Graph *G, uint64_t vertex, int *visited) {
    visited[vertex] = 1;
    std::cout << vertex << " ";

    for (size_t i = G->getRowPointerAt(vertex);
                i < G->getRowPointerAt(vertex + 1); i++) {
        uint64_t neighbor = G->getColIndexAt(i);
        if (!visited[neighbor])
            dfs(G, neighbor, visited);
    }
}

void DFS::dfsAPI(Graph *G, uint64_t vertex, int *visited, int component_id,
                                                            int* component) {
    visited[vertex] = 1;
    component[vertex] = component_id;

    uint64_t row_start = G->getRowPointerAt(vertex);
    uint64_t row_end = G->getRowPointerAt(vertex + 1);

    for (size_t i = row_start; i < row_end ; i++) {
        uint64_t neighbor = G->getColIndexAt(i);
        if (!visited[neighbor])
            dfsAPI(G, neighbor, visited, component_id, component);
    }

}
}
