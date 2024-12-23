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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mempool.h>
#include <adlist.h>

typedef struct {
    void        *elts;
    uintptr_t    nelts;
    size_t       size;
    uintptr_t    nalloc;
    mem_pool_t  *pool;
} array_t;

#define array_num(a) ((a)->nelts)
#define array_total(a) ((a)->nalloc)

array_t *array_create(mem_pool_t *p, uintptr_t n, size_t size) {
    array_t *a;

    a = mem_palloc(p, sizeof(array_t));

    a->nelts = 0;
    a->nalloc = n;
    a->pool = p;
    a->size = size;

    a->elts = mem_palloc(p, n * size);
    TEST_ASSERT_TRUE(a != NULL);

    return a;
}

void *
array_push(array_t *a) {
    void        *elt, *new;
    size_t       size;
    mem_pool_t  *p;

    if (a->nelts == a->nalloc) {
        /* the array is full */
        size = a->size * a->nalloc;

        p = a->pool;

        if ((u_char *)a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;
            a->nalloc++;

        } else {
            /* allocate a new array */

            new = mem_palloc(p, 2 * size);
            if (new == NULL) {
                return NULL;
            }

            (void)memcpy(new, a->elts, size);
            a->elts = new;
            a->nalloc *= 2;
        }
    }

    elt = (u_char *)a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}

void test_mempool(void) {
    mem_pool_t *p;
    array_t *a;
    int tmp;

    p = mem_create_pool(1024);
    TEST_ASSERT_TRUE(NULL != p);

    a = array_create(p, 1000, sizeof(int));
    TEST_ASSERT_TRUE(NULL != a);

    for (tmp = 0; tmp < 10000; tmp++) {
        int *i = array_push(a);
        TEST_ASSERT_TRUE(NULL != i);
        printf("push %d: %p\n", tmp, (void*)i);
        *i = tmp;
    }
    TEST_ASSERT_EQUAL(10000, array_num(a));
    TEST_ASSERT_EQUAL(16000, array_total(a));
}