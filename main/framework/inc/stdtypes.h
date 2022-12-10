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



#ifndef __FRAMEWORK_STDTYPES_H_
#define __FRAMEWORK_STDTYPES_H_

#include <vector>
#include <queue>
#include <hash_map>

//_________________________________________________________________________________________________________________
// own includes
//_________________________________________________________________________________________________________________
#include <general.h>

//_________________________________________________________________________________________________________________
// interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

//_________________________________________________________________________________________________________________
// other includes
//_________________________________________________________________________________________________________________
#include <comphelper/sequenceasvector.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//	namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________


struct ShortHashCode
{
	size_t operator()( const ::sal_Int16& nShort ) const
	{
		return (size_t)nShort;
	}
};

struct Int32HashCode
{
	size_t operator()( const ::sal_Int32& nValue ) const
	{
		return (size_t)nValue;
	}
};

struct KeyEventHashCode
{
	size_t operator()( const css::awt::KeyEvent& aEvent ) const
	{
		return (size_t)(aEvent.KeyCode  +
                        //aEvent.KeyChar  +
                        //aEvent.KeyFunc  +
                        aEvent.Modifiers);
	}
};

struct KeyEventEqualsFunc
{
    bool operator()(const css::awt::KeyEvent aKey1,
                    const css::awt::KeyEvent aKey2) const
    {
		return (
                (aKey1.KeyCode   == aKey2.KeyCode  ) &&
                //(aKey1.KeyChar   == aKey2.KeyChar  ) &&
                //(aKey1.KeyFunc   == aKey2.KeyFunc  ) &&
                (aKey1.Modifiers == aKey2.Modifiers)
               );
    }
};

//_________________________________________________________________________________________________________________

/**
    Basic string list based on a std::vector()
    It implements some additional funtionality which can be useful but
    is missing at the normal vector implementation.
*/
class OUStringList : public ::comphelper::SequenceAsVector< ::rtl::OUString >
{
	public:

        // insert given element as the first one into the vector
        void push_front( const ::rtl::OUString& sElement )
		{
			insert( begin(), sElement );
		}

        // search for given element
        iterator find( const ::rtl::OUString& sElement )
        {
            return ::std::find(begin(), end(), sElement);
        }
        
        const_iterator findConst( const ::rtl::OUString& sElement ) const
        {
            return ::std::find(begin(), end(), sElement);
        }

        // the only way to free used memory really!
        void free()
		{
            OUStringList().swap( *this );
		}
};

//_________________________________________________________________________________________________________________

/**
    Basic string queue based on a std::queue()
    It implements some additional funtionality which can be useful but
    is missing at the normal std implementation.
*/
typedef ::std::queue< ::rtl::OUString > OUStringQueue;

//_________________________________________________________________________________________________________________

/**
    Basic hash based on a std::hash_map() which provides key=[OUString] and value=[template type] pairs
    It implements some additional funtionality which can be useful but
    is missing at the normal hash implementation.
*/
template< class TType >
class BaseHash : public ::std::hash_map< ::rtl::OUString                    ,
                                         TType                              ,
                                         rtl::OUStringHash                  ,
                                         ::std::equal_to< ::rtl::OUString > >
{
	public:

        // the only way to free used memory really!
		void free()
		{
            BaseHash().swap( *this );
		}
};

//_________________________________________________________________________________________________________________

/**
    Basic OUString hash.
    Key and values are OUStrings.
*/
typedef BaseHash< ::rtl::OUString > OUStringHashMap;

//_________________________________________________________________________________________________________________

/**
    It can be used to map names (e.g. of properties) to her corresponding handles.
    Our helper class OPropertySetHelper works optimized with handles but sometimes we have only a property name.
    Mapping between these two parts of a property should be done in the fastest way :-)
*/
typedef BaseHash< sal_Int32 > NameToHandleHash;

//_________________________________________________________________________________________________________________

/**
    Sometimes we need this template to implement listener container ...
    and we need it at different positions ...
    So it's better to declare it one times only!
*/
typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString                    ,
                                                        rtl::OUStringHash,
                                                        ::std::equal_to< ::rtl::OUString > >    ListenerHash;

}       // namespace framework

#endif  // #ifndef __FRAMEWORK_STDTYPES_H_
