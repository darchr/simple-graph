#include "graph.hh"

#include <cstdint>
#include "dmalloc.hh"

namespace simple {
Graph::Graph(std::string path, int host_id, bool test = false,
                bool verbose = false) : _test(test), _verbose(verbose) {
    // Allocate the graph using dalloc. Ignore the class method. Make sure that
    // the user is running it in gem5. If not, the test mode must be true.
    if (_test)
        _graph = hmalloc(1 << 30, host_id);
    else
        _graph = dmalloc(1 << 30, host_id);

    // Allocation complete! If I am the master, set the synchronization
    if (host_id == 0) {
        // Wait, first write the graph!
        graphWriter(path);
        *((uint64_t *) (_graph + getOffset(SYNC))) = 1;
        // Honestly, the master can now end the process!
    }
    else {
        // variable to true and let the workers start working.
        do {
            _local_sync_copy = \
                        *((uint64_t *) (_graph + getOffset(SYNC)));
            if (_local_sync_copy == 1)
                break;
        } while (_local_sync_copy != 1);

        // The start of the graph is stored in *_graph; Set the rest of the
        // metadata variables. The master sets these variables up and sets its
        // own private variables. But the workers needs to wait until master
        // sets these up. The setter methods right now sets both these
        // values up, so this part has to be done manually;
        _V = *((uint64_t *) (_graph + getOffset(VERTEX)));
        _E = *((uint64_t *) (_graph + getOffset(EDGE)));
        _size_row_pointer = \
                        *((uint64_t *) (_graph + getOffset(ROWP_SIZE)));
        _size_col_idx = \
                        *((uint64_t *) (_graph + getOffset(COLI_SIZE)));
        _size_weights = \
                    *((uint64_t *) (_graph + getOffset(WEIGHT_SIZE)));
        if (_size_weights == 0)
            _has_weight = false;
        else
            _has_weight = true;

        // finally allocate the pointer arrays
        row_pointer = &_graph[METADATA];
        column_index = &_graph[METADATA + getRowPointerSize()];
        if (_has_weight == true)
            weights = &_graph[METADATA + getRowPointerSize() +
                                                            getColIndexSize()];
        
        // The worker is ready to work.
    }
    // Be very careful when to use the synchronization variable. It is very
    // expensive! The allocation is complete and the workers are ready to
    // start working on the graph!
    if (_verbose)
        printGraph();
}

uint64_t Graph::getOffset(size_t index) {
    // Returns the offset which when added to the start address gives the value
    // stored at that offset.
    return index * sizeof(uint64_t);
}

void Graph::graphWriter(std::string path) {
    // Uses fstream from C++
    std::fstream file;
    char *filename = new char[path.length()];
    std::strcpy(filename, path.c_str());
    file.open(filename, std::ios::in);
    if (!file) {
        printf("The specified graph file %s does not exist!\n", filename);
        exit(EXIT_FAILURE);
    }
    // The start of the mmap is stored in _graph.
    // We assume a simple storage for the graph i.e. also stored in CSR format.
    if (file.is_open()) {
        std::string lines;
        int line_count = 0;
        while (getline(file, lines)) {
            // printf("line : *%s*\n", lines.c_str());
            // The format is very specific.
            // Line 1: V
            // Line 2: E
            // Line 3: row_pointer_array
            // Line 4: column_index_array
            // Line 5: weights (optional)
            char *end;
            char *cstr = new char[lines.length()];
            std::strcpy(cstr, lines.c_str());

            // For each line, do something!
            if (line_count == 0) {
                setV(strtoull(cstr, &end, 10));
            }
            else if (line_count == 1) {
                setE(strtoull(cstr, &end, 10));
            }
            else if (line_count == 2) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata.
                this->row_pointer = &_graph[METADATA];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                uint64_t size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    row_pointer[size++] = strtoull(words, &end, 10);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setRowPointerSize(size);
            }
            else if (line_count == 3) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                column_index = &_graph[METADATA + getRowPointerSize()];

                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                uint64_t size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    column_index[size++] = strtoull(words, &end, 10);
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                // The next array is ready to be initialized.
                setColIndexSize(size);
            }
            // If the graph has weights, then there will be another line with
            // valid weights.
            else if (line_count == 4) {
                // We will start writing the graph into the mmap space.
                // Allocate the arrays to make sure that the program is easier
                // to understand. We know that this array starts after the
                // metadata + end of the row pointer array.
                weights = &_graph[METADATA + getRowPointerSize() +
                                                            getColIndexSize()];

                // See if this is an extra line first.
                if (lines.length() == 0 || lines.length() == 1) {
                    _has_weight = false;
                    setWeightsSize(0);
                }
                else {
                    _has_weight = true;
                    // First break up all the numbers
                    char *words = strtok(cstr, " ");
                    uint64_t size = 0;
                    
                    while (words != nullptr) {
                        // The setter methods should easily do the trick!
                        weights[size++] = strtoull(words, &end, 10);
                        words = strtok(nullptr, " ");
                    }
                    // Finally set the offset value correctly.
                    setWeightsSize(size);
                }
            }
            else {
                printf("Invalid csr format! c = %d\n", line_count);
                exit(EXIT_FAILURE);
            }
            line_count++;
        }
        if (line_count == 4) {
            // There are only  four lines in the file. therefore, set the
            // weight count to 0
            _has_weight = false; 
            setWeightsSize(0);
        }
    }
    file.close();
}

