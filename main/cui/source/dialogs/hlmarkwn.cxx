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
#include "precompiled_cui.hxx"

#include <vcl/wrkwin.hxx>
#include <dialmgr.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

// UNO-Stuff
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <cuires.hrc>
#include "hlmarkwn.hrc"
#include "hlmarkwn.hxx"
#include "hltpbase.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;

/*************************************************************************
|*
|* Userdata-struct for tree-entries
|*
|************************************************************************/

struct TargetData
{
	OUString		aUStrLinkname;
	sal_Bool		bIsTarget;

	TargetData ( OUString aUStrLName, sal_Bool bTarget )
		:	bIsTarget ( bTarget )
	{
		if ( bIsTarget )
			aUStrLinkname = aUStrLName;
	}
};


//########################################################################
//#                                                                      #
//# Tree-Window                                                          #
//#                                                                      #
//########################################################################

SvxHlmarkTreeLBox::SvxHlmarkTreeLBox( Window* pParent, const ResId& rResId )
: SvTreeListBox	( pParent, rResId ),
  mpParentWnd	( (SvxHlinkDlgMarkWnd*) pParent )
{
	SetNodeDefaultImages();
}

void SvxHlmarkTreeLBox::Paint( const Rectangle& rRect )
{
	if( mpParentWnd->mnError == LERR_NOERROR )
	{
		SvTreeListBox::Paint(rRect);
	}
	else
	{
		Erase();

		Rectangle aDrawRect( Point( 0, 0 ), GetSizePixel() );

		String aStrMessage;

		switch( mpParentWnd->mnError )
		{
		case LERR_NOENTRIES :
			aStrMessage = CUI_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_NOENTRIES );
			break;
		case LERR_DOCNOTOPEN :
			aStrMessage = CUI_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_DOCNOTOPEN );
			break;
		}

		DrawText( aDrawRect, aStrMessage, TEXT_DRAW_LEFT | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
	}

}

//########################################################################
//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHlinkDlgMarkWnd::SvxHlinkDlgMarkWnd( SvxHyperlinkTabPageBase *pParent )
:	ModalDialog( (Window*)pParent, CUI_RES ( RID_SVXFLOAT_HYPERLINK_MARKWND ) ),
	maBtApply( this, CUI_RES (BT_APPLY) ),
	maBtClose( this, CUI_RES (BT_CLOSE) ),
	maLbTree ( this, CUI_RES (TLB_MARK) ),
	mbUserMoved ( sal_False ),
	mbFirst	    ( sal_True ),
	mpParent	( pParent ),
	mnError		( LERR_NOERROR )
{
	FreeResource();

	maBtApply.SetClickHdl		( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );
	maBtClose.SetClickHdl		( LINK ( this, SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl ) );
	maLbTree.SetDoubleClickHdl	( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );

	// Tree-ListBox mit Linien versehen
	maLbTree.SetStyle( maLbTree.GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
							WB_HASBUTTONS |  //WB_HASLINESATROOT |
							WB_HSCROLL | WB_HASBUTTONSATROOT );
							
    maLbTree.SetAccessibleName(String(CUI_RES(STR_MARK_TREE)));
							
}

SvxHlinkDlgMarkWnd::~SvxHlinkDlgMarkWnd()
{
	ClearTree();
}

/*************************************************************************
|*
|* Set an errorstatus
|*
|************************************************************************/

sal_uInt16 SvxHlinkDlgMarkWnd::SetError( sal_uInt16 nError)
{
	sal_uInt16 nOldError = mnError;
	mnError = nError;

	if( mnError != LERR_NOERROR )
		ClearTree();

	maLbTree.Invalidate();

	return nOldError;
}

/*************************************************************************
|*
|* Move window
|*
|************************************************************************/

sal_Bool SvxHlinkDlgMarkWnd::MoveTo ( Point aNewPos )
{
	if ( !mbUserMoved )
	{
		sal_Bool bOldStatus = mbUserMoved;
		SetPosPixel ( aNewPos );
		mbUserMoved = bOldStatus;
	}

	return mbUserMoved;
}

void SvxHlinkDlgMarkWnd::Move ()
{
	Window::Move();

	if ( IsReallyVisible() )
		mbUserMoved = sal_True;
}

sal_Bool SvxHlinkDlgMarkWnd::ConnectToDialog( sal_Bool bDoit )
{
	sal_Bool bOldStatus = mbUserMoved;

	mbUserMoved = !bDoit;

	return bOldStatus;
}

