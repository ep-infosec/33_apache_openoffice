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



//----------------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <svtools/svtreebx.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <unotools/charclass.hxx>
#include <tools/urlobj.hxx>
#include <formula/formulahelper.hxx>
#include <formula/IFunctionDescription.hxx>

#include "tokenuno.hxx"
#include "formula.hxx"
#include "formdata.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "appoptio.hxx"
#include "docsh.hxx"
#include "funcdesc.hxx"
#include "formula/token.hxx"
#include "tokenarray.hxx"
#include "sc.hrc"
#include "servuno.hxx"
#include "unonames.hxx"
#include "externalrefmgr.hxx"

#include <com/sun/star/table/CellAddress.hpp>

//============================================================================
using namespace formula;
using namespace com::sun::star;

ScDocument*	ScFormulaDlg::pDoc = NULL;
ScAddress ScFormulaDlg::aCursorPos;



//	--------------------------------------------------------------------------
//		Initialisierung / gemeinsame Funktionen  fuer Dialog
//	--------------------------------------------------------------------------

ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
									Window* pParent, ScViewData* pViewData,formula::IFunctionManager* _pFunctionMgr )
    : formula::FormulaDlg( pB, pCW, pParent, true,true,true,this, _pFunctionMgr,this)
    , m_aHelper(this,pB)
{
    m_aHelper.SetWindow(this);
    ScModule* pScMod = SC_MOD();
    pScMod->InputEnterHandler();
    ScTabViewShell* pScViewShell = NULL;

	// title has to be from the view that opened the dialog,
	// even if it's not the current view

	SfxObjectShell* pParentDoc = NULL;
	if ( pB )
	{
		SfxDispatcher* pMyDisp = pB->GetDispatcher();
		if (pMyDisp)
		{
			SfxViewFrame* pMyViewFrm = pMyDisp->GetFrame();
			if (pMyViewFrm)
            {
                pScViewShell = PTR_CAST( ScTabViewShell, pMyViewFrm->GetViewShell() );
                if( pScViewShell )
		            pScViewShell->UpdateInputHandler(sal_True);
				pParentDoc = pMyViewFrm->GetObjectShell();
            }
		}
	}
	//if ( !pParentDoc && pScViewShell )					// use current only if above fails
	//	pParentDoc = pScViewShell->GetObjectShell();
	//if ( pParentDoc )
	//	aDocName = pParentDoc->GetTitle();

    if ( pDoc == NULL )
        pDoc = pViewData->GetDocument();
    m_xParser.set(ScServiceProvider::MakeInstance(SC_SERVICE_FORMULAPARS,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet> xSet(m_xParser,uno::UNO_QUERY);
    xSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_COMPILEFAP)),uno::makeAny(sal_True));

    m_xOpCodeMapper.set(ScServiceProvider::MakeInstance(SC_SERVICE_OPCODEMAPPER,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);

    ScInputHandler*	pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

	DBG_ASSERT( pInputHdl, "Missing input handler :-/" );

	if ( pInputHdl )
		pInputHdl->NotifyChange( NULL );

	m_aHelper.enableInput( sal_False );
	m_aHelper.EnableSpreadsheets();
    m_aHelper.Init();
	m_aHelper.SetDispatcherLock( sal_True );

    notifyChange();
    fill();

	ScFormEditData* pData = pScMod->GetFormEditData();
	if (!pData)
	{
		//Nun wird es Zeit den Inputhandler festzulegen
		pScMod->SetRefInputHdl(pScMod->GetInputHdl());

		pDoc = pViewData->GetDocument();
		SCCOL nCol = pViewData->GetCurX();
		SCROW nRow = pViewData->GetCurY();
		SCTAB nTab = pViewData->GetTabNo();
		aCursorPos = ScAddress( nCol, nRow, nTab );

		pScMod->InitFormEditData();								// neu anlegen
        pData = pScMod->GetFormEditData();
		pData->SetInputHandler(pScMod->GetInputHdl());
		pData->SetDocShell(pViewData->GetDocShell());

		DBG_ASSERT(pData,"FormEditData ist nicht da");

        formula::FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;			// Default...

		//	Formel vorhanden? Dann editieren

		String aFormula;
		pDoc->GetFormula( nCol, nRow, nTab, aFormula );
		sal_Bool bEdit   = ( aFormula.Len() > 1 );
        sal_Bool bMatrix = sal_False;
		if ( bEdit )
		{
            bMatrix = CheckMatrix(aFormula);

			xub_StrLen nFStart = 0;
			xub_StrLen nFEnd   = 0;
			if ( GetFormulaHelper().GetNextFunc( aFormula, sal_False, nFStart, &nFEnd) )
			{
				pScMod->InputReplaceSelection( aFormula );
				pScMod->InputSetSelection( nFStart, nFEnd );
                xub_StrLen PrivStart, PrivEnd;
				pScMod->InputGetSelection( PrivStart, PrivEnd);

                eMode = SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,sal_True,sal_True);
				pData->SetFStart( nFStart );
			}
			else
				bEdit = sal_False;
		}

		if ( !bEdit )
		{
			String aNewFormula = '=';
			if ( aFormula.Len() > 0 && aFormula.GetChar(0) == '=' )
				aNewFormula=aFormula;

			pScMod->InputReplaceSelection( aNewFormula );
			pScMod->InputSetSelection( 1, aNewFormula.Len()+1 );
            xub_StrLen PrivStart, PrivEnd;
			pScMod->InputGetSelection( PrivStart, PrivEnd);
            SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,sal_False,sal_False);

			pData->SetFStart( 1 );		// hinter dem "="
		}

		pData->SetMode( (sal_uInt16) eMode );
		String rStrExp = GetMeText();

		pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );

		Update(rStrExp);
    } // if (!pData)

}

