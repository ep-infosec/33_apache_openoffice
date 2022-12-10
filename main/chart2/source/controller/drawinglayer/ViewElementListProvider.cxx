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
#include "precompiled_chart2.hxx"
#include "ViewElementListProvider.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "chartview/DataPointSymbolSupplier.hxx"
#include "macros.hxx"
#include "DrawViewWrapper.hxx"
#include <svx/xtable.hxx>
#include <svx/XPropertyTable.hxx>
#include <svx/unofill.hxx>
#include <svx/unoapi.hxx>

// header for class NameOrIndex
#include <svx/xit.hxx>
// header for class XFillBitmapItem
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
// header for class XLineStartItem
#include <svx/xlnstit.hxx>
// header for class XLineEndItem
#include <svx/xlnedit.hxx>

//------------
//oldChartModelWrapper

// header for class SfxItemPool
#include <svl/itempool.hxx>
// header for class FontList
#include <svtools/ctrltool.hxx>
// header for class Application
#include <vcl/svapp.hxx>
//------------
// header for class SdrObject
#include <svx/svdobj.hxx>


//---------------
//for creation of a symbol Graphic
// header for class VirtualDevice
#include <vcl/virdev.hxx>
// header for class SdrView
#include <svx/svdview.hxx>
//---------------

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

ViewElementListProvider::ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper )
                        : m_pDrawModelWrapper( pDrawModelWrapper )
                        , m_pFontList(NULL)
{
}

