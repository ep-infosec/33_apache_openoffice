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
#include "precompiled_sfx2.hxx"
#include <sfx2/new.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/morebtn.hxx>
#include <svtools/svmedit.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>

#include "new.hrc"
#include "doc.hrc"
#include <sfx2/sfx.hrc>
#include "helpid.hrc"
#include "sfxtypes.hxx"
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "fltfnc.hxx"
#include <sfx2/viewsh.hxx>
#include "sfx2/viewfac.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include "preview.hxx"
#include <sfx2/printer.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/jobset.hxx>
#include <svtools/accessibilityoptions.hxx>

// Draw modes
#define OUTPUT_DRAWMODE_COLOR		(DRAWMODE_DEFAULT)
#define OUTPUT_DRAWMODE_GRAYSCALE   (DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT)
#define OUTPUT_DRAWMODE_BLACKWHITE  (DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT)
#define OUTPUT_DRAWMODE_CONTRAST	(DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT)

//========================================================================

#define MORE_BTN(x) pMoreBt->x

//========================================================================

void SfxPreviewBase_Impl::SetObjectShell( SfxObjectShell* pObj )
{
    ::boost::shared_ptr<GDIMetaFile> pFile = pObj
        ? pObj->GetPreviewMetaFile()
        : ::boost::shared_ptr<GDIMetaFile>();
	pMetaFile = pFile;
	Invalidate();
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl(
	Window* pParent, const ResId& rResId )
	: Window(pParent, rResId), pMetaFile()
{
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl( Window* pParent )
	: Window(pParent, 0 ), pMetaFile()
{
	Resize();
	Show();
}

SfxPreviewBase_Impl::~SfxPreviewBase_Impl()
{
}

void SfxPreviewBase_Impl::Resize()
{
	Invalidate();
}

void SfxPreviewBase_Impl::SetGDIFile( ::boost::shared_ptr<GDIMetaFile> pFile )
{
	pMetaFile = pFile;
	Invalidate();
}

SfxFrameWindow* SfxPreviewWin_Impl::PreviewFactory(
	SfxFrame* pFrame, const String& /*rName*/ )
{
	return new SfxFrameWindow( new SfxPreviewWin_Impl(
		&pFrame->GetCurrentViewFrame()->GetWindow()  ) );
}

void SfxPreviewWin_Impl::ImpPaint(
	const Rectangle&, GDIMetaFile* pFile, Window* pWindow )
{
	Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
	DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
				"size of first page is 0, overload GetFirstPageSize or set vis-area!" );
#define FRAME 4
	long nWidth = pWindow->GetOutputSize().Width() - 2*FRAME;
	long nHeight = pWindow->GetOutputSize().Height() - 2*FRAME;
	if( nWidth < 0 ) nWidth = 0;
	if( nHeight < 0 ) nHeight = 0;

	double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
	double dRatioPreV=((double) nWidth ) / nHeight;
	Size aSize;
	Point aPoint;
	if (dRatio>dRatioPreV)
	{
		aSize=Size(nWidth, (sal_uInt16)(nWidth/dRatio));
		aPoint=Point( 0, (sal_uInt16)((nHeight-aSize.Height())/2));
	}
	else
	{
		aSize=Size((sal_uInt16)(nHeight*dRatio), nHeight);
		aPoint=Point((sal_uInt16)((nWidth-aSize.Width())/2),0);
	}
	Point bPoint=Point(nWidth,nHeight)-aPoint;


	pWindow->SetLineColor();
	Color aLightGrayCol( COL_LIGHTGRAY );
	pWindow->SetFillColor( aLightGrayCol );
	pWindow->DrawRect( Rectangle( Point( 0,0 ), pWindow->GetOutputSize() ) );
	if ( pFile )
	{
		Color aBlackCol( COL_BLACK );
		Color aWhiteCol( COL_WHITE );
		pWindow->SetLineColor( aBlackCol );
		pWindow->SetFillColor( aWhiteCol );
		pWindow->DrawRect( Rectangle( aPoint + Point( FRAME, FRAME ), bPoint + Point( FRAME, FRAME ) ) );
//!		pFile->Move( Point( FRAME, FRAME ) );
//!		pFile->Scale( Fraction( aTmpSize.Width(), aSize.Width() ),
//!					  Fraction( aTmpSize.Height(), aSize.Height() ) );
		pFile->WindStart();
		pFile->Play( pWindow, aPoint + Point( FRAME, FRAME ), aSize  );
	}
}

void SfxPreviewWin_Impl::Paint( const Rectangle& rRect )
{
	ImpPaint( rRect, pMetaFile.get(), this );
}

SfxPreviewWin::SfxPreviewWin(
	Window* pParent, const ResId& rResId, SfxObjectShellLock &rDocSh )
	: Window(pParent, rResId), rDocShell( rDocSh )
{
	SetHelpId( HID_PREVIEW_FRAME );

    // adjust contrast mode initially
    bool bUseContrast = UseHighContrastSetting();
    SetDrawMode( bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    // #107818# This preview window is for document previews.  Therefore
    // right-to-left mode should be off
    EnableRTL( sal_False );
}

void SfxPreviewWin::Paint( const Rectangle& rRect )
{
	SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &rDocShell );
	if ( pFrame && pFrame->GetViewShell() &&
		 pFrame->GetViewShell()->GetPrinter() &&
		 pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
    {
        return;
    }

    Size            aTmpSize( rDocShell->GetFirstPageSize() );
    GDIMetaFile     aMtf;
    VirtualDevice   aDevice;

    DBG_ASSERT( aTmpSize.Height() * aTmpSize.Width(), "size of first page is 0, overload GetFirstPageSize or set vis-area!" );

    aMtf.SetPrefSize( aTmpSize );
    aDevice.EnableOutput( sal_False );
    aDevice.SetMapMode( rDocShell->GetMapUnit() );
    aDevice.SetDrawMode( GetDrawMode() );
    aMtf.Record( &aDevice );
    rDocShell->DoDraw( &aDevice, Point(0,0), aTmpSize, JobSetup(), ASPECT_THUMBNAIL );
    aMtf.Stop();
    aMtf.WindStart();
    SfxPreviewWin_Impl::ImpPaint( rRect, &aMtf, this );
}

void SfxPreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
	Window::DataChanged( rDCEvt );

    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        // adjust contrast mode
        bool bUseContrast = UseHighContrastSetting();
        SetDrawMode( bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
    }
}

bool SfxPreviewWin::UseHighContrastSetting() const
{
    return GetSettings().GetStyleSettings().GetHighContrastMode();
}


class SfxNewFileDialog_Impl
{
	FixedText aRegionFt;
	ListBox aRegionLb;
	FixedText aTemplateFt;
	ListBox aTemplateLb;

	CheckBox aPreviewBtn;
	SfxPreviewWin_Impl aPreviewWin;

	FixedText aTitleFt;
	Edit aTitleEd;
	FixedText aThemaFt;
	Edit aThemaEd;
	FixedText aKeywordsFt;
	Edit aKeywordsEd;
	FixedText aDescFt;
	MultiLineEdit aDescEd;
    FixedLine aDocinfoGb;

    CheckBox aTextStyleCB;
    CheckBox aFrameStyleCB;
    CheckBox aPageStyleCB;
    CheckBox aNumStyleCB;
    CheckBox aMergeStyleCB;
    PushButton aLoadFilePB;

    OKButton aOkBt;
	CancelButton aCancelBt;
	HelpButton aHelpBt;
	MoreButton* pMoreBt;
	Timer aPrevTimer;
	String aNone;
    String sLoadTemplate;

	sal_uInt16 nFlags;
	SfxDocumentTemplates aTemplates;
	SfxObjectShellLock xDocShell;
	SfxNewFileDialog* pAntiImpl;

	void ClearInfo();
	DECL_LINK( Update, void * );

	DECL_LINK( RegionSelect, ListBox * );
	DECL_LINK( TemplateSelect, ListBox * );
	DECL_LINK( DoubleClick, ListBox * );
	void TogglePreview(CheckBox *);
	DECL_LINK( Expand, MoreButton * );
	DECL_LINK( PreviewClick, CheckBox * );
    DECL_LINK( LoadFile, PushButton* );
	sal_uInt16  GetSelectedTemplatePos() const;

public:

	SfxNewFileDialog_Impl( SfxNewFileDialog* pAntiImplP, sal_uInt16 nFlags );
	~SfxNewFileDialog_Impl();

		// Liefert sal_False, wenn '- Keine -' als Vorlage eingestellt ist
		// Nur wenn IsTemplate() sal_True liefert, koennen Vorlagennamen
		// erfragt werden
	sal_Bool IsTemplate() const;
	String GetTemplateRegion() const;
	String GetTemplateName() const;
	String GetTemplateFileName() const;

    sal_uInt16  GetTemplateFlags()const;
    void    SetTemplateFlags(sal_uInt16 nSet);
};


//-------------------------------------------------------------------------

void SfxNewFileDialog_Impl::ClearInfo()
{
	const String aNo;
	aTitleEd.SetText(aNo);
	aThemaEd.SetText(aNo);
	aKeywordsEd.SetText(aNo);
	aDescEd.SetText(aNo);
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, Update, void *, EMPTYARG )
{
	if ( xDocShell.Is() )
	{
		if ( xDocShell->GetProgress() )
			return sal_False;
		xDocShell.Clear();
	}

	const sal_uInt16 nEntry = GetSelectedTemplatePos();
	if(!nEntry)
	{
		ClearInfo();
		aPreviewWin.Invalidate();
		aPreviewWin.SetObjectShell( 0);
		return 0;
	}

	if ( aPreviewBtn.IsChecked() && (nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
	{

        String aFileName = aTemplates.GetPath( aRegionLb.GetSelectEntryPos(), nEntry-1);
        INetURLObject aTestObj( aFileName );
        if( aTestObj.GetProtocol() == INET_PROT_NOT_VALID )
        {
            // temp. fix until Templates are managed by UCB compatible service
            // does NOT work with locally cached components !
            String aTemp;
            utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aTemp );
            aFileName = aTemp;
        }

        INetURLObject aObj( aFileName );
		for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
			  pTmp;
			  pTmp = SfxObjectShell::GetNext(*pTmp) )
		{
			//! fsys bug op==
			if ( pTmp->GetMedium())
				// ??? HasName() MM
                if( INetURLObject( pTmp->GetMedium()->GetName() ) == aObj )
				{
					xDocShell = pTmp;
					break;
				}
		}

		if ( !xDocShell.Is() )
		{
			Window *pParent = Application::GetDefDialogParent();
			Application::SetDefDialogParent( pAntiImpl );
			SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,pAntiImpl);
			SfxApplication *pSfxApp = SFX_APP();
			sal_uIntPtr lErr;
			SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
			pSet->Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
			pSet->Put( SfxBoolItem( SID_PREVIEW, sal_True ) );
			lErr = pSfxApp->LoadTemplate( xDocShell, aFileName, sal_True, pSet );
			if( lErr )
				ErrorHandler::HandleError(lErr);
			Application::SetDefDialogParent( pParent );
			if ( !xDocShell.Is() )
			{
				aPreviewWin.SetObjectShell( 0 );
				return sal_False;
			}
		}

		aPreviewWin.SetObjectShell( xDocShell );
	}
	return sal_True;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, RegionSelect, ListBox *, pBox )
{
	if ( xDocShell.Is() && xDocShell->GetProgress() )
		return 0;

	const sal_uInt16 nRegion = pBox->GetSelectEntryPos();
	const sal_uInt16 nCount = aTemplates.GetRegionCount()? aTemplates.GetCount(nRegion): 0;
	aTemplateLb.SetUpdateMode(sal_False);
	aTemplateLb.Clear();
	String aSel=aRegionLb.GetSelectEntry();
	sal_uInt16 nc=aSel.Search('(');
	if (nc-1&&nc!=STRING_NOTFOUND)
		aSel.Erase(nc-1);
	if (aSel.CompareIgnoreCaseToAscii( String(SfxResId(STR_STANDARD)) )==COMPARE_EQUAL)
		aTemplateLb.InsertEntry(aNone);
	for (sal_uInt16 i = 0; i < nCount; ++i)
		aTemplateLb.InsertEntry(aTemplates.GetName(nRegion, i));
	aTemplateLb.SelectEntryPos(0);
	aTemplateLb.SetUpdateMode(sal_True);
	aTemplateLb.Invalidate();
	aTemplateLb.Update();
	return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, Expand, MoreButton *, EMPTYARG )
{
	TemplateSelect(&aTemplateLb);
	return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, Expand, MoreButton *, pMoreButton )

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, PreviewClick, CheckBox *, pBox )
{
	if ( xDocShell.Is() && xDocShell->GetProgress() )
		return 0;

	sal_uInt16 nEntry = GetSelectedTemplatePos();
	if ( nEntry && pBox->IsChecked() )
	{
		if(!Update(0))
			aPreviewWin.Invalidate();
	}
	else
	{
		if (xDocShell.Is())
			xDocShell.Clear();
		aPreviewWin.SetObjectShell( 0 );
	}
	return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, TemplateSelect, ListBox *, EMPTYARG )
{
	// noch am Laden
	if ( xDocShell && xDocShell->GetProgress() )
		return 0;

	if ( !MORE_BTN(GetState()) )
		// Dialog nicht aufgeklappt
		return 0;
    if ( aPreviewBtn.IsChecked() )
        aPrevTimer.Start();
	return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )
{
    (void)pListBox;
    // noch am Laden
	if ( !xDocShell.Is() || !xDocShell->GetProgress() )
		pAntiImpl->EndDialog(RET_OK);
	return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, LoadFile, PushButton *, EMPTYARG )
{
    pAntiImpl->EndDialog(RET_TEMPLATE_LOAD);
	return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, LoadFile, PushButton *, EMPTYARG )
