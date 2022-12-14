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

#ifndef __ACCRELATION_H_
#define __ACCRELATION_H_

#include "resource.h"       // main symbols


#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include "UNOXWrapper.h"

/**
 * CAccRelation implements IAccessibleRelation interface.
 */
class ATL_NO_VTABLE CAccRelation :
            public CComObjectRoot,
            public CComCoClass<CAccRelation, &CLSID_AccRelation>,
            public IAccessibleRelation,
            public CUNOXWrapper
{
public:
    CAccRelation()
    {
            }
    virtual ~CAccRelation()
    {
            }

    DECLARE_REGISTRY_RESOURCEID(IDR_ACCRELATION)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CAccRelation)
    COM_INTERFACE_ENTRY(IAccessibleRelation)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    END_COM_MAP()

    // IAccessibleRelation
public:
    // IAccessibleRelation

    // Gets what the type of relation is.
    STDMETHOD(get_relationType)(BSTR * relationType);

    // Gets what the type of localized relation is.
    STDMETHOD(get_localizedRelationType)(BSTR * relationType);

    // Gets how many targets this relation have.
    STDMETHOD(get_nTargets)(long * nTargets);

    // Gets one accessible relation target.
    STDMETHOD(get_target)(long targetIndex, IUnknown * * target);

    // Gets multiple accessible relation targets.
    STDMETHOD(get_targets)(long maxTargets, IUnknown * * target, long * nTargets);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XSubInterface)(long pXSubInterface);

    //static OLECHAR* getRelationTypeOLECHAR(int type);
    static BSTR getRelationTypeBSTR(int type);

private:

    com::sun::star::accessibility::AccessibleRelation relation;
};

#endif //__ACCRELATION_H_
