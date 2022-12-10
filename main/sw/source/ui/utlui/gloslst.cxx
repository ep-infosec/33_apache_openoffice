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



#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <vcl/lstbox.hxx>
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svl/fstathelper.hxx>
#endif
#include <unotools/pathoptions.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>
#include <swunohelper.hxx>

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _GLOSLST_HRC
#include <gloslst.hrc>
#endif


#define STRING_DELIM (char)0x0A
#define GLOS_TIMEOUT 30000   // alle 30 s updaten
#define FIND_MAX_GLOS 20


struct TripleString
{
	String sGroup;
	String sBlock;
	String sShort;
};

typedef TripleString* TripleStringPtr;
SV_DECL_PTRARR_DEL( TripleStrings, TripleStringPtr, 0, 4 )
SV_IMPL_PTRARR( TripleStrings, TripleStringPtr )

class SwGlossDecideDlg : public ModalDialog
{
	OKButton 		aOk;
	CancelButton 	aCancel;
	HelpButton		aHelp;
	ListBox			aListLB;
    FixedLine       aFL;

	DECL_LINK(DoubleClickHdl, ListBox*);
	DECL_LINK(SelectHdl, ListBox*);

	public:
		SwGlossDecideDlg(Window* pParent);
	ListBox& 	GetListBox() {return aListLB;}
};

/*-----------------21.01.97 13.25-------------------

--------------------------------------------------*/

SwGlossDecideDlg::SwGlossDecideDlg(Window* pParent) :
	ModalDialog(pParent, SW_RES(DLG_GLOSSARY_DECIDE_DLG)),
	aOk(this, 		SW_RES(PB_OK)),
	aCancel(this, 	SW_RES(PB_CANCEL)),
	aHelp(this, 	SW_RES(PB_HELP)),
	aListLB(this, 	SW_RES(LB_LIST)),
    aFL(this,    SW_RES(FL_GLOSS))
{
	FreeResource();
	aListLB.SetDoubleClickHdl(LINK(this, SwGlossDecideDlg, DoubleClickHdl));
	aListLB.SetSelectHdl(LINK(this, SwGlossDecideDlg, SelectHdl));
}

/*-----------------21.01.97 13.25-------------------

--------------------------------------------------*/

IMPL_LINK(SwGlossDecideDlg, DoubleClickHdl, ListBox*, EMPTYARG)
{
	EndDialog(RET_OK);
	return 0;
}
/*-----------------21.01.97 13.29-------------------

--------------------------------------------------*/

IMPL_LINK(SwGlossDecideDlg, SelectHdl, ListBox*, EMPTYARG)
{
	aOk.Enable(LISTBOX_ENTRY_NOTFOUND != aListLB.GetSelectEntryPos());
	return 0;
}

/********************************************************************

********************************************************************/


SwGlossaryList::SwGlossaryList() :
	bFilled(sal_False)
{
	SvtPathOptions aPathOpt;
	sPath = aPathOpt.GetAutoTextPath();
	SetTimeout(GLOS_TIMEOUT);
}

/********************************************************************

********************************************************************/


SwGlossaryList::~SwGlossaryList()
{
	ClearGroups();
}

/********************************************************************
 * Wenn der GroupName bereits bekannt ist, dann wird nur
 * rShortName gefuellt, sonst wird rGroupName ebenfals gesetzt und
 * bei Bedarf nach der richtigen Gruppe gefragt
********************************************************************/


sal_Bool SwGlossaryList::GetShortName(const String& rLongName,
								String& rShortName, String& rGroupName )
{
	if(!bFilled)
		Update();

	TripleStrings aTripleStrings;

	sal_uInt16 nCount = aGroupArr.Count();
	sal_uInt16 nFound = 0;
	for(sal_uInt16 i = 0; i < nCount; i++ )
	{
		AutoTextGroup* pGroup = aGroupArr.GetObject(i);
		if(!rGroupName.Len() || rGroupName == pGroup->sName)
			for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
			{
				String sLong = pGroup->sLongNames.GetToken(j, STRING_DELIM);
				if((rLongName == sLong))
				{
					TripleString* pTriple = new TripleString;
					pTriple->sGroup = pGroup->sName;
					pTriple->sBlock = sLong;
					pTriple->sShort = pGroup->sShortNames.GetToken(j, STRING_DELIM);
					aTripleStrings.Insert(pTriple, nFound++);
				}
			}
	}

	sal_Bool bRet = sal_False;
	nCount = aTripleStrings.Count();
	if(1 == nCount )
	{
		TripleString* pTriple = aTripleStrings[0];
		rShortName = pTriple->sShort;
		rGroupName = pTriple->sGroup;
		bRet = sal_True;
	}
	else if(1 < nCount)
	{
		SwGlossDecideDlg aDlg(0);
		String sTitle = aDlg.GetText();
        sTitle += ' ';
        sTitle += aTripleStrings[0]->sBlock;
		aDlg.SetText(sTitle);

		ListBox& rLB = aDlg.GetListBox();
		for(sal_uInt16 i = 0; i < nCount; i++ )
			rLB.InsertEntry(aTripleStrings[i]->sGroup.GetToken(0, GLOS_DELIM));

		rLB.SelectEntryPos(0);
		if(RET_OK == aDlg.Execute() &&
			LISTBOX_ENTRY_NOTFOUND != rLB.GetSelectEntryPos())
		{
			TripleString* pTriple = aTripleStrings[rLB.GetSelectEntryPos()];
			rShortName = pTriple->sShort;
			rGroupName = pTriple->sGroup;
			bRet = sal_True;
		}
		else
			bRet = sal_False;
	}
	return bRet;
}

