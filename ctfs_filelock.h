#ifndef CTFS_FILELOCK_H
#define CTFS_FILELOCK_H

#include "ctfs_runtime.h"
#include <pthread.h>

struct ct_fl_t;

/* block list and wait list segments */
struct ct_fl_seg{
    struct ct_fl_seg *prev;
    struct ct_fl_seg *next;
    struct ct_fl_t *addr;
};
typedef struct ct_fl_seg ct_fl_seg;

/* File lock */
struct ct_fl_t {
	struct ct_fl_t *fl_prev;
    struct ct_fl_t *fl_next;   		/* single liked list to other locks on this file */
	struct ct_fl_seg *fl_block; 		/* locks that is blocking this lock */
	struct ct_fl_seg *fl_wait; 		/* locks that is waiting for this lock*/

    int fl_fd;    					/*  Which fd has this lock*/
	volatile int fl_lock;			/* lock variable*/
	unsigned char fl_type;			/* type of the current lock*/
	unsigned int fl_pid;
    unsigned int fl_start;          /* starting address of the range lock*/
    unsigned int fl_end;            /* ending address of the range lock*/
};
typedef struct ct_fl_t ct_fl_t;

/************************************************ 
 * Implement atomic functions for file range lock
 ************************************************/
// TODO: defince range lock macros here, maybe use "__sync_fetch_and_add"
#define TEST_AND_SET(addr)                               \
__sync_lock_test_and_set(addr, 1)

#define TEST_AND_SET_RELEASE(addr)                       \
__sync_lock_release(addr)

#define FETCH_AND_INCREMENT(addr)                         \
__sync_fetch_and_add(addr, 1)

#define FETCH_AND_DECREMENT(addr)                         \
__sync_fetch_and_sub(addr, 1)

#define FENCE()                                          \
__sync_synchronize()

/************************************************ 
 * Implement actual range lock
 ************************************************/

ct_fl_t* ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...);

ct_fl_t* ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_release(ct_fl_t *node);

void ctfs_file_range_lock_release_all(int fd);

/************************************************ 
 * Implement range lock mechanism functions
 ************************************************/

ct_fl_t* ctfs_lock_list_add_node(int fd, off_t start, size_t n, int flag);

ct_fl_t* ctfs_lock_list_find_node(int fd, off_t start, size_t n, int flag);

void ctfs_lock_list_remove_node(ct_fl_t *node);

void print_all_info();

#endif