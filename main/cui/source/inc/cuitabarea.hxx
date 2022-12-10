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


#ifndef _CUI_TAB_AREA_HXX
#define _CUI_TAB_AREA_HXX

// include ---------------------------------------------------------------

#include <svx/tabarea.hxx>
#include <svx/SvxColorValueSet.hxx>

/*************************************************************************
|*
|* Fl"achen-Tab-Dialog
|*
\************************************************************************/

class SvxAreaTabDialog : public SfxTabDialog
{
private:
	SdrModel*           mpDrawModel;
//	const SdrView*		mpView;

	XColorListSharedPtr     maColorTab;
	XColorListSharedPtr     maNewColorTab;
	XGradientListSharedPtr  maGradientList;
	XGradientListSharedPtr  maNewGradientList;
	XHatchListSharedPtr     maHatchingList;
	XHatchListSharedPtr     maNewHatchingList;
	XBitmapListSharedPtr    maBitmapList;
	XBitmapListSharedPtr    maNewBitmapList;

	const SfxItemSet&   mrOutAttrs;

	ChangeType          mnColorTableState;
	ChangeType          mnBitmapListState;
	ChangeType          mnGradientListState;
	ChangeType          mnHatchingListState;

	sal_uInt16              mnPageType;
	sal_uInt16              mnDlgType;
	sal_uInt16              mnPos;
	sal_Bool                mbAreaTP;

	virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

protected:
	virtual short       Ok();
#ifdef _SVX_TABAREA_CXX
	DECL_LINK( CancelHdlImpl, void * );
	void                SavePalettes();
#endif

public:
	SvxAreaTabDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, bool bShadow );
	~SvxAreaTabDialog();

	void SetNewColorTable( XColorListSharedPtr aColTab ) { maNewColorTab = aColTab; }
	XColorListSharedPtr GetNewColorTable() const { return maNewColorTab; }
	const XColorListSharedPtr GetColorTable() const { return maColorTab; }

	void SetNewGradientList( XGradientListSharedPtr aGrdLst) { maNewGradientList = aGrdLst; }
	XGradientListSharedPtr GetNewGradientList() const { return maNewGradientList; }
	const XGradientListSharedPtr GetGradientList() const { return maGradientList; }

	void SetNewHatchingList( XHatchListSharedPtr aHtchLst) { maNewHatchingList = aHtchLst; }
	XHatchListSharedPtr GetNewHatchingList() const { return maNewHatchingList; }
	const XHatchListSharedPtr GetHatchingList() const { return maHatchingList; }

	void SetNewBitmapList( XBitmapListSharedPtr aBmpLst) { maNewBitmapList = aBmpLst; }
	XBitmapListSharedPtr GetNewBitmapList() const { return maNewBitmapList; }
	const XBitmapListSharedPtr GetBitmapList() const { return maBitmapList; }
};

/*************************************************************************
|*
|* Transparence-Tab-Page
|*
\************************************************************************/

class SvxTransparenceTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

	const SfxItemSet&   rOutAttrs;
	RECT_POINT          eRP;

	sal_uInt16             nPageType;
	sal_uInt16             nDlgType;

	// main selection
    FixedLine           aFlProp;
    RadioButton         aRbtTransOff;
    RadioButton         aRbtTransLinear;
	RadioButton         aRbtTransGradient;

	// linear transparency
	MetricField         aMtrTransparent;

	// gradient transparency
	FixedText           aFtTrgrType;
	ListBox             aLbTrgrGradientType;
	FixedText           aFtTrgrCenterX;
	MetricField         aMtrTrgrCenterX;
	FixedText           aFtTrgrCenterY;
	MetricField         aMtrTrgrCenterY;
	FixedText           aFtTrgrAngle;
	MetricField         aMtrTrgrAngle;
	FixedText           aFtTrgrBorder;
	MetricField         aMtrTrgrBorder;
	FixedText           aFtTrgrStartValue;
	MetricField         aMtrTrgrStartValue;
	FixedText           aFtTrgrEndValue;
	MetricField         aMtrTrgrEndValue;

	// preview
	SvxXRectPreview     aCtlBitmapPreview;
	SvxXRectPreview     aCtlXRectPreview;
	sal_Bool				bBitmap;

	XOutdevItemPool*    pXPool;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

