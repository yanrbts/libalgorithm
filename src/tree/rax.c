/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 * Copyright (c) 2017-Present, Redis Ltd. All rights reserved.
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
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <rax.h>
#include <zmalloc.h>

#define rax_malloc  zmalloc
#define rax_realloc zrealloc
#define rax_free    zfree

/* -------------------------------- Debugging ------------------------------ */

void raxDebugShowNode(const char *msg, raxNode *n);

/* Turn debugging messages on/off by compiling with RAX_DEBUG_MSG macro on.
 * When RAX_DEBUG_MSG is defined by default Rax operations will emit a lot
 * of debugging info to the standard output, however you can still turn
 * debugging on/off in order to enable it only when you suspect there is an
 * operation causing a bug using the function raxSetDebugMsg(). */
#ifdef RAX_DEBUG_MSG
#define debugf(...)                                                            \
    if (raxDebugMsg) {                                                         \
        printf("%s:%s:%d:\t", __FILE__, __func__, __LINE__);                   \
        printf(__VA_ARGS__);                                                   \
        fflush(stdout);                                                        \
    }

#define debugnode(msg,n) raxDebugShowNode(msg,n)
#else
#define debugf(...)
#define debugnode(msg,n)
#endif

/* By default log debug info if RAX_DEBUG_MSG is defined. */
static int raxDebugMsg = 1;

/* When debug messages are enabled, turn them on/off dynamically. By
 * default they are enabled. Set the state to 0 to disable, and 1 to
 * re-enable. */
void raxSetDebugMsg(int onoff) {
    raxDebugMsg = onoff;
}

/* ------------------------- raxStack functions --------------------------
 * The raxStack is a simple stack of pointers that is capable of switching
 * from using a stack-allocated array to dynamic heap once a given number of
 * items are reached. It is used in order to retain the list of parent nodes
 * while walking the radix tree in order to implement certain operations that
 * need to navigate the tree upward.
 * ------------------------------------------------------------------------- */