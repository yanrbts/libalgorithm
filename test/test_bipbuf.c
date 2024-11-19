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
#include <bipbuffer.h>

void test_bipbuffer(void) {
    void *me;

    me = bipBufNew(16);
    TEST_ASSERT_EQUAL_INT(16, bipBufSize(me));
    TEST_ASSERT_TRUE(bipBufIsEmpty(me));

    bipBufOffer(me, (unsigned char*)strdup("abcd"), 4);
    TEST_ASSERT_FALSE(bipBufIsEmpty(me));
    unsigned char *p = bipBufPoll(me, 4);
    TEST_ASSERT(strncmp(p, "abcd", 4) == 0);
    TEST_ASSERT_TRUE(bipBufIsEmpty(me));

    bipBufOffer(me, (unsigned char*)strdup("abcd"), 4);
    TEST_ASSERT(bipBufUsed(me) == 4);
    TEST_ASSERT(bipBufUnused(me) == 12);

    bipBufFree(me);
    me = bipBufNew(0);
    TEST_ASSERT(0 == bipBufOffer(me, (unsigned char*)strdup("1000"), 4));
    bipBufFree(me);

    me = bipBufNew(6);
    bipBufOffer(me, (unsigned char*)"abcdef", 6);
    bipBufPoll(me, 4);
    bipBufOffer(me, (unsigned char*)"1234", 4);

    p = bipBufPoll(me, 2);
    TEST_ASSERT(strncmp(p, "ef", 2) == 0);
    p = bipBufPoll(me, 4);
    TEST_ASSERT(strncmp(p, "1234", 4) == 0);
    bipBufFree(me);

    me = bipBufNew(6);
    bipBufOffer(me, (unsigned char*)"abcd", 4);
    bipBufOffer(me, (unsigned char*)"abcd", 4);
    p = bipBufPoll(me, 2);
    TEST_ASSERT(strncmp(p, "ab", 2) == 0);
    bipBufOffer(me, (unsigned char*)"abcd", 4);
    TEST_ASSERT_TRUE(NULL == bipBufPoll(me, 6));
    bipBufFree(me);

    me = bipBufNew(16);
    bipBufOffer(me, (unsigned char*)"abcd", 4);
    bipBufOffer(me, (unsigned char*)"efgh", 4);
    TEST_ASSERT_EQUAL_INT(8, bipBufUsed(me));
    TEST_ASSERT_EQUAL_INT(8, bipBufUnused(me));
    TEST_ASSERT_TRUE(0 == strncmp("abcd", bipBufPeek(me, 4), 4));
    TEST_ASSERT_TRUE(0 == strncmp("abcdefgh", bipBufPeek(me, 8), 8));
    bipBufPoll(me, 4);
    TEST_ASSERT_TRUE(0 == strncmp("efgh", bipBufPeek(me, 4), 4));
    TEST_ASSERT_EQUAL_INT(8, bipBufUnused(me));
    bipBufPoll(me, 4);
    TEST_ASSERT_EQUAL_INT(16, bipBufUnused(me));
}