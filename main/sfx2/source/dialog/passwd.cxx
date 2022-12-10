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
#include "precompiled_sfx2.hxx"

// Include ---------------------------------------------------------------
#include <vcl/msgbox.hxx>
#ifndef GCC
#endif

#include <sfx2/passwd.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"

#include "dialog.hrc"
#include "passwd.hrc"

#include "vcl/sound.hxx"
#include "vcl/arrange.hxx"

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxPasswordDialog, EditModifyHdl, Edit *, pEdit )
{
    if( mbAsciiOnly && (pEdit == &maPasswordED || pEdit == &maPassword2ED) )
    {
        rtl::OUString aTest( pEdit->GetText() );
        const sal_Unicode* pTest = aTest.getStr();
        sal_Int32 nLen = aTest.getLength();
        rtl::OUStringBuffer aFilter( nLen );
        bool bReset = false;
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            if( *pTest > 0x007f )
                bReset = true;
            else
                aFilter.append( *pTest );
            pTest++;
        }
        if( bReset )
        {
            Sound::Beep( SOUND_ERROR );
            pEdit->SetSelection( Selection( 0, nLen ) );
            pEdit->ReplaceSelected( aFilter.makeStringAndClear() );
        }
        
    }
    bool bEnable = maPasswordED.GetText().Len() >= mnMinLen;
    if( maPassword2ED.IsVisible() )
        bEnable = (bEnable && (maPassword2ED.GetText().Len() >= mnMinLen));
	maOKBtn.Enable( bEnable );
	return 0;
}
IMPL_LINK_INLINE_END( SfxPasswordDialog, EditModifyHdl, Edit *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SfxPasswordDialog, OKHdl, OKButton *, EMPTYARG )
{
    bool bConfirmFailed = ( ( mnExtras & SHOWEXTRAS_CONFIRM ) == SHOWEXTRAS_CONFIRM ) &&
		                    ( GetConfirm() != GetPassword() );
    if( ( mnExtras & SHOWEXTRAS_CONFIRM2 ) == SHOWEXTRAS_CONFIRM2 && ( GetConfirm2() != GetPassword2() ) )
        bConfirmFailed = true;
	if ( bConfirmFailed )
	{
		ErrorBox aBox( this, SfxResId( MSG_ERROR_WRONG_CONFIRM ) );
		aBox.Execute();
		maConfirmED.SetText( String() );
		maConfirmED.GrabFocus();
	}
	else
		EndDialog( RET_OK );
	return 0;
}

// CTOR / DTOR -----------------------------------------------------------

SfxPasswordDialog::SfxPasswordDialog( Window* pParent, const String* pGroupText ) :

	ModalDialog( pParent, SfxResId ( DLG_PASSWD ) ),

	maPasswordBox	( this, SfxResId( GB_PASSWD_PASSWORD ) ),
	maUserFT		( this, SfxResId( FT_PASSWD_USER ) ),
	maUserED		( this, SfxResId( ED_PASSWD_USER ) ),
	maPasswordFT	( this, SfxResId( FT_PASSWD_PASSWORD ) ),
	maPasswordED	( this, SfxResId( ED_PASSWD_PASSWORD ) ),
	maConfirmFT		( this, SfxResId( FT_PASSWD_CONFIRM ) ),
	maConfirmED		( this, SfxResId( ED_PASSWD_CONFIRM ) ),
	maPassword2Box	( this, 0 ),
	maPassword2FT	( this, SfxResId( FT_PASSWD_PASSWORD2 ) ),
	maPassword2ED	( this, SfxResId( ED_PASSWD_PASSWORD2 ) ),
	maConfirm2FT	( this, SfxResId( FT_PASSWD_CONFIRM2 ) ),
	maConfirm2ED	( this, SfxResId( ED_PASSWD_CONFIRM2 ) ),
	maOKBtn			( this, SfxResId( BTN_PASSWD_OK ) ),
	maCancelBtn		( this, SfxResId( BTN_PASSWD_CANCEL ) ),
	maHelpBtn		( this, SfxResId( BTN_PASSWD_HELP ) ),

	mnMinLen		( 1 ),
	mnExtras		( 0 ),
    mbAsciiOnly     ( false )

