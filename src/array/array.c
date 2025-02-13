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
#include <zmalloc.h>
#include <array.h>

#define amalloc  zmalloc
#define arealloc zrealloc
#define afree    zfree

struct array *array_create(uint32_t n, size_t size) {
    struct array *a;

    assert(n != 0 && size != 0);

    a = amalloc(sizeof(*a));
    if (a == NULL) return NULL;

    a->elem = amalloc(n * size);
    if (a->elem == NULL) {
        afree(a);
        return NULL;
    }

    a->nelem = 0;
    a->size = size;
    a->nalloc = n;

    return a;
}

void array_destroy(struct array *a) {
    array_deinit(a);
    afree(a);
}

int array_init(struct array *a, uint32_t n, size_t size) {
    assert(n != 0 && size != 0);

    a->elem = amalloc(n * size);
    if (a->elem == NULL)
        return -1;

    a->nelem = 0;
    a->size = size;
    a->nalloc = n;

    return 0;
}

void array_deinit(struct array *a) {
    assert(a->nelem == 0);

    if (a->elem != NULL) {
        afree(a->elem);
    }
}

uint32_t array_idx(const struct array *a, const void *elem) {
    const uint8_t *p, *q;
    uint32_t off, idx;

    assert(elem >= a->elem);

    p = a->elem;
    q = elem;
    off = (uint32_t)(q - p);

    assert(off % (uint32_t)a->size == 0);

    idx = off / (uint32_t)a->size;

    return idx;
}

void *array_push(struct array *a) {
    void *elem, *new;
    size_t size;

    if (a->nelem == a->nalloc) {
        /* the array is full; allocate new array */
        size = a->size * a->nalloc;
        new = arealloc(a->elem, 2*size);
        if (new == NULL) {
            return NULL;
        }

        a->elem = new;
        a->nalloc *= 2;
    }

    elem = (uint8_t*)a->elem + a->size * a->nelem;
    a->nelem++;

    return elem;
}

void *array_pop(struct array *a) {
    void *elem;

    assert(a->nelem != 0);

    a->nelem--;
    elem = (uint8_t *)a->elem + a->size * a->nelem;

    return elem;
}

void *array_get(const struct array *a, uint32_t idx) {
    void *elem;

    assert(a->nelem != 0);
    assert(idx < a->nelem);

    elem = (uint8_t *)a->elem + (a->size * idx);

    return elem;
}

void *array_top(const struct array *a) {
    assert(a->nelem != 0);

    return array_get(a, a->nelem - 1);
}

void array_swap(struct array *a, struct array *b) {
    struct array tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
 * Sort nelem elements of the array in ascending order based on the
 * compare comparator.
 */
void array_sort(struct array *a, array_compare_t compare) {
    assert(a->nelem != 0);

    qsort(a->elem, a->nelem, a->size, compare);
}

/*
 * Calls the func once for each element in the array as long as func returns
 * success. On failure short-circuits and returns the error status.
 */
int array_each(const struct array *a, array_each_t func, void *data) {
    uint32_t i, nelem;

    assert(array_n(a) != 0);
    assert(func != NULL);

    for (i = 0, nelem = array_n(a); i < nelem; i++) {
        void *elem = array_get(a, i);
        int status;

        status = func(elem, data);
        if (status != 0) {
            return status;
        }
    }

    return 0;
}


