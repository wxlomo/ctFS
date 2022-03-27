/********************************
 * 
 * 
 * 
 *******************************/

#include "ctfs_runtime.h"

#ifdef CTFS_DEBUG
uint64_t ctfs_debug_temp;
#endif

ct_runtime_t ct_rt;


struct timespec stopwatch_start;
struct timespec stopwatch_stop;

void timer_start(){
	clock_gettime(CLOCK_MONOTONIC, &stopwatch_start);
}

uint64_t timer_end(){
	clock_gettime(CLOCK_MONOTONIC, &stopwatch_stop);
	return calc_diff(stopwatch_start, stopwatch_stop);
}


ct_runtime_t* get_rt(){
    return &ct_rt;
}

void ct_time_stamp(struct timespec * time){
    clock_gettime(CLOCK_REALTIME, time);
}

int ct_time_greater(struct timespec * time1, struct timespec * time2){
    if(time1->tv_sec > time2->tv_sec){
        return 1;
    }
    else if(time1->tv_sec < time2->tv_sec){
        return 0;
    }
    else if(time1->tv_nsec > time2->tv_nsec){
        return 1;
    }
    else
        return 0;
}

/************************************************ 
 * Implement file range lock
 ************************************************/
// TODO: implement lock funchtions here, maybe use "__sync_fetch_and_add"
void ctfs_file_range_lock_init(){

}

void ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...){
    while(TEST_AND_SET(&ct_rt.file_range_lock[fd]->fl_lock));
    ct_fl_t *temp = ctfs_lock_list_add_node(start, n, flag);
    // TODO: ctfs_block_list_update(temp);
    // TODO: ctfs_wait_list_add_to_all(temp);
    TEST_AND_SET_RELEASE(&ct_rt.file_range_lock[fd]->fl_lock);
    while(!ctfs_block_list_is_empty(temp)){
        FENCE();
    }
}

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...){
    while(TEST_AND_SET(&ct_rt.file_range_lock[fd]->fl_lock));
    ct_fl_t *temp = ctfs_lock_list_remove_node(start, n, flag);
    // TODO: ctfs_lock_list_remove_from_all(temp);
    free(temp);
    TEST_AND_SET_RELEASE(&ct_rt.file_range_lock[fd]->fl_lock);
}

void ctfs_file_range_lock_release_all(int fd){
    for(ct_fl_t *temp = ct_rt.file_range_lock[fd]->fl_next; temp->fl_next != NULL; temp = temp->fl_next){
        free(temp->fl)
    }
}

/************************************************ 
 * Implement linked list actions
 ************************************************/

inline int check_overlap(struct ct_fl_t *lock1, struct ct_fl_t *lock2){
    return ((lock1->fl_start <= lock2->fl_start) && (lock1->fl_end >= lock2->fl_start)) ||\
    ((lock2->fl_start <= lock1->fl_start) && (lock2->fl_end >= lock1->fl_start));
}

inline void ctfs_lock_list_add_node(off_t start, size_t n, int flag){
    ct_fl_t *temp, *tail, *last;
    tail = head.fl_next;   //get the head of the lock list
    temp = (ct_fl_t*)malloc(sizeof(ct_fl_t));
    temp->fl_next = NULL;
    temp->fl_prev = NULL;
    temp->fl_start = start;
    temp->fl_end = start + n - 1;
    while(tail != NULL){
        last = tail;
        tail = tail->fl_next;    
    }
    temp->fl_prev = last;
    last->fl_next = temp;
}

inline void ctfs_lock_list_remove_node(ct_fl_t *node){
    ct_fl_t *prev, *next;
    prev = node->fl_prev;
    next = node->fl_next;
    prev->fl_next = next;

    if (next != NULL)
        next->fl_prev = prev;
}

/************************************************ 
 * Implement read and write lock
 ************************************************/

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
