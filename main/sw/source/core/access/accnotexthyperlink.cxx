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

#include <comphelper/accessiblekeybindinghelper.hxx>
#include <swurl.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <ndtxt.hxx>
#include <txtinet.hxx>
#include <frmfmt.hxx>

#include <accnotexthyperlink.hxx>

#include <fmturl.hxx>

#include <svtools/imap.hxx>
#include <svtools/imapobj.hxx>

#include <accmap.hxx>
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
//using namespace ::rtl;

SwAccessibleNoTextHyperlink::SwAccessibleNoTextHyperlink( SwAccessibleNoTextFrame *p, const SwFrm *aFrm, sal_uInt16 nIndex) :
	xFrame( p ),
	mpFrm( aFrm ),
	mnIndex(nIndex)
{
}

// XAccessibleAction
sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionCount() 
		throw (RuntimeException)
{
	SwFmtURL aURL( GetFmt()->GetURL() );
	ImageMap* pMap = aURL.GetMap();
	if( pMap != NULL )
	{
		return	pMap->GetIMapObjectCount();
	}else if( aURL.GetURL().Len() )
	{
		return 1;
	}

	return 0;
}

sal_Bool SAL_CALL SwAccessibleNoTextHyperlink::doAccessibleAction( sal_Int32 nIndex ) 
		throw (IndexOutOfBoundsException, RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());

	if(nIndex < 0 || nIndex >= getAccessibleActionCount())
		throw new IndexOutOfBoundsException;

	sal_Bool bRet = sal_False;
	SwFmtURL aURL( GetFmt()->GetURL() );
	ImageMap* pMap = aURL.GetMap();
	if( pMap != NULL )
	{	
		IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
		if(pMapObj->GetURL().Len())
		{
			ViewShell *pVSh = xFrame->GetShell();
			if( pVSh )
			{
				LoadURL( pMapObj->GetURL(), pVSh, URLLOAD_NOFILTER, 
						 &pMapObj->GetTarget() );
				bRet = sal_True;
			}
		}
	}
	else if( aURL.GetURL().Len() )
	{
		ViewShell *pVSh = xFrame->GetShell();
		if( pVSh )
		{
			LoadURL( aURL.GetURL(), pVSh, URLLOAD_NOFILTER, 
					 &aURL.GetTargetFrameName() );
			bRet = sal_True;
		}
	}

	return bRet;
}
		
rtl::OUString SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionDescription(
		sal_Int32 nIndex ) 
		throw (IndexOutOfBoundsException, RuntimeException)
{
	rtl::OUString sDesc;

	if(nIndex < 0 || nIndex >= getAccessibleActionCount())
		throw new IndexOutOfBoundsException;

	SwFmtURL aURL( GetFmt()->GetURL() );
	ImageMap* pMap = aURL.GetMap();
	if( pMap != NULL )
	{
		IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
		if(pMapObj->GetDesc().Len())
			sDesc = rtl::OUString( pMapObj->GetDesc() );
		else if(pMapObj->GetURL().Len())
			sDesc = rtl::OUString( pMapObj->GetURL() );
	}
	else if( aURL.GetURL().Len() )
		sDesc = rtl::OUString( aURL.GetName() );
		//sDesc = rtl::OUString( aURL.GetName() );

	return sDesc;
}

Reference< XAccessibleKeyBinding > SAL_CALL
	SwAccessibleNoTextHyperlink::getAccessibleActionKeyBinding( sal_Int32 nIndex ) 
	throw (IndexOutOfBoundsException, RuntimeException)
{
	Reference< XAccessibleKeyBinding > xKeyBinding;

	if(nIndex < 0 || nIndex >= getAccessibleActionCount())
		throw new IndexOutOfBoundsException;

	bool bIsValid = sal_False;
	SwFmtURL aURL( GetFmt()->GetURL() );
	ImageMap* pMap = aURL.GetMap();
	if( pMap != NULL )
	{
		IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
		if(pMapObj->GetURL().Len())
			bIsValid = sal_True;
	}else if( aURL.GetURL().Len() )
		bIsValid = sal_True;
	
	if(bIsValid)
	{
		::comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper =
		   	new ::comphelper::OAccessibleKeyBindingHelper();
		xKeyBinding = pKeyBindingHelper;

		::com::sun::star::awt::KeyStroke aKeyStroke;
		aKeyStroke.Modifiers = 0;
		aKeyStroke.KeyCode = KEY_RETURN;
		aKeyStroke.KeyChar = 0;
		aKeyStroke.KeyFunc = 0;
		pKeyBindingHelper->AddKeyBinding( aKeyStroke );
	}

	return xKeyBinding;
}

// XAccessibleHyperlink
Any SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionAnchor(
		sal_Int32 nIndex ) 
		throw (IndexOutOfBoundsException, RuntimeException)
{
	if(nIndex < 0 || nIndex >= getAccessibleActionCount())
		throw new IndexOutOfBoundsException;

	Any aRet;
	//SwFrm* pAnchor = ((SwFlyFrm*)mpFrm)->GetAnchor();
	Reference< XAccessible > xAnchor = xFrame->GetAccessibleMap()->GetContext(mpFrm, sal_True);
	//SwAccessibleNoTextFrame* pFrame = xFrame.get();
	//Reference< XAccessible > xAnchor = (XAccessible*)pFrame;
	aRet <<= xAnchor;
	return aRet;
}

Any SAL_CALL SwAccessibleNoTextHyperlink::getAccessibleActionObject( 
			sal_Int32 nIndex ) 
	throw (IndexOutOfBoundsException, RuntimeException)
{
	if(nIndex < 0 || nIndex >= getAccessibleActionCount())
		throw new IndexOutOfBoundsException;

	SwFmtURL aURL( GetFmt()->GetURL() );
	::rtl::OUString retText;
	ImageMap* pMap = aURL.GetMap();
	if( pMap != NULL )
	{
		IMapObject* pMapObj = pMap->GetIMapObject(nIndex);
		if(pMapObj->GetURL().Len())
			retText = rtl::OUString( pMapObj->GetURL() );
	}else if( aURL.GetURL().Len() )
		retText = rtl::OUString( aURL.GetURL() );

	Any aRet;
	aRet <<= retText;
	return aRet;
}

sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getStartIndex() 
		throw (RuntimeException)
{
	return 0;
}

sal_Int32 SAL_CALL SwAccessibleNoTextHyperlink::getEndIndex() 
		throw (RuntimeException)
{
	return 0;
}

sal_Bool SAL_CALL SwAccessibleNoTextHyperlink::isValid(  ) 
		throw (::com::sun::star::uno::RuntimeException)
{
	SwFmtURL aURL( GetFmt()->GetURL() );

	if( aURL.GetMap() || aURL.GetURL().Len() )
		return sal_True;
	return sal_False;
}
