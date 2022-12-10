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

#include "MediaObjectBar.hxx"
#include <avmedia/mediaitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>

#include "app.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"

using namespace sd;

#define MediaObjectBar
#include "sdslots.hxx"

namespace sd {

// ------------------
// - MediaObjectBar -
// ------------------

TYPEINIT1( MediaObjectBar, SfxShell );

// -----------------------------------------------------------------------------

SFX_IMPL_INTERFACE( MediaObjectBar, SfxShell, SdResId( STR_MEDIAOBJECTBARSHELL ) )
{
}

// -----------------------------------------------------------------------------

MediaObjectBar::MediaObjectBar( ViewShell* pSdViewShell, ::sd::View* pSdView ) :
	SfxShell( pSdViewShell->GetViewShell() ),
    mpView( pSdView ),
    mpViewSh( pSdViewShell )
{
	DrawDocShell* pDocShell = mpViewSh->GetDocSh();

	SetPool( &pDocShell->GetPool() );
	SetUndoManager( pDocShell->GetUndoManager() );
	SetRepeatTarget( mpView );
	SetHelpId( SD_IF_SDDRAWMEDIAOBJECTBAR );
	SetName( String( SdResId( RID_DRAW_MEDIA_TOOLBOX ) ) );
}

// -----------------------------------------------------------------------------

MediaObjectBar::~MediaObjectBar()
{
	SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void MediaObjectBar::GetState( SfxItemSet& rSet )
{
	SfxWhichIter	aIter( rSet );
	sal_uInt16			nWhich = aIter.FirstWhich();

	while( nWhich )
	{
		if( SID_AVMEDIA_TOOLBOX == nWhich ) 
		{
			SdrMarkList* pMarkList = new SdrMarkList( mpView->GetMarkedObjectList() );
			bool		 bDisable = true;

			if( 1 == pMarkList->GetMarkCount() )
			{
				SdrObject* pObj =pMarkList->GetMark( 0 )->GetMarkedSdrObj();

				if( pObj && pObj->ISA( SdrMediaObj ) )
				{
					::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );
					
					static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
					rSet.Put( aItem );
					bDisable = false;
				}
			}

			if( bDisable )
				rSet.DisableItem( SID_AVMEDIA_TOOLBOX );
			
			delete pMarkList;
		}

		nWhich = aIter.NextWhich();
	}
}

// -----------------------------------------------------------------------------

void MediaObjectBar::Execute( SfxRequest& rReq )
{
	if( SID_AVMEDIA_TOOLBOX == rReq.GetSlot() ) 
	{
		const SfxItemSet*	pArgs = rReq.GetArgs();
		const SfxPoolItem*	pItem;
		
		if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, sal_False, &pItem ) ) )
			pItem = NULL;
			
		if( pItem )
		{
			SdrMarkList* pMarkList = new SdrMarkList( mpView->GetMarkedObjectList() );

			if( 1 == pMarkList->GetMarkCount() )
			{
				SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

				if( pObj && pObj->ISA( SdrMediaObj ) )
				{
					static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem( 
						static_cast< const ::avmedia::MediaItem& >( *pItem ) );
				}
			}

			delete pMarkList;
		}
	}
}

} // end of namespace sd
