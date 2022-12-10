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

#include "scitems.hxx"
#include <svx/fmdpage.hxx>
#include <svx/fmview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <svx/unoshape.hxx>

#include <svl/numuno.hxx>
#include <svl/smplhint.hxx>
#include <unotools/undoopt.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/charclass.hxx>
#include <tools/multisel.hxx>
#include <tools/resary.hxx>
#include <toolkit/awt/vclxdevice.hxx>

#include <ctype.h>
#include <float.h>	// DBL_MAX

#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRanges2.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <comphelper/processfactory.hxx>

#include "docuno.hxx"
#include "cellsuno.hxx"
#include "nameuno.hxx"
#include "datauno.hxx"
#include "miscuno.hxx"
#include "notesuno.hxx"
#include "styleuno.hxx"
#include "linkuno.hxx"
#include "servuno.hxx"
#include "targuno.hxx"
#include "convuno.hxx"
#include "optuno.hxx"
#include "forbiuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "docfunc.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "drwlayer.hxx"
#include "rangeutl.hxx"
#include "markdata.hxx"
#include "docoptio.hxx"
#include "scextopt.hxx"
#include "unoguard.hxx"
#include "unonames.hxx"
#include "shapeuno.hxx"
#include "viewuno.hxx"
#include "tabvwsh.hxx"
#include "printfun.hxx"
#include "pfuncache.hxx"
#include "scmod.hxx"
#include "rangeutl.hxx"
#include "ViewSettingsSequenceDefines.hxx"
#include "sheetevents.hxx"
#include "sc.hrc"
#include "scresid.hxx"

using namespace com::sun::star;

// #i111553# provides the name of the VBA constant for this document type (e.g. 'ThisExcelDoc' for Calc)
#define SC_UNO_VBAGLOBNAME "VBAGlobalConstantName"

//------------------------------------------------------------------------

//	alles ohne Which-ID, Map nur fuer PropertySetInfo

//!	umbenennen, sind nicht mehr nur Options
const SfxItemPropertyMapEntry* lcl_GetDocOptPropertyMap()
{
    static SfxItemPropertyMapEntry aDocOptPropertyMap_Impl[] =
	{
        {MAP_CHAR_LEN(SC_UNO_APPLYFMDES),        0, &getBooleanCppuType(),									  0, 0},
        {MAP_CHAR_LEN(SC_UNO_AREALINKS),         0, &getCppuType((uno::Reference<sheet::XAreaLinks>*)0),      0, 0},
        {MAP_CHAR_LEN(SC_UNO_AUTOCONTFOC),       0, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_BASICLIBRARIES),    0, &getCppuType((uno::Reference< script::XLibraryContainer >*)0), beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_DIALOGLIBRARIES),   0, &getCppuType((uno::Reference< script::XLibraryContainer >*)0), beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_VBAGLOBNAME),       0, &getCppuType(static_cast< const rtl::OUString * >(0)),    beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_CALCASSHOWN),       PROP_UNO_CALCASSHOWN, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_CLOCAL),        0, &getCppuType((lang::Locale*)0),                           0, 0},
        {MAP_CHAR_LEN(SC_UNO_CJK_CLOCAL),        0, &getCppuType((lang::Locale*)0),                           0, 0},
        {MAP_CHAR_LEN(SC_UNO_CTL_CLOCAL),        0, &getCppuType((lang::Locale*)0),                           0, 0},
        {MAP_CHAR_LEN(SC_UNO_COLLABELRNG),       0, &getCppuType((uno::Reference<sheet::XLabelRanges>*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_DDELINKS),          0, &getCppuType((uno::Reference<container::XNameAccess>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_DEFTABSTOP),        PROP_UNO_DEFTABSTOP, &getCppuType((sal_Int16*)0),                              0, 0},
        {MAP_CHAR_LEN(SC_UNO_EXTERNALDOCLINKS),  0, &getCppuType((uno::Reference<sheet::XExternalDocLinks>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_FORBIDDEN),         0, &getCppuType((uno::Reference<i18n::XForbiddenCharacters>*)0), beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_HASDRAWPAGES),      0, &getBooleanCppuType(),                                    beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_IGNORECASE),        PROP_UNO_IGNORECASE, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITERENABLED),       PROP_UNO_ITERENABLED, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITERCOUNT),         PROP_UNO_ITERCOUNT, &getCppuType((sal_Int32*)0),                              0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITEREPSILON),       PROP_UNO_ITEREPSILON, &getCppuType((double*)0),                                 0, 0},
        {MAP_CHAR_LEN(SC_UNO_LOOKUPLABELS),      PROP_UNO_LOOKUPLABELS, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_MATCHWHOLE),        PROP_UNO_MATCHWHOLE, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_NAMEDRANGES),       0, &getCppuType((uno::Reference<sheet::XNamedRanges>*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_NAMEDRANGES2),       0, &getCppuType((uno::Reference<sheet::XNamedRanges2>*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_DATABASERNG),       0, &getCppuType((uno::Reference<sheet::XDatabaseRanges>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_NULLDATE),          PROP_UNO_NULLDATE, &getCppuType((util::Date*)0),                             0, 0},
        {MAP_CHAR_LEN(SC_UNO_ROWLABELRNG),       0, &getCppuType((uno::Reference<sheet::XLabelRanges>*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_SHEETLINKS),        0, &getCppuType((uno::Reference<container::XNameAccess>*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_SPELLONLINE),       PROP_UNO_SPELLONLINE, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_STANDARDDEC),       PROP_UNO_STANDARDDEC, &getCppuType((sal_Int16*)0),                              0, 0},
        {MAP_CHAR_LEN(SC_UNO_REGEXENABLED),      PROP_UNO_REGEXENABLED, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_RUNTIMEUID),        0, &getCppuType(static_cast< const rtl::OUString * >(0)),    beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_HASVALIDSIGNATURES),0, &getBooleanCppuType(),                                    beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN(SC_UNO_ISLOADED),          0, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_ISUNDOENABLED),     0, &getBooleanCppuType(),                                    0, 0},
        {MAP_CHAR_LEN(SC_UNO_ISADJUSTHEIGHTENABLED), 0, &getBooleanCppuType(),                                0, 0},
        {MAP_CHAR_LEN(SC_UNO_ISEXECUTELINKENABLED), 0, &getBooleanCppuType(),                                 0, 0},
        {MAP_CHAR_LEN(SC_UNO_ISCHANGEREADONLYENABLED), 0, &getBooleanCppuType(),                              0, 0},
        {MAP_CHAR_LEN(SC_UNO_REFERENCEDEVICE),   0, &getCppuType((uno::Reference<awt::XDevice>*)0),           beans::PropertyAttribute::READONLY, 0},
        {MAP_CHAR_LEN("BuildId"),                0, &::getCppuType(static_cast< const rtl::OUString * >(0)), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_CODENAME),        0, &getCppuType(static_cast< const rtl::OUString * >(0)),    0, 0},

        {0,0,0,0,0,0}
	};
	return aDocOptPropertyMap_Impl;
}

//!	StandardDecimals als Property und vom NumberFormatter ????????

const SfxItemPropertyMapEntry* lcl_GetColumnsPropertyMap()
{
    static SfxItemPropertyMapEntry aColumnsPropertyMap_Impl[] =
	{
		{MAP_CHAR_LEN(SC_UNONAME_MANPAGE),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_CELLVIS),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_OWIDTH),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_CELLWID),	0,	&getCppuType((sal_Int32*)0),	0, 0 },
        {0,0,0,0,0,0}
	};
	return aColumnsPropertyMap_Impl;
}

const SfxItemPropertyMapEntry* lcl_GetRowsPropertyMap()
{
    static SfxItemPropertyMapEntry aRowsPropertyMap_Impl[] =
	{
		{MAP_CHAR_LEN(SC_UNONAME_CELLHGT),	0,	&getCppuType((sal_Int32*)0),	0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_CELLFILT),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_OHEIGHT),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_MANPAGE),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_NEWPAGE),	0,	&getBooleanCppuType(),			0, 0 },
		{MAP_CHAR_LEN(SC_UNONAME_CELLVIS),	0,	&getBooleanCppuType(),			0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_CELLBACK), ATTR_BACKGROUND, &::getCppuType((const sal_Int32*)0), 0, MID_BACK_COLOR },
        {MAP_CHAR_LEN(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND, &::getBooleanCppuType(), 0, MID_GRAPHIC_TRANSPARENT },
        // not sorted, not used with SfxItemPropertyMapEntry::GetByName
        {0,0,0,0,0,0}
	};
	return aRowsPropertyMap_Impl;
}

//!	move these functions to a header file
inline long TwipsToHMM(long nTwips)	{ return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)	{ return (nHMM * 72 + 63) / 127; }

//------------------------------------------------------------------------

#define SCMODELOBJ_SERVICE			"com.sun.star.sheet.SpreadsheetDocument"
#define SCDOCSETTINGS_SERVICE		"com.sun.star.sheet.SpreadsheetDocumentSettings"
#define SCDOC_SERVICE				"com.sun.star.document.OfficeDocument"

SC_SIMPLE_SERVICE_INFO( ScAnnotationsObj, "ScAnnotationsObj", "com.sun.star.sheet.CellAnnotations" )
SC_SIMPLE_SERVICE_INFO( ScDrawPagesObj, "ScDrawPagesObj", "com.sun.star.drawing.DrawPages" )
SC_SIMPLE_SERVICE_INFO( ScScenariosObj, "ScScenariosObj", "com.sun.star.sheet.Scenarios" )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettingsObj, "ScSpreadsheetSettingsObj", "com.sun.star.sheet.SpreadsheetDocumentSettings" )
SC_SIMPLE_SERVICE_INFO( ScTableColumnsObj, "ScTableColumnsObj", "com.sun.star.table.TableColumns" )
SC_SIMPLE_SERVICE_INFO( ScTableRowsObj, "ScTableRowsObj", "com.sun.star.table.TableRows" )
SC_SIMPLE_SERVICE_INFO( ScTableSheetsObj, "ScTableSheetsObj", "com.sun.star.sheet.Spreadsheets" )

//------------------------------------------------------------------------

class ScPrintUIOptions : public vcl::PrinterOptionsHelper
{
public:
    ScPrintUIOptions();
    void SetDefaults();
};

ScPrintUIOptions::ScPrintUIOptions()
{
    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    sal_Bool bSuppress = rPrintOpt.GetSkipEmpty();

    ResStringArray aStrings( ScResId( SCSTR_PRINT_OPTIONS ) );
    DBG_ASSERT( aStrings.Count() >= 10, "resource incomplete" );
    if( aStrings.Count() < 10 ) // bad resource ?
        return;

    m_aUIProperties.realloc( 8 );

    // create Section for spreadsheet (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    String aAppGroupname( aStrings.GetString( 9 ) );
    aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                    aOpt.GetModuleName( SvtModuleOptions::E_SCALC ) ); 
    m_aUIProperties[0].Value = getGroupControlOpt( aAppGroupname, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:TabPage:AppPage" ) ) );

    // create subgroup for pages
    m_aUIProperties[1].Value = getSubgroupControlOpt( rtl::OUString( aStrings.GetString( 0 ) ), rtl::OUString() );

    // create a bool option for empty pages
    m_aUIProperties[2].Value = getBoolControlOpt( rtl::OUString( aStrings.GetString( 1 ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:IsIncludeEmptyPages:CheckBox" ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsIncludeEmptyPages" ) ),
                                                  ! bSuppress
                                                  );
    // create Subgroup for print content
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    m_aUIProperties[3].Value = getSubgroupControlOpt( rtl::OUString( aStrings.GetString( 2 ) ),
                                                      rtl::OUString(),
                                                      aPrintRangeOpt
                                                      );
    
    // create a choice for the content to create
    uno::Sequence< rtl::OUString > aChoices( 3 ), aHelpIds( 3 );
    aChoices[0] = aStrings.GetString( 3 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ) );
    aChoices[1] = aStrings.GetString( 4 );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ) );
    aChoices[2] = aStrings.GetString( 5 );
    aHelpIds[2] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2" ) );
    m_aUIProperties[4].Value = getChoiceControlOpt( rtl::OUString(),
                                                    aHelpIds,
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) ),
                                                    aChoices,
                                                    nContent );
                                                    
    // create Subgroup for print range
    aPrintRangeOpt.mbInternalOnly = sal_True;
    m_aUIProperties[5].Value = getSubgroupControlOpt( rtl::OUString( aStrings.GetString( 6 ) ),
                                                      rtl::OUString(),
                                                      aPrintRangeOpt
                                                      );

    // create a choice for the range to print
    rtl::OUString aPrintRangeName( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    aChoices.realloc( 2 );
    aHelpIds.realloc( 2 );
    aChoices[0] = aStrings.GetString( 7 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:0" ) );
    aChoices[1] = aStrings.GetString( 8 );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:1" ) );
    m_aUIProperties[6].Value = getChoiceControlOpt( rtl::OUString(),
                                                    aHelpIds,
                                                    aPrintRangeName,
                                                    aChoices,
                                                    0 );

    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, sal_True );
    m_aUIProperties[7].Value = getEditControlOpt( rtl::OUString(),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PageRange:Edit" ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ),
                                                  rtl::OUString(),
                                                  aPageRangeOpt
                                                  ); 
}

void ScPrintUIOptions::SetDefaults()
{
    // re-initialize the default values from print options

    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    sal_Bool bSuppress = rPrintOpt.GetSkipEmpty();

    for (sal_Int32 nUIPos=0; nUIPos<m_aUIProperties.getLength(); ++nUIPos)
    {
        uno::Sequence<beans::PropertyValue> aUIProp;
        if ( m_aUIProperties[nUIPos].Value >>= aUIProp )
        {
            for (sal_Int32 nPropPos=0; nPropPos<aUIProp.getLength(); ++nPropPos)
            {
                rtl::OUString aName = aUIProp[nPropPos].Name;
                if ( aName.equalsAscii("Property") )
                {
                    beans::PropertyValue aPropertyValue;
                    if ( aUIProp[nPropPos].Value >>= aPropertyValue )
                    {
                        if ( aPropertyValue.Name.equalsAscii( "PrintContent" ) )
                        {
                            aPropertyValue.Value <<= nContent;
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                        else if ( aPropertyValue.Name.equalsAscii( "IsIncludeEmptyPages" ) )
                        {
                            ScUnoHelpFunctions::SetBoolInAny( aPropertyValue.Value, ! bSuppress );
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                    }
                }
            }
            m_aUIProperties[nUIPos].Value <<= aUIProp;
        }
    }
}

// static
void ScModelObj::CreateAndSet(ScDocShell* pDocSh)
{
	if (pDocSh)
		pDocSh->SetBaseModel( new ScModelObj(pDocSh) );
}

ScModelObj::ScModelObj( ScDocShell* pDocSh ) :
	SfxBaseModel( pDocSh ),
	aPropSet( lcl_GetDocOptPropertyMap() ),
	pDocShell( pDocSh ),
    pPrintFuncCache( NULL ),
    pPrinterOptions( NULL ),
    maChangesListeners( m_aMutex )
{
	// pDocShell may be NULL if this is the base of a ScDocOptionsObj
	if ( pDocShell )
	{
		pDocShell->GetDocument()->AddUnoObject(*this);		// SfxModel is derived from SfxListener
	}
}

ScModelObj::~ScModelObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);

	if (xNumberAgg.is())
		xNumberAgg->setDelegator(uno::Reference<uno::XInterface>());

	delete pPrintFuncCache;
    delete pPrinterOptions;
}

