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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

//svdraw.hxx
#define _SVDRAW_HXX
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS
#define _SDR_NOVIEWS
#define _SDR_NOVIEWMARKER
#define _SDR_NODRAGMETHODS
#define _SDR_NOUNDO
#define _SDR_NOXOUTDEV
#include <vcl/wrkwin.hxx>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <svx/dialmgr.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TPBITMAP_CXX

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/xoutbmp.hxx"
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "tabarea.hrc"
#include "defdlgname.hxx" //CHINA001 #include "dlgname.hxx"
//#include "dlgname.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <dialmgr.hxx>
#include "sfx2/opengrf.hxx"
#include "paragrph.hrc"

#define DLGWIN this->GetParent()->GetParent()

/*************************************************************************
|*
|*	Dialog zum Aendern und Definieren der Bitmaps
|*
\************************************************************************/

SvxBitmapTabPage::SvxBitmapTabPage
(
	Window* pParent,
	const SfxItemSet& rInAttrs
) :

	SvxTabPage			( pParent, CUI_RES( RID_SVXPAGE_BITMAP ), rInAttrs ),

    aCtlPixel           ( this, CUI_RES( CTL_PIXEL ) ),
    aFtPixelEdit        ( this, CUI_RES( FT_PIXEL_EDIT ) ),
    aFtColor            ( this, CUI_RES( FT_COLOR ) ),
	aLbColor			( this, CUI_RES( LB_COLOR ) ),
	aFtBackgroundColor	( this, CUI_RES( FT_BACKGROUND_COLOR ) ),
	aLbBackgroundColor	( this, CUI_RES( LB_BACKGROUND_COLOR ) ),
    // This fix text is used only to provide the name for the following
    // bitmap list box.  The fixed text is not displayed.
    aLbBitmapsHidden    ( this, CUI_RES( FT_BITMAPS_HIDDEN ) ),
	aLbBitmaps			( this, CUI_RES( LB_BITMAPS ) ),
    aFlProp             ( this, CUI_RES( FL_PROP ) ),
	aCtlPreview			( this, CUI_RES( CTL_PREVIEW ) ),
	aBtnAdd				( this, CUI_RES( BTN_ADD ) ),
    aBtnModify          ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnImport          ( this, CUI_RES( BTN_IMPORT ) ),
    aBtnDelete          ( this, CUI_RES( BTN_DELETE ) ),
	aBtnLoad			( this, CUI_RES( BTN_LOAD ) ),
	aBtnSave			( this, CUI_RES( BTN_SAVE ) ),

    aBitmapCtl          ( this, aCtlPreview.GetSizePixel() ),
    rOutAttrs           ( rInAttrs ),

    maColorTab(),
    maBitmapList(),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_BITMAP ),
    aXBitmapItem        ( String(), Graphic() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
	aBtnLoad.SetModeImage( Image( CUI_RES( RID_SVXIMG_LOAD_H ) ), BMP_COLOR_HIGHCONTRAST );
	aBtnSave.SetModeImage( Image( CUI_RES( RID_SVXIMG_SAVE_H ) ), BMP_COLOR_HIGHCONTRAST );
	FreeResource();

	// diese Page braucht ExchangeSupport
	SetExchangeSupport();

	// Setzen des Output-Devices
	rXFSet.Put( aXFStyleItem );
	rXFSet.Put( aXBitmapItem );
	//aCtlPreview.SetAttributes( aXFillAttr );

	aBtnAdd.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickAddHdl_Impl ) );
	aBtnImport.SetClickHdl(
		LINK( this, SvxBitmapTabPage, ClickImportHdl_Impl ) );
	aBtnModify.SetClickHdl(
		LINK( this, SvxBitmapTabPage, ClickModifyHdl_Impl ) );
	aBtnDelete.SetClickHdl(
		LINK( this, SvxBitmapTabPage, ClickDeleteHdl_Impl ) );
	aBtnLoad.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickLoadHdl_Impl ) );
	aBtnSave.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickSaveHdl_Impl ) );

	aLbBitmaps.SetSelectHdl(
		LINK( this, SvxBitmapTabPage, ChangeBitmapHdl_Impl ) );
	aLbColor.SetSelectHdl(
		LINK( this, SvxBitmapTabPage, ChangePixelColorHdl_Impl ) );
	aLbBackgroundColor.SetSelectHdl(
		LINK( this, SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl ) );

	String accName = String(CUI_RES(STR_EXAMPLE)); 
	aCtlPreview.SetAccessibleName(accName); 
	aCtlPixel.SetAccessibleRelationMemberOf( &aFlProp );
	aCtlPixel.SetAccessibleRelationLabeledBy( &aFtPixelEdit );
	aLbBitmaps.SetAccessibleRelationLabeledBy(&aLbBitmaps);
	aBtnAdd.SetAccessibleRelationMemberOf( &aFlProp );
	aBtnModify.SetAccessibleRelationMemberOf( &aFlProp );
	aBtnImport.SetAccessibleRelationMemberOf( &aFlProp );
	aBtnDelete.SetAccessibleRelationMemberOf( &aFlProp );	

}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Construct()
{
	// Farbtabellen
	aLbColor.Fill( maColorTab );
	aLbBackgroundColor.CopyEntries( aLbColor );

	// Bitmaptabelle
	aLbBitmaps.Fill( maBitmapList );
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::ActivatePage( const SfxItemSet&  )
{
	sal_uInt16 nPos;
	sal_uInt16 nCount;

	if( *pDlgType == 0 ) // Flaechen-Dialog
	{
		*pbAreaTP = sal_False;

		if( maColorTab.get() )
		{
			// ColorTable
			if( *pnColorTableState & CT_CHANGED ||
				*pnColorTableState & CT_MODIFIED )
			{
				if( *pnColorTableState & CT_CHANGED )
					maColorTab = ( (SvxAreaTabDialog*) DLGWIN )->GetNewColorTable();

				// LbColor
				nPos = aLbColor.GetSelectEntryPos();
				aLbColor.Clear();
				aLbColor.Fill( maColorTab );
				nCount = aLbColor.GetEntryCount();
				if( nCount == 0 )
					; // Dieser Fall sollte nicht auftreten
				else if( nCount <= nPos )
					aLbColor.SelectEntryPos( 0 );
				else
					aLbColor.SelectEntryPos( nPos );

				// LbColorBackground
				nPos = aLbBackgroundColor.GetSelectEntryPos();
				aLbBackgroundColor.Clear();
				aLbBackgroundColor.CopyEntries( aLbColor );
				nCount = aLbBackgroundColor.GetEntryCount();
				if( nCount == 0 )
					; // Dieser Fall sollte nicht auftreten
				else if( nCount <= nPos )
					aLbBackgroundColor.SelectEntryPos( 0 );
				else
					aLbBackgroundColor.SelectEntryPos( nPos );

				ChangePixelColorHdl_Impl( this );
				ChangeBackgrndColorHdl_Impl( this );
			}

			// Ermitteln (evtl. abschneiden) des Namens und in
			// der GroupBox darstellen
			String			aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
			INetURLObject	aURL( maBitmapList->GetPath() );

			aURL.Append( maBitmapList->GetName() );
			DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

			if( aURL.getBase().getLength() > 18 )
			{
				aString += String(aURL.getBase()).Copy( 0, 15 );
				aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
			}
			else
				aString += String(aURL.getBase());

			if( *pPageType == PT_BITMAP && *pPos != LISTBOX_ENTRY_NOTFOUND )
			{
				aLbBitmaps.SelectEntryPos( *pPos );
			}
			// Farben koennten geloescht worden sein
			ChangeBitmapHdl_Impl( this );

			*pPageType = PT_BITMAP;
			*pPos = LISTBOX_ENTRY_NOTFOUND;
		}
	}
}

// -----------------------------------------------------------------------

int SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet)
{
	if ( CheckChanges_Impl() == -1L )
		return KEEP_PAGE;

    if( _pSet )
        FillItemSet( *_pSet );

	return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

sal_Bool SvxBitmapTabPage::FillItemSet( SfxItemSet& _rOutAttrs )
{
	if( *pDlgType == 0 && *pbAreaTP == sal_False ) // Flaechen-Dialog
	{
		if(PT_BITMAP == *pPageType)
		{
			const sal_uInt16 nPos(aLbBitmaps.GetSelectEntryPos());
            
            _rOutAttrs.Put(XFillStyleItem(XFILL_BITMAP));
            
            if(LISTBOX_ENTRY_NOTFOUND != nPos)
			{
                const XBitmapEntry* pXBitmapEntry = maBitmapList->GetBitmap(nPos);
				const String aString(aLbBitmaps.GetSelectEntry());
                
                _rOutAttrs.Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
			}
			else
			{
				const BitmapEx aBitmapEx(aBitmapCtl.GetBitmapEx());

                _rOutAttrs.Put(XFillBitmapItem(String(), Graphic(aBitmapEx)));
			}
		}
	}

    return sal_True;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Reset( const SfxItemSet&  )
{
	// aLbBitmaps.SelectEntryPos( 0 );

	aBitmapCtl.SetLines( aCtlPixel.GetLineCount() );
	aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );
	aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
	aBitmapCtl.SetBmpArray( aCtlPixel.GetBitmapPixelPtr() );

	// Bitmap holen und darstellen
	const XFillBitmapItem aBmpItem(String(), Graphic(aBitmapCtl.GetBitmapEx()));
	rXFSet.Put( aBmpItem );
	aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
	aCtlPreview.Invalidate();

	ChangeBitmapHdl_Impl( this );

	// Status der Buttons ermitteln
	if( maBitmapList.get() && maBitmapList->Count() )
	{
		aBtnAdd.Enable();
		aBtnModify.Enable();
		aBtnDelete.Enable();
		aBtnSave.Enable();
	}
	else
	{
		aBtnModify.Disable();
		aBtnDelete.Disable();
		aBtnSave.Disable();
	}
}

// -----------------------------------------------------------------------

SfxTabPage* SvxBitmapTabPage::Create( Window* pWindow,
                const SfxItemSet& rSet )
{
    return new SvxBitmapTabPage( pWindow, rSet );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ChangeBitmapHdl_Impl, void *, EMPTYARG )
{
    GraphicObject* pGraphicObject = 0;
	int nPos(aLbBitmaps.GetSelectEntryPos());

	if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        pGraphicObject = new GraphicObject(maBitmapList->GetBitmap(nPos)->GetGraphicObject());
    }
	else
	{
		const SfxPoolItem* pPoolItem = 0;

        if(SFX_ITEM_SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
		{
			const XFillStyle eXFS((XFillStyle)((const XFillStyleItem*)pPoolItem)->GetValue());

            if((XFILL_BITMAP == eXFS) && (SFX_ITEM_SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
			{
                pGraphicObject = new GraphicObject(((const XFillBitmapItem*)pPoolItem)->GetGraphicObject());
			}
		}

        if(!pGraphicObject)
		{
			aLbBitmaps.SelectEntryPos(0);
			nPos = aLbBitmaps.GetSelectEntryPos();
			
            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                pGraphicObject = new GraphicObject(maBitmapList->GetBitmap(nPos)->GetGraphicObject());
            }
		}
	}

    if(pGraphicObject)
	{
        BitmapColor aBack;
        BitmapColor aFront;
        bool bIs8x8(isHistorical8x8(pGraphicObject->GetGraphic().GetBitmap(), aBack, aFront));

        aLbColor.SetNoSelection();
		aLbBackgroundColor.SetNoSelection();

		if(bIs8x8)
		{
			aCtlPixel.SetPaintable( sal_True );
			aCtlPixel.Enable();
            aFtPixelEdit.Enable();
			aFtColor.Enable();
			aLbColor.Enable();
			aFtBackgroundColor.Enable();
			aLbBackgroundColor.Enable();
			aBtnModify.Enable();
			aBtnAdd.Enable();

			// Setzen des PixelControls
			aCtlPixel.SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

			Color aPixelColor = aFront;
			Color aBackColor = aBack;

            // #123564# This causes the wrong color to be selected
            // as foreground color when the 1st bitmap in the bitmap
            // list is selected. I see no reason why this is done,
            // thus I will take it out
            //
            //if( 0 == aLbBitmaps.GetSelectEntryPos() )
            //{
            //	aLbColor.SelectEntry( Color( COL_BLACK ) );
            //	ChangePixelColorHdl_Impl( this );
            //}
            //else

            aLbColor.SelectEntry( aPixelColor );

            if( aLbColor.GetSelectEntryCount() == 0 )
            {
                aLbColor.InsertEntry( aPixelColor, String() );
                aLbColor.SelectEntry( aPixelColor );
            }

            aLbBackgroundColor.SelectEntry( aBackColor );

            if( aLbBackgroundColor.GetSelectEntryCount() == 0 )
            {
                aLbBackgroundColor.InsertEntry( aBackColor, String() );
                aLbBackgroundColor.SelectEntry( aBackColor );
            }

            // update aBitmapCtl, rXFSet and aCtlPreview
            aBitmapCtl.SetPixelColor( aPixelColor );
            aBitmapCtl.SetBackgroundColor( aBackColor );
            rXFSet.Put(XFillBitmapItem(String(), Graphic(aBitmapCtl.GetBitmapEx())));
            aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
            aCtlPreview.Invalidate();
        }
        else
		{
			aCtlPixel.Reset();
			aCtlPixel.SetPaintable( sal_False );
			aCtlPixel.Disable();
            aFtPixelEdit.Disable();
			aFtColor.Disable();
			aLbColor.Disable();
			aFtBackgroundColor.Disable();
			aLbBackgroundColor.Disable();
			aBtnModify.Disable();
			aBtnAdd.Disable();
		}

        aCtlPixel.Invalidate();

		// Bitmap darstellen
		const XFillBitmapItem aXBmpItem(String(), *pGraphicObject);
		rXFSet.Put( aXBmpItem );

		aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
		aCtlPreview.Invalidate();

		bBmpChanged = sal_False;
		delete pGraphicObject;
	}
	
    return 0;
}

// -----------------------------------------------------------------------

long SvxBitmapTabPage::CheckChanges_Impl()
{
	sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();
	if( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		String aString = aLbBitmaps.GetSelectEntry();

		if( bBmpChanged )
		{
			ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
			SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
			DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
			AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( DLGWIN, RID_SVXDLG_MESSBOX,
														String( SVX_RES( RID_SVXSTR_BITMAP ) ),
														String( CUI_RES( RID_SVXSTR_ASK_CHANGE_BITMAP ) ),
														&aWarningBoxImage  );
			DBG_ASSERT(aMessDlg, "Dialogdiet fail!");//CHINA001
			aMessDlg->SetButtonText( MESS_BTN_1, //CHINA001 aMessDlg.SetButtonText( MESS_BTN_1,
									String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
			aMessDlg->SetButtonText( MESS_BTN_2, //CHINA001 aMessDlg.SetButtonText( MESS_BTN_2,
									String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

			short nRet = aMessDlg->Execute(); //CHINA001 short nRet = aMessDlg.Execute();

			switch( nRet )
			{
				case RET_BTN_1: // Aendern
				{
					ClickModifyHdl_Impl( this );
				}
				break;

				case RET_BTN_2: // Hinzufuegen
				{
					ClickAddHdl_Impl( this );
					nPos = aLbBitmaps.GetSelectEntryPos();
				}
				break;

				case RET_CANCEL:
					// return( -1L );
				break;
				// return( sal_True ); // Abbruch
			}
			delete aMessDlg; //add by CHINA001
		}
	}
	nPos = aLbBitmaps.GetSelectEntryPos();
	if( nPos != LISTBOX_ENTRY_NOTFOUND )
		*pPos = nPos;
	return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
	ResMgr& rMgr = CUI_MGR();
	String aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
	String aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_BITMAP ) );
	String aName;

	long nCount = maBitmapList.get() ? maBitmapList->Count() : 0;
	long j = 1;
	sal_Bool bDifferent = sal_False;

	while( !bDifferent )
	{
		aName  = aNewName;
		aName += sal_Unicode(' ');
		aName += UniString::CreateFromInt32( j++ );
		bDifferent = sal_True;

		for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == maBitmapList->GetBitmap( i )->GetName() )
				bDifferent = sal_False;
	}

	SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
	DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
	AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
	DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
	WarningBox*    pWarnBox = NULL;
	sal_uInt16         nError   = RID_SVXSTR_WARN_NAME_DUPLICATE;

	while( pDlg->Execute() == RET_OK )
	{
		pDlg->GetName( aName );

		bDifferent = sal_True;

		for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == maBitmapList->GetBitmap( i )->GetName() )
				bDifferent = sal_False;

		if( bDifferent ) {
			nError = 0;
			break;
		}

		if( !pWarnBox )
		{
			pWarnBox = new WarningBox( DLGWIN,
									   WinBits( WB_OK_CANCEL ),
									   String( ResId( nError, rMgr ) ) );
			pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
		}

		if( pWarnBox->Execute() != RET_OK )
			break;
	}

    delete pDlg;
	delete pWarnBox;

	if( !nError )
	{
		XBitmapEntry* pEntry = 0;
		if( aCtlPixel.IsEnabled() )
		{
			const BitmapEx aBitmapEx(aBitmapCtl.GetBitmapEx());

			pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
		}
		else // Es muss sich um eine nicht vorhandene importierte Bitmap handeln
		{
			const SfxPoolItem* pPoolItem = 0;

            if(SFX_ITEM_SET == rOutAttrs.GetItemState(XATTR_FILLBITMAP, true, &pPoolItem))
			{
                pEntry = new XBitmapEntry(dynamic_cast< const XFillBitmapItem* >(pPoolItem)->GetGraphicObject(), aName);
			}
		}

		DBG_ASSERT( pEntry, "SvxBitmapTabPage::ClickAddHdl_Impl(), pEntry == 0 ?" );

		if( pEntry )
		{
			maBitmapList->Insert( pEntry );
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
			aLbBitmaps.Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
			aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

			// Flag fuer modifiziert setzen
			*pnBitmapListState |= CT_MODIFIED;

			ChangeBitmapHdl_Impl( this );
		}
	}

	// Status der Buttons ermitteln
	if( maBitmapList.get() && maBitmapList->Count() )
	{
		aBtnModify.Enable();
		aBtnDelete.Enable();
		aBtnSave.Enable();
	}
	return 0L;
}