//-------------------------------------------------------------------------

sal_uInt16  SfxNewFileDialog_Impl::GetSelectedTemplatePos() const
{
	sal_uInt16 nEntry=aTemplateLb.GetSelectEntryPos();
	String aSel=aRegionLb.GetSelectEntry().Copy();
	sal_uInt16 nc=aSel.Search('(');
	if (nc-1&&nc!=STRING_NOTFOUND)
		aSel.Erase(nc-1);
	if (aSel.CompareIgnoreCaseToAscii(String(SfxResId(STR_STANDARD)))!=COMPARE_EQUAL)
		nEntry++;
	if (!aTemplateLb.GetSelectEntryCount())
		nEntry=0;
	return nEntry;
}

//-------------------------------------------------------------------------

sal_Bool SfxNewFileDialog_Impl::IsTemplate() const
{
	return GetSelectedTemplatePos()!=0;

}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateFileName() const
{
	if(!IsTemplate() || !aTemplates.GetRegionCount())
		return String();
	return aTemplates.GetPath(aRegionLb.GetSelectEntryPos(),
							  GetSelectedTemplatePos()-1);
}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateRegion() const
{
	if(!IsTemplate() || !aTemplates.GetRegionCount())
		return String();
	return aRegionLb.GetSelectEntry();
}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateName() const
{
	if(!IsTemplate() || !aTemplates.GetRegionCount())
		return String();
	return aTemplateLb.GetSelectEntry();
}