#ifdef _SVX_TPAREA_CXX
	DECL_LINK(ClickTransOffHdl_Impl, void * );
	DECL_LINK(ClickTransLinearHdl_Impl, void * );
	DECL_LINK(ClickTransGradientHdl_Impl, void * );
	DECL_LINK(ModifyTransparentHdl_Impl, void*);
	DECL_LINK(ChangeTrgrTypeHdl_Impl, void*);
	DECL_LINK(ModifiedTrgrHdl_Impl, void*);
#endif

	void ActivateLinear(sal_Bool bActivate);
	void ActivateGradient(sal_Bool bActivate);
	void SetControlState_Impl(XGradientStyle eXGS);

	sal_Bool InitPreview ( const SfxItemSet& rSet );
	void InvalidatePreview (sal_Bool bEnable = sal_True );

public:
	SvxTransparenceTabPage(Window* pParent, const SfxItemSet& rInAttrs);
	void Construct();

	static SfxTabPage* Create(Window*, const SfxItemSet&);
	static sal_uInt16* GetRanges();

	virtual sal_Bool FillItemSet(SfxItemSet&);
	virtual void Reset(const SfxItemSet&);
	virtual void ActivatePage(const SfxItemSet& rSet);
	virtual int  DeactivatePage(SfxItemSet* pSet);
	virtual void PointChanged(Window* pWindow, RECT_POINT eRP);

	//CHINA001 void SetPageType(sal_uInt16 *pInType) { pPageType = pInType; }
	//CHINA001 void SetDlgType(sal_uInt16* pInType) { pDlgType = pInType; }
	void SetPageType(sal_uInt16 nInType) { nPageType = nInType; } //add CHINA001 
	void SetDlgType(sal_uInt16 nInType) { nDlgType = nInType; }//add CHINA001 
	virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001 
};

/*************************************************************************
|*
|* Fl"achen-Tab-Page
|*
\************************************************************************/

class SvxAreaTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
    FixedLine           aFlProp;
    ListBox             aTypeLB;

	ColorLB             aLbColor;
	GradientLB          aLbGradient;
	HatchingLB          aLbHatching;
	BitmapLB            aLbBitmap;
	SvxXRectPreview     aCtlBitmapPreview;

	TriStateBox         aTsbStepCount;
    FixedLine           aFlStepCount;
	NumericField        aNumFldStepCount;

	CheckBox			aCbxHatchBckgrd;
	ColorLB             aLbHatchBckgrdColor;

    FixedLine           aFlSize;
	TriStateBox			aTsbOriginal;
	TriStateBox			aTsbScale;
	FixedText			aFtXSize;
	MetricField			aMtrFldXSize;
	FixedText			aFtYSize;
	MetricField			aMtrFldYSize;
    FixedLine           aFlPosition;
	SvxRectCtl			aCtlPosition;
	FixedText			aFtXOffset;
	MetricField			aMtrFldXOffset;
	FixedText			aFtYOffset;
	MetricField			aMtrFldYOffset;
	TriStateBox         aTsbTile;
	TriStateBox         aTsbStretch;
    FixedLine           aFlOffset;
	RadioButton			aRbtRow;
	RadioButton			aRbtColumn;
	MetricField			aMtrFldOffset;

	SvxXRectPreview     aCtlXRectPreview;

	const SfxItemSet&   rOutAttrs;
	RECT_POINT          eRP;

	XColorListSharedPtr     maColorTab;
	XGradientListSharedPtr  maGradientList;
	XHatchListSharedPtr     maHatchingList;
	XBitmapListSharedPtr    maBitmapList;

    // Placeholders for pointer-based entries; these will be inited
    // to point to these so that the page is usable without that
    // SvxAreaTabDialog has to call the setter methods (e.g. SetColorChgd).
    // Without that the pages used in SvxAreaTabDialog are not usable
    ChangeType          maFixed_ChangeType;
    sal_Bool            maFixed_sal_Bool;

	ChangeType*         pnColorTableState;
	ChangeType*         pnBitmapListState;
	ChangeType*         pnGradientListState;
	ChangeType*         pnHatchingListState;

	sal_uInt16          nPageType;
	sal_uInt16          nDlgType;
	sal_uInt16          nPos;

	sal_Bool*           pbAreaTP;

	XOutdevItemPool*    pXPool;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

	SfxMapUnit			ePoolUnit;
	FieldUnit			eFUnit;

    //UUUU
    bool                mbOfferImportButton;
    bool                mbPositionsAdapted;
    bool                mbDirectGraphicSet;
    Graphic             maDirectGraphic;
    String              maDirectName;
    PushButton          maBtnImport;