/******************************************************************************/
/******************************************************************************/


//------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

IMPL_LINK( SvxBitmapTabPage, ClickImportHdl_Impl, void *, EMPTYARG )
{
	ResMgr& rMgr = CUI_MGR();
	SvxOpenGraphicDialog aDlg( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Import" ) ) );
	aDlg.EnableLink(sal_False);

	if( !aDlg.Execute() )
	{
		Graphic 		aGraphic;
		int 			nError = 1;

		EnterWait();
		nError = aDlg.GetGraphic( aGraphic );
		LeaveWait();

		if( !nError )
		{
			String aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
			WarningBox*    pWarnBox = NULL;

			// convert file URL to UI name
			String 			aName;
			INetURLObject	aURL( aDlg.GetPath() );
			SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
			DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
			AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, String(aURL.GetName()).GetToken( 0, '.' ), aDesc );
			DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
			nError = RID_SVXSTR_WARN_NAME_DUPLICATE;

			while( pDlg->Execute() == RET_OK )
			{
				pDlg->GetName( aName );

				sal_Bool bDifferent = sal_True;
				long nCount     = maBitmapList->Count();

				for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == maBitmapList->GetBitmap( i )->GetName() )
						bDifferent = sal_False;

				if( bDifferent ) {
					nError = 0;
					break;
				}

				if( !pWarnBox )
				{
					pWarnBox = new WarningBox( DLGWIN,
											   WinBits( WB_OK_CANCEL ),
											   String( ResId( nError, rMgr ) ) );
					pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
				}


				if( pWarnBox->Execute() != RET_OK )
					break;
			}

            delete pDlg;
			delete pWarnBox;

			if( !nError )
			{
				XBitmapEntry* pEntry = new XBitmapEntry( aGraphic, aName );
				maBitmapList->Insert( pEntry );

                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                aLbBitmaps.Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
				aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

				// Flag fuer modifiziert setzen
				*pnBitmapListState |= CT_MODIFIED;

				ChangeBitmapHdl_Impl( this );
			}
		}
		else
			// Graphik konnte nicht geladen werden
			ErrorBox( DLGWIN,
					  WinBits( WB_OK ),
					  String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
	}

	return 0L;
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();

	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		ResMgr& rMgr = CUI_MGR();
		String aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
		String aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, rMgr ) );
        String aName( maBitmapList->GetBitmap( nPos )->GetName() );
		String aOldName = aName;

		SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
		DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
		AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
		DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

		long nCount = maBitmapList.get() ? maBitmapList->Count() : 0;
		sal_Bool bDifferent = sal_False;
		sal_Bool bLoop = sal_True;
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

		while( bLoop && pDlg->Execute() == RET_OK )
		{
			pDlg->GetName( aName );
			bDifferent = sal_True;

			for( long i = 0; i < nCount && bDifferent; i++ )
			{
                if( aName == maBitmapList->GetBitmap( i )->GetName() &&
					aName != aOldName )
					bDifferent = sal_False;
			}

			if( bDifferent )
			{
				bLoop = sal_False;

				const BitmapEx aBitmapEx(aBitmapCtl.GetBitmapEx());

                // #123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
                XBitmapEntry* pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
                delete maBitmapList->Replace(pEntry, nPos);

				aLbBitmaps.Modify( rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry, nPos );
				aLbBitmaps.SelectEntryPos( nPos );

				// Flag fuer modifiziert setzen
				*pnBitmapListState |= CT_MODIFIED;

				bBmpChanged = sal_False;
			}
			else
			{
				WarningBox aBox( DLGWIN, WinBits( WB_OK ), String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
				aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
				aBox.Execute();
			}
		}
		delete pDlg;
	}
	return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();

	if( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
			String( CUI_RES( RID_SVXSTR_ASK_DEL_BITMAP ) ) );

		if( aQueryBox.Execute() == RET_YES )
		{
			delete maBitmapList->Remove( nPos );
			aLbBitmaps.RemoveEntry( nPos );
			aLbBitmaps.SelectEntryPos( 0 );

			aCtlPreview.Invalidate();
			aCtlPixel.Invalidate();

			ChangeBitmapHdl_Impl( this );

			// Flag fuer modifiziert setzen
			*pnBitmapListState |= CT_MODIFIED;
		}
	}
	// Status der Buttons ermitteln
	if( !maBitmapList.get() || !maBitmapList->Count() )
	{
		aBtnModify.Disable();
		aBtnDelete.Disable();
		aBtnSave.Disable();
	}
	return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickLoadHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nReturn = RET_YES;
	ResMgr& rMgr = CUI_MGR();

	if ( *pnBitmapListState & CT_MODIFIED )
	{
		nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
			String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

		if ( nReturn == RET_YES )
			maBitmapList->Save();
	}

	if ( nReturn != RET_CANCEL )
	{
    	::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
		String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
		aDlg.AddFilter( aStrFilterType, aStrFilterType );
		INetURLObject aFile( SvtPathOptions().GetPalettePath() );
		aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

    	if ( aDlg.Execute() == ERRCODE_NONE )
		{
			EnterWait();
			INetURLObject aURL( aDlg.GetPath() );
			INetURLObject aPathURL( aURL );

			aPathURL.removeSegment();
			aPathURL.removeFinalSlash();

			// Tabelle speichern
			XBitmapListSharedPtr aBmpList(XPropertyListFactory::CreateSharedXBitmapList(aPathURL.GetMainURL(INetURLObject::NO_DECODE)));
			aBmpList->SetName( aURL.getName() );
			if( aBmpList->Load() )
			{
				if( aBmpList.get() )
				{
					maBitmapList = aBmpList;
					( (SvxAreaTabDialog*) DLGWIN )->SetNewBitmapList( maBitmapList );

					aLbBitmaps.Clear();
					aLbBitmaps.Fill( maBitmapList );
					Reset( rOutAttrs );

					maBitmapList->SetName( aURL.getName() );

					// Ermitteln (evtl. abschneiden) des Namens und in
					// der GroupBox darstellen
					String aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
					aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

					if ( aURL.getBase().getLength() > 18 )
					{
						aString += String(aURL.getBase()).Copy( 0, 15 );
						aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
					}
					else
						aString += String(aURL.getBase());

					// Flag fuer gewechselt setzen
					*pnBitmapListState |= CT_CHANGED;
					// Flag fuer modifiziert entfernen
					*pnBitmapListState &= ~CT_MODIFIED;
				}
				LeaveWait();
			}
			else
			{
				LeaveWait();
				ErrorBox( DLGWIN, WinBits( WB_OK ),
					String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
			}
		}
	}

	// Status der Buttons ermitteln
	if( maBitmapList.get() && maBitmapList->Count() )
	{
		aBtnModify.Enable();
		aBtnDelete.Enable();
		aBtnSave.Enable();
	}
	else
	{
		aBtnModify.Disable();
		aBtnDelete.Disable();
		aBtnSave.Disable();
	}
	return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ClickSaveHdl_Impl, void *, EMPTYARG )
{
   	::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
	String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
	aDlg.AddFilter( aStrFilterType, aStrFilterType );

	INetURLObject aFile( SvtPathOptions().GetPalettePath() );
	DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

	if( maBitmapList->GetName().Len() )
	{
		aFile.Append( maBitmapList->GetName() );

		if( !aFile.getExtension().getLength() )
			aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "sob" ) ) );
	}

	aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
	if ( aDlg.Execute() == ERRCODE_NONE )
	{
		INetURLObject	aURL( aDlg.GetPath() );
		INetURLObject	aPathURL( aURL );

		aPathURL.removeSegment();
		aPathURL.removeFinalSlash();

		maBitmapList->SetName( aURL.getName() );
		maBitmapList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

		if( maBitmapList->Save() )
		{
			// Ermitteln (evtl. abschneiden) des Namens und in
			// der GroupBox darstellen
			String aString( CUI_RES( RID_SVXSTR_TABLE ) );
			aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

			if ( aURL.getBase().getLength() > 18 )
			{
				aString += String(aURL.getBase()).Copy( 0, 15 );
				aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
			}
			else
				aString += String(aURL.getBase());

			// Flag fuer gespeichert setzen
			*pnBitmapListState |= CT_SAVED;
			// Flag fuer modifiziert entfernen
			*pnBitmapListState &= ~CT_MODIFIED;
		}
		else
		{
			ErrorBox( DLGWIN, WinBits( WB_OK ),
				String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
		}
	}

	return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ChangePixelColorHdl_Impl, void *, EMPTYARG )
{
	aCtlPixel.SetPixelColor( aLbColor.GetSelectEntryColor() );
	aCtlPixel.Invalidate();

	aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );

	// Bitmap holen und darstellen
	rXFSet.Put(XFillBitmapItem(String(), Graphic(aBitmapCtl.GetBitmapEx())));
	aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
	aCtlPreview.Invalidate();

	bBmpChanged = sal_True;

	return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl, void *, EMPTYARG )
{
	aCtlPixel.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
	aCtlPixel.Invalidate();

	aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );

	// Bitmap holen und darstellen
	rXFSet.Put(XFillBitmapItem(String(), Graphic(aBitmapCtl.GetBitmapEx())));
	aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
	aCtlPreview.Invalidate();

	bBmpChanged = sal_True;

	return 0L;
}

//------------------------------------------------------------------------

void SvxBitmapTabPage::PointChanged( Window* pWindow, RECT_POINT )
{
	if( pWindow == &aCtlPixel )
	{
        aBitmapCtl.SetBmpArray( aCtlPixel.GetBitmapPixelPtr() );

		// Bitmap holen und darstellen
		rXFSet.Put(XFillBitmapItem(String(), Graphic(aBitmapCtl.GetBitmapEx())));
		aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
		aCtlPreview.Invalidate();

		bBmpChanged = sal_True;
	}
}




Window*	SvxBitmapTabPage::GetParentLabeledBy( const Window* pLabeled ) const
{
    if (pLabeled == &aLbBitmaps)
        return const_cast<FixedText*>(&aLbBitmapsHidden);
    else
        return SvxTabPage::GetParentLabeledBy (pLabeled);
}
