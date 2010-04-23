/************************************************************************
 *
 * Copyright 2007 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

/*************************************************************************
    Description : using linked list to provide stack and fifo api
*************************************************************************/

#ifndef __LS_HEADER__
#define __LS_HEADER__

typedef struct ls_struct	ls_t;
typedef struct ls_entry_struct	ls_entry_t;

extern ls_t *		ls_alloc(void);
extern void         ls_free(ls_t* ls);
extern void 		ls_push_head(ls_t *ls, void *data);
extern void 		ls_push_tail(ls_t *ls, void *data);
extern void *		ls_pop_head(ls_t *ls);
extern void *		ls_pop_tail(ls_t *ls);
extern void *		ls_pop_head_n(ls_t *ls, int n);

extern ls_entry_t *	ls_get_head_entry(ls_t *ls);
extern ls_entry_t *	ls_get_tail_entry(ls_t *ls);
extern ls_entry_t * ls_get_head_entry_n(ls_t *ls, int n);
extern ls_entry_t * ls_get_tail_entry_n(ls_t *ls, int n);

extern ls_entry_t *	ls_entry_get_next(ls_entry_t *entry);
extern ls_entry_t *	ls_entry_get_prev(ls_entry_t *entry);
extern void *		ls_entry_get_data(ls_entry_t *entry);
extern void *		ls_entry_delete(ls_entry_t *entry); 

extern int          ls_size(ls_t *ls);

#define ls_foreach_head2tail(ls, data) { \
    ls_entry_t *entry; \
    for (entry = ls_get_head_entry(ls); entry != 0; entry = ls_entry_get_prev(entry)) { \
        data = ls_entry_get_data(entry);
#define ls_foreach_head2tail_end } }

#define ls_foreach_entry_head2tail(ls, entry) { \
    for (entry = ls_get_head_entry(ls); entry != 0; entry = ls_entry_get_prev(entry)) {
#define ls_foreach_entry_head2tail_end } }

#define ls_foreach_tail2head(ls, data) { \
    ls_entry_t *entry; \
    for (entry = ls_get_tail_entry(ls); entry != 0; entry = ls_entry_get_next(entry)) { \
        data = ls_entry_get_data(entry);
#define ls_foreach_tail2head_end } }

#define ls_foreach_entry_tail2head(ls, entry) { \
    for (entry = ls_get_tail_entry(ls); entry != 0; entry = ls_entry_get_next(entry)) {
#define ls_foreach_entry_tail2head_end } }

#endif
