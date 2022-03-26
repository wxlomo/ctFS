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


inline void bit_lock_acquire(uint64_t *addr, uint64_t num){
    while(FETCH_AND_SET_BIT(addr, num) & ((char)0x01 << num%8)){
        //spinlock
    }
}

inline void bit_lock_release(uint64_t *addr, uint64_t num){
    FETCH_AND_unSET_BIT(addr, num);
}

/************************************************ 
 * Implement file range lock
 ************************************************/
// TODO: implement lock funchtions here, maybe use "__sync_fetch_and_add"
void ctfs_file_range_lock_init(){

}

/************************************************
Inputs: fd, starting position, size, lock type
Output: 0 if acquire the lock, error elsewise
*************************************************/
void ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...){
    printf(fd);
    // Range parameters already checked in pread/pwrite

    switch (flag) {

        case 0: // when it is requiring an read lock
            while (ct_rt.file_range_lock[fd]->fl_next != NULL) {
                
            }

        case 1: // when it is requiring an write lock

    }



}

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release_all(int fd){

}