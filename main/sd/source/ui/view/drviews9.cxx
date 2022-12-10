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

#include "DrawViewShell.hxx"
#include <vcl/wrkwin.hxx>
#include <svx/xgrad.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/bindings.hxx>

#ifndef _SFX_DISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#include <svx/xtable.hxx>
#include <svx/gallery.hxx>
#include <vcl/graph.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <svl/whiter.hxx>
#include <basic/sbstar.hxx>

#include <sfx2/viewfrm.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "drawview.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "fupoor.hxx"

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>

namespace sd {

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif


/*************************************************************************
|*
|* SfxRequests fuer Gallery bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecGallery(SfxRequest& rReq)
{
	// waehrend einer Diashow wird nichts ausgefuehrt!
	if(HasCurrentFunction(SID_PRESENTATION))
		return;

	const SfxItemSet* pArgs = rReq.GetArgs();

    SFX_ITEMSET_ARG( pArgs, pGalleryItem, SvxGalleryItem, SID_GALLERY_FORMATS, sal_False );
    if ( !pGalleryItem )
        return;

    GetDocSh()->SetWaitCursor( sal_True );

    sal_Int8 nType( pGalleryItem->GetType() );
    // Graphik einfuegen
    if (nType == com::sun::star::gallery::GalleryItemType::GRAPHIC)
    {
        Graphic aGraphic( pGalleryItem->GetGraphic() );

        // Ggf. Groesse reduzieren
        Window aWindow (GetActiveWindow());
        aWindow.SetMapMode(aGraphic.GetPrefMapMode());
        Size aSizePix = aWindow.LogicToPixel(aGraphic.GetPrefSize());
        aWindow.SetMapMode( MapMode(MAP_100TH_MM) );
        Size aSize = aWindow.PixelToLogic(aSizePix);

        // Groesse ggf. auf Seitengroesse begrenzen
        SdrPage* pPage = mpDrawView->GetSdrPageView()->GetPage();
        Size aPageSize = pPage->GetSize();
        aPageSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
        aPageSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();


        // Falls Grafik zu gross, wird die Grafik
        // in die Seite eingepasst
        if ( ( ( aSize.Height() > aPageSize.Height() ) || ( aSize.Width()	> aPageSize.Width() ) ) &&
            aSize.Height() && aPageSize.Height() )
        {
            float fGrfWH =	(float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =	(float)aPageSize.Width() /
                            (float)aPageSize.Height();

            // Grafik an Pagesize anpassen (skaliert)
            if ((fGrfWH != 0.F) && (fGrfWH < fWinWH))
            {
                aSize.Width() = (long)(aPageSize.Height() * fGrfWH);
                aSize.Height()= aPageSize.Height();
            }
            else
            {
                aSize.Width() = aPageSize.Width();
                aSize.Height()= (long)(aPageSize.Width() / fGrfWH);
            }
        }


        // Ausgaberechteck fuer Grafik setzen
        Point aPnt ((aPageSize.Width()	- aSize.Width())  / 2,
                    (aPageSize.Height() - aSize.Height()) / 2);
        aPnt += Point(pPage->GetLftBorder(), pPage->GetUppBorder());
        Rectangle aRect (aPnt, aSize);

        SdrGrafObj* pGrafObj = NULL;

        sal_Bool bInsertNewObject = sal_True;

        if ( mpDrawView->AreObjectsMarked() )
        {
            /******************************************************
            * Ist ein leeres Graphik-Objekt vorhanden?
            ******************************************************/
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

