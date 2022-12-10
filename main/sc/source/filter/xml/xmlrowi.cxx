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



// INCLUDE ---------------------------------------------------------------

#include "xmlrowi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "global.hxx"
#include "xmlstyli.hxx"
#include "document.hxx"
#include "docuno.hxx"
#include "olinetab.hxx"
#include "sheetdata.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>

#include <com/sun/star/table/CellAddress.hpp>

#define SC_ISVISIBLE "IsVisible"
#define SC_OPTIMALHEIGHT "OptimalHeight"
#define SC_ISFILTERED "IsFiltered"

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableRowContext::ScXMLTableRowContext( ScXMLImport& rImport,
									  sal_uInt16 nPrfx,
									  const ::rtl::OUString& rLName,
									  const ::com::sun::star::uno::Reference<
									  ::com::sun::star::xml::sax::XAttributeList>& xAttrList ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	sVisibility(GetXMLToken(XML_VISIBLE)),
	nRepeatedRows(1),
    bHasCell(sal_False)
{
	rtl::OUString sCellStyleName;
	sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
	const SvXMLTokenMap& rAttrTokenMap(GetScImport().GetTableRowAttrTokenMap());
	for( sal_Int16 i=0; i < nAttrCount; ++i )
	{
		const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
		rtl::OUString aLocalName;
		sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
											sAttrName, &aLocalName ));
		const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

		switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
		{
			case XML_TOK_TABLE_ROW_ATTR_STYLE_NAME:
			{
				sStyleName = sValue;
			}
			break;
			case XML_TOK_TABLE_ROW_ATTR_VISIBILITY:
			{
				sVisibility = sValue;
			}
			break;
			case XML_TOK_TABLE_ROW_ATTR_REPEATED:
			{
                nRepeatedRows = std::max( sValue.toInt32(), (sal_Int32) 1 );
			}
			break;
			case XML_TOK_TABLE_ROW_ATTR_DEFAULT_CELL_STYLE_NAME:
			{
				sCellStyleName = sValue;
			}
			break;
			/*case XML_TOK_TABLE_ROW_ATTR_USE_OPTIMAL_HEIGHT:
			{
				sOptimalHeight = sValue;
			}
			break;*/
		}
	}
	GetScImport().GetTables().AddRow();
	GetScImport().GetTables().SetRowStyle(sCellStyleName);
}

ScXMLTableRowContext::~ScXMLTableRowContext()
{
}

SvXMLImportContext *ScXMLTableRowContext::CreateChildContext( sal_uInt16 nPrefix,
											const ::rtl::OUString& rLName,
											const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext *pContext(0);

	const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowElemTokenMap());
	switch( rTokenMap.Get( nPrefix, rLName ) )
	{
	case XML_TOK_TABLE_ROW_CELL:
//		if( IsInsertCellPossible() )
        {
            bHasCell = sal_True;
			pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
													  rLName, xAttrList, sal_False, nRepeatedRows
													  //this
													  );
        }
		break;
	case XML_TOK_TABLE_ROW_COVERED_CELL:
//		if( IsInsertCellPossible() )
        {
            bHasCell = sal_True;
			pContext = new ScXMLTableRowCellContext( GetScImport(), nPrefix,
													  rLName, xAttrList, sal_True, nRepeatedRows
													  //this
													  );
        }
		break;
	}

	if( !pContext )
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

	return pContext;
}

