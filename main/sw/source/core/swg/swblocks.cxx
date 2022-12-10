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
#include "precompiled_sw.hxx"


#include <sfx2/docfilt.hxx>
#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svl/fstathelper.hxx>
#endif
#include <svl/macitem.hxx>
#include <unotools/charclass.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <swblocks.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>		// Progress
#include <SwXMLTextBlocks.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <swunohelper.hxx>

#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
#include <swerror.h>

SV_IMPL_OP_PTRARR_SORT( SwBlockNames, SwBlockName* );

//////////////////////////////////////////////////////////////////////////

// Hash-Code errechnen (muss nicht eindeutig sein)


sal_uInt16 SwImpBlocks::Hash( const String& r )
{
	sal_uInt16 n = 0;
	xub_StrLen nLen = r.Len();
	if( nLen > 8 )
		nLen = 8;
	const sal_Unicode* p = r.GetBuffer();
	while( nLen-- )
		n = ( n << 1 ) + *p++;
	return n;
}


SwBlockName::SwBlockName( const String& rShort, const String& rLong, long n )
	: nPos( n ), aShort( rShort ), aLong( rLong ), aPackageName (rShort),
	bIsOnlyTxtFlagInit( sal_False ), bIsOnlyTxt( sal_False )
{
	nHashS = SwImpBlocks::Hash( rShort );
	nHashL = SwImpBlocks::Hash( rLong );
}
SwBlockName::SwBlockName( const String& rShort, const String& rLong, const String& rPackageName)
	: nPos( 0 ), aShort( rShort ), aLong( rLong ), aPackageName (rPackageName),
	bIsOnlyTxtFlagInit( sal_False ), bIsOnlyTxt( sal_False )
{
	nHashS = SwImpBlocks::Hash( rShort );
	nHashL = SwImpBlocks::Hash( rLong );
}


// Ist die angegebene Datei ein Storage oder gibt es sie nicht?

short SwImpBlocks::GetFileType( const String& rFile )
{
	if( !FStatHelper::IsDocument( rFile ) )
		return SWBLK_NO_FILE;
	if( SwXMLTextBlocks::IsFileUCBStorage( rFile ) )
		return SWBLK_XML;
	if( SvStorage::IsStorageFile( rFile ) )
		return SWBLK_SW3;
    //otherwise return NONE
    return SWBLK_NONE;
}


SwImpBlocks::SwImpBlocks( const String& rFile, sal_Bool )
	: aFile( rFile ), pDoc( 0 ), nCur( (sal_uInt16)-1 ),
    bReadOnly( sal_True ), bInPutMuchBlocks( sal_False )
{
	FStatHelper::GetModifiedDateTimeOfFile( rFile,
											&aDateModified, &aTimeModified );
	INetURLObject aObj(rFile);
	aObj.setExtension( aEmptyStr );
	aName = aObj.GetBase();
}


SwImpBlocks::~SwImpBlocks()
{
	aNames.DeleteAndDestroy( 0, aNames.Count() );
}

// Loeschen des Inhaltes des Dokuments
void SwImpBlocks::ClearDoc()
{
	pDoc->ClearDoc();
}

sal_uLong SwImpBlocks::GetDocForConversion( sal_uInt16 n )
{
	return GetDoc( n );
}

// Erzeugen eines PaMs, der das ganze Dokument umfasst
SwPaM* SwImpBlocks::MakePaM()
{
	SwPaM* pPam = new SwPaM( pDoc->GetNodes().GetEndOfContent() );
	pPam->Move( fnMoveBackward, fnGoDoc );
	pPam->SetMark();
	pPam->Move( fnMoveForward, fnGoDoc );
	pPam->Exchange();
	return pPam;
}


sal_uInt16 SwImpBlocks::GetCount() const
{
	return aNames.Count();
}

// Case Insensitive
sal_uInt16 SwImpBlocks::GetIndex( const String& rShort ) const
{
	String s( GetAppCharClass().upper( rShort ) );
	sal_uInt16 nHash = Hash( s );
	for( sal_uInt16 i = 0; i < aNames.Count(); i++ )
	{
		SwBlockName* pName = aNames[ i ];
		if( pName->nHashS == nHash
		 && pName->aShort == s )
			return i;
	}
	return (sal_uInt16) -1;
}


