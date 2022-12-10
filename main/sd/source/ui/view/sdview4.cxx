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
#include "precompiled_sd.hxx"

#include "View.hxx"
#include <unotools/localfilehelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xfillit.hxx>
#include <svx/svdundo.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediawindow.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/filter.hxx>
#include "app.hrc"
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "fuinsfil.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "imapinfo.hxx"
#include "sdpage.hxx"
#include "view/SlideSorterView.hxx"
#include "undo/undoobjects.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <svtools/soerr.hxx>
#include <sfx2/ipclient.hxx>
#include <svx/svdoashp.hxx>
#include "glob.hrc"

using namespace com::sun::star;

namespace sd {

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

/*************************************************************************
|*
|* Graphik einfuegen
|* Wird ein leeres Graphikobjekt uebergeben, so wird dieses gefuellt.
|* Andernfalls wird ein an der gegebenen Position vorhandenes Objekt
|* gefuellt. Ist an der Position kein Objekt vorhanden, so wird ein neues
|* Objekt erzeugt und ein Pointer auf dieses Objekt zurueckgegeben.
|*
\************************************************************************/

SdrGrafObj* View::InsertGraphic( const Graphic& rGraphic, sal_Int8& rAction,
								   const Point& rPos, SdrObject* pObj, ImageMap* pImageMap )
{
	SdrEndTextEdit();
	mnAction = rAction;

	// Liegt ein Objekt an der Position rPos?
	SdrGrafObj*		pNewGrafObj = NULL;
	SdrPageView*	pPV = GetSdrPageView();
	SdrObject*		pPickObj = pObj;
	const bool bOnMaster = pPV && pPV->GetPage() && pPV->GetPage()->IsMasterPage();

	if(pPV && this->ISA(::sd::slidesorter::view::SlideSorterView))
	{
		if(!pPV->GetPageRect().IsInside(rPos))
			pPV = 0L;
	}

	if( !pPickObj && pPV )
	{
		SdrPageView* pPageView = pPV;
		PickObj(rPos, getHitTolLog(), pPickObj, pPageView);
	}

    const bool bIsGraphic(0 != dynamic_cast< SdrGrafObj* >(pPickObj));

    if(pPickObj 
        && !bIsGraphic 
        && !(pPickObj->IsEmptyPresObj() )
        && pPickObj->IsClosedObj() 
        && !dynamic_cast< SdrOle2Obj* >(pPickObj))
    {
        // fill style change (fill object with graphic), independent of mnAction
        // and thus of DND_ACTION_LINK or DND_ACTION_MOVE
        if( IsUndoEnabled() )
        {
            BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoAttrObject(*pPickObj));
            EndUndo();
        }

        SfxItemSet aSet(mpDocSh->GetPool(), XATTR_FILLSTYLE, XATTR_FILLBITMAP);

        aSet.Put(XFillStyleItem(XFILL_BITMAP));
        aSet.Put(XFillBitmapItem(&mpDocSh->GetPool(), rGraphic));
        pPickObj->SetMergedItemSetAndBroadcast(aSet);
    }
    else if(DND_ACTION_LINK == mnAction 
        && pPickObj 
        && pPV  
        && (bIsGraphic || (pPickObj->IsEmptyPresObj() && !bOnMaster))) // #121603# Do not use pObj, it may be NULL
	{
        // hit on SdrGrafObj with wanted new linked graphic (or PresObj placeholder hit)
		if( IsUndoEnabled() )
			BegUndo(String(SdResId(STR_INSERTGRAPHIC)));	

		SdPage* pPage = (SdPage*) pPickObj->GetPage();

		if( bIsGraphic )
		{
			// Das Objekt wird mit der Bitmap gefuellt
			pNewGrafObj = (SdrGrafObj*) pPickObj->Clone();
			pNewGrafObj->SetGraphic(rGraphic);
		}
		else
		{
			pNewGrafObj = new SdrGrafObj( rGraphic, pPickObj->GetLogicRect() );
			pNewGrafObj->SetEmptyPresObj(sal_True);
		}

		if ( pNewGrafObj->IsEmptyPresObj() )
		{
			Rectangle aRect( pNewGrafObj->GetLogicRect() );
			pNewGrafObj->AdjustToMaxRect( aRect, sal_False );
			pNewGrafObj->SetOutlinerParaObject(NULL);
			pNewGrafObj->SetEmptyPresObj(sal_False);
		}

		if (pPage && pPage->IsPresObj(pPickObj))
		{
			// Neues PresObj in die Liste eintragen
			pPage->InsertPresObj( pNewGrafObj, PRESOBJ_GRAPHIC );
			pNewGrafObj->SetUserCall(pPickObj->GetUserCall());
		}

		if (pImageMap)
			pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));

		ReplaceObjectAtView(pPickObj, *pPV, pNewGrafObj); // maybe ReplaceObjectAtView

		if( IsUndoEnabled() )
			EndUndo();
	}
	else if ( pPV )
	{
		// create  new object
		Size aSize;

		if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
		{
			::OutputDevice* pOutDev = 0;
			if( mpViewSh )
				pOutDev = mpViewSh->GetActiveWindow();

			if( !pOutDev )
				pOutDev = Application::GetDefaultDevice();

			if( pOutDev )
				aSize = pOutDev->PixelToLogic( rGraphic.GetPrefSize(), MAP_100TH_MM );
		}
		else
		{
			aSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
											    rGraphic.GetPrefMapMode(),
											    MapMode( MAP_100TH_MM ) );
		}

		pNewGrafObj = new SdrGrafObj( rGraphic, Rectangle( rPos, aSize ) );
		SdrPage* pPage = pPV->GetPage();
		Size aPageSize( pPage->GetSize() );
		aPageSize.Width()  -= pPage->GetLftBorder() + pPage->GetRgtBorder();
		aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();
		pNewGrafObj->AdjustToMaxRect( Rectangle( Point(), aPageSize ), sal_True );
