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
#include "precompiled_editeng.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <editeng/tstpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/editids.hrc>

#include <editdoc.hxx>
#include <editdbg.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <stdlib.h>	// qsort

using namespace ::com::sun::star;


// ------------------------------------------------------------

sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, short nScriptType )
{
	sal_uInt16 nId = nItemId;
	
	if ( ( nScriptType == i18n::ScriptType::ASIAN ) ||
		 ( nScriptType == i18n::ScriptType::COMPLEX ) )
	{
		switch ( nItemId ) 
		{
			case EE_CHAR_LANGUAGE:
				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_LANGUAGE_CJK : EE_CHAR_LANGUAGE_CTL;
			break;
			case EE_CHAR_FONTINFO:
				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTINFO_CJK : EE_CHAR_FONTINFO_CTL;
			break;
			case EE_CHAR_FONTHEIGHT:
				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTHEIGHT_CJK : EE_CHAR_FONTHEIGHT_CTL;
			break;
			case EE_CHAR_WEIGHT:
				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_WEIGHT_CJK : EE_CHAR_WEIGHT_CTL;
			break;
			case EE_CHAR_ITALIC:
				nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_ITALIC_CJK : EE_CHAR_ITALIC_CTL;
			break;
		}
	}
	
	return nId;
}

sal_Bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType )
{
	sal_Bool bValid = sal_True;
	
	switch ( nItemId ) 
	{
		case EE_CHAR_LANGUAGE:
			bValid = nScriptType == i18n::ScriptType::LATIN;
		break;
		case EE_CHAR_LANGUAGE_CJK:
			bValid = nScriptType == i18n::ScriptType::ASIAN;
		break;
		case EE_CHAR_LANGUAGE_CTL:
			bValid = nScriptType == i18n::ScriptType::COMPLEX;
		break;
		case EE_CHAR_FONTINFO:
			bValid = nScriptType == i18n::ScriptType::LATIN;
		break;
		case EE_CHAR_FONTINFO_CJK:
			bValid = nScriptType == i18n::ScriptType::ASIAN;
		break;
		case EE_CHAR_FONTINFO_CTL:
			bValid = nScriptType == i18n::ScriptType::COMPLEX;
		break;
		case EE_CHAR_FONTHEIGHT:
			bValid = nScriptType == i18n::ScriptType::LATIN;
		break;
		case EE_CHAR_FONTHEIGHT_CJK:
			bValid = nScriptType == i18n::ScriptType::ASIAN;
		break;
		case EE_CHAR_FONTHEIGHT_CTL:
			bValid = nScriptType == i18n::ScriptType::COMPLEX;
		break;
		case EE_CHAR_WEIGHT:
			bValid = nScriptType == i18n::ScriptType::LATIN;
		break;
		case EE_CHAR_WEIGHT_CJK:
			bValid = nScriptType == i18n::ScriptType::ASIAN;
		break;
		case EE_CHAR_WEIGHT_CTL:
			bValid = nScriptType == i18n::ScriptType::COMPLEX;
		break;
		case EE_CHAR_ITALIC:
			bValid = nScriptType == i18n::ScriptType::LATIN;
		break;
		case EE_CHAR_ITALIC_CJK:
			bValid = nScriptType == i18n::ScriptType::ASIAN;
		break;
		case EE_CHAR_ITALIC_CTL:
			bValid = nScriptType == i18n::ScriptType::COMPLEX;
		break;
	}

	return bValid;
}


// ------------------------------------------------------------

// Sollte spaeter zentral nach TOOLS/STRING (Aktuell: 303)
// fuer Grep: WS_TARGET

DBG_NAME( EE_TextPortion );
DBG_NAME( EE_EditLine );
DBG_NAME( EE_ContentNode );
DBG_NAME( EE_CharAttribList );

