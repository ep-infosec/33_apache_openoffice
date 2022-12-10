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
#include <hintids.hxx>
#include <tools/stream.hxx>
#include <svtools/rtftoken.h>
#include <svtools/rtfkeywd.hxx>
#include <svl/intitem.hxx>
#include <svtools/rtfout.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <shellio.hxx>
#include <fltini.hxx>
#include <swtypes.hxx>
#include <swparrtf.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <charfmt.hxx>
#include <charatr.hxx>
#include <paratr.hxx>
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <numrule.hxx>

#define RTF_NUMRULE_NAME "RTF_Num"

static sal_Int16 lcl_GetStyleForNFC( sal_Int16 nNFC)
{
    static const sal_Int16 aMap[] =
    {
    style::NumberingType::ARABIC,                   // 0 - Arabic (1, 2, 3)
    style::NumberingType::ROMAN_UPPER,              // 1 - Uppercase Roman numeral (I, II, III)
    style::NumberingType::ROMAN_LOWER,              // 2 - Lowercase Roman numeral (i, ii, iii)
    style::NumberingType::CHARS_UPPER_LETTER_N,     // 3 - Uppercase letter (A, B, C)
    style::NumberingType::CHARS_LOWER_LETTER_N,     // 4 - Lowercase letter (a, b, c)
    style::NumberingType::ARABIC,                   // 5 - Ordinal number (1st, 2nd, 3rd)
    style::NumberingType::ARABIC,                   // 6 - Cardinal text number (One, Two Three)
    style::NumberingType::ARABIC,                   // 7 - Ordinal text number (First, Second, Third)
    style::NumberingType::NUMBER_NONE,              // 8 - Not defined
    style::NumberingType::NUMBER_NONE,              // 9 - Not defined
    style::NumberingType::NUMBER_LOWER_ZH,          // 10 - Kanji numbering without the digit character (*dbnum1).
    style::NumberingType::NUMBER_LOWER_ZH,          // 11 - Kanji numbering with the digit character (*dbnum2).
    style::NumberingType::AIU_HALFWIDTH_JA,         // 12 - phonetic Katakana characters in "aiueo" order (*aiueo).
    style::NumberingType::IROHA_HALFWIDTH_JA,       // 13 - phonetic katakana characters in "iroha" order (*iroha).
    style::NumberingType::FULLWIDTH_ARABIC,         // 14 - Double Byte character
    style::NumberingType::ARABIC,                   // 15 - Single Byte character
    style::NumberingType::NUMBER_TRADITIONAL_JA,    // 16 - Kanji numbering 3 (*dbnum3).
    style::NumberingType::ARABIC,                   // 17 - Kanji numbering 4 (*dbnum4).
    style::NumberingType::ARABIC,                   // 18 - Circle numbering (*circlenum). - decimalEnclosedCircleChinese
    style::NumberingType::FULLWIDTH_ARABIC,         // 19 - Double-byte Arabic numbering
    style::NumberingType::AIU_FULLWIDTH_JA,         // 20 - phonetic double-byte Katakana characters (*aiueo*dbchar).
    style::NumberingType::IROHA_FULLWIDTH_JA,       // 21 - phonetic double-byte katakana characters (*iroha*dbchar).
    style::NumberingType::ARABIC,                   // 22 - Arabic with leading zero (01, 02, 03, ..., 10, 11)
    style::NumberingType::CHAR_SPECIAL,             // 23 - Bullet (no number at all)
    style::NumberingType::HANGUL_SYLLABLE_KO,       // 24 - Korean numbering 2 (*ganada).
    style::NumberingType::HANGUL_JAMO_KO,           // 25 - Korean numbering 1 (*chosung).
    style::NumberingType::ARABIC,                   // 26 - Chinese numbering 1 (*gb1). - decimalEnclosedFullstop
    style::NumberingType::ARABIC,                   // 27 - Chinese numbering 2 (*gb2). - decimalEnclosedParen
    style::NumberingType::ARABIC,                   // 28 - Chinese numbering 3 (*gb3). - decimalEnclosedCircleChinese
    style::NumberingType::ARABIC,                   // 29 - Chinese numbering 4 (*gb4). - ideographEnclosedCircle
    style::NumberingType::TIAN_GAN_ZH,              // 30 - Chinese Zodiac numbering 1 (* zodiac1)
    style::NumberingType::DI_ZI_ZH,                 // 31 - Chinese Zodiac numbering 2 (* zodiac2)
    style::NumberingType::ARABIC,                   // 32 - Chinese Zodiac numbering 3 (* zodiac3)
    style::NumberingType::NUMBER_LOWER_ZH,          // 33 - Taiwanese double-byte numbering 1
    style::NumberingType::NUMBER_UPPER_ZH_TW,       // 34 - Taiwanese double-byte numbering 2
    style::NumberingType::NUMBER_LOWER_ZH,          // 35 - Taiwanese double-byte numbering 3
    style::NumberingType::ARABIC,                   // 36 - Taiwanese double-byte numbering 4
    style::NumberingType::NUMBER_LOWER_ZH,          // 37 - Chinese double-byte numbering 1
    style::NumberingType::NUMBER_UPPER_ZH,          // 38 - Chinese double-byte numbering 2
    style::NumberingType::NUMBER_LOWER_ZH,          // 39 - Chinese double-byte numbering 3
    style::NumberingType::ARABIC,                   // 40 - Chinese double-byte numbering 4
    style::NumberingType::NUMBER_HANGUL_KO,         // 41 - Korean double-byte numbering 1
    style::NumberingType::NUMBER_HANGUL_KO,         // 42 - Korean double-byte numbering 2
    style::NumberingType::NUMBER_HANGUL_KO,         // 43 - Korean double-byte numbering 3
    style::NumberingType::NUMBER_LOWER_ZH,          // 44 - Korean double-byte numbering 4
    style::NumberingType::CHARS_HEBREW,             // 45 - Hebrew non-standard decimal
    style::NumberingType::CHARS_ARABIC,             // 46 - Arabic Alif Ba Tah
    style::NumberingType::CHARS_HEBREW,             // 47 - Hebrew Biblical standard
    style::NumberingType::ARABIC                    // 48 - Arabic Abjad style

    };
    const int nLen = sizeof(aMap)/sizeof(aMap[0]);
    sal_Int16 nRet = style::NumberingType::NUMBER_NONE;
    if (nNFC>=0 && nNFC<nLen)
        nRet = aMap[nNFC];
    else if (nNFC==255) 
        nRet = style::NumberingType::CHAR_SPECIAL;
    return nRet;
}