//		pNewGrafObj->AdjustToMaxRect( Rectangle( pPV->GetOffset(), aPageSize ), sal_True );

		sal_uLong	nOptions = SDRINSERT_SETDEFLAYER;
		sal_Bool	bIsPresTarget = sal_False;

        if ((mpViewSh
                && mpViewSh->GetViewShell()!=NULL
                && mpViewSh->GetViewShell()->GetIPClient()
                && mpViewSh->GetViewShell()->GetIPClient()->IsObjectInPlaceActive())
            || this->ISA(::sd::slidesorter::view::SlideSorterView))
			nOptions |= SDRINSERT_DONTMARK;

		if( ( mnAction & DND_ACTION_MOVE ) && pPickObj && (pPickObj->IsEmptyPresObj() || pPickObj->GetUserCall()) )
		{
			SdPage* pP = static_cast< SdPage* >( pPickObj->GetPage() );

			if ( pP && pP->IsMasterPage() )
				bIsPresTarget = pP->IsPresObj(pPickObj);
		}

		if( ( mnAction & DND_ACTION_MOVE ) && pPickObj && !bIsPresTarget )
		{
			// replace object
			if (pImageMap)
				pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));

			Rectangle aPickObjRect(pPickObj->GetCurrentBoundRect());
			Size aPickObjSize(aPickObjRect.GetSize());
			Rectangle aObjRect(pNewGrafObj->GetCurrentBoundRect());
			Size aObjSize(aObjRect.GetSize());

			Fraction aScaleWidth(aPickObjSize.Width(), aObjSize.Width());
			Fraction aScaleHeight(aPickObjSize.Height(), aObjSize.Height());
			pNewGrafObj->NbcResize(aObjRect.TopLeft(), aScaleWidth, aScaleHeight);

			Point aVec = aPickObjRect.TopLeft() - aObjRect.TopLeft();
			pNewGrafObj->NbcMove(Size(aVec.X(), aVec.Y()));

			const bool bUndo = IsUndoEnabled();

			if( bUndo )
				BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
			pNewGrafObj->NbcSetLayer(pPickObj->GetLayer());
			SdrPage* pP = pPV->GetPage();
			pP->InsertObject(pNewGrafObj);
			if( bUndo )
			{
				AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoNewObject(*pNewGrafObj));
				AddUndo(mpDoc->GetSdrUndoFactory().CreateUndoDeleteObject(*pPickObj));
			}
			pP->RemoveObject(pPickObj->GetOrdNum());

			if( bUndo )
			{
				EndUndo();
			}
			else
			{
				SdrObject::Free(pPickObj);
			}
			mnAction = DND_ACTION_COPY;
		}
		else
		{
			InsertObjectAtView(pNewGrafObj, *pPV, nOptions);

			if( pImageMap )
				pNewGrafObj->InsertUserData(new SdIMapInfo(*pImageMap));
		}
	}

	rAction = mnAction;

	return pNewGrafObj;
}

