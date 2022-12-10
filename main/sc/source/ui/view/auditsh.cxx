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



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svl/srchitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#include "auditsh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "document.hxx"

//------------------------------------------------------------------------

#define ScAuditingShell
#include "scslots.hxx"

//------------------------------------------------------------------------

TYPEINIT1( ScAuditingShell, SfxShell );

SFX_IMPL_INTERFACE(ScAuditingShell, SfxShell, ScResId(SCSTR_AUDITSHELL))
{
	SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_AUDIT) );
}


//------------------------------------------------------------------------

ScAuditingShell::ScAuditingShell(ScViewData* pData) :
	SfxShell(pData->GetViewShell()),
	pViewData( pData ),
	nFunction( SID_FILL_ADD_PRED )
{
	SetPool( &pViewData->GetViewShell()->GetPool() );
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
	SetHelpId( HID_SCSHELL_AUDIT );
	SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Auditing")));
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Auditing));
}

//------------------------------------------------------------------------

ScAuditingShell::~ScAuditingShell()
{
}

//------------------------------------------------------------------------

void ScAuditingShell::Execute( SfxRequest& rReq )
{
	SfxBindings& rBindings = pViewData->GetBindings();
	sal_uInt16 nSlot = rReq.GetSlot();
	switch ( nSlot )
	{
		case SID_FILL_ADD_PRED:
		case SID_FILL_DEL_PRED:
		case SID_FILL_ADD_SUCC:
		case SID_FILL_DEL_SUCC:
			nFunction = nSlot;
			rBindings.Invalidate( SID_FILL_ADD_PRED );
			rBindings.Invalidate( SID_FILL_DEL_PRED );
			rBindings.Invalidate( SID_FILL_ADD_SUCC );
			rBindings.Invalidate( SID_FILL_DEL_SUCC );
			break;
		case SID_CANCEL:		// Escape
		case SID_FILL_NONE:
			pViewData->GetViewShell()->SetAuditShell( sal_False );
			break;

		case SID_FILL_SELECT:
			{
				const SfxItemSet* pReqArgs = rReq.GetArgs();
				if ( pReqArgs )
				{
					const SfxPoolItem* pXItem;
					const SfxPoolItem* pYItem;
					if ( pReqArgs->GetItemState( SID_RANGE_COL, sal_True, &pXItem ) == SFX_ITEM_SET
					  && pReqArgs->GetItemState( SID_RANGE_ROW, sal_True, &pYItem ) == SFX_ITEM_SET )
					{
						DBG_ASSERT( pXItem->ISA(SfxInt16Item) && pYItem->ISA(SfxInt32Item),
										"falsche Items" );
						SCsCOL nCol = static_cast<SCsCOL>(((const SfxInt16Item*) pXItem)->GetValue());
						SCsROW nRow = static_cast<SCsROW>(((const SfxInt32Item*) pYItem)->GetValue());
						ScViewFunc* pView = pViewData->GetView();
						pView->MoveCursorAbs( nCol, nRow, SC_FOLLOW_LINE, sal_False, sal_False );
						switch ( nFunction )
						{
							case SID_FILL_ADD_PRED:
								pView->DetectiveAddPred();
								break;
							case SID_FILL_DEL_PRED:
								pView->DetectiveDelPred();
								break;
							case SID_FILL_ADD_SUCC:
								pView->DetectiveAddSucc();
								break;
							case SID_FILL_DEL_SUCC:
								pView->DetectiveDelSucc();
								break;
						}
					}
				}
			}
			break;
	}
}

//------------------------------------------------------------------------

void ScAuditingShell::GetState( SfxItemSet& rSet )
{
	rSet.Put( SfxBoolItem( nFunction, sal_True ) );			// aktive Funktion markieren
}
