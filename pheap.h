#ifndef PHEAP_H_INCLUDED
#define PHEAP_H_INCLUDED

// desired heap size in bytes, (including chunk hdr metadata).  Actual heap size
// may be slightly larger due to imperfect chunk alignment (will not be smaller)
#define DESIRED_HEAP_SIZE 5242880
#define DESIRED_HANDLE 0x7fdeaceea000     // desired heap entry handle.  If null, the kernel chooses a handle upon first run
#define BACKING_FILE "./backing_file"
#define MAGIC_NUMBER 0xbaadbaadcafef00d

// exposed so that the client can incur the (possibly large) setup overhead whenever convenient
// client NEEDS to call pheap_init before accessing any memory that was previously allocated in the
// persistent heap.  We recommend that the client call pheap_init *every time* at the start of his/her program
extern void pheap_init();

extern void *pmalloc(size_t size);

extern void *prealloc(void *ptr, size_t size);

extern void pfree(void *ptr);

extern size_t psync();

#endif
