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
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#include <svx/dialogs.hrc>
#include <i18npool/mslangid.hxx>
#include <sot/storinfo.hxx>
#include <sot/storage.hxx>
#include <svl/zforlist.hxx>
#include <svtools/ctrltool.hxx>
#include <unotools/lingucfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/akrnitem.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <rtl/logfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyznitem.hxx>
#include <editeng/svxacorr.hxx>
#include <vcl/svapp.hxx>
#include <view.hxx>
#include <prtopt.hxx>
#include <fmtcol.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <docfac.hxx>
#include <docstyle.hxx>
#include <shellio.hxx>
#include <tox.hxx>
#include <swdtflvr.hxx>
#include <dbmgr.hxx>
#include <usrpref.hxx>
#include <fontcfg.hxx>
#include <poolfmt.hxx>
#include <modcfg.hxx>
#include <globdoc.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <unotxdoc.hxx>
#include <linkenum.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <swerror.h>
#include <globals.hrc>

// #i18732#
#include <fmtfollowtextflow.hxx>

#include <unochart.hxx>

// text grid
#include <tgrditem.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;

/*-----------------21.09.96 15.29-------------------

--------------------------------------------------*/


/*--------------------------------------------------------------------
	Beschreibung: Document laden
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::InitNew" );

    sal_Bool bRet = SfxObjectShell::InitNew( xStor );
	ASSERT( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
	sal_Bool bHTMLTemplSet = sal_False;
	if( bRet )
	{
		AddLink();		// pDoc / pIo ggf. anlegen

        sal_Bool bWeb = ISA( SwWebDocShell );
        if ( bWeb )
			bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );//Styles aus HTML.vor
		else if( ISA( SwGlobalDocShell ) )
			GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);		// Globaldokument


/*
		//JP 12.07.95: so einfach waere es fuer die neu Mimik
		pDoc->SetDefault( SvxTabStopItem( 1,
					GetStar Writer App()->GetUsrPref()->GetDefTabDist(),
					SVX_TAB_ADJUST_DEFAULT,
					RES_PARATR_TABSTOP));
*/
		if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
            SwTransferable::InitOle( this, *mpDoc );

        // set forbidden characters if necessary
        SvxAsianConfig aAsian;
        Sequence<Locale> aLocales =  aAsian.GetStartEndCharLocales();
        if(aLocales.getLength())
        {
            const Locale* pLocales = aLocales.getConstArray();
            for(sal_Int32 i = 0; i < aLocales.getLength(); i++)
            {
                ForbiddenCharacters aForbidden;
                aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine);
                LanguageType  eLang = SvxLocaleToLanguage(pLocales[i]);
                mpDoc->setForbiddenCharacters( eLang, aForbidden);
            }
        }
        mpDoc->set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION,
                  !aAsian.IsKerningWesternTextOnly());
        mpDoc->setCharacterCompressionType(static_cast<SwCharCompressType>(aAsian.GetCharDistanceCompression()));
        mpDoc->setPrintData(*SW_MOD()->GetPrtOptions(bWeb));

        SubInitNew();

        // fuer alle

		SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
        SfxPrinter* pPrt = mpDoc->getPrinter( false );

		String sEntry;
        sal_uInt16 aFontWhich[] =
        {   RES_CHRATR_FONT,
            RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_FONT
        };
        sal_uInt16 aFontHeightWhich[] =
        {
            RES_CHRATR_FONTSIZE,
            RES_CHRATR_CJK_FONTSIZE,
            RES_CHRATR_CTL_FONTSIZE
        };
        sal_uInt16 aFontIds[] =
        {
            FONT_STANDARD,
            FONT_STANDARD_CJK,
            FONT_STANDARD_CTL
        };
        sal_uInt16 nFontTypes[] =
        {
            DEFAULTFONT_LATIN_TEXT,
            DEFAULTFONT_CJK_TEXT,
            DEFAULTFONT_CTL_TEXT
        };
        sal_uInt16 aLangTypes[] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        for(sal_uInt8 i = 0; i < 3; i++)
        {
            sal_uInt16 nFontWhich = aFontWhich[i];
            sal_uInt16 nFontId = aFontIds[i];
            SvxFontItem* pFontItem = 0;
            const SvxLanguageItem& rLang = (const SvxLanguageItem&)mpDoc->GetDefault( aLangTypes[i] );
            LanguageType eLanguage = rLang.GetLanguage();
            if(!pStdFont->IsFontDefault(nFontId))
            {
                sEntry = pStdFont->GetFontFor(nFontId);
                
                Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                {
                    aFont = pPrt->GetFontMetric( aFont );
                }

                pFontItem = new SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                            aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich);
            }
            else
            {
                // #107782# OJ use korean language if latin was used
                if ( i == 0 )
				{
					LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
					switch( eUiLanguage )
					{
						case LANGUAGE_KOREAN:
						case LANGUAGE_KOREAN_JOHAB:
							eLanguage = eUiLanguage;
						break;
					}
				}

				Font aLangDefFont = OutputDevice::GetDefaultFont(
                    nFontTypes[i],
                    eLanguage,
                    DEFAULTFONT_FLAGS_ONLYONE );
                pFontItem = new SvxFontItem(aLangDefFont.GetFamily(), aLangDefFont.GetName(),
                                    aEmptyStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich);
            }
            mpDoc->SetDefault(*pFontItem);
			if( !bHTMLTemplSet )
			{
				SwTxtFmtColl *pColl = mpDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
                pColl->ResetFmtAttr(nFontWhich);
			}
            delete pFontItem;
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( FONT_STANDARD, i, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( nFontId, eLanguage );
            mpDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
            if( !bHTMLTemplSet )
            {
                SwTxtFmtColl *pColl = mpDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
                pColl->ResetFmtAttr(aFontHeightWhich[i]);
            }

        }
        sal_uInt16 aFontIdPoolId[] =
        {
            FONT_OUTLINE,       RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST,          RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION,       RES_POOLCOLL_LABEL,
            FONT_INDEX,         RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CJK,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CJK,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CJK,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CJK,     RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CTL,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CTL,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CTL,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CTL,     RES_POOLCOLL_REGISTER_BASE
        };

        sal_uInt16 nFontWhich = RES_CHRATR_FONT;
        sal_uInt16 nFontHeightWhich = RES_CHRATR_FONTSIZE;
        LanguageType eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();
        for(sal_uInt8 nIdx = 0; nIdx < 24; nIdx += 2)
        {
            if(nIdx == 8)
            {
                nFontWhich = RES_CHRATR_CJK_FONT;
                nFontHeightWhich = RES_CHRATR_CJK_FONTSIZE;
                eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
            }
            else if(nIdx == 16)
            {
                nFontWhich = RES_CHRATR_CTL_FONT;
                nFontHeightWhich = RES_CHRATR_CTL_FONTSIZE;
                eLanguage = static_cast<const SvxLanguageItem&>(mpDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
            }
            SwTxtFmtColl *pColl = 0;
            if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
            {
                sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);
                
                Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                    aFont = pPrt->GetFontMetric( aFont );
                
                pColl = mpDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
                if( !bHTMLTemplSet ||
					SFX_ITEM_SET != pColl->GetAttrSet().GetItemState(
													nFontWhich, sal_False ) )
				{
                    pColl->SetFmtAttr(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
										          aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
				}
            }
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( static_cast< sal_Int8 >(aFontIdPoolId[nIdx]), 0, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( aFontIdPoolId[nIdx], eLanguage );
            if(!pColl)
                pColl = mpDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
            SvxFontHeightItem aFontHeight( (const SvxFontHeightItem&)pColl->GetFmtAttr( nFontHeightWhich, sal_True ));
            if(aFontHeight.GetHeight() != sal::static_int_cast<sal_uInt32, sal_Int32>(nFontHeight))
            {
                aFontHeight.SetHeight(nFontHeight);
                pColl->SetFmtAttr( aFontHeight );
            }
        }

        // the default for documents created via 'File/New' should be 'on'
        // (old documents, where this property was not yet implemented, will get the
        // value 'false' in the SwDoc c-tor)
        mpDoc->set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT, 
                SW_MOD()->GetUsrPref( bWeb )->IsAlignMathObjectsToBaseline() );
    }

    /* #106748# If the default frame direction of a document is RTL
        the default adjusment is to the right. */
    if( !bHTMLTemplSet &&
		FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
        mpDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST ) );

    // OD 09.10.2003 #i18732# - set dynamic pool default for
    // item RES_FOLLOW_TEXT_FLOW to sal_False for *new document*.
    // Thus, redo this change in method <SwDoc::RemoveAllFmtLanguageDependencies()>,
    // which is called from <SwDocShell::ConvertFrom(..)> in order to restore
    // the static pool default.
    mpDoc->SetDefault( SwFmtFollowTextFlow( sal_False ) );