SfxItemInfo aItemInfos[EDITITEMCOUNT] = {
		{ SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },         // EE_PARA_WRITINGDIR
		{ 0, SFX_ITEM_POOLABLE },								// EE_PARA_XMLATTRIBS
		{ SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },	// EE_PARA_HANGINGPUNCTUATION
		{ SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },
		{ SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },	    // EE_PARA_ASIANCJKSPACING
		{ SID_ATTR_NUMBERING_RULE, SFX_ITEM_POOLABLE },		    // EE_PARA_NUMBULL
		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_HYPHENATE
		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_BULLETSTATE
		{ 0, SFX_ITEM_POOLABLE },							    // EE_PARA_OUTLLRSPACE
		{ SID_ATTR_PARA_OUTLLEVEL, SFX_ITEM_POOLABLE },
		{ SID_ATTR_PARA_BULLET, SFX_ITEM_POOLABLE },
		{ SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },
		{ SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
		{ SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
		{ 0, SFX_ITEM_POOLABLE },							// EE_CHAR_RUBI_DUMMY
		{ 0, SFX_ITEM_POOLABLE },							// EE_CHAR_XMLATTRIBS
		{ SID_ATTR_CHAR_OVERLINE, SFX_ITEM_POOLABLE },
		{ 0, SFX_ITEM_POOLABLE },							// EE_FEATURE_TAB
		{ 0, SFX_ITEM_POOLABLE },							// EE_FEATURE_LINEBR
		{ SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },	// EE_FEATURE_NOTCONV
		{ SID_FIELD, SFX_ITEM_POOLABLE }
};

sal_uInt16 aV1Map[] = {
	3999, 4001, 4002, 4003, 4004, 4005, 4006,
	4007, 4008, 4009, 4010, 4011, 4012, 4013, 4017, 4018, 4019 // MI: 4019?
};

sal_uInt16 aV2Map[] = {
	3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006,	4007, 4008, 4009,
	4010, 4011, 4012, 4013, 4014, 4015, 4016, 4018, 4019, 4020
};

sal_uInt16 aV3Map[] = {
	3997, 3998, 3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006,	4007,
	4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019,
	4020, 4021
};

sal_uInt16 aV4Map[] = {
	3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003, 
	4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013, 
	4014, 4015, 4016, 4017, 4018, 
	/* CJK Items inserted here: EE_CHAR_LANGUAGE - EE_CHAR_XMLATTRIBS */
	4034, 4035, 4036, 4037 
};

sal_uInt16 aV5Map[] = {
	3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003,
	4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013,
	4014, 4015, 4016, 4017, 4018, 4019, 4020, 4021, 4022, 4023,
    4024, 4025, 4026, 4027, 4028, 4029, 4030, 4031, 4032, 4033,
	/* EE_CHAR_OVERLINE inserted here */
	4035, 4036, 4037, 4038
};

SV_IMPL_PTRARR( DummyContentList, ContentNode* );

int SAL_CALL CompareStart( const void* pFirst, const void* pSecond )
{
	if ( (*((EditCharAttrib**)pFirst))->GetStart() < (*((EditCharAttrib**)pSecond))->GetStart() )
		return (-1);
	else if ( (*((EditCharAttrib**)pFirst))->GetStart() > (*((EditCharAttrib**)pSecond))->GetStart() )
		return (1);
	return 0;
}

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE )
{
	// das neue Attribut im Pool anlegen
	const SfxPoolItem& rNew = rPool.Put( rAttr );

	EditCharAttrib* pNew = 0;
	switch( rNew.Which() )
	{
		case EE_CHAR_LANGUAGE:
		case EE_CHAR_LANGUAGE_CJK:
		case EE_CHAR_LANGUAGE_CTL:
		{
			pNew = new EditCharAttribLanguage( (const SvxLanguageItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_COLOR:
		{
			pNew = new EditCharAttribColor( (const SvxColorItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_FONTINFO:
		case EE_CHAR_FONTINFO_CJK:
		case EE_CHAR_FONTINFO_CTL:
		{
			pNew = new EditCharAttribFont( (const SvxFontItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_FONTHEIGHT:
		case EE_CHAR_FONTHEIGHT_CJK:
		case EE_CHAR_FONTHEIGHT_CTL:
		{
			pNew = new EditCharAttribFontHeight( (const SvxFontHeightItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_FONTWIDTH:
		{
			pNew = new EditCharAttribFontWidth( (const SvxCharScaleWidthItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_WEIGHT:
		case EE_CHAR_WEIGHT_CJK:
		case EE_CHAR_WEIGHT_CTL:
		{
			pNew = new EditCharAttribWeight( (const SvxWeightItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_UNDERLINE:
		{
			pNew = new EditCharAttribUnderline( (const SvxUnderlineItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_OVERLINE:
		{
			pNew = new EditCharAttribOverline( (const SvxOverlineItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_EMPHASISMARK:
		{
			pNew = new EditCharAttribEmphasisMark( (const SvxEmphasisMarkItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_RELIEF:
		{
			pNew = new EditCharAttribRelief( (const SvxCharReliefItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_STRIKEOUT:
		{
			pNew = new EditCharAttribStrikeout( (const SvxCrossedOutItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_ITALIC:
		case EE_CHAR_ITALIC_CJK:
		case EE_CHAR_ITALIC_CTL:
		{
			pNew = new EditCharAttribItalic( (const SvxPostureItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_OUTLINE:
		{
			pNew = new EditCharAttribOutline( (const SvxContourItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_SHADOW:
		{
			pNew = new EditCharAttribShadow( (const SvxShadowedItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_ESCAPEMENT:
		{
			pNew = new EditCharAttribEscapement( (const SvxEscapementItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_PAIRKERNING:
		{
			pNew = new EditCharAttribPairKerning( (const SvxAutoKernItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_KERNING:
		{
			pNew = new EditCharAttribKerning( (const SvxKerningItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_WLM:
		{
			pNew = new EditCharAttribWordLineMode( (const SvxWordLineModeItem&)rNew, nS, nE );
		}
		break;
		case EE_CHAR_XMLATTRIBS:
		{
			pNew = new EditCharAttrib( rNew, nS, nE );	// Attrib is only for holding XML information...
		}
		break;
		case EE_FEATURE_TAB:
		{
			pNew = new EditCharAttribTab( (const SfxVoidItem&)rNew, nS );
		}
		break;
		case EE_FEATURE_LINEBR:
		{
			pNew = new EditCharAttribLineBreak( (const SfxVoidItem&)rNew, nS );
		}
		break;
		case EE_FEATURE_FIELD:
		{
			pNew = new EditCharAttribField( (const SvxFieldItem&)rNew, nS );
		}
		break;
		default:
		{
			DBG_ERROR( "Ungueltiges Attribut!" );
		}
	}
	return pNew;
}

// -------------------------------------------------------------------------
// class EditLine
// -------------------------------------------------------------------------

EditLine::EditLine()
{
	DBG_CTOR( EE_EditLine, 0 );

	nStart = nEnd = 0;
	nStartPortion = 0;				// damit in ungueltiger Zeile ohne Portions von einer gueltigen Zeile mit der Portion Nr0 unterscieden werden kann.
	nEndPortion = 0;
	nHeight = 0;
	nStartPosX = 0;
	nTxtHeight = 0;
    nTxtWidth = 0;
	nCrsrHeight = 0;
	nMaxAscent = 0;
	bHangingPunctuation = sal_False;
	bInvalid = sal_True;
}

EditLine::EditLine( const EditLine& r )
{
	DBG_CTOR( EE_EditLine, 0 );

	nEnd = r.nEnd;
	nStart = r.nStart;
	nStartPortion = r.nStartPortion;
	nEndPortion = r.nEndPortion;
	bHangingPunctuation = r.bHangingPunctuation;

	nHeight = 0;
	nStartPosX = 0;
	nTxtHeight = 0;
    nTxtWidth = 0;
	nCrsrHeight = 0;
	nMaxAscent = 0;
	bInvalid = sal_True;
}

EditLine::~EditLine()
{
	DBG_DTOR( EE_EditLine, 0 );
}

EditLine* EditLine::Clone() const
{
	EditLine* pL = new EditLine;
	if ( aPositions.Count() )
	{
		pL->aPositions.Insert (aPositions.GetData(), aPositions.Count(), 0);
	}
	pL->nStartPosX 		= nStartPosX;
	pL->nStart 			= nStart;
	pL->nEnd 			= nEnd;
	pL->nStartPortion 	= nStartPortion;
	pL->nEndPortion 	= nEndPortion;
	pL->nHeight 		= nHeight;
	pL->nTxtWidth 		= nTxtWidth;
	pL->nTxtHeight 		= nTxtHeight;
	pL->nCrsrHeight 	= nCrsrHeight;
	pL->nMaxAscent 		= nMaxAscent;

	return pL;
}

sal_Bool operator == ( const EditLine& r1,  const EditLine& r2  )
{
	if ( r1.nStart != r2.nStart )
		return sal_False;

	if ( r1.nEnd != r2.nEnd )
		return sal_False;

	if ( r1.nStartPortion != r2.nStartPortion )
		return sal_False;

	if ( r1.nEndPortion != r2.nEndPortion )
		return sal_False;

	return sal_True;
}

EditLine& EditLine::operator = ( const EditLine& r )
{
	nEnd = r.nEnd;
	nStart = r.nStart;
	nEndPortion = r.nEndPortion;
	nStartPortion = r.nStartPortion;
	return *this;
}


sal_Bool operator != ( const EditLine& r1,  const EditLine& r2  )
{
	return !( r1 == r2 );
}

Size EditLine::CalcTextSize( ParaPortion& rParaPortion )
{
	Size aSz;
	Size aTmpSz;
	TextPortion* pPortion;

	sal_uInt16 nIndex = GetStart();

	DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize vor CreatePortions !" );

	for ( sal_uInt16 n = nStartPortion; n <= nEndPortion; n++ )
	{
		pPortion = rParaPortion.GetTextPortions().GetObject(n);
		switch ( pPortion->GetKind() )
		{
			case PORTIONKIND_TEXT:
			case PORTIONKIND_FIELD:
			case PORTIONKIND_HYPHENATOR:
			{
				aTmpSz = pPortion->GetSize();
				aSz.Width() += aTmpSz.Width();
				if ( aSz.Height() < aTmpSz.Height() )
					aSz.Height() = aTmpSz.Height();
			}
			break;
			case PORTIONKIND_TAB:
//			case PORTIONKIND_EXTRASPACE:
			{
				aSz.Width() += pPortion->GetSize().Width();
			}
			break;
		}
		nIndex = nIndex + pPortion->GetLen();
	}

	SetHeight( (sal_uInt16)aSz.Height() );
	return aSz;
}

// -------------------------------------------------------------------------
// class EditLineList
// -------------------------------------------------------------------------
EditLineList::EditLineList()
{
}

EditLineList::~EditLineList()
{
	Reset();
}

void EditLineList::Reset()
{
	for ( sal_uInt16 nLine = 0; nLine < Count(); nLine++ )
		delete GetObject(nLine);
	Remove( 0, Count() );
}

void EditLineList::DeleteFromLine( sal_uInt16 nDelFrom )
{
	DBG_ASSERT( nDelFrom <= (Count() - 1), "DeleteFromLine: Out of range" );
	for ( sal_uInt16 nL = nDelFrom; nL < Count(); nL++ )
		delete GetObject(nL);
	Remove( nDelFrom, Count()-nDelFrom );
}

sal_uInt16 EditLineList::FindLine( sal_uInt16 nChar, sal_Bool bInclEnd )
{
	for ( sal_uInt16 nLine = 0; nLine < Count(); nLine++ )
	{
		EditLine* pLine = GetObject( nLine );
		if ( ( bInclEnd && ( pLine->GetEnd() >= nChar ) ) ||
			 ( pLine->GetEnd() > nChar ) )
		{
			return nLine;
		}
	}

	DBG_ASSERT( !bInclEnd, "Zeile nicht gefunden: FindLine" );
	return ( Count() - 1 );
}

// -------------------------------------------------------------------------
// class EditSelection
// -------------------------------------------------------------------------
sal_Bool EditPaM::DbgIsBuggy( EditDoc& rDoc )
{
	if ( !pNode )
		return sal_True;
	if ( rDoc.GetPos( pNode ) >= rDoc.Count() )
		return sal_True;
	if ( nIndex > pNode->Len() )
		return sal_True;

	return sal_False;
}

sal_Bool EditSelection::DbgIsBuggy( EditDoc& rDoc )
{
	if ( aStartPaM.DbgIsBuggy( rDoc ) )
		return sal_True;
	if ( aEndPaM.DbgIsBuggy( rDoc ) )
		return sal_True;

	return sal_False;
}

EditSelection::EditSelection()
{
}

EditSelection::EditSelection( const EditPaM& rStartAndAnd )
{
	// koennte noch optimiert werden!
	// nicht erst Def-CTOR vom PaM rufen!
	aStartPaM = rStartAndAnd;
	aEndPaM = rStartAndAnd;
}

EditSelection::EditSelection( const EditPaM& rStart, const EditPaM& rEnd )
{
	// koennte noch optimiert werden!
	aStartPaM = rStart;
	aEndPaM = rEnd;
}

EditSelection& EditSelection::operator = ( const EditPaM& rPaM )
{
	aStartPaM = rPaM;
	aEndPaM = rPaM;
	return *this;
}

sal_Bool EditSelection::IsInvalid() const
{
	EditPaM aEmptyPaM;

	if ( aStartPaM == aEmptyPaM )
		return sal_True;

	if ( aEndPaM == aEmptyPaM )
		return sal_True;

	return sal_False;
}

sal_Bool EditSelection::Adjust( const ContentList& rNodes )
{
	DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index im Wald in Adjust(1)" );
	DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index im Wald in Adjust(2)" );

	ContentNode* pStartNode = aStartPaM.GetNode();
	ContentNode* pEndNode = aEndPaM.GetNode();

	sal_uInt16 nStartNode = rNodes.GetPos( pStartNode );
	sal_uInt16 nEndNode = rNodes.GetPos( pEndNode );

	DBG_ASSERT( nStartNode != USHRT_MAX, "Node im Wald in Adjust(1)" );
	DBG_ASSERT( nEndNode != USHRT_MAX, "Node im Wald in Adjust(2)" );

	sal_Bool bSwap = sal_False;
	if ( nStartNode > nEndNode )
		bSwap = sal_True;
	else if ( ( nStartNode == nEndNode ) && ( aStartPaM.GetIndex() > aEndPaM.GetIndex() ) )
		bSwap = sal_True;

	if ( bSwap )
	{
		EditPaM aTmpPaM( aStartPaM );
		aStartPaM = aEndPaM;
		aEndPaM = aTmpPaM;
	}

	return bSwap;
}


// -------------------------------------------------------------------------
// class EditPaM
// -------------------------------------------------------------------------
sal_Bool operator == ( const EditPaM& r1,  const EditPaM& r2  )
{
	if ( r1.GetNode() != r2.GetNode() )
		return sal_False;

	if ( r1.GetIndex() != r2.GetIndex() )
		return sal_False;

	return sal_True;
}

EditPaM& EditPaM::operator = ( const EditPaM& rPaM )
{
	nIndex = rPaM.nIndex;
	pNode = rPaM.pNode;
	return *this;
}

sal_Bool operator != ( const EditPaM& r1,  const EditPaM& r2  )
{
	return !( r1 == r2 );
}


// -------------------------------------------------------------------------
// class ContentNode
// -------------------------------------------------------------------------
ContentNode::ContentNode( SfxItemPool& rPool ) : aContentAttribs( rPool )
{
	DBG_CTOR( EE_ContentNode, 0 );
	pWrongList = NULL;
}

ContentNode::ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs ) :
	XubString( rStr ), aContentAttribs( rContentAttribs )
{
	DBG_CTOR( EE_ContentNode, 0 );
	pWrongList = NULL;
}

ContentNode::~ContentNode()
{
	DBG_DTOR( EE_ContentNode, 0 );
#ifndef SVX_LIGHT
	delete pWrongList;
#endif
}

void ContentNode::ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNew, SfxItemPool& rItemPool )
{
	if ( !nNew )
		return;

	// Da Features anders behandelt werden als normale Zeichenattribute,
	// kann sich hier auch die Sortierung der Start-Liste aendern!
	// In jedem if..., in dem weiter (n) Moeglichkeiten aufgrund von
	// bFeature oder Spezialfall existieren,
	// muessen (n-1) Moeglichkeiten mit bResort versehen werden.
	// Die wahrscheinlichste Moeglichkeit erhaelt kein bResort,
	// so dass nicht neu sortiert wird, wenn sich alle Attribute
	// gleich verhalten.
	sal_Bool bResort = sal_False;
	sal_Bool bExpandedEmptyAtIndexNull = sal_False;

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
	while ( pAttrib )
	{
		if ( pAttrib->GetEnd() >= nIndex )
		{
			// Alle Attribute hinter der Einfuegeposition verschieben...
			if ( pAttrib->GetStart() > nIndex )
			{
				pAttrib->MoveForward( nNew );
			}
			// 0: Leeres Attribut expandieren, wenn an Einfuegestelle
			else if ( pAttrib->IsEmpty() )
			{
				// Index nicht pruefen, leeres durfte nur dort liegen.
				// Wenn spaeter doch Ueberpruefung:
				//   Spezialfall: Start == 0; AbsLen == 1, nNew = 1 => Expand, weil durch Absatzumbruch!
				// Start <= nIndex, End >= nIndex => Start=End=nIndex!
//				if ( pAttrib->GetStart() == nIndex )
				pAttrib->Expand( nNew );
				if ( pAttrib->GetStart() == 0 )
					bExpandedEmptyAtIndexNull = sal_True;
			}
			// 1: Attribut startet davor, geht bis Index...
			else if ( pAttrib->GetEnd() == nIndex ) // Start muss davor liegen
			{
				// Nur expandieren, wenn kein Feature,
				// und wenn nicht in ExcludeListe!
				// Sonst geht z.B. ein UL bis zum neuen ULDB, beide expandieren
//				if ( !pAttrib->IsFeature() && !rExclList.FindAttrib( pAttrib->Which() ) )
				if ( !pAttrib->IsFeature() && !aCharAttribList.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
				{
					if ( !pAttrib->IsEdge() )
						pAttrib->Expand( nNew );
				}
				else
					bResort = sal_True;
			}
			// 2: Attribut startet davor, geht hinter Index...
			else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
			{
				DBG_ASSERT( !pAttrib->IsFeature(), "Grosses Feature?!" );
				pAttrib->Expand( nNew );
			}
			// 3: Attribut startet auf Index...
			else if ( pAttrib->GetStart() == nIndex )
			{
				if ( pAttrib->IsFeature() )
				{
					pAttrib->MoveForward( nNew );
					bResort = sal_True;
				}
				else
				{
					sal_Bool bExpand = sal_False;
					if ( nIndex == 0 )
					{
						bExpand = sal_True;
						if( bExpandedEmptyAtIndexNull )
						{
							// Check if this kind of attribut was empty and expanded here...
							sal_uInt16 nW = pAttrib->GetItem()->Which();
							for ( sal_uInt16 nA = 0; nA < nAttr; nA++ )
							{
								EditCharAttrib* pA = aCharAttribList.GetAttribs()[nA];
								if ( ( pA->GetStart() == 0 ) && ( pA->GetItem()->Which() == nW ) )
								{
									bExpand = sal_False;
									break;
								}
							}

						}
					}
					if ( bExpand )
					{
						pAttrib->Expand( nNew );
						bResort = sal_True;
					}
					else
					{
						pAttrib->MoveForward( nNew );
					}
				}
			}
		}

		if ( pAttrib->IsEdge() )
			pAttrib->SetEdge( sal_False );

		DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

		DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribut verdreht!" );
		DBG_ASSERT( ( pAttrib->GetEnd() <= Len() ), "Expand: Attrib groesser als Absatz!" );
		if ( pAttrib->IsEmpty() )
		{
			DBG_ERROR( "Leeres Attribut nach ExpandAttribs?" );
			bResort = sal_True;
			aCharAttribList.GetAttribs().Remove( nAttr );
			rItemPool.Remove( *pAttrib->GetItem() );
			delete pAttrib;
			nAttr--;
		}
		nAttr++;
		pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
	}

	if ( bResort )
		aCharAttribList.ResortAttribs();

#ifndef SVX_LIGHT
	if ( pWrongList )
	{
		sal_Bool bSep = ( GetChar( nIndex ) == ' ' ) || IsFeature( nIndex );
		pWrongList->TextInserted( nIndex, nNew, bSep );
	}
#endif // !SVX_LIGHT

#ifdef EDITDEBUG
	DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), sal_True ), "Expand: Start-Liste verdreht" );
#endif
}

void ContentNode::CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDeleted, SfxItemPool& rItemPool )
{
	if ( !nDeleted )
		return;

	// Da Features anders behandelt werden als normale Zeichenattribute,
	// kann sich hier auch die Sortierung der Start-Liste aendern!
	sal_Bool bResort = sal_False;
	sal_Bool bDelAttr = sal_False;
	sal_uInt16 nEndChanges = nIndex+nDeleted;

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
	while ( pAttrib )
	{
		bDelAttr = sal_False;
		if ( pAttrib->GetEnd() >= nIndex )
		{
			// Alles Attribute hinter der Einfuegeposition verschieben...
			if ( pAttrib->GetStart() >= nEndChanges )
			{
				pAttrib->MoveBackward( nDeleted );
			}
			// 1. Innenliegende Attribute loeschen...
			else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
			{
				// Spezialfall: Attrubt deckt genau den Bereich ab
				// => als leeres Attribut behalten.
				if ( !pAttrib->IsFeature() && ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
					pAttrib->GetEnd() = nIndex;	// leer
				else
					bDelAttr = sal_True;
			}
			// 2. Attribut beginnt davor, endet drinnen oder dahinter...
			else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
			{
				DBG_ASSERT( !pAttrib->IsFeature(), "Collapsing Feature!" );
				if ( pAttrib->GetEnd() <= nEndChanges )	// endet drinnen
					pAttrib->GetEnd() = nIndex;
				else
					pAttrib->Collaps( nDeleted );		// endet dahinter
			}
			// 3. Attribut beginnt drinnen, endet dahinter...
			else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
			{
				// Features duerfen nicht expandieren!
				if ( pAttrib->IsFeature() )
				{
					pAttrib->MoveBackward( nDeleted );
					bResort = sal_True;
				}
				else
				{
					pAttrib->GetStart() = nEndChanges;
					pAttrib->MoveBackward( nDeleted );
				}
			}
		}
		DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

		DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collaps: Attribut verdreht!" );
		DBG_ASSERT( ( pAttrib->GetEnd() <= Len()) || bDelAttr, "Collaps: Attrib groesser als Absatz!" );
		if ( bDelAttr /* || pAttrib->IsEmpty() */ )
		{
			bResort = sal_True;
			aCharAttribList.GetAttribs().Remove( nAttr );
			rItemPool.Remove( *pAttrib->GetItem() );
			delete pAttrib;
			nAttr--;
		}
		else if ( pAttrib->IsEmpty() )
			aCharAttribList.HasEmptyAttribs() = sal_True;

		nAttr++;
		pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
	}

	if ( bResort )
		aCharAttribList.ResortAttribs();

#ifndef SVX_LIGHT
	if ( pWrongList )
		pWrongList->TextDeleted( nIndex, nDeleted );
#endif // !SVX_LIGHT

#ifdef EDITDEBUG
	DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), sal_True ), "Collaps: Start-Liste verdreht" );
#endif
}

void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, sal_Bool bKeepEndingAttribs )
{
	DBG_ASSERT( pPrevNode, "kopieren von Attributen auf einen NULL-Pointer ?" );

	xub_StrLen nCut = pPrevNode->Len();

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
	while ( pAttrib )
	{
		if ( pAttrib->GetEnd() < nCut )
		{
			// bleiben unveraendert....
			;
		}
		else if ( pAttrib->GetEnd() == nCut )
		{
			// muessen als leeres Attribut kopiert werden.
			if ( bKeepEndingAttribs && !pAttrib->IsFeature() && !aCharAttribList.FindAttrib( pAttrib->GetItem()->Which(), 0 ) )
			{
				EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, 0 );
				DBG_ASSERT( pNewAttrib, "MakeCharAttrib fehlgeschlagen!" );
				aCharAttribList.InsertAttrib( pNewAttrib );
			}
		}
		else if ( pAttrib->IsInside( nCut ) || ( !nCut && !pAttrib->GetStart() && !pAttrib->IsFeature() ) )
		{
			// Wenn ganz vorne gecuttet wird, muss das Attribut erhalten bleiben!
			// muessen kopiert und geaendert werden
			EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, pAttrib->GetEnd()-nCut );
			DBG_ASSERT( pNewAttrib, "MakeCharAttrib fehlgeschlagen!" );
			aCharAttribList.InsertAttrib( pNewAttrib );
			// stutzen:
			pAttrib->GetEnd() = nCut;
		}
		else
		{
			// alle dahinter verschieben in den neuen Node (this)
//			pPrevNode->GetCharAttribs().RemoveAttrib( pAttrib );
			pPrevNode->GetCharAttribs().GetAttribs().Remove( nAttr );
			aCharAttribList.InsertAttrib( pAttrib );
			DBG_ASSERT( pAttrib->GetStart() >= nCut, "Start < nCut!" );
			DBG_ASSERT( pAttrib->GetEnd() >= nCut, "End < nCut!" );
			pAttrib->GetStart() = pAttrib->GetStart() - nCut;
			pAttrib->GetEnd() = pAttrib->GetEnd() - nCut;
			nAttr--;
		}
		nAttr++;
		pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
	}
}

void ContentNode::AppendAttribs( ContentNode* pNextNode )
{
	DBG_ASSERT( pNextNode, "kopieren von Attributen von einen NULL-Pointer ?" );

	sal_uInt16 nNewStart = Len();

#ifdef EDITDEBUG
	DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute VOR AppendAttribs kaputt" );
#endif

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
	while ( pAttrib )
	{
		// alle Attribute verschieben in den aktuellen Node (this)
		sal_Bool bMelted = sal_False;
		if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
		{
			// Evtl koennen Attribute zusammengefasst werden:
			sal_uInt16 nTmpAttr = 0;
			EditCharAttrib* pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
			while ( !bMelted && pTmpAttrib )
			{
				if ( pTmpAttrib->GetEnd() == nNewStart )
				{
					if ( ( pTmpAttrib->Which() == pAttrib->Which() ) &&
						 ( *(pTmpAttrib->GetItem()) == *(pAttrib->GetItem() ) ) )
					{
						pTmpAttrib->GetEnd() =
                            pTmpAttrib->GetEnd() + pAttrib->GetLen();
						pNextNode->GetCharAttribs().GetAttribs().Remove( nAttr );
						// Vom Pool abmelden ?!
						delete pAttrib;
						bMelted = sal_True;
					}
				}
				++nTmpAttr;
				pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
			}
		}

		if ( !bMelted )
		{
			pAttrib->GetStart() = pAttrib->GetStart() + nNewStart;
			pAttrib->GetEnd() = pAttrib->GetEnd() + nNewStart;
			aCharAttribList.InsertAttrib( pAttrib );
			++nAttr;
		}
		pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
	}
	// Fuer die Attribute, die nur ruebergewandert sind:
	pNextNode->GetCharAttribs().Clear();

#ifdef EDITDEBUG
	DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute NACH AppendAttribs kaputt" );
#endif
}

void ContentNode::CreateDefFont()
{
	// Erst alle Informationen aus dem Style verwenden...
	SfxStyleSheet* pS = aContentAttribs.GetStyleSheet();
	if ( pS )
		CreateFont( GetCharAttribs().GetDefFont(), pS->GetItemSet() );
	
	// ... dann die harte Absatzformatierung rueberbuegeln...
	CreateFont( GetCharAttribs().GetDefFont(),
		GetContentAttribs().GetItems(), pS == NULL );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle )
{
	aContentAttribs.SetStyleSheet( pS );
	
	// Erst alle Informationen aus dem Style verwenden...
	GetCharAttribs().GetDefFont() = rFontFromStyle;
	// ... dann die harte Absatzformatierung rueberbuegeln...
	CreateFont( GetCharAttribs().GetDefFont(),
		GetContentAttribs().GetItems(), pS == NULL );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, sal_Bool bRecalcFont )
{
	aContentAttribs.SetStyleSheet( pS );
	if ( bRecalcFont )
		CreateDefFont();
}

void ContentNode::DestroyWrongList()
{
#ifndef SVX_LIGHT
	delete pWrongList;
#endif
	pWrongList = NULL;
}

void ContentNode::CreateWrongList()
{
	DBG_ASSERT( !pWrongList, "WrongList existiert schon!" );
#ifndef SVX_LIGHT
	pWrongList = new WrongList;
#endif
}

void ContentNode::SetWrongList( WrongList* p ) 	
{ 
	DBG_ASSERT( !pWrongList, "WrongList existiert schon!" );
    pWrongList = p; 
}

// -------------------------------------------------------------------------
// class ContentAttribs
// -------------------------------------------------------------------------
ContentAttribs::ContentAttribs( SfxItemPool& rPool ) :
					aAttribSet( rPool, EE_PARA_START, EE_CHAR_END )
{
	pStyle = 0;
}

ContentAttribs::ContentAttribs( const ContentAttribs& rRef ) :
					aAttribSet( rRef.aAttribSet )
{
	pStyle = rRef.pStyle;
}

ContentAttribs::~ContentAttribs()
{
}

SvxTabStop ContentAttribs::FindTabStop( long nCurPos, sal_uInt16 nDefTab )
{
	const SvxTabStopItem& rTabs = (const SvxTabStopItem&) GetItem( EE_PARA_TABS );
	for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
	{
		const SvxTabStop& rTab = rTabs[i];
		if ( rTab.GetTabPos() > nCurPos  )
			return rTab;
	}

	// DefTab ermitteln...
	SvxTabStop aTabStop;
	long x = nCurPos / nDefTab + 1;
	aTabStop.GetTabPos() = nDefTab * x;
	return aTabStop;
}

void ContentAttribs::SetStyleSheet( SfxStyleSheet* pS )
{
    sal_Bool bStyleChanged = ( pStyle != pS );
	pStyle = pS;
    // #104799# Only when other style sheet, not when current style sheet modified
	if ( pStyle && bStyleChanged )
	{
		// Gezielt die Attribute aus der Absatzformatierung entfernen, die im Style
		// spezifiziert sind, damit die Attribute des Styles wirken koennen.
		const SfxItemSet& rStyleAttribs = pStyle->GetItemSet();
		for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
		{
            // #99635# Don't change bullet on/off
			if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SFX_ITEM_ON ) )
				aAttribSet.ClearItem( nWhich );
		}
	}
}

const SfxPoolItem& ContentAttribs::GetItem( sal_uInt16 nWhich )
{
	// Harte Absatzattribute haben Vorrang!
	SfxItemSet* pTakeFrom = &aAttribSet;
	if ( pStyle && ( aAttribSet.GetItemState( nWhich, sal_False ) != SFX_ITEM_ON  ) )
		pTakeFrom = &pStyle->GetItemSet();

	return pTakeFrom->Get( nWhich );
}

sal_Bool ContentAttribs::HasItem( sal_uInt16 nWhich )
{
	sal_Bool bHasItem = sal_False;
	if ( aAttribSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_ON  )
		bHasItem = sal_True;
	else if ( pStyle && pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON )
		bHasItem = sal_True;

	return bHasItem;
}



//	----------------------------------------------------------------------
//	class ItemList
//	----------------------------------------------------------------------
const SfxPoolItem* ItemList::FindAttrib( sal_uInt16 nWhich )
{
	const SfxPoolItem* pItem = First();
	while ( pItem && ( pItem->Which() != nWhich ) )
		pItem = Next();

	return pItem;
}

// -------------------------------------------------------------------------
// class EditDoc
// -------------------------------------------------------------------------
EditDoc::EditDoc( SfxItemPool* pPool )
{
	if ( pPool ) 
	{
		pItemPool = pPool;
		bOwnerOfPool = sal_False;
	}
	else
	{
		pItemPool = new EditEngineItemPool( sal_False );
		bOwnerOfPool = sal_True;
	}
	
	nDefTab = DEFTAB;
	bIsVertical = sal_False;
	bIsFixedCellHeight = sal_False;

	// Don't create a empty node, Clear() will be called in EditEngine-CTOR

	SetModified( sal_False );
};

EditDoc::~EditDoc()
{
	ImplDestroyContents();
	if ( bOwnerOfPool ) 
        SfxItemPool::Free(pItemPool);
}

void EditDoc::ImplDestroyContents()
{
	for ( sal_uInt16 nNode = Count(); nNode; )
		RemoveItemsFromPool( GetObject( --nNode ) );
	DeleteAndDestroy( 0, Count() );
}

void EditDoc::RemoveItemsFromPool( ContentNode* pNode )
{
	for ( sal_uInt16 nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
	{
		EditCharAttrib* pAttr = pNode->GetCharAttribs().GetAttribs()[nAttr];
		GetItemPool().Remove( *pAttr->GetItem() );
	}
}

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent, short nScriptType )
{
	Font aPrevFont( rFont );
	rFont.SetAlign( ALIGN_BASELINE );
	rFont.SetTransparent( sal_True );

    sal_uInt16 nWhich_FontInfo = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );
    sal_uInt16 nWhich_Language = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    sal_uInt16 nWhich_FontHeight = GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType );
    sal_uInt16 nWhich_Weight = GetScriptItemId( EE_CHAR_WEIGHT, nScriptType );
    sal_uInt16 nWhich_Italic = GetScriptItemId( EE_CHAR_ITALIC, nScriptType );

	if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontInfo ) == SFX_ITEM_ON ) )
	{
		const SvxFontItem& rFontItem = (const SvxFontItem&)rSet.Get( nWhich_FontInfo );
		rFont.SetName( rFontItem.GetFamilyName() );
		rFont.SetFamily( rFontItem.GetFamily() );
		rFont.SetPitch( rFontItem.GetPitch() );
		rFont.SetCharSet( rFontItem.GetCharSet() );
	}
	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Language ) == SFX_ITEM_ON ) )
		rFont.SetLanguage( ((const SvxLanguageItem&)rSet.Get( nWhich_Language )).GetLanguage() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_COLOR ) == SFX_ITEM_ON ) )
		rFont.SetColor( ((const SvxColorItem&)rSet.Get( EE_CHAR_COLOR )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontHeight ) == SFX_ITEM_ON ) )
		rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem&)rSet.Get( nWhich_FontHeight ) ).GetHeight() ) );
	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Weight ) == SFX_ITEM_ON ) )
		rFont.SetWeight( ((const SvxWeightItem&)rSet.Get( nWhich_Weight )).GetWeight() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON ) )
		rFont.SetUnderline( ((const SvxUnderlineItem&)rSet.Get( EE_CHAR_UNDERLINE )).GetLineStyle() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OVERLINE ) == SFX_ITEM_ON ) )
		rFont.SetOverline( ((const SvxOverlineItem&)rSet.Get( EE_CHAR_OVERLINE )).GetLineStyle() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON ) )
		rFont.SetStrikeout( ((const SvxCrossedOutItem&)rSet.Get( EE_CHAR_STRIKEOUT )).GetStrikeout() );
	if ( bSearchInParent || ( rSet.GetItemState( nWhich_Italic ) == SFX_ITEM_ON ) )
		rFont.SetItalic( ((const SvxPostureItem&)rSet.Get( nWhich_Italic )).GetPosture() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OUTLINE ) == SFX_ITEM_ON ) )
		rFont.SetOutline( ((const SvxContourItem&)rSet.Get( EE_CHAR_OUTLINE )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_SHADOW ) == SFX_ITEM_ON ) )
		rFont.SetShadow( ((const SvxShadowedItem&)rSet.Get( EE_CHAR_SHADOW )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_ESCAPEMENT ) == SFX_ITEM_ON ) )
	{
        const SvxEscapementItem& rEsc = (const SvxEscapementItem&) rSet.Get( EE_CHAR_ESCAPEMENT );

        sal_uInt16 nProp = rEsc.GetProp();
		rFont.SetPropr( (sal_uInt8)nProp );

        short nEsc = rEsc.GetEsc();
	    if ( nEsc == DFLT_ESC_AUTO_SUPER )
		    nEsc = 100 - nProp;
	    else if ( nEsc == DFLT_ESC_AUTO_SUB )
		    nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
	    rFont.SetEscapement( nEsc );
	}
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_PAIRKERNING ) == SFX_ITEM_ON ) )
		rFont.SetKerning( ((const SvxAutoKernItem&)rSet.Get( EE_CHAR_PAIRKERNING )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_KERNING ) == SFX_ITEM_ON ) )
		rFont.SetFixKerning( ((const SvxKerningItem&)rSet.Get( EE_CHAR_KERNING )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_WLM ) == SFX_ITEM_ON ) )
		rFont.SetWordLineMode( ((const SvxWordLineModeItem&)rSet.Get( EE_CHAR_WLM )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_EMPHASISMARK ) == SFX_ITEM_ON ) )
		rFont.SetEmphasisMark( ((const SvxEmphasisMarkItem&)rSet.Get( EE_CHAR_EMPHASISMARK )).GetValue() );
	if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_RELIEF ) == SFX_ITEM_ON ) )
		rFont.SetRelief( (FontRelief)((const SvxCharReliefItem&)rSet.Get( EE_CHAR_RELIEF )).GetValue() );

	// Ob ich jetzt den ganzen Font vergleiche, oder vor jeder Aenderung
	// pruefe, ob der Wert sich aendert, bleibt sich relativ gleich.
	// So ggf ein MakeUniqFont im Font mehr, dafuer bei Aenderung schnellerer
	// Abbruch der Abfrage, oder ich musste noch jedesmal ein bChanged pflegen.
	if ( rFont == aPrevFont  )
		rFont = aPrevFont;	// => Gleicher ImpPointer fuer IsSameInstance
}

void EditDoc::CreateDefFont( sal_Bool bUseStyles )
{
	SfxItemSet aTmpSet( GetItemPool(), EE_PARA_START, EE_CHAR_END );
	CreateFont( aDefFont, aTmpSet );
	aDefFont.SetVertical( IsVertical() );
	aDefFont.SetOrientation( IsVertical() ? 2700 : 0 );

	for ( sal_uInt16 nNode = 0; nNode < Count(); nNode++ )
	{
		ContentNode* pNode = GetObject( nNode );
		pNode->GetCharAttribs().GetDefFont() = aDefFont;
		if ( bUseStyles )
			pNode->CreateDefFont();
	}
}

static const sal_Unicode aCR[] = { 0x0d, 0x00 };
static const sal_Unicode aLF[] = { 0x0a, 0x00 };
static const sal_Unicode aCRLF[] = { 0x0d, 0x0a, 0x00 };

XubString EditDoc::GetSepStr( LineEnd eEnd )
{
	XubString aSep;
	if ( eEnd == LINEEND_CR )
		aSep = aCR;
	else if ( eEnd == LINEEND_LF )
		aSep = aLF;
	else
		aSep = aCRLF;
	return aSep;
}

XubString EditDoc::GetText( LineEnd eEnd ) const
{
	sal_uLong nLen = GetTextLen();
	sal_uInt16 nNodes = Count();

	String aSep = EditDoc::GetSepStr( eEnd );
	sal_uInt16 nSepSize = aSep.Len();

	if ( nSepSize )
		nLen += nNodes * nSepSize;
	if ( nLen > 0xFFFb / sizeof(xub_Unicode) )
	{
		DBG_ERROR( "Text zu gross fuer String" );
		return XubString();
	}
	xub_Unicode* pStr = new xub_Unicode[nLen+1];
	xub_Unicode* pCur = pStr;
	sal_uInt16 nLastNode = nNodes-1;
	for ( sal_uInt16 nNode = 0; nNode < nNodes; nNode++ )
	{
		XubString aTmp( GetParaAsString( GetObject(nNode) ) );
		memcpy( pCur, aTmp.GetBuffer(), aTmp.Len()*sizeof(sal_Unicode) );
		pCur += aTmp.Len();
		if ( nSepSize && ( nNode != nLastNode ) )
		{
			memcpy( pCur, aSep.GetBuffer(), nSepSize*sizeof(sal_Unicode ) );
			pCur += nSepSize;
		}
	}
	*pCur = '\0';
	XubString aASCIIText( pStr );
	delete[] pStr;
	return aASCIIText;
}

XubString EditDoc::GetParaAsString( sal_uInt16 nNode ) const
{
	return GetParaAsString( SaveGetObject( nNode ) );
}

XubString EditDoc::GetParaAsString( ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, sal_Bool bResolveFields ) const
{
	if ( nEndPos > pNode->Len() )
		nEndPos = pNode->Len();

	DBG_ASSERT( nStartPos <= nEndPos, "Start und Ende vertauscht?" );

	sal_uInt16 nIndex = nStartPos;
	XubString aStr;
	EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( nIndex );
	while ( nIndex < nEndPos )
	{
		sal_uInt16 nEnd = nEndPos;
		if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
			nEnd = pNextFeature->GetStart();
		else
			pNextFeature = 0;	// Feature interessiert unten nicht

		DBG_ASSERT( nEnd >= nIndex, "Ende vorm Index?" );
        //!! beware of sub string length  of -1 which is also defined as STRING_LEN and 
        //!! thus would result in adding the whole sub string up to the end of the node !!
        if (nEnd > nIndex)
		    aStr += XubString( *pNode, nIndex, nEnd - nIndex );

		if ( pNextFeature )
		{
			switch ( pNextFeature->GetItem()->Which() )
			{
				case EE_FEATURE_TAB:	aStr += '\t';
				break;
				case EE_FEATURE_LINEBR:	aStr += '\x0A';
				break;
				case EE_FEATURE_FIELD:	if ( bResolveFields )
											aStr += ((EditCharAttribField*)pNextFeature)->GetFieldValue();
				break;
				default:	DBG_ERROR( "Was fuer ein Feature ?" );
			}
			pNextFeature = pNode->GetCharAttribs().FindFeature( ++nEnd );
		}
		nIndex = nEnd;
	}
	return aStr;
}

sal_uLong EditDoc::GetTextLen() const
{
	sal_uLong nLen = 0;
	for ( sal_uInt16 nNode = 0; nNode < Count(); nNode++ )
	{
		ContentNode* pNode = GetObject( nNode );
		nLen += pNode->Len();
		// Felder k�nnen laenger sein als der Platzhalter im Node.
		const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
		for ( sal_uInt16 nAttr = rAttrs.Count(); nAttr; )
		{
			EditCharAttrib* pAttr = rAttrs[--nAttr];
			if ( pAttr->Which() == EE_FEATURE_FIELD )
			{
				sal_uInt16 nFieldLen = ((EditCharAttribField*)pAttr)->GetFieldValue().Len();
				if ( !nFieldLen )
					nLen--;
				else
					nLen += nFieldLen-1;
			}
		}
	}
	return nLen;
}

EditPaM EditDoc::Clear()
{
	ImplDestroyContents();

	ContentNode* pNode = new ContentNode( GetItemPool() );
	Insert( pNode, 0 );

	CreateDefFont( sal_False );

	SetModified( sal_False );

	EditPaM aPaM( pNode, 0 );
	return aPaM;
}

void EditDoc::SetModified( sal_Bool b )	
{ 
    bModified = b;
    if ( bModified )
    {
        aModifyHdl.Call( NULL );
    }
}

EditPaM EditDoc::RemoveText()
{
	// Das alte ItemSetmerken, damit z.B. im Chart Font behalten bleibt
	ContentNode* pPrevFirstNode = GetObject(0);
	SfxStyleSheet* pPrevStyle = pPrevFirstNode->GetStyleSheet();
	SfxItemSet aPrevSet( pPrevFirstNode->GetContentAttribs().GetItems() );
	Font aPrevFont( pPrevFirstNode->GetCharAttribs().GetDefFont() );

	ImplDestroyContents();

	ContentNode* pNode = new ContentNode( GetItemPool() );
	Insert( pNode, 0 );

	pNode->SetStyleSheet( pPrevStyle, sal_False );
	pNode->GetContentAttribs().GetItems().Set( aPrevSet );
	pNode->GetCharAttribs().GetDefFont() = aPrevFont;

	SetModified( sal_True );

	EditPaM aPaM( pNode, 0 );
	return aPaM;
}

void EditDoc::InsertText( const EditPaM& rPaM, xub_Unicode c )
{
    DBG_ASSERT( c != 0x0A, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
    DBG_ASSERT( c != 0x0D, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
    DBG_ASSERT( c != '\t', "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );

    rPaM.GetNode()->Insert( c, rPaM.GetIndex() );
    rPaM.GetNode()->ExpandAttribs( rPaM.GetIndex(), 1, GetItemPool() );

    SetModified( sal_True );
}

EditPaM EditDoc::InsertText( EditPaM aPaM, const XubString& rStr )
{
	DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
	DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
	DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "EditDoc::InsertText: Zeilentrenner in Absatz nicht erlaubt!" );
	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertText1" );

	aPaM.GetNode()->Insert( rStr, aPaM.GetIndex() );
	aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), rStr.Len(), GetItemPool() );
	aPaM.GetIndex() = aPaM.GetIndex() + rStr.Len();

	SetModified( sal_True );

	return aPaM;
}

EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, sal_Bool bKeepEndingAttribs )
{
	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertParaBreak" );
	ContentNode* pCurNode = aPaM.GetNode();
	sal_uInt16 nPos = GetPos( pCurNode );
	XubString aStr = aPaM.GetNode()->Copy( aPaM.GetIndex() );
	aPaM.GetNode()->Erase( aPaM.GetIndex() );

	// the paragraph attributes...
	ContentAttribs aContentAttribs( aPaM.GetNode()->GetContentAttribs() );

	// for a new paragraph we like to have the bullet/numbering visible by default
	aContentAttribs.GetItems().Put( SfxBoolItem( EE_PARA_BULLETSTATE, sal_True), EE_PARA_BULLETSTATE );

	// ContenNode-CTOR kopiert auch die Absatzattribute
	ContentNode* pNode = new ContentNode( aStr, aContentAttribs );

	// Den Default-Font kopieren
	pNode->GetCharAttribs().GetDefFont() = aPaM.GetNode()->GetCharAttribs().GetDefFont();
	SfxStyleSheet* pStyle = aPaM.GetNode()->GetStyleSheet();
	if ( pStyle )
	{
		XubString aFollow( pStyle->GetFollow() );
		if ( aFollow.Len() && ( aFollow != pStyle->GetName() ) )
		{
			SfxStyleSheetBase* pNext = pStyle->GetPool().Find( aFollow, pStyle->GetFamily() );
			pNode->SetStyleSheet( (SfxStyleSheet*)pNext );
		}
	}

	// Zeichenattribute muessen ggf. kopiert bzw gestutzt werden:
	pNode->CopyAndCutAttribs( aPaM.GetNode(), GetItemPool(), bKeepEndingAttribs );

	Insert( pNode, nPos+1 );

	SetModified( sal_True );

	aPaM.SetNode( pNode );
	aPaM.SetIndex( 0 );
	return aPaM;
}

EditPaM EditDoc::InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem  )
{
	DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertFeature" );

	aPaM.GetNode()->Insert( CH_FEATURE, aPaM.GetIndex() );
	aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1, GetItemPool() );

	// Fuer das Feature ein Feature-Attribut anlegen...
	EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rItem, aPaM.GetIndex(), aPaM.GetIndex()+1 );
	DBG_ASSERT( pAttrib, "Warum kann ich kein Feature anlegen ?" );
	aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttrib );

	SetModified( sal_True );

	aPaM.GetIndex()++;
	return aPaM;
}

