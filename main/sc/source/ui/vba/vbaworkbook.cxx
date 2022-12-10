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


#include <vbahelper/helperdecl.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/unwrapargs.hxx>

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XlFileFormat.hpp>

#include "scextopt.hxx"
#include "vbaworksheet.hxx"
#include "vbaworksheets.hxx"
#include "vbaworkbook.hxx"
#include "vbawindows.hxx"
#include "vbastyles.hxx"
#include "excelvbahelper.hxx"
#include "vbapalette.hxx"
#include <osl/file.hxx>
#include <stdio.h>
#include "vbanames.hxx"  // Amelia Wang
#include "nameuno.hxx"
#include "docoptio.hxx"
#include "unonames.hxx"

// Much of the impl. for the equivalend UNO module is
// sc/source/ui/unoobj/docuno.cxx, viewuno.cxx

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class ActiveSheet : public ScVbaWorksheet
{
protected:
	virtual uno::Reference< frame::XModel > getModel()
	{ 	
		return getCurrentExcelDoc( mxContext ); 
	}
	virtual uno::Reference< sheet::XSpreadsheet > getSheet()
	{ 
		uno::Reference< frame::XModel > xModel = getModel();
		uno::Reference< sheet::XSpreadsheet > xSheet;
		if ( xModel.is() )
		{
			uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
                        	xModel->getCurrentController(), uno::UNO_QUERY );
			if ( xSpreadsheet.is() )
				xSheet = xSpreadsheet->getActiveSheet(); 
		}
		return xSheet;
	}
public:
	ActiveSheet( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext ) : ScVbaWorksheet( xParent, xContext ) {}
		
};

uno::Sequence< sal_Int32 > ScVbaWorkbook::ColorData;

void ScVbaWorkbook::initColorData( const uno::Sequence< sal_Int32 >& sColors )
{
		const sal_Int32* pSource = sColors.getConstArray();
		sal_Int32* pDest = ColorData.getArray();
		const sal_Int32* pEnd = pSource + sColors.getLength();
		for ( ; pSource != pEnd; ++pSource, ++pDest )
			*pDest = *pSource;
}


void SAL_CALL 
ScVbaWorkbook::ResetColors(  ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
		uno::Reference< container::XIndexAccess > xIndexAccess( ScVbaPalette::getDefaultPalette(), uno::UNO_QUERY_THROW );
		sal_Int32 nLen = xIndexAccess->getCount();
		ColorData.realloc( nLen );
	
		uno::Sequence< sal_Int32 > dDefaultColors( nLen );
		sal_Int32* pDest = dDefaultColors.getArray();
		for ( sal_Int32 index=0; index < nLen; ++pDest, ++index )
			xIndexAccess->getByIndex( index )  >>= (*pDest);
		initColorData( dDefaultColors );
}

::uno::Any SAL_CALL 
ScVbaWorkbook::Colors( const ::uno::Any& Index ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
	uno::Any aRet;
	if ( Index.getValue() )
	{
		sal_Int32 nIndex = 0;
		Index >>= nIndex;
		aRet = uno::makeAny( XLRGBToOORGB( ColorData[ --nIndex ] ) );			
	}
	else
		aRet = uno::makeAny( ColorData );
	return aRet;
}

::sal_Int32 SAL_CALL 
ScVbaWorkbook::FileFormat(  ) throw (::script::BasicErrorException, ::uno::RuntimeException)
{
        sal_Int32 aFileFormat = 0;
        rtl::OUString aFilterName;
        uno::Sequence< beans::PropertyValue > aArgs = getModel()->getArgs();

		// #FIXME - seems suspect should we not walk through the properties
		// to find the FilterName
        if (aArgs[0].Name.equalsAscii( "FilterName")) {
            aArgs[0].Value >>= aFilterName;
        } else {
           aArgs[1].Value >>= aFilterName;
        }

        if (aFilterName.equalsAscii("Text - txt - csv (StarCalc)")) {
            aFileFormat = excel::XlFileFormat::xlCSV; //xlFileFormat.
        }

        if (aFilterName.equalsAscii("DBF")) {
            aFileFormat = excel::XlFileFormat::xlDBF4;
        }

        if (aFilterName.equalsAscii("DIF")) {
            aFileFormat = excel::XlFileFormat::xlDIF;
        }

        if (aFilterName.equalsAscii("Lotus")) {
            aFileFormat = excel::XlFileFormat::xlWK3;
        }

        if (aFilterName.equalsAscii("MS Excel 4.0")) {
            aFileFormat = excel::XlFileFormat::xlExcel4Workbook;
        }

        if (aFilterName.equalsAscii("MS Excel 5.0/95")) {
            aFileFormat = excel::XlFileFormat::xlExcel5;
        }

        if (aFilterName.equalsAscii("MS Excel 97")) {
            aFileFormat = excel::XlFileFormat::xlExcel9795;
        }

        if (aFilterName.equalsAscii("HTML (StarCalc)")) {
            aFileFormat = excel::XlFileFormat::xlHtml;
        }

        if (aFilterName.equalsAscii("calc_StarOffice_XML_Calc_Template")) {
            aFileFormat = excel::XlFileFormat::xlTemplate;
        }

        if (aFilterName.equalsAscii("StarOffice XML (Calc)")) {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }
        if (aFilterName.equalsAscii("calc8")) {
            aFileFormat = excel::XlFileFormat::xlWorkbookNormal;
        }

        return aFileFormat;
}

void 
ScVbaWorkbook::init()
{
	if ( !ColorData.getLength() )
		ResetColors();
}
ScVbaWorkbook::ScVbaWorkbook( 	const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext) :ScVbaWorkbook_BASE( xParent, xContext )
{
	//#FIXME this persists the color data per office instance and
	// not per workbook instance, need to hook the data into XModel
	// ( e.g. we already store the imported palette in there )
	// so we should,
	// a) make the class that does that a service
	// b) make that service implement XIndexContainer
	init();
}