// --> collapsing borders FME 2005-05-27 #i29550#
    mpDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, sal_True ) );
// <-- collapsing

    //#i16874# AutoKerning as default for new documents
    mpDoc->SetDefault( SvxAutoKernItem( sal_True, RES_CHRATR_AUTOKERN ) );

    // --> OD 2005-02-10 #i42080# - Due to the several calls of method <SetDefault(..)>
    // at the document instance, the document is modified. Thus, reset this
    // status here. Note: In method <SubInitNew()> this is also done.
    mpDoc->ResetModified();
    // <--

	return bRet;
}

/*--------------------------------------------------------------------
	Beschreibung:	Ctor mit SfxCreateMode ?????
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( SfxObjectCreateMode eMode ) :
    SfxObjectShell ( eMode ),
	mpDoc(0),
	mpFontList(0),
	mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    mbInUpdateFontList(false)
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
    Init_Impl();
}

/*--------------------------------------------------------------------
	Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell ( i_nSfxCreationFlags ),
	mpDoc(0),
	mpFontList(0),
	mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    mbInUpdateFontList(false)
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
    Init_Impl();
}

/*--------------------------------------------------------------------
	Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode ):
    SfxObjectShell ( eMode ),
	mpDoc(pD),
	mpFontList(0),
	mpView( 0 ),
    mpWrtShell( 0 ),
    mpOLEChildList( 0 ),
    mnUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    mbInUpdateFontList(false)
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
    Init_Impl();
}

/*--------------------------------------------------------------------
	Beschreibung:	Dtor
 --------------------------------------------------------------------*/


 SwDocShell::~SwDocShell()
{
    // disable chart related objects now because in ~SwDoc it may be to late for this
    if( mpDoc )
    {
        mpDoc->GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = mpDoc->GetChartDataProvider();
        if (pPCD)
            pPCD->dispose();
    }

    RemoveLink();
	delete mpFontList;

	// wir als BroadCaster werden auch unser eigener Listener
	// (fuer DocInfo/FileNamen/....)
	EndListening( *this );

    delete mpOLEChildList;
}
/* -----------------------------10.09.2001 15:59------------------------------

 ---------------------------------------------------------------------------*/
