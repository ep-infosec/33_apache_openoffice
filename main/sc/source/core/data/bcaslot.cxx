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
#include "precompiled_sc.hxx"

#include <boost/mem_fn.hpp>

#include <sfx2/objsh.hxx>
#include <svl/listener.hxx>
#include <svl/listeneriter.hxx>

#include "document.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "scerrors.hxx"
#include "docoptio.hxx"
#include "refupdat.hxx"
#include "table.hxx"

// Number of slots per dimension
// must be integer divisors of MAXCOLCOUNT respectively MAXROWCOUNT
#define BCA_SLOTS_COL ((MAXCOLCOUNT_DEFINE) / 16)
#if MAXROWCOUNT_DEFINE == 32000
#define BCA_SLOTS_ROW 256
#define BCA_SLICE 125
#else
#define BCA_SLICE 128
#define BCA_SLOTS_ROW ((MAXROWCOUNT_DEFINE) / BCA_SLICE)
#endif
#define BCA_SLOT_COLS ((MAXCOLCOUNT_DEFINE) / BCA_SLOTS_COL)
#define BCA_SLOT_ROWS ((MAXROWCOUNT_DEFINE) / BCA_SLOTS_ROW)
// multiple?
#if (BCA_SLOT_COLS * BCA_SLOTS_COL) != (MAXCOLCOUNT_DEFINE)
#error bad BCA_SLOTS_COL value!
#endif
#if (BCA_SLOT_ROWS * BCA_SLOTS_ROW) != (MAXROWCOUNT_DEFINE)
#error bad BCA_SLOTS_ROW value!
#endif
// size of slot array if linear
#define BCA_SLOTS_DEFINE (BCA_SLOTS_COL * BCA_SLOTS_ROW)
// Arbitrary 2**31/8, assuming size_t can hold at least 2^31 values and
// sizeof_ptr is at most 8 bytes. You'd probably doom your machine's memory
// anyway, once you reached these values..
#if BCA_SLOTS_DEFINE > 268435456
#error BCA_SLOTS_DEFINE DOOMed!
#endif

// STATIC DATA -----------------------------------------------------------

TYPEINIT1( ScHint, SfxSimpleHint );
TYPEINIT1( ScAreaChangedHint, SfxHint );

struct ScSlotData
{
    SCROW  nStartRow;   // first row of this segment
    SCROW  nStopRow;    // first row of next segment
    SCSIZE nSlice;      // slice size in this segment
    SCSIZE nCumulated;  // cumulated slots of previous segments

    ScSlotData( SCROW r1, SCROW r2, SCSIZE s, SCSIZE c ) : nStartRow(r1), nStopRow(r2), nSlice(s), nCumulated(c) {}
};
typedef ::std::vector< ScSlotData > ScSlotDistribution;
#if MAXROWCOUNT_DEFINE <= 65536
// Linear distribution.
static ScSlotDistribution aSlotDistribution( ScSlotData( 0, MAXROWCOUNT, BCA_SLOT_ROWS, 0));
static SCSIZE nBcaSlotsRow = BCA_SLOTS_ROW;
static SCSIZE nBcaSlots = BCA_SLOTS_DEFINE;
#else
// Logarithmic or any other distribution.
// Upper sheet part usually is more populated and referenced and gets fine 
// grained resolution, larger data in larger hunks.
// Could be further enhanced by also applying a different distribution of 
// column slots.
static SCSIZE initSlotDistribution( ScSlotDistribution & rSD, SCSIZE & rBSR )
{
    SCSIZE nSlots = 0;
    SCROW nRow1 = 0;
    SCROW nRow2 = 32*1024;
    SCSIZE nSlice = 128;
    // Must be sorted by row1,row2!
    while (nRow2 <= MAXROWCOUNT)
    {
        //fprintf( stderr, "r1,r2,slice,cum: %7zu, %7zu, %7zu, %7zu\n", (size_t)nRow1, (size_t)nRow2, (size_t)nSlice, (size_t)nSlots);
        // {0,32k,128,0;32k,64k,256,0+256;64k,128k,512,0+256+128;128k,256k,1024,0+256+128+128;256k,512k,2048,...;512k,1M,4096,...}
        rSD.push_back( ScSlotData( nRow1, nRow2, nSlice, nSlots));
        nSlots += (nRow2 - nRow1) / nSlice;
        nRow1 = nRow2;
        nRow2 *= 2;
        nSlice *= 2;
    }
    //fprintf( stderr, "Slices: %zu, slots per sheet: %zu, memory per referenced sheet: %zu\n", (size_t) nSlots, (size_t) nSlots * BCA_SLOTS_COL, (size_t) nSlots * BCA_SLOTS_COL * sizeof(void*));
    rBSR = nSlots;
    return nSlots;
}
static ScSlotDistribution aSlotDistribution;
static SCSIZE nBcaSlotsRow;
static SCSIZE nBcaSlots = initSlotDistribution( aSlotDistribution, nBcaSlotsRow) * BCA_SLOTS_COL;
// Ensure that all static variables are initialized with this one call.
#endif


