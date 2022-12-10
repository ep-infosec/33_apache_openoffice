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

#include <svl/intitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fhgtitem.hxx>

#define _OUTLINER_CXX
#include <editeng/outliner.hxx>
#include <outlundo.hxx>


OutlinerUndoBase::OutlinerUndoBase( sal_uInt16 _nId, Outliner* pOutliner )
	: EditUndo( _nId, NULL )
{
	DBG_ASSERT( pOutliner, "Undo: Outliner?!" );
	mpOutliner = pOutliner;
}

OutlinerUndoChangeParaFlags::OutlinerUndoChangeParaFlags( Outliner* pOutliner, sal_uInt16 nPara, sal_uInt16 nOldFlags, sal_uInt16 nNewFlags )
: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
	mnPara = nPara;
	mnOldFlags = nOldFlags;
	mnNewFlags = nNewFlags;
}

void OutlinerUndoChangeParaFlags::Undo()
{
	ImplChangeFlags( mnOldFlags );
}

void OutlinerUndoChangeParaFlags::Redo()
{
	ImplChangeFlags( mnNewFlags );
}

void OutlinerUndoChangeParaFlags::ImplChangeFlags( sal_uInt16 nFlags )
{
	Outliner* pOutliner = GetOutliner();
	Paragraph* pPara = pOutliner->GetParagraph( mnPara );
	if( pPara )
	{
		pOutliner->nDepthChangedHdlPrevDepth = pPara->GetDepth();
		pOutliner->mnDepthChangeHdlPrevFlags = pPara->nFlags;
		pOutliner->pHdlParagraph = pPara;

		pPara->nFlags = nFlags;
		pOutliner->DepthChangedHdl();
	}
}

OutlinerUndoChangeParaNumberingRestart::OutlinerUndoChangeParaNumberingRestart( Outliner* pOutliner, sal_uInt16 nPara,
        sal_Int16 nOldNumberingStartValue, sal_Int16 nNewNumberingStartValue,
        sal_Bool  bOldParaIsNumberingRestart, sal_Bool bNewParaIsNumberingRestart )
: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
	mnPara = nPara;

    maUndoData.mnNumberingStartValue = nOldNumberingStartValue;
    maUndoData.mbParaIsNumberingRestart = bOldParaIsNumberingRestart;
    maRedoData.mnNumberingStartValue = nNewNumberingStartValue;
    maRedoData.mbParaIsNumberingRestart = bNewParaIsNumberingRestart;
}

void OutlinerUndoChangeParaNumberingRestart::Undo()
{
	ImplApplyData( maUndoData );
}

void OutlinerUndoChangeParaNumberingRestart::Redo()
{
	ImplApplyData( maRedoData );
}

void OutlinerUndoChangeParaNumberingRestart::ImplApplyData( const ParaRestartData& rData )
{
	Outliner* pOutliner = GetOutliner();
    pOutliner->SetNumberingStartValue( mnPara, rData.mnNumberingStartValue );
    pOutliner->SetParaIsNumberingRestart( mnPara, rData.mbParaIsNumberingRestart );
}

OutlinerUndoChangeDepth::OutlinerUndoChangeDepth( Outliner* pOutliner, sal_uInt16 nPara, sal_Int16 nOldDepth, sal_Int16 nNewDepth )
	: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
	mnPara = nPara;
	mnOldDepth = nOldDepth;
	mnNewDepth = nNewDepth;
}

void OutlinerUndoChangeDepth::Undo()
{
	GetOutliner()->ImplInitDepth( mnPara, mnOldDepth, sal_False );
}

void OutlinerUndoChangeDepth::Redo()
{
	GetOutliner()->ImplInitDepth( mnPara, mnNewDepth, sal_False );
}

void OutlinerUndoChangeDepth::Repeat()
{
	DBG_ERROR( "Repeat not implemented!" );
}


OutlinerUndoCheckPara::OutlinerUndoCheckPara( Outliner* pOutliner, sal_uInt16 nPara )
	: OutlinerUndoBase( OLUNDO_DEPTH, pOutliner )
{
	mnPara = nPara;
}

void OutlinerUndoCheckPara::Undo()
{
	Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
	pPara->Invalidate();
	GetOutliner()->ImplCalcBulletText( mnPara, sal_False, sal_False );
}

void OutlinerUndoCheckPara::Redo()
{
	Paragraph* pPara = GetOutliner()->GetParagraph( mnPara );
	pPara->Invalidate();
	GetOutliner()->ImplCalcBulletText( mnPara, sal_False, sal_False );
}

void OutlinerUndoCheckPara::Repeat()
{
	DBG_ERROR( "Repeat not implemented!" );
}

DBG_NAME(OLUndoExpand);

OLUndoExpand::OLUndoExpand(Outliner* pOut, sal_uInt16 _nId )
	: EditUndo( _nId, 0 )
{
	DBG_CTOR(OLUndoExpand,0);
	DBG_ASSERT(pOut,"Undo:No Outliner");
	pOutliner = pOut;
	nCount = 0;
	pParas = 0;
}


OLUndoExpand::~OLUndoExpand()
{
	DBG_DTOR(OLUndoExpand,0);
	delete pParas;
}


void OLUndoExpand::Restore( sal_Bool bUndo )
{
	DBG_CHKTHIS(OLUndoExpand,0);
	DBG_ASSERT(pOutliner,"Undo:No Outliner");
	DBG_ASSERT(pOutliner->pEditEngine,"Outliner already deleted");
	Paragraph* pPara;

	sal_Bool bExpand = sal_False;
	sal_uInt16 _nId = GetId();
	if((_nId == OLUNDO_EXPAND && !bUndo) || (_nId == OLUNDO_COLLAPSE && bUndo))
		bExpand = sal_True;
	if( !pParas )
	{
		pPara = pOutliner->GetParagraph( (sal_uLong)nCount );
		if( bExpand )
			pOutliner->Expand( pPara );
		else
			pOutliner->Collapse( pPara );
	}
	else
	{
		for( sal_uInt16 nIdx = 0; nIdx < nCount; nIdx++ )
		{
			pPara = pOutliner->GetParagraph( (sal_uLong)(pParas[nIdx]) );
			if( bExpand )
				pOutliner->Expand( pPara );
			else
				pOutliner->Collapse( pPara );
		}
	}
}


void OLUndoExpand::Undo()
{
	DBG_CHKTHIS(OLUndoExpand,0);
	Restore( sal_True );
}


void OLUndoExpand::Redo()
{
	DBG_CHKTHIS(OLUndoExpand,0);
	Restore( sal_False );
}


void OLUndoExpand::Repeat()
{
	DBG_CHKTHIS(OLUndoExpand,0);
	DBG_ERROR("Not implemented");
}
