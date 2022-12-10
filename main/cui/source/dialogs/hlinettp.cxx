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

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <svl/adrparse.hxx>

#include "hlinettp.hxx"
#include "hyperdlg.hrc"
#include "hlmarkwn_def.hxx" //ADD CHINA001

#define STD_DOC_SUBPATH		"internal"
#define STD_DOC_NAME		"url_transfer.htm"

sal_Char __READONLY_DATA sAnonymous[]    = "anonymous";
sal_Char __READONLY_DATA sHTTPScheme[]   = INET_HTTP_SCHEME;
sal_Char __READONLY_DATA sHTTPSScheme[]  = INET_HTTPS_SCHEME;
sal_Char __READONLY_DATA sFTPScheme[]    = INET_FTP_SCHEME;
sal_Char __READONLY_DATA sTelnetScheme[] = INET_TELNET_SCHEME;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkInternetTp::SvxHyperlinkInternetTp ( Window *pParent,
												 const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_INTERNET ),
                              rItemSet ) ,
    maGrpLinkTyp           ( this, CUI_RES (GRP_LINKTYPE) ),
	maRbtLinktypInternet	( this, CUI_RES (RB_LINKTYP_INTERNET) ),
	maRbtLinktypFTP			( this, CUI_RES (RB_LINKTYP_FTP) ),
	maRbtLinktypTelnet		( this, CUI_RES (RB_LINKTYP_TELNET) ),
	maFtTarget				( this, CUI_RES (FT_TARGET_HTML) ),
	maCbbTarget				( this, INET_PROT_HTTP ),
	maBtBrowse				( this, CUI_RES (BTN_BROWSE) ),
	maFtLogin				( this, CUI_RES (FT_LOGIN) ),
	maEdLogin				( this, CUI_RES (ED_LOGIN) ),
	maBtTarget				( this, CUI_RES (BTN_TARGET) ),
	maFtPassword			( this, CUI_RES (FT_PASSWD) ),
	maEdPassword			( this, CUI_RES (ED_PASSWD) ),
	maCbAnonymous			( this, CUI_RES (CBX_ANONYMOUS) ),
    mbMarkWndOpen           ( sal_False )
{
	// Set HC bitmaps and display display of bitmap names.
	maBtBrowse.SetModeImage( Image( CUI_RES( IMG_BROWSE_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtBrowse.EnableTextDisplay (sal_False);
	maBtTarget.SetModeImage( Image( CUI_RES( IMG_TARGET_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtTarget.EnableTextDisplay (sal_False);

	InitStdControls();
	FreeResource();

	// Init URL-Box (pos&size, Open-Handler)
	maCbbTarget.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
		                          LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
	maCbbTarget.Show();
	maCbbTarget.SetHelpId( HID_HYPERDLG_INET_PATH );

	// Find Path to Std-Doc
	String aStrBasePaths( SvtPathOptions().GetTemplatePath() );
	for( xub_StrLen n = 0; n < aStrBasePaths.GetTokenCount(); n++ )
	{
	    INetURLObject aURL( aStrBasePaths.GetToken( n ) );
		aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_SUBPATH ) ) );
		aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_NAME ) ) );
		if ( FileExists( aURL ) )
        {
			maStrStdDocURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
            break;
        }
	}
	SetExchangeSupport ();

	///////////////////////////////////////
	// set defaults
	maRbtLinktypInternet.Check ();
	maFtLogin.Show( sal_False );
	maFtPassword.Show( sal_False );
	maEdLogin.Show( sal_False );
	maEdPassword.Show( sal_False );
	maCbAnonymous.Show( sal_False );
	maBtTarget.Enable( sal_False );
	maBtBrowse.Enable( maStrStdDocURL != aEmptyStr );

	///////////////////////////////////////
	// overload handlers
    Link aLink( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maRbtLinktypInternet.SetClickHdl( aLink );
    maRbtLinktypFTP.SetClickHdl     ( aLink );
    maRbtLinktypTelnet.SetClickHdl  ( aLink );
	maCbAnonymous.SetClickHdl       ( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
	maBtBrowse.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickBrowseHdl_Impl ) );
	maBtTarget.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickTargetHdl_Impl ) );
	maEdLogin.SetModifyHdl			( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );
	maCbbTarget.SetLoseFocusHdl		( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
	maCbbTarget.SetModifyHdl		( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
	maTimer.SetTimeoutHdl			( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );

	maFtTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
	maCbbTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );	
	maBtTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
	maBtTarget.SetAccessibleRelationLabeledBy( &maFtTarget );
	maBtBrowse.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
	maBtBrowse.SetAccessibleRelationLabeledBy( &maFtTarget );
}

SvxHyperlinkInternetTp::~SvxHyperlinkInternetTp ()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/