/*************************************************************************
|*
|* Interface to refresh tree
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::RefreshTree ( String aStrURL )
{
    String aEmptyStr;
	OUString aUStrURL;

	EnterWait();

	ClearTree();

	xub_StrLen nPos = aStrURL.Search ( sal_Unicode('#') );

	if( nPos != 0 )
		aUStrURL = ::rtl::OUString( aStrURL );

	if( !RefreshFromDoc ( aUStrURL ) )
		maLbTree.Invalidate();

	if ( nPos != STRING_NOTFOUND )
	{
		String aStrMark = aStrURL.Copy ( nPos+1 );
		SelectEntry ( aStrMark );
	}

	LeaveWait();

	maStrLastURL = aStrURL;
}

/*************************************************************************
|*
|* get links from document
|*
|************************************************************************/

sal_Bool SvxHlinkDlgMarkWnd::RefreshFromDoc( OUString aURL )
{
	mnError = LERR_NOERROR;

	uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
	if( xFactory.is() )
	{
		uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
					uno::UNO_QUERY );
		if( xDesktop.is() )
		{
			uno::Reference< lang::XComponent > xComp;

			if( aURL.getLength() )
			{
				// load from url
				uno::Reference< frame::XComponentLoader > xLoader( xDesktop, uno::UNO_QUERY );
				if( xLoader.is() )
				{
					try
					{
						uno::Sequence< beans::PropertyValue > aArg(1);
						aArg.getArray()[0].Name = OUString::createFromAscii( "Hidden" );
						aArg.getArray()[0].Value <<= (sal_Bool) sal_True;
						xComp = xLoader->loadComponentFromURL( aURL, OUString::createFromAscii( "_blank" ), 0, aArg );
					}
					catch( const io::IOException& )
					{

					}
					catch( const lang::IllegalArgumentException& )
					{

					}
				}
			}
			else
			{
				// the component with user focus ( current document )
				xComp = xDesktop->getCurrentComponent();
			}

			if( xComp.is() )
			{
				uno::Reference< document::XLinkTargetSupplier > xLTS( xComp, uno::UNO_QUERY );

				if( xLTS.is() )
				{
					if( FillTree( xLTS->getLinks() ) == 0 )
						mnError = LERR_NOENTRIES;
				}
				else
					mnError = LERR_DOCNOTOPEN;

				if ( aURL.getLength() )
					xComp->dispose();
			}
			else
			{
				if( aURL.getLength() )
					mnError=LERR_DOCNOTOPEN;
			}
		}
	}
	return (mnError==0);
}
/*
void SvxHlinkDlgMarkWnd::Error(int nNr)
{
	switch(nNr)
	{
		case 0:
		{
			Rectangle aDrawRect( Point( 0, 0 ), maLbTree.GetSizePixel() );
			//maLbTree.SetTextColor( Color(COL_BLACK) );
			//maLbTree.DrawText( aDrawRect, "Keine Ziele im Dokument vorhanden.", TEXT_DRAW_LEFT);// | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
			maLbTree.DrawText( Point(0,0), "Keine Ziele im Dokument vorhanden.");
			maLbTree.DrawLine(aDrawRect.TopLeft(), aDrawRect.BottomRight() );
		}
		break;
		case 1:
			Rectangle aDrawRect( Point( 0, 0 ), maLbTree.GetSizePixel() );
			maLbTree.DrawText( aDrawRect, "Das Dokument konnte nicht ge�ffnet werden.", TEXT_DRAW_LEFT | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
			break;
	}
}
*/
/*************************************************************************
|*
|* Fill Tree-Control
|*
|************************************************************************/

