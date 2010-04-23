/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/
 
#include <malloc.h>
#include "ls.h"

typedef struct ll_struct ll_t;

struct ls_entry_struct {
    ll_t *ll;
    ls_entry_t *next;
    ls_entry_t *prev;
    void *data;
};

struct ll_struct {
    ls_entry_t *head;
    ls_entry_t *tail;
    ls_t *ls;
};

struct ls_struct {
    ll_t *data_list;
    ll_t *free_list;
};

#define LS_MALLOC(t, n)	(t *) malloc(sizeof(t) * n)

/****************************************************************************
    Internal routines
****************************************************************************/
static ll_t *ll_alloc(ls_t *ls)
{
    ll_t *ll;

    ll = LS_MALLOC(ll_t, 1);
    ll->head = ll->tail = 0;
    ll->ls = ls;

    return ll;
}

static void ll_push_head(ll_t * ll, ls_entry_t * entry)
{
    if (ll->head == 0) {
        ll->head = ll->tail = entry;
        entry->next = entry->prev = 0;
    } else {
        entry->next = 0;
        entry->prev = ll->head;
        ll->head->next = entry;
        ll->head = entry;
    }
    entry->ll = ll;
}

static void ll_push_tail(ll_t * ll, ls_entry_t * entry)
{
    if (ll->tail == 0) {
        ll->head = ll->tail = entry;
        entry->next = entry->prev = 0;
    } else {
        entry->prev = 0;
        entry->next = ll->tail;
        ll->tail->prev = entry;
        ll->tail = entry;
    }
    entry->ll = ll;
}

static ls_entry_t *ll_pop_head(ll_t * ll)
{
    ls_entry_t *entry;

    if (ll->head == 0) {
        entry = 0;
    } else if (ll->head == ll->tail) {
        entry = ll->head;
        ll->head = ll->tail = 0;
    } else {
        entry = ll->head;
        ll->head = entry->prev;
        ll->head->next = 0;
    }

    return entry;
}

static void ll_pop_entry(ls_entry_t *entry)
{
    ll_t *ll;

	ll = entry->ll;

	if (ll->head == entry) {
	    ll->head = entry->prev;
	}

	if (ll->tail == entry) {
	    ll->tail = entry->next;
	}

	if (entry->next != 0) {
	    entry->next->prev = entry->prev;
	}

	if (entry->prev != 0) {
	    entry->prev->next = entry->next;
	}
}

static ls_entry_t *ll_pop_tail(ll_t * ll)
{
    ls_entry_t *entry;

    if (ll->tail == 0) {
        entry = 0;
    } else if (ll->head == ll->tail) {
        entry = ll->tail;
        ll->head = ll->tail = 0;
    } else {
        entry = ll->tail;
        ll->tail = entry->next;
        ll->tail->prev = 0;
    }

    return entry;
}

static ls_entry_t *ls_entry_alloc(ls_t * ls)
{
    ls_entry_t *entry;

    entry = ll_pop_head(ls->free_list);
    if (entry == 0) {
        entry = LS_MALLOC(ls_entry_t, 1);
    }

    entry->ll = 0;
    return entry;
}

/****************************************************************************
    Exported routines
****************************************************************************/
ls_t *ls_alloc(void)
{
    ls_t *ls;

    ls = LS_MALLOC(ls_t, 1);
    ls->data_list = ll_alloc(ls);
    ls->free_list = ll_alloc(ls);

    return ls;
}

void ls_free(ls_t *ls)
{
   if(ls) {
       if(ls->data_list) {
           ls_entry_t *entry = ll_pop_head(ls->data_list);
           while(entry)
           {
               free(entry);
               entry = ll_pop_head(ls->data_list);
           }
           free(ls->data_list);
           ls->data_list = 0;
       }
       if(ls->free_list) {
           ls_entry_t *entry = ll_pop_head(ls->free_list);
           while(entry)
           {
               free(entry);
               entry = ll_pop_head(ls->free_list);
           }
           free(ls->free_list);
           ls->free_list = 0;
       }

       free(ls);
   }
}

void ls_push_head(ls_t * ls, void *data)
{
    ls_entry_t *entry;

    entry = ls_entry_alloc(ls);
    entry->data = data;
    ll_push_head(ls->data_list, entry);
}

void ls_push_tail(ls_t * ls, void *data)
{
    ls_entry_t *entry;

    entry = ls_entry_alloc(ls);
    entry->data = data;
    ll_push_tail(ls->data_list, entry);
}

void *ls_pop_head(ls_t * ls)
{
    ls_entry_t *entry;
    void *data;

    entry = ll_pop_head(ls->data_list);
    if (entry != 0) {
        data = entry->data;
        ll_push_head(ls->free_list, entry);
    } else {
        data = 0;
    }

    return data;
}

void *ls_pop_head_n(ls_t * ls, int n)
{
    ls_entry_t *entry;
	void *data = 0;
	int i;

	entry = ls_get_head_entry(ls);
	for (i = 0; i < n && entry != 0; i++) {
	    entry = ls_entry_get_prev(entry);
	}

	if (entry != 0) {
		data = entry->data;
		ll_pop_entry(entry);
        ll_push_head(ls->free_list, entry);
	}

	return data;
}

void *ls_pop_tail(ls_t * ls)
{
    ls_entry_t *entry;
    void *data;

    entry = ll_pop_tail(ls->data_list);
    if (entry != 0) {
        data = entry->data;
        ll_push_head(ls->free_list, entry);
    } else {
        data = 0;
    }

    return data;
}

ls_entry_t * ls_get_head_entry(ls_t *ls)
{
    return ls->data_list->head;
}

ls_entry_t * ls_get_tail_entry(ls_t *ls)
{
    return ls->data_list->tail;
}

ls_entry_t * ls_entry_get_next(ls_entry_t *entry)
{
    return entry->next;
}

ls_entry_t * ls_entry_get_prev(ls_entry_t *entry)
{
    return entry->prev;
}

void *ls_entry_get_data(ls_entry_t *entry)
{
    return entry->data;
}

ls_entry_t* ls_get_head_entry_n(ls_t *ls, int n)
{
    ls_entry_t *entry = ls->data_list->head;
    int index = 0;

    while(entry != 0 && index++ < n)
    {
        entry = entry->prev;
    }

    return entry;
}

ls_entry_t* ls_get_tail_entry_n(ls_t *ls, int n)
{
    ls_entry_t *entry = ls->data_list->tail;
    int index = 0;

    while(entry != 0 && index++ < n)
    {
        entry = entry->next;
    }

    return entry;
}

int ls_size(ls_t *ls)
{
    int size = 0;
    ls_entry_t *entry = ls->data_list->head;

    while(entry != 0)
    {
        size++;
        entry = entry->prev;
    }
    return size;
}

void *ls_entry_delete(ls_entry_t *entry) 
{
    void *data = ls_entry_get_data(entry);
    ll_pop_entry(entry);
	ll_push_head(entry->ll->ls->free_list, entry);

    return data;
}
