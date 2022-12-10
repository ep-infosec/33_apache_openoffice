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
#include "precompiled_unotools.hxx"
#include <tools/debug.hxx>

#include "unotools/propertysetinfo.hxx"
#include "unotools/propertysethelper.hxx"

///////////////////////////////////////////////////////////////////////

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace utl
{
class PropertySetHelperImpl
{
public:
	PropertyMapEntry* find( const OUString& aName ) const throw();
	
	PropertySetInfo* mpInfo;
};
}

PropertyMapEntry* PropertySetHelperImpl::find( const OUString& aName ) const throw()
{
	PropertyMap::const_iterator aIter = mpInfo->getPropertyMap()->find( aName );

	if( mpInfo->getPropertyMap()->end() != aIter )
	{
		return (*aIter).second;
	}
	else
	{
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////

PropertySetHelper::PropertySetHelper( utl::PropertySetInfo* pInfo ) throw()
{
	mp = new PropertySetHelperImpl;
	mp->mpInfo = pInfo;
	pInfo->acquire();
}

PropertySetHelper::~PropertySetHelper() throw()
{
	mp->mpInfo->release();
	delete mp;
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo(  ) throw(RuntimeException)
{
	return mp->mpInfo;
}

void SAL_CALL PropertySetHelper::setPropertyValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
	PropertyMapEntry* aEntries[2];
	aEntries[0] = mp->find( aPropertyName );

	if( NULL == aEntries[0] )
		throw UnknownPropertyException();

	aEntries[1] = NULL;

	_setPropertyValues( (const PropertyMapEntry**)aEntries, &aValue );
}

Any SAL_CALL PropertySetHelper::getPropertyValue( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	PropertyMapEntry* aEntries[2];
	aEntries[0] = mp->find( PropertyName );

	if( NULL == aEntries[0] )
		throw UnknownPropertyException();

	aEntries[1] = NULL;

	Any aAny;
	_getPropertyValues( (const PropertyMapEntry**)aEntries, &aAny );

	return aAny;
}

void SAL_CALL PropertySetHelper::addPropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	// todo
}

void SAL_CALL PropertySetHelper::removePropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	// todo
}

void SAL_CALL PropertySetHelper::addVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	// todo
}

void SAL_CALL PropertySetHelper::removeVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	// todo
}

// XMultiPropertySet
void SAL_CALL PropertySetHelper::setPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
	const sal_Int32 nCount = aPropertyNames.getLength();

	if( nCount != aValues.getLength() )
		throw IllegalArgumentException();

	if( nCount )
	{
		PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
		const OUString* pNames = aPropertyNames.getConstArray();

		sal_Bool bUnknown = sal_False;
		sal_Int32 n;
		for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
		{
			pEntries[n] = mp->find( *pNames );
			bUnknown = NULL == pEntries[n];
		}

		if( !bUnknown )
			_setPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getConstArray() );

		delete [] pEntries;

		if( bUnknown )
			throw UnknownPropertyException();
	}
}

Sequence< Any > SAL_CALL PropertySetHelper::getPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames ) throw(RuntimeException)
{
	const sal_Int32 nCount = aPropertyNames.getLength();

	Sequence< Any > aValues;
	if( nCount )
	{
		PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
		const OUString* pNames = aPropertyNames.getConstArray();

		sal_Bool bUnknown = sal_False;
		sal_Int32 n;
		for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
		{
			pEntries[n] = mp->find( *pNames );
			bUnknown = NULL == pEntries[n];
		}

		if( !bUnknown )
			_getPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getArray() );

		delete [] pEntries;

		if( bUnknown )
			throw UnknownPropertyException();
	}

	return aValues;
}

void SAL_CALL PropertySetHelper::addPropertiesChangeListener( const Sequence< ::rtl::OUString >& /*aPropertyNames*/, const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw(RuntimeException)
{
	// todo
}

void SAL_CALL PropertySetHelper::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw(RuntimeException)
{
	// todo
}

void SAL_CALL PropertySetHelper::firePropertiesChangeEvent( const Sequence< ::rtl::OUString >& /*aPropertyNames*/, const Reference< XPropertiesChangeListener >& /*xListener*/ ) throw(RuntimeException)
{
	// todo
}

// XPropertyState
PropertyState SAL_CALL PropertySetHelper::getPropertyState( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
	PropertyMapEntry* aEntries[2];

	aEntries[0] = mp->find( PropertyName );
	if( aEntries[0] == NULL )
		throw UnknownPropertyException();

	aEntries[1] = NULL;

	PropertyState aState;
	_getPropertyStates( (const PropertyMapEntry**)aEntries, &aState );

	return aState;
}

Sequence< PropertyState > SAL_CALL PropertySetHelper::getPropertyStates( const Sequence< ::rtl::OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
	const sal_Int32 nCount = aPropertyName.getLength();

	Sequence< PropertyState > aStates( nCount );

	if( nCount )
	{
		const OUString* pNames = aPropertyName.getConstArray();

		sal_Bool bUnknown = sal_False;

		PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];

		sal_Int32 n;
		for( n = 0; !bUnknown && (n < nCount); n++, pNames++ )
		{
			pEntries[n] = mp->find( *pNames );
			bUnknown = NULL == pEntries[n];
		}

		pEntries[nCount] = NULL;

		if( !bUnknown )
			_getPropertyStates( (const PropertyMapEntry**)pEntries, aStates.getArray() );

		delete [] pEntries;

		if( bUnknown )
			throw UnknownPropertyException();
	}

	return aStates;
}

void SAL_CALL PropertySetHelper::setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
	PropertyMapEntry *pEntry  = mp->find( PropertyName );
	if( NULL == pEntry )
		throw UnknownPropertyException();

	_setPropertyToDefault( pEntry );
}

Any SAL_CALL PropertySetHelper::getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
	PropertyMapEntry* pEntry = mp->find( aPropertyName );
	if( NULL == pEntry )
		throw UnknownPropertyException();

	return _getPropertyDefault( pEntry );
}

void PropertySetHelper::_getPropertyStates( const utl::PropertyMapEntry** /*ppEntries*/, PropertyState* /*pStates*/ ) throw(UnknownPropertyException )
{
	DBG_ERROR( "you have to implement this yourself!" );
}

void PropertySetHelper::_setPropertyToDefault( const utl::PropertyMapEntry* /*pEntry*/ )  throw(UnknownPropertyException )
{
	DBG_ERROR( "you have to implement this yourself!" );
}

Any PropertySetHelper::_getPropertyDefault( const utl::PropertyMapEntry* /*pEntry*/ ) throw(UnknownPropertyException, WrappedTargetException )
{
	DBG_ERROR( "you have to implement this yourself!" );

	Any aAny;
	return aAny;
}
