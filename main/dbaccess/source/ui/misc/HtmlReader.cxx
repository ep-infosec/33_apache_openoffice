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
#include "precompiled_dbui.hxx"
#include "HtmlReader.hxx"
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/tenccvt.hxx>
#include <comphelper/extract.hxx>
#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include <sfx2/sfxhtml.hxx>
#include <tools/debug.hxx>
#include <tools/tenccvt.hxx>
#include "moduledbu.hxx"
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <tools/color.hxx>
#include "WCopyTable.hxx"
#include "WExtendPages.hxx"
#include "WNameMatch.hxx"
#include "WColumnSelect.hxx"
#include "QEnumTypes.hxx"
#include "WCPage.hxx"
#include <tools/inetmime.hxx>
#include <svl/inettype.hxx>
#include <rtl/tencinfo.h>
#include "UITools.hxx"
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

#define DBAUI_HTML_FONTSIZES	8		// wie Export, HTML-Options
#define HTML_META_NONE			0
#define HTML_META_AUTHOR		1
#define HTML_META_DESCRIPTION	2
#define HTML_META_KEYWORDS		3
#define HTML_META_REFRESH		4
#define HTML_META_CLASSIFICATION 5
#define HTML_META_CREATED		6
#define HTML_META_CHANGEDBY		7
#define HTML_META_CHANGED		8
#define HTML_META_GENERATOR		9
#define HTML_META_SDFOOTNOTE	10
#define HTML_META_SDENDNOTE		11
#define HTML_META_CONTENT_TYPE	12

