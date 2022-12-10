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



#include <vcl/svapp.hxx>

#include "chartlis.hxx"
#include "brdcst.hxx"
#include "document.hxx"
#include "reftokenhelper.hxx"

using namespace com::sun::star;
using ::std::vector;
using ::std::list;
using ::std::hash_set;
using ::std::auto_ptr;
using ::std::unary_function;
using ::std::for_each;

//2do: DocOption TimeOut?
//#define SC_CHARTTIMEOUT 1000		// eine Sekunde keine Aenderung/KeyEvent

// Update chart listeners quickly, to get a similar behavior to loaded charts
// which register UNO listeners.
#define SC_CHARTTIMEOUT 10


// ====================================================================

class ScChartUnoData
{
	uno::Reference< chart::XChartDataChangeEventListener >	xListener;
	uno::Reference< chart::XChartData >						xSource;

public:
			ScChartUnoData( const uno::Reference< chart::XChartDataChangeEventListener >& rL,
							const uno::Reference< chart::XChartData >& rS ) :
					xListener( rL ), xSource( rS ) {}
			~ScChartUnoData() {}

	const uno::Reference< chart::XChartDataChangeEventListener >& GetListener() const	{ return xListener; }
	const uno::Reference< chart::XChartData >& GetSource() const						{ return xSource; }
};


// === ScChartListener ================================================

ScChartListener::ExternalRefListener::ExternalRefListener(ScChartListener& rParent, ScDocument* pDoc) :
    mrParent(rParent), mpDoc(pDoc)
{
}

ScChartListener::ExternalRefListener::~ExternalRefListener()
{
    if (!mpDoc || mpDoc->IsInDtorClear())
        // The document is being destroyed.  Do nothing.
        return;

    // Make sure to remove all pointers to this object.
    mpDoc->GetExternalRefManager()->removeLinkListener(this);
}

void ScChartListener::ExternalRefListener::notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType)
{
    switch (eType)
    {
        case ScExternalRefManager::LINK_MODIFIED:
        {
            if (maFileIds.count(nFileId))
                // We are listening to this external document.  Send an update 
                // requst to the chart.
                mrParent.SetUpdateQueue();
        }
        break;
        case ScExternalRefManager::LINK_BROKEN:
            removeFileId(nFileId);
        break;
    }
}

void ScChartListener::ExternalRefListener::addFileId(sal_uInt16 nFileId)
{
    maFileIds.insert(nFileId);
}

void ScChartListener::ExternalRefListener::removeFileId(sal_uInt16 nFileId)
{
    maFileIds.erase(nFileId);
}

hash_set<sal_uInt16>& ScChartListener::ExternalRefListener::getAllFileIds()
{
    return maFileIds;
}

// ----------------------------------------------------------------------------

ScChartListener::ScChartListener( const String& rName, ScDocument* pDocP,
        const ScRange& rRange ) :
    StrData( rName ),
    SvtListener(),
    mpExtRefListener(NULL), 
    mpTokens(new vector<ScSharedTokenRef>), 
    pUnoData( NULL ),
    pDoc( pDocP ),
    bUsed( sal_False ),
    bDirty( sal_False ),
    bSeriesRangesScheduled( sal_False )
{
    SetRangeList( rRange );
}

ScChartListener::ScChartListener( const String& rName, ScDocument* pDocP,
        const ScRangeListRef& rRangeList ) :
    StrData( rName ),
    SvtListener(),
    mpExtRefListener(NULL), 
    mpTokens(new vector<ScSharedTokenRef>), 
    pUnoData( NULL ),
    pDoc( pDocP ),
    bUsed( sal_False ),
    bDirty( sal_False ),
    bSeriesRangesScheduled( sal_False )
{
    ScRefTokenHelper::getTokensFromRangeList(*mpTokens, *rRangeList);
}

ScChartListener::ScChartListener( const String& rName, ScDocument* pDocP, vector<ScSharedTokenRef>* pTokens ) :
    StrData( rName ),
    SvtListener(),
    mpExtRefListener(NULL), 
    mpTokens(pTokens),
    pUnoData( NULL ),
    pDoc( pDocP ),
    bUsed( sal_False ),
    bDirty( sal_False ),
    bSeriesRangesScheduled( sal_False )
{
}