EditPaM EditDoc::ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight )
{
	const EditPaM aPaM( pLeft, pLeft->Len() );

	// Erst die Attribute, da sonst nLen nicht stimmt!
	pLeft->AppendAttribs( pRight );
	// Dann den Text...
	*pLeft += *pRight;

	// der rechte verschwindet.
	RemoveItemsFromPool( pRight );
	sal_uInt16 nRight = GetPos( pRight );
	Remove( nRight );
	delete pRight;

	SetModified( sal_True );

	return aPaM;
}

EditPaM EditDoc::RemoveChars( EditPaM aPaM, sal_uInt16 nChars )
{
	// Evtl. Features entfernen!
	aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
	aPaM.GetNode()->CollapsAttribs( aPaM.GetIndex(), nChars, GetItemPool() );

	SetModified( sal_True );

	return aPaM;
}

void EditDoc::InsertAttribInSelection( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem )
{
	DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
	DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );

	// fuer Optimierung:
	// dieses endet am Anfang der Selektion => kann erweitert werden
	EditCharAttrib* pEndingAttrib = 0;
	// dieses startet am Ende der Selektion => kann erweitert werden
	EditCharAttrib* pStartingAttrib = 0;

	DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );

	RemoveAttribs( pNode, nStart, nEnd, pStartingAttrib, pEndingAttrib, rPoolItem.Which() );

	if ( pStartingAttrib && pEndingAttrib &&
		 ( *(pStartingAttrib->GetItem()) == rPoolItem ) &&
		 ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
	{
		// wird ein groesses Attribut.
		pEndingAttrib->GetEnd() = pStartingAttrib->GetEnd();
		GetItemPool().Remove( *(pStartingAttrib->GetItem()) );
		pNode->GetCharAttribs().GetAttribs().Remove( pNode->GetCharAttribs().GetAttribs().GetPos( pStartingAttrib ) );
		delete pStartingAttrib;
	}
	else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
		pStartingAttrib->GetStart() = nStart;
	else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
		pEndingAttrib->GetEnd() = nEnd;
	else
		InsertAttrib( rPoolItem, pNode, nStart, nEnd );

	if ( pStartingAttrib )
		pNode->GetCharAttribs().ResortAttribs();

	SetModified( sal_True );
}

