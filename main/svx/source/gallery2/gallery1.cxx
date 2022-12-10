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
#include "precompiled_svx.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/vcompat.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include "svx/gallery.hxx"
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "svx/galtheme.hxx"
#include "svx/gallery1.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#define ENABLE_BYTESTRING_STREAM_OPERATORS

// --------------
// - Namespaces -
// --------------

using namespace ::rtl;
using namespace ::com::sun::star;

// ---------------------
// - GalleryThemeEntry -
// ---------------------

GalleryThemeEntry::GalleryThemeEntry( const INetURLObject& rBaseURL, const String& rName,
									  sal_uInt32 _nFileNumber, sal_Bool _bReadOnly, sal_Bool _bImported,
									  sal_Bool _bNewFile, sal_uInt32 _nId, sal_Bool _bThemeNameFromResource ) :
		nFileNumber                             ( _nFileNumber ),
		nId                                     ( _nId ),
		bReadOnly                               ( _bReadOnly || _bImported ),
		bImported                               ( _bImported ),
		bThemeNameFromResource  ( _bThemeNameFromResource )
{
	INetURLObject aURL( rBaseURL );
	DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
	String aFileName( String( RTL_CONSTASCII_USTRINGPARAM( "sg" ) ) );

	aURL.Append( ( aFileName += String::CreateFromInt32( nFileNumber ) ) += String( RTL_CONSTASCII_USTRINGPARAM( ".thm" ) ) );
	aThmURL = ImplGetURLIgnoreCase( aURL );

	aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdg" ) ) );
	aSdgURL = ImplGetURLIgnoreCase( aURL );

	aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdv" ) ) );
	aSdvURL = ImplGetURLIgnoreCase( aURL );

	SetModified( _bNewFile );

	if( nId && bThemeNameFromResource )
    {
        const ResId aId (GAL_RESID( RID_GALLERYSTR_THEME_START + (sal_uInt16) nId));
        if (aId.GetpResource() == NULL)
            OSL_TRACE("");
        aName = String(aId);
    }

	if( !aName.Len() )
		aName = rName;
}

// -----------------------------------------------------------------------------

