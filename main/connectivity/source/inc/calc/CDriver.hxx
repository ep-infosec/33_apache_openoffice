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



#ifndef _CONNECTIVITY_CALC_DRIVER_HXX_
#define _CONNECTIVITY_CALC_DRIVER_HXX_

#include "file/FDriver.hxx"

namespace connectivity
{
	namespace calc
	{
		::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
			ODriver_CreateInstance(const ::com::sun::star::uno::Reference<
						::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
					throw( ::com::sun::star::uno::Exception );

		class ODriver : public file::OFileDriver
		{
		public:
			ODriver(const ::com::sun::star::uno::Reference<
								::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) :
				file::OFileDriver(_rxFactory){}

			static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
			::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);

			// XDriver
			virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL
					connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence<
								::com::sun::star::beans::PropertyValue >& info )
						throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
			virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url )
						throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
			virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
		};
	}
}

#endif //_CONNECTIVITY_CALC_DRIVER_HXX_