//-------------------------------------------------------------------------

void AdjustPosSize_Impl(Window *pWin, short nMoveOffset, short nSizeOffset)
{
	Point aPos(pWin->GetPosPixel());
	Size aSize(pWin->GetSizePixel());
	aPos.X() -= nMoveOffset;
	aSize.Width() += nSizeOffset;
	pWin->SetPosSizePixel(aPos, aSize);
}
//-------------------------------------------------------------------------
sal_uInt16  SfxNewFileDialog_Impl::GetTemplateFlags()const
{
    sal_uInt16 nRet = aTextStyleCB.IsChecked() ? SFX_LOAD_TEXT_STYLES : 0;
    if(aFrameStyleCB.IsChecked())
        nRet |= SFX_LOAD_FRAME_STYLES;
    if(aPageStyleCB.IsChecked())
        nRet |= SFX_LOAD_PAGE_STYLES;
    if(aNumStyleCB.IsChecked())
        nRet |= SFX_LOAD_NUM_STYLES;
    if(aMergeStyleCB.IsChecked())
        nRet |= SFX_MERGE_STYLES;
    return nRet;
}
//-------------------------------------------------------------------------
void    SfxNewFileDialog_Impl::SetTemplateFlags(sal_uInt16 nSet)
{
    aTextStyleCB.Check(  0 != (nSet&SFX_LOAD_TEXT_STYLES ));
    aFrameStyleCB.Check( 0 != (nSet&SFX_LOAD_FRAME_STYLES));
    aPageStyleCB.Check(  0 != (nSet&SFX_LOAD_PAGE_STYLES ));
    aNumStyleCB.Check(   0 != (nSet&SFX_LOAD_NUM_STYLES  ));
    aMergeStyleCB.Check( 0 != (nSet&SFX_MERGE_STYLES     ));
}

