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
#include "precompiled_sc.hxx"

// System - Includes ---------------------------------------------------------



#include <svl/undo.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

// INCLUDE -------------------------------------------------------------------

#include "acredlin.hxx"
#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "acredlin.hrc"
#include "simpref.hxx"
#include "scmod.hxx"
#include "popmenu.hxx"
#include "tabvwsh.hxx"

// defines -------------------------------------------------------------------

#define ABS_SREF		  SCA_VALID \
						| SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF		  ABS_SREF \
						| SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D		ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D		ABS_DREF | SCA_TAB_3D



#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute();

inline void EnableDisable( Window& rWin, sal_Bool bEnable )
{
	if (bEnable)
		rWin.Enable();
	else
		rWin.Disable();
}

#define	RD_SPECIAL_NONE			0
#define	RD_SPECIAL_CONTENT		1
#define	RD_SPECIAL_VISCONTENT	2

//============================================================================
//	class ScRedlinData
//----------------------------------------------------------------------------

ScRedlinData::ScRedlinData()
	:RedlinData()
{
	nInfo=RD_SPECIAL_NONE;
	nActionNo=0;
	pData=NULL;
	bDisabled=sal_False;
	bIsRejectable=sal_False;
	bIsAcceptable=sal_False;
	nTable=SCTAB_MAX;
	nCol=SCCOL_MAX;
	nRow=SCROW_MAX;
}

ScRedlinData::~ScRedlinData()
{
	nInfo=RD_SPECIAL_NONE;
	nActionNo=0;
	pData=NULL;
	bDisabled=sal_False;
	bIsRejectable=sal_False;
	bIsAcceptable=sal_False;
}


//============================================================================
//	class ScAcceptChgDlg
//----------------------------------------------------------------------------
ScAcceptChgDlg::ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
					  ScViewData*		ptrViewData)

	:	SfxModelessDialog( pB, pCW, pParent, ScResId(RID_SCDLG_CHANGES) ),
		//
		aAcceptChgCtr			( this, ScResId( CTR_REDLINING ) ),
		//
		pViewData		( ptrViewData ),
		pDoc			( ptrViewData->GetDocument() ),
		aLocalRangeName ( *(pDoc->GetRangeName()) ),
		//
		aStrInsertCols			( ScResId( STR_INSERT_COLS)),
		aStrInsertRows			( ScResId( STR_INSERT_ROWS)),
		aStrInsertTabs			( ScResId( STR_INSERT_TABS)),
		aStrDeleteCols			( ScResId( STR_DELETE_COLS)),
		aStrDeleteRows			( ScResId( STR_DELETE_ROWS)),
		aStrDeleteTabs			( ScResId( STR_DELETE_TABS)),
		aStrMove				( ScResId( STR_MOVE	    )),
		aStrContent				( ScResId( STR_CONTENT	)),
		aStrReject				( ScResId( STR_REJECT	    )),
		aStrAllAccepted			( ScResId( STR_ACCEPTED	)),
		aStrAllRejected			( ScResId( STR_REJECTED	)),
		aStrNoEntry				( ScResId( STR_NO_ENTRY   )),
		aStrContentWithChild	( ScResId( STR_CONTENT_WITH_CHILD)),
		aStrChildContent		( ScResId( STR_CHILD_CONTENT)),
		aStrChildOrgContent		( ScResId( STR_CHILD_ORGCONTENT)),
		aStrEmpty				( ScResId( STR_EMPTY		))
{
	FreeResource();
//	bScAcceptChgDlgIsDead=sal_False;
	bNoSelection=sal_False;
	bNeedsUpdate=sal_False;
	bIgnoreMsg=sal_False;
	nAcceptCount=0;
	nRejectCount=0;
	bAcceptEnableFlag=sal_True;
	bRejectEnableFlag=sal_True;
	bHasFilterEntry=sal_False;
	bUseColor=sal_False;
	aReOpenTimer.SetTimeout(50);
	aReOpenTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, ReOpenTimerHdl ));

	//	dialog is now only hidden, not deleted, on switching views,
	//	so there's no need to restore settings when reopening

	MinSize=aAcceptChgCtr.GetMinSizePixel();
	MinSize.Height()+=2;
	MinSize.Width()+=2;
	SetMinOutputSizePixel(MinSize);
	aUnknown.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Unknown"));

	pTPFilter=aAcceptChgCtr.GetFilterPage();
	pTPView=aAcceptChgCtr.GetViewPage();
	pTheView=pTPView->GetTableControl();
	aSelectionTimer.SetTimeout(100);
	aSelectionTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, UpdateSelectionHdl ));

	pTPFilter->SetReadyHdl(LINK( this, ScAcceptChgDlg, FilterHandle ));
	pTPFilter->SetRefHdl(LINK( this, ScAcceptChgDlg, RefHandle ));
	pTPFilter->SetModifyHdl(LINK( this, ScAcceptChgDlg, FilterModified));
	pTPFilter->HideRange(sal_False);
	pTPView->InsertCalcHeader();
	pTPView->SetRejectClickHdl( LINK( this, ScAcceptChgDlg,RejectHandle));
	pTPView->SetAcceptClickHdl( LINK(this, ScAcceptChgDlg, AcceptHandle));
	pTPView->SetRejectAllClickHdl( LINK( this, ScAcceptChgDlg,RejectAllHandle));
	pTPView->SetAcceptAllClickHdl( LINK(this, ScAcceptChgDlg, AcceptAllHandle));
	pTheView->SetCalcView();
	pTheView->SetStyle(pTheView->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
	pTheView->SetExpandingHdl( LINK(this, ScAcceptChgDlg, ExpandingHandle));
	pTheView->SetSelectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
	pTheView->SetDeselectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
	pTheView->SetCommandHdl( LINK(this, ScAcceptChgDlg, CommandHdl));
	pTheView->SetColCompareHdl( LINK(this, ScAcceptChgDlg,ColCompareHdl));
	pTheView->SetSelectionMode(MULTIPLE_SELECTION);
	pTheView->SetHighlightRange(1);

	Init();

	aAcceptChgCtr.SetMinSizeHdl( LINK( this, ScAcceptChgDlg, MinSizeHandle ));

	UpdateView();
	SvLBoxEntry* pEntry=pTheView->First();
	if(pEntry!=NULL)
	{
		pTheView->Select(pEntry);
	}
}
ScAcceptChgDlg::~ScAcceptChgDlg()
{
//	bScAcceptChgDlgIsDead=sal_True;
	ClearView();
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	if(pChanges!=NULL)
	{
		Link aLink;
		pChanges->SetModifiedLink(aLink);
	}
}

void ScAcceptChgDlg::ReInit(ScViewData* ptrViewData)
{
	pViewData=ptrViewData;
	if(pViewData!=NULL)
	{
		pDoc=ptrViewData->GetDocument();
	}
	else
	{
		pDoc=NULL;
	}

	bNoSelection=sal_False;
	bNeedsUpdate=sal_False;
	bIgnoreMsg=sal_False;
	nAcceptCount=0;
	nRejectCount=0;
	bAcceptEnableFlag=sal_True;
	bRejectEnableFlag=sal_True;

	//	#91781# don't call Init here (switching between views), just set link below
	//	(dialog is just hidden, not deleted anymore, when switching views)
	ClearView();
	UpdateView();

	if ( pDoc )
	{
		ScChangeTrack* pChanges = pDoc->GetChangeTrack();
		if ( pChanges )
			pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg, ChgTrackModHdl ) );
	}
}