void  SwDocShell::Init_Impl()
{
    SetPool(&SW_MOD()->GetPool());
	SetBaseModel(new SwXTextDocument(this));
	// wir als BroadCaster werden auch unser eigener Listener
	// (fuer DocInfo/FileNamen/....)
	StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
    SetAutoStyleFilterIndex(3);

	// set map unit to twip
	SetMapUnit( MAP_TWIP );
}
/*--------------------------------------------------------------------
	Beschreibung: AddLink
 --------------------------------------------------------------------*/


void SwDocShell::AddLink()
{
	if( !mpDoc )
	{
		SwDocFac aFactory;
		mpDoc = aFactory.GetDoc();
		mpDoc->acquire();
		mpDoc->set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell) );
	}
	else
		mpDoc->acquire();
	mpDoc->SetDocShell( this );		// am Doc den DocShell-Pointer setzen
	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
	((SwXTextDocument*)xDoc.get())->Reactivate(this);

	SetPool(&mpDoc->GetAttrPool());

	// am besten erst wenn eine sdbcx::View erzeugt wird !!!
	mpDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
}

/*--------------------------------------------------------------------
	Beschreibung:	neue FontList erzeugen Aenderung Drucker
 --------------------------------------------------------------------*/


void SwDocShell::UpdateFontList()
{
    if(!mbInUpdateFontList)
    {
        mbInUpdateFontList = true;
        ASSERT(mpDoc, "Kein Doc keine FontList");
        if( mpDoc )
        {
            delete mpFontList;
            mpFontList = new FontList( mpDoc->getReferenceDevice( true ) );
            PutItem( SvxFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        mbInUpdateFontList = false;
    }
}

/*--------------------------------------------------------------------
	Beschreibung: RemoveLink
 --------------------------------------------------------------------*/


void SwDocShell::RemoveLink()
{
	// Uno-Object abklemmen
	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
	((SwXTextDocument*)xDoc.get())->Invalidate();
	aFinishedTimer.Stop();
	if(mpDoc)
	{
		if( mxBasePool.is() )
		{
			static_cast<SwDocStyleSheetPool*>(mxBasePool.get())->dispose();
			mxBasePool.clear();
		}
        sal_Int8 nRefCt = static_cast< sal_Int8 >(mpDoc->release());
		mpDoc->SetOle2Link(Link());
		mpDoc->SetDocShell( 0 );
		if( !nRefCt )
			delete mpDoc;
		mpDoc = 0;       // wir haben das Doc nicht mehr !!
	}
}
void SwDocShell::InvalidateModel()
{
	// Uno-Object abklemmen
	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
	((SwXTextDocument*)xDoc.get())->Invalidate();
}
void SwDocShell::ReactivateModel()
{
	// Uno-Object abklemmen
	uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
	((SwXTextDocument*)xDoc.get())->Reactivate(this);
}

/*--------------------------------------------------------------------
	Beschreibung: Laden, Default-Format
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::Load( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Load" );
	sal_Bool bRet = sal_False;
    if( SfxObjectShell::Load( rMedium ))
	{
		RTL_LOGFILE_CONTEXT_TRACE( aLog, "after SfxInPlaceObject::Load" );
		if( mpDoc )              // fuer Letzte Version !!
			RemoveLink();       // das existierende Loslassen

		AddLink();      // Link setzen und Daten updaten !!

		// Das Laden
		// fuer MD
			ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
			mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
            if(GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
            {
                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
                mnUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;
            }

		SwWait aWait( *this, true );
		sal_uInt32 nErr = ERR_SWG_READ_ERROR;
		switch( GetCreateMode() )
		{
//		case SFX_CREATE_MODE_INTERNAL:
//			nErr = 0;
//			break;

		case SFX_CREATE_MODE_ORGANIZER:
			{
				if( ReadXML )
				{
					ReadXML->SetOrganizerMode( sal_True );
                    SwReader aRdr( rMedium, aEmptyStr, mpDoc );
					nErr = aRdr.Read( *ReadXML );
					ReadXML->SetOrganizerMode( sal_False );
				}
			}
			break;

		case SFX_CREATE_MODE_INTERNAL:
		case SFX_CREATE_MODE_EMBEDDED:
			{
				// fuer MWERKS (Mac-Compiler): kann nicht selbststaendig casten
                SwTransferable::InitOle( this, *mpDoc );
			}
			// SfxProgress unterdruecken, wenn man Embedded ist
			SW_MOD()->SetEmbeddedLoadSave( sal_True );
			// kein break;

		case SFX_CREATE_MODE_STANDARD:
		case SFX_CREATE_MODE_PREVIEW:
			{
                Reader *pReader = ReadXML;
				if( pReader )
				{
					// die DocInfo vom Doc am DocShell-Medium setzen
					RTL_LOGFILE_CONTEXT_TRACE( aLog, "before ReadDocInfo" );
                    SwReader aRdr( rMedium, aEmptyStr, mpDoc );
					RTL_LOGFILE_CONTEXT_TRACE( aLog, "before Read" );
					nErr = aRdr.Read( *pReader );
					RTL_LOGFILE_CONTEXT_TRACE( aLog, "after Read" );

					// If a XML document is loaded, the global doc/web doc
					// flags have to be set, because they aren't loaded
					// by this formats.
					if( ISA( SwWebDocShell ) )
					{
						if( !mpDoc->get(IDocumentSettingAccess::HTML_MODE) )
							mpDoc->set(IDocumentSettingAccess::HTML_MODE, true);
					}
					if( ISA( SwGlobalDocShell ) )
					{
						if( !mpDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
							mpDoc->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
					}
				}
#ifdef DBG_UTIL
				else
					ASSERT( sal_False, "nothing works without Sw3Reader" );
#endif
			}
			break;

		default:
			ASSERT( sal_False, "Load: new CreateMode?" );

		}

		UpdateFontList();
        InitDrawModelAndDocShell(this, mpDoc ? mpDoc->GetDrawModel() : 0);

		SetError( nErr, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
		bRet = !IsError( nErr );

        // --> OD 2006-11-07 #i59688#
//        // StartFinishedLoading rufen.
//        if( bRet && !pDoc->IsInLoadAsynchron() &&
//            GetCreateMode() == SFX_CREATE_MODE_STANDARD )
//            StartLoadFinishedTimer();
        if ( bRet && !mpDoc->IsInLoadAsynchron() &&
             GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        {
            LoadingFinished();
        }
        // <--

		// SfxProgress unterdruecken, wenn man Embedded ist
		SW_MOD()->SetEmbeddedLoadSave( sal_False );
	}

	return bRet;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::LoadFrom( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::LoadFrom" );
	sal_Bool bRet = sal_False;
	if( mpDoc )
		RemoveLink();

	AddLink();		// Link setzen und Daten updaten !!

	do {		// middle check loop
		sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        //const String& rNm = pStor->GetName();
		String aStreamName;
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("styles.xml"));
        uno::Reference < container::XNameAccess > xAccess( rMedium.GetStorage(), uno::UNO_QUERY );
        if ( xAccess->hasByName( aStreamName ) && rMedium.GetStorage()->isStreamElement( aStreamName ) )
		{
			// Das Laden
			SwWait aWait( *this, true );
			{
				ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
				mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
				if( ReadXML )
				{
					ReadXML->SetOrganizerMode( sal_True );
                    SwReader aRdr( rMedium, aEmptyStr, mpDoc );
					nErr = aRdr.Read( *ReadXML );
					ReadXML->SetOrganizerMode( sal_False );
				}
			}
		}
        else
        {
            DBG_ERROR("Code removed!");
        /*
        //TODO/LATER: looks like some binary stuff?!
			// sollte es sich um eine 2. Vrolage handeln ??
			if( SvStorage::IsStorageFile( rNm ) )
				break;

			const SfxFilter* pFltr = SwIoSystem::GetFileFilter( rNm, aEmptyStr );
			if( !pFltr || !pFltr->GetUserData().EqualsAscii( FILTER_SWG ))
				break;

			SfxMedium aMed( rNm, STREAM_STD_READ, sal_False );
			if( 0 == ( nErr = aMed.GetInStream()->GetError() ) )
			{
				SwWait aWait( *this, sal_True );
				SwReader aRead( aMed, rNm, pDoc );
				nErr = aRead.Read( *ReadSwg );
			}
         */
        }

		SetError( nErr, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
		bRet = !IsError( nErr );

	} while( sal_False );

    SfxObjectShell::LoadFrom( rMedium );
	mpDoc->ResetModified();
	return bRet;
}