void SvxHyperlinkInternetTp::FillDlgFields ( String& aStrURL )
{
	INetURLObject aURL( aStrURL );
	String aStrScheme = GetSchemeFromURL( aStrURL );

    // set additional controls for FTP: Username / Password
	if ( aStrScheme.SearchAscii( sFTPScheme ) == 0 )
	{
		if ( String(aURL.GetUser()).ToLowerAscii().SearchAscii ( sAnonymous ) == 0 )
			setAnonymousFTPUser();
		else
			setFTPUser(aURL.GetUser(), aURL.GetPass());

        //do not show password and user in url
        if(aURL.GetUser().getLength()!=0 || aURL.GetPass().getLength()!=0 )
            aURL.SetUserAndPass(aEmptyStr,aEmptyStr);
	}

	// set URL-field
    // Show the scheme, #72740
    if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
		maCbbTarget.SetText( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );
    else
        maCbbTarget.SetText( aStrURL ); // #77696#

    SetScheme( aStrScheme );
}

void SvxHyperlinkInternetTp::setAnonymousFTPUser()
{
	maEdLogin.SetText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sAnonymous ) ) );
	SvAddressParser aAddress( SvtUserOptions().GetEmail() );
	maEdPassword.SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : String() );

	maFtLogin.Disable ();
	maFtPassword.Disable ();
	maEdLogin.Disable ();
	maEdPassword.Disable ();
	maCbAnonymous.Check();
}

