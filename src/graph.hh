#ifndef GRAPH_hH_
#define GRAPH_HH_

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cinttypes>
#include <map>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

// To implement a graph, we need to hardcode the metadata of the graph. In this
// simple graph processing framework, we'll support simple directed and
// weighted graphs. The length of the metadata includes the number of vertices,
// edges, size of the row pointer index array, size of the column index array,
// size of the weights, and a synchroinization variable.
#define METADATA 6

namespace simple {
// Instead of hardcoding offset indices as numbers, I am using a enum for
// better understandability.
// TODO: Update the .cc file!
#pragma once
enum OffsetList {
    VERTEX,
    EDGE,
    ROWP_SIZE,
    COLI_SIZE,
    WEIGHT_SIZE,
    SYNC
};

// Synchronization states should also be encoded into enum
#pragma once
enum SyncState {
    ALLOCATING,
    READY,
    COMPLETE
};

#pragma once
class Graph {
    private:
        // Graph class stores a pointer to the mmaped region of the actual
        // graph. For any graph related operation, the *_graph pointer is used
        // to find the head of the graph.
        // std::map<std::string, void*> addr_map;
        // void* _mmap;
        int* _graph;
        // The CSRs metadata is stored as independent variables in the local
        // memory. The metadata includes the number of edges, vertices, size of
        // the row_pointer array, size of the column_idx array and a
        // synchronization variable aligned by uint64_t. The metadata is also
        // stored on the mmap.
        // ___________________________________________________________ .. _____
        // | V | E | size | size | sync | row | col | weights                 |
        // |___|___|______|______|______|_____|_____|_________________ .. ____|
        //
        // The graph constructor will set these variables.
        //
        uint64_t _V;
        uint64_t _E;
        size_t _size_row_pointer;
        size_t _size_col_idx;
        size_t _size_weights;
        uint64_t _local_sync_copy;
        // need to set a boolean to determine if there are weights
        bool _has_weight;
        // Assign a verbose variable and another for test
        bool _test;
        bool _verbose;
        // The master node will write the graph from the given file into the
        // mmaped space!
        void graphWriter(std::string path);
        // Other placeholder methods
        int* getGraphPointer(size_t size, int host_id);
    protected:
        // Need to add a couple of set methods to make my life easier to set
        // the metadata and debug more easily
        // TODO
        void setV(uint64_t value);
        void setE(uint64_t value);
        void setRowPointerSize(size_t value);
        void setColIndexSize(size_t value);
        void setWeightsSize(size_t value);
    public:
        // The graph object should be allocated however, this object does not
        // store the graph. Instead it is a wrapper around a mmap
        Graph(std::string path, int host_id, bool test, bool verbose);
        // To remove offset value retrival, we'll use three pointers for
        // getting the rowpointer, colindex and the weights directly from the
        // object; the read/write permissions will be set by the mmap call
        // depending upon the host_id.
        int *row_pointer;
        int *column_index;
        int *weights;
        // Format of the graph should include uint64_t to support outgoing
        // edges of up to 2^64 - 1.
        uint64_t getOffset(size_t index);
        // We finally need some public methods that makes life easier to
        // program the grraph algorithm
        uint64_t getV();
        uint64_t getE();
        uint64_t getRowPointerSize();
        uint64_t getColIndexSize();
        uint64_t getWeightsSize();
        void printGraph();
        volatile char* getStart();
};
}
// extern class Graph *G;
#endif
