#include "sssp.hh"


namespace simple {
SSSP::SSSP(Graph *G, uint64_t source) {
    dijkstra(G, source);
    std::cout << std::endl;
}

void SSSP::dijkstra(Graph *G, uint64_t source) {
    // dist will be in the local memory
    uint64_t *dist = (uint64_t *) malloc (G->getV() * sizeof(uint64_t));
    int *visited = (int *) calloc (G->getV(), sizeof(int));

    // initialize distances
    for (size_t i = 0 ; i < G->getV() ; i++)
        dist[i] = INT_MAX;

    dist[source] = 0;

    for (size_t count = 0 ; count < G->getV() - 1 ; count++) {
        int minDist = INT_MAX;
        int minIndex = -1;

        for (size_t v = 0 ; v < G->getV() ; v++) {
            if (!visited[v] && dist[v] < minDist) {
                minDist = dist[v];
                minIndex = v;
            }
        }

        visited[minIndex] = 1;

        for (size_t i = G->getRowPointerAt(minIndex);
                                i < G->getRowPointerAt(minIndex + 1); i++) {
            uint64_t neighbor = G->getColIndexAt(i);
            uint64_t weight = G->getWeightsAt(i);
            if (!visited[neighbor] &&
                        dist[minIndex] != INT_MAX &&
                        dist[minIndex] + weight < dist[neighbor]) {
                dist[neighbor] = dist[minIndex] + weight;
            }
        }
    }
    std::cout << "Vertex Distance from Source " << source << std::endl;
    for (size_t i = 0 ; i < G->getV() ; i++)
        std::cout << i << " \t\t " << dist[i] << std::endl;
}
}