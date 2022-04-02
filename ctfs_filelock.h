/************************************************ 
 * ctfs_filelock.h
 * ctFS File Range Lock Implementation
 * 
 * Editor : Siyan Zhang
 *          Weixuan Yang
 *          Hongjian Zhu
 ************************************************/

#ifndef CTFS_FILELOCK_H
#define CTFS_FILELOCK_H

#include "ctfs_runtime.h"

/* Block list and wait list segments */
typedef struct ct_fl_seg{
    struct ct_fl_seg *prev;
    struct ct_fl_seg *next;
    struct ct_fl_t *addr;
} ct_fl_seg;

/* File lock */
typedef struct ct_fl_t {
	struct ct_fl_t *fl_prev;
    struct ct_fl_t *fl_next;   		// single liked list to other locks on this file
	struct ct_fl_seg *fl_block; 	// locks that is blocking this lock
	struct ct_fl_seg *fl_wait; 		// locks that is waiting for this lock

    int fl_fd;    					// Which fd has this lock
	volatile int fl_lock;			// lock variable
	unsigned char fl_type;			// type of the current lock
	unsigned int fl_pid;
    unsigned int fl_start;          // starting address of the range lock
    unsigned int fl_end;            // ending address of the range lock
} ct_fl_t;

/* Atomic functions */
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

/* Range lock functions */

void ctfs_file_range_lock_init_all();

ct_fl_t* ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag);

ct_fl_t* ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag);

int ctfs_file_range_lock_release(int fd, ct_fl_t *node);

void ctfs_file_range_lock_release_all(int fd);

#endif