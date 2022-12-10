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

#include <string.h>			// fuer strchr()

#include <com/sun/star/i18n/UnicodeType.hdl>
#include <com/sun/star/i18n/WordType.hdl>

#include <unotools/charclass.hxx>

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <mvsave.hxx>		// Strukturen zum Sichern beim Move/Delete
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <rubylist.hxx>
#include <pam.hxx>
#include <swundo.hxx>		// fuer die UndoIds
#include <breakit.hxx>
#include <crsskip.hxx>

SV_IMPL_PTRARR( SwRubyList, SwRubyListEntryPtr )

using namespace ::com::sun::star::i18n;


/*
 * Members in the list:
 *   - String - the orig text
 *   - SwFmtRuby - the ruby attribut
 *
 *
 */
sal_uInt16 SwDoc::FillRubyList( const SwPaM& rPam, SwRubyList& rList,
							sal_uInt16 nMode )
{
	const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
				*__pStartCrsr = _pStartCrsr;
	sal_Bool bCheckEmpty = &rPam != _pStartCrsr;
	do {
		const SwPosition* pStt = _pStartCrsr->Start(),
				    	* pEnd = pStt == _pStartCrsr->GetPoint()
												? _pStartCrsr->GetMark()
												: _pStartCrsr->GetPoint();
		if( !bCheckEmpty || ( pStt != pEnd && *pStt != *pEnd ))
		{
			SwPaM aPam( *pStt );
			do {
				SwRubyListEntry* pNew = new SwRubyListEntry;
				if( pEnd != pStt )
				{
					aPam.SetMark();
					*aPam.GetMark() = *pEnd;
				}
				if( _SelectNextRubyChars( aPam, *pNew, nMode ))
				{
					rList.Insert( pNew, rList.Count() );
					aPam.DeleteMark();
				}
				else
				{
					delete pNew;
		 			if( *aPam.GetPoint() < *pEnd )
		 			{
						// goto next paragraph
						aPam.DeleteMark();
						aPam.Move( fnMoveForward, fnGoNode );
		 			}
		 			else
						break;
				}
			} while( 30 > rList.Count() && *aPam.GetPoint() < *pEnd );
		}
	} while( 30 > rList.Count() &&
		(_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

	return rList.Count();
}

sal_uInt16 SwDoc::SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
							sal_uInt16 nMode )
{
    GetIDocumentUndoRedo().StartUndo( UNDO_SETRUBYATTR, NULL );
	SvUShortsSort aDelArr;
	aDelArr.Insert( RES_TXTATR_CJK_RUBY );

	sal_uInt16 nListEntry = 0;

	const SwPaM *_pStartCrsr = (SwPaM*)rPam.GetNext(),
				*__pStartCrsr = _pStartCrsr;
	sal_Bool bCheckEmpty = &rPam != _pStartCrsr;
	do {
		const SwPosition* pStt = _pStartCrsr->Start(),
				    	* pEnd = pStt == _pStartCrsr->GetPoint()
												? _pStartCrsr->GetMark()
												: _pStartCrsr->GetPoint();
		if( !bCheckEmpty || ( pStt != pEnd && *pStt != *pEnd ))
		{

			SwPaM aPam( *pStt );
			do {
				SwRubyListEntry aCheckEntry;
				if( pEnd != pStt )
				{
					aPam.SetMark();
					*aPam.GetMark() = *pEnd;
				}
				if( _SelectNextRubyChars( aPam, aCheckEntry, nMode ))
				{
					const SwRubyListEntry* pEntry = rList[ nListEntry++ ];
					if( aCheckEntry.GetRubyAttr() != pEntry->GetRubyAttr() )
					{
						// set/reset the attribut
						if( pEntry->GetRubyAttr().GetText().Len() )
                        {
                            InsertPoolItem( aPam, pEntry->GetRubyAttr(), 0 );
                        }
                        else
                        {
                            ResetAttrs( aPam, sal_True, &aDelArr );
                        }
                    }

					if( aCheckEntry.GetText() != pEntry->GetText() &&
						pEntry->GetText().Len() )
					{
						// text is changed, so replace the original
                        ReplaceRange( aPam, pEntry->GetText(), false );
                    }
					aPam.DeleteMark();
				}
				else
				{
		 			if( *aPam.GetPoint() < *pEnd )
		 			{
						// goto next paragraph
						aPam.DeleteMark();
						aPam.Move( fnMoveForward, fnGoNode );
		 			}
		 			else
					{
						const SwRubyListEntry* pEntry = rList[ nListEntry++ ];

						// set/reset the attribut
						if( pEntry->GetRubyAttr().GetText().Len() &&
							pEntry->GetText().Len() )
						{
                            InsertString( aPam, pEntry->GetText() );
							aPam.SetMark();
							aPam.GetMark()->nContent -= pEntry->GetText().Len();
                            InsertPoolItem(
                                aPam, pEntry->GetRubyAttr(), nsSetAttrMode::SETATTR_DONTEXPAND );
						}
						else
							break;
						aPam.DeleteMark();
					}
				}
			} while( nListEntry < rList.Count() && *aPam.GetPoint() < *pEnd );
		}
	} while( 30 > rList.Count() &&
		(_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );

    GetIDocumentUndoRedo().EndUndo( UNDO_SETRUBYATTR, NULL );

	return nListEntry;
}

sal_Bool SwDoc::_SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rEntry, sal_uInt16 )
{
	// Point must be the startposition, Mark is optional the end position
	SwPosition* pPos = rPam.GetPoint();
   	const SwTxtNode* pTNd = pPos->nNode.GetNode().GetTxtNode();
	const String* pTxt = &pTNd->GetTxt();
	xub_StrLen nStart = pPos->nContent.GetIndex(), nEnd = pTxt->Len();

	sal_Bool bHasMark = rPam.HasMark();
	if( bHasMark )
	{
		// in the same node?
		if( rPam.GetMark()->nNode == pPos->nNode )
		{
			// then use that end
			xub_StrLen nTEnd = rPam.GetMark()->nContent.GetIndex();
			if( nTEnd < nEnd )
				nEnd = nTEnd;
		}
		rPam.DeleteMark();
	}

	// ----- search the start
	// --- look where a ruby attribut starts
	sal_uInt16 nHtIdx = USHRT_MAX;
	const SwpHints* pHts = pTNd->GetpSwpHints();
	const SwTxtAttr* pAttr = 0;
	if( pHts )
	{
		const SwTxtAttr* pHt;
		for( nHtIdx = 0; nHtIdx < pHts->Count(); ++nHtIdx )
			if( RES_TXTATR_CJK_RUBY == ( pHt = (*pHts)[ nHtIdx ])->Which() &&
				*pHt->GetAnyEnd() > nStart )
			{
				if( *pHt->GetStart() < nEnd )
				{
					pAttr = pHt;
					if( !bHasMark && nStart > *pAttr->GetStart() )
					{
						nStart = *pAttr->GetStart();
						pPos->nContent = nStart;
					}
				}
				break;
			}
	}

	if( !bHasMark && nStart && ( !pAttr || nStart != *pAttr->GetStart()) )
	{
		// skip to the word begin!
		long nWordStt = pBreakIt->GetBreakIter()->getWordBoundary(
							*pTxt, nStart,
							pBreakIt->GetLocale( pTNd->GetLang( nStart )),
							WordType::ANYWORD_IGNOREWHITESPACES,
							sal_True ).startPos;
		if( nWordStt < nStart && -1 != nWordStt )
		{
			nStart = (xub_StrLen)nWordStt;
			pPos->nContent = nStart;
		}
	}

	sal_Bool bAlphaNum = sal_False;
	long nWordEnd = nEnd;
	CharClass& rCC = GetAppCharClass();
	while(  nStart < nEnd )
	{
		if( pAttr && nStart == *pAttr->GetStart() )
		{
			pPos->nContent = nStart;
			if( !rPam.HasMark() )
			{
				rPam.SetMark();
				pPos->nContent = *pAttr->GetAnyEnd();
				if( pPos->nContent.GetIndex() > nEnd )
					pPos->nContent = nEnd;
				rEntry.SetRubyAttr( pAttr->GetRuby() );
			}
			break;
		}

		sal_Int32 nChType = rCC.getType( *pTxt, nStart );
		sal_Bool bIgnoreChar = sal_False, bIsAlphaNum = sal_False, bChkNxtWrd = sal_False;
		switch( nChType )
		{
		case UnicodeType::UPPERCASE_LETTER:
		case UnicodeType::LOWERCASE_LETTER:
		case UnicodeType::TITLECASE_LETTER:
		case UnicodeType::DECIMAL_DIGIT_NUMBER:
				bChkNxtWrd = bIsAlphaNum = sal_True;
				break;

		case UnicodeType::SPACE_SEPARATOR:
		case UnicodeType::CONTROL:
/*??*/	case UnicodeType::PRIVATE_USE:
		case UnicodeType::START_PUNCTUATION:
		case UnicodeType::END_PUNCTUATION:
			bIgnoreChar = sal_True;
			break;


		case UnicodeType::OTHER_LETTER:
			bChkNxtWrd = sal_True;
			// no break!
//		case UnicodeType::UNASSIGNED:
//		case UnicodeType::MODIFIER_LETTER:
//		case UnicodeType::NON_SPACING_MARK:
//		case UnicodeType::ENCLOSING_MARK:
//		case UnicodeType::COMBINING_SPACING_MARK:
//		case UnicodeType::LETTER_NUMBER:
//		case UnicodeType::OTHER_NUMBER:
//		case UnicodeType::LINE_SEPARATOR:
//		case UnicodeType::PARAGRAPH_SEPARATOR:
//		case UnicodeType::FORMAT:
//		case UnicodeType::SURROGATE:
//		case UnicodeType::DASH_PUNCTUATION:
//		case UnicodeType::CONNECTOR_PUNCTUATION:
///*?? */case UnicodeType::OTHER_PUNCTUATION:
//--> char '!' is to ignore!
//		case UnicodeType::MATH_SYMBOL:
//		case UnicodeType::CURRENCY_SYMBOL:
//		case UnicodeType::MODIFIER_SYMBOL:
//		case UnicodeType::OTHER_SYMBOL:
//		case UnicodeType::INITIAL_PUNCTUATION:
//		case UnicodeType::FINAL_PUNCTUATION:
		default:
				bIsAlphaNum = sal_False;
				break;
		}

		if( rPam.HasMark() )
		{
			if( bIgnoreChar || bIsAlphaNum != bAlphaNum || nStart >= nWordEnd )
				break;
		}
		else if( !bIgnoreChar )
		{
			rPam.SetMark();
			bAlphaNum = bIsAlphaNum;
			if( bChkNxtWrd && pBreakIt->GetBreakIter().is() )
			{
				// search the end of this word
				nWordEnd = pBreakIt->GetBreakIter()->getWordBoundary(
							*pTxt, nStart,
							pBreakIt->GetLocale( pTNd->GetLang( nStart )),
							WordType::ANYWORD_IGNOREWHITESPACES,
							sal_True ).endPos;
				if( 0 > nWordEnd || nWordEnd > nEnd || nWordEnd == nStart )
					nWordEnd = nEnd;
			}
		}
		pTNd->GoNext( &pPos->nContent, CRSR_SKIP_CHARS );
		nStart = pPos->nContent.GetIndex();
	}

	nStart = rPam.GetMark()->nContent.GetIndex();
	rEntry.SetText( pTxt->Copy( nStart,
						   rPam.GetPoint()->nContent.GetIndex() - nStart ));
	return rPam.HasMark();
}

SwRubyListEntry::~SwRubyListEntry()
{
}
