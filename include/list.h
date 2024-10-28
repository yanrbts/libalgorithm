/* Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 *
 * This file implements the specification you can find at:
 *
 *  https://github.com/gluster/glusterfs.git
 *
 * Copyright (c) 2017, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
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
#ifndef __LIST_H__
#define __LIST_H__

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define INIT_LIST_HEAD(head)                \
    do {                                    \
        (head)->next = (head)->prev = head; \
    } while (0)

static inline void
list_add(struct list_head *new, struct list_head *head) {
    new->prev = head;
    new->next = head->next;

    new->prev->next = new;
    new->next->prev = new;
}

static inline void
list_add_tail(struct list_head *new, struct list_head *head) {
    new->next = head;
    new->prev = head->prev;

    new->prev->next = new;
    new->next->prev = new;
}

/* This function will insert the element to the list in a order.
   Order will be based on the compare function provided as a input.
   If element to be inserted in ascending order compare should return:
    0: if both the arguments are equal
   >0: if first argument is greater than second argument
   <0: if first argument is less than second argument */
static inline void
list_add_order(struct list_head *new, struct list_head *head,
               int (*compare)(struct list_head *, struct list_head *))
{
    struct list_head *pos = head->prev;

    while (pos != head) {
        if (compare(new, pos) >= 0)
            break;
        
        /* Iterate the list in the reverse order. This will have
           better efficiency if the elements are inserted in the
           ascending order */
        pos = pos->prev;
    }
    list_add(new, pos);
}

/* Delete a list entry by making the prev/next entries
   point to each other.

   This is only for internal list manipulation where we know
   the prev/next entries already! */
static inline void
_list_del(struct list_head *old) {
    old->next->prev = old->prev;
    old->prev->next = old->next;
}

#define LIST_POISON1 ((void *)0xdead000000000100)
#define LIST_POISON2 ((void *)0xdead000000000122)

static inline void
list_del(struct list_head *old) {
    _list_del(old);

    old->next = (struct list_head *)LIST_POISON1;
    old->prev = (struct list_head *)LIST_POISON2;
}

static inline void
list_del_init(struct list_head *old) {
    _list_del(old);

    old->next = old;
    old->prev = old;
}

static inline void
list_move(struct list_head *list, struct list_head *head) {
    _list_del(list);
    list_add(list, head);
}

static inline void
list_move_tail(struct list_head *list, struct list_head *head) {
    _list_del(list);
    list_add_tail(list, head);
}

static inline int
list_empty(struct list_head *head) {
    return (head->next == head);
}

static inline void
__list_splice(struct list_head *list, struct list_head *head) {
    (list->prev)->next = (head->next);
    (head->next)->prev = (list->prev);

    (head)->next = (list->next);
    (list->next)->prev = (head);
}

static inline void
list_splice(struct list_head *list, struct list_head *head) {
    if (list_empty(list))
        return;

    __list_splice(list, head);
}

/* Splice moves @list to the head of the list at @head. */
static inline void
list_splice_init(struct list_head *list, struct list_head *head)
{
    if (list_empty(list))
        return;

    __list_splice(list, head);
    INIT_LIST_HEAD(list);
}

static inline void
__list_append(struct list_head *list, struct list_head *head)
{
    (head->prev)->next = (list->next);
    (list->next)->prev = (head->prev);
    (head->prev) = (list->prev);
    (list->prev)->next = head;
}

static inline void
list_append(struct list_head *list, struct list_head *head)
{
    if (list_empty(list))
        return;

    __list_append(list, head);
}

/* Append moves @list to the end of @head */
static inline void
list_append_init(struct list_head *list, struct list_head *head)
{
    if (list_empty(list))
        return;

    __list_append(list, head);
    INIT_LIST_HEAD(list);
}

#endif