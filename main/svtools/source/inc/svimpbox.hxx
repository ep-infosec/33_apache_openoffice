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



#ifndef _SVIMPLBOX_HXX
#define _SVIMPLBOX_HXX

#ifndef _SELENG_HXX
#include <vcl/seleng.hxx>
#endif
#ifndef _SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#include <vcl/vclevent.hxx>
// #102891# ----------------
#include <unotools/intlwrapper.hxx>
// #97680# -----------------
#include <vector>
#include "svtaccessiblefactory.hxx"

class SvTreeListBox;
class Point;
class DropEvent;
class SvLBoxTreeList;
class SvImpLBox;
class SvLBoxEntry;
class SvLBoxItem;
class SvLBoxTab;
class TabBar;

class ImpLBSelEng : public FunctionSet
{
	SvImpLBox* 			pImp;
	SelectionEngine*	pSelEng;
	SvTreeListBox*		pView;

public:
	ImpLBSelEng( SvImpLBox* pImp, SelectionEngine* pSelEng,
				 SvTreeListBox* pView );
	virtual ~ImpLBSelEng();
	void 		BeginDrag();
	void 		CreateAnchor();
	void		DestroyAnchor();
	sal_Bool 		SetCursorAtPoint( const Point& rPoint,
					sal_Bool bDontSelectAtCursor=sal_False );
	sal_Bool 		IsSelectionAtPoint( const Point& rPoint );
	void 		DeselectAtPoint( const Point& rPoint );
	void 		DeselectAll();
};

// Flags fuer nFlag
#define F_VER_SBARSIZE_WITH_HBAR		0x0001
#define F_HOR_SBARSIZE_WITH_VBAR		0x0002
#define F_IGNORE_NEXT_MOUSEMOVE			0x0004	// OS/2 only
#define F_IN_SCROLLING					0x0008
#define F_DESEL_ALL						0x0010
#define F_START_EDITTIMER				0x0020 	// MAC only
#define F_IGNORE_SELECT					0x0040
#define F_IN_RESIZE						0x0080
#define F_REMOVED_ENTRY_INVISIBLE		0x0100
#define F_REMOVED_RECALC_MOST_RIGHT		0x0200
#define F_IGNORE_CHANGED_TABS			0x0400
#define F_PAINTED						0x0800
#define F_IN_PAINT						0x1000
#define F_ENDSCROLL_SET_VIS_SIZE	   	0x2000
#define F_FILLING 						0x4000


class SvImpLBox
{
friend class ImpLBSelEng;
friend class SvTreeListBox;
private:
	SvTreeListBox* 		pView;
	SvLBoxTreeList*		pTree;
	SvLBoxEntry*		pCursor;
	SvLBoxEntry*		pStartEntry;
	SvLBoxEntry*  		pAnchor;
	SvLBoxEntry*		pMostRightEntry;
	SvLBoxButton* 		pActiveButton;
	SvLBoxEntry* 		pActiveEntry;
	SvLBoxTab* 			pActiveTab;
	TabBar*				pTabBar;

	ScrollBar   		aVerSBar;
	ScrollBar   		aHorSBar;
	ScrollBarBox 		aScrBarBox;

    ::svt::AccessibleFactoryAccess
                        m_aFactoryAccess;

	static Image*		s_pDefCollapsed;
	static Image*		s_pDefExpanded;
	static Image*		s_pDefCollapsedHC;
	static Image*		s_pDefExpandedHC;
	static oslInterlockedCount	s_nImageRefCount; /// When 0 all static images will be destroyed

	// Node Bitmaps
	enum ImageType
	{
		itNodeExpanded = 0,		// node is expanded ( usually a bitmap showing a minus )
		itNodeCollapsed,		// node is collapsed ( usually a bitmap showing a plus )
		itNodeDontKnow,			// don't know the node state
		itEntryDefExpanded,		// default for expanded entries
		itEntryDefCollapsed,	// default for collapsed entries

		IT_IMAGE_COUNT
	};

	// all our images
	Image				m_aNodeAndEntryImages[ IT_IMAGE_COUNT ];
	// plus the high contrast versions
	Image				m_aNodeAndEntryImages_HC[ IT_IMAGE_COUNT ];