// ==========================================================================
DBG_NAME(OHTMLReader)
// ==========================================================================
// OHTMLReader
// ==========================================================================
OHTMLReader::OHTMLReader(SvStream& rIn,const SharedConnection& _rxConnection,
						const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
						const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
						const TColumnVector* pList,
						const OTypeInfoMap* _pInfoMap) 
	:HTMLParser(rIn)
	,ODatabaseExport( _rxConnection, _rxNumberF, _rM, pList, _pInfoMap, rIn )
	,m_nTableCount(0)
	,m_nColumnWidth(87)
	,m_bMetaOptions(sal_False)
	,m_bSDNum(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::OHTMLReader" );
	DBG_CTOR(OHTMLReader,NULL);
	SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( sal_True );
}
// ---------------------------------------------------------------------------
OHTMLReader::OHTMLReader(SvStream& rIn,
						 sal_Int32 nRows,
						 const TPositions &_rColumnPositions,
						 const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
						 const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
						 const TColumnVector* pList,
						 const OTypeInfoMap* _pInfoMap,
                         sal_Bool _bAutoIncrementEnabled)
	:HTMLParser(rIn)
	,ODatabaseExport( nRows, _rColumnPositions, _rxNumberF, _rM, pList, _pInfoMap, _bAutoIncrementEnabled, rIn )
	,m_nTableCount(0)
	,m_nColumnWidth(87)
	,m_bMetaOptions(sal_False)
	,m_bSDNum(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::OHTMLReader" );
	DBG_CTOR(OHTMLReader,NULL);
	SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );
    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( sal_True );
}
// ---------------------------------------------------------------------------
OHTMLReader::~OHTMLReader()
{
	DBG_DTOR(OHTMLReader,NULL);
}
// ---------------------------------------------------------------------------
SvParserState OHTMLReader::CallParser()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::CallParser" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	rInput.Seek(STREAM_SEEK_TO_BEGIN);
	rInput.ResetError();
	SvParserState  eParseState = HTMLParser::CallParser();
	SetColumnTypes(m_pColumnList,m_pInfoMap);
	return m_bFoundTable ? eParseState : SVPAR_ERROR;
}
// -----------------------------------------------------------------------------
void OHTMLReader::NextToken( int nToken )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::NextToken" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	if(m_bError || !m_nRows) // falls Fehler oder keine Rows mehr zur "Uberpr"ufung dann gleich zur"uck
		return;
	if ( nToken ==  HTML_META )
		setTextEncoding();

	if(m_xConnection.is())	  // gibt an welcher CTOR gerufen wurde und damit, ob eine Tabelle erstellt werden soll
	{
		switch(nToken)
		{
			case HTML_TABLE_ON:
				++m_nTableCount;
				{	// es kann auch TD oder TH sein, wenn es vorher kein TABLE gab
                    const HTMLOptions* pHtmlOptions = GetOptions();
                    sal_Int16 nArrLen = pHtmlOptions->Count();
					for ( sal_Int16 i = 0; i < nArrLen; i++ )
					{
                        const HTMLOption* pOption = (*pHtmlOptions)[i];
						switch( pOption->GetToken() )
						{
							case HTML_O_WIDTH:
							{	// Prozent: von Dokumentbreite bzw. aeusserer Zelle
								m_nColumnWidth = GetWidthPixel( pOption );
							}
							break;
						}
					}
				}
			case HTML_THEAD_ON:
			case HTML_TBODY_ON:
                {
                    sal_uInt32 nTell = rInput.Tell(); // ver?ndert vielleicht die Position des Streams
				    if ( !m_xTable.is() ) 
                    {// erste Zeile als Header verwenden
					    m_bError = !CreateTable(nToken);
                        if ( m_bAppendFirstLine )
                            rInput.Seek(nTell);
                    }
                }
				break;
			case HTML_TABLE_OFF:
				if(!--m_nTableCount)
				{
					m_xTable = NULL;
				}
				break;
			case HTML_TABLEROW_ON:
				if ( m_pUpdateHelper.get() )
				{
					try
					{
						m_pUpdateHelper->moveToInsertRow(); // sonst neue Zeile anh"angen
					}
					catch(SQLException& e)
					// UpdateFehlerbehandlung
					{
						showErrorDialog(e);
					}
				}
				else
					m_bError = sal_True;
				break;
			case HTML_TEXTTOKEN:
			case HTML_SINGLECHAR:
				if ( m_bInTbl ) //&& !m_bSDNum ) // wichtig, da wir sonst auch die Namen der Fonts bekommen
				    m_sTextToken += aToken;
				break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
			case HTML_TABLEDATA_ON:
                fetchOptions();
				break;
			case HTML_TABLEDATA_OFF:
				{
                    if ( m_sCurrent.Len() )
                        m_sTextToken = m_sCurrent;
					try
					{
						insertValueIntoColumn();
					}
					catch(SQLException& e)
					// UpdateFehlerbehandlung
					{
						showErrorDialog(e);
					}
                    m_sCurrent.Erase();
					m_nColumnPos++;
                    eraseTokens();
					m_bSDNum = m_bInTbl = sal_False;
				}
				break;
			case HTML_TABLEROW_OFF:
				if ( !m_pUpdateHelper.get() )
				{
					m_bError = sal_True;
					break;
				}
				try
				{
					m_nRowCount++;
					if (m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
						m_pUpdateHelper->updateInt(1,m_nRowCount);
					m_pUpdateHelper->insertRow();
				}
				catch(SQLException& e)
				//////////////////////////////////////////////////////////////////////
				// UpdateFehlerbehandlung
				{
					showErrorDialog(e);
				}
				m_nColumnPos = 0;
				break;
		}
	}
	else // Zweig nur f"ur Typpr"ufung g"ultig
	{
		switch(nToken)
		{
			case HTML_THEAD_ON:
			case HTML_TBODY_ON:
				// Der Spalten Kopf z"ahlt nicht mit
				if(m_bHead)
				{
					do
					{}
					while(GetNextToken() != HTML_TABLEROW_OFF);
					m_bHead = sal_False;
				}
				break;
			case HTML_TABLEDATA_ON:
			case HTML_TABLEHEADER_ON:
				fetchOptions();
				break;
			case HTML_TEXTTOKEN:
			case HTML_SINGLECHAR:
                if ( m_bInTbl ) // && !m_bSDNum ) // wichtig, da wir sonst auch die Namen der Fonts bekommen
				    m_sTextToken += aToken;
				break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += m_sTextToken;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
			case HTML_TABLEDATA_OFF:
                if ( m_sCurrent.Len() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
				m_nColumnPos++;
				m_bSDNum = m_bInTbl = sal_False;
                m_sCurrent.Erase();
				break;
			case HTML_TABLEROW_OFF:
                if ( m_sCurrent.Len() )
                    m_sTextToken = m_sCurrent;
                adjustFormat();
				m_nColumnPos = 0;
				m_nRows--;
                m_sCurrent.Erase();
				break;
		}
	}
}
// -----------------------------------------------------------------------------
void OHTMLReader::fetchOptions()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::fetchOptions" );
	m_bInTbl = sal_True;
	const HTMLOptions* options = GetOptions();
	sal_Int16 nArrLen = options->Count();
	for ( sal_Int16 i = 0; i < nArrLen; i++ )
	{
		const HTMLOption* pOption = (*options)[i];
		switch( pOption->GetToken() )
		{
			case HTML_O_SDVAL:
			{
                m_sValToken = pOption->GetString();
				//m_sTextToken = pOption->GetString();
				m_bSDNum = sal_True;
			}
			break;
			case HTML_O_SDNUM:
                m_sNumToken = pOption->GetString();
			break;
		}
	}
}
//---------------------------------------------------------------------------------
void OHTMLReader::TableDataOn(SvxCellHorJustify& eVal,int nToken)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::TableDataOn" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	sal_Bool bHorJustifyCenterTH = (nToken == HTML_TABLEHEADER_ON);
	const HTMLOptions* pHtmlOptions = GetOptions();
	sal_Int16 nArrLen = pHtmlOptions->Count();
	for ( sal_Int16 i = 0; i < nArrLen; i++ )
	{
		const HTMLOption* pOption = (*pHtmlOptions)[i];
		switch( pOption->GetToken() )
		{
			case HTML_O_ALIGN:
			{
				bHorJustifyCenterTH = sal_False;
				const String& rOptVal = pOption->GetString();
				if (rOptVal.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_right ))
					eVal = SVX_HOR_JUSTIFY_RIGHT;
				else if (rOptVal.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_center ))
					eVal = SVX_HOR_JUSTIFY_CENTER;
				else if (rOptVal.EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_AL_left ))
					eVal = SVX_HOR_JUSTIFY_LEFT;
				else
					eVal = SVX_HOR_JUSTIFY_STANDARD;
			}
			break;
			case HTML_O_WIDTH:
				m_nWidth = GetWidthPixel( pOption );
			break;
		}
	}
}