ScBroadcastAreaSlot::ScBroadcastAreaSlot( ScDocument* pDocument,
        ScBroadcastAreaSlotMachine* pBASMa ) :
    aTmpSeekBroadcastArea( ScRange()),
    pDoc( pDocument ),
    pBASM( pBASMa )
{
}


ScBroadcastAreaSlot::~ScBroadcastAreaSlot()
{
    for ( ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* none */)
    {
        // Prevent hash from accessing dangling pointer in case area is
        // deleted.
        ScBroadcastArea* pArea = *aIter;
        // Erase all so no hash will be accessed upon destruction of the
        // hash_set.
        aBroadcastAreaTbl.erase( aIter++);
        if (!pArea->DecRef())
            delete pArea;
    }
}


bool ScBroadcastAreaSlot::CheckHardRecalcStateCondition() const
{
    if ( pDoc->GetHardRecalcState() )
        return true;
    if (aBroadcastAreaTbl.size() >= aBroadcastAreaTbl.max_size())
    {   // this is more hypothetical now, check existed for old SV_PTRARR_SORT
        if ( !pDoc->GetHardRecalcState() )
        {
            pDoc->SetHardRecalcState( 1 );

            SfxObjectShell* pShell = pDoc->GetDocumentShell();
            DBG_ASSERT( pShell, "Missing DocShell :-/" );

			if ( pShell )
				pShell->SetError( SCWARN_CORE_HARD_RECALC, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

            pDoc->SetAutoCalc( sal_False );
            pDoc->SetHardRecalcState( 2 );
        }
        return true;
    }
    return false;
}


bool ScBroadcastAreaSlot::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    bool bNewArea = false;
    DBG_ASSERT(pListener, "StartListeningArea: pListener Null");
    if (CheckHardRecalcStateCondition())
        return false;
    if ( !rpArea )
    {
        // Even if most times the area doesn't exist yet and immediately trying 
        // to new and insert it would save an attempt to find it, on mass 
        // operations like identical large [HV]LOOKUP() areas the new/delete 
        // would add quite some penalty for all but the first formula cell.
        ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
        if (aIter != aBroadcastAreaTbl.end())
            rpArea = *aIter;
        else
        {
            rpArea = new ScBroadcastArea( rRange);
            if (aBroadcastAreaTbl.insert( rpArea).second)
            {
                rpArea->IncRef();
                bNewArea = true;
            }
            else
            {
                DBG_ERRORFILE("StartListeningArea: area not found and not inserted in slot?!?");
                delete rpArea;
                rpArea = 0;
            }
        }
        if (rpArea)
            pListener->StartListening( rpArea->GetBroadcaster());
    }
    else
    {
        if (aBroadcastAreaTbl.insert( rpArea).second)
            rpArea->IncRef();
    }
    return bNewArea;
}


void ScBroadcastAreaSlot::InsertListeningArea( ScBroadcastArea* pArea )
{
    DBG_ASSERT( pArea, "InsertListeningArea: pArea NULL");
    if (CheckHardRecalcStateCondition())
        return;
    if (aBroadcastAreaTbl.insert( pArea).second)
        pArea->IncRef();
}


