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

// System - Includes ---------------------------------------------------------
#include <vcl/svapp.hxx>
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svtools/txtcmp.hxx>
#include <unotools/charclass.hxx>
// INCLUDE -------------------------------------------------------------------
#include <editeng/unolingu.hxx>
#include <svx/dialmgr.hxx>
#include "ctredlin.hrc"
#include <svx/ctredlin.hxx>
#include "helpid.hrc"

//============================================================================
//	Local Defines and Function
//----------------------------------------------------------------------------

inline void EnableDisable( Window& rWin, sal_Bool bEnable )
{
	if (bEnable)
		rWin.Enable();
	else
		rWin.Disable();
}

static long nStaticTabs[]=
{
	5,10,65,120,170,220
};

#define	MIN_DISTANCE	6
#define WRITER_AUTHOR   1
#define WRITER_DATE		2
#define CALC_AUTHOR		2
#define CALC_DATE		3

RedlinData::RedlinData()
{
	bDisabled=sal_False;
	pData=NULL;
}
RedlinData::~RedlinData()
{
}

//============================================================================
//	class SvxRedlinEntry (Eintraege fuer Liste)
//----------------------------------------------------------------------------

SvxRedlinEntry::SvxRedlinEntry()
	:SvLBoxEntry()
{
}

SvxRedlinEntry::~SvxRedlinEntry()
{
	RedlinData* pRedDat=(RedlinData*) GetUserData();
	if(pRedDat!=NULL)
	{
		delete pRedDat;
	}
}

/*************************************************************************
#* Funktionen der in den SvxRedlinTable eingefuegten Items
#************************************************************************/

DBG_NAME(SvLBoxColorString);

/*************************************************************************
#*	Member:		SvLBoxColorString							Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:		SvLBoxColorString
#*
#*  Funktion:	Konstruktor der Klasse SvLBoxColorString
#*
#*  Input:		Box- Entry,Flags, Text fuer Anzeige, Schrift
#*
#*	Output:		---
#*
#************************************************************************/

SvLBoxColorString::SvLBoxColorString( SvLBoxEntry*pEntry,sal_uInt16 nFlags,const XubString& rStr,
									const Color& rCol)

: SvLBoxString( pEntry, nFlags, rStr )
{
	DBG_CTOR(SvLBoxColorString,0);
	aPrivColor=rCol;
	SetText( pEntry, rStr );
}

/*************************************************************************
#*	Member:		SvLBoxColorString							Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:		SvLBoxColorString
#*
#*  Funktion:	Default Konstruktor der Klasse SvLBoxColorString
#*
#*  Input:		---
#*
#*	Output:		---
#*
#************************************************************************/

SvLBoxColorString::SvLBoxColorString()
: SvLBoxString()
{
	DBG_CTOR(SvLBoxColorString,0);
}

/*************************************************************************
#*	Member:		~SvLBoxColorString							Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:		SvLBoxColorString
#*
#*  Funktion:	Destruktor der Klasse SvLBoxColorString
#*
#*  Input:		---
#*
#*	Output:		---
#*
#************************************************************************/

SvLBoxColorString::~SvLBoxColorString()
{
	DBG_DTOR(SvLBoxColorString,0);
}


/*************************************************************************
#*	Member:		SvLBoxColorString							Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:		SvLBoxColorString
#*
#*  Funktion:	Erzeugt einen neuen SvLBoxColorString
#*
#*  Input:		---
#*
#*	Output:		SvLBoxColorString
#*
#************************************************************************/

SvLBoxItem* SvLBoxColorString::Create() const
{
	DBG_CHKTHIS(SvLBoxColorString,0);
	return new SvLBoxColorString;
}



/*************************************************************************
#*	Member:		SvLBoxColorString							Datum:23.10.97
#*------------------------------------------------------------------------
#*
#*  Klasse:		SvLBoxColorString
#*
#*  Funktion:	Zeichenroutine des SvLBoxColorString. Gezeichnet wird
#*				der entsprechende Text mit der eingestellten Farbe
#*				im Ausgabe- Device.
#*
#*  Input:		Position, Ausgabe- Device, Flag fuer Selection,
#*				Zeiger auf den Eintrag
#*
#*	Output:		---
#*
#************************************************************************/

void SvLBoxColorString::Paint( const Point& rPos, SvLBox& rDev,
							 sal_uInt16 nFlags,	SvLBoxEntry* pEntry )
{
	Color aColor=rDev.GetTextColor();
	Color a2Color=aColor;
	if(!(nFlags & SVLISTENTRYFLAG_SELECTED))
	{
		rDev.SetTextColor(aPrivColor);
	}
	SvLBoxString::Paint(rPos,rDev,nFlags,pEntry );
	rDev.SetTextColor(a2Color);
}

//============================================================================
//	class SvxRedlinTable
//----------------------------------------------------------------------------

SvxRedlinTable::SvxRedlinTable( Window* pParent,WinBits nBits ):
		SvxSimpleTable(pParent,nBits )
{
	bAuthor=sal_False;
	bDate=sal_False;
	bIsCalc=sal_False;
	bComment=sal_False;
	nDatePos=WRITER_DATE;
	pCommentSearcher=NULL;
    SetNodeDefaultImages();
}

SvxRedlinTable::SvxRedlinTable( Window* pParent,const ResId& rResId):
		SvxSimpleTable(pParent,rResId)
{
	bAuthor=sal_False;
	bDate=sal_False;
	bIsCalc=sal_False;
	bComment=sal_False;
	nDatePos=WRITER_DATE;
	pCommentSearcher=NULL;
    SetNodeDefaultImages();
}

SvxRedlinTable::~SvxRedlinTable()
{
	if(pCommentSearcher!=NULL)
		delete pCommentSearcher;
}