//-------------------------------------------------------------------------

SfxNewFileDialog_Impl::SfxNewFileDialog_Impl(
	SfxNewFileDialog* pAntiImplP, sal_uInt16 nFl)
	:	aRegionFt( pAntiImplP, SfxResId( FT_REGION ) ),
		aRegionLb( pAntiImplP, SfxResId( LB_REGION ) ),
		aTemplateFt( pAntiImplP, SfxResId( FT_TEMPLATE ) ),
		aTemplateLb( pAntiImplP, SfxResId( LB_TEMPLATE ) ),
		aPreviewBtn( pAntiImplP, SfxResId( BTN_PREVIEW ) ),
		aPreviewWin( pAntiImplP, SfxResId( WIN_PREVIEW ) ),
		aTitleFt( pAntiImplP, SfxResId( FT_TITLE ) ),
		aTitleEd( pAntiImplP, SfxResId( ED_TITLE ) ),
		aThemaFt( pAntiImplP, SfxResId( FT_THEMA ) ),
		aThemaEd( pAntiImplP, SfxResId( ED_THEMA ) ),
		aKeywordsFt( pAntiImplP, SfxResId( FT_KEYWORDS ) ),
		aKeywordsEd( pAntiImplP, SfxResId( ED_KEYWORDS ) ),
		aDescFt( pAntiImplP, SfxResId( FT_DESC ) ),
		aDescEd( pAntiImplP, SfxResId( ED_DESC ) ),
		aDocinfoGb( pAntiImplP, SfxResId( GB_DOCINFO ) ),
        aTextStyleCB( pAntiImplP, SfxResId(  CB_TEXT_STYLE )),
        aFrameStyleCB( pAntiImplP, SfxResId( CB_FRAME_STYLE )),
        aPageStyleCB( pAntiImplP, SfxResId(  CB_PAGE_STYLE )),
        aNumStyleCB( pAntiImplP, SfxResId(   CB_NUM_STYLE  )),
        aMergeStyleCB( pAntiImplP, SfxResId( CB_MERGE_STYLE )),
        aLoadFilePB( pAntiImplP, SfxResId(   PB_LOAD_FILE )),
	    aOkBt( pAntiImplP, SfxResId( BT_OK ) ),
		aCancelBt( pAntiImplP, SfxResId( BT_CANCEL ) ),
		aHelpBt( pAntiImplP, SfxResId( BT_HELP ) ),
		pMoreBt( new MoreButton( pAntiImplP, SfxResId( BT_MORE ) ) ),
		aNone( SfxResId(STR_NONE) ),
        sLoadTemplate( SfxResId(STR_LOAD_TEMPLATE)),
		nFlags(nFl),
		pAntiImpl( pAntiImplP )
{
	short nMoveOffset = *(short *)pAntiImplP->GetClassRes();
	pAntiImplP->IncrementRes(sizeof(short));
	short nExpandSize= *(short *)pAntiImplP->GetClassRes();
	pAntiImplP->IncrementRes(sizeof(short));
	pAntiImplP->FreeResource();

	if (!nFlags)
		MORE_BTN(Hide());
    else if(SFXWB_LOAD_TEMPLATE == nFlags)
    {
        aLoadFilePB.SetClickHdl(LINK(this, SfxNewFileDialog_Impl, LoadFile));
        aLoadFilePB.Show();
        aTextStyleCB.Show();
        aFrameStyleCB.Show();
        aPageStyleCB.Show();
        aNumStyleCB.Show();
        aMergeStyleCB.Show();
        Size aSize(pAntiImplP->GetOutputSizePixel());
        Size aTmp(pAntiImplP->LogicToPixel(Size(16, 16), MAP_APPFONT));
        aSize.Height() += aTmp.Height();
        pAntiImplP->SetOutputSizePixel(aSize);
        pMoreBt->Hide();
        aTextStyleCB.Check();
        pAntiImplP->SetText(sLoadTemplate);
    }
    else
	{
		MORE_BTN(SetClickHdl(LINK(this, SfxNewFileDialog_Impl, Expand)));
		if((nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
		{
			MORE_BTN(AddWindow(&aPreviewBtn));
			MORE_BTN(AddWindow(&aPreviewWin));
			aPreviewBtn.SetClickHdl(LINK(this, SfxNewFileDialog_Impl, PreviewClick));
            aTemplateLb.SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, TemplateSelect));
		}
		else
		{
			aPreviewBtn.Hide();
			aPreviewWin.Hide();
			nMoveOffset = (short)pAntiImplP->LogicToPixel(
				Size(nMoveOffset, nMoveOffset), MAP_APPFONT).Width();
			nExpandSize = (short)pAntiImplP->LogicToPixel(
				Size(nExpandSize, nExpandSize), MAP_APPFONT).Width();
			AdjustPosSize_Impl(&aTitleFt, nMoveOffset, 0);
			AdjustPosSize_Impl(&aTitleEd, nMoveOffset, nExpandSize);
			AdjustPosSize_Impl(&aThemaFt, nMoveOffset, 0);
			AdjustPosSize_Impl(&aThemaEd, nMoveOffset, nExpandSize);
			AdjustPosSize_Impl(&aKeywordsFt, nMoveOffset, 0);
			AdjustPosSize_Impl(&aKeywordsEd, nMoveOffset, nExpandSize);
			AdjustPosSize_Impl(&aDescFt , nMoveOffset, 0);
			AdjustPosSize_Impl(&aDescEd , nMoveOffset, nExpandSize);
			AdjustPosSize_Impl(&aDocinfoGb, nMoveOffset, nExpandSize);
		}
	}

	String &rExtra = pAntiImplP->GetExtraData();
	sal_uInt16 nTokCount = rExtra.GetTokenCount( '|' );
	if( nTokCount > 0 && nFlags )
		MORE_BTN(SetState( rExtra.GetToken( 0, '|' ) == 'Y' ));
	if( nTokCount > 1 && nFlags )
		aPreviewBtn.Check( rExtra.GetToken( 1 ,'|' ) == 'Y' );

	aTemplateLb.SetDoubleClickHdl(LINK(this, SfxNewFileDialog_Impl, DoubleClick));

	// update the template configuration if necessary
	{
		WaitObject aWaitCursor( pAntiImplP->GetParent() );
		aTemplates.Update( sal_True /* be smart */ );
	}
	// fill the list boxes
	const sal_uInt16 nCount = aTemplates.GetRegionCount();
	if (nCount)
	{
		for(sal_uInt16 i = 0; i < nCount; ++i)
			aRegionLb.InsertEntry(aTemplates.GetFullRegionName(i));
		aRegionLb.SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, RegionSelect));
	}

	aPrevTimer.SetTimeout( 500 );
	aPrevTimer.SetTimeoutHdl( LINK( this, SfxNewFileDialog_Impl, Update));

