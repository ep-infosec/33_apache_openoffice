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



#ifndef SC_CHARTLIS_HXX
#define SC_CHARTLIS_HXX


#include <vcl/timer.hxx>
#include <svl/listener.hxx>
#include "collect.hxx"
#include "rangelst.hxx"
#include "token.hxx"
#include "externalrefmgr.hxx"

#include <memory>
#include <vector>
#include <list>
#include <hash_set>

class ScDocument;
class ScChartUnoData;
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>

class SC_DLLPUBLIC ScChartListener : public StrData, public SvtListener
{
public:
    class ExternalRefListener : public ScExternalRefManager::LinkListener
    {
    public:
        ExternalRefListener(ScChartListener& rParent, ScDocument* pDoc);
        virtual ~ExternalRefListener();
        virtual void notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType);
        void addFileId(sal_uInt16 nFileId);
        void removeFileId(sal_uInt16 nFileId);
        ::std::hash_set<sal_uInt16>& getAllFileIds();

    private:
        ExternalRefListener();
        ExternalRefListener(const ExternalRefListener& r);

        ScChartListener& mrParent;
        ::std::hash_set<sal_uInt16> maFileIds;
        ScDocument*                 mpDoc;
    };

private:

    ::std::auto_ptr<ExternalRefListener>                mpExtRefListener;
    ::std::auto_ptr< ::std::vector<ScSharedTokenRef> >  mpTokens;

	ScChartUnoData*	pUnoData;
	ScDocument* 	pDoc;
	sal_Bool			bUsed;	// fuer ScChartListenerCollection::FreeUnused
	sal_Bool			bDirty;
	sal_Bool			bSeriesRangesScheduled;

					// not implemented
	ScChartListener& operator=( const ScChartListener& );

public:
                    ScChartListener( const String& rName, ScDocument* pDoc,
                                     const ScRange& rRange );
                    ScChartListener( const String& rName, ScDocument* pDoc,
                                     const ScRangeListRef& rRangeListRef );
                    ScChartListener( const String& rName, ScDocument* pDoc,
                                     ::std::vector<ScSharedTokenRef>* pTokens );
                    ScChartListener( const ScChartListener& );
	virtual			~ScChartListener();
	virtual ScDataObject*	Clone() const;

	void			SetUno( const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >& rListener,
							const com::sun::star::uno::Reference< com::sun::star::chart::XChartData >& rSource );
	com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >	GetUnoListener() const;
	com::sun::star::uno::Reference< com::sun::star::chart::XChartData >						GetUnoSource() const;

	sal_Bool			IsUno() const	{ return (pUnoData != NULL); }

	virtual void 	Notify( SvtBroadcaster& rBC, const SfxHint& rHint );
	void			StartListeningTo();
	void			EndListeningTo();
	void			ChangeListening( const ScRangeListRef& rRangeListRef,
									sal_Bool bDirty = sal_False );
	void			Update();
	ScRangeListRef	GetRangeList() const;
	void			SetRangeList( const ScRangeListRef& rNew );
	void			SetRangeList( const ScRange& rNew );
	sal_Bool			IsUsed() const { return bUsed; }
	void			SetUsed( sal_Bool bFlg ) { bUsed = bFlg; }
	sal_Bool			IsDirty() const { return bDirty; }
	void			SetDirty( sal_Bool bFlg ) { bDirty = bFlg; }

    void            UpdateChartIntersecting( const ScRange& rRange );

	// if chart series ranges are to be updated later on (e.g. DeleteTab, InsertTab)
	void			ScheduleSeriesRanges()		{ bSeriesRangesScheduled = sal_True; }
	void			UpdateScheduledSeriesRanges();
	void			UpdateSeriesRanges();

    ExternalRefListener* GetExtRefListener();
    void            SetUpdateQueue();

	sal_Bool			operator==( const ScChartListener& );
	sal_Bool			operator!=( const ScChartListener& r )
						{ return !operator==( r ); }
};

// ============================================================================

class ScChartHiddenRangeListener
{
public:
    ScChartHiddenRangeListener();
    virtual ~ScChartHiddenRangeListener();
    virtual void notify() = 0;
};

// ============================================================================

class ScChartListenerCollection : public ScStrCollection
{
public:
    struct RangeListenerItem
    {
        ScRange                     maRange;
        ScChartHiddenRangeListener* mpListener;
        explicit RangeListenerItem(const ScRange& rRange, ScChartHiddenRangeListener* p);
    };

private:
    ::std::list<RangeListenerItem> maHiddenListeners;

	Timer			aTimer;
	ScDocument*		pDoc;

					DECL_LINK( TimerHdl, Timer* );

					// not implemented
	ScChartListenerCollection& operator=( const ScChartListenerCollection& );

    using ScStrCollection::operator==;

public:
					ScChartListenerCollection( ScDocument* pDoc );
					ScChartListenerCollection( const ScChartListenerCollection& );
	virtual	ScDataObject*	Clone() const;

	virtual			~ScChartListenerCollection();

					// nur nach copy-ctor noetig, wenn neu ins Dok gehaengt
	void			StartAllListeners();

	void			ChangeListening( const String& rName,
									const ScRangeListRef& rRangeListRef,
									sal_Bool bDirty = sal_False );
	// FreeUnused nur wie in ScDocument::UpdateChartListenerCollection verwenden!
	void			FreeUnused();
	void			FreeUno( const com::sun::star::uno::Reference< com::sun::star::chart::XChartDataChangeEventListener >& rListener,
							 const com::sun::star::uno::Reference< com::sun::star::chart::XChartData >& rSource );
	void			StartTimer();
	void			UpdateDirtyCharts();
	void SC_DLLPUBLIC SetDirty();
	void			SetDiffDirty( const ScChartListenerCollection&,
						sal_Bool bSetChartRangeLists = sal_False );

	void			SetRangeDirty( const ScRange& rRange );		// z.B. Zeilen/Spalten

	void			UpdateScheduledSeriesRanges();
    void            UpdateChartsContainingTab( SCTAB nTab );

	sal_Bool			operator==( const ScChartListenerCollection& );

    /** 
     * Start listening on hide/show change within specified cell range.  A 
     * single listener may listen on multiple ranges when the caller passes 
     * the same pointer multiple times with different ranges. 
     *  
     * Note that the caller is responsible for managing the life-cycle of the 
     * listener instance. 
     */
    void            StartListeningHiddenRange( const ScRange& rRange, 
                                               ScChartHiddenRangeListener* pListener );

    /** 
     * Remove all ranges associated with passed listener instance from the 
     * list of hidden range listeners.  This does not delete the passed 
     * listener instance. 
     */
    void            EndListeningHiddenRange( ScChartHiddenRangeListener* pListener );
};


#endif