uno::Reference< uno::XAggregation> ScModelObj::GetFormatter()
{
    // pDocShell may be NULL if this is the base of a ScDocOptionsObj
    if ( !xNumberAgg.is() && pDocShell )
    {
        // setDelegator veraendert den RefCount, darum eine Referenz selber halten
		// (direkt am m_refCount, um sich beim release nicht selbst zu loeschen)
		comphelper::increment( m_refCount );
        // waehrend des queryInterface braucht man ein Ref auf das
		// SvNumberFormatsSupplierObj, sonst wird es geloescht.
		uno::Reference<util::XNumberFormatsSupplier> xFormatter(new SvNumberFormatsSupplierObj(pDocShell->GetDocument()->GetFormatTable() ));
		{
			xNumberAgg.set(uno::Reference<uno::XAggregation>( xFormatter, uno::UNO_QUERY ));
			// extra block to force deletion of the temporary before setDelegator
		}

		// beim setDelegator darf die zusaetzliche Ref nicht mehr existieren
		xFormatter = NULL;

		if (xNumberAgg.is())
			xNumberAgg->setDelegator( (cppu::OWeakObject*)this );
        comphelper::decrement( m_refCount );
    } // if ( !xNumberAgg.is() )
    return xNumberAgg;
}

ScDocument* ScModelObj::GetDocument() const
{
	if (pDocShell)
		return pDocShell->GetDocument();
	return NULL;
}

SfxObjectShell* ScModelObj::GetEmbeddedObject() const
{
	return pDocShell;
}

void ScModelObj::UpdateAllRowHeights(const ScMarkData* pTabMark, bool bCalcOutputFactor)
{
    if (pDocShell)
    {
        if (bCalcOutputFactor)
            pDocShell->CalcOutputFactor();
        pDocShell->UpdateAllRowHeights(pTabMark);
    }
}

void ScModelObj::BeforeXMLLoading()
{
	if (pDocShell)
		pDocShell->BeforeXMLLoading();
}

void ScModelObj::AfterXMLLoading(sal_Bool bRet)
{
	if (pDocShell)
		pDocShell->AfterXMLLoading(bRet);
}

ScSheetSaveData* ScModelObj::GetSheetSaveData()
{
    if (pDocShell)
        return pDocShell->GetSheetSaveData();
    return NULL;
}

void ScModelObj::RepaintRange( const ScRange& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PAINT_GRID );
}

uno::Any SAL_CALL ScModelObj::queryInterface( const uno::Type& rType )
												throw(uno::RuntimeException)
{
	SC_QUERYINTERFACE( sheet::XSpreadsheetDocument )
	SC_QUERYINTERFACE( document::XActionLockable )
	SC_QUERYINTERFACE( sheet::XCalculatable )
	SC_QUERYINTERFACE( util::XProtectable )
	SC_QUERYINTERFACE( drawing::XDrawPagesSupplier )
	SC_QUERYINTERFACE( sheet::XGoalSeek )
	SC_QUERYINTERFACE( sheet::XConsolidatable )
	SC_QUERYINTERFACE( sheet::XDocumentAuditing )
	SC_QUERYINTERFACE( style::XStyleFamiliesSupplier )
	SC_QUERYINTERFACE( view::XRenderable )
	SC_QUERYINTERFACE( document::XLinkTargetSupplier )
	SC_QUERYINTERFACE( beans::XPropertySet )
	SC_QUERYINTERFACE( lang::XMultiServiceFactory )
	SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( util::XChangesNotifier )

	uno::Any aRet(SfxBaseModel::queryInterface( rType ));
    if ( !aRet.hasValue()
        && rType != ::getCppuType((uno::Reference< com::sun::star::document::XDocumentEventBroadcaster>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::frame::XController>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::frame::XFrame>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::script::XInvocation>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::reflection::XIdlClassProvider>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::beans::XFastPropertySet>*)0)
        && rType != ::getCppuType((uno::Reference< com::sun::star::awt::XWindow>*)0))
    {
        GetFormatter();
	    if ( xNumberAgg.is() )
		    aRet = xNumberAgg->queryAggregation( rType );
    }

	return aRet;
}

void SAL_CALL ScModelObj::acquire() throw()
{
	SfxBaseModel::acquire();
}

void SAL_CALL ScModelObj::release() throw()
{
	SfxBaseModel::release();
}

uno::Sequence<uno::Type> SAL_CALL ScModelObj::getTypes() throw(uno::RuntimeException)
{
	static uno::Sequence<uno::Type> aTypes;
	if ( aTypes.getLength() == 0 )
	{
		uno::Sequence<uno::Type> aParentTypes(SfxBaseModel::getTypes());
		long nParentLen = aParentTypes.getLength();
		const uno::Type* pParentPtr = aParentTypes.getConstArray();

		uno::Sequence<uno::Type> aAggTypes;
		if ( GetFormatter().is() )
		{
			const uno::Type& rProvType = ::getCppuType((uno::Reference<lang::XTypeProvider>*) 0);
			uno::Any aNumProv(xNumberAgg->queryAggregation(rProvType));
			if(aNumProv.getValueType() == rProvType)
			{
				uno::Reference<lang::XTypeProvider> xNumProv(
					*(uno::Reference<lang::XTypeProvider>*)aNumProv.getValue());
				aAggTypes = xNumProv->getTypes();
			}
		}
		long nAggLen = aAggTypes.getLength();
		const uno::Type* pAggPtr = aAggTypes.getConstArray();

        const long nThisLen = 15;
		aTypes.realloc( nParentLen + nAggLen + nThisLen );
		uno::Type* pPtr = aTypes.getArray();
		pPtr[nParentLen + 0] = getCppuType((const uno::Reference<sheet::XSpreadsheetDocument>*)0);
		pPtr[nParentLen + 1] = getCppuType((const uno::Reference<document::XActionLockable>*)0);
		pPtr[nParentLen + 2] = getCppuType((const uno::Reference<sheet::XCalculatable>*)0);
		pPtr[nParentLen + 3] = getCppuType((const uno::Reference<util::XProtectable>*)0);
		pPtr[nParentLen + 4] = getCppuType((const uno::Reference<drawing::XDrawPagesSupplier>*)0);
		pPtr[nParentLen + 5] = getCppuType((const uno::Reference<sheet::XGoalSeek>*)0);
		pPtr[nParentLen + 6] = getCppuType((const uno::Reference<sheet::XConsolidatable>*)0);
		pPtr[nParentLen + 7] = getCppuType((const uno::Reference<sheet::XDocumentAuditing>*)0);
		pPtr[nParentLen + 8] = getCppuType((const uno::Reference<style::XStyleFamiliesSupplier>*)0);
		pPtr[nParentLen + 9] = getCppuType((const uno::Reference<view::XRenderable>*)0);
		pPtr[nParentLen +10] = getCppuType((const uno::Reference<document::XLinkTargetSupplier>*)0);
		pPtr[nParentLen +11] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
		pPtr[nParentLen +12] = getCppuType((const uno::Reference<lang::XMultiServiceFactory>*)0);
		pPtr[nParentLen +13] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);
        pPtr[nParentLen +14] = getCppuType((const uno::Reference<util::XChangesNotifier>*)0);

		long i;
		for (i=0; i<nParentLen; i++)
			pPtr[i] = pParentPtr[i];					// parent types first

		for (i=0; i<nAggLen; i++)
			pPtr[nParentLen+nThisLen+i] = pAggPtr[i];	// aggregated types last
	}
	return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScModelObj::getImplementationId()
													throw(uno::RuntimeException)
{
	static uno::Sequence< sal_Int8 > aId;
	if( aId.getLength() == 0 )
	{
		aId.realloc( 16 );
		rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
	}
	return aId;
}

void ScModelObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
	//	Not interested in reference update hints here

	if ( rHint.ISA( SfxSimpleHint ) )
	{
		sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
		if ( nId == SFX_HINT_DYING )
		{
			pDocShell = NULL;		// has become invalid
			if (xNumberAgg.is())
			{
				SvNumberFormatsSupplierObj* pNumFmt =
					SvNumberFormatsSupplierObj::getImplementation(
						uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
				if ( pNumFmt )
					pNumFmt->SetNumberFormatter( NULL );
			}

			DELETEZ( pPrintFuncCache );		// must be deleted because it has a pointer to the DocShell
		}
		else if ( nId == SFX_HINT_DATACHANGED )
		{
			//	cached data for rendering become invalid when contents change
			//	(if a broadcast is added to SetDrawModified, is has to be tested here, too)

			DELETEZ( pPrintFuncCache );

            // handle "OnCalculate" sheet events (search also for VBA event handlers)
            if ( pDocShell )
            {
                ScDocument* pDoc = pDocShell->GetDocument();
                if ( pDoc->GetVbaEventProcessor().is() )
                {
                    // If the VBA event processor is set, HasAnyCalcNotification is much faster than HasAnySheetEventScript
                    if ( pDoc->HasAnyCalcNotification() && pDoc->HasAnySheetEventScript( SC_SHEETEVENT_CALCULATE, true ) )
                        HandleCalculateEvents();
                }
                else
                {
                    if ( pDoc->HasAnySheetEventScript( SC_SHEETEVENT_CALCULATE ) )
                        HandleCalculateEvents();
                }
            }
		}
	}
	else if ( rHint.ISA( ScPointerChangedHint ) )
	{
		sal_uInt16 nFlags = ((const ScPointerChangedHint&)rHint).GetFlags();
		if (nFlags & SC_POINTERCHANGED_NUMFMT)
		{
			//	NumberFormatter-Pointer am Uno-Objekt neu setzen

			if (GetFormatter().is())
			{
				SvNumberFormatsSupplierObj* pNumFmt =
					SvNumberFormatsSupplierObj::getImplementation(
						uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
				if ( pNumFmt && pDocShell )
					pNumFmt->SetNumberFormatter( pDocShell->GetDocument()->GetFormatTable() );
			}
		}
	}

    // always call parent - SfxBaseModel might need to handle the same hints again
    SfxBaseModel::Notify( rBC, rHint );     // SfxBaseModel is derived from SfxListener
}

// XSpreadsheetDocument

uno::Reference<sheet::XSpreadsheets> SAL_CALL ScModelObj::getSheets() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return new ScTableSheetsObj(pDocShell);
	return NULL;
}

// XStyleFamiliesSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getStyleFamilies()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return new ScStyleFamiliesObj(pDocShell);
	return NULL;
}

// XRenderable

OutputDevice* lcl_GetRenderDevice( const uno::Sequence<beans::PropertyValue>& rOptions )
{
	OutputDevice* pRet = NULL;
	const beans::PropertyValue* pPropArray = rOptions.getConstArray();
	long nPropCount = rOptions.getLength();
	for (long i = 0; i < nPropCount; i++)
	{
		const beans::PropertyValue& rProp = pPropArray[i];
		String aPropName(rProp.Name);

		if (aPropName.EqualsAscii( SC_UNONAME_RENDERDEV ))
		{
            uno::Reference<awt::XDevice> xRenderDevice(rProp.Value, uno::UNO_QUERY);
			if ( xRenderDevice.is() )
			{
				VCLXDevice* pDevice = VCLXDevice::GetImplementation( xRenderDevice );
				if ( pDevice )
				{
					pRet = pDevice->GetOutputDevice();
					pRet->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
				}
			}
		}
	}
	return pRet;
}

bool lcl_ParseTarget( const String& rTarget, ScRange& rTargetRange, Rectangle& rTargetRect,
                        bool& rIsSheet, ScDocument* pDoc, SCTAB nSourceTab )
{
    // test in same order as in SID_CURRENTCELL execute

    ScAddress aAddress;
    ScRangeUtil aRangeUtil;
    SCTAB nNameTab;
    sal_Int32 nNumeric = 0;

    bool bRangeValid = false;
    bool bRectValid = false;

    if ( rTargetRange.Parse( rTarget, pDoc ) & SCA_VALID )
    {
        bRangeValid = true;             // range reference
    }
    else if ( aAddress.Parse( rTarget, pDoc ) & SCA_VALID )
    {
        rTargetRange = aAddress;
        bRangeValid = true;             // cell reference
    }
    else if ( aRangeUtil.MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange, RUTL_NAMES ) ||
              aRangeUtil.MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange, RUTL_DBASE ) )
    {
        bRangeValid = true;             // named range or database range
    }
    else if ( ByteString( rTarget, RTL_TEXTENCODING_ASCII_US ).IsNumericAscii() &&
              ( nNumeric = rTarget.ToInt32() ) > 0 && nNumeric <= MAXROW+1 )
    {
        // row number is always mapped to cell A(row) on the same sheet
        rTargetRange = ScAddress( 0, (SCROW)(nNumeric-1), nSourceTab );     // target row number is 1-based
        bRangeValid = true;             // row number
    }
    else if ( pDoc->GetTable( rTarget, nNameTab ) )
    {
        rTargetRange = ScAddress(0,0,nNameTab);
        bRangeValid = true;             // sheet name
        rIsSheet = true;                // needs special handling (first page of the sheet)
    }
    else
    {
        // look for named drawing object

        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if ( pDrawLayer )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB i=0; i<nTabCount && !bRangeValid; i++)
            {
                SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(i));
                DBG_ASSERT(pPage,"Page ?");
                if (pPage)
                {
                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                    SdrObject* pObject = aIter.Next();
                    while (pObject && !bRangeValid)
                    {
                        if ( ScDrawLayer::GetVisibleName( pObject ) == rTarget )
                        {
                            rTargetRect = pObject->GetLogicRect();              // 1/100th mm
                            rTargetRange = pDoc->GetRange( i, rTargetRect );    // underlying cells
                            bRangeValid = bRectValid = true;                    // rectangle is valid
                        }
                        pObject = aIter.Next();
                    }
                }
            }
        }
    }
    if ( bRangeValid && !bRectValid )
    {
        //  get rectangle for cell range
        rTargetRect = pDoc->GetMMRect( rTargetRange.aStart.Col(), rTargetRange.aStart.Row(),
                                       rTargetRange.aEnd.Col(),   rTargetRange.aEnd.Row(),
                                       rTargetRange.aStart.Tab() );
    }

    return bRangeValid;
}

