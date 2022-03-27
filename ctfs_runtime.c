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

}

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...){

}

void ctfs_file_range_lock_release_all(int fd){

}

/************************************************ 
 * Implement linked list actions
 ************************************************/

inline void ctfs_lock_list_add_node(off_t start, size_t n, int flag){
    ct_fl_t *temp, *tail, *last;
    tail = head.fl_next;   //get the head of the lock list
    temp = (ct_fl_t*)malloc(sizeof(ct_fl_t));
    temp->fl_next = NULL;
    temp->fl_prev = NULL;
    temp->fl_start = start;
    temp->fl_end = start + n - 1;
    //TODO: get locklist here
    while(tail != NULL){
        //check if current list contains a lock that is not compatable
        if(check_overlap(tail, temp)){
            printf("Overlap found, abourt");
            //overlap found stop the process
            return -1;
        }
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
    free(node);
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