sal_uInt16 SwImpBlocks::GetLongIndex( const String& rLong ) const
{
	sal_uInt16 nHash = Hash( rLong );
	for( sal_uInt16 i = 0; i < aNames.Count(); i++ )
	{
		SwBlockName* pName = aNames[ i ];
		if( pName->nHashL == nHash
		 && pName->aLong == rLong )
			return i;
	}
	return (sal_uInt16) -1;
}


const String& SwImpBlocks::GetShortName( sal_uInt16 n ) const
{
	if( n < aNames.Count() )
		return aNames[ n ]->aShort;
	return aEmptyStr;
}


const String& SwImpBlocks::GetLongName( sal_uInt16 n ) const
{
	if( n < aNames.Count() )
		return aNames[ n ]->aLong;
	return aEmptyStr;
}

const String& SwImpBlocks::GetPackageName( sal_uInt16 n ) const
{
	if( n < aNames.Count() )
		return aNames[ n ]->aPackageName;
	return aEmptyStr;
}

void SwImpBlocks::AddName( const String& rShort, const String& rLong,
							sal_Bool bOnlyTxt )
{
	sal_uInt16 nIdx = GetIndex( rShort );
	if( nIdx != (sal_uInt16) -1 )
		aNames.DeleteAndDestroy( nIdx );
	SwBlockName* pNew = new SwBlockName( rShort, rLong, 0L );
	pNew->bIsOnlyTxtFlagInit = sal_True;
	pNew->bIsOnlyTxt = bOnlyTxt;
	aNames.C40_PTR_INSERT( SwBlockName, pNew );
}



sal_Bool SwImpBlocks::IsFileChanged() const
{
	Date aTempDateModified( aDateModified );
	Time aTempTimeModified( aTimeModified );
	return FStatHelper::GetModifiedDateTimeOfFile( aFile,
							&aTempDateModified, &aTempTimeModified ) &&
		  ( aDateModified != aTempDateModified ||
			aTimeModified != aTempTimeModified );
}


void SwImpBlocks::Touch()
{
	FStatHelper::GetModifiedDateTimeOfFile( aFile,
											&aDateModified, &aTimeModified );
}

sal_Bool SwImpBlocks::IsOnlyTextBlock( const String& ) const
{
	return sal_False;
}

sal_uLong SwImpBlocks::GetMacroTable( sal_uInt16, SvxMacroTableDtor&, sal_Bool )
{
	return 0;
}

sal_uLong SwImpBlocks::SetMacroTable( sal_uInt16 ,
								const SvxMacroTableDtor& , sal_Bool )
{
	return 0;
}

sal_Bool SwImpBlocks::PutMuchEntries( sal_Bool )
{
	return sal_False;
}

////////////////////////////////////////////////////////////////////////////


SwTextBlocks::SwTextBlocks( const String& rFile )
	: pImp( 0 ), nErr( 0 )
{
	INetURLObject aObj(rFile);
	String sFileName = aObj.GetMainURL( INetURLObject::NO_DECODE );
	switch( SwImpBlocks::GetFileType( rFile ) )
	{
    //case SWBLK_SW2:     pImp = new Sw2TextBlocks( sFileName );  break;
    //case SWBLK_SW3:     pImp = new Sw3TextBlocks( sFileName );  break;
	case SWBLK_XML:		pImp = new SwXMLTextBlocks( sFileName ); break;
	case SWBLK_NO_FILE:	pImp = new SwXMLTextBlocks( sFileName ); break;
	}
	if( !pImp )
		nErr = ERR_SWG_FILE_FORMAT_ERROR;
}

SwTextBlocks::~SwTextBlocks()
{
	delete pImp;
}

const String& SwTextBlocks::GetName()
{
	return pImp ? pImp->aName : aEmptyStr;
}


void SwTextBlocks::SetName( const String& r )
{
	if( pImp )
		pImp->SetName( r );
}


