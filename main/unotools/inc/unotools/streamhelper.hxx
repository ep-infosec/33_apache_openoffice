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


#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#define _UNOTOOLS_STREAMHELPER_HXX_
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <tools/stream.hxx>

namespace utl
{
	namespace stario	= ::com::sun::star::io;
	namespace staruno	= ::com::sun::star::uno;

/**
 * The helper implementation for a using input streams based on SvLockBytes.
 *
 * @author	Dirk Grobler
 * @since	00/28/03
 */
    typedef ::cppu::WeakImplHelper2<stario::XInputStream, stario::XSeekable> InputStreamHelper_Base;
	// needed for some compilers
class UNOTOOLS_DLLPUBLIC OInputStreamHelper : public InputStreamHelper_Base
{
	::osl::Mutex	m_aMutex;
	SvLockBytesRef	m_xLockBytes;
	sal_uInt32		m_nActPos;
	sal_Int32		m_nAvailable;	// this is typically the chunk(buffer) size

public:
	OInputStreamHelper(const SvLockBytesRef& _xLockBytes,
					   sal_uInt32 _nAvailable,
					   sal_uInt32 _nPos = 0)
		:m_xLockBytes(_xLockBytes)
		,m_nActPos(_nPos)
		,m_nAvailable(_nAvailable){}

// staruno::XInterface
	virtual void SAL_CALL acquire() throw ();
	virtual void SAL_CALL release() throw ();

// stario::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  ) throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (stario::NotConnectedException, stario::IOException, staruno::RuntimeException);

    virtual void SAL_CALL seek( sal_Int64 location ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

/**
 * The helper implementation for a using output streams based on SvLockBytes.
 *
 * @author	Dirk Grobler
 * @since	00/28/03
 */
typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamHelper_Base;
	// needed for some compilers
class UNOTOOLS_DLLPUBLIC OOutputStreamHelper : public OutputStreamHelper_Base
{
	::osl::Mutex	m_aMutex;
	SvLockBytesRef	m_xLockBytes;
	sal_uInt32		m_nActPos;

public:
	OOutputStreamHelper(const SvLockBytesRef& _xLockBytes, sal_uInt32 _nPos = 0)
		:m_xLockBytes(_xLockBytes)
		,m_nActPos(_nPos){}

// staruno::XInterface
	virtual void SAL_CALL acquire() throw ();
	virtual void SAL_CALL release() throw ();

// stario::XOutputStream
    virtual void SAL_CALL writeBytes( const staruno::Sequence< sal_Int8 >& aData ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
};

}	// namespace utl


#endif // _UNOTOOLS_STREAM_WRAPPER_HXX_