void __EXPORT ScAcceptChgDlg::Init()
{
	String	aAreaStr;
	ScRange aRange;

	DBG_ASSERT( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	if(pChanges!=NULL)
	{
		pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg,ChgTrackModHdl));
		aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
		pTPFilter->ClearAuthors();
		ScStrCollection aUserColl=pChanges->GetUserCollection();
		for(sal_uInt16	i=0;i<aUserColl.GetCount();i++)
			pTPFilter->InsertAuthor(aUserColl[i]->GetString());
	}

	ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();
	if ( pViewSettings!=NULL )
		aChangeViewSet = *pViewSettings;
    // adjust TimeField for filter tabpage
    aChangeViewSet.AdjustDateMode( *pDoc );

	pTPFilter->CheckDate(aChangeViewSet.HasDate());
	pTPFilter->SetFirstDate(aChangeViewSet.GetTheFirstDateTime());
	pTPFilter->SetFirstTime(aChangeViewSet.GetTheFirstDateTime());
	pTPFilter->SetLastDate(aChangeViewSet.GetTheLastDateTime());
	pTPFilter->SetLastTime(aChangeViewSet.GetTheLastDateTime());
	pTPFilter->SetDateMode((sal_uInt16)aChangeViewSet.GetTheDateMode());
	pTPFilter->CheckComment(aChangeViewSet.HasComment());
	pTPFilter->SetComment(aChangeViewSet.GetTheComment());

	pTPFilter->CheckAuthor(aChangeViewSet.HasAuthor());
	String aString=aChangeViewSet.GetTheAuthorToShow();
	if(aString.Len()!=0)
	{
		pTPFilter->SelectAuthor(aString);
		if(pTPFilter->GetSelectedAuthor()!=aString)
		{
			pTPFilter->InsertAuthor(aString);
			pTPFilter->SelectAuthor(aString);
		}
	}
	else
	{
		pTPFilter->SelectedAuthorPos(0);
	}

	pTPFilter->CheckRange(aChangeViewSet.HasRange());

	ScRange* pRangeEntry=aChangeViewSet.GetTheRangeList().GetObject(0);
	aRangeList=aChangeViewSet.GetTheRangeList();

	if(pRangeEntry!=NULL)
	{
		String aRefStr;
		pRangeEntry->Format( aRefStr, ABS_DREF3D, pDoc );
		pTPFilter->SetRange(aRefStr);
	}

	Point aPoint(1,1);
	aAcceptChgCtr.SetPosPixel(aPoint);
	InitFilter();
}



void ScAcceptChgDlg::ClearView()
{
	nAcceptCount=0;
	nRejectCount=0;
	pTheView->SetUpdateMode(sal_False);

	pTheView->Clear();
	pTheView->SetUpdateMode(sal_True);
}

String* ScAcceptChgDlg::MakeTypeString(ScChangeActionType eType)
{
	String* pStr;

	switch(eType)
	{

		case SC_CAT_INSERT_COLS:	pStr=&aStrInsertCols;break;

		case SC_CAT_INSERT_ROWS:	pStr=&aStrInsertRows;break;

		case SC_CAT_INSERT_TABS:	pStr=&aStrInsertTabs;break;

		case SC_CAT_DELETE_COLS:	pStr=&aStrDeleteCols;break;

		case SC_CAT_DELETE_ROWS:	pStr=&aStrDeleteRows;break;

		case SC_CAT_DELETE_TABS:	pStr=&aStrDeleteTabs;break;

		case SC_CAT_MOVE:			pStr=&aStrMove;break;

		case SC_CAT_CONTENT:		pStr=&aStrContent;break;

		case SC_CAT_REJECT:			pStr=&aStrReject;break;

		default:					pStr=&aUnknown;break;
	}
	return pStr;
}


sal_Bool ScAcceptChgDlg::IsValidAction(const ScChangeAction* pScChangeAction)
{
	if(pScChangeAction==NULL) return sal_False;

	sal_Bool bFlag=sal_False;

	ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
	String aUser=pScChangeAction->GetUser();
	DateTime aDateTime=pScChangeAction->GetDateTime();

	ScChangeActionType eType=pScChangeAction->GetType();
	String aString;
	String aDesc;

	String aComment=pScChangeAction->GetComment();
	aComment.EraseAllChars('\n');

	if(eType==SC_CAT_CONTENT)
	{
		if(!pScChangeAction->IsDialogParent())
		{
			pScChangeAction->GetDescription( aDesc, pDoc, sal_True);
		}
	}
	else
	{
		pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());
	}

	if(aDesc.Len()>0)
	{
		aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
		aComment+=aDesc;
		aComment+=')';
	}

	if(pTheView->IsValidEntry(&aUser,&aDateTime,&aComment))
	{
		if(pTPFilter->IsRange())
		{
			ScRange* pRangeEntry=aRangeList.First();

			while(pRangeEntry!=NULL)
			{
				if(pRangeEntry->Intersects(aRef)) break;
				pRangeEntry=aRangeList.Next();
			}

			if(pRangeEntry!=NULL)
			{
				bFlag=sal_True;
			}
		}
		else
		{
			bFlag=sal_True;
		}
	}

	return bFlag;
}

SvLBoxEntry* ScAcceptChgDlg::InsertChangeAction(const ScChangeAction* pScChangeAction,
                                                   ScChangeActionState /* eState */, SvLBoxEntry* pParent,
												   sal_Bool bDelMaster,sal_Bool bDisabled,sal_uLong nPos)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	if(pScChangeAction==NULL || pChanges==NULL) return NULL;

	SvLBoxEntry* pEntry=NULL;

	sal_Bool bFlag=sal_False;

	ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
	String aUser=pScChangeAction->GetUser();
	DateTime aDateTime=pScChangeAction->GetDateTime();

	String aRefStr;
	ScChangeActionType eType=pScChangeAction->GetType();
	String aString;
	String aDesc;

	ScRedlinData* pNewData=new ScRedlinData;
	pNewData->pData=(void *)pScChangeAction;
	pNewData->nActionNo=pScChangeAction->GetActionNumber();
	pNewData->bIsAcceptable=pScChangeAction->IsClickable();
	pNewData->bIsRejectable=pScChangeAction->IsRejectable();
	pNewData->bDisabled=!pNewData->bIsAcceptable | bDisabled;
	pNewData->aDateTime=aDateTime;
	pNewData->nRow	= aRef.aStart.Row();
	pNewData->nCol	= aRef.aStart.Col();
	pNewData->nTable= aRef.aStart.Tab();

	if(eType==SC_CAT_CONTENT)
	{
		if(pScChangeAction->IsDialogParent())
		{
			aString=aStrContentWithChild;
			pNewData->nInfo=RD_SPECIAL_VISCONTENT;
			pNewData->bIsRejectable=sal_False;
			pNewData->bIsAcceptable=sal_False;
		}
		else
		{
			aString=*MakeTypeString(eType);
			pScChangeAction->GetDescription( aDesc, pDoc, sal_True);
		}
	}
	else
	{
		aString=*MakeTypeString(eType);

		if(bDelMaster)
		{
			pScChangeAction->GetDescription( aDesc, pDoc,sal_True);
			pNewData->bDisabled=sal_True;
			pNewData->bIsRejectable=sal_False;
		}
		else
			pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

	}

	aString+='\t';
	pScChangeAction->GetRefString(aRefStr, pDoc, sal_True);
	aString+=aRefStr;
	aString+='\t';

	sal_Bool bIsGenerated;

	if(!pChanges->IsGenerated(pScChangeAction->GetActionNumber()))
	{
		aString+=aUser;
		aString+='\t';

        aString+=ScGlobal::pLocaleData->getDate(aDateTime);
		aString+=' ';
        aString+=ScGlobal::pLocaleData->getTime(aDateTime);
		aString+='\t';
		bIsGenerated=sal_False;
	}
	else
	{
		aString+='\t';
		aString+='\t';
		bIsGenerated=sal_True;
	}

	String aComment=pScChangeAction->GetComment();
	aComment.EraseAllChars('\n');
	if(aDesc.Len()>0)
	{
		aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
		aComment+=aDesc;
		aComment+=')';
	}

	aString+=aComment;

	if(pTheView->IsValidEntry(&aUser,&aDateTime)|| bIsGenerated)
	{
		if(pTheView->IsValidComment(&aComment))
		{
			if(pTPFilter->IsRange())
			{
				ScRange* pRangeEntry=aRangeList.First();

				while(pRangeEntry!=NULL)
				{
					if(pRangeEntry->Intersects(aRef)) break;
					pRangeEntry=aRangeList.Next();
				}
				//SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
				if(pRangeEntry!=NULL)
				{
					bHasFilterEntry=sal_True;
					bFlag=sal_True;
				}
			}
			else if(!bIsGenerated)
			{
				bHasFilterEntry=sal_True;
				bFlag=sal_True;
			}
		}
	}

	if(!bFlag&& bUseColor&& pParent==NULL)
	{
		pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_LIGHTBLUE),pParent,nPos);
	}
	else if(bFlag&& bUseColor&& pParent!=NULL)
	{
		pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_GREEN),pParent,nPos);
		SvLBoxEntry* pExpEntry=pParent;

		while(pExpEntry!=NULL && !pTheView->IsExpanded(pExpEntry))
		{
			SvLBoxEntry* pTmpEntry=pTheView->GetParent(pExpEntry);

			if(pTmpEntry!=NULL) pTheView->Expand(pExpEntry);

			pExpEntry=pTmpEntry;
		}
	}
	else
	{
		pEntry=pTheView->InsertEntry(aString,pNewData,pParent,nPos);
	}
	return pEntry;
}

