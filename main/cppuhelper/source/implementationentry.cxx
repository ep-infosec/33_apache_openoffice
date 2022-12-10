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
#include "precompiled_cppuhelper.hxx"
#include <cppuhelper/implementationentry.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace cppu {
	
sal_Bool component_writeInfoHelper(
	void *, void *pRegistryKey , const struct ImplementationEntry entries[] )
{
	sal_Bool bRet = sal_False;
	try
	{
		if( pRegistryKey )
		{
			for( sal_Int32 i = 0; entries[i].create ; i ++ )
			{
				OUStringBuffer buf( 124 );
				buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/") );
				buf.append( entries[i].getImplementationName() );
				buf.appendAscii(RTL_CONSTASCII_STRINGPARAM( "/UNO/SERVICES" ) );
				Reference< XRegistryKey > xNewKey(
					reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear()  ) );

				Sequence< OUString > seq = entries[i].getSupportedServiceNames();
				const OUString *pArray = seq.getConstArray();
				for ( sal_Int32 nPos = 0 ; nPos < seq.getLength(); nPos ++ )
					xNewKey->createKey( pArray[nPos] );
			}
			bRet = sal_True;
		}
	}
	catch ( InvalidRegistryException & )
	{
		OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
	}
	return bRet;
}


void * component_getFactoryHelper(
	const sal_Char * pImplName, void *, void *,
	const struct ImplementationEntry entries[] )
{
	
  	void * pRet = 0;
	Reference< XSingleComponentFactory > xFactory;
	
	for( sal_Int32 i = 0 ; entries[i].create ; i ++ )
	{
		OUString implName = entries[i].getImplementationName();
		if( 0 == implName.compareToAscii( pImplName ) )
		{
			xFactory = entries[i].createFactory(
				entries[i].create,
				implName,
				entries[i].getSupportedServiceNames(),
				entries[i].moduleCounter );
		}
	}

	if( xFactory.is() )
	{
		xFactory->acquire();
		pRet = xFactory.get();
	}
	return pRet;
}

}
