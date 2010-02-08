// PtsClock.h: interface for the PtsClock.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PTSCLOCK_H__0749C5ED_CB35_473C_B4F1_8DE42837390A__INCLUDED_)
#define AFX_PTSCLOCK_H__0749C5ED_CB35_473C_B4F1_8DE42837390A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define I64C(x)		x##i64
#define UI64C(x)		x##ui64

typedef INT64 mtime_t;

/* clock.c */
enum /* Synchro states */
{
    SYNCHRO_OK     = 0,
    SYNCHRO_START  = 1,
    SYNCHRO_REINIT = 2,
};

typedef struct
{
    /* Synchronization information */
    mtime_t                 delta_cr;
    mtime_t                 cr_ref, sysdate_ref;
    mtime_t                 last_cr; /* reference to detect unexpected stream
                                      * discontinuities                      */
    mtime_t                 last_pts;
    unsigned long           c_average_count;
                           /* counter used to compute dynamic average values */
    int                     i_synchro_state;

    /* Config */
    int                     i_cr_average;
} input_clock_t;

void input_ClockInit( input_clock_t *, int i_cr_average );
void    input_ClockSetPCR(input_clock_t *, mtime_t );
mtime_t input_ClockGetTS(input_clock_t *, mtime_t );
mtime_t mdate();

void GetRefTime(input_clock_t * pClock, UINT64 *pRefPts, UINT64 *pRefTime);

#endif // !defined(AFX_PTSCLOCK_H__0749C5ED_CB35_473C_B4F1_8DE42837390A__INCLUDED_)