sal_Bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt16 nWhich )
{
	EditCharAttrib* pStarting;
	EditCharAttrib* pEnding;
	return RemoveAttribs( pNode, nStart, nEnd, pStarting, pEnding, nWhich );
}

sal_Bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich )
{
	DBG_ASSERT( pNode, "Wohin mit dem Attribut?" );
	DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribut zu gross!" );

	// dieses endet am Anfang der Selektion => kann erweitert werden
	rpEnding = 0;
	// dieses startet am Ende der Selektion => kann erweitert werden
	rpStarting = 0;

	sal_Bool bChanged = sal_False;

	DBG_ASSERT( nStart <= nEnd, "Kleiner Rechenfehler in InsertAttribInSelection" );

	// ueber die Attribute iterieren...
	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
	while ( pAttr )
	{
		sal_Bool bRemoveAttrib = sal_False;
		// MT 11.9.97:
		// Ich denke dass in dieser Methode generell keine Features geloescht
		// werden sollen.
		// => Dann koennen die Feature-Abfragen weiter unten entfallen
		sal_uInt16 nAttrWhich = pAttr->Which();
		if ( ( nAttrWhich < EE_FEATURE_START ) && ( !nWhich || ( nAttrWhich == nWhich ) ) )
		{
			// Attribut beginnt in Selection
			if ( ( pAttr->GetStart() >= nStart ) && ( pAttr->GetStart() <= nEnd ) )
			{
				bChanged = sal_True;
				if ( pAttr->GetEnd() > nEnd )
				{
					pAttr->GetStart() = nEnd;	// dann faengt es dahinter an
					rpStarting = pAttr;
					if ( nWhich )
						break;	// es kann kein weiteres Attrib hier liegen
				}
				else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
				{
					// Feature nur loeschen, wenn genau an der Stelle
					bRemoveAttrib = sal_True;
				}
			}

			// Attribut endet in Selection
			else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
			{
				bChanged = sal_True;
				if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
				{
					pAttr->GetEnd() = nStart;	// dann hoert es hier auf
					rpEnding = pAttr;
				}
				else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
				{
					// Feature nur loeschen, wenn genau an der Stelle
					bRemoveAttrib = sal_True;
				}
			}
			// Attribut ueberlappt die Selektion
			else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
			{
				bChanged = sal_True;
				if ( pAttr->GetStart() == nStart )
				{
					pAttr->GetStart() = nEnd;
					rpStarting = pAttr;
					if ( nWhich )
						break;	// es kann weitere Attribute geben!
				}
				else if ( pAttr->GetEnd() == nEnd )
				{
					pAttr->GetEnd() = nStart;
					rpEnding = pAttr;
					if ( nWhich )
						break;	// es kann weitere Attribute geben!
				}
				else // Attribut muss gesplittet werden...
				{
					sal_uInt16 nOldEnd = pAttr->GetEnd();
					pAttr->GetEnd() = nStart;
					rpEnding = pAttr;
					InsertAttrib( *pAttr->GetItem(), pNode, nEnd, nOldEnd );
					if ( nWhich )
						break;	// es kann weitere Attribute geben!
				}
			}
		}
		if ( bRemoveAttrib )
		{
			DBG_ASSERT( ( pAttr != rpStarting ) && ( pAttr != rpEnding ), "Loeschen und behalten des gleichen Attributs ?" );
			DBG_ASSERT( !pAttr->IsFeature(), "RemoveAttribs: Remove a feature?!" );
			pNode->GetCharAttribs().GetAttribs().Remove(nAttr);
			GetItemPool().Remove( *pAttr->GetItem() );
			delete pAttr;
			nAttr--;
		}
		nAttr++;
		pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
	}

	if ( bChanged )
    {
        // char attributes need to be sorted by start again
        pNode->GetCharAttribs().ResortAttribs();

        SetModified( sal_True );
    }

	return bChanged;
}