void ScXMLTableRowContext::EndElement()
{
	ScXMLImport& rXMLImport(GetScImport());
    if (!bHasCell && nRepeatedRows > 1)
    {
        for (sal_Int32 i = 0; i < nRepeatedRows - 1; ++i) //one row is always added
    	    GetScImport().GetTables().AddRow();
        DBG_ERRORFILE("it seems here is a nonvalid file; possible missing of table:table-cell element");
    }
    sal_Int32 nSheet = rXMLImport.GetTables().GetCurrentSheet();
	sal_Int32 nCurrentRow(rXMLImport.GetTables().GetCurrentRow());
	uno::Reference<sheet::XSpreadsheet> xSheet(rXMLImport.GetTables().GetCurrentXSheet());
    ScDocument* pDoc = rXMLImport.GetDocument();
	if(xSheet.is())
	{
		sal_Int32 nFirstRow(nCurrentRow - nRepeatedRows + 1);
		if (nFirstRow > MAXROW)
			nFirstRow = MAXROW;
		if (nCurrentRow > MAXROW)
			nCurrentRow = MAXROW;
		uno::Reference <table::XCellRange> xCellRange(xSheet->getCellRangeByPosition(0, nFirstRow, 0, nCurrentRow));
		if (xCellRange.is())
		{
			uno::Reference<table::XColumnRowRange> xColumnRowRange (xCellRange, uno::UNO_QUERY);
			if (xColumnRowRange.is())
			{
				uno::Reference <beans::XPropertySet> xRowProperties(xColumnRowRange->getRows(), uno::UNO_QUERY);
				if (xRowProperties.is())
				{
					if (sStyleName.getLength())
					{
						XMLTableStylesContext *pStyles((XMLTableStylesContext *)rXMLImport.GetAutoStyles());
                        if ( pStyles )
                        {
                            XMLTableStyleContext* pStyle((XMLTableStyleContext *)pStyles->FindStyleChildContext(
                                XML_STYLE_FAMILY_TABLE_ROW, sStyleName, sal_True));
                            if (pStyle)
                            {
                                pStyle->FillPropertySet(xRowProperties);

                                if ( nSheet != pStyle->GetLastSheet() )
                                {
                                    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
                                    pSheetData->AddRowStyle( sStyleName, ScAddress( 0, (SCROW)nFirstRow, (SCTAB)nSheet ) );
                                    pStyle->SetLastSheet(nSheet);
                                }
                            }
                        }
					}
					sal_Bool bVisible (sal_True);
					sal_Bool bFiltered (sal_False);
					if (IsXMLToken(sVisibility, XML_COLLAPSE))
					{
						bVisible = sal_False;
					}
					else if (IsXMLToken(sVisibility, XML_FILTER))
					{
						bVisible = sal_False;
						bFiltered = sal_True;
					}

                    // #i116164# call SetRowHidden/SetRowFiltered directly, so the tree doesn't have to be rebuilt
                    // to compare with existing hidden flags.
                    if (!bVisible && pDoc)
                        pDoc->SetRowHidden((SCROW)nFirstRow, (SCROW)nCurrentRow, (SCTAB)nSheet, true);
                    if (bFiltered && pDoc)
                        pDoc->SetRowFiltered((SCROW)nFirstRow, (SCROW)nCurrentRow, (SCTAB)nSheet, true);

                    //if (!bVisible)
                    //    xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISVISIBLE)), uno::makeAny(bVisible));
                    //if (bFiltered)
                    //    xRowProperties->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ISFILTERED)), uno::makeAny(bFiltered));
				}
			}
		}
	}
}

ScXMLTableRowsContext::ScXMLTableRowsContext( ScXMLImport& rImport,
									  sal_uInt16 nPrfx,
									  const ::rtl::OUString& rLName,
									  const ::com::sun::star::uno::Reference<
									  ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
									  const sal_Bool bTempHeader, const sal_Bool bTempGroup ) :
	SvXMLImportContext( rImport, nPrfx, rLName ),
	nHeaderStartRow(0),
	nHeaderEndRow(0),
	nGroupStartRow(0),
	nGroupEndRow(0),
	bHeader(bTempHeader),
	bGroup(bTempGroup),
	bGroupDisplay(sal_True)
{
	// don't have any attributes
	if (bHeader)
	{
		nHeaderStartRow = rImport.GetTables().GetCurrentRow();
		++nHeaderStartRow;
	}
	else if (bGroup)
	{
		nGroupStartRow = rImport.GetTables().GetCurrentRow();
		++nGroupStartRow;
		sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
		for( sal_Int16 i=0; i < nAttrCount; ++i )
		{
			const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
			rtl::OUString aLocalName;
			sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
												sAttrName, &aLocalName ));
			const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

            if ((nPrefix == XML_NAMESPACE_TABLE) && IsXMLToken(aLocalName, XML_DISPLAY))
				bGroupDisplay = IsXMLToken(sValue, XML_TRUE);
		}
	}
}

