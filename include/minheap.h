/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 * Copyright (c) Author: Armon Dadgar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MINHEAP_H__
#define __MINHEAP_H__

/* Structure for a single heap entry */
typedef struct heap_entry {
    void *key;      /* Key for this entry */
    void *value;    /* Value for this entry */
} heap_entry;

/* Main struct for representing the heap 
 * map_pages is determined by the number of pointers needed to 
 * manage the pages where heap entries are stored. Since each 
 * pointer takes up space, the number of pages required to 
 * store all these pointers is given by map_pages.
 * 
 * minimum_pages defines the actual content storage area 
 * (i.e., the number of pages needed to store the heap entries). 
 * map_pages is generally a smaller number compared to minimum_pages, 
 * as the mapping table holds pointers, not the actual data.
 */
typedef struct heap {
    int (*compare_func)(void*, void*); /* The key comparison function to use */
    int active_entries;  /* The number of entries in the heap */
    int minimum_pages;   /* The minimum number of pages to maintain, based on the initial cap.*/
    int allocated_pages; /* The number of pages in memory that are allocated */
    int map_pages;       /* The number of pages used for the map table */
    void **mapping_table; /* Pointer to the table, which maps to the pages */
} heap;

/* Functions */

/**
 * Creates a new heap
 * @param h Pointer to a heap structure that is initialized
 * @param initial_size What should the initial size of the heap be. If <= 0, then it will be set to the minimum
 * permissable size, of 1 page (512 entries on 32bit system with 4K pages).
 * @param comp_func A pointer to a function that can be used to compare the keys. If NULL, it will be set
 * to a function which treats keys as signed ints. This function must take two keys, given as pointers and return an int.
 * It should return -1 if key 1 is smaller, 0 if they are equal, and 1 if key 2 is smaller.
 */
heap *heapCreate(int initial_size, int (*comp_func)(void*,void*));

/**
 * Destroys and cleans up a heap.
 * @param h The heap to destroy.
 */
void heapDestroy(heap *h);

/**
 * Returns the size of the heap
 * @param h Pointer to a heap structure
 * @return The number of entries in the heap.
 */
int heapSize(heap *h);

/**
 * Inserts a new element into a heap.
 * @param h The heap to insert into
 * @param key The key of the new entry
 * @param value The value of the new entry
 */
void heapInsert(heap *h, void *key, void *value);

/**
 * Returns the element with the smallest key in the heap.
 * @param h Pointer to the heap structure
 * @param key A pointer to a pointer, to set to the minimum key
 * @param value Set to the value corresponding with the key
 * @return 1 if the minimum element exists and is set, 0 if there are no elements.
 */
int heapMin(heap *h, void **key, void **value);

/**
 * Deletes the element with the smallest key from the heap.
 * @param h Pointer to the heap structure
 * @param key A pointer to a pointer, to set to the minimum key
 * @param valu Set to the value corresponding with the key
 * @return 1if the minimum element exists and is deleted, 0 if there are no elements.
 */
int heapDelMin(heap *h, void **key, void **value);

/**
 * Calls a function for each entry in the heap.
 * @param h The heap to iterate over
 * @param func The function to call on each entry. Should take a void* key and value.
 */
void heapForeach(heap *h, void (*func)(void*,void*));

#endif