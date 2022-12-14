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

#include "stdafx.h"
#include "UAccCOM2.h"
#include "EnumVariant.h"
#include "MAccessible.h"

#include "act.hxx"

/////////////////////////////////////////////////////////////////////////////
// CEnumVariant



/**
   * enumarate method,get next element
   * @param  cElements The number of elements to be returned. 
   * @param  pvar      An array of at least size celt in which the elements are to be returned.
   * @param  pcElementFetched Pointer to the number of elements returned in rgVar, or Null??
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Next(ULONG cElements,VARIANT __RPC_FAR *pvar,ULONG __RPC_FAR *pcElementFetched)
{
    long l1;
    ULONG l2;

    if (pvar == NULL)
        return E_INVALIDARG;

	CHECK_ENABLE_INF
    if (pcElementFetched != NULL)
        *pcElementFetched = 0;

    // Retrieve the next cElements.
    for (l1=m_lCurrent, l2=0; l1<m_pXAccessibleSelection->getSelectedAccessibleChildCount() &&
            l2<cElements; l1++, l2++)
    {
        Reference< XAccessible > pRXAcc = m_pXAccessibleSelection->getSelectedAccessibleChild(l1);
        IAccessible* pChild = NULL;
        BOOL isGet = CMAccessible::get_IAccessibleFromXAccessible((long)pRXAcc.get(),&pChild);
        if(isGet)
        {
            pvar[l2].vt = VT_I4;
            ((IMAccessible*)pChild)->Get_XAccChildID(&pvar[l2].lVal);
        }
        else if(pRXAcc.is())
        {
            if(CMAccessible::g_pAgent)
                CMAccessible::g_pAgent->InsertAccObj(pRXAcc.get(),pUNOInterface,NULL);
            BOOL isGet = CMAccessible::get_IAccessibleFromXAccessible((long)pRXAcc.get(),&pChild);
            if(isGet)
            {
                pvar[l2].vt = VT_I4;
                ((IMAccessible*)pChild)->Get_XAccChildID(&pvar[l2].lVal);
            }
        }
    }
    // Set count of elements retrieved.
    if (pcElementFetched != NULL)
        *pcElementFetched = l2;
    m_lCurrent = l1;

    return (l2 < cElements) ? S_FALSE : NOERROR;
}

/**
   * skip the elements in the given range when enumarate elements
   * @param  cElements The number of elements to skip. 
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Skip(ULONG cElements)
{
	CHECK_ENABLE_INF
    m_lCurrent += cElements;
    if (m_lCurrent > (long)(m_lLBound+m_pXAccessibleSelection->getSelectedAccessibleChildCount()))
    {
        m_lCurrent =  m_lLBound+m_pXAccessibleSelection->getSelectedAccessibleChildCount();
        return E_FAIL;
    }
    else
        return NOERROR;
}


/**
   * reset the enumaration position to initial value
   * @param 
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Reset( void)
{
    m_lCurrent = m_lLBound;
    return NOERROR;
}


/**
   *create a new IEnumVariant object,
   *copy current enumaration container and its state to 
   *the new object
   *AT will use the copy object to get elements
   * @param ppenum On return, pointer to the location of the clone enumerator??
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum)
{
    CEnumVariant * penum = NULL;
    HRESULT hr;
    if (ppenum == NULL)
        return E_INVALIDARG;

    *ppenum = NULL;

    hr = Create(&penum);
    if( hr == S_OK )
    {
        penum->PutSelection((long)pUNOInterface);
        *ppenum = penum;
    }
    else
    {
        if (penum)
            penum->Release();
    }
    return hr;
}

/**
   *Static public method to create a CLSID_EnumVariant com object.
   * @param ppenum Pointer to accept com object.
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Create(CEnumVariant __RPC_FAR *__RPC_FAR *ppenum)
{
	ActivateActContext();
	HRESULT hr = CoCreateInstance(CLSID_EnumVariant,NULL,
                                 CLSCTX_SERVER,IID_IEnumVariant,(void **)ppenum);
	DeactivateActContext();
	if (S_OK != hr)
    {
        return E_FAIL;
    }

    return S_OK;
}

/**
   *Return count of elements in current container
   * @param.
   * @return count of elements in current container.
   */
long CEnumVariant::GetCountOfElements()
{
	CHECK_ENABLE_INF_ZERO

    if(m_pXAccessibleSelection.is())
        return m_pXAccessibleSelection->getSelectedAccessibleChildCount();
    return 0;
}

/**
   * Set member m_pXAccessibleSelection to NULL and m_lCurrent to m_lLBound.
   * @param.
   * @return Result
   */
STDMETHODIMP CEnumVariant::ClearEnumeration()
{
    pUNOInterface = NULL;
    m_pXAccessibleSelection = NULL;
    m_lCurrent = m_lLBound;
    return S_OK;
}

/**
   *Static method to fetch XAccessibleSelection
   * @param pXAcc XAccessible interface.
   * @return XAccessibleSelection interface.
   */
static Reference<XAccessibleSelection> GetXAccessibleSelection(XAccessible* pXAcc)
{
    XAccessibleSelection* pSelection = NULL;
    Reference< XAccessibleContext > pRContext;

    if( pXAcc == NULL)
        return NULL;

    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return NULL;

    Reference< XAccessibleSelection > pRSelection(pRContext,UNO_QUERY);
    if( !pRSelection.is() )
        return NULL;

    return pRSelection;
}

/**
   * Put valid UNO XAccessible interface.
   * @param pXSelection XAccessible interface.
   * @return Result..
   */
STDMETHODIMP CEnumVariant::PutSelection(long pXSelection)
{
    pUNOInterface = (XAccessible*)pXSelection;
    m_pXAccessibleSelection = GetXAccessibleSelection(pUNOInterface);
    return S_OK;
}
