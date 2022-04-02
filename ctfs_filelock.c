/************************************************ 
 * ctfs_filelock.c
 * ctFS File Range Lock Implementation
 * 
 * Editor : Siyan Zhang
 *          Weixuan Yang
 *          Hongjian Zhu
 ************************************************/

#include "ctfs_filelock.h"

/* Link list functions */

static inline int check_overlap(struct ct_fl_t *lock1, struct ct_fl_t *lock2){
    return ((lock1->fl_start <= lock2->fl_start) && (lock1->fl_end >= lock2->fl_start)) ||\
    ((lock2->fl_start <= lock1->fl_start) && (lock2->fl_end >= lock1->fl_start));
}

static inline int check_access_conflict(struct ct_fl_t *node1, struct ct_fl_t *node2){
    /* check if two given file access mode have conflicts */
    return !((node1->fl_type == O_RDONLY) && (node2->fl_type == O_RDONLY));
}

static inline void ctfs_lock_add_blocking(ct_fl_t *current, ct_fl_t *node){
    /* add the conflicted node into the head of the blocking list of the current node*/
    ct_fl_seg *temp;
    temp = (ct_fl_seg*)malloc(sizeof(ct_fl_seg));
    temp->prev = NULL;
    temp->next = current->fl_block;
    temp->addr = node;

    if(current->fl_block != NULL){
        current->fl_block->prev = temp;
    }
    current->fl_block = temp;
}

static inline void ctfs_lock_add_waiting(ct_fl_t *current, ct_fl_t *node){
    /*add the current node to the wait list head of the conflicted node*/
    ct_fl_seg *temp;
    temp = (ct_fl_seg*)malloc(sizeof(ct_fl_seg));
    temp->prev = NULL;
    temp->next = node->fl_wait;
    temp->addr = current;
    if(node->fl_wait != NULL){
        node->fl_wait->prev = temp;
    }
    node->fl_wait = temp;
}

static inline void ctfs_lock_remove_blocking(ct_fl_t *current){
    /* remove the current node from others' blocking list*/
    ct_fl_seg *temp, *temp1, *prev, *next;
    temp = current->fl_wait;
    while(temp != NULL){    //go through all node this is waiting for current node
        temp1 = temp->addr->fl_block;
        while(temp1 != NULL){   //go thorough the blocking list on other nodes to find itself
            //compare range and mode
            //if((temp1->addr->fl_start == current->fl_start) && (temp1->addr->fl_end == current->fl_end) && (temp1->addr->fl_type == current->fl_type)){
            //or simply compares the address
            if(temp1->addr == current){
                prev = temp1->prev;
                next = temp1->next;
                if(prev != NULL)
                    prev->next = next;
                else
                    temp->addr->fl_block = NULL;    //last one in the blocking list
                if(next != NULL)
                    next->prev = prev;
                free(temp1);
                break;
            }
            temp1 = temp1->next;
        }
        if(temp->next == NULL){ //last member in waiting list
            free(temp);
            current->fl_wait = NULL;
            break;
        }
        temp = temp->next;
        if(temp != NULL)    //if not the last member
            free(temp->prev);
    }
}

static inline ct_fl_t* ctfs_lock_list_add_node(int fd, off_t start, size_t n, int flag){
    /* add a new node to the lock list upon the request(combined into lock_acq below) */
    ct_fl_t *temp, *tail, *last;
    temp = (ct_fl_t*)malloc(sizeof(ct_fl_t));
    temp->fl_next = NULL;
    temp->fl_prev = NULL;
    temp->fl_block = NULL;
    temp->fl_wait = NULL;
    temp->fl_type = flag;
    temp->fl_fd = fd;
    temp->fl_start = start;
    temp->fl_end = start + n - 1;

    while(TEST_AND_SET(&ct_rt.file_range_lock[fd]->fl_lock));

    if(ct_rt.file_range_lock[fd] != NULL){
        tail = ct_rt.file_range_lock[fd];   //get the head of the lock list
        while(tail != NULL){
            //check if current list contains a lock that is not compatable
            if(check_overlap(tail, temp) && check_access_conflict(tail, temp)){
                ctfs_lock_add_blocking(temp, tail); //add the conflicted lock into blocking list
                ctfs_lock_add_waiting(temp, tail); //add the new node to the waiting list of the conflicted node
            }
            last = tail;
            tail = tail->fl_next;
        }
        temp->fl_prev = last;
        last->fl_next = temp;
    } else {
        ct_rt.file_range_lock[fd] = temp;
    }
   
    TEST_AND_SET_RELEASE(&ct_rt.file_range_lock[fd]->fl_lock);

    return temp;
}

static inline void ctfs_lock_list_remove_node(int fd, ct_fl_t *node){
    /* remove a node from the lock list upon the request */
    ct_fl_t *prev, *next;

    while(TEST_AND_SET(&ct_rt.file_range_lock[fd]->fl_lock));

    prev = node->fl_prev;
    next = node->fl_next;
    if (prev == NULL){
        if(next == NULL)
            ct_rt.file_range_lock[fd] = NULL;    //last one member in the lock list;
        else{
            ct_rt.file_range_lock[fd] = next;    //delete the very first node in list
            next->fl_prev = NULL;
        }
    } else {
        prev->fl_next = next;
        if (next != NULL)
            next->fl_prev = prev;
    }
    ctfs_lock_remove_blocking(node);

    TEST_AND_SET_RELEASE(&ct_rt.file_range_lock[fd]->fl_lock);

    free(node);
}

/* Range lock functions */

void ctfs_file_range_lock_init_all(){
    for(unsigned int fd = 0; fd < CT_MAX_FD; fd++){
        ct_rt.file_range_lock[fd] = NULL;
    }
}

ct_fl_t* ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag){
    ct_fl_t *temp = ctfs_lock_list_add_node(fd, start, n, flag);
    while(temp->fl_block != NULL);
    return temp;
}

ct_fl_t* ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag){
    ct_fl_t *temp = ctfs_lock_list_add_node(fd, start, n, flag);
    if(temp->fl_block == NULL) return temp;
    else return NULL;
}

int ctfs_file_range_lock_release(int fd, ct_fl_t *node){
    if(node == NULL) return 0;
    ctfs_lock_list_remove_node(fd, node);
    return 1;
}

void ctfs_file_range_lock_release_all(int fd){
    for(ct_fl_t *temp = ct_rt.file_range_lock[fd]->fl_next; temp->fl_next != NULL; temp = temp->fl_next){
        free(temp);
    }
    ct_rt.file_range_lock[fd] = NULL;
}