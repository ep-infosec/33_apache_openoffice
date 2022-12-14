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

#if !defined(AFX_ACCEDITABLETEXT_H__0C41AFBE_5A87_4D9D_A284_CEC264D91F81__INCLUDED_)
#define AFX_ACCEDITABLETEXT_H__0C41AFBE_5A87_4D9D_A284_CEC264D91F81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include "UNOXWrapper.h"

/**
 * CAccEditableText implements IAccessibleEditableText interface.
 */
class CAccEditableText :
            public CComObjectRoot,
            public CComCoClass<CAccEditableText,&CLSID_AccEditableText>,
            public IAccessibleEditableText,
            public CUNOXWrapper
{
public:
    CAccEditableText()
    {
        
    }
    virtual ~CAccEditableText()
    {
        
    }

    BEGIN_COM_MAP(CAccEditableText)
    COM_INTERFACE_ENTRY(IAccessibleEditableText)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccEditableText*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_AccEditableText)

    // IAccessibleEditableText
public:
    // IAccessibleEditableText

    // Copys a range of text to the clipboard.
    STDMETHOD(copyText)(long startOffset, long endOffset);

    // Deletes a range of text.
    STDMETHOD(deleteText)(long startOffset, long endOffset);

    // Inserts text at a specified offset.
    STDMETHOD(insertText)(long offset, BSTR * text);

    // Cuts a range of text to the clipboard.
    STDMETHOD(cutText)(long startOffset, long endOffset);

    // Pastes text from clipboard at specified offset.
    STDMETHOD(pasteText)(long offset);

    // Replaces range of text with new text.
    STDMETHOD(replaceText)(long startOffset, long endOffset, BSTR * text);


    // Sets attributes of range of text.
    STDMETHOD(setAttributes)(long startOffset, long endOffset, BSTR * attributes);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

private:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleEditableText> pRXEdtTxt;

    void get_AnyFromOLECHAR(const ::rtl::OUString &ouName, const ::rtl::OUString &ouValue, com::sun::star::uno::Any &rAny);

    inline com::sun::star::accessibility::XAccessibleEditableText* GetXInterface()
    {
        return pRXEdtTxt.get();
    }
};

#endif // !defined(AFX_ACCEDITABLETEXT_H__0C41AFBE_5A87_4D9D_A284_CEC264D91F81__INCLUDED_)