/********************************************************************

********************************************************************/


sal_uInt16 	SwGlossaryList::GetGroupCount()
{
	if(!bFilled)
		Update();
	return aGroupArr.Count();
}

/********************************************************************

********************************************************************/


String SwGlossaryList::GetGroupName(sal_uInt16 nPos, sal_Bool bNoPath, String* pTitle)
{
	DBG_ASSERT(aGroupArr.Count() > nPos, "Gruppe nicht vorhanden");
	String sRet(aEmptyStr);
	if(nPos < aGroupArr.Count())
	{
		AutoTextGroup* pGroup = aGroupArr.GetObject(nPos);
		sRet = pGroup->sName;
		if(bNoPath)
			sRet = sRet.GetToken(0, GLOS_DELIM);
		if(pTitle)
			*pTitle = pGroup->sTitle;
	}
	return sRet;

}

/********************************************************************

********************************************************************/


sal_uInt16 	SwGlossaryList::GetBlockCount(sal_uInt16 nGroup)
{
	DBG_ASSERT(aGroupArr.Count() > nGroup, "Gruppe nicht vorhanden");
	if(nGroup < aGroupArr.Count())
	{
		AutoTextGroup* pGroup = aGroupArr.GetObject(nGroup);
		return pGroup->nCount;
	}
	return 0;
}

/********************************************************************

********************************************************************/


String	SwGlossaryList::GetBlockName(sal_uInt16 nGroup, sal_uInt16 nBlock, String& rShortName)
{
	DBG_ASSERT(aGroupArr.Count() > nGroup, "Gruppe nicht vorhanden");
	if(nGroup < aGroupArr.Count())
	{
		AutoTextGroup* pGroup = aGroupArr.GetObject(nGroup);
		rShortName = pGroup->sShortNames.GetToken(nBlock, STRING_DELIM);
		return pGroup->sLongNames.GetToken(nBlock, STRING_DELIM);
	}
	return aEmptyStr;
}

/********************************************************************

********************************************************************/


void SwGlossaryList::Update()
{
	if(!IsActive())
		Start();

	SvtPathOptions aPathOpt;
	String sTemp( aPathOpt.GetAutoTextPath() );
	if(sTemp != sPath)
	{
		sPath = sTemp;
		bFilled = sal_False;
		ClearGroups();
	}
	SwGlossaries* pGlossaries = ::GetGlossaries();
	const SvStrings* pPathArr = pGlossaries->GetPathArray();
	String sExt( SwGlossaries::GetExtension() );
	if(!bFilled)
	{
		sal_uInt16 nGroupCount = pGlossaries->GetGroupCnt();
		for(sal_uInt16 i = 0; i < nGroupCount; i++)
		{
			String sGrpName = pGlossaries->GetGroupName(i);
			sal_uInt16 nPath = (sal_uInt16)sGrpName.GetToken(1, GLOS_DELIM).ToInt32();
			if(nPath < pPathArr->Count())
			{
				AutoTextGroup* pGroup = new AutoTextGroup;
				pGroup->sName = sGrpName;

				FillGroup(pGroup, pGlossaries);
				String sName = *(*pPathArr)[nPath];
				sName += INET_PATH_TOKEN;
				sName += pGroup->sName.GetToken(0, GLOS_DELIM);
				sName += sExt;

				FStatHelper::GetModifiedDateTimeOfFile( sName,
												&pGroup->aDateModified,
												&pGroup->aDateModified );

				aGroupArr.Insert( pGroup, i );
			}
		}
		bFilled = sal_True;
	}
	else
	{
		for(sal_uInt16 nPath = 0; nPath < pPathArr->Count(); nPath++)
		{
			SvStringsDtor aFoundGroupNames;
			SvStrings aFiles( 16, 16 );
			SvPtrarr aDateTimeArr( 16, 16 );

			SWUnoHelper::UCB_GetFileListOfFolder( *(*pPathArr)[nPath], aFiles,
													&sExt, &aDateTimeArr );
			for( sal_uInt16 nFiles = 0, nFEnd = aFiles.Count();
					nFiles < nFEnd; ++nFiles )
			{
				String* pTitle = aFiles[ nFiles ];
				::DateTime* pDT = (::DateTime*) aDateTimeArr[ nFiles ];

				String sName( pTitle->Copy( 0, pTitle->Len() - sExt.Len() ));

				aFoundGroupNames.Insert( new String(sName),
											aFoundGroupNames.Count());
				sName += GLOS_DELIM;
				sName += String::CreateFromInt32( nPath );
				AutoTextGroup* pFound = FindGroup( sName );
				if( !pFound )
				{
					pFound = new AutoTextGroup;
					pFound->sName = sName;
					FillGroup( pFound, pGlossaries );
					pFound->aDateModified = *pDT;

					aGroupArr.Insert(pFound, aGroupArr.Count());
				}
				else if( pFound->aDateModified < *pDT )
				{
					FillGroup(pFound, pGlossaries);
					pFound->aDateModified = *pDT;
				}

				// don't need any more these pointers
				delete pTitle;
				delete pDT;
			}

			sal_uInt16 nArrCount = aGroupArr.Count();
			for( sal_uInt16 i = nArrCount; i; --i)
			{
				// evtl. geloeschte Gruppen entfernen
				AutoTextGroup* pGroup = aGroupArr.GetObject(i - 1);
				sal_uInt16 nGroupPath = (sal_uInt16)pGroup->sName.GetToken( 1,
														GLOS_DELIM).ToInt32();
				// nur die Gruppen werden geprueft, die fuer den
				// aktuellen Teilpfad registriert sind
				if(nGroupPath == nPath)
				{
					sal_Bool bFound = sal_False;
					String sCompareGroup = pGroup->sName.GetToken(0, GLOS_DELIM);
					for( sal_uInt16 j = 0; j < aFoundGroupNames.Count() && !bFound; ++j)
					{
						bFound = sCompareGroup == *aFoundGroupNames[j];
					}
					if(!bFound)
					{
						aGroupArr.Remove(i - 1);
						delete pGroup;
					}
				}
			}
		}
	}
}