// If rpArea != NULL then no listeners are stopped, only the area is removed
// and the reference count decremented.
void ScBroadcastAreaSlot::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    DBG_ASSERT(pListener, "EndListeningArea: pListener Null");
    if ( !rpArea )
    {
        ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
        if (aIter == aBroadcastAreaTbl.end())
            return;
        rpArea = *aIter;
        pListener->EndListening( rpArea->GetBroadcaster() );
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {   // if nobody is listening we can dispose it
            aBroadcastAreaTbl.erase( aIter);
            if ( !rpArea->DecRef() )
            {
                delete rpArea;
                rpArea = NULL;
            }
        }
    }
    else
    {
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {
            ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
            if (aIter == aBroadcastAreaTbl.end())
                return;
            DBG_ASSERT( *aIter == rpArea, "EndListeningArea: area pointer mismatch");
            aBroadcastAreaTbl.erase( aIter);
            if ( !rpArea->DecRef() )
            {
                delete rpArea;
                rpArea = NULL;
            }
        }
    }
}


ScBroadcastAreas::const_iterator ScBroadcastAreaSlot::FindBroadcastArea(
        const ScRange& rRange ) const
{
    aTmpSeekBroadcastArea.UpdateRange( rRange);
    return aBroadcastAreaTbl.find( &aTmpSeekBroadcastArea);
}


sal_Bool ScBroadcastAreaSlot::AreaBroadcast( const ScHint& rHint) const
{
    if (aBroadcastAreaTbl.empty())
        return sal_False;
    sal_Bool bIsBroadcasted = sal_False;

    // issue 118012 
    // do not iterate on <aBoardcastAreaTbl> as its reveals that its iterators
    // are destroyed during notification.
    std::vector< ScBroadcastArea* > aCopyForIteration( aBroadcastAreaTbl.begin(), aBroadcastAreaTbl.end() );
    std::for_each( aCopyForIteration.begin(), aCopyForIteration.end(), boost::mem_fn( &ScBroadcastArea::IncRef ) );

    const ScAddress& rAddress = rHint.GetAddress();
    const std::vector< ScBroadcastArea* >::const_iterator aEnd( aCopyForIteration.end() );
    std::vector< ScBroadcastArea* >::const_iterator aIter;
    for ( aIter = aCopyForIteration.begin(); aIter != aEnd; ++aIter )
    {
        ScBroadcastArea* pArea = *aIter;
        // check, if copied item has been already removed from <aBroadcastAreaTbl>
        if ( aBroadcastAreaTbl.find( pArea ) == aBroadcastAreaTbl.end() )
        {
            continue;
        }

        const ScRange& rAreaRange = pArea->GetRange();
        if (rAreaRange.In( rAddress))
        {
            if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
            {
                pArea->GetBroadcaster().Broadcast( rHint);
                bIsBroadcasted = sal_True;
            }
        }
    }

    // delete no longer referenced <ScBroadcastArea> instances
    for ( aIter = aCopyForIteration.begin(); aIter != aEnd; ++aIter )
    {
        ScBroadcastArea* pArea = *aIter;
        if ( !pArea->DecRef() )
        {
            delete pArea;
        }
    }

    return bIsBroadcasted;
}


