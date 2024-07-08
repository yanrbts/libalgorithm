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
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <zmalloc.h>
#include <skiplist.h>

#define ZSKIPLIST_MAXLEVEL 32 /* Should be enough for 2^64 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */
#define ZSKIPLIST_MAX_SEARCH 10

/* Create a skiplist node with the specified number of levels.
 * The SDS string 'ele' is referenced by the node after the call. */
zskiplistNode *zslCreateNode(int level, double score, void *ele) {
    zskiplistNode *zn;

    zn = zmalloc(sizeof(*zn)+level*sizeof(struct zskiplistLevel));
    zn->score = score;
    zn->ele = ele;

    return zn;
}

zskiplist *zslCreate(memcmp mp) {
    int j;
    zskiplist *zsl;

    zsl = zmalloc(sizeof(*zsl));
    zsl->level = 1;
    zsl->length = 0;
    zsl->header = zslCreateNode(ZSKIPLIST_MAXLEVEL, 0, NULL);
    for (j = 0; j < ZSKIPLIST_MAXLEVEL; j++) {
        zsl->header->level[j].forward = NULL;
        zsl->header->level[j].span = 0;
    }
    zsl->header->backward = NULL;
    zsl->tail = NULL;
    zsl->mcmp = mp;
    return zsl;
}

/* Free the specified skiplist node. The referenced SDS string representation
 * of the element is freed too, unless node->ele is set to NULL before calling
 * this function. */
void zslFreeNode(zskiplistNode *node) {
    if (node->ele)
        zfree(node->ele);
    zfree(node);
}

/* Free a whole skiplist. */
void zslFree(zskiplist *zsl) {
    zskiplistNode *node = zsl->header->level[0].forward, *next;

    zfree(zsl->header);
    while (node) {
        next = node->level[0].forward;
        zslFreeNode(node);
        node = next;
    }
    zfree(zsl);
}

/* Returns a random level for the new skiplist node we are going to create.
 * The return value of this function is between 1 and ZSKIPLIST_MAXLEVEL
 * (both inclusive), with a powerlaw-alike distribution where higher
 * levels are less likely to be returned. */
int zslRandomLevel(void) {
    static const int threshold = ZSKIPLIST_P*RAND_MAX;
    int level = 1;
    while (random() < threshold)
        level += 1;
    return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

/* Insert a new node in the skiplist. Assumes the element does not already
 * exist (up to the caller to enforce that). The skiplist takes ownership
 * of the passed SDS string 'ele'. */
zskiplistNode *zslInsert(zskiplist *zsl, double score, void *ele) {
    zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    unsigned long rank[ZSKIPLIST_MAXLEVEL];
    int i, level;

    /* If it is not a digital terminal */
    assert(!isnan(score));
    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        /* store rank that is crossed to reach the insert position */
        rank[i] = i == (zsl->level-1) ? 0 : rank[i+1];
        while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                    (x->level[i].forward->score == score &&
                    zsl->mcmp(x->level[i].forward->ele, ele) < 0)))
        {
            rank[i] += x->level[i].span;
            x = x->level[i].forward;
        }
        update[i] = x;
    }
    /* we assume the element is not already inside, since we allow duplicated
     * scores, reinserting the same element should never happen since the
     * caller of zslInsert() should test in the hash table if the element is
     * already inside or not. */
    level = zslRandomLevel();
    if (level > zsl->level) {
        for (i = zsl->level; i < level; i++) {
            rank[i] = 0;
            update[i] = zsl->header;
            update[i]->level[i].span = zsl->length;
        }
        zsl->level = level;
    }
    x = zslCreateNode(level, score, ele);
    for (i = 0; i < level; i++) {
        x->level[i].forward = update[i]->level[i].forward;
        update[i]->level[i].forward = x;

        /* update span covered by update[i] as x is inserted here */
        x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
        update[i]->level[i].span = (rank[0] - rank[i]) + 1;
    }

    /* increment span for untouched levels */
    for (i = level; i < zsl->level; i++) {
        update[i]->level[i].span++;
    }

    x->backward = (update[0] == zsl->header) ? NULL : update[0];
    if (x->level[0].forward)
        x->level[0].forward->backward = x;
    else
        zsl->tail = x;
    zsl->length++;
    return x;
}

