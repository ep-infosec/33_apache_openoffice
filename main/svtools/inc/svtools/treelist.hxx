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



#ifndef _SVTREELIST_HXX
#define _SVTREELIST_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>
#include <tools/list.hxx>

#ifndef _TABLE_HXX
#include <tools/table.hxx>
#endif
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#define LISTACTION_INSERTED			1
#define LISTACTION_REMOVING         2
#define LISTACTION_REMOVED          3
#define LISTACTION_MOVING			4
#define LISTACTION_MOVED			5
#define LISTACTION_CLEARING 		6
#define LISTACTION_INSERTED_TREE	7
#define LISTACTION_INVALIDATE_ENTRY	8
#define LISTACTION_RESORTING		9
#define LISTACTION_RESORTED			10
#define LISTACTION_CLEARED			11

#define SV_TREELIST_ROOT_ENTRY  (SvListEntry*)0
#define SV_TREELIST_ERROR		 0xFFFFFFFF

// Entryflags, die an der View haengen
#define SVLISTENTRYFLAG_SELECTED		0x0001
#define SVLISTENTRYFLAG_EXPANDED		0x0002
#define SVLISTENTRYFLAG_FOCUSED			0x0004
#define SVLISTENTRYFLAG_CURSORED		0x0008
#define SVLISTENTRYFLAG_NOT_SELECTABLE	0x0010
// IAccessible2 implementation
#define SVLISTENTRYFLAG_OBJSELECTED	0x0040

class SvListEntry;

class SvTreeEntryList : public List // SvEntryListStd
{
public:
	SvTreeEntryList(sal_uInt16 nInitPos=16, sal_uInt16 nResize=16 )
			: List( nInitPos, nResize )
	{}
	SvTreeEntryList(sal_uInt16 BlockSize, sal_uInt16 InitSize, sal_uInt16 Resize )
		: List(BlockSize, InitSize, Resize )
	{}

	void DestroyAll();
};

class SVT_DLLPUBLIC SvListEntry
{
friend class SvTreeList;
friend class SvListView;

private:
	SvListEntry*		pParent;
	SvTreeEntryList*	pChilds;
	sal_uLong				nAbsPos;
	sal_uLong				nListPos;

	void				SetListPositions();
	void				InvalidateChildrensListPositions()
						{
							nListPos |= 0x80000000;
						}
public:
						SvListEntry();
						SvListEntry( const SvListEntry& );
	virtual				~SvListEntry();
	sal_Bool				HasChilds() { return (sal_Bool)(pChilds!=0); }
	sal_Bool				HasChildListPos() const
	{
		if( pParent && !(pParent->nListPos & 0x80000000) )
			return sal_True;
		else return sal_False;
	}
	sal_uLong				GetChildListPos() const
	{
		if( pParent && (pParent->nListPos & 0x80000000) )
			pParent->SetListPositions();
		return ( nListPos & 0x7fffffff );
	}
	virtual void 		Clone( SvListEntry* pSource );
};

class SvListView;

class SvViewData
{
friend class SvTreeList;
friend class SvListView;

	sal_uLong		nVisPos;
protected:
	sal_uInt16 		nFlags;
public:
	SvViewData();
	SvViewData( const SvViewData& );
	virtual ~SvViewData();

	sal_Bool IsSelected() const { return (sal_Bool)(nFlags&SVLISTENTRYFLAG_SELECTED)!=0; }
	sal_Bool IsExpanded() const { return (sal_Bool)(nFlags&SVLISTENTRYFLAG_EXPANDED)!=0; }
	sal_Bool HasFocus() const { return (sal_Bool)(nFlags&SVLISTENTRYFLAG_FOCUSED)!=0; }
	void SetFocus( sal_Bool bFocus)
	{
		if ( !bFocus )
			nFlags &= (~SVLISTENTRYFLAG_FOCUSED);
		else
			nFlags |= SVLISTENTRYFLAG_FOCUSED;
	}
	sal_Bool IsCursored() const { return (sal_Bool)(nFlags&SVLISTENTRYFLAG_CURSORED)!=0; }
	void SetCursored( sal_Bool bCursored )
	{
		if ( !bCursored )
			nFlags &= (~SVLISTENTRYFLAG_CURSORED);
		else
			nFlags |= SVLISTENTRYFLAG_CURSORED;
	}

