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
#include "precompiled_cui.hxx"

#include "showcols.hxx"
#include "fmsearch.hrc"

#include <tools/shl.hxx>
#include <dialmgr.hxx>
#include <vcl/msgbox.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>

#define CUIFM_PROP_HIDDEN rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Hidden" ) )
#define CUIFM_PROP_LABEL  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ) )

//==========================================================================
//	FmShowColsDialog
//==========================================================================
DBG_NAME(FmShowColsDialog)
//--------------------------------------------------------------------------
FmShowColsDialog::FmShowColsDialog(Window* pParent)
	:ModalDialog(pParent, CUI_RES(RID_SVX_DLG_SHOWGRIDCOLUMNS))
	,m_aList(this, CUI_RES(1))
	,m_aLabel(this, CUI_RES(1))
	,m_aOK(this, CUI_RES(1))
	,m_aCancel(this, CUI_RES(1))
{
	DBG_CTOR(FmShowColsDialog,NULL);
	m_aList.EnableMultiSelection(sal_True);
	m_aOK.SetClickHdl( LINK( this, FmShowColsDialog, OnClickedOk ) );

	FreeResource();
}

//--------------------------------------------------------------------------
FmShowColsDialog::~FmShowColsDialog()
{
	DBG_DTOR(FmShowColsDialog,NULL);
}

//--------------------------------------------------------------------------
IMPL_LINK( FmShowColsDialog, OnClickedOk, Button*, EMPTYARG )
{
	DBG_ASSERT(m_xColumns.is(), "FmShowColsDialog::OnClickedOk : you should call SetColumns before executing the dialog !");
	if (m_xColumns.is())
	{
		::com::sun::star::uno::Any aCol;
		::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xCol;
		for (sal_uInt16 i=0; i<m_aList.GetSelectEntryCount(); ++i)
		{
			m_xColumns->getByIndex(sal::static_int_cast<sal_Int32>(reinterpret_cast<sal_uIntPtr>(m_aList.GetEntryData(m_aList.GetSelectEntryPos(i))))) >>= xCol;
			if (xCol.is())
			{
				try
				{
					//CHINA001 xCol->setPropertyValue(::svxform::FM_PROP_HIDDEN, ::cppu::bool2any(sal_False));
					xCol->setPropertyValue(CUIFM_PROP_HIDDEN, ::cppu::bool2any(sal_False));
				}
				catch(...)
				{
					DBG_ERROR("FmShowColsDialog::OnClickedOk Exception occurred!");
				}
			}
		}
	}

	EndDialog(RET_OK);
	return 0L;
}

//--------------------------------------------------------------------------
void FmShowColsDialog::SetColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer>& xCols)
{
	DBG_ASSERT(xCols.is(), "FmShowColsDialog::SetColumns : invalid columns !");
	if (!xCols.is())
		return;
	m_xColumns = xCols.get();

	m_aList.Clear();

	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>  xCurCol;
	String sCurName;
	for (sal_uInt16 i=0; i<xCols->getCount(); ++i)
	{
		sCurName.Erase();
		::cppu::extractInterface(xCurCol, xCols->getByIndex(i));
		sal_Bool bIsHidden = sal_False;
		try
		{
			//CHINA001 ::com::sun::star::uno::Any aHidden = xCurCol->getPropertyValue(::svxform::FM_PROP_HIDDEN);
			::com::sun::star::uno::Any aHidden = xCurCol->getPropertyValue(CUIFM_PROP_HIDDEN);
			bIsHidden = ::comphelper::getBOOL(aHidden);

			::rtl::OUString sName;
			//CHINA001 xCurCol->getPropertyValue(::svxform::FM_PROP_LABEL) >>= sName;
			
			xCurCol->getPropertyValue(CUIFM_PROP_LABEL) >>= sName;
			sCurName = sName.getStr();
		}
		catch(...)
		{
			DBG_ERROR("FmShowColsDialog::SetColumns Exception occurred!");
		}

		// if the col is hidden, put it into the list
		if (bIsHidden)
			m_aList.SetEntryData( m_aList.InsertEntry(sCurName), reinterpret_cast<void*>((sal_Int64)i) );
	}
}

