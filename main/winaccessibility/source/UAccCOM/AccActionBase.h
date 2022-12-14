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

//////////////////////////////////////////////////////////////////////
// AccActionBase.h: interface for the CAccActionBase class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCACTIONBASE_H__F87FAD24_D66E_4D22_9B24_3304A303DC84__INCLUDED_)
#define AFX_ACCACTIONBASE_H__F87FAD24_D66E_4D22_9B24_3304A303DC84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WNT

#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include "UNOXWrapper.h"

class ATL_NO_VTABLE CAccActionBase  : public CUNOXWrapper
{
public:
    CAccActionBase();
    virtual ~CAccActionBase();

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

    // get the localized action Name
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

    static void GetkeyBindingStrByXkeyBinding( const com::sun::star::uno::Sequence< com::sun::star::awt::KeyStroke > &keySet, OLECHAR* pString );

protected:

    static OLECHAR* getOLECHARFromKeyCode(long key);

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleAction> pRXAct;

private:
    inline com::sun::star::accessibility::XAccessibleAction* GetXInterface()
    {
        return pRXAct.get();
    }
};

#endif // !defined(AFX_ACCACTIONBASE_H__F87FAD24_D66E_4D22_9B24_3304A303DC84__INCLUDED_)
