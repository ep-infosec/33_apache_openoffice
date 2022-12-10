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
#include "precompiled_sdext.hxx"

#include "pppoptimizer.hxx"
#include "impoptimizer.hxx"
#include <osl/file.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#define SERVICE_NAME "com.sun.star.presentation.PresentationOptimizer"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

using ::rtl::OUString;

// ----------------
// - PPPOptimizer -
// ----------------

PPPOptimizer::PPPOptimizer( const Reference< XComponentContext > &rxContext ) :
    mxContext( rxContext )
{
    OSL_TRACE("PPPOptimizer::PPPOptimizer");
}

// -----------------------------------------------------------------------------

PPPOptimizer::~PPPOptimizer()
{
    OSL_TRACE("PPPOptimizer::~PPPOptimizer");
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void SAL_CALL PPPOptimizer::initialize( const Sequence< Any >& aArguments )
	throw ( Exception, RuntimeException )
{
    OSL_TRACE("PPPOptimizer::initialize");
	if( aArguments.getLength() != 1 )
		throw IllegalArgumentException();

	Reference< XFrame > xFrame;
	aArguments[ 0 ] >>= xFrame;
	if ( xFrame.is() )
		mxController = xFrame->getController();
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString SAL_CALL PPPOptimizer::getImplementationName()
	throw ( RuntimeException )
{
	return PPPOptimizer_getImplementationName();
}

sal_Bool SAL_CALL PPPOptimizer::supportsService( const OUString& rServiceName )
	throw ( RuntimeException )
{
    return rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
}

Sequence< OUString > SAL_CALL PPPOptimizer::getSupportedServiceNames()
	throw ( RuntimeException )
{
    return PPPOptimizer_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------
// XDispatchProvider
// -----------------------------------------------------------------------------

Reference< com::sun::star::frame::XDispatch > SAL_CALL PPPOptimizer::queryDispatch(
	const URL& aURL, const ::rtl::OUString& /* aTargetFrameName */, sal_Int32 /* nSearchFlags */ ) throw( RuntimeException )
{
	Reference < XDispatch > xRet;
    if ( aURL.Protocol.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
        "vnd.com.sun.star.presentation.PresentationOptimizer:" ) ) )
    {
//		if ( aURL.Path.compareToAscii( "Function1" ) == 0 )
		xRet = this;
    }
	return xRet;
}

//------------------------------------------------------------------------------

Sequence< Reference< com::sun::star::frame::XDispatch > > SAL_CALL PPPOptimizer::queryDispatches(
	const Sequence< com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw( RuntimeException )
{
	Sequence< Reference< com::sun::star::frame::XDispatch> > aReturn( aDescripts.getLength() );
	Reference< com::sun::star::frame::XDispatch>* pReturn = aReturn.getArray();
	const com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
	for (sal_Int16 i = 0; i < aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
	{
		*pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
	}
	return aReturn;
}

// -----------------------------------------------------------------------------
// XDispatch
// -----------------------------------------------------------------------------

void SAL_CALL PPPOptimizer::dispatch( const URL& rURL, const Sequence< PropertyValue >& lArguments )
    throw( RuntimeException )
{
    OSL_TRACE("PPPOptimizer::dispatch");
	if ( mxController.is() && rURL.Protocol.equalsAsciiL(
        RTL_CONSTASCII_STRINGPARAM(
            "vnd.com.sun.star.presentation.PresentationOptimizer:" )))
	{
		if ( rURL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("optimize") ) )
		{
			Reference< XModel > xModel( mxController->getModel() );
			if ( xModel.is() )
			{
				try
				{
					ImpOptimizer aOptimizer( mxContext, xModel );
					aOptimizer.Optimize( lArguments );
				}
				catch( Exception& )
				{
				}
			}
		}
	}
}

//===============================================
void SAL_CALL PPPOptimizer::addStatusListener( const Reference< XStatusListener >&, const URL& )
	throw( RuntimeException )
{
    // TODO
    OSL_ENSURE( sal_False, "PPPOptimizer::addStatusListener()\nNot implemented yet!" );
}

//===============================================
void SAL_CALL PPPOptimizer::removeStatusListener( const Reference< XStatusListener >&, const URL& )
    throw( RuntimeException )
{
    // TODO
    OSL_ENSURE( sal_False, "PPPOptimizer::removeStatusListener()\nNot implemented yet!" );
}

// -----------------------------------------------------------------------------
// returning filesize, on error zero is returned
sal_Int64 PPPOptimizer::GetFileSize( const rtl::OUString& rURL )
{
	sal_Int64 nFileSize = 0;
	osl::DirectoryItem aItem;
	if ( osl::DirectoryItem::get( rURL, aItem ) == osl::FileBase::E_None )
	{
		osl::FileStatus aStatus( osl_FileStatus_Mask_FileSize );
		if ( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None )
		{
			nFileSize = aStatus.getFileSize();
		}
	}
	return nFileSize;
}

// -----------------------------------------------------------------------------

OUString PPPOptimizer_getImplementationName()
{
	return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.presentation.PresentationOptimizer" ) );
}

Sequence< OUString > PPPOptimizer_getSupportedServiceNames()
{
	Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return aRet;
}

Reference< XInterface > PPPOptimizer_createInstance( const Reference< XComponentContext > & rSMgr )
	throw( Exception )
{
	return (cppu::OWeakObject*) new PPPOptimizer( rSMgr );
}
