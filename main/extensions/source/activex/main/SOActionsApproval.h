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

// SOActionsApproval.h: Definition of the SOActionsApproval class
//
//////////////////////////////////////////////////////////////////////

#if !defined __SODOCUMENTEVENTLISTENER_H_
#define __SODOCUMENTEVENTLISTENER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols
#include <ExDispID.h>
#include <ExDisp.h>
#include <shlguid.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif

#include <atlctl.h>

#include "so_activex.h"

/////////////////////////////////////////////////////////////////////////////
// SOActionsApproval

class SOActionsApproval :
	public IDispatchImpl<ISOActionsApproval, &IID_ISOActionsApproval, &LIBID_SO_ACTIVEXLib>,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<SOActionsApproval,&CLSID_SOActionsApproval>
{
public:
	SOActionsApproval() {}
	virtual ~SOActionsApproval() {}

BEGIN_COM_MAP(SOActionsApproval)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISOActionsApproval)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(SOActionsApproval)
// Remove the comment from the line above if you don't want your object to
// support aggregation.

DECLARE_REGISTRY_RESOURCEID(IDR_SODOCUMENTEVENTLISTENER)

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ISOActionsApproval
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE approveAction( 
            /* [in] */ long nActionID,
            /* [retval][out] */ boolean *pbApproval)
		{
			// only PreventClose is approved
			USES_CONVERSION;
			*pbApproval = ( nActionID == 1 );

			return S_OK;
		}

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Bridge_implementedInterfaces(
            /* [retval][out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pVal)
		{
			*pVal = SafeArrayCreateVector( VT_BSTR, 0, 1 );

			if( !*pVal )
				return E_FAIL;

			long ix = 0;
            CComBSTR aInterface( OLESTR( "com.sun.star.embed.XActionsApproval" ) );
			SafeArrayPutElement( *pVal, &ix, aInterface );

			return S_OK;
		}
};

#endif // __SODOCUMENTEVENTLISTENER_H_

