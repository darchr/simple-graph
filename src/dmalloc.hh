// So, this is an allocator file that initializes the graph using mmap
// I am targeting a C-like header, which should be compatible with both C/C++.
//

#ifndef DMALLOC_HH_
#define DMALLOC_HH_

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

int* dmalloc(size_t size, int host_id) {
    // hehe, I love the name
    //
    // This function will create a MAP_SHARED memory map for the graph. The
    // idea is to allocate the graph (potentially a large graph) in the
    // remote/disaggregated memory and only the master will have RDWR flag. All
    // other hosts should only have READ permission.
    //
    // @params
    // :size: Size of requested memory in bytes
    // :host_id: An ID sent to dictate whether this is a master or a worker
    //
    // @returns
    // A pointer to the mmap call
    //
    int fd;
    // It is assumed that we are working with DAX devices. Will change change
    // to something else if needed later.
    const char *path = "/dev/dax0.0";
    
    // check if the caller is the master node
    if (host_id == 0)
        fd = open(path, O_RDWR);
    else
        // these are all graph workers. Prevent these workers from writing into
        // the graph.
        fd = open(path, O_RDONLY);

    // make sure that the open call was successful. Otherwise notify the user
    // before exiting.
    if (fd < 0) {
        printf("Error mounting! Make sure that the mount point %s is valid\n",
                path);
        exit(EXIT_FAILURE);
    }
    
    // Try allocating the required size for the graph. This might be
    // complicated if the graph is very large!
    int *ptr = (int *) mmap (
                    nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // The map may fail due to several reasons but we notify the user.
    if (ptr == MAP_FAILED) {
        printf("The mmap call failed! Maybe it's too huge?\n");
        exit(EXIT_FAILURE);
    }
    // The mmap was successful! return the pointer to the user.
    return ptr;
}

int* hmalloc(size_t size, int host_id) {
    // To be used with HUGETLBFS page on a single host for testing.
    //
    // This function will create a MAP_SHARED memory map for the graph. The
    // idea is to allocate the graph (potentially a large graph) in the
    // remote/disaggregated memory and only the master will have RDWR flag. All
    // other hosts should only have READ permission.
    //
    // @params
    // :size: Size of requested memory in bytes
    // :host_id: An ID sent to dictate whether this is a master or a worker
    //
    // @returns
    // A pointer to the mmap call
    //
    FILE *fp = fopen("/mnt/huge", "w+");
    // It is assumed that we are working with DAX devices. Will change change
    // to something else if needed later.

    // make sure that the open call was successful. Otherwise notify the user
    // before exiting.
    if (fp == nullptr) {
        printf("Error mounting! Make sure that the mount point %s is valid\n",
                "non");
        exit(EXIT_FAILURE);
    }
    
    // Try allocating the required size for the graph. This might be
    // complicated if the graph is very large!
    int* ptr = (int *) mmap(
        0x0, 1 << 30 , PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS|  MAP_HUGETLB | (30UL << MAP_HUGE_SHIFT),
        fileno(fp), 0);

    // The map may fail due to several reasons but we notify the user.
    if (ptr == MAP_FAILED) {
        printf("The mmap call failed! Maybe it's too huge?\n");
        exit(EXIT_FAILURE);
    }

    // now return a volatile char with the address
    // volatile char* start_address = (volatile char *) ptr;
    // std::cout << &start_address << std::endl;
    // The mmap was successful! return the pointer to the user.
    return ptr;

}

#endif