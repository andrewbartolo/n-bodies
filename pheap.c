#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>


#include "pheap.h"

#define PAYLOAD_CHUNKS(size) ((size / sizeof(mem_chunk_hdr_t)) + ((0 == size % \
                             sizeof(mem_chunk_hdr_t)) ? 0 : 1))
#define PAYLOAD_SIZE(size) (PAYLOAD_CHUNKS(size) * sizeof(mem_chunk_hdr_t))
#define COMPLETE_CHUNKS(size) (PAYLOAD_CHUNKS(size) + 1)
#define COMPLETE_SIZE(size) (COMPLETE_CHUNKS(size) * sizeof(mem_chunk_hdr_t))
#define NUM_PAGES(size) ((size / pagesize) + ((0 == size % pagesize) ? 0 : 1))

/* for simplicity the size of this next struct is the basic unit
   of allocation in persistent heap; its size defines alignment */
typedef struct mem_chunk_hdr {
  size_t size;                           /* size of this chunk's PAYLOAD ONLY, in bytes */
  struct mem_chunk_hdr *next;            /* ptr to the next block in the free list */
} mem_chunk_hdr_t;

// the struct below defines the complete mmaped backing_file
typedef struct pheap {
  // note - handle must remain the first member of the struct in order to be read by the "locate handle" mmap call
  uint64_t magic_number;
  struct pheap *handle;           /* ptr to the start of the entire persistent heap */
  mem_chunk_hdr_t *freelist;      /* ptr to the first block in the freelist */
  mem_chunk_hdr_t heap[COMPLETE_CHUNKS(DESIRED_HEAP_SIZE)];
} pheap_t;

static pheap_t *handle;  // entry point for the current mmaping of the heap
static long pagesize;    // should this be a member of pheap_t instead?
static bool first_run = true;


// void die(char *msg) {
//   fputs(msg, stderr);
//   exit(1);
// }

/**
 * Sets up the persistent heap for first, or subsequent, use.
 * Upon first invocation, if DESIRED_HANDLE == NULL, the kernel
 * chooses a location for the heap.  Otherwise, the handle stored at
 * offset `sizeof(magic_number)` is used.
 */
void pheap_init() {
  bool heap_exists;
  int backing_file_fd;

  pagesize = sysconf(_SC_PAGESIZE);

  // check if backing_file exists (first time using heap?)
  heap_exists = (0 == access(BACKING_FILE, R_OK | W_OK | X_OK));
  // NOTE - had to chmod0777 the containing folder to get this to work when run as root
  backing_file_fd = open(BACKING_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR
                         | S_IXUSR);

  // Linux guarantees that the mmaped region will be aligned to a page boundary,
  // even if mmap's first argument (desired starting address) is not page-aligned.
  if (!heap_exists) {
    posix_fallocate(backing_file_fd, 0, sizeof(pheap_t));
 
    // needs to be MAP_SHARED to be able to msync() back to the file
    handle = (pheap_t *)mmap((void *)DESIRED_HANDLE, sizeof(pheap_t), PROT_READ
                      | PROT_WRITE | PROT_EXEC, MAP_SHARED, backing_file_fd, 0);

    handle->magic_number = MAGIC_NUMBER;
    handle->handle = handle;
    handle->heap[0].size = PAYLOAD_SIZE(DESIRED_HEAP_SIZE);
    // TODO - may not need this or below if fallocate zeroes bytes anyway
    handle->heap[0].next = NULL;
    handle->freelist = handle->heap;
    
    msync(handle, sizeof(pheap_t), MS_SYNC);  // don't log initial setup write time
  }
  else {
    pheap_t *initial_check = (pheap_t *)mmap(NULL, sizeof(uint64_t) + sizeof(pheap_t *),
                                 PROT_READ, MAP_PRIVATE, backing_file_fd, 0);
    assert(*(uint64_t *)initial_check == MAGIC_NUMBER);
    handle = *(pheap_t **)((char *)initial_check + sizeof(uint64_t));
    munmap(initial_check, sizeof(uint64_t) + sizeof(pheap_t *));

    mmap(handle, sizeof(pheap_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED,
         backing_file_fd, 0);
  }
  
  first_run = false;
}

/**
 * Uses a first-fit strategy to allocate chunks of memory.
 * When some block's next pointer is null, we have reached
 * the end of the freelist.
 */
void *pmalloc(size_t size) {
  if (first_run) pheap_init();
  // cannot malloc a chunk of size 0, nor if no free chunks available
  if (0 == size || NULL == handle->freelist) return NULL;

  // takes possible partially-filled chunk into account
  size_t needed_size = PAYLOAD_SIZE(size);

  mem_chunk_hdr_t *prev = NULL, *curr = handle->freelist;

  // find a large-enough chunk
  while (curr->size < needed_size) {
    if (NULL == curr->next) return NULL;      // traversed entire free list; couldn't find large-enough block
    prev = curr;
    curr = curr->next;
  }

  // if we've gotten here, a suitably-sized chunk has been found

  size_t excess_size = curr->size - needed_size;
  // prepend possible excess from chunk back into the free list
  if (0 == excess_size) { 
    if (handle->freelist == curr) {    // if first block in freelist
        handle->freelist = curr->next;
    }
    else {
      prev->next = curr->next;
    }
  }

  // if we have enough room left for more than just a header (small optimization so that 0-sized chunks do not get added into the freelist)
  else if (excess_size >= (2 * sizeof(mem_chunk_hdr_t))) {
    // split the chunk, adding the unneeded space back into the freelist in place of curr (don't prepend).  TODO - prepend instead?
    mem_chunk_hdr_t *new_chunk = (mem_chunk_hdr_t *)((char *)curr + sizeof(mem_chunk_hdr_t) + needed_size);
    new_chunk->size = excess_size - sizeof(mem_chunk_hdr_t);
    curr->size = needed_size;

    if (handle->freelist == curr) {   // if curr was first block in freelist
      handle->freelist = new_chunk;
    }
    else {
      prev->next = new_chunk;
    }
    new_chunk->next = curr->next;
  }

  return 1 + curr;    // return a pointer to the payload
}

/**
 * Naive realloc; simply allocates a new block of size 'size', memcpys the old contents over,
 * and frees the new block.
 */
void *prealloc(void *ptr, size_t size) {
  void *new_chunk = pmalloc(size);
  memcpy(new_chunk, ptr, ((mem_chunk_hdr_t *)ptr - 1)->size);   // TODO - make a macro to get size from payload ptr
  pfree(ptr)
;  return new_chunk;
}

// prepends the newly-freed block to the freelist.  basic; does not coalesce.
void pfree(void *ptr) {
  if (first_run) pheap_init();

  mem_chunk_hdr_t *newly_freed = (mem_chunk_hdr_t *)ptr - 1;
  newly_freed->next = handle->freelist;
  handle->freelist = newly_freed;
}

// returns the number of microseconds representing the (wall-clock) amount of
// time required for writeback to the backing file
size_t psync() {
  if (first_run) pheap_init();

  struct timeval before, after, elapsed;

  gettimeofday(&before, NULL);
  msync(handle, sizeof(pheap_t), MS_SYNC);
  gettimeofday(&after, NULL);

  timersub(&after, &before, &elapsed);

  // note that gettimeofday() measures *wall-clock* time...
  // should we assume that time required for disk write >> time for context switches, etc?
  // printf("%ju page(s) written back in %ld s, %ld μs\n", NUM_PAGES(size), elapsed.tv_sec, elapsed.tv_usec);
  return (1000000 * elapsed.tv_sec) + elapsed.tv_usec;
}