void ScFormulaDlg::notifyChange()
{
	ScModule* pScMod = SC_MOD();

	ScInputHandler*	pInputHdl = pScMod->GetInputHdl();
	if ( pInputHdl )
		pInputHdl->NotifyChange( NULL );
}
// -----------------------------------------------------------------------------
void ScFormulaDlg::fill()
{
    ScModule* pScMod = SC_MOD();
	ScFormEditData* pData = pScMod->GetFormEditData();
    notifyChange();
	String rStrExp;
	if (pData)
	{
		//	Daten schon vorhanden -> Zustand wiederherstellen (nach Umschalten)
		//	pDoc und aCursorPos nicht neu initialisieren
		//pDoc = pViewData->GetDocument();
		if(IsInputHdl(pData->GetInputHandler()))
		{
			pScMod->SetRefInputHdl(pData->GetInputHandler());
		}
		else
		{
			PtrTabViewShell pTabViewShell;
			ScInputHandler*	pInputHdl = GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

			if ( pInputHdl == NULL ) //DocShell hat keinen InputHandler mehr,
			{					//hat der Anwender halt Pech gehabt.
				disableOk();
				pInputHdl = pScMod->GetInputHdl();
			}
			else
			{
				pInputHdl->SetRefViewShell(pTabViewShell);
			}
			pScMod->SetRefInputHdl(pInputHdl);
			pData->SetInputHandler(pInputHdl);
		}

        String aOldFormulaTmp(pScMod->InputGetFormulaStr());
        pScMod->InputSetSelection( 0, aOldFormulaTmp.Len());

		rStrExp=pData->GetUndoStr();
		pScMod->InputReplaceSelection(rStrExp);

        SetMeText(rStrExp);

		pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );

        Update();
		// Jetzt nochmals zurueckschalten, da evtl. neues Doc geoeffnet wurde!
		pScMod->SetRefInputHdl(NULL);
	}
}

__EXPORT ScFormulaDlg::~ScFormulaDlg()
{
	ScModule* pScMod = SC_MOD();
	ScFormEditData* pData = pScMod->GetFormEditData();

	if (pData) // wird nicht ueber Close zerstoert;
	{
		//Referenz Inputhandler zuruecksetzen
		pScMod->SetRefInputHdl(NULL);
	} // if (pData) // wird nicht ueber Close zerstoert;

	delete pCell;
}