StringCompare SvxRedlinTable::ColCompare(SvLBoxEntry* pLeft,SvLBoxEntry* pRight)
{
	StringCompare eCompare=COMPARE_EQUAL;

	if(aColCompareLink.IsSet())
	{
		SvSortData aRedlinCompare;
		aRedlinCompare.pLeft=pLeft;
		aRedlinCompare.pRight=pRight;
		eCompare=(StringCompare) aColCompareLink.Call(&aRedlinCompare);
	}
	else
	{
		if(nDatePos==GetSortedCol())
		{
			RedlinData *pLeftData=(RedlinData *)(pLeft->GetUserData());
			RedlinData *pRightData=(RedlinData *)(pRight->GetUserData());

			if(pLeftData!=NULL && pRightData!=NULL)
			{
				if(pLeftData->aDateTime < pRightData->aDateTime)
				{
					eCompare=COMPARE_LESS;
				}
				else if(pLeftData->aDateTime > pRightData->aDateTime)
				{
					eCompare=COMPARE_GREATER;
				}
			}
			else
				eCompare=SvxSimpleTable::ColCompare(pLeft,pRight);
		}
		else
		{
			eCompare=SvxSimpleTable::ColCompare(pLeft,pRight);
		}

	}

	return eCompare;
}
void SvxRedlinTable::SetCalcView(sal_Bool bFlag)
{
	bIsCalc=bFlag;
	if(bFlag)
	{
		nDatePos=CALC_DATE;
	}
	else
	{
		nDatePos=WRITER_DATE;
	}
}


void SvxRedlinTable::UpdateFilterTest()
{
	Date aDateMax;
	sal_uInt16 nYEAR=aDateMax.GetYear()+100;
	aDateMax.SetYear(nYEAR);
	Date aDateMin(1,1,1989);
	Time aTMin(0);
	Time aTMax(23,59,59);


	DateTime aDTMin(aDateMin);
	DateTime aDTMax(aDateMax);

	switch(nDaTiMode)
	{
		case FLT_DATE_BEFORE:
								aDaTiFilterFirst=aDTMin;
								aDaTiFilterLast=aDaTiFirst;
								break;
		case FLT_DATE_SAVE:
		case FLT_DATE_SINCE:
								aDaTiFilterFirst=aDaTiFirst;
								aDaTiFilterLast=aDTMax;
								break;
		case FLT_DATE_EQUAL:
								aDaTiFilterFirst=aDaTiFirst;
								aDaTiFilterLast=aDaTiFirst;
								aDaTiFilterFirst.SetTime(aTMin.GetTime());
								aDaTiFilterLast.SetTime(aTMax.GetTime());
								break;
		case FLT_DATE_NOTEQUAL:
								aDaTiFilterFirst=aDaTiFirst;
								aDaTiFilterLast=aDaTiFirst;
								aDaTiFilterFirst.SetTime(aTMin.GetTime());
								aDaTiFilterLast.SetTime(aTMax.GetTime());
								break;
		case FLT_DATE_BETWEEN:
								aDaTiFilterFirst=aDaTiFirst;
								aDaTiFilterLast=aDaTiLast;
								break;
	}
}


void SvxRedlinTable::SetFilterDate(sal_Bool bFlag)
{
	bDate=bFlag;
}

void SvxRedlinTable::SetDateTimeMode(sal_uInt16 nMode)
{
	nDaTiMode=nMode;
}

void SvxRedlinTable::SetFirstDate(const Date& aDate)
{
	aDaTiFirst.SetDate(aDate.GetDate());
}

void SvxRedlinTable::SetLastDate(const Date& aDate)
{
	aDaTiLast.SetDate(aDate.GetDate());
}