ScVbaWorkbook::ScVbaWorkbook( 	const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, css::uno::Reference< css::frame::XModel > xModel ) : ScVbaWorkbook_BASE( xParent, xContext, xModel )
{ 
	init();
}

ScVbaWorkbook::ScVbaWorkbook( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) : ScVbaWorkbook_BASE( args, xContext )
{
	init();
}

uno::Reference< excel::XWorksheet >
ScVbaWorkbook::getActiveSheet() throw (uno::RuntimeException)
{
	uno::Reference< frame::XModel > xModel( getCurrentExcelDoc( mxContext ), uno::UNO_SET_THROW );
	uno::Reference< sheet::XSpreadsheetView > xView( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xView->getActiveSheet(), uno::UNO_SET_THROW );
    // #162503# return the original sheet module wrapper object, instead of a new instance
    uno::Reference< excel::XWorksheet > xWorksheet( excel::getUnoSheetModuleObj( xSheet ), uno::UNO_QUERY );
    if( xWorksheet.is() ) return xWorksheet;
    // #i116936# excel::getUnoSheetModuleObj() may return null in documents without global VBA mode enabled
    return new ScVbaWorksheet( this, mxContext, xSheet, xModel );
}

uno::Any SAL_CALL
ScVbaWorkbook::Sheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	return Worksheets( aIndex );
}

uno::Any SAL_CALL
ScVbaWorkbook::Worksheets( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	uno::Reference< frame::XModel > xModel( getModel() );	
	uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY_THROW );
	uno::Reference<container::XIndexAccess > xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
	uno::Reference< XCollection > xWorkSheets(  new ScVbaWorksheets( this, mxContext, xSheets, xModel ) );
	if (  aIndex.getValueTypeClass() == uno::TypeClass_VOID )
	{
		return uno::Any( xWorkSheets );	
	}
	// pass on to collection
	return uno::Any( xWorkSheets->Item( aIndex, uno::Any() ) );
}
uno::Any SAL_CALL
ScVbaWorkbook::Windows( const uno::Any& aIndex ) throw (uno::RuntimeException)
{

	uno::Reference< excel::XWindows >  xWindows( new ScVbaWindows( getParent(), mxContext ) );
	if ( aIndex.getValueTypeClass() == uno::TypeClass_VOID )
		return uno::Any( xWindows );
	return uno::Any( xWindows->Item( aIndex, uno::Any() ) );
}

void SAL_CALL
ScVbaWorkbook::Activate() throw (uno::RuntimeException)
{
    VbaDocumentBase::Activate();
}

::sal_Bool
ScVbaWorkbook::getProtectStructure() throw (uno::RuntimeException)
{
	uno::Reference< util::XProtectable > xProt( getModel(), uno::UNO_QUERY_THROW );
	return xProt->isProtected();
}

::sal_Bool SAL_CALL ScVbaWorkbook::getPrecisionAsDisplayed() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
    return pDoc->GetDocOptions().IsCalcAsShown();
}

void SAL_CALL ScVbaWorkbook::setPrecisionAsDisplayed( sal_Bool _precisionAsDisplayed ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetCalcAsShown( _precisionAsDisplayed );
    pDoc->SetDocOptions( aOpt );
}

void
ScVbaWorkbook::SaveCopyAs( const rtl::OUString& sFileName ) throw ( uno::RuntimeException)
{
	rtl::OUString aURL;
	osl::FileBase::getFileURLFromSystemPath( sFileName, aURL );
	uno::Reference< frame::XStorable > xStor( getModel(), uno::UNO_QUERY_THROW );
	uno::Sequence<  beans::PropertyValue > storeProps(1);
	storeProps[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
	storeProps[0].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MS Excel 97" ) );
	xStor->storeToURL( aURL, storeProps );
}

css::uno::Any SAL_CALL 
ScVbaWorkbook::Styles( const uno::Any& Item ) throw (uno::RuntimeException)
{
	// quick look and Styles object doesn't seem to have a valid parent
	// or a least the object browser just shows an object that has no 
	// variables ( therefore... leave as NULL for now ) 
	uno::Reference< XCollection > dStyles = new ScVbaStyles( uno::Reference< XHelperInterface >(), mxContext, getModel() );
	if ( Item.hasValue() )
		return dStyles->Item( Item, uno::Any() );
	return uno::makeAny( dStyles );
}

// Amelia Wang
uno::Any SAL_CALL
ScVbaWorkbook::Names( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
	uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_SET_THROW );
	uno::Reference< beans::XPropertySet > xProps( xModel, uno::UNO_QUERY_THROW );
	uno::Reference< sheet::XNamedRanges > xNamedRanges(  xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NamedRanges") ) ), uno::UNO_QUERY_THROW );
	uno::Reference< XCollection > xNames( new ScVbaNames( this, mxContext, xNamedRanges, xModel ) );
    if ( aIndex.hasValue() )
        return uno::Any( xNames->Item( aIndex, uno::Any() ) );
	return uno::Any( xNames );
}

rtl::OUString& 
ScVbaWorkbook::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaWorkbook") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
ScVbaWorkbook::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Workbook" ) );
	}
	return aServiceNames;
}

::rtl::OUString SAL_CALL
ScVbaWorkbook::getCodeName() throw (css::uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xModelProp( getModel(), uno::UNO_QUERY_THROW );
    return xModelProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CodeName" ) ) ).get< ::rtl::OUString >();
}

namespace workbook
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWorkbook, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWorkbook",
    "ooo.vba.excel.Workbook" );
}