SvLBoxEntry* ScAcceptChgDlg::InsertFilteredAction(const ScChangeAction* pScChangeAction,
												   ScChangeActionState eState,SvLBoxEntry* pParent,
												   sal_Bool bDelMaster,sal_Bool bDisabled,sal_uLong nPos)
{

	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	if(pScChangeAction==NULL || pChanges==NULL) return NULL;

	sal_Bool bIsGenerated=pChanges->IsGenerated(pScChangeAction->GetActionNumber());

	SvLBoxEntry* pEntry=NULL;

	int bFlag=sal_False;

	ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
	String aUser=pScChangeAction->GetUser();
	DateTime aDateTime=pScChangeAction->GetDateTime();

	if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
	{
		if(pTPFilter->IsRange())
		{
			ScRange* pRangeEntry=aRangeList.First();

			while(pRangeEntry!=NULL)
			{
				if(pRangeEntry->Intersects(aRef)) break;
				pRangeEntry=aRangeList.Next();
			}
			//SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
			if(pRangeEntry!=NULL &&
				pScChangeAction->GetState()==eState)
			{
				bFlag=sal_True;
			}
		}
		else if(pScChangeAction->GetState()==eState && !bIsGenerated)
		{
			bFlag=sal_True;
		}
	}

	if(bFlag)
	{

		String aRefStr;
		ScChangeActionType eType=pScChangeAction->GetType();
		String aString;
		String aDesc;


		ScRedlinData* pNewData=new ScRedlinData;
		pNewData->pData=(void *)pScChangeAction;
		pNewData->nActionNo=pScChangeAction->GetActionNumber();
		pNewData->bIsAcceptable=pScChangeAction->IsClickable();
		pNewData->bIsRejectable=pScChangeAction->IsRejectable();
		pNewData->bDisabled=!pNewData->bIsAcceptable | bDisabled;
		pNewData->aDateTime=aDateTime;
		pNewData->nRow	= aRef.aStart.Row();
		pNewData->nCol	= aRef.aStart.Col();
		pNewData->nTable= aRef.aStart.Tab();

		if(eType==SC_CAT_CONTENT)
		{
			if(pScChangeAction->IsDialogParent())
			{
				aString=aStrContentWithChild;
				pNewData->nInfo=RD_SPECIAL_VISCONTENT;
				pNewData->bIsRejectable=sal_False;
				pNewData->bIsAcceptable=sal_False;
			}
			else
			{
				aString=*MakeTypeString(eType);
				pScChangeAction->GetDescription( aDesc, pDoc, sal_True);
			}
		}
		else
		{
			aString=*MakeTypeString(eType);

			if(bDelMaster)
			{
				pScChangeAction->GetDescription( aDesc, pDoc,sal_True);
				pNewData->bDisabled=sal_True;
				pNewData->bIsRejectable=sal_False;
			}
			else
				pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

		}

		aString+='\t';
		pScChangeAction->GetRefString(aRefStr, pDoc, sal_True);
		aString+=aRefStr;
		aString+='\t';

		if(!bIsGenerated)
		{
			aString+=aUser;
			aString+='\t';
            aString+=ScGlobal::pLocaleData->getDate(aDateTime);
			aString+=' ';
            aString+=ScGlobal::pLocaleData->getTime(aDateTime);
			aString+='\t';
		}
		else
		{
			aString+='\t';
			aString+='\t';
		}

		String aComment=pScChangeAction->GetComment();
		aComment.EraseAllChars('\n');
		if(aDesc.Len()>0)
		{
			aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
			aComment+=aDesc;
			aComment+=')';
		}
		if(pTheView->IsValidComment(&aComment))
		{
			aString+=aComment;
			pEntry=pTheView->InsertEntry(aString,pNewData,pParent,nPos);
		}
		else
			delete pNewData;
	}
	return pEntry;
}

SvLBoxEntry* ScAcceptChgDlg::InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
														  SvLBoxEntry* pParent, sal_uLong nSpecial)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	SvLBoxEntry* pEntry=NULL;

	if(pScChangeAction==NULL || pChanges==NULL) return NULL;

	sal_Bool bIsGenerated=pChanges->IsGenerated(pScChangeAction->GetActionNumber());

	sal_Bool bFlag=sal_False;

	ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
	String aUser=pScChangeAction->GetUser();
	DateTime aDateTime=pScChangeAction->GetDateTime();

	if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
	{
		if(pTPFilter->IsRange())
		{
			ScRange* pRangeEntry=aRangeList.First();

			while(pRangeEntry!=NULL)
			{
				if(pRangeEntry->Intersects(aRef)) break;
				pRangeEntry=aRangeList.Next();
			}
			//SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
			if(pRangeEntry!=NULL)
			{
				bFlag=sal_True;
			}
		}
		else if(!bIsGenerated)
			bFlag=sal_True;
	}

	String aRefStr;
	String aString;
	String a2String;
	String aDesc;

	if(nSpecial==RD_SPECIAL_CONTENT)
	{
		pScChangeAction->GetOldString(a2String);
		if(a2String.Len()==0) a2String=aStrEmpty;

		//aString+="\'";
		aString+=a2String;
		//aString+="\'";

		aDesc=aStrChildOrgContent;
		aDesc.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
	}
	else
	{
		pScChangeAction->GetNewString(a2String);
		if(a2String.Len()==0)
		{
			a2String=aStrEmpty;
			aString+=a2String;
		}
		else
		{
			aString+='\'';
			aString+=a2String;
			aString+='\'';
			a2String=aString;
		}
		aDesc=aStrChildContent;

	}

	aDesc+=a2String;
	aString+='\t';
	pScChangeAction->GetRefString(aRefStr, pDoc, sal_True);
	aString+=aRefStr;
	aString+='\t';

	if(!bIsGenerated)
	{
		aString+=aUser;
		aString+='\t';

        aString+=ScGlobal::pLocaleData->getDate(aDateTime);
		aString+=' ';
        aString+=ScGlobal::pLocaleData->getTime(aDateTime);
		aString+='\t';
	}
	else
	{
		aString+='\t';
		aString+='\t';
	}
	String aComment=pScChangeAction->GetComment();
	aComment.EraseAllChars('\n');

	if(aDesc.Len()>0)
	{
		aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
		aComment+=aDesc;
		aComment+=')';
	}

	aString+=aComment;

	ScRedlinData* pNewData=new ScRedlinData;
	pNewData->nInfo=nSpecial;
	pNewData->pData=(void *)pScChangeAction;
	pNewData->nActionNo=pScChangeAction->GetActionNumber();
	pNewData->bIsAcceptable=pScChangeAction->IsClickable();
	pNewData->bIsRejectable=sal_False;
	pNewData->bDisabled=!pNewData->bIsAcceptable;
	pNewData->aDateTime=aDateTime;
	pNewData->nRow	= aRef.aStart.Row();
	pNewData->nCol	= aRef.aStart.Col();
	pNewData->nTable= aRef.aStart.Tab();

	if(pTheView->IsValidComment(&aComment) && bFlag)
	{
		bHasFilterEntry=sal_True;
		pEntry=pTheView->InsertEntry(aString,pNewData,pParent);
	}
	else
		pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_LIGHTBLUE),pParent);
	return pEntry;
}