	// wg. kompat. hier
	Size    			aOutputSize;
	SelectionEngine 	aSelEng;
	ImpLBSelEng			aFctSet;
	Timer				aAsyncBeginDragTimer;
	Point				aAsyncBeginDragPos;

	long				nYoffsNodeBmp;
	long				nNodeBmpTabDistance; // typisch kleiner 0
	long				nNodeBmpWidth;
	long				nNextVerVisSize;
	long 				nMostRight;
	sal_uLong     			nVisibleCount;  // Anzahl Zeilen im Control
	sal_uLong				nCurUserEvent; //-1 == kein Userevent amn Laufen
	short				nHorSBarHeight, nVerSBarWidth;
	sal_uInt16				nFlags;
	sal_uInt16				nCurTabPos;

	WinBits				m_nStyle;
    ExtendedWinBits     nExtendedWinBits;
	sal_Bool				bSimpleTravel : 1; // ist sal_True bei SINGLE_SELECTION
	sal_Bool				bUpdateMode : 1;
	sal_Bool				bInVScrollHdl : 1;
	sal_Bool				bAsyncBeginDrag : 1;
	sal_Bool				bSubLstOpRet : 1;	// open/close sublist with return/enter, defaulted with sal_False
	sal_Bool				bSubLstOpLR : 1;	// open/close sublist with cursor left/right, defaulted with sal_False
	sal_Bool				bContextMenuHandling : 1;
	sal_Bool				bIsCellFocusEnabled : 1;

    sal_Bool            bAreChildrenTransient;

	Point				aEditClickPos;
	Timer				aEditTimer;

	// #102891# -------------------
	IntlWrapper *		pIntlWrapper;

	// #97680# --------------------
	std::vector< short > aContextBmpWidthVector;

	DECL_LINK( EditTimerCall, Timer * );

	DECL_LINK( BeginDragHdl, void* );
	DECL_LINK( MyUserEvent,void*);
	void				StopUserEvent();

	void 				InvalidateEntriesFrom( long nY ) const;
	void 				InvalidateEntry( long nY ) const;
	void 				ShowVerSBar();
	// setzt Thumb auf FirstEntryToDraw
	void 				SyncVerThumb();
	sal_Bool 				IsLineVisible( long nY ) const;
	long 				GetEntryLine( SvLBoxEntry* pEntry ) const;
	void				FillView();
	void 				CursorDown();
	void 				CursorUp();
	void				KeyLeftRight( long nDiff );
	void 				PageDown( sal_uInt16 nDelta );
	void 				PageUp( sal_uInt16 nDelta );

	void				SetCursor( SvLBoxEntry* pEntry, sal_Bool bForceNoSelect = sal_False );

	void 				DrawNet();

	// ScrollBar-Handler
	DECL_LINK( ScrollUpDownHdl, ScrollBar * );
	DECL_LINK( ScrollLeftRightHdl, ScrollBar * );
	DECL_LINK( EndScrollHdl, ScrollBar * );

	void				SetNodeBmpYOffset( const Image& );
	void				SetNodeBmpTabDistance();

	// Selection-Engine
	SvLBoxEntry*  		MakePointVisible( const Point& rPoint,
							sal_Bool bNotifyScroll=sal_True );

	void        		SetAnchorSelection( SvLBoxEntry* pOld,
							SvLBoxEntry* pNewCursor );
	void        		BeginDrag();
	sal_Bool				ButtonDownCheckCtrl( const MouseEvent& rMEvt,
							SvLBoxEntry* pEntry, long nY	);
	sal_Bool				MouseMoveCheckCtrl( const MouseEvent& rMEvt,
							SvLBoxEntry* pEntry );
	sal_Bool				ButtonUpCheckCtrl( const MouseEvent& rMEvt );
	sal_Bool 				ButtonDownCheckExpand( const MouseEvent&,
							SvLBoxEntry*,long nY );

	void				PositionScrollBars( Size& rOSize, sal_uInt16 nMask );
	sal_uInt16				AdjustScrollBars( Size& rSize );

	void				BeginScroll();
	void				EndScroll();
	sal_Bool				InScroll() const { return (sal_Bool)(nFlags & F_IN_SCROLLING)!=0;}
	Rectangle			GetVisibleArea() const;
	sal_Bool				EntryReallyHit(SvLBoxEntry* pEntry,const Point& rPos,long nLine);
	void				InitScrollBarBox();
	SvLBoxTab*			NextTab( SvLBoxTab* );

