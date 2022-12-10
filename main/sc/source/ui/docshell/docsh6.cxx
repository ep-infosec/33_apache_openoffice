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



#ifndef PCH
#include "scitems.hxx"

#include <svx/pageitem.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/linkmgr.hxx>
#endif

// INCLUDE ---------------------------------------------------------------

//#include <svxlink.hxx>

#include "docsh.hxx"

#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "tablink.hxx"
#include "collect.hxx"

struct ScStylePair
{
	SfxStyleSheetBase *pSource;
	SfxStyleSheetBase *pDest;
};


// STATIC DATA -----------------------------------------------------------

//----------------------------------------------------------------------

//
//	Ole
//

void __EXPORT ScDocShell::SetVisArea( const Rectangle & rVisArea )
{
	//	with the SnapVisArea call in SetVisAreaOrSize, it's safe to always
	//	use both the size and position of the VisArea
	SetVisAreaOrSize( rVisArea, sal_True );
}

void lcl_SetTopRight( Rectangle& rRect, const Point& rPos )
{
    Size aSize = rRect.GetSize();
    rRect.Right() = rPos.X();
    rRect.Left() = rPos.X() - aSize.Width() + 1;
    rRect.Top() = rPos.Y();
    rRect.Bottom() = rPos.Y() + aSize.Height() - 1;
}

void ScDocShell::SetVisAreaOrSize( const Rectangle& rVisArea, sal_Bool bModifyStart )
{
    sal_Bool bNegativePage = aDocument.IsNegativePage( aDocument.GetVisibleTab() );

	Rectangle aArea = rVisArea;
	if (bModifyStart)
	{
	    // when loading, don't check for negative values, because the sheet orientation
	    // might be set later
    	if ( !aDocument.IsImportingXML() )
    	{
    		if ( ( bNegativePage ? (aArea.Right() > 0) : (aArea.Left() < 0) ) || aArea.Top() < 0 )
    		{
    			//	VisArea start position can't be negative.
    			//	Move the VisArea, otherwise only the upper left position would
    			//	be changed in SnapVisArea, and the size would be wrong.

                Point aNewPos( 0, Max( aArea.Top(), (long) 0 ) );
                if ( bNegativePage )
                {
                    aNewPos.X() = Min( aArea.Right(), (long) 0 );
                    lcl_SetTopRight( aArea, aNewPos );
                }
                else
                {
                    aNewPos.X() = Max( aArea.Left(), (long) 0 );
                    aArea.SetPos( aNewPos );
                }
    		}
    	}
	}
	else
    {
        Rectangle aOldVisArea = SfxObjectShell::GetVisArea();
        if ( bNegativePage )
            lcl_SetTopRight( aArea, aOldVisArea.TopRight() );
        else
            aArea.SetPos( aOldVisArea.TopLeft() );
    }

	//		hier Position anpassen!

	//	#92248# when loading an ole object, the VisArea is set from the document's
	//	view settings and must be used as-is (document content may not be complete yet).
	if ( !aDocument.IsImportingXML() )
		aDocument.SnapVisArea( aArea );

    //TODO/LATER: it's unclear which IPEnv is used here
    /*
	SvInPlaceEnvironment* pEnv = GetIPEnv();
	if (pEnv)
	{
		Window* pWin = pEnv->GetEditWin();
		pEnv->MakeScale( aArea.GetSize(), MAP_100TH_MM,
							pWin->LogicToPixel( aArea.GetSize() ) );
    } */

    //TODO/LATER: formerly in SvInplaceObject
    SfxObjectShell::SetVisArea( aArea );

	if (bIsInplace)						// Zoom in der InPlace View einstellen
	{
		ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
		if (pViewSh)
		{
			if (pViewSh->GetViewData()->GetDocShell() == this)
				pViewSh->UpdateOleZoom();
		}
		//else
		//	DataChanged( SvDataType() );			// fuer Zuppeln wenn nicht IP-aktiv
	}

	if (aDocument.IsEmbedded())
	{
		ScRange aOld;
		aDocument.GetEmbedded( aOld);
		aDocument.SetEmbedded( aArea );
		ScRange aNew;
		aDocument.GetEmbedded( aNew);
		if (aOld != aNew)
			PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB,PAINT_GRID);

        //TODO/LATER: currently not implemented
        //ViewChanged( ASPECT_CONTENT );          // auch im Container anzeigen
	}
}