void SwDocShell::SubInitNew()
{
	ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
	mxBasePool = new SwDocStyleSheetPool( *mpDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
	UpdateFontList();
    InitDrawModelAndDocShell(this, mpDoc ? mpDoc->GetDrawModel() : 0);

    mpDoc->setLinkUpdateMode( GLOBALSETTING );
    mpDoc->setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );

	sal_Bool bWeb = ISA(SwWebDocShell);

	sal_uInt16 nRange[] =	{
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_CHRATR_COLOR, RES_CHRATR_COLOR,
        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
        0, 0, 0  };
	if(!bWeb)
	{
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 3 ] = RES_PARATR_TABSTOP;
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 2 ] = RES_PARATR_HYPHENZONE;
	}
	SfxItemSet aDfltSet( mpDoc->GetAttrPool(), nRange );

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   nVal = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
	aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );

    if(!bWeb)
	{
		SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) mpDoc->GetDefault(
														RES_PARATR_HYPHENZONE) );
        aHyp.GetMinLead()   = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinLeading);
        aHyp.GetMinTrail()  = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinTrailing);

		aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = static_cast< sal_uInt16 >(SW_MOD()->GetUsrPref(sal_False)->GetDefTab());
		if( nNewPos )
			aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                          SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ) );
	}
    aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );

	mpDoc->SetDefault( aDfltSet );

	//default page mode for text grid
	if(!bWeb)
	{
		sal_Bool bSquaredPageMode = SW_MOD()->GetUsrPref(sal_False)->IsSquaredPageMode();
		mpDoc->SetDefaultPageMode( bSquaredPageMode );
	}

	mpDoc->ResetModified();
}

/*
 * Document Interface Access
 */
IDocumentDeviceAccess* SwDocShell::getIDocumentDeviceAccess() { return mpDoc; }
const IDocumentSettingAccess* SwDocShell::getIDocumentSettingAccess() const { return mpDoc; }
IDocumentChartDataProviderAccess* SwDocShell::getIDocumentChartDataProviderAccess() { return mpDoc; }
