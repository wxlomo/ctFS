#ifndef CTFS_FILELOCK_H
#define CTFS_FILELOCK_H

#include "ctfs_format.h"

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

	volatile int fl_lock;			/* lock itself*/

	int fl_fd;    					/* Which fd has this lock */
	unsigned char fl_type;			/* type of the current lock: O_RDONLY, O_WRONLY, or O_RDWR */
	unsigned int fl_pid;
    unsigned int fl_start;          /* starting address of the range lock*/
    unsigned int fl_end;            /* ending address of the range lock*/
    struct ct_fl_t *node_id;        /* For Debug Only */
};
typedef struct ct_fl_t ct_fl_t;

#define TEST_AND_SET(addr)                         \
__sync_lock_test_and_set((char*) ((uint64_t)addr), (int)1)
#define TEST_AND_SET_RELEASE(addr)                 \
__sync_lock_release((char*) ((uint64_t)addr))

/*range lock related functions*/
void ctfs_lock_add_blocking(ct_fl_t *current, ct_fl_t *node);
void ctfs_lock_add_waiting(ct_fl_t *current, ct_fl_t *node);
void ctfs_lock_remove_blocking(ct_fl_t *current);
int check_overlap(ct_fl_t *node1, ct_fl_t *node2);
int check_access_conflict(ct_fl_t *node1, ct_fl_t *node2);
ct_fl_t* ctfs_lock_list_add_node(int fd, off_t start, size_t n, int flag);
void ctfs_lock_list_remove_node(int fd, ct_fl_t *node);
void ctfs_lock_list_init();
ct_fl_t*  ctfs_rlock_lock(int fd, off_t offset, size_t count, int flag);
void ctfs_rlock_unlock(int fd, ct_fl_t *node);
char* enum_to_string(int mode);		//for debugging only

#endif