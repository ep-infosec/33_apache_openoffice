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

#if !defined(AFX_ACCVALUE_H__CBA4972C_4188_4A18_A3CD_4A1FA3DDED38__INCLUDED_)
#define AFX_ACCVALUE_H__CBA4972C_4188_4A18_A3CD_4A1FA3DDED38__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"           // main symbols


#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include "UNOXWrapper.h"

/**
 * CAccValue implements IAccessibleValue interface.
 */
class CAccValue :
            public CComObjectRoot,
            public CComCoClass<CAccValue,&CLSID_AccValue>,
            public IAccessibleValue,
            public CUNOXWrapper
{
public:
    CAccValue()
    {
            }
    virtual ~CAccValue()
    {
            }

    BEGIN_COM_MAP(CAccValue)
    COM_INTERFACE_ENTRY(IAccessibleValue)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccValue*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_AccValue)

    // IAccessibleValue
public:
    // IAccessibleValue

    // Returns the value of this object as a number.
    STDMETHOD(get_currentValue)(VARIANT *currentValue);

    // Sets the value of this object to the given number.
    STDMETHOD(setCurrentValue)(VARIANT value);

    // Returns the maximal value that can be represented by this object.
    STDMETHOD(get_maximumValue)(VARIANT *maximumValue);

    // Returns the minimal value that can be represented by this object.
    STDMETHOD(get_minimumValue)(VARIANT *mininumValue);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

private:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleValue> pRXVal;

    inline com::sun::star::accessibility::XAccessibleValue* GetXInterface()
    {
        return pRXVal.get();
    }

};

#endif // !defined(AFX_ACCVALUE_H__CBA4972C_4188_4A18_A3CD_4A1FA3DDED38__INCLUDED_)