long ScAcceptChgDlg::PreNotify( NotifyEvent& rNEvt )
{
	if(rNEvt.GetType()==EVENT_GETFOCUS && bNeedsUpdate)
	{
		ClearView();
		UpdateView();
		bNoSelection=sal_False;
	}

	return SfxModelessDialog::PreNotify(rNEvt);
}


void ScAcceptChgDlg::UpdateView()
{
	bNeedsUpdate=sal_False;
	DateTime aDateTime;
	SvLBoxEntry* pParent=NULL;
	ScChangeTrack* pChanges=NULL;
	const ScChangeAction* pScChangeAction=NULL;
	bAcceptEnableFlag=sal_True;
	bRejectEnableFlag=sal_True;
	SetPointer(Pointer(POINTER_WAIT));
	pTheView->SetUpdateMode(sal_False);
	sal_Bool bFilterFlag=pTPFilter->IsDate()||pTPFilter->IsRange()||
					 pTPFilter->IsAuthor()||pTPFilter->IsComment();

	bUseColor=bFilterFlag;

	if(pDoc!=NULL)
	{
		pChanges=pDoc->GetChangeTrack();
		if(pChanges!=NULL)
		{
			pScChangeAction=pChanges->GetFirst();
		}
	}
	ScChangeActionTable ActionTable;
	sal_Bool bTheFlag=sal_False;

	while(pScChangeAction!=NULL)
	{
		bHasFilterEntry=sal_False;
		switch(pScChangeAction->GetState())
		{
			case SC_CAS_VIRGIN:

				if(pScChangeAction->IsDialogRoot())
				{
					if(pScChangeAction->IsDialogParent())
						pParent=InsertChangeAction(pScChangeAction,SC_CAS_VIRGIN);
					else
						pParent=InsertFilteredAction(pScChangeAction,SC_CAS_VIRGIN);
				}
				else
					pParent=NULL;

				bTheFlag=sal_True;
				break;

			case SC_CAS_ACCEPTED:
				pParent=NULL;
				nAcceptCount++;
				break;

			case SC_CAS_REJECTED:
				pParent=NULL;
				nRejectCount++;
				break;
		}

		if(pParent!=NULL && pScChangeAction->IsDialogParent())
		{
			if(!bFilterFlag)
			{
				pParent->EnableChildsOnDemand(sal_True);
			}
			else
			{
				sal_Bool bTestFlag=bHasFilterEntry;
				bHasFilterEntry=sal_False;
				if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                    pTheView->RemoveEntry(pParent);
			}
		}

		pScChangeAction=pScChangeAction->GetNext();
	}

    if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
		bTheFlag=sal_False;

	pTPView->EnableAccept(bTheFlag);
	pTPView->EnableAcceptAll(bTheFlag);
	pTPView->EnableReject(bTheFlag);
	pTPView->EnableRejectAll(bTheFlag);

	if(nAcceptCount>0)
	{
		pParent=pTheView->InsertEntry(
            aStrAllAccepted, static_cast< RedlinData * >(NULL),
            static_cast< SvLBoxEntry * >(NULL));
		pParent->EnableChildsOnDemand(sal_True);
	}
	if(nRejectCount>0)
	{
		pParent=pTheView->InsertEntry(
            aStrAllRejected, static_cast< RedlinData * >(NULL),
            static_cast< SvLBoxEntry * >(NULL));
		pParent->EnableChildsOnDemand(sal_True);
	}
	pTheView->SetUpdateMode(sal_True);
	SetPointer(Pointer(POINTER_ARROW));
	SvLBoxEntry* pEntry=pTheView->First();
	if(pEntry!=NULL)
	{
		pTheView->Select(pEntry);
	}
}

//----------------------------------------------------------------------------
sal_Bool ScAcceptChgDlg::Close()
{
	return SfxModelessDialog::Close();
}

void ScAcceptChgDlg::Resize()
{
	SfxModelessDialog::Resize();
    Size aOutSize=GetOutputSizePixel();
    aAcceptChgCtr.SetSizePixel(aOutSize);
}

IMPL_LINK( ScAcceptChgDlg, MinSizeHandle, SvxAcceptChgCtr*, pCtr )
{
	if(pCtr==&aAcceptChgCtr)
	{
		if(!IsRollUp())
			SetOutputSizePixel(MinSize);
	}
	return 0;
}

