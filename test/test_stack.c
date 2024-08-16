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
#include <stack.h>

void test_stack(void) {
    stack s;

    stackInit(&s);
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"123"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"3434343434343"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"dfsdfsdfsdfsdffdfdfds"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"dfsdfsdf"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"hhhh dsfsdfdsf"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"dfsdfdn"));
    TEST_ASSERT_TRUE(stackPush(&s, (void*)"ddkkkk"));

    TEST_ASSERT_EQUAL_INT(7, stackSize(&s));
    TEST_ASSERT_EQUAL(0, strcmp("ddkkkk", stackPeek(&s)));
    TEST_ASSERT_EQUAL_INT(7, stackSize(&s));

    stackPop(&s);
    TEST_ASSERT_EQUAL(0, strcmp("dfsdfdn", stackPeek(&s)));
    TEST_ASSERT_EQUAL_INT(6, stackSize(&s));
    stackFree(&s);
}