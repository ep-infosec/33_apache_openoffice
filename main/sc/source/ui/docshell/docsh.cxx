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
// System - Includes -----------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/algitem.hxx>

#include <sot/clsids.hxx>
#include <unotools/securityoptions.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/waitobj.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/zforlist.hxx>
#include <svl/PasswordHelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/objface.hxx>
#include <svl/srchitem.hxx>
#include <unotools/fltrcfg.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/sharecontrolfile.hxx>
#include <unotools/charclass.hxx>
#include <vcl/virdev.hxx>
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include <sfx2/request.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>

#include "scabstdlg.hxx" //CHINA001
#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_30 SOT_FORMATSTR_ID_STARCALC

// INCLUDE ---------------------------------------------------------------

#include "cell.hxx"
#include "global.hxx"
#include "filter.hxx"
#include "scmod.hxx"
#include "tabvwsh.hxx"
#include "docfunc.hxx"
#include "imoptdlg.hxx"
#include "impex.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "brdcst.hxx"
#include "stlpool.hxx"
#include "autostyl.hxx"
#include "attrib.hxx"
#include "asciiopt.hxx"
#include "waitoff.hxx"
#include "docpool.hxx"		// LoadCompleted
#include "progress.hxx"
#include "pntlock.hxx"
#include "collect.hxx"
#include "docuno.hxx"
#include "appoptio.hxx"
#include "detdata.hxx"
#include "printfun.hxx"
#include "dociter.hxx"
#include "cellform.hxx"
#include "chartlis.hxx"
#include "hints.hxx"
#include "xmlwrap.hxx"
#include "drwlayer.hxx"
#include "refreshtimer.hxx"
#include "dbcolect.hxx"
#include "scextopt.hxx"
#include "compiler.hxx"
#include "cfgids.hxx"
#include "warnpassword.hxx"
#include "optsolver.hxx"
#include "sheetdata.hxx"
#include "tabprotection.hxx"
#include "dpobject.hxx"

#include "docsh.hxx"
#include "docshimp.hxx"
#include <sfx2/viewfrm.hxx>
#include <rtl/logfile.hxx>

#include <comphelper/processfactory.hxx>
#include "uiitems.hxx"
#include "cellsuno.hxx"

using namespace com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// STATIC DATA -----------------------------------------------------------

//	Stream-Namen im Storage

const sal_Char __FAR_DATA ScDocShell::pStarCalcDoc[] = STRING_SCSTREAM;		// "StarCalcDocument"
const sal_Char __FAR_DATA ScDocShell::pStyleName[] = "SfxStyleSheets";

//	Filter-Namen (wie in sclib.cxx)

static const sal_Char __FAR_DATA pFilterSc50[]		= "StarCalc 5.0";
//static const sal_Char __FAR_DATA pFilterSc50Temp[]	= "StarCalc 5.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc40[]		= "StarCalc 4.0";
//static const sal_Char __FAR_DATA pFilterSc40Temp[]	= "StarCalc 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc30[]		= "StarCalc 3.0";
//static const sal_Char __FAR_DATA pFilterSc30Temp[]	= "StarCalc 3.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterSc10[]		= "StarCalc 1.0";
static const sal_Char __FAR_DATA pFilterXML[]		= "StarOffice XML (Calc)";
static const sal_Char __FAR_DATA pFilterAscii[]		= "Text - txt - csv (StarCalc)";
static const sal_Char __FAR_DATA pFilterLotus[]		= "Lotus";
static const sal_Char __FAR_DATA pFilterQPro6[]		= "Quattro Pro 6.0";
static const sal_Char __FAR_DATA pFilterExcel4[]	= "MS Excel 4.0";
static const sal_Char __FAR_DATA pFilterEx4Temp[]	= "MS Excel 4.0 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel5[]	= "MS Excel 5.0/95";
static const sal_Char __FAR_DATA pFilterEx5Temp[]	= "MS Excel 5.0/95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel95[]	= "MS Excel 95";
static const sal_Char __FAR_DATA pFilterEx95Temp[]	= "MS Excel 95 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterExcel97[]	= "MS Excel 97";
static const sal_Char __FAR_DATA pFilterEx97Temp[]	= "MS Excel 97 Vorlage/Template";
static const sal_Char __FAR_DATA pFilterEx07Xml[]   = "MS Excel 2007 XML";
static const sal_Char __FAR_DATA pFilterDBase[]		= "dBase";
static const sal_Char __FAR_DATA pFilterDif[]		= "DIF";
static const sal_Char __FAR_DATA pFilterSylk[]		= "SYLK";
static const sal_Char __FAR_DATA pFilterHtml[]		= "HTML (StarCalc)";
static const sal_Char __FAR_DATA pFilterHtmlWebQ[]	= "calc_HTML_WebQuery";
static const sal_Char __FAR_DATA pFilterRtf[]		= "Rich Text Format (StarCalc)";

//----------------------------------------------------------------------

#define ScDocShell
#include "scslots.hxx"

namespace
{
    template< bool bByName >
    inline sal_uInt8 GetMediumFlag( const String & rName )
    {
        sal_uInt8 bResult = E_MEDIUM_FLAG_NONE;

#define SFX2_FILTER_ENTRY( entry ) { #entry, (sizeof #entry)/sizeof((#entry)[0]) - 1 },
        static const struct
        {
            const char * mpFilterTypeName;
            unsigned mnFilterTypeLen;
        } szMSFilterTypes [] = 
        {
            SFX2_FILTER_ENTRY(calc_MS_Excel_40)
            SFX2_FILTER_ENTRY(calc_MS_Excel_40_VorlageTemplate)
            SFX2_FILTER_ENTRY(calc_MS_Excel_5095)
            SFX2_FILTER_ENTRY(calc_MS_Excel_5095_VorlageTemplate)
            SFX2_FILTER_ENTRY(calc_MS_Excel_95)
            SFX2_FILTER_ENTRY(calc_MS_Excel_95_VorlageTemplate)
            SFX2_FILTER_ENTRY(calc_MS_Excel_97)
            SFX2_FILTER_ENTRY(calc_MS_Excel_97_VorlageTemplate)
            SFX2_FILTER_ENTRY(calc_MS_Excel_2003_XML)
            SFX2_FILTER_ENTRY(MS Excel 2007 XML)
            SFX2_FILTER_ENTRY(MS Excel 2007 XML Template)
            SFX2_FILTER_ENTRY(MS Excel 2007 Binary)
        };

        static const struct
        {
            const char * mpFilterName;
            size_t mnFilterNameLen;
        } szMSFilterNames [] = 
        {
            { pFilterExcel4, strlen( pFilterExcel4 ) },
            { pFilterEx4Temp, strlen( pFilterEx4Temp ) },
            { pFilterExcel95, strlen( pFilterExcel95 ) },
            { pFilterEx95Temp, strlen( pFilterEx95Temp ) },
            { pFilterExcel5, strlen( pFilterExcel5 ) },
            { pFilterEx5Temp, strlen( pFilterEx5Temp ) },
            { pFilterExcel97, strlen( pFilterExcel97 ) },
            { pFilterEx97Temp, strlen( pFilterEx97Temp ) },
            SFX2_FILTER_ENTRY(Microsoft Excel 2003 XML)
            { pFilterEx07Xml, strlen( pFilterEx07Xml ) },
            SFX2_FILTER_ENTRY(Microsoft Excel 2007 XML Template)
            SFX2_FILTER_ENTRY(Microsoft Excel 2007 Binary)
        };

        enum{
            e_calc_MS_Excel_40,
            e_calc_MS_Excel_40_VorlageTemplate,
            e_calc_MS_Excel_5095,
            e_calc_MS_Excel_5095_VorlageTemplate,
            e_calc_MS_Excel_95,
            Se_calc_MS_Excel_95_VorlageTemplate,
            e_calc_MS_Excel_97,
            e_calc_MS_Excel_97_VorlageTemplate,
            e_calc_MS_Excel_2003_XML,
            e_MS_Excel_2007_XML,
            e_MS_Excel_2007_XML_Template,
            e_MS_Excel_2007_Binary
        };

#undef SFX2_FILTER_ENTRY

        if( bByName )
        {
            for( unsigned i = 0; i < (sizeof szMSFilterNames)/sizeof(szMSFilterNames[0] ); i++ )
                if( rName.Len() == szMSFilterNames[i].mnFilterNameLen 
                    && std::equal( szMSFilterNames[i].mpFilterName, szMSFilterNames[i].mpFilterName + szMSFilterNames[i].mnFilterNameLen, rName.GetBuffer() ) )
                    bResult |= ( E_MEDIUM_FLAG_EXCEL | ( ( i == e_MS_Excel_2007_XML ) * E_MEDIUM_FLAG_MSXML ) );
        }
        else
        {
            for( unsigned i = 0; i < (sizeof szMSFilterTypes)/sizeof(szMSFilterTypes[0] ); i++ )
                if( rName.Len() == szMSFilterTypes[i].mnFilterTypeLen 
                    && std::equal( szMSFilterTypes[i].mpFilterTypeName, szMSFilterTypes[i].mpFilterTypeName + szMSFilterTypes[i].mnFilterTypeLen, rName.GetBuffer() ) )
                    bResult |= ( E_MEDIUM_FLAG_EXCEL | ( ( i == e_MS_Excel_2007_XML ) * E_MEDIUM_FLAG_MSXML ) );
        }

        return bResult;
    }

    inline sal_uInt8 GetMediumFlag( const SfxMedium * pMedium )
    {
        if( const SfxFilter * pFilter = pMedium ? pMedium->GetFilter() : NULL )
            return GetMediumFlag<false>( pFilter->GetTypeName() );

        return E_MEDIUM_FLAG_NONE;
    }
}

SFX_IMPL_INTERFACE(ScDocShell,SfxObjectShell, ScResId(SCSTR_DOCSHELL))
{
	SFX_CHILDWINDOW_REGISTRATION( SID_HYPERLINK_INSERT );
}

//	GlobalName der aktuellen Version:
SFX_IMPL_OBJECTFACTORY( ScDocShell, SvGlobalName(SO3_SC_CLASSID), SFXOBJECTSHELL_STD_NORMAL, "scalc" )

TYPEINIT1( ScDocShell, SfxObjectShell );		// SfxInPlaceObject: kein Type-Info ?

//------------------------------------------------------------------

void __EXPORT ScDocShell::FillClass( SvGlobalName* pClassName,
										sal_uInt32* pFormat,
                                        String* /* pAppName */,
										String* pFullTypeName,
										String* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
	if ( nFileFormat == SOFFICE_FILEFORMAT_60 )
	{
		*pClassName		= SvGlobalName( SO3_SC_CLASSID_60 );
		*pFormat		= SOT_FORMATSTR_ID_STARCALC_60;
		*pFullTypeName	= String( ScResId( SCSTR_LONG_SCDOC_NAME ) );
		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
	}
	else if ( nFileFormat == SOFFICE_FILEFORMAT_8 )
	{
		*pClassName		= SvGlobalName( SO3_SC_CLASSID_60 );
        *pFormat		= bTemplate ? SOT_FORMATSTR_ID_STARCALC_8_TEMPLATE : SOT_FORMATSTR_ID_STARCALC_8;
		*pFullTypeName	= String( RTL_CONSTASCII_USTRINGPARAM("calc8") );
		*pShortTypeName	= String( ScResId( SCSTR_SHORT_SCDOC_NAME ) );
	}
	else
	{
		DBG_ERROR("wat fuer ne Version?");
	}
}

//------------------------------------------------------------------

void ScDocShell::DoEnterHandler()
{
	ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
	if (pViewSh)
		if (pViewSh->GetViewData()->GetDocShell() == this)
			SC_MOD()->InputEnterHandler();
}

//------------------------------------------------------------------

SCTAB ScDocShell::GetSaveTab()
{
	SCTAB nTab = 0;
	ScTabViewShell* pSh = GetBestViewShell();
	if (pSh)
	{
		const ScMarkData& rMark = pSh->GetViewData()->GetMarkData();
		for ( nTab = 0; nTab <= MAXTAB; nTab++ )	// erste markierte Tabelle
			if ( rMark.GetTableSelect( nTab ) )
				break;
	}
	return nTab;
}

sal_uInt16 ScDocShell::GetHiddenInformationState( sal_uInt16 nStates )
{
	// get global state like HIDDENINFORMATION_DOCUMENTVERSIONS
    sal_uInt16 nState = SfxObjectShell::GetHiddenInformationState( nStates );

	if ( nStates & HIDDENINFORMATION_RECORDEDCHANGES )
    {
        if ( aDocument.GetChangeTrack() && aDocument.GetChangeTrack()->GetFirst() )
          nState |= HIDDENINFORMATION_RECORDEDCHANGES;
    }
    if ( nStates & HIDDENINFORMATION_NOTES )
    {
        SCTAB nTableCount = aDocument.GetTableCount();
        SCTAB nTable = 0;
        sal_Bool bFound(sal_False);
	    while ( nTable < nTableCount && !bFound )
	    {
            ScCellIterator aCellIter( &aDocument, 0,0, nTable, MAXCOL,MAXROW, nTable );
            for( ScBaseCell* pCell = aCellIter.GetFirst(); pCell && !bFound; pCell = aCellIter.GetNext() )
                if (pCell->HasNote())
                    bFound = sal_True;
            nTable++;
        }

        if (bFound)
            nState |= HIDDENINFORMATION_NOTES;
    }

	return nState;
}