sal_Bool ScFormulaDlg::IsInputHdl(ScInputHandler* pHdl)
{
	sal_Bool bAlive = sal_False;

	//	gehoert der InputHandler zu irgendeiner ViewShell ?

	TypeId aScType = TYPE(ScTabViewShell);
	SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
	while ( pSh && !bAlive )
	{
		if (((ScTabViewShell*)pSh)->GetInputHandler() == pHdl)
			bAlive = sal_True;
		pSh = SfxViewShell::GetNext( *pSh, &aScType );
	}

	return bAlive;

}

ScInputHandler* ScFormulaDlg::GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh)
{
	ScInputHandler* pHdl=NULL;

	SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
	while( pFrame && pHdl==NULL)
	{
		SfxViewShell* p = pFrame->GetViewShell();
		ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
		if(pViewSh!=NULL)
		{
			pHdl=pViewSh->GetInputHandler();
			if(ppViewSh!=NULL) *ppViewSh=pViewSh;
		}
		pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
	}


	return pHdl;
}


sal_Bool __EXPORT ScFormulaDlg::Close()
{
	DoEnter(sal_False);
	return sal_True;
}

//	--------------------------------------------------------------------------
//							Funktionen fuer rechte Seite
//	--------------------------------------------------------------------------
bool ScFormulaDlg::calculateValue( const String& rStrExp, String& rStrResult )
{
	sal_Bool bResult = sal_True;

    ::std::auto_ptr<ScFormulaCell> pFCell( new ScFormulaCell( pDoc, aCursorPos, rStrExp ) );

	// #35521# HACK! um bei ColRowNames kein #REF! zu bekommen,
	// wenn ein Name eigentlich als Bereich in die Gesamt-Formel
	// eingefuegt wird, bei der Einzeldarstellung aber als
	// single-Zellbezug interpretiert wird
	sal_Bool bColRowName = pCell->HasColRowName();
	if ( bColRowName )
	{
		// ColRowName im RPN-Code?
		if ( pCell->GetCode()->GetCodeLen() <= 1 )
		{	// ==1: einzelner ist als Parameter immer Bereich
			// ==0: es waere vielleicht einer, wenn..
			String aBraced( '(' );
			aBraced += rStrExp;
			aBraced += ')';
			pFCell.reset( new ScFormulaCell( pDoc, aCursorPos, aBraced ) );
		}
		else
			bColRowName = sal_False;
	}

	sal_uInt16 nErrCode = pFCell->GetErrCode();
	if ( nErrCode == 0 )
	{
		SvNumberFormatter& aFormatter = *(pDoc->GetFormatTable());
		Color* pColor;
		if ( pFCell->IsValue() )
		{
			double n = pFCell->GetValue();
			sal_uLong nFormat = aFormatter.GetStandardFormat( n, 0,
							pFCell->GetFormatType(), ScGlobal::eLnge );
			aFormatter.GetOutputString( n, nFormat,
										rStrResult, &pColor );
		}
		else
		{
			String aStr;

			pFCell->GetString( aStr );
			sal_uLong nFormat = aFormatter.GetStandardFormat(
							pFCell->GetFormatType(), ScGlobal::eLnge);
			aFormatter.GetOutputString( aStr, nFormat,
										rStrResult, &pColor );
		}

		ScRange aTestRange;
		if ( bColRowName || (aTestRange.Parse(rStrExp) & SCA_VALID) )
			rStrResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ..." ));
			// Bereich
	}
	else
		rStrResult += ScGlobal::GetErrorString(nErrCode);

	if(!isUserMatrix() && pFCell->GetMatrixFlag())
	{
		CheckMatrix();
	}

	return bResult;
}



//	virtuelle Methoden von ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    ::std::pair<formula::RefButton*,formula::RefEdit*> aPair = RefInputStartBefore( pEdit, pButton );
    m_aHelper.RefInputStart( aPair.second, aPair.first);
    RefInputStartAfter( aPair.second, aPair.first );
}
void ScFormulaDlg::RefInputDone( sal_Bool bForced )
{
    m_aHelper.RefInputDone( bForced );
    RefInputDoneAfter( bForced );
}