// -----------------------------------------------------------------------------

SdrMediaObj* View::InsertMediaURL( const rtl::OUString& rMediaURL, sal_Int8& rAction,
								   const Point& rPos, const Size& rSize )
{
	SdrEndTextEdit();
	mnAction = rAction;

	SdrMediaObj*	pNewMediaObj = NULL;
	SdrPageView*	pPV = GetSdrPageView();
	SdrObject*		pPickObj = GetEmptyPresentationObject( PRESOBJ_MEDIA );

	if(pPV && this->ISA(::sd::slidesorter::view::SlideSorterView ))
	{
		if(!pPV->GetPageRect().IsInside(rPos))
			pPV = 0L;
	}

	if( !pPickObj && pPV )
	{
		SdrPageView* pPageView = pPV;
		PickObj(rPos, getHitTolLog(), pPickObj, pPageView);
	}

	if( mnAction == DND_ACTION_LINK && pPickObj && pPV && pPickObj->ISA( SdrMediaObj ) )
	{
		pNewMediaObj = static_cast< SdrMediaObj* >( pPickObj->Clone() );
		pNewMediaObj->setURL( rMediaURL );

    	BegUndo(String(SdResId(STR_UNDO_DRAGDROP)));
		ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj);
		EndUndo();
	}
	else if( pPV )
	{
		Rectangle aRect( rPos, rSize );
		if( pPickObj )
			aRect = pPickObj->GetLogicRect();


    	pNewMediaObj = new SdrMediaObj( aRect );

		bool bIsPres = false;
		if( pPickObj )
		{
			SdPage* pPage = static_cast< SdPage* >(pPickObj->GetPage());
			bIsPres = pPage && pPage->IsPresObj(pPickObj);
			if( bIsPres )
			{
				pPage->InsertPresObj( pNewMediaObj, PRESOBJ_MEDIA );
			}
		}

		if( pPickObj )
			ReplaceObjectAtView(pPickObj, *pPV, pNewMediaObj);
		else
			InsertObjectAtView( pNewMediaObj, *pPV, SDRINSERT_SETDEFLAYER );

	    pNewMediaObj->setURL( rMediaURL );

		if( pPickObj )
		{
			pNewMediaObj->AdjustToMaxRect( pPickObj->GetLogicRect() );
			if( bIsPres )
				pNewMediaObj->SetUserCall(pPickObj->GetUserCall());
		}
	}

	rAction = mnAction;

	return pNewMediaObj;
}

/*************************************************************************
|*
|* Timer-Handler fuer InsertFile beim Drop()
|*
\************************************************************************/