void ScDocShell::BeforeXMLLoading()
{
    aDocument.DisableIdle( sal_True );

    // prevent unnecessary broadcasts and updates
    DBG_ASSERT(pModificator == NULL, "The Modificator should not exist");
	pModificator = new ScDocShellModificator( *this );

    aDocument.SetImportingXML( sal_True );
    aDocument.EnableExecuteLink( false );   // #i101304# to be safe, prevent nested loading from external references
    aDocument.EnableUndo( sal_False );
	// prevent unnecessary broadcasts and "half way listeners"
	aDocument.SetInsertingFromOtherDoc( sal_True );

	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
		ScColumn::bDoubleAlloc = sal_True;
}

void ScDocShell::AfterXMLLoading(sal_Bool bRet)
{
	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
	{
		UpdateLinks();
		// don't prevent establishing of listeners anymore
		aDocument.SetInsertingFromOtherDoc( sal_False );
		if ( bRet )
		{
			ScChartListenerCollection* pChartListener = aDocument.GetChartListenerCollection();
			if (pChartListener)
				pChartListener->UpdateDirtyCharts();

			// #95582#; set the table names of linked tables to the new path
			SCTAB nTabCount = aDocument.GetTableCount();
			for (SCTAB i = 0; i < nTabCount; ++i)
			{
				if (aDocument.IsLinked( i ))
				{
					String aName;
					aDocument.GetName(i, aName);
					String aLinkTabName = aDocument.GetLinkTab(i);
					xub_StrLen nLinkTabNameLength = aLinkTabName.Len();
					xub_StrLen nNameLength = aName.Len();
					if (nLinkTabNameLength < nNameLength)
					{

						// remove the quottes on begin and end of the docname and restore the escaped quotes
						const sal_Unicode* pNameBuffer = aName.GetBuffer();
						if ( *pNameBuffer == '\'' && // all docnames have to have a ' character on the first pos
							ScGlobal::UnicodeStrChr( pNameBuffer, SC_COMPILER_FILE_TAB_SEP ) )
						{
							rtl::OUStringBuffer aDocURLBuffer;
							sal_Bool bQuote = sal_True;			// Dokumentenname ist immer quoted
							++pNameBuffer;
							while ( bQuote && *pNameBuffer )
							{
								if ( *pNameBuffer == '\'' && *(pNameBuffer-1) != '\\' )
									bQuote = sal_False;
								else if( !(*pNameBuffer == '\\' && *(pNameBuffer+1) == '\'') )
									aDocURLBuffer.append(*pNameBuffer);		// falls escaped Quote: nur Quote in den Namen
								++pNameBuffer;
							}


							if( *pNameBuffer == SC_COMPILER_FILE_TAB_SEP )  // after the last quote of the docname should be the # char
							{
								xub_StrLen nIndex = nNameLength - nLinkTabNameLength;
								INetURLObject aINetURLObject(aDocURLBuffer.makeStringAndClear());
								if(	aName.Equals(aLinkTabName, nIndex, nLinkTabNameLength) &&
									(aName.GetChar(nIndex - 1) == '#') && // before the table name should be the # char
									!aINetURLObject.HasError()) // the docname should be a valid URL
								{
                        	    	aName = ScGlobal::GetDocTabName( aDocument.GetLinkDoc( i ), aDocument.GetLinkTab( i ) );
	                            	aDocument.RenameTab(i, aName, sal_True, sal_True);
								}
								// else;  nothing has to happen, because it is a user given name
							}
							// else;  nothing has to happen, because it is a user given name
						}
						// else;  nothing has to happen, because it is a user given name
					}
					// else;  nothing has to happen, because it is a user given name
				}
			}

            // #i94570# DataPilot table names have to be unique, or the tables can't be accessed by API.
            // If no name (or an invalid name, skipped in ScXMLDataPilotTableContext::EndElement) was set, create a new name.
            ScDPCollection* pDPCollection = aDocument.GetDPCollection();
            if ( pDPCollection )
            {
                sal_uInt16 nDPCount = pDPCollection->GetCount();
                for (sal_uInt16 nDP=0; nDP<nDPCount; nDP++)
                {
                    ScDPObject* pDPObj = (*pDPCollection)[nDP];
                    if ( !pDPObj->GetName().Len() )
                        pDPObj->SetName( pDPCollection->CreateNewName() );
                }
            }
		}
		ScColumn::bDoubleAlloc = sal_False;
    }
    else
		aDocument.SetInsertingFromOtherDoc( sal_False );
		
	aDocument.SetImportingXML( sal_False );
    aDocument.EnableExecuteLink( true );
    aDocument.EnableUndo( sal_True );
    bIsEmpty = sal_False;

    if (pModificator)
    {
        delete pModificator;
        pModificator = NULL;
    }
    else
    {
        DBG_ERROR("The Modificator should exist");
    }

    aDocument.DisableIdle( sal_False );
}

sal_Bool ScDocShell::LoadXML( SfxMedium* pLoadMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::LoadXML" );

	//	MacroCallMode is no longer needed, state is kept in SfxObjectShell now

	// no Seek(0) here - always loading from storage, GetInStream must not be called

    BeforeXMLLoading();

    // #i62677# BeforeXMLLoading is also called from ScXMLImport::startDocument when invoked
    // from an external component. The XMLFromWrapper flag is only set here, when called
    // through ScDocShell.
    aDocument.SetXMLFromWrapper( sal_True );

    ScXMLImportWrapper aImport( aDocument, pLoadMedium, xStor );

    sal_Bool bRet(sal_False);
    ErrCode nError = ERRCODE_NONE;
	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
        bRet = aImport.Import(sal_False, nError);
	else
        bRet = aImport.Import(sal_True, nError);

    if ( nError )
        pLoadMedium->SetError( nError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    aDocument.SetXMLFromWrapper( sal_False );
    AfterXMLLoading(bRet);

	//!	row heights...

	return bRet;
}

sal_Bool ScDocShell::SaveXML( SfxMedium* pSaveMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScDocShell::SaveXML" );

    aDocument.DisableIdle( sal_True );

    ScXMLImportWrapper aImport( aDocument, pSaveMedium, xStor );
	sal_Bool bRet(sal_False);
	if (GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
		bRet = aImport.Export(sal_False);
	else
		bRet = aImport.Export(sal_True);

    aDocument.DisableIdle( sal_False );

    return bRet;
}

sal_Bool __EXPORT ScDocShell::Load( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Load" );

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

	//	only the latin script language is loaded
	//	-> initialize the others from options (before loading)
    InitOptions(true);

	GetUndoManager()->Clear();

    sal_Bool bRet = SfxObjectShell::Load( rMedium );
	if( bRet )
	{
        if (GetMedium())
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
            nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
        }

		{
			//	prepare a valid document for XML filter
			//	(for ConvertFrom, InitNew is called before)
			aDocument.MakeTable(0);
			aDocument.GetStyleSheetPool()->CreateStandardStyles();
			aDocument.UpdStlShtPtrsFrmNms();

            bRet = LoadXML( &rMedium, NULL );
		}
	}

    if (!bRet && !rMedium.GetError())
        rMedium.SetError( SVSTREAM_FILEFORMAT_ERROR, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    if (rMedium.GetError())
        SetError( rMedium.GetError(), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

	InitItems();
	CalcOutputFactor();

	// #73762# invalidate eventually temporary table areas
	if ( bRet )
		aDocument.InvalidateTableArea();

	bIsEmpty = sal_False;
	FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
	return bRet;
}

void __EXPORT ScDocShell::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (rHint.ISA(ScTablesHint) )
    {
        const ScTablesHint& rScHint = static_cast< const ScTablesHint& >( rHint );
        if (rScHint.GetId() == SC_TAB_INSERTED)
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents = aDocument.GetVbaEventProcessor();
            if ( xVbaEvents.is() ) try
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[0] <<= rScHint.GetTab1();
                xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_NEWSHEET, aArgs );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

	if (rHint.ISA(SfxSimpleHint))								// ohne Parameter
	{
		sal_uLong nSlot = ((const SfxSimpleHint&)rHint).GetId();
		switch ( nSlot )
		{
			case SFX_HINT_TITLECHANGED:
				aDocument.SetName( SfxShell::GetName() );
				//	RegisterNewTargetNames gibts nicht mehr
				SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DOCNAME_CHANGED ));	// Navigator
				break;
		}
	}
	else if (rHint.ISA(SfxStyleSheetHint))						// Vorlagen geaendert
		NotifyStyle((const SfxStyleSheetHint&) rHint);
	else if (rHint.ISA(ScAutoStyleHint))
	{
		//!	direct call for AutoStyles

		//	this is called synchronously from ScInterpreter::ScStyle,
		//	modifying the document must be asynchronous
		//	(handled by AddInitial)

		ScAutoStyleHint& rStlHint = (ScAutoStyleHint&)rHint;
		ScRange aRange = rStlHint.GetRange();
		String aName1 = rStlHint.GetStyle1();
		String aName2 = rStlHint.GetStyle2();
		sal_uInt32 nTimeout = rStlHint.GetTimeout();

		if (!pAutoStyleList)
			pAutoStyleList = new ScAutoStyleList(this);
		pAutoStyleList->AddInitial( aRange, aName1, nTimeout, aName2 );
	}
    else if ( rHint.ISA( SfxEventHint ) )
    {
        sal_uLong nEventId = ((SfxEventHint&)rHint).GetEventId();
        switch ( nEventId )
        {
            case SFX_EVENT_LOADFINISHED:
                {
                    // the readonly documents should not be opened in shared mode
                    if ( HasSharedXMLFlagSet() && !SC_MOD()->IsInSharedDocLoading() && !IsReadOnly() )
                    {
                        if ( SwitchToShared( sal_True, sal_False ) )
                        {
                            ScViewData* pViewData = GetViewData();
                            ScTabView* pTabView = ( pViewData ? dynamic_cast< ScTabView* >( pViewData->GetView() ) : NULL );
                            if ( pTabView )
                            {
                                pTabView->UpdateLayerLocks();
                            }
                        }
                        else
                        {
                            // switching to shared mode has failed, the document should be opened readonly
                            // TODO/LATER: And error message should be shown here probably
                            SetReadOnlyUI( sal_True );
                        }
                    }
                }
                break;
            case SFX_EVENT_VIEWCREATED:
                {
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocLoading() )
                    {
                        ScAppOptions aAppOptions = SC_MOD()->GetAppOptions();
                        if ( aAppOptions.GetShowSharedDocumentWarning() )
                        {
                            WarningBox aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                ScGlobal::GetRscString( STR_SHARED_DOC_WARNING ) );
                            aBox.SetDefaultCheckBoxText();
                            aBox.Execute();
                            sal_Bool bChecked = aBox.GetCheckBoxState();
                            if ( bChecked )
                            {
                                aAppOptions.SetShowSharedDocumentWarning( !bChecked );
                                SC_MOD()->SetAppOptions( aAppOptions );
                            }
                        }
                    }

                    try
                    {
                        uno::Reference< uno::XComponentContext > xContext;
                        uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
                        uno::Reference< beans::XPropertySet > xProp( xServiceManager, uno::UNO_QUERY_THROW );
                        xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ) >>= xContext;
                        if ( xContext.is() )
                        {
                            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xServiceManager, uno::UNO_QUERY_THROW );
                            uno::Reference< container::XEnumeration> xEnum = xEnumAccess->createContentEnumeration(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocumentJob" ) ) );
                            if ( xEnum.is() )
                            {
                                while ( xEnum->hasMoreElements() )
                                {
                                    uno::Any aAny = xEnum->nextElement();
                                    uno::Reference< lang::XSingleComponentFactory > xFactory;
                                    aAny >>= xFactory;
                                    if ( xFactory.is() )
                                    {
                                        uno::Reference< task::XJob > xJob( xFactory->createInstanceWithContext( xContext ), uno::UNO_QUERY_THROW );
                                        uno::Sequence< beans::NamedValue > aArgsForJob(1);
                                        ScViewData* pViewData = GetViewData();
                                        SfxViewShell* pViewShell = ( pViewData ? pViewData->GetViewShell() : NULL );
                                        SfxViewFrame* pViewFrame = ( pViewShell ? pViewShell->GetViewFrame() : NULL );
                                        SfxFrame* pFrame = ( pViewFrame ? &pViewFrame->GetFrame() : NULL );
                                        uno::Reference< frame::XController > xController = ( pFrame ? pFrame->GetController() : 0 );
                                        uno::Reference< sheet::XSpreadsheetView > xSpreadsheetView( xController, uno::UNO_QUERY_THROW );
                                        aArgsForJob[0] = beans::NamedValue( ::rtl::OUString::createFromAscii( "SpreadsheetView" ),
                                            uno::makeAny( xSpreadsheetView ) );
                                        xJob->execute( aArgsForJob );
                                    }
                                }
                            }
                        }
                    }
                    catch ( uno::Exception & )
                    {
                    }
                }
                break;
            case SFX_EVENT_SAVEDOC:
                {
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocSaving() )
                    {
                        bool bSuccess = false;
                        bool bRetry = true;
                        while ( bRetry )
                        {
                            bRetry = false;
                            uno::Reference< frame::XModel > xModel;
                            try
                            {
                                // load shared file
                                xModel.set( LoadSharedDocument(), uno::UNO_QUERY_THROW );
                                uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY_THROW );

                                // check if shared flag is set in shared file
                                bool bShared = false;
                                ScModelObj* pDocObj = ScModelObj::getImplementation( xModel );
                                ScDocShell* pSharedDocShell = ( pDocObj ? dynamic_cast< ScDocShell* >( pDocObj->GetObjectShell() ) : NULL );
                                if ( pSharedDocShell )
                                {
                                    bShared = pSharedDocShell->HasSharedXMLFlagSet();
                                }

                                // #i87870# check if shared status was disabled and enabled again
                                bool bOwnEntry = false;
                                bool bEntriesNotAccessible = false;
                                try
                                {
                                    ::svt::ShareControlFile aControlFile( GetSharedFileURL() );
                                    bOwnEntry = aControlFile.HasOwnEntry();
                                }
                                catch ( uno::Exception& )
                                {
                                    bEntriesNotAccessible = true;
                                }

                                if ( bShared && bOwnEntry )
                                {
                                    uno::Reference< frame::XStorable > xStorable( xModel, uno::UNO_QUERY_THROW );

                                    if ( xStorable->isReadonly() )
                                    {
                                        xCloseable->close( sal_True );

                                        String aUserName( ScGlobal::GetRscString( STR_UNKNOWN_USER ) );
                                        bool bNoLockAccess = false;
                                        try
                                        {
                                            ::svt::DocumentLockFile aLockFile( GetSharedFileURL() );
                                            uno::Sequence< ::rtl::OUString > aData = aLockFile.GetLockData();
                                            if ( aData.getLength() > LOCKFILE_SYSUSERNAME_ID )
                                            {
                                                if ( aData[LOCKFILE_OOOUSERNAME_ID].getLength() > 0 )
                                                {
                                                    aUserName = aData[LOCKFILE_OOOUSERNAME_ID];
                                                }
                                                else if ( aData[LOCKFILE_SYSUSERNAME_ID].getLength() > 0 )
                                                {
                                                    aUserName = aData[LOCKFILE_SYSUSERNAME_ID];
                                                }
                                            }
                                        }
                                        catch ( uno::Exception& )
                                        {
                                            bNoLockAccess = true;
                                        }

                                        if ( bNoLockAccess )
                                        {
                                            // TODO/LATER: in future an error regarding impossibility to open file for writing could be shown
                                            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                        }
                                        else
                                        {
                                            String aMessage( ScGlobal::GetRscString( STR_FILE_LOCKED_SAVE_LATER ) );
                                            aMessage.SearchAndReplaceAscii( "%1", aUserName );

                                            WarningBox aBox( GetActiveDialogParent(), WinBits( WB_RETRY_CANCEL | WB_DEF_RETRY ), aMessage );
                                            if ( aBox.Execute() == RET_RETRY )
                                            {
                                                bRetry = true;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        // merge changes from shared file into temp file
                                        bool bSaveToShared = false;
                                        if ( pSharedDocShell )
                                        {
                                            bSaveToShared = MergeSharedDocument( pSharedDocShell );
                                        }

                                        // close shared file
                                        xCloseable->close( sal_True );

                                        // TODO: keep file lock on shared file

                                        // store to shared file
                                        if ( bSaveToShared )
                                        {
                                            bool bChangedViewSettings = false;
                                            ScChangeViewSettings* pChangeViewSet = aDocument.GetChangeViewSettings();
                                            if ( pChangeViewSet && pChangeViewSet->ShowChanges() )
                                            {
                                                pChangeViewSet->SetShowChanges( sal_False );
                                                pChangeViewSet->SetShowAccepted( sal_False );
                                                aDocument.SetChangeViewSettings( *pChangeViewSet );
                                                bChangedViewSettings = true;
                                            }

                                            uno::Reference< frame::XStorable > xStor( GetModel(), uno::UNO_QUERY_THROW );
                                            // TODO/LATER: More entries from the MediaDescriptor might be interesting for the merge
                                            uno::Sequence< beans::PropertyValue > aValues(1);
                                            aValues[0].Name = ::rtl::OUString::createFromAscii( "FilterName" );
                                            aValues[0].Value <<= ::rtl::OUString( GetMedium()->GetFilter()->GetFilterName() );

                                            SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False);
                                            if ( pPasswordItem && pPasswordItem->GetValue().Len() )
                                            {
                                                aValues.realloc( 2 );
                                                aValues[1].Name = ::rtl::OUString::createFromAscii( "Password" );
                                                aValues[1].Value <<= ::rtl::OUString( pPasswordItem->GetValue() );
                                            }

                                            SC_MOD()->SetInSharedDocSaving( true );
                                            xStor->storeToURL( GetSharedFileURL(), aValues );
                                            SC_MOD()->SetInSharedDocSaving( false );

                                            if ( bChangedViewSettings )
                                            {
                                                pChangeViewSet->SetShowChanges( sal_True );
                                                pChangeViewSet->SetShowAccepted( sal_True );
                                                aDocument.SetChangeViewSettings( *pChangeViewSet );
                                            }
                                        }

                                        bSuccess = true;
                                        GetUndoManager()->Clear();
                                    }
                                }
                                else
                                {
                                    xCloseable->close( sal_True );

                                    if ( bEntriesNotAccessible )
                                    {
                                        // TODO/LATER: in future an error regarding impossibility to write to share control file could be shown
                                        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                                    }
                                    else
                                    {
                                        WarningBox aBox( GetActiveDialogParent(), WinBits( WB_OK ),
                                            ScGlobal::GetRscString( STR_DOC_NOLONGERSHARED ) );
                                        aBox.Execute();

                                        SfxBindings* pBindings = GetViewBindings();
                                        if ( pBindings )
                                        {
                                            pBindings->ExecuteSynchron( SID_SAVEASDOC );
                                        }
                                    }
                                }
                            }
                            catch ( uno::Exception& )
                            {
                                DBG_ERROR( "SFX_EVENT_SAVEDOC: caught exception\n" );
                                SC_MOD()->SetInSharedDocSaving( false );

                                try
                                {
                                    uno::Reference< util::XCloseable > xClose( xModel, uno::UNO_QUERY_THROW );
                                    xClose->close( sal_True );
                                }
                                catch ( uno::Exception& )
                                {
                                }
                            }
                        }

                        if ( !bSuccess )
                            SetError( ERRCODE_IO_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) ); // this error code will produce no error message, but will break the further saving process
                    }
                    if (pSheetSaveData)
                        pSheetSaveData->SetInSupportedSave(true);
                }
                break;
            case SFX_EVENT_SAVEASDOC:
            case SFX_EVENT_SAVETODOC:
                // #i108978# If no event is sent before saving, there will also be no "...DONE" event,
                // and SAVE/SAVEAS can't be distinguished from SAVETO. So stream copying is only enabled
                // if there is a SAVE/SAVEAS/SAVETO event first.
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(true);
                break;
            case SFX_EVENT_SAVEDOCDONE:
                {
                    if ( IsDocShared() && !SC_MOD()->IsInSharedDocSaving() )
                    {
                    }
                    UseSheetSaveEntries();      // use positions from saved file for next saving
                    if (pSheetSaveData)
                        pSheetSaveData->SetInSupportedSave(false);
                }
                break;
            case SFX_EVENT_SAVEASDOCDONE:
                // new positions are used after "save" and "save as", but not "save to"
                UseSheetSaveEntries();      // use positions from saved file for next saving
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(false);
                break;
            case SFX_EVENT_SAVETODOCDONE:
                // only reset the flag, don't use the new positions
                if (pSheetSaveData)
                    pSheetSaveData->SetInSupportedSave(false);
                break;
            default:
                {
                }
                break;
        }
    }
}

	// Inhalte fuer Organizer laden


