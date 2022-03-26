#ifndef CTFS_UTIL_H
#define CTFS_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <x86intrin.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <inttypes.h>

/************************************************ 
 * Implement utility functions for synchronization
 ************************************************/
#define FETCH_AND_SET_BIT(addr, num)                         \
__sync_fetch_and_or((char*) (((uint64_t)addr)+(num/8)),      \
((char)0x01 << (num%8)))

#define FETCH_AND_unSET_BIT(addr, num)                        \
__sync_and_and_fetch((char*) (((uint64_t)addr)+(num/8)),      \
~((char)0x01 << (num%8))) 

typedef volatile int ctfs_lock_t;

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

//NOTE: the current open lock is implemented with the following spain locks below
#define ctfs_lock_try(lock)     pthread_spin_trylock(&lock)
#define ctfs_lock_acquire(lock) pthread_spin_lock(&lock)
#define ctfs_lock_release(lock) pthread_spin_unlock(&lock)
#define ctfs_lock_init(lock) pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE)

// void bit_lock_acquire(uint64_t *addr, uint64_t num);
// void bit_lock_release(uint64_t *addr, uint64_t num);


/************************************************ 
 * Implement utility functions for bitmap
 ************************************************/
#define BITS_PER_BYTE  8
#define ARRAY_INDEX(b) ((b) / BITS_PER_BYTE) // Given the block number, calculate the index to the uint array
#define BIT_OFFSET(b)  ((b) % BITS_PER_BYTE) // Calculate the bit position within the located uint element

void set_bit (uint64_t *bitmap, size_t b);

void clear_bit (uint64_t *bitmap, size_t b);

int get_bit (uint64_t *bitmap, size_t b);

int64_t find_free_bit_tiny(uint64_t *bitmap, size_t size);

int64_t find_free_bit (uint64_t *bitmap, size_t size, size_t hint);

void bitlock_acquire(uint64_t *bitlock, uint64_t location);

int bitlock_try_acquire(uint32_t *bitlock, uint32_t bit, uint32_t tries);

void bitlock_release(uint64_t *bitlock, uint64_t location);

void avx_cpy(void *dest, const void *src, size_t size);

void avx_cpyt(void *dest, void *src, size_t size);

// void big_memcpy(void *dest, const void *src, size_t n);

/************************************************ 
 * Implement file range lock
 ************************************************/

void ctfs_file_range_lock_init();

void ctfs_file_range_lock_acquire(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_try_acquire(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_release(int fd, off_t start, size_t n, int flag, ...);

void ctfs_file_range_lock_release_all(int fd);

/************************************************ 
 * Implement read and write lock
 ************************************************/

inline void ctfs_read_lock_acquire(ct_fl_t *lock);

inline void ctfs_write_lock_acquire(ct_fl_t *lock);

inline void ctfs_read_lock_release(ct_fl_t *lock);

inline void ctfs_write_lock_acquire(ct_fl_t *lock);

/***********************************************
 * Debug
 ***********************************************/
// #define CTFS_DEBUG 2
#ifdef CTFS_DEBUG
extern uint64_t ctfs_debug_temp;
#endif

#endif //CTFS_UTIL_H