sal_Bool ScBroadcastAreaSlot::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint) const
{
    if (aBroadcastAreaTbl.empty())
        return sal_False;
    sal_Bool bIsBroadcasted = sal_False;

    // issue 118012 
    // do not iterate on <aBoardcastAreaTbl> as its reveals that its iterators
    // are destroyed during notification.
    std::vector< ScBroadcastArea* > aCopyForIteration( aBroadcastAreaTbl.begin(), aBroadcastAreaTbl.end() );
    std::for_each( aCopyForIteration.begin(), aCopyForIteration.end(), boost::mem_fn( &ScBroadcastArea::IncRef ) );

    const std::vector< ScBroadcastArea* >::const_iterator aEnd( aCopyForIteration.end() );
    std::vector< ScBroadcastArea* >::const_iterator aIter;
    for ( aIter = aCopyForIteration.begin(); aIter != aEnd; ++aIter )
    {
        ScBroadcastArea* pArea = *aIter;
        // check, if copied item has been already removed from <aBroadcastAreaTbl>
        if ( aBroadcastAreaTbl.find( pArea ) == aBroadcastAreaTbl.end() )
        {
            continue;
        }

        const ScRange& rAreaRange = pArea->GetRange();
        if (rAreaRange.Intersects( rRange ))
        {
            if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
            {
                pArea->GetBroadcaster().Broadcast( rHint);
                bIsBroadcasted = sal_True;
            }
        }
    }

    // delete no longer referenced <ScBroadcastArea> instances
    for ( aIter = aCopyForIteration.begin(); aIter != aEnd; ++aIter )
    {
        ScBroadcastArea* pArea = *aIter;
        if ( !pArea->DecRef() )
        {
            delete pArea;
        }
    }

    return bIsBroadcasted;
}


void ScBroadcastAreaSlot::DelBroadcastAreasInRange( const ScRange& rRange )
{
    if (aBroadcastAreaTbl.empty())
        return;
    for (ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* increment in body */ )
    {
        const ScRange& rAreaRange = (*aIter)->GetRange();
        if (rRange.In( rAreaRange))
        {
            ScBroadcastArea* pArea = *aIter;
            aBroadcastAreaTbl.erase( aIter++);  // erase before modifying
            if (!pArea->DecRef())
            {
                if (pBASM->IsInBulkBroadcast())
                    pBASM->RemoveBulkArea( pArea);
                delete pArea;
            }
        }
        else
            ++aIter;
    }
}


void ScBroadcastAreaSlot::UpdateRemove( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if (aBroadcastAreaTbl.empty())
        return;

    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
    for ( ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* increment in body */ )
    {
        ScBroadcastArea* pArea = *aIter;
        if ( pArea->IsInUpdateChain() )
        {
            aBroadcastAreaTbl.erase( aIter++);
            pArea->DecRef();
        }
        else
        {
            pArea->GetRange().GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);
            if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ))
            {
                aBroadcastAreaTbl.erase( aIter++);
                pArea->DecRef();
                if (pBASM->IsInBulkBroadcast())
                    pBASM->RemoveBulkArea( pArea);
                pArea->SetInUpdateChain( sal_True );
                ScBroadcastArea* pUC = pBASM->GetEOUpdateChain();
                if ( pUC )
                    pUC->SetUpdateChainNext( pArea );
                else    // no tail => no head
                    pBASM->SetUpdateChain( pArea );
                pBASM->SetEOUpdateChain( pArea );
            }
            else
                ++aIter;
        }
    }
}


void ScBroadcastAreaSlot::UpdateRemoveArea( ScBroadcastArea* pArea )
{
    ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.find( pArea));
    if (aIter == aBroadcastAreaTbl.end())
        return;
    if (*aIter != pArea)
        DBG_ERRORFILE( "UpdateRemoveArea: area pointer mismatch");
    else
    {
        aBroadcastAreaTbl.erase( aIter);
        pArea->DecRef();
    }
}


void ScBroadcastAreaSlot::UpdateInsert( ScBroadcastArea* pArea )
{
    ::std::pair< ScBroadcastAreas::iterator, bool > aPair = 
        aBroadcastAreaTbl.insert( pArea );
    if (aPair.second)
        pArea->IncRef();
    else
    {
        // Identical area already exists, add listeners.
        ScBroadcastArea* pTarget = *(aPair.first);
        if (pArea != pTarget)
        {
            SvtBroadcaster& rTarget = pTarget->GetBroadcaster();
            SvtListenerIter it( pArea->GetBroadcaster());
            for (SvtListener* pListener = it.GetCurr(); pListener;
                    pListener = it.GoNext())
            {
                pListener->StartListening( rTarget);
            }
        }
    }
}


// --- ScBroadcastAreaSlotMachine -------------------------------------

