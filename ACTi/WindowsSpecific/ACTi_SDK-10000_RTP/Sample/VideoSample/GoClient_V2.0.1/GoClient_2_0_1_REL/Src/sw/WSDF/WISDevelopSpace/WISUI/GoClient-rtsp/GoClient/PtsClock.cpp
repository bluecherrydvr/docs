// PtsClock.cpp: implementation of the PtsClock.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PtsClock.h"

/* Maximum gap allowed between two CRs. */
#define CR_MAX_GAP 2000000

/* Latency introduced on DVDs with CR == 0 on chapter change - this is from
 * my dice --Meuuh */
#define CR_MEAN_PTS_GAP 300000

static void ClockNewRef( input_clock_t * p_pgrm,
                         mtime_t i_clock, mtime_t i_sysdate );

/*****************************************************************************
 * ClockToSysdate: converts a movie clock to system date
 *****************************************************************************/
static mtime_t ClockToSysdate(input_clock_t *cl, mtime_t i_clock )
{
    mtime_t     i_sysdate = 0;

    if( cl->i_synchro_state == SYNCHRO_OK )
    {
        i_sysdate = (mtime_t)(i_clock - cl->cr_ref)
                        * (mtime_t)1000
                        * (mtime_t)300;
        i_sysdate /= 27;
        i_sysdate /= 1000;
        i_sysdate += (mtime_t)cl->sysdate_ref;
    }

    return( i_sysdate );
}

/*****************************************************************************
 * ClockCurrent: converts current system date to clock units
 *****************************************************************************
 * Caution : the synchro state must be SYNCHRO_OK for this to operate.
 *****************************************************************************/
static mtime_t ClockCurrent(input_clock_t *cl )
{
    return( (mdate() - cl->sysdate_ref) * 27 / 300
             + cl->cr_ref );
}

/*****************************************************************************
 * ClockNewRef: writes a new clock reference
 *****************************************************************************/
static void ClockNewRef( input_clock_t *cl,
                         mtime_t i_clock, mtime_t i_sysdate )
{
    cl->cr_ref = i_clock;
    cl->sysdate_ref = i_sysdate ;
}

/*****************************************************************************
 * input_ClockInit: reinitializes the clock reference after a stream
 *                  discontinuity
 *****************************************************************************/
void input_ClockInit( input_clock_t *cl, int i_cr_average )
{
    cl->i_synchro_state = SYNCHRO_START;

    cl->last_cr = 0;
    cl->last_pts = 0;
    cl->cr_ref = 0;
    cl->sysdate_ref = 0;
    cl->delta_cr = 0;
    cl->c_average_count = 0;

    cl->i_cr_average = i_cr_average;
}

/*****************************************************************************
 * input_ClockSetPCR: manages a clock reference
 *****************************************************************************/
void input_ClockSetPCR(input_clock_t *cl, mtime_t i_clock )
{
    if( ( cl->i_synchro_state != SYNCHRO_OK ) ||
        ( i_clock == 0 && cl->last_cr != 0 ) )
    {
        /* Feed synchro with a new reference point. */
        ClockNewRef( cl, i_clock,
                     cl->last_pts + CR_MEAN_PTS_GAP > mdate() ?
                     cl->last_pts + CR_MEAN_PTS_GAP : mdate() );
        cl->i_synchro_state = SYNCHRO_OK;

        cl->last_cr = 0;
        cl->delta_cr = 0;
        cl->c_average_count = 0;
    }
    else
    {
        if ( cl->last_cr != 0 &&
               (    (cl->last_cr - i_clock) > CR_MAX_GAP
                 || (cl->last_cr - i_clock) < - CR_MAX_GAP ) )
        {
            /* Stream discontinuity, for which we haven't received a
             * warning from the stream control facilities (dd-edited
             * stream ?). */
            //msg_Warn( p_input, "clock gap, unexpected stream discontinuity" );
            input_ClockInit( cl, cl->i_cr_average );
            /* FIXME needed ? */
        }

        cl->last_cr = i_clock;

        /* Smooth clock reference variations. */
        mtime_t i_extrapoled_clock = ClockCurrent(cl);

        /* Bresenham algorithm to smooth variations. */
        cl->delta_cr = ( cl->delta_cr * (cl->i_cr_average - 1)
                           + ( i_extrapoled_clock - i_clock ) )
                       / cl->i_cr_average;
    }
}

/*****************************************************************************
 * input_ClockGetTS: manages a PTS or DTS
 *****************************************************************************/
mtime_t input_ClockGetTS(input_clock_t *cl, mtime_t i_ts )
{
    if( cl->i_synchro_state != SYNCHRO_OK )
        return 0;

    cl->last_pts = ClockToSysdate(cl, i_ts + cl->delta_cr );
    return cl->last_pts + 300000;
}

mtime_t mdate()
{
    /* We don't need the real date, just the value of a high precision timer */
    static UINT64 freq = I64C(-1);
    UINT64 usec_time;
	
    if( freq == I64C(-1) )
    {
	/* Extract from the Tcl source code:
	* (http://www.cs.man.ac.uk/fellowsd-bin/TIP/7.html)
	*
	* Some hardware abstraction layers use the CPU clock
	* in place of the real-time clock as a performance counter
	* reference.  This results in:
	*    - inconsistent results among the processors on
	*      multi-processor systems.
	*    - unpredictable changes in performance counter frequency
	*      on "gearshift" processors such as Transmeta and
	*      SpeedStep.
	* There seems to be no way to test whether the performance
	* counter is reliable, but a useful heuristic is that
	* if its frequency is 1.193182 MHz or 3.579545 MHz, it's
	* derived from a colorburst crystal and is therefore
	* the RTC rather than the TSC.  If it's anything else, we
	* presume that the performance counter is unreliable.
		*/
		
        freq = ( QueryPerformanceFrequency( (LARGE_INTEGER *)&freq ) &&
			(freq == I64C(1193182) || freq == I64C(3579545) ) )
			? freq : 0;
    }
	
    if( freq != 0 )
    {
        /* Microsecond resolution */
        QueryPerformanceCounter( (LARGE_INTEGER *)&usec_time );
        return ( usec_time * 1000000 ) / freq;
    }
    else
    {
	/* Fallback on GetTickCount() which has a milisecond resolution
	* (actually, best case is about 10 ms resolution)
	* GetTickCount() only returns a DWORD thus will wrap after
		* about 49.7 days so we try to detect the wrapping. */
		
        static CRITICAL_SECTION date_lock;
        static UINT64 i_previous_time = I64C(-1);
        static int i_wrap_counts = -1;
		
        if( i_wrap_counts == -1 )
        {
            /* Initialization */
            i_previous_time = I64C(1000) * GetTickCount();
            InitializeCriticalSection( &date_lock );
            i_wrap_counts = 0;
        }
		
        EnterCriticalSection( &date_lock );
        usec_time = I64C(1000) *
            (i_wrap_counts * I64C(0x100000000) + GetTickCount());
        if( i_previous_time > usec_time )
        {
            /* Counter wrapped */
            i_wrap_counts++;
            usec_time += I64C(0x100000000000);
        }
        i_previous_time = usec_time;
        LeaveCriticalSection( &date_lock );
		
        return usec_time;
	}
}

void GetRefTime(input_clock_t *pClock, UINT64 *pRefPts, UINT64 *pRefTime)
{
	*pRefPts = pClock->cr_ref;
	*pRefTime = pClock->sysdate_ref;
}