	sal_Bool				SetMostRight( SvLBoxEntry* pEntry );
	void				FindMostRight( SvLBoxEntry* EntryToIgnore );
	void 				FindMostRight( SvLBoxEntry* pParent, SvLBoxEntry* EntryToIgnore );
	void 				FindMostRight_Impl( SvLBoxEntry* pParent,SvLBoxEntry* EntryToIgnore  );
	void				NotifyTabsChanged();

	inline sal_Bool			IsExpandable() const		// if element at cursor can be expanded in general
							{ return pCursor->HasChilds() || pCursor->HasChildsOnDemand(); }
	inline sal_Bool			IsNowExpandable() const		// if element at cursor can be expanded at this moment
							{ return IsExpandable() && !pView->IsExpanded( pCursor ); }

	static	void		implInitDefaultNodeImages();

	// #102891# -------------------
	void 				UpdateIntlWrapper();

	// #97680# --------------------
	short				UpdateContextBmpWidthVector( SvLBoxEntry* pEntry, short nWidth );
	void				UpdateContextBmpWidthMax( SvLBoxEntry* pEntry );
	void				UpdateContextBmpWidthVectorFromMovedEntry( SvLBoxEntry* pEntry );

	void				CalcCellFocusRect( SvLBoxEntry* pEntry, Rectangle& rRect );

    inline sal_Bool     AreChildrenTransient() const { return bAreChildrenTransient; }
    inline void         SetChildrenNotTransient() { bAreChildrenTransient = sal_False; }

public:
	SvImpLBox( SvTreeListBox* pView, SvLBoxTreeList*, WinBits nWinStyle );
	~SvImpLBox();

	void 				Clear();
    void                SetStyle( WinBits i_nWinStyle );
	void 				SetExtendedWindowBits( ExtendedWinBits _nBits );
    ExtendedWinBits     GetExtendedWindowBits() const { return nExtendedWinBits; }
	void 				SetModel( SvLBoxTreeList* pModel ) { pTree = pModel;}

	void 				EntryInserted( SvLBoxEntry*);
	void 				RemovingEntry( SvLBoxEntry* pEntry );
	void 				EntryRemoved();
	void 				MovingEntry( SvLBoxEntry* pEntry );
	void 				EntryMoved( SvLBoxEntry* pEntry );
	void 				TreeInserted( SvLBoxEntry* pEntry );

	void 				IndentChanged( short nIndentPixel );
	void 				EntryExpanded( SvLBoxEntry* pEntry );
	void 				EntryCollapsed( SvLBoxEntry* pEntry );
	void 				CollapsingEntry( SvLBoxEntry* pEntry );
	void 				EntrySelected( SvLBoxEntry*, sal_Bool bSelect );

	void	 			Paint( const Rectangle& rRect );
	void 				RepaintSelectionItems();
	void 				MouseButtonDown( const MouseEvent& );
	void 				MouseButtonUp( const MouseEvent& );
	void 				MouseMove( const MouseEvent&);
	sal_Bool 				KeyInput( const KeyEvent& );
	void 				Resize();
	void 				GetFocus();
	void 				LoseFocus();
	void 				UpdateAll(
							sal_Bool bInvalidateCompleteView= sal_True,
							sal_Bool bUpdateVerSBar = sal_True );
	void 				SetEntryHeight( short nHeight );
	void 				PaintEntry( SvLBoxEntry* pEntry );
	void 				InvalidateEntry( SvLBoxEntry* );
	void 				RecalcFocusRect();

	inline void 		SelectEntry( SvLBoxEntry* pEntry, sal_Bool bSelect );
	void 				SetDragDropMode( DragDropMode eDDMode );
	void 				SetSelectionMode( SelectionMode eSelMode  );
	void				SetAddMode( sal_Bool ) { aSelEng.AddAlways(sal_False); }
	sal_Bool				IsAddMode() const { return aSelEng.IsAlwaysAdding(); }

