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

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>

#include <tools/urlobj.hxx>

#include "multipat.hxx"
#include "multifil.hxx"
#include <dialmgr.hxx>

#include "multipat.hrc"
#include <cuires.hrc>

// #97807# -------------
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <ucbhelper/contentbroker.hxx>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

// class SvxMultiFileDialog ----------------------------------------------

IMPL_LINK( SvxMultiFileDialog, AddHdl_Impl, PushButton *, pBtn )
{
    sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

	if ( IsClassPathMode() )
	{
		aDlg.SetTitle( CUI_RES( RID_SVXSTR_ARCHIVE_TITLE ) );
		aDlg.AddFilter( CUI_RES( RID_SVXSTR_ARCHIVE_HEADLINE ), String::CreateFromAscii("*.jar;*.zip") );
	}

	if ( aDlg.Execute() == ERRCODE_NONE )
    {
		// #97807# URL content comparison of entries -----------
		INetURLObject aFile( aDlg.GetPath() );
		String sInsFile = aFile.getFSysPath( INetURLObject::FSYS_DETECT );
		::ucbhelper::Content aContent( aFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
		Reference< XContent > xContent = aContent.get();
		OSL_ENSURE( xContent.is(), "AddHdl_Impl: invalid content interface!" );
		Reference< XContentIdentifier > xID = xContent->getIdentifier();
		OSL_ENSURE( xID.is(), "AddHdl_Impl: invalid ID interface!" );
		// ensure the content of files are valid

		sal_uInt16 nCount = aPathLB.GetEntryCount();
		sal_Bool bDuplicated = sal_False;
		try
		{
			if( nCount > 0 ) // start comparison
			{
				sal_uInt16 i;
				::ucbhelper::Content & VContent = aContent; // temporary Content reference
				Reference< XContent > xVContent;
				Reference< XContentIdentifier > xVID;
				for( i = 0; i < nCount; i++ )
				{
					String sVFile = aPathLB.GetEntry( i );
					std::map< String, ::ucbhelper::Content >::iterator aCur = aFileContentMap.find( sVFile );
					if( aCur == aFileContentMap.end() ) // look for File Content in aFileContentMap, but not find it.
					{
						INetURLObject aVFile( sVFile, INetURLObject::FSYS_DETECT );
						aFileContentMap[sVFile] = ::ucbhelper::Content( aVFile.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
						VContent = aFileContentMap.find( sVFile )->second;
					}
					else
						VContent = aCur->second;
					xVContent = VContent.get();
					OSL_ENSURE( xVContent.is(), "AddHdl_Impl: invalid content interface!" );
					xVID = xVContent->getIdentifier();
					OSL_ENSURE( xVID.is(), "AddHdl_Impl: invalid ID interface!" );
					if ( xID.is() && xVID.is() )
					{
						// get a generic content provider
						::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
						Reference< XContentProvider > xProvider;
						if ( pBroker )
							xProvider = pBroker->getContentProviderInterface();
						if ( xProvider.is() )
						{
							if ( 0 == xProvider->compareContentIds( xID, xVID ) )
							{
								bDuplicated = sal_True;
								break;
							}
						}
					}
				}
			} // end of if the entries are more than zero.
		} // end of try(}
		catch( const Exception& ) // catch every exception of comparison
   		{
			OSL_ENSURE( sal_False, "AddHdl_Impl: caught an unexpected exception!" );
   		}

		if ( bDuplicated ) // #97807# --------------------
		{
			String sMsg( CUI_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
			sMsg.SearchAndReplaceAscii( "%1", sInsFile );
			InfoBox( pBtn, sMsg ).Execute();
		}
		else
		{
			sal_uInt16 nPos = aPathLB.InsertEntry( sInsFile, LISTBOX_APPEND );
			aPathLB.SetEntryData( nPos, (void*) new String( sInsFile ) );
		}

	} // end of if ( aDlg.Execute() == ERRCODE_NONE )
	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxMultiFileDialog, DelHdl_Impl, PushButton *, EMPTYARG )
{
	sal_uInt16 nPos = aPathLB.GetSelectEntryPos();
	aPathLB.RemoveEntry( nPos );
	sal_uInt16 nCnt = aPathLB.GetEntryCount();

	if ( nCnt )
	{
		nCnt--;

		if ( nPos > nCnt )
			nPos = nCnt;
		aPathLB.SelectEntryPos( nPos );
	}
	return 0;
}

// -----------------------------------------------------------------------

SvxMultiFileDialog::SvxMultiFileDialog( Window* pParent, sal_Bool bEmptyAllowed ) :

	SvxMultiPathDialog( pParent, bEmptyAllowed )

{
	aAddBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, AddHdl_Impl ) );
	aDelBtn.SetClickHdl( LINK( this, SvxMultiFileDialog, DelHdl_Impl ) );
	SetText( CUI_RES( RID_SVXSTR_FILE_TITLE ) );
    aPathFL.SetText( CUI_RES( RID_SVXSTR_FILE_HEADLINE ) );
	aDelBtn.Enable();
}

// -----------------------------------------------------------------------

SvxMultiFileDialog::~SvxMultiFileDialog()
{
}