IMPL_LINK( ScAcceptChgDlg, RefHandle, SvxTPFilter*, EMPTYARG )
{
	sal_uInt16 nId  =ScSimpleRefDlgWrapper::GetChildWindowId();

	ScSimpleRefDlgWrapper::SetDefaultPosSize(GetPosPixel(),GetSizePixel(),sal_True);

	SC_MOD()->SetRefDialog( nId, sal_True );

	SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
	ScSimpleRefDlgWrapper* pWnd =(ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

	if(pWnd!=NULL)
	{
//		bSimpleRefDlgStarted=sal_True;
		sal_uInt16 nAcceptId=ScAcceptChgDlgWrapper::GetChildWindowId();
		pViewFrm->ShowChildWindow(nAcceptId,sal_False);
		pWnd->SetCloseHdl(LINK( this, ScAcceptChgDlg,RefInfoHandle));
		pWnd->SetRefString(pTPFilter->GetRange());
		pWnd->SetAutoReOpen(sal_False);
		Window* pWin=pWnd->GetWindow();
		pWin->SetPosSizePixel(GetPosPixel(),GetSizePixel());
		Hide();
		pWin->SetText(GetText());
		pWnd->StartRefInput();
	}
	return 0;
}

IMPL_LINK( ScAcceptChgDlg, RefInfoHandle, String*, pResult)
{
	sal_uInt16 nId;

//	bSimpleRefDlgStarted=sal_False;
	ScSimpleRefDlgWrapper::SetAutoReOpen(sal_True);

	SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
	if(pResult!=NULL)
	{
		pTPFilter->SetRange(*pResult);
		FilterHandle(pTPFilter);

		nId = ScSimpleRefDlgWrapper::GetChildWindowId();
		ScSimpleRefDlgWrapper* pWnd = (ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

		if(pWnd!=NULL)
		{
			Window* pWin=pWnd->GetWindow();
            Size aWinSize=pWin->GetSizePixel();
            aWinSize.Width()=GetSizePixel().Width();
            SetPosSizePixel(pWin->GetPosPixel(),aWinSize);
			Invalidate();
		}
		nId = ScAcceptChgDlgWrapper::GetChildWindowId();
		pViewFrm->ShowChildWindow( nId, sal_True );
	}
	else
	{
		nId = ScAcceptChgDlgWrapper::GetChildWindowId();
		pViewFrm->SetChildWindow( nId, sal_False );
	}
	return 0;
}

IMPL_LINK( ScAcceptChgDlg, FilterHandle, SvxTPFilter*, pRef )
{
	if(pRef!=NULL)
	{
		ClearView();
		aRangeList.Clear();
		aRangeList.Parse(pTPFilter->GetRange(),pDoc);
		UpdateView();
	}
	return 0;
}

IMPL_LINK( ScAcceptChgDlg, RejectHandle, SvxTPView*, pRef )
{
	SetPointer(Pointer(POINTER_WAIT));

	bIgnoreMsg=sal_True;
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	if(pRef!=NULL)
	{
		SvLBoxEntry* pEntry=pTheView->FirstSelected();
		while(pEntry!=NULL)
		{
			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			if(pEntryData!=NULL)
			{
				ScChangeAction* pScChangeAction=
						(ScChangeAction*) pEntryData->pData;

				if(pScChangeAction->GetType()==SC_CAT_INSERT_TABS)
				{
					pViewData->SetTabNo(0);
				}
				pChanges->Reject(pScChangeAction);
			}
			pEntry = pTheView->NextSelected(pEntry);
		}
		ScDocShell* pDocSh=pViewData->GetDocShell();
		pDocSh->PostPaintExtras();
		pDocSh->PostPaintGridAll();
		pDocSh->GetUndoManager()->Clear();
		pDocSh->SetDocumentModified();
		ClearView();
		UpdateView();
	}
	SetPointer(Pointer(POINTER_ARROW));

	bIgnoreMsg=sal_False;
	return 0;
}
IMPL_LINK( ScAcceptChgDlg, AcceptHandle, SvxTPView*, pRef )
{
	SetPointer(Pointer(POINTER_WAIT));

	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	bIgnoreMsg=sal_True;
	if(pRef!=NULL)
	{
		SvLBoxEntry* pEntry=pTheView->FirstSelected();
		while(pEntry!=NULL)
		{
			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			if(pEntryData!=NULL)
			{
				ScChangeAction* pScChangeAction=
						(ScChangeAction*) pEntryData->pData;
				if(pScChangeAction->GetType()==SC_CAT_CONTENT)
				{
					if(pEntryData->nInfo==RD_SPECIAL_CONTENT)
					{
						pChanges->SelectContent(pScChangeAction,sal_True);
					}
					else
					{
						pChanges->SelectContent(pScChangeAction);
					}
				}
				else
					pChanges->Accept(pScChangeAction);
			}
			pEntry = pTheView->NextSelected(pEntry);
		}
		ScDocShell* pDocSh=pViewData->GetDocShell();
		pDocSh->PostPaintExtras();
		pDocSh->PostPaintGridAll();
		pDocSh->SetDocumentModified();
		ClearView();
		UpdateView();
	}
	bIgnoreMsg=sal_False;

	return 0;
}

void ScAcceptChgDlg::RejectFiltered()
{
	if(pDoc==NULL) return;
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	const ScChangeAction* pScChangeAction=NULL;

	if(pChanges!=NULL)
	{
		pScChangeAction=pChanges->GetLast();
	}

	while(pScChangeAction!=NULL)
	{
		if(pScChangeAction->IsDialogRoot())
		{
			if(IsValidAction(pScChangeAction))
			{
				pChanges->Reject((ScChangeAction*)pScChangeAction);
			}
		}
		pScChangeAction=pScChangeAction->GetPrev();
	}
}
void ScAcceptChgDlg::AcceptFiltered()
{
	if(pDoc==NULL) return;
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	const ScChangeAction* pScChangeAction=NULL;

	if(pChanges!=NULL)
	{
		pScChangeAction=pChanges->GetLast();
	}

	while(pScChangeAction!=NULL)
	{
		if(pScChangeAction->IsDialogRoot())
		{
			if(IsValidAction(pScChangeAction))
			{
				pChanges->Accept((ScChangeAction*)pScChangeAction);
			}
		}
		pScChangeAction=pScChangeAction->GetPrev();
	}
}

IMPL_LINK( ScAcceptChgDlg, RejectAllHandle, SvxTPView*, EMPTYARG )
{
	SetPointer(Pointer(POINTER_WAIT));
	bIgnoreMsg=sal_True;
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	if(pChanges!=NULL)
	{
		if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
		{
			RejectFiltered();
		}
		else
		{
			pChanges->RejectAll();
		}
		pViewData->SetTabNo(0);

		ScDocShell* pDocSh=pViewData->GetDocShell();
		pDocSh->PostPaintExtras();
		pDocSh->PostPaintGridAll();
		pDocSh->GetUndoManager()->Clear();
		pDocSh->SetDocumentModified();
		ClearView();
		UpdateView();
	}
	SetPointer(Pointer(POINTER_ARROW));

	bIgnoreMsg=sal_False;

	return 0;
}

IMPL_LINK( ScAcceptChgDlg, AcceptAllHandle, SvxTPView*, EMPTYARG )
{
	SetPointer(Pointer(POINTER_WAIT));

	bIgnoreMsg=sal_True;
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	if(pChanges!=NULL)
	{
		if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
		{
			AcceptFiltered();
		}
		else
		{
			pChanges->AcceptAll();
		}
		ScDocShell* pDocSh=pViewData->GetDocShell();
		pDocSh->PostPaintExtras();
		pDocSh->PostPaintGridAll();
		pDocSh->SetDocumentModified();
		ClearView();
		UpdateView();
	}
	bIgnoreMsg=sal_False;
	SetPointer(Pointer(POINTER_ARROW));

	return 0;
}

IMPL_LINK( ScAcceptChgDlg, SelectHandle, SvxRedlinTable*, EMPTYARG )
{
	if(!bNoSelection)
	{
		aSelectionTimer.Start();
	}
	bNoSelection=sal_False;
	return 0;
}

void ScAcceptChgDlg::GetDependents(  const ScChangeAction* pScChangeAction,
									ScChangeActionTable& aActionTable,
									SvLBoxEntry* pEntry)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();

	SvLBoxEntry* pParent=pTheView->GetParent(pEntry);
	if(pParent!=NULL)
	{
		ScRedlinData *pParentData=(ScRedlinData *)(pParent->GetUserData());
		ScChangeAction* pParentAction=(ScChangeAction*) pParentData->pData;
		if(pParentAction!=pScChangeAction)
		{
			pChanges->GetDependents((ScChangeAction*) pScChangeAction,
						aActionTable,pScChangeAction->IsMasterDelete());
		}
		else
		{
			pChanges->GetDependents((ScChangeAction*) pScChangeAction,
						aActionTable);
		}
	}
	else
	{
		pChanges->GetDependents((ScChangeAction*) pScChangeAction,
					aActionTable,pScChangeAction->IsMasterDelete());
	}
}

sal_Bool ScAcceptChgDlg::InsertContentChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
	sal_Bool bTheTestFlag=sal_True;
	ScRedlinData *pEntryData=(ScRedlinData *)(pParent->GetUserData());
	const ScChangeAction* pScChangeAction = (ScChangeAction*) pEntryData->pData;
	sal_Bool bParentInserted = sal_False;
	// If the parent is a MatrixOrigin then place it in the right order before
	// the MatrixReferences. Also if it is the first content change at this
	// position don't insert the first dependent MatrixReference as the special
	// content (original value) but insert the predecessor of the MatrixOrigin
	// itself instead.
	if ( pScChangeAction->GetType() == SC_CAT_CONTENT &&
			((const ScChangeActionContent*)pScChangeAction)->IsMatrixOrigin() )
	{
		pActionTable->Insert( pScChangeAction->GetActionNumber(),
			(ScChangeAction*) pScChangeAction );
		bParentInserted = sal_True;
	}
	SvLBoxEntry* pEntry=NULL;

	const ScChangeActionContent* pCChild=(const ScChangeActionContent*)pActionTable->First();
	while(pCChild!=NULL)
	{
		if( pCChild->GetState()==SC_CAS_VIRGIN )
			break;
		pCChild=(const ScChangeActionContent*)pActionTable->Next();
	}

	if(pCChild==NULL) return sal_True;

	SvLBoxEntry* pOriginal=InsertChangeActionContent(pCChild,pParent,RD_SPECIAL_CONTENT);
	if(pOriginal!=NULL)
	{
		bTheTestFlag=sal_False;
		ScRedlinData *pParentData=(ScRedlinData *)(pOriginal->GetUserData());
		pParentData->pData=(void *)pScChangeAction;
		pParentData->nActionNo=pScChangeAction->GetActionNumber();
		pParentData->bIsAcceptable=pScChangeAction->IsRejectable();	// select old value
		pParentData->bIsRejectable=sal_False;
		pParentData->bDisabled=sal_False;
	}
	while(pCChild!=NULL)
	{
		if(pCChild->GetState()==SC_CAS_VIRGIN)
		{
			pEntry=InsertChangeActionContent(pCChild,pParent,RD_SPECIAL_NONE);

			if(pEntry!=NULL)
				bTheTestFlag=sal_False;
		}
		pCChild=(const ScChangeActionContent*)pActionTable->Next();
	}

	if ( !bParentInserted )
	{
		pEntry=InsertChangeActionContent((const ScChangeActionContent*)
								pScChangeAction,pParent,RD_SPECIAL_NONE);

		if(pEntry!=NULL)
		{
			bTheTestFlag=sal_False;
			ScRedlinData *pParentData=(ScRedlinData *)(pEntry->GetUserData());
			pParentData->pData=(void *)pScChangeAction;
			pParentData->nActionNo=pScChangeAction->GetActionNumber();
			pParentData->bIsAcceptable=pScChangeAction->IsClickable();
			pParentData->bIsRejectable=sal_False;
			pParentData->bDisabled=sal_False;
		}
	}

	return bTheTestFlag;

}