/********************************************************************

********************************************************************/


void SwGlossaryList::Timeout()
{
	// nur, wenn eine SwView den Fokus hat, wird automatisch upgedated
	if(::GetActiveView())
		Update();
}

/********************************************************************

********************************************************************/


AutoTextGroup*	SwGlossaryList::FindGroup(const String& rGroupName)
{
	for(sal_uInt16 i = 0; i < aGroupArr.Count(); i++)
	{
		AutoTextGroup* pRet = aGroupArr.GetObject(i);
		if(pRet->sName == rGroupName)
			return pRet;
	}
	return 0;
}

/********************************************************************

********************************************************************/


void SwGlossaryList::FillGroup(AutoTextGroup* pGroup, SwGlossaries* pGlossaries)
{
	SwTextBlocks*	pBlock = pGlossaries->GetGroupDoc(pGroup->sName);
	pGroup->nCount = pBlock ? pBlock->GetCount() : 0;
	pGroup->sLongNames = pGroup->sShortNames = aEmptyStr;
	if(pBlock)
		pGroup->sTitle = pBlock->GetName();

	for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
	{
		pGroup->sLongNames  += pBlock->GetLongName(j);
		pGroup->sLongNames  += STRING_DELIM;
		pGroup->sShortNames += pBlock->GetShortName(j);
		pGroup->sShortNames += STRING_DELIM;
	}
	pGlossaries->PutGroupDoc(pBlock);
}

/********************************************************************
	Alle (nicht mehr als FIND_MAX_GLOS) gefunden Bausteine mit
	passendem Anfang zurueckgeben
********************************************************************/

sal_Bool SwGlossaryList::HasLongName(const String& rBegin, SvStringsISortDtor* pLongNames )
{
	if(!bFilled)
		Update();
	sal_uInt16 nFound = 0;
	sal_uInt16 nCount = aGroupArr.Count();
	sal_uInt16 nBeginLen = rBegin.Len();
	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

	for(sal_uInt16 i = 0; i < nCount; i++ )
	{
		AutoTextGroup* pGroup = aGroupArr.GetObject(i);
		for(sal_uInt16 j = 0; j < pGroup->nCount; j++)
		{
			String sBlock = pGroup->sLongNames.GetToken(j, STRING_DELIM);
			if( rSCmp.isEqual( sBlock.Copy(0, nBeginLen), rBegin ) &&
				nBeginLen + 1 < sBlock.Len())
			{
				String* pBlock = new String(sBlock);
				pLongNames->Insert(pBlock);
				nFound++;
				if(FIND_MAX_GLOS == nFound)
					break;
			}
		}
	}
	return nFound > 0;
}

/********************************************************************

********************************************************************/
void	SwGlossaryList::ClearGroups()
{
	sal_uInt16 nCount = aGroupArr.Count();
	for( sal_uInt16 i = 0; i < nCount; ++i )
		delete aGroupArr.GetObject( i );

	aGroupArr.Remove( 0, nCount );
	bFilled = sal_False;
}


