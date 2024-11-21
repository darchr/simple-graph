#include "cc.hh"

namespace simple {
CC::CC(Graph *G) {
    // To perform connected components, we need DFS, which can be used interna-
    // lly.
    DFS *dfs = new DFS();

    int *component = (int *) calloc (G->getV(), sizeof(int));
    int *visited = (int *) calloc (G->getV(), sizeof(int));
    int component_id = 0;

    for (size_t i = 0 ; i < G->getV(); i++) {
        if (!visited[i]) {
            // Need to call an API that does DFS and does not print the results
            // on stdout.
            dfs->dfsAPI(G, i, visited, component_id, component);
            component_id++;
        }
    }
    free(visited);

    printCC(component, G->getV());
    free(component);
    delete dfs;
    std::cout << std::endl;
}
void CC::printCC(int* component, size_t size) {
    for (size_t i = 0 ; i < size ; i++)
        std::cout << "  Vertex: " << i << " Component: " << component[i] << std::endl;
}
}