void ScFormulaDlg::SetReference( const ScRange& rRef, ScDocument* pRefDoc )
{
    const IFunctionDescription* pFunc = getCurrentFunctionDescription();
	if ( pFunc && pFunc->getSuppressedArgumentCount() > 0 )
	{
        Selection theSel;
		sal_Bool bRefNull = UpdateParaWin(theSel);

		if ( rRef.aStart != rRef.aEnd && bRefNull )
		{
			RefInputStart(GetActiveEdit());
		}

        String		aRefStr;
		sal_Bool bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
		if ( bOtherDoc )
		{
			//	Referenz auf anderes Dokument - wie inputhdl.cxx

			DBG_ASSERT(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

			String aTmp;
			rRef.Format( aTmp, SCA_VALID|SCA_TAB_3D, pRefDoc );		// immer 3d

			SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();

            // #i75893# convert escaped URL of the document to something user friendly
//           String aFileName = pObjSh->GetMedium()->GetName();
            String aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

			aRefStr = '\'';
			aRefStr += aFileName;
			aRefStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "'#" ));
			aRefStr += aTmp;
		}
		else
		{
			sal_uInt16 nFmt = ( rRef.aStart.Tab() == aCursorPos.Tab() )
								? SCA_VALID
								: SCA_VALID | SCA_TAB_3D;
			rRef.Format( aRefStr, nFmt, pRefDoc, pRefDoc->GetAddressConvention() );
		}

		UpdateParaWin(theSel,aRefStr);
	}
}

sal_Bool ScFormulaDlg::IsRefInputMode() const
{
    const IFunctionDescription*	pDesc = getCurrentFunctionDescription();
	sal_Bool bRef = (pDesc && (pDesc->getSuppressedArgumentCount() > 0)) && (pDoc!=NULL);
	return bRef;
}

sal_Bool ScFormulaDlg::IsDocAllowed(SfxObjectShell* pDocSh) const
{
	//	not allowed: different from this doc, and no name
	//	pDocSh is always a ScDocShell
	if ( pDocSh && ((ScDocShell*)pDocSh)->GetDocument() != pDoc && !pDocSh->HasName() )
		return sal_False;

	return sal_True;		// everything else is allowed
}

void ScFormulaDlg::SetActive()
{
    const IFunctionDescription* pFunc = getCurrentFunctionDescription();
	if ( pFunc && pFunc->getSuppressedArgumentCount() > 0 )
	{
		RefInputDone();
        SetEdSelection();
	}
}

void ScFormulaDlg::SaveLRUEntry(const ScFuncDesc* pFuncDescP)
{
	if (pFuncDescP && pFuncDescP->nFIndex!=0)
	{
		ScModule* pScMod = SC_MOD();
		pScMod->InsertEntryToLRUList(pFuncDescP->nFIndex);
	}
}

