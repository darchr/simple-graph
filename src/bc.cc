#include "bc.hh"

namespace simple {
BC::BC(Graph *G) {
    // All the variables used to navigate the graph and generate BC data will
    // be local to the host.
    double* centrality = (double*)calloc(G->getV(), sizeof(double));
    for (uint64_t s = 0 ; s < G->getV() ; s++) {
        uint64_t *stack = (uint64_t *) malloc (G->getV() * sizeof(uint64_t));
        uint64_t *queue = (uint64_t *) malloc (G->getV() * sizeof(uint64_t));

        uint64_t *pred_count = (uint64_t *) calloc (G->getV(), sizeof(uint64_t));
        uint64_t** preds = (uint64_t**)malloc(G->getV() * sizeof(uint64_t*));

        double* sigma = (double*)calloc(G->getV(), sizeof(double));
        double* dist = (double*)calloc(G->getV(), sizeof(double));
        double* delta = (double*)calloc(G->getV(), sizeof(double));
        
        for (size_t i = 0; i < G->getV(); i++) {
            preds[i] = (uint64_t *) malloc (G->getV() * sizeof(uint64_t));
        }
        
        sigma[s] = 1;
        dist[s] = 0;
        uint64_t queue_start = 0, queue_end = 0;
        queue[queue_end++] = s;

        while (queue_start < queue_end) {
            int v = queue[queue_start++];
            stack[queue_start - 1] = v;

            uint64_t row_start = G->row_pointer[v];
            uint64_t row_end = G->row_pointer[v + 1];

            for (size_t i = row_start; i < row_end; i++) {
                uint64_t w = G->column_index[i];

                if (dist[w] == 0 && w != s) {
                    queue[queue_end++] = w;
                    dist[w] = dist[v] + 1;
                }
                if (dist[w] == dist[v] + 1) {
                    sigma[w] += sigma[v];
                    preds[w][pred_count[w]++] = v;
                }
            }
        }

        while (queue_start > 0) {
            uint64_t w = stack[--queue_start];
            for (size_t i = 0; i < pred_count[w]; i++) {
                uint64_t v = preds[w][i];
                delta[v] += (sigma[v] / sigma[w]) * (1 + delta[w]);
            }
            if (w != s) {
                centrality[w] += delta[w];
            }
        }

        free(stack);
        free(queue);
        free(pred_count);
        for (size_t i = 0; i < G->getV(); i++) {
            free(preds[i]);
        }
        free(preds);
        free(sigma);
        free(dist);
        free(delta);
    }
    for (size_t i = 0 ; i < G->getV() ; i++)
        std::cout << "  Vertex: " << i << " : " << centrality[i] << std::endl;
    std::cout << std::endl;
}
}