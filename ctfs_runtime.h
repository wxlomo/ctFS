#ifndef CTFS_RUNTIME_H
#define CTFS_RUNTIME_H

#include "ctfs_format.h"


/* File descriptor */
struct ct_fd_t{
	ct_inode_t     *inode;
	size_t      	offset;
	int         	flags;
	uint64_t		prefaulted_start;
	uint64_t		prefaulted_bytes;
	struct dirent	temp_dirent;
#ifdef CTFS_DEBUG
	uint64_t		cpy_time;
	uint64_t		pswap_time;
#endif
};
typedef struct ct_fd_t ct_fd_t;

struct ct_fl_t;

/* block list and wait list segments */
struct ct_fl_seg{
    struct ct_fl_seg *prev;
    struct ct_fl_seg *next;
    struct ct_fl_t *addr;
};
typedef struct ct_fl_seg ct_fl_seg;

/* File lock */
struct ct_fl_t {
	struct ct_fl_t *fl_prev;
    struct ct_fl_t *fl_next;   		/* single liked list to other locks on this file */
	struct ct_fl_seg fl_block; 		/* locks that is blocking this lock */
	struct ct_fl_seg fl_wait; 		/* locks that is waiting for this lock*/

    int fl_fd;    					/*  Which fd has this lock*/
	volatile int fl_lock;			/* lock variable*/
	unsigned char fl_type;			/* type of the current lock*/
	unsigned int fl_pid;
    unsigned int fl_start;          /* starting address of the range lock*/
    unsigned int fl_end;            /* ending address of the range lock*/
};
typedef struct ct_fl_t ct_fl_t;

/* end of in-RAM structures */
struct failsafe_frame;

struct ct_runtime{
	uint64_t            base_addr;
	ct_super_blk_pt     super_blk;
	ct_alloc_prot_pt    alloc_prot;
	void*               lvl9_bmp;
	pgg_hd_group_pt     first_pgg;
	size_t              active_write;
	time_t				starting_time;
	int					errorn;
//64 B

	// inode
	void*               inode_bmp;
	ct_inode_pt         inode_start;
	char 				indoe_bmp_lock_padding[48];
	ctfs_lock_t         inode_bmp_lock;
	char 				indoe_bmp_lock_padding_[60];
	uint64_t			inode_rt_lock[CT_INODE_BITLOCK_SLOTS / 64];
	uint64_t			inode_rw_lock[CT_INODE_RW_SLOTS / 64];

	// current dir
	ct_inode_pt			current_dir;
	char				current_path[16384];

	// fd
	ct_fd_t             fd[CT_MAX_FD];
	char 				open_lock_padding[56];
	ctfs_lock_t			open_lock;
	char 				open_lock_padding_[60];
	//TODO: implement range lock list here
	ct_fl_t*			file_range_lock[CT_MAX_FD];
	//one entry per inode

	// ppg lock
	uint64_t			pgg_lock;
	char				pgg_lock_padding[56];
	// failsafe
	uint64_t			failsafe_clock;
	struct failsafe_frame* failsafe_frame;

	char				mpk[3];
};
typedef struct ct_runtime ct_runtime_t;
// int a = sizeof(ct_runtime_t);
extern ct_runtime_t ct_rt;

/* Inode frame
 * used for inode related functions
 */
struct ct_inode_frame{
	// starting inode. Ignored if path starts with '/'
	ct_inode_pt		inode_start;
	// string of the path
	const char 		*path;
	// the target's inode returned
	ct_inode_pt		current;
	// target's parent inode returned
	ct_inode_pt		parent;
	// parent's dirent returned where the current locates
	ct_dirent_pt	dirent;
	// the mode provided for last level if request for create
	mode_t			i_mode;
	// flags. Both in and out. 
	uint16_t		flag;
};
typedef struct ct_inode_frame ct_inode_frame_t;

#define CT_INODE_FRAME_PARENT				0b00001		// request parent returned
#define CT_INODE_FRAME_CREATE				0b00010		// request create if non-exist
#define CT_INODE_FRAME_SAME_INODE_LOCK		0b00100		// set if current and parent share the same lock
#define CT_INODE_FRAME_INSTALL				0b01000		// install the current inode to the given location. No lock held when return.

#define ct_super ct_rt.super_blk

/************************************************ 
 * Implement atomic functions for file range lock
 ************************************************/
// TODO: defince range lock macros here, maybe use "__sync_fetch_and_add"
#define TEST_AND_SET(addr)                               \
__sync_lock_test_and_set(addr, 1)

#define TEST_AND_SET_RELEASE(addr)                       \
__sync_lock_release(addr)

#define FETCH_AND_INCREMENT(addr)                         \
__sync_fetch_and_add(addr, 1)

#define FETCH_AND_DECREMENT(addr)                         \
__sync_fetch_and_sub(addr, 1)

#define FENCE()                                          \
__sync_synchronize()

/* core help functions */
// inode related
void inode_rt_lock(index_t inode_n);
void inode_rt_unlock(index_t inode_n);
void inode_rw_lock(index_t inode_n);
void inode_rw_unlock(index_t inode_n);
void inode_wb(ct_inode_pt inode);
index_t inode_alloc();
void inode_dealloc(index_t index);
void inode_set_root();
int inode_path2inode(ct_inode_frame_t * frame);
int inode_resize(ct_inode_pt inode, size_t size);

void ct_time_stamp(struct timespec * time);
int ct_time_greater(struct timespec * time1, struct timespec * time2);
static inline long calc_diff(struct timespec start, struct timespec end){
    return (end.tv_sec * (long)(1000000000) + end.tv_nsec) -
    (start.tv_sec * (long)(1000000000) + start.tv_nsec);
};
void timer_start();
uint64_t timer_end();

/************************************************ 
 * Implement file range lock
 ************************************************/

void ctfs_file_range_lock_init();

void ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_release_all(int fd);

#endif