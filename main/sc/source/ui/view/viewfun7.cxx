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
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>



// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xoutbmp.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/embed/Aspects.hpp>

#include "document.hxx"		// fuer MapMode Initialisierung in PasteDraw
#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "drwtrans.hxx"
#include "globstr.hrc"
#include "chartlis.hxx"
#include "docuno.hxx"
#include "docsh.hxx"
#include "convuno.hxx"

extern Point aDragStartDiff;

// STATIC DATA -----------------------------------------------------------

sal_Bool bPasteIsMove = sal_False;

using namespace com::sun::star;

//==================================================================

void lcl_AdjustInsertPos( ScViewData* pData, Point& rPos, Size& rSize )
{
//	SdrPage* pPage = pData->GetDocument()->GetDrawLayer()->GetPage( pData->GetTabNo() );
	SdrPage* pPage = pData->GetScDrawView()->GetModel()->GetPage( static_cast<sal_uInt16>(pData->GetTabNo()) );
	DBG_ASSERT(pPage,"pPage ???");
	Size aPgSize( pPage->GetSize() );
	if (aPgSize.Width() < 0)
		aPgSize.Width() = -aPgSize.Width();
	long x = aPgSize.Width() - rPos.X() - rSize.Width();
	long y = aPgSize.Height() - rPos.Y() - rSize.Height();
	// ggf. Ajustments (80/200) fuer Pixel-Rundungsfehler
	if( x < 0 )
		rPos.X() += x + 80;
	if( y < 0 )
		rPos.Y() += y + 200;
	rPos.X() += rSize.Width() / 2;			// Position bei Paste gibt Mittelpunkt an
	rPos.Y() += rSize.Height() / 2;
}