ScChartListener::ScChartListener( const ScChartListener& r ) :
    StrData( r ),
    SvtListener(),
    mpExtRefListener(NULL), 
    mpTokens(new vector<ScSharedTokenRef>(*r.mpTokens)),
    pUnoData( NULL ),
    pDoc( r.pDoc ),
    bUsed( sal_False ),
    bDirty( r.bDirty ),
    bSeriesRangesScheduled( r.bSeriesRangesScheduled )
{
    if ( r.pUnoData )
        pUnoData = new ScChartUnoData( *r.pUnoData );

    if (r.mpExtRefListener.get())
    {
        // Re-register this new listener for the files that the old listener 
        // was listening to.

        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const hash_set<sal_uInt16>& rFileIds = r.mpExtRefListener->getAllFileIds();
        mpExtRefListener.reset(new ExternalRefListener(*this, pDoc));
        hash_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
        {
            pRefMgr->addLinkListener(*itr, mpExtRefListener.get());
            mpExtRefListener->addFileId(*itr);
        }
    }
}

ScChartListener::~ScChartListener()
{
	if ( HasBroadcaster() )
		EndListeningTo();
	delete pUnoData;

    if (mpExtRefListener.get())
    {
        // Stop listening to all external files.
        ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
        const hash_set<sal_uInt16>& rFileIds = mpExtRefListener->getAllFileIds();
        hash_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
            pRefMgr->removeLinkListener(*itr, mpExtRefListener.get());
    }
}

ScDataObject* ScChartListener::Clone() const
{
	return new ScChartListener( *this );
}

void ScChartListener::SetUno(
		const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
		const uno::Reference< chart::XChartData >& rSource )
{
//	DBG_ASSERT( rListener.is() && rSource.is(), "Nullpointer bei SetUno" );
	delete pUnoData;
	pUnoData = new ScChartUnoData( rListener, rSource );
}

uno::Reference< chart::XChartDataChangeEventListener > ScChartListener::GetUnoListener() const
{
	if ( pUnoData )
		return pUnoData->GetListener();
	return uno::Reference< chart::XChartDataChangeEventListener >();
}

uno::Reference< chart::XChartData > ScChartListener::GetUnoSource() const
{
	if ( pUnoData )
		return pUnoData->GetSource();
	return uno::Reference< chart::XChartData >();
}

void ScChartListener::Notify( SvtBroadcaster&, const SfxHint& rHint )
{
    const ScHint* p = dynamic_cast<const ScHint*>(&rHint);
    if (p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)))
        SetUpdateQueue();
}

void ScChartListener::Update()
{
	if ( pDoc->IsInInterpreter() )
	{	// #73482# If interpreting do nothing and restart timer so we don't
		// interfere with interpreter and don't produce an Err522 or similar.
		// This may happen if we are rescheduled via Basic function.
		pDoc->GetChartListenerCollection()->StartTimer();
		return ;
	}
	if ( pUnoData )
	{
		bDirty = sal_False;
		//!	irgendwann mal erkennen, was sich innerhalb des Charts geaendert hat
		chart::ChartDataChangeEvent aEvent( pUnoData->GetSource(),
										chart::ChartDataChangeType_ALL,
										0, 0, 0, 0 );
		pUnoData->GetListener()->chartDataChanged( aEvent );
	}
	else if ( pDoc->GetAutoCalc() )
	{
		bDirty = sal_False;
		pDoc->UpdateChart( GetString());
	}
}

ScRangeListRef ScChartListener::GetRangeList() const
{
    ScRangeListRef aRLRef(new ScRangeList);
    ScRefTokenHelper::getRangeListFromTokens(*aRLRef, *mpTokens);
    return aRLRef;
}

void ScChartListener::SetRangeList( const ScRangeListRef& rNew )
{
    vector<ScSharedTokenRef> aTokens;
    ScRefTokenHelper::getTokensFromRangeList(aTokens, *rNew);
    mpTokens->swap(aTokens);
}

void ScChartListener::SetRangeList( const ScRange& rRange )
{
    ScSharedTokenRef pToken;
    ScRefTokenHelper::getTokenFromRange(pToken, rRange);
    mpTokens->push_back(pToken);
}