IMPL_LINK( View, DropInsertFileHdl, Timer*, EMPTYARG )
{
	DBG_ASSERT( mpViewSh, "sd::View::DropInsertFileHdl(), I need a view shell to work!" );
	if( !mpViewSh )
		return 0;

    SfxErrorContext aEc( ERRCTX_ERROR, mpViewSh->GetActiveWindow(), RID_SO_ERRCTX );
    ErrCode nError = 0;

	::std::vector< String >::const_iterator aIter( maDropFileVector.begin() );

    while( (aIter != maDropFileVector.end()) && !nError )
    {
	    String          aCurrentDropFile( *aIter );
        INetURLObject   aURL( aCurrentDropFile );
	    sal_Bool		    bOK = sal_False;

	    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
	    {
		    String aURLStr;
		    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aCurrentDropFile, aURLStr );
		    aURL = INetURLObject( aURLStr );
	    }

        GraphicFilter*  pGraphicFilter = GraphicFilter::GetGraphicFilter();
	    Graphic			aGraphic;

        aCurrentDropFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

		if( !::avmedia::MediaWindow::isMediaURL( aCurrentDropFile ) )
		{
			if( !pGraphicFilter->ImportGraphic( aGraphic, aURL ) )
			{
				sal_Int8    nTempAction = ( aIter == maDropFileVector.begin() ) ? mnAction : 0;
				const bool bLink = ( ( nTempAction & DND_ACTION_LINK ) != 0 );
				SdrGrafObj* pGrafObj = InsertGraphic( aGraphic, nTempAction, maDropPos, NULL, NULL );

				if(pGrafObj && bLink)
                {
    				pGrafObj->SetGraphicLink( aCurrentDropFile, String() );
                }

				// return action from first inserted graphic
				if( aIter == maDropFileVector.begin() )
					mnAction = nTempAction;

				bOK = sal_True;
			}
			if( !bOK )
			{
    			const SfxFilter*        pFoundFilter = NULL;
				SfxMedium	            aSfxMedium( aCurrentDropFile, STREAM_READ | STREAM_SHARE_DENYNONE, sal_False );
				ErrCode		            nErr = SFX_APP()->GetFilterMatcher().GuessFilter(  aSfxMedium, &pFoundFilter, SFX_FILTER_IMPORT, SFX_FILTER_NOTINSTALLED | SFX_FILTER_EXECUTABLE );

				if( pFoundFilter && !nErr )
				{
					::std::vector< String > aFilterVector;
					const String	        aFilterName( pFoundFilter->GetFilterName() );
					String	        		aLowerAsciiFileName( aCurrentDropFile );
					aLowerAsciiFileName.ToLowerAscii();

					FuInsertFile::GetSupportedFilterVector( aFilterVector );

					if( ( ::std::find( aFilterVector.begin(), aFilterVector.end(), pFoundFilter->GetMimeType() ) != aFilterVector.end() ) ||
						aFilterName.SearchAscii( "Text" ) != STRING_NOTFOUND ||
						aFilterName.SearchAscii( "Rich" ) != STRING_NOTFOUND ||
						aFilterName.SearchAscii( "RTF" ) != STRING_NOTFOUND ||
						aFilterName.SearchAscii( "HTML" ) != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".sdd") != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".sda") != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".sxd") != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".sxi") != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".std") != STRING_NOTFOUND ||
						aLowerAsciiFileName.SearchAscii(".sti") != STRING_NOTFOUND )
					{
						::sd::Window* pWin = mpViewSh->GetActiveWindow();
						SfxRequest      aReq(SID_INSERTFILE, 0, mpDoc->GetItemPool());
						SfxStringItem   aItem1( ID_VAL_DUMMY0, aCurrentDropFile ), aItem2( ID_VAL_DUMMY1, pFoundFilter->GetFilterName() );

						aReq.AppendItem( aItem1 );
						aReq.AppendItem( aItem2 );
						FuInsertFile::Create( mpViewSh, pWin, this, mpDoc, aReq );
						bOK = sal_True;
					}
				}
			}
		}

	    if( !bOK )
	    {
	        Size aPrefSize;

	        if( ::avmedia::MediaWindow::isMediaURL( aCurrentDropFile ) &&
	            ::avmedia::MediaWindow::isMediaURL( aCurrentDropFile, true, &aPrefSize ) )
	        {
			    if( aPrefSize.Width() && aPrefSize.Height() )
			    {
					::sd::Window* pWin = mpViewSh->GetActiveWindow();

				    if( pWin )
					    aPrefSize = pWin->PixelToLogic( aPrefSize, MAP_100TH_MM );
				    else
					    aPrefSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_100TH_MM );
			    }
			    else
				    aPrefSize  = Size( 5000, 5000 );

    		    InsertMediaURL( aCurrentDropFile, mnAction, maDropPos, aPrefSize ) ;
	        }
	        else if( mnAction & DND_ACTION_LINK )
			    static_cast< DrawViewShell* >( mpViewSh )->InsertURLButton( aCurrentDropFile, aCurrentDropFile, String(), &maDropPos );
		    else
		    {
			    if( mpViewSh )
			    {
					try
					{
						//TODO/MBA: testing
						::rtl::OUString aName;
						uno::Sequence < beans::PropertyValue > aMedium(1);
						aMedium[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
						aMedium[0].Value <<= ::rtl::OUString( aCurrentDropFile );

						uno::Reference < embed::XEmbeddedObject > xObj = mpDocSh->GetEmbeddedObjectContainer().
								InsertEmbeddedObject( aMedium, aName );

						uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
						if ( xPersist.is())
						{
							// TODO/LEAN: VisualArea access can switch the object to running state
							sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
	                        
							xPersist->storeOwn();

							awt::Size aSz;
							try
							{
								aSz = xObj->getVisualAreaSize( nAspect );
							}
							catch( embed::NoVisualAreaSizeException& )
							{
								// the default size will be set later
							}
							
							Size        aSize( aSz.Width, aSz.Height );
							Rectangle   aRect;

							if (!aSize.Width() || !aSize.Height())
							{
								aSize.Width()   = 1410;
								aSize.Height()  = 1000;
							}

							aRect = Rectangle( maDropPos, aSize );

							SdrOle2Obj* pOleObj = new SdrOle2Obj( svt::EmbeddedObjectRef( xObj, nAspect ), aName, aRect );
							sal_uLong       nOptions = SDRINSERT_SETDEFLAYER;

							if (mpViewSh != NULL)
							{
								OSL_ASSERT (mpViewSh->GetViewShell()!=NULL);
								SfxInPlaceClient* pIpClient =
									mpViewSh->GetViewShell()->GetIPClient();
								if (pIpClient!=NULL && pIpClient->IsObjectInPlaceActive())
									nOptions |= SDRINSERT_DONTMARK;
							}

							InsertObjectAtView( pOleObj, *GetSdrPageView(), nOptions );
							pOleObj->SetLogicRect( aRect );
							aSz.Width = aRect.GetWidth();
							aSz.Height = aRect.GetHeight();
							xObj->setVisualAreaSize( nAspect,aSz );
						}
					}
					catch( uno::Exception& )
					{
						nError = ERRCODE_IO_GENERAL;
						// TODO/LATER: better error handling
					}
				}
		    }
	    }

        ++aIter;
    }

    if( nError )
        ErrorHandler::HandleError( nError );

    return nError;
}