sal_Bool ScModelObj::FillRenderMarkData( const uno::Any& aSelection,
                                     const uno::Sequence< beans::PropertyValue >& rOptions,
                                     ScMarkData& rMark,
                                     ScPrintSelectionStatus& rStatus, String& rPagesStr ) const
{
	DBG_ASSERT( !rMark.IsMarked() && !rMark.IsMultiMarked(), "FillRenderMarkData: MarkData must be empty" );
	DBG_ASSERT( pDocShell, "FillRenderMarkData: DocShell must be set" );

	sal_Bool bDone = sal_False;
    
    uno::Reference<frame::XController> xView;

    // defaults when no options are passed: all sheets, include empty pages
    sal_Bool bSelectedSheetsOnly = sal_False;
    sal_Bool bIncludeEmptyPages = sal_True;

    bool bHasPrintContent = false;
    sal_Int32 nPrintContent = 0;        // all sheets / selected sheets / selected cells
    sal_Int32 nPrintRange = 0;          // all pages / pages
    rtl::OUString aPageRange;           // "pages" edit value

    for( sal_Int32 i = 0, nLen = rOptions.getLength(); i < nLen; i++ )
    {
        if( rOptions[i].Name.equalsAscii( "IsOnlySelectedSheets" ) )
        {
            rOptions[i].Value >>= bSelectedSheetsOnly;
        }
        else if( rOptions[i].Name.equalsAscii( "IsIncludeEmptyPages" ) )
        {
            rOptions[i].Value >>= bIncludeEmptyPages;
        }
        else if( rOptions[i].Name.equalsAscii( "PageRange" ) )
        {
            rOptions[i].Value >>= aPageRange;
        }
        else if( rOptions[i].Name.equalsAscii( "PrintRange" ) )
        {
            rOptions[i].Value >>= nPrintRange;
        }
        else if( rOptions[i].Name.equalsAscii( "PrintContent" ) )
        {
            bHasPrintContent = true;
            rOptions[i].Value >>= nPrintContent;
        }
        else if( rOptions[i].Name.equalsAscii( "View" ) )
        {
            rOptions[i].Value >>= xView;
        }
    }

    // "Print Content" selection wins over "Selected Sheets" option
    if ( bHasPrintContent )
        bSelectedSheetsOnly = ( nPrintContent != 0 );

    uno::Reference<uno::XInterface> xInterface(aSelection, uno::UNO_QUERY);
	if ( xInterface.is() )
	{
		ScCellRangesBase* pSelObj = ScCellRangesBase::getImplementation( xInterface );
        uno::Reference< drawing::XShapes > xShapes( xInterface, uno::UNO_QUERY );
        if ( pSelObj && pSelObj->GetDocShell() == pDocShell )
		{
			sal_Bool bSheet = ( ScTableSheetObj::getImplementation( xInterface ) != NULL );
			sal_Bool bCursor = pSelObj->IsCursorOnly();
			const ScRangeList& rRanges = pSelObj->GetRangeList();

			rMark.MarkFromRangeList( rRanges, sal_False );
			rMark.MarkToSimple();

            if ( rMark.IsMultiMarked() )
            {
                // #i115266# copy behavior of old printing:
                // treat multiple selection like a single selection with the enclosing range
                ScRange aMultiMarkArea;
                rMark.GetMultiMarkArea( aMultiMarkArea );
                rMark.ResetMark();
                rMark.SetMarkArea( aMultiMarkArea );
            }

			if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
			{
				// a sheet object is treated like an empty selection: print the used area of the sheet

				if ( bCursor || bSheet )				// nothing selected -> use whole tables
				{
					rMark.ResetMark();		// doesn't change table selection
					rStatus.SetMode( SC_PRINTSEL_CURSOR );
				}
				else
					rStatus.SetMode( SC_PRINTSEL_RANGE );

				rStatus.SetRanges( rRanges );
				bDone = sal_True;
			}
			// multi selection isn't supported
		}
        else if( xShapes.is() )
        {
            //print a selected ole object
            uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY );
            if( xIndexAccess.is() )
            {
                // multi selection isn't supported yet
                uno::Reference< drawing::XShape > xShape( xIndexAccess->getByIndex(0), uno::UNO_QUERY );
                SvxShape* pShape = SvxShape::getImplementation( xShape );
                if( pShape )
                {
                    SdrObject *pSdrObj = pShape->GetSdrObject();
                    if( pDocShell )
                    {
                        ScDocument* pDoc = pDocShell->GetDocument();
                        if( pDoc && pSdrObj )
                        {
                            Rectangle aObjRect = pSdrObj->GetCurrentBoundRect();
                            SCTAB nCurrentTab = ScDocShell::GetCurTab();
                            ScRange aRange = pDoc->GetRange( nCurrentTab, aObjRect );
                            rMark.SetMarkArea( aRange );

                            if( rMark.IsMarked() && !rMark.IsMultiMarked() )
                            {
                                rStatus.SetMode( SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS );
                                bDone = sal_True;
                            }
                        }
                    }
                }
            }
        }
		else if ( ScModelObj::getImplementation( xInterface ) == this )
		{
			//	render the whole document
			//	-> no selection, all sheets

			SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
			for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
				rMark.SelectTable( nTab, sal_True );
			rStatus.SetMode( SC_PRINTSEL_DOCUMENT );
			bDone = sal_True;
		}
		// other selection types aren't supported
	}

    // restrict to selected sheets if a view is available
    if ( bSelectedSheetsOnly && xView.is() )
    {
        ScTabViewObj* pViewObj = ScTabViewObj::getImplementation( xView );
        if (pViewObj)
        {
            ScTabViewShell* pViewSh = pViewObj->GetViewShell();
            if (pViewSh)
            {
                // #i95280# when printing from the shell, the view is never activated,
                // so Excel view settings must also be evaluated here.
                ScExtDocOptions* pExtOpt = pDocShell->GetDocument()->GetExtDocOptions();
                if ( pExtOpt && pExtOpt->IsChanged() )
                {
                    pViewSh->GetViewData()->ReadExtOptions(*pExtOpt);        // Excel view settings
                    pViewSh->SetTabNo( pViewSh->GetViewData()->GetTabNo(), sal_True );
                    pExtOpt->SetChanged( false );
                }

                const ScMarkData& rViewMark = pViewSh->GetViewData()->GetMarkData();
                SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();
                for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
                    if (!rViewMark.GetTableSelect(nTab))
                        rMark.SelectTable( nTab, sal_False );
            }
        }
    }

    ScPrintOptions aNewOptions;
    aNewOptions.SetSkipEmpty( !bIncludeEmptyPages );
    aNewOptions.SetAllSheets( !bSelectedSheetsOnly );
    rStatus.SetOptions( aNewOptions );

    // "PrintRange" enables (1) or disables (0) the "PageRange" edit
    if ( nPrintRange == 1 )
        rPagesStr = aPageRange;
    else
        rPagesStr.Erase();

	return bDone;
}


sal_Int32 SAL_CALL ScModelObj::getRendererCount( const uno::Any& aSelection,
                                    const uno::Sequence<beans::PropertyValue>& rOptions )
								throw (lang::IllegalArgumentException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScMarkData aMark;
	ScPrintSelectionStatus aStatus;
    String aPagesStr;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
		return 0;

	//	The same ScPrintFuncCache object in pPrintFuncCache is used as long as
	//	the same selection is used (aStatus) and the document isn't changed
	//	(pPrintFuncCache is cleared in Notify handler)

	if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
	{
		delete pPrintFuncCache;
		pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
	}
    sal_Int32 nPages = pPrintFuncCache->GetPageCount();

    sal_Int32 nSelectCount = nPages;
    if ( aPagesStr.Len() )
    {
        MultiSelection aPageRanges( aPagesStr );
        aPageRanges.SetTotalRange( Range( 1, nPages ) );
        nSelectCount = aPageRanges.GetSelectCount();
    }
    return nSelectCount;
}

sal_Int32 lcl_GetRendererNum( sal_Int32 nSelRenderer, const String& rPagesStr, sal_Int32 nTotalPages )
{
    if ( !rPagesStr.Len() )
        return nSelRenderer;

    MultiSelection aPageRanges( rPagesStr );
    aPageRanges.SetTotalRange( Range( 1, nTotalPages ) );

    sal_Int32 nSelected = aPageRanges.FirstSelected();
    while ( nSelRenderer > 0 )
    {
        nSelected = aPageRanges.NextSelected();
        --nSelRenderer;
    }
    return nSelected - 1;       // selection is 1-based
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScModelObj::getRenderer( sal_Int32 nSelRenderer,
                                    const uno::Any& aSelection, const uno::Sequence<beans::PropertyValue>& rOptions  )
								throw (lang::IllegalArgumentException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScMarkData aMark;
	ScPrintSelectionStatus aStatus;
    String aPagesStr;
    // #i115266# if FillRenderMarkData fails, keep nTotalPages at 0, but still handle getRenderer(0) below
    long nTotalPages = 0;
    if ( FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
    {
        if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
        {
            delete pPrintFuncCache;
            pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
        }
        nTotalPages = pPrintFuncCache->GetPageCount();
    }
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
	if ( nRenderer >= nTotalPages )
    {
        if ( nSelRenderer == 0 )
        {
            // getRenderer(0) is used to query the settings, so it must always return something

            SCTAB nCurTab = 0;      //! use current sheet from view?
            ScPrintFunc aDefaultFunc( pDocShell, pDocShell->GetPrinter(), nCurTab );
            Size aTwips = aDefaultFunc.GetPageSize();
            awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

            uno::Sequence<beans::PropertyValue> aSequence(1);
            beans::PropertyValue* pArray = aSequence.getArray();
            pArray[0].Name = rtl::OUString::createFromAscii( SC_UNONAME_PAGESIZE );
            pArray[0].Value <<= aPageSize;

            if( ! pPrinterOptions )
                pPrinterOptions = new ScPrintUIOptions;
            else
                pPrinterOptions->SetDefaults();
            pPrinterOptions->appendPrintUIOptions( aSequence );
            return aSequence;
        }
        else
            throw lang::IllegalArgumentException();
    }

	//	printer is used as device (just for page layout), draw view is not needed

	SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );

	ScRange aRange;
	const ScRange* pSelRange = NULL;
	if ( aMark.IsMarked() )
	{
		aMark.GetMarkArea( aRange );
		pSelRange = &aRange;
	}
	ScPrintFunc aFunc( pDocShell, pDocShell->GetPrinter(), nTab,
						pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
	aFunc.SetRenderFlag( sal_True );

	Range aPageRange( nRenderer+1, nRenderer+1 );
	MultiSelection aPage( aPageRange );
	aPage.SetTotalRange( Range(0,RANGE_MAX) );
	aPage.Select( aPageRange );

	long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
	long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, sal_False, NULL );

	ScRange aCellRange;
	sal_Bool bWasCellRange = aFunc.GetLastSourceRange( aCellRange );
	Size aTwips = aFunc.GetPageSize();
	awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

    long nPropCount = bWasCellRange ? 3 : 2;
	uno::Sequence<beans::PropertyValue> aSequence(nPropCount);
	beans::PropertyValue* pArray = aSequence.getArray();
	pArray[0].Name = rtl::OUString::createFromAscii( SC_UNONAME_PAGESIZE );
	pArray[0].Value <<= aPageSize;
    // #i111158# all positions are relative to the whole page, including non-printable area
    pArray[1].Name = rtl::OUString::createFromAscii( SC_UNONAME_INC_NP_AREA );
    pArray[1].Value = uno::makeAny( sal_True );
	if ( bWasCellRange )
	{
		table::CellRangeAddress aRangeAddress( nTab,
						aCellRange.aStart.Col(), aCellRange.aStart.Row(),
						aCellRange.aEnd.Col(), aCellRange.aEnd.Row() );
        pArray[2].Name = rtl::OUString::createFromAscii( SC_UNONAME_SOURCERANGE );
        pArray[2].Value <<= aRangeAddress;
	}

    if( ! pPrinterOptions )
        pPrinterOptions = new ScPrintUIOptions;
    else
        pPrinterOptions->SetDefaults();
    pPrinterOptions->appendPrintUIOptions( aSequence );
	return aSequence;
}

void SAL_CALL ScModelObj::render( sal_Int32 nSelRenderer, const uno::Any& aSelection,
									const uno::Sequence<beans::PropertyValue>& rOptions )
								throw(lang::IllegalArgumentException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScMarkData aMark;
	ScPrintSelectionStatus aStatus;
    String aPagesStr;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
		throw lang::IllegalArgumentException();

	if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
	{
		delete pPrintFuncCache;
		pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
	}
	long nTotalPages = pPrintFuncCache->GetPageCount();
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
	if ( nRenderer >= nTotalPages )
		throw lang::IllegalArgumentException();

	OutputDevice* pDev = lcl_GetRenderDevice( rOptions );
	if ( !pDev )
		throw lang::IllegalArgumentException();

	SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );
	ScDocument* pDoc = pDocShell->GetDocument();

	FmFormView* pDrawView = NULL;
	Rectangle aFull( 0, 0, LONG_MAX, LONG_MAX );

	// #114135#
	ScDrawLayer* pModel = pDoc->GetDrawLayer();

	if( pModel )
	{
		pDrawView = new FmFormView( pModel, pDev );
		pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));
		pDrawView->SetPrintPreview( sal_True );
	}

	ScRange aRange;
	const ScRange* pSelRange = NULL;
	if ( aMark.IsMarked() )
	{
		aMark.GetMarkArea( aRange );
		pSelRange = &aRange;
	}

	//	to increase performance, ScPrintState might be used here for subsequent
	//	pages of the same sheet

	ScPrintFunc aFunc( pDev, pDocShell, nTab, pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
	aFunc.SetDrawView( pDrawView );
	aFunc.SetRenderFlag( sal_True );
    if( aStatus.GetMode() == SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS )
        aFunc.SetExclusivelyDrawOleAndDrawObjects();

	Range aPageRange( nRenderer+1, nRenderer+1 );
	MultiSelection aPage( aPageRange );
	aPage.SetTotalRange( Range(0,RANGE_MAX) );
	aPage.Select( aPageRange );

	long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
	long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pDev->GetExtOutDevData() );
    if ( nRenderer == nTabStart )
    {
        // first page of a sheet: add outline item for the sheet name

        if ( pPDFData && pPDFData->GetIsExportBookmarks() )
        {
            // the sheet starts at the top of the page
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            sal_Int32 nDestID = pPDFData->CreateDest( aArea );
            String aTabName;
            pDoc->GetName( nTab, aTabName );
            sal_Int32 nParent = -1;     // top-level
            pPDFData->CreateOutlineItem( nParent, aTabName, nDestID );
        }
        //--->i56629
        // add the named destination stuff
        if( pPDFData && pPDFData->GetIsExportNamedDestinations() )
        {
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            String aTabName;
            pDoc->GetName( nTab, aTabName );
//need the PDF page number here
            pPDFData->CreateNamedDest( aTabName, aArea );
        }
        //<---i56629
    }

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, sal_True, NULL );

    //  resolve the hyperlinks for PDF export

    if ( pPDFData )
    {
        //  iterate over the hyperlinks that were output for this page

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIter = rBookmarks.begin();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIEnd = rBookmarks.end();
        while ( aIter != aIEnd )
        {
            rtl::OUString aBookmark = aIter->aBookmark;
            if ( aBookmark.toChar() == (sal_Unicode) '#' )
            {
                //  try to resolve internal link

                String aTarget( aBookmark.copy( 1 ) );

                ScRange aTargetRange;
                Rectangle aTargetRect;      // 1/100th mm
                bool bIsSheet = false;
                bool bValid = lcl_ParseTarget( aTarget, aTargetRange, aTargetRect, bIsSheet, pDoc, nTab );

                if ( bValid )
                {
                    sal_Int32 nPage = -1;
                    Rectangle aArea;
                    if ( bIsSheet )
                    {
                        //  Get first page for sheet (if nothing from that sheet is printed,
                        //  this page can show a different sheet)
                        nPage = pPrintFuncCache->GetTabStart( aTargetRange.aStart.Tab() );
                        aArea = pDev->PixelToLogic( Rectangle( 0,0,0,0 ) );
                    }
                    else
                    {
                        pPrintFuncCache->InitLocations( aMark, pDev );      // does nothing if already initialized

                        ScPrintPageLocation aLocation;
                        if ( pPrintFuncCache->FindLocation( aTargetRange.aStart, aLocation ) )
                        {
                            nPage = aLocation.nPage;

                            // get the rectangle of the page's cell range in 1/100th mm
                            ScRange aLocRange = aLocation.aCellRange;
                            Rectangle aLocationMM = pDoc->GetMMRect(
                                       aLocRange.aStart.Col(), aLocRange.aStart.Row(),
                                       aLocRange.aEnd.Col(),   aLocRange.aEnd.Row(),
                                       aLocRange.aStart.Tab() );
                            Rectangle aLocationPixel = aLocation.aRectangle;

                            // Scale and move the target rectangle from aLocationMM to aLocationPixel,
                            // to get the target rectangle in pixels.

                            Fraction aScaleX( aLocationPixel.GetWidth(), aLocationMM.GetWidth() );
                            Fraction aScaleY( aLocationPixel.GetHeight(), aLocationMM.GetHeight() );

                            long nX1 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Left() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nX2 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Right() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nY1 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Top() - aLocationMM.Top(), 1 ) * aScaleY );
                            long nY2 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Bottom() - aLocationMM.Top(), 1 ) * aScaleY );

                            if ( nX1 > aLocationPixel.Right() ) nX1 = aLocationPixel.Right();
                            if ( nX2 > aLocationPixel.Right() ) nX2 = aLocationPixel.Right();
                            if ( nY1 > aLocationPixel.Bottom() ) nY1 = aLocationPixel.Bottom();
                            if ( nY2 > aLocationPixel.Bottom() ) nY2 = aLocationPixel.Bottom();

                            // The link target area is interpreted using the device's MapMode at
                            // the time of the CreateDest call, so PixelToLogic can be used here,
                            // regardless of the MapMode that is actually selected.

                            aArea = pDev->PixelToLogic( Rectangle( nX1, nY1, nX2, nY2 ) );
                        }
                    }

                    if ( nPage >= 0 )
                    {
                        if ( aIter->nLinkId != -1 )
                            pPDFData->SetLinkDest( aIter->nLinkId, pPDFData->CreateDest( aArea, nPage ) );
                        else
                            pPDFData->DescribeRegisteredDest( aIter->nDestId, aArea, nPage );
                    }
                }
            }
            else
            {
                //  external link, use as-is
                pPDFData->SetLinkURL( aIter->nLinkId, aBookmark );
            }
            aIter++;
        }
        rBookmarks.clear();
    }

	if ( pDrawView )
		pDrawView->HideSdrPage();
	delete pDrawView;
}

// XLinkTargetSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getLinks() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return new ScLinkTargetTypesObj(pDocShell);
	return NULL;
}

// XActionLockable

sal_Bool SAL_CALL ScModelObj::isActionLocked() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bLocked = sal_False;
	if (pDocShell)
		bLocked = ( pDocShell->GetLockCount() != 0 );
	return bLocked;
}

void SAL_CALL ScModelObj::addActionLock() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		pDocShell->LockDocument();
}

void SAL_CALL ScModelObj::removeActionLock() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		pDocShell->UnlockDocument();
}

void SAL_CALL ScModelObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		pDocShell->SetLockCount(nLock);
}

sal_Int16 SAL_CALL ScModelObj::resetActionLocks() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_uInt16 nRet = 0;
	if (pDocShell)
	{
		nRet = pDocShell->GetLockCount();
		pDocShell->SetLockCount(0);
	}
	return nRet;
}

void SAL_CALL ScModelObj::lockControllers() throw (::com::sun::star::uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SfxBaseModel::lockControllers();
	if (pDocShell)
		pDocShell->LockPaint();
}

void SAL_CALL ScModelObj::unlockControllers() throw (::com::sun::star::uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (hasControllersLocked())
	{
		SfxBaseModel::unlockControllers();
		if (pDocShell)
			pDocShell->UnlockPaint();
	}
}

// XCalculate

void SAL_CALL ScModelObj::calculate() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		pDocShell->DoRecalc(sal_True);
	else
	{
		DBG_ERROR("keine DocShell");		//! Exception oder so?
	}
}

void SAL_CALL ScModelObj::calculateAll() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		pDocShell->DoHardRecalc(sal_True);
	else
	{
		DBG_ERROR("keine DocShell");		//! Exception oder so?
	}
}

sal_Bool SAL_CALL ScModelObj::isAutomaticCalculationEnabled() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return pDocShell->GetDocument()->GetAutoCalc();

	DBG_ERROR("keine DocShell");		//! Exception oder so?
	return sal_False;
}

void SAL_CALL ScModelObj::enableAutomaticCalculation( sal_Bool bEnabled )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		if ( pDoc->GetAutoCalc() != bEnabled )
		{
			pDoc->SetAutoCalc( bEnabled );
			pDocShell->SetDocumentModified();
		}
	}
	else
	{
		DBG_ERROR("keine DocShell");		//! Exception oder so?
	}
}

// XProtectable

void SAL_CALL ScModelObj::protect( const rtl::OUString& aPassword ) throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    // #i108245# if already protected, don't change anything
    if ( pDocShell && !pDocShell->GetDocument()->IsDocProtected() )
	{
		String aString(aPassword);

		ScDocFunc aFunc(*pDocShell);
		aFunc.Protect( TABLEID_DOC, aString, sal_True );
	}
}

void SAL_CALL ScModelObj::unprotect( const rtl::OUString& aPassword )
						throw(lang::IllegalArgumentException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		String aString(aPassword);

		ScDocFunc aFunc(*pDocShell);
        sal_Bool bDone = aFunc.Unprotect( TABLEID_DOC, aString, sal_True );
        if (!bDone)
            throw lang::IllegalArgumentException();
	}
}

sal_Bool SAL_CALL ScModelObj::isProtected() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return pDocShell->GetDocument()->IsDocProtected();

	DBG_ERROR("keine DocShell");		//! Exception oder so?
	return sal_False;
}

// XDrawPagesSupplier

uno::Reference<drawing::XDrawPages> SAL_CALL ScModelObj::getDrawPages() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return new ScDrawPagesObj(pDocShell);

	DBG_ERROR("keine DocShell");		//! Exception oder so?
	return NULL;
}

#if 0
// XPrintable

rtl::OUString ScModelObj::getPrinterName(void) const
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		SfxPrinter* pPrinter = pDocShell->GetPrinter();
		if (pPrinter)
			return pPrinter->GetName();
	}

	DBG_ERROR("getPrinterName: keine DocShell oder kein Printer");
	return rtl::OUString();
}

void ScModelObj::setPrinterName(const rtl::OUString& PrinterName)
{
	ScUnoGuard aGuard;
	//	Drucker setzen - wie in SfxViewShell::ExecPrint_Impl

	if (pDocShell)
	{
		SfxPrinter* pPrinter = pDocShell->GetPrinter();
		if (pPrinter)
		{
			String aString(PrinterName);
			SfxPrinter* pNewPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), aString );
			if (pNewPrinter->IsKnown())
				pDocShell->SetPrinter( pNewPrinter, SFX_PRINTER_PRINTER );
			else
				delete pNewPrinter;
		}
	}
}

XPropertySetRef ScModelObj::createPrintOptions(void)
{
	ScUnoGuard aGuard;
	return new ScPrintSettingsObj;		//! ScPrintSettingsObj implementieren!
}

void ScModelObj::print(const XPropertySetRef& xOptions)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		//!	xOptions auswerten (wie denn?)

		//!	muss noch
	}
}
#endif

// XGoalSeek

sheet::GoalResult SAL_CALL ScModelObj::seekGoal(
								const table::CellAddress& aFormulaPosition,
								const table::CellAddress& aVariablePosition,
								const ::rtl::OUString& aGoalValue )
									throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sheet::GoalResult aResult;
	aResult.Divergence = DBL_MAX;		// nichts gefunden
	if (pDocShell)
	{
		WaitObject aWait( pDocShell->GetActiveDialogParent() );
		String aGoalString(aGoalValue);
		ScDocument* pDoc = pDocShell->GetDocument();
		double fValue = 0.0;
		sal_Bool bFound = pDoc->Solver(
					(SCCOL)aFormulaPosition.Column, (SCROW)aFormulaPosition.Row, aFormulaPosition.Sheet,
					(SCCOL)aVariablePosition.Column, (SCROW)aVariablePosition.Row, aVariablePosition.Sheet,
					aGoalString, fValue );
		aResult.Result = fValue;
		if (bFound)
			aResult.Divergence = 0.0;	//! das ist gelogen
	}
	return aResult;
}

// XConsolidatable

uno::Reference<sheet::XConsolidationDescriptor> SAL_CALL ScModelObj::createConsolidationDescriptor(
								sal_Bool bEmpty ) throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScConsolidationDescriptor* pNew = new ScConsolidationDescriptor;
	if ( pDocShell && !bEmpty )
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		const ScConsolidateParam* pParam = pDoc->GetConsolidateDlgData();
		if (pParam)
			pNew->SetParam( *pParam );
	}
	return pNew;
}

void SAL_CALL ScModelObj::consolidate(
		const uno::Reference<sheet::XConsolidationDescriptor>& xDescriptor )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	//	das koennte theoretisch ein fremdes Objekt sein, also nur das
	//	oeffentliche XConsolidationDescriptor Interface benutzen, um
	//	die Daten in ein ScConsolidationDescriptor Objekt zu kopieren:
	//!	wenn es schon ein ScConsolidationDescriptor ist, direkt per getImplementation?

	ScConsolidationDescriptor aImpl;
	aImpl.setFunction( xDescriptor->getFunction() );
	aImpl.setSources( xDescriptor->getSources() );
	aImpl.setStartOutputPosition( xDescriptor->getStartOutputPosition() );
	aImpl.setUseColumnHeaders( xDescriptor->getUseColumnHeaders() );
	aImpl.setUseRowHeaders( xDescriptor->getUseRowHeaders() );
	aImpl.setInsertLinks( xDescriptor->getInsertLinks() );

	if (pDocShell)
	{
		const ScConsolidateParam& rParam = aImpl.GetParam();
		pDocShell->DoConsolidate( rParam, sal_True );
		pDocShell->GetDocument()->SetConsolidateDlgData( &rParam );
	}
}

// XDocumentAuditing

void SAL_CALL ScModelObj::refreshArrows() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScDocFunc aFunc(*pDocShell);
		aFunc.DetectiveRefresh();
	}
}