sal_Bool __EXPORT ScDocShell::LoadFrom( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::LoadFrom" );

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

	WaitObject aWait( GetActiveDialogParent() );

	sal_Bool bRet = sal_False;

    if (GetMedium())
    {
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
        nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;
    }

    //  until loading/saving only the styles in XML is implemented,
    //  load the whole file
    bRet = LoadXML( &rMedium, NULL );
    InitItems();

    SfxObjectShell::LoadFrom( rMedium );

	return bRet;
}

static void lcl_parseHtmlFilterOption(const OUString& rOption, LanguageType& rLang, bool& rDateConvert)
{
    OUStringBuffer aBuf;
    OUString aTokens[2];
    sal_Int32 n = rOption.getLength();
    const sal_Unicode* p = rOption.getStr();
    sal_Int32 nTokenId = 0;
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const sal_Unicode c = p[i];
        if (c == sal_Unicode(' '))
        {
            if (aBuf.getLength())
                aTokens[nTokenId++] = aBuf.makeStringAndClear();
        }
        else
            aBuf.append(c);

        if (nTokenId >= 2)
            break;
    }

    if (aBuf.getLength())
        aTokens[nTokenId] = aBuf.makeStringAndClear();

    rLang = static_cast<LanguageType>(aTokens[0].toInt32());
    rDateConvert = static_cast<bool>(aTokens[1].toInt32());
}

namespace {

class LoadMediumGuard
{
public:
    explicit LoadMediumGuard(ScDocument* pDoc) :
        mpDoc(pDoc)
    {
        mpDoc->SetLoadingMedium(true);
    }

    ~LoadMediumGuard()
    {
        mpDoc->SetLoadingMedium(false);
    }
private:
    ScDocument* mpDoc;
};

}

sal_Bool __EXPORT ScDocShell::ConvertFrom( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertFrom" );

    LoadMediumGuard aLoadGuard(&aDocument);

	sal_Bool bRet = sal_False;				// sal_False heisst Benutzerabbruch !!
									// bei Fehler: Fehler am Stream setzen!!

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

	GetUndoManager()->Clear();

	// ob nach dem Import optimale Spaltenbreiten gesetzt werden sollen
	sal_Bool bSetColWidths = sal_False;
	sal_Bool bSetSimpleTextColWidths = sal_False;
	sal_Bool bSimpleColWidth[MAXCOLCOUNT];
	memset( bSimpleColWidth, 1, (MAXCOLCOUNT) * sizeof(sal_Bool) );
	ScRange aColWidthRange;
	// ob nach dem Import optimale Zeilenhoehen gesetzt werden sollen
	sal_Bool bSetRowHeights = sal_False;

	aConvFilterName.Erase(); //@ #BugId 54198

	//	Alle Filter brauchen die komplette Datei am Stueck (nicht asynchron),
	//	darum vorher per CreateFileStream dafuer sorgen, dass die komplette
	//	Datei uebertragen wird.
	rMedium.GetPhysicalName();	//! CreateFileStream direkt rufen, wenn verfuegbar

    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    nCanUpdate = pUpdateDocItem ? pUpdateDocItem->GetValue() : com::sun::star::document::UpdateDocMode::NO_UPDATE;

    const SfxFilter* pFilter = rMedium.GetFilter();
	if (pFilter)
	{
		String aFltName = pFilter->GetFilterName();

		aConvFilterName=aFltName; //@ #BugId 54198

		sal_Bool bCalc3 = ( aFltName.EqualsAscii(pFilterSc30) );
		sal_Bool bCalc4 = ( aFltName.EqualsAscii(pFilterSc40) );
		if (!bCalc3 && !bCalc4)
			aDocument.SetInsertingFromOtherDoc( sal_True );

        if (aFltName.EqualsAscii(pFilterXML))
			bRet = LoadXML( &rMedium, NULL );
		else if (aFltName.EqualsAscii(pFilterSc10))
		{
			SvStream* pStream = rMedium.GetInStream();
			if (pStream)
			{
				FltError eError = ScFormatFilter::Get().ScImportStarCalc10( *pStream, &aDocument );
				if (eError != eERR_OK)
				{
					if (!GetError())
						SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
				}
				else
					bRet = sal_True;
			}
		}
		else if (aFltName.EqualsAscii(pFilterLotus))
		{
			String sItStr;
			SfxItemSet*	 pSet = rMedium.GetItemSet();
			const SfxPoolItem* pItem;
			if ( pSet && SFX_ITEM_SET ==
				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
			{
				sItStr = ((const SfxStringItem*)pItem)->GetValue();
			}

			if (sItStr.Len() == 0)
			{
				//	default for lotus import (from API without options):
				//	IBM_437 encoding
				sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_437 );
			}

			ScColumn::bDoubleAlloc = sal_True;
			FltError eError = ScFormatFilter::Get().ScImportLotus123( rMedium, &aDocument,
												ScGlobal::GetCharsetValue(sItStr));
			ScColumn::bDoubleAlloc = sal_False;
			if (eError != eERR_OK)
			{
				if (!GetError())
					SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

				if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
					bRet = sal_True;
			}
			else
				bRet = sal_True;
			bSetColWidths = sal_True;
			bSetRowHeights = sal_True;
		}
		else if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterExcel5) ||
				   aFltName.EqualsAscii(pFilterExcel95) || aFltName.EqualsAscii(pFilterExcel97) ||
				   aFltName.EqualsAscii(pFilterEx4Temp) || aFltName.EqualsAscii(pFilterEx5Temp) ||
				   aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp) )
		{
			EXCIMPFORMAT eFormat = EIF_AUTO;
			if ( aFltName.EqualsAscii(pFilterExcel4) || aFltName.EqualsAscii(pFilterEx4Temp) )
				eFormat = EIF_BIFF_LE4;
			else if ( aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
					  aFltName.EqualsAscii(pFilterEx5Temp) || aFltName.EqualsAscii(pFilterEx95Temp) )
				eFormat = EIF_BIFF5;
			else if ( aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx97Temp) )
				eFormat = EIF_BIFF8;

			MakeDrawLayer();				//! im Filter
            CalcOutputFactor();             // #93255# prepare update of row height
			ScColumn::bDoubleAlloc = sal_True;
			FltError eError = ScFormatFilter::Get().ScImportExcel( rMedium, &aDocument, eFormat );
			ScColumn::bDoubleAlloc = sal_False;
			aDocument.UpdateFontCharSet();
			if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
				aDocument.UpdateChartListenerCollection();				//! fuer alle Importe?

			// #75299# all graphics objects must have names
			aDocument.EnsureGraphicNames();

			if (eError == SCWARN_IMPORT_RANGE_OVERFLOW)
			{
				if (!GetError())
					SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
				bRet = sal_True;
			}
			else if (eError != eERR_OK)
			{
				if (!GetError())
					SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
			}
			else
				bRet = sal_True;

            // #93255# update of row height done inside of Excel filter to speed up chart import
