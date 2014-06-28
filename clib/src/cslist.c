/*
 * gslist.c: Singly-linked list implementation
 *
 * Authors:
 *   Duncan Mak (duncan@novell.com)
 *   Raja R Harinath (rharinath@novell.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * (C) 2006 Novell, Inc.
 */

#include <config.h>

#include <stdio.h>
#include <clib.h>

c_slist_t *
c_slist_alloc(void)
{
    return c_new0(c_slist_t, 1);
}

void
c_slist_free_1(c_slist_t *list)
{
    c_free(list);
}

c_slist_t *
c_slist_append(c_slist_t *list, void *data)
{
    return c_slist_concat(list, c_slist_prepend(NULL, data));
}

/* This is also a list node constructor. */
c_slist_t *
c_slist_prepend(c_slist_t *list, void *data)
{
    c_slist_t *head = c_slist_alloc();
    head->data = data;
    head->next = list;

    return head;
}

/*
 * Insert the given data in a new node after the current node.
 * Return new node.
 */
static inline c_slist_t *
insert_after(c_slist_t *list, void *data)
{
    list->next = c_slist_prepend(list->next, data);
    return list->next;
}

/*
 * Return the node prior to the node containing 'data'.
 * If the list is empty, or the first node contains 'data', return NULL.
 * If no node contains 'data', return the last node.
 */
static inline c_slist_t *
find_prev(c_slist_t *list, const void *data)
{
    c_slist_t *prev = NULL;
    while (list) {
        if (list->data == data)
            break;
        prev = list;
        list = list->next;
    }
    return prev;
}

/* like 'find_prev', but searches for node 'link' */
static inline c_slist_t *
find_prev_link(c_slist_t *list, c_slist_t *link)
{
    c_slist_t *prev = NULL;
    while (list) {
        if (list == link)
            break;
        prev = list;
        list = list->next;
    }
    return prev;
}

c_slist_t *
c_slist_insert_before(c_slist_t *list, c_slist_t *sibling, void *data)
{
    c_slist_t *prev = find_prev_link(list, sibling);

    if (!prev)
        return c_slist_prepend(list, data);

    insert_after(prev, data);
    return list;
}

void
c_slist_free(c_slist_t *list)
{
    while (list) {
        c_slist_t *next = list->next;
        c_slist_free_1(list);
        list = next;
    }
}

c_slist_t *
c_slist_copy(c_slist_t *list)
{
    c_slist_t *copy, *tmp;

    if (!list)
        return NULL;

    copy = c_slist_prepend(NULL, list->data);
    tmp = copy;

    for (list = list->next; list; list = list->next)
        tmp = insert_after(tmp, list->data);

    return copy;
}

c_slist_t *
c_slist_concat(c_slist_t *list1, c_slist_t *list2)
{
    if (!list1)
        return list2;

    c_slist_last(list1)->next = list2;
    return list1;
}

void
c_slist_foreach(c_slist_t *list, c_iter_func_t func, void *user_data)
{
    while (list) {
        (*func)(list->data, user_data);
        list = list->next;
    }
}

c_slist_t *
c_slist_last(c_slist_t *list)
{
    if (!list)
        return NULL;

    while (list->next)
        list = list->next;

    return list;
}

c_slist_t *
c_slist_find(c_slist_t *list, const void *data)
{
    for (; list; list = list->next)
        if (list->data == data)
            return list;
    return NULL;
}

c_slist_t *
c_slist_find_custom(c_slist_t *list, const void *data, c_compare_func_t func)
{
    if (!func)
        return NULL;

    while (list) {
        if (func(list->data, data) == 0)
            return list;

        list = list->next;
    }

    return NULL;
}

unsigned int
c_slist_length(c_slist_t *list)
{
    unsigned int length = 0;

    while (list) {
        length++;
        list = list->next;
    }

    return length;
}

c_slist_t *
c_slist_remove(c_slist_t *list, const void *data)
{
    c_slist_t *prev = find_prev(list, data);
    c_slist_t *current = prev ? prev->next : list;

    if (current) {
        if (prev)
            prev->next = current->next;
        else
            list = current->next;
        c_slist_free_1(current);
    }

    return list;
}

c_slist_t *
c_slist_remove_all(c_slist_t *list, const void *data)
{
    c_slist_t *next = list;
    c_slist_t *prev = NULL;
    c_slist_t *current;

    while (next) {
        c_slist_t *tmp_prev = find_prev(next, data);
        if (tmp_prev)
            prev = tmp_prev;
        current = prev ? prev->next : list;

        if (!current)
            break;

        next = current->next;

        if (prev)
            prev->next = next;
        else
            list = next;
        c_slist_free_1(current);
    }

    return list;
}

c_slist_t *
c_slist_remove_link(c_slist_t *list, c_slist_t *link)
{
    c_slist_t *prev = find_prev_link(list, link);
    c_slist_t *current = prev ? prev->next : list;

    if (current) {
        if (prev)
            prev->next = current->next;
        else
            list = current->next;
        current->next = NULL;
    }

    return list;
}

c_slist_t *
c_slist_delete_link(c_slist_t *list, c_slist_t *link)
{
    list = c_slist_remove_link(list, link);
    c_slist_free_1(link);

    return list;
}

c_slist_t *
c_slist_reverse(c_slist_t *list)
{
    c_slist_t *prev = NULL;
    while (list) {
        c_slist_t *next = list->next;
        list->next = prev;
        prev = list;
        list = next;
    }

    return prev;
}

c_slist_t *
c_slist_insert_sorted(c_slist_t *list, void *data, c_compare_func_t func)
{
    c_slist_t *prev = NULL;

    if (!func)
        return list;

    if (!list || func(list->data, data) > 0)
        return c_slist_prepend(list, data);

    /* Invariant: func (prev->data, data) <= 0) */
    for (prev = list; prev->next; prev = prev->next)
        if (func(prev->next->data, data) > 0)
            break;

    /* ... && (prev->next == 0 || func (prev->next->data, data) > 0)) */
    insert_after(prev, data);
    return list;
}

int
c_slist_index(c_slist_t *list, const void *data)
{
    int index = 0;

    while (list) {
        if (list->data == data)
            return index;

        index++;
        list = list->next;
    }

    return -1;
}

c_slist_t *
c_slist_nth(c_slist_t *list, unsigned int n)
{
    for (; list; list = list->next) {
        if (n == 0)
            break;
        n--;
    }
    return list;
}

void *
c_slist_nth_data(c_slist_t *list, unsigned int n)
{
    c_slist_t *node = c_slist_nth(list, n);
    return node ? node->data : NULL;
}

typedef c_slist_t list_node;
#include "sort.frag.h"

c_slist_t *
c_slist_sort(c_slist_t *list, c_compare_func_t func)
{
    if (!list || !list->next)
        return list;
    return do_sort(list, func);
}
