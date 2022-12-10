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



#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#define _XMLOFF_TRANSFORMERACTIONS_HXX

#include <rtl/ustring.hxx>
#include <xmloff/nmspmap.hxx>

#include <hash_map>
#include "TransformerActionInit.hxx"
#include "TransformerAction.hxx"

struct NameKey_Impl
{
	sal_uInt16 m_nPrefix;
	::rtl::OUString m_aLocalName;

	inline NameKey_Impl( sal_uInt16 nPrfx, 
						 ::xmloff::token::XMLTokenEnum eLclNm ) :
		m_nPrefix( nPrfx ), 
		m_aLocalName( ::xmloff::token::GetXMLToken( eLclNm ) )
	{
	}

	inline NameKey_Impl( sal_uInt16 nPrfx, const ::rtl::OUString& rLclNm ) :
		m_nPrefix( nPrfx ), 
		m_aLocalName( rLclNm )
	{
	}

	inline NameKey_Impl() :
		m_nPrefix( XML_NAMESPACE_UNKNOWN )
	{
	}

	inline void SetLocalName( ::xmloff::token::XMLTokenEnum eLclNm )
	{
		m_aLocalName = ::xmloff::token::GetXMLToken( eLclNm );
	}
};

// -----------------------------------------------------------------------------

struct NameHash_Impl
{
	inline size_t operator()( const NameKey_Impl& r ) const;
	inline bool operator()( const NameKey_Impl& r1,
				   			const NameKey_Impl& r2 ) const;
};

inline size_t NameHash_Impl::operator()( const NameKey_Impl& r ) const
{
	return static_cast< size_t >( r.m_nPrefix ) + 
		   static_cast< size_t >( r.m_aLocalName.hashCode() );
}

inline bool NameHash_Impl::operator()(
		const NameKey_Impl& r1,
		const NameKey_Impl& r2 ) const
{
	return r1.m_nPrefix == r2.m_nPrefix && r1.m_aLocalName == r2.m_aLocalName;
}

// -----------------------------------------------------------------------------

struct TransformerAction_Impl
{
	sal_uInt32 m_nActionType;
	sal_uInt32 m_nParam1;
	sal_uInt32 m_nParam2;
	sal_uInt32 m_nParam3;

	inline TransformerAction_Impl( sal_uInt32 nActnTp, sal_uInt32 nPrm1,
								   sal_uInt32 nPrm2, sal_uInt32 nPrm3 ) :
		m_nActionType( nActnTp ), 
		m_nParam1( nPrm1 ), 
		m_nParam2( nPrm2 ), 
		m_nParam3( nPrm3 )
	{

	}
	inline TransformerAction_Impl() :
		m_nActionType( XML_TACTION_EOT ), 
		m_nParam1( 0 ), 
		m_nParam2( 0 ), 
		m_nParam3( 0 )
	{
	}

	sal_uInt16 GetQNamePrefixFromParam1() const
	{
		return static_cast< sal_uInt16 >( m_nParam1 >> 16 );
	}

	sal_uInt16 GetQNamePrefixFromParam2() const
	{
		return static_cast< sal_uInt16 >( m_nParam2 >> 16 );
	}

	sal_uInt16 GetQNamePrefixFromParam3() const
	{
		return static_cast< sal_uInt16 >( m_nParam3 >> 16 );
	}

	::xmloff::token::XMLTokenEnum GetQNameTokenFromParam1() const
	{
		return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam1 & 0xffff );
	}

	::xmloff::token::XMLTokenEnum GetQNameTokenFromParam2() const
	{
		return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam2 & 0xffff );
	}

	::xmloff::token::XMLTokenEnum GetQNameTokenFromParam3() const
	{
		return static_cast< ::xmloff::token::XMLTokenEnum>( m_nParam3 & 0xffff );
	}

};


// -----------------------------------------------------------------------------

class XMLTransformerActions :
	public ::std::hash_map< NameKey_Impl, TransformerAction_Impl, 
						    NameHash_Impl, NameHash_Impl >
{
public:
	XMLTransformerActions( XMLTransformerActionInit *pInit );
	~XMLTransformerActions();

	void Add( XMLTransformerActionInit *pInit );
};

#endif	//  _XMLOFF_TRANSFORMERACTIONS_HXX