//            bSetRowHeights = sal_True;      //  #75357# optimal row heights must be updated
		}
		else if (aFltName.EqualsAscii(pFilterAscii))
		{
			SfxItemSet*	 pSet = rMedium.GetItemSet();
			const SfxPoolItem* pItem;
			ScAsciiOptions aOptions;
			sal_Bool bOptInit = sal_False;

			if ( pSet && SFX_ITEM_SET ==
				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
			{
				aOptions.ReadFromString( ((const SfxStringItem*)pItem)->GetValue() );
				bOptInit = sal_True;
			}

			if ( !bOptInit )
			{
				//	default for ascii import (from API without options):
				//	ISO8859-1/MS_1252 encoding, comma, double quotes

				aOptions.SetCharSet( RTL_TEXTENCODING_MS_1252 );
				aOptions.SetFieldSeps( (sal_Unicode) ',' );
				aOptions.SetTextSep( (sal_Unicode) '"' );
			}

			FltError eError = eERR_OK;
			sal_Bool bOverflow = sal_False;

			if( ! rMedium.IsStorage() )
			{
				ScImportExport	aImpEx( &aDocument );
				aImpEx.SetExtOptions( aOptions );

				SvStream* pInStream = rMedium.GetInStream();
				if (pInStream)
				{
					pInStream->SetStreamCharSet( aOptions.GetCharSet() );
					pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL() );
					eError = bRet ? eERR_OK : SCERR_IMPORT_CONNECT;
					aDocument.StartAllListeners();
					aDocument.SetDirty();
					bOverflow = aImpEx.IsOverflow();
				}
				else
				{
					DBG_ERROR( "No Stream" );
				}
			}

			if (eError != eERR_OK)
			{
				if (!GetError())
					SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
			}
			else if ( bOverflow )
			{
				if (!GetError())
					SetError(SCWARN_IMPORT_RANGE_OVERFLOW, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
			}
			bSetColWidths = sal_True;
			bSetSimpleTextColWidths = sal_True;
		}
		else if (aFltName.EqualsAscii(pFilterDBase))
		{
			String sItStr;
			SfxItemSet*	 pSet = rMedium.GetItemSet();
			const SfxPoolItem* pItem;
			if ( pSet && SFX_ITEM_SET ==
				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
			{
				sItStr = ((const SfxStringItem*)pItem)->GetValue();
			}

			if (sItStr.Len() == 0)
			{
				//	default for dBase import (from API without options):
				//	IBM_850 encoding

				sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
			}

			sal_uLong eError = DBaseImport( rMedium.GetPhysicalName(),
					ScGlobal::GetCharsetValue(sItStr), bSimpleColWidth );

			if (eError != eERR_OK)
			{
				if (!GetError())
					SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
				bRet = ( eError == SCWARN_IMPORT_RANGE_OVERFLOW );
			}
			else
				bRet = sal_True;

			aColWidthRange.aStart.SetRow( 1 );	// Spaltenheader nicht
			bSetColWidths = sal_True;
			bSetSimpleTextColWidths = sal_True;
			// Memo-Felder fuehren zu einem bSimpleColWidth[nCol]==FALSE
			for ( SCCOL nCol=0; nCol <= MAXCOL && !bSetRowHeights; nCol++ )
			{
				if ( !bSimpleColWidth[nCol] )
					bSetRowHeights = sal_True;
			}
		}
		else if (aFltName.EqualsAscii(pFilterDif))
		{
			SvStream* pStream = rMedium.GetInStream();
			if (pStream)
			{
				FltError eError;
				String sItStr;
				SfxItemSet*	 pSet = rMedium.GetItemSet();
				const SfxPoolItem* pItem;
				if ( pSet && SFX_ITEM_SET ==
					 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
				{
					sItStr = ((const SfxStringItem*)pItem)->GetValue();
				}

				if (sItStr.Len() == 0)
				{
					//	default for DIF import (from API without options):
					//	ISO8859-1/MS_1252 encoding

					sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
				}

				eError = ScFormatFilter::Get().ScImportDif( *pStream, &aDocument, ScAddress(0,0,0),
									ScGlobal::GetCharsetValue(sItStr));
				if (eError != eERR_OK)
				{
					if (!GetError())
						SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

					if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
						bRet = sal_True;
				}
				else
					bRet = sal_True;
			}
			bSetColWidths = sal_True;
			bSetSimpleTextColWidths = sal_True;
			bSetRowHeights = sal_True;
		}
		else if (aFltName.EqualsAscii(pFilterSylk))
		{
			FltError eError = SCERR_IMPORT_UNKNOWN;
			if( !rMedium.IsStorage() )
			{
				ScImportExport aImpEx( &aDocument );

				SvStream* pInStream = rMedium.GetInStream();
				if (pInStream)
				{
					pInStream->Seek( 0 );
                    bRet = aImpEx.ImportStream( *pInStream, rMedium.GetBaseURL(), SOT_FORMATSTR_ID_SYLK );
					eError = bRet ? eERR_OK : SCERR_IMPORT_UNKNOWN;
					aDocument.StartAllListeners();
					aDocument.SetDirty();
				}
				else
				{
					DBG_ERROR( "No Stream" );
				}
			}

			if ( eError != eERR_OK && !GetError() )
				SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
			bSetColWidths = sal_True;
			bSetSimpleTextColWidths = sal_True;
			bSetRowHeights = sal_True;
		}
		else if (aFltName.EqualsAscii(pFilterQPro6))
        {
            ScColumn::bDoubleAlloc = sal_True;
            FltError eError = ScFormatFilter::Get().ScImportQuattroPro( rMedium, &aDocument);
            ScColumn::bDoubleAlloc = sal_False;
            if (eError != eERR_OK)
            {
                if (!GetError())
                    SetError( eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
                    bRet = sal_True;
            }
            else
                bRet = sal_True;
            // TODO: Filter should set column widths. Not doing it here, it may
            // result in very narrow or wide columns, depending on content.
            // Setting row heights makes cells with font size attribution or
            // wrapping enabled look nicer..
            bSetRowHeights = sal_True;
        }
		else if (aFltName.EqualsAscii(pFilterRtf))
		{
			FltError eError = SCERR_IMPORT_UNKNOWN;
			if( !rMedium.IsStorage() )
			{
				SvStream* pInStream = rMedium.GetInStream();
				if (pInStream)
				{
					pInStream->Seek( 0 );
					ScRange aRange;
                    eError = ScFormatFilter::Get().ScImportRTF( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange );
					if (eError != eERR_OK)
					{
						if (!GetError())
							SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

						if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
							bRet = sal_True;
					}
					else
						bRet = sal_True;
					aDocument.StartAllListeners();
					aDocument.SetDirty();
					bSetColWidths = sal_True;
					bSetRowHeights = sal_True;
				}
				else
				{
					DBG_ERROR( "No Stream" );
				}
			}

			if ( eError != eERR_OK && !GetError() )
				SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
		}
		else if (aFltName.EqualsAscii(pFilterHtml) || aFltName.EqualsAscii(pFilterHtmlWebQ))
		{
			FltError eError = SCERR_IMPORT_UNKNOWN;
			sal_Bool bWebQuery = aFltName.EqualsAscii(pFilterHtmlWebQ);
			if( !rMedium.IsStorage() )
			{
				SvStream* pInStream = rMedium.GetInStream();
				if (pInStream)
				{
                    LanguageType eLang = LANGUAGE_SYSTEM;
                    bool bDateConvert = false;
                    SfxItemSet*	 pSet = rMedium.GetItemSet();
                    const SfxPoolItem* pItem;
                    if ( pSet && SFX_ITEM_SET ==
                         pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
                    {
                        String aFilterOption = (static_cast<const SfxStringItem*>(pItem))->GetValue();
                        lcl_parseHtmlFilterOption(aFilterOption, eLang, bDateConvert);
                    }

					pInStream->Seek( 0 );
					ScRange aRange;
					// HTML macht eigenes ColWidth/RowHeight
					CalcOutputFactor();
                    SvNumberFormatter aNumFormatter(aDocument.GetServiceManager(), eLang);
                    eError = ScFormatFilter::Get().ScImportHTML( *pInStream, rMedium.GetBaseURL(), &aDocument, aRange,
											GetOutputFactor(), !bWebQuery, &aNumFormatter, bDateConvert );
					if (eError != eERR_OK)
					{
						if (!GetError())
							SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));

						if( ( eError & ERRCODE_WARNING_MASK ) == ERRCODE_WARNING_MASK )
							bRet = sal_True;
					}
					else
						bRet = sal_True;
					aDocument.StartAllListeners();
					aDocument.SetDirty();
				}
				else
				{
					DBG_ERROR( "No Stream" );
				}
			}

			if ( eError != eERR_OK && !GetError() )
				SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
		}
		else
		{
			if (!GetError())
				SetError(SCERR_IMPORT_NI, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
		}

		if (!bCalc3)
			aDocument.SetInsertingFromOtherDoc( sal_False );
	}
	else
	{
		DBG_ERROR("Kein Filter bei ConvertFrom");
	}

	InitItems();
	CalcOutputFactor();
	if ( bRet && (bSetColWidths || bSetRowHeights) )
	{	// Spaltenbreiten/Zeilenhoehen anpassen, Basis 100% Zoom
		Fraction aZoom( 1, 1 );
		double nPPTX = ScGlobal::nScreenPPTX * (double) aZoom
			/ GetOutputFactor();	// Faktor ist Drucker zu Bildschirm
		double nPPTY = ScGlobal::nScreenPPTY * (double) aZoom;
		VirtualDevice aVirtDev;
		//	all sheets (for Excel import)
		SCTAB nTabCount = aDocument.GetTableCount();
		for (SCTAB nTab=0; nTab<nTabCount; nTab++)
		{
            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( nTab, nEndCol, nEndRow );
			aColWidthRange.aEnd.SetCol( nEndCol );
			aColWidthRange.aEnd.SetRow( nEndRow );
			ScMarkData aMark;
			aMark.SetMarkArea( aColWidthRange );
			aMark.MarkToMulti();
			// Reihenfolge erst Breite dann Hoehe ist wichtig (vergl. hund.rtf)
			if ( bSetColWidths )
			{
				for ( SCCOL nCol=0; nCol <= nEndCol; nCol++ )
				{
					sal_uInt16 nWidth = aDocument.GetOptimalColWidth(
						nCol, nTab, &aVirtDev, nPPTX, nPPTY, aZoom, aZoom, sal_False, &aMark,
						(bSetSimpleTextColWidths && bSimpleColWidth[nCol]) );
					aDocument.SetColWidth( nCol, nTab,
						nWidth + (sal_uInt16)ScGlobal::nLastColWidthExtra );
				}
			}
//			if ( bSetRowHeights )
//			{
//				//	nExtra must be 0
//				aDocument.SetOptimalHeight(	0, nEndRow, nTab, 0, &aVirtDev,
//					nPPTX, nPPTY, aZoom, aZoom, sal_False );
//			}
		}
		if ( bSetRowHeights )
			UpdateAllRowHeights();		// with vdev or printer, depending on configuration
	}
	FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );

	GetUndoManager()->Clear();
	// #73762# invalidate eventually temporary table areas
	if ( bRet )
		aDocument.InvalidateTableArea();

	bIsEmpty = sal_False;

	return bRet;
}


ScDocShell::PrepareSaveGuard::PrepareSaveGuard( ScDocShell& rDocShell )
    : mrDocShell( rDocShell)
{
    // DoEnterHandler not here (because of AutoSave), is in ExecuteSave.

    ScChartListenerCollection* pCharts = mrDocShell.aDocument.GetChartListenerCollection();
    if (pCharts)
        pCharts->UpdateDirtyCharts();                           // Charts to be updated.
    mrDocShell.aDocument.StopTemporaryChartLock();
    if (mrDocShell.pAutoStyleList)
        mrDocShell.pAutoStyleList->ExecuteAllNow();             // Execute template timeouts now.
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            pRefMgr->setAllCacheTableReferencedStati( false);
            mrDocShell.aDocument.MarkUsedExternalReferences();  // Mark tables of external references to be written.
        }
    }
    if (mrDocShell.GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        mrDocShell.SfxObjectShell::SetVisArea( Rectangle() );   // "Normally" worked on => no VisArea.
}