sal_Bool SwTextBlocks::IsOld() const
{
	if (pImp)
	{
		short nType = pImp->GetFileType();
		if (SWBLK_SW3 == nType || SWBLK_SW2 == nType )
			return sal_True;
	}
	return sal_False;
}


/*
sal_uLong SwTextBlocks::ConvertToNew()
{
	// Wir nehmen die aktuelle Datei, benennen diese in .BAK um
	// und kreieren den neuen Storage
	if( IsOld() )
	{
		// Erst mal muessen wir die Datei freigeben
		short nType = pImp->GetFileType();
		Sw2TextBlocks *pTwo = NULL;
		Sw3TextBlocks *pThree = NULL;
		SwImpBlocks *pOld = NULL;

		pImp->nCur = (sal_uInt16) -1;
		String aName( pImp->aFile );
		delete pImp; pImp = NULL;
		// Jetzt wird umbenannt
		INetURLObject aOldFull( aName );
		INetURLObject aNewFull( aName );

		aOldFull.SetExtension( String::CreateFromAscii("bak") );
		String aOld( aOldFull.GetMainURL( INetURLObject::NO_DECODE ) );
		String aNew( aNewFull.GetMainURL( INetURLObject::NO_DECODE ) );

		sal_Bool bError = !SWUnoHelper::UCB_CopyFile( aNew, aOld, sal_True );
		if( bError )
		{
			if (nType == SWBLK_SW2)
				pImp = new Sw2TextBlocks( aOld );
			else
				pImp = new Sw3TextBlocks( aOld );
			return nErr = ERR_SWG_CANNOT_WRITE;
		}

		// Die Datei ist erfolgreich umbenannt. Jetzt wird der Storage
		// aufgesetzt
		if (nType == SWBLK_SW2)
			pOld = pTwo = new Sw2TextBlocks( aOld );
		else
			pOld = pThree = new Sw3TextBlocks( aOld );
		SwXMLTextBlocks* pNew = new SwXMLTextBlocks( aName );
		pNew->SetName ( pOld->GetName());
		// Wir kopieren den Doc-Ptr in das alte System
		// den alten SvPersist heben wir uns aber auf,
		// da dieser die ganze Zeit leben bleibt
		// und lesen die Dateivorlagen erneut ein
		SvPersist* pPersist2 = pOld->pDoc->GetPersist();
		if (SWBLK_SW2 == nType )
		{
			delete pOld->pDoc;
			pOld->pDoc = pNew->pDoc;nLinkCt
			nErr = pTwo->LoadDoc();
		}
		else
		{
			nErr = pThree->OpenFile ( sal_True );
			// Within this call, Sw3IoImp::SetDoc calls RemoveLink
			// on the old document, and deletes it if the
			// ref count is now zero
			pThree->SetDoc ( pNew->pDoc );
			pOld->pDoc->AddLink();
		}
		if( !nErr && 0 == ( nErr = pNew->OpenFile( sal_False )) )
		{
			nErr = pNew->SetConvertMode( sal_True );
			// jetzt werden die Bausteine einfach umkopiert!
			if( !nErr )
			{
				if (SWBLK_SW2 == nType)
					pTwo->StatLineStartPercent();
				sal_uInt16 nCount = pOld->GetCount();
				for( sal_uInt16 i = 0; i < nCount; i++ )
				{
					pNew->ClearDoc();
					String aShort( pOld->GetShortName( i ) );
					String aLong( pOld->GetLongName( i ) );
					pNew->AddName( aShort, aLong );
					if ( SWBLK_SW3 == nType && pThree->IsOnlyTextBlock(aShort) )
					{
						String sText;
						pThree->GetText( aShort, sText );
						pNew->PutText( aShort, aLong, sText );
					}
					else
					{
						if (SWBLK_SW2 == nType )
						{
							// I think this is how it should work (!!!!!!) mtg
							pNew->pDoc->SetPersist( pPersist2 );
						}
						nErr = pOld->GetDocForConversion( i );
						if( nErr )
							break;
						nErr = pNew->BeginPutDoc( aShort, aLong );
						if( nErr )
							break;
						nErr = pNew->PutDoc();
						if( nErr )
							break;
					}

					// convert macros, too
					SvxMacroTableDtor aMacroTable;
					pOld->GetMacroTable( i, aMacroTable, sal_True );
					pNew->SetMacroTable( i, aMacroTable, sal_True );

					if (SWBLK_SW2 == nType )
						pNew->pDoc->SetPersist( 0 );
				}
				if (SWBLK_SW2 == nType )
					::EndProgress( pOld->pDoc->GetDocShell() );
			}
			if( !nErr )
				nErr = pNew->SetConvertMode( sal_False );
		}
		if ( SWBLK_SW3 == nType )
		{
			pThree->CloseFile();
		}
		else
		{
			// Haben wir es geschafft?
			pOld->pDoc = NULL;
		}
		pNew->ClearDoc();
		if( !nErr )
		{
			delete pOld;
			pImp = pNew;
			SWUnoHelper::UCB_DeleteFile( aOld );
			pNew->MakeBlockList();
		}
		else
		{
			delete pOld; delete pNew;
			SWUnoHelper::UCB_DeleteFile( aNew );
			SWUnoHelper::UCB_CopyFile( aOld, aNew, sal_True );
			if ( SWBLK_SW2 == nType )
				pImp = new Sw2TextBlocks( aOld );
			else
				pImp = new Sw3TextBlocks( aOld );
		}
		pNew->CloseFile();
		FStatHelper::GetModifiedDateTimeOfFile( aNew,
							&pImp->aDateModified, &pImp->aTimeModified );
	}
	return nErr;
} */


