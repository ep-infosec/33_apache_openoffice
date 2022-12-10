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

#include "fuediglu.hxx"
#include <svl/eitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdglue.hxx>
#include <sfx2/request.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "View.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"

namespace sd {

TYPEINIT1( FuEditGluePoints, FuDraw );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuEditGluePoints::FuEditGluePoints (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument*	pDoc,
    SfxRequest& rReq) 
    : FuDraw(pViewSh, pWin, pView, pDoc, rReq)
	 //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point, 
	 //and SHIFT+ENTER key to decide the position and draw the new insert point
	 ,bBeginInsertPoint(sal_False),
	oldPoint(0,0)
{
}

FunctionReference FuEditGluePoints::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent )
{
	FuEditGluePoints* pFunc;
	FunctionReference xFunc( pFunc = new FuEditGluePoints( pViewSh, pWin, pView, pDoc, rReq ) );
	xFunc->DoExecute(rReq);
	pFunc->SetPermanent( bPermanent );
	return xFunc;
}

void FuEditGluePoints::DoExecute( SfxRequest& rReq )
{
	FuDraw::DoExecute( rReq );
	mpView->SetInsGluePointMode(sal_False);
    mpViewShell->GetViewShellBase().GetToolBarManager()->AddToolBar(
        ToolBarManager::TBG_FUNCTION,
        ToolBarManager::msGluePointsToolBar);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuEditGluePoints::~FuEditGluePoints()
{
	mpView->BrkAction();
	mpView->UnmarkAllGluePoints();
	mpView->SetInsGluePointMode(sal_False);
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/

sal_Bool FuEditGluePoints::MouseButtonDown(const MouseEvent& rMEvt)
{
	mpView->SetActualWin( mpWindow );

	sal_Bool bReturn = FuDraw::MouseButtonDown(rMEvt);

	if (mpView->IsAction())
	{
		if (rMEvt.IsRight())
			mpView->BckAction();

		return sal_True;
	}

	if (rMEvt.IsLeft())
	{
		bReturn = sal_True;
		sal_uInt16 nHitLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(HITPIX,0)).Width() );
		sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
		mpWindow->CaptureMouse();

		SdrViewEvent aVEvt;
		SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

		if (eHit == SDRHIT_HANDLE)
		{
			/******************************************************************
			* Handle draggen
			******************************************************************/
			SdrHdl* pHdl = aVEvt.pHdl;

			if (mpView->IsGluePointMarked(aVEvt.pObj, aVEvt.nGlueId) && rMEvt.IsShift())
			{
				mpView->UnmarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
				pHdl = NULL;
			}

			if (pHdl)
			{
				// Handle draggen
				mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
			}
		}
		else if (eHit == SDRHIT_MARKEDOBJECT && mpView->IsInsGluePointMode())
		{
			/******************************************************************
			* Klebepunkt einfuegen
			******************************************************************/
			mpView->BegInsGluePoint(aMDPos);
		}
		else if (eHit == SDRHIT_MARKEDOBJECT && rMEvt.IsMod1())
		{
			/******************************************************************
			* Klebepunkt selektieren
			******************************************************************/
			if (!rMEvt.IsShift())
				mpView->UnmarkAllGluePoints();

			mpView->BegMarkGluePoints(aMDPos);
		}
		else if (eHit == SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() && !rMEvt.IsMod2())
		{
			/******************************************************************
			* Objekt verschieben
			******************************************************************/
			mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, NULL, nDrgLog);
		}
		else if (eHit == SDRHIT_GLUEPOINT)
		{
			/******************************************************************
			* Klebepunkt selektieren
			******************************************************************/
			if (!rMEvt.IsShift())
				mpView->UnmarkAllGluePoints();

			mpView->MarkGluePoint(aVEvt.pObj, aVEvt.nGlueId, aVEvt.pPV);
			SdrHdl* pHdl = mpView->GetGluePointHdl(aVEvt.pObj, aVEvt.nGlueId);

			if (pHdl)
			{
				mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
			}
		}
		else
		{
			/******************************************************************
			* Objekt selektieren oder draggen
			******************************************************************/
			if (!rMEvt.IsShift() && !rMEvt.IsMod2() && eHit == SDRHIT_UNMARKEDOBJECT)
			{
			   mpView->UnmarkAllObj();
			}

			sal_Bool bMarked = sal_False;

			if (!rMEvt.IsMod1())
			{
				if (rMEvt.IsMod2())
				{
					bMarked = mpView->MarkNextObj(aMDPos, nHitLog, rMEvt.IsShift());
				}
				else
				{
					bMarked = mpView->MarkObj(aMDPos, nHitLog, rMEvt.IsShift());
				}
			}

			if (bMarked &&
				(!rMEvt.IsShift() || eHit == SDRHIT_MARKEDOBJECT))
			{
				// Objekt verschieben
				mpView->BegDragObj(aMDPos, (OutputDevice*) NULL, aVEvt.pHdl, nDrgLog);
			}
			else if (mpView->AreObjectsMarked())
			{
				/**************************************************************
				* Klebepunkt selektieren
				**************************************************************/
				if (!rMEvt.IsShift())
					mpView->UnmarkAllGluePoints();

				mpView->BegMarkGluePoints(aMDPos);
			}
			else
			{
				/**************************************************************
				* Objekt selektieren
				**************************************************************/
				mpView->BegMarkObj(aMDPos);
			}
		}

		ForcePointer(&rMEvt);
	}

	return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/