void EditDoc::InsertAttrib( const SfxPoolItem& rPoolItem, ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd )
{
	// Diese Methode prueft nicht mehr, ob ein entspr. Attribut
	// schon an der Stelle existiert!

	EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rPoolItem, nStart, nEnd );
	DBG_ASSERT( pAttrib, "MakeCharAttrib fehlgeschlagen!" );
	pNode->GetCharAttribs().InsertAttrib( pAttrib );

	SetModified( sal_True );
}

void EditDoc::InsertAttrib( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem )
{
	if ( nStart != nEnd )
	{
		InsertAttribInSelection( pNode, nStart, nEnd, rPoolItem );
	}
	else
	{
		// Pruefen, ob schon ein neues Attribut mit der WhichId an der Stelle:
		EditCharAttrib* pAttr = pNode->GetCharAttribs().FindEmptyAttrib( rPoolItem.Which(), nStart );
		if ( pAttr )
		{
			// Attribut entfernen....
			pNode->GetCharAttribs().GetAttribs().Remove(
				pNode->GetCharAttribs().GetAttribs().GetPos( pAttr ) );
                        delete pAttr; // #i120865#, need free it explicitly
		}

		// pruefen, ob ein 'gleiches' Attribut an der Stelle liegt.
		pAttr = pNode->GetCharAttribs().FindAttrib( rPoolItem.Which(), nStart );
		if ( pAttr )
		{
			if ( pAttr->IsInside( nStart ) )	// splitten
			{
				// ???????????????????????????????
				// eigentlich noch pruefen, ob wirklich splittet, oder return !
				// ???????????????????????????????
				sal_uInt16 nOldEnd = pAttr->GetEnd();
				pAttr->GetEnd() = nStart;
				pAttr = MakeCharAttrib( GetItemPool(), *(pAttr->GetItem()), nStart, nOldEnd );
				pNode->GetCharAttribs().InsertAttrib( pAttr );
			}
			else if ( pAttr->GetEnd() == nStart )
			{
				DBG_ASSERT( !pAttr->IsEmpty(), "Doch noch ein leeres Attribut?" );
				// pruefen, ob genau das gleiche Attribut
				if ( *(pAttr->GetItem()) == rPoolItem )
					return;
			}
		}
		InsertAttrib( rPoolItem, pNode, nStart, nStart );
	}

	SetModified( sal_True );
}

