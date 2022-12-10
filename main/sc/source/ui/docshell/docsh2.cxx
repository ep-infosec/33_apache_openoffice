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


#include <svx/svdpage.hxx>


#include <svx/xtable.hxx>

#include "scitems.hxx"
#include <tools/gen.hxx>
#include <svtools/ctrltool.hxx>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <sfx2/printer.hxx>
#include <svl/smplhint.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/unolingu.hxx>
#include <rtl/logfile.hxx>

#include <comphelper/processfactory.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <sfx2/app.hxx>

// INCLUDE ---------------------------------------------------------------
/*
#include <svdrwetc.hxx>
#include <svdrwobx.hxx>
#include <sostor.hxx>
*/
#include "drwlayer.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "docshimp.hxx"
#include "docfunc.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------

sal_Bool __EXPORT ScDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::InitNew" );

    sal_Bool bRet = SfxObjectShell::InitNew( xStor );

	aDocument.MakeTable(0);
	//	zusaetzliche Tabellen werden von der ersten View angelegt,
	//	wenn bIsEmpty dann noch sal_True ist

	if( bRet )
	{
		Size aSize( (long) ( STD_COL_WIDTH			 * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
					(long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
		// hier muss auch der Start angepasst werden
		SetVisAreaOrSize( Rectangle( Point(), aSize ), sal_True );
	}

	// InitOptions sets the document languages, must be called before CreateStandardStyles
    InitOptions(false);

	aDocument.GetStyleSheetPool()->CreateStandardStyles();
	aDocument.UpdStlShtPtrsFrmNms();

	//	SetDocumentModified ist in Load/InitNew nicht mehr erlaubt!

	InitItems();
	CalcOutputFactor();
#if 0
	uno::Any aGlobs;
        uno::Sequence< uno::Any > aArgs(1);
        aArgs[ 0 ] <<= GetModel();
	aGlobs <<= ::comphelper::getProcessServiceFactory()->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.excel.Globals" ) ), aArgs );
	GetBasicManager()->SetGlobalUNOConstant( "VBAGlobals", aGlobs );
        // Fake ThisComponent being setup by Activate ( which is a view 
        // related thing ), 
        //  a) if another document is opened then in theory  ThisComponent
        //     will be reset as before, 
        //  b) when this document is  'really' Activated then ThisComponent
        //     again will be set as before
        // The only wrinkle seems if this document is loaded 'InVisible'
        // but.. I don't see that this is possible from the vba API 
        // I could be wrong though
        // There may be implications setting the current component
        // too early :-/ so I will just manually set the Basic Variables
        BasicManager* pAppMgr = SFX_APP()->GetBasicManager();
        if ( pAppMgr )
            pAppMgr->SetGlobalUNOConstant( "ThisExcelDoc", aArgs[ 0 ] );
#endif

	return bRet;
}

//------------------------------------------------------------------

sal_Bool ScDocShell::IsEmpty() const
{
	return bIsEmpty;
}


void ScDocShell::SetEmpty(sal_Bool bSet)
{
	bIsEmpty = bSet;
}

//------------------------------------------------------------------

void ScDocShell::InitItems()
{
	// AllItemSet fuer Controller mit benoetigten Items fuellen:

    // if ( pImpl->pFontList )
    //  delete pImpl->pFontList;

	//	Druck-Optionen werden beim Drucken und evtl. in GetPrinter gesetzt

    // pImpl->pFontList = new FontList( GetPrinter(), Application::GetDefaultDevice() );
    //PutItem( SvxFontListItem( pImpl->pFontList, SID_ATTR_CHAR_FONTLIST ) );
	UpdateFontList();

	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
	if (pDrawLayer)
	{
        PutItem( SvxColorTableItem  ( pDrawLayer->GetColorTableFromSdrModel(), SID_COLOR_TABLE ) );
        PutItem( SvxGradientListItem( pDrawLayer->GetGradientListFromSdrModel(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem   ( pDrawLayer->GetHatchListFromSdrModel(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem  ( pDrawLayer->GetBitmapListFromSdrModel(), SID_BITMAP_LIST ) );
        PutItem( SvxDashListItem    ( pDrawLayer->GetDashListFromSdrModel(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem ( pDrawLayer->GetLineEndListFromSdrModel(), SID_LINEEND_LIST ) );

			//	andere Anpassungen nach dem Anlegen des DrawLayers

		pDrawLayer->SetNotifyUndoActionHdl( LINK( pDocFunc, ScDocFunc, NotifyDrawUndo ) );

		//if (SfxObjectShell::HasSbxObject())
		pDrawLayer->UpdateBasic();			// DocShell-Basic in DrawPages setzen
	}
	else
	{
		//	always use global color table instead of local copy
        PutItem( SvxColorTableItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ) );
	}

	if ( !aDocument.GetForbiddenCharacters().isValid() ||
			!aDocument.IsValidAsianCompression() || !aDocument.IsValidAsianKerning() )
	{
		//	get settings from SvxAsianConfig
		SvxAsianConfig aAsian( sal_False );

		if ( !aDocument.GetForbiddenCharacters().isValid() )
		{
			// set forbidden characters if necessary
			uno::Sequence<lang::Locale> aLocales = aAsian.GetStartEndCharLocales();
			if (aLocales.getLength())
			{
				vos::ORef<SvxForbiddenCharactersTable> xForbiddenTable =
						new SvxForbiddenCharactersTable( aDocument.GetServiceManager() );

				const lang::Locale* pLocales = aLocales.getConstArray();
				for (sal_Int32 i = 0; i < aLocales.getLength(); i++)
				{
					i18n::ForbiddenCharacters aForbidden;
					aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine );
					LanguageType eLang = SvxLocaleToLanguage(pLocales[i]);
					//pDoc->SetForbiddenCharacters( eLang, aForbidden );

					xForbiddenTable->SetForbiddenCharacters( eLang, aForbidden );
				}

				aDocument.SetForbiddenCharacters( xForbiddenTable );
			}
		}

		if ( !aDocument.IsValidAsianCompression() )
		{
			// set compression mode from configuration if not already set (e.g. XML import)
            aDocument.SetAsianCompression( sal::static_int_cast<sal_uInt8>( aAsian.GetCharDistanceCompression() ) );
		}

		if ( !aDocument.IsValidAsianKerning() )
		{
			// set asian punctuation kerning from configuration if not already set (e.g. XML import)
			aDocument.SetAsianKerning( !aAsian.IsKerningWesternTextOnly() );	// reversed
		}
	}
}

//------------------------------------------------------------------

void ScDocShell::ResetDrawObjectShell()
{
	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
	if (pDrawLayer)
		pDrawLayer->SetObjectShell( NULL );
}

//------------------------------------------------------------------

void __EXPORT ScDocShell::Activate()
{
}


void __EXPORT ScDocShell::Deactivate()
{
}

//------------------------------------------------------------------


ScDrawLayer* ScDocShell::MakeDrawLayer()
{
	ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
	if (!pDrawLayer)
	{
		RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::MakeDrawLayer" );

		aDocument.InitDrawLayer(this);
		pDrawLayer = aDocument.GetDrawLayer();
		InitItems();											// incl. Undo und Basic
		Broadcast( SfxSimpleHint( SC_HINT_DRWLAYER_NEW ) );
		if (nDocumentLock)
			pDrawLayer->setLock(sal_True);
	}
	return pDrawLayer;
}