void lcl_ExpandNumFmts( SwNumRule& rRule )
{
	// dann noch das NumFormat in alle Ebenen setzen
	for( sal_uInt8 n = 1; n < MAXLEVEL; ++n )
		if( !rRule.GetNumFmt( n ) )
		{
			SwNumFmt aNumFmt( rRule.Get( 0 ));
			aNumFmt.SetAbsLSpace( aNumFmt.GetAbsLSpace() * ( n + 1 ) );
			rRule.Set( n, aNumFmt );
		}
}

SfxItemSet& GetNumChrFmt( SwDoc& rDoc, SwNumRule& rRule, sal_uInt8 nNumLvl )
{
	SwCharFmt* pChrFmt = rRule.Get( nNumLvl ).GetCharFmt();
	if( !pChrFmt )
	{
		String sNm( rRule.GetName() );
		( sNm += ' ' ) += String::CreateFromInt32( nNumLvl + 1 );
		pChrFmt = rDoc.MakeCharFmt( sNm, rDoc.GetDfltCharFmt() );
		if( !rRule.GetNumFmt( nNumLvl ))
			rRule.Set( nNumLvl, rRule.Get( nNumLvl ) );
		((SwNumFmt*)rRule.GetNumFmt( nNumLvl ))->SetCharFmt( pChrFmt );
	}
	return (SfxItemSet&)pChrFmt->GetAttrSet();
}

