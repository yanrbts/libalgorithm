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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <skiplist.h>

static int zslMemCmp(const void *s1, const void *s2, size_t size) {
    int v1, v2;

    (void)size;

    v1 = (int)(intptr_t)s1;
    v2 = (int)(intptr_t)s2;

    if (v1 < v2) return -1;
    else if (v1 == v2) return 0;
    else return 1;
}

void test_zslCreate(void) {
    zskiplist *zl;

    zl = zslCreate(NULL);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);
    zslFree(zl);
}

void test_zslInsert(void) {
    zskiplist *zl;
    zskiplistNode *n;

    zl = zslCreate(NULL);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);

    int num = 42;
    n = zslInsert(zl, 1.0, (void*)&num, sizeof(num));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL(42, *((int*)n->ele));

    n = zslInsert(zl, 1.1, (void*)(intptr_t)43, sizeof(intptr_t));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL(43, (int)((intptr_t)n->ele));
    // zslFree(zl);
}

void test_zslNthInRange(void) {
    zskiplist *zl;
    zskiplistNode *n;

    zl = zslCreate(zslMemCmp);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);

    n = zslInsert(zl, 1.0, (void*)(intptr_t)42, sizeof(intptr_t));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, (int)((intptr_t)n->ele));

    n = zslInsert(zl, 1.1, (void*)(intptr_t)43, sizeof(intptr_t));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, (int)((intptr_t)n->ele));

    n = zslInsert(zl, 2.0, (void*)(intptr_t)44, sizeof(intptr_t));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, (int)((intptr_t)n->ele));

    n = zslInsert(zl, 2.5, (void*)(intptr_t)45, sizeof(intptr_t));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, (int)((intptr_t)n->ele));
    
    ///////////////////////////////////////////////////////////////////////////
    zrangespec range = { .min = 1.0, .max = 1.1, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range, 1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, (int)((intptr_t)n->ele));

    zrangespec range2 = { .min = 1.0, .max = 2.5, .minex = 1, .maxex = 1 };
    n = zslNthInRange(zl, &range2, 0);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, (int)((intptr_t)n->ele));

    zrangespec range3 = { .min = 1.0, .max = 2.5, .minex = 1, .maxex = 0 };
    n = zslNthInRange(zl, &range3, 2);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, (int)((intptr_t)n->ele));

    zrangespec range4 = { .min = 1.5, .max = 2.5, .minex = 0, .maxex = 1 };
    n = zslNthInRange(zl, &range4, 0);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, (int)((intptr_t)n->ele));

    zrangespec range5 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 1 };
    n = zslNthInRange(zl, &range5, -1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, (int)((intptr_t)n->ele));

    zrangespec range6 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range6, -1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, (int)((intptr_t)n->ele));

    zrangespec range7 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range7, -2);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, (int)((intptr_t)n->ele));

    zrangespec range8 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range8, -(zl->length));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, (int)((intptr_t)n->ele));
}

void test_zslGetRank(void) {
    zskiplist *zl;
    zskiplistNode *n;
    unsigned long rank;
    
    zl = zslCreate(zslMemCmp);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);

    int num0 = 42;
    n = zslInsert(zl, 1.0, (void*)&num0, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));

    int num1 = 43;
    n = zslInsert(zl, 1.1, (void*)&num1, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));

    int num2 = 44;
    n = zslInsert(zl, 1.5, (void*)&num2, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));

    int num3 = 45;
    n = zslInsert(zl, 2.0, (void*)&num3, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));

    rank = zslGetRank(zl, 2.0, (void*)&num3, sizeof(int));
    TEST_ASSERT_EQUAL_INT(4, rank);

    rank = zslGetRank(zl, 1.0, (void*)&num0, sizeof(int));
    TEST_ASSERT_EQUAL_INT(1, rank);

    rank = zslGetRank(zl, 1.1, (void*)&num1, sizeof(int));
    TEST_ASSERT_EQUAL_INT(2, rank);
}

void test_zslDelete(void) {
    zskiplist *zl;
    zskiplistNode *n, *update;
    unsigned long rank;
    int ret;
    
    zl = zslCreate(zslMemCmp);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);

    int num0 = 42;
    n = zslInsert(zl, 1.0, (void*)&num0, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));

    int num1 = 43;
    n = zslInsert(zl, 1.1, (void*)&num1, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));

    int num2 = 44;
    n = zslInsert(zl, 1.5, (void*)&num2, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));

    int num3 = 45;
    n = zslInsert(zl, 2.0, (void*)&num3, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));
    //////////////////////////////////////////////////////////////////////
    zrangespec range0 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range0, 1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));

    ret = zslDelete(zl, 1.1, (void*)&num1, sizeof(int), &update);
    TEST_ASSERT_EQUAL_INT(1, ret);

    zrangespec range1 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range1, 1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));

    ret = zslDelete(zl, 0.0, (void*)&num2, sizeof(int), &update);
    TEST_ASSERT_EQUAL_INT(0, ret);

    zrangespec range2 = { .min = 0.0, .max = 2.5, .minex = 0, .maxex = 0 };
    n = zslNthInRange(zl, &range2, 1);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));

    n = zl->header->level[0].forward;
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));
    n = n->level[0].forward;

    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));
    n = n->level[0].forward;

    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));
    n = n->level[0].forward;
}

void test_zslIterator(void) {
    zskiplist *zl;
    zskiplistNode *n;
    zskiplistIter *iter;

    zl = zslCreate(zslMemCmp);
    TEST_ASSERT_NOT_EQUAL(NULL, zl);

    int num0 = 42;
    n = zslInsert(zl, 1.0, (void*)&num0, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));

    int num1 = 43;
    n = zslInsert(zl, 1.1, (void*)&num1, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));

    int num2 = 44;
    n = zslInsert(zl, 1.5, (void*)&num2, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));

    int num3 = 45;
    n = zslInsert(zl, 2.0, (void*)&num3, sizeof(int));
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));
    ////////////////////////////////////////////////////////////
    iter = zslGetIterator(zl, 0);
    n = zslNext(iter);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));

    zslReleaseIterator(iter);

    iter = zslGetIterator(zl, 1);
    n = zslNext(iter);
    TEST_ASSERT_NOT_EQUAL(NULL, n);
    TEST_ASSERT_EQUAL_INT(45, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(44, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(43, *((int*)n->ele));
    n = zslNext(iter);
    TEST_ASSERT_EQUAL_INT(42, *((int*)n->ele));
    zslReleaseIterator(iter);
}