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


#include <tools/urlobj.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>
#include <ndindex.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>
#include <editeng/svxacorr.hxx>

#include <editeng/acorrcfg.hxx>
#include <sfx2/docfile.hxx>
#include <docsh.hxx>

#include <vector>
/* -----------------------------05.08.2002 12:43------------------------------

 ---------------------------------------------------------------------------*/
class SwAutoCompleteClient : public SwClient
{
    SwAutoCompleteWord* pAutoCompleteWord;
    SwDoc*              pDoc;
#ifdef DBG_UTIL
    static sal_uLong nSwAutoCompleteClientCount;
#endif
public:
    SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc);
    SwAutoCompleteClient(const SwAutoCompleteClient& rClient);
    ~SwAutoCompleteClient();

    SwAutoCompleteClient& operator=(const SwAutoCompleteClient& rClient);

    const SwDoc& GetDoc(){return *pDoc;}
#ifdef DBG_UTIL
    static sal_uLong GetElementCount() {return nSwAutoCompleteClientCount;}
#endif
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};
/* -----------------------------05.08.2002 12:48------------------------------

 ---------------------------------------------------------------------------*/
typedef std::vector<SwAutoCompleteClient> SwAutoCompleteClientVector;

class SwAutoCompleteWord_Impl
{
    SwAutoCompleteClientVector  aClientVector;
    SwAutoCompleteWord&         rAutoCompleteWord;
public:
    SwAutoCompleteWord_Impl(SwAutoCompleteWord& rParent) :
        rAutoCompleteWord(rParent){}
    void AddDocument(SwDoc& rDoc);
    void RemoveDocument(const SwDoc& rDoc);
};

/* -----------------------------05.08.2002 14:11------------------------------

 ---------------------------------------------------------------------------*/
typedef const SwDoc* SwDocPtr;
typedef std::vector<SwDocPtr> SwDocPtrVector;
class SwAutoCompleteString : public String
{
#ifdef DBG_UTIL
    static sal_uLong nSwAutoCompleteStringCount;
#endif
    SwDocPtrVector aSourceDocs;
    public:
        SwAutoCompleteString(const String& rStr, xub_StrLen nPos, xub_StrLen nLen);

        ~SwAutoCompleteString();
        void        AddDocument(const SwDoc& rDoc);
        //returns true if last document reference has been removed
        sal_Bool     RemoveDocument(const SwDoc& rDoc);
#ifdef DBG_UTIL
    static sal_uLong GetElementCount() {return nSwAutoCompleteStringCount;}
#endif
};
#ifdef DBG_UTIL
    sal_uLong SwAutoCompleteClient::nSwAutoCompleteClientCount = 0;
    sal_uLong SwAutoCompleteString::nSwAutoCompleteStringCount = 0;
#endif
/* -----------------------------06.08.2002 08:57------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteClient::SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc) :
        pAutoCompleteWord(&rToTell),
        pDoc(&rSwDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#ifdef DBG_UTIL
    ++nSwAutoCompleteClientCount;
#endif
}
/* -----------------------------05.08.2002 14:07------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteClient::SwAutoCompleteClient(const SwAutoCompleteClient& rClient) :
    SwClient(),
    pAutoCompleteWord(rClient.pAutoCompleteWord),
    pDoc(rClient.pDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#ifdef DBG_UTIL
    ++nSwAutoCompleteClientCount;
#endif
}
/* -----------------------------05.08.2002 14:10------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteClient::~SwAutoCompleteClient()
{
#ifdef DBG_UTIL
    --nSwAutoCompleteClientCount;
#endif
}
/* -----------------06.03.2003 15:30-----------------

 --------------------------------------------------*/