void SvxRedlinTable::SetFirstTime(const Time& aTime)
{
	aDaTiFirst.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetLastTime(const Time& aTime)
{
	aDaTiLast.SetTime(aTime.GetTime());
}

void SvxRedlinTable::SetFilterAuthor(sal_Bool bFlag)
{
	bAuthor=bFlag;
}

void SvxRedlinTable::SetAuthor(const String &aString)
{
	aAuthor=aString;
}

void SvxRedlinTable::SetFilterComment(sal_Bool bFlag)
{
	bComment=bFlag;
}

void SvxRedlinTable::SetCommentParams( const utl::SearchParam* pSearchPara )
{
	if(pSearchPara!=NULL)
	{
		if(pCommentSearcher!=NULL) delete pCommentSearcher;

		pCommentSearcher=new utl::TextSearch(*pSearchPara, LANGUAGE_SYSTEM );
	}
}

sal_Bool SvxRedlinTable::IsValidWriterEntry(const String& rString,RedlinData *pUserData)
{
	sal_Bool nTheFlag=sal_True;
	String aString=rString.GetToken(WRITER_AUTHOR,'\t');
	if(pUserData==NULL)
	{
		if(bAuthor)
		{
			if(aAuthor.CompareTo(aString)==COMPARE_EQUAL)
				nTheFlag=sal_True;
			else
				nTheFlag=sal_False;
		}
	}
	else
	{
		DateTime aDateTime=pUserData->aDateTime;
		nTheFlag=IsValidEntry(&aString,&aDateTime);
	}
	return nTheFlag;
}

sal_Bool SvxRedlinTable::IsValidCalcEntry(const String& rString,RedlinData *pUserData)
{
	sal_Bool nTheFlag=sal_True;
	String aString=rString.GetToken(CALC_AUTHOR,'\t');
	if(pUserData==NULL)
	{
		if(bAuthor)
		{
			if(aAuthor.CompareTo(aString)==COMPARE_EQUAL)
				nTheFlag=sal_True;
			else
				nTheFlag=sal_False;
		}
	}
	else
	{
		DateTime aDateTime=pUserData->aDateTime;
		nTheFlag=IsValidEntry(&aString,&aDateTime);
	}
	return nTheFlag;
}

sal_Bool SvxRedlinTable::IsValidEntry(const String* pAuthorStr,
								  const DateTime *pDateTime,const String* pCommentStr)
{
	bool nTheFlag=true;
	if(bAuthor)
	{
		nTheFlag = aAuthor.CompareTo(*pAuthorStr)==COMPARE_EQUAL;
	}
	if(bDate && nTheFlag)
	{
		if(nDaTiMode!=FLT_DATE_NOTEQUAL)
		{
			nTheFlag=pDateTime->IsBetween(aDaTiFilterFirst,aDaTiFilterLast);
		}
		else
		{
			nTheFlag=!(pDateTime->IsBetween(aDaTiFilterFirst,aDaTiFilterLast));
		}
	}
	if(bComment && nTheFlag)
	{
		xub_StrLen nStartPos = 0;
		xub_StrLen nEndPos = pCommentStr->Len();

		nTheFlag=pCommentSearcher->SearchFrwrd( *pCommentStr, &nStartPos, &nEndPos);
	}
	return nTheFlag;
}

sal_Bool SvxRedlinTable::IsValidEntry(const String* pAuthorStr,const DateTime *pDateTime)
{
	sal_Bool nTheFlag=sal_True;
	if(bAuthor)
	{
		if(aAuthor.CompareTo(*pAuthorStr)==COMPARE_EQUAL)
			nTheFlag=sal_True;
		else
			nTheFlag=sal_False;
	}
	if(bDate && nTheFlag)
	{
		if(nDaTiMode!=FLT_DATE_NOTEQUAL)
		{
			nTheFlag=pDateTime->IsBetween(aDaTiFilterFirst,aDaTiFilterLast);
		}
		else
		{
			nTheFlag=!(pDateTime->IsBetween(aDaTiFilterFirst,aDaTiFilterLast));
		}
	}
	return nTheFlag;
}

sal_Bool SvxRedlinTable::IsValidComment(const String* pCommentStr)
{
	bool nTheFlag=true;

	if(bComment)
	{
		xub_StrLen nStartPos = 0;
		xub_StrLen nEndPos = pCommentStr->Len();

		nTheFlag=pCommentSearcher->SearchFrwrd( *pCommentStr, &nStartPos, &nEndPos);
	}
	return nTheFlag;
}

SvLBoxEntry* SvxRedlinTable::InsertEntry(const String& rStr,RedlinData *pUserData,
								SvLBoxEntry* pParent,sal_uIntPtr nPos)
{
	aEntryColor=GetTextColor();
	if(pUserData!=NULL)
	{
		if(pUserData->bDisabled)
			aEntryColor=Color(COL_GRAY);
	}

	XubString aStr= rStr;

	XubString aFirstStr( aStr );
	xub_StrLen nEnd = aFirstStr.Search( sal_Unicode( '\t' ) );
	if( nEnd != STRING_NOTFOUND )
	{
		aFirstStr.Erase( nEnd );
		aCurEntry = aStr;
		aCurEntry.Erase( 0, ++nEnd );
	}
	else
		aCurEntry.Erase();

	return SvTreeListBox::InsertEntry( aFirstStr, pParent, sal_False, nPos, pUserData );

}

SvLBoxEntry* SvxRedlinTable::InsertEntry(const String& rStr,RedlinData *pUserData,const Color& aColor,
								SvLBoxEntry* pParent,sal_uIntPtr nPos)
{
	aEntryColor=aColor;

	XubString aStr= rStr;

	XubString aFirstStr( aStr );
	xub_StrLen nEnd = aFirstStr.Search( sal_Unicode ( '\t' ) );
	if( nEnd != STRING_NOTFOUND )
	{
		aFirstStr.Erase( nEnd );
		aCurEntry = aStr;
		aCurEntry.Erase( 0, ++nEnd );
	}
	else
		aCurEntry.Erase();

	return SvTreeListBox::InsertEntry( aFirstStr, pParent, sal_False, nPos, pUserData );
}

SvLBoxEntry* SvxRedlinTable::CreateEntry() const
{
	return new SvxRedlinEntry;
}

void SvxRedlinTable::InitEntry( SvLBoxEntry* pEntry, const XubString& rStr,
	const Image& rColl, const Image& rExp, SvLBoxButtonKind eButtonKind )
{
	SvLBoxButton* pButton;
	SvLBoxString* pString;
	SvLBoxContextBmp* pContextBmp;

	if( nTreeFlags & TREEFLAG_CHKBTN )
	{
		pButton= new SvLBoxButton( pEntry,eButtonKind,0,pCheckButtonData );
		pEntry->AddItem( pButton );
	}

	pContextBmp= new SvLBoxContextBmp( pEntry,0, rColl,rExp,
									 SVLISTENTRYFLAG_EXPANDED);
	pEntry->AddItem( pContextBmp );

	pString = new SvLBoxColorString( pEntry, 0, rStr ,aEntryColor);
	pEntry->AddItem( pString );

	XubString aToken;

	xub_Unicode* pCurToken = (xub_Unicode*)aCurEntry.GetBuffer();
	sal_uInt16 nCurTokenLen;
	xub_Unicode* pNextToken = (xub_Unicode*)GetToken( pCurToken, nCurTokenLen );
	sal_uInt16 nCount = TabCount(); nCount--;

	for( sal_uInt16 nToken = 0; nToken < nCount; nToken++ )
	{
		if( pCurToken && nCurTokenLen )
			// aToken.Assign( pCurToken, nCurTokenLen );
			aToken = XubString( pCurToken, nCurTokenLen );
		else
			aToken.Erase();

		SvLBoxColorString* pStr = new SvLBoxColorString( pEntry, 0, aToken ,aEntryColor);
		pEntry->AddItem( pStr );

		pCurToken = pNextToken;
		if( pCurToken )
			pNextToken = (xub_Unicode*)GetToken( pCurToken, nCurTokenLen );
		else
			nCurTokenLen = 0;
	}
}




//============================================================================
//	class SvxTPView
//----------------------------------------------------------------------------

SvxTPView::SvxTPView( Window * pParent)
	: TabPage( pParent, SVX_RES(SID_REDLIN_VIEW_PAGE)),
	aViewData	( this, SVX_RES( DG_VIEW) ),
	PbAccept	( this, SVX_RES(PB_ACCEPT	 ) ),
    PbReject    ( this, SVX_RES(PB_REJECT  ) ),
    PbAcceptAll ( this, SVX_RES(PB_ACCEPTALL  ) ),
    PbRejectAll ( this, SVX_RES(PB_REJECTALL  ) ),
	PbUndo		( this, SVX_RES(PB_UNDO  ) ),
	aTitle1		( SVX_RES( STR_TITLE1 ) ),		// lokale Resource
	aTitle2		( SVX_RES( STR_TITLE2 ) ),
	aTitle3		( SVX_RES( STR_TITLE3 ) ),
	aTitle4		( SVX_RES( STR_TITLE4 ) ),
	aTitle5		( SVX_RES( STR_TITLE5 ) ),
	aStrMyName	( SVX_RES( STR_VIEW) )
{
	aViewData.SetAccessibleName(String(SVX_RES(STR_TREE)));
	FreeResource();

    aViewData.SetHelpId(HID_REDLINING_VIEW_DG_VIEW_TABLE);
    aViewData.SetHeaderBarHelpId(HID_REDLINING_VIEW_DG_VIEW_HEADER);

	aMinSize=GetSizePixel();

	Link aLink=LINK( this, SvxTPView, PbClickHdl);

	PbAccept.SetClickHdl(aLink);
	PbAcceptAll.SetClickHdl(aLink);
	PbReject.SetClickHdl(aLink);
	PbRejectAll.SetClickHdl(aLink);
	PbUndo.SetClickHdl(aLink);

	nDistance=PbAccept.GetSizePixel().Height()+2*MIN_DISTANCE;
	aViewData.SetTabs(nStaticTabs);
}

String SvxTPView::GetMyName() const
{
	return aStrMyName;
}

void SvxTPView::Resize()
{
	Size aSize=GetOutputSizePixel();
	Point aPos=aViewData.GetPosPixel();
	aSize.Height()-=aPos.Y()+nDistance;
	aSize.Width()-=2*aPos.X();

	long newY=aPos.Y()+aSize.Height()+MIN_DISTANCE;
	aPos=PbAccept.GetPosPixel();
	aPos.Y()=newY;
	PbAccept.SetPosPixel(aPos);
	aPos=PbAcceptAll.GetPosPixel();
	aPos.Y()=newY;
	PbAcceptAll.SetPosPixel(aPos);
	aPos=PbReject.GetPosPixel();
	aPos.Y()=newY;
	PbReject.SetPosPixel(aPos);
	aPos=PbRejectAll.GetPosPixel();
	aPos.Y()=newY;
	PbRejectAll.SetPosPixel(aPos);

	if(PbUndo.IsVisible())
	{
		aPos=PbUndo.GetPosPixel();
		aPos.Y()=newY;
		PbUndo.SetPosPixel(aPos);
	}
	aViewData.SetSizePixel(aSize);
}

void SvxTPView::InsertWriterHeader()
{
	String aStrTab(sal_Unicode('\t'));
	String aString(aTitle1);
	aString+=aStrTab;
	aString+=aTitle3;
	aString+=aStrTab;
	aString+=aTitle4;
	aString+=aStrTab;
	aString+=aTitle5;
	aViewData.ClearHeader();
	aViewData.InsertHeaderEntry(aString);
}

void SvxTPView::InsertCalcHeader()
{
	String aStrTab(sal_Unicode('\t'));
	String aString(aTitle1);
	aString+=aStrTab;
	aString+=aTitle2;
	aString+=aStrTab;
	aString+=aTitle3;
	aString+=aStrTab;
	aString+=aTitle4;
	aString+=aStrTab;
	aString+=aTitle5;
	aViewData.ClearHeader();
	aViewData.InsertHeaderEntry(aString);
}

void SvxTPView::EnableAccept(sal_Bool nFlag)
{
	PbAccept.Enable(nFlag);
}

void SvxTPView::EnableAcceptAll(sal_Bool nFlag)
{
	PbAcceptAll.Enable(nFlag);
}

void SvxTPView::EnableReject(sal_Bool nFlag)
{
	PbReject.Enable(nFlag);
}

void SvxTPView::EnableRejectAll(sal_Bool nFlag)
{
	PbRejectAll.Enable(nFlag);
}

void SvxTPView::ShowUndo(sal_Bool nFlag)
{
	PbUndo.Show(nFlag);
}

void SvxTPView::EnableUndo(sal_Bool nFlag)
{
	PbUndo.Enable(nFlag);
}

Size SvxTPView::GetMinSizePixel()
{
	Size aSize=aMinSize;
	if(PbUndo.IsVisible())
	{
		sal_uIntPtr nSize=PbUndo.GetSizePixel().Width()
					+PbUndo.GetPosPixel().X()
					+PbAccept.GetPosPixel().X();

		aSize.Width()=nSize;
	}

	return aSize;
}


SvxRedlinTable* SvxTPView::GetTableControl()
{
	return &aViewData;
}

IMPL_LINK( SvxTPView, PbClickHdl, PushButton*, pPushB )
{
	if(pPushB==&PbAccept)
	{
		AcceptClickLk.Call(this);
	}
	else if(pPushB==&PbAcceptAll)
	{
		AcceptAllClickLk.Call(this);
	}
	else if(pPushB==&PbReject)
	{
		RejectClickLk.Call(this);
	}
	else if(pPushB==&PbRejectAll)
	{
		RejectAllClickLk.Call(this);
	}
	else if(pPushB==&PbUndo)
	{
		UndoClickLk.Call(this);
	}

	return 0;
}




//============================================================================
//	class SvxTPFilter
//----------------------------------------------------------------------------

SvxTPFilter::SvxTPFilter( Window * pParent)
	: TabPage( pParent, SVX_RES(SID_REDLIN_FILTER_PAGE)),
    pRedlinTable(NULL),
    aCbDate     ( this, SVX_RES( CB_DATE ) ),
	aLbDate		( this, SVX_RES( LB_DATE ) ),
	aDfDate		( this, SVX_RES( DF_DATE ) ),
	aTfDate		( this, SVX_RES( TF_DATE ) ),
	aIbClock 	( this, SVX_RES( IB_CLOCK ) ),
	aFtDate2	( this, SVX_RES( FT_DATE2 ) ),
	aDfDate2	( this, SVX_RES( DF_DATE2 ) ),
	aTfDate2	( this, SVX_RES( TF_DATE2 ) ),
	aIbClock2 	( this, SVX_RES( IB_CLOCK2) ),
	aCbAuthor	( this, SVX_RES( CB_AUTOR ) ),
	aLbAuthor	( this, SVX_RES( LB_AUTOR ) ),
	aCbRange 	( this, SVX_RES( CB_RANGE ) ),
	aEdRange 	( this, SVX_RES( ED_RANGE ) ),
	aBtnRange 	( this, SVX_RES( BTN_REF ) ),
	aLbAction	( this, SVX_RES( LB_ACTION ) ),
	aCbComment	( this, SVX_RES( CB_COMMENT) ),
	aEdComment	( this, SVX_RES( ED_COMMENT) ),
    aActionStr  (       SVX_RES( STR_ACTION) ),
    aStrMyName  (       SVX_RES( STR_FILTER) ),
    bModified   (sal_False)
{
    aLbDate.SetAccessibleName( String( SVX_RES( STR_DATE_COMBOX) ) );
    aDfDate.SetAccessibleName( String( SVX_RES( STR_DATE_SPIN) ) );
    aTfDate.SetAccessibleName( String( SVX_RES( STR_DATE_TIME_SPIN) ) );
    aDfDate2.SetAccessibleName( String( SVX_RES( STR_DATE_SPIN1) ) );
    aTfDate2.SetAccessibleName( String( SVX_RES( STR_DATE_TIME_SPIN1) ) );
	aLbAuthor.SetAccessibleName(aCbAuthor.GetText());
	aLbAction.SetAccessibleName( String( SVX_RES( STR_ACTION) ) );
	aEdComment.SetAccessibleName(aCbComment.GetText());
    Image aImgTimeHC( SVX_RES( IMG_TIME_H ) );
	FreeResource();

    aIbClock.SetModeImage( aImgTimeHC, BMP_COLOR_HIGHCONTRAST );
    aIbClock2.SetModeImage( aImgTimeHC, BMP_COLOR_HIGHCONTRAST );

	aDfDate.SetShowDateCentury( sal_True );
	aDfDate2.SetShowDateCentury( sal_True );

	aRangeStr=aCbRange.GetText();
	aLbDate.SelectEntryPos(0);
	aLbDate.SetSelectHdl( LINK( this, SvxTPFilter, SelDateHdl ) );
	aIbClock.SetClickHdl( LINK( this, SvxTPFilter, TimeHdl) );
	aIbClock2.SetClickHdl( LINK( this, SvxTPFilter,TimeHdl) );
	aBtnRange.SetClickHdl( LINK( this, SvxTPFilter, RefHandle));

	Link aLink=LINK( this, SvxTPFilter, RowEnableHdl) ;
	aCbDate.SetClickHdl(aLink);
	aCbAuthor.SetClickHdl(aLink);
	aCbRange.SetClickHdl(aLink);
	aCbComment.SetClickHdl(aLink);

	Link a2Link=LINK( this, SvxTPFilter, ModifyDate);
	aDfDate.SetModifyHdl(a2Link);
	aTfDate.SetModifyHdl(a2Link);
	aDfDate2.SetModifyHdl(a2Link);
	aTfDate2.SetModifyHdl(a2Link);

	Link a3Link=LINK( this, SvxTPFilter, ModifyHdl);
	aEdRange.SetModifyHdl(a3Link);
	aEdComment.SetModifyHdl(a3Link);
	aLbAction.SetSelectHdl(a3Link);
	aLbAuthor.SetSelectHdl(a3Link);

	RowEnableHdl(&aCbDate);
	RowEnableHdl(&aCbAuthor);
	RowEnableHdl(&aCbRange);
	RowEnableHdl(&aCbComment);

	Date aDate;
	Time aTime;
	aDfDate.SetDate(aDate);
	aTfDate.SetTime(aTime);
	aDfDate2.SetDate(aDate);
	aTfDate2.SetTime(aTime);
	HideRange();
	ShowAction();
	bModified=sal_False;

	aLbDate.SetAccessibleRelationLabeledBy(&aCbDate);
	aLbAuthor.SetAccessibleRelationLabeledBy(&aCbAuthor);
	aLbAction.SetAccessibleRelationLabeledBy(&aCbRange);
	aEdRange.SetAccessibleRelationLabeledBy(&aCbRange);
	aBtnRange.SetAccessibleRelationLabeledBy(&aCbRange);
	aEdComment.SetAccessibleRelationLabeledBy(&aCbComment);
	aDfDate2.SetAccessibleRelationLabeledBy(&aDfDate2);
	aTfDate2.SetAccessibleRelationLabeledBy(&aTfDate2);
}

void SvxTPFilter::SetRedlinTable(SvxRedlinTable* pTable)
{
	pRedlinTable=pTable;
}

String SvxTPFilter::GetMyName() const
{
	return aStrMyName;
}

void SvxTPFilter::DisableRange(sal_Bool bFlag)
{
	if(bFlag)
	{
		aCbRange.Disable();
		aEdRange.Disable();
		aBtnRange.Disable();
	}
	else
	{
		aCbRange.Enable();
		aEdRange.Enable();
		aBtnRange.Enable();
	}
}

void SvxTPFilter::ShowDateFields(sal_uInt16 nKind)
{
	String aEmpty;
	switch(nKind)
	{
		case FLT_DATE_BEFORE:
				EnableDateLine1(sal_True);
				EnableDateLine2(sal_False);
				break;
		case FLT_DATE_SINCE:
				EnableDateLine1(sal_True);
				EnableDateLine2(sal_False);
				break;
		case FLT_DATE_EQUAL:
				EnableDateLine1(sal_True);
				aTfDate.Disable();
				aTfDate.SetText(aEmpty);
				EnableDateLine2(sal_False);
				break;
		case FLT_DATE_NOTEQUAL:
				EnableDateLine1(sal_True);
				aTfDate.Disable();
				aTfDate.SetText(aEmpty);
				EnableDateLine2(sal_False);
				break;
		case FLT_DATE_BETWEEN:
				EnableDateLine1(sal_True);
				EnableDateLine2(sal_True);
				break;
		case FLT_DATE_SAVE:
				EnableDateLine1(sal_False);
				EnableDateLine2(sal_False);
				break;
	}
}

void SvxTPFilter::EnableDateLine1(sal_Bool bFlag)
{
	if(bFlag && aCbDate.IsChecked())
	{
		aDfDate.Enable();
		aTfDate.Enable();
		aIbClock.Enable();
	}
	else
	{
		aDfDate.Disable();
		aTfDate.Disable();
		aIbClock.Disable();
	}
}
void SvxTPFilter::EnableDateLine2(sal_Bool bFlag)
{
	String aEmpty;
	if(bFlag && aCbDate.IsChecked())
	{
		aFtDate2.Enable();
		aDfDate2.Enable();
		aTfDate2.Enable();
		aIbClock2.Enable();
	}
	else
	{
		aFtDate2.Disable();
		aDfDate2.Disable();
		aDfDate2.SetText(aEmpty);
		aTfDate2.Disable();
		aTfDate2.SetText(aEmpty);
		aIbClock2.Disable();
	}
}

Date SvxTPFilter::GetFirstDate() const
{
	return aDfDate.GetDate();
}

void SvxTPFilter::SetFirstDate(const Date &aDate)
{
	aDfDate.SetDate(aDate);
}

Time SvxTPFilter::GetFirstTime() const
{
	return aTfDate.GetTime();
}

void SvxTPFilter::SetFirstTime(const Time &aTime)
{
	aTfDate.SetTime(aTime);
}


Date SvxTPFilter::GetLastDate() const
{
	return aDfDate2.GetDate();
}

void SvxTPFilter::SetLastDate(const Date &aDate)
{
	aDfDate2.SetDate(aDate);
}

Time SvxTPFilter::GetLastTime() const
{
	return aTfDate2.GetTime();
}

void SvxTPFilter::SetLastTime(const Time &aTime)
{
	aTfDate2.SetTime(aTime);
}

void SvxTPFilter::SetDateMode(sal_uInt16 nMode)
{
	aLbDate.SelectEntryPos(nMode);
	SelDateHdl(&aLbDate);
}

sal_uInt16 SvxTPFilter::GetDateMode()
{
	return (sal_uInt16) aLbDate.GetSelectEntryPos();
}
void SvxTPFilter::ClearAuthors()
{
	aLbAuthor.Clear();
}

void SvxTPFilter::InsertAuthor( const String& rString, sal_uInt16 nPos)
{
	aLbAuthor.InsertEntry(rString,nPos);
}

String SvxTPFilter::GetSelectedAuthor()	const
{
	return aLbAuthor.GetSelectEntry();
}

sal_uInt16	SvxTPFilter::GetSelectedAuthorPos()
{
	return (sal_uInt16) aLbAuthor.GetSelectEntryPos();
}

void SvxTPFilter::SelectedAuthorPos(sal_uInt16 nPos)
{
	aLbAuthor.SelectEntryPos(nPos);
}

sal_uInt16 SvxTPFilter::SelectAuthor(const String& aString)
{
	aLbAuthor.SelectEntry(aString);
	return aLbAuthor.GetSelectEntryPos();
}

void SvxTPFilter::SetRange(const String& rString)
{
	aEdRange.SetText(rString);
}

String SvxTPFilter::GetRange() const
{
	return aEdRange.GetText();
}

void SvxTPFilter::SetFocusToRange()
{
	aEdRange.GrabFocus();
}

void SvxTPFilter::HideRange(sal_Bool bHide)
{
	if(bHide)
	{
		aCbRange.Hide();
		aEdRange.Hide();
		aBtnRange.Hide();
	}
	else
	{
		ShowAction(sal_False);
		aCbRange.SetText(aRangeStr);
		aCbRange.Show();
		aEdRange.Show();
		aBtnRange.Show();
	}
}

void SvxTPFilter::HideClocks(sal_Bool bHide)
{
	if(bHide)
	{
		aIbClock. Hide();
		aIbClock2.Hide();
	}
	else
	{
		aIbClock. Show();
		aIbClock2.Show();
	}
}

void SvxTPFilter::SetComment(const String &rComment)
{
	aEdComment.SetText(rComment);
}
String SvxTPFilter::GetComment()const
{
	return aEdComment.GetText();
}

sal_Bool SvxTPFilter::IsDate()
{
	return aCbDate.IsChecked();
}

sal_Bool SvxTPFilter::IsAuthor()
{
	return aCbAuthor.IsChecked();
}

sal_Bool SvxTPFilter::IsRange()
{
	return aCbRange.IsChecked();
}
sal_Bool SvxTPFilter::IsAction()
{
	return aCbRange.IsChecked();
}

sal_Bool SvxTPFilter::IsComment()
{
	return aCbComment.IsChecked();
}

void SvxTPFilter::CheckDate(sal_Bool bFlag)
{
	aCbDate.Check(bFlag);
	RowEnableHdl(&aCbDate);
	bModified=sal_False;
}

void SvxTPFilter::CheckAuthor(sal_Bool bFlag)
{
	aCbAuthor.Check(bFlag);
	RowEnableHdl(&aCbAuthor);
	bModified=sal_False;
}

void SvxTPFilter::CheckRange(sal_Bool bFlag)
{
	aCbRange.Check(bFlag);
	RowEnableHdl(&aCbRange);
	bModified=sal_False;
}

void SvxTPFilter::CheckAction(sal_Bool bFlag)
{
	aCbRange.Check(bFlag);
	RowEnableHdl(&aCbRange);
	bModified=sal_False;
}

void SvxTPFilter::CheckComment(sal_Bool bFlag)
{
	aCbComment.Check(bFlag);
	RowEnableHdl(&aCbComment);
	bModified=sal_False;
}

void SvxTPFilter::ShowAction(sal_Bool bShow)
{
	if(!bShow)
	{
		aCbRange.Hide();
		aLbAction.Hide();
		aCbRange.SetHelpId(HID_REDLINING_FILTER_CB_RANGE);
	}
	else
	{
		HideRange();
		aCbRange.SetText(aActionStr);
		aCbRange.SetHelpId(HID_REDLINING_FILTER_CB_ACTION);
		aCbRange.Show();
		aLbAction.Show();

	}
}

ListBox* SvxTPFilter::GetLbAction()
{
	return &aLbAction;
}

IMPL_LINK( SvxTPFilter, SelDateHdl, ListBox*, pLb )
{
	ShowDateFields((sal_uInt16)aLbDate.GetSelectEntryPos());
	ModifyHdl(pLb);
	return 0;
}

IMPL_LINK( SvxTPFilter, RowEnableHdl, CheckBox*, pCB )
{
	if(pCB==&aCbDate)
	{
		aLbDate.Enable(aCbDate.IsChecked());
		aLbDate.Invalidate();
		EnableDateLine1(sal_False);
		EnableDateLine2(sal_False);
		if(aCbDate.IsChecked()) SelDateHdl(&aLbDate);
	}
	else if(pCB==&aCbAuthor)
	{
		aLbAuthor.Enable(aCbAuthor.IsChecked());
		aLbAuthor.Invalidate();
	}
	else if(pCB==&aCbRange)
	{
		aLbAction.Enable(aCbRange.IsChecked());
		aLbAction.Invalidate();
		aEdRange.Enable(aCbRange.IsChecked());
		aBtnRange.Enable(aCbRange.IsChecked());
	}
	else if(pCB==&aCbComment)
	{
		aEdComment.Enable(aCbComment.IsChecked());
		aEdComment.Invalidate();
	}

	ModifyHdl(pCB);
	return 0;
}

IMPL_LINK( SvxTPFilter, TimeHdl, ImageButton*,pIB )
{
	Date aDate;
	Time aTime;
	if(pIB==&aIbClock)
	{
		aDfDate.SetDate(aDate);
		aTfDate.SetTime(aTime);
	}
	else if(pIB==&aIbClock2)
	{
		aDfDate2.SetDate(aDate);
		aTfDate2.SetTime(aTime);
	}
	ModifyHdl(&aDfDate);
	return 0;
}

IMPL_LINK( SvxTPFilter, ModifyHdl, void*, pCtr)
{
	if(pCtr!=NULL)
	{
		if(pCtr==&aCbDate  || pCtr==&aLbDate ||
		   pCtr==&aDfDate  || pCtr==&aTfDate ||
		   pCtr==&aIbClock || pCtr==&aFtDate2||
		   pCtr==&aDfDate2 || pCtr==&aTfDate2||
		   pCtr==&aIbClock2)
		{
			aModifyDateLink.Call(this);
		}
		else if(pCtr==&aCbAuthor || pCtr==&aLbAuthor)
		{
			aModifyAuthorLink.Call(this);
		}
		else if(pCtr==&aCbRange  || pCtr==&aEdRange ||
				pCtr==&aBtnRange )
		{
			aModifyRefLink.Call(this);
		}
		else if(pCtr==&aCbComment || pCtr==&aEdComment)
		{
			aModifyComLink.Call(this);
		}

		bModified=sal_True;
		aModifyLink.Call(this);
	}
	return 0;
}

void SvxTPFilter::DeactivatePage()
{
	if(bModified)
	{
		if(pRedlinTable!=NULL)
		{
			pRedlinTable->SetFilterDate(IsDate());
			pRedlinTable->SetDateTimeMode(GetDateMode());
			pRedlinTable->SetFirstDate(aDfDate.GetDate());
			pRedlinTable->SetLastDate(aDfDate2.GetDate());
			pRedlinTable->SetFirstTime(aTfDate.GetTime());
			pRedlinTable->SetLastTime(aTfDate2.GetTime());
			pRedlinTable->SetFilterAuthor(IsAuthor());
			pRedlinTable->SetAuthor(GetSelectedAuthor());

			pRedlinTable->SetFilterComment(IsComment());

			utl::SearchParam aSearchParam( aEdComment.GetText(),
					utl::SearchParam::SRCH_REGEXP,sal_False,sal_False,sal_False );

			pRedlinTable->SetCommentParams(&aSearchParam);

			pRedlinTable->UpdateFilterTest();
		}

		aReadyLink.Call(this);
	}
	bModified=sal_False;
	TabPage::DeactivatePage();
}

void SvxTPFilter::Enable( bool bEnable, bool bChild)
{
	TabPage::Enable(bEnable,bChild);
	if(aCbDate.IsEnabled())
	{
		RowEnableHdl(&aCbDate);
		RowEnableHdl(&aCbAuthor);
		RowEnableHdl(&aCbRange);
		RowEnableHdl(&aCbComment);
	}
}
void SvxTPFilter::Disable( bool bChild)
{
	Enable( false, bChild );
}

void SvxTPFilter::SetAccessibleRelationMemberOf( Window* pWin )
{
	aCbDate.SetAccessibleRelationMemberOf(pWin);
	aLbDate.SetAccessibleRelationMemberOf(pWin);
	aDfDate.SetAccessibleRelationMemberOf(pWin);
	aTfDate.SetAccessibleRelationMemberOf(pWin);
	aIbClock.SetAccessibleRelationMemberOf(pWin);
	aFtDate2.SetAccessibleRelationMemberOf(pWin);
	aDfDate2.SetAccessibleRelationMemberOf(pWin);
	aTfDate2.SetAccessibleRelationMemberOf(pWin);
	aIbClock2.SetAccessibleRelationMemberOf(pWin);
	aCbAuthor.SetAccessibleRelationMemberOf(pWin);
	aLbAuthor.SetAccessibleRelationMemberOf(pWin);
	aCbRange.SetAccessibleRelationMemberOf(pWin);
	aEdRange.SetAccessibleRelationMemberOf(pWin);
	aBtnRange.SetAccessibleRelationMemberOf(pWin);
	aLbAction.SetAccessibleRelationMemberOf(pWin);
	aCbComment.SetAccessibleRelationMemberOf(pWin);
	aEdComment.SetAccessibleRelationMemberOf(pWin);
}

IMPL_LINK( SvxTPFilter, ModifyDate, void*,pTF)
{

	Date aDate;
	Time aTime(0);
	if(&aDfDate==pTF)
	{
		if(aDfDate.GetText().Len()==0)
		   aDfDate.SetDate(aDate);

		if(pRedlinTable!=NULL)
			pRedlinTable->SetFirstDate(aDfDate.GetDate());
	}
	else if(&aDfDate2==pTF)
	{
		if(aDfDate2.GetText().Len()==0)
		   aDfDate2.SetDate(aDate);

		if(pRedlinTable!=NULL)
			pRedlinTable->SetLastDate(aDfDate2.GetDate());
	}
	else if(&aTfDate==pTF)
	{
		if(aTfDate.GetText().Len()==0)
		   aTfDate.SetTime(aTime);

		if(pRedlinTable!=NULL)
			pRedlinTable->SetFirstTime(aTfDate.GetTime());
	}
	else if(&aTfDate2==pTF)
	{
		if(aTfDate2.GetText().Len()==0)
		   aTfDate2.SetTime(aTime);

		if(pRedlinTable!=NULL)
			pRedlinTable->SetLastTime(aTfDate2.GetTime());

	}
	ModifyHdl(&aDfDate);
	return 0;
}

IMPL_LINK( SvxTPFilter, RefHandle, PushButton*, pRef )
{
	if(pRef!=NULL)
	{
		aRefLink.Call(this);
	}
	return 0;
}

static Size gDiffSize;
//============================================================================
//	class SvxAcceptChgCtr
//----------------------------------------------------------------------------

SvxAcceptChgCtr::SvxAcceptChgCtr( Window* pParent, WinBits nWinStyle)
		:	Control(pParent,nWinStyle |WB_DIALOGCONTROL),
			aTCAccept(this,WB_TABSTOP |WB_DIALOGCONTROL)
{
	pTPFilter=new SvxTPFilter(&aTCAccept);
	pTPView=new SvxTPView(&aTCAccept);
	aMinSize=pTPView->GetMinSizePixel();

	aTCAccept.InsertPage( TP_VIEW,	 pTPView->GetMyName());
	aTCAccept.InsertPage( TP_FILTER, pTPFilter->GetMyName());
	aTCAccept.SetTabPage( TP_VIEW,	 pTPView);
	aTCAccept.SetTabPage( TP_FILTER, pTPFilter);
	aTCAccept.SetHelpId(HID_REDLINING_TABCONTROL);

	aTCAccept.SetTabPageSizePixel(aMinSize);
	Size aSize=aTCAccept.GetSizePixel();

	gDiffSize.Height()=aSize.Height()-aMinSize.Height();
	gDiffSize.Width()=aSize.Width()-aMinSize.Width();


	pTPFilter->SetRedlinTable(GetViewTable());

	aTCAccept.Show();
	ShowViewPage();
}

SvxAcceptChgCtr::SvxAcceptChgCtr( Window* pParent, const ResId& rResId )
		:	Control(pParent,rResId ),
			aTCAccept(this,WB_TABSTOP |WB_DIALOGCONTROL)
{
	pTPFilter=new SvxTPFilter(&aTCAccept);
	pTPView=new SvxTPView(&aTCAccept);
	aMinSize=pTPView->GetMinSizePixel();

	aTCAccept.InsertPage( TP_VIEW,	 pTPView->GetMyName());
	aTCAccept.InsertPage( TP_FILTER, pTPFilter->GetMyName());
	aTCAccept.SetTabPage( TP_VIEW,	 pTPView);
	aTCAccept.SetTabPage( TP_FILTER, pTPFilter);
	aTCAccept.SetHelpId(HID_REDLINING_TABCONTROL);

	aTCAccept.SetTabPageSizePixel(aMinSize);
	Size aSize=aTCAccept.GetSizePixel();

	gDiffSize.Height()=aSize.Height()-aMinSize.Height();
	gDiffSize.Width()=aSize.Width()-aMinSize.Width();


	pTPFilter->SetRedlinTable(GetViewTable());
	WinBits nWinStyle=GetStyle()|WB_DIALOGCONTROL;
	SetStyle(nWinStyle);

	aTCAccept.Show();
	ShowViewPage();
	Resize();
}

SvxAcceptChgCtr::~SvxAcceptChgCtr()
{
	delete pTPView;
	delete pTPFilter;
}

void SvxAcceptChgCtr::Resize()
{
	aMinSize=pTPView->GetMinSizePixel();
	Size aSize=GetOutputSizePixel();
	sal_Bool bFlag=sal_False;

	if(aMinSize.Height()>aSize.Height())
	{
		aSize.Height()=aMinSize.Height();
		bFlag=sal_True;
	}
	if(aMinSize.Width()>aSize.Width())
	{
		aSize.Width()=aMinSize.Width();
		bFlag=sal_True;
	}

	if(bFlag)
	{
		SetOutputSizePixel(aSize);
		aMinSizeLink.Call(this);
	}

	aSize.Height()-=2;
	aSize.Width()-=2;
	aTCAccept.SetSizePixel(aSize);
}

Size SvxAcceptChgCtr::GetMinSizePixel() const
{
	Size aSize=pTPView->GetMinSizePixel();
	aSize.Height()+=gDiffSize.Height();
	aSize.Width()+=gDiffSize.Width();
	return aSize;
}

void SvxAcceptChgCtr::ShowFilterPage()
{
	aTCAccept.SetCurPageId(TP_FILTER);
}

void SvxAcceptChgCtr::ShowViewPage()
{
	aTCAccept.SetCurPageId(TP_VIEW);
}

sal_Bool SvxAcceptChgCtr::IsFilterPageVisible()
{
	return (aTCAccept.GetCurPageId()==TP_FILTER);
}

sal_Bool SvxAcceptChgCtr::IsViewPageVisible()
{
	return (aTCAccept.GetCurPageId()==TP_VIEW);
}

SvxTPFilter* SvxAcceptChgCtr::GetFilterPage()
{
	return pTPFilter;
}

SvxTPView* SvxAcceptChgCtr::GetViewPage()
{
	return pTPView;
}

SvxRedlinTable* SvxAcceptChgCtr::GetViewTable()
{
	if(pTPView!=NULL)
	{
		return pTPView->GetTableControl();
	}
	else
	{
		return NULL;
	}
}