ScBroadcastAreaSlotMachine::TableSlots::TableSlots()
{
    ppSlots = new ScBroadcastAreaSlot* [ nBcaSlots ];
    memset( ppSlots, 0 , sizeof( ScBroadcastAreaSlot* ) * nBcaSlots );
}


ScBroadcastAreaSlotMachine::TableSlots::~TableSlots()
{
    for ( ScBroadcastAreaSlot** pp = ppSlots + nBcaSlots; --pp >= ppSlots; /* nothing */ )
    {
        if (*pp)
            delete *pp;
    }
    delete [] ppSlots;
}


ScBroadcastAreaSlotMachine::ScBroadcastAreaSlotMachine(
        ScDocument* pDocument ) :
    pBCAlways( NULL ),
    pDoc( pDocument ),
    pUpdateChain( NULL ),
    pEOUpdateChain( NULL ),
    nInBulkBroadcast( 0 )
{
}


ScBroadcastAreaSlotMachine::~ScBroadcastAreaSlotMachine()
{
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.begin());
            iTab != aTableSlotsMap.end(); ++iTab)
    {
        delete (*iTab).second;
    }
    delete pBCAlways;
}


inline SCSIZE ScBroadcastAreaSlotMachine::ComputeSlotOffset(
        const ScAddress& rAddress ) const
{
    SCROW nRow = rAddress.Row();
    SCCOL nCol = rAddress.Col();
    if ( !ValidRow(nRow) || !ValidCol(nCol) )
    {
        DBG_ERRORFILE( "Row/Col invalid, using first slot!" );
        return 0;
    }
    for (size_t i=0; i < aSlotDistribution.size(); ++i)
    {
        if (nRow < aSlotDistribution[i].nStopRow)
        {
            const ScSlotData& rSD = aSlotDistribution[i];
            return rSD.nCumulated +
                (static_cast<SCSIZE>(nRow - rSD.nStartRow)) / rSD.nSlice +
                static_cast<SCSIZE>(nCol) / BCA_SLOT_COLS * nBcaSlotsRow;
        }
    }
    DBG_ERRORFILE( "No slot found, using last!" );
    return nBcaSlots - 1;
}


void ScBroadcastAreaSlotMachine::ComputeAreaPoints( const ScRange& rRange,
        SCSIZE& rStart, SCSIZE& rEnd, SCSIZE& rRowBreak ) const
{
    rStart = ComputeSlotOffset( rRange.aStart );
    rEnd = ComputeSlotOffset( rRange.aEnd );
    // count of row slots per column minus one
    rRowBreak = ComputeSlotOffset(
        ScAddress( rRange.aStart.Col(), rRange.aEnd.Row(), 0 ) ) - rStart;
}


inline void ComputeNextSlot( SCSIZE & nOff, SCSIZE & nBreak, ScBroadcastAreaSlot** & pp,
        SCSIZE & nStart, ScBroadcastAreaSlot** const & ppSlots, SCSIZE const & nRowBreak )
{
    if ( nOff < nBreak )
    {
        ++nOff;
        ++pp;
    }
    else
    {
        nStart += nBcaSlotsRow;
        nOff = nStart;
        pp = ppSlots + nOff;
        nBreak = nOff + nRowBreak;
    }
}


