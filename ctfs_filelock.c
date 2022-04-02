/************************************************ 
 * ctfs_filelock.c
 * ctFS File Range Lock Implementation
 * 
 * Editor : Siyan Zhang
 *          Weixuan Yang
 *          Hongjian Zhu
 ************************************************/

#include "ctfs_filelock.h"

/* Range lock functions */

ct_fl_t* ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...){
    ct_fl_t *temp = ctfs_lock_list_add_node(fd, start, n, flag);
    while(temp->fl_block != NULL){
        FENCE();
    }
    return temp;
}

ct_fl_t* ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...){
    ct_fl_t *temp = ctfs_lock_list_add_node(fd, start, n, flag);
    if(temp->fl_block != NULL) return temp;
    else return NULL;
}

void ctfs_file_range_lock_release(int fd, ct_fl_t *node){
    ctfs_lock_list_remove_node(fd, node);
}

void ctfs_file_range_lock_release_all(int fd){
    for(ct_fl_t *temp = ct_rt.file_range_lock[fd]->fl_next; temp->fl_next != NULL; temp = temp->fl_next){
        free(temp);
    }
}