void SvxHyperlinkInternetTp::setFTPUser(const String& rUser, const String& rPassword)
{
	maEdLogin.SetText ( rUser );
	maEdPassword.SetText ( rPassword );

	maFtLogin.Enable ();
	maFtPassword.Enable ();
	maEdLogin.Enable ();
	maEdPassword.Enable ();
	maCbAnonymous.Check(sal_False);
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkInternetTp::GetCurentItemData ( String& aStrURL, String& aStrName,
												 String& aStrIntName, String& aStrFrame,
												 SvxLinkInsertMode& eMode )
{
    aStrURL = CreateAbsoluteURL();
	GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

String SvxHyperlinkInternetTp::CreateAbsoluteURL() const
{
    String aStrURL = maCbbTarget.GetText();
    String aScheme = GetSchemeFromURL(aStrURL);

    INetURLObject aURL(aStrURL);

	if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

	// username and password for ftp-url
	if( aURL.GetProtocol() == INET_PROT_FTP && maEdLogin.GetText().Len()!=0 )
		aURL.SetUserAndPass ( maEdLogin.GetText(), maEdPassword.GetText() );

	if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        return aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkInternetTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
	return( new SvxHyperlinkInternetTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetInitFocus()
{
	maCbbTarget.GrabFocus();
}

/*************************************************************************
|*
|* Contens of editfield "Taregt" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl, void *, EMPTYARG )
{
    String aScheme = GetSchemeFromURL( maCbbTarget.GetText() );
    if(aScheme.Len()!=0)
        SetScheme( aScheme );

	// start timer
	maTimer.SetTimeout( 2500 );
	maTimer.Start();

	return( 0L );
}

/*************************************************************************
|*
|* If target-field was modify, to browse the new doc afeter timeout
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    RefreshMarkWindow();
	return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "Login" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl, void *, EMPTYARG )
{
	String aStrLogin ( maEdLogin.GetText() );
	if ( aStrLogin.EqualsIgnoreCaseAscii( sAnonymous ) )
	{
		maCbAnonymous.Check();
		ClickAnonymousHdl_Impl(NULL);
	}

	return( 0L );
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkInternetTp::SetScheme( const String& aScheme )
{
    //if  aScheme is empty or unknown the default beaviour is like it where HTTP

    sal_Bool bFTP = aScheme.SearchAscii( sFTPScheme ) == 0;
    sal_Bool bTelnet = sal_False;
    if( !bFTP )
        bTelnet = aScheme.SearchAscii( sTelnetScheme ) == 0;
    sal_Bool bInternet = !(bFTP || bTelnet);

    //update protocol button selection:
    maRbtLinktypFTP.Check(bFTP);
    maRbtLinktypTelnet.Check(bTelnet);
    maRbtLinktypInternet.Check(bInternet);

    //update target:
    RemoveImproperProtocol(aScheme);
    maCbbTarget.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for FTP:
    maFtLogin.Show( bFTP );
	maFtPassword.Show( bFTP );
	maEdLogin.Show( bFTP );
	maEdPassword.Show( bFTP );
	maCbAnonymous.Show( bFTP );

    //update 'link target in document'-window and opening-button
    if( aScheme.SearchAscii( sHTTPScheme ) == 0 || aScheme.Len() == 0 )
    {
        maBtTarget.Enable();
	    if ( mbMarkWndOpen )
		    ShowMarkWnd ();
    }
    else
    {
        //disable for https, ftp and telnet
        maBtTarget.Disable();
		if ( mbMarkWndOpen )
			HideMarkWnd ();
    }
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkInternetTp::RemoveImproperProtocol(const String& aProperScheme)
{
    String aStrURL ( maCbbTarget.GetText() );
	if ( aStrURL != aEmptyStr )
	{
		String aStrScheme = GetSchemeFromURL( aStrURL );
		if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
		{
			aStrURL.Erase ( 0, aStrScheme.Len() );
            maCbbTarget.SetText ( aStrURL );
		}
	}
}

String SvxHyperlinkInternetTp::GetSchemeFromButtons() const
{
    if( maRbtLinktypFTP.IsChecked() )
	{
		return String::CreateFromAscii( INET_FTP_SCHEME );
	}
	else if( maRbtLinktypTelnet.IsChecked() )
	{
        return String::CreateFromAscii( INET_TELNET_SCHEME );
	}
	return String::CreateFromAscii( INET_HTTP_SCHEME );
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons() const
{
	if( maRbtLinktypFTP.IsChecked() )
	{
		return INET_PROT_FTP;
	}
	else if( maRbtLinktypTelnet.IsChecked() )
	{
        return INET_PROT_TELNET;
	}
	return INET_PROT_HTTP;
}

/*************************************************************************
|*
|* Click on Radiobutton : Internet, FTP or Telnet
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, Click_SmartProtocol_Impl, void*, EMPTYARG )
{
    String aScheme = GetSchemeFromButtons();
    SetScheme( aScheme );
	return( 0L );
}

/*************************************************************************
|*
|* Click on Checkbox : Anonymous user
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl, void *, EMPTYARG )
{
	// disable login-editfields if checked
	if ( maCbAnonymous.IsChecked() )
	{
        if ( maEdLogin.GetText().ToLowerAscii().SearchAscii ( sAnonymous ) == 0 )
        {
            maStrOldUser = aEmptyStr;
	        maStrOldPassword = aEmptyStr;
        }
        else
        {
            maStrOldUser = maEdLogin.GetText();
	        maStrOldPassword = maEdPassword.GetText();
        }

		setAnonymousFTPUser();
	}
	else
		setFTPUser(maStrOldUser, maStrOldPassword);

	return( 0L );
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl, void *, EMPTYARG )
{
	RefreshMarkWindow();
	return (0L);
}

/*************************************************************************
|*
|* Click on imagebutton : Browse
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickBrowseHdl_Impl, void *, EMPTYARG )
{
	/////////////////////////////////////////////////
	// Open URL if available

    SfxStringItem aName( SID_FILE_NAME, maStrStdDocURL );
	SfxStringItem aRefererItem( SID_REFERER, UniString::CreateFromAscii(
		                        RTL_CONSTASCII_STRINGPARAM( "private:user" ) ) );
	SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_True );
	SfxBoolItem aSilent( SID_SILENT, sal_True );
	SfxBoolItem aReadOnly( SID_DOC_READONLY, sal_True );

    SfxBoolItem aBrowse( SID_BROWSE, sal_True );

    const SfxPoolItem *ppItems[] = { &aName, &aNewView, &aSilent, &aReadOnly, &aRefererItem, &aBrowse, NULL };
    (((SvxHpLinkDlg*)mpDialog)->GetBindings())->Execute( SID_OPENDOC, ppItems, 0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

	return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : Target
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTargetHdl_Impl, void *, EMPTYARG )
{
	RefreshMarkWindow();
	ShowMarkWnd ();
	mbMarkWndOpen = IsMarkWndVisible ();

	return( 0L );
}

void SvxHyperlinkInternetTp::RefreshMarkWindow()
{
    if ( maRbtLinktypInternet.IsChecked() && IsMarkWndVisible() )
    {
        EnterWait();
		String aStrURL( CreateAbsoluteURL() );
		if ( aStrURL != aEmptyStr )
			mpMarkWnd->RefreshTree ( aStrURL );
        else
            mpMarkWnd->SetError( LERR_DOCNOTOPEN );
        LeaveWait();
	}

}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetMarkStr ( String& aStrMark )
{
	String aStrURL ( maCbbTarget.GetText() );

	const sal_Unicode sUHash = '#';
	xub_StrLen nPos = aStrURL.SearchBackward( sUHash );

	if( nPos != STRING_NOTFOUND )
		aStrURL.Erase ( nPos );

	aStrURL += sUHash;
	aStrURL += aStrMark;

	maCbbTarget.SetText ( aStrURL );
}

/*************************************************************************
|*
|* Enable Browse-Button in subject to the office is in onlinemode
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetOnlineMode( sal_Bool /*bEnable*/ )
{
	// State of target-button in subject to the current url-string
	// ( Can't display any targets in an document, if there is no
	//   valid url to a document )
	String aStrCurrentTarget( maCbbTarget.GetText() );
	aStrCurrentTarget.EraseTrailingChars();

	if( aStrCurrentTarget == aEmptyStr                ||
		aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPScheme )  ||
		aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
		maBtTarget.Enable( sal_False );
	else
		maBtTarget.Enable( sal_True );
}
