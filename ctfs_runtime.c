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
 * Implement read and write lock
 ************************************************/

void ctfs_read_lock_acquire(ct_fl_t *lock){
    for(;;){
        while(lock->fl_wcount){  /* write lock acquired */
            FENCE();
        }
        FETCH_AND_INCREMENT(&lock->fl_rcount);
        if(lock->fl_wcount){  /* high priority write */
            FETCH_AND_DECREMENT(&lock->fl_rcount);  /* restore if new write comes in */
        }
        else break;
    }
}

void ctfs_write_lock_acquire(ct_fl_t *lock){
    while(TEST_AND_SET(&lock->fl_wcount));
    while(lock->fl_rcount){
        FENCE();
    }
}

void ctfs_read_lock_release(ct_fl_t *lock){
    FETCH_AND_DECREMENT(&lock->fl_rcount);
}

void ctfs_write_lock_release(ct_fl_t *lock){
    TEST_AND_SET_RELEASE(&lock->fl_wcount);
}
