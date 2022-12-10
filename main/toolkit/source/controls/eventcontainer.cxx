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
#include "precompiled_toolkit.hxx"


#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#ifndef _CPPUHELER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#include <cppuhelper/factory.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include "toolkit/controls/eventcontainer.hxx"
#include <com/sun/star/script/ScriptEventDescriptor.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace std;


namespace toolkit
{
	
// Methods XElementAccess
Type NameContainer_Impl::getElementType()
	throw(RuntimeException)
{
	return mType;
}

sal_Bool NameContainer_Impl::hasElements() 
	throw(RuntimeException)
{
	sal_Bool bRet = (mnElementCount > 0);
	return bRet;
}

// Methods XNameAccess
Any NameContainer_Impl::getByName( const OUString& aName ) 
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}
	sal_Int32 iHashResult = (*aIt).second;
	Any aRetAny = mValues.getConstArray()[ iHashResult ];
	return aRetAny;
}

Sequence< OUString > NameContainer_Impl::getElementNames()
	throw(RuntimeException)
{
	return mNames;
}

sal_Bool NameContainer_Impl::hasByName( const OUString& aName ) 
	throw(RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	sal_Bool bRet = ( aIt != mHashMap.end() );
	return bRet;
}


// Methods XNameReplace
void NameContainer_Impl::replaceByName( const OUString& aName, const Any& aElement ) 
	throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
	Type aAnyType = aElement.getValueType();
	if( mType != aAnyType )
		throw IllegalArgumentException();

	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}
	sal_Int32 iHashResult = (*aIt).second;
	Any aOldElement = mValues.getConstArray()[ iHashResult ];
	mValues.getArray()[ iHashResult ] = aElement;

	// Fire event
	ContainerEvent aEvent;		
	aEvent.Source = *this;
	aEvent.Element <<= aElement;
	aEvent.ReplacedElement = aOldElement;
	aEvent.Accessor <<= aName;
	maContainerListeners.elementReplaced( aEvent );
}


// Methods XNameContainer
void NameContainer_Impl::insertByName( const OUString& aName, const Any& aElement ) 
	throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
	Type aAnyType = aElement.getValueType();
	if( mType != aAnyType )
		throw IllegalArgumentException();

	NameContainerNameMap::iterator aIt = mHashMap.find( aName );
	if( aIt != mHashMap.end() )
	{
		throw ElementExistException();
	}

	sal_Int32 nCount = mNames.getLength();
	mNames.realloc( nCount + 1 );
	mValues.realloc( nCount + 1 );
	mNames.getArray()[ nCount ] = aName;
	mValues.getArray()[ nCount ] = aElement;
	mHashMap[ aName ] = nCount;

	// Fire event
	ContainerEvent aEvent;		
	aEvent.Source = *this;
	aEvent.Element <<= aElement;
	aEvent.Accessor <<= aName;
	maContainerListeners.elementInserted( aEvent );
}

void NameContainer_Impl::removeByName( const OUString& Name ) 
	throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
	NameContainerNameMap::iterator aIt = mHashMap.find( Name );
	if( aIt == mHashMap.end() )
	{
		throw NoSuchElementException();
	}

	sal_Int32 iHashResult = (*aIt).second;
	Any aOldElement = mValues.getConstArray()[ iHashResult ];

	// Fire event
	ContainerEvent aEvent;		
	aEvent.Source = *this;
	aEvent.Element = aOldElement;
	aEvent.Accessor <<= Name;
	maContainerListeners.elementRemoved( aEvent );

	mHashMap.erase( aIt );
	sal_Int32 iLast = mNames.getLength() - 1;
	if( iLast != iHashResult )
	{
		OUString* pNames = mNames.getArray();
		Any* pValues = mValues.getArray();
		pNames[ iHashResult ] = pNames[ iLast ];
		pValues[ iHashResult ] = pValues[ iLast ];
		mHashMap[ pNames[ iHashResult ] ] = iHashResult;
	}
	mNames.realloc( iLast );
	mValues.realloc( iLast );

}

// Methods XContainer
void NameContainer_Impl::addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
	maContainerListeners.addInterface( l );
}

void NameContainer_Impl::removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
	maContainerListeners.removeInterface( l );
}



// Ctor
ScriptEventContainer::ScriptEventContainer( void )
	: NameContainer_Impl( getCppuType( (ScriptEventDescriptor*) NULL ) )
{
}

}




