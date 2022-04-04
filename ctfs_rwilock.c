/********************************
 * ctFS inode RW Lock
 * ctfs_rwilock.c
 * 
 * Editor: Weixuan Yang
 *******************************/

#include "ctfs_filelock.h"

/* acquire the inode read lock */
inline void ctfs_ilock_read_acquire(index_t n){
    for(;;){
        while(ct_fl.il_lock[n]->il_wcount){
            __sync_synchronize();
        }
        __sync_fetch_and_add((char*)(&ct_fl.il_lock[n]->il_rcount), (int)1);
        if(ct_fl.il_lock[n]->il_wcount){
            __sync_fetch_and_sub((char*)(&ct_fl.il_lock[n]->il_rcount), (int)1);
        }
        else break;
    }
}

/* acquire the inode write lock */
inline void ctfs_ilock_write_acquire(index_t n){
    while(__sync_lock_test_and_set((char*)(&ct_fl.il_lock[n]->il_wcount), (int)1));
    while(ct_fl.il_lock[n]->il_rcount){
        __sync_synchronize();
    }
}

/* release the inode read lock */
inline void ctfs_ilock_read_release(index_t n){
    __sync_fetch_and_sub((char*)(&ct_fl.il_lock[n]->il_rcount), (int)1);
}

/* release the inode write lock */
inline void ctfs_ilock_write_release(index_t n){
    __sync_lock_release((char*)(&ct_fl.il_lock[n]->il_wcount), (int)1);
}

/* acquire the inode lock */
void ctfs_ilock_acquire(index_t inode_n, int flag){
    switch(flag){
        case O_RDONLY:
            ctfs_ilock_read_acquire(inode_n);
            break;
        case O_WRONLY:
            ctfs_ilock_write_acquire(inode_n);
            break;
    }
}

/* release the inode read lock */
void ctfs_ilock_release(index_t inode_n, int flag){
    switch(flag){
        case O_RDONLY:
            ctfs_ilock_read_release(inode_n);
            break;
        case O_WRONLY:
            ctfs_ilock_write_release(inode_n);
            break;
    }
}