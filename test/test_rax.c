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
#include <rax.h>

/* Regression test #1: Iterator wrong element returned after seek. */
void test_rax_regression(void) {
    rax *rax = raxNew();

    raxInsert(rax, (unsigned char*)"LKE", 3, (void*)(long)1, NULL);
    raxInsert(rax, (unsigned char*)"TQ", 2, (void*)(long)2, NULL);
    raxInsert(rax, (unsigned char*)"B", 1, (void*)(long)3, NULL);
    raxInsert(rax, (unsigned char*)"FY", 2, (void*)(long)4, NULL);
    raxInsert(rax, (unsigned char*)"WI", 2, (void*)(long)5, NULL);

    raxIterator iter;
    raxStart(&iter, rax);
    raxSeek(&iter, ">", (unsigned char*)"FMP", 3);
    if (raxNext(&iter)) {
        if (iter.key_len != 2 ||
            memcmp(iter.key,"FY",2))
        {
            printf("Regression test 1 failed: 'FY' expected, got: '%.*s'\n",
                (int)iter.key_len, (char*)iter.key);
            // return 1;
        }
    }

    raxStop(&iter);
    raxFree(rax);
    // return 0;
}

/* Overwriting insert. Just a wrapper for raxGenericInsert() that will
 * update the element if there is already one for the same key. 
 * the associated data is updated (only if 'overwrite' is set to 1), and 0 is returned,
 * otherwise the element is inserted and 1 is returned.*/
void test_raxInsert(void) {
    int ret = 0;
    rax *rt = raxNew();
    ret = raxInsert(rt,(unsigned char *)"a",1,(void *)100,NULL);
    TEST_ASSERT_EQUAL_INT(1, ret);
    ret = raxInsert(rt,(unsigned char *)"ab",2,(void *)101,NULL);
    TEST_ASSERT_EQUAL_INT(1, ret);
    ret = raxInsert(rt,(unsigned char *)"abc",3,(void *)NULL,NULL);
    TEST_ASSERT_EQUAL_INT(1, ret);
    ret = raxInsert(rt,(unsigned char *)"abcd",4,(void *)NULL,NULL);
    TEST_ASSERT_EQUAL_INT(1, ret);
    ret = raxInsert(rt,(unsigned char *)"abc",3,(void *)102,NULL);
    TEST_ASSERT_EQUAL_INT(0, ret);
    raxFree(rt);
}