sal_Bool FuEditGluePoints::MouseMove(const MouseEvent& rMEvt)
{
	mpView->SetActualWin( mpWindow );

	FuDraw::MouseMove(rMEvt);

	if (mpView->IsAction())
	{
		Point aPix(rMEvt.GetPosPixel());
		Point aPnt( mpWindow->PixelToLogic(aPix) );
		ForceScroll(aPix);
		mpView->MovAction(aPnt);
	}

	ForcePointer(&rMEvt);

	return sal_True;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/

sal_Bool FuEditGluePoints::MouseButtonUp(const MouseEvent& rMEvt)
{
	mpView->SetActualWin( mpWindow );

	sal_Bool bReturn = sal_False;

	if (mpView->IsAction())
	{
		bReturn = sal_True;
		mpView->EndAction();
	}

	FuDraw::MouseButtonUp(rMEvt);

	sal_uInt16 nDrgLog = sal_uInt16 ( mpWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
	Point aPos = mpWindow->PixelToLogic( rMEvt.GetPosPixel() );

	if (Abs(aMDPos.X() - aPos.X()) < nDrgLog &&
		Abs(aMDPos.Y() - aPos.Y()) < nDrgLog &&
		!rMEvt.IsShift() && !rMEvt.IsMod2())
	{
		SdrViewEvent aVEvt;
		SdrHitKind eHit = mpView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

		if (eHit == SDRHIT_NONE)
		{
			// Klick auf der Stelle: deselektieren
			mpView->UnmarkAllObj();
		}
	}

	mpWindow->ReleaseMouse();

	return bReturn;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

sal_Bool FuEditGluePoints::KeyInput(const KeyEvent& rKEvt)
{
	mpView->SetActualWin( mpWindow );

	//Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point, 
	//and SHIFT+ENTER key to decide the position and draw the new insert point

	sal_Bool bReturn = sal_False;
	
	switch (rKEvt.GetKeyCode().GetCode())
	{
		case KEY_UP:
		case KEY_DOWN:
		case KEY_LEFT:
		case KEY_RIGHT:
		{
			if(rKEvt.GetKeyCode().IsShift()&& mpView->IsInsGluePointMode() ){
				long nX = 0;
				long nY = 0;
				sal_uInt16  nCode = rKEvt.GetKeyCode().GetCode();
				if (nCode == KEY_UP)
				{
					// Scroll nach oben
					nX = 0;
					nY =-1;
				}
				else if (nCode == KEY_DOWN)
				{
					// Scroll nach unten
					nX = 0;
					nY = 1;
				}
				else if (nCode == KEY_LEFT)
				{
					// Scroll nach links
					nX =-1;
					nY = 0;
				}
				else if (nCode == KEY_RIGHT)
				{
					// Scroll nach rechts
					nX = 1;
					nY = 0;
				}
				Point centerPoint;
				Rectangle rect = mpView->GetMarkedObjRect();
				centerPoint = mpWindow->LogicToPixel(rect.Center());
				Point aPoint = bBeginInsertPoint? oldPoint:centerPoint;
				Point ePoint = aPoint + Point(nX,nY);
				mpWindow->SetPointerPosPixel(ePoint);
				//simulate mouse move action
				MouseEvent eMevt(ePoint,1,2,MOUSE_LEFT, 0);
				MouseMove(eMevt);
	                     Point aPix(eMevt.GetPosPixel());
				oldPoint = ePoint;
				bBeginInsertPoint = sal_True;
				bReturn = sal_True;
			}
		}
		break;
		case KEY_RETURN:
			if(rKEvt.GetKeyCode().IsShift() && mpView->IsInsGluePointMode() )
			{
				if(bBeginInsertPoint)
				{
					mpWindow->SetPointerPosPixel(oldPoint);
					//simulate mouse button down action
					MouseEvent aMevt(oldPoint,1,3,MOUSE_LEFT,KEY_SHIFT);
					// MT IA2: Not used?
					// sal_uInt16 ubuttons = aMevt.GetButtons();
					// sal_uInt16 uMod		= aMevt.GetModifier();
					MouseButtonDown(aMevt);
					mpWindow->CaptureMouse();
					//simulate mouse button up action
					MouseEvent rMEvt(oldPoint+Point(0,0),1,17, MOUSE_LEFT, KEY_SHIFT);
					MouseButtonUp(rMEvt);
					bReturn= sal_True;
				}
			}
			break;
	}
	if(!bReturn)
		bReturn = FuDraw::KeyInput(rKEvt);
	return bReturn;
}

 //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point, 
 //and SHIFT+ENTER key to decide the position and draw the new insert point
void FuEditGluePoints::ForcePointer(const MouseEvent* pMEvt)
{
	if(bBeginInsertPoint && pMEvt)
	{
		MouseEvent aMEvt(pMEvt->GetPosPixel(), pMEvt->GetClicks(), 
			pMEvt->GetMode(), pMEvt->GetButtons(), pMEvt->GetModifier() & ~KEY_SHIFT);  
		FuDraw::ForcePointer(&aMEvt);
	}
	else
	{
		FuDraw::ForcePointer(pMEvt);
	}
}
/*************************************************************************
|*
|* Command-event
|*
\************************************************************************/

sal_Bool FuEditGluePoints::Command(const CommandEvent& rCEvt)
{
	mpView->SetActualWin( mpWindow );
	return FuPoor::Command( rCEvt );
}

/*************************************************************************
|*
|* Funktion aktivieren
|*
\************************************************************************/

void FuEditGluePoints::Activate()
{
	mpView->SetGluePointEditMode();
	FuDraw::Activate();
}

/*************************************************************************
|*
|* Funktion deaktivieren
|*
\************************************************************************/

void FuEditGluePoints::Deactivate()
{
	mpView->SetGluePointEditMode( sal_False );
	FuDraw::Deactivate();
}

/*************************************************************************
|*
|* Request verarbeiten
|*
\************************************************************************/

void FuEditGluePoints::ReceiveRequest(SfxRequest& rReq)
{
	switch (rReq.GetSlot())
	{
		case SID_GLUE_INSERT_POINT:
		{
			mpView->SetInsGluePointMode(!mpView->IsInsGluePointMode());
		}
		break;

		case SID_GLUE_ESCDIR_LEFT:
		{
			mpView->SetMarkedGluePointsEscDir( SDRESC_LEFT,
					!mpView->IsMarkedGluePointsEscDir( SDRESC_LEFT ) );
		}
		break;

		case SID_GLUE_ESCDIR_RIGHT:
		{
			mpView->SetMarkedGluePointsEscDir( SDRESC_RIGHT,
					!mpView->IsMarkedGluePointsEscDir( SDRESC_RIGHT ) );
		}
		break;

		case SID_GLUE_ESCDIR_TOP:
		{
			mpView->SetMarkedGluePointsEscDir( SDRESC_TOP,
					!mpView->IsMarkedGluePointsEscDir( SDRESC_TOP ) );
		}
		break;

		case SID_GLUE_ESCDIR_BOTTOM:
		{
			mpView->SetMarkedGluePointsEscDir( SDRESC_BOTTOM,
					!mpView->IsMarkedGluePointsEscDir( SDRESC_BOTTOM ) );
		}
		break;

		case SID_GLUE_PERCENT:
		{
			const SfxItemSet* pSet = rReq.GetArgs();
			const SfxPoolItem& rItem = pSet->Get(SID_GLUE_PERCENT);
			sal_Bool bPercent = ((const SfxBoolItem&) rItem).GetValue();
			mpView->SetMarkedGluePointsPercent(bPercent);
		}
		break;

		case SID_GLUE_HORZALIGN_CENTER:
		{
			mpView->SetMarkedGluePointsAlign(sal_False, SDRHORZALIGN_CENTER);
		}
		break;

		case SID_GLUE_HORZALIGN_LEFT:
		{
			mpView->SetMarkedGluePointsAlign(sal_False, SDRHORZALIGN_LEFT);
		}
		break;

		case SID_GLUE_HORZALIGN_RIGHT:
		{
			mpView->SetMarkedGluePointsAlign(sal_False, SDRHORZALIGN_RIGHT);
		}
		break;

		case SID_GLUE_VERTALIGN_CENTER:
		{
			mpView->SetMarkedGluePointsAlign(sal_True, SDRVERTALIGN_CENTER);
		}
		break;

		case SID_GLUE_VERTALIGN_TOP:
		{
			mpView->SetMarkedGluePointsAlign(sal_True, SDRVERTALIGN_TOP);
		}
		break;

		case SID_GLUE_VERTALIGN_BOTTOM:
		{
			mpView->SetMarkedGluePointsAlign(sal_True, SDRVERTALIGN_BOTTOM);
		}
		break;
	}

	// Zum Schluss Basisklasse rufen
	FuPoor::ReceiveRequest(rReq);
}


} // end of namespace sd
