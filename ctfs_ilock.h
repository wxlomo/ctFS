/********************************
 * ctFS inode RW Lock 
 * (testing, not included)
 * ctfs_ilock.h
 * 
 * Editor: Weixuan Yang
 *******************************/

#ifndef CTFS_ILOCK_H
#define CTFS_ILOCK_H

#include "ctfs_filelock.h"

/* inode lock */
typedef struct ct_il_t{
    int il_rcount;
    int il_wcount;
}ct_il_t;

typedef struct ct_il_frame{
    ct_il_t*           il_lock[CT_INODE_RW_SLOTS]; // one lock per inode
}ct_il_frame;
ct_il_frame ct_il;

/* inode rw lock related functions */
void ctfs_ilock_acquire(index_t inode_n, int flag);   // acquire the inode lock
void ctfs_ilock_release(index_t inode_n, int flag);   // release the inode lock

#endif