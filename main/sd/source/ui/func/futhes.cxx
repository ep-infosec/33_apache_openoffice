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


#include "futhes.hxx"

#include <tools/pstm.hxx>
#include <editeng/outliner.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <editeng/eeitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/svxerr.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/processfactory.hxx>
#include "app.hrc"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "app.hxx"
#include "View.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "sdresid.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

class SfxRequest;

namespace sd {

TYPEINIT1( FuThesaurus, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuThesaurus::FuThesaurus( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
				  SdDrawDocument* pDoc, SfxRequest& rReq )
	   : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuThesaurus::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
	FunctionReference xFunc( new FuThesaurus( pViewSh, pWin, pView, pDoc, rReq ) );
	xFunc->DoExecute(rReq);
	return xFunc;
}

void FuThesaurus::DoExecute( SfxRequest& )
{
	SfxErrorContext aContext(ERRCTX_SVX_LINGU_THESAURUS, String(),
							 mpWindow, RID_SVXERRCTX, &DIALOG_MGR() );

	if( mpViewShell && mpViewShell->ISA(DrawViewShell) )
	{
		SdrTextObj* pTextObj = NULL;

		if ( mpView->AreObjectsMarked() )
		{
			const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

			if ( rMarkList.GetMarkCount() == 1 )
			{
				SdrMark* pMark = rMarkList.GetMark(0);
				SdrObject* pObj = pMark->GetMarkedSdrObj();

				if ( pObj->ISA(SdrTextObj) )
				{
					pTextObj = (SdrTextObj*) pObj;
				}
			}
		}

		::Outliner* pOutliner = mpView->GetTextEditOutliner();
		const OutlinerView* pOutlView = mpView->GetTextEditOutlinerView();

		if ( pTextObj && pOutliner && pOutlView )
		{
			if ( !pOutliner->GetSpeller().is() )
			{
	            Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
	            if ( xSpellChecker.is() )
		            pOutliner->SetSpeller( xSpellChecker );

	            Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
	            if( xHyphenator.is() )
		            pOutliner->SetHyphenator( xHyphenator );

				pOutliner->SetDefaultLanguage( mpDoc->GetLanguage( EE_CHAR_LANGUAGE ) );
			}

			EESpellState eState = ( (OutlinerView*) pOutlView)->StartThesaurus();
			DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

			if (eState == EE_SPELL_NOLANGUAGE)
			{
				ErrorBox(mpWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
			}
		}
	}
	else if ( mpViewShell->ISA(OutlineViewShell) )
	{
		Outliner* pOutliner = mpDoc->GetOutliner();
		OutlinerView* pOutlView = pOutliner->GetView(0);

		if ( !pOutliner->GetSpeller().is() )
		{
	        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
	        if ( xSpellChecker.is() )
		        pOutliner->SetSpeller( xSpellChecker );

	        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
	        if( xHyphenator.is() )
		        pOutliner->SetHyphenator( xHyphenator );

			pOutliner->SetDefaultLanguage( mpDoc->GetLanguage( EE_CHAR_LANGUAGE ) );
		}

		EESpellState eState = pOutlView->StartThesaurus();
		DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

		if (eState == EE_SPELL_NOLANGUAGE)
		{
			ErrorBox(mpWindow, WB_OK, String(SdResId(STR_NOLANGUAGE))).Execute();
		}
	}
}

} // end of namespace sd
