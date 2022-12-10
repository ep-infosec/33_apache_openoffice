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



#ifndef _XMLSTREAMIO_XMLSECIMPL_HXX_
#define _XMLSTREAMIO_XMLSECIMPL_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.hxx>
#endif
#include <com/sun/star/xml/crypto/XUriBinding.hpp>

int xmlEnableStreamInputCallbacks( void ) ;
void xmlDisableStreamInputCallbacks( void ) ;

int xmlRegisterStreamInputCallbacks( 
	::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding >& aUriBinding
) ;

int xmlUnregisterStreamInputCallbacks( void ) ;

#endif //_XMLSTREAMIO_XMLSECIMPL_HXX_

