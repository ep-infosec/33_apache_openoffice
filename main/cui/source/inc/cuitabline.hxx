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


#ifndef _CUI_TAB_LINE_HXX
#define _CUI_TAB_LINE_HXX

// include ---------------------------------------------------------------
#include <svx/tabline.hxx>
/*************************************************************************
|*
|* Transform-Tab-Dialog
|*
\************************************************************************/

class SvxLineTabDialog : public SfxTabDialog
{
private:
	SdrModel*           pDrawModel;
	const SdrObject*    pObj;

	const SfxItemSet&   rOutAttrs;

	XColorListSharedPtr     maColorTab;
	XColorListSharedPtr     maNewColorTab;
	XDashListSharedPtr      maDashList;
	XDashListSharedPtr      maNewDashList;
	XLineEndListSharedPtr   maLineEndList;
	XLineEndListSharedPtr   maNewLineEndList;
	sal_Bool                bObjSelected;

	ChangeType          nLineEndListState;
	ChangeType          nDashListState;
	ChangeType          mnColorTableState;

	sal_uInt16              nPageType;
	sal_uInt16              nDlgType;
	sal_uInt16              nPosDashLb;
	sal_uInt16              nPosLineEndLb;
	sal_uInt16              mnPos;
	sal_Bool                mbAreaTP;

	virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

protected:
	virtual short       Ok();
#ifdef _SVX_TABLINE_CXX
	DECL_LINK( CancelHdlImpl, void * );
	void                SavePalettes();
#endif

public:
	SvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr,
					  SdrModel* pModel, const SdrObject* pObj = NULL,
					  sal_Bool bHasObj = sal_True );
	~SvxLineTabDialog();

	void SetNewDashList( XDashListSharedPtr aInLst) { maNewDashList = aInLst; }
	XDashListSharedPtr GetNewDashList() const { return maNewDashList; }
	const XDashListSharedPtr GetDashList() const { return maDashList; }

	void SetNewLineEndList( XLineEndListSharedPtr aInLst) { maNewLineEndList = aInLst; }
	XLineEndListSharedPtr GetNewLineEndList() const { return maNewLineEndList; }
	const XLineEndListSharedPtr GetLineEndList() const { return maLineEndList; }

	void SetNewColorTable( XColorListSharedPtr aColTab ) { maNewColorTab = aColTab; }
	XColorListSharedPtr GetNewColorTable() const { return maNewColorTab; }
	const XColorListSharedPtr GetColorTable() const { return maColorTab; }
};

/*************************************************************************
|*
|* Linien-Tab-Page
|*
\************************************************************************/

class SvxLineTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;
private:
	FixedLine           aFlLine;
	FixedText           aFtLineStyle;
	LineLB              aLbLineStyle;
	FixedText           aFtColor;
	ColorLB             aLbColor;
	FixedText           aFtLineWidth;
	MetricField         aMtrLineWidth;
	FixedText           aFtTransparent;
	MetricField         aMtrTransparent;
	FixedLine           aFlLineEnds;
	LineEndLB           aLbStartStyle;
	MetricField         aMtrStartWidth;
	TriStateBox         aTsbCenterStart;
	FixedText           aFtLineEndsStyle;
	LineEndLB           aLbEndStyle;
	FixedText           aFtLineEndsWidth;
	MetricField         aMtrEndWidth;
	TriStateBox         aTsbCenterEnd;
	CheckBox            aCbxSynchronize;
	FixedLine           aFLSeparator;
	SvxXLinePreview     aCtlPreview;

	// #116827#
	FixedLine           maFLEdgeStyle;
	FixedText           maFTEdgeStyle;
	LineEndLB           maLBEdgeStyle;

     // LineCaps
     FixedText          maFTCapStyle;
     LineEndLB          maLBCapStyle;

    //#58425# Symbole auf einer Linie (z.B. StarChart) ->
    SdrObjList*         pSymbolList; //a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview. The list position is to be used cyclic.
	bool				bNewSize;
	Graphic				aAutoSymbolGraphic; //a graphic to be displayed in the preview in case that an automatic symbol is chosen
	long				nNumMenuGalleryItems;
	long				nSymbolType;
	SfxItemSet*         pSymbolAttr; //attributes for the shown symbols; only necessary if not equal to line properties
	FixedLine			aFlSymbol;
    MenuButton          aSymbolMB;
    FixedText           aSymbolWidthFT;
	MetricField			aSymbolWidthMF;
	FixedText			aSymbolHeightFT;
	MetricField			aSymbolHeightMF;
	CheckBox			aSymbolRatioCB;
	List				aGrfNames;
	List				aGrfBrushItems;
	String				sNumCharFmtName;
	sal_Bool				bLastWidthModified;
	Size				aSymbolLastSize;
	Graphic				aSymbolGraphic;
	Size				aSymbolSize;
	sal_Bool				bSymbols;
    
    const SfxItemSet&   rOutAttrs;
	RECT_POINT          eRP;
	sal_Bool                bObjSelected;

	XOutdevItemPool*    pXPool;
	XLineStyleItem      aXLStyle;
	XLineWidthItem      aXWidth;
	XLineDashItem       aXDash;
	XLineColorItem      aXColor;
	XLineAttrSetItem    aXLineAttr;
	SfxItemSet&         rXLSet;

	XColorListSharedPtr     maColorTab;
	XDashListSharedPtr      maDashList;
	XLineEndListSharedPtr   maLineEndList;

	ChangeType*         pnLineEndListState;
	ChangeType*         pnDashListState;
	ChangeType*         pnColorTableState;
	sal_uInt16             nPageType; //add CHINA001 
	sal_uInt16             nDlgType; //add CHINA001 
	sal_uInt16*             pPosDashLb;
	sal_uInt16*             pPosLineEndLb;

	SfxMapUnit          ePoolUnit;

	// #63083#
	sal_Int32				nActLineWidth;
    
    //Handler f�r Gallery-Popup-Menue-Button + Size
    DECL_LINK( GraphicHdl_Impl, MenuButton * );
    DECL_LINK( MenuCreateHdl_Impl, MenuButton * );
    DECL_STATIC_LINK( SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem* );
    DECL_LINK( SizeHdl_Impl, MetricField * );
    DECL_LINK( RatioHdl_Impl, CheckBox * );
    // <- Symbole

#ifdef _SVX_TPLINE_CXX
	DECL_LINK( ClickInvisibleHdl_Impl, void * );
	DECL_LINK( ChangeStartHdl_Impl, void * );
	DECL_LINK( ChangeEndHdl_Impl, void * );
	DECL_LINK( ChangePreviewHdl_Impl, void * );
	DECL_LINK( ChangeTransparentHdl_Impl, void * );

	// #116827#
	DECL_LINK( ChangeEdgeStyleHdl_Impl, void * );

     // LineCaps
     DECL_LINK ( ChangeCapStyleHdl_Impl, void * );

	sal_Bool FillXLSet_Impl();
#endif

	void FillListboxes();
public:

	void ShowSymbolControls(sal_Bool bOn);

	SvxLineTabPage( Window* pParent, const SfxItemSet& rInAttrs );
	virtual ~SvxLineTabPage();

	void    Construct();


	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	static  sal_uInt16*    GetRanges();

	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet& );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

	virtual void FillUserData();

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
	void    SetDashList( XDashListSharedPtr aDshLst ) { maDashList = aDshLst; }
	void    SetLineEndList( XLineEndListSharedPtr aLneEndLst) { maLineEndList = aLneEndLst; }
	void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

	void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; }//CHINA001 void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }	//CHINA001 void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPosDashLb( sal_uInt16* pInPos ) { pPosDashLb = pInPos; }
	void    SetPosLineEndLb( sal_uInt16* pInPos ) { pPosLineEndLb = pInPos; }

	void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }
	void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }
	void	SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }

	virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001 
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************
|*
|* Linien-Definitions-Tab-Page
|*
\************************************************************************/

class SvxLineDefTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;
private:
	FixedLine           aFlDefinition;
	FixedText           aFTLinestyle;
	LineLB              aLbLineStyles;
	FixedText           aFtType;
	ListBox             aLbType1;
	ListBox             aLbType2;
	FixedText           aFtNumber;
	NumericField        aNumFldNumber1;
	NumericField        aNumFldNumber2;
	FixedText           aFtLength;
	MetricField         aMtrLength1;
	MetricField         aMtrLength2;
	FixedText           aFtDistance;
	MetricField         aMtrDistance;
	CheckBox            aCbxSynchronize;
	PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;
	SvxXLinePreview     aCtlPreview;

	const SfxItemSet&   rOutAttrs;
	XDash               aDash;
	sal_Bool                bObjSelected;

	XOutdevItemPool*    pXPool;
	XLineStyleItem      aXLStyle;
	XLineWidthItem      aXWidth;
	XLineDashItem       aXDash;
	XLineColorItem      aXColor;
	XLineAttrSetItem    aXLineAttr;
	SfxItemSet&         rXLSet;

	XDashListSharedPtr  maDashList;

	ChangeType*         pnDashListState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPosDashLb;

	SfxMapUnit          ePoolUnit;
	FieldUnit           eFUnit;

#ifdef _SVX_TPLNEDEF_CXX
	void FillDash_Impl();
	void FillDialog_Impl();

	DECL_LINK( ClickAddHdl_Impl, void * );
	DECL_LINK( ClickModifyHdl_Impl, void * );
	DECL_LINK( ClickDeleteHdl_Impl, void * );
	DECL_LINK( SelectLinestyleHdl_Impl, void * );
	DECL_LINK( ChangePreviewHdl_Impl, void * );
	DECL_LINK( ChangeNumber1Hdl_Impl, void * );
	DECL_LINK( ChangeNumber2Hdl_Impl, void * );
	DECL_LINK( ClickLoadHdl_Impl, void * );
	DECL_LINK( ClickSaveHdl_Impl, void * );
	DECL_LINK( ChangeMetricHdl_Impl, void * );
	DECL_LINK( SelectTypeHdl_Impl, void * );

	void CheckChanges_Impl();
#endif

public:
	SvxLineDefTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	void    SetDashList( XDashListSharedPtr aDshLst ) { maDashList = aDshLst; }
	void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPosDashLb( sal_uInt16* pInPos ) { pPosDashLb = pInPos; }

	void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************
|*
|* Linienenden-Definitions-Tab-Page
|*
\************************************************************************/

class SvxLineEndDefTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
	FixedLine           aFlTip;
	FixedText           aFTTitle;
	Edit                aEdtName;
	FixedText           aFTLineEndStyle;
	LineEndLB           aLbLineEnds;
	PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;
	FixedInfo           aFiTip;
	SvxXLinePreview     aCtlPreview;

	const SfxItemSet&   rOutAttrs;
	const SdrObject*    pPolyObj;
	sal_Bool                bObjSelected;

	XOutdevItemPool*    pXPool;
	XLineStyleItem      aXLStyle;
	XLineWidthItem      aXWidth;
	XLineColorItem      aXColor;
	XLineAttrSetItem    aXLineAttr;
	SfxItemSet&         rXLSet;

	XLineEndListSharedPtr   maLineEndList;

	ChangeType*         pnLineEndListState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPosLineEndLb;

#ifdef _SVX_TPLNEEND_CXX
	DECL_LINK( ClickAddHdl_Impl, void * );
	DECL_LINK( ClickModifyHdl_Impl, void * );
	DECL_LINK( ClickDeleteHdl_Impl, void * );
	DECL_LINK( ClickLoadHdl_Impl, void * );
	DECL_LINK( ClickSaveHdl_Impl, void * );
	DECL_LINK( SelectLineEndHdl_Impl, void * );
	long ChangePreviewHdl_Impl( void* p );

	void CheckChanges_Impl();
#endif

public:
	SvxLineEndDefTabPage( Window* pParent, const SfxItemSet& rInAttrs );
	~SvxLineEndDefTabPage();

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	void    SetLineEndList( XLineEndListSharedPtr aInList ) { maLineEndList = aInList; }
	void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }
	void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPosLineEndLb( sal_uInt16* pInPos ) { pPosLineEndLb = pInPos; }

	void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};


#endif // _CUI_TAB_LINE_HXX