void ScViewFunc::PasteDraw( const Point& rLogicPos, SdrModel* pModel,
        sal_Bool bGroup, sal_Bool bSameDocClipboard )
{
	MakeDrawLayer();
	Point aPos( rLogicPos );

	//	#64184# MapMode am Outliner-RefDevice muss stimmen (wie in FuText::MakeOutliner)
	//!	mit FuText::MakeOutliner zusammenfassen?
	MapMode aOldMapMode;
	OutputDevice* pRef = GetViewData()->GetDocument()->GetDrawLayer()->GetRefDevice();
	if (pRef)
	{
		aOldMapMode = pRef->GetMapMode();
		pRef->SetMapMode( MapMode(MAP_100TH_MM) );
	}

	sal_Bool bNegativePage = GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() );

	SdrView* pDragEditView = NULL;
	ScModule* pScMod = SC_MOD();
	const ScDragData& rData = pScMod->GetDragData();
	ScDrawTransferObj* pDrawTrans = rData.pDrawTransfer;
	if (pDrawTrans)
	{
		pDragEditView = pDrawTrans->GetDragSourceView();

		aPos -= aDragStartDiff;
		if ( bNegativePage )
		{
			if (aPos.X() > 0) aPos.X() = 0;
		}
		else
		{
			if (aPos.X() < 0) aPos.X() = 0;
		}
		if (aPos.Y() < 0) aPos.Y() = 0;
	}

	ScDrawView* pScDrawView = GetScDrawView();
	if (bGroup)
		pScDrawView->BegUndo( ScGlobal::GetRscString( STR_UNDO_PASTE ) );

	sal_Bool bSameDoc = ( pDragEditView && pDragEditView->GetModel() == pScDrawView->GetModel() );
	if (bSameDoc)
	{
			// lokal kopieren - incl. Charts

		Point aSourceStart = pDragEditView->GetAllMarkedRect().TopLeft();
		long nDiffX = aPos.X() - aSourceStart.X();
		long nDiffY = aPos.Y() - aSourceStart.Y();

			// innerhalb einer Page verschieben?

		if ( bPasteIsMove &&
				pScDrawView->GetSdrPageView()->GetPage() ==
				pDragEditView->GetSdrPageView()->GetPage() )
		{
			if ( nDiffX != 0 || nDiffY != 0 )
				pDragEditView->MoveAllMarked(Size(nDiffX,nDiffY), sal_False);
		}
		else
		{
			SdrModel* pDrawModel = pDragEditView->GetModel();
            SCTAB nTab = GetViewData()->GetTabNo();
            SdrPage* pDestPage = pDrawModel->GetPage( static_cast< sal_uInt16 >( nTab ) );
			DBG_ASSERT(pDestPage,"nanu, Page?");

            ::std::vector< ::rtl::OUString > aExcludedChartNames;
            if ( pDestPage )
            {
                ScChartHelper::GetChartNames( aExcludedChartNames, pDestPage );
            }

			SdrMarkList aMark = pDragEditView->GetMarkedObjectList();
			aMark.ForceSort();
			sal_uLong nMarkAnz=aMark.GetMarkCount();
			for (sal_uLong nm=0; nm<nMarkAnz; nm++) {
				const SdrMark* pM=aMark.GetMark(nm);
				const SdrObject* pObj=pM->GetMarkedSdrObj();

				// #116235#
				SdrObject* pNeuObj=pObj->Clone();
				//SdrObject* pNeuObj=pObj->Clone(pDestPage,pDrawModel);

				if (pNeuObj!=NULL)
				{
					pNeuObj->SetModel(pDrawModel);
					pNeuObj->SetPage(pDestPage);

					//	#68787# copy graphics within the same model - always needs new name
					if ( pNeuObj->ISA(SdrGrafObj) && !bPasteIsMove )
						pNeuObj->SetName(((ScDrawLayer*)pDrawModel)->GetNewGraphicName());

					if (nDiffX!=0 || nDiffY!=0)
						pNeuObj->NbcMove(Size(nDiffX,nDiffY));
					pDestPage->InsertObject( pNeuObj );
					pScDrawView->AddUndo(new SdrUndoInsertObj( *pNeuObj ));

					//	Chart braucht nicht mehr getrennt behandelt zu werden,
					//	weil es seine Daten jetzt selber hat
				}
			}

			if (bPasteIsMove)
				pDragEditView->DeleteMarked();

            ScDocument* pDocument = GetViewData()->GetDocument();
            ScDocShell* pDocShell = GetViewData()->GetDocShell();
            ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : NULL );
            if ( pDocument && pDestPage && pModelObj && pDrawTrans )
            {
                const ScRangeListVector& rProtectedChartRangesVector( pDrawTrans->GetProtectedChartRangesVector() );
                ScChartHelper::CreateProtectedChartListenersAndNotify( pDocument, pDestPage, pModelObj, nTab,
                    rProtectedChartRangesVector, aExcludedChartNames, bSameDoc );
            }
		}
	}
	else
	{
		bPasteIsMove = sal_False;		// kein internes Verschieben passiert

        SdrView aView(pModel);      // #i71529# never create a base class of SdrView directly!
		SdrPageView* pPv = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
		aView.MarkAllObj(pPv);
		Size aSize = aView.GetAllMarkedRect().GetSize();
		lcl_AdjustInsertPos( GetViewData(), aPos, aSize );

		//	#41333# Markierung nicht aendern, wenn Ole-Objekt aktiv
		//	(bei Drop aus Ole-Objekt wuerde sonst mitten im ExecuteDrag deaktiviert!)

		sal_uLong nOptions = 0;
		SfxInPlaceClient* pClient = GetViewData()->GetViewShell()->GetIPClient();
        if ( pClient && pClient->IsObjectInPlaceActive() )
			nOptions |= SDRINSERT_DONTMARK;

        ::std::vector< ::rtl::OUString > aExcludedChartNames;
        SCTAB nTab = GetViewData()->GetTabNo();
        SdrPage* pPage = pScDrawView->GetModel()->GetPage( static_cast< sal_uInt16 >( nTab ) );
        DBG_ASSERT( pPage, "Page?" );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        // #89247# Set flag for ScDocument::UpdateChartListeners() which is
        // called during paste.
        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( sal_True );

		pScDrawView->Paste( *pModel, aPos, NULL, nOptions );

        if ( !bSameDocClipboard )
            GetViewData()->GetDocument()->SetPastingDrawFromOtherDoc( sal_False );

		// #68991# Paste puts all objects on the active (front) layer
		// controls must be on SC_LAYER_CONTROLS
		if (pPage)
		{
			SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
			SdrObject* pObject = aIter.Next();
			while (pObject)
			{
				if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
					pObject->NbcSetLayer(SC_LAYER_CONTROLS);
				pObject = aIter.Next();
			}
		}

		// #75299# all graphics objects must have names
		GetViewData()->GetDocument()->EnsureGraphicNames();

        ScDocument* pDocument = GetViewData()->GetDocument();
        ScDocShell* pDocShell = GetViewData()->GetDocShell();
        ScModelObj* pModelObj = ( pDocShell ? ScModelObj::getImplementation( pDocShell->GetModel() ) : NULL );
        ScDrawTransferObj* pTransferObj = ScDrawTransferObj::GetOwnClipboard( NULL );
        if ( pDocument && pPage && pModelObj && ( pTransferObj || pDrawTrans ) )
        {
            const ScRangeListVector& rProtectedChartRangesVector(
                pTransferObj ? pTransferObj->GetProtectedChartRangesVector() : pDrawTrans->GetProtectedChartRangesVector() );
            ScChartHelper::CreateProtectedChartListenersAndNotify( pDocument, pPage, pModelObj, nTab,
                rProtectedChartRangesVector, aExcludedChartNames, bSameDocClipboard );
        }
	}

	if (bGroup)
	{
		pScDrawView->GroupMarked();
		pScDrawView->EndUndo();
	}

	if (pRef)
		pRef->SetMapMode( aOldMapMode );

    // GetViewData()->GetViewShell()->SetDrawShell( sal_True );
    // #99759# It is not sufficient to just set the DrawShell if we pasted, for
    // example, a chart.  SetDrawShellOrSub() would only work for D&D in the
    // same document but not if inserting from the clipboard, therefore
    // MarkListHasChanged() is what we need.
    pScDrawView->MarkListHasChanged();

}

