/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
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
#include <zmalloc.h>
#include <stack.h>

#define st_malloc  zmalloc
#define st_realloc zrealloc
#define st_free    zfree

/* Initialize the stack. */
void stackInit(stack *ts) {
    ts->stack = ts->static_items;
    ts->items = 0;
    ts->maxitems = STACK_STATIC_ITEMS;
    ts->oom = 0;
}

/* If the number of inserted elements exceeds the maximum allowed, 
 * the capacity needs to be doubled.
 * Push an item into the stack, returns 1 on success, 0 on out of memory. */
int stackPush(stack *ts, void *obj) {
    if (ts->items == ts->maxitems) {
        if (ts->stack == ts->static_items) {
            ts->stack = st_malloc(sizeof(void*)*ts->maxitems*2);
            if (ts->stack == NULL) {
                ts->stack = ts->static_items;
                ts->oom = 1;
                errno = ENOMEM;
                return 0;
            }
            memcpy(ts->stack, ts->static_items, sizeof(void*)*ts->maxitems);
        } else {
            void **newalloc = st_realloc(ts->stack, sizeof(void*)*ts->maxitems*2);
            if (newalloc == NULL) {
                ts->oom = 1;
                errno = ENOMEM;
                return 0;
            }
            ts->stack = newalloc;
        }
        ts->maxitems *= 2;
    }
    ts->stack[ts->items] = obj;
    ts->items++;
    return 1;
}

/* Pop an item from the stack, the function returns NULL if there are no
 * items to pop. */
void *stackPop(stack *ts) {
    if (ts->items == 0) return NULL;
    ts->items--;
    return ts->stack[ts->items];
}

size_t stackSize(stack *ts) {
    return ts->items;
}

/* Get the top element, but do not modify the number 
 * of elements in the stack or delete the element from the stack.*/
void *stackPeek(stack *ts) {
    if (ts->items == 0) return NULL;
    return ts->stack[ts->items-1];
}

/* Only the stack pointer is released, but the data is not released. 
 * The data needs to be released according to the actual situation.*/
void stackFree(stack *ts) {
    if (ts->stack != ts->static_items) st_free(ts->stack);
}