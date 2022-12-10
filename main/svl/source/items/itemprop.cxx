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
#include "precompiled_svl.hxx"

#include <svl/itemprop.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <hash_map>
/*************************************************************************
	UNO III Implementation
*************************************************************************/
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

/*-- 16.02.2009 10:03:55---------------------------------------------------

  -----------------------------------------------------------------------*/

struct equalOUString
{
  bool operator()(const ::rtl::OUString& r1, const ::rtl::OUString&  r2) const
  {
    return r1.equals( r2 );
  }
};
  
typedef ::std::hash_map< ::rtl::OUString,
                                 SfxItemPropertySimpleEntry,
                                 ::rtl::OUStringHash,
                                 equalOUString > SfxItemPropertyHashMap_t;

class SfxItemPropertyMap_Impl : public SfxItemPropertyHashMap_t
{
public:    
    mutable uno::Sequence< beans::Property > m_aPropSeq;

    SfxItemPropertyMap_Impl(){}
    SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource );
};
SfxItemPropertyMap_Impl::SfxItemPropertyMap_Impl( const SfxItemPropertyMap_Impl* pSource )
{
    this->SfxItemPropertyHashMap_t::operator=( *pSource );
    m_aPropSeq = pSource->m_aPropSeq;
}        

/*-- 16.02.2009 10:03:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries ) :
    m_pImpl( new SfxItemPropertyMap_Impl )
{
    while( pEntries->pName )
    {
        ::rtl::OUString sEntry(pEntries->pName, pEntries->nNameLen, RTL_TEXTENCODING_ASCII_US );
        (*m_pImpl) [ sEntry ] = pEntries;
        ++pEntries;
    }    
}
/*-- 16.02.2009 12:46:41---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertyMap::SfxItemPropertyMap( const SfxItemPropertyMap* pSource ) :
    m_pImpl( new SfxItemPropertyMap_Impl( pSource->m_pImpl ) )
{
}
/*-- 16.02.2009 10:03:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertyMap::~SfxItemPropertyMap()
{
    delete m_pImpl;
}
/*-- 16.02.2009 10:03:51---------------------------------------------------

  -----------------------------------------------------------------------*/
const SfxItemPropertySimpleEntry* SfxItemPropertyMap::getByName( const ::rtl::OUString &rName ) const
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        return 0;
    return &aIter->second;
}

/*-- 16.02.2009 10:44:24---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence<beans::Property> SfxItemPropertyMap::getProperties() const
{
    if( !m_pImpl->m_aPropSeq.getLength() )
    {
        m_pImpl->m_aPropSeq.realloc( m_pImpl->size() );
        beans::Property* pPropArray = m_pImpl->m_aPropSeq.getArray();
        sal_uInt32 n = 0;
        SfxItemPropertyHashMap_t::const_iterator aIt = m_pImpl->begin();
        while( aIt != m_pImpl->end() )
        //for ( const SfxItemPropertyMap *pMap = _pMap; pMap->pName; ++pMap )
        {
            const SfxItemPropertySimpleEntry* pEntry = &(*aIt).second;
            pPropArray[n].Name = (*aIt).first;
            pPropArray[n].Handle = pEntry->nWID;
            if(pEntry->pType)
                pPropArray[n].Type = *pEntry->pType;
            pPropArray[n].Attributes =
                sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
            n++;
            ++aIt;
        }
    }

    return m_pImpl->m_aPropSeq;
}
/*-- 16.02.2009 11:04:31---------------------------------------------------
    
  -----------------------------------------------------------------------*/