sal_uInt16 SwTextBlocks::GetCount() const
{
	return pImp ? pImp->GetCount() : 0;
}


sal_uInt16 SwTextBlocks::GetIndex( const String& r ) const
{
	return pImp ? pImp->GetIndex( r ) : (sal_uInt16) -1;
}


sal_uInt16 SwTextBlocks::GetLongIndex( const String& r ) const
{
	return pImp ? (sal_uInt16)(pImp->GetLongIndex( r )) : (sal_uInt16) -1;
}


const String& SwTextBlocks::GetShortName( sal_uInt16 n ) const
{
	if( pImp )
		return pImp->GetShortName( n );
	return aEmptyStr;
}


const String& SwTextBlocks::GetLongName( sal_uInt16 n ) const
{
	if( pImp )
		return pImp->GetLongName( n );
	return aEmptyStr;
}


sal_Bool SwTextBlocks::Delete( sal_uInt16 n )
{
	if( pImp && !pImp->bInPutMuchBlocks )
	{
		if( pImp->IsFileChanged() )
			nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
		else if( 0 == (nErr = pImp->OpenFile( sal_False ) ))
		{
			nErr = pImp->Delete( n );
			if( !nErr )
				pImp->aNames.DeleteAndDestroy( n );
			if( n == pImp->nCur )
				pImp->nCur = (sal_uInt16) -1;
			if( !nErr )
				nErr = pImp->MakeBlockList();
		}
		pImp->CloseFile();
		pImp->Touch();

		return sal_Bool( nErr == 0 );
	}
	return sal_False;
}


sal_uInt16 SwTextBlocks::Rename( sal_uInt16 n, const String* s, const String* l )
{
	sal_uInt16 nIdx = (sal_uInt16)-1;
	if( pImp && !pImp->bInPutMuchBlocks )
	{
		pImp->nCur = nIdx;
		String aNew, aLong;
		if( s )
			aNew = aLong = *s;
		if( l )
			aLong = *l;
		if( !aNew.Len() )
		{
			ASSERT( sal_False, "No short name specified in Rename" );
			nErr = ERR_SWG_INTERNAL_ERROR; return (sal_uInt16) -1;
		}

		if( pImp->IsFileChanged() )
			nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
		else if( 0 == (	nErr = pImp->OpenFile( sal_False )))
		{
			// Vorher den neuen Eintrag in die Liste setzen!
			GetAppCharClass().toUpper( aNew );
 			nErr = pImp->Rename( n, aNew, aLong );
			if( !nErr )
			{
				sal_Bool bOnlyTxt = pImp->aNames[ n ]->bIsOnlyTxt;
				pImp->aNames.DeleteAndDestroy( n );
				pImp->AddName( aNew, aLong, bOnlyTxt );
				nErr = pImp->MakeBlockList();
			}
		}
		pImp->CloseFile();
		pImp->Touch();
		if( !nErr )
			nIdx = pImp->GetIndex( aNew );
	}
	return nIdx;
}