void ScFormulaDlg::doClose(sal_Bool /*_bOk*/)
{
    m_aHelper.DoClose( ScFormulaDlgWrapper::GetChildWindowId() );
}
void ScFormulaDlg::insertEntryToLRUList(const formula::IFunctionDescription*	_pDesc)
{
    const ScFuncDesc* pDesc = dynamic_cast<const ScFuncDesc*>(_pDesc);
    SaveLRUEntry(pDesc);
}
void ScFormulaDlg::showReference(const String& _sFormula)
{
    ShowReference(_sFormula);
}
void ScFormulaDlg::ShowReference(const String& _sFormula)
{
    m_aHelper.ShowReference(_sFormula);
}
void ScFormulaDlg::HideReference( sal_Bool bDoneRefMode )
{
    m_aHelper.HideReference(bDoneRefMode);
}
void ScFormulaDlg::ViewShellChanged( ScTabViewShell* pScViewShell )
{
    m_aHelper.ViewShellChanged( pScViewShell );
}
void ScFormulaDlg::AddRefEntry( )
{

}
sal_Bool ScFormulaDlg::IsTableLocked( ) const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden
    return sal_False;
}
void ScFormulaDlg::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ToggleCollapsed(pEdit,pButton);
}
void ScFormulaDlg::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ReleaseFocus(pEdit,pButton);
}
void ScFormulaDlg::dispatch(sal_Bool _bOK,sal_Bool _bMartixChecked)
{
    SfxBoolItem	  aRetItem( SID_DLG_RETOK, _bOK );
	SfxBoolItem	  aMatItem( SID_DLG_MATRIX, _bMartixChecked );
	SfxStringItem aStrItem( SCITEM_STRING, getCurrentFormula() );

	// Wenn durch Dokument-Umschalterei die Eingabezeile weg war/ist,
	// ist der String leer. Dann nicht die alte Formel loeschen.
	if ( !aStrItem.GetValue().Len() )
		aRetItem.SetValue( sal_False );		// sal_False = Cancel

	m_aHelper.SetDispatcherLock( sal_False ); // Modal-Modus ausschalten

    clear();

	GetBindings().GetDispatcher()->Execute( SID_INS_FUNCTION,
							  SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
							  &aRetItem, &aStrItem, &aMatItem, 0L );
}
void ScFormulaDlg::setDispatcherLock( sal_Bool bLock )
{
    m_aHelper.SetDispatcherLock( bLock );
}
void ScFormulaDlg::setReferenceInput(const formula::FormEditData* _pData)
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = const_cast<ScFormEditData*>(dynamic_cast<const ScFormEditData*>(_pData));
    pScMod->SetRefInputHdl(pData->GetInputHandler());
}
void ScFormulaDlg::deleteFormData()
{
    ScModule* pScMod = SC_MOD();
    pScMod->ClearFormEditData();		// pData wird ungueltig!
}
void ScFormulaDlg::clear()
{
    pDoc = NULL;

	//Referenz Inputhandler zuruecksetzen
    ScModule* pScMod = SC_MOD();
	pScMod->SetRefInputHdl(NULL);

	// Enable() der Eingabezeile erzwingen:
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
	if ( pScViewShell )
		pScViewShell->UpdateInputHandler();
}
void ScFormulaDlg::switchBack()
{
    ScModule* pScMod = SC_MOD();
    // auf das Dokument zurueckschalten
	// (noetig, weil ein fremdes oben sein kann - #34222#)
	ScInputHandler* pHdl = pScMod->GetInputHdl();
	if ( pHdl )
	{
		pHdl->ViewShellGone(NULL);	// -> aktive View neu holen
		pHdl->ShowRefFrame();
	}

	// aktuelle Tabelle ggF. restaurieren (wg. Maus-RefInput)
	ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
	if ( pScViewShell )
	{
		ScViewData* pVD=pScViewShell->GetViewData();
		SCTAB nExecTab = aCursorPos.Tab();
		if ( nExecTab != pVD->GetTabNo() )
			pScViewShell->SetTabNo( nExecTab );

		SCROW nRow=aCursorPos.Row();
		SCCOL nCol=aCursorPos.Col();

		if(pVD->GetCurX()!=nCol || pVD->GetCurY()!=nRow)
			pScViewShell->SetCursor(nCol,nRow);
    }
}
formula::FormEditData* ScFormulaDlg::getFormEditData() const
{
    ScModule* pScMod = SC_MOD();
	return pScMod->GetFormEditData();
}
void ScFormulaDlg::setCurrentFormula(const String& _sReplacement)
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputReplaceSelection(_sReplacement);
}
void ScFormulaDlg::setSelection(xub_StrLen _nStart,xub_StrLen _nEnd)
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputSetSelection( _nStart, _nEnd );
}
void ScFormulaDlg::getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputGetSelection( _nStart, _nEnd );
}
String ScFormulaDlg::getCurrentFormula() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->InputGetFormulaStr();
}
formula::IFunctionManager* ScFormulaDlg::getFunctionManager()
{
    return ScGlobal::GetStarCalcFunctionMgr();
}
uno::Reference< sheet::XFormulaParser> ScFormulaDlg::getFormulaParser() const
{
    return m_xParser;
}
uno::Reference< sheet::XFormulaOpCodeMapper> ScFormulaDlg::getFormulaOpCodeMapper() const
{
    return m_xOpCodeMapper;
}

table::CellAddress ScFormulaDlg::getReferencePosition() const
{
    return table::CellAddress(aCursorPos.Tab(),aCursorPos.Col(),aCursorPos.Row());
}

::std::auto_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill( _aTokenList, pDoc->GetExternalRefManager());
    return pArray;
}