namespace {

class StartEndListening : public unary_function<ScSharedTokenRef, void>
{
public:
    StartEndListening(ScDocument* pDoc, ScChartListener& rParent, bool bStart) :
        mpDoc(pDoc), mrParent(rParent), mbStart(bStart) {}

    void operator() (const ScSharedTokenRef& pToken)
    {
        if (!ScRefTokenHelper::isRef(pToken))
            return;

        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        if (bExternal)
        {
            sal_uInt16 nFileId = pToken->GetIndex();
            ScExternalRefManager* pRefMgr = mpDoc->GetExternalRefManager();
            ScChartListener::ExternalRefListener* pExtRefListener = mrParent.GetExtRefListener();
            if (mbStart)
            {
                pRefMgr->addLinkListener(nFileId, pExtRefListener);
                pExtRefListener->addFileId(nFileId);
            }
            else
            {
                pRefMgr->removeLinkListener(nFileId, pExtRefListener);
                pExtRefListener->removeFileId(nFileId);
            }
        }
        else
        {
            ScRange aRange;
            ScRefTokenHelper::getRangeFromToken(aRange, pToken, bExternal);
            if (mbStart)
                startListening(aRange);
            else
                endListening(aRange);
        }
    }

private:
    void startListening(const ScRange& rRange)
    {
        if (rRange.aStart == rRange.aEnd)
            mpDoc->StartListeningCell(rRange.aStart, &mrParent);
        else
            mpDoc->StartListeningArea(rRange, &mrParent);
    }

    void endListening(const ScRange& rRange)
    {
        if (rRange.aStart == rRange.aEnd)
            mpDoc->EndListeningCell(rRange.aStart, &mrParent);
        else
            mpDoc->EndListeningArea(rRange, &mrParent);
    }

private:
    ScDocument* mpDoc;
    ScChartListener& mrParent;
    bool mbStart;
};

}

void ScChartListener::StartListeningTo()
{
    if (!mpTokens.get() || mpTokens->empty())
        // no references to listen to.
        return;

    for_each(mpTokens->begin(), mpTokens->end(), StartEndListening(pDoc, *this, true));
}

void ScChartListener::EndListeningTo()
{
    if (!mpTokens.get() || mpTokens->empty())
        // no references to listen to.
        return;

    for_each(mpTokens->begin(), mpTokens->end(), StartEndListening(pDoc, *this, false));
}


void ScChartListener::ChangeListening( const ScRangeListRef& rRangeListRef,
			sal_Bool bDirtyP  )
{
	EndListeningTo();
	SetRangeList( rRangeListRef );
	StartListeningTo();
	if ( bDirtyP )
		SetDirty( sal_True );
}


void ScChartListener::UpdateScheduledSeriesRanges()
{
	if ( bSeriesRangesScheduled )
	{
		bSeriesRangesScheduled = sal_False;
		UpdateSeriesRanges();
	}
}


void ScChartListener::UpdateChartIntersecting( const ScRange& rRange )
{
    ScSharedTokenRef pToken;
    ScRefTokenHelper::getTokenFromRange(pToken, rRange);

    if (ScRefTokenHelper::intersects(*mpTokens, pToken))
    {
        // force update (chart has to be loaded), don't use ScChartListener::Update
        pDoc->UpdateChart( GetString());
    }
}


void ScChartListener::UpdateSeriesRanges()
{
    ScRangeListRef pRangeList(new ScRangeList);
    ScRefTokenHelper::getRangeListFromTokens(*pRangeList, *mpTokens);
    pDoc->SetChartRangeList(GetString(), pRangeList);
}

ScChartListener::ExternalRefListener* ScChartListener::GetExtRefListener()
{
    if (!mpExtRefListener.get())
        mpExtRefListener.reset(new ExternalRefListener(*this, pDoc));

    return mpExtRefListener.get();
}

void ScChartListener::SetUpdateQueue()
{
    bDirty = true;
    pDoc->GetChartListenerCollection()->StartTimer();
}