sal_uLong SwTextBlocks::CopyBlock( SwTextBlocks& rSource, String& rSrcShort,
								const String& rLong )
{
	sal_Bool bIsOld = sal_False;
	if (rSource.pImp)
	{
		short nType = rSource.pImp->GetFileType();
		if (SWBLK_SW2 == nType || SWBLK_SW3 == nType )
			bIsOld = sal_True;
	}
	if( bIsOld ) //rSource.IsOld() )
		nErr = ERR_SWG_OLD_GLOSSARY;
	else if( pImp->bInPutMuchBlocks )
		nErr = ERR_SWG_INTERNAL_ERROR;
	else
		nErr = pImp->CopyBlock(*rSource.pImp, rSrcShort, rLong);
	return nErr;
}

sal_Bool SwTextBlocks::BeginGetDoc( sal_uInt16 n )
{
	if( pImp && !pImp->bInPutMuchBlocks )
	{
// diese Optimierierung darf es nicht mehr geben. OLE-Objecte muessen auf
// ihre SubStorages zugreifem koennen!
//		if( n == pImp->nCur )
//			return sal_True;

		if( pImp->IsFileChanged() )
			nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
		else if( 0 == (	nErr = pImp->OpenFile( sal_True )))
		{
			pImp->ClearDoc();
			nErr = pImp->GetDoc( n );
			if( nErr )
				pImp->nCur = (sal_uInt16)-1;
			else
				pImp->nCur = n;
		}
		return sal_Bool( nErr == 0 );
	}
	return sal_False;
}


void SwTextBlocks::EndGetDoc()
{
	if( pImp && !pImp->bInPutMuchBlocks )
		pImp->CloseFile();
}


sal_Bool SwTextBlocks::BeginPutDoc( const String& s, const String& l )
{
	if( pImp )
	{
		sal_Bool bOk = pImp->bInPutMuchBlocks;
		if( !bOk )
		{
			if( pImp->IsFileChanged() )
				nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
			else
				nErr = pImp->OpenFile( sal_False );
			bOk = 0 == nErr;
		}
		if( bOk )
		{
			String aNew( s );
			GetAppCharClass().toUpper( aNew );
			nErr = pImp->BeginPutDoc( aNew, l );
		}
		if( nErr )
			pImp->CloseFile();
	}
	return 0 == nErr;
}


sal_uInt16 SwTextBlocks::PutDoc()
{
	sal_uInt16 nIdx = (sal_uInt16)-1;
	if( pImp )
	{
		nErr = pImp->PutDoc();
		if( !nErr )
		{
			pImp->nCur = GetIndex( pImp->aShort );
			if( pImp->nCur != (sal_uInt16) -1 )
				pImp->aNames[ pImp->nCur ]->aLong = pImp->aLong;
			else
			{
				pImp->AddName( pImp->aShort, pImp->aLong );
				pImp->nCur = pImp->GetIndex( pImp->aShort );
			}
			if( !pImp->bInPutMuchBlocks )
				nErr = pImp->MakeBlockList();
		}
		if( !pImp->bInPutMuchBlocks )
		{
			pImp->CloseFile();
			pImp->Touch();
		}
		nIdx = pImp->nCur;
	}
	return nIdx;
}

