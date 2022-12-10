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
#include "precompiled_dbui.hxx"
#ifndef DBAUI_DLGSIZE_HRC
#include "dlgsize.hrc"
#endif
#ifndef _DBAUI_DLGSIZE_HXX
#include "dlgsize.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................


#define DEF_ROW_HEIGHT  45
#define DEF_COL_WIDTH   227

DBG_NAME(DlgSize)
//==================================================================
DlgSize::DlgSize( Window* pParent, sal_Int32 nVal, sal_Bool bRow, sal_Int32 _nAlternativeStandard )
		:ModalDialog( pParent, ModuleRes(bRow ? DLG_ROWHEIGHT : DLG_COLWIDTH))
		,m_nPrevValue(nVal)
		,m_nStandard(bRow ? DEF_ROW_HEIGHT : DEF_COL_WIDTH)
		,aFT_VALUE(this,	ModuleRes( FT_VALUE))
		,aMF_VALUE(this,	ModuleRes( MF_VALUE))
		,aCB_STANDARD(this, ModuleRes(CB_STANDARD))
		,aPB_OK(this,		ModuleRes(PB_OK))
		,aPB_CANCEL(this,	ModuleRes(PB_CANCEL))
		,aPB_HELP(this,		ModuleRes(PB_HELP))
{
    DBG_CTOR(DlgSize,NULL);

	if ( _nAlternativeStandard > 0 )
		m_nStandard = _nAlternativeStandard;
	aCB_STANDARD.SetClickHdl(LINK(this,DlgSize,CbClickHdl));

	aMF_VALUE.EnableEmptyFieldValue(sal_True);
	sal_Bool bDefault = -1 == nVal;
	aCB_STANDARD.Check(bDefault);
	if (bDefault)
	{
		SetValue(m_nStandard);
		m_nPrevValue = m_nStandard;
	}
	LINK(this,DlgSize,CbClickHdl).Call(&aCB_STANDARD);

	FreeResource();
}

//------------------------------------------------------------------------------
DlgSize::~DlgSize()
{

    DBG_DTOR(DlgSize,NULL);
}

//------------------------------------------------------------------------------
void DlgSize::SetValue( sal_Int32 nVal )
{
	aMF_VALUE.SetValue(nVal, FUNIT_CM );
}

//------------------------------------------------------------------------------
sal_Int32 DlgSize::GetValue()
{
	if (aCB_STANDARD.IsChecked())
		return -1;
	return (sal_Int32)aMF_VALUE.GetValue( FUNIT_CM );
}

//------------------------------------------------------------------------------
IMPL_LINK( DlgSize, CbClickHdl, Button *, pButton )
{

	if( pButton == &aCB_STANDARD )
	{
		aMF_VALUE.Enable(!aCB_STANDARD.IsChecked());
		if (aCB_STANDARD.IsChecked())
		{
			m_nPrevValue = static_cast<sal_Int32>(aMF_VALUE.GetValue(FUNIT_CM));
				// don't use getValue as this will use aCB_STANDARD.to determine if we're standard
			aMF_VALUE.SetEmptyFieldValue();
		}
		else
		{
			SetValue( m_nPrevValue );
		}
	}
	return 0;
}
// -----------------------------------------------------------------------------
//.........................................................................
}	// namespace dbaui
//.........................................................................