sal_Bool ScAcceptChgDlg::InsertAcceptedORejected(SvLBoxEntry* pParent)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	sal_Bool bTheTestFlag=sal_True;

	ScChangeActionState eState = SC_CAS_VIRGIN;
	String aString=pTheView->GetEntryText( pParent);
	String a2String=aString.Copy(0,aStrAllAccepted.Len());
	if(a2String==aStrAllAccepted)
	{
		eState=SC_CAS_ACCEPTED;
	}
	else
	{
		a2String=aString.Copy(0,aStrAllRejected.Len());
		if(a2String==aStrAllRejected)
		{
			eState=SC_CAS_REJECTED;
		}
	}

	ScChangeAction* pScChangeAction=pChanges->GetFirst();
	while(pScChangeAction!=NULL)
	{
		if(pScChangeAction->GetState()==eState &&
			InsertFilteredAction(pScChangeAction,eState,pParent)!=NULL)
			bTheTestFlag=sal_False;
		pScChangeAction=pScChangeAction->GetNext();
	}
	return bTheTestFlag;
}

sal_Bool ScAcceptChgDlg::InsertChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	sal_Bool bTheTestFlag=sal_True;
	SvLBoxEntry* pEntry=NULL;
	const ScChangeAction* pChild=(const ScChangeAction*)pActionTable->First();
	while(pChild!=NULL)
	{
		pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,sal_False,sal_True);

		if(pEntry!=NULL)
		{
			bTheTestFlag=sal_False;

			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			pEntryData->bIsRejectable=sal_False;
			pEntryData->bIsAcceptable=sal_False;
			pEntryData->bDisabled=sal_True;

			if(pChild->IsDialogParent())
			{

				//pEntry->EnableChildsOnDemand(sal_True);
				Expand(pChanges,pChild,pEntry);
			}
		}
		pChild=pActionTable->Next();
	}
	return bTheTestFlag;
}
sal_Bool ScAcceptChgDlg::InsertDeletedChilds(const ScChangeAction* pScChangeAction,
										 ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	sal_Bool bTheTestFlag=sal_True;
	SvLBoxEntry* pEntry=NULL;
	ScChangeActionTable aDelActionTable;
	const ScChangeAction* pChild=(const ScChangeAction*)pActionTable->First();

	while(pChild!=NULL)
	{

		if(pScChangeAction!=pChild)
			pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,sal_False,sal_True);
		else
			pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,sal_True,sal_True);

		if(pEntry!=NULL)
		{
			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			pEntryData->bIsRejectable=sal_False;
			pEntryData->bIsAcceptable=sal_False;
			pEntryData->bDisabled=sal_True;

			bTheTestFlag=sal_False;
			if ( pChild->IsDialogParent() )
			{
				Expand(pChanges,pChild,pEntry);
				/*
				pChanges->GetDependents((ScChangeAction*) pChild,aDelActionTable);
				if(aDelActionTable.First()!=NULL)
				{
					pEntry->EnableChildsOnDemand(sal_True);
				}
				aDelActionTable.Clear();
				*/
			}
		}
		pChild=pActionTable->Next();
	}
	return bTheTestFlag;
}

sal_Bool ScAcceptChgDlg::Expand(ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
							SvLBoxEntry* pEntry,sal_Bool bFilter)
{
	sal_Bool bTheTestFlag=sal_True;

	if(pChanges!=NULL &&pEntry!=NULL &&pScChangeAction!=NULL)
	{
		ScChangeActionTable aActionTable;

		GetDependents( pScChangeAction,aActionTable,pEntry);

		switch(pScChangeAction->GetType())
		{
			case SC_CAT_CONTENT:
			{
				InsertContentChilds(&aActionTable,pEntry);
				bTheTestFlag=!bHasFilterEntry;
				break;
			}
			case SC_CAT_DELETE_COLS:
			case SC_CAT_DELETE_ROWS:
			case SC_CAT_DELETE_TABS:
			{
				InsertDeletedChilds(pScChangeAction,&aActionTable,pEntry);
				bTheTestFlag=!bHasFilterEntry;
				break;
			}
			default:
			{
				if(!bFilter)
					bTheTestFlag=InsertChilds(&aActionTable,pEntry);
				break;
			}
		}
		aActionTable.Clear();
	}
	return bTheTestFlag;
}

IMPL_LINK( ScAcceptChgDlg, ExpandingHandle, SvxRedlinTable*, pTable )
{
	ScChangeTrack* pChanges=pDoc->GetChangeTrack();
	ScChangeAction* pScChangeAction=NULL;
	SetPointer(Pointer(POINTER_WAIT));
	if(pTable!=NULL && pChanges!=NULL)
	{
		ScChangeActionTable aActionTable;
		SvLBoxEntry* pEntry=pTheView->GetHdlEntry();
		if(pEntry!=NULL)
		{
			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			if(pEntryData!=NULL)
			{
				pScChangeAction=(ScChangeAction*) pEntryData->pData;
			}

			if(pEntry->HasChildsOnDemand())
			{
				sal_Bool bTheTestFlag=sal_True;
				pEntry->EnableChildsOnDemand(sal_False);
                pTheView->RemoveEntry(pTheView->FirstChild(pEntry));

				if(pEntryData!=NULL)
				{
					pScChangeAction=(ScChangeAction*) pEntryData->pData;

					GetDependents( pScChangeAction,aActionTable,pEntry);

					switch(pScChangeAction->GetType())
					{
						case SC_CAT_CONTENT:
						{
							bTheTestFlag=InsertContentChilds(&aActionTable,pEntry);
							break;
						}
						case SC_CAT_DELETE_COLS:
						case SC_CAT_DELETE_ROWS:
						case SC_CAT_DELETE_TABS:
						{
							bTheTestFlag=InsertDeletedChilds(pScChangeAction,&aActionTable,pEntry);
							break;
						}
						default:
						{
							bTheTestFlag=InsertChilds(&aActionTable,pEntry);
							break;
						}
					}
					aActionTable.Clear();

				}
				else
				{
					bTheTestFlag=InsertAcceptedORejected(pEntry);
				}
				if(bTheTestFlag) pTheView->InsertEntry(aStrNoEntry,NULL,Color(COL_GRAY),pEntry);
			}

		}
	}
	SetPointer(Pointer(POINTER_ARROW));
	return (sal_uLong) sal_True;
}


void ScAcceptChgDlg::AppendChanges(ScChangeTrack* pChanges,sal_uLong nStartAction,
                                   sal_uLong nEndAction, sal_uLong /* nPos */)
{
	if(pChanges!=NULL)
	{
		DateTime aDateTime;
		SvLBoxEntry* pParent=NULL;
		const ScChangeAction* pScChangeAction=NULL;
		bAcceptEnableFlag=sal_True;
		bRejectEnableFlag=sal_True;
		SetPointer(Pointer(POINTER_WAIT));
		pTheView->SetUpdateMode(sal_False);

		ScChangeActionTable ActionTable;
		sal_Bool bTheFlag=sal_False;

		sal_Bool bFilterFlag=pTPFilter->IsDate()||pTPFilter->IsRange()||
					 pTPFilter->IsAuthor()||pTPFilter->IsComment();

		bUseColor=bFilterFlag;

		for(sal_uLong i=nStartAction;i<=nEndAction;i++)
		{
			pScChangeAction=pChanges->GetAction(i);
			if(pScChangeAction==NULL) continue;


			switch(pScChangeAction->GetState())
			{
				case SC_CAS_VIRGIN:

					if(pScChangeAction->IsDialogRoot())
					{
						if(pScChangeAction->IsDialogParent())
							pParent=InsertChangeAction(pScChangeAction,SC_CAS_VIRGIN);
						else
							pParent=InsertFilteredAction(pScChangeAction,SC_CAS_VIRGIN);
					}
					else
						pParent=NULL;

					bTheFlag=sal_True;
					break;

				case SC_CAS_ACCEPTED:
					pParent=NULL;
					nAcceptCount++;
					break;

				case SC_CAS_REJECTED:
					pParent=NULL;
					nRejectCount++;
					break;
			}

			if(pParent!=NULL && pScChangeAction->IsDialogParent())
			{
				if(!bFilterFlag)
				{
					pParent->EnableChildsOnDemand(sal_True);
				}
				else
				{
					sal_Bool bTestFlag=bHasFilterEntry;
					bHasFilterEntry=sal_False;
					if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                        pTheView->RemoveEntry(pParent);
				}
			}

			pScChangeAction=pScChangeAction->GetNext();
		}

        if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
			bTheFlag=sal_False;

		pTPView->EnableAccept(bTheFlag);
		pTPView->EnableAcceptAll(bTheFlag);
		pTPView->EnableReject(bTheFlag);
		pTPView->EnableRejectAll(bTheFlag);

		pTheView->SetUpdateMode(sal_True);
		SetPointer(Pointer(POINTER_ARROW));
	}
}