#ifdef _SVX_TPAREA_CXX
	DECL_LINK( SelectDialogTypeHdl_Impl, ListBox * );
	DECL_LINK( ClickInvisibleHdl_Impl, void * );
	DECL_LINK( ClickColorHdl_Impl, void * );
	DECL_LINK( ModifyColorHdl_Impl, void * );
	DECL_LINK( ModifyHatchBckgrdColorHdl_Impl, void * );
	DECL_LINK( ClickGradientHdl_Impl, void * );
	DECL_LINK( ModifyGradientHdl_Impl, void * );
	DECL_LINK( ClickHatchingHdl_Impl, void * );
	DECL_LINK( ModifyHatchingHdl_Impl, void * );
	DECL_LINK( ToggleHatchBckgrdColorHdl_Impl, void * );
	DECL_LINK( ClickBitmapHdl_Impl, void * );
	DECL_LINK( ModifyBitmapHdl_Impl, void * );
//	DECL_LINK( ModifyTransparentHdl_Impl, void * );
	DECL_LINK( ModifyStepCountHdl_Impl, void * );

    //UUUU
    DECL_LINK( ClickImportHdl_Impl, void * );

	DECL_LINK( ModifyTileHdl_Impl, void * );
	DECL_LINK( ClickScaleHdl_Impl, void * );
#endif

public:
	SvxAreaTabPage( Window* pParent, const SfxItemSet& rInAttrs );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	static  sal_uInt16*	    GetRanges();

	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );
	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );
	virtual void PointChanged( Window* pWindow, RECT_POINT eRP );
	
	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
	void    SetGradientList( XGradientListSharedPtr aGrdLst) { maGradientList = aGrdLst; }
	void    SetHatchingList( XHatchListSharedPtr aHtchLst) { maHatchingList = aHtchLst; }
	void    SetBitmapList( XBitmapListSharedPtr aBmpLst) { maBitmapList = aBmpLst; }

	//CHINA001 void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; } //add CHINA001 
	//CHINA001 void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }//add CHINA001 
	//CHINA001 void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
	void    SetPos( sal_uInt16 nInPos ) { nPos = nInPos; }//add CHINA001 
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }
	virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001 
	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }
	void    SetGrdChgd( ChangeType* pIn ) { pnGradientListState = pIn; }
	void    SetHtchChgd( ChangeType* pIn ) { pnHatchingListState = pIn; }
	void    SetBmpChgd( ChangeType* pIn ) { pnBitmapListState = pIn; }
};

/*************************************************************************
|*
|* Schatten-Tab-Page
|*
\************************************************************************/

class SvxShadowTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
    FixedLine           aFlProp;
    TriStateBox         aTsbShowShadow;
	FixedText           aFtPosition;
	SvxRectCtl          aCtlPosition;
	FixedText           aFtDistance;
	MetricField         aMtrDistance;
	FixedText           aFtShadowColor;
	ColorLB             aLbShadowColor;
	FixedText           aFtTransparent;
	MetricField         aMtrTransparent;
	SvxXShadowPreview	aCtlXRectPreview;

	const SfxItemSet&   rOutAttrs;
	RECT_POINT          eRP;

	XColorListSharedPtr maColorTab;
	ChangeType*         pnColorTableState;
	sal_uInt16				nPageType;	//add CHINA001 
	sal_uInt16				nDlgType;	//add CHINA001 
	sal_uInt16*             pPos;
	sal_Bool*               pbAreaTP;

	sal_Bool				bDisable;

	XOutdevItemPool*    pXPool;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;
	SfxMapUnit			ePoolUnit;

#ifdef _SVX_TPSHADOW_CXX
	DECL_LINK( ClickShadowHdl_Impl, void * );
	DECL_LINK( ModifyShadowHdl_Impl, void * );
#endif

public:
	SvxShadowTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();
	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	static  sal_uInt16*	    GetRanges();

	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );
	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );
	virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
//CHINA001	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
//CHINA001	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; } //add CHINA001 
	void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }	//add CHINA001 
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }
	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }
	virtual void PageCreated (SfxAllItemSet aSet); //add CHINA001 
	void	DisablePage( sal_Bool bIn ) { bDisable = bIn; }
};

/*************************************************************************
|*
|* Farbverlauf-Tab-Page
|*
\************************************************************************/

class SvxGradientTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
    FixedLine           aFlProp;
	FixedText           aFtType;
	ListBox             aLbGradientType;
	FixedText           aFtCenterX;
	MetricField         aMtrCenterX;
	FixedText           aFtCenterY;
	MetricField         aMtrCenterY;
	FixedText           aFtAngle;
	MetricField         aMtrAngle;
	FixedText           aFtBorder;
	MetricField         aMtrBorder;
	FixedText           aFtColorFrom;
	ColorLB             aLbColorFrom;
	MetricField         aMtrColorFrom;
	FixedText           aFtColorTo;
	ColorLB             aLbColorTo;
	MetricField         aMtrColorTo;
	GradientLB          aLbGradients;
	SvxXRectPreview     aCtlPreview;
	PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;

	const SfxItemSet&   rOutAttrs;

	XColorListSharedPtr     maColorTab;
	XGradientListSharedPtr  maGradientList;

	ChangeType*         pnGradientListState;
	ChangeType*         pnColorTableState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPos;
	sal_Bool*               pbAreaTP;

	XOutdevItemPool*    pXPool;
	XFillStyleItem      aXFStyleItem;
	XFillGradientItem   aXGradientItem;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

#ifdef _SVX_TPGRADNT_CXX
	DECL_LINK( ClickAddHdl_Impl, void * );
	DECL_LINK( ClickModifyHdl_Impl, void * );
	DECL_LINK( ClickDeleteHdl_Impl, void * );
	DECL_LINK( ChangeGradientHdl_Impl, void * );
	DECL_LINK( ModifiedHdl_Impl, void * );
	DECL_LINK( ClickLoadHdl_Impl, void * );
	DECL_LINK( ClickSaveHdl_Impl, void * );

	long CheckChanges_Impl();
	void SetControlState_Impl( XGradientStyle eXGS );
#endif

public:
	SvxGradientTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
	void    SetGradientList( XGradientListSharedPtr aGrdLst) { maGradientList = aGrdLst; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

	void    SetGrdChgd( ChangeType* pIn ) { pnGradientListState = pIn; }
	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }
};

/*************************************************************************
|*
|* Schraffuren-Tab-Page
|*
\************************************************************************/

class SvxHatchTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
	FixedText           aFtDistance;
	MetricField         aMtrDistance;
	FixedText           aFtAngle;
	MetricField         aMtrAngle;
	SvxRectCtl          aCtlAngle;
    FixedLine           aFlProp;
	FixedText           aFtLineType;
	ListBox             aLbLineType;
	FixedText           aFtLineColor;
	ColorLB             aLbLineColor;
	HatchingLB          aLbHatchings;
	SvxXRectPreview     aCtlPreview;
	PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;

	const SfxItemSet&   rOutAttrs;
	RECT_POINT          eRP;

	XColorListSharedPtr maColorTab;
	XHatchListSharedPtr maHatchingList;

	ChangeType*         pnHatchingListState;
	ChangeType*         pnColorTableState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPos;
	sal_Bool*               pbAreaTP;

	XOutdevItemPool*    pXPool;
	XFillStyleItem      aXFStyleItem;
	XFillHatchItem      aXHatchItem;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

	SfxMapUnit			ePoolUnit;

#ifdef _SVX_TPHATCH_CXX
	DECL_LINK( ChangeHatchHdl_Impl, void * );
	DECL_LINK( ModifiedHdl_Impl, void * );
	DECL_LINK( ClickAddHdl_Impl, void * );
	DECL_LINK( ClickModifyHdl_Impl, void * );
	DECL_LINK( ClickDeleteHdl_Impl, void * );
	DECL_LINK( ClickLoadHdl_Impl, void * );
	DECL_LINK( ClickSaveHdl_Impl, void * );

	long CheckChanges_Impl();
#endif

public:
	SvxHatchTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
	void    SetHatchingList( XHatchListSharedPtr aHtchLst) { maHatchingList = aHtchLst; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

	void    SetHtchChgd( ChangeType* pIn ) { pnHatchingListState = pIn; }
	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }

	virtual void		DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************
|*
|* Bitmap-Tab-Page
|*
\************************************************************************/

class SvxBitmapTabPage : public SvxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
	SvxPixelCtl         aCtlPixel;
    FixedText           aFtPixelEdit;
	FixedText           aFtColor;
	ColorLB             aLbColor;
	FixedText           aFtBackgroundColor;
	ColorLB             aLbBackgroundColor;
	FixedText           aLbBitmapsHidden;
	BitmapLB            aLbBitmaps;
    FixedLine           aFlProp;
	SvxXRectPreview     aCtlPreview;
	PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnImport;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;

	SvxBitmapCtl        aBitmapCtl;

	const SfxItemSet&   rOutAttrs;

	XColorListSharedPtr     maColorTab;
	XBitmapListSharedPtr    maBitmapList;

	ChangeType*         pnBitmapListState;
	ChangeType*         pnColorTableState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPos;
	sal_Bool*               pbAreaTP;

	sal_Bool                bBmpChanged;

	XOutdevItemPool*    pXPool;
	XFillStyleItem      aXFStyleItem;
	XFillBitmapItem     aXBitmapItem;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

#ifdef _SVX_TPBITMAP_CXX
	DECL_LINK( ClickAddHdl_Impl, void * );
	DECL_LINK( ClickImportHdl_Impl, void * );
	DECL_LINK( ClickModifyHdl_Impl, void * );
	DECL_LINK( ClickDeleteHdl_Impl, void * );
	DECL_LINK( ChangeBitmapHdl_Impl, void * );
	DECL_LINK( ChangePixelColorHdl_Impl, void * );
	DECL_LINK( ChangeBackgrndColorHdl_Impl, void * );
	DECL_LINK( ClickLoadHdl_Impl, void * );
	DECL_LINK( ClickSaveHdl_Impl, void * );

	long CheckChanges_Impl();
#endif

public:
	SvxBitmapTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }
	void    SetBitmapList( XBitmapListSharedPtr aBmpLst) { maBitmapList = aBmpLst; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

	void    SetBmpChgd( ChangeType* pIn ) { pnBitmapListState = pIn; }
	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }

    /** Return a label that is associated with the given control.  This
        label is used to the determine the name for the control.
        @param pLabeled
            The control for which to return a label.
        @return
            Return a label control that provides a name for the specified
            control.
    */
    virtual Window*	GetParentLabeledBy( const Window* pLabeled ) const;
};

/*************************************************************************
|*
|* Farben-Tab-Page
|*
\************************************************************************/

class SvxColorTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
    FixedLine           aFlProp;
	FixedText           aFtName;
	Edit                aEdtName;
    FixedText           aFtColor;
	ColorLB             aLbColor;

    FixedText           aTableNameFT;
    SvxColorValueSet    aValSetColorTable;

    SvxXRectPreview     aCtlPreviewOld;
	SvxXRectPreview     aCtlPreviewNew;

    ListBox             aLbColorModel;
    FixedText           aFtColorModel1;
	MetricField         aMtrFldColorModel1;
    FixedText           aFtColorModel2;
	MetricField         aMtrFldColorModel2;
    FixedText           aFtColorModel3;
	MetricField         aMtrFldColorModel3;

    FixedText           aFtColorModel4;
	MetricField         aMtrFldColorModel4;

    PushButton          aBtnAdd;
	PushButton          aBtnModify;
	PushButton          aBtnWorkOn;
	PushButton          aBtnDelete;
	ImageButton         aBtnLoad;
	ImageButton         aBtnSave;

	const SfxItemSet&   rOutAttrs;

	XColorListSharedPtr maColorTab;

	ChangeType*         pnColorTableState;
	sal_uInt16*             pPageType;
	sal_uInt16*             pDlgType;
	sal_uInt16*             pPos;
	sal_Bool*               pbAreaTP;

	XOutdevItemPool*    pXPool;
	XFillStyleItem      aXFStyleItem;
	XFillColorItem      aXFillColorItem;
	XFillAttrSetItem    aXFillAttr;
	SfxItemSet&         rXFSet;

	ColorModel          eCM;

	Color				aAktuellColor;

#ifdef _SVX_TPCOLOR_CXX
	void    ConvertColorValues (Color& rColor, ColorModel eModell);
	void    RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK );
	void    CmykToRgb_Impl( Color& rColor, const sal_uInt16 nKey );
	sal_uInt16  ColorToPercent_Impl( sal_uInt16 nColor );
	sal_uInt16  PercentToColor_Impl( sal_uInt16 nPercent );

    void ImpColorCountChanged();

	//-----------------------------------------------------------------------------------------------------
	DECL_LINK( ClickAddHdl_Impl, void * );		// Button 'Hinzufuegen'
	DECL_LINK( ClickModifyHdl_Impl, void * );	// Button 'Aendern'
	DECL_LINK( ClickDeleteHdl_Impl, void * );	// Button 'loeschen'
	DECL_LINK( ClickWorkOnHdl_Impl, void * );	// Button 'Bearbeiten'

	DECL_LINK( SelectColorLBHdl_Impl, void * );	// Farbe aus Listbox ausw�hlen
	DECL_LINK( SelectValSetHdl_Impl, void * );	// Farbe aus Farbpalette (links) ausw�hlen
	DECL_LINK( SelectColorModelHdl_Impl, void * );	// Auswahl Listbox 'Farbmodell'
	long ChangeColorHdl_Impl( void* p );
	DECL_LINK( ModifiedHdl_Impl, void * );		// Inhalt der Farbwerte-Felder wurde ver�ndert
	DECL_LINK( ClickLoadHdl_Impl, void * );		// Button 'Farbtabelle laden'
	DECL_LINK( ClickSaveHdl_Impl, void * );		// Button 'Farbtabelle sichern'

	long CheckChanges_Impl();
#endif

public:
	SvxColorTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

	void    Construct();

	static  SfxTabPage* Create( Window*, const SfxItemSet& );
	virtual sal_Bool FillItemSet( SfxItemSet& );
	virtual void Reset( const SfxItemSet & );

	virtual void ActivatePage( const SfxItemSet& rSet );
	virtual int  DeactivatePage( SfxItemSet* pSet );

	void    SetColorTable( XColorListSharedPtr aColTab ) { maColorTab = aColTab; }

	void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
	void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
	void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
	void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

	void    SetColorChgd( ChangeType* pIn ) { pnColorTableState = pIn; }

	virtual void FillUserData();
};


#endif // _CUI_TAB_AREA_HXX