//---------------------------------------------------------------------------------
void OHTMLReader::TableFontOn(FontDescriptor& _rFont,sal_Int32 &_rTextColor)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::TableFontOn" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	const HTMLOptions* pHtmlOptions = GetOptions();
	sal_Int16 nArrLen = pHtmlOptions->Count();
	for ( sal_Int16 i = 0; i < nArrLen; i++ )
	{
		const HTMLOption* pOption = (*pHtmlOptions)[i];
		switch( pOption->GetToken() )
		{
		case HTML_O_COLOR:
			{
				Color aColor;
				pOption->GetColor( aColor );
				_rTextColor = aColor.GetRGBColor();
			}
			break;
		case HTML_O_FACE :
			{
				const String& rFace = pOption->GetString();
				String aFontName;
				xub_StrLen nPos = 0;
				while( nPos != STRING_NOTFOUND )
				{	// Fontliste, VCL: Semikolon als Separator, HTML: Komma
					String aFName = rFace.GetToken( 0, ',', nPos );
					aFName.EraseTrailingChars().EraseLeadingChars();
					if( aFontName.Len() )
						aFontName += ';';
					aFontName += aFName;
				}
				if ( aFontName.Len() )
					_rFont.Name = ::rtl::OUString(aFontName);
			}
			break;
		case HTML_O_SIZE :
			{
				sal_Int16 nSize = (sal_Int16) pOption->GetNumber();
				if ( nSize == 0 )
					nSize = 1;
				else if ( nSize < DBAUI_HTML_FONTSIZES )
					nSize = DBAUI_HTML_FONTSIZES;

				_rFont.Height = nSize;
			}
			break;
		}
	}
}
// ---------------------------------------------------------------------------
sal_Int16 OHTMLReader::GetWidthPixel( const HTMLOption* pOption )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::GetWidthPixel" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	const String& rOptVal = pOption->GetString();
	if ( rOptVal.Search('%') != STRING_NOTFOUND )
	{	// Prozent
		DBG_ASSERT( m_nColumnWidth, "WIDTH Option: m_nColumnWidth==0 und Width%" );
		return (sal_Int16)((pOption->GetNumber() * m_nColumnWidth) / 100);
	}
	else
	{
		if ( rOptVal.Search('*') != STRING_NOTFOUND )
		{	// relativ zu was?!?
//2do: ColArray aller relativen Werte sammeln und dann MakeCol
			return 0;
		}
		else
			return (sal_Int16)pOption->GetNumber();	// Pixel
	}
}
// ---------------------------------------------------------------------------
sal_Bool OHTMLReader::CreateTable(int nToken)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::CreateTable" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	String aTempName(ModuleRes(STR_TBL_TITLE));
	aTempName = aTempName.GetToken(0,' ');
	aTempName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTempName )));

	int nTmpToken2 = nToken;
	sal_Bool bCaption = sal_False;
	sal_Bool bTableHeader = sal_False;
	String aColumnName;
	SvxCellHorJustify eVal;

	String aTableName;
	FontDescriptor aFont = ::dbaui::CreateFontDescriptor(Application::GetSettings().GetStyleSettings().GetAppFont());
	sal_Int32 nTextColor = 0;
	do
	{
		switch(nTmpToken2)
		{
			case HTML_TEXTTOKEN:
			case HTML_SINGLECHAR:
				if(bTableHeader)
					aColumnName += aToken;
				if(bCaption)
					aTableName += aToken;
				break;
            case HTML_PARABREAK_OFF:
                m_sCurrent += aColumnName;
                break;
            case HTML_PARABREAK_ON:
                m_sTextToken.Erase();
                break;
			case HTML_TABLEDATA_ON:
                // m_bAppendFirstLine = true;
                // run through
			case HTML_TABLEHEADER_ON:
				TableDataOn(eVal,nTmpToken2);
				bTableHeader = sal_True;
				break;
			case HTML_TABLEDATA_OFF:
                // m_bAppendFirstLine = true;
                // run through
			case HTML_TABLEHEADER_OFF:
				{
					aColumnName.EraseLeadingChars();
					aColumnName.EraseTrailingChars();
					if (!aColumnName.Len() || m_bAppendFirstLine )
						aColumnName = String(ModuleRes(STR_COLUMN_NAME));
                    else if ( m_sCurrent.Len() )
                        aColumnName = m_sCurrent;

                    aColumnName.EraseLeadingChars();
					aColumnName.EraseTrailingChars();
					CreateDefaultColumn(aColumnName);
					aColumnName.Erase();
                    m_sCurrent.Erase();

					eVal = SVX_HOR_JUSTIFY_STANDARD;
					bTableHeader = sal_False;
				}
				break;

			case HTML_TITLE_ON:
			case HTML_CAPTION_ON:
				bCaption = sal_True;
				break;
			case HTML_TITLE_OFF:
			case HTML_CAPTION_OFF:
				aTableName.EraseLeadingChars();
				aTableName.EraseTrailingChars();
				if(!aTableName.Len())
					aTableName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTableName)));
				else
					aTableName = aTempName;
				bCaption = sal_False;
				break;
			case HTML_FONT_ON:
				TableFontOn(aFont,nTextColor);
				break;
			case HTML_BOLD_ON:
				aFont.Weight = ::com::sun::star::awt::FontWeight::BOLD;
				break;
			case HTML_ITALIC_ON:
				aFont.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
				break;
			case HTML_UNDERLINE_ON:
				aFont.Underline = ::com::sun::star::awt::FontUnderline::SINGLE;
				break;
			case HTML_STRIKE_ON:
				aFont.Strikeout = ::com::sun::star::awt::FontStrikeout::SINGLE;
				break;
		}
	}
	while((nTmpToken2 = GetNextToken()) != HTML_TABLEROW_OFF);

    if ( m_sCurrent.Len() )
        aColumnName = m_sCurrent;
    aColumnName.EraseLeadingChars();
	aColumnName.EraseTrailingChars();
	if(aColumnName.Len())
		CreateDefaultColumn(aColumnName);

	if ( m_vDestVector.empty() )
		return sal_False;	

	if(!aTableName.Len())
		aTableName = aTempName;

	m_bInTbl		= sal_False;
	m_bFoundTable	= sal_True;

	if ( isCheckEnabled() )
		return sal_True;

	return !executeWizard(aTableName,makeAny(nTextColor),aFont) && m_xTable.is();
}
// -----------------------------------------------------------------------------
void OHTMLReader::setTextEncoding()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::setTextEncoding" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	m_bMetaOptions = sal_True;
    ParseMetaOptions(NULL, NULL);
}

// -----------------------------------------------------------------------------
void OHTMLReader::release()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::release" );
	DBG_CHKTHIS(OHTMLReader,NULL);
	ReleaseRef();
}
// -----------------------------------------------------------------------------
TypeSelectionPageFactory OHTMLReader::getTypeSelectionPageFactory()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OHTMLReader::getTypeSelectionPageFactory" );
	DBG_CHKTHIS(OHTMLReader,NULL);
    return &OWizHTMLExtend::Create;
}
// -----------------------------------------------------------------------------

