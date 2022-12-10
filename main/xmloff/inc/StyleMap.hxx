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



#ifndef _XMLOFF_STYLEMAP_HXX
#define _XMLOFF_STYLEMAP_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase1.hxx>
#include <hash_map>

struct StyleNameKey_Impl
{
	sal_uInt16 m_nFamily;
	::rtl::OUString m_aName;

	inline StyleNameKey_Impl( sal_uInt16 nFamily, 
						 	  const ::rtl::OUString& rName ) :
		m_nFamily( nFamily ), 
		m_aName( rName )
	{
	}

	inline StyleNameKey_Impl() :
		m_nFamily( 0 )
	{
	}
};

struct StyleNameHash_Impl
{
	inline size_t operator()( const StyleNameKey_Impl& r ) const;
	inline bool operator()( const StyleNameKey_Impl& r1,
				   			const StyleNameKey_Impl& r2 ) const;
};

inline size_t StyleNameHash_Impl::operator()( const StyleNameKey_Impl& r ) const
{
	return static_cast< size_t >( r.m_nFamily ) + 
		   static_cast< size_t >( r.m_aName.hashCode() );
}

inline bool StyleNameHash_Impl::operator()(
		const StyleNameKey_Impl& r1,
		const StyleNameKey_Impl& r2 ) const
{
	return r1.m_nFamily == r2.m_nFamily && r1.m_aName == r2.m_aName;
}

class StyleMap : 
	public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XUnoTunnel>,
	public ::std::hash_map< StyleNameKey_Impl, ::rtl::OUString, 
						    StyleNameHash_Impl, StyleNameHash_Impl >
{

public:

	StyleMap();
	virtual ~StyleMap();

	static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
	static StyleMap* getImplementation( 
			::com::sun::star::uno::Reference< 
				::com::sun::star::uno::XInterface > ) throw();

	// XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( 
				const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);			 
};

#endif	//  _XMLOFF_STYLEMAP_HXX