void EditDoc::FindAttribs( ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, SfxItemSet& rCurSet )
{
	DBG_ASSERT( pNode, "Wo soll ich suchen ?" );
	DBG_ASSERT( nStartPos <= nEndPos, "Ungueltiger Bereich!" );

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
	// keine Selection...
	if ( nStartPos == nEndPos )
	{
		while ( pAttr && ( pAttr->GetStart() <= nEndPos) )
		{
			const SfxPoolItem* pItem = 0;
			// Attribut liegt dadrueber...
			if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
				pItem = pAttr->GetItem();
			// Attribut endet hier, ist nicht leer
			else if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
			{
				if ( !pNode->GetCharAttribs().FindEmptyAttrib( pAttr->GetItem()->Which(), nStartPos ) )
					pItem = pAttr->GetItem();
			}
			// Attribut endet hier, ist leer
			else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
			{
				pItem = pAttr->GetItem();
			}
			// Attribut beginnt hier
			else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
			{
				if ( nStartPos == 0 ) 	// Sonderfall
					pItem = pAttr->GetItem();
			}

			if ( pItem )
			{
				sal_uInt16 nWhich = pItem->Which();
				if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
				{
					rCurSet.Put( *pItem );
				}
				else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
				{
					const SfxPoolItem& rItem = rCurSet.Get( nWhich );
					if ( rItem != *pItem )
					{
						rCurSet.InvalidateItem( nWhich );
					}
				}
			}
			nAttr++;
			pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
		}
	}
	else	// Selektion
	{
		while ( pAttr && ( pAttr->GetStart() < nEndPos) )
		{
			const SfxPoolItem* pItem = 0;
			// Attribut liegt dadrueber...
			if ( ( pAttr->GetStart() <= nStartPos ) && ( pAttr->GetEnd() >= nEndPos ) )
				pItem = pAttr->GetItem();
			// Attribut startet mitten drin...
			else if ( pAttr->GetStart() >= nStartPos )
			{
				// !!! pItem = pAttr->GetItem();
				// einfach nur pItem reicht nicht, da ich z.B. bei Shadow
				// niemals ein ungleiches Item finden wuerde, da ein solche
				// seine Anwesenheit durch Abwesenheit repraesentiert!
				// if ( ... )
				// Es muesste geprueft werden, on genau das gleiche Attribut
				// an der Bruchstelle aufsetzt, was recht aufwendig ist.
				// Da ich beim Einfuegen von Attributen aber etwas optimiere
				// tritt der Fall nicht so schnell auf...
				// Also aus Geschwindigkeitsgruenden:
				rCurSet.InvalidateItem( pAttr->GetItem()->Which() );

			}
			// Attribut endet mitten drin...
			else if ( pAttr->GetEnd() > nStartPos )
			{
				// pItem = pAttr->GetItem();
				// s.o.
				/*-----------------31.05.95 16:01-------------------
				 Ist falsch, wenn das gleiche Attribut sofort wieder
				 eingestellt wird!
				 => Sollte am besten nicht vorkommen, also gleich beim
					Setzen von Attributen richtig machen!
				--------------------------------------------------*/
				rCurSet.InvalidateItem( pAttr->GetItem()->Which() );
			}

			if ( pItem )
			{
				sal_uInt16 nWhich = pItem->Which();
				if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
				{
					rCurSet.Put( *pItem );
				}
				else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
				{
					const SfxPoolItem& rItem = rCurSet.Get( nWhich );
					if ( rItem != *pItem )
					{
						rCurSet.InvalidateItem( nWhich );
					}
				}
			}
			nAttr++;
			pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
		}
	}
}