//	 else
//		  aRegionLb.InsertEntry(String(SfxResId(STR_STANDARD)));
	aRegionLb.SelectEntryPos(0);
	RegionSelect(&aRegionLb);
}

//-------------------------------------------------------------------------

SfxNewFileDialog_Impl::~SfxNewFileDialog_Impl()
{
	String &rExtra = pAntiImpl->GetExtraData();
	rExtra = MORE_BTN(GetState()) ? 'Y' : 'N';
	rExtra += '|';
	rExtra += aPreviewBtn.IsChecked() ? 'Y' : 'N';

	delete pMoreBt;
}
//-------------------------------------------------------------------------
SfxNewFileDialog::SfxNewFileDialog(Window *pParent, sal_uInt16 nFlags)
	: SfxModalDialog( pParent, SfxResId( DLG_NEW_FILE ) )
{
	pImpl = new SfxNewFileDialog_Impl( this, nFlags );
}
//-------------------------------------------------------------------------
SfxNewFileDialog::~SfxNewFileDialog()
{
	delete pImpl;
}
//-------------------------------------------------------------------------
sal_Bool SfxNewFileDialog::IsTemplate() const
{
	return pImpl->IsTemplate();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateRegion() const
{
	return pImpl->GetTemplateRegion();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateName() const
{
	return pImpl->GetTemplateName();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateFileName() const
{
	return pImpl->GetTemplateFileName();
}
//-------------------------------------------------------------------------
sal_uInt16 SfxNewFileDialog::GetTemplateFlags()const
{
    return pImpl->GetTemplateFlags();

}
//-------------------------------------------------------------------------
void    SfxNewFileDialog::SetTemplateFlags(sal_uInt16 nSet)
{
    pImpl->SetTemplateFlags(nSet);
}