void zslDeleteNode(zskiplist *zsl, zskiplistNode *x, zskiplistNode **update) {
    int i;
    for (i = 0; i < zsl->level; i++) {
        if (update[i]->level[i].forward == x) {
            update[i]->level[i].span += x->level[i].span - 1;
            update[i]->level[i].forward = x->level[i].forward;
        } else {
            update[i]->level[i].span -= 1;
        }
    }
    if (x->level[0].forward) {
        x->level[0].forward->backward = x->backward;
    } else {
        zsl->tail = x->backward;
    }
    while (zsl->level > 1 && zsl->header->level[zsl->level-1].forward == NULL)
        zsl->level--;
    zsl->length--;
}

/* Delete an element with matching score/element from the skiplist.
 * The function returns 1 if the node was found and deleted, otherwise
 * 0 is returned.
 *
 * If 'node' is NULL the deleted node is freed by zslFreeNode(), otherwise
 * it is not freed (but just unlinked) and *node is set to the node pointer,
 * so that it is possible for the caller to reuse the node (including the
 * referenced SDS string at node->ele). */
int zslDelete(zskiplist *zsl, double score, void *ele, zskiplistNode **node) {
    zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    int i;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
                (x->level[i].forward->score < score ||
                    (x->level[i].forward->score == score &&
                     zsl->mcmp(x->level[i].forward->ele,ele) < 0)))
        {
            x = x->level[i].forward;
        }
        update[i] = x;
    }
    /* We may have multiple elements with the same score, what we need
     * is to find the element with both the right score and object. */
    x = x->level[0].forward;
    if (x && score == x->score && zsl->mcmp(x->ele, ele) == 0) {
        zslDeleteNode(zsl, x, update);
        if (!node)
            zslFreeNode(x);
        else
            *node = x;
        return 1;
    }
    return 0; /* not found */
}

/* Update the score of an element inside the sorted set skiplist.
 * Note that the element must exist and must match 'score'.
 * This function does not update the score in the hash table side, the
 * caller should take care of it.
 *
 * Note that this function attempts to just update the node, in case after
 * the score update, the node would be exactly at the same position.
 * Otherwise the skiplist is modified by removing and re-adding a new
 * element, which is more costly.
 *
 * The function returns the updated element skiplist node pointer. */
zskiplistNode *zslUpdateScore(zskiplist *zsl, double curscore, void *ele, double newscore) {
    zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    int i;

    /* We need to seek to element to update to start: this is useful anyway,
     * we'll have to update or remove it. */
    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
                (x->level[i].forward->score < curscore ||
                    (x->level[i].forward->score == curscore &&
                     zsl->mcmp(x->level[i].forward->ele, ele) < 0)))
        {
            x = x->level[i].forward;
        }
        update[i] = x;
    }

    /* Jump to our element: note that this function assumes that the
     * element with the matching score exists. */
    x = x->level[0].forward;
    assert(x && curscore == x->score && zsl->mcmp(x->ele, ele) == 0);

    /* If the node, after the score update, would be still exactly
     * at the same position, we can just update the score without
     * actually removing and re-inserting the element in the skiplist. */
    if ((x->backward == NULL || x->backward->score < newscore) &&
        (x->level[0].forward == NULL || x->level[0].forward->score > newscore))
    {
        x->score = newscore;
        return x;
    }

    /* No way to reuse the old node: we need to remove and insert a new
     * one at a different place. */
    zslDeleteNode(zsl, x, update);
    zskiplistNode *newnode = zslInsert(zsl, newscore, x->ele);
    /* We reused the old node x->ele SDS string, free the node now
     * since zslInsert created a new one. */
    x->ele = NULL;
    zslFreeNode(x);
    return newnode;
}

int zslValueGteMin(double value, zrangespec *spec) {
    return spec->minex ? (value > spec->min) : (value >= spec->min);
}

int zslValueLteMax(double value, zrangespec *spec) {
    return spec->maxex ? (value < spec->max) : (value <= spec->max);
}

