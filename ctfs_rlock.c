/********************************
 * ctFS File Range Lock
 * ctfs_rlock.c
 * 
 * Editor: Siyan Zhang
 *         Weixuan Yang
 *         Hongjian Zhu
 *******************************/

#include "ctfs_rlock.h"

/* per lock list lock acquire */
inline void rl_lock_acquire(uint8_t* addr){
    while(__sync_lock_test_and_set((char*) ((uint64_t)addr), (int)1));
}

/* per lock list lock release */
inline void rl_lock_release(uint8_t* addr){
    __sync_lock_release((char*) ((uint64_t)addr));
}

/* check if two given range have conflicts */
inline int check_overlap(struct ct_fl_t *node1, struct ct_fl_t *node2){
    return ((node1->fl_start <= node2->fl_start) && (node1->fl_end >= node2->fl_start)) ||\
    ((node2->fl_start <= node1->fl_start) && (node2->fl_end >= node1->fl_start));
}

/* check if two given file access mode have conflicts */
inline int check_access_conflict(struct ct_fl_t *node1, struct ct_fl_t *node2){
    return !((node1->fl_flag == O_RDONLY) && (node2->fl_flag == O_RDONLY));
}

/* add the conflicted node into the head of the blocking list of the current node */
inline void ctfs_rlock_add_blocking(ct_fl_t *current, ct_fl_t *node){
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

/* add the current node to the wait list head of the conflicted node */
inline void ctfs_rlock_add_waiting(ct_fl_t *current, ct_fl_t *node){
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

/* remove the current node from others' blocking list */
inline void ctfs_rlock_remove_blocking(ct_fl_t *current){
    ct_fl_seg *temp, *temp1, *prev, *next;
    temp = current->fl_wait;
    while(temp != NULL){    // go through all node this is waiting for current node
        temp1 = temp->addr->fl_block;
        while(temp1 != NULL){   // go thorough the blocking list on other nodes to find itself
            if(temp1->addr == current){
                prev = temp1->prev;
                next = temp1->next;
                if(prev != NULL)
                    prev->next = next;
                else
                    temp->addr->fl_block = NULL;    // last one in the blocking list
                if(next != NULL)
                    next->prev = prev;
                free(temp1);
                break;
            }
            temp1 = temp1->next;
        }
#ifdef CTFS_DEBUG
        printf("\tNode %p removed from Node %p blocking list\n", current, temp);
#endif
        if(temp->next == NULL){ // last member in waiting list
            free(temp);
            current->fl_wait = NULL;
            break;
        }
        temp = temp->next;
        if(temp != NULL)    // if not the last member
            free(temp->prev);
    }
}

/* add a new node to the lock list upon the request(combined into lock_acq below) */
static inline ct_fl_t* ctfs_rlock_add_node(int fd, off_t start, size_t n, int flag){
    ct_fl_t *temp, *tail, *last;
    temp = (ct_fl_t*)malloc(sizeof(ct_fl_t));
    temp->fl_next = NULL;
    temp->fl_prev = NULL;
    temp->fl_block = NULL;
    temp->fl_wait = NULL;
    temp->fl_flag = flag;
    temp->fl_fd = fd;
    temp->fl_start = start;
    temp->fl_end = start + n - 1;
#ifdef CTFS_DEBUG
    temp->node_id = temp;
#endif

    rl_lock_acquire(&ct_fl.fl_lock[fd]);

    if(ct_fl.fl[fd] != NULL){
        tail = ct_fl.fl[fd];   // get the head of the lock list
        while(tail != NULL){ // check if current list contains a lock that is not compatable
            if(check_access_conflict(tail, temp)){
                if(check_overlap(tail, temp)){
                    ctfs_rlock_add_blocking(temp, tail); //add the conflicted lock into blocking list
#ifdef CTFS_DEBUG
                    printf("\tNode %p is blocking the Node %p\n", tail, temp);
#endif
                    ctfs_rlock_add_waiting(temp, tail); //add the new node to the waiting list of the conflicted node
#ifdef CTFS_DEBUG
                    printf("\tNode %p is waiting the Node %p\n", temp, tail);
#endif
                }
            }
            last = tail;
            tail = tail->fl_next;
        }
        temp->fl_prev = last;
        last->fl_next = temp;
    } else {
        ct_fl.fl[fd] = temp;
    }
#ifdef CTFS_DEBUG
    printf("Node %p added, Range: %u - %u, mode: %s\n", temp, temp->fl_start, temp->fl_end, enum_to_string(temp->fl_flag));
#endif

    rl_lock_release(&ct_fl.fl_lock[fd]);

    return temp;
}

/* remove a node from the lock list upon the request */
static inline void ctfs_rlock_remove_node(int fd, ct_fl_t *node){
    ct_fl_t *prev, *next;

    rl_lock_acquire(&ct_fl.fl_lock[fd]);

    prev = node->fl_prev;
    next = node->fl_next;
    if (prev == NULL){
        if(next == NULL)
            ct_fl.fl[fd] = NULL;    // last one member in the lock list;
        else{
            ct_fl.fl[fd] = next;    // delete the very first node in list
            next->fl_prev = NULL;
        }
    } else {
        prev->fl_next = next;
        if (next != NULL)
            next->fl_prev = prev;
    }
    ctfs_rlock_remove_blocking(node);
#ifdef CTFS_DEBUG
    printf("Node %p removed, Range: %u - %u, mode: %s\n", node, node->fl_start, node->fl_end, enum_to_string(node->fl_flag));
#endif

    rl_lock_release(&ct_fl.fl_lock[fd]);

    free(node);
}

/* initialization */
void ctfs_rlock_init(int fd){
    switch(fd){
        case 0: // all
            for (int i = 0; i < CT_MAX_FD; i++){
                ct_fl.fl[fd] = NULL;
                ct_fl.fl[fd] = 0;
            }
            break;
        default:
            ct_fl.fl[fd] = NULL;
            ct_fl.fl[fd] = 0;
    }
}

/* acquire a range lock, return the address of the lock */
ct_fl_t*  __attribute__((optimize("O0"))) ctfs_rlock_acquire(int fd, off_t offset, size_t count, int flag){
    ct_fl_t *node = ctfs_rlock_add_node(fd, offset, count, flag);
	while(node->fl_block != NULL){} // wait for blocker finshed
    return node;
}

/* release the range lock */
void ctfs_rlock_release(int fd, ct_fl_t *node){
    assert(node != NULL);
    ctfs_rlock_remove_node(fd, node);
}
