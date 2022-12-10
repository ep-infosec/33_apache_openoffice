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
#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#define _SVX_TPLNEEND_CXX

#include <cuires.hrc>
#include "tabline.hrc"
#include "helpid.hrc"
#include <svx/dialmgr.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include "svx/drawitem.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "cuitabline.hxx"
#include <svx/svxdlg.hxx> //CHINA001
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/dialogs.hrc>

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH   32
#define BITMAP_HEIGHT  12
#define XOUT_WIDTH    150

/*************************************************************************
|*
|*	Dialog zum Definieren von Linienenden-Stilen
|*
\************************************************************************/

SvxLineEndDefTabPage::SvxLineEndDefTabPage
(
	Window* pParent,
	const SfxItemSet& rInAttrs
) :

	SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_LINEEND_DEF ), rInAttrs ),

    aFlTip              ( this, CUI_RES( FL_TIP ) ),
    aFTTitle            ( this, CUI_RES( FT_TITLE ) ),
    aEdtName            ( this, CUI_RES( EDT_NAME ) ),
    aFTLineEndStyle     ( this, CUI_RES( FT_LINE_END_STYLE ) ),
    aLbLineEnds         ( this, CUI_RES( LB_LINEENDS ) ),
    aBtnAdd             ( this, CUI_RES( BTN_ADD ) ),
    aBtnModify          ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnDelete          ( this, CUI_RES( BTN_DELETE ) ),
    aBtnLoad            ( this, CUI_RES( BTN_LOAD ) ),
	aBtnSave 		    ( this, CUI_RES( BTN_SAVE ) ),
    aFiTip              ( this, CUI_RES( FI_TIP ) ),
    aCtlPreview         ( this, CUI_RES( CTL_PREVIEW ) ),

    rOutAttrs           ( rInAttrs ),
    pPolyObj( NULL ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_SOLID ),
    aXWidth             ( XOUT_WIDTH ),
    aXColor             ( String(), COL_BLACK ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
    maLineEndList()
{
	aBtnLoad.SetModeImage( Image( CUI_RES( RID_SVXIMG_LOAD_H ) ), BMP_COLOR_HIGHCONTRAST );
	aBtnSave.SetModeImage( Image( CUI_RES( RID_SVXIMG_SAVE_H ) ), BMP_COLOR_HIGHCONTRAST );

	FreeResource();

	// diese Page braucht ExchangeSupport
	SetExchangeSupport();

	rXLSet.Put( aXLStyle );
	rXLSet.Put( aXWidth );
	rXLSet.Put( aXColor );
	rXLSet.Put( XLineStartWidthItem( aCtlPreview.GetOutputSize().Height()  / 2 ) );
	rXLSet.Put( XLineEndWidthItem( aCtlPreview.GetOutputSize().Height() / 2 ) );

	// #i34740#
	aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

	aBtnAdd.SetClickHdl(
		LINK( this, SvxLineEndDefTabPage, ClickAddHdl_Impl ) );
	aBtnModify.SetClickHdl(
		LINK( this, SvxLineEndDefTabPage, ClickModifyHdl_Impl ) );
	aBtnDelete.SetClickHdl(
		LINK( this, SvxLineEndDefTabPage, ClickDeleteHdl_Impl ) );
	aBtnLoad.SetClickHdl(
		LINK( this, SvxLineEndDefTabPage, ClickLoadHdl_Impl ) );
	aBtnSave.SetClickHdl(
		LINK( this, SvxLineEndDefTabPage, ClickSaveHdl_Impl ) );

	aLbLineEnds.SetSelectHdl(
		LINK( this, SvxLineEndDefTabPage, SelectLineEndHdl_Impl ) );

	aBtnAdd.SetAccessibleRelationMemberOf(&aFlTip);
	aBtnModify.SetAccessibleRelationMemberOf(&aFlTip);
	aBtnDelete.SetAccessibleRelationMemberOf(&aFlTip);
}

//------------------------------------------------------------------------

SvxLineEndDefTabPage::~SvxLineEndDefTabPage()
{
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Construct()
{
	aLbLineEnds.Fill( maLineEndList );

	bool bCreateArrowPossible = true;

	if( !pPolyObj )
	{
		bCreateArrowPossible = false;
	}
	else if( !pPolyObj->ISA( SdrPathObj ) )
	{
		SdrObjTransformInfoRec aInfoRec;
		pPolyObj->TakeObjInfo( aInfoRec );
		SdrObject* pNewObj = 0;
		if( aInfoRec.bCanConvToPath )
			pNewObj = pPolyObj->ConvertToPolyObj( sal_True, sal_False );

		bCreateArrowPossible = pNewObj && pNewObj->ISA( SdrPathObj );
        SdrObject::Free( pNewObj );
	}

	if( !bCreateArrowPossible )
		aBtnAdd.Disable();
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::ActivatePage( const SfxItemSet& )
{
	if( *pDlgType == 0 ) // Flaechen-Dialog
	{
		// ActivatePage() wird aufgerufen bevor der Dialog PageCreated() erhaelt !!!
		if( maLineEndList.get() )
		{
            if( *pPosLineEndLb != LISTBOX_ENTRY_NOTFOUND )
			{
				aLbLineEnds.SelectEntryPos( *pPosLineEndLb );
				SelectLineEndHdl_Impl( this );
			}
			INetURLObject	aURL( maLineEndList->GetPath() );

			aURL.Append( maLineEndList->GetName() );
			DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
/*			// Ermitteln (evtl. abschneiden) des Namens und in
			// der GroupBox darstellen
			String			aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

			if ( aURL.getBase().Len() > 18 )
			{
				aString += aURL.getBase().Copy( 0, 15 );
				aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
			}
			else
				aString += aURL.getBase();

			aGrpLineEnds.SetText( aString );
*/
			*pPageType = 0; // 3
			*pPosLineEndLb = LISTBOX_ENTRY_NOTFOUND;
		}
	}
}

// -----------------------------------------------------------------------

int SvxLineEndDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
	CheckChanges_Impl();

    if( _pSet )
        FillItemSet( *_pSet );

	return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::CheckChanges_Impl()
{
	sal_uInt16 nPos = aLbLineEnds.GetSelectEntryPos();

	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		String aString = aEdtName.GetText();

		if( aString != aLbLineEnds.GetSelectEntry() )
		{
			QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
				CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_LINEEND ) );

			if ( aQueryBox.Execute() == RET_YES )
				ClickModifyHdl_Impl( this );
		}
	}
	nPos = aLbLineEnds.GetSelectEntryPos();

	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		*pPosLineEndLb = nPos;
}

