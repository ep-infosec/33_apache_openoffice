/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/svwin.h>
#ifdef __MINGW32__
#include <excpt.h>
#endif
#include <win/saldata.hxx>
#include <win/saltimer.h>
#include <win/salinst.h>

// =======================================================================

// Maximale Periode
#define MAX_SYSPERIOD	  65533

// =======================================================================

void ImplSalStartTimer( sal_uLong nMS, sal_Bool bMutex )
{
	SalData* pSalData = GetSalData();

	// Remenber the time of the timer
	pSalData->mnTimerMS = nMS;
	if ( !bMutex )
		pSalData->mnTimerOrgMS = nMS;

	// Periode darf nicht zu gross sein, da Windows mit sal_uInt16 arbeitet
	if ( nMS > MAX_SYSPERIOD )
		nMS = MAX_SYSPERIOD;

	// Gibt es einen Timer, dann zerstoren
	if ( pSalData->mnTimerId )
		KillTimer( 0, pSalData->mnTimerId );

	// Make a new timer with new period
	pSalData->mnTimerId = SetTimer( 0, 0, (UINT)nMS, SalTimerProc );
	pSalData->mnNextTimerTime = pSalData->mnLastEventTime + nMS;
}

// -----------------------------------------------------------------------

WinSalTimer::~WinSalTimer()
{
}

void WinSalTimer::Start( sal_uLong nMS )
{
    // switch to main thread
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

void WinSalTimer::Stop()
{
	SalData* pSalData = GetSalData();

	// If we have a timer, than
	if ( pSalData->mnTimerId )
	{
		KillTimer( 0, pSalData->mnTimerId );
		pSalData->mnTimerId = 0;
		pSalData->mnNextTimerTime = 0;
	}
}

// -----------------------------------------------------------------------

void CALLBACK SalTimerProc( HWND, UINT, UINT_PTR nId, DWORD )
{
#ifdef __MINGW32__
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
	    SalData* pSalData = GetSalData();
        ImplSVData* pSVData = ImplGetSVData();

	    // Test for MouseLeave
	    SalTestMouseLeave();
        
        bool bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
	    if ( pSVData->mpSalTimer && ! bRecursive )
	    {
		    // Try to acquire the mutex. If we don't get the mutex then we
		    // try this a short time later again.
		    if ( ImplSalYieldMutexTryToAcquire() )
		    {
                bRecursive = pSalData->mbInTimerProc && (nId != SALTIMERPROC_RECURSIVE);
				if ( pSVData->mpSalTimer && ! bRecursive )
				{
					pSalData->mbInTimerProc = TRUE;
					pSVData->mpSalTimer->CallCallback();
					pSalData->mbInTimerProc = FALSE;
					ImplSalYieldMutexRelease();
	
					// Run the timer in the correct time, if we start this
					// with a small timeout, because we don't get the mutex
					if ( pSalData->mnTimerId &&
						(pSalData->mnTimerMS != pSalData->mnTimerOrgMS) )
						ImplSalStartTimer( pSalData->mnTimerOrgMS, FALSE );
				}
		    }
		    else
			    ImplSalStartTimer( 10, TRUE );
	    }
    }
#ifdef __MINGW32__
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
}
