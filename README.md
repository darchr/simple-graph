# A Simple Graph Processing Software for Shared Disaggregated Memory

A single threaded graph processing framework where is memory is manually
managed.
Useful for disaggregated memories.

This is a simple framework for graph processing.
I have implemented this to quickly test multihost graph processing.

Depends on:
* cpp-arg-parse (`github.com/kaustav-goswami/cpp-arg-parse`)
* cmake

```sh
make -j4
# The master node is called via
./simple_graph \
    --algorithm allocator \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 0
# The worker nodes are called via
./simple_graph --algorithm bfs \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 1
./simple_graph --algorithm dfs \
    --graph tests/graph.csr \
    --total-hosts 3 \
    --host-id 2
```

The graph is read from a file in csr format and is stored in class Graph as a
CSR graph in memory.

`class Graph` is a host-side object, that has a pointer to the start of the
remote memory.
It assumes that the remote memory is a memory-mapped range on the
disaggreged/remote device.
Only the allocator initializes the graph, which also needs to be the master
node.
Once the setup is complete, a synchronization variable is set in the shared
memory range (needs to be uncacheable; must be hardware managed).
The workers then begin working on the algorithm specified.

Current implementation is single-threaded.

## Graph Format

The input graphs can be **directed** and **weighted**.
The program expects that the graph will be in csr format with:
```
<Vertex Count>
<Edge Count>
<Values of the Row Pointer Array separated by " ">
<Values of the Column Index Array separated by " ">
<(Optional) Weights Array separated by " ">
```

Sample graphs are stored in `tests/` directory.

## Building

```sh
git clone git@github.com:kaustav-goswami/simple-graph.git
cd simple-graph
mkdir build
cd build
cmake ..
make -j4
```

## Supported Graph Algorithms

* BFS
* DFS
* BC
* SSSP

Cite: The algorithm implementation code is mostly AI generated.