ScDocShell::PrepareSaveGuard::~PrepareSaveGuard()
{
    if (mrDocShell.aDocument.HasExternalRefManager())
    {
        ScExternalRefManager* pRefMgr = mrDocShell.aDocument.GetExternalRefManager();
        if (pRefMgr && pRefMgr->hasExternalData())
        {
            // Prevent accidental data loss due to lack of knowledge.
            pRefMgr->setAllCacheTableReferencedStati( true);
        }
    }
}


sal_Bool __EXPORT ScDocShell::Save()
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::Save" );

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

	//	wait cursor is handled with progress bar
    sal_Bool bRet = SfxObjectShell::Save();
	if( bRet )
        bRet = SaveXML( GetMedium(), NULL );
	return bRet;
}


sal_Bool __EXPORT ScDocShell::SaveAs( SfxMedium& rMedium )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::SaveAs" );

#if ENABLE_SHEET_PROTECTION
    ScTabViewShell* pViewShell = GetBestViewShell();
    if (pViewShell && ScPassHashHelper::needsPassHashRegen(aDocument, PASSHASH_OOO))
    {
        if (!pViewShell->ExecuteRetypePassDlg(PASSHASH_OOO))
            // password re-type cancelled.  Don't save the document.
            return false;
    }
#endif

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    PrepareSaveGuard aPrepareGuard( *this);

	aDocument.setDocAccTitle(String());
	// SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this, TYPE(SfxTopViewFrame));
	SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
	if (pFrame1)
	{
		Window* pWindow = &pFrame1->GetWindow();
		if ( pWindow )
		{
			Window* pSysWin = pWindow->GetSystemWindow();
			if ( pSysWin )
			{
				pSysWin->SetAccessibleName(String());
			}
		}
	}
	//	wait cursor is handled with progress bar
    sal_Bool bRet = SfxObjectShell::SaveAs( rMedium );
	if( bRet )
        bRet = SaveXML( &rMedium, NULL );

	return bRet;
}


sal_Bool __EXPORT ScDocShell::IsInformationLost()
{
/*
	const SfxFilter *pFilt = GetMedium()->GetFilter();
	sal_Bool bRet = pFilt && pFilt->IsAlienFormat() && bNoInformLost;
	if (bNoInformLost)					// nur einmal!!
		bNoInformLost = sal_False;
	return bRet;
*/
	//!!! bei Gelegenheit ein korrekte eigene Behandlung einbauen

	return SfxObjectShell::IsInformationLost();
}


// Xcl-like column width measured in characters of standard font.
xub_StrLen lcl_ScDocShell_GetColWidthInChars( sal_uInt16 nWidth )
{
    // double fColScale = 1.0;
	double	f = nWidth;
	f *= 1328.0 / 25.0;
	f += 90.0;
	f *= 1.0 / 23.0;
	// f /= fColScale * 256.0;
	f /= 256.0;

	return xub_StrLen( f );
}


void lcl_ScDocShell_GetFixedWidthString( String& rStr, const ScDocument& rDoc,
        SCTAB nTab, SCCOL nCol, sal_Bool bValue, SvxCellHorJustify eHorJust )
{
    xub_StrLen nLen = lcl_ScDocShell_GetColWidthInChars(
            rDoc.GetColWidth( nCol, nTab ) );
    if ( nLen < rStr.Len() )
    {
        if ( bValue )
            rStr.AssignAscii( "###" );
        rStr.Erase( nLen );
    }
    if ( nLen > rStr.Len() )
    {
        if ( bValue && eHorJust == SVX_HOR_JUSTIFY_STANDARD )
            eHorJust = SVX_HOR_JUSTIFY_RIGHT;
        switch ( eHorJust )
        {
            case SVX_HOR_JUSTIFY_RIGHT:
            {
                String aTmp;
                aTmp.Fill( nLen - rStr.Len() );
                rStr.Insert( aTmp, 0 );
            }
            break;
            case SVX_HOR_JUSTIFY_CENTER:
            {
                xub_StrLen nLen2 = (nLen - rStr.Len()) / 2;
                String aTmp;
                aTmp.Fill( nLen2 );
                rStr.Insert( aTmp, 0 );
                rStr.Expand( nLen );
            }
            break;
            default:
                rStr.Expand( nLen );
        }
    }
}


void lcl_ScDocShell_WriteEmptyFixedWidthString( SvStream& rStream,
        const ScDocument& rDoc, SCTAB nTab, SCCOL nCol )
{
    String aString;
    lcl_ScDocShell_GetFixedWidthString( aString, rDoc, nTab, nCol, sal_False,
            SVX_HOR_JUSTIFY_STANDARD );
    rStream.WriteUnicodeOrByteText( aString );
}