sal_Bool ScChartListener::operator==( const ScChartListener& r )
{
    bool b1 = (mpTokens.get() && !mpTokens->empty());
    bool b2 = (r.mpTokens.get() && !r.mpTokens->empty());

    if (pDoc != r.pDoc || bUsed != r.bUsed || bDirty != r.bDirty || 
        bSeriesRangesScheduled != r.bSeriesRangesScheduled ||
        GetString() != r.GetString() || b1 != b2)
        return false;

    if (!b1 && !b2)
        // both token list instances are empty.
        return true;

    return *mpTokens == *r.mpTokens;
}

// ============================================================================

ScChartHiddenRangeListener::ScChartHiddenRangeListener()
{
}

ScChartHiddenRangeListener::~ScChartHiddenRangeListener()
{
    // empty d'tor
}

// === ScChartListenerCollection ======================================

ScChartListenerCollection::RangeListenerItem::RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p) :
    maRange(rRange), mpListener(p)
{
}

ScChartListenerCollection::ScChartListenerCollection( ScDocument* pDocP ) :
	ScStrCollection( 4, 4, sal_False ),
	pDoc( pDocP )
{
	aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::ScChartListenerCollection(
		const ScChartListenerCollection& rColl ) :
	ScStrCollection( rColl ),
	pDoc( rColl.pDoc )
{
	aTimer.SetTimeoutHdl( LINK( this, ScChartListenerCollection, TimerHdl ) );
}

ScChartListenerCollection::~ScChartListenerCollection()
{
	//	#96783# remove ChartListener objects before aTimer dtor is called, because
	//	ScChartListener::EndListeningTo may cause ScChartListenerCollection::StartTimer
	//	to be called if an empty ScNoteCell is deleted

	if (GetCount())
		FreeAll();
}

ScDataObject*	ScChartListenerCollection::Clone() const
{
	return new ScChartListenerCollection( *this );
}

void ScChartListenerCollection::StartAllListeners()
{
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		((ScChartListener*) pItems[ nIndex ])->StartListeningTo();
	}
}

void ScChartListenerCollection::ChangeListening( const String& rName,
		const ScRangeListRef& rRangeListRef, sal_Bool bDirty )
{
	ScChartListener aCLSearcher( rName, pDoc, rRangeListRef );
	ScChartListener* pCL;
	sal_uInt16 nIndex;
	if ( Search( &aCLSearcher, nIndex ) )
	{
		pCL = (ScChartListener*) pItems[ nIndex ];
		pCL->EndListeningTo();
		pCL->SetRangeList( rRangeListRef );
	}
	else
	{
		pCL = new ScChartListener( aCLSearcher );
		Insert( pCL );
	}
	pCL->StartListeningTo();
	if ( bDirty )
		pCL->SetDirty( sal_True );
}

void ScChartListenerCollection::FreeUnused()
{
	// rueckwaerts wg. Pointer-Aufrueckerei im Array
	for ( sal_uInt16 nIndex = nCount; nIndex-- >0; )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		//	Uno-Charts nicht rauskicken
		//	(werden per FreeUno von aussen geloescht)
		if ( !pCL->IsUno() )
		{
			if ( pCL->IsUsed() )
				pCL->SetUsed( sal_False );
			else
				Free( pCL );
		}
	}
}

void ScChartListenerCollection::FreeUno( const uno::Reference< chart::XChartDataChangeEventListener >& rListener,
										 const uno::Reference< chart::XChartData >& rSource )
{
	// rueckwaerts wg. Pointer-Aufrueckerei im Array
	for ( sal_uInt16 nIndex = nCount; nIndex-- >0; )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		if ( pCL->IsUno() &&
			 pCL->GetUnoListener() == rListener &&
			 pCL->GetUnoSource() == rSource )
		{
			Free( pCL );
		}
		//!	sollte nur einmal vorkommen?
	}
}

void ScChartListenerCollection::StartTimer()
{
	aTimer.SetTimeout( SC_CHARTTIMEOUT );
	aTimer.Start();
}

IMPL_LINK( ScChartListenerCollection, TimerHdl, Timer*, EMPTYARG )
{
	if ( Application::AnyInput( INPUT_KEYBOARD ) )
	{
		aTimer.Start();
		return 0;
	}
	UpdateDirtyCharts();
	return 0;
}