                if (pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF)
                {
                    pGrafObj = (SdrGrafObj*) pObj;

                    if( pGrafObj->IsEmptyPresObj() )
                    {
                        /******************************************
                        * Das leere Graphik-Objekt bekommt eine neue
                        * Graphik
                        ******************************************/
                        bInsertNewObject = sal_False;

                        SdrGrafObj* pNewGrafObj = (SdrGrafObj*) pGrafObj->Clone();
                        pNewGrafObj->SetEmptyPresObj(sal_False);
                        pNewGrafObj->SetOutlinerParaObject(NULL);
                        pNewGrafObj->SetGraphic(aGraphic);

                        String aStr(mpDrawView->GetDescriptionOfMarkedObjects());
                        aStr += sal_Unicode(' ');
                        aStr += String(SdResId(STR_UNDO_REPLACE));
                        mpDrawView->BegUndo(aStr);
                        SdrPageView* pPV = mpDrawView->GetSdrPageView();
                        mpDrawView->ReplaceObjectAtView(pGrafObj, *pPV, pNewGrafObj);
                        mpDrawView->EndUndo();
                    }
                }
            }
        }


        if( bInsertNewObject )
        {
            pGrafObj = new SdrGrafObj(aGraphic, aRect);
            SdrPageView* pPV = mpDrawView->GetSdrPageView();
            mpDrawView->InsertObjectAtView(pGrafObj, *pPV, SDRINSERT_SETDEFLAYER);
        }

        // Soll nur ein Link benutzt werden?
        if( pGrafObj && pGalleryItem->IsLink() )
            pGrafObj->SetGraphicLink( pGalleryItem->GetURL(), pGalleryItem->GetFilterName() );
    }
    // insert sound
    else if( nType == com::sun::star::gallery::GalleryItemType::MEDIA )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGalleryItem->GetURL() );
        GetViewFrame()->GetDispatcher()->Execute( SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON, &aMediaURLItem, 0L );
    }

    GetDocSh()->SetWaitCursor( sal_False );
}


/*************************************************************************
|*
|* Statuswerte fuer Gallery zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetGalleryState(SfxItemSet& )
{
}

/*************************************************************************
|*
|* Makros fuer die Attributeinstellungen bearbeiten
|*
\************************************************************************/

