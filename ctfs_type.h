#ifndef CTFS_TYPE_H
#define CTFS_TYPE_H
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <linux/magic.h>
#include <linux/falloc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>
#include <x86intrin.h>
#include <stdarg.h> 

#include "ctfs_config.h"
#include "lib_dax.h"
#include "ctfs_util.h"
#ifdef __GNUC__
#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

/***********************************************
 * clear cache utility
 ***********************************************/
#define cache_wb_one(addr) _mm_clwb(addr)

#define cache_wb(target, size)              \
    for(size_t i=0; i<size; i+=64){         \
        _mm_clwb((void*)target + i);        \
    }                                       \
    _mm_clwb((void*)target + size - 1);


/* Convensions */
#define PAGE_MASK   (~((uint64_t)0x0fff))

typedef uint64_t index_t;
typedef uint64_t relptr_t;   // relative pointer
typedef int8_t pgg_level_t;

#define CT_ABS2REL(addr)    (((relptr_t)(addr)) - ct_rt.base_addr)
#define CT_REL2ABS(addr)    ((void*)((addr) + ct_rt.base_addr))


/* end of Convensions */

/******************************************
 * In-pmem structures 
 ******************************************/

/* inode. No need for block map
 * 128B. Provides same functionality as ext4

 * The following mask values are defined for the file type:

    S_IFMT     0170000   bit mask for the file type bit field
    S_IFSOCK   0140000   socket
    S_IFLNK    0120000   symbolic link
    S_IFREG    0100000   regular file
    S_IFBLK    0060000   block device
    S_IFDIR    0040000   directory
    S_IFCHR    0020000   character device
    S_IFIFO    0010000   FIFO
 */
struct ct_inode{
	// 64-bit fields
    ino_t       i_number;
    size_t      i_size;         // the size of the file
    relptr_t    i_block;        // the number of blocks allocated to the file
	size_t		i_ndirent;      // indicate the size of inode table
    nlink_t     i_nlink;        // number of hard links to the file

	// 32-bit fields
	uint32_t	i_lock;
    uid_t       i_uid;          // user ID of the owner of the file
    gid_t       i_gid;          // ID of the group owner of the file
    mode_t      i_mode;         // file type and mode
	// 16-bit fields
    pgg_level_t i_level;
    char        i_finish_swap;

    // times
    struct timespec i_atim; /* Time of last access */
    struct timespec i_mtim; /* Time of last modification */
    struct timespec i_ctim; /* Time of last status change */
    time_t          i_otim; /* Time of last open */

	// 8-bit fields
	

    /* Padding.
     * 128B - 120B = 8
     */
    char        padding[7];
};
typedef struct ct_inode ct_inode_t;
typedef ct_inode_t* ct_inode_pt;

struct ct_dirent{
    __ino64_t d_ino;
    __off64_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[CT_MAX_NAME + 1];
};
typedef struct ct_dirent ct_dirent_t;
typedef ct_dirent_t* ct_dirent_pt;

/******************************************
 * In-RAM structures 
 ******************************************/

struct hlist_node {

};

// the head of the hash table for locks
struct hlist_head {
	struct hlist_node *first;
};


/*
 * The global file_lock_list is only used for displaying /proc/locks, so we
 * keep a list on each CPU, with each list protected by its own spinlock.
 * Global serialization is done using file_rwsem.
 *
 * Note that alterations to the list also require that the relevant flc_lock is
 * held.
 * for more detail: https://elixir.bootlin.com/linux/v5.17/source/fs/locks.c#L124
 */

struct file_lock_list_struct {
	uint64_t lock;
	struct hlist_head hlist;
};

typedef void *fl_owner_t;

struct ct_list_head {
	struct ct_list_head *next, *prev;
};

//More details on file lock struct: https://elixir.bootlin.com/linux/v4.14.224/source/include/linux/fs.h#L1003
struct ct_file_lock {
    struct ct_file_lock *fl_next;   /* singly linked list for this inode  */
    struct ct_list_head fl_list;	/* link into file_lock_context */
    fl_owner_t fl_owner;            /* should be pointed to current file*/
    int fl_file;    /* it was a file pointer, however ctFS open() only returns a int*/
    unsigned int fl_flags;
	unsigned char fl_type;
	unsigned int fl_pid;
    unsigned int fl_start;          /* starting address of the range lock*/
    unsigned int fl_end;            /* ending address of the range lock*/
};

#endif