#include "graph.hh"

#include <cstdint>
#include "dmalloc.hh"

namespace simple {
Graph::Graph(std::string path, int host_id, bool test = false,
                bool verbose = false) : _test(test), _verbose(verbose) {
    // Allocate the graph using dalloc. Ignore the class method. Make sure that
    // the user is running it in gem5. If not, the test mode must be true.
    if (_test)
        _graph = hmalloc(1 >> 30, host_id);
    else
        _graph = dmalloc(1 >> 30, host_id);

    // Allocation complete! If I am the master, set the synchronization
    if (host_id == 0) {
        // Wait, first write the graph!
        graphWriter(path);
        *((uint64_t *) (this->_graph + getOffset(SYNC))) = 1;
        // Honestly, the master can now end the process!
    }
    else {
        // variable to true and let the workers start working.
        do {
            _local_sync_copy = \
                        *((uint64_t *) (this->_graph + getOffset(SYNC)));
            if (_local_sync_copy == 1)
                break;
        } while (_local_sync_copy != 1);

        // The start of the graph is stored in *_graph; Set the rest of the
        // metadata variables. The master sets these variables up and sets its
        // own private variables. But the workers needs to wait until master
        // sets these up.
        this->_V = *((uint64_t *) (this->_graph + getOffset(VERTEX)));
        this->_E = *((uint64_t *) (this->_graph + getOffset(EDGE)));
        this->_size_row_pointer = \
                        *((uint64_t *) (this->_graph + getOffset(ROWP_SIZE)));
        this->_size_col_idx = \
                        *((uint64_t *) (this->_graph + getOffset(COLI_SIZE)));
        this->_size_weights = \
                    *((uint64_t *) (this->_graph + getOffset(WEIGHT_SIZE)));
        if (this->_size_weights == 0)
            _has_weight = false;
        else
            _has_weight = true;
    }
    // Be very careful when to use the synchronization variable. It is very
    // expensive! The allocation is complete and the workers are ready to
    // start working on the graph!
    if (_verbose)
        printGraph();
}

int* Graph::getGraphPointer(size_t size, int host_id) {
    perror("NotImplementedError! Use dalloc( .. ) instead\n");
    exit(EXIT_FAILURE);
}

uint64_t Graph::getOffset(size_t index) {
    // Returns the offset which when added to the start address gives the value
    // stored at that offset.
    return index * sizeof(uint64_t);
}

uint64_t Graph::getRowAtIndex(size_t index) {
    // Similar to getOffset which additional bounds check! The first five
    // uint64_t entries are reserved for the metadata.
    if (index >= METADATA  && index < (this->_size_row_pointer + METADATA))
        return getOffset(index);
    else {
        printf("The row pointer index %" PRIu64 " is invalid!\n", index);
        exit(EXIT_FAILURE);
    }
}

uint64_t Graph::getColAtIndex_(size_t index) {
    // Similar to getRowAtIndex which additional bounds check! The first five
    // uint64_t entries are reserved for the metadata.
    if (index > (this->_size_row_pointer + METADATA) && 
            index < (this->_size_col_idx + this->_size_row_pointer + METADATA))
        return getOffset(index);
    else {
        printf("The column index index %" PRIu64 " is invalid!\n", index);
        exit(EXIT_FAILURE);
    }
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
                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                uint64_t size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    setRowPointerAt(size++, strtoull(words, &end, 10));
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                setRowPointerSize(size);
            }
            else if (line_count == 3) {
                // First break up all the numbers, then set the total size.
                char *words = strtok(cstr, " ");
                uint64_t size = 0;
                while (words != nullptr) {
                    // The setter methods should easily do the trick!
                    setColIndexAt(size++, strtoull(words, &end, 10));
                    words = strtok(nullptr, " ");
                }
                // Finally set the offset value correctly via the setter method
                setColIndexSize(size);
            }
            // If the graph has weights, then there will be another line with
            // valid weights.
            else if (line_count == 4) {
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
                        setWeightsAt(size++, strtoull(words, &end, 10));
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

volatile char* Graph::getStart() {
    std::cout << &this->_graph << " " << std::endl;
    return 0;
    // return this->_graph;
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
        std::cout << getRowPointerAt(i) << " ";

    std::cout << "]\nF = [ " ;
    for (size_t i = 0 ; i < getColIndexSize() ; i++)
        std::cout << getColIndexAt(i) << " ";
        
    std::cout << "]\nW = [ ";
    
    if (_has_weight) {
        for (size_t i = 0 ; i < getWeightsSize() ; i++)
        std::cout << getWeightsAt(i) << " ";
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
// The effective index is calculated and is oblivious to the programmer
uint64_t Graph::getRowPointerAt(size_t index) {
    return *((uint64_t *) (this->_graph + getOffset(METADATA + index)));
}
uint64_t Graph::getColIndexAt(size_t index) {
    return *((uint64_t *) (this->_graph + getOffset(METADATA +
                                                getRowPointerSize() + index)));
}
uint64_t Graph::getWeightsAt(size_t index) {
    return *((uint64_t *) (this->_graph + getOffset(METADATA +
                            getRowPointerSize() + getColIndexSize() + index)));
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
void Graph::setRowPointerAt(size_t index, uint64_t value) {
    *((uint64_t *) (_graph + getOffset(METADATA + index))) = value;
}
void Graph::setColIndexAt(size_t index, uint64_t value) {
    *((uint64_t *) (_graph + getOffset(METADATA +
                                        getRowPointerSize() + index))) = value;
}
void Graph::setWeightsAt(size_t index, uint64_t value) {
    *((uint64_t *) (_graph + getOffset(METADATA +
                    getRowPointerSize() + getColIndexSize() + index))) = value;
}
}