	sal_uInt16 GetFlags() const { return nFlags; }

	void SetSelectable( bool bSelectable )
	{
		if( bSelectable )
			nFlags &= (~SVLISTENTRYFLAG_NOT_SELECTABLE);
		else
			nFlags |= SVLISTENTRYFLAG_NOT_SELECTABLE;
	}
	bool IsSelectable() const { return (bool)(nFlags&SVLISTENTRYFLAG_NOT_SELECTABLE)==0; }
	// IAccessible2 implementation
	void SetObjectSelected(sal_Bool bSelected)
	{
	if ( !bSelected )
		nFlags &= (~SVLISTENTRYFLAG_OBJSELECTED);
	else
		nFlags |= SVLISTENTRYFLAG_OBJSELECTED;
	}
};

enum SvSortMode { SortAscending, SortDescending, SortNone };

// Rueckgabewerte Sortlink:
// siehe International::Compare( pLeft, pRight )
// ( Compare(a,b) ==> b.Compare(a) ==> strcmp(a,b) )
struct SvSortData
{
	SvListEntry* pLeft;
	SvListEntry* pRight;
};

class SVT_DLLPUBLIC SvTreeList
{
	friend class SvListView;

	List			aViewList;
	sal_uLong           nEntryCount;

	Link			aCloneLink;
	Link			aCompareLink;
	SvSortMode		eSortMode;

	sal_uInt16			nRefCount;

	sal_Bool			bAbsPositionsValid;