void Graph::printGraph() {
    // This method prints the graph in CSR format to verify whether the
    // allocation is correct. The user has to enable verbose.

    std::cout << "== Printing the graph ==\n" << std::endl;
    std::cout << "V = " << getV() << std::endl;
    std::cout << "E = " << getE() << std::endl;
    std::cout << "Row Ptr Array Size = " << getRowPointerSize() << std::endl;
    std::cout << "Column Idx Array Size = " << getColIndexSize() << std::endl;
    std::cout << "Weights Size = " << getWeightsSize() << std::endl;

    std::cout << "N = [ ";
    for (size_t i = 0 ; i < getRowPointerSize() ; i++)
        std::cout << this->row_pointer[i] << " ";

    std::cout << "]\nF = [ " ;
    for (size_t i = 0 ; i < getColIndexSize() ; i++)
        std::cout << column_index[i] << " ";
        
    std::cout << "]\nW = [ ";
    
    if (_has_weight) {
        for (size_t i = 0 ; i < getWeightsSize() ; i++)
        std::cout << weights[i] << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "== End of the graph ==\n" << std::endl;
}

// Defining all the getter methods here. For most of the code, use the getter
// methods instead of using the private variable directly.
uint64_t Graph::getV() {
    return _V;
}

uint64_t Graph::getE() {
    return _E;
}
uint64_t Graph::getRowPointerSize() {
    return _size_row_pointer;
}
uint64_t Graph::getColIndexSize() {
    return _size_col_idx;
}
uint64_t Graph::getWeightsSize() {
    return _size_weights;
}

// Writing protected setter methods. Only this and its children should be able
// to set the private variables. The setter methods SETS BOTH THE MMAP and THE
// PRIVATE variables!
void Graph::setV(uint64_t value) {
    *((uint64_t *) (_graph + getOffset(VERTEX))) = value;
    _V = value;
}
void Graph::setE(uint64_t value) {
    *((uint64_t *) (_graph + getOffset(EDGE))) = value;
    _E = value;
}
void Graph::setRowPointerSize(size_t size) {
    *((uint64_t *) (_graph + getOffset(ROWP_SIZE))) = size;
    _size_row_pointer = size;
}
void Graph::setColIndexSize(size_t size) {
    *((uint64_t *) (_graph + getOffset(COLI_SIZE))) = size;
    _size_col_idx = size;
}
void Graph::setWeightsSize(size_t size) {
    *((uint64_t *) (_graph + getOffset(WEIGHT_SIZE))) = size;
    _size_weights = size;
}
}