SwAutoCompleteClient& SwAutoCompleteClient::operator=(const SwAutoCompleteClient& rClient)
{
    pAutoCompleteWord = rClient.pAutoCompleteWord;
    pDoc = rClient.pDoc;
    if(rClient.GetRegisteredIn())
        ((SwModify*)rClient.GetRegisteredIn())->Add(this);
    else if(GetRegisteredIn())
        GetRegisteredInNonConst()->Remove(this);
    return *this;
}
/* -----------------------------05.08.2002 12:49------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoCompleteClient::Modify( const SfxPoolItem* pOld, const SfxPoolItem *)
{
    switch( pOld ? pOld->Which() : 0 )
	{
	case RES_REMOVE_UNO_OBJECT:
	case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
            pAutoCompleteWord->DocumentDying(*pDoc);
		break;

    }
}
/* -----------------------------05.08.2002 13:03------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoCompleteWord_Impl::AddDocument(SwDoc& rDoc)
{
    SwAutoCompleteClientVector::iterator aIt;
    for(aIt = aClientVector.begin(); aIt != aClientVector.end(); aIt++)
    {
        if(&aIt->GetDoc() == &rDoc)
            return;
    }
    aClientVector.push_back(SwAutoCompleteClient(rAutoCompleteWord, rDoc));
}
/* -----------------------------05.08.2002 14:33------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoCompleteWord_Impl::RemoveDocument(const SwDoc& rDoc)
{
    SwAutoCompleteClientVector::iterator aIt;
    for(aIt = aClientVector.begin(); aIt != aClientVector.end(); aIt++)
    {
        if(&aIt->GetDoc() == &rDoc)
        {
            aClientVector.erase(aIt);
            return;
        }
    }
}
/* -----------------------------06.08.2002 08:54------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteString::SwAutoCompleteString(const String& rStr, xub_StrLen nPos, xub_StrLen nLen) :
            String( rStr, nPos, nLen )
{
#ifdef DBG_UTIL
    ++nSwAutoCompleteStringCount;
#endif
}
/* -----------------------------05.08.2002 14:22------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteString::~SwAutoCompleteString()
{
#ifdef DBG_UTIL
    --nSwAutoCompleteStringCount;
#endif
}
/* -----------------------------05.08.2002 14:17------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoCompleteString::AddDocument(const SwDoc& rDoc)
{
    SwDocPtrVector::iterator aIt;
    for(aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); aIt++)
    {
        if(*aIt == &rDoc)
            return;
    }
    SwDocPtr pNew = &rDoc;
    aSourceDocs.push_back(pNew);
}
/* -----------------------------05.08.2002 14:36------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwAutoCompleteString::RemoveDocument(const SwDoc& rDoc)
{
    SwDocPtrVector::iterator aIt;
    for(aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); aIt++)
    {
        if(*aIt == &rDoc)
        {
            aSourceDocs.erase(aIt);
            return !aSourceDocs.size();
        }
    }
    return sal_False;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwAutoCompleteWord::SwAutoCompleteWord( sal_uInt16 nWords, sal_uInt16 nMWrdLen )
	: aWordLst( 0, 255 ), aLRULst( 0, 255 ),
    pImpl(new SwAutoCompleteWord_Impl(*this)),
	nMaxCount( nWords ),
	nMinWrdLen( nMWrdLen ),
	bLockWordLst( sal_False )
{
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
    for(sal_uInt16 nPos = aWordLst.Count(); nPos; nPos--)
    {
        SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)aWordLst[ nPos - 1 ];
        aWordLst.Remove( nPos - 1 );
        delete pCurrent;
    }
    delete pImpl;
#ifdef DBG_UTIL
    sal_uLong nStrings = SwAutoCompleteString::GetElementCount();
    sal_uLong nClients = SwAutoCompleteClient::GetElementCount();
    DBG_ASSERT(!nStrings && !nClients, "AutoComplete: clients or string count mismatch");
#endif
}

sal_Bool SwAutoCompleteWord::InsertWord( const String& rWord, SwDoc& rDoc )
{
    SwDocShell* pDocShell = rDoc.GetDocShell();
    SfxMedium* pMedium = pDocShell ? pDocShell->GetMedium() : 0;
    // strings from help module should not be added
    if( pMedium )
    {
        const INetURLObject& rURL = pMedium->GetURLObject();
        if ( rURL.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
            return sal_False;
    }

    String aNewWord(rWord);
    aNewWord.EraseAllChars( CH_TXTATR_INWORD );
    aNewWord.EraseAllChars( CH_TXTATR_BREAKWORD );

    pImpl->AddDocument(rDoc);
    sal_Bool bRet = sal_False;
    xub_StrLen nWrdLen = aNewWord.Len();
    while( nWrdLen && '.' == aNewWord.GetChar( nWrdLen-1 ))
		--nWrdLen;

	if( !bLockWordLst && nWrdLen >= nMinWrdLen )
	{
        SwAutoCompleteString* pAutoString;
        StringPtr pNew = pAutoString = new SwAutoCompleteString( aNewWord, 0, nWrdLen );
        pAutoString->AddDocument(rDoc);
		sal_uInt16 nInsPos;
		if( aWordLst.Insert( pNew, nInsPos ) )
		{
			bRet = sal_True;
			if( aLRULst.Count() < nMaxCount )
				aLRULst.Insert( pNew, 0 );
			else
			{
				// der letzte muss entfernt werden
				// damit der neue vorne Platz hat
				String* pDel = (String*)aLRULst[ nMaxCount - 1 ];

				void** ppData = (void**)aLRULst.GetData();
				memmove( ppData+1, ppData, (nMaxCount - 1) * sizeof( void* ));
				*ppData = pNew;

				aWordLst.Remove( pDel );
                delete (SwAutoCompleteString*)pDel;
			}
		}
		else
		{
            delete (SwAutoCompleteString*)pNew;
			// dann aber auf jedenfall nach "oben" moven
			pNew = aWordLst[ nInsPos ];

            //add the document to the already inserted string
            SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)pNew;
            pCurrent->AddDocument(rDoc);

			nInsPos = aLRULst.GetPos( (void*)pNew );
			ASSERT( USHRT_MAX != nInsPos, "String nicht gefunden" );
			if( nInsPos )
			{
				void** ppData = (void**)aLRULst.GetData();
				memmove( ppData+1, ppData, nInsPos * sizeof( void* ) );
				*ppData = pNew;
			}
		}
	}
	return bRet;
}

void SwAutoCompleteWord::SetMaxCount( sal_uInt16 nNewMax )
{
	if( nNewMax < nMaxCount && aLRULst.Count() > nNewMax )
	{
		// dann die unten ueberhaengenden entfernen
		sal_uInt16 nLRUIndex = nNewMax-1;
		while( nNewMax < aWordLst.Count() && nLRUIndex < aLRULst.Count())
		{
			sal_uInt16 nPos = aWordLst.GetPos( (String*)aLRULst[ nLRUIndex++ ] );
			ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
            void * pDel = aWordLst[nPos];
            aWordLst.Remove(nPos);
            delete (SwAutoCompleteString*)pDel;
        }
		aLRULst.Remove( nNewMax-1, aLRULst.Count() - nNewMax );
	}
	nMaxCount = nNewMax;
}

void SwAutoCompleteWord::SetMinWordLen( sal_uInt16 n )
{
	// will man wirklich alle Worte, die kleiner als die neue Min Laenge
	// sind entfernen?
	// JP 02.02.99 - erstmal nicht.

	// JP 11.03.99 - mal testhalber eingebaut
	if( n < nMinWrdLen )
	{
		for( sal_uInt16 nPos = 0; nPos < aWordLst.Count(); ++nPos  )
			if( aWordLst[ nPos ]->Len() < n )
			{
				void* pDel = aWordLst[ nPos ];
                aWordLst.Remove(nPos);

				sal_uInt16 nDelPos = aLRULst.GetPos( pDel );
				ASSERT( USHRT_MAX != nDelPos, "String nicht gefunden" );
				aLRULst.Remove( nDelPos );
				--nPos;
                delete (SwAutoCompleteString*)pDel;
            }
	}

	nMinWrdLen = n;
}

sal_Bool SwAutoCompleteWord::GetRange( const String& rWord, sal_uInt16& rStt,
									sal_uInt16& rEnd ) const
{
	const StringPtr pStr = (StringPtr)&rWord;
	aWordLst.Seek_Entry( pStr, &rStt );
	rEnd = rStt;

	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
	while( rEnd < aWordLst.Count() && rSCmp.isMatch( rWord, *aWordLst[ rEnd ]))
		++rEnd;

	return rStt < rEnd;
}

void SwAutoCompleteWord::CheckChangedList( const SvStringsISortDtor& rNewLst )
{
	sal_uInt16 nMyLen = aWordLst.Count(), nNewLen = rNewLst.Count();
	sal_uInt16 nMyPos = 0, nNewPos = 0;

	for( ; nMyPos < nMyLen && nNewPos < nNewLen; ++nMyPos, ++nNewPos )
	{
		const StringPtr pStr = rNewLst[ nNewPos ];
		while( aWordLst[ nMyPos ] != pStr )
		{
			void* pDel = aWordLst[ nMyPos ];
            aWordLst.Remove(nMyPos);

			sal_uInt16 nPos = aLRULst.GetPos( pDel );
			ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
			aLRULst.Remove( nPos );
            delete (SwAutoCompleteString*)pDel;
            if( nMyPos >= --nMyLen )
				break;
		}
	}
    //remove the elements at the end of the array
    if( nMyPos < nMyLen )
	{
        //clear LRU array first then delete the string object
        for( ; nNewPos < nMyLen; ++nNewPos )
		{
			void* pDel = aWordLst[ nNewPos ];
			sal_uInt16 nPos = aLRULst.GetPos( pDel );
			ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
			aLRULst.Remove( nPos );
            delete (SwAutoCompleteString*)pDel;
		}
        //remove from array
        aWordLst.Remove( nMyPos, nMyLen - nMyPos );
    }
}
/* -----------------------------05.08.2002 12:54------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoCompleteWord::DocumentDying(const SwDoc& rDoc)
{
    pImpl->RemoveDocument(rDoc);

    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    const sal_Bool bDelete = !pACorr->GetSwFlags().bAutoCmpltKeepList;
    for(sal_uInt16 nPos = aWordLst.Count(); nPos; nPos--)
    {
        SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)aWordLst[ nPos - 1 ];
        if(pCurrent->RemoveDocument(rDoc) && bDelete)
        {
            aWordLst.Remove( nPos - 1 );
            sal_uInt16 nLRUPos = aLRULst.GetPos( (void*)pCurrent );
            DBG_ASSERT(nLRUPos < USHRT_MAX, "word not found in LRU list" );
            aLRULst.Remove( nLRUPos );
            delete pCurrent;
        }
    }
}