// XViewDataSupplier
uno::Reference< container::XIndexAccess > SAL_CALL ScModelObj::getViewData(  )
    throw (uno::RuntimeException)
{
    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        ScUnoGuard aGuard;
        if (pDocShell && pDocShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
        {
            xRet.set(uno::Reference < container::XIndexAccess >::query(::comphelper::getProcessServiceFactory()->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.IndexedPropertyValues")))));

            uno::Reference < container::XIndexContainer > xCont( xRet, uno::UNO_QUERY );
            DBG_ASSERT( xCont.is(), "ScModelObj::getViewData() failed for OLE object" );
            if( xCont.is() )
            {
                uno::Sequence< beans::PropertyValue > aSeq;
                aSeq.realloc(1);
                String sName;
                pDocShell->GetDocument()->GetName( pDocShell->GetDocument()->GetVisibleTab(), sName );
                rtl::OUString sOUName(sName);
                aSeq[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ACTIVETABLE));
                aSeq[0].Value <<= sOUName;
                xCont->insertByIndex( 0, uno::makeAny( aSeq ) );
            }
        }
    }

    return xRet;
}

//	XPropertySet (Doc-Optionen)
//!	auch an der Applikation anbieten?

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScModelObj::getPropertySetInfo()
														throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	static uno::Reference<beans::XPropertySetInfo> aRef(
		new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
	return aRef;
}

void SAL_CALL ScModelObj::setPropertyValue(
						const rtl::OUString& aPropertyName, const uno::Any& aValue )
				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
						lang::IllegalArgumentException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	String aString(aPropertyName);

	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		const ScDocOptions& rOldOpt = pDoc->GetDocOptions();
		ScDocOptions aNewOpt = rOldOpt;

        sal_Bool bOpt = ScDocOptionsHelper::setPropertyValue( aNewOpt, *aPropSet.getPropertyMap(), aPropertyName, aValue );
		if (bOpt)
		{
			// done...
		}
		else if ( aString.EqualsAscii( SC_UNONAME_CLOCAL ) )
		{
			lang::Locale aLocale;
			if ( aValue >>= aLocale )
			{
				LanguageType eLatin, eCjk, eCtl;
				pDoc->GetLanguage( eLatin, eCjk, eCtl );
				eLatin = ScUnoConversion::GetLanguage(aLocale);
				pDoc->SetLanguage( eLatin, eCjk, eCtl );
			}
		}
        else if ( aString.EqualsAscii( SC_UNO_CODENAME ) )
        {
            rtl::OUString sCodeName;
            if ( aValue >>= sCodeName )
                pDoc->SetCodeName( sCodeName );
        } 
		else if ( aString.EqualsAscii( SC_UNO_CJK_CLOCAL ) )
		{
			lang::Locale aLocale;
			if ( aValue >>= aLocale )
			{
				LanguageType eLatin, eCjk, eCtl;
				pDoc->GetLanguage( eLatin, eCjk, eCtl );
				eCjk = ScUnoConversion::GetLanguage(aLocale);
				pDoc->SetLanguage( eLatin, eCjk, eCtl );
			}
		}
		else if ( aString.EqualsAscii( SC_UNO_CTL_CLOCAL ) )
		{
			lang::Locale aLocale;
			if ( aValue >>= aLocale )
			{
				LanguageType eLatin, eCjk, eCtl;
				pDoc->GetLanguage( eLatin, eCjk, eCtl );
				eCtl = ScUnoConversion::GetLanguage(aLocale);
				pDoc->SetLanguage( eLatin, eCjk, eCtl );
			}
		}
		else if ( aString.EqualsAscii( SC_UNO_APPLYFMDES ) )
		{
			//	model is created if not there
			ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
			pModel->SetOpenInDesignMode( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

			SfxBindings* pBindings = pDocShell->GetViewBindings();
			if (pBindings)
				pBindings->Invalidate( SID_FM_OPEN_READONLY );
		}
		else if ( aString.EqualsAscii( SC_UNO_AUTOCONTFOC ) )
		{
			//	model is created if not there
			ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
			pModel->SetAutoControlFocus( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

			SfxBindings* pBindings = pDocShell->GetViewBindings();
			if (pBindings)
				pBindings->Invalidate( SID_FM_AUTOCONTROLFOCUS );
		}
        else if ( aString.EqualsAscii( SC_UNO_ISLOADED ) )
        {
            pDocShell->SetEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISUNDOENABLED ) )
        {
            sal_Bool bUndoEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            pDoc->EnableUndo( bUndoEnabled );
            sal_uInt16 nCount = ( bUndoEnabled ?
                static_cast< sal_uInt16 >( SvtUndoOptions().GetUndoCount() ) : 0 );
            pDocShell->GetUndoManager()->SetMaxUndoActionCount( nCount );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISADJUSTHEIGHTENABLED ) )
        {
            bool bOldAdjustHeightEnabled = pDoc->IsAdjustHeightEnabled();
            bool bAdjustHeightEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if( bOldAdjustHeightEnabled != bAdjustHeightEnabled )
            {
                pDoc->EnableAdjustHeight( bAdjustHeightEnabled );
                if( bAdjustHeightEnabled )
                    pDocShell->UpdateAllRowHeights();
            }
        }
        else if ( aString.EqualsAscii( SC_UNO_ISEXECUTELINKENABLED ) )
        {
            pDoc->EnableExecuteLink( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISCHANGEREADONLYENABLED ) )
        {
            pDoc->EnableChangeReadOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
		else if ( aString.EqualsAscii( "BuildId" ) )
		{
			aValue >>= maBuildId;
		}
        else if ( aString.EqualsAscii( "SavedObject" ) )    // set from chart after saving
        {
            rtl::OUString aObjName;
            aValue >>= aObjName;
            if ( aObjName.getLength() )
                pDoc->RestoreChartListener( aObjName );
        }

		if ( aNewOpt != rOldOpt )
		{
			pDoc->SetDocOptions( aNewOpt );
            //  Don't recalculate while loading XML, when the formula text is stored.
            //  Recalculation after loading is handled separately.
            //! Recalc only for options that need it?
            if ( !pDoc->IsImportingXML() )
                pDocShell->DoHardRecalc( sal_True );
			pDocShell->SetDocumentModified();
		}
	}
}

uno::Any SAL_CALL ScModelObj::getPropertyValue( const rtl::OUString& aPropertyName )
				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	String aString(aPropertyName);
	uno::Any aRet;

	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		const ScDocOptions& rOpt = pDoc->GetDocOptions();
        aRet = ScDocOptionsHelper::getPropertyValue( rOpt, *aPropSet.getPropertyMap(), aPropertyName );
		if ( aRet.hasValue() )
		{
			// done...
		}
		else if ( aString.EqualsAscii( SC_UNONAME_CLOCAL ) )
		{
			LanguageType eLatin, eCjk, eCtl;
			pDoc->GetLanguage( eLatin, eCjk, eCtl );

			lang::Locale aLocale;
			ScUnoConversion::FillLocale( aLocale, eLatin );
			aRet <<= aLocale;
		}
        else if ( aString.EqualsAscii( SC_UNO_CODENAME ) )
        {
            rtl::OUString sCodeName = pDoc->GetCodeName();
            aRet <<= sCodeName;
        } 

		else if ( aString.EqualsAscii( SC_UNO_CJK_CLOCAL ) )
		{
			LanguageType eLatin, eCjk, eCtl;
			pDoc->GetLanguage( eLatin, eCjk, eCtl );

			lang::Locale aLocale;
			ScUnoConversion::FillLocale( aLocale, eCjk );
			aRet <<= aLocale;
		}
		else if ( aString.EqualsAscii( SC_UNO_CTL_CLOCAL ) )
		{
			LanguageType eLatin, eCjk, eCtl;
			pDoc->GetLanguage( eLatin, eCjk, eCtl );

			lang::Locale aLocale;
			ScUnoConversion::FillLocale( aLocale, eCtl );
			aRet <<= aLocale;
		}
		else if ( aString.EqualsAscii( SC_UNO_NAMEDRANGES2 ) )
		{
			aRet <<= uno::Reference<sheet::XNamedRanges2>(new ScNamedRangesObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_NAMEDRANGES ) )
		{
			aRet <<= uno::Reference<sheet::XNamedRanges>(new ScNamedRangesObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_DATABASERNG ) )
		{
			aRet <<= uno::Reference<sheet::XDatabaseRanges>(new ScDatabaseRangesObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_COLLABELRNG ) )
		{
			aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, sal_True ));
		}
		else if ( aString.EqualsAscii( SC_UNO_ROWLABELRNG ) )
		{
			aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, sal_False ));
		}
		else if ( aString.EqualsAscii( SC_UNO_AREALINKS ) )
		{
			aRet <<= uno::Reference<sheet::XAreaLinks>(new ScAreaLinksObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_DDELINKS ) )
		{
			aRet <<= uno::Reference<container::XNameAccess>(new ScDDELinksObj( pDocShell ));
		}
        else if ( aString.EqualsAscii( SC_UNO_EXTERNALDOCLINKS ) )
        {
            aRet <<= uno::Reference<sheet::XExternalDocLinks>(new ScExternalDocLinksObj(pDocShell));
        }
		else if ( aString.EqualsAscii( SC_UNO_SHEETLINKS ) )
		{
			aRet <<= uno::Reference<container::XNameAccess>(new ScSheetLinksObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_APPLYFMDES ) )
		{
			// default for no model is TRUE
			ScDrawLayer* pModel = pDoc->GetDrawLayer();
			sal_Bool bOpenInDesign = pModel ? pModel->GetOpenInDesignMode() : sal_True;
			ScUnoHelpFunctions::SetBoolInAny( aRet, bOpenInDesign );
		}
		else if ( aString.EqualsAscii( SC_UNO_AUTOCONTFOC ) )
		{
			// default for no model is FALSE
			ScDrawLayer* pModel = pDoc->GetDrawLayer();
			sal_Bool bAutoControlFocus = pModel ? pModel->GetAutoControlFocus() : sal_False;
			ScUnoHelpFunctions::SetBoolInAny( aRet, bAutoControlFocus );
		}
		else if ( aString.EqualsAscii( SC_UNO_FORBIDDEN ) )
		{
			aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
		}
		else if ( aString.EqualsAscii( SC_UNO_HASDRAWPAGES ) )
		{
			ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetDocument()->GetDrawLayer() != 0) );
        }
        else if ( aString.EqualsAscii( SC_UNO_BASICLIBRARIES ) )
        {
            aRet <<= pDocShell->GetBasicContainer();
		}
        else if ( aString.EqualsAscii( SC_UNO_DIALOGLIBRARIES ) )
        {
            aRet <<= pDocShell->GetDialogContainer();
        }
        else if ( aString.EqualsAscii( SC_UNO_VBAGLOBNAME ) )
        {
            /*  #i111553# This property provides the name of the constant that
                will be used to store this model in the global Basic manager.
                That constant will be equivalent to 'ThisComponent' but for
                each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
                constant can co-exist, as required by VBA. */
            aRet <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ThisExcelDoc" ) );
        }
        else if ( aString.EqualsAscii( SC_UNO_RUNTIMEUID ) )
        {
            aRet <<= getRuntimeUID();
        }
        else if ( aString.EqualsAscii( SC_UNO_HASVALIDSIGNATURES ) )
        {
            aRet <<= hasValidSignatures();
        }
        else if ( aString.EqualsAscii( SC_UNO_ISLOADED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, !pDocShell->IsEmpty() );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISUNDOENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsUndoEnabled() );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISADJUSTHEIGHTENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsAdjustHeightEnabled() );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISEXECUTELINKENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsExecuteLinkEnabled() );
        }
        else if ( aString.EqualsAscii( SC_UNO_ISCHANGEREADONLYENABLED ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDoc->IsChangeReadOnlyEnabled() );
        }
        else if ( aString.EqualsAscii( SC_UNO_REFERENCEDEVICE ) )
        {
            VCLXDevice* pXDev = new VCLXDevice();
            pXDev->SetOutputDevice( pDoc->GetRefDevice() );
            aRet <<= uno::Reference< awt::XDevice >( pXDev );
        }
        else if ( aString.EqualsAscii( "BuildId" ) )
		{
			aRet <<= maBuildId;
		}
        else if ( aString.EqualsAscii( "InternalDocument" ) )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetCreateMode() == SFX_CREATE_MODE_INTERNAL) );
        }
	}

	return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScModelObj )

// XMultiServiceFactory

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstance(
								const rtl::OUString& aServiceSpecifier )
								throw(uno::Exception, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<uno::XInterface> xRet;
	String aNameStr(aServiceSpecifier);
	sal_uInt16 nType = ScServiceProvider::GetProviderType(aNameStr);
	if ( nType != SC_SERVICE_INVALID )
	{
		//	drawing layer tables must be kept as long as the model is alive
		//	return stored instance if already set
		switch ( nType )
		{
			case SC_SERVICE_GRADTAB:	xRet.set(xDrawGradTab);	    break;
			case SC_SERVICE_HATCHTAB:	xRet.set(xDrawHatchTab);	break;
			case SC_SERVICE_BITMAPTAB:	xRet.set(xDrawBitmapTab);	break;
			case SC_SERVICE_TRGRADTAB:	xRet.set(xDrawTrGradTab);	break;
			case SC_SERVICE_MARKERTAB:	xRet.set(xDrawMarkerTab);	break;
			case SC_SERVICE_DASHTAB:	xRet.set(xDrawDashTab); 	break;
            case SC_SERVICE_CHDATAPROV: xRet.set(xChartDataProv);   break;
            case SC_SERVICE_VBAOBJECTPROVIDER: xRet.set(xObjProvider); break;
		}

        // #i64497# If a chart is in a temporary document during clipoard paste,
        // there should be no data provider, so that own data is used
        bool bCreate =
            ! ( nType == SC_SERVICE_CHDATAPROV &&
                ( pDocShell->GetCreateMode() == SFX_CREATE_MODE_INTERNAL ));
        // this should never happen, i.e. the temporary document should never be
        // loaded, because this unlinks the data
        OSL_ASSERT( bCreate );

		if ( !xRet.is() && bCreate )
		{
			xRet.set(ScServiceProvider::MakeInstance( nType, pDocShell ));

			//	store created instance
			switch ( nType )
			{
				case SC_SERVICE_GRADTAB:	xDrawGradTab.set(xRet); 	break;
				case SC_SERVICE_HATCHTAB:	xDrawHatchTab.set(xRet);	break;
				case SC_SERVICE_BITMAPTAB:	xDrawBitmapTab.set(xRet);	break;
				case SC_SERVICE_TRGRADTAB:	xDrawTrGradTab.set(xRet);	break;
				case SC_SERVICE_MARKERTAB:	xDrawMarkerTab.set(xRet);	break;
				case SC_SERVICE_DASHTAB:	xDrawDashTab.set(xRet); 	break;
                case SC_SERVICE_CHDATAPROV: xChartDataProv.set(xRet);   break;
                case SC_SERVICE_VBAOBJECTPROVIDER: xObjProvider.set(xRet); break;
			}
		}
	}
	else
	{
		//	alles was ich nicht kenn, werf ich der SvxFmMSFactory an den Hals,
		//	da wird dann 'ne Exception geworfen, wenn's nicht passt...

        try
		{
			xRet.set(SvxFmMSFactory::createInstance(aServiceSpecifier));
			// extra block to force deletion of the temporary before ScShapeObj ctor (setDelegator)
		}
        catch ( lang::ServiceNotRegisteredException & )
        {
        }

		//	#96117# if the drawing factory created a shape, a ScShapeObj has to be used
		//	to support own properties like ImageMap:

		uno::Reference<drawing::XShape> xShape( xRet, uno::UNO_QUERY );
		if ( xShape.is() )
		{
			xRet.clear();				// for aggregation, xShape must be the object's only ref
			new ScShapeObj( xShape );	// aggregates object and modifies xShape
			xRet.set(xShape);
		}
	}
	return xRet;
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstanceWithArguments(
								const rtl::OUString& ServiceSpecifier,
								const uno::Sequence<uno::Any>& aArgs )
								throw(uno::Exception, uno::RuntimeException)
{
	//!	unterscheiden zwischen eigenen Services und denen vom Drawing-Layer?

	ScUnoGuard aGuard;
	uno::Reference<uno::XInterface> xInt(createInstance(ServiceSpecifier));

	if ( aArgs.getLength() )
	{
		//	used only for cell value binding so far - it can be initialized after creating

		uno::Reference<lang::XInitialization> xInit( xInt, uno::UNO_QUERY );
		if ( xInit.is() )
            xInit->initialize( aArgs );
	}

	return xInt;
}

uno::Sequence<rtl::OUString> SAL_CALL ScModelObj::getAvailableServiceNames()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;

	//!	warum sind die Parameter bei concatServiceNames nicht const ???
	//!	return concatServiceNames( ScServiceProvider::GetAllServiceNames(),
	//!							   SvxFmMSFactory::getAvailableServiceNames() );

	uno::Sequence<rtl::OUString> aMyServices(ScServiceProvider::GetAllServiceNames());
	uno::Sequence<rtl::OUString> aDrawServices(SvxFmMSFactory::getAvailableServiceNames());

	return concatServiceNames( aMyServices, aDrawServices );
}