void SwRTFParser::ReadListLevel( SwNumRule& rRule, sal_uInt8 nNumLvl )
{
	int nToken;
	int nNumOpenBrakets = 1;		// die erste wurde schon vorher erkannt !!
	int nLvlTxtLevel = 0, nLvlNumberLevel = 0;
	String sLvlText, sLvlNumber;
	SwNumFmt* pCurNumFmt;
	String aStringFollow = aEmptyStr;

	if( MAXLEVEL >= nNumLvl )
	{
		pCurNumFmt = (SwNumFmt*)rRule.GetNumFmt( nNumLvl );
		pCurNumFmt->SetAbsLSpace( 0 );
		pCurNumFmt->SetFirstLineOffset( 0 );
	}
	else
		pCurNumFmt = 0;

	while( nNumOpenBrakets && IsParserWorking() )
	{
		switch( ( nToken = GetNextToken() ))
		{
		case '}':
			if( nNumOpenBrakets )
			{
				if( nLvlTxtLevel == nNumOpenBrakets )
				{
					if( DelCharAtEnd( sLvlText, ';' ).Len() &&
						sLvlText.Len() && sLvlText.Len() ==
						(sal_uInt16)(sLvlText.GetChar( 0 )) + 1 )
						sLvlText.Erase( 0, 1 );
					nLvlTxtLevel = 0;
				}
				if( nLvlNumberLevel == nNumOpenBrakets )
				{
					DelCharAtEnd( sLvlNumber, ';' );
					nLvlNumberLevel = 0;
				}
			}
			--nNumOpenBrakets;
			break;

		case '{':
			{
				if( RTF_IGNOREFLAG != GetNextToken() )
					nToken = SkipToken( -1 );
				// Unknown und alle bekannten nicht ausgewerteten Gruppen
				// sofort ueberspringen
				else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
//						RTF_PANOSE != nToken && RTF_FALT != nToken &&
//						RTF_FALT != nToken && RTF_FNAME != nToken &&
//						RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
					nToken = SkipToken( -2 );
				else
				{
					// gleich herausfiltern
					ReadUnknownData();
					nToken = GetNextToken();
					if( '}' != nToken )
						eState = SVPAR_ERROR;
					break;
				}
				++nNumOpenBrakets;
			}
			break;

		case RTF_LEVELNFC:
			{
				sal_Int16 eType = lcl_GetStyleForNFC( nTokenValue ) ; 
                
				if( pCurNumFmt )
					pCurNumFmt->SetNumberingType(eType);
			}
			break;

		case RTF_LEVELJC:
			{
				SvxAdjust eAdj = SVX_ADJUST_LEFT;
				switch( nTokenValue )
				{
				case 1:		eAdj = SVX_ADJUST_CENTER;	break;
				case 2:		eAdj = SVX_ADJUST_RIGHT;	break;
				}
				if( pCurNumFmt )
					pCurNumFmt->SetNumAdjust( eAdj );
			}
			break;

		case RTF_LEVELSTARTAT:
			if( pCurNumFmt && -1 != nTokenValue )
				pCurNumFmt->SetStart( sal_uInt16( nTokenValue ));
			break;

		case RTF_LEVELTEXT:
			nLvlTxtLevel = nNumOpenBrakets;
			break;

		case RTF_LEVELNUMBERS:
			nLvlNumberLevel = nNumOpenBrakets;
			break;


		case RTF_TEXTTOKEN:
			if( nLvlTxtLevel == nNumOpenBrakets )
				sLvlText += aToken;
			else if( nLvlNumberLevel == nNumOpenBrakets )
				sLvlNumber += aToken;
			break;

		case RTF_LEVELFOLLOW:
		  /* removed; waiting for swnum02 to be integrated!
			switch (nTokenValue)
			{
			case 0:
				aStringFollow=String('\t');
				break;
			case 1:
				aStringFollow=String(' ');
				break;
			}
			*/
			break;

		case RTF_LEVELOLD:
		case RTF_LEVELPREV:
		case RTF_LEVELPREVSPACE:
		case RTF_LEVELINDENT:
		case RTF_LEVELSPACE:
		case RTF_LEVELLEGAL:
		case RTF_LEVELNORESTART:
			break;

		default:
			if( pCurNumFmt && (
				RTF_CHRFMT == (nToken & ~(0xff | RTF_SWGDEFS) ) ||
				RTF_PARFMT == (nToken & ~(0xff | RTF_SWGDEFS) ) ))
			{
				SfxItemSet aSet( pDoc->GetAttrPool(), aTxtNodeSetRange );
				// put the current CharFmtAttr into the set
				SfxItemSet& rCFmtSet = GetNumChrFmt( *pDoc, rRule, nNumLvl );
				aSet.Put( rCFmtSet );
				// and put the current "LRSpace" into the set
				{
                    SvxLRSpaceItem aLR( RES_LR_SPACE );
					aLR.SetTxtLeft( pCurNumFmt->GetAbsLSpace() );
					aLR.SetTxtFirstLineOfst(pCurNumFmt->GetFirstLineOffset());
					aSet.Put( aLR );
				}

				ReadAttr( nToken, &aSet );

                //#i24880# Word appears to ignore char background for numbering
                aSet.ClearItem(RES_CHRATR_BACKGROUND);

				// put all CharFmt Items into the charfmt
				rCFmtSet.Put( aSet );

				// test for LRSpace Item. If exist then set all relevant
				// values on the numrule format
				const SfxPoolItem* pItem;
				if( SFX_ITEM_SET == aSet.GetItemState( RES_LR_SPACE,
						sal_False, &pItem ))
				{
					const SvxLRSpaceItem& rLR = *(SvxLRSpaceItem*)pItem;
                    pCurNumFmt->SetAbsLSpace( static_cast< short >(rLR.GetTxtLeft()) );
					pCurNumFmt->SetFirstLineOffset( rLR.GetTxtFirstLineOfst());
				}

				// dann aus der Vorlage den Font holen
				if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
					pCurNumFmt->SetBulletFont( FindFontOfItem(
								pCurNumFmt->GetCharFmt()->GetFont() ) );
			}
			break;
		}
	}

	if( IsParserWorking() && pCurNumFmt )
	{
		// dann erzeuge mal die Pre/Postfix-Strings
		if( sLvlText.Len() &&
			SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
		{
			pCurNumFmt->SetBulletChar( sLvlText.GetChar( 0 ) );
			// dann aus der Vorlage den Font holen
			if( pCurNumFmt->GetCharFmt() )
				pCurNumFmt->SetBulletFont( FindFontOfItem(
						pCurNumFmt->GetCharFmt()->GetFont() ) );
		}
		else if( sLvlNumber.Len() && sLvlText.Len() )
		{
			// in sLvlText steht der Text, in sLvlNumber die Position
			// der Ebenen in sLvlText
			pCurNumFmt->SetPrefix(
				sLvlText.Copy( 0, sal_uInt16( sLvlNumber.GetChar( 0 ))-1 ));
			pCurNumFmt->SetSuffix( sLvlText.Copy(
					sal_uInt16( sLvlNumber.GetChar( sLvlNumber.Len()-1 )) ));
			// wieviele Levels stehen im String?
			pCurNumFmt->SetIncludeUpperLevels( (sal_uInt8)sLvlNumber.Len() );
		}
		else
		{
			pCurNumFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
			pCurNumFmt->SetSuffix( sLvlText );
		}

		String newSuffix=pCurNumFmt->GetSuffix();
		newSuffix+=aStringFollow;
		pCurNumFmt->SetSuffix(newSuffix);
		/* removed; waiting for swnum02 to be integrated!
		if (aStringFollow.GetChar(0)=='\t' && !pCurNumFmt->IsItemize())
		{
			pCurNumFmt->SetAbsLSpace(0);
			pCurNumFmt->SetFirstLineOffset(0);
		}
		*/
	}

	SkipToken( -1 );
}

void SwRTFParser::ReadListTable()
{
	int nToken;
	int nNumOpenBrakets = 1;		// die erste wurde schon vorher erkannt !!
	bNewNumList = sal_True;

	sal_uInt8 nNumLvl = 0;
	SwNumRule* pCurRule = 0;
	SwListEntry aEntry;

	while( nNumOpenBrakets && IsParserWorking() )
	{
		switch( ( nToken = GetNextToken() ))
		{
		case '}':		if( --nNumOpenBrakets && IsParserWorking() )
						{
							// Style konnte vollstaendig gelesen werden,
							// also ist das noch ein stabiler Status
							SaveState( RTF_LISTTABLE );
							if( 1 == nNumOpenBrakets )
							{
								if( aEntry.nListId )
									aListArr.push_back( aEntry );
								aEntry.Clear();
							}
						}
						break;

		case '{':
			{
				if( RTF_IGNOREFLAG != GetNextToken() )
					nToken = SkipToken( -1 );
				// Unknown und alle bekannten nicht ausgewerteten Gruppen
				// sofort ueberspringen
				else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
//						RTF_PANOSE != nToken && RTF_FALT != nToken &&
//						RTF_FALT != nToken && RTF_FNAME != nToken &&
//						RTF_FONTEMB != nToken && RTF_FONTFILE != nToken )
					nToken = SkipToken( -2 );
				else
				{
					// gleich herausfiltern
					ReadUnknownData();
					nToken = GetNextToken();
					if( '}' != nToken )
						eState = SVPAR_ERROR;
					break;
				}
				++nNumOpenBrakets;
			}
			break;

		case RTF_LIST:
			{
				if( pCurRule && pCurRule->IsContinusNum() )
					lcl_ExpandNumFmts( *pCurRule );

				String sTmp( String::CreateFromAscii(
					RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
				aEntry.nListDocPos = pDoc->MakeNumRule( sTmp );
				pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                // --> OD 2008-07-08 #i91400#
                pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ),
                                   *pDoc );
                // <--
				pCurRule->SetAutoRule( sal_False );
				nNumLvl = (sal_uInt8)-1;
			}
			break;

		case RTF_LISTID:			aEntry.nListId = nTokenValue;		break;
		case RTF_LISTTEMPLATEID:	aEntry.nListTemplateId = nTokenValue; break;

		case RTF_LISTRESTARTHDN:
			break;
		case RTF_LISTNAME:
			if (nNextCh=='}') break;  // #118989# empty listnames
			if( RTF_TEXTTOKEN == GetNextToken() )
			{
				String sTmp( DelCharAtEnd( aToken, ';' ));
				if( sTmp.Len() && !pDoc->FindNumRulePtr( sTmp ))
                {
                    // --> OD 2008-07-08 #i91400#
                    pCurRule->SetName( sTmp, *pDoc );
                    // <--
                }
			}
			SkipGroup();
			break;

		case RTF_LISTSIMPLE:
			pCurRule->SetContinusNum( sal_True );
			break;

		case RTF_LISTLEVEL:
			{
				if( ++nNumLvl < MAXLEVEL )
					pCurRule->Set( nNumLvl, pCurRule->Get( nNumLvl ));
				ReadListLevel( *pCurRule, nNumLvl );
			}
			break;
		}
	}

	if( pCurRule && pCurRule->IsContinusNum() )
		lcl_ExpandNumFmts( *pCurRule );

	SkipToken( -1 );		// die schliesende Klammer wird "oben" ausgewertet
}

sal_Bool lcl_IsEqual( SwNumRule* pOrigRule, SwNumRule* pRule )
{
	sal_Bool bRet = 0;
	if( pOrigRule && pRule )
	{
		bRet =  pOrigRule->GetRuleType() == pRule->GetRuleType() &&
				pOrigRule->IsContinusNum() == pRule->IsContinusNum() &&
				pOrigRule->IsAbsSpaces() == pRule->IsAbsSpaces();
		if( bRet )
			for( sal_uInt8 n = 0; bRet && n < MAXLEVEL; ++n )
			{
				const SwNumFmt* pOFmt = pOrigRule->GetNumFmt( n ),
							  *	pFmt = pRule->GetNumFmt( n );
				if( pFmt && pOFmt )
				{
					SwCharFmt* pOCFmt = pOFmt->GetCharFmt(),
							 * pCFmt = pFmt->GetCharFmt();
					if( pOCFmt && pCFmt )
					{
                        bRet = 0 != (pCFmt->GetAttrSet() == pOCFmt->GetAttrSet());
					}
					else
						bRet = !pCFmt && !pOCFmt;
					if( bRet )
					{
						((SwNumFmt*)pOFmt)->SetCharFmt( 0 );
						((SwNumFmt*)pFmt)->SetCharFmt( 0 );
						bRet = *pOFmt == *pFmt;
						((SwNumFmt*)pOFmt)->SetCharFmt( pOCFmt );
						((SwNumFmt*)pFmt)->SetCharFmt( pCFmt );
					}
				}
				else
					bRet = !pFmt && !pOFmt;
			}
	}
	return bRet;
}

void SwRTFParser::ReadListOverrideTable()
{
	int nToken;
	int nNumOpenBrakets = 1;		// die erste wurde schon vorher erkannt !!
	SwListEntry aEntry;
	SwNumRule* pRule = 0, *pOrigRule = 0;
	sal_uInt8 nNumLvl = 0;
	sal_Bool bOverrideFormat = sal_False, bOverrideStart = sal_False;

	while( nNumOpenBrakets && IsParserWorking() )
	{
		switch( ( nToken = GetNextToken() ))
		{
		case '}':
			if( --nNumOpenBrakets && IsParserWorking() )
			{
				// Style konnte vollstaendig gelesen werden,
				// also ist das noch ein stabiler Status
				SaveState( RTF_LISTOVERRIDETABLE );

				if( 1 == nNumOpenBrakets )
				{
					bOverrideFormat = sal_False, bOverrideStart = sal_False;
					if( pRule )
					{
						if( lcl_IsEqual( pOrigRule, pRule ))
						{
							// no changes on the rule -> use the original rule
							aEntry.nListDocPos = pDoc->FindNumRule(
													pOrigRule->GetName() );
							// delete the temp Rule
							RemoveUnusedNumRule( pRule );
						}
						else if( pRule->IsContinusNum() )
							lcl_ExpandNumFmts( *pRule );
					}

					if( aEntry.nListId && aEntry.nListNo )
					{
						int nMatch=-1;
						for( size_t n = aListArr.size(); n; )
						{
							if( aListArr[ --n ].nListId == aEntry.nListId)
							{
								nMatch=n;
								break;
							}
						}
						if(nMatch>=0)
						{
                            sal_uInt16 nMatch2 = static_cast< sal_uInt16 >(nMatch);
                            if (!aListArr[nMatch2].nListNo )
							{
                                aListArr[nMatch2].nListNo = aEntry.nListNo;
							}
							else
							{
                                aEntry.nListDocPos=aListArr[nMatch2].nListDocPos;
                                aEntry.nListTemplateId=aListArr[nMatch2].nListTemplateId;
								aListArr.push_back( aEntry );
							}
							if(pOrigRule)
                                aListArr[nMatch2].nListDocPos = aEntry.nListDocPos;
						}
					}
					aEntry.Clear();
					pOrigRule = 0;
					pRule = 0;
				}
			}
			break;

		case '{':
			{
				if( RTF_IGNOREFLAG != GetNextToken() )
					nToken = SkipToken( -1 );
				// Unknown und alle bekannten nicht ausgewerteten Gruppen
				// sofort ueberspringen
				else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
					nToken = SkipToken( -2 );
				else
				{
					// gleich herausfiltern
					ReadUnknownData();
					nToken = GetNextToken();
					if( '}' != nToken )
						eState = SVPAR_ERROR;
					break;
				}
				++nNumOpenBrakets;
			}
			break;

		case RTF_LISTOVERRIDE:		aEntry.Clear();					break;
		case RTF_LISTID:			aEntry.nListId = nTokenValue;	break;
		case RTF_LS:				aEntry.nListNo = nTokenValue;	break;
		case RTF_LISTOVERRIDECOUNT:
			if( nTokenValue )
			{
				pRule = 0;
				// dann erzeugen wir mal schnell eine Kopie von der NumRule,
				// denn diese wird jetzt mal kurz veraendert.
				if( aEntry.nListId )
					for( size_t n = 0; n < aListArr.size(); ++n )
						if( aListArr[ n ].nListId == aEntry.nListId )
						{
							pRule = pDoc->GetNumRuleTbl()[
												aListArr[ n ].nListDocPos ];
							pOrigRule = pRule;

							String sTmp( String::CreateFromAscii(
								RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
							aEntry.nListDocPos = pDoc->MakeNumRule( sTmp, pRule );
							pRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
                            // --> OD 2008-07-08 #i91400#
                            pRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ),
                                            *pDoc );
                            // <--
							pRule->SetAutoRule( sal_False );
							nNumLvl = (sal_uInt8)-1;
							aListArr.push_back( aEntry );
							break;
						}

			}
			break;

		case RTF_LISTLEVEL:
			if( pRule && bOverrideFormat )
			{
				if( ++nNumLvl < MAXLEVEL )
					pRule->Set( nNumLvl, pRule->Get( nNumLvl ));
				ReadListLevel( *pRule, nNumLvl );
			}
			break;

		case RTF_LEVELSTARTAT:
			if( pRule && bOverrideStart )
			{
			}
			break;

		case RTF_LISTOVERRIDESTART:
			bOverrideStart = sal_True;
			break;

		case RTF_LISTOVERRIDEFORMAT:
			bOverrideFormat = sal_True;
			break;

		case RTF_LFOLEVEL:
			// was fehlt noch?
			break;
		}
	}

	// search the outline numrule and set it into the doc
	if( GetStyleTbl().Count() )
	{
		if( !bStyleTabValid )
			MakeStyleTab();

        const SfxPoolItem* pItem( 0 );
        const SwTxtFmtColl* pColl( 0 );
        sal_uInt16 nRulePos( USHRT_MAX );
		const SwNumRule *pNumRule = 0;
        SvxRTFStyleType* pStyle = GetStyleTbl().First();
		do {
            // --> OD 2007-12-17 #151213#
            // suppress deletion of outline list style.
            // refactoring of code: no assignments in if-condition
//            if( MAXLEVEL > pStyle->nOutlineNo &&
//                0 != ( pColl = aTxtCollTbl.Get( (sal_uInt16)GetStyleTbl().
//                                                        GetCurKey() )) &&
//                SFX_ITEM_SET == pColl->GetItemState( RES_PARATR_NUMRULE,
//                                                    sal_False, &pItem ) &&
//                USHRT_MAX != (nRulePos = pDoc->FindNumRule(
//                                ((SwNumRuleItem*)pItem)->GetValue() )) &&
//                (pNumRule = pDoc->GetNumRuleTbl()[ nRulePos ])->IsAutoRule() )
            if ( MAXLEVEL > pStyle->nOutlineNo )
            {
                pColl = aTxtCollTbl.Get( (sal_uInt16)GetStyleTbl().GetCurKey() );
                if ( pColl )
                {
                    const SfxItemState eItemState =
                        pColl->GetItemState( RES_PARATR_NUMRULE, sal_False, &pItem );
                    if ( eItemState == SFX_ITEM_SET )
                    {
                        nRulePos = pDoc->FindNumRule( ((SwNumRuleItem*)pItem)->GetValue() );
                        if ( nRulePos != USHRT_MAX )
                        {
                            pNumRule = pDoc->GetNumRuleTbl()[ nRulePos ];
                            if ( pNumRule->IsAutoRule() &&
                                 pNumRule != pDoc->GetOutlineNumRule() )
                            {
                                pDoc->SetOutlineNumRule( *pNumRule );
                                pDoc->DelNumRule( pNumRule->GetName() );
                                // now pNumRule pointer is invalid !!!

                                // now decrement all position in the listtable, which will
                                // behind the doc-rule position
                                for( size_t n = aListArr.size(); n; )
                                {
                                    SwListEntry& rEntry = aListArr[ --n ];
                                    if( rEntry.nListDocPos == nRulePos )
                                        aListArr.erase( aListArr.begin()+n );
                                    else if( rEntry.nListDocPos > nRulePos )
                                        --rEntry.nListDocPos;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            // <--

			pStyle->aAttrSet.ClearItem( FN_PARAM_NUM_LEVEL );

		} while( 0 != (pStyle = GetStyleTbl().Next()) );
	}

	SkipToken( -1 );		// die schliesende Klammer wird "oben" ausgewertet
}

SwNumRule* SwRTFParser::GetNumRuleOfListNo( long nListNo, sal_Bool bRemoveFromList )
{
	SwNumRule* pRet = 0;
	SwListEntry* pEntry;
	for( size_t n = aListArr.size(); n; )
		if( ( pEntry = &aListArr[ --n ])->nListNo == nListNo )
		{
			if( bRemoveFromList )
				aListArr.erase( aListArr.begin()+n );
			else
			{
				pEntry->bRuleUsed = sal_True;
				pRet = pDoc->GetNumRuleTbl()[ pEntry->nListDocPos ];
			}
			break;
		}
	return pRet;
}

void SwRTFParser::RemoveUnusedNumRule( SwNumRule* pRule )
{
	if( pRule )
	{
		for ( sal_uInt8 nLvl = 0; nLvl < MAXLEVEL; ++nLvl )
		{
			SwNumFmt& rNFmt = (SwNumFmt&)pRule->Get( nLvl );
			SwCharFmt* pCFmt = rNFmt.GetCharFmt();
			if( pCFmt )
			{
                rNFmt.ForgetCharFmt();
				if( !pCFmt->GetDepends() )
					pDoc->DelCharFmt( pCFmt );
			}
		}
		pDoc->DelNumRule( pRule->GetName() );
	}
#ifdef DBG_UTIL
	else
	{
		ASSERT( pRule, "NumRulePointer 0 kann nicht geloescht werden" );
	}
#endif
}

void SwRTFParser::RemoveUnusedNumRules()
{
	SwListEntry* pEntry;
	SvPtrarr aDelArr;
	size_t n;
	for( n = aListArr.size(); n; )
	{
		if( !( pEntry = &aListArr[ --n ])->bRuleUsed )
		{
			// really *NOT* used by anyone else?
			sal_Bool unused=sal_True;
            for(size_t j = 0;  j < aListArr.size();  ++j)
			{
				if (aListArr[n].nListNo==aListArr[j].nListNo)
					unused&=!aListArr[j].bRuleUsed;
			}
			if (unused)
			{
				void * p = pDoc->GetNumRuleTbl()[pEntry->nListDocPos];
				// dont delete named char formats
				if( USHRT_MAX == aDelArr.GetPos( p ) &&
					((SwNumRule*)p)->GetName().EqualsAscii( RTF_NUMRULE_NAME, 0,
									sizeof( RTF_NUMRULE_NAME )) )
					aDelArr.Insert( p, aDelArr.Count() );
			}
		}
	}

	for( n = aDelArr.Count(); n; )
	{
		SwNumRule* pDel = (SwNumRule*)aDelArr[ --n ];
		RemoveUnusedNumRule( pDel );
	}
}

const Font* SwRTFParser::FindFontOfItem( const SvxFontItem& rItem ) const
{
	SvxRTFFontTbl& rFntTbl = ((SwRTFParser*)this)->GetFontTbl();
	const Font* pFnt = rFntTbl.First();
	while( pFnt )
	{
		if( pFnt->GetFamily() == rItem.GetFamily() &&
			pFnt->GetName() == rItem.GetFamilyName() &&
			pFnt->GetStyleName() == rItem.GetStyleName() &&
			pFnt->GetPitch() == rItem.GetPitch() &&
			pFnt->GetCharSet() == rItem.GetCharSet() )
			return pFnt;

		pFnt = rFntTbl.Next();
	}
	return 0;
}


SwNumRule *SwRTFParser::ReadNumSecLevel( int nToken )
{
	// lese die \pnseclvl - Gruppe
	// nTokenValue gibt schon den richtigen Level vor 1 - 9!
	sal_uInt8 nLevel = 0;
	long nListNo = 0;
	sal_Bool bContinus = sal_True;

	if( RTF_PNSECLVL == nToken )
	{
		// suche die Rule - steht unter Nummer 3
		nListNo = 3;
		bContinus = sal_False;
		nLevel = MAXLEVEL <= (unsigned long) nTokenValue ? MAXLEVEL - 1
			: (!nTokenValue ? 0 : sal_uInt8( nTokenValue - 1 ));
	}
	else
	{
		switch( nToken = GetNextToken() )
		{
		case RTF_PNLVL:			nListNo = 3;
								bContinus = sal_False;
								nLevel = MAXLEVEL <= (unsigned long) nTokenValue
													? MAXLEVEL - 1
                                    : (!nTokenValue ? 0 : sal_uInt8( nTokenValue-1 ));
								break;

		case RTF_PNLVLBODY:
            nListNo = 2;
            break;
		case RTF_PNLVLBLT:
            nListNo = 1;
            break;
		case RTF_PNLVLCONT:
    		SkipGroup();
			return 0;
		default:
			SkipGroup();
			return 0;
		}
	}

	// suche die Rule - steht unter Nummer 3
    sal_uInt16 nNewFlag = static_cast< sal_uInt16 >(1 << nListNo);
	SwNumRule* pCurRule = GetNumRuleOfListNo( nListNo,
										0 != ( nNewNumSectDef & nNewFlag ) );
	if( !pCurRule )
	{
		// dann muessen wir die mal anlegen
		nNewNumSectDef &= ~nNewFlag;
		String sTmp( String::CreateFromAscii(
						RTL_CONSTASCII_STRINGPARAM( RTF_NUMRULE_NAME " 1" )));
		SwListEntry aEntry( nListNo, 0, pDoc->MakeNumRule( sTmp ));
		aEntry.nListNo = nListNo;
		aListArr.push_back( aEntry );
		pCurRule = pDoc->GetNumRuleTbl()[ aEntry.nListDocPos ];
        // --> OD 2008-07-08 #i91400#
        pCurRule->SetName( pDoc->GetUniqueNumRuleName( &sTmp, sal_False ), *pDoc );
        // <--
		pCurRule->SetAutoRule( sal_False );
		pCurRule->SetContinusNum( bContinus );
	}

	if( !pCurRule->GetNumFmt( nLevel ))
		pCurRule->Set( nLevel, pCurRule->Get( nLevel ));
	SwNumFmt* pCurNumFmt = (SwNumFmt*)pCurRule->GetNumFmt( nLevel );
	if( RTF_PNLVLBLT == nToken )
		pCurNumFmt->SetNumberingType(SVX_NUM_CHAR_SPECIAL);
	pCurNumFmt->SetSuffix( aEmptyStr );
	pCurNumFmt->SetPrefix( aEmptyStr );
	pCurNumFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);

	if( bStyleTabValid && RTF_PNSECLVL != nToken )
	{
		// dann den akt. Lvl und Rule am Absatz setzen.
		// Dieses muss aber in den vorherigen "Kontext", sprich in den vor
		// der Klammer offenen Attrset. Darum das SetNewGroup davor und dahinter
		SetNewGroup( sal_False );
		GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
		GetAttrSet().Put( SwNumRuleItem( pCurRule->GetName() ));
		SetNewGroup( sal_True );
	}

	FontUnderline eUnderline;
	int nNumOpenBrakets = 1;		// die erste wurde schon vorher erkannt !!
	while( nNumOpenBrakets && IsParserWorking() )
	{
		switch( ( nToken = GetNextToken() ))
		{
		case '}':
			if( --nNumOpenBrakets && IsParserWorking() )
			{
				// Style konnte vollstaendig gelesen werden,
				// also ist das noch ein stabiler Status
				SaveState( RTF_PNSECLVL );
			}
			break;

		case '{':
			{
				if( RTF_IGNOREFLAG != GetNextToken() )
					nToken = SkipToken( -1 );
				// Unknown und alle bekannten nicht ausgewerteten Gruppen
				// sofort ueberspringen
				else if( RTF_UNKNOWNCONTROL != ( nToken = GetNextToken() ))
					nToken = SkipToken( -2 );
				else
				{
					// gleich herausfiltern
					ReadUnknownData();
					nToken = GetNextToken();
					if( '}' != nToken )
						eState = SVPAR_ERROR;
					break;
				}
				++nNumOpenBrakets;
			}
			break;

		case RTF_PNCARD:
		case RTF_PNORD:
		case RTF_PNORDT:
		case RTF_PNDEC: 	pCurNumFmt->SetNumberingType(SVX_NUM_ARABIC);				break;
		case RTF_PNUCLTR:	pCurNumFmt->SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N);	break;
		case RTF_PNUCRM:    pCurNumFmt->SetNumberingType(SVX_NUM_ROMAN_UPPER);			break;
		case RTF_PNLCLTR:   pCurNumFmt->SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N);	break;
		case RTF_PNLCRM:    pCurNumFmt->SetNumberingType(SVX_NUM_ROMAN_LOWER);			break;

		case RTF_PNF:
			{
				const Font& rSVFont = GetFont( sal_uInt16(nTokenValue) );
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
							SvxFontItem( rSVFont.GetFamily(),
								rSVFont.GetName(), rSVFont.GetStyleName(),
                                rSVFont.GetPitch(), rSVFont.GetCharSet(),
                                            RES_CHRATR_FONT ));
				if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
					pCurNumFmt->SetBulletFont( &rSVFont );
			}
			break;
		case RTF_PNFS:
			{
				if( -1 == nTokenValue )
					nTokenValue = 240;
				else
					nTokenValue *= 10;
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                            SvxFontHeightItem( (const sal_uInt16)nTokenValue, 100, RES_CHRATR_FONTSIZE ));
			}
			break;

		case RTF_PNB:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxWeightItem(
                                nTokenValue ? WEIGHT_BOLD : WEIGHT_NORMAL, RES_CHRATR_WEIGHT ));
			}
			break;

		case RTF_PNI:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxPostureItem(
                            nTokenValue ? ITALIC_NORMAL : ITALIC_NONE, RES_CHRATR_POSTURE ));
			}
			break;

		case RTF_PNCAPS:
		case RTF_PNSCAPS:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCaseMapItem(
								nTokenValue ? SVX_CASEMAP_KAPITAELCHEN
                                            : SVX_CASEMAP_NOT_MAPPED, RES_CHRATR_CASEMAP ));
			}
			break;
		case RTF_PNSTRIKE:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxCrossedOutItem(
                        nTokenValue ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ));
			}
			break;

		case RTF_PNCF:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put( SvxColorItem(
                            GetColor( sal_uInt16(nTokenValue) ), RES_CHRATR_COLOR ));
			}
			break;


		case RTF_PNUL:
			eUnderline = nTokenValue ? UNDERLINE_SINGLE : UNDERLINE_NONE;
			goto NUMATTR_SETUNDERLINE;
		case RTF_PNULD:
			eUnderline = UNDERLINE_DOTTED;
			goto NUMATTR_SETUNDERLINE;
		case RTF_PNULDB:
			eUnderline = UNDERLINE_DOUBLE;
			goto NUMATTR_SETUNDERLINE;
		case RTF_PNULNONE:
			eUnderline = UNDERLINE_NONE;
			goto NUMATTR_SETUNDERLINE;
		case RTF_PNULW:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                                    SvxWordLineModeItem( sal_True, RES_CHRATR_WORDLINEMODE ));
			}
			eUnderline = UNDERLINE_SINGLE;
			goto NUMATTR_SETUNDERLINE;

