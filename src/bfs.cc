#include "bfs.hh"

namespace simple {
BFS::BFS(Graph *G, uint64_t starting_node) {

    // XXX: can overflow, maybe warn the user?
    int *visited = (int *) calloc (G->getV(), sizeof(int));
    uint64_t *queue = (uint64_t *) malloc (G->getV() * sizeof(uint64_t));

    int front = 0, rear = 0;
    visited[starting_node] = 1;
    queue[rear++] = starting_node;

    while (front < rear) {
        uint64_t current_vertex = queue[front++];

        std::cout << current_vertex << " ";

        uint64_t row_start = G->row_pointer[current_vertex];
        uint64_t row_end = G->row_pointer[current_vertex + 1];

        for (uint64_t i = row_start; i < row_end ; i++) {
            int neighbor = G->column_index[i];
            if (!visited[neighbor]) {
                visited[neighbor] = 1;
                queue[rear++] = neighbor;
            }
        }
    }

    free(visited);
    free(queue);

    std::cout << std::endl;
}
}