// -------------------------------------------------------------------------
// class EditCharAttribList
// -------------------------------------------------------------------------

CharAttribList::CharAttribList()
{
	DBG_CTOR( EE_CharAttribList, 0 );
	bHasEmptyAttribs = sal_False;
}

CharAttribList::~CharAttribList()
{
	DBG_DTOR( EE_CharAttribList, 0 );

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
	while ( pAttr )
	{
		delete pAttr;
		++nAttr;
		pAttr = GetAttrib( aAttribs, nAttr );
	}
	Clear();
}

void CharAttribList::InsertAttrib( EditCharAttrib* pAttrib )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// optimieren: binaere Suche ? !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// MT: 26.11.98
	// Vielleicht aber auch einfach nur rueckwaerts iterieren:
	// Der haeufigste und kritischste Fall: Attribute kommen bereits
	// sortiert an (InsertBinTextObject!)
	// Hier waere auch binaere Suche nicht optimal.
	// => Wuerde einiges bringen!

	const sal_uInt16 nCount = Count();
	const sal_uInt16 nStart = pAttrib->GetStart(); // vielleicht besser fuer Comp.Opt.

	if ( pAttrib->IsEmpty() )
		bHasEmptyAttribs = sal_True;

	sal_Bool bInserted = sal_False;
	for ( sal_uInt16 x = 0; x < nCount; x++ )
	{
		EditCharAttribPtr pCurAttrib = aAttribs[x];
		if ( pCurAttrib->GetStart() > nStart )
		{
			aAttribs.Insert( pAttrib, x );
			bInserted = sal_True;
			break;
		}
	}
	if ( !bInserted )
		aAttribs.Insert( pAttrib, nCount );
}

