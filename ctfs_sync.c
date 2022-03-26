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
#include "ctfs_runtime.h"


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

int check_overlap(struct ct_fl_t *lock1, struct ct_fl_t *lock2){
    return ((lock1->fl_start <= lock2->fl_start) && (lock1->fl_end >= lock2->fl_start)) ||\
    ((lock2->fl_start <= lock1->fl_start) && (lock2->fl_end >= lock1->fl_start));
}

// TODO: implement lock funchtions here, maybe use "__sync_fetch_and_add"
void ctfs_file_range_lock_init(){

}

//add a new node to the lock list upon the request
void ctfs_lock_list_add_node(int fd, off_t start, size_t n, int flag){
    ct_fl_t *temp, *tail;
    tail = ct_rt.file_range_lock[fd];   //get the head of the lock list
    temp = (ct_fl_t*)malloc(sizeof(ct_fl_t));
    temp->fl_next = NULL;
    temp->fl_count = 1;
    temp->fl_start = start;
    temp->fl_end = start + n - 1;
    temp->fl_flags = flag;
    //TODO: get lock list here
    while(tail != NULL){
        //check if current list contains a lock that is not compatable
        if(check_overlap(tail, temp)){
            //overlap found stop the process
            return -1;
        }
        tail = tail->fl_next;    
    }
    tail->fl_next = temp;
    //TODO: release locklist here

}

void ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release_all(int fd){

}