void ScChartListenerCollection::UpdateDirtyCharts()
{
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		if ( pCL->IsDirty() )
			pCL->Update();
		if ( aTimer.IsActive() && !pDoc->IsImportingXML())
			break;						// da kam einer dazwischen
	}
}


void ScChartListenerCollection::SetDirty()
{
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		pCL->SetDirty( sal_True );
	}
	StartTimer();
}


void ScChartListenerCollection::SetDiffDirty(
			const ScChartListenerCollection& rCmp, sal_Bool bSetChartRangeLists )
{
	sal_Bool bDirty = sal_False;
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		sal_uInt16 nFound;
		sal_Bool bFound = rCmp.Search( pCL, nFound );
		if ( !bFound ||	(*pCL != *((const ScChartListener*) rCmp.pItems[ nFound ])) )
		{
			if ( bSetChartRangeLists )
			{
				if ( bFound )
				{
					const ScRangeListRef& rList1 = pCL->GetRangeList();
					const ScRangeListRef& rList2 =
						((const ScChartListener*) rCmp.pItems[ nFound ])->GetRangeList();
					sal_Bool b1 = rList1.Is();
					sal_Bool b2 = rList2.Is();
					if ( b1 != b2 || (b1 && b2 && (*rList1 != *rList2)) )
						pDoc->SetChartRangeList( pCL->GetString(), rList1 );
				}
				else
					pDoc->SetChartRangeList( pCL->GetString(), pCL->GetRangeList() );
			}
			bDirty = sal_True;
			pCL->SetDirty( sal_True );
		}
	}
	if ( bDirty )
		StartTimer();
}


void ScChartListenerCollection::SetRangeDirty( const ScRange& rRange )
{
	sal_Bool bDirty = sal_False;
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		const ScRangeListRef& rList = pCL->GetRangeList();
		if ( rList.Is() && rList->Intersects( rRange ) )
		{
			bDirty = sal_True;
			pCL->SetDirty( sal_True );
		}
	}
	if ( bDirty )
		StartTimer();

    // New hidden range listener implementation
    for (list<RangeListenerItem>::iterator itr = maHiddenListeners.begin(), itrEnd = maHiddenListeners.end();
          itr != itrEnd; ++itr)
    {
        if (itr->maRange.Intersects(rRange))
            itr->mpListener->notify();
    }
}


void ScChartListenerCollection::UpdateScheduledSeriesRanges()
{
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		pCL->UpdateScheduledSeriesRanges();
	}
}


void ScChartListenerCollection::UpdateChartsContainingTab( SCTAB nTab )
{
	ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		ScChartListener* pCL = (ScChartListener*) pItems[ nIndex ];
		pCL->UpdateChartIntersecting( aRange );
	}
}


sal_Bool ScChartListenerCollection::operator==( const ScChartListenerCollection& r )
{
	// hier nicht ScStrCollection::operator==() verwenden, der umstaendlich via
	// IsEqual und Compare laeuft, stattdessen ScChartListener::operator==()
	if ( pDoc != r.pDoc || nCount != r.nCount )
		return sal_False;
	for ( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
	{
		if ( *((ScChartListener*) pItems[ nIndex ]) !=
				*((ScChartListener*) r.pItems[ nIndex ]) )
			return sal_False;
	}
	return sal_True;
}

void ScChartListenerCollection::StartListeningHiddenRange( const ScRange& rRange, ScChartHiddenRangeListener* pListener )
{
    RangeListenerItem aItem(rRange, pListener);
    maHiddenListeners.push_back(aItem);
}

namespace {

struct MatchListener : public ::std::unary_function<
        ScChartListenerCollection::RangeListenerItem, bool>
{
    MatchListener(const ScChartHiddenRangeListener* pMatch) :
        mpMatch(pMatch)
    {
    }

    bool operator() (const ScChartListenerCollection::RangeListenerItem& rItem) const
    {
        return mpMatch == rItem.mpListener;
    }

private:
    const ScChartHiddenRangeListener* mpMatch;
};

}
void ScChartListenerCollection::EndListeningHiddenRange( ScChartHiddenRangeListener* pListener )
{
    maHiddenListeners.remove_if(MatchListener(pListener));
}

