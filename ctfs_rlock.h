/********************************
 * ctFS File Range Lock
 * ctfs_rlock.h
 * 
 * Editor: Siyan Zhang
 *         Weixuan Yang
 *         Hongjian Zhu
 *******************************/

#ifndef CTFS_RLOCK_H
#define CTFS_RLOCK_H

#include "ctfs_format.h"

/* block list and wait list segments */
typedef struct ct_fl_seg{
    struct ct_fl_seg  *prev;
    struct ct_fl_seg  *next;
    struct ct_fl_t    *addr;
}ct_fl_seg;

/* File lock */
typedef struct ct_fl_t{
	struct ct_fl_t    *fl_prev;
    struct ct_fl_t    *fl_next;            // doubly linked list to other locks on this file
	ct_fl_seg         *fl_block; 	       // locks that is blocking this lock
	ct_fl_seg         *fl_wait; 		   // locks that is waiting for this lock
	int                fl_fd;    	       // Which fd has this lock
	unsigned char      fl_type;		       // type of the current lock: O_RDONLY, O_WRONLY, or O_RDWR
	unsigned int       fl_pid;
    unsigned int       fl_start;           // starting address of the range lock
    unsigned int       fl_end;             // ending address of the range lock
    struct ct_fl_t    *node_id;            // For Debug Only
}ct_fl_t;

/* File lock frame */
typedef struct ct_fl_frame{
    ct_fl_t*           fl[CT_MAX_FD];	   // one list per opened file
	uint8_t           fl_lock[CT_MAX_FD]; // one lock per list
}ct_fl_frame;
ct_fl_frame ct_fl;

/* range lock related functions */
void      ctfs_rlock_init(int fd);                                          // initialization
ct_fl_t*  ctfs_rlock_acquire(int fd, off_t offset, size_t count, int flag); // acquire a range lock, return the address of the lock
void      ctfs_rlock_release(int fd, ct_fl_t *node);                        // release the range lock

#endif