int SvxHlinkDlgMarkWnd::FillTree( uno::Reference< container::XNameAccess > xLinks, SvLBoxEntry* pParentEntry )
{
	int nEntries=0;
	const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
	const sal_uLong nLinks = aNames.getLength();
	const OUString* pNames = aNames.getConstArray();

    Color aMaskColor( COL_LIGHTMAGENTA );
	const OUString aProp_LinkDisplayName( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayName" ) );
	const OUString aProp_LinkTarget( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.LinkTarget" ) );
	const OUString aProp_LinkDisplayBitmap( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayBitmap" ) );
	for( sal_uLong i = 0; i < nLinks; i++ )
	{
		uno::Any aAny;
		OUString aLink( *pNames++ );

		sal_Bool bError = sal_False;
		try
		{
			aAny = xLinks->getByName( aLink );
		}
		catch(const uno::Exception&)
		{
			// if the name of the target was invalid (like empty headings)
			// no object can be provided
			bError = sal_True;
		}
		if(bError)
			continue;

		uno::Reference< beans::XPropertySet > xTarget;

		if( aAny >>= xTarget )
		{
			try
			{
				// get name to display
				aAny = xTarget->getPropertyValue( aProp_LinkDisplayName );
				OUString aDisplayName;
				aAny >>= aDisplayName;
				String aStrDisplayname ( aDisplayName );

				// is it a target ?
				uno::Reference< lang::XServiceInfo > xSI( xTarget, uno::UNO_QUERY );
				sal_Bool bIsTarget = xSI->supportsService( aProp_LinkTarget );

				// create userdata
				TargetData *pData = new TargetData ( aLink, bIsTarget );

				SvLBoxEntry* pEntry;

				try
				{
					// get bitmap for the tree-entry
					uno::Reference< awt::XBitmap > aXBitmap( xTarget->getPropertyValue( aProp_LinkDisplayBitmap ), uno::UNO_QUERY );
					if( aXBitmap.is() )
					{
                        Image aBmp( VCLUnoHelper::GetBitmap( aXBitmap ).GetBitmap(), aMaskColor );
                        // insert Displayname into treelist with bitmaps
						pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                        aBmp, aBmp,
														pParentEntry,
														sal_False, LIST_APPEND,
														(void*)pData );
                        maLbTree.SetExpandedEntryBmp( pEntry, aBmp, BMP_COLOR_HIGHCONTRAST );
                        maLbTree.SetCollapsedEntryBmp( pEntry, aBmp, BMP_COLOR_HIGHCONTRAST );
                        nEntries++;
					}
					else
					{
						// insert Displayname into treelist without bitmaps
						pEntry = maLbTree.InsertEntry ( aStrDisplayname,
														pParentEntry,
														sal_False, LIST_APPEND,
														(void*)pData );
						nEntries++;
					}
				}
				catch(const com::sun::star::uno::Exception&)
				{
					// insert Displayname into treelist without bitmaps
					pEntry = maLbTree.InsertEntry ( aStrDisplayname,
													pParentEntry,
													sal_False, LIST_APPEND,
													(void*)pData );
					nEntries++;
				}

				uno::Reference< document::XLinkTargetSupplier > xLTS( xTarget, uno::UNO_QUERY );
				if( xLTS.is() )
					nEntries += FillTree( xLTS->getLinks(), pEntry );
			}
			catch(const com::sun::star::uno::Exception&)
			{
			}
		}
	}

	return nEntries;
}

/*************************************************************************
|*
|* Clear Tree
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::ClearTree()
{
	SvLBoxEntry* pEntry = maLbTree.First();

	while ( pEntry )
	{
		TargetData* pUserData = ( TargetData * ) pEntry->GetUserData();
		delete pUserData;

		pEntry = maLbTree.Next( pEntry );
	}

	maLbTree.Clear();
}

/*************************************************************************
|*
|* Find Entry for String
|*
|************************************************************************/

SvLBoxEntry* SvxHlinkDlgMarkWnd::FindEntry ( String aStrName )
{
	sal_Bool bFound=sal_False;
	SvLBoxEntry* pEntry = maLbTree.First();

	while ( pEntry && !bFound )
	{
		TargetData* pUserData = ( TargetData * ) pEntry->GetUserData ();
		if ( aStrName == String( pUserData->aUStrLinkname ) )
			bFound = sal_True;
		else
			pEntry = maLbTree.Next( pEntry );
	}

	return pEntry;
}

/*************************************************************************
|*
|* Select Entry
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::SelectEntry ( String aStrMark )
{
	SvLBoxEntry* pEntry = FindEntry ( aStrMark );
	if ( pEntry )
	{
		maLbTree.Select ( pEntry );
		maLbTree.MakeVisible ( pEntry );
	}
}

/*************************************************************************
|*
|* Click on Apply-Button / Doubleclick on item in tree
|*
|************************************************************************/

IMPL_LINK ( SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl, void *, EMPTYARG )
{
	SvLBoxEntry* pEntry = maLbTree.GetCurEntry();

	if ( pEntry )
	{
		TargetData *pData = ( TargetData * )pEntry->GetUserData();

		if ( pData->bIsTarget )
		{
			String aStrMark ( pData->aUStrLinkname );
			mpParent->SetMarkStr ( aStrMark );
		}
	}

	return( 0L );
}

/*************************************************************************
|*
|* Click on Close-Button
|*
|************************************************************************/

IMPL_LINK ( SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl, void *, EMPTYARG )
{
    Close();

	return( 0L );
}