beans::Property SfxItemPropertyMap::getPropertyByName( const ::rtl::OUString rName ) const 
    throw( beans::UnknownPropertyException )
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    if( aIter == m_pImpl->end() )
        throw UnknownPropertyException();    
    const SfxItemPropertySimpleEntry* pEntry = &aIter->second;
    beans::Property aProp;
    aProp.Name = rName;
    aProp.Handle = pEntry->nWID;
    if(pEntry->pType)
        aProp.Type = *pEntry->pType;
    aProp.Attributes = sal::static_int_cast< sal_Int16 >(pEntry->nFlags);
    return aProp;
}
/*-- 16.02.2009 11:09:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SfxItemPropertyMap::hasPropertyByName( const ::rtl::OUString& rName ) const 
{
    SfxItemPropertyHashMap_t::const_iterator aIter = m_pImpl->find(rName);
    return aIter != m_pImpl->end();
}
/*-- 16.02.2009 11:25:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SfxItemPropertyMap::mergeProperties( const uno::Sequence< beans::Property >& rPropSeq )
{
    const beans::Property* pPropArray = rPropSeq.getConstArray();
    sal_uInt32 nElements = rPropSeq.getLength();
    for( sal_uInt32 nElement = 0; nElement < nElements; ++nElement )
    {
        SfxItemPropertySimpleEntry aTemp(
            sal::static_int_cast< sal_Int16 >( pPropArray[nElement].Handle ), //nWID
            &pPropArray[nElement].Type, //pType
            pPropArray[nElement].Attributes, //nFlags
            0 ); //nMemberId
        (*m_pImpl)[pPropArray[nElement].Name] = aTemp;
    }
}
/*-- 18.02.2009 12:04:42---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyEntryVector_t SfxItemPropertyMap::getPropertyEntries() const
{
    PropertyEntryVector_t aRet;
    aRet.reserve(m_pImpl->size());
    
    SfxItemPropertyHashMap_t::const_iterator aIt = m_pImpl->begin();
    while( aIt != m_pImpl->end() )
    {
        const SfxItemPropertySimpleEntry* pEntry = &(*aIt).second;
        aRet.push_back( SfxItemPropertyNamedEntry( (*aIt).first, * pEntry ) );
        ++aIt;
    }
    return aRet;
}    
/*-- 18.02.2009 15:11:06---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_uInt32 SfxItemPropertyMap::getSize() const
{
    return m_pImpl->size();
}    
/*-- 16.02.2009 13:44:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertySet::~SfxItemPropertySet()
{
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SfxItemPropertySet::FillItem(SfxItemSet&, sal_uInt16, sal_Bool) const
{
	return sal_False;
}
/* -----------------------------06.06.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
			const SfxItemSet& rSet, Any& rAny ) const
						throw(RuntimeException)
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, sal_True, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    // return item values as uno::Any
    if(eState >= SFX_ITEM_DEFAULT && pItem)
    {
        pItem->QueryValue( rAny, rEntry.nMemberId );
    }
    else
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
        if(FillItem(aSet, rEntry.nWID, sal_True))
        {
            const SfxPoolItem& rItem = aSet.Get(rEntry.nWID);
            rItem.QueryValue( rAny, rEntry.nMemberId );
        }
        else if(0 == (rEntry.nFlags & PropertyAttribute::MAYBEVOID))
            throw RuntimeException();
    }


    // convert general SfxEnumItem values to specific values
    if( rEntry.pType && TypeClass_ENUM == rEntry.pType->getTypeClass() &&
         rAny.getValueTypeClass() == TypeClass_LONG )
    {
        sal_Int32 nTmp = *(sal_Int32*)rAny.getValue();
        rAny.setValue( &nTmp, *rEntry.pType );
    }
}
/* -----------------------------06.06.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::getPropertyValue( const rtl::OUString &rName,
			const SfxItemSet& rSet, Any& rAny ) const
						throw(RuntimeException, UnknownPropertyException)
{
    // detect which-id 
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
		throw UnknownPropertyException();
    getPropertyValue( *pEntry,rSet, rAny );
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
Any SfxItemPropertySet::getPropertyValue( const rtl::OUString &rName,
			const SfxItemSet& rSet ) const
						throw(RuntimeException, UnknownPropertyException)
{
	Any aVal;
	getPropertyValue( rName,rSet, aVal );
	return aVal;
}
/* -----------------------------15.11.00 14:46--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
											const Any& aVal,
											SfxItemSet& rSet ) const
											throw(RuntimeException,
													IllegalArgumentException)
{
    // get the SfxPoolItem
    const SfxPoolItem* pItem = 0;
    SfxPoolItem *pNewItem = 0;
    SfxItemState eState = rSet.GetItemState( rEntry.nWID, sal_True, &pItem );
    if(SFX_ITEM_SET != eState && SFX_WHICH_MAX > rEntry.nWID )
        pItem = &rSet.GetPool()->GetDefaultItem(rEntry.nWID);
    //maybe there's another way to find an Item
    if(eState < SFX_ITEM_DEFAULT)
    {
        SfxItemSet aSet(*rSet.GetPool(), rEntry.nWID, rEntry.nWID);
        if(FillItem(aSet, rEntry.nWID, sal_False))
        {
            const SfxPoolItem &rItem = aSet.Get(rEntry.nWID);
            pNewItem = rItem.Clone();
        }
    }
    if(!pNewItem && pItem)
    {
        pNewItem = pItem->Clone();
    }
    if(pNewItem)
    {
        if( !pNewItem->PutValue( aVal, rEntry.nMemberId ) )
        {
            DELETEZ(pNewItem);
            throw IllegalArgumentException();
        }
        // apply new item 
        rSet.Put( *pNewItem, rEntry.nWID );
        delete pNewItem;
    }
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
void SfxItemPropertySet::setPropertyValue( const rtl::OUString &rName,
											const Any& aVal,
											SfxItemSet& rSet ) const
											throw(RuntimeException,
													IllegalArgumentException,
													UnknownPropertyException)
{
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if ( !pEntry )
	{
		throw UnknownPropertyException();
	}
    setPropertyValue(*pEntry, aVal, rSet);
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
PropertyState SfxItemPropertySet::getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw()
{
	PropertyState eRet = PropertyState_DIRECT_VALUE;
    sal_uInt16 nWhich = rEntry.nWID;

	// item state holen
	SfxItemState eState = rSet.GetItemState( nWhich, sal_False );
	// item-Wert als UnoAny zurueckgeben
	if(eState == SFX_ITEM_DEFAULT)
		eRet = PropertyState_DEFAULT_VALUE;
	else if(eState < SFX_ITEM_DEFAULT)
		eRet = PropertyState_AMBIGUOUS_VALUE;
	return eRet;
}
PropertyState   SfxItemPropertySet::getPropertyState(
	const rtl::OUString& rName, const SfxItemSet& rSet) const
                                    throw(UnknownPropertyException)  
{
	PropertyState eRet = PropertyState_DIRECT_VALUE;

	// which-id ermitteln
    const SfxItemPropertySimpleEntry* pEntry = m_aMap.getByName( rName );
    if( !pEntry || !pEntry->nWID )
	{
		throw UnknownPropertyException();
	}
    sal_uInt16 nWhich = pEntry->nWID;

	// item holen
	const SfxPoolItem* pItem = 0;
	SfxItemState eState = rSet.GetItemState( nWhich, sal_False, &pItem );
	if(!pItem && nWhich != rSet.GetPool()->GetSlotId(nWhich))
		pItem = &rSet.GetPool()->GetDefaultItem(nWhich);
	// item-Wert als UnoAny zurueckgeben
	if(eState == SFX_ITEM_DEFAULT)
		eRet = PropertyState_DEFAULT_VALUE;
	else if(eState < SFX_ITEM_DEFAULT)
		eRet = PropertyState_AMBIGUOUS_VALUE;
	return eRet;
}
/* -----------------------------21.02.00 11:26--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XPropertySetInfo>
    SfxItemPropertySet::getPropertySetInfo() const
{
    if( !m_xInfo.is() )
        m_xInfo = new SfxItemPropertySetInfo( &m_aMap );
    return m_xInfo;
}
/*-- 16.02.2009 13:49:25---------------------------------------------------

  -----------------------------------------------------------------------*/