void ScAcceptChgDlg::RemoveEntrys(sal_uLong nStartAction,sal_uLong nEndAction)
{

	pTheView->SetUpdateMode(sal_False);

	SvLBoxEntry* pEntry=pTheView->GetCurEntry();

	ScRedlinData *pEntryData=NULL;

	if(pEntry!=NULL)
		pEntryData=(ScRedlinData *)(pEntry->GetUserData());

	sal_uLong nAction=0;
	if(pEntryData!=NULL)
	{
		nAction=pEntryData->nActionNo;
	}

	if(nAction>=nStartAction && nAction<=nEndAction)
	{
		pTheView->SetCurEntry(pTheView->GetModel()->GetEntry(0));
	}

	sal_Bool bRemove=sal_False;

	// MUST do it backwards, don't delete parents before children and GPF
	pEntry=pTheView->Last();
	while(pEntry!=NULL)
	{
		bRemove=sal_False;
		pEntryData=(ScRedlinData *)(pEntry->GetUserData());
		if(pEntryData!=NULL)
		{
			nAction=pEntryData->nActionNo;

			if(nStartAction<=nAction && nAction<=nEndAction) bRemove=sal_True;


		}
		SvLBoxEntry* pPrevEntry = pTheView->Prev(pEntry);

		if(bRemove)
		{
			//delete pEntryData;
            pTheView->RemoveEntry(pEntry);
		}
		pEntry=pPrevEntry;
	}
	pTheView->SetUpdateMode(sal_True);

}

void ScAcceptChgDlg::UpdateEntrys(ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction)
{
	pTheView->SetUpdateMode(sal_False);

	sal_uLong nPos=LIST_APPEND;

	sal_Bool bRemove=sal_False;

	SvLBoxEntry* pEntry=pTheView->First();
	SvLBoxEntry* pNextEntry = (pEntry ? pTheView->NextSibling(pEntry) : NULL);
	SvLBoxEntry* pLastEntry=NULL;
	while(pEntry!=NULL)
	{
		bRemove=sal_False;
		ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
		if(pEntryData!=NULL)
		{
			ScChangeAction* pScChangeAction=
					(ScChangeAction*) pEntryData->pData;

			sal_uLong nAction=pScChangeAction->GetActionNumber();

			if(nStartAction<=nAction && nAction<=nEndAction) bRemove=sal_True;
		}

		if(bRemove)
		{
			nPos=pEntry->GetChildListPos();
            pTheView->RemoveEntry(pEntry);

			if(pLastEntry==NULL) pLastEntry=pTheView->First();
			if(pLastEntry!=NULL)
			{
				pNextEntry=pTheView->Next(pLastEntry);

				if(pNextEntry==NULL)
				{
					pNextEntry=pLastEntry;
					pLastEntry=NULL;
				}
			}
			else
				pNextEntry=NULL;

		}
		else
		{
			pLastEntry = pEntry;
			pNextEntry = pTheView->Next(pEntry);
		}
		pEntry=pNextEntry;
	}

	if(nStartAction==nEndAction)
		AppendChanges(pChgTrack,nStartAction,nEndAction,nPos);
	else
		AppendChanges(pChgTrack,nStartAction,nEndAction);

	pTheView->SetUpdateMode(sal_True);

}

IMPL_LINK( ScAcceptChgDlg, ChgTrackModHdl, ScChangeTrack*, pChgTrack)
{
//	if(bScAcceptChgDlgIsDead) return 0;

	ScChangeTrackMsgQueue& aMsgQueue= pChgTrack->GetMsgQueue();

	ScChangeTrackMsgInfo* pTrackInfo=aMsgQueue.Get();
	sal_uLong	nStartAction;
	sal_uLong	nEndAction;

	while(pTrackInfo!=NULL)
	{
		nStartAction=pTrackInfo->nStartAction;
		nEndAction=pTrackInfo->nEndAction;

		if(!bIgnoreMsg)
		{
			bNoSelection=sal_True;

			switch(pTrackInfo->eMsgType)
			{
				case SC_CTM_APPEND: AppendChanges(pChgTrack,nStartAction,nEndAction);
									break;
				case SC_CTM_REMOVE: RemoveEntrys(nStartAction,nEndAction);
									break;
				case SC_CTM_PARENT:
				case SC_CTM_CHANGE: //bNeedsUpdate=sal_True;
									UpdateEntrys(pChgTrack,nStartAction,nEndAction);
									break;
                default:
                {
                    // added to avoid warnings
                }
			}
		}
		delete pTrackInfo;
		pTrackInfo=aMsgQueue.Get();
	}

	return 0;
}
IMPL_LINK( ScAcceptChgDlg, ReOpenTimerHdl, Timer*, EMPTYARG )
{
	ScSimpleRefDlgWrapper::SetAutoReOpen(sal_True);
	aAcceptChgCtr.ShowFilterPage();
	RefHandle(NULL);

	return 0;
}

IMPL_LINK( ScAcceptChgDlg, UpdateSelectionHdl, Timer*, EMPTYARG )
{
    ScTabView* pTabView = pViewData->GetView();

    sal_Bool bAcceptFlag = sal_True;
    sal_Bool bRejectFlag = sal_True;
    sal_Bool bContMark = sal_False;

    pTabView->DoneBlockMode();  // clears old marking
    SvLBoxEntry* pEntry = pTheView->FirstSelected();
    while( pEntry )
	{
        ScRedlinData* pEntryData = (ScRedlinData*) pEntry->GetUserData();
        if( pEntryData )
		{
            bRejectFlag &= pEntryData->bIsRejectable;
            bAcceptFlag &= pEntryData->bIsAcceptable;

            const ScChangeAction* pScChangeAction = (ScChangeAction*) pEntryData->pData;
            if( pScChangeAction && (pScChangeAction->GetType() != SC_CAT_DELETE_TABS) &&
                    (!pEntryData->bDisabled || pScChangeAction->IsVisible()) )
            {
                const ScBigRange& rBigRange = pScChangeAction->GetBigRange();
                if( rBigRange.IsValid( pDoc ) && IsActive() )
                {
                    sal_Bool bSetCursor = !pTheView->NextSelected( pEntry );
                    pTabView->MarkRange( rBigRange.MakeRange(), bSetCursor, bContMark );
                    bContMark = sal_True;
                }
            }
		}
		else
		{
            bAcceptFlag = sal_False;
            bRejectFlag = sal_False;
		}
        bAcceptEnableFlag = bAcceptFlag;
        bRejectEnableFlag = bRejectFlag;

        pEntry = pTheView->NextSelected( pEntry );
	}

    ScChangeTrack* pChanges = pDoc->GetChangeTrack();
    sal_Bool bEnable = pDoc->IsDocEditable() && pChanges && !pChanges->IsProtected();
    pTPView->EnableAccept( bAcceptFlag && bEnable );
    pTPView->EnableReject( bRejectFlag && bEnable );

	return 0;
}