	SvLBoxEntry* 		GetCurrentEntry() const { return pCursor; }
	sal_Bool				IsEntryInView( SvLBoxEntry* ) const;
	SvLBoxEntry* 		GetEntry( const Point& rPos ) const;
	// gibt letzten Eintrag zurueck, falls Pos unter letztem Eintrag
	SvLBoxEntry* 		GetClickedEntry( const Point& ) const;
	SvLBoxEntry* 		GetCurEntry() const { return pCursor; }
	void				SetCurEntry( SvLBoxEntry* );
	Point 				GetEntryPosition( SvLBoxEntry* ) const;
	void 				MakeVisible( SvLBoxEntry* pEntry, sal_Bool bMoveToTop=sal_False );

	void 				PaintDDCursor( SvLBoxEntry* );

	// Images
	inline Image&		implGetImageLocation( const ImageType _eType, BmpColorMode _eMode );
	inline Image&		implGetImageLocationWithFallback( const ImageType _eType, BmpColorMode _eMode ) const;

	inline void 		SetExpandedNodeBmp( const Image& _rImg, BmpColorMode _eMode = BMP_COLOR_NORMAL );
	inline void 		SetCollapsedNodeBmp( const Image& _rImg, BmpColorMode _eMode = BMP_COLOR_NORMAL  );
	inline void 		SetDontKnowNodeBmp( const Image& rImg, BmpColorMode _eMode = BMP_COLOR_NORMAL );