// XServiceInfo

rtl::OUString SAL_CALL ScModelObj::getImplementationName() throw(uno::RuntimeException)
{
	return rtl::OUString::createFromAscii( "ScModelObj" );
}

sal_Bool SAL_CALL ScModelObj::supportsService( const rtl::OUString& rServiceName )
													throw(uno::RuntimeException)
{
	String aServiceStr(rServiceName);
	return aServiceStr.EqualsAscii( SCMODELOBJ_SERVICE ) ||
		   aServiceStr.EqualsAscii( SCDOCSETTINGS_SERVICE ) ||
		   aServiceStr.EqualsAscii( SCDOC_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScModelObj::getSupportedServiceNames()
													throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
	rtl::OUString* pArray = aRet.getArray();
	pArray[0] = rtl::OUString::createFromAscii( SCMODELOBJ_SERVICE );
	pArray[1] = rtl::OUString::createFromAscii( SCDOCSETTINGS_SERVICE );
	return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScModelObj::getSomething(
				const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
	if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
									rId.getConstArray(), 16 ) )
	{
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
	}

    if ( rId.getLength() == 16 &&
        0 == rtl_compareMemory( SfxObjectShell::getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pDocShell ));
    }

	//	aggregated number formats supplier has XUnoTunnel, too
	//	interface from aggregated object must be obtained via queryAggregation

	sal_Int64 nRet = SfxBaseModel::getSomething( rId );
	if ( nRet )
		return nRet;

	if ( GetFormatter().is() )
	{
		const uno::Type& rTunnelType = ::getCppuType((uno::Reference<lang::XUnoTunnel>*) 0);
		uno::Any aNumTunnel(xNumberAgg->queryAggregation(rTunnelType));
		if(aNumTunnel.getValueType() == rTunnelType)
		{
			uno::Reference<lang::XUnoTunnel> xTunnelAgg(
				*(uno::Reference<lang::XUnoTunnel>*)aNumTunnel.getValue());
			return xTunnelAgg->getSomething( rId );
		}
	}

	return 0;
}

// static
const uno::Sequence<sal_Int8>& ScModelObj::getUnoTunnelId()
{
	static uno::Sequence<sal_Int8> * pSeq = 0;
	if( !pSeq )
	{
		osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
		if( !pSeq )
		{
			static uno::Sequence< sal_Int8 > aSeq( 16 );
			rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
			pSeq = &aSeq;
		}
	}
	return *pSeq;
}

// static
ScModelObj* ScModelObj::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
	ScModelObj* pRet = NULL;
	uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
	if (xUT.is())
        pRet = reinterpret_cast<ScModelObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
	return pRet;
}

// XChangesNotifier

void ScModelObj::addChangesListener( const uno::Reference< util::XChangesListener >& aListener )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    maChangesListeners.addInterface( aListener );
}

void ScModelObj::removeChangesListener( const uno::Reference< util::XChangesListener >& aListener )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    maChangesListeners.removeInterface( aListener );
}

bool ScModelObj::HasChangesListeners() const
{
    if ( maChangesListeners.getLength() > 0 )
        return true;

    // "change" event set in any sheet?
    return pDocShell && pDocShell->GetDocument()->HasAnySheetEventScript(SC_SHEETEVENT_CHANGE);
}

void ScModelObj::NotifyChanges( const ::rtl::OUString& rOperation, const ScRangeList& rRanges,
    const uno::Sequence< beans::PropertyValue >& rProperties )
{
    if ( pDocShell && HasChangesListeners() )
    {
        util::ChangesEvent aEvent;
        aEvent.Source.set( static_cast< cppu::OWeakObject* >( this ) );
        aEvent.Base <<= aEvent.Source;

        sal_uLong nRangeCount = rRanges.Count();
        aEvent.Changes.realloc( static_cast< sal_Int32 >( nRangeCount ) );
        for ( sal_uLong nIndex = 0; nIndex < nRangeCount; ++nIndex )
        {
            uno::Reference< table::XCellRange > xRangeObj;

            ScRange aRange( *rRanges.GetObject( nIndex ) );
            if ( aRange.aStart == aRange.aEnd )
            {
                xRangeObj.set( new ScCellObj( pDocShell, aRange.aStart ) );
            }
            else
            {
                xRangeObj.set( new ScCellRangeObj( pDocShell, aRange ) );
            }

            util::ElementChange& rChange = aEvent.Changes[ static_cast< sal_Int32 >( nIndex ) ];
            rChange.Accessor <<= rOperation;
            rChange.Element <<= rProperties;
            rChange.ReplacedElement <<= xRangeObj;
        }

        ::cppu::OInterfaceIteratorHelper aIter( maChangesListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
        		static_cast< util::XChangesListener* >( aIter.next() )->changesOccurred( aEvent );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    // handle sheet events
    //! separate method with ScMarkData? Then change HasChangesListeners back.
    if ( rOperation.compareToAscii("cell-change") == 0 && pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( rRanges, sal_False );
        ScDocument* pDoc = pDocShell->GetDocument();
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
            if (aMarkData.GetTableSelect(nTab))
            {
                const ScSheetEvents* pEvents = pDoc->GetSheetEvents(nTab);
                if (pEvents)
                {
                    const rtl::OUString* pScript = pEvents->GetScript(SC_SHEETEVENT_CHANGE);
                    if (pScript)
                    {
                        ScRangeList aTabRanges;     // collect ranges on this sheet
                        sal_uLong nRangeCount = rRanges.Count();
                        for ( sal_uLong nIndex = 0; nIndex < nRangeCount; ++nIndex )
                        {
                            ScRange aRange( *rRanges.GetObject( nIndex ) );
                            if ( aRange.aStart.Tab() == nTab )
                                aTabRanges.Append( aRange );
                        }
                        sal_uLong nTabRangeCount = aTabRanges.Count();
                        if ( nTabRangeCount > 0 )
                        {
                            uno::Reference<uno::XInterface> xTarget;
                            if ( nTabRangeCount == 1 )
                            {
                                ScRange aRange( *aTabRanges.GetObject( 0 ) );
                                if ( aRange.aStart == aRange.aEnd )
                                    xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellObj( pDocShell, aRange.aStart ) ) );
                                else
                                    xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangeObj( pDocShell, aRange ) ) );
                            }
                            else
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangesObj( pDocShell, aTabRanges ) ) );

                            uno::Sequence<uno::Any> aParams(1);
                            aParams[0] <<= xTarget;

                            uno::Any aRet;
                            uno::Sequence<sal_Int16> aOutArgsIndex;
                            uno::Sequence<uno::Any> aOutArgs;

                            /*ErrCode eRet =*/ pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                        }
                    }
                }
            }
    }
}

void ScModelObj::HandleCalculateEvents()
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        // don't call events before the document is visible
        // (might also set a flag on SFX_EVENT_LOADFINISHED and only disable while loading)
        if ( pDoc->IsDocVisible() )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
            {
                if (pDoc->HasCalcNotification(nTab))
                {
                    if (const ScSheetEvents* pEvents = pDoc->GetSheetEvents( nTab ))
                    {
                        if (const rtl::OUString* pScript = pEvents->GetScript(SC_SHEETEVENT_CALCULATE))
                        {
                            uno::Any aRet;
                            uno::Sequence<uno::Any> aParams;
                            uno::Sequence<sal_Int16> aOutArgsIndex;
                            uno::Sequence<uno::Any> aOutArgs;
                            pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                        }
                    }

                    try
                    {
                        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( pDoc->GetVbaEventProcessor(), uno::UNO_SET_THROW );
                        uno::Sequence< uno::Any > aArgs( 1 );
                        aArgs[ 0 ] <<= nTab;
                        xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( SC_SHEETEVENT_CALCULATE ), aArgs );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }
        }
        pDoc->ResetCalcNotifications();
    }
}

//------------------------------------------------------------------------

ScDrawPagesObj::ScDrawPagesObj(ScDocShell* pDocSh) :
	pDocShell( pDocSh )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScDrawPagesObj::~ScDrawPagesObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScDrawPagesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Referenz-Update interessiert hier nicht

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

uno::Reference<drawing::XDrawPage> ScDrawPagesObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
	if (pDocShell)
	{
		ScDrawLayer* pDrawLayer = pDocShell->MakeDrawLayer();
		DBG_ASSERT(pDrawLayer,"kann Draw-Layer nicht anlegen");
		if ( pDrawLayer && nIndex >= 0 && nIndex < pDocShell->GetDocument()->GetTableCount() )
		{
			SdrPage* pPage = pDrawLayer->GetPage((sal_uInt16)nIndex);
			DBG_ASSERT(pPage,"Draw-Page nicht gefunden");
			if (pPage)
			{
				return uno::Reference<drawing::XDrawPage> (pPage->getUnoPage(), uno::UNO_QUERY);
			}
		}
	}
	return NULL;
}

// XDrawPages

uno::Reference<drawing::XDrawPage> SAL_CALL ScDrawPagesObj::insertNewByIndex( sal_Int32 nPos )
											throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<drawing::XDrawPage> xRet;
	if (pDocShell)
	{
		String aNewName;
		pDocShell->GetDocument()->CreateValidTabName(aNewName);
		ScDocFunc aFunc(*pDocShell);
		if ( aFunc.InsertTable( (SCTAB)nPos, aNewName, sal_True, sal_True ) )
			xRet.set(GetObjectByIndex_Impl( nPos ));
	}
	return xRet;
}

void SAL_CALL ScDrawPagesObj::remove( const uno::Reference<drawing::XDrawPage>& xPage )
											throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SvxDrawPage* pImp = SvxDrawPage::getImplementation( xPage );
	if ( pDocShell && pImp )
	{
		SdrPage* pPage = pImp->GetSdrPage();
		if (pPage)
		{
			SCTAB nPageNum = static_cast<SCTAB>(pPage->GetPageNum());
			ScDocFunc aFunc(*pDocShell);
			aFunc.DeleteTable( nPageNum, sal_True, sal_True );
		}
	}
}

// XIndexAccess

sal_Int32 SAL_CALL ScDrawPagesObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return pDocShell->GetDocument()->GetTableCount();
	return 0;
}

uno::Any SAL_CALL ScDrawPagesObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<drawing::XDrawPage> xPage(GetObjectByIndex_Impl(nIndex));
	if (xPage.is())
        return uno::makeAny(xPage);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScDrawPagesObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<drawing::XDrawPage>*)0);
}

sal_Bool SAL_CALL ScDrawPagesObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

//------------------------------------------------------------------------

ScTableSheetsObj::ScTableSheetsObj(ScDocShell* pDocSh) :
	pDocShell( pDocSh )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableSheetsObj::~ScTableSheetsObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableSheetsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Referenz-Update interessiert hier nicht

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// XSpreadsheets

ScTableSheetObj* ScTableSheetsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
	if ( pDocShell && nIndex >= 0 && nIndex < pDocShell->GetDocument()->GetTableCount() )
		return new ScTableSheetObj( pDocShell, static_cast<SCTAB>(nIndex) );

	return NULL;
}

ScTableSheetObj* ScTableSheetsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
	if (pDocShell)
	{
		SCTAB nIndex;
		String aString(aName);
		if ( pDocShell->GetDocument()->GetTable( aString, nIndex ) )
			return new ScTableSheetObj( pDocShell, nIndex );
	}
	return NULL;
}

