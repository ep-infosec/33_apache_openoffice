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


#ifndef _SVX_BACKGRND_HXX
#define _SVX_BACKGRND_HXX

// include ---------------------------------------------------------------

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#include <svx/SvxColorValueSet.hxx>
#include <svx/dlgctrl.hxx>

// class SvxBackgroundTabPage --------------------------------------------
/*
{k:\svx\prototyp\dialog\backgrnd.bmp}
	[Beschreibung]
	Mit dieser TabPage kann eine Brush (z.B. fuer die Hintergrundfarbe eines
	Rahmens) eingestellt werden.
	[Items]
	<SvxBrushItem>:     <SID_ATTR_BRUSH>;
*/

//------------------------------------------------------------------------
// forwards:

class BackgroundPreviewImpl;
class SvxOpenGraphicDialog;
struct SvxBackgroundTable_Impl;
struct SvxBackgroundPara_Impl;
struct SvxBackgroundPage_Impl;
class SvxBrushItem;
//------------------------------------------------------------------------

class SvxBackgroundTabPage : public SvxTabPage
{
	using TabPage::DeactivatePage;
public:
	static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
	static sal_uInt16*      GetRanges();

	virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
	virtual void        Reset( const SfxItemSet& rSet );
	virtual void        FillUserData();
	virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

	void                ShowSelector(); // Umschalt-ListBox aktivieren
	void                ShowTblControl(); // fuer den Writer (Zellen/Zeilen/Tabelle)
	void                ShowParaControl(sal_Bool bCharOnly = sal_False); // fuer den Writer (Absatz/Zeichen)
    void                EnableTransparency(sal_Bool bColor, sal_Bool bGraphic);
	virtual void		PageCreated (SfxAllItemSet aSet); //add CHINA001
protected:
	virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
	SvxBackgroundTabPage( Window* pParent, const SfxItemSet& rCoreSet );
	~SvxBackgroundTabPage();

	FixedText               aSelectTxt;
	ListBox                 aLbSelect;
	const String            aStrBrowse;
	const String            aStrUnlinked;
     FixedText               aTblDesc;	
	 ListBox                 aTblLBox; 
     ListBox		         aParaLBox;
	// Hintergrundfarbe ------------------------------------
	Control					aBorderWin;
	SvxColorValueSet        aBackgroundColorSet;
    FixedLine               aBackgroundColorBox;
	BackgroundPreviewImpl*  pPreviewWin1;
    //color transparency
    FixedText               aColTransFT;
    MetricField             aColTransMF;
	CheckBox                aBtnPreview;
    // Hintergrund-Bitmap ----------------------------------
 	FixedLine               aGbFile;
	PushButton              aBtnBrowse;
	CheckBox                aBtnLink;
  	FixedLine               aGbPosition;
	RadioButton             aBtnPosition;
	RadioButton             aBtnArea;
	RadioButton             aBtnTile;
	SvxRectCtl              aWndPosition;
	FixedInfo               aFtFile;
    //transparency of graphics
    FixedLine               aGraphTransFL;
    MetricField             aGraphTransMF;

    BackgroundPreviewImpl*  pPreviewWin2;

	// DDListBox fuer Writer -------------------------------
	//------------------------------------------------------
	Color		aBgdColor;
	sal_uInt16		nHtmlMode;
	sal_Bool        bAllowShowSelector	: 1;
	sal_Bool        bIsGraphicValid		: 1;
	sal_Bool		bLinkOnly			: 1;
    sal_Bool        bResized            : 1;
    sal_Bool        bColTransparency    : 1;
    sal_Bool        bGraphTransparency  : 1;
    Graphic     aBgdGraphic;
	String      aBgdGraphicPath;
	String      aBgdGraphicFilter;

	SvxBackgroundPage_Impl* pPageImpl;
	SvxOpenGraphicDialog* pImportDlg;

	// Items fuer Sw-Tabelle muessen gesammelt werden
	SvxBackgroundTable_Impl*	pTableBck_Impl;
	// auch fuer die Absatzvorlage
	SvxBackgroundPara_Impl*	pParaBck_Impl;

#ifdef _SVX_BACKGRND_CXX
	void                FillColorValueSets_Impl();
	void                ShowColorUI_Impl();
	void                ShowBitmapUI_Impl();
	sal_Bool                LoadLinkedGraphic_Impl();
	void                RaiseLoadError_Impl();
	void                SetGraphicPosition_Impl( SvxGraphicPosition ePos );
	SvxGraphicPosition  GetGraphicPosition_Impl();
	void				FillControls_Impl(const SvxBrushItem& rBgdAttr,
											const String& rUserData);
	sal_Bool 				FillItemSetWithWallpaperItem( SfxItemSet& rCoreSet, sal_uInt16 nSlot);
	void				ResetFromWallpaperItem( const SfxItemSet& rSet );

	DECL_LINK( LoadTimerHdl_Impl, Timer* );
	DECL_LINK( SelectHdl_Impl, ListBox* );
	DECL_LINK( BrowseHdl_Impl, PushButton* );
	DECL_LINK( RadioClickHdl_Impl, RadioButton* );
	DECL_LINK( FileClickHdl_Impl, CheckBox* );
	DECL_LINK( BackgroundColorHdl_Impl, ValueSet* );
	DECL_LINK( TblDestinationHdl_Impl, ListBox* );
	DECL_LINK( ParaDestinationHdl_Impl, ListBox* );
#endif
};

#endif // #ifndef _SVX_BACKGRND_HXX


