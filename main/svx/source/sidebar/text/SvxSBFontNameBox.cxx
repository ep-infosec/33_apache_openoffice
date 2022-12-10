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

#include "precompiled_svx.hxx"

#include "SvxSBFontNameBox.hxx"

#include <unotools/fontoptions.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>


const static sal_uInt16 MAX_MRU_FONTNAME_ENTRIES = 5;


namespace svx { namespace sidebar {

namespace {
    bool GetDocFontList_Impl( const FontList** ppFontList, SvxSBFontNameBox* pBox )
    {
        bool bChanged = false;
        const SfxObjectShell* pDocSh = SfxObjectShell::Current();
        SvxFontListItem* pFontListItem = NULL;

        if ( pDocSh )
            pFontListItem =
                (SvxFontListItem*)pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

        if ( pFontListItem )
        {
            const FontList*	pNewFontList = pFontListItem->GetFontList();
            DBG_ASSERT( pNewFontList, "Doc-FontList not available!" );

            if ( !*ppFontList )
            {
                *ppFontList = pNewFontList;
                bChanged = true;
            }
            else
            {
                bChanged = ( *ppFontList != pNewFontList );
                if( !bChanged && pBox!=NULL )
                    bChanged = ( pBox->GetListCount() != pNewFontList->GetFontNameCount() );
                HACK(vergleich ist unvollstaendig)

                    if ( bChanged )
                        *ppFontList = pNewFontList;
            }

            if ( pBox )
                pBox->Enable();
        }
        else if ( pBox )
            pBox->Disable();

        // in die FontBox ggf. auch die neue Liste f"ullen
        if ( pBox && bChanged )
        {
            if ( *ppFontList )
                pBox->Fill( *ppFontList );
            else
                pBox->Clear();
        }
        return bChanged;
    }
}




SvxSBFontNameBox::SvxSBFontNameBox( Window* pParent,  const ResId& rResId  ) :
	FontNameBox	( pParent, rResId )
,	pFontList	( NULL )
,	nFtCount	( 0 )
,	bInput(false)
,	pBindings(NULL)
{
    EnableControls_Impl();
//	StartListening( *SFX_APP() );
}

void SvxSBFontNameBox::EnableControls_Impl()
{
	SvtFontOptions aFontOpt;
	bool bEnable = aFontOpt.IsFontHistoryEnabled();
	sal_uInt16 nEntries = bEnable ? MAX_MRU_FONTNAME_ENTRIES : 0;
	if ( GetMaxMRUCount() != nEntries )
	{
		// refill in the next GetFocus-Handler
		pFontList = NULL;
		Clear();
		SetMaxMRUCount( nEntries );
	}

	bEnable = aFontOpt.IsFontWYSIWYGEnabled();
	EnableWYSIWYG( bEnable );
	EnableSymbols( bEnable );
}

void SvxSBFontNameBox::FillList()
{
	Selection aOldSel = GetSelection();
	GetDocFontList_Impl( &pFontList, this );
	aCurText = GetText();
	SetSelection( aOldSel );
}

long SvxSBFontNameBox::PreNotify( NotifyEvent& rNEvt )
{
	const sal_uInt16 nType (rNEvt.GetType());

	if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
		FillList();
    return FontNameBox::PreNotify( rNEvt );
}
//<<modify
long SvxSBFontNameBox::Notify( NotifyEvent& rNEvt) //SfxBroadcaster& rBC, const SfxHint& rHint 
{
	//SfxItemSetHint* pHint = PTR_CAST(SfxItemSetHint, &rHint);
	//if ( pHint )
	//	EnableControls_Impl();
	bool bHandle = 0;
	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const sal_uInt16 nCode (rNEvt.GetKeyEvent()->GetKeyCode().GetCode());

		if( nCode == KEY_RETURN)
		{
			bHandle = 1;
			Select();
		}
	}

	return bHandle ? bHandle : FontNameBox::Notify( rNEvt );
} 
void SvxSBFontNameBox::Select()
{
	FontNameBox::Select();

	if ( !IsTravelSelect() )
	{
		FillList();
		FontInfo aInfo( pFontList->Get( GetText(),WEIGHT_NORMAL, ITALIC_NORMAL ) );//meWeight, meItalic 
    
		SvxFontItem aFontItem( aInfo.GetFamily(), aInfo.GetName(), aInfo.GetStyleName(),
			aInfo.GetPitch(), aInfo.GetCharSet(), SID_ATTR_CHAR_FONT );

		pBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONT, SFX_CALLMODE_RECORD, &aFontItem, 0L );
		pBindings->Invalidate(SID_ATTR_CHAR_FONT,true,false);
	}
}
void SvxSBFontNameBox::SetBindings(SfxBindings* pB)
{
	pBindings = pB;
}

} } // end of namespace svx::sidebar
