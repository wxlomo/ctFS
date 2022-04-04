/********************************
 * ctFS File Range Lock
 * ctfs_filelock.h
 * 
 * Editor: Siyan Zhang
 *         Weixuan Yang
 *         Hongjian Zhu
 *******************************/

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
    struct ct_fl_t *fl_next;   		// doubly linked list to other locks on this file
	struct ct_fl_seg *fl_block; 	// locks that is blocking this lock
	struct ct_fl_seg *fl_wait; 		// locks that is waiting for this lock
	int fl_fd;    					// Which fd has this lock
	unsigned char fl_type;			// type of the current lock: O_RDONLY, O_WRONLY, or O_RDWR
	unsigned int fl_pid;
    unsigned int fl_start;          // starting address of the range lock
    unsigned int fl_end;            // ending address of the range lock
    struct ct_fl_t *node_id;        // For Debug Only
};
typedef struct ct_fl_t ct_fl_t;

/*range lock related functions*/
void ctfs_lock_list_init(int fd);                                        // initialization
ct_fl_t*  ctfs_rlock_lock(int fd, off_t offset, size_t count, int flag); // acquire a range lock, return the address of the lock
void ctfs_rlock_unlock(int fd, ct_fl_t *node);                           // release the range lock

#endif