void ScBroadcastAreaSlotMachine::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener )
{
    //fprintf( stderr, "StartListeningArea (c,r,t): %d, %d, %d, %d, %d, %d\n", (int)rRange.aStart.Col(), (int)rRange.aStart.Row(), (int)rRange.aStart.Tab(), (int)rRange.aEnd.Col(), (int)rRange.aEnd.Row(), (int)rRange.aEnd.Tab());
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( !pBCAlways )
            pBCAlways = new SvtBroadcaster;
        pListener->StartListening( *pBCAlways );
    }
    else
    {
        bool bDone = false;
        for (SCTAB nTab = rRange.aStart.Tab();
                !bDone && nTab <= rRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.insert( TableSlotsMap::value_type(
                            nTab, new TableSlots)).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            ScBroadcastArea* pArea = NULL;
            while ( !bDone && nOff <= nEnd )
            {
                if ( !*pp )
                    *pp = new ScBroadcastAreaSlot( pDoc, this );
                if (!pArea)
                {
                    // If the call to StartListeningArea didn't create the 
                    // ScBroadcastArea, listeners were added to an already 
                    // existing identical area that doesn't need to be inserted 
                    // to slots again.
                    if (!(*pp)->StartListeningArea( rRange, pListener, pArea))
                        bDone = true;
                }
                else
                    (*pp)->InsertListeningArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
    }
}


void ScBroadcastAreaSlotMachine::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener )
{
    //fprintf( stderr, "EndListeningArea (c,r,t): %d, %d, %d, %d, %d, %d\n", (int)rRange.aStart.Col(), (int)rRange.aStart.Row(), (int)rRange.aStart.Tab(), (int)rRange.aEnd.Col(), (int)rRange.aEnd.Row(), (int)rRange.aEnd.Tab());
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        DBG_ASSERT( pBCAlways, "ScBroadcastAreaSlotMachine::EndListeningArea: BCA_LISTEN_ALWAYS but none established");
        if ( pBCAlways )
        {
            pListener->EndListening( *pBCAlways);
            if (!pBCAlways->HasListeners())
            {
                delete pBCAlways;
                pBCAlways = NULL;
            }
        }
    }
    else
    {
        SCTAB nEndTab = rRange.aEnd.Tab();
        for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
                iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
        {
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            ScBroadcastArea* pArea = NULL;
            if (nOff == 0 && nEnd == nBcaSlots-1)
            {
                // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they 
                // happen for insertion and deletion of sheets.
                ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
                do
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, pListener, pArea );
                } while (++pp < pStop);
            }
            else
            {
                while ( nOff <= nEnd )
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, pListener, pArea );
                    ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
                }
            }
        }
    }
}


sal_Bool ScBroadcastAreaSlotMachine::AreaBroadcast( const ScHint& rHint ) const
{
    const ScAddress& rAddress = rHint.GetAddress();
    if ( rAddress == BCA_BRDCST_ALWAYS )
    {
        if ( pBCAlways )
        {
            pBCAlways->Broadcast( rHint );
            return sal_True;
        }
        else
            return sal_False;
    }
    else
    {
        TableSlotsMap::const_iterator iTab( aTableSlotsMap.find( rAddress.Tab()));
        if (iTab == aTableSlotsMap.end())
            return sal_False;
        ScBroadcastAreaSlot* pSlot = (*iTab).second->getAreaSlot( 
                ComputeSlotOffset( rAddress));
        if ( pSlot )
            return pSlot->AreaBroadcast( rHint );
        else
            return sal_False;
    }
}


sal_Bool ScBroadcastAreaSlotMachine::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint ) const
{
    sal_Bool bBroadcasted = sal_False;
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::const_iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                bBroadcasted |= (*pp)->AreaBroadcastInRange( rRange, rHint );
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
        }
    }
    return bBroadcasted;
}


void ScBroadcastAreaSlotMachine::DelBroadcastAreasInRange(
        const ScRange& rRange )
{
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == nBcaSlots-1)
        {
            // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they 
            // happen for insertion and deletion of sheets.
            ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
            do
            {
                if ( *pp )
                    (*pp)->DelBroadcastAreasInRange( rRange );
            } while (++pp < pStop);
        }
        else
        {
            while ( nOff <= nEnd )
            {
                if ( *pp )
                    (*pp)->DelBroadcastAreasInRange( rRange );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
    }
}