	SvListEntry*    FirstVisible() const { return First(); }
	SvListEntry*    NextVisible( const SvListView*,SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
	SvListEntry*	PrevVisible( const SvListView*,SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
	SvListEntry*	LastVisible( const SvListView*,sal_uInt16* pDepth=0 ) const;
	SvListEntry*	NextVisible( const SvListView*,SvListEntry* pEntry, sal_uInt16& rDelta ) const;
	SvListEntry*	PrevVisible( const SvListView*,SvListEntry* pEntry, sal_uInt16& rDelta ) const;

	sal_Bool			IsEntryVisible( const SvListView*,SvListEntry* pEntry ) const;
	SvListEntry*	GetEntryAtVisPos( const SvListView*,sal_uLong nVisPos ) const;
	sal_uLong			GetVisiblePos( const SvListView*,SvListEntry* pEntry ) const;
	sal_uLong			GetVisibleCount( const SvListView* ) const;
	sal_uLong			GetVisibleChildCount( const SvListView*,SvListEntry* pParent ) const;

	SvListEntry*    FirstSelected( const SvListView*) const;
	SvListEntry*    NextSelected( const SvListView*,SvListEntry* pEntry ) const;
	SvListEntry*	PrevSelected( const SvListView*,SvListEntry* pEntry ) const;
	SvListEntry*	LastSelected( const SvListView*) const;

	sal_Bool            Select( SvListView*,SvListEntry* pEntry, sal_Bool bSelect=sal_True );
	sal_uLong			SelectChilds( SvListView*,SvListEntry* pParent, sal_Bool bSelect );
	void			SelectAll( SvListView*,sal_Bool bSelect ); // ruft nicht Select-Hdl
	sal_uLong			GetChildSelectionCount( const SvListView*,SvListEntry* pParent ) const;

	void            Expand( SvListView*,SvListEntry* pParent );
	void            Collapse( SvListView*,SvListEntry* pParent );

//#if 0 // _SOLAR__PRIVATE
	SVT_DLLPRIVATE void 			SetAbsolutePositions();
	SVT_DLLPRIVATE SvTreeEntryList*CloneChilds( SvTreeEntryList* pChilds,
								 SvListEntry* pNewParent,
								 sal_uLong& nCloneCount ) const;
	SVT_DLLPRIVATE void			SetListPositions( SvTreeEntryList* );

	// rPos wird bei SortModeNone nicht geaendert
	SVT_DLLPRIVATE void			GetInsertionPos( SvListEntry* pEntry, SvListEntry* pParent,
						sal_uLong& rPos );
	SVT_DLLPRIVATE void			ResortChilds( SvListEntry* pParent );
//#endif /* _SOLAR__PRIVATE */

protected:

	SvListEntry*      pRootItem;

public:

	SvTreeList();
	virtual ~SvTreeList();

	void			InsertView( SvListView* );
	void			RemoveView( SvListView* );
	sal_uLong			GetViewCount() const { return aViewList.Count(); }
	SvListView*		GetView(sal_uLong nPos) const {return (SvListView*)aViewList.GetObject(nPos);}
	void			Broadcast( sal_uInt16 nActionId, SvListEntry* pEntry1=0,
						SvListEntry* pEntry2=0, sal_uLong nPos=0 );
	// informiert alle Listener
	void			InvalidateEntry( SvListEntry* );

	sal_uLong			GetEntryCount() const { return nEntryCount; }
	SvListEntry*    First() const;
	SvListEntry*    Next( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
	SvListEntry*	Prev( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const;
	SvListEntry*	Last( sal_uInt16* pDepth=0 ) const;

	SvListEntry*	FirstChild( SvListEntry* pParent ) const;
	SvListEntry*	NextSibling( SvListEntry* pEntry ) const;
	SvListEntry*	PrevSibling( SvListEntry* pEntry ) const;
	SvListEntry*	LastSibling( SvListEntry* pEntry ) const;

	sal_uLong			Insert( SvListEntry* pEntry,SvListEntry* pPar,sal_uLong nPos=LIST_APPEND);
	sal_uLong			Insert( SvListEntry* pEntry,sal_uLong nRootPos = LIST_APPEND ) { return Insert(pEntry, pRootItem, nRootPos ); }
	void			InsertTree( SvListEntry* pTree, SvListEntry* pTarget );
	void			InsertTree( SvListEntry* pTree, SvListEntry* pTargetParent,
								sal_uLong nListPos );
	// Entries muessen im gleichen Model stehen!
	void            Move( SvListEntry* pSource, SvListEntry* pTarget );
	// erzeugt ggf. Child-List
	sal_uLong   		Move( SvListEntry* pSource, SvListEntry* pTargetParent,
						  sal_uLong nListPos);
	void			Copy( SvListEntry* pSource, SvListEntry* pTarget );
	sal_uLong   		Copy( SvListEntry* pSource, SvListEntry* pTargetParent,
						  sal_uLong nListPos);

	sal_Bool            Remove( SvListEntry* pEntry );
	void            Clear();

	sal_Bool			HasChilds( SvListEntry* pEntry ) const;
	sal_Bool			HasParent( SvListEntry* pEntry ) const 	{ return (sal_Bool)(pEntry->pParent!=pRootItem); }
	sal_Bool			IsChild( SvListEntry* pParent, SvListEntry* pChild ) const;
	sal_Bool			IsInChildList( SvListEntry* pParent, SvListEntry* pChild) const;
	SvListEntry*	GetEntry( SvListEntry* pParent, sal_uLong nPos ) const;
	SvListEntry*	GetEntry( sal_uLong nRootPos ) const;
	SvListEntry*	GetEntryAtAbsPos( sal_uLong nAbsPos ) const;
	SvListEntry*	GetParent( SvListEntry* pEntry ) const;
	SvListEntry*	GetRootLevelParent( SvListEntry* pEntry ) const;
	SvTreeEntryList* GetChildList( SvListEntry* pParent ) const;

	sal_uLong			GetAbsPos( SvListEntry* pEntry ) const;
	sal_uLong			GetRelPos( SvListEntry* pChild ) const { return pChild->GetChildListPos(); }
	sal_uLong			GetChildCount( SvListEntry* pParent ) const;
	sal_uInt16          GetDepth( SvListEntry* pEntry ) const;
	sal_Bool			IsAtRootDepth( SvListEntry* pEntry ) const { return (sal_Bool)(pEntry->pParent==pRootItem); }

	// das Model ruft zum Clonen von Entries den Clone-Link auf,
	// damit man sich nicht vom Model ableiten muss, wenn man
	// sich von SvListEntry ableitet.
	// Deklaration des Clone-Handlers:
	// DECL_LINK(CloneHdl,SvListEntry*);
	// der Handler muss einen SvListEntry* zurueckgeben
	SvListEntry*	Clone( SvListEntry* pEntry, sal_uLong& nCloneCount ) const;
	void			SetCloneLink( const Link& rLink ) { aCloneLink=rLink; }
	const Link&		GetCloneLink() const { return aCloneLink; }
	virtual SvListEntry* CloneEntry( SvListEntry* ) const; // ruft den Clone-Link
	virtual SvListEntry* CreateEntry() const; // zum 'new'en von Entries

	sal_uInt16			GetRefCount() const { return nRefCount; }
	void			SetRefCount( sal_uInt16 nRef ) { nRefCount = nRef; }

	void			SetSortMode( SvSortMode eMode ) { eSortMode = eMode; }
	SvSortMode		GetSortMode() const { return eSortMode; }
	virtual StringCompare Compare( SvListEntry*, SvListEntry* ) const;
	void			SetCompareHdl( const Link& rLink ) { aCompareLink = rLink; }
	const Link& 	GetCompareHdl() const { return aCompareLink; }
	void			Resort();

	void			CheckIntegrity() const;
};

class SVT_DLLPUBLIC SvListView
{
	friend class SvTreeList;

	sal_uLong           nVisibleCount;
	sal_uLong			nSelectionCount;
	sal_Bool            bVisPositionsValid;

//#if 0 // _SOLAR__PRIVATE
	SVT_DLLPRIVATE void			InitTable();
	SVT_DLLPRIVATE void			ClearTable();
	SVT_DLLPRIVATE void			RemoveViewData( SvListEntry* pParent );
//#endif

protected:
	Table			aDataTable;  // Mapping SvListEntry -> ViewData
	SvTreeList*		pModel;

	void ActionMoving( SvListEntry* pEntry,SvListEntry* pTargetPrnt,sal_uLong nChildPos);
	void ActionMoved( SvListEntry* pEntry,SvListEntry* pTargetPrnt,sal_uLong nChildPos);
	void ActionInserted( SvListEntry* pEntry );
	void ActionInsertedTree( SvListEntry* pEntry );
	void ActionRemoving( SvListEntry* pEntry );
	void ActionRemoved( SvListEntry* pEntry );
	void ActionClear();

public:

	SvListView();   // !!! setzt das Model auf 0
	SvListView( SvTreeList* pModel );
	virtual ~SvListView();
	void            Clear();
	SvTreeList*		GetModel() const { return pModel; }
	virtual void	SetModel( SvTreeList* );
	virtual void	ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
						SvListEntry* pEntry2, sal_uLong nPos );

	sal_uLong			GetVisibleCount() const { return pModel->GetVisibleCount( (SvListView*)this );}
	SvListEntry*    FirstVisible() const { return pModel->FirstVisible(); }
	SvListEntry*    NextVisible( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const {return pModel->NextVisible(this,pEntry,pDepth); }
	SvListEntry*	PrevVisible( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const {return pModel->PrevVisible(this,pEntry,pDepth); }
	SvListEntry*	LastVisible( sal_uInt16* pDepth=0 ) const { return pModel->LastVisible(this,pDepth);}
	SvListEntry*	NextVisible( SvListEntry* pEntry, sal_uInt16& rDelta ) const { return pModel->NextVisible(this,pEntry,rDelta); }
	SvListEntry*	PrevVisible( SvListEntry* pEntry, sal_uInt16& rDelta ) const { return pModel->PrevVisible(this,pEntry,rDelta); }

	sal_uLong           GetSelectionCount() const { return nSelectionCount; }
	SvListEntry*	FirstSelected() const { return pModel->FirstSelected(this);}
	SvListEntry*    NextSelected( SvListEntry* pEntry ) const { return pModel->NextSelected(this,pEntry); }
	SvListEntry*	PrevSelected( SvListEntry* pEntry ) const { return pModel->PrevSelected(this,pEntry); }
	SvListEntry*	LastSelected() const { return pModel->LastSelected(this); }
	SvListEntry*	GetEntryAtVisPos( sal_uLong nVisPos ) const { return pModel->GetEntryAtVisPos((SvListView*)this,nVisPos); }
	sal_uLong			GetVisiblePos( SvListEntry* pEntry ) const { return pModel->GetVisiblePos((SvListView*)this,pEntry); }

	sal_uLong			GetVisibleChildCount(SvListEntry* pParent ) const { return pModel->GetVisibleChildCount((SvListView*)this,pParent); }
	sal_uLong			GetChildSelectionCount( SvListEntry* pParent ) const { return pModel->GetChildSelectionCount((SvListView*)this,pParent); }
	void            Expand( SvListEntry* pParent ) { pModel->Expand((SvListView*)this,pParent); }
	void            Collapse( SvListEntry* pParent ) { pModel->Collapse((SvListView*)this,pParent); }
	sal_Bool            Select( SvListEntry* pEntry, sal_Bool bSelect=sal_True ) { return pModel->Select((SvListView*)this,pEntry,bSelect); }
	sal_uLong			SelectChilds( SvListEntry* pParent, sal_Bool bSelect ) { return pModel->SelectChilds((SvListView*)this,pParent, bSelect); }
	// ruft nicht Select-Hdl
	virtual void	SelectAll( sal_Bool bSelect, sal_Bool ) { pModel->SelectAll((SvListView*)this, bSelect); }
	sal_Bool			IsEntryVisible( SvListEntry* pEntry ) const	{ return pModel->IsEntryVisible((SvListView*)this,pEntry); }
	sal_Bool			IsExpanded( SvListEntry* pEntry ) const;
	sal_Bool			IsSelected( SvListEntry* pEntry ) const;
	sal_Bool			HasEntryFocus( SvListEntry* pEntry ) const;
	void			SetEntryFocus( SvListEntry* pEntry, sal_Bool bFocus ) const;
	SvViewData*		GetViewData( SvListEntry* pEntry ) const;
	sal_Bool			HasViewData() const { return aDataTable.Count() > 1;}	// eine ROOT gibts immer
	virtual	SvViewData* CreateViewData( SvListEntry* pEntry );
	virtual	void InitViewData( SvViewData*, SvListEntry* pEntry );

	virtual void 	ModelHasCleared();
	virtual void 	ModelHasInserted( SvListEntry* pEntry );
	virtual void 	ModelHasInsertedTree( SvListEntry* pEntry );
	virtual void 	ModelIsMoving( SvListEntry* pSource, SvListEntry* pTargetParent,
								sal_uLong nPos	);
	virtual void 	ModelHasMoved( SvListEntry* pSource );
	virtual void 	ModelIsRemoving( SvListEntry* pEntry );
	virtual void 	ModelHasRemoved( SvListEntry* pEntry );
	virtual void 	ModelHasEntryInvalidated( SvListEntry* pEntry );
};

inline sal_Bool SvListView::IsExpanded( SvListEntry* pEntry ) const
{
	DBG_ASSERT(pEntry,"IsExpanded:No Entry");
	SvViewData* pData = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
	DBG_ASSERT(pData,"Entry not in Table");
	return pData && pData->IsExpanded();
}
inline sal_Bool SvListView::IsSelected( SvListEntry* pEntry ) const
{
	DBG_ASSERT(pEntry,"IsExpanded:No Entry");
	SvViewData* pData = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
	DBG_ASSERT(pData,"Entry not in Table");
	return pData && pData->IsSelected();
}
inline sal_Bool SvListView::HasEntryFocus( SvListEntry* pEntry ) const
{
	DBG_ASSERT(pEntry,"IsExpanded:No Entry");
	SvViewData* pData = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
	DBG_ASSERT(pData,"Entry not in Table");
	return pData && pData->HasFocus();
}
inline void SvListView::SetEntryFocus( SvListEntry* pEntry, sal_Bool bFocus ) const
{
	DBG_ASSERT(pEntry,"SetEntryFocus:No Entry");
	SvViewData* pData = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
	DBG_ASSERT(pData,"Entry not in Table");
	pData->SetFocus(bFocus);
}

inline SvViewData* SvListView::GetViewData( SvListEntry* pEntry ) const
{
#ifndef DBG_UTIL
	return (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
#else
	SvViewData* pResult = (SvViewData*)aDataTable.Get( (sal_uLong)pEntry );
	DBG_ASSERT(pResult,"Entry not in model or wrong view");
	return pResult;
#endif
}

inline sal_Bool SvTreeList::HasChilds( SvListEntry* pEntry ) const
{
	if ( !pEntry )
		pEntry = pRootItem;
	return (sal_Bool)(pEntry->pChilds != 0);
}

inline SvListEntry* SvTreeList::GetEntry( SvListEntry* pParent, sal_uLong nPos ) const
{   if ( !pParent )
		pParent = pRootItem;
	SvListEntry* pRet = 0;
	if ( pParent->pChilds )
		pRet = (SvListEntry*)(pParent->pChilds->GetObject(nPos));
	return pRet;
}

inline SvListEntry* SvTreeList::GetEntry( sal_uLong nRootPos ) const
{
	SvListEntry* pRet;
	if ( nEntryCount )
		pRet = (SvListEntry*)(pRootItem->pChilds->GetObject(nRootPos));
	else
		pRet = 0;
	return pRet;
}

inline SvTreeEntryList* SvTreeList::GetChildList( SvListEntry* pParent ) const
{
	if ( !pParent )
		pParent = pRootItem;
	return pParent->pChilds;
}

inline SvListEntry* SvTreeList::GetParent( SvListEntry* pEntry ) const
{
	SvListEntry* pParent = pEntry->pParent;
	if ( pParent==pRootItem )
		pParent = 0;
	return pParent;
}

#define DECLARE_SVTREELIST( ClassName, Type )									\
class ClassName : public SvTreeList                                       	\
{                                                                           \
public:                                                                     \
	Type		First() const                                               \
					{ return (Type)SvTreeList::First(); }                     \
    Type    	Next( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const           \
					{ return (Type)SvTreeList::Next(pEntry,pDepth); }         \
    Type		Prev( SvListEntry* pEntry, sal_uInt16* pDepth=0 ) const           \
					{ return (Type)SvTreeList::Prev(pEntry,pDepth); }         \
    Type		Last( sal_uInt16* pDepth=0 ) const           					\
					{ return (Type)SvTreeList::Last(pDepth); }         		\
																			\
	Type 		Clone( SvListEntry* pEntry, sal_uLong& nCloneCount ) const        \
					{ return (Type)SvTreeList::Clone(pEntry,nCloneCount); }   \
	Type		GetEntry( SvListEntry* pParent, sal_uLong nPos ) const            \
					{ return (Type)SvTreeList::GetEntry(pParent,nPos); }      \
	Type		GetEntry( sal_uLong nRootPos ) const                            \
					{ return (Type)SvTreeList::GetEntry(nRootPos); }          \
	Type		GetParent( SvListEntry* pEntry ) const                        \
					{ return (Type)SvTreeList::GetParent(pEntry); }           \
    using SvTreeList::FirstChild;                                             \
	Type		FirstChild( Type pParent ) const							\
					{ return (Type)SvTreeList::FirstChild(pParent); }			\
    using SvTreeList::NextSibling;                                             \
	Type		NextSibling( Type pEntry ) const							\
					{ return (Type)SvTreeList::NextSibling(pEntry); }			\
    using SvTreeList::PrevSibling;                                             \
	Type		PrevSibling( Type pEntry ) const							\
					{ return (Type)SvTreeList::PrevSibling(pEntry); }			\
    using SvTreeList::LastSibling;                                             \
	Type		LastSibling( Type pEntry ) const							\
					{ return (Type)SvTreeList::LastSibling(pEntry); }			\
	Type		GetEntryAtAbsPos( sal_uLong nAbsPos ) const						\
					{ return (Type)SvTreeList::GetEntryAtAbsPos( nAbsPos); }	\
};

#endif
