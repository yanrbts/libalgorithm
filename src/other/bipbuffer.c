/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 * Copyright (c) 2011, Willem-Hendrik Thiart
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
 * 
 * 
 * +------------------------------------------------+
 * |          |<-- a_size -->|<-- b_size -->|       |
 * +------------------------------------------------+
 * ^          ^              ^              ^
 * |          |              |              |
 * Buffer     a_start        a_end          b_end
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcpy */
#include <bipbuffer.h>

static size_t bipbuf_sizeof(const unsigned int size) {
    return sizeof(bipbuf_t) + size;
}

int bipBufUnused(const bipbuf_t *me) {
    if (1 == me->b_inuse)
        /* distance between region B and region A */
        return me->a_start - me->b_end;
    else
        return me->size - me->a_end;
}

int bipBufSize(const bipbuf_t *me) {
    return me->size;
}

int bipBufUsed(const bipbuf_t *me) {
    return (me->a_end - me->a_start) + me->b_end;
}

void bipBufInit(bipbuf_t *me, const unsigned int size) {
    me->a_start = me->a_end = me->b_end = 0;
    me->size = size;
    me->b_inuse = 0;
}

bipbuf_t *bipBufNew(const unsigned int size) {
    bipbuf_t *me = malloc(bipbuf_sizeof(size));
    if (!me) return NULL;
    bipBufInit(me, size);
    return me;
}

void bipBufFree(bipbuf_t *me) {
    free(me);
}

int bipBufIsEmpty(const bipbuf_t *me) {
    return me->a_start == me->a_end;
}

/* find out if we should turn on region B
 * ie. is the distance from A to buffer's end less than B to A? */
static void __check_for_switch_to_b(bipbuf_t *me) {
    if (me->size - me->a_end < me->a_start - me->b_end)
        me->b_inuse = 1;
}

int bipBufOffer(bipbuf_t *me, const unsigned char *data, const int size) {
    /* not enough space */
    if (bipBufUnused(me) < size)
        return 0;
    
    if (1 == me->b_inuse) {
        memcpy(me->data+me->b_end, data, size);
        me->b_end += size;
    } else {
        memcpy(me->data+me->a_end, data, size);
        me->a_end += size;
    }

    __check_for_switch_to_b(me);
    return size;
}

unsigned char *bipBufPeek(const bipbuf_t *me, const unsigned int size) {
    /* make sure we can actually peek at this data */
    if (me->size < me->a_start + size)
        return NULL;
    
    if (bipBufIsEmpty(me))
        return NULL;
    
    return (unsigned char*)me->data + me->a_start;
}

unsigned char *bipBufPoll(bipbuf_t* me, const unsigned int size) {
    if (bipBufIsEmpty(me))
        return NULL;

    /* make sure we can actually poll this data */
    if (me->size < me->a_start+size)
        return NULL;
    
    void *end = me->data+me->a_start;
    me->a_start += size;

    /* we seem to be empty.. */
    if (me->a_start == me->a_end) {
        /* replace a with region b */
        if (1 == me->b_inuse){
            me->a_start = 0;
            me->a_end = me->b_end;
            me->b_end = me->b_inuse = 0;
        } else
            /* safely move cursor back to the start because we are empty */
            me->a_start = me->a_end = 0;
    }

    __check_for_switch_to_b(me);
    return end;
}