/* Returns if there is a part of the zset is in range. */
int zslIsInRange(zskiplist *zsl, zrangespec *range) {
    zskiplistNode *x;

    /* Test for ranges that will always be empty. */
    if (range->min > range->max ||
        (range->min == range->max && (range->minex || range->maxex)))
        return 0;
    
    x = zsl->tail;
    if (x == NULL || !zslValueGteMin(x->score, range))
        return 0;
    x = zsl->header->level[0].forward;
    if (x == NULL || !zslValueLteMax(x->score, range))
        return 0;
    return 1;
}

/* Finds an element by its rank from start node. The rank argument needs to be 1-based. */
zskiplistNode *zslGetElementByRankFromNode(zskiplistNode *start_node, 
    int start_level, unsigned long rank) {
    zskiplistNode *x;
    unsigned long traversed = 0;
    int i;

    x = start_node;
    for (i = start_level; i >= 0; i--) {
        while (x->level[i].forward && (traversed + x->level[i].span) <= rank) {
            traversed += x->level[i].span;
            x = x->level[i].forward;
        }
        if (traversed == rank) {
            return x;
        }
    }
    return NULL;
}

/* Find the Nth node that is contained in the specified range. N should be 0-based.
 * Negative N works for reversed order (-1 represents the last element). Returns
 * NULL when no element is contained in the range. */
zskiplistNode *zslNthInRange(zskiplist *zsl, zrangespec *range, long n) {
    zskiplistNode *x;
    int i;
    long edge_rank = 0;
    long last_highest_level_rank = 0;
    zskiplistNode *last_highest_level_node = NULL;
    unsigned long rank_diff;

    /* If everything is out of range, return early. */
    if (!zslIsInRange(zsl, range)) return NULL;

    /* Go forward while *OUT* of range at level of zsl->level-1. */
    x = zsl->header;
    i = zsl->level - 1;
    while (x->level[i].forward && !zslValueGteMin(x->level[i].forward->score, range)) {
        edge_rank += x->level[i].span;
        x = x->level[i].forward;
    }
    /* Remember the last node which has zsl->level-1 levels and its rank. */
    last_highest_level_node = x;
    last_highest_level_rank = edge_rank;

    if (n >= 0) {
        for (i = zsl->level - 2; i >= 0; i--) {
            /* Go forward while *OUT* of range. */
            while (x->level[i].forward && !zslValueGteMin(x->level[i].forward->score, range)) {
                /* Count the rank of the last element smaller than the range. */
                edge_rank += x->level[i].span;
                x = x->level[i].forward;
            }
        }
        /* Check if zsl is long enough. */
        if ((unsigned long)(edge_rank+n) >= zsl->length) return NULL;
        if (n < ZSKIPLIST_MAX_SEARCH) {
            /* If offset is small, we can just jump node by node */
            /* rank+1 is the first element in range, so we need n+1 steps to reach target. */
            for (i = 0; i < n + 1; i++) {
                x = x->level[0].forward;
            }
        } else {
            /* If offset is big, we can jump from the last zsl->level-1 node. */
            rank_diff = edge_rank + 1 + n - last_highest_level_rank;
            x = zslGetElementByRankFromNode(last_highest_level_node, zsl->level - 1, rank_diff);
        }
        /* Check if score <= max. */
        if (x && !zslValueLteMax(x->score, range)) return NULL;
    } else {
        for (i = zsl->level - 1; i >= 0; i--) {
            /* Go forward while *IN* range. */
            while (x->level[i].forward && zslValueLteMax(x->level[i].forward->score, range)) {
                /* Count the rank of the last element in range. */
                edge_rank += x->level[i].span;
                x = x->level[i].forward;
            }
        }
        /* Check if the range is big enough. */
        if (edge_rank < -n) return NULL;
        if (n + 1 > -ZSKIPLIST_MAX_SEARCH) {
            /* If offset is small, we can just jump node by node */
            /* rank is the -1th element in range, so we need -n-1 steps to reach target. */
            for (i = 0; i < -n - 1; i++) {
                x = x->backward;
            }
        } else {
            /* If offset is big, we can jump from the last zsl->level-1 node. */
            /* rank is the last element in range, n is -1-based, so we need n+1 to count backwards. */
            rank_diff = edge_rank + 1 + n - last_highest_level_rank;
            x = zslGetElementByRankFromNode(last_highest_level_node, zsl->level - 1, rank_diff);
        }
        /* Check if score >= min. */
        if (x && !zslValueGteMin(x->score, range)) return NULL;
    }
    return x;
}