ViewElementListProvider::~ViewElementListProvider()
{
    if(m_pFontList)
        delete m_pFontList;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

XColorListSharedPtr ViewElementListProvider::GetColorTable() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetColorTableFromSdrModel();
    return XColorListSharedPtr();
}
XDashListSharedPtr ViewElementListProvider::GetDashList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetDashListFromSdrModel();
    return XDashListSharedPtr();
}
XLineEndListSharedPtr ViewElementListProvider::GetLineEndList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetLineEndListFromSdrModel();
    return XLineEndListSharedPtr();
}
XGradientListSharedPtr ViewElementListProvider::GetGradientList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetGradientListFromSdrModel();
    return XGradientListSharedPtr();
}
XHatchListSharedPtr ViewElementListProvider::GetHatchList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetHatchListFromSdrModel();
    return XHatchListSharedPtr();
}
XBitmapListSharedPtr ViewElementListProvider::GetBitmapList() const
{
    if(m_pDrawModelWrapper)
        return m_pDrawModelWrapper->GetBitmapListFromSdrModel();
    return XBitmapListSharedPtr();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//create chartspecific symbols for linecharts
SdrObjList* ViewElementListProvider::GetSymbolList() const
{
    SdrObjList* m_pSymbolList = NULL;
    uno::Reference< drawing::XShapes > m_xSymbols(NULL);//@todo this keeps the first drawinglayer alive ...
    try
    {
        if(!m_pSymbolList || !m_pSymbolList->GetObjCount())
        {
            //@todo use mutex

            //get shape factory
	        uno::Reference< lang::XMultiServiceFactory > xShapeFactory( m_pDrawModelWrapper->getShapeFactory() );

            //get hidden draw page (target):
	        uno::Reference<drawing::XShapes> xTarget( m_pDrawModelWrapper->getHiddenDrawPage(), uno::UNO_QUERY );

            //create symbols via uno and convert to native sdr objects
            drawing::Direction3D aSymbolSize(220,220,0); // should be 250, but 250 -> 280 ??
            m_xSymbols =  DataPointSymbolSupplier::create2DSymbolList( xShapeFactory, xTarget, aSymbolSize );

            SdrObject* pSdrObject = DrawViewWrapper::getSdrObject( uno::Reference< drawing::XShape >( m_xSymbols, uno::UNO_QUERY ) );
            if(pSdrObject)
                m_pSymbolList = pSdrObject->GetSubList();
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
    return m_pSymbolList;
}

Graphic ViewElementListProvider::GetSymbolGraphic( sal_Int32 nStandardSymbol, const SfxItemSet* pSymbolShapeProperties ) const
{
    SdrObjList* pSymbolList = this->GetSymbolList();
    if( !pSymbolList->GetObjCount() )
        return Graphic();
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    if( nStandardSymbol >= static_cast<sal_Int32>(pSymbolList->GetObjCount()) )
        nStandardSymbol %= pSymbolList->GetObjCount();
    SdrObject* pObj = pSymbolList->GetObj(nStandardSymbol);
	
	VirtualDevice aVDev;
	aVDev.SetMapMode(MapMode(MAP_100TH_MM));
	SdrModel* pModel = new SdrModel();
	pModel->GetItemPool().FreezeIdRanges();
	SdrPage* pPage = new SdrPage( *pModel, sal_False );
	pPage->SetSize(Size(1000,1000));
	pModel->InsertPage( pPage, 0 );
	SdrView* pView = new SdrView( pModel, &aVDev );
	pView->hideMarkHandles();
	SdrPageView* pPageView = pView->ShowSdrPage(pPage);

    pObj=pObj->Clone();
	pPage->NbcInsertObject(pObj);
	pView->MarkObj(pObj,pPageView);
    if( pSymbolShapeProperties )
        pObj->SetMergedItemSet(*pSymbolShapeProperties);

	GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());

	Graphic aGraph(aMeta);
    Size aSize = pObj->GetSnapRect().GetSize();
	aGraph.SetPrefSize(aSize);
	aGraph.SetPrefMapMode(MAP_100TH_MM);

	pView->UnmarkAll();
	pObj=pPage->RemoveObject(0);
    SdrObject::Free( pObj );
	delete pView;
	delete pModel;

	return aGraph;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

FontList* ViewElementListProvider::getFontList() const
{
    //was old chart:
    //SvxFontListItem* SfxObjectShell::.GetItem(SID_ATTR_CHAR_FONTLIST)

    if(!m_pFontList)
    {
        OutputDevice* pRefDev    = m_pDrawModelWrapper ? m_pDrawModelWrapper->getReferenceDevice() : NULL;
        OutputDevice* pDefaultOut = Application::GetDefaultDevice();	// #67730#
        m_pFontList = new FontList( pRefDev ? pRefDev    : pDefaultOut
                                , pRefDev ? pDefaultOut : NULL
                                , sal_False );
    }
    return m_pFontList;
}

/*
SfxPrinter* ObjectPropertiesDialogParameter::getPrinter()
{
    //was old chart:
    //SfxPrinter* SchChartDocShell::GetPrinter()

    // OLE-Objekt: kein Printer anlegen ??? see old chart: :UpdateTablePointers
    //@todo get printer from calc or other container
    //return NULL;

    SfxPrinter* pPrinter = NULL;
    bool bOwnPrinter = true;
	if (!pPrinter)
	{
		SfxBoolItem aItem(SID_PRINTER_NOTFOUND_WARN, sal_True);
		// ItemSet mit speziellem Poolbereich anlegen
		SfxItemSet* pSet = new SfxItemSet(GetPool(),
										  SID_PRINTER_NOTFOUND_WARN,
										  SID_PRINTER_NOTFOUND_WARN, 0);
		pSet->Put(aItem);
		pPrinter = new SfxPrinter(pSet); //@todo ->need to remember and delete
		bOwnPrinter = sal_True;

		MapMode aMapMode = pPrinter->GetMapMode();
		aMapMode.SetMapUnit(MAP_100TH_MM);
		pPrinter->SetMapMode(aMapMode);

		if (pChDoc)
		{
			if (pPrinter != pChDoc->GetRefDevice())
				pChDoc->SetRefDevice(pPrinter);

			if (pPrinter != pChDoc->GetOutliner()->GetRefDevice())
				pChDoc->GetOutliner()->SetRefDevice(pPrinter);
		}
	}
	return pPrinter;
}
*/

//.............................................................................
} //namespace chart
//.............................................................................