// for all affected: remove, chain, update range, insert, and maybe delete
void ScBroadcastAreaSlotMachine::UpdateBroadcastAreas(
        UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    // remove affected and put in chain
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == nBcaSlots-1)
        {
            // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they 
            // happen for insertion and deletion of sheets.
            ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
            do
            {
                if ( *pp )
                    (*pp)->UpdateRemove( eUpdateRefMode, rRange, nDx, nDy, nDz );
            } while (++pp < pStop);
        }
        else
        {
            while ( nOff <= nEnd )
            {
                if ( *pp )
                    (*pp)->UpdateRemove( eUpdateRefMode, rRange, nDx, nDy, nDz );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
    }

    // Updating an area's range will modify the hash key, remove areas from all 
    // affected slots. Will be reinserted later with the updated range.
    ScBroadcastArea* pChain = pUpdateChain;
    while (pChain)
    {
        ScBroadcastArea* pArea = pChain;
        pChain = pArea->GetUpdateChainNext();
        ScRange aRange( pArea->GetRange());
        // remove from slots
        for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab() && pArea->GetRef(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
            {
                DBG_ERRORFILE( "UpdateBroadcastAreas: Where's the TableSlot?!?");
                continue;   // for
            }
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( nOff <= nEnd && pArea->GetRef() )
            {
                if (*pp)
                    (*pp)->UpdateRemoveArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
        
    }

    // shift sheets
    if (nDz)
    {
        if (nDz < 0)
        {
            TableSlotsMap::iterator iDel( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab() - nDz));
            // Remove sheets, if any, iDel or/and iTab may as well point to end().
            while (iDel != iTab)
            {
                delete (*iDel).second;
                aTableSlotsMap.erase( iDel++);
            }
            // shift remaining down
            while (iTab != aTableSlotsMap.end())
            {
                SCTAB nTab = (*iTab).first + nDz;
                aTableSlotsMap[nTab] = (*iTab).second;
                aTableSlotsMap.erase( iTab++);
            }
        }
        else
        {
            TableSlotsMap::iterator iStop( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            if (iStop != aTableSlotsMap.end())
            {
                bool bStopIsBegin = (iStop == aTableSlotsMap.begin());
                if (!bStopIsBegin)
                    --iStop;
                TableSlotsMap::iterator iTab( aTableSlotsMap.end());
                --iTab;
                while (iTab != iStop)
                {
                    SCTAB nTab = (*iTab).first + nDz;
                    aTableSlotsMap[nTab] = (*iTab).second;
                    aTableSlotsMap.erase( iTab--);
                }
                // Shift the very first, iTab==iStop in this case.
                if (bStopIsBegin)
                {
                    SCTAB nTab = (*iTab).first + nDz;
                    aTableSlotsMap[nTab] = (*iTab).second;
                    aTableSlotsMap.erase( iStop);
                }
            }
        }
    }

    // work off chain
    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
    while ( pUpdateChain )
    {
        ScBroadcastArea* pArea = pUpdateChain;
        ScRange aRange( pArea->GetRange());
        pUpdateChain = pArea->GetUpdateChainNext();

        // update range
        aRange.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ))
        {
            aRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
            pArea->UpdateRange( aRange );
            pArea->GetBroadcaster().Broadcast( ScAreaChangedHint( aRange ) );   // for DDE
        }

        // insert to slots
        for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.insert( TableSlotsMap::value_type(
                            nTab, new TableSlots)).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( nOff <= nEnd )
            {
                if (!*pp)
                    *pp = new ScBroadcastAreaSlot( pDoc, this );
                (*pp)->UpdateInsert( pArea );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }

        // unchain
        pArea->SetUpdateChainNext( NULL );
        pArea->SetInUpdateChain( sal_False );

        // Delete if not inserted to any slot. RemoveBulkArea(pArea) was 
        // already executed in UpdateRemove().
        if (!pArea->GetRef())
            delete pArea;
    }
    pEOUpdateChain = NULL;
}


void ScBroadcastAreaSlotMachine::EnterBulkBroadcast()
{
    ++nInBulkBroadcast;
}


void ScBroadcastAreaSlotMachine::LeaveBulkBroadcast()
{
    if (nInBulkBroadcast > 0)
    {
        if (--nInBulkBroadcast == 0)
            ScBroadcastAreasBulk().swap( aBulkBroadcastAreas);
    }
}


bool ScBroadcastAreaSlotMachine::InsertBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.insert( pArea ).second;
}


size_t ScBroadcastAreaSlotMachine::RemoveBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.erase( pArea );
}
