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


#include "vbaaddin.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaAddin::SwVbaAddin( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const rtl::OUString& rFileURL, sal_Bool bAutoload ) throw ( uno::RuntimeException ) :
    SwVbaAddin_BASE( rParent, rContext ), msFileURL( rFileURL ), mbAutoload( bAutoload ), mbInstalled( bAutoload )
{
}

SwVbaAddin::~SwVbaAddin()
{
}

::rtl::OUString SAL_CALL SwVbaAddin::getName() throw (uno::RuntimeException)
{
    rtl::OUString sName;
    INetURLObject aURL( msFileURL );
    ::osl::File::getSystemPathFromFileURL( aURL.GetLastName(), sName );
    return sName;
}

void SAL_CALL
SwVbaAddin::setName( const rtl::OUString& ) throw ( css::uno::RuntimeException )
{
    throw uno::RuntimeException( rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(" Fail to set name")), uno::Reference< uno::XInterface >() );
}

::rtl::OUString SAL_CALL SwVbaAddin::getPath() throw (uno::RuntimeException)
{
    INetURLObject aURL( msFileURL );
    aURL.CutLastName();
    return aURL.GetURLPath();
}

::sal_Bool SAL_CALL SwVbaAddin::getAutoload() throw (uno::RuntimeException)
{
    return mbAutoload;
}

::sal_Bool SAL_CALL SwVbaAddin::getInstalled() throw (uno::RuntimeException)
{
    return mbInstalled;
}

void SAL_CALL SwVbaAddin::setInstalled( ::sal_Bool _installed ) throw (uno::RuntimeException)
{
    if( _installed != mbInstalled )
    {
        mbInstalled = _installed;
        // TODO: should call AutoExec and AutoExit etc.
    }
}

rtl::OUString& 
SwVbaAddin::getServiceImplName()
{
	static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaAddin") );
	return sImplName;
}

uno::Sequence< rtl::OUString > 
SwVbaAddin::getServiceNames()
{
	static uno::Sequence< rtl::OUString > aServiceNames;
	if ( aServiceNames.getLength() == 0 )
	{
		aServiceNames.realloc( 1 );
		aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Addin" ) );
	}
	return aServiceNames;
}