NUMATTR_SETUNDERLINE:
			{
				GetNumChrFmt( *pDoc, *pCurRule, nLevel ).Put(
                        SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ));
			}
			break;

		case RTF_PNINDENT:
			if( 0 > short( nTokenValue ) )
				nTokenValue = - (short)nTokenValue;
			pCurNumFmt->SetFirstLineOffset( - short( nTokenValue ));
			pCurNumFmt->SetAbsLSpace( (nLevel + 1 ) * sal_uInt16( nTokenValue ));
			break;
		case RTF_PNSP:
			pCurNumFmt->SetCharTextDistance( sal_uInt16( nTokenValue ));
			break;

		case RTF_PNPREV:
			if( nLevel )
			{
				sal_uInt8 nPrev = 2, nLast = nLevel;
				while( nLast && 1 < pCurRule->Get( --nLast ).GetIncludeUpperLevels() )
					++nPrev;
				pCurNumFmt->SetIncludeUpperLevels( nPrev );
			}
			break;

		case RTF_PNQC:	pCurNumFmt->SetNumAdjust( SVX_ADJUST_CENTER ); 	break;
		case RTF_PNQL:	pCurNumFmt->SetNumAdjust( SVX_ADJUST_LEFT ); 		break;
		case RTF_PNQR:	pCurNumFmt->SetNumAdjust( SVX_ADJUST_RIGHT );		break;

		case RTF_PNSTART:
			pCurNumFmt->SetStart( sal_uInt16( nTokenValue ));
			break;

		case RTF_PNNUMONCE:
		case RTF_PNACROSS:
		case RTF_PNHANG:
		case RTF_PNRESTART:		break;

		case RTF_PNTXTA:
			{
				String sTmp;
				GetTextToEndGroup( sTmp );
				if( SVX_NUM_CHAR_SPECIAL == pCurNumFmt->GetNumberingType() )
				{
					pCurNumFmt->SetBulletChar( sTmp.GetChar( 0 ) );
					if( pCurNumFmt->GetCharFmt() )
						pCurNumFmt->SetBulletFont( FindFontOfItem(
									pCurNumFmt->GetCharFmt()->GetFont() ) );
					sTmp.Erase();
				}
				pCurNumFmt->SetSuffix( sTmp );
			}
			break;

		case RTF_PNTXTB:
			{
				String sTmp;
				pCurNumFmt->SetPrefix( GetTextToEndGroup( sTmp ) );
			}
			break;
		}
	}

	// falls vollstaendige Numerierung an ist und das Zeichen davor ein
	// Punkt ist, dann will RTF den Punkt als Trenner zwischen den Ebenen
	// haben - das haben wir aber schon als default
	if( 1 < pCurNumFmt->GetIncludeUpperLevels() &&
		1 == pCurNumFmt->GetPrefix().Len() &&
		'.' == pCurNumFmt->GetPrefix().GetChar( 0 ) &&
		SVX_NUM_CHAR_SPECIAL != pCurNumFmt->GetNumberingType() )
		pCurNumFmt->SetPrefix( aEmptyStr );

	// falls das ein nicht numerierter Absatz mit ein Prefix-Text mit
	// einem Zeichen ist, dann setze den als Bulletzeichen
	if( pCurNumFmt->GetCharFmt() && SVX_NUM_NUMBER_NONE == pCurNumFmt->GetNumberingType() &&
		3 == nListNo && 1 == pCurNumFmt->GetPrefix().Len() )
	{
		SwCharFmt* pChFmt = pCurNumFmt->GetCharFmt();
		pCurNumFmt->SetNumberingType(SVX_NUM_CHAR_SPECIAL);
		pCurNumFmt->SetBulletFont( FindFontOfItem( pChFmt->GetFont() ) );

		pCurNumFmt->SetBulletChar( pCurNumFmt->GetPrefix().GetChar( 0 ) );
		pCurNumFmt->SetPrefix( aEmptyStr );

		// den Font oder sogar das gesamte CharFormat loeschen?
		if( SFX_ITEM_SET == pChFmt->GetItemState( RES_CHRATR_FONT, sal_False ))
		{
			if( 1 == pChFmt->GetAttrSet().Count() )
			{
				pCurNumFmt->SetCharFmt( 0 );
				pDoc->DelCharFmt( pChFmt );
			}
			else
                pChFmt->ResetFmtAttr( RES_CHRATR_FONT );
		}
	}

	SkipToken( -1 );		// die schliesende Klammer wird "oben" ausgewertet
    return pCurRule;
}


/*  */

// dann noch die Ausgabe-Funktionen (nur fuer WinWord 97)

sal_Bool lcl_IsExportNumRule( const SwNumRule& rRule, sal_uInt8* pEnd = 0 )
{
	sal_uInt8 nEnd = MAXLEVEL;
	while( nEnd-- && !rRule.GetNumFmt( nEnd ))
		;
	++nEnd;

	const SwNumFmt* pNFmt;
	sal_uInt8 nLvl;

	for( nLvl = 0; nLvl < nEnd; ++nLvl )
		if( SVX_NUM_NUMBER_NONE != ( pNFmt = &rRule.Get( nLvl ))
			->GetNumberingType() || pNFmt->GetPrefix().Len() ||
			(pNFmt->GetSuffix().Len() && pNFmt->GetSuffix() != aDotStr ))
			break;

	if( pEnd )
		*pEnd = nEnd;
	return nLvl != nEnd;
}