void SAL_CALL ScTableSheetsObj::insertNewByName( const rtl::OUString& aName, sal_Int16 nPosition )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if (pDocShell)
	{
		String aNamStr(aName);
		ScDocFunc aFunc(*pDocShell);
		bDone = aFunc.InsertTable( nPosition, aNamStr, sal_True, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::moveByName( const rtl::OUString& aName, sal_Int16 nDestination )
											throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if (pDocShell)
	{
		String aNamStr(aName);
		SCTAB nSource;
		if ( pDocShell->GetDocument()->GetTable( aNamStr, nSource ) )
			bDone = pDocShell->MoveTable( nSource, nDestination, sal_False, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::copyByName( const rtl::OUString& aName,
								const rtl::OUString& aCopy, sal_Int16 nDestination )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if (pDocShell)
	{
		String aNamStr(aName);
		String aNewStr(aCopy);
		SCTAB nSource;
		if ( pDocShell->GetDocument()->GetTable( aNamStr, nSource ) )
		{
			bDone = pDocShell->MoveTable( nSource, nDestination, sal_True, sal_True );
			if (bDone)
			{
                // #i92477# any index past the last sheet means "append" in MoveTable
                SCTAB nResultTab = static_cast<SCTAB>(nDestination);
                SCTAB nTabCount = pDocShell->GetDocument()->GetTableCount();    // count after copying
                if (nResultTab >= nTabCount)
                    nResultTab = nTabCount - 1;

                ScDocFunc aFunc(*pDocShell);
                bDone = aFunc.RenameTable( nResultTab, aNewStr, sal_True, sal_True );
			}
		}
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::insertByName( const rtl::OUString& aName, const uno::Any& aElement )
							throw(lang::IllegalArgumentException, container::ElementExistException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	sal_Bool bIllArg = sal_False;

	//!	Type of aElement can be some specific interface instead of XInterface

	if ( pDocShell )
	{
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
		if ( xInterface.is() )
		{
			ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
			if ( pSheetObj && !pSheetObj->GetDocShell() )	// noch nicht eingefuegt?
			{
				ScDocument* pDoc = pDocShell->GetDocument();
				String aNamStr(aName);
				SCTAB nDummy;
				if ( pDoc->GetTable( aNamStr, nDummy ) )
				{
					//	name already exists
					throw container::ElementExistException();
				}
				else
				{
					SCTAB nPosition = pDoc->GetTableCount();
					ScDocFunc aFunc(*pDocShell);
					bDone = aFunc.InsertTable( nPosition, aNamStr, sal_True, sal_True );
					if (bDone)
						pSheetObj->InitInsertSheet( pDocShell, nPosition );
					//	Dokument und neuen Range am Objekt setzen
				}
			}
			else
				bIllArg = sal_True;
		}
		else
			bIllArg = sal_True;
	}

	if (!bDone)
	{
		if (bIllArg)
			throw lang::IllegalArgumentException();
		else
			throw uno::RuntimeException();		// ElementExistException is handled above
	}
}

void SAL_CALL ScTableSheetsObj::replaceByName( const rtl::OUString& aName, const uno::Any& aElement )
							throw(lang::IllegalArgumentException, container::NoSuchElementException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	sal_Bool bIllArg = sal_False;

	//!	Type of aElement can be some specific interface instead of XInterface

	if ( pDocShell )
	{
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
		if ( xInterface.is() )
		{
			ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
			if ( pSheetObj && !pSheetObj->GetDocShell() )	// noch nicht eingefuegt?
			{
				String aNamStr(aName);
				SCTAB nPosition;
				if ( pDocShell->GetDocument()->GetTable( aNamStr, nPosition ) )
				{
					ScDocFunc aFunc(*pDocShell);
					if ( aFunc.DeleteTable( nPosition, sal_True, sal_True ) )
					{
						//	InsertTable kann jetzt eigentlich nicht schiefgehen...
						bDone = aFunc.InsertTable( nPosition, aNamStr, sal_True, sal_True );
						if (bDone)
							pSheetObj->InitInsertSheet( pDocShell, nPosition );
					}
				}
				else
				{
					//	not found
					throw container::NoSuchElementException();
				}
			}
			else
				bIllArg = sal_True;
		}
		else
			bIllArg = sal_True;
	}

	if (!bDone)
	{
		if (bIllArg)
			throw lang::IllegalArgumentException();
		else
			throw uno::RuntimeException();		// NoSuchElementException is handled above
	}
}

void SAL_CALL ScTableSheetsObj::removeByName( const rtl::OUString& aName )
								throw(container::NoSuchElementException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if (pDocShell)
	{
		SCTAB nIndex;
		String aString(aName);
		if ( pDocShell->GetDocument()->GetTable( aString, nIndex ) )
		{
			ScDocFunc aFunc(*pDocShell);
			bDone = aFunc.DeleteTable( nIndex, sal_True, sal_True );
		}
		else
		{
			//	not found
			throw container::NoSuchElementException();
		}
	}

	if (!bDone)
		throw uno::RuntimeException();		// NoSuchElementException is handled above
}

// XCellRangesAccess

uno::Reference< table::XCell > SAL_CALL ScTableSheetsObj::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
	if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellByPosition(nColumn, nRow);
}

uno::Reference< table::XCellRange > SAL_CALL ScTableSheetsObj::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
	if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellRangeByPosition(nLeft, nTop, nRight, nBottom);
}

uno::Sequence < uno::Reference< table::XCellRange > > SAL_CALL ScTableSheetsObj::getCellRangesByName( const rtl::OUString& aRange )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
    uno::Sequence < uno::Reference < table::XCellRange > > xRet;

	ScRangeList aRangeList;
    ScDocument* pDoc = pDocShell->GetDocument();
    if (ScRangeStringConverter::GetRangeListFromString( aRangeList, aRange, pDoc, ::formula::FormulaGrammar::CONV_OOO, ';' ))
    {
	    sal_Int32 nCount = aRangeList.Count();
        if (nCount)
        {
            xRet.realloc(nCount);
	        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
	        {
		        const ScRange* pRange = aRangeList.GetObject( nIndex );
		        if( pRange )
                    xRet[nIndex] = new ScCellRangeObj(pDocShell, *pRange);
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw lang::IllegalArgumentException();
    return xRet;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableSheetsObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableSheetsObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
		return pDocShell->GetDocument()->GetTableCount();
	return 0;
}

uno::Any SAL_CALL ScTableSheetsObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByIndex_Impl(nIndex));
	if (xSheet.is())
        return uno::makeAny(xSheet);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScTableSheetsObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<sheet::XSpreadsheet>*)0);
}

sal_Bool SAL_CALL ScTableSheetsObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScTableSheetsObj::getByName( const rtl::OUString& aName )
			throw(container::NoSuchElementException,
					lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByName_Impl(aName));
	if (xSheet.is())
        return uno::makeAny(xSheet);
	else
		throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScTableSheetsObj::getElementNames()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		SCTAB nCount = pDoc->GetTableCount();
		String aName;
		uno::Sequence<rtl::OUString> aSeq(nCount);
		rtl::OUString* pAry = aSeq.getArray();
		for (SCTAB i=0; i<nCount; i++)
		{
			pDoc->GetName( i, aName );
			pAry[i] = aName;
		}
		return aSeq;
	}
	return uno::Sequence<rtl::OUString>();
}

sal_Bool SAL_CALL ScTableSheetsObj::hasByName( const rtl::OUString& aName )
										throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		SCTAB nIndex;
		if ( pDocShell->GetDocument()->GetTable( String(aName), nIndex ) )
			return sal_True;
	}
	return sal_False;
}

//------------------------------------------------------------------------

ScTableColumnsObj::ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT, SCCOL nSC, SCCOL nEC) :
	pDocShell( pDocSh ),
	nTab	 ( nT ),
	nStartCol( nSC ),
	nEndCol	 ( nEC )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableColumnsObj::~ScTableColumnsObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableColumnsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	if ( rHint.ISA( ScUpdateRefHint ) )
	{
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

		//!	Referenz-Update fuer Tab und Start/Ende
	}
	else if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// XTableColumns

ScTableColumnObj* ScTableColumnsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
	SCCOL nCol = static_cast<SCCOL>(nIndex) + nStartCol;
	if ( pDocShell && nCol <= nEndCol )
		return new ScTableColumnObj( pDocShell, nCol, nTab );

	return NULL;	// falscher Index
}

ScTableColumnObj* ScTableColumnsObj::GetObjectByName_Impl(const rtl::OUString& aName) const
{
	SCCOL nCol = 0;
	String aString(aName);
	if ( ::AlphaToCol( nCol, aString) )
		if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
			return new ScTableColumnObj( pDocShell, nCol, nTab );

	return NULL;
}

void SAL_CALL ScTableColumnsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartCol+nPosition <= nEndCol &&
			nStartCol+nPosition+nCount-1 <= MAXCOL )
	{
		ScDocFunc aFunc(*pDocShell);
		ScRange aRange( (SCCOL)(nStartCol+nPosition), 0, nTab,
						(SCCOL)(nStartCol+nPosition+nCount-1), MAXROW, nTab );
		bDone = aFunc.InsertCells( aRange, NULL, INS_INSCOLS, sal_True, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScTableColumnsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	//	Der zu loeschende Bereich muss innerhalb des Objekts liegen
	if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartCol+nIndex+nCount-1 <= nEndCol )
	{
		ScDocFunc aFunc(*pDocShell);
		ScRange aRange( (SCCOL)(nStartCol+nIndex), 0, nTab,
						(SCCOL)(nStartCol+nIndex+nCount-1), MAXROW, nTab );
		bDone = aFunc.DeleteCells( aRange, NULL, DEL_DELCOLS, sal_True, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableColumnsObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.TableColumnsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableColumnsObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return nEndCol - nStartCol + 1;
}

uno::Any SAL_CALL ScTableColumnsObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<table::XCellRange> xColumn(GetObjectByIndex_Impl(nIndex));
	if (xColumn.is())
        return uno::makeAny(xColumn);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScTableColumnsObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScTableColumnsObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

uno::Any SAL_CALL ScTableColumnsObj::getByName( const rtl::OUString& aName )
			throw(container::NoSuchElementException,
					lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<table::XCellRange> xColumn(GetObjectByName_Impl(aName));
	if (xColumn.is())
        return uno::makeAny(xColumn);
	else
		throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScTableColumnsObj::getElementNames()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCCOL nCount = nEndCol - nStartCol + 1;
	uno::Sequence<rtl::OUString> aSeq(nCount);
	rtl::OUString* pAry = aSeq.getArray();
	for (SCCOL i=0; i<nCount; i++)
		pAry[i] = ::ScColToAlpha( nStartCol + i );

	return aSeq;
}

sal_Bool SAL_CALL ScTableColumnsObj::hasByName( const rtl::OUString& aName )
										throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCCOL nCol = 0;
	String aString(aName);
	if ( ::AlphaToCol( nCol, aString) )
		if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
			return sal_True;

	return sal_False;		// nicht gefunden
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnsObj::getPropertySetInfo()
														throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	static uno::Reference<beans::XPropertySetInfo> aRef(
		new SfxItemPropertySetInfo( lcl_GetColumnsPropertyMap() ));
	return aRef;
}

void SAL_CALL ScTableColumnsObj::setPropertyValue(
						const rtl::OUString& aPropertyName, const uno::Any& aValue )
				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
						lang::IllegalArgumentException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScDocFunc aFunc(*pDocShell);
	SCCOLROW nColArr[2];
	nColArr[0] = nStartCol;
	nColArr[1] = nEndCol;
	String aNameString(aPropertyName);

	if ( aNameString.EqualsAscii( SC_UNONAME_CELLWID ) )
	{
		sal_Int32 nNewWidth = 0;
		if ( aValue >>= nNewWidth )
			aFunc.SetWidthOrHeight( sal_True, 1, nColArr, nTab, SC_SIZE_ORIGINAL,
									(sal_uInt16)HMMToTwips(nNewWidth), sal_True, sal_True );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
	{
		sal_Bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
		ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
		aFunc.SetWidthOrHeight( sal_True, 1, nColArr, nTab, eMode, 0, sal_True, sal_True );
		//	SC_SIZE_DIRECT with size 0: hide
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_OWIDTH ) )
	{
		sal_Bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
		if (bOpt)
			aFunc.SetWidthOrHeight( sal_True, 1, nColArr, nTab,
									SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, sal_True, sal_True );
		// sal_False for columns currently has no effect
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) || aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
	{
		//!	single function to set/remove all breaks?
		sal_Bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
		for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
			if (bSet)
				aFunc.InsertPageBreak( sal_True, ScAddress(nCol,0,nTab), sal_True, sal_True, sal_True );
			else
				aFunc.RemovePageBreak( sal_True, ScAddress(nCol,0,nTab), sal_True, sal_True, sal_True );
	}
}

uno::Any SAL_CALL ScTableColumnsObj::getPropertyValue( const rtl::OUString& aPropertyName )
				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScDocument* pDoc = pDocShell->GetDocument();
	String aNameString(aPropertyName);
	uno::Any aAny;

	//!	loop over all columns for current state?

	if ( aNameString.EqualsAscii( SC_UNONAME_CELLWID ) )
	{
		// for hidden column, return original height
		sal_uInt16 nWidth = pDoc->GetOriginalWidth( nStartCol, nTab );
		aAny <<= (sal_Int32)TwipsToHMM(nWidth);
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
	{
        SCCOL nLastCol;
        bool bVis = !pDoc->ColHidden(nStartCol, nTab, nLastCol);
		ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_OWIDTH ) )
	{
		sal_Bool bOpt = !(pDoc->GetColFlags( nStartCol, nTab ) & CR_MANUALSIZE);
		ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) )
	{
        ScBreakType nBreak = pDoc->HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
	{
        ScBreakType nBreak = pDoc->HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) );
	}

	return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableColumnsObj )

//------------------------------------------------------------------------

ScTableRowsObj::ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT, SCROW nSR, SCROW nER) :
	pDocShell( pDocSh ),
	nTab	 ( nT ),
	nStartRow( nSR ),
	nEndRow	 ( nER )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScTableRowsObj::~ScTableRowsObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScTableRowsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	if ( rHint.ISA( ScUpdateRefHint ) )
	{
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

		//!	Referenz-Update fuer Tab und Start/Ende
	}
	else if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// XTableRows

ScTableRowObj* ScTableRowsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
	SCROW nRow = static_cast<SCROW>(nIndex) + nStartRow;
	if ( pDocShell && nRow <= nEndRow )
		return new ScTableRowObj( pDocShell, nRow, nTab );

	return NULL;	// falscher Index
}

void SAL_CALL ScTableRowsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartRow+nPosition <= nEndRow &&
			nStartRow+nPosition+nCount-1 <= MAXROW )
	{
		ScDocFunc aFunc(*pDocShell);
		ScRange aRange( 0, (SCROW)(nStartRow+nPosition), nTab,
						MAXCOL, (SCROW)(nStartRow+nPosition+nCount-1), nTab );
		bDone = aFunc.InsertCells( aRange, NULL, INS_INSROWS, sal_True, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScTableRowsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	//	Der zu loeschende Bereich muss innerhalb des Objekts liegen
	if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartRow+nIndex+nCount-1 <= nEndRow )
	{
		ScDocFunc aFunc(*pDocShell);
		ScRange aRange( 0, (SCROW)(nStartRow+nIndex), nTab,
						MAXCOL, (SCROW)(nStartRow+nIndex+nCount-1), nTab );
		bDone = aFunc.DeleteCells( aRange, NULL, DEL_DELROWS, sal_True, sal_True );
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableRowsObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.TableRowsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableRowsObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return nEndRow - nStartRow + 1;
}

uno::Any SAL_CALL ScTableRowsObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<table::XCellRange> xRow(GetObjectByIndex_Impl(nIndex));
	if (xRow.is())
        return uno::makeAny(xRow);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScTableRowsObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<table::XCellRange>*)0);
}