sal_uInt16 SwTextBlocks::PutText( const String& rShort, const String& rName,
							  const String& rTxt )
{
	sal_uInt16 nIdx = (sal_uInt16) -1;
	if( pImp )
	{
		sal_Bool bOk = pImp->bInPutMuchBlocks;
		if( !bOk )
		{
			if( pImp->IsFileChanged() )
				nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
			else
				nErr = pImp->OpenFile( sal_False );
			bOk = 0 == nErr;
		}
		if( bOk )
		{
			String aNew( rShort );
			GetAppCharClass().toUpper( aNew );
			nErr = pImp->PutText( aNew, rName, rTxt );
			pImp->nCur = (sal_uInt16) -1;
			if( !nErr )
			{
				nIdx = GetIndex( pImp->aShort );
				if( nIdx != (sal_uInt16) -1 )
					pImp->aNames[ nIdx ]->aLong = rName;
				else
				{
					pImp->AddName( pImp->aShort, rName, sal_True );
					nIdx = pImp->GetIndex( pImp->aShort );
				}
				if( !pImp->bInPutMuchBlocks )
					nErr = pImp->MakeBlockList();
			}
		}
		if( !pImp->bInPutMuchBlocks )
		{
			pImp->CloseFile();
			pImp->Touch();
		}
	}
	return nIdx;
}


SwDoc* SwTextBlocks::GetDoc()
{
	if( pImp )
		return pImp->pDoc;
	return 0;
}


void SwTextBlocks::ClearDoc()
{
	if( pImp )
		pImp->ClearDoc();
	pImp->nCur = (sal_uInt16) -1;
}


const String& SwTextBlocks::GetFileName() const
{
	return pImp->GetFileName();
}


sal_Bool SwTextBlocks::IsReadOnly() const
{
	return pImp->bReadOnly;
}

sal_Bool SwTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
	sal_Bool bRet = sal_False;
	if( pImp && !pImp->bInPutMuchBlocks )
	{
		SwBlockName* pBlkNm = pImp->aNames[ nIdx ];
		if( !pBlkNm->bIsOnlyTxtFlagInit &&
			!pImp->IsFileChanged() && !pImp->OpenFile( sal_True ) )
		{
			pBlkNm->bIsOnlyTxt = pImp->IsOnlyTextBlock( pBlkNm->aShort );
			pBlkNm->bIsOnlyTxtFlagInit = sal_True;
			pImp->CloseFile();
		}
		bRet = pBlkNm->bIsOnlyTxt;
	}
	return bRet;
}

sal_Bool SwTextBlocks::IsOnlyTextBlock( const String& rShort ) const
{
	sal_uInt16 nIdx = pImp->GetIndex( rShort );
	if( USHRT_MAX != nIdx )
	{
		if( pImp->aNames[ nIdx ]->bIsOnlyTxtFlagInit )
			return pImp->aNames[ nIdx ]->bIsOnlyTxt;
		return IsOnlyTextBlock( nIdx );
	}

	ASSERT( sal_False, "invalid name" );
	return sal_False;
}

sal_Bool SwTextBlocks::GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTbl )
{
    sal_Bool bRet = sal_True;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->GetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

sal_Bool SwTextBlocks::SetMacroTable( sal_uInt16 nIdx,
								const SvxMacroTableDtor& rMacroTbl )
{
    sal_Bool bRet = sal_True;
	if ( pImp && !pImp->bInPutMuchBlocks )
		bRet = ( 0 == pImp->SetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

sal_Bool SwTextBlocks::StartPutMuchBlockEntries()
{
	sal_Bool bRet = sal_False;
	if( !IsOld() && pImp )
		bRet = pImp->PutMuchEntries( sal_True );
	return bRet;
}

void SwTextBlocks::EndPutMuchBlockEntries()
{
	if( pImp )
		pImp->PutMuchEntries( sal_False );
}

/*-- 20.09.2004 10:25:33---------------------------------------------------

  -----------------------------------------------------------------------*/
String    SwTextBlocks::GetBaseURL() const
{
    String sRet;
    if(pImp)
        sRet = pImp->GetBaseURL();
    return sRet;
}
/*-- 20.09.2004 10:25:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTextBlocks::SetBaseURL( const String& rURL )
{
    if(pImp)
        pImp->SetBaseURL(rURL);
}