	inline const Image& GetExpandedNodeBmp( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
	inline const Image& GetCollapsedNodeBmp( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;
	inline const Image&	GetDontKnowNodeBmp( BmpColorMode _eMode = BMP_COLOR_NORMAL ) const;

	inline void			SetDefaultEntryExpBmp( const Image& _rImg, BmpColorMode _eMode = BMP_COLOR_NORMAL );
	inline void			SetDefaultEntryColBmp( const Image& _rImg, BmpColorMode _eMode = BMP_COLOR_NORMAL );
	inline const Image&	GetDefaultEntryExpBmp( BmpColorMode _eMode = BMP_COLOR_NORMAL );
	inline const Image&	GetDefaultEntryColBmp( BmpColorMode _eMode = BMP_COLOR_NORMAL );

	static const Image&	GetDefaultExpandedNodeImage( BmpColorMode _eMode = BMP_COLOR_NORMAL );
	static const Image&	GetDefaultCollapsedNodeImage( BmpColorMode _eMode = BMP_COLOR_NORMAL );

	const Size& 		GetOutputSize() const { return aOutputSize;}
	void 				KeyUp( sal_Bool bPageUp, sal_Bool bNotifyScroll = sal_True );
	void 				KeyDown( sal_Bool bPageDown, sal_Bool bNotifyScroll = sal_True );
	void                Command( const CommandEvent& rCEvt );

	void				Invalidate();
	void				DestroyAnchor() { pAnchor=0; aSelEng.Reset(); }
	void        		SelAllDestrAnch( sal_Bool bSelect,
							sal_Bool bDestroyAnchor = sal_True,
							sal_Bool bSingleSelToo = sal_False );
	void				ShowCursor( sal_Bool bShow );

	sal_Bool    			RequestHelp( const HelpEvent& rHEvt );
	void				EndSelection();
	sal_Bool				IsNodeButton( const Point& rPosPixel, SvLBoxEntry* pEntry ) const;
	void				RepaintScrollBars();
	void				EnableAsyncDrag( sal_Bool b) { bAsyncBeginDrag = b; }
	void				SetUpdateMode( sal_Bool );
	void 				SetUpdateModeFast( sal_Bool );
	sal_Bool				GetUpdateMode() const { return bUpdateMode; }
	Rectangle			GetClipRegionRect() const;
	sal_Bool				HasHorScrollBar() const { return aHorSBar.IsVisible(); }
	void				ShowFocusRect( const SvLBoxEntry* pEntry );
	void				SetTabBar( TabBar* pTabBar );
	void				CancelPendingEdit();

	void				CallEventListeners( sal_uLong nEvent, void* pData = NULL );

	/** Enables, that one cell of a tablistbox entry can be focused */
	inline sal_Bool			IsCellFocusEnabled() const { return bIsCellFocusEnabled; }
	inline void			EnableCellFocus() { bIsCellFocusEnabled = sal_True; }
	bool				SetCurrentTabPos( sal_uInt16 _nNewPos );
	inline sal_uInt16		GetCurrentTabPos() const { return nCurTabPos; }

	bool				IsSelectable( const SvLBoxEntry* pEntry );
};

inline Image& SvImpLBox::implGetImageLocation( const ImageType _eType, BmpColorMode _eMode )
{
	DBG_ASSERT( ( BMP_COLOR_HIGHCONTRAST == _eMode ) || ( BMP_COLOR_NORMAL == _eMode ),
		"SvImpLBox::implGetImageLocation: invalid mode!" );
	DBG_ASSERT( ( _eType >= 0 ) && ( _eType < IT_IMAGE_COUNT ),
		"SvImpLBox::implGetImageLocation: invalid image index (will crash)!" );

	Image* _pSet = ( BMP_COLOR_HIGHCONTRAST == _eMode ) ? m_aNodeAndEntryImages_HC : m_aNodeAndEntryImages;
	return *( _pSet + (sal_Int32)_eType );
}

inline Image& SvImpLBox::implGetImageLocationWithFallback( const ImageType _eType, BmpColorMode _eMode ) const
{
	Image& rImage = const_cast< SvImpLBox* >( this )->implGetImageLocation( _eType, _eMode );
	if ( !rImage )
		// fallback to normal images in case the one for the special mode has not been set
		rImage = const_cast< SvImpLBox* >( this )->implGetImageLocation( _eType, BMP_COLOR_NORMAL );
	return rImage;
}

inline void SvImpLBox::SetDontKnowNodeBmp( const Image& rImg, BmpColorMode _eMode )
{
	implGetImageLocation( itNodeDontKnow, _eMode ) = rImg;
}

inline void SvImpLBox::SetExpandedNodeBmp( const Image& rImg, BmpColorMode _eMode )
{
	implGetImageLocation( itNodeExpanded, _eMode ) = rImg;
	SetNodeBmpYOffset( rImg );
}

inline void SvImpLBox::SetCollapsedNodeBmp( const Image& rImg, BmpColorMode _eMode )
{
	implGetImageLocation( itNodeCollapsed, _eMode ) = rImg;
	SetNodeBmpYOffset( rImg );
}

inline const Image&	SvImpLBox::GetDontKnowNodeBmp( BmpColorMode _eMode ) const
{
	return implGetImageLocationWithFallback( itNodeDontKnow, _eMode );
}

inline const Image& SvImpLBox::GetExpandedNodeBmp( BmpColorMode _eMode ) const
{
	return implGetImageLocationWithFallback( itNodeExpanded, _eMode );
}

inline const Image& SvImpLBox::GetCollapsedNodeBmp( BmpColorMode _eMode ) const
{
	return implGetImageLocationWithFallback( itNodeCollapsed, _eMode );
}

inline void SvImpLBox::SetDefaultEntryExpBmp( const Image& _rImg, BmpColorMode _eMode )
{
	implGetImageLocation( itEntryDefExpanded, _eMode ) = _rImg;
}

inline void SvImpLBox::SetDefaultEntryColBmp( const Image& _rImg, BmpColorMode _eMode )
{
	implGetImageLocation( itEntryDefCollapsed, _eMode ) = _rImg;
}

inline const Image&	SvImpLBox::GetDefaultEntryExpBmp( BmpColorMode _eMode )
{
	return implGetImageLocationWithFallback( itEntryDefExpanded, _eMode );
}

inline const Image&	SvImpLBox::GetDefaultEntryColBmp( BmpColorMode _eMode )
{
	return implGetImageLocationWithFallback( itEntryDefCollapsed, _eMode );
}

inline Point SvImpLBox::GetEntryPosition( SvLBoxEntry* pEntry ) const
{
	return Point( 0, GetEntryLine( pEntry ) );
}

inline void SvImpLBox::PaintEntry( SvLBoxEntry* pEntry )
{
	long nY = GetEntryLine( pEntry );
	pView->PaintEntry( pEntry, nY );
}

inline sal_Bool SvImpLBox::IsLineVisible( long nY ) const
{
	sal_Bool bRet = sal_True;
	if ( nY < 0 || nY >= aOutputSize.Height() )
		bRet = sal_False;
	return bRet;
}

inline void SvImpLBox::TreeInserted( SvLBoxEntry* pInsTree )
{
	EntryInserted( pInsTree );
}

#endif // #ifndef _SVIMPLBOX_HXX