sal_Bool SAL_CALL ScTableRowsObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowsObj::getPropertySetInfo()
														throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	static uno::Reference<beans::XPropertySetInfo> aRef(
		new SfxItemPropertySetInfo( lcl_GetRowsPropertyMap() ));
	return aRef;
}

void SAL_CALL ScTableRowsObj::setPropertyValue(
						const rtl::OUString& aPropertyName, const uno::Any& aValue )
				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
						lang::IllegalArgumentException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScDocFunc aFunc(*pDocShell);
	ScDocument* pDoc = pDocShell->GetDocument();
	SCCOLROW nRowArr[2];
	nRowArr[0] = nStartRow;
	nRowArr[1] = nEndRow;
	String aNameString(aPropertyName);

    if ( aNameString.EqualsAscii( SC_UNONAME_OHEIGHT ) )
    {
        sal_Int32 nNewHeight = 0;
        if ( pDoc->IsImportingXML() && ( aValue >>= nNewHeight ) )
        {
            // used to set the stored row height for rows with optimal height when loading.

            // TODO: It's probably cleaner to use a different property name
            // for this.
            pDoc->SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)HMMToTwips(nNewHeight) );
        }
        else
        {
            sal_Bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bOpt)
                aFunc.SetWidthOrHeight( sal_False, 1, nRowArr, nTab, SC_SIZE_OPTIMAL, 0, sal_True, sal_True );
            else
            {
                //! manually set old heights again?
            }
        }
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLHGT ) )
	{
		sal_Int32 nNewHeight = 0;
		if ( aValue >>= nNewHeight )
			aFunc.SetWidthOrHeight( sal_False, 1, nRowArr, nTab, SC_SIZE_ORIGINAL,
									(sal_uInt16)HMMToTwips(nNewHeight), sal_True, sal_True );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
	{
		sal_Bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
		ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
		aFunc.SetWidthOrHeight( sal_False, 1, nRowArr, nTab, eMode, 0, sal_True, sal_True );
		//	SC_SIZE_DIRECT with size 0: hide
	}
    else if ( aNameString.EqualsAscii( SC_UNONAME_VISFLAG ) )
    {
        // #i116460# Shortcut to only set the flag, without drawing layer update etc.
        // Should only be used from import filters.
        pDoc->SetRowHidden(nStartRow, nEndRow, nTab, !ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    }
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLFILT ) )
	{
		//!	undo etc.
		if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            pDoc->SetRowFiltered(nStartRow, nEndRow, nTab, true);
        else
            pDoc->SetRowFiltered(nStartRow, nEndRow, nTab, false);
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE) || aNameString.EqualsAscii( SC_UNONAME_MANPAGE) )
	{
		//!	single function to set/remove all breaks?
		sal_Bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
		for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
			if (bSet)
				aFunc.InsertPageBreak( sal_False, ScAddress(0,nRow,nTab), sal_True, sal_True, sal_True );
			else
				aFunc.RemovePageBreak( sal_False, ScAddress(0,nRow,nTab), sal_True, sal_True, sal_True );
	}
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLBACK ) || aNameString.EqualsAscii( SC_UNONAME_CELLTRAN ) )
    {
        // #i57867# Background color is specified for row styles in the file format,
        // so it has to be supported along with the row properties (import only).

        // Use ScCellRangeObj to set the property for all cells in the rows
        // (this means, the "row attribute" must be set before individual cell attributes).

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        xRangeObj->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScTableRowsObj::getPropertyValue( const rtl::OUString& aPropertyName )
				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (!pDocShell)
		throw uno::RuntimeException();

	ScDocument* pDoc = pDocShell->GetDocument();
	String aNameString(aPropertyName);
	uno::Any aAny;

	//!	loop over all rows for current state?

	if ( aNameString.EqualsAscii( SC_UNONAME_CELLHGT ) )
	{
		// for hidden row, return original height
		sal_uInt16 nHeight = pDoc->GetOriginalHeight( nStartRow, nTab );
		aAny <<= (sal_Int32)TwipsToHMM(nHeight);
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLVIS ) )
	{
        SCROW nLastRow;
        bool bVis = !pDoc->RowHidden(nStartRow, nTab, nLastRow);
		ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_CELLFILT ) )
	{
        bool bVis = pDoc->RowFiltered(nStartRow, nTab);
		ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_OHEIGHT ) )
	{
		sal_Bool bOpt = !(pDoc->GetRowFlags( nStartRow, nTab ) & CR_MANUALSIZE);
		ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_NEWPAGE ) )
	{
        ScBreakType nBreak = pDoc->HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak );
	}
	else if ( aNameString.EqualsAscii( SC_UNONAME_MANPAGE ) )
	{
        ScBreakType nBreak = pDoc->HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) );
	}
    else if ( aNameString.EqualsAscii( SC_UNONAME_CELLBACK ) || aNameString.EqualsAscii( SC_UNONAME_CELLTRAN ) )
    {
        // Use ScCellRangeObj to get the property from the cell range
        // (for completeness only, this is not used by the XML filter).

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        aAny = xRangeObj->getPropertyValue( aPropertyName );
    }

	return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableRowsObj )

//------------------------------------------------------------------------

//UNUSED2008-05  ScSpreadsheetSettingsObj::ScSpreadsheetSettingsObj(ScDocShell* pDocSh) :
//UNUSED2008-05  pDocShell( pDocSh )
//UNUSED2008-05  {
//UNUSED2008-05      pDocShell->GetDocument()->AddUnoObject(*this);
//UNUSED2008-05  }

ScSpreadsheetSettingsObj::~ScSpreadsheetSettingsObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScSpreadsheetSettingsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Referenz-Update interessiert hier nicht

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettingsObj::getPropertySetInfo()
														throw(uno::RuntimeException)
{
	//!	muss noch
	return NULL;
}

void SAL_CALL ScSpreadsheetSettingsObj::setPropertyValue(
                        const rtl::OUString& /* aPropertyName */, const uno::Any& /* aValue */ )
				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
						lang::IllegalArgumentException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	//!	muss noch
}

uno::Any SAL_CALL ScSpreadsheetSettingsObj::getPropertyValue( const rtl::OUString& /* aPropertyName */ )
				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	//!	muss noch
	return uno::Any();
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettingsObj )

//------------------------------------------------------------------------

ScAnnotationsObj::ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT) :
	pDocShell( pDocSh ),
	nTab( nT )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAnnotationsObj::~ScAnnotationsObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScAnnotationsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//!	nTab bei Referenz-Update anpassen!!!

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

bool ScAnnotationsObj::GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const
{
	if (pDocShell)
	{
		sal_Int32 nFound = 0;
		ScDocument* pDoc = pDocShell->GetDocument();
		ScCellIterator aCellIter( pDoc, 0,0, nTab, MAXCOL,MAXROW, nTab );
		for( ScBaseCell* pCell = aCellIter.GetFirst(); pCell; pCell = aCellIter.GetNext() )
		{
            if (pCell->HasNote())
			{
				if (nFound == nIndex)
				{
					rPos = ScAddress( aCellIter.GetCol(), aCellIter.GetRow(), aCellIter.GetTab() );
					return true;
				}
				++nFound;
			}
		}
	}
	return false;
}

ScAnnotationObj* ScAnnotationsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
	if (pDocShell)
	{
		ScAddress aPos;
		if ( GetAddressByIndex_Impl( nIndex, aPos ) )
			return new ScAnnotationObj( pDocShell, aPos );
	}
	return NULL;
}

// XSheetAnnotations

void SAL_CALL ScAnnotationsObj::insertNew(
        const table::CellAddress& aPosition, const ::rtl::OUString& rText )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		DBG_ASSERT( aPosition.Sheet == nTab, "addAnnotation mit falschem Sheet" );
		ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, nTab );

        ScDocFunc aFunc( *pDocShell );
        aFunc.ReplaceNote( aPos, rText, 0, 0, sal_True );
	}
}

void SAL_CALL ScAnnotationsObj::removeByIndex( sal_Int32 nIndex ) throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScAddress aPos;
		if ( GetAddressByIndex_Impl( nIndex, aPos ) )
		{
			ScMarkData aMarkData;
			aMarkData.SelectTable( aPos.Tab(), sal_True );
			aMarkData.SetMultiMarkArea( ScRange(aPos) );

			ScDocFunc aFunc(*pDocShell);
			aFunc.DeleteContents( aMarkData, IDF_NOTE, sal_True, sal_True );
		}
	}
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAnnotationsObj::createEnumeration()
													throw(uno::RuntimeException)
{
	//!	iterate directly (more efficiently)?

	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.CellAnnotationsEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScAnnotationsObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_uLong nCount = 0;
	if (pDocShell)
	{
        ScCellIterator aCellIter( pDocShell->GetDocument(), 0,0, nTab, MAXCOL,MAXROW, nTab );
        for( ScBaseCell* pCell = aCellIter.GetFirst(); pCell; pCell = aCellIter.GetNext() )
            if (pCell->HasNote())
				++nCount;
	}
	return nCount;
}

uno::Any SAL_CALL ScAnnotationsObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<sheet::XSheetAnnotation> xAnnotation(GetObjectByIndex_Impl(nIndex));
	if (xAnnotation.is())
        return uno::makeAny(xAnnotation);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScAnnotationsObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<sheet::XSheetAnnotation>*)0);
}

sal_Bool SAL_CALL ScAnnotationsObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

//------------------------------------------------------------------------

ScScenariosObj::ScScenariosObj(ScDocShell* pDocSh, SCTAB nT) :
	pDocShell( pDocSh ),
	nTab	 ( nT )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScScenariosObj::~ScScenariosObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScScenariosObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	if ( rHint.ISA( ScUpdateRefHint ) )
	{
//        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

		//!	Referenz-Update fuer Tab und Start/Ende
	}
	else if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// XScenarios

sal_Bool ScScenariosObj::GetScenarioIndex_Impl( const rtl::OUString& rName, SCTAB& rIndex )
{
	//!	Case-insensitiv ????

	if ( pDocShell )
	{
		String aString(rName);

		String aTabName;
		ScDocument* pDoc = pDocShell->GetDocument();
		SCTAB nCount = (SCTAB)getCount();
		for (SCTAB i=0; i<nCount; i++)
			if (pDoc->GetName( nTab+i+1, aTabName ))
				if ( aTabName == aString )
				{
					rIndex = i;
					return sal_True;
				}
	}

	return sal_False;
}

ScTableSheetObj* ScScenariosObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
	sal_uInt16 nCount = (sal_uInt16)getCount();
	if ( pDocShell && nIndex >= 0 && nIndex < nCount )
		return new ScTableSheetObj( pDocShell, nTab+static_cast<SCTAB>(nIndex)+1 );

	return NULL;	// kein Dokument oder falscher Index
}

ScTableSheetObj* ScScenariosObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
	SCTAB nIndex;
	if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
		return new ScTableSheetObj( pDocShell, nTab+nIndex+1 );

	return NULL;	// nicht gefunden
}

void SAL_CALL ScScenariosObj::addNewByName( const rtl::OUString& aName,
								const uno::Sequence<table::CellRangeAddress>& aRanges,
								const rtl::OUString& aComment )
									throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if ( pDocShell )
	{
		ScMarkData aMarkData;
		aMarkData.SelectTable( nTab, sal_True );

		sal_uInt16 nRangeCount = (sal_uInt16)aRanges.getLength();
		if (nRangeCount)
		{
			const table::CellRangeAddress* pAry = aRanges.getConstArray();
			for (sal_uInt16 i=0; i<nRangeCount; i++)
			{
				DBG_ASSERT( pAry[i].Sheet == nTab, "addScenario mit falscher Tab" );
				ScRange aRange( (SCCOL)pAry[i].StartColumn, (SCROW)pAry[i].StartRow, nTab,
								(SCCOL)pAry[i].EndColumn,   (SCROW)pAry[i].EndRow,   nTab );

				aMarkData.SetMultiMarkArea( aRange );
			}
		}

		String aNameStr(aName);
		String aCommStr(aComment);

		Color aColor( COL_LIGHTGRAY );	// Default
		sal_uInt16 nFlags = SC_SCENARIO_SHOWFRAME | SC_SCENARIO_PRINTFRAME | SC_SCENARIO_TWOWAY | SC_SCENARIO_PROTECT;

		pDocShell->MakeScenario( nTab, aNameStr, aCommStr, aColor, nFlags, aMarkData );
	}
}

void SAL_CALL ScScenariosObj::removeByName( const rtl::OUString& aName )
											throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCTAB nIndex;
	if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
	{
		ScDocFunc aFunc(*pDocShell);
		aFunc.DeleteTable( nTab+nIndex+1, sal_True, sal_True );
	}
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScScenariosObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.ScenariosEnumeration")));
}

// XIndexAccess

sal_Int32 SAL_CALL ScScenariosObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCTAB nCount = 0;
	if ( pDocShell )
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		if (!pDoc->IsScenario(nTab))
		{
			SCTAB nTabCount = pDoc->GetTableCount();
			SCTAB nNext = nTab + 1;
			while (nNext < nTabCount && pDoc->IsScenario(nNext))
			{
				++nCount;
				++nNext;
			}
		}
	}
	return nCount;
}

uno::Any SAL_CALL ScScenariosObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<sheet::XScenario> xScen(GetObjectByIndex_Impl(nIndex));
	if (xScen.is())
        return uno::makeAny(xScen);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScScenariosObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return getCppuType((uno::Reference<sheet::XScenario>*)0);
}

sal_Bool SAL_CALL ScScenariosObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

uno::Any SAL_CALL ScScenariosObj::getByName( const rtl::OUString& aName )
			throw(container::NoSuchElementException,
					lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<sheet::XScenario> xScen(GetObjectByName_Impl(aName));
	if (xScen.is())
        return uno::makeAny(xScen);
	else
		throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScScenariosObj::getElementNames()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCTAB nCount = (SCTAB)getCount();
	uno::Sequence<rtl::OUString> aSeq(nCount);

	if ( pDocShell )	// sonst ist auch Count = 0
	{
		String aTabName;
		ScDocument* pDoc = pDocShell->GetDocument();
		rtl::OUString* pAry = aSeq.getArray();
		for (SCTAB i=0; i<nCount; i++)
			if (pDoc->GetName( nTab+i+1, aTabName ))
				pAry[i] = aTabName;
	}

	return aSeq;
}

sal_Bool SAL_CALL ScScenariosObj::hasByName( const rtl::OUString& aName )
										throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	SCTAB nIndex;
	return GetScenarioIndex_Impl( aName, nIndex );
}