sal_Bool ScDocShell::IsOle()
{
	return (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
}

void ScDocShell::UpdateOle( const ScViewData* pViewData, sal_Bool bSnapSize )
{
	//	wenn's gar nicht Ole ist, kann man sich die Berechnungen sparen
	//	(VisArea wird dann beim Save wieder zurueckgesetzt)

	if (GetCreateMode() == SFX_CREATE_MODE_STANDARD)
		return;

	DBG_ASSERT(pViewData,"pViewData==0 bei ScDocShell::UpdateOle");

    Rectangle aOldArea = SfxObjectShell::GetVisArea();
	Rectangle aNewArea = aOldArea;

	sal_Bool bChange = sal_False;
	sal_Bool bEmbedded = aDocument.IsEmbedded();
	if (bEmbedded)
		aNewArea = aDocument.GetEmbeddedRect();
	else
	{
	    SCTAB nTab = pViewData->GetTabNo();
		if ( nTab != aDocument.GetVisibleTab() )
		{
			aDocument.SetVisibleTab( nTab );
			bChange = sal_True;
		}

        sal_Bool bNegativePage = aDocument.IsNegativePage( nTab );
		SCCOL nX = pViewData->GetPosX(SC_SPLIT_LEFT);
		SCROW nY = pViewData->GetPosY(SC_SPLIT_BOTTOM);
        Rectangle aMMRect = aDocument.GetMMRect( nX,nY, nX,nY, nTab );
        if (bNegativePage)
            lcl_SetTopRight( aNewArea, aMMRect.TopRight() );
        else
            aNewArea.SetPos( aMMRect.TopLeft() );
		if (bSnapSize)
			aDocument.SnapVisArea(aNewArea);            // uses the new VisibleTab
	}

	if (aNewArea != aOldArea)
	{
		SetVisAreaOrSize( aNewArea, sal_True );	// hier muss auch der Start angepasst werden
		bChange = sal_True;
	}

//	if (bChange)
//		DataChanged( SvDataType() );		//! passiert auch bei SetModified
}

//
//	Style-Krempel fuer Organizer etc.
//

SfxStyleSheetBasePool* __EXPORT ScDocShell::GetStyleSheetPool()
{
	return (SfxStyleSheetBasePool*)aDocument.GetStyleSheetPool();
}


//	nach dem Laden von Vorlagen aus einem anderen Dokment (LoadStyles, Insert)
//	muessen die SetItems (ATTR_PAGE_HEADERSET, ATTR_PAGE_FOOTERSET) auf den richtigen
//	Pool umgesetzt werden, bevor der Quell-Pool geloescht wird.

void lcl_AdjustPool( SfxStyleSheetBasePool* pStylePool )
{
	pStylePool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, 0xffff);
	SfxStyleSheetBase *pStyle = pStylePool->First();
	while ( pStyle )
	{
		SfxItemSet& rStyleSet = pStyle->GetItemSet();

		const SfxPoolItem* pItem;
		if (rStyleSet.GetItemState(ATTR_PAGE_HEADERSET,sal_False,&pItem) == SFX_ITEM_SET)
		{
			SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
			SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
			pDestSet->Put(rSrcSet);
			rStyleSet.Put(SvxSetItem(ATTR_PAGE_HEADERSET,pDestSet));
		}
		if (rStyleSet.GetItemState(ATTR_PAGE_FOOTERSET,sal_False,&pItem) == SFX_ITEM_SET)
		{
			SfxItemSet& rSrcSet = ((SvxSetItem*)pItem)->GetItemSet();
			SfxItemSet* pDestSet = new SfxItemSet(*rStyleSet.GetPool(),rSrcSet.GetRanges());
			pDestSet->Put(rSrcSet);
			rStyleSet.Put(SvxSetItem(ATTR_PAGE_FOOTERSET,pDestSet));
		}

		pStyle = pStylePool->Next();
	}
}