/*************************************************************************
|*
|* Timer-Handler fuer Errorhandling beim Drop()
|*
\************************************************************************/

IMPL_LINK( View, DropErrorHdl, Timer*, EMPTYARG )
{
	InfoBox( mpViewSh ? mpViewSh->GetActiveWindow() : 0, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
	return 0;
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

/*************************************************************************
|*
|* Redraw sperren oder erlauben
|*
\************************************************************************/

void View::LockRedraw(sal_Bool bLock)
{
	if (bLock)
	{
		mnLockRedrawSmph++;
		DBG_ASSERT(mnLockRedrawSmph, "Ueberlauf im LockRedraw");
	}
	else
	{
		DBG_ASSERT(mnLockRedrawSmph, "Unterlauf im LockRedraw");
		mnLockRedrawSmph--;

		// alle gespeicherten Redraws ausfuehren
		if (!mnLockRedrawSmph)
		{
			while (mpLockedRedraws && mpLockedRedraws->Count())
			{
				SdViewRedrawRec* pRec = (SdViewRedrawRec*)mpLockedRedraws->First();
				OutputDevice* pCurrentOut = pRec->mpOut;
				Rectangle aBoundRect(pRec->aRect);
				mpLockedRedraws->Remove(pRec);
				delete pRec;

				pRec = (SdViewRedrawRec*)mpLockedRedraws->First();
				while (pRec)
				{
					if (pRec->mpOut == pCurrentOut)
					{
						aBoundRect.Union(pRec->aRect);
						mpLockedRedraws->Remove(pRec);
						delete pRec;
						pRec = (SdViewRedrawRec*)mpLockedRedraws->GetCurObject();
					}
					else
					{
						pRec = (SdViewRedrawRec*)mpLockedRedraws->Next();
					}
				}

				CompleteRedraw(pCurrentOut, Region(aBoundRect));
			}
			delete mpLockedRedraws;
			mpLockedRedraws = NULL;
		}
	}
}




/*************************************************************************
|*
|* StyleSheet aus der Sleketion besorgen
|*
\************************************************************************/

SfxStyleSheet* View::GetStyleSheet() const
{
	return SdrView::GetStyleSheet();
}

} // end of namespace sd