ScXMLTableRowsContext::~ScXMLTableRowsContext()
{
}

SvXMLImportContext *ScXMLTableRowsContext::CreateChildContext( sal_uInt16 nPrefix,
											const ::rtl::OUString& rLName,
											const ::com::sun::star::uno::Reference<
									  	::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
	SvXMLImportContext *pContext(0);

	const SvXMLTokenMap& rTokenMap(GetScImport().GetTableRowsElemTokenMap());
	switch( rTokenMap.Get( nPrefix, rLName ) )
	{
	case XML_TOK_TABLE_ROWS_ROW_GROUP:
		pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
												   rLName, xAttrList,
												   sal_False, sal_True );
		break;
	case XML_TOK_TABLE_ROWS_HEADER_ROWS:
		pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
												   rLName, xAttrList,
												   sal_True, sal_False );
		break;
	case XML_TOK_TABLE_ROWS_ROWS:
		pContext = new ScXMLTableRowsContext( GetScImport(), nPrefix,
												   rLName, xAttrList,
												   sal_False, sal_False );
		break;
	case XML_TOK_TABLE_ROWS_ROW:
			pContext = new ScXMLTableRowContext( GetScImport(), nPrefix,
													  rLName, xAttrList//,
													  //this
													  );
		break;
	}

	if( !pContext )
		pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

	return pContext;
}

void ScXMLTableRowsContext::EndElement()
{
	ScXMLImport& rXMLImport(GetScImport());
	if (bHeader)
	{
		nHeaderEndRow = rXMLImport.GetTables().GetCurrentRow();
		if (nHeaderStartRow <= nHeaderEndRow)
		{
			uno::Reference <sheet::XPrintAreas> xPrintAreas (rXMLImport.GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
			if (xPrintAreas.is())
			{
				if (!xPrintAreas->getPrintTitleRows())
				{
					xPrintAreas->setPrintTitleRows(sal_True);
					table::CellRangeAddress aRowHeaderRange;
					aRowHeaderRange.StartRow = nHeaderStartRow;
					aRowHeaderRange.EndRow = nHeaderEndRow;
					xPrintAreas->setTitleRows(aRowHeaderRange);
				}
				else
				{
					table::CellRangeAddress aRowHeaderRange(xPrintAreas->getTitleRows());
					aRowHeaderRange.EndRow = nHeaderEndRow;
					xPrintAreas->setTitleRows(aRowHeaderRange);
				}
			}
		}
	}
	else if (bGroup)
	{
		nGroupEndRow = rXMLImport.GetTables().GetCurrentRow();
		sal_Int32 nSheet(rXMLImport.GetTables().GetCurrentSheet());
		if (nGroupStartRow <= nGroupEndRow)
		{
			ScDocument* pDoc(GetScImport().GetDocument());
			if (pDoc)
			{
				GetScImport().LockSolarMutex();
				ScOutlineTable* pOutlineTable(pDoc->GetOutlineTable(static_cast<SCTAB>(nSheet), sal_True));
				ScOutlineArray* pRowArray(pOutlineTable->GetRowArray());
				sal_Bool bResized;
				pRowArray->Insert(static_cast<SCROW>(nGroupStartRow), static_cast<SCROW>(nGroupEndRow), bResized, !bGroupDisplay, sal_True);
				GetScImport().UnlockSolarMutex();
			}
		}
	}
}
