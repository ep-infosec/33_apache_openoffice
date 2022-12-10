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

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <editeng/flstitem.hxx>

#include <svx/svxids.hrc>

#include <svx/drawitem.hxx>
#include <svl/intitem.hxx>
#include <svx/ofaitem.hxx>
#include <svx/svxgrahicitem.hxx>
#include <svx/svdmodel.hxx>
#include <svl/cjkoptions.hxx>


#include <svx/dialogs.hrc>

#include <svx/svxdlg.hxx>
#include <svx/tabline.hxx>
#include <svl/style.hxx>
#include <svx/xtable.hxx>

#include "DrawDocShell.hxx"
#include "tabtempl.hxx"
#include "tabtempl.hrc"
#include "sdresid.hxx"
#include "dlg_char.hxx"
#include "paragr.hxx"
#include <svx/flagsdef.hxx>

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdTabTemplateDlg::SdTabTemplateDlg( Window* pParent,
								const SfxObjectShell* pDocShell,
								SfxStyleSheetBase& rStyleBase,
								SdrModel* pModel,
								SdrView* pView ) :
		SfxStyleDialog      ( pParent, SdResId( TAB_TEMPLATE ), rStyleBase, sal_False ),
		rDocShell			( *pDocShell ),
		pSdrView			( pView ),
		maColorTab			( pModel->GetColorTableFromSdrModel() ),
		maGradientList		( pModel->GetGradientListFromSdrModel() ),
		maHatchingList		( pModel->GetHatchListFromSdrModel() ),
		maBitmapList 		( pModel->GetBitmapListFromSdrModel() ),
		maDashList			( pModel->GetDashListFromSdrModel() ),
		maLineEndList		( pModel->GetLineEndListFromSdrModel() )
{
	FreeResource();

	// Listbox fuellen und Select-Handler ueberladen

	AddTabPage( RID_SVXPAGE_LINE);
	AddTabPage( RID_SVXPAGE_AREA);
	AddTabPage( RID_SVXPAGE_SHADOW);
	AddTabPage( RID_SVXPAGE_TRANSPARENCE);
	AddTabPage( RID_SVXPAGE_CHAR_NAME );
	AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
	AddTabPage( RID_SVXPAGE_STD_PARAGRAPH );
	AddTabPage( RID_SVXPAGE_TEXTATTR );
	AddTabPage( RID_SVXPAGE_TEXTANIMATION );
	AddTabPage( RID_SVXPAGE_MEASURE);
	AddTabPage( RID_SVXPAGE_CONNECTION);
	AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH );
	AddTabPage( RID_SVXPAGE_TABULATOR );
	SvtCJKOptions aCJKOptions;
	if( aCJKOptions.IsAsianTypographyEnabled() )
		AddTabPage( RID_SVXPAGE_PARA_ASIAN );
	else
		RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );

	nDlgType = 1;
	nPageType = 0;
	nPos = 0;

	nColorTableState = CT_NONE;
	nBitmapListState = CT_NONE;
	nGradientListState = CT_NONE;
	nHatchingListState = CT_NONE;
}

// -----------------------------------------------------------------------

SdTabTemplateDlg::~SdTabTemplateDlg()
{
}

// -----------------------------------------------------------------------

void SdTabTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
	switch( nId )
	{
		case RID_SVXPAGE_LINE:
			aSet.Put (SvxColorTableItem(maColorTab,SID_COLOR_TABLE));
			aSet.Put (SvxDashListItem(maDashList,SID_DASH_LIST));
			aSet.Put (SvxLineEndListItem(maLineEndList,SID_LINEEND_LIST));
			aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
			rPage.PageCreated(aSet);
			break;

		case RID_SVXPAGE_AREA:
			aSet.Put (SvxColorTableItem(maColorTab,SID_COLOR_TABLE));
			aSet.Put (SvxGradientListItem(maGradientList,SID_GRADIENT_LIST));
			aSet.Put (SvxHatchListItem(maHatchingList,SID_HATCH_LIST));
			aSet.Put (SvxBitmapListItem(maBitmapList,SID_BITMAP_LIST));
			aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
			aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
			aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,nPos));
			rPage.PageCreated(aSet);


		break;

		case RID_SVXPAGE_SHADOW:
				aSet.Put (SvxColorTableItem(maColorTab,SID_COLOR_TABLE)); //add CHINA001
				aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
				aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
				rPage.PageCreated(aSet);
			break;

		case RID_SVXPAGE_TRANSPARENCE:
					aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
					aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
					rPage.PageCreated(aSet);
		break;

		case RID_SVXPAGE_CHAR_NAME:
		{
			SvxFontListItem aItem(*( (const SvxFontListItem*)
				( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

			aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
			rPage.PageCreated(aSet);
		}
		break;

		case RID_SVXPAGE_CHAR_EFFECTS:
			aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
			rPage.PageCreated(aSet);
		break;

		case RID_SVXPAGE_STD_PARAGRAPH:
		break;

		case RID_SVXPAGE_TEXTATTR:
		{
			aSet.Put(OfaPtrItem(SID_SVXTEXTATTRPAGE_VIEW,pSdrView));
			rPage.PageCreated(aSet);
		}
		break;

		case RID_SVXPAGE_TEXTANIMATION:
		break;

		case RID_SVXPAGE_MEASURE:
			aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
			rPage.PageCreated(aSet);
		break;

		case RID_SVXPAGE_CONNECTION:
		{
			aSet.Put (OfaPtrItem(SID_OBJECT_LIST,pSdrView));
			rPage.PageCreated(aSet);
		}
		break;
	}
}

// -----------------------------------------------------------------------

const SfxItemSet* SdTabTemplateDlg::GetRefreshedSet()
{
    SfxItemSet* pRet = GetInputSetImpl();

    if( pRet )
    {
        pRet->ClearItem();
        pRet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    }
    else
        pRet = new SfxItemSet( GetStyleSheet().GetItemSet() );

	return pRet;
}



