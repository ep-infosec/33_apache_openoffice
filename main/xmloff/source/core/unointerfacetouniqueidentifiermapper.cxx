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
#include "precompiled_xmloff.hxx"
#include "unointerfacetouniqueidentifiermapper.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::rtl::OUString;

namespace comphelper
{

UnoInterfaceToUniqueIdentifierMapper::UnoInterfaceToUniqueIdentifierMapper()
: mnNextId( 1 )
{
}

/** returns a unique identifier for the given uno object. IF a uno object is 
	registered more than once, the returned identifier is always the same.
*/
const OUString& UnoInterfaceToUniqueIdentifierMapper::registerReference( const Reference< XInterface >& rInterface )
{
	IdMap_t::const_iterator aIter;
	if( findReference( rInterface, aIter ) )
	{
		return (*aIter).first;
	}
	else
	{
		OUString aId( RTL_CONSTASCII_USTRINGPARAM( "id" ) );
		aId += OUString::valueOf( mnNextId++ );
		return (*maEntries.insert( IdMap_t::value_type( aId, rInterface ) ).first).first;
	}
}

/** registers the given uno object with the given identifier.

	@returns
		false, if the given identifier already exists and is not associated with the given interface
*/
bool UnoInterfaceToUniqueIdentifierMapper::registerReference( const OUString& rIdentifier, const Reference< XInterface >& rInterface )
{
	IdMap_t::const_iterator aIter;
	if( findReference( rInterface, aIter ) )
	{
		return rIdentifier != (*aIter).first;
	}
	else if( findIdentifier( rIdentifier, aIter ) )
	{
		return false;
	}
	else
	{
		maEntries.insert( IdMap_t::value_type( rIdentifier, rInterface ) );

		// see if this is a reference like something we would generate in the future
		const sal_Unicode *p = rIdentifier.getStr();
		sal_Int32 nLength = rIdentifier.getLength();

		// see if the identifier is 'id' followed by a pure integer value
		if( nLength < 2 || p[0] != 'i' || p[1] != 'd' )
			return true;

		nLength -= 2;
		p += 2;

		while(nLength--)
		{
			if( (*p < '0') || (*p > '9') )
				return true; // a custom id, that will never conflict with genereated id's

			p++;
		}

		// the identifier is a pure integer value
		// so we make sure we will never generate
		// an integer value like this one
		sal_Int32 nId = rIdentifier.copy(2).toInt32();
		if( mnNextId <= nId )
			mnNextId = nId + 1;

		return true;
	}
}

/** @returns
		the identifier for the given uno object. If this uno object is not already
		registered, an empty string is returned
*/
const OUString& UnoInterfaceToUniqueIdentifierMapper::getIdentifier( const Reference< XInterface >& rInterface ) const
{
	IdMap_t::const_iterator aIter;
	if( findReference( rInterface, aIter ) )
	{
		return (*aIter).first;
	}
	else
	{
		static const OUString aEmpty;
		return aEmpty;
	}
}

/** @returns
	the uno object that is registered with the given identifier. If no uno object
	is registered with the given identifier, an empty reference is returned.
*/
const Reference< XInterface >& UnoInterfaceToUniqueIdentifierMapper::getReference( const OUString& rIdentifier ) const
{
	IdMap_t::const_iterator aIter;
	if( findIdentifier( rIdentifier, aIter ) )
	{
		return (*aIter).second;
	}
	else
	{
		static const Reference< XInterface > aEmpty;
		return aEmpty;
	}
}

bool UnoInterfaceToUniqueIdentifierMapper::findReference( const Reference< XInterface >& rInterface, IdMap_t::const_iterator& rIter ) const
{
	rIter = maEntries.begin();
	const IdMap_t::const_iterator aEnd( maEntries.end() );
	while( rIter != aEnd )
	{
		if( (*rIter).second == rInterface )
			return true;

		rIter++;
	}

	return false;
}

bool UnoInterfaceToUniqueIdentifierMapper::findIdentifier( const OUString& rIdentifier, IdMap_t::const_iterator& rIter ) const
{
	rIter = maEntries.find( rIdentifier );
	return rIter != maEntries.end();
}

}