INetURLObject GalleryThemeEntry::ImplGetURLIgnoreCase( const INetURLObject& rURL ) const
{
	INetURLObject	aURL( rURL );
	String			aFileName;
	sal_Bool			bExists = sal_False;

	// check original file name
	if( FileExists( aURL ) )
		bExists = sal_True;
	else
	{
		// check upper case file name
		aURL.setName( aURL.getName().toAsciiUpperCase() );

		if( FileExists( aURL ) )
			bExists = sal_True;
		else
		{
			// check lower case file name
			aURL.setName( aURL.getName().toAsciiLowerCase() );

			if( FileExists( aURL ) )
				bExists = sal_True;
		}
	}

	return aURL;
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetName( const String& rNewName )
{
	if( aName != rNewName )
	{
		aName = rNewName;
		SetModified( sal_True );
		bThemeNameFromResource = sal_False;
	}
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName )
{
	nId = nNewId;
	SetModified( sal_True );
	bThemeNameFromResource = ( nId && bResetThemeName );
}

// ---------------------------
// - GalleryImportThemeEntry -
// ---------------------------

SvStream& operator<<( SvStream& rOut, const GalleryImportThemeEntry& rEntry )
{
	ByteString aDummy;

	rOut << ByteString( rEntry.aThemeName, RTL_TEXTENCODING_UTF8 ) <<
			ByteString( rEntry.aUIName, RTL_TEXTENCODING_UTF8 ) <<
			ByteString( String(rEntry.aURL.GetMainURL( INetURLObject::NO_DECODE )), RTL_TEXTENCODING_UTF8 ) <<
			ByteString( rEntry.aImportName, RTL_TEXTENCODING_UTF8 ) <<
			aDummy;

	return rOut;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, GalleryImportThemeEntry& rEntry )
{
	ByteString aTmpStr;

	rIn >> aTmpStr; rEntry.aThemeName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
	rIn >> aTmpStr; rEntry.aUIName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
	rIn >> aTmpStr; rEntry.aURL = INetURLObject( String( aTmpStr, RTL_TEXTENCODING_UTF8 ) );
	rIn >> aTmpStr; rEntry.aImportName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
	rIn >> aTmpStr;

	return rIn;
}

// --------------------------
// - GalleryThemeCacheEntry -
// --------------------------

class GalleryThemeCacheEntry;
DBG_NAME(GalleryThemeCacheEntry)
class GalleryThemeCacheEntry
{
private:

	const GalleryThemeEntry*        mpThemeEntry;
	GalleryTheme*                           mpTheme;

public:

								GalleryThemeCacheEntry( const GalleryThemeEntry* pThemeEntry, GalleryTheme* pTheme ) :
									mpThemeEntry( pThemeEntry ), mpTheme( pTheme ) {DBG_CTOR(GalleryThemeCacheEntry,NULL);}
								~GalleryThemeCacheEntry() { delete mpTheme;DBG_DTOR(GalleryThemeCacheEntry,NULL); }

	const GalleryThemeEntry*        GetThemeEntry() const { return mpThemeEntry; }
	GalleryTheme*                           GetTheme() const { return mpTheme; }
};

// -----------
// - Gallery -
// -----------
Gallery::Gallery( const String& rMultiPath )
:		nReadTextEncoding	( gsl_getSystemTextEncoding() )
,		nLastFileNumber		( 0 )
,		bMultiPath			( sal_False )
{
	ImplLoad( rMultiPath );
}

// ------------------------------------------------------------------------

Gallery::~Gallery()
{
	// Themen-Liste loeschen
	for( GalleryThemeEntry* pThemeEntry = aThemeList.First(); pThemeEntry; pThemeEntry = aThemeList.Next() )
		delete pThemeEntry;

	// Import-Liste loeschen
	for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
		delete pImportEntry;
}

// ------------------------------------------------------------------------

Gallery* Gallery::GetGalleryInstance()
{
	static Gallery* pGallery = NULL;

	if( !pGallery )
	{
	    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		if( !pGallery )
		{
			pGallery = new Gallery( SvtPathOptions().GetGalleryPath() );
		}
	}

	return pGallery;
}

// ------------------------------------------------------------------------

void Gallery::ImplLoad( const String& rMultiPath )
{
	const sal_uInt16    nTokenCount = rMultiPath.GetTokenCount( ';' );
    sal_Bool        bIsReadOnlyDir;

	bMultiPath = ( nTokenCount > 0 );

    INetURLObject aCurURL(SvtPathOptions().GetConfigPath());
	ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

	if( !bIsReadOnlyDir )
	    aUserURL = aCurURL;

	if( bMultiPath )
	{
		aRelURL = INetURLObject( rMultiPath.GetToken( 0, ';' ) );

		for( sal_uInt16 i = 0UL; i < nTokenCount; i++ )
		{
		    aCurURL = INetURLObject(rMultiPath.GetToken( i, ';' ));

			ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

			if( !bIsReadOnlyDir )
			    aUserURL = aCurURL;
	    }
	}
	else
		aRelURL = INetURLObject( rMultiPath );

	DBG_ASSERT( aUserURL.GetProtocol() != INET_PROT_NOT_VALID, "no writable Gallery user directory available" );
	DBG_ASSERT( aRelURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

	ImplLoadImports();
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbDirIsReadOnly )
{
    rbDirIsReadOnly = sal_False;

	try
	{
		uno::Reference< ucb::XCommandEnvironment > xEnv;
		::ucbhelper::Content					   aCnt( rBaseURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );

		uno::Sequence< OUString > aProps( 1 );
		aProps.getArray()[ 0 ] = OUString::createFromAscii( "Url" );

		uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

		try
		{
		    // check readonlyness the very hard way
        	INetURLObject   aTestURL( rBaseURL );
	        String          aTestFile( RTL_CONSTASCII_USTRINGPARAM( "cdefghij.klm" ) );

    	    aTestURL.Append( aTestFile );
	        SvStream* pTestStm = ::utl::UcbStreamHelper::CreateStream( aTestURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );

	        if( pTestStm )
	        {
	            *pTestStm << 1;

	            if( pTestStm->GetError() )
	                rbDirIsReadOnly = sal_True;

	            delete pTestStm;
	            KillFile( aTestURL );
	        }
	        else
	            rbDirIsReadOnly = sal_True;
	    }
	    catch( const ucb::ContentCreationException& )
	    {
	    }
	    catch( const uno::RuntimeException& )
	    {
	    }
	    catch( const uno::Exception& )
	    {
	    }

		if( xResultSet.is() )
		{
			uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

			if( xContentAccess.is() )
			{
                static const ::rtl::OUString s_sTitle(RTL_CONSTASCII_USTRINGPARAM("Title"));
                static const ::rtl::OUString s_sIsReadOnly(RTL_CONSTASCII_USTRINGPARAM("IsReadOnly"));
                static const ::rtl::OUString s_sSDG_EXT(RTL_CONSTASCII_USTRINGPARAM("sdg"));
                static const ::rtl::OUString s_sSDV_EXT(RTL_CONSTASCII_USTRINGPARAM("sdv"));
                
				while( xResultSet->next() )
				{
					INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

					if(aThmURL.GetExtension().equalsIgnoreAsciiCaseAscii("thm"))
					{
						INetURLObject	aSdgURL( aThmURL); aSdgURL.SetExtension( s_sSDG_EXT );
						INetURLObject	aSdvURL( aThmURL ); aSdvURL.SetExtension( s_sSDV_EXT );
						
						OUString		aTitle;
						sal_Bool		bReadOnly = sal_False;

						try
						{
						    ::ucbhelper::Content aThmCnt( aThmURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );
						    ::ucbhelper::Content aSdgCnt( aSdgURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );
						    ::ucbhelper::Content aSdvCnt( aSdvURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );

						    try
						    {
						        aThmCnt.getPropertyValue( s_sTitle ) >>= aTitle;
						    }
	                        catch( const uno::RuntimeException& )
	                        {
	                        }
	                        catch( const uno::Exception& )
	                        {
	                        }

						    if( aTitle.getLength() )
						    {
						        try
						        {
							        aThmCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
						        }
	                            catch( const uno::RuntimeException& )
	                            {
	                            }
	                            catch( const uno::Exception& )
	                            {
	                            }

							    if( !bReadOnly )
							    {
							        try
							        {
								        aSdgCnt.getPropertyValue( s_sTitle ) >>= aTitle;
						            }
	                                catch( const ::com::sun::star::uno::RuntimeException& )
	                                {
	                                }
	                                catch( const ::com::sun::star::uno::Exception& )
	                                {
	                                }

								    if( aTitle.getLength() )
								    {
								        try
								        {
	    								    aSdgCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
    					                }
	                                    catch( const uno::RuntimeException& )
	                                    {
	                                    }
	                                    catch( const uno::Exception& )
	                                    {
	                                    }
							        }
							    }

							    if( !bReadOnly )
							    {
							        try
							        {
								        aSdvCnt.getPropertyValue( s_sTitle ) >>= aTitle;
    					            }
	                                catch( const ::com::sun::star::uno::RuntimeException& )
	                                {
	                                }
	                                catch( const ::com::sun::star::uno::Exception& )
	                                {
	                                }

								    if( aTitle.getLength() )
								    {
								        try
								        {
    									    aSdvCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
    					                }
	                                    catch( const uno::RuntimeException& )
	                                    {
	                                    }
	                                    catch( const uno::Exception& )
	                                    {
	                                    }
								    }
							    }

							    GalleryThemeEntry* pEntry = GalleryTheme::CreateThemeEntry( aThmURL, rbDirIsReadOnly || bReadOnly );

							    if( pEntry )
							    {
								    const sal_uIntPtr nFileNumber = (sal_uIntPtr) String(aThmURL.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32();

								    aThemeList.Insert( pEntry, LIST_APPEND );

								    if( nFileNumber > nLastFileNumber )
									    nLastFileNumber = nFileNumber;
							    }
						    }
						}
	                    catch( const ucb::ContentCreationException& )
	                    {
	                    }
	                    catch( const uno::RuntimeException& )
	                    {
	                    }
	                    catch( const uno::Exception& )
	                    {
	                    }
					}
				}
			}
		}
	}
	catch( const ucb::ContentCreationException& )
	{
	}
	catch( const uno::RuntimeException& )
	{
	}
	catch( const uno::Exception& )
	{
	}
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadImports()
{
	INetURLObject aURL( GetUserURL() );

	aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "gallery.sdi" ) ) );

	if( FileExists( aURL ) )
	{
		SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

		if( pIStm )
		{
			GalleryThemeEntry*			pThemeEntry;
			GalleryImportThemeEntry*	pImportEntry;
			INetURLObject               aFile;
			sal_uInt32                      nInventor;
			sal_uInt32                      nCount;
			sal_uInt16                      nId;
			sal_uInt16                      i;
			sal_uInt16                      nTempCharSet;

			for( pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
				delete pImportEntry;

			aImportList.Clear();
			*pIStm >> nInventor;

			if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
			{
				*pIStm >> nId >> nCount >> nTempCharSet;

				for( i = 0; i < nCount; i++ )
				{
					pImportEntry = new GalleryImportThemeEntry;

					*pIStm >> *pImportEntry;
					aImportList.Insert( pImportEntry, LIST_APPEND );
					aFile = INetURLObject( pImportEntry->aURL );
					pThemeEntry = new GalleryThemeEntry( aFile,
														 pImportEntry->aUIName,
														 String(aFile.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32(),
														 sal_True, sal_True, sal_False, 0, sal_False );

					aThemeList.Insert( pThemeEntry, LIST_APPEND );
				}
			}

			delete pIStm;
		}
	}
}

// ------------------------------------------------------------------------

void Gallery::ImplWriteImportList()
{
	INetURLObject aURL( GetUserURL() );
	aURL.Append( ( String( "gallery.sdi", RTL_TEXTENCODING_UTF8 ) ) );
	SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

	if( pOStm )
	{
		const sal_uInt32 nInventor = (sal_uInt32) COMPAT_FORMAT( 'S', 'G', 'A', '3' );
		const sal_uInt16 nId = 0x0004;

		*pOStm << nInventor << nId << (sal_uInt32) aImportList.Count() << (sal_uInt16) gsl_getSystemTextEncoding();

		for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
			*pOStm << *pImportEntry;

		if( pOStm->GetError() )
			ErrorHandler::HandleError( ERRCODE_IO_GENERAL );

		delete pOStm;
	}
}

// ------------------------------------------------------------------------

GalleryThemeEntry* Gallery::ImplGetThemeEntry( const String& rThemeName )
{
	GalleryThemeEntry* pFound = NULL;

	if( rThemeName.Len() )
		for( GalleryThemeEntry* pEntry = aThemeList.First(); pEntry && !pFound; pEntry = aThemeList.Next() )
			if( rThemeName == pEntry->GetThemeName() )
				pFound = pEntry;

	return pFound;
}

// ------------------------------------------------------------------------

GalleryImportThemeEntry* Gallery::ImplGetImportThemeEntry( const String& rImportName )
{
	GalleryImportThemeEntry* pFound = NULL;

	for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry && !pFound; pImportEntry = aImportList.Next() )
		if ( rImportName == pImportEntry->aUIName )
			pFound = pImportEntry;

	return pFound;
}

// ------------------------------------------------------------------------

String Gallery::GetThemeName( sal_uIntPtr nThemeId ) const
{
	GalleryThemeEntry* pFound = NULL;

	for( sal_uIntPtr n = 0, nCount = aThemeList.Count(); n < nCount; n++ )
	{
		GalleryThemeEntry* pEntry = aThemeList.GetObject( n );

		if( nThemeId == pEntry->GetId() )
			pFound = pEntry;
	}

	// try fallback, if no entry was found
	if( !pFound )
	{
		ByteString      aFallback;

		switch( nThemeId )
		{
			case( GALLERY_THEME_3D ): aFallback = "3D"; break;
			case( GALLERY_THEME_BULLETS ): aFallback = "Bullets"; break;
			case( GALLERY_THEME_HOMEPAGE ): aFallback = "Homepage"; break;
			case( GALLERY_THEME_POWERPOINT ): aFallback = "private://gallery/hidden/imgppt"; break;
			case( GALLERY_THEME_FONTWORK ): aFallback = "private://gallery/hidden/fontwork"; break;
			case( GALLERY_THEME_FONTWORK_VERTICAL ): aFallback = "private://gallery/hidden/fontworkvertical"; break;
			case( GALLERY_THEME_RULERS ): aFallback = "Rulers"; break;
			case( GALLERY_THEME_SOUNDS ): aFallback = "Sounds"; break;

            case( RID_GALLERYSTR_THEME_ARROWS ): aFallback = "Arrows"; break;
            case( RID_GALLERYSTR_THEME_COMPUTERS ): aFallback = "Computers"; break;
            case( RID_GALLERYSTR_THEME_DIAGRAMS ): aFallback = "Diagrams"; break;
            case( RID_GALLERYSTR_THEME_EDUCATION ): aFallback = "Education"; break;
            case( RID_GALLERYSTR_THEME_ENVIRONMENT ): aFallback = "Environment"; break;
            case( RID_GALLERYSTR_THEME_FINANCE ): aFallback = "Finance"; break;
            case( RID_GALLERYSTR_THEME_PEOPLE ): aFallback = "People"; break;
            case( RID_GALLERYSTR_THEME_SYMBOLS ): aFallback = "Symbols"; break;
            case( RID_GALLERYSTR_THEME_TRANSPORT ): aFallback = "Transport"; break;
            case( RID_GALLERYSTR_THEME_TXTSHAPES ): aFallback = "Textshapes"; break;

            default:
			break;
		}

		pFound = ( (Gallery*) this )->ImplGetThemeEntry( String::CreateFromAscii( aFallback.GetBuffer() ) );
	}

	return( pFound ? pFound->GetThemeName() : String() );
}

// ------------------------------------------------------------------------

sal_Bool Gallery::HasTheme( const String& rThemeName )
{
	return( ImplGetThemeEntry( rThemeName ) != NULL );
}

// ------------------------------------------------------------------------

sal_Bool Gallery::CreateTheme( const String& rThemeName, sal_uInt32 nNumFrom )
{
	sal_Bool bRet = sal_False;

	if( !HasTheme( rThemeName ) && ( GetUserURL().GetProtocol() != INET_PROT_NOT_VALID ) )
	{
		nLastFileNumber = nNumFrom > nLastFileNumber ? nNumFrom : nLastFileNumber + 1;
		GalleryThemeEntry* pNewEntry = new GalleryThemeEntry( GetUserURL(), rThemeName,
															  nLastFileNumber,
															  sal_False, sal_False, sal_True, 0, sal_False );

		aThemeList.Insert( pNewEntry, LIST_APPEND );
		delete( new GalleryTheme( this, pNewEntry ) );
		Broadcast( GalleryHint( GALLERY_HINT_THEME_CREATED, rThemeName ) );
		bRet = sal_True;
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Gallery::CreateImportTheme( const INetURLObject& rURL, const String& rImportName )
{
	INetURLObject	aURL( rURL );
	sal_Bool			bRet = sal_False;

	DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

	if( FileExists( aURL ) )
	{
		SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

		if( pIStm )
		{
			sal_uIntPtr   nStmErr;
			sal_uInt16  nId;

			*pIStm >> nId;

			if( nId > 0x0004 )
				ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
			else
			{
				ByteString				aTmpStr;
				String                  aThemeName; *pIStm >> aTmpStr; aThemeName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
				GalleryThemeEntry*      pThemeEntry = new GalleryThemeEntry( aURL, rImportName,
																			 String(aURL.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32(),
																			 sal_True, sal_True, sal_True, 0, sal_False );
				GalleryTheme*           pImportTheme = new GalleryTheme( this, pThemeEntry );

				pIStm->Seek( STREAM_SEEK_TO_BEGIN );
				*pIStm >> *pImportTheme;
				nStmErr = pIStm->GetError();

				if( nStmErr )
				{
					delete pThemeEntry;
					ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
				}
				else
				{
					String  aName( rImportName );
					String  aNewName( aName );
					sal_uIntPtr   nCount = 0;

					aName += ' ';

					while ( HasTheme( aNewName ) && ( nCount++ < 16000 ) )
					{
						aNewName = aName;
						aNewName += String::CreateFromInt32( nCount );
					}

					pImportTheme->SetImportName( aNewName );
					aThemeList.Insert( pThemeEntry, LIST_APPEND );

					// Thema in Import-Liste eintragen und Import-Liste     speichern
					GalleryImportThemeEntry* pImportEntry = new GalleryImportThemeEntry;
					pImportEntry->aThemeName = pImportEntry->aUIName = aNewName;
					pImportEntry->aURL = rURL;
					pImportEntry->aImportName = rImportName;
					aImportList.Insert( pImportEntry, LIST_APPEND );
					ImplWriteImportList();
					bRet = sal_True;
				}

				delete pImportTheme;
			}

			delete pIStm;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Gallery::RenameTheme( const String& rOldName, const String& rNewName )
{
	GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );
	sal_Bool                            bRet = sal_False;

	// Ueberpruefen, ob neuer Themenname schon vorhanden ist
	if( pThemeEntry && !HasTheme( rNewName ) && ( !pThemeEntry->IsReadOnly() || pThemeEntry->IsImported() ) )
	{
		SfxListener   aListener;
		GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

		if( pThm )
		{
			const String aOldName( rOldName );

			pThemeEntry->SetName( rNewName );
			pThm->ImplWrite();

			if( pThemeEntry->IsImported() )
			{
				pThm->SetImportName( rNewName );

				GalleryImportThemeEntry* pImportEntry = ImplGetImportThemeEntry( rOldName );

				if( pImportEntry )
				{
					pImportEntry->aUIName = rNewName;
					ImplWriteImportList();
				}
			}

			Broadcast( GalleryHint( GALLERY_HINT_THEME_RENAMED, aOldName, pThm->GetName() ) );
			ReleaseTheme( pThm, aListener );
			bRet = sal_True;
		}
	}

	return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Gallery::RemoveTheme( const String& rThemeName )
{
	GalleryThemeEntry*	pThemeEntry = ImplGetThemeEntry( rThemeName );
	sal_Bool                bRet = sal_False;

	if( pThemeEntry && ( !pThemeEntry->IsReadOnly() || pThemeEntry->IsImported() ) )
	{
		Broadcast( GalleryHint( GALLERY_HINT_CLOSE_THEME, rThemeName ) );

		if( pThemeEntry->IsImported() )
		{
			GalleryImportThemeEntry* pImportEntry = ImplGetImportThemeEntry( rThemeName );

			if( pImportEntry )
			{
				delete aImportList.Remove( pImportEntry );
				ImplWriteImportList();
			}
		}
		else
		{
			SfxListener		aListener;
			GalleryTheme*	pThm = AcquireTheme( rThemeName, aListener );

            if( pThm )
            {
                INetURLObject	aThmURL( pThm->GetThmURL() );
                INetURLObject	aSdgURL( pThm->GetSdgURL() );
                INetURLObject	aSdvURL( pThm->GetSdvURL() );

                ReleaseTheme( pThm, aListener );

                KillFile( aThmURL );
                KillFile( aSdgURL );
                KillFile( aSdvURL );
            }
		}

		delete aThemeList.Remove( pThemeEntry );
		Broadcast( GalleryHint( GALLERY_HINT_THEME_REMOVED, rThemeName ) );

		bRet = sal_True;
	}

	return bRet;
}

// ------------------------------------------------------------------------

INetURLObject Gallery::GetImportURL( const String& rThemeName )
{
	INetURLObject				aURL;
	GalleryImportThemeEntry*	pImportEntry = ImplGetImportThemeEntry( rThemeName );

	if( pImportEntry )
	{
		aURL = pImportEntry->aURL;
		DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
	}

	return aURL;
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry )
{
	GalleryTheme* pTheme = NULL;

	if( pThemeEntry )
	{
		GalleryThemeCacheEntry* pEntry;

		for( pEntry = (GalleryThemeCacheEntry*) aThemeCache.First(); pEntry && !pTheme; pEntry = (GalleryThemeCacheEntry*) aThemeCache.Next() )
			if( pThemeEntry == pEntry->GetThemeEntry() )
				pTheme = pEntry->GetTheme();

		if( !pTheme )
		{
			INetURLObject aURL;

			if( !pThemeEntry->IsImported() )
				aURL = pThemeEntry->GetThmURL();
			else
				aURL = GetImportURL( pThemeEntry->GetThemeName() );

			DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

			if( FileExists( aURL ) )
			{
				SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

				if( pIStm )
				{
					pTheme = new GalleryTheme( this, (GalleryThemeEntry*) pThemeEntry );
					*pIStm >> *pTheme;

					if( pIStm->GetError() )
						delete pTheme, pTheme = NULL;
					else if( pThemeEntry->IsImported() )
						pTheme->SetImportName( pThemeEntry->GetThemeName() );

					delete pIStm;
				}
			}

			if( pTheme )
				aThemeCache.Insert( new GalleryThemeCacheEntry( pThemeEntry, pTheme ), LIST_APPEND );
		}
	}

	return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ImplDeleteCachedTheme( GalleryTheme* pTheme )
{
	GalleryThemeCacheEntry* pEntry;
	sal_Bool                                    bDone = sal_False;

	for( pEntry = (GalleryThemeCacheEntry*) aThemeCache.First(); pEntry && !bDone; pEntry = (GalleryThemeCacheEntry*) aThemeCache.Next() )
	{
		if( pTheme == pEntry->GetTheme() )
		{
			delete (GalleryThemeCacheEntry*) aThemeCache.Remove( pEntry );
			bDone = sal_True;
		}
	}
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::AcquireTheme( const String& rThemeName, SfxListener& rListener )
{
	GalleryTheme*           pTheme = NULL;
	GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

	if( pThemeEntry && ( ( pTheme = ImplGetCachedTheme( pThemeEntry ) ) != NULL ) )
		rListener.StartListening( *pTheme );

	return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener )
{
	if( pTheme )
	{
		rListener.EndListening( *pTheme );

		if( !pTheme->HasListeners() )
			ImplDeleteCachedTheme( pTheme );
	}
}

sal_Bool GalleryThemeEntry::IsDefault() const 
{ return( ( nId > 0 ) && ( nId != ( RID_GALLERYSTR_THEME_MYTHEME - RID_GALLERYSTR_THEME_START ) ) ); }