//
// die vorgehensweise fuer die attributaenderung ist praktisch ueberall identisch
// 1. bisherige attribute auslesen
// 2. parameter aus dem basic-set auslesen
// 3. gewaehltes item aus dem attribut-set loeschen
// 4. neues attribut-item erzeugen
// 5. item in den set eintragen
//
void DrawViewShell::AttrExec (SfxRequest &rReq)
{
	// waehrend einer Diashow wird nichts ausgefuehrt!
	if(HasCurrentFunction(SID_PRESENTATION))
		return;

	CheckLineTo (rReq);

	SfxBindings&	rBindings = GetViewFrame()->GetBindings();
	SfxItemSet*		pAttr = new SfxItemSet ( GetDoc()->GetPool() );

	GetView()->GetAttributes( *pAttr );
	const SfxItemSet* pArgs = rReq.GetArgs();

	switch (rReq.GetSlot ())
	{
		// neuen fuellstil setzen
		case SID_SETFILLSTYLE :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pFillStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
					if (CHECK_RANGE (XFILL_NONE, (sal_Int32)pFillStyle->GetValue (), XFILL_BITMAP))
					{
						pAttr->ClearItem (XATTR_FILLSTYLE);
						pAttr->Put (XFillStyleItem ((XFillStyle) pFillStyle->GetValue ()), XATTR_FILLSTYLE);
						rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		// linienstil neu bestimmen
		case SID_SETLINESTYLE :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pLineStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
					if (CHECK_RANGE (XLINE_NONE, (sal_Int32)pLineStyle->GetValue (), XLINE_DASH))
					{
						pAttr->ClearItem (XATTR_LINESTYLE);
						pAttr->Put (XLineStyleItem ((XLineStyle) pLineStyle->GetValue ()), XATTR_LINESTYLE);
						rBindings.Invalidate (SID_ATTR_LINE_STYLE);
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		// linienbreite setzen
		case SID_SETLINEWIDTH :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pLineWidth, SfxUInt32Item, ID_VAL_WIDTH, sal_False);
					pAttr->ClearItem (XATTR_LINEWIDTH);
					pAttr->Put (XLineWidthItem (pLineWidth->GetValue ()), XATTR_LINEWIDTH);
					rBindings.Invalidate (SID_ATTR_LINE_WIDTH);
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SETFILLCOLOR :
			if (pArgs)
				if (pArgs->Count () == 3)
				{
					SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
					SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
					SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

					pAttr->ClearItem (XATTR_FILLCOLOR);
					pAttr->ClearItem (XATTR_FILLSTYLE);
					pAttr->Put (XFillColorItem (-1, Color ((sal_uInt8) pRed->GetValue (),
                                                           (sal_uInt8) pGreen->GetValue (),
														   (sal_uInt8) pBlue->GetValue ())),
								XATTR_FILLCOLOR);
					pAttr->Put (XFillStyleItem (XFILL_SOLID), XATTR_FILLSTYLE);
					rBindings.Invalidate (SID_ATTR_FILL_COLOR);
					rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SETLINECOLOR :
			if (pArgs)
				if (pArgs->Count () == 3)
				{
					SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
					SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
					SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

					pAttr->ClearItem (XATTR_LINECOLOR);
					pAttr->Put (XLineColorItem (-1, Color ((sal_uInt8) pRed->GetValue (),
                                                           (sal_uInt8) pGreen->GetValue (),
														   (sal_uInt8) pBlue->GetValue ())),
								XATTR_LINECOLOR);
					rBindings.Invalidate (SID_ATTR_LINE_COLOR);
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SETGRADSTARTCOLOR :
		case SID_SETGRADENDCOLOR :
			if (pArgs)
				if (pArgs->Count () == 4)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
					SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
					SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
					SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

					XGradientListSharedPtr aGradientList = GetDoc()->GetGradientListFromSdrModel();
					long		  nCounts		 = aGradientList->Count ();
					Color		  aColor ((sal_uInt8) pRed->GetValue (),
                                          (sal_uInt8) pGreen->GetValue (),
										  (sal_uInt8) pBlue->GetValue ());
					long i;

					pAttr->ClearItem (XATTR_FILLGRADIENT);
					pAttr->ClearItem (XATTR_FILLSTYLE);

					for ( i = 0; i < nCounts; i ++)
					{
						XGradientEntry *pEntry = aGradientList->GetGradient (i);

						if (pEntry->GetName () == pName->GetValue ())
						{
							XGradient aGradient(pEntry->GetGradient());

							if (rReq.GetSlot () == SID_SETGRADSTARTCOLOR) aGradient.SetStartColor (aColor);
							else aGradient.SetEndColor (aColor);

							pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
							pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
							break;
						}
					}

					if (i >= nCounts)
					{
						Color aBlack (0, 0, 0);
						XGradient aGradient ((rReq.GetSlot () == SID_SETGRADSTARTCOLOR)
												 ? aColor
												 : aBlack,
											 (rReq.GetSlot () == SID_SETGRADENDCOLOR)
												 ? aColor
												 : aBlack);

						GetDoc()->GetGradientListFromSdrModel()->Insert (new XGradientEntry (aGradient, pName->GetValue ()));

						pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
						pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
					}

					rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SETHATCHCOLOR :
			if (pArgs)
				if (pArgs->Count () == 4)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
					SFX_REQUEST_ARG (rReq, pRed, SfxUInt32Item, ID_VAL_RED, sal_False);
					SFX_REQUEST_ARG (rReq, pGreen, SfxUInt32Item, ID_VAL_GREEN, sal_False);
					SFX_REQUEST_ARG (rReq, pBlue, SfxUInt32Item, ID_VAL_BLUE, sal_False);

					XHatchListSharedPtr aHatchList = GetDoc()->GetHatchListFromSdrModel();
					long	   nCounts	   = aHatchList.get() ? aHatchList->Count() : 0;
					Color	   aColor ((sal_uInt8) pRed->GetValue (),
                                       (sal_uInt8) pGreen->GetValue (),
									   (sal_uInt8) pBlue->GetValue ());
					long i;

					pAttr->ClearItem (XATTR_FILLHATCH);
					pAttr->ClearItem (XATTR_FILLSTYLE);

					for ( i = 0; i < nCounts; i ++)
					{
						XHatchEntry *pEntry = aHatchList->GetHatch (i);

						if (pEntry->GetName () == pName->GetValue ())
						{
							XHatch aHatch(pEntry->GetHatch());

							aHatch.SetColor (aColor);

							pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
							pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
							break;
						}
					}

					if (i >= nCounts)
					{
						XHatch aHatch (aColor);

						GetDoc()->GetHatchListFromSdrModel()->Insert (new XHatchEntry (aHatch, pName->GetValue ()));

						pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
						pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
					}

					rBindings.Invalidate (SID_ATTR_FILL_HATCH);
					rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		// einstellungen fuer liniendash
		case SID_DASH :
			if (pArgs)
				if (pArgs->Count () == 7)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
					SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
					SFX_REQUEST_ARG (rReq, pDots, SfxUInt32Item, ID_VAL_DOTS, sal_False);
					SFX_REQUEST_ARG (rReq, pDotLen, SfxUInt32Item, ID_VAL_DOTLEN, sal_False);
					SFX_REQUEST_ARG (rReq, pDashes, SfxUInt32Item, ID_VAL_DASHES, sal_False);
					SFX_REQUEST_ARG (rReq, pDashLen, SfxUInt32Item, ID_VAL_DASHLEN, sal_False);
					SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, sal_False);

					if (CHECK_RANGE (XDASH_RECT, (sal_Int32)pStyle->GetValue (), XDASH_ROUNDRELATIVE))
					{
						XDash aNewDash ((XDashStyle) pStyle->GetValue (), (short) pDots->GetValue (), pDotLen->GetValue (),
										(short) pDashes->GetValue (), pDashLen->GetValue (), pDistance->GetValue ());

						pAttr->ClearItem (XATTR_LINEDASH);
						pAttr->ClearItem (XATTR_LINESTYLE);

						XDashListSharedPtr aDashList = GetDoc()->GetDashListFromSdrModel();
						long	   nCounts	  = aDashList.get() ? aDashList->Count() : 0;
						XDashEntry *pEntry	  = new XDashEntry (aNewDash, pName->GetValue ());
						long i;

						for ( i = 0; i < nCounts; i++ )
							if (aDashList->GetDash (i)->GetName () == pName->GetValue ())
								break;

						if (i < nCounts)
							aDashList->Replace (pEntry, i);
						else
							aDashList->Insert (pEntry);

						pAttr->Put (XLineDashItem (pName->GetValue (), aNewDash), XATTR_LINEDASH);
						pAttr->Put (XLineStyleItem (XLINE_DASH), XATTR_LINESTYLE);
						rBindings.Invalidate (SID_ATTR_LINE_DASH);
						rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		// einstellungen fuer farbverlauf
		case SID_GRADIENT :
			if (pArgs)
				if (pArgs->Count () == 8)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
					SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
					SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, sal_False);
					SFX_REQUEST_ARG (rReq, pBorder, SfxUInt32Item, ID_VAL_BORDER, sal_False);
					SFX_REQUEST_ARG (rReq, pCenterX, SfxUInt32Item, ID_VAL_CENTER_X, sal_False);
					SFX_REQUEST_ARG (rReq, pCenterY, SfxUInt32Item, ID_VAL_CENTER_Y, sal_False);
					SFX_REQUEST_ARG (rReq, pStart, SfxUInt32Item, ID_VAL_STARTINTENS, sal_False);
					SFX_REQUEST_ARG (rReq, pEnd, SfxUInt32Item, ID_VAL_ENDINTENS, sal_False);

					if (CHECK_RANGE (XGRAD_LINEAR, (sal_Int32)pStyle->GetValue (), XGRAD_RECT) &&
						CHECK_RANGE (0, (sal_Int32)pAngle->GetValue (), 360) &&
						CHECK_RANGE (0, (sal_Int32)pBorder->GetValue (), 100) &&
						CHECK_RANGE (0, (sal_Int32)pCenterX->GetValue (), 100) &&
						CHECK_RANGE (0, (sal_Int32)pCenterY->GetValue (), 100) &&
						CHECK_RANGE (0, (sal_Int32)pStart->GetValue (), 100) &&
						CHECK_RANGE (0, (sal_Int32)pEnd->GetValue (), 100))
					{
						pAttr->ClearItem (XATTR_FILLGRADIENT);
						pAttr->ClearItem (XATTR_FILLSTYLE);

						XGradientListSharedPtr aGradientList = GetDoc()->GetGradientListFromSdrModel();
						long		   nCounts		  = aGradientList->Count ();
						long i;

						for ( i = 0; i < nCounts; i++ )
						{
							XGradientEntry *pEntry = aGradientList->GetGradient (i);

							if (pEntry->GetName () == pName->GetValue ())
							{
								XGradient aGradient(pEntry->GetGradient());

								aGradient.SetGradientStyle ((XGradientStyle) pStyle->GetValue ());
								aGradient.SetAngle (pAngle->GetValue () * 10);
								aGradient.SetBorder ((short) pBorder->GetValue ());
								aGradient.SetXOffset ((short) pCenterX->GetValue ());
								aGradient.SetYOffset ((short) pCenterY->GetValue ());
								aGradient.SetStartIntens ((short) pStart->GetValue ());
								aGradient.SetEndIntens ((short) pEnd->GetValue ());

								pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
								pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
								break;
							}
						}

						if (i >= nCounts)
						{
							Color aBlack (0, 0, 0);
							XGradient aGradient (aBlack, aBlack, (XGradientStyle) pStyle->GetValue (),
												 pAngle->GetValue () * 10, (short) pCenterX->GetValue (),
												 (short) pCenterY->GetValue (), (short) pBorder->GetValue (),
												 (short) pStart->GetValue (), (short) pEnd->GetValue ());

							aGradientList->Insert (new XGradientEntry (aGradient, pName->GetValue ()));
							pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
							pAttr->Put (XFillGradientItem (pName->GetValue (), aGradient), XATTR_FILLGRADIENT);
						}

						rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
						rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		// einstellungen fuer schraffur
		case SID_HATCH :
			if (pArgs)
				if (pArgs->Count () == 4)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);
					SFX_REQUEST_ARG (rReq, pStyle, SfxUInt32Item, ID_VAL_STYLE, sal_False);
					SFX_REQUEST_ARG (rReq, pDistance, SfxUInt32Item, ID_VAL_DISTANCE, sal_False);
					SFX_REQUEST_ARG (rReq, pAngle, SfxUInt32Item, ID_VAL_ANGLE, sal_False);

					if (CHECK_RANGE (XHATCH_SINGLE, (sal_Int32)pStyle->GetValue (), XHATCH_TRIPLE) &&
						CHECK_RANGE (0, (sal_Int32)pAngle->GetValue (), 360))
					{
						pAttr->ClearItem (XATTR_FILLHATCH);
						pAttr->ClearItem (XATTR_FILLSTYLE);

						XHatchListSharedPtr aHatchList = GetDoc()->GetHatchListFromSdrModel();
						long	   nCounts	   = aHatchList.get() ? aHatchList->Count() : 0;
						long i;

						for ( i = 0; i < nCounts; i++ )
						{
							XHatchEntry *pEntry = aHatchList->GetHatch (i);

							if (pEntry->GetName () == pName->GetValue ())
							{
								XHatch aHatch(pEntry->GetHatch());

								aHatch.SetHatchStyle ((XHatchStyle) pStyle->GetValue ());
								aHatch.SetDistance (pDistance->GetValue ());
								aHatch.SetAngle (pAngle->GetValue () * 10);

								pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
								pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
								break;
							}
						}

						if (i >= nCounts)
						{
							Color  aBlack (0, 0, 0);
							XHatch aHatch (aBlack, (XHatchStyle) pStyle->GetValue (), pDistance->GetValue (),
										   pAngle->GetValue () * 10);

							aHatchList->Insert (new XHatchEntry (aHatch, pName->GetValue ()));
							pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
							pAttr->Put (XFillHatchItem (pName->GetValue (), aHatch), XATTR_FILLHATCH);
						}

						rBindings.Invalidate (SID_ATTR_FILL_HATCH);
						rBindings.Invalidate (SID_ATTR_FILL_STYLE);
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SELECTGRADIENT :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);

					XGradientListSharedPtr aGradientList = GetDoc()->GetGradientListFromSdrModel();
					long		   nCounts		  = aGradientList->Count ();

					for (long i = 0;
							  i < nCounts;
							  i ++)
					{
						XGradientEntry *pEntry = aGradientList->GetGradient (i);

						if (pEntry->GetName () == pName->GetValue ())
						{
							pAttr->ClearItem (XATTR_FILLGRADIENT);
							pAttr->ClearItem (XATTR_FILLSTYLE);
							pAttr->Put (XFillStyleItem (XFILL_GRADIENT), XATTR_FILLSTYLE);
							pAttr->Put (XFillGradientItem (pName->GetValue (), pEntry->GetGradient ()), XATTR_FILLGRADIENT);

							rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
							rBindings.Invalidate (SID_ATTR_FILL_STYLE);
							break;
						}
					}

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_SELECTHATCH :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pName, SfxStringItem, ID_VAL_INDEX, sal_False);

					XHatchListSharedPtr aHatchList = GetDoc()->GetHatchListFromSdrModel();
					long	   nCounts	   = aHatchList.get() ? aHatchList->Count() : 0;

					for (long i = 0;
							  i < nCounts;
							  i ++)
					{
						XHatchEntry *pEntry = aHatchList->GetHatch (i);

						if (pEntry->GetName () == pName->GetValue ())
						{
							pAttr->ClearItem (XATTR_FILLHATCH);
							pAttr->ClearItem (XATTR_FILLSTYLE);
							pAttr->Put (XFillStyleItem (XFILL_HATCH), XATTR_FILLSTYLE);
							pAttr->Put (XFillHatchItem (pName->GetValue (), pEntry->GetHatch ()), XATTR_FILLHATCH);

							rBindings.Invalidate (SID_ATTR_FILL_HATCH);
							rBindings.Invalidate (SID_ATTR_FILL_STYLE);
							break;
						}
					}

					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

		case SID_UNSELECT :
			mpDrawView->UnmarkAll ();
			break;

		case SID_GETRED :
			if (pArgs)
				if (pArgs->Count () == 1)
				{
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			break;

/*		  case SID_SETFONTFAMILYNAME :
		case SID_SETFONTSTYLENAME :
		case SID_SETFONTFAMILY :
		case SID_SETFONTPITCH :
		case SID_SETFONTCHARSET :
		case SID_SETFONTPOSTURE :
		case SID_SETFONTWEIGHT :
		case SID_SETFONTUNDERLINE :
		case SID_SETFONTCROSSEDOUT :
		case SID_SETFONTSHADOWED :
		case SID_SETFONTCONTOUR :
		case SID_SETFONTCOLOR :
		case SID_SETFONTLANGUAGE :
		case SID_SETFONTWORDLINE :
		case SID_SETFONTCASEMAP :
		case SID_SETFONTESCAPE :
		case SID_SETFONTKERNING :
			break;*/

		default :
			;
	}

	mpDrawView->SetAttributes (*(const SfxItemSet *) pAttr);
	rReq.Ignore ();
	delete pAttr;
}

/*************************************************************************
|*
|* Makros fuer die Attributeinstellungen bearbeiten
|*
\************************************************************************/

void DrawViewShell::AttrState (SfxItemSet& rSet)
{
	SfxWhichIter	 aIter (rSet);
	sal_uInt16			 nWhich = aIter.FirstWhich ();
	SfxItemSet aAttr( GetDoc()->GetPool() );
	mpDrawView->GetAttributes( aAttr );

	while (nWhich)
	{
		switch (nWhich)
		{
			case SID_GETFILLSTYLE :
			{
				const XFillStyleItem &rFillStyleItem = (const XFillStyleItem &) aAttr.Get (XATTR_FILLSTYLE);

				rSet.Put (SfxUInt32Item (nWhich, (long) rFillStyleItem.GetValue ()));
				break;
			}

			case SID_GETLINESTYLE :
			{
				const XLineStyleItem &rLineStyleItem = (const XLineStyleItem &) aAttr.Get (XATTR_LINESTYLE);

				rSet.Put (SfxUInt32Item (nWhich, (long) rLineStyleItem.GetValue ()));
				break;
			}

			case SID_GETLINEWIDTH :
			{
				const XLineWidthItem &rLineWidthItem = (const XLineWidthItem &) aAttr.Get (XATTR_LINEWIDTH);

				rSet.Put (SfxUInt32Item (nWhich, (long) rLineWidthItem.GetValue ()));
				break;
			}

			case SID_GETGREEN :
			case SID_GETRED :
			case SID_GETBLUE :
			{
				const SfxUInt32Item &rWhatKind = (const SfxUInt32Item &) rSet.Get (ID_VAL_WHATKIND);
				Color				aColor;

				switch (rWhatKind.GetValue ())
				{
					case 1 :
					{
						const XLineColorItem &rLineColorItem = (const XLineColorItem &) aAttr.Get (XATTR_LINECOLOR);

						aColor = rLineColorItem.GetColorValue ();
						break;
					}

					case 2 :
					{
						const XFillColorItem &rFillColorItem = (const XFillColorItem &) aAttr.Get (XATTR_FILLCOLOR);

						aColor = rFillColorItem.GetColorValue ();
						break;
					}

					case 3 :
					case 4 :
					{
						const XFillGradientItem &rFillGradientItem = (const XFillGradientItem &) aAttr.Get (XATTR_FILLGRADIENT);
						const XGradient 		&rGradient		   = rFillGradientItem.GetGradientValue ();

						aColor = (rWhatKind.GetValue () == 3)
									? rGradient.GetStartColor ()
									: rGradient.GetEndColor ();
						break;
					}

					case 5:
					{
						const XFillHatchItem &rFillHatchItem = (const XFillHatchItem &) aAttr.Get (XATTR_FILLHATCH);
						const XHatch		 &rHatch		 = rFillHatchItem.GetHatchValue ();

						aColor = rHatch.GetColor ();
						break;
					}

					default :
						;
				}

				rSet.Put (SfxUInt32Item (nWhich, (long) ((nWhich == SID_GETRED)
															 ? aColor.GetRed ()
															 : (nWhich == SID_GETGREEN)
																   ? aColor.GetGreen ()
																   : aColor.GetBlue ())));
				break;
			}

			default :
				;
		}

		nWhich = aIter.NextWhich ();
	}
}

} // end of namespace sd
