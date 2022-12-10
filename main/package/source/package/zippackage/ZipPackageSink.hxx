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


#ifndef _ZIP_PACKAGE_SINK_HXX
#define _ZIP_PACKAGE_SINK_HXX

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <cppuhelper/implbase1.hxx>

class ZipPackageSink : public ::cppu::WeakImplHelper1
<
	com::sun::star::io::XActiveDataSink
>
{
protected:
	com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
public:
	ZipPackageSink();
	virtual ~ZipPackageSink();
    virtual void SAL_CALL setInputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream ) 
		throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) 
		throw(::com::sun::star::uno::RuntimeException);
};
#endif
