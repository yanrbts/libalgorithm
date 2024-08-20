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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <strings.h>
#include <zmalloc.h>
#include <minheap.h>

#define LEFT_CHILD(i)   ((i<<1)+1)
#define RIGHT_CHILD(i)  ((i<<1)+2)
#define PARENT_ENTRY(i) ((i-1)>>1)
#define SWAP_ENTRIES(parent, child)  do {   \
    void* temp_key = (parent)->key;         \
    (parent)->key = (child)->key;           \
    (child)->key = temp_key;                \
                                            \
    void* temp_value = (parent)->value;     \
    (parent)->value = (child)->value;       \
    (child)->value = temp_value;            \
} while (0)
#define GET_ENTRY(index, map_table) (((heap_entry*)*(map_table + index/ENTRIES_PER_PAGE))+(index % ENTRIES_PER_PAGE))

/* Stores the number of heap_entry structures
 * we can fit into a single page of memory.
 *
 * This is determined by the page size, so we
 * need to determine this at run time.
 * 
 */
#define PAGE_SIZE           getpagesize() /* Ensure PAGE_SIZE is always initialized, 
                                           * Stores the number of bytes in a single page of memory. */
#define ENTRIES_PER_PAGE    (PAGE_SIZE / sizeof(heap_entry))

/* Helper function to map a number of pages into memory
 * Returns NULL on error, otherwise returns a pointer to the
 * first page.*/
static void *mapInPages(int page_count) {
    if (page_count == 0) return NULL;

    /* creates a new mapping in the virtual address space of the calling process */
    void *addr = mmap(NULL,               /* If  addr is NULL, then the kernel chooses the (page-aligned) 
                                           * address at which to create the mapping; */
                    page_count*PAGE_SIZE, /* The length argument specifies the length of the mapping  
                                           * (which  must be greater than 0)*/
                    PROT_READ|PROT_WRITE, /* Pages may be read | Pages may be written */
                    MAP_ANONYMOUS|MAP_PRIVATE, /* Create a private copy-on-write mapping , 
                                           * The mapping is not backed by any file; its contents are 
                                           * initialized to zero.  The fd argument is ignored; */
                    -1, 0);

    if (addr == MAP_FAILED) {
        return NULL;
    } else {
        bzero(addr, page_count*PAGE_SIZE);
        return addr;
    }
}

/* Helper function to map a number of pages out of memory */
static void mapOutPages(void *addr, int page_count) {
    if (addr == NULL || page_count == 0)
        return;
    
    /* The munmap() system call deletes the mappings for the specified address range, 
     * and causes further references to  addresses  within the range to generate 
     * invalid memory references.  The region is also automatically unmapped 
     * when the process is terminated.  On the other hand, closing the file 
     * descriptor does not unmap the region.*/
    int ret = munmap(addr, page_count*PAGE_SIZE);
    assert(ret == 0);
}

/* This is a comparison function that treats keys as signed ints */
int compareIntKeys(void *key1, void *key2) {
    int kv1 = *((int*)key1);
    int kv2 = *((int*)key2);

    if (kv1 < kv2)
        return -1;
    else if (kv1 == kv2)
        return 0;
    else
        return 1;
}

heap *heapCreate(int initial_size, int (*comp_func)(void*,void*)) {
    heap *h = zmalloc(sizeof(*h));
    if (h == NULL) return NULL;

    if (initial_size <= 0)
        initial_size = ENTRIES_PER_PAGE;
    
    if (comp_func == NULL)
        comp_func = compareIntKeys;
    
    
    h->compare_func = comp_func;
    h->active_entries = 0;
    // Determine how many pages of entries we need
    h->minimum_pages = (initial_size/ENTRIES_PER_PAGE + ((initial_size % ENTRIES_PER_PAGE > 0) ? 1 : 0));

    // h->map_pages = sizeof(void*)*h->minimum_pages/PAGE_SIZE+1;
    /* Determine how big the map table should be
     * Total Size=Number of Entries × Size per Entry
     * Number of Pages= Total Size / Page Size
     * In practice, you add PAGE_SIZE - 1 to the total size before 
     * performing the division to ensure you round up correctly */
    h->map_pages = (sizeof(void*) * h->minimum_pages + PAGE_SIZE - 1) / PAGE_SIZE;
    // Allocate the map table
    h->mapping_table = (void**)mapInPages(h->map_pages);
    if (h->mapping_table == NULL)
        goto err;

    void *addr = mapInPages(h->minimum_pages);
    if (addr == NULL)
        goto err;

    for (int i = 0; i < h->minimum_pages; i++) {
        *(h->mapping_table+i) = addr+(i*PAGE_SIZE);
    }
    h->allocated_pages = h->minimum_pages;

    return h;
err:
    if (h) {
        if (h->mapping_table)
            mapOutPages(h->mapping_table, h->map_pages);
        zfree(h);
    }
    return NULL;
}