void CharAttribList::ResortAttribs()
{
	if ( Count() )
	{
#if defined __SUNPRO_CC
#pragma disable_warn
#endif
		qsort( (void*)aAttribs.GetData(), aAttribs.Count(), sizeof( EditCharAttrib* ), CompareStart );
#if defined __SUNPRO_CC
#pragma enable_warn
#endif
	}
}

void CharAttribList::OptimizeRanges( SfxItemPool& rItemPool )
{
	for ( sal_uInt16 n = 0; n < aAttribs.Count(); n++ )
	{
        EditCharAttrib* pAttr = aAttribs.GetObject( n );
        for ( sal_uInt16 nNext = n+1; nNext < aAttribs.Count(); nNext++ )
        {
            EditCharAttrib* p = aAttribs.GetObject( nNext );
            if ( !pAttr->IsFeature() && ( p->GetStart() == pAttr->GetEnd() ) && ( p->Which() == pAttr->Which() ) )
            {
                if ( *p->GetItem() == *pAttr->GetItem() )
                {
                    pAttr->GetEnd() = p->GetEnd();
                    aAttribs.Remove( nNext );
                    rItemPool.Remove( *p->GetItem() );
                    delete p;
                }
                break;  // only 1 attr with same which can start here.
            }
            else if ( p->GetStart() > pAttr->GetEnd() )
            {
                break;
            }
        }
	}
}

EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
	// Rueckwaerts, falls eins dort endet, das naechste startet.
	// => Das startende gilt...
	sal_uInt16 nAttr = aAttribs.Count()-1;
	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
	while ( pAttr )
	{
		if ( ( pAttr->Which() == nWhich ) && pAttr->IsIn(nPos) )
			return pAttr;
		pAttr = GetAttrib( aAttribs, --nAttr );
	}
	return 0;
}

EditCharAttrib* CharAttribList::FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos ) const
{
	DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
	const sal_uInt16 nAttribs = aAttribs.Count();
	for ( sal_uInt16 nAttr = 0; nAttr < nAttribs; nAttr++ )
	{
		EditCharAttrib* pAttr = aAttribs[ nAttr ];
		if ( ( pAttr->GetStart() >= nFromPos ) && ( pAttr->Which() == nWhich ) )
			return pAttr;
	}
	return 0;
}

sal_Bool CharAttribList::HasAttrib( sal_uInt16 nWhich ) const
{
	for ( sal_uInt16 nAttr = aAttribs.Count(); nAttr; )
	{
		const EditCharAttrib* pAttr = aAttribs[--nAttr];
		if ( pAttr->Which() == nWhich )
			return sal_True;
	}
	return sal_False;
}

sal_Bool CharAttribList::HasAttrib( sal_uInt16 nStartPos, sal_uInt16 nEndPos ) const
{
	sal_Bool bAttr = sal_False;
	for ( sal_uInt16 nAttr = aAttribs.Count(); nAttr && !bAttr; )
	{
		const EditCharAttrib* pAttr = aAttribs[--nAttr];
		if ( ( pAttr->GetStart() < nEndPos ) && ( pAttr->GetEnd() > nStartPos ) )
			return bAttr = sal_True;
	}
	return bAttr;
}


sal_Bool CharAttribList::HasBoundingAttrib( sal_uInt16 nBound )
{
	// Rueckwaerts, falls eins dort endet, das naechste startet.
	// => Das startende gilt...
	sal_uInt16 nAttr = aAttribs.Count()-1;
	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
	while ( pAttr && ( pAttr->GetEnd() >= nBound ) )
	{
		if ( ( pAttr->GetStart() == nBound ) || ( pAttr->GetEnd() == nBound ) )
			return sal_True;
		pAttr = GetAttrib( aAttribs, --nAttr );
	}
	return sal_False;
}

EditCharAttrib* CharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
	if ( !bHasEmptyAttribs )
		return 0;
	sal_uInt16 nAttr = 0;
	EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
	while ( pAttr && ( pAttr->GetStart() <= nPos ) )
	{
		if ( ( pAttr->GetStart() == nPos ) && ( pAttr->GetEnd() == nPos ) && ( pAttr->Which() == nWhich ) )
			return pAttr;
		nAttr++;
		pAttr = GetAttrib( aAttribs, nAttr );
	}
	return 0;
}

EditCharAttrib*	CharAttribList::FindFeature( sal_uInt16 nPos ) const
{

	sal_uInt16 nAttr = 0;
	EditCharAttrib* pNextAttrib = GetAttrib( aAttribs, nAttr );

	// erstmal zur gewuenschten Position...
	while ( pNextAttrib && ( pNextAttrib->GetStart() < nPos ) )
	{
		nAttr++;
		pNextAttrib = GetAttrib( aAttribs, nAttr );
	}

	// jetzt das Feature suchen...
	while ( pNextAttrib && !pNextAttrib->IsFeature() )
	{
		nAttr++;
		pNextAttrib = GetAttrib( aAttribs, nAttr );
	}

	return pNextAttrib;
}


void CharAttribList::DeleteEmptyAttribs( SfxItemPool& rItemPool )
{
	for ( sal_uInt16 nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
	{
		EditCharAttrib* pAttr = aAttribs[nAttr];
		if ( pAttr->IsEmpty() )
		{
			rItemPool.Remove( *pAttr->GetItem() );
			aAttribs.Remove( nAttr );
			delete pAttr;
			nAttr--;
		}
	}
	bHasEmptyAttribs = sal_False;
}

sal_Bool CharAttribList::DbgCheckAttribs()
{
#ifdef  DBG_UTIL
	sal_Bool bOK = sal_True;
	for ( sal_uInt16 nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
	{
		EditCharAttrib* pAttr = aAttribs[nAttr];
		if ( pAttr->GetStart() > pAttr->GetEnd() )
		{
			bOK = sal_False;
			DBG_ERROR( "Attr verdreht" );
		}
		else if ( pAttr->IsFeature() && ( pAttr->GetLen() != 1 ) )
		{
			bOK = sal_False;
			DBG_ERROR( "Feature, Len != 1" );
		}
	}
	return bOK;
#else
	return sal_True;
#endif
}



SvxFontTable::SvxFontTable()
{
}

SvxFontTable::~SvxFontTable()
{
	SvxFontItem* pItem = First();
	while( pItem )
	{
		delete pItem;
		pItem = Next();
	}
}

sal_uLong SvxFontTable::GetId( const SvxFontItem& rFontItem )
{
	SvxFontItem* pItem = First();
	while ( pItem )
	{
		if ( *pItem == rFontItem )
			return GetCurKey();
		pItem = Next();
	}
	DBG_WARNING( "Font nicht gefunden: GetId()" );
	return 0;
}

SvxColorList::SvxColorList()
{
}

SvxColorList::~SvxColorList()
{
	SvxColorItem* pItem = First();
	while( pItem )
	{
		delete pItem;
		pItem = Next();
	}
}

sal_uLong SvxColorList::GetId( const SvxColorItem& rColorItem )
{
	SvxColorItem* pItem = First();
	while ( pItem )
	{
		if ( *pItem == rColorItem )
			return GetCurPos();
		pItem = Next();
	}
	DBG_WARNING( "Color nicht gefunden: GetId()" );
	return 0;
}

EditEngineItemPool::EditEngineItemPool( sal_Bool bPersistenRefCounts )
	: SfxItemPool( String( "EditEngineItemPool", RTL_TEXTENCODING_ASCII_US ), EE_ITEMS_START, EE_ITEMS_END,
					aItemInfos, 0, bPersistenRefCounts )
{
	SetVersionMap( 1, 3999, 4015, aV1Map );
	SetVersionMap( 2, 3999, 4019, aV2Map );
	SetVersionMap( 3, 3997, 4020, aV3Map );
	SetVersionMap( 4, 3994, 4022, aV4Map );
	SetVersionMap( 5, 3994, 4037, aV5Map );

	DBG_ASSERT( EE_DLL(), "EditDLL?!" );
	SfxPoolItem** ppDefItems = EE_DLL()->GetGlobalData()->GetDefItems();
	SetDefaults( ppDefItems );
}

EditEngineItemPool::~EditEngineItemPool()
{
}

SvStream& EditEngineItemPool::Store( SvStream& rStream ) const
{
	// Bei einem 3.1-Export muess ein Hack eingebaut werden, da BUG im
	// SfxItemSet::Load, aber nicht nachtraeglich in 3.1 fixbar.

	// Der eingestellte Range muss nach Store erhalten bleiben, weil dann
	// erst die ItemSets gespeichert werden...

	long nVersion = rStream.GetVersion();
	sal_Bool b31Format = ( nVersion && ( nVersion <= SOFFICE_FILEFORMAT_31 ) )
						? sal_True : sal_False;

	EditEngineItemPool* pThis = (EditEngineItemPool*)this;
	if ( b31Format )
		pThis->SetStoringRange( 3997, 4022 );
	else
		pThis->SetStoringRange( EE_ITEMS_START, EE_ITEMS_END );

	return SfxItemPool::Store( rStream );
}