{
	maPasswordED.SetAccessibleName(String(SfxResId(TEXT_PASSWD)));
	FreeResource();
	
	// setup layout
    boost::shared_ptr<vcl::RowOrColumn> xLayout =
        boost::dynamic_pointer_cast<vcl::RowOrColumn>( getLayout() );
    xLayout->setOuterBorder( 0 );

    // get edit size, should be used as minimum
    Size aEditSize( maUserED.GetSizePixel() );

    // add labelcolumn for the labeled edit fields
    boost::shared_ptr<vcl::LabelColumn> xEdits( new vcl::LabelColumn( xLayout.get() ) );
    size_t nChildIndex = xLayout->addChild( xEdits );
    xLayout->setBorders( nChildIndex, -2, -2, -2, 0 );
    
    // add group box
    xEdits->addWindow( &maPasswordBox );
    
    // add user line
    xEdits->addRow( &maUserFT, &maUserED, -2, aEditSize );

    // add password line
    xEdits->addRow( &maPasswordFT, &maPasswordED, -2, aEditSize );

    // add confirm line
    xEdits->addRow( &maConfirmFT, &maConfirmED, -2, aEditSize );
    
    // add second group box
    xEdits->addWindow( &maPassword2Box );

    // add second password line
    xEdits->addRow( &maPassword2FT, &maPassword2ED, -2, aEditSize );

    // add second confirm line
    xEdits->addRow( &maConfirm2FT, &maConfirm2ED, -2, aEditSize );
    
    // add a FixedLine
    FixedLine* pLine = new FixedLine( this, 0 );
    pLine->Show();
    addWindow( pLine, true );
    xLayout->addWindow( pLine );

    // add button column
    Size aBtnSize( maCancelBtn.GetSizePixel() );
    boost::shared_ptr<vcl::RowOrColumn> xButtons( new vcl::RowOrColumn( xLayout.get(), false ) );
    nChildIndex = xLayout->addChild( xButtons );
    xLayout->setBorders( nChildIndex, -2, 0, -2, -2 );

    size_t nBtnIndex = xButtons->addWindow( &maHelpBtn, 0, aBtnSize );
    xButtons->addChild( new vcl::Spacer( xButtons.get() ) );
    nBtnIndex = xButtons->addWindow( &maOKBtn, 0, aBtnSize );
    nBtnIndex = xButtons->addWindow( &maCancelBtn, 0, aBtnSize );

	Link aLink = LINK( this, SfxPasswordDialog, EditModifyHdl );
	maPasswordED.SetModifyHdl( aLink );
	maPassword2ED.SetModifyHdl( aLink );
	aLink = LINK( this, SfxPasswordDialog, OKHdl );
	maOKBtn.SetClickHdl( aLink );

	if ( pGroupText )
		  maPasswordBox.SetText( *pGroupText );
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMinLen( sal_uInt16 nLen )
{
	mnMinLen = nLen;
	EditModifyHdl( NULL );
}

// -----------------------------------------------------------------------

void SfxPasswordDialog::SetMaxLen( sal_uInt16 nLen )
{
    maPasswordED.SetMaxTextLen( nLen );
    maConfirmED.SetMaxTextLen( nLen );
	EditModifyHdl( NULL );
}

// -----------------------------------------------------------------------

short SfxPasswordDialog::Execute()
{
    maUserFT.Hide();
    maUserED.Hide();
    maConfirmFT.Hide();
    maConfirmED.Hide();
    maPasswordFT.Hide();
    maPassword2Box.Hide();
    maPassword2FT.Hide();
    maPassword2ED.Hide();
    maPassword2FT.Hide();
    maConfirm2FT.Hide();
    maConfirm2ED.Hide();

    if( mnExtras != SHOWEXTRAS_NONE )
        maPasswordFT.Show();
    if( (mnExtras & SHOWEXTRAS_USER ) )
    {
        maUserFT.Show();
        maUserED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_CONFIRM ) )
    {
        maConfirmFT.Show();
        maConfirmED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_PASSWORD2) )
    {
        maPassword2Box.Show();
        maPassword2FT.Show();
        maPassword2ED.Show();
    }
    if( (mnExtras & SHOWEXTRAS_CONFIRM2 ) )
    {
        maConfirm2FT.Show();
        maConfirm2ED.Show();
    }
    
    boost::shared_ptr<vcl::RowOrColumn> xLayout =
        boost::dynamic_pointer_cast<vcl::RowOrColumn>( getLayout() );
	SetSizePixel( xLayout->getOptimalSize( WINDOWSIZE_PREFERRED ) );

	return ModalDialog::Execute();
}