struct SfxItemPropertySetInfo_Impl
{
    SfxItemPropertyMap*         m_pOwnMap;
};
/*-- 16.02.2009 13:49:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMap *pMap ) :
    m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( pMap );
}
/*-- 16.02.2009 13:49:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertySetInfo::SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries ) :
    m_pImpl( new SfxItemPropertySetInfo_Impl )
{
    m_pImpl->m_pOwnMap = new SfxItemPropertyMap( pEntries );
}
/* -----------------------------21.02.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Property > SAL_CALL
        SfxItemPropertySetInfo::getProperties(  )
            throw(RuntimeException)
{
    return m_pImpl->m_pOwnMap->getProperties();
}
/*-- 16.02.2009 13:49:27---------------------------------------------------

  -----------------------------------------------------------------------*/
const SfxItemPropertyMap* SfxItemPropertySetInfo::getMap() const 
{
    return m_pImpl->m_pOwnMap;
}

/*-- 16.02.2009 12:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxItemPropertySetInfo::~SfxItemPropertySetInfo()
{
    delete m_pImpl->m_pOwnMap;
    delete m_pImpl;
}
/* -----------------------------21.02.00 11:27--------------------------------

 ---------------------------------------------------------------------------*/
Property SAL_CALL
		SfxItemPropertySetInfo::getPropertyByName( const ::rtl::OUString& rName )
			throw(UnknownPropertyException, RuntimeException)
{
    return m_pImpl->m_pOwnMap->getPropertyByName( rName );
}
/* -----------------------------21.02.00 11:28--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL
		SfxItemPropertySetInfo::hasPropertyByName( const ::rtl::OUString& rName )
			throw(RuntimeException)
{
    return m_pImpl->m_pOwnMap->hasPropertyByName( rName );
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
SfxExtItemPropertySetInfo::SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMapEntry *pMap,
								const Sequence<Property>& rPropSeq ) :
                aExtMap( pMap )
{
    aExtMap.mergeProperties( rPropSeq );
}    
/*-- 16.02.2009 12:06:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxExtItemPropertySetInfo::~SfxExtItemPropertySetInfo()
{
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Property > SAL_CALL
		SfxExtItemPropertySetInfo::getProperties(  ) throw(RuntimeException)
{
    return aExtMap.getProperties();
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
Property SAL_CALL
SfxExtItemPropertySetInfo::getPropertyByName( const rtl::OUString& rPropertyName )
			throw(UnknownPropertyException, RuntimeException)
{
    return aExtMap.getPropertyByName( rPropertyName );
}
/* -----------------------------21.02.00 12:03--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SAL_CALL
SfxExtItemPropertySetInfo::hasPropertyByName( const rtl::OUString& rPropertyName )
			throw(RuntimeException)
{
    return aExtMap.hasPropertyByName( rPropertyName );
}