void ScDocShell::AsciiSave( SvStream& rStream, const ScImportOptions& rAsciiOpt )
{
    sal_Unicode cDelim    = rAsciiOpt.nFieldSepCode;
    sal_Unicode cStrDelim = rAsciiOpt.nTextSepCode;
    CharSet eCharSet      = rAsciiOpt.eCharSet;
    sal_Bool bFixedWidth      = rAsciiOpt.bFixedWidth;
    sal_Bool bSaveAsShown     = rAsciiOpt.bSaveAsShown;

	CharSet eOldCharSet = rStream.GetStreamCharSet();
	rStream.SetStreamCharSet( eCharSet );
	sal_uInt16 nOldNumberFormatInt = rStream.GetNumberFormatInt();
    ByteString aStrDelimEncoded;    // only used if not Unicode
    UniString aStrDelimDecoded;     // only used if context encoding
    ByteString aDelimEncoded;
    UniString aDelimDecoded;
    sal_Bool bContextOrNotAsciiEncoding;
	if ( eCharSet == RTL_TEXTENCODING_UNICODE )
    {
		rStream.StartWritingUnicodeText();
        bContextOrNotAsciiEncoding = sal_False;
    }
    else
    {
        aStrDelimEncoded = ByteString( cStrDelim, eCharSet );
        aDelimEncoded = ByteString( cDelim, eCharSet );
        rtl_TextEncodingInfo aInfo;
        aInfo.StructSize = sizeof(aInfo);
        if ( rtl_getTextEncodingInfo( eCharSet, &aInfo ) )
        {
            bContextOrNotAsciiEncoding =
                (((aInfo.Flags & RTL_TEXTENCODING_INFO_CONTEXT) != 0) ||
                 ((aInfo.Flags & RTL_TEXTENCODING_INFO_ASCII) == 0));
            if ( bContextOrNotAsciiEncoding )
            {
                aStrDelimDecoded = String( aStrDelimEncoded, eCharSet );
                aDelimDecoded = String( aDelimEncoded, eCharSet );
            }
        }
        else
            bContextOrNotAsciiEncoding = sal_False;
    }

	SCCOL nStartCol = 0;
	SCROW nStartRow = 0;
	SCTAB nTab = GetSaveTab();
	SCCOL nEndCol;
	SCROW nEndRow;
	aDocument.GetCellArea( nTab, nEndCol, nEndRow );

	ScProgress aProgress( this, ScGlobal::GetRscString( STR_SAVE_DOC ), nEndRow );

	String aString;

	ScTabViewShell*	pViewSh = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
	const ScViewOptions& rOpt = (pViewSh)
								? pViewSh->GetViewData()->GetOptions()
								: aDocument.GetViewOptions();
	sal_Bool bShowFormulas = rOpt.GetOption( VOPT_FORMULAS );
	sal_Bool bTabProtect = aDocument.IsTabProtected( nTab );

	SCCOL nCol;
	SCROW nRow;
	SCCOL nNextCol = nStartCol;
	SCROW nNextRow = nStartRow;
	SCCOL nEmptyCol;
	SCROW nEmptyRow;
	SvNumberFormatter& rFormatter = *aDocument.GetFormatTable();

	ScHorizontalCellIterator aIter( &aDocument, nTab, nStartCol, nStartRow,
		nEndCol, nEndRow );
	ScBaseCell* pCell;
    while ( ( pCell = aIter.GetNext( nCol, nRow ) ) != NULL )
    {
        sal_Bool bProgress = sal_False;		// only upon line change
        if ( nNextRow < nRow )
        {   // empty rows or/and empty columns up to end of row
            bProgress = sal_True;
            for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
            {   // remaining columns of last row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            endlub( rStream );
            nNextRow++;
            for ( nEmptyRow = nNextRow; nEmptyRow < nRow; nEmptyRow++ )
            {   // completely empty rows
                for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
                {
                    if ( bFixedWidth )
                        lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                                aDocument, nTab, nEmptyCol );
                    else if ( cDelim != 0 )
                        rStream.WriteUniOrByteChar( cDelim );
                }
                endlub( rStream );
            }
            for ( nEmptyCol = nStartCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // empty columns at beginning of row
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
            nNextRow = nRow;
        }
        else if ( nNextCol < nCol )
        {   // empty columns in same row
            for ( nEmptyCol = nNextCol; nEmptyCol < nCol; nEmptyCol++ )
            {   // columns in between
                if ( bFixedWidth )
                    lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                            aDocument, nTab, nEmptyCol );
                else if ( cDelim != 0 )
                    rStream.WriteUniOrByteChar( cDelim );
            }
        }
        if ( nCol == nEndCol )
        {
            bProgress = sal_True;
            nNextCol = nStartCol;
            nNextRow = nRow + 1;
        }
        else
            nNextCol = nCol + 1;

        CellType eType = pCell->GetCellType();
        if ( bTabProtect )
        {
            const ScProtectionAttr* pProtAttr =
                (const ScProtectionAttr*) aDocument.GetAttr(
                                                            nCol, nRow, nTab, ATTR_PROTECTION );
            if ( pProtAttr->GetHideCell() ||
                    ( eType == CELLTYPE_FORMULA && bShowFormulas &&
                      pProtAttr->GetHideFormula() ) )
                eType = CELLTYPE_NONE;	// hide
        }
        sal_Bool bString;
        switch ( eType )
        {
            case CELLTYPE_NOTE:
            case CELLTYPE_NONE:
                aString.Erase();
                bString = sal_False;
                break;
            case CELLTYPE_FORMULA :
                {
                    sal_uInt16 nErrCode;
                    if ( bShowFormulas )
                    {
                        ((ScFormulaCell*)pCell)->GetFormula( aString );
                        bString = sal_True;
                    }
                    else if ( ( nErrCode = ((ScFormulaCell*)pCell)->GetErrCode() ) != 0 )
                    {
                        aString = ScGlobal::GetErrorString( nErrCode );
                        bString = sal_True;
                    }
                    else if ( ((ScFormulaCell*)pCell)->IsValue() )
                    {
                        sal_uInt32 nFormat;
                        aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                        if ( bFixedWidth || bSaveAsShown )
                        {
                            Color* pDummy;
                            ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                            bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                        }
                        else
                        {
                            ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                            bString = sal_False;
                        }
                    }
                    else
                    {
                        if ( bSaveAsShown )
                        {
                            sal_uInt32 nFormat;
                            aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                            Color* pDummy;
                            ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                        }
                        else
                            ((ScFormulaCell*)pCell)->GetString( aString );
                        bString = sal_True;
                    }
                }
                break;
            case CELLTYPE_STRING :
                if ( bSaveAsShown )
                {
                    sal_uInt32 nFormat;
                    aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                    Color* pDummy;
                    ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                }
                else
                    ((ScStringCell*)pCell)->GetString( aString );
                bString = sal_True;
                break;
            case CELLTYPE_EDIT :
                {
                    const EditTextObject* pObj;
                    static_cast<const ScEditCell*>(pCell)->GetData( pObj);
                    EditEngine& rEngine = aDocument.GetEditEngine();
                    rEngine.SetText( *pObj);
                    aString = rEngine.GetText();  // including LF
                    bString = sal_True;
                }
                break;
            case CELLTYPE_VALUE :
                {
                    sal_uInt32 nFormat;
                    aDocument.GetNumberFormat( nCol, nRow, nTab, nFormat );
                    if ( bFixedWidth || bSaveAsShown )
                    {
                        Color* pDummy;
                        ScCellFormat::GetString( pCell, nFormat, aString, &pDummy, rFormatter );
                        bString = bSaveAsShown && rFormatter.IsTextFormat( nFormat);
                    }
                    else
                    {
                        ScCellFormat::GetInputString( pCell, nFormat, aString, rFormatter );
                        bString = sal_False;
                    }
                }
                break;
            default:
                DBG_ERROR( "ScDocShell::AsciiSave: unknown CellType" );
                aString.Erase();
                bString = sal_False;
        }

        if ( bFixedWidth )
        {
            SvxCellHorJustify eHorJust = (SvxCellHorJustify)
                ((const SvxHorJustifyItem*) aDocument.GetAttr( nCol, nRow,
                nTab, ATTR_HOR_JUSTIFY ))->GetValue();
            lcl_ScDocShell_GetFixedWidthString( aString, aDocument, nTab, nCol,
                    !bString, eHorJust );
            rStream.WriteUnicodeOrByteText( aString );
        }
        else
        {
            if (!bString && cStrDelim != 0 && aString.Len() > 0)
            {
                sal_Unicode c = aString.GetChar(0);
                bString = (c == cStrDelim || c == ' ' ||
                        aString.GetChar( aString.Len()-1) == ' ' ||
                        aString.Search( cStrDelim) != STRING_NOTFOUND);
                if (!bString && cDelim != 0)
                    bString = (aString.Search( cDelim) != STRING_NOTFOUND);
            }
            if ( bString )
            {
                if ( cStrDelim != 0 ) //@ BugId 55355
                {
                    if ( eCharSet == RTL_TEXTENCODING_UNICODE )
                    {
                        xub_StrLen nPos = aString.Search( cStrDelim );
                        // #i116636# quotes are needed if text delimiter (quote), field delimiter, or LF is in the cell text
                        bool bNeedQuotes = rAsciiOpt.bQuoteAllText ||
                                            ( nPos != STRING_NOTFOUND ) ||
                                            ( aString.Search( cDelim ) != STRING_NOTFOUND ) ||
                                            ( aString.Search( sal_Unicode(_LF) ) != STRING_NOTFOUND );
                        while ( nPos != STRING_NOTFOUND )
                        {
                            aString.Insert( cStrDelim, nPos );
                            nPos = aString.Search( cStrDelim, nPos+2 );
                        }
                        if ( bNeedQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        rStream.WriteUnicodeText( aString );
                        if ( bNeedQuotes )
                            rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                    }
                    else
                    {
                        // #105549# This is nasty. The Unicode to byte encoding
                        // may convert typographical quotation marks to ASCII
                        // quotation marks, which may interfer with the delimiter,
                        // so we have to escape delimiters after the string has
                        // been encoded. Since this may happen also with UTF-8
                        // encoded typographical quotation marks if such was
                        // specified as a delimiter we have to check for the full
                        // encoded delimiter string, not just one character.
                        // Now for RTL_TEXTENCODING_ISO_2022_... and similar brain
                        // dead encodings where one code point (and especially a
                        // low ASCII value) may represent different characters, we
                        // have to convert forth and back and forth again. Same for
                        // UTF-7 since it is a context sensitive encoding too.

                        if ( bContextOrNotAsciiEncoding )
                        {
                            // to byte encoding
                            ByteString aStrEnc( aString, eCharSet );
                            // back to Unicode
                            UniString aStrDec( aStrEnc, eCharSet );
                            // search on re-decoded string
                            xub_StrLen nPos = aStrDec.Search( aStrDelimDecoded );
                            bool bNeedQuotes = rAsciiOpt.bQuoteAllText ||
                                                ( nPos != STRING_NOTFOUND ) ||
                                                ( aStrDec.Search( aDelimDecoded ) != STRING_NOTFOUND ) ||
                                                ( aStrDec.Search( sal_Unicode(_LF) ) != STRING_NOTFOUND );
                            while ( nPos != STRING_NOTFOUND )
                            {
                                aStrDec.Insert( aStrDelimDecoded, nPos );
                                nPos = aStrDec.Search( aStrDelimDecoded,
                                        nPos+1+aStrDelimDecoded.Len() );
                            }
                            // write byte re-encoded
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                            rStream.WriteUnicodeOrByteText( aStrDec, eCharSet );
                            if ( bNeedQuotes )
                                rStream.WriteUniOrByteChar( cStrDelim, eCharSet );
                        }
                        else
                        {
                            ByteString aStrEnc( aString, eCharSet );
                            // search on encoded string
                            xub_StrLen nPos = aStrEnc.Search( aStrDelimEncoded );
                            bool bNeedQuotes = rAsciiOpt.bQuoteAllText ||
                                                ( nPos != STRING_NOTFOUND ) ||
                                                ( aStrEnc.Search( aDelimEncoded ) != STRING_NOTFOUND ) ||
                                                ( aStrEnc.Search( sal_Char(_LF) ) != STRING_NOTFOUND );
                            while ( nPos != STRING_NOTFOUND )
                            {
                                aStrEnc.Insert( aStrDelimEncoded, nPos );
                                nPos = aStrEnc.Search( aStrDelimEncoded,
                                        nPos+1+aStrDelimEncoded.Len() );
                            }
                            // write byte encoded
                            if ( bNeedQuotes )
                                rStream.Write( aStrDelimEncoded.GetBuffer(),
                                        aStrDelimEncoded.Len() );
                            rStream.Write( aStrEnc.GetBuffer(), aStrEnc.Len() );
                            if ( bNeedQuotes )
                                rStream.Write( aStrDelimEncoded.GetBuffer(),
                                        aStrDelimEncoded.Len() );
                        }
                    }
                }
                else
                    rStream.WriteUnicodeOrByteText( aString );
            }
            else
                rStream.WriteUnicodeOrByteText( aString );
        }

        if( nCol < nEndCol )
        {
            if(cDelim!=0) //@ BugId 55355
                rStream.WriteUniOrByteChar( cDelim );
        }
        else
            endlub( rStream );

        if ( bProgress )
            aProgress.SetStateOnPercent( nRow );
    }

	// write out empty if requested
	if ( nNextRow <= nEndRow )
	{
        for ( nEmptyCol = nNextCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {	// remaining empty columns of last row
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
		endlub( rStream );
		nNextRow++;
	}
	for ( nEmptyRow = nNextRow; nEmptyRow <= nEndRow; nEmptyRow++ )
	{	// entire empty rows
        for ( nEmptyCol = nStartCol; nEmptyCol < nEndCol; nEmptyCol++ )
        {
            if ( bFixedWidth )
                lcl_ScDocShell_WriteEmptyFixedWidthString( rStream,
                        aDocument, nTab, nEmptyCol );
            else if ( cDelim != 0 )
                rStream.WriteUniOrByteChar( cDelim );
        }
		endlub( rStream );
	}

	rStream.SetStreamCharSet( eOldCharSet );
	rStream.SetNumberFormatInt( nOldNumberFormatInt );
}

sal_Bool __EXPORT ScDocShell::ConvertTo( SfxMedium &rMed )
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ConvertTo" );

	ScRefreshTimerProtector( aDocument.GetRefreshTimerControlAddress() );

    //  #i6500# don't call DoEnterHandler here (doesn't work with AutoSave),
    //  it's already in ExecuteSave (as for Save and SaveAs)

	if (pAutoStyleList)
		pAutoStyleList->ExecuteAllNow();				// Vorlagen-Timeouts jetzt ausfuehren
	if (GetCreateMode()== SFX_CREATE_MODE_STANDARD)
        SfxObjectShell::SetVisArea( Rectangle() );     // normal bearbeitet -> keine VisArea

	DBG_ASSERT( rMed.GetFilter(), "Filter == 0" );

	sal_Bool bRet = sal_False;
	String aFltName = rMed.GetFilter()->GetFilterName();

/*
	if (aFltName.EqualsAscii(pFilterLotus))
	{
		SvStream* pStream = rMed.GetOutStream();
		if (pStream)
		{
			FltError eError = ScFormatFilter::Get().ScExportLotus123( *pStream, &aDocument, ExpWK1,
												CHARSET_IBMPC_437 );
			bRet = eError == eERR_OK;
		}
	}
	else
*/
    if (aFltName.EqualsAscii(pFilterXML))
	{
        //TODO/LATER: this shouldn't happen!
        DBG_ERROR("XML filter in ConvertFrom?!");
		bRet = SaveXML( &rMed, NULL );
	}
	else if (aFltName.EqualsAscii(pFilterExcel5) || aFltName.EqualsAscii(pFilterExcel95) ||
			 aFltName.EqualsAscii(pFilterExcel97) || aFltName.EqualsAscii(pFilterEx5Temp) ||
			 aFltName.EqualsAscii(pFilterEx95Temp) || aFltName.EqualsAscii(pFilterEx97Temp) ||
			 aFltName.EqualsAscii(pFilterEx07Xml))
	{
		WaitObject aWait( GetActiveDialogParent() );

        bool bDoSave = true;
        if( ScTabViewShell* pViewShell = GetBestViewShell() )
        {
            ScExtDocOptions* pExtDocOpt = aDocument.GetExtDocOptions();
            if( !pExtDocOpt )
                aDocument.SetExtDocOptions( pExtDocOpt = new ScExtDocOptions );
            pViewShell->GetViewData()->WriteExtOptions( *pExtDocOpt );

            /*  #115980# #i104990# If the imported document contains a medium
                password, determine if we can save it, otherwise ask the users
                whether they want to save without it. */
            if( (rMed.GetFilter()->GetFilterFlags() & SFX_FILTER_ENCRYPTION) == 0 )
            {
                SfxItemSet* pItemSet = rMed.GetItemSet();
                const SfxPoolItem* pItem = 0;
                if( pItemSet && pItemSet->GetItemState( SID_PASSWORD, sal_True, &pItem ) == SFX_ITEM_SET )
                {
                    bDoSave = ScWarnPassword::WarningOnPassword( rMed );
                    // #i42858# remove password from medium (warn only one time)
                    if( bDoSave )
                        pItemSet->ClearItem( SID_PASSWORD );
                }
            }

#if ENABLE_SHEET_PROTECTION
            if( bDoSave )
            {
                bool bNeedRetypePassDlg = ScPassHashHelper::needsPassHashRegen( aDocument, PASSHASH_XL );
                bDoSave = !bNeedRetypePassDlg || pViewShell->ExecuteRetypePassDlg( PASSHASH_XL );
            }
#endif
        }

        if( bDoSave )
        {
            ExportFormatExcel eFormat = ExpBiff5;
            if( aFltName.EqualsAscii( pFilterExcel97 ) || aFltName.EqualsAscii( pFilterEx97Temp ) )
                eFormat = ExpBiff8;
            if( aFltName.EqualsAscii( pFilterEx07Xml ) )
                eFormat = Exp2007Xml;
            FltError eError = ScFormatFilter::Get().ScExportExcel5( rMed, &aDocument, eFormat, RTL_TEXTENCODING_MS_1252 );

            if( eError && !GetError() )
                SetError( eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

            // don't return false for warnings
            bRet = ((eError & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK) || (eError == eERR_OK);
        }
        else
        {
            // export aborted, i.e. "Save without password" warning
            SetError( ERRCODE_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        }
    }
	else if (aFltName.EqualsAscii(pFilterAscii))
	{
		SvStream* pStream = rMed.GetOutStream();
		if (pStream)
		{
			String sItStr;
			SfxItemSet*	 pSet = rMed.GetItemSet();
			const SfxPoolItem* pItem;
			if ( pSet && SFX_ITEM_SET ==
				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
			{
				sItStr = ((const SfxStringItem*)pItem)->GetValue();
			}

			if ( sItStr.Len() == 0 )
			{
				//	default for ascii export (from API without options):
				//	ISO8859-1/MS_1252 encoding, comma, double quotes

				ScImportOptions aDefOptions( ',', '"', RTL_TEXTENCODING_MS_1252 );
				sItStr = aDefOptions.BuildString();
			}

			WaitObject aWait( GetActiveDialogParent() );
			ScImportOptions aOptions( sItStr );
			AsciiSave( *pStream, aOptions );
			bRet = sal_True;

			if (aDocument.GetTableCount() > 1)
				if (!rMed.GetError())
					rMed.SetError(SCWARN_EXPORT_ASCII, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
		}
	}
	else if (aFltName.EqualsAscii(pFilterDBase))
	{
		String sCharSet;
		SfxItemSet*	pSet = rMed.GetItemSet();
		const SfxPoolItem* pItem;
		if ( pSet && SFX_ITEM_SET ==
			 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
		{
			sCharSet = ((const SfxStringItem*)pItem)->GetValue();
		}

		if (sCharSet.Len() == 0)
		{
			//	default for dBase export (from API without options):
			//	IBM_850 encoding

			sCharSet = ScGlobal::GetCharsetString( RTL_TEXTENCODING_IBM_850 );
		}

		WaitObject aWait( GetActiveDialogParent() );
// HACK damit Sba geoffnetes TempFile ueberschreiben kann
		rMed.CloseOutStream();
		sal_Bool bHasMemo = sal_False;

		sal_uLong eError = DBaseExport( rMed.GetPhysicalName(),
						ScGlobal::GetCharsetValue(sCharSet), bHasMemo );

		if ( eError != eERR_OK && (eError & ERRCODE_WARNING_MASK) )
		{
//!			if ( !rMed.GetError() )
//!				rMed.SetError( eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
			eError = eERR_OK;
		}
//!		else if ( aDocument.GetTableCount() > 1 && !rMed.GetError() )
//!			rMed.SetError( SCWARN_EXPORT_ASCII, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

		INetURLObject aTmpFile( rMed.GetPhysicalName(), INET_PROT_FILE );
		if ( bHasMemo )
			aTmpFile.setExtension( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("dbt")) );
		if ( eError != eERR_OK )
		{
			if (!GetError())
				SetError(eError, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
			if ( bHasMemo && IsDocument( aTmpFile ) )
				KillFile( aTmpFile );
		}
		else
		{
			bRet = sal_True;
			if ( bHasMemo )
			{
				SfxStringItem* pNameItem =
					(SfxStringItem*) rMed.GetItemSet()->GetItem( SID_FILE_NAME );
				INetURLObject aDbtFile( pNameItem->GetValue(), INET_PROT_FILE );
				aDbtFile.setExtension( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("dbt")) );
				if ( IsDocument( aDbtFile ) && !KillFile( aDbtFile ) )
					bRet = sal_False;
				if ( bRet && !MoveFile( aTmpFile, aDbtFile ) )
					bRet = sal_False;
				if ( !bRet )
				{
					KillFile( aTmpFile );
					if ( !GetError() )
						SetError( SCERR_EXPORT_DATA, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
				}
			}
		}
	}
	else if (aFltName.EqualsAscii(pFilterDif))
	{
		SvStream* pStream = rMed.GetOutStream();
		if (pStream)
		{
			String sItStr;
			SfxItemSet*	 pSet = rMed.GetItemSet();
			const SfxPoolItem* pItem;
			if ( pSet && SFX_ITEM_SET ==
				 pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
			{
				sItStr = ((const SfxStringItem*)pItem)->GetValue();
			}

			if (sItStr.Len() == 0)
			{
				//	default for DIF export (from API without options):
				//	ISO8859-1/MS_1252 encoding

				sItStr = ScGlobal::GetCharsetString( RTL_TEXTENCODING_MS_1252 );
			}

			WaitObject aWait( GetActiveDialogParent() );
			ScFormatFilter::Get().ScExportDif( *pStream, &aDocument, ScAddress(0,0,0),
				ScGlobal::GetCharsetValue(sItStr) );
			bRet = sal_True;

			if (aDocument.GetTableCount() > 1)
				if (!rMed.GetError())
					rMed.SetError(SCWARN_EXPORT_ASCII, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ));
		}
	}
	else if (aFltName.EqualsAscii(pFilterSylk))
	{
		SvStream* pStream = rMed.GetOutStream();
		if ( pStream )
		{
			WaitObject aWait( GetActiveDialogParent() );

            SCCOL nEndCol;
            SCROW nEndRow;
            aDocument.GetCellArea( 0, nEndCol, nEndRow );
			ScRange aRange( 0,0,0, nEndCol,nEndRow,0 );

			ScImportExport aImExport( &aDocument, aRange );
            aImExport.SetFormulas( sal_True );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_SYLK );
		}
	}
	else if (aFltName.EqualsAscii(pFilterHtml))
	{
		SvStream* pStream = rMed.GetOutStream();
		if ( pStream )
		{
            WaitObject aWait( GetActiveDialogParent() );
			ScImportExport aImExport( &aDocument );
			aImExport.SetStreamPath( rMed.GetName() );
            bRet = aImExport.ExportStream( *pStream, rMed.GetBaseURL( true ), SOT_FORMATSTR_ID_HTML );
            if ( bRet && aImExport.GetNonConvertibleChars().Len() )
                SetError( *new StringErrorInfo(
                    SCWARN_EXPORT_NONCONVERTIBLE_CHARS,
                    aImExport.GetNonConvertibleChars(),
                    ERRCODE_BUTTON_OK | ERRCODE_MSG_INFO ), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
		}
	}
	else
	{
		if (GetError())
			SetError(SCERR_IMPORT_NI, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
	}
	return bRet;
}


sal_Bool __EXPORT ScDocShell::SaveCompleted( const uno::Reference < embed::XStorage >& xStor )
{
    return SfxObjectShell::SaveCompleted( xStor );
}


sal_Bool __EXPORT ScDocShell::DoSaveCompleted( SfxMedium * pNewStor )
{
	sal_Bool bRet = SfxObjectShell::DoSaveCompleted( pNewStor );

	//	SC_HINT_DOC_SAVED fuer Wechsel ReadOnly -> Read/Write
	Broadcast( SfxSimpleHint( SC_HINT_DOC_SAVED ) );
	return bRet;
}


sal_Bool ScDocShell::QuerySlotExecutable( sal_uInt16 nSlotId )
{
    // #i112634# ask VBA event handlers whether to save or print the document
    
    using namespace ::com::sun::star::script::vba;

    sal_Int32 nVbaEventId = VBAEventId::NO_EVENT;
    uno::Sequence< uno::Any > aArgs;
    switch( nSlotId )
    {
        case SID_SAVEDOC:
        case SID_SAVEASDOC:
            nVbaEventId = VBAEventId::WORKBOOK_BEFORESAVE;
            aArgs.realloc( 1 );
            aArgs[ 0 ] <<= (nSlotId == SID_SAVEASDOC);
        break;
        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            nVbaEventId = VBAEventId::WORKBOOK_BEFOREPRINT;
        break;
    }

    sal_Bool bSlotExecutable = sal_True;
    if( nVbaEventId != VBAEventId::NO_EVENT ) try
    {
        uno::Reference< XVBAEventProcessor > xEventProcessor( aDocument.GetVbaEventProcessor(), uno::UNO_QUERY_THROW );
        xEventProcessor->processVbaEvent( nVbaEventId, aArgs );
    }
    catch( util::VetoException& )
    {
        bSlotExecutable = sal_False;
    }
    catch( uno::Exception& )
    {
    }
    return bSlotExecutable;
}


sal_uInt16 __EXPORT ScDocShell::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
	if(SC_MOD()->GetCurRefDlgId()>0)
	{
		SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
		if( pFrame )
		{
			SfxViewShell* p = pFrame->GetViewShell();
			ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
			if(pViewSh!=NULL)
			{
				Window *pWin=pViewSh->GetWindow();
				if(pWin!=NULL) pWin->GrabFocus();
			}
		}

		return sal_False;
	}
	if ( aDocument.IsInLinkUpdate() || aDocument.IsInInterpreter() )
	{
		ErrorMessage(STR_CLOSE_ERROR_LINK);
		return sal_False;
	}

	DoEnterHandler();

	// start 'Workbook_BeforeClose' VBA event handler for possible veto
    if( !IsInPrepareClose() )
    {
        try
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( aDocument.GetVbaEventProcessor(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_BEFORECLOSE, aArgs );
        }
        catch( util::VetoException& )
        {
            // if event processor throws VetoException, macro has vetoed close
		    return sal_False;
		}
        catch( uno::Exception& )
        {
        }
    }
	// end handler code

	sal_uInt16 nRet = SfxObjectShell::PrepareClose( bUI, bForBrowsing );
	if (nRet == sal_True)						// sal_True = schliessen
		aDocument.DisableIdle(sal_True);		// nicht mehr drin rumpfuschen !!!

	return nRet;
}

void ScDocShell::PrepareReload()
{
	SfxObjectShell::PrepareReload();	// tut nichts?

	//	Das Disconnect von DDE-Links kann Reschedule ausloesen.
	//	Wenn die DDE-Links erst im Dokument-dtor geloescht werden, kann beim Reload
	//	aus diesem Reschedule das DDE-Link-Update fuer das neue Dokument ausgeloest
	//	werden. Dabei verklemmt sicht dann irgendwas.
	//	-> Beim Reload die DDE-Links des alten Dokuments vorher disconnecten

	aDocument.DisconnectDdeLinks();
}


String ScDocShell::GetOwnFilterName()			// static
{
	return String::CreateFromAscii(pFilterSc50);
}

String ScDocShell::GetHtmlFilterName()
{
    return String::CreateFromAscii(pFilterHtml);
}

String ScDocShell::GetWebQueryFilterName()		// static
{
	return String::CreateFromAscii(pFilterHtmlWebQ);
}

String ScDocShell::GetAsciiFilterName()			// static
{
	return String::CreateFromAscii(pFilterAscii);
}

String ScDocShell::GetLotusFilterName()			// static
{
	return String::CreateFromAscii(pFilterLotus);
}

String ScDocShell::GetDBaseFilterName()			// static
{
	return String::CreateFromAscii(pFilterDBase);
}

String ScDocShell::GetDifFilterName()			// static
{
	return String::CreateFromAscii(pFilterDif);
}

sal_Bool ScDocShell::HasAutomaticTableName( const String& rFilter )		// static
{
	//	sal_True for those filters that keep the default table name
	//	(which is language specific)

	return rFilter.EqualsAscii( pFilterAscii )
		|| rFilter.EqualsAscii( pFilterLotus )
		|| rFilter.EqualsAscii( pFilterExcel4 )
		|| rFilter.EqualsAscii( pFilterEx4Temp )
		|| rFilter.EqualsAscii( pFilterDBase )
		|| rFilter.EqualsAscii( pFilterDif )
		|| rFilter.EqualsAscii( pFilterSylk )
		|| rFilter.EqualsAscii( pFilterHtml )
		|| rFilter.EqualsAscii( pFilterRtf );
}

//==================================================================

#define __SCDOCSHELL_INIT \
		aDocument		( SCDOCMODE_DOCUMENT, this ), \
        aDdeTextFmt(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("TEXT"))), \
		nPrtToScreenFactor( 1.0 ), \
        pImpl           ( new DocShell_Impl ), \
		bHeaderOn		( sal_True ), \
		bFooterOn		( sal_True ), \
        bNoInformLost   ( sal_True ), \
		bIsEmpty		( sal_True ), \
		bIsInUndo		( sal_False ), \
		bDocumentModifiedPending( sal_False ), \
		nDocumentLock	( 0 ), \
        nCanUpdate (com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG), \
        bUpdateEnabled  ( sal_True ), \
        pOldAutoDBRange ( NULL ), \
		pDocHelper 		( NULL ), \
		pAutoStyleList	( NULL ), \
		pPaintLockData	( NULL ), \
		pOldJobSetup	( NULL ), \
        pSolverSaveData ( NULL ), \
        pSheetSaveData  ( NULL ), \
        pModificator    ( NULL )

//------------------------------------------------------------------

ScDocShell::ScDocShell( const ScDocShell& rShell )
    :   SvRefBase(),
        SotObject(),
	    SfxObjectShell( rShell.GetCreateMode() ),
        SfxListener(),
		__SCDOCSHELL_INIT
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

	SetPool( &SC_MOD()->GetPool() );

	bIsInplace = rShell.bIsInplace;

	pDocFunc = new ScDocFunc(*this);

	//	SetBaseModel needs exception handling
	ScModelObj::CreateAndSet( this );

	StartListening(*this);
	SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
	if (pStlPool)
		StartListening(*pStlPool);

	GetPageOnFromPageStyleSet( NULL, 0, bHeaderOn, bFooterOn );
	SetHelpId( HID_SCSHELL_DOCSH );

	//	InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

ScDocShell::ScDocShell( const sal_uInt64 i_nSfxCreationFlags )
	:	SfxObjectShell( i_nSfxCreationFlags )
    ,   __SCDOCSHELL_INIT
{
	RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::ScDocShell" );

	SetPool( &SC_MOD()->GetPool() );

	bIsInplace = (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
	//	wird zurueckgesetzt, wenn nicht inplace

    // #118840# set flag at ScDocument that it is used temporary (e.g. inplace 
    // for transporting a chart over the clipboard)
    if(bIsInplace)
    {
        aDocument.mbIsTemporary = true;
    }

	pDocFunc = new ScDocFunc(*this);

	//	SetBaseModel needs exception handling
	ScModelObj::CreateAndSet( this );

	StartListening(*this);
	SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
	if (pStlPool)
		StartListening(*pStlPool);
	SetHelpId( HID_SCSHELL_DOCSH );

	aDocument.GetDBCollection()->SetRefreshHandler(
		LINK( this, ScDocShell, RefreshDBDataHdl ) );

	//	InitItems und CalcOutputFactor werden jetzt nach bei Load/ConvertFrom/InitNew gerufen
}

//------------------------------------------------------------------

__EXPORT ScDocShell::~ScDocShell()
{
	ResetDrawObjectShell();	// #55570# falls der Drawing-Layer noch versucht, darauf zuzugreifen

	SfxStyleSheetPool* pStlPool = aDocument.GetStyleSheetPool();
	if (pStlPool)
		EndListening(*pStlPool);
	EndListening(*this);

	delete pAutoStyleList;

	SfxApplication *pSfxApp = SFX_APP();
	if ( pSfxApp->GetDdeService() )				// DDE vor Dokument loeschen
		pSfxApp->RemoveDdeTopic( this );

	delete pDocFunc;
	delete aDocument.mpUndoManager;
	aDocument.mpUndoManager = 0;
    delete pImpl;

	delete pPaintLockData;

	delete pOldJobSetup;		// gesetzt nur bei Fehler in StartJob()

    delete pSolverSaveData;
    delete pSheetSaveData;
    delete pOldAutoDBRange;

    if (pModificator)
    {
        DBG_ERROR("The Modificator should not exist");
        delete pModificator;
    }
}

//------------------------------------------------------------------

::svl::IUndoManager* __EXPORT ScDocShell::GetUndoManager()
{
	return aDocument.GetUndoManager();
}

void ScDocShell::SetModified( sal_Bool bModified )
{
    if ( SfxObjectShell::IsEnableSetModified() )
	{
    	SfxObjectShell::SetModified( bModified );
		Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
	}
}


void ScDocShell::SetDocumentModified( sal_Bool bIsModified /* = sal_True */ )
{
	//	BroadcastUno muss auch mit pPaintLockData sofort passieren
	//!	auch bei SetDrawModified, wenn Drawing angebunden ist
	//!	dann eigener Hint???

	if ( pPaintLockData && bIsModified )
	{
        // #i115009# broadcast BCA_BRDCST_ALWAYS, so a component can read recalculated results
        // of RecalcModeAlways formulas (like OFFSET) after modifying cells
        aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
        aDocument.InvalidateTableArea();    // #i105279# needed here
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );

		pPaintLockData->SetModified();			// spaeter...
		return;
	}

	SetDrawModified( bIsModified );

	if ( bIsModified )
	{
		if ( aDocument.IsAutoCalcShellDisabled() )
			SetDocumentModifiedPending( sal_True );
		else
		{
			SetDocumentModifiedPending( sal_False );
            aDocument.InvalidateStyleSheetUsage();
			aDocument.InvalidateTableArea();
            aDocument.InvalidateLastTableOpParams();
			aDocument.Broadcast( SC_HINT_DATACHANGED, BCA_BRDCST_ALWAYS, NULL );
			if ( aDocument.IsForcedFormulaPending() && aDocument.GetAutoCalc() )
				aDocument.CalcFormulaTree( sal_True );
			PostDataChanged();

			//	Detective AutoUpdate:
			//	Update if formulas were modified (DetectiveDirty) or the list contains
			//	"Trace Error" entries (#75362# - Trace Error can look completely different
			//	after changes to non-formula cells).

			ScDetOpList* pList = aDocument.GetDetOpList();
			if ( pList && ( aDocument.IsDetectiveDirty() || pList->HasAddError() ) &&
				 pList->Count() && !IsInUndo() && SC_MOD()->GetAppOptions().GetDetectiveAuto() )
			{
				GetDocFunc().DetectiveRefresh(sal_True);	// sal_True = caused by automatic update
			}
			aDocument.SetDetectiveDirty(sal_False);			// always reset, also if not refreshed
		}

        // #b6697848# notify UNO objects after BCA_BRDCST_ALWAYS etc.
        aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
	}
}

//	SetDrawModified - ohne Formel-Update
//	(Drawing muss auch beim normalen SetDocumentModified upgedated werden,
//	 z.B. bei Tabelle loeschen etc.)

void ScDocShell::SetDrawModified( sal_Bool bIsModified /* = sal_True */ )
{
	sal_Bool bUpdate = ( bIsModified != IsModified() );

	SetModified( bIsModified );

    SfxBindings* pBindings = GetViewBindings();
	if (bUpdate)
	{
		if (pBindings)
		{
			pBindings->Invalidate( SID_SAVEDOC );
			pBindings->Invalidate( SID_DOC_MODIFIED );
		}
	}

	if (bIsModified)
	{
        if (pBindings)
        {
            // #i105960# Undo etc used to be volatile.
            // They always have to be invalidated, including drawing layer or row height changes
            // (but not while pPaintLockData is set).
            pBindings->Invalidate( SID_UNDO );
            pBindings->Invalidate( SID_REDO );
            pBindings->Invalidate( SID_REPEAT );
        }

		if ( aDocument.IsChartListenerCollectionNeedsUpdate() )
		{
			aDocument.UpdateChartListenerCollection();
			SFX_APP()->Broadcast(SfxSimpleHint( SC_HINT_DRAW_CHANGED ));	// Navigator
		}
		SC_MOD()->AnythingChanged();
	}
}

void ScDocShell::SetInUndo(sal_Bool bSet)
{
	bIsInUndo = bSet;
}


void ScDocShell::GetDocStat( ScDocStat& rDocStat )
{
	SfxPrinter* pPrinter = GetPrinter();

	aDocument.GetDocStat( rDocStat );
	rDocStat.nPageCount = 0;

	if ( pPrinter )
		for ( SCTAB i=0; i<rDocStat.nTableCount; i++ )
            rDocStat.nPageCount = sal::static_int_cast<sal_uInt16>( rDocStat.nPageCount +
                (sal_uInt16) ScPrintFunc( this, pPrinter, i ).GetTotalPages() );
}


SfxDocumentInfoDialog* __EXPORT ScDocShell::CreateDocumentInfoDialog(
										 Window *pParent, const SfxItemSet &rSet )
{
	SfxDocumentInfoDialog* pDlg   = new SfxDocumentInfoDialog( pParent, rSet );
	ScDocShell*			   pDocSh = PTR_CAST(ScDocShell,SfxObjectShell::Current());

	//nur mit Statistik, wenn dieses Doc auch angezeigt wird, nicht
	//aus dem Doc-Manager

	if( pDocSh == this )
	{
		ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
		DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001
		::CreateTabPage ScDocStatPageCreate = 	pFact->GetTabPageCreatorFunc( RID_SCPAGE_STAT );
		DBG_ASSERT(ScDocStatPageCreate, "Tabpage create fail!");//CHINA001
		pDlg->AddTabPage( 42,
			ScGlobal::GetRscString( STR_DOC_STAT ),
			ScDocStatPageCreate,
			NULL);
//CHINA001		pDlg->AddTabPage( 42,
//CHINA001		ScGlobal::GetRscString( STR_DOC_STAT ),
//CHINA001		ScDocStatPage::Create,
//CHINA001		NULL );
	}
	return pDlg;
}

Window* ScDocShell::GetActiveDialogParent()
{
	ScTabViewShell* pViewSh	= ScTabViewShell::GetActiveViewShell();
	if ( pViewSh )
		return pViewSh->GetDialogParent();
	else
		return Application::GetDefDialogParent();
}

void ScDocShell::SetSolverSaveData( const ScOptSolverSave& rData )
{
    delete pSolverSaveData;
    pSolverSaveData = new ScOptSolverSave( rData );
}

ScSheetSaveData* ScDocShell::GetSheetSaveData()
{
    if (!pSheetSaveData)
        pSheetSaveData = new ScSheetSaveData;

    return pSheetSaveData;
}

void ScDocShell::UseSheetSaveEntries()
{
    if (pSheetSaveData)
    {
        pSheetSaveData->UseSaveEntries();   // use positions from saved file for next saving

        bool bHasEntries = false;
        SCTAB nTabCount = aDocument.GetTableCount();
        SCTAB nTab;
        for (nTab = 0; nTab < nTabCount; ++nTab)
            if (pSheetSaveData->HasStreamPos(nTab))
                bHasEntries = true;

        if (!bHasEntries)
        {
            // if no positions were set (for example, export to other format),
            // reset all "valid" flags

            for (nTab = 0; nTab < nTabCount; ++nTab)
                if (aDocument.IsStreamValid(nTab))
                    aDocument.SetStreamValid(nTab, sal_False);
        }
    }
}

// --- ScDocShellModificator ------------------------------------------

ScDocShellModificator::ScDocShellModificator( ScDocShell& rDS )
		:
		rDocShell( rDS ),
		aProtector( rDS.GetDocument()->GetRefreshTimerControlAddress() )
{
	ScDocument* pDoc = rDocShell.GetDocument();
	bAutoCalcShellDisabled = pDoc->IsAutoCalcShellDisabled();
	bIdleDisabled = pDoc->IsIdleDisabled();
	pDoc->SetAutoCalcShellDisabled( sal_True );
	pDoc->DisableIdle( sal_True );
}


ScDocShellModificator::~ScDocShellModificator()
{
	ScDocument* pDoc = rDocShell.GetDocument();
	pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
	if ( !bAutoCalcShellDisabled && rDocShell.IsDocumentModifiedPending() )
		rDocShell.SetDocumentModified();	// last one shuts off the lights
	pDoc->DisableIdle( bIdleDisabled );
}


void ScDocShellModificator::SetDocumentModified()
{
	ScDocument* pDoc = rDocShell.GetDocument();
	if ( !pDoc->IsImportingXML() )
	{
		// AutoCalcShellDisabled temporaer restaurieren
		sal_Bool bDisabled = pDoc->IsAutoCalcShellDisabled();
		pDoc->SetAutoCalcShellDisabled( bAutoCalcShellDisabled );
		rDocShell.SetDocumentModified();
		pDoc->SetAutoCalcShellDisabled( bDisabled );
	}
	else
	{
		// uno broadcast is necessary for api to work
		// -> must also be done during xml import
		pDoc->BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
	}
}

//<!--Added by PengYunQuan for Validity Cell Range Picker
sal_Bool ScDocShell::AcceptStateUpdate() const
{
	if( SfxObjectShell::AcceptStateUpdate() )
		return sal_True;

	if( SC_MOD()->Find1RefWindow( SFX_APP()->GetTopWindow() ) )
		return sal_True;

	return sal_False;
}
//-->Added by PengYunQuan for Validity Cell Range Picker


bool ScDocShell::IsChangeRecording() const
{
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    return pChangeTrack != NULL;
}

    
bool ScDocShell::HasChangeRecordProtection() const
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack)
        bRes = pChangeTrack->IsProtected();
    return bRes;
}

    
void ScDocShell::SetChangeRecording( bool bActivate )
{
    bool bOldChangeRecording = IsChangeRecording();

    if (bActivate)
    {
        aDocument.StartChangeTracking();
        ScChangeViewSettings aChangeViewSet;
        aChangeViewSet.SetShowChanges(sal_True);
        aDocument.SetChangeViewSettings(aChangeViewSet);
    }
    else
    {
        aDocument.EndChangeTracking();
        PostPaintGridAll();
    }

    if (bOldChangeRecording != IsChangeRecording())
    {
        UpdateAcceptChangesDialog();
        // Slots invalidieren
        SfxBindings* pBindings = GetViewBindings();
        if (pBindings)
            pBindings->InvalidateAll(sal_False);
    }
}    

        
bool ScDocShell::SetProtectionPassword( const String &rNewPassword )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack)
    {
        sal_Bool bProtected = pChangeTrack->IsProtected();

        if (rNewPassword.Len())
        {
            // when password protection is applied change tracking must always be active
            SetChangeRecording( true );
        
            ::com::sun::star::uno::Sequence< sal_Int8 > aProtectionHash;
            SvPasswordHelper::GetHashPassword( aProtectionHash, rNewPassword );
            pChangeTrack->SetProtection( aProtectionHash );
        }
        else
        {
            pChangeTrack->SetProtection( ::com::sun::star::uno::Sequence< sal_Int8 >() );
        }
        bRes = true;

        if ( bProtected != pChangeTrack->IsProtected() )
        {
            UpdateAcceptChangesDialog();
            SetDocumentModified();
        }
    }

    return bRes;
}    

        
bool ScDocShell::GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash )
{
    bool bRes = false;
    ScChangeTrack* pChangeTrack = aDocument.GetChangeTrack();
    if (pChangeTrack && pChangeTrack->IsProtected())
    {
        rPasswordHash = pChangeTrack->GetProtection();
        bRes = true;
    }
    return bRes;
}    

void ScDocShell::BeforeLoading( SfxMedium& /*rMedium*/, const ::rtl::OUString & rstrTypeName, const ::rtl::OUString & /*rstrFilterName*/ )
{
    const sal_uInt8 nMediumFlag = GetMediumFlag<false>( rstrTypeName );

    if( nMediumFlag & E_MEDIUM_FLAG_MSXML )
    {
        aDocument.SetImportingMSXML( true );

        if ( GetCreateMode() != SFX_CREATE_MODE_ORGANIZER )
            ScColumn::bDoubleAlloc = sal_True;
    }
}

void ScDocShell::AfterLoading( SfxMedium& /*rMedium*/, const ::rtl::OUString & rstrTypeName, const ::rtl::OUString & /*rstrFilterName*/ )
{
    const sal_uInt8 nMediumFlag = GetMediumFlag<false>( rstrTypeName );

	if( nMediumFlag & E_MEDIUM_FLAG_MSXML )
	{
        aDocument.SetImportingMSXML( false );

        if ( GetCreateMode() != SFX_CREATE_MODE_ORGANIZER )
            ScColumn::bDoubleAlloc = sal_False;
    
		// After loading, the XEmbeddedObject was probably set modified flag, so reset the flag to false.
		uno::Sequence < ::rtl::OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
		for ( sal_Int32 n = 0; n < aNames.getLength(); n++ )
		{
			::rtl::OUString	aName = aNames[n];
			uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aName );
			OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
			if ( xObj.is() )
			{
				try
				{
					sal_Int32 nState = xObj->getCurrentState();
					if ( nState != embed::EmbedStates::LOADED )
					{
						uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
						if ( xModifiable.is() )
							xModifiable->setModified(sal_False);
					}
				}
				catch( uno::Exception& )
				{}
			}
		}
	}
}
        