void heapDestroy(heap *h) {
    if (h == NULL) return;

    void **map_table = h->mapping_table;
    if (map_table == NULL) return;

    for (int i = 0; i < h->allocated_pages; i++) {
        mapOutPages(*(map_table+i), 1);
    }
    // Map out the map table
    mapOutPages(map_table, h->map_pages);

    h->active_entries = 0;
    h->allocated_pages = 0;
    h->map_pages = 0;
    h->mapping_table = NULL;

    zfree(h);
}

int heapSize(heap *h) {
    return h->active_entries;
}

void heapInsert(heap *h, void *key, void *value) {
    int max_entries = h->allocated_pages * ENTRIES_PER_PAGE;

    if (h->active_entries+1 > max_entries) {
        int map_pages = h->map_pages;

        int mapable_pages = map_pages * PAGE_SIZE / sizeof(void*);
        if (h->allocated_pages+1 > mapable_pages) {
            void *new_table = mapInPages(map_pages+1);
            void *old_table = (void*)h->mapping_table;

            memcpy(new_table, old_table, map_pages*PAGE_SIZE);

            mapOutPages(old_table, map_pages);

            h->mapping_table = (void**)new_table;
            h->map_pages = map_pages + 1;
        }

        void *addr = mapInPages(1);
        *(h->mapping_table+h->allocated_pages) = addr;
        h->allocated_pages++;
    }

    int (*cmp_func)(void*,void*) = h->compare_func;
    void **map_table = h->mapping_table;

    int current_index = h->active_entries;
    heap_entry *cur = GET_ENTRY(current_index, map_table);
    int parent_index;
    heap_entry *parent;
    while (current_index > 0) {
        parent_index = PARENT_ENTRY(current_index);
        parent = GET_ENTRY(parent_index, map_table);
        if (cmp_func(key, parent->key) < 0) {
            cur->key = parent->key;
            cur->value = parent->value;

            current_index = parent_index;
            cur = parent;
        } else {
            break;
        }
    }
    cur->key = key;
    cur->value = value; 

    h->active_entries++;
}

int heapMin(heap *h, void **key, void **value) {
    if (h->active_entries == 0)
        return 0;
    
    heap_entry *root = GET_ENTRY(0, h->mapping_table);

    *key = root->key;
    *value = root->value;

    return 1;
}

int heapDelMin(heap *h, void **key, void **value) {
    if (h->active_entries == 0)
        return 0;
    
    int current_index = 0;
    void **map_table = h->mapping_table;
    heap_entry *current = GET_ENTRY(current_index, map_table);

    *key = current->key;
    *value = current->value;

    h->active_entries--;

    int entries = h->active_entries;
    // If there are any other nodes, we may need to move them up
    if (h->active_entries > 0) {
        heap_entry *last = GET_ENTRY(entries, map_table);
        current->key = last->key;
        current->value = last->value;

        heap_entry* left_child;
        heap_entry* right_child;
        int left_child_index;
        int (*cmp_func)(void*,void*) = h->compare_func;

        while (left_child_index = LEFT_CHILD(current_index), left_child_index < entries) {
            left_child = GET_ENTRY(left_child_index, map_table);
            // We have a left + right child
            if (left_child_index+1 < entries) {
                right_child = GET_ENTRY((left_child_index+1), map_table);

                // Find the smaller child
                if (cmp_func(left_child->key, right_child->key) <= 0) {
                    // Swap with the left if it is smaller
                    if (cmp_func(current->key, left_child->key) == 1) {
                        SWAP_ENTRIES(current,left_child);
                        current_index = left_child_index;
                        current = left_child;
                    } else {
                        break;
                    }
                } else {
                    // Right child is smaller
                    if (cmp_func(current->key, right_child->key) == 1) {
                        SWAP_ENTRIES(current,right_child);
                        current_index = left_child_index+1;
                        current = right_child;
                    } else {
                        // Current is smaller
                        break;
                    }
                }
            } else if (cmp_func(current->key, left_child->key) == 1){
                // We only have a left child, only do something if the left is smaller
                SWAP_ENTRIES(current,left_child);
                current_index = left_child_index;
                current = left_child;
            } else {
                break;
            }
        }
    }

    // Check if we should release a page of memory
    int used_pages = entries / ENTRIES_PER_PAGE + ((entries % ENTRIES_PER_PAGE > 0) ? 1 : 0);
    // Allow one empty page, but not two
    if (h->allocated_pages > used_pages+1 && h->allocated_pages > h->minimum_pages) {
        // Get the address of the page to delete
        void *addr = *(map_table+h->allocated_pages-1);
        mapOutPages(addr, 1);
        h->allocated_pages--;
    }
    return 1;
}

void heapForeach(heap *h, void (*func)(void*,void*)) {
    int index = 0;
    int entries = h->active_entries;
    heap_entry* entry;
    void **map_table = h->mapping_table;

    for (; index<entries; index++) {
        entry = GET_ENTRY(index, map_table);
        func(entry->key, entry->value);
    }
}