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
#include <sys/time.h>
#include <time.h>
#include <intset.h>
#include <endianconv.h>

static long long usec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec)*1000000)+tv.tv_usec;
}

static uint8_t _intsetValueEncoding(int64_t v) {
    if (v < INT32_MIN || v > INT32_MAX)
        return sizeof(int64_t);
    else if (v < INT16_MIN || v > INT16_MAX)
        return sizeof(int32_t);
    else
        return sizeof(int16_t);
}

static intset *createSet(int bits, int size) {
    uint64_t mask = (1<<bits)-1;
    uint64_t value;
    intset *is = intsetNew();

    for (int i = 0; i < size; i++) {
        if (bits > 32) {
            value = (rand()*rand()) & mask;
        } else {
            value = rand() & mask;
        }
        is = intsetAdd(is, value, NULL);
    }
    return is;
}

static void checkConsistency(intset *is) {
    for (uint32_t i = 0; i < (intrev32ifbe(is->length)-1); i++) {
        uint32_t encoding = intrev32ifbe(is->encoding);

        if (encoding == sizeof(int16_t)) {
            int16_t *i16 = (int16_t*)is->contents;
            TEST_ASSERT_TRUE(i16[i] < i16[i+1]);
        } else if (encoding == sizeof(int32_t)) {
            int32_t *i32 = (int32_t*)is->contents;
            TEST_ASSERT_TRUE(i32[i] < i32[i+1]);
        } else {
            int64_t *i64 = (int64_t*)is->contents;
            TEST_ASSERT_TRUE(i64[i] < i64[i+1]);
        }
    }
}

void test_intset() {
    intset *is;
    srand(time(NULL));

    TEST_ASSERT_TRUE(_intsetValueEncoding(-32768) == sizeof(int16_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(+32767) == sizeof(int16_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(-32769) == sizeof(int32_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(+32768) == sizeof(int32_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(-2147483648) == sizeof(int32_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(+2147483647) == sizeof(int32_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(-2147483649) == sizeof(int64_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(+2147483648) == sizeof(int64_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(-9223372036854775808ull) ==
                    sizeof(int64_t));
    TEST_ASSERT_TRUE(_intsetValueEncoding(+9223372036854775807ull) ==
                    sizeof(int64_t));
}

