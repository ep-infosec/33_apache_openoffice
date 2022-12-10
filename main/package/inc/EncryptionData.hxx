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


#ifndef _ENCRYPTION_DATA_HXX_
#define _ENCRYPTION_DATA_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

class BaseEncryptionData : public cppu::OWeakObject
{
public:
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aSalt;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aInitVector;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aDigest;
    sal_Int32 m_nIterationCount;

    BaseEncryptionData()
    : m_nIterationCount ( 0 ){}

    BaseEncryptionData( const BaseEncryptionData& aData )
    : cppu::OWeakObject()
    , m_aSalt( aData.m_aSalt )
    , m_aInitVector( aData.m_aInitVector )
    , m_aDigest( aData.m_aDigest )
    , m_nIterationCount( aData.m_nIterationCount )
    {}
};

class EncryptionData : public BaseEncryptionData
{
public:
    ::com::sun::star::uno::Sequence < sal_Int8 > m_aKey;
    sal_Int32 m_nEncAlg;
    sal_Int32 m_nCheckAlg;
    sal_Int32 m_nDerivedKeySize;
    sal_Int32 m_nStartKeyGenID;

    EncryptionData( const BaseEncryptionData& aData, const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey, sal_Int32 nEncAlg, sal_Int32 nCheckAlg, sal_Int32 nDerivedKeySize, sal_Int32 nStartKeyGenID )
    : BaseEncryptionData( aData )
    , m_aKey( aKey )
    , m_nEncAlg( nEncAlg )
    , m_nCheckAlg( nCheckAlg )
    , m_nDerivedKeySize( nDerivedKeySize )
    , m_nStartKeyGenID( nStartKeyGenID )
    {}

    EncryptionData( const EncryptionData& aData )
    : BaseEncryptionData( aData )
    , m_aKey( aData.m_aKey )
    , m_nEncAlg( aData.m_nEncAlg )
    , m_nCheckAlg( aData.m_nCheckAlg )
    , m_nDerivedKeySize( aData.m_nDerivedKeySize )
    , m_nStartKeyGenID( aData.m_nStartKeyGenID )
    {}
};

#endif