sal_Bool ScViewFunc::PasteObject( const Point& rPos, const uno::Reference < embed::XEmbeddedObject >& xObj,
								const Size* pDescSize, const Graphic* pReplGraph, const ::rtl::OUString& aMediaType, sal_Int64 nAspect )
{
	MakeDrawLayer();
    if ( xObj.is() )
	{
        ::rtl::OUString aName;
        //TODO/MBA: is that OK?
        comphelper::EmbeddedObjectContainer& aCnt = GetViewData()->GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer();
        if ( !aCnt.HasEmbeddedObject( xObj ) )
            aCnt.InsertEmbeddedObject( xObj, aName );
        else
            aName = aCnt.GetEmbeddedObjectName( xObj );

		svt::EmbeddedObjectRef aObjRef( xObj, nAspect );
		if ( pReplGraph )
			aObjRef.SetGraphic( *pReplGraph, aMediaType );

		Size aSize;
		if ( nAspect == embed::Aspects::MSOLE_ICON )
		{
			MapMode aMapMode( MAP_100TH_MM );
			aSize = aObjRef.GetSize( &aMapMode );
		}
		else
		{
        	// working with visual area can switch object to running state
        	MapUnit aMapObj = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nAspect ) );
        	MapUnit aMap100 = MAP_100TH_MM;

        	if ( pDescSize && pDescSize->Width() && pDescSize->Height() )
        	{
            	// use size from object descriptor if given
            	aSize = OutputDevice::LogicToLogic( *pDescSize, aMap100, aMapObj );
            	awt::Size aSz;
            	aSz.Width = aSize.Width();
            	aSz.Height = aSize.Height();
            	xObj->setVisualAreaSize( nAspect, aSz );
        	}
		
        	awt::Size aSz;
			try
			{
				aSz = xObj->getVisualAreaSize( nAspect );
			}
			catch ( embed::NoVisualAreaSizeException& )
			{
				// the default size will be set later
			}
		
        	aSize = Size( aSz.Width, aSz.Height );
			aSize = OutputDevice::LogicToLogic( aSize, aMapObj, aMap100 );	// fuer SdrOle2Obj

			if( aSize.Height() == 0 || aSize.Width() == 0 )
			{
				DBG_ERROR("SvObjectDescriptor::GetSize == 0");
				aSize.Width() = 5000;
				aSize.Height() = 5000;
				aSize = OutputDevice::LogicToLogic( aSize, aMap100, aMapObj );
            	aSz.Width = aSize.Width();
            	aSz.Height = aSize.Height();
            	xObj->setVisualAreaSize( nAspect, aSz );
			}
		}

		// don't call AdjustInsertPos
		Point aInsPos = rPos;
		if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
			aInsPos.X() -= aSize.Width();
		Rectangle aRect( aInsPos, aSize );

		ScDrawView* pDrView = GetScDrawView();
        SdrOle2Obj* pSdrObj = new SdrOle2Obj( aObjRef, aName, aRect );

		SdrPageView* pPV = pDrView->GetSdrPageView();
		pDrView->InsertObjectSafe( pSdrObj, *pPV );				// nicht markieren wenn Ole
		GetViewData()->GetViewShell()->SetDrawShell( sal_True );
		return sal_True;
	}
	else
		return sal_False;
}

