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

#ifndef __ACCACTION_H_
#define __ACCACTION_H_

#include "resource.h"       // main symbols
#include "AccActionBase.h"

/**
 * CAccAction implements IAccessibleAction interface.
 */
class ATL_NO_VTABLE CAccAction :
            public CComObjectRoot,
            public CComCoClass<CAccAction, &CLSID_AccAction>,
            public IAccessibleAction,
            public CAccActionBase
{
public:
    CAccAction()
    {
        
    }
    ~CAccAction()
    {
        
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_ACCACTION)

    BEGIN_COM_MAP(CAccAction)
    COM_INTERFACE_ENTRY(IAccessibleAction)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccAction*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    // IAccessibleAction
public:
    // IAccessibleAction

    // Returns the number of action.
    STDMETHOD(nActions)(/*[out,retval]*/long* nActions);

    // Performs specified action on the object.
    STDMETHOD(doAction)(/* [in] */ long actionIndex);

    // Gets description of specified action.
    STDMETHOD(get_description)(long actionIndex,BSTR __RPC_FAR *description);

    // added , 2006/06/28, for driver 07/11
    // get the action name
    STDMETHOD(get_name)( long actionIndex, BSTR __RPC_FAR *name);

    // get the localized action name
    STDMETHOD(get_localizedName)( long actionIndex, BSTR __RPC_FAR *localizedName);

    // Returns key binding object (if any) associated with specified action
    // key binding is string.
    // e.g. "alt+d" (like IAccessible::get_accKeyboardShortcut).
    STDMETHOD(get_keyBinding)(
        /* [in] */ long actionIndex,
        /* [in] */ long nMaxBinding,
        /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
        /* [retval][out] */ long __RPC_FAR *nBinding);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XSubInterface)(long pXSubInterface);

};

#endif //__ACCACTION_H_