IMPL_LINK( ScAcceptChgDlg, CommandHdl, Control*, EMPTYARG )
{

	const CommandEvent aCEvt(pTheView->GetCommandEvent());

	if(aCEvt.GetCommand()==COMMAND_CONTEXTMENU)
	{
		ScPopupMenu aPopup(ScResId(RID_POPUP_CHANGES));

		aPopup.SetMenuFlags(MENU_FLAG_HIDEDISABLEDENTRIES);

		SvLBoxEntry* pEntry=pTheView->GetCurEntry();
		if(pEntry!=NULL)
		{
			pTheView->Select(pEntry);
		}
		else
		{
			aPopup.Deactivate();
		}

		sal_uInt16 nSortedCol= pTheView->GetSortedCol();

		if(nSortedCol!=0xFFFF)
		{
			sal_uInt16 nItemId=nSortedCol+SC_SUB_SORT+1;

			aPopup.CheckItem(nItemId);

			PopupMenu *pSubMenu = aPopup.GetPopupMenu(SC_SUB_SORT);
			if (pSubMenu)
			{
				pSubMenu->CheckItem(nItemId);
			}
		}

		aPopup.EnableItem(SC_CHANGES_COMMENT,sal_False);

		if(pDoc->IsDocEditable() && pEntry!=NULL)
		{
			ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
			if(pEntryData!=NULL)
			{
				ScChangeAction* pScChangeAction=
						(ScChangeAction*) pEntryData->pData;
				if(pScChangeAction!=NULL && !pTheView->GetParent(pEntry))
					aPopup.EnableItem(SC_CHANGES_COMMENT);
			}
		}

		sal_uInt16 nCommand=aPopup.Execute( this, GetPointerPosPixel() );


		if(nCommand)
		{
			if(nCommand==SC_CHANGES_COMMENT)
			{
				if(pEntry!=NULL)
				{
					ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
					if(pEntryData!=NULL)
					{
						ScChangeAction* pScChangeAction=
								(ScChangeAction*) pEntryData->pData;

						pViewData->GetDocShell()->ExecuteChangeCommentDialog( pScChangeAction, this,sal_False);
					}
				}
			}
			else
			{
				sal_Bool bSortDir=pTheView->GetSortDirection();
				sal_uInt16 nDialogCol=nCommand-SC_SUB_SORT-1;
				if(nSortedCol==nDialogCol) bSortDir=!bSortDir;
				pTheView->SortByCol(nDialogCol,bSortDir);
				/*
				SC_SUB_SORT
				SC_SORT_ACTION
				SC_SORT_POSITION
				SC_SORT_AUTHOR
				SC_SORT_DATE
				SC_SORT_COMMENT
				*/
			}
		}
	}
	return 0;
}

void ScAcceptChgDlg::Initialize(SfxChildWinInfo *pInfo)
{
	String aStr;
	if(pInfo!=NULL)
	{
		if ( pInfo->aExtraString.Len() )
		{
			xub_StrLen nPos = pInfo->aExtraString.Search(
				String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("AcceptChgDat:")));

			// Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
			// er nicht vorhanden ist, liegt eine "altere Version vor
			if ( nPos != STRING_NOTFOUND )
			{
				xub_StrLen n1 = pInfo->aExtraString.Search('(', nPos);
				if ( n1 != STRING_NOTFOUND )
				{
					xub_StrLen n2 = pInfo->aExtraString.Search(')', n1);
					if ( n2 != STRING_NOTFOUND )
					{
						// Alignment-String herausschneiden
						aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
						pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
						aStr.Erase(0, n1-nPos+1);
					}
				}
			}
		}
	}
	SfxModelessDialog::Initialize(pInfo);

	if ( aStr.Len())
	{
		sal_uInt16 nCount=(sal_uInt16)aStr.ToInt32();

		for(sal_uInt16 i=0;i<nCount;i++)
		{
			xub_StrLen n1 = aStr.Search(';');
			aStr.Erase(0, n1+1);
			pTheView->SetTab(i,(sal_uInt16)aStr.ToInt32(),MAP_PIXEL);
		}
	}
}

//-------------------------------------------------------------------------

void ScAcceptChgDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
	SfxModelessDialog::FillInfo(rInfo);
	rInfo.aExtraString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "AcceptChgDat:(" ));

	sal_uInt16	nCount=pTheView->TabCount();

	rInfo.aExtraString += String::CreateFromInt32(nCount);
	rInfo.aExtraString += ';';
	for(sal_uInt16 i=0;i<nCount;i++)
	{
		rInfo.aExtraString += String::CreateFromInt32(pTheView->GetTab(i));
		rInfo.aExtraString += ';';
	}
	rInfo.aExtraString += ')';
}

void ScAcceptChgDlg::InitFilter()
{
	if(pTPFilter->IsDate()||pTPFilter->IsRange()||
		pTPFilter->IsAuthor()||pTPFilter->IsComment())
	{
		pTheView->SetFilterDate(pTPFilter->IsDate());
		pTheView->SetDateTimeMode(pTPFilter->GetDateMode());
		pTheView->SetFirstDate(pTPFilter->GetFirstDate());
		pTheView->SetLastDate(pTPFilter->GetLastDate());
		pTheView->SetFirstTime(pTPFilter->GetFirstTime());
		pTheView->SetLastTime(pTPFilter->GetLastTime());
		pTheView->SetFilterAuthor(pTPFilter->IsAuthor());
		pTheView->SetAuthor(pTPFilter->GetSelectedAuthor());

		pTheView->SetFilterComment(pTPFilter->IsComment());

		utl::SearchParam aSearchParam( pTPFilter->GetComment(),
				utl::SearchParam::SRCH_REGEXP,sal_False,sal_False,sal_False );

		pTheView->SetCommentParams(&aSearchParam);

		pTheView->UpdateFilterTest();
	}
}

//UNUSED2008-05  void ScAcceptChgDlg::SetMyStaticData()
//UNUSED2008-05  {
//UNUSED2008-05  }

IMPL_LINK( ScAcceptChgDlg, FilterModified, SvxTPFilter*, EMPTYARG )
{
	return 0;
}

#define CALC_DATE		3
#define CALC_POS		1

IMPL_LINK( ScAcceptChgDlg, ColCompareHdl, SvSortData*, pSortData )
{
	StringCompare eCompare=COMPARE_EQUAL;
	SCCOL nSortCol= static_cast<SCCOL>(pTheView->GetSortedCol());

	if(pSortData)
	{
		SvLBoxEntry* pLeft = (SvLBoxEntry*)(pSortData->pLeft );
		SvLBoxEntry* pRight = (SvLBoxEntry*)(pSortData->pRight );

		if(CALC_DATE==nSortCol)
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
				return eCompare;
			}
		}
		else if(CALC_POS==nSortCol)
		{
			ScRedlinData *pLeftData=(ScRedlinData *)(pLeft->GetUserData());
			ScRedlinData *pRightData=(ScRedlinData *)(pRight->GetUserData());

			if(pLeftData!=NULL && pRightData!=NULL)
			{
				eCompare=COMPARE_GREATER;

				if(pLeftData->nTable < pRightData->nTable)
				{
					eCompare=COMPARE_LESS;
				}
				else if(pLeftData->nTable == pRightData->nTable)
				{
					if(pLeftData->nRow < pRightData->nRow)
					{
						eCompare=COMPARE_LESS;
					}
					else if(pLeftData->nRow == pRightData->nRow)
					{
						if(pLeftData->nCol < pRightData->nCol)
						{
							eCompare=COMPARE_LESS;
						}
						else if(pLeftData->nCol == pRightData->nCol)
						{
							eCompare=COMPARE_EQUAL;
						}
					}
				}

				return eCompare;
			}
		}

		SvLBoxItem* pLeftItem = pTheView->GetEntryAtPos( pLeft, static_cast<sal_uInt16>(nSortCol));
		SvLBoxItem* pRightItem = pTheView->GetEntryAtPos( pRight, static_cast<sal_uInt16>(nSortCol));

		if(pLeftItem != NULL && pRightItem != NULL)
		{
			sal_uInt16 nLeftKind=pLeftItem->IsA();
			sal_uInt16 nRightKind=pRightItem->IsA();

			if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
				nLeftKind == SV_ITEM_ID_LBOXSTRING )
			{
				eCompare= (StringCompare) ScGlobal::GetCaseCollator()->compareString(
										((SvLBoxString*)pLeftItem)->GetText(),
										((SvLBoxString*)pRightItem)->GetText());

				if(eCompare==COMPARE_EQUAL) eCompare=COMPARE_LESS;
			}
		}


	}
	return eCompare;
}