sal_Bool ScViewFunc::PasteBitmapEx( const Point& rPos, const BitmapEx& rBmpEx )
{
	String aEmpty;
	Graphic aGraphic(rBmpEx);
	return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteMetaFile( const Point& rPos, const GDIMetaFile& rMtf )
{
	String aEmpty;
	Graphic aGraphic(rMtf);
	return PasteGraphic( rPos, aGraphic, aEmpty, aEmpty );
}

sal_Bool ScViewFunc::PasteGraphic( const Point& rPos, const Graphic& rGraphic,
								const String& rFile, const String& rFilter )
{
	MakeDrawLayer();
	ScDrawView* pScDrawView = GetScDrawView();

    // #123922# check if the drop was over an existing object; if yes, evtl. replace
    // the graphic for a SdrGraphObj (including link state updates) or adapt the fill 
    // style for other objects
    if(pScDrawView)
    {
        SdrObject* pPickObj = 0;
        SdrPageView* pPageView = pScDrawView->GetSdrPageView();

        if(pPageView)
        {
            pScDrawView->PickObj(rPos, pScDrawView->getHitTolLog(), pPickObj, pPageView);
        }

        if(pPickObj)
        {
            const String aBeginUndo(ScGlobal::GetRscString(STR_UNDO_DRAGDROP));
            SdrObject* pResult = pScDrawView->ApplyGraphicToObject(
                *pPickObj, 
                rGraphic, 
                aBeginUndo, 
                rFile, 
                rFilter);

            if(pResult)
            {
                // we are done; mark the modified/new object
                pScDrawView->MarkObj(pResult, pScDrawView->GetSdrPageView());
                return sal_True;
            }
        }
    }

	Point aPos( rPos );
	Window* pWin = GetActiveWin();
	MapMode aSourceMap = rGraphic.GetPrefMapMode();
	MapMode aDestMap( MAP_100TH_MM );

	if (aSourceMap.GetMapUnit() == MAP_PIXEL)
	{
			//	Pixel-Korrektur beruecksichtigen, damit Bitmap auf dem Bildschirm stimmt

		Fraction aScaleX, aScaleY;
		pScDrawView->CalcNormScale( aScaleX, aScaleY );
		aDestMap.SetScaleX(aScaleX);
		aDestMap.SetScaleY(aScaleY);
	}

	Size aSize = pWin->LogicToLogic( rGraphic.GetPrefSize(), &aSourceMap, &aDestMap );
//	lcl_AdjustInsertPos( GetViewData(), aPos, aSize );
	if ( GetViewData()->GetDocument()->IsNegativePage( GetViewData()->GetTabNo() ) )
		aPos.X() -= aSize.Width();

	GetViewData()->GetViewShell()->SetDrawShell( sal_True );
    Rectangle aRect(aPos, aSize);
	SdrGrafObj* pGrafObj = new SdrGrafObj(rGraphic, aRect);

    // #118522# calling SetGraphicLink here doesn't work

	//	#49961# Pfad wird nicht mehr als Name der Grafik gesetzt

	ScDrawLayer* pLayer = (ScDrawLayer*) pScDrawView->GetModel();
	String aName = pLayer->GetNewGraphicName();					// "Grafik x"
	pGrafObj->SetName(aName);

	// nicht markieren wenn Ole
	pScDrawView->InsertObjectSafe(pGrafObj, *pScDrawView->GetSdrPageView());

    // #118522# SetGraphicLink has to be used after inserting the object,
    // otherwise an empty graphic is swapped in and the contact stuff crashes.
    // See #i37444#.
	if (rFile.Len())
		pGrafObj->SetGraphicLink( rFile, rFilter );

	return sal_True;
}

