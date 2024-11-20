#include <iostream>
#include <string>
#include <cassert>

#include "graph.hh"
#include "dfs.hh"
#include "sssp.hh"
#include "bfs.hh"
#include "bc.hh"
#include "../ext/cpp-arg-parse/src/argparse.hh"

using namespace simple;
int main(int argc, char *argv[]) {
    // Parse the input arguments
    std::string info = "This program processes graphs!";

    // Write the expected number of arguments. Keeping it simple!
    int expected_count = 7;

    // Argparse is ready to be initialized.
    Argparse args(argc, expected_count, info);

    // Allocate memory for the parser and initialize the arguments.
    args.allocArgs(expected_count);
    args.initArgs("-a", "--algorithm", "algorithm to run",
                                            "bfs, dfs, sssp, bc, allocator");
    args.initArgs("-g", "--graph", "path to a graph", "");
    args.initArgs("-t", "--total-hosts", "total number of hosts", "");
    args.initArgs("-i", "--host-id", "ID of the current host", "");
    // special arguments
    args.initArgs("-h", "--help", "display this message", "");
    args.initArgs("-v", "--verbose", "enable verbose", "");
    args.initArgs("-z", "--test", "enable testing mode", "0, 1");

    args.setArgs(argv);

    // All arguments parsed! Now create the initial errors!
    // check if there is --help or --verbose present in the initial string.
    if(args.getArgs("-h") == "1") {
        // help requested.
        args.printHelpArgs();
        return 0;
    }
    // See if the user wants verbose enabled.
    bool verbose = false;
    if(args.getArgs("-v") == "1")
        verbose = true;

    bool test_mode = false;
    if (args.getArgs("-z") == "1")
        test_mode = true;
    else if (args.getArgs("-z") == "0") {}
    else {
        std::cout << "Unknown value!" << std::endl;
        return -1;
    }

    // convert host id
    int host_id = -1;
    try {
        host_id = std::stoi(args.getArgs("-i"));
    }
    catch (std::exception &err) {
        std::cout << "Illegal host!" << std::endl;
        return -1;
    }

    // Determine the algorithm to run:
    int algo = 0;
    if (args.getArgs("-a") == "bfs")
        algo = 1;
    else if (args.getArgs("-a") == "dfs")
        algo = 2;
    else if (args.getArgs("-a") == "sssp")
        algo = 3;
    else if (args.getArgs("-a") == "bc")
        algo = 4;
    else if (args.getArgs("-a") == "allocator") {
        assert(host_id == 0);
        algo = 5;
    }
    else {
        std::cout << "Unkonwn algo: " << args.getArgs("-a") << std::endl;
        return -1;
    }

    // Create a Graph object for the given graph.
    Graph *G = new Graph(args.getArgs("-g"), host_id);

    std::cout << "======= Graph Processing Started for ";
    switch (algo) {
        case 0:
        case 1: {   // BFS code
            std::cout << "BFS =======" << std::endl; 
            BFS *worker = new BFS(G, 0);
            free(worker);
            break;
        }
        case 2: {   // DFS code
            std::cout << "DFS =======" << std::endl; 
            DFS *worker = new DFS(G, 0);
            free(worker);
            break;
        }
        case 3: {   // SSSP case
            std::cout << "SSSP =======" << std::endl; 
            SSSP *worker = new SSSP(G, 0);
            free(worker);
            break;
        }
        case 4: {   // BC case
            std::cout << "BC =======" << std::endl; 
            BC *worker = new BC(G);
            free(worker);
            break;
        }
        case 5: {   // Assigned to the allocator right now.
                break;
        }
        default: {
            std::cout << "switch failed!" << std::endl;
            return -1;
        }
            
    }
    // Clear the memory? This will delete the local copy of the object. But
    // make sure that the master does not clear the memory.
    if (host_id != 0)
        free(G);
    else {
        // can the master end without munmapping the mmap??
        // TODO: Remove this when finalizing the code for sharing
        free(G);
    }
    return 0;
}
