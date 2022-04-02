//
// Created by Ding Yuan on 2018-01-15.
//

#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>

#include "ctfs_type.h"

#define FL_READ 0
#define FL_WRITE 1


inline void bit_lock_acquire(uint64_t *addr, uint64_t num){
    while(FETCH_AND_SET_BIT(addr, num) & ((char)0x01 << num%8)){
        //spinlock
    }
}

inline void bit_lock_release(uint64_t *addr, uint64_t num){
    FETCH_AND_unSET_BIT(addr, num);
}

// inline void ctfs_read_lock_acquire(ct_fl_t *lock){
//     for(;;){
//         while(lock->fl_wcount){  /* write lock acquired */
//             FENCE();
//         }
//         FETCH_AND_INCREMENT(&lock->fl_rcount);
//         if(lock->fl_wcount){  /* high priority write */
//             FETCH_AND_DECREMENT(&lock->fl_rcount);  /* restore if new write comes in */
//         }
//         else break;
//     }
// }

// inline void ctfs_write_lock_acquire(ct_fl_t *lock){
//     while(TEST_AND_SET(&lock->fl_wcount));
//     while(lock->fl_rcount){
//         FENCE();
//     }
// }

// inline void ctfs_read_lock_release(ct_fl_t *lock){
//     FETCH_AND_DECREMENT(&lock->fl_rcount);
// }

// inline void ctfs_write_lock_release(ct_fl_t *lock){
//     TEST_AND_SET_RELEASE(&lock->fl_wcount);
// }