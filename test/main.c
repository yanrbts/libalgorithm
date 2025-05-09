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
#include <unity.h>
#include "test_zsl.c"
#include "test_rax.c"
#include "test_intset.c"
#include "test_listpack.c"
#include "test_stack.c"
#include "test_minheap.c"
#include "test_sds.c"
#include "test_avltree.c"
#include "test_bipbuf.c"
#include "test_mempool.c"
#include "test_array.c"
#include "test_ringbuf.c"

void setUp(void) {

}

void tearDown(void) {

}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_zslCreate);
    RUN_TEST(test_zslInsert);
    RUN_TEST(test_zslNthInRange);
    RUN_TEST(test_zslGetRank);
    RUN_TEST(test_zslDelete);
    RUN_TEST(test_zslIterator);
    // rax test
    RUN_TEST(test_rax_regression);
    RUN_TEST(test_raxInsert);
    // intset test
    RUN_TEST(test_intset);
    // listpack test
    // RUN_TEST(test_listpack);
    // stack test
    RUN_TEST(test_stack);
    // minheap test
    // RUN_TEST(test_minheap);
    // sds test
    // RUN_TEST(test_sds);
    RUN_TEST(test_avltree);
    RUN_TEST(test_bipbuffer);
    RUN_TEST(test_mempool);
    RUN_TEST(test_arrayfunc);
    RUN_TEST(test_ringbuf);

    return UNITY_END();
}