// -----------------------------------------------------------------------

sal_Bool SvxLineEndDefTabPage::FillItemSet( SfxItemSet& rSet )
{
	if( *pDlgType == 0 ) // Linien-Dialog
	{
		if( *pPageType == 3 )
		{
			CheckChanges_Impl();

			long nPos = aLbLineEnds.GetSelectEntryPos();
            XLineEndEntry* pEntry = maLineEndList->GetLineEnd( nPos );

            rSet.Put( XLineStartItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
            rSet.Put( XLineEndItem( pEntry->GetName(), pEntry->GetLineEnd() ) );
		}
	}
	return( sal_True );
}

// -----------------------------------------------------------------------

void SvxLineEndDefTabPage::Reset( const SfxItemSet& )
{
	aLbLineEnds.SelectEntryPos( 0 );

	// Update lineend
	if( maLineEndList.get() && maLineEndList->Count() > 0 )
	{
		int nPos = aLbLineEnds.GetSelectEntryPos();

        XLineEndEntry* pEntry = maLineEndList->GetLineEnd( nPos );

		aEdtName.SetText( aLbLineEnds.GetSelectEntry() );

		rXLSet.Put( XLineStartItem( String(), pEntry->GetLineEnd() ) );
		rXLSet.Put( XLineEndItem( String(), pEntry->GetLineEnd() ) );

		// #i34740#
		aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

		aCtlPreview.Invalidate();
	}

	// Status der Buttons ermitteln
	if( maLineEndList.get() && maLineEndList->Count() )
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
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineEndDefTabPage::Create( Window* pWindow,
                const SfxItemSet& rSet )
{
    return( new SvxLineEndDefTabPage( pWindow, rSet ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, SelectLineEndHdl_Impl, void *, EMPTYARG )
{
	if( maLineEndList.get() && maLineEndList->Count() > 0 )
	{
		int nPos = aLbLineEnds.GetSelectEntryPos();

        XLineEndEntry* pEntry = maLineEndList->GetLineEnd( nPos );

		aEdtName.SetText( aLbLineEnds.GetSelectEntry() );

		rXLSet.Put( XLineStartItem( String(), pEntry->GetLineEnd() ) );
		rXLSet.Put( XLineEndItem( String(), pEntry->GetLineEnd() ) );

		// #i34740#
		aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

		aCtlPreview.Invalidate();

		// Wird erst hier gesetzt, um den Style nur dann zu uebernehmen,
		// wenn in der ListBox ein Eintrag ausgewaehlt wurde
		*pPageType = 3;
	}
	return( 0L );
}

//------------------------------------------------------------------------

long SvxLineEndDefTabPage::ChangePreviewHdl_Impl( void* )
{
	aCtlPreview.Invalidate();
	return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nPos = aLbLineEnds.GetSelectEntryPos();

	if( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		ResMgr& rMgr = CUI_MGR();
		String aDesc( ResId( RID_SVXSTR_DESC_LINEEND, rMgr ) );
		String aName( aEdtName.GetText() );
		long nCount = maLineEndList.get() ? maLineEndList->Count() : 0;
		sal_Bool bDifferent = sal_True;

		// Pruefen, ob Name schon vorhanden ist
		for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == maLineEndList->GetLineEnd( i )->GetName() )
				bDifferent = sal_False;

		// Wenn ja, wird wiederholt ein neuer Name angefordert
		if ( !bDifferent )
		{
			WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
				String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
			aWarningBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
			aWarningBox.Execute();

			SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
			DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
			AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
			DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
			sal_Bool bLoop = sal_True;

			while( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
			{
				pDlg->GetName( aName );
				bDifferent = sal_True;

				for( long i = 0; i < nCount && bDifferent; i++ )
				{
                    if( aName == maLineEndList->GetLineEnd( i )->GetName() )
						bDifferent = sal_False;
				}

				if( bDifferent )
					bLoop = sal_False;
				else
					aWarningBox.Execute();
			}
			delete( pDlg );
		}

        // Wenn nicht vorhanden, wird Eintrag aufgenommen
        if( bDifferent )
        {
            const XLineEndEntry* pOldEntry = maLineEndList->GetLineEnd( nPos );

            if(pOldEntry)
            {
                // #123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
                XLineEndEntry* pEntry = new XLineEndEntry(pOldEntry->GetLineEnd(), aName);
                delete maLineEndList->Replace(pEntry, nPos);

                aEdtName.SetText( aName );

                aLbLineEnds.Modify( *pEntry, nPos, maLineEndList->GetUiBitmap( nPos ) );
                aLbLineEnds.SelectEntryPos( nPos );

                // Flag fuer modifiziert setzen
                *pnLineEndListState |= CT_MODIFIED;

                *pPageType = 3;
            }
            else
            {
                OSL_ENSURE(false, "LineEnd to be modified not existing (!)");
            }
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
	if( pPolyObj )
	{
		const SdrObject* pNewObj;
		SdrObject* pConvPolyObj = NULL;

		if( pPolyObj->ISA( SdrPathObj ) )
		{
			pNewObj = pPolyObj;
		}
		else
		{
			SdrObjTransformInfoRec aInfoRec;
			pPolyObj->TakeObjInfo( aInfoRec );

			if( aInfoRec.bCanConvToPath )
			{
				pNewObj = pConvPolyObj = pPolyObj->ConvertToPolyObj( sal_True, sal_False );

				if( !pNewObj || !pNewObj->ISA( SdrPathObj ) )
					return( 0L ); // Abbruch, zusaetzliche Sicherheit, die bei
							// Gruppenobjekten aber nichts bringt.
			}
			else return( 0L ); // Abbruch
		}

		basegfx::B2DPolyPolygon aNewPolyPolygon(((SdrPathObj*)pNewObj)->GetPathPoly());
		basegfx::B2DRange aNewRange(basegfx::tools::getRange(aNewPolyPolygon));

		// Normalisieren
        aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(
            -aNewRange.getMinX(), -aNewRange.getMinY()));

		// Loeschen des angelegten PolyObjektes
        SdrObject::Free( pConvPolyObj );

		XLineEndEntry* pEntry;

		ResMgr& rMgr = CUI_MGR();
		String aNewName( SVX_RES( RID_SVXSTR_LINEEND ) );
		String aDesc( ResId( RID_SVXSTR_DESC_LINEEND, rMgr ) );
		String aName;

		long nCount = maLineEndList.get() ? maLineEndList->Count() : 0;
		long j = 1;
		sal_Bool bDifferent = sal_False;

		while ( !bDifferent )
		{
			aName = aNewName;
			aName += sal_Unicode(' ');
			aName += UniString::CreateFromInt32( j++ );
			bDifferent = sal_True;

			for( long i = 0; i < nCount && bDifferent; i++ )
                if ( aName == maLineEndList->GetLineEnd( i )->GetName() )
					bDifferent = sal_False;
		}

		SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
		DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
		AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
		DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
		sal_Bool bLoop = sal_True;

		while ( bLoop && pDlg->Execute() == RET_OK )
		{
			pDlg->GetName( aName );
			bDifferent = sal_True;

			for( long i = 0; i < nCount && bDifferent; i++ )
			{
                if( aName == maLineEndList->GetLineEnd( i )->GetName() )
					bDifferent = sal_False;
			}

			if( bDifferent )
			{
				bLoop = sal_False;
				pEntry = new XLineEndEntry( aNewPolyPolygon, aName );

                long nLineEndCount = maLineEndList.get() ? maLineEndList->Count() : 0;
                maLineEndList->Insert( pEntry, nLineEndCount );

				// Zur ListBox hinzufuegen
				aLbLineEnds.Append( *pEntry, maLineEndList->GetUiBitmap( nLineEndCount ) );
				aLbLineEnds.SelectEntryPos( aLbLineEnds.GetEntryCount() - 1 );

				// Flag fuer modifiziert setzen
				*pnLineEndListState |= CT_MODIFIED;

				SelectLineEndHdl_Impl( this );
			}
			else
			{
				WarningBox aBox( DLGWIN, WinBits( WB_OK ),String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
				aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
				aBox.Execute();
			}
		}
		delete pDlg;
	}
	else
		aBtnAdd.Disable();

	// Status der Buttons ermitteln
	if ( maLineEndList.get() && maLineEndList->Count() )
	{
		aBtnModify.Enable();
		aBtnDelete.Enable();
		aBtnSave.Enable();
	}
	return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nPos = aLbLineEnds.GetSelectEntryPos();

	if( nPos != LISTBOX_ENTRY_NOTFOUND )
	{
		QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
			String( CUI_RES( RID_SVXSTR_ASK_DEL_LINEEND ) ) );

		if ( aQueryBox.Execute() == RET_YES )
		{
			delete maLineEndList->Remove( nPos );
			aLbLineEnds.RemoveEntry( nPos );
			aLbLineEnds.SelectEntryPos( 0 );

			SelectLineEndHdl_Impl( this );
			*pPageType = 0; // LineEnd soll nicht uebernommen werden

			// Flag fuer modifiziert setzen
			*pnLineEndListState |= CT_MODIFIED;

			ChangePreviewHdl_Impl( this );
		}
	}
	// Status der Buttons ermitteln
	if( !maLineEndList.get() || !maLineEndList->Count() )
	{
		aBtnModify.Disable();
		aBtnDelete.Disable();
		aBtnSave.Disable();
	}
	return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickLoadHdl_Impl, void *, EMPTYARG )
{
	ResMgr& rMgr = CUI_MGR();
	sal_uInt16 nReturn = RET_YES;

	if ( *pnLineEndListState & CT_MODIFIED )
	{
		nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
			String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

		if ( nReturn == RET_YES )
			maLineEndList->Save();
	}

	if ( nReturn != RET_CANCEL )
	{
    	::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
		String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soe" ) );
		aDlg.AddFilter( aStrFilterType, aStrFilterType );
		INetURLObject aFile( SvtPathOptions().GetPalettePath() );
		aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

		if( aDlg.Execute() == ERRCODE_NONE )
		{
			INetURLObject aURL( aDlg.GetPath() );
			INetURLObject aPathURL( aURL );

			aPathURL.removeSegment();
			aPathURL.removeFinalSlash();

			// Liste speichern
			XLineEndListSharedPtr aLeList(XPropertyListFactory::CreateSharedXLineEndList(aPathURL.GetMainURL(INetURLObject::NO_DECODE)));
			aLeList->SetName( aURL.getName() );
			if( aLeList->Load() )
			{
				if( aLeList.get() )
				{
					maLineEndList = aLeList;
					( (SvxLineTabDialog*) DLGWIN )->SetNewLineEndList( maLineEndList );
					aLbLineEnds.Clear();
					aLbLineEnds.Fill( maLineEndList );
					Reset( rOutAttrs );

					maLineEndList->SetName( aURL.getName() );

/*					// Ermitteln (evtl. abschneiden) des Namens und in
					// der GroupBox darstellen
					String aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
					aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
					if( aURL.getBase().Len() > 18 )
					{
						aString += aURL.getBase().Copy( 0, 15 );
						aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
					}
					else
						aString += aURL.getBase();

					aGrpLineEnds.SetText( aString );
*/
					// Flag fuer gewechselt setzen
					*pnLineEndListState |= CT_CHANGED;
					// Flag fuer modifiziert entfernen
					*pnLineEndListState &= ~CT_MODIFIED;
				}
			}
			else
				ErrorBox( DLGWIN, WinBits( WB_OK ),
					String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
		}
	}

	// Status der Buttons ermitteln
	if ( maLineEndList.get() && maLineEndList->Count() )
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
	return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineEndDefTabPage, ClickSaveHdl_Impl, void *, EMPTYARG )
{
   	::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
	String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soe" ) );
	aDlg.AddFilter( aStrFilterType, aStrFilterType );

	INetURLObject aFile( SvtPathOptions().GetPalettePath() );
	DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

	if( maLineEndList.get() && maLineEndList->GetName().Len() )
	{
		aFile.Append( maLineEndList->GetName() );

		if( !aFile.getExtension().getLength() )
			aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "soe" ) ) );
	}

	aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
	if ( aDlg.Execute() == ERRCODE_NONE )
	{
		INetURLObject	aURL( aDlg.GetPath() );
		INetURLObject	aPathURL( aURL );

		aPathURL.removeSegment();
		aPathURL.removeFinalSlash();

		maLineEndList->SetName( aURL.getName() );
		maLineEndList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

		if( maLineEndList->Save() )
		{
/*			// Ermitteln (evtl. abschneiden) des Namens und in
			// der GroupBox darstellen
			String aString( CUI_RES( RID_SVXSTR_TABLE ) );
			aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

			if ( aURL.getBase().Len() > 18 )
			{
				aString += aURL.getBase().Copy( 0, 15 );
				aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
			}
			else
				aString += aURL.getBase();
			aGrpLineEnds.SetText( aString );
*/
			// Flag fuer gespeichert setzen
			*pnLineEndListState |= CT_SAVED;
			// Flag fuer modifiziert entfernen
			*pnLineEndListState &= ~CT_MODIFIED;
		}
		else
		{
			ErrorBox( DLGWIN, WinBits( WB_OK ),
				String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
		}
	}
	return( 0L );
}

void SvxLineEndDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
	SfxTabPage::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
	{
		sal_uInt16 nOldSelect = aLbLineEnds.GetSelectEntryPos();
		aLbLineEnds.Clear();
		aLbLineEnds.Fill( maLineEndList );
		aLbLineEnds.SelectEntryPos( nOldSelect );
	}
}

