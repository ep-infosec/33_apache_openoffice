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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#include "swtypes.hxx"

#include "multmrk.hxx"
#include "toxmgr.hxx"

#include "index.hrc"
#include "multmrk.hrc"


SwMultiTOXMarkDlg::SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr& rTOXMgr ) :

	SvxStandardDialog(pParent, SW_RES(DLG_MULTMRK)),

    aTOXFL(this,    SW_RES(FL_TOX)),
	aEntryFT(this, 	SW_RES(FT_ENTRY)),
	aTextFT(this, 	SW_RES(FT_TEXT)),
	aTOXFT(this, 	SW_RES(FT_TOX)),
    aTOXLB(this,    SW_RES(LB_TOX)),
    aOkBT(this,     SW_RES(OK_BT)),
	aCancelBT(this, SW_RES(CANCEL_BT)),
	rMgr( rTOXMgr ),
	nPos(0)
{
	aTOXLB.SetSelectHdl(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

	sal_uInt16 nSize = rMgr.GetTOXMarkCount();
	for(sal_uInt16 i=0; i < nSize; ++i)
		aTOXLB.InsertEntry(rMgr.GetTOXMark(i)->GetText());

	aTOXLB.SelectEntryPos(0);
	aTextFT.SetText(rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());

	FreeResource();
}


IMPL_LINK_INLINE_START( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )
{
	if(pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
	{	SwTOXMark* pMark = rMgr.GetTOXMark(pBox->GetSelectEntryPos());
		aTextFT.SetText(pMark->GetTOXType()->GetTypeName());
		nPos = pBox->GetSelectEntryPos();
	}
	return 0;
}
IMPL_LINK_INLINE_END( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )


void SwMultiTOXMarkDlg::Apply()
{
	rMgr.SetCurTOXMark(nPos);
}

/*-----------------25.02.94 22:06-------------------
 dtor ueberladen
--------------------------------------------------*/


SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg() {}



