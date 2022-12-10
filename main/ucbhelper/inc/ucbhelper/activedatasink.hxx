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



#ifndef _UCBHELPER_ACTIVEDATASINK_HXX
#define _UCBHELPER_ACTIVEDATASINK_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{

//=========================================================================

/**
  * This class implements the interface com::sun::star::io::XActiveDataSink.
  * Instances of this class can be passed with the parameters of an
  * "open" command.
  */

class UCBHELPER_DLLPUBLIC ActiveDataSink : public cppu::OWeakObject,
					   public com::sun::star::lang::XTypeProvider,
					   public com::sun::star::io::XActiveDataSink
{
	com::sun::star::uno::Reference<
		com::sun::star::io::XInputStream > m_xStream;

public:
 	// XInterface
	XINTERFACE_DECL()

	// XTypeProvider
	XTYPEPROVIDER_DECL()

	// XActiveDataSink methods.
    virtual void SAL_CALL
	setInputStream( const com::sun::star::uno::Reference<
							com::sun::star::io::XInputStream >& aStream )
		throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
		com::sun::star::io::XInputStream > SAL_CALL
	getInputStream()
		throw( com::sun::star::uno::RuntimeException );
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_ACTIVEDATASINK_HXX */
