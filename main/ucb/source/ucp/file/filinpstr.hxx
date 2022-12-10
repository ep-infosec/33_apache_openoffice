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

 
#ifndef _FILINPSTR_HXX_
#define _FILINPSTR_HXX_

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>

#include "filrec.hxx"

namespace fileaccess {

	// forward declaration
	
	class shell;


	class XInputStream_impl
		: public cppu::OWeakObject,
		  public com::sun::star::lang::XTypeProvider,
		  public com::sun::star::io::XInputStream,
		  public com::sun::star::io::XSeekable
	{
	public:
		
		XInputStream_impl( shell* pMyShell,const rtl::OUString& aUncPath, sal_Bool bLock );
		
		virtual ~XInputStream_impl();
		
		/**
		 *  Returns an error code as given by filerror.hxx
		 */
		
		sal_Int32 SAL_CALL CtorSuccess();
		sal_Int32 SAL_CALL getMinorError();


		// XTypeProvider

		XTYPEPROVIDER_DECL()
		
		virtual com::sun::star::uno::Any SAL_CALL
		queryInterface(
			const com::sun::star::uno::Type& rType )
			throw( com::sun::star::uno::RuntimeException);
		
		virtual void SAL_CALL
		acquire(
			void )
			throw();
		
		virtual void SAL_CALL
		release(
			void )
			throw();

		virtual sal_Int32 SAL_CALL
		readBytes(
			com::sun::star::uno::Sequence< sal_Int8 >& aData,
			sal_Int32 nBytesToRead )
			throw( com::sun::star::io::NotConnectedException,
				   com::sun::star::io::BufferSizeExceededException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException);

		virtual sal_Int32 SAL_CALL
		readSomeBytes(
			com::sun::star::uno::Sequence< sal_Int8 >& aData,
			sal_Int32 nMaxBytesToRead )
			throw( com::sun::star::io::NotConnectedException,
				   com::sun::star::io::BufferSizeExceededException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException);

		virtual void SAL_CALL
		skipBytes(
			sal_Int32 nBytesToSkip )
			throw( com::sun::star::io::NotConnectedException,
				   com::sun::star::io::BufferSizeExceededException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

		virtual sal_Int32 SAL_CALL
		available(
			void )
			throw( com::sun::star::io::NotConnectedException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

		virtual void SAL_CALL
		closeInput(
			void )
			throw( com::sun::star::io::NotConnectedException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

		virtual void SAL_CALL
		seek(
			sal_Int64 location )
			throw( com::sun::star::lang::IllegalArgumentException,
				   com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

		virtual sal_Int64 SAL_CALL
		getPosition(
			void )
			throw( com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

		virtual sal_Int64 SAL_CALL
		getLength(
			void )
			throw( com::sun::star::io::IOException,
				   com::sun::star::uno::RuntimeException );

	private:
		
		shell*                                             m_pMyShell;
		com::sun::star::uno::Reference< 
		com::sun::star::ucb::XContentProvider >            m_xProvider;
		sal_Bool                                           m_nIsOpen;

        sal_Bool                                           m_bLock;

		ReconnectingFile                                   m_aFile;

		sal_Int32                                          m_nErrorCode;
		sal_Int32                                          m_nMinorErrorCode;
	};
	

} // end namespace XInputStream_impl

#endif
