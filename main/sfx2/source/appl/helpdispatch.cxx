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
#include "precompiled_sfx2.hxx"

#include "helpdispatch.hxx"
#include <sfx2/sfxuno.hxx>
#include "newhelp.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

// class HelpInterceptor_Impl --------------------------------------------

HelpDispatch_Impl::HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
									  const ::com::sun::star::uno::Reference<
									  	::com::sun::star::frame::XDispatch >& _xDisp ) :

	m_rInterceptor	( _rInterceptor ),
	m_xRealDispatch	( _xDisp )

{
}

// -----------------------------------------------------------------------

HelpDispatch_Impl::~HelpDispatch_Impl()
{
}

// -----------------------------------------------------------------------
// XDispatch

void SAL_CALL HelpDispatch_Impl::dispatch(

	const URL& aURL, const Sequence< PropertyValue >& aArgs ) throw( RuntimeException )

{
	DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );

	// search for a keyword (dispatch from the basic ide)
	sal_Bool bHasKeyword = sal_False;
	String sKeyword;
	const PropertyValue* pBegin = aArgs.getConstArray();
	const PropertyValue* pEnd	= pBegin + aArgs.getLength();
	for ( ; pBegin != pEnd; ++pBegin )
	{
		if ( 0 == ( *pBegin ).Name.compareToAscii( "HelpKeyword" ) )
		{
			rtl::OUString sHelpKeyword;
			if ( ( ( *pBegin ).Value >>= sHelpKeyword ) && sHelpKeyword.getLength() > 0 )
			{
				sKeyword = String( sHelpKeyword );
				bHasKeyword = ( sKeyword.Len() > 0 );
				break;
			}
		}
	}

	// if a keyword was found, then open it
	SfxHelpWindow_Impl* pHelpWin = m_rInterceptor.GetHelpWindow();
	DBG_ASSERT( pHelpWin, "invalid HelpWindow" );
	if ( bHasKeyword )
    {
		pHelpWin->OpenKeyword( sKeyword );
        return;
    }

    pHelpWin->loadHelpContent(aURL.Complete);
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::addStatusListener(

	const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
	DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
	m_xRealDispatch->addStatusListener( xControl, aURL );
}

// -----------------------------------------------------------------------

void SAL_CALL HelpDispatch_Impl::removeStatusListener(

	const Reference< XStatusListener >& xControl, const URL& aURL ) throw( RuntimeException )

{
	DBG_ASSERT( m_xRealDispatch.is(), "invalid dispatch" );
	m_xRealDispatch->removeStatusListener( xControl, aURL );
}

