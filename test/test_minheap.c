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
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <minheap.h>

void test_minheap(void) {
    int count = 10000000; // 10M
    heap *h = heapCreate(0, NULL);

    int *key = (int*)malloc(count*sizeof(int));
    char *value = "The meaning of life.";

    unsigned int val = 42;
    srand(val);

    int min = INT_MAX;
    // Use a pseudo-random generator for the other keys
    for (int i = 0; i < count; i++) {
        *(key+i) = rand();

        // Check for a new min
        if (*(key+i) < min)
            min = *(key+i);

        // Insert into the heap
        heapInsert(h, key+i, value);
    }
    TEST_ASSERT_EQUAL_INT(10000000, heapSize(h));
    int *min_key;
    char*min_val;
    int *prev_key = &min;
    int n = 10;
    while (heapDelMin(h, (void**)&min_key, (void**)&min_val)) {
        // Verify that the values are getting larger
        if (n != 0) {
            printf("%ld ", (long)*min_key);
            n--;
        } else {
            printf("%ld \n", (long)*min_key);
            n = 10;
        }
            
        if (*prev_key > *min_key) {
            printf("Previous key is greater than current key!\n");
        }
        prev_key = min_key;
    }
    TEST_ASSERT_EQUAL_INT(0, heapSize(h));
    // Clean up the heap
    heapDestroy(h);
}