void __EXPORT ScDocShell::LoadStyles( SfxObjectShell &rSource )
{
	aDocument.StylesToNames();

	SfxObjectShell::LoadStyles(rSource);
	lcl_AdjustPool( GetStyleSheetPool() );		// SetItems anpassen

	aDocument.UpdStlShtPtrsFrmNms();

	UpdateAllRowHeights();

		//	Paint

	PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );
}

void ScDocShell::LoadStylesArgs( ScDocShell& rSource, sal_Bool bReplace, sal_Bool bCellStyles, sal_Bool bPageStyles )
{
	//	similar to LoadStyles, but with selectable behavior for XStyleLoader::loadStylesFromURL call

	if ( !bCellStyles && !bPageStyles )		// nothing to do
		return;

	ScStyleSheetPool* pSourcePool = rSource.GetDocument()->GetStyleSheetPool();
	ScStyleSheetPool* pDestPool = aDocument.GetStyleSheetPool();

	SfxStyleFamily eFamily = bCellStyles ?
			( bPageStyles ? SFX_STYLE_FAMILY_ALL : SFX_STYLE_FAMILY_PARA ) :
			SFX_STYLE_FAMILY_PAGE;
	SfxStyleSheetIterator aIter( pSourcePool, eFamily );
	sal_uInt16 nSourceCount = aIter.Count();
	if ( nSourceCount == 0 )
		return;								// no source styles

	ScStylePair* pStyles = new ScStylePair[ nSourceCount ];
	sal_uInt16 nFound = 0;

	//	first create all new styles

	SfxStyleSheetBase* pSourceStyle = aIter.First();
	while (pSourceStyle)
	{
		String aName = pSourceStyle->GetName();
		SfxStyleSheetBase* pDestStyle = pDestPool->Find( pSourceStyle->GetName(), pSourceStyle->GetFamily() );
		if ( pDestStyle )
		{
			// touch existing styles only if replace flag is set
			if ( bReplace )
			{
				pStyles[nFound].pSource = pSourceStyle;
				pStyles[nFound].pDest = pDestStyle;
				++nFound;
			}
		}
		else
		{
			pStyles[nFound].pSource = pSourceStyle;
			pStyles[nFound].pDest = &pDestPool->Make( aName, pSourceStyle->GetFamily(), pSourceStyle->GetMask() );
			++nFound;
		}

		pSourceStyle = aIter.Next();
	}

	//	then copy contents (after inserting all styles, for parent etc.)

	for ( sal_uInt16 i = 0; i < nFound; ++i )
	{
		pStyles[i].pDest->GetItemSet().PutExtended(
			pStyles[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
		if(pStyles[i].pSource->HasParentSupport())
			pStyles[i].pDest->SetParent(pStyles[i].pSource->GetParent());
		// follow is never used
	}

	lcl_AdjustPool( GetStyleSheetPool() );		// adjust SetItems
	UpdateAllRowHeights();
	PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID | PAINT_LEFT );		// Paint

	delete[] pStyles;
}


sal_Bool __EXPORT ScDocShell::Insert( SfxObjectShell &rSource,
								sal_uInt16 nSourceIdx1, sal_uInt16 nSourceIdx2, sal_uInt16 nSourceIdx3,
								sal_uInt16 &nIdx1, sal_uInt16 &nIdx2, sal_uInt16 &nIdx3, sal_uInt16 &rIdxDeleted )
{
	sal_Bool bRet = SfxObjectShell::Insert( rSource, nSourceIdx1, nSourceIdx2, nSourceIdx3,
											nIdx1, nIdx2, nIdx3, rIdxDeleted );
	if (bRet)
		lcl_AdjustPool( GetStyleSheetPool() );		// SetItems anpassen

	return bRet;
}

void ScDocShell::UpdateLinks()
{
	sfx2::LinkManager* pLinkManager = aDocument.GetLinkManager();
	ScStrCollection aNames;

	// nicht mehr benutzte Links raus

	sal_uInt16 nCount = pLinkManager->GetLinks().Count();
	for (sal_uInt16 k=nCount; k>0; )
	{
		--k;
		::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[k];
		if (pBase->ISA(ScTableLink))
		{
			ScTableLink* pTabLink = (ScTableLink*)pBase;
			if (pTabLink->IsUsed())
			{
				StrData* pData = new StrData(pTabLink->GetFileName());
				if (!aNames.Insert(pData))
					delete pData;
			}
			else		// nicht mehr benutzt -> loeschen
			{
				pTabLink->SetAddUndo(sal_True);
				pLinkManager->Remove(k);
			}
		}
	}


	// neue Links eintragen

	SCTAB nTabCount = aDocument.GetTableCount();
	for (SCTAB i=0; i<nTabCount; i++)
		if (aDocument.IsLinked(i))
		{
			String aDocName = aDocument.GetLinkDoc(i);
			String aFltName = aDocument.GetLinkFlt(i);
			String aOptions = aDocument.GetLinkOpt(i);
			sal_uLong nRefresh	= aDocument.GetLinkRefreshDelay(i);
			sal_Bool bThere = sal_False;
			for (SCTAB j=0; j<i && !bThere; j++)				// im Dokument mehrfach?
				if (aDocument.IsLinked(j)
						&& aDocument.GetLinkDoc(j) == aDocName
						&& aDocument.GetLinkFlt(j) == aFltName
						&& aDocument.GetLinkOpt(j) == aOptions)
						// Ignore refresh delay in compare, it should be the
						// same for identical links and we don't want dupes
						// if it ain't.
					bThere = sal_True;

			if (!bThere)										// schon als Filter eingetragen?
			{
				StrData* pData = new StrData(aDocName);
				if (!aNames.Insert(pData))
				{
					delete pData;
					bThere = sal_True;
				}
			}
			if (!bThere)
			{
				ScTableLink* pLink = new ScTableLink( this, aDocName, aFltName, aOptions, nRefresh );
				pLink->SetInCreate( sal_True );
				pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aDocName, &aFltName );
				pLink->Update();
				pLink->SetInCreate( sal_False );
			}
		}
}

sal_Bool ScDocShell::ReloadTabLinks()
{
	sfx2::LinkManager* pLinkManager = aDocument.GetLinkManager();

	sal_Bool bAny = sal_False;
	sal_uInt16 nCount = pLinkManager->GetLinks().Count();
	for (sal_uInt16 i=0; i<nCount; i++ )
	{
        ::sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[i];
		if (pBase->ISA(ScTableLink))
		{
			ScTableLink* pTabLink = (ScTableLink*)pBase;
//			pTabLink->SetAddUndo(sal_False);		//! Undo's zusammenfassen
			// Painting only after Update() makes no sense:
			// ScTableLink::Refresh() will post a Paint only is bDoPaint is true
			//pTabLink->SetPaint(sal_False);			//	Paint nur einmal am Ende
			pTabLink->Update();
			//pTabLink->SetPaint(sal_True);
//			pTabLink->SetAddUndo(sal_True);
			bAny = sal_True;
		}
	}

	if ( bAny )
	{
		//	Paint nur einmal
		PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
									PAINT_GRID | PAINT_TOP | PAINT_LEFT );

		SetDocumentModified();
	}

	return sal_True;		//! Fehler erkennen
}


