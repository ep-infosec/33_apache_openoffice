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



#using <mscorlib.dll>
#using "cli_ure.dll"
#using "cli_uretypes.dll"

#include "rtl/ustring.hxx"
#include "uno/mapping.hxx"

#include <vcclr.h>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


namespace uno
{
namespace util
{

//------------------------------------------------------------------------------
inline ::System::String * ustring_to_String( ::rtl::OUString const & ustr )
{
    return new ::System::String( ustr.getStr(), 0, ustr.getLength() );
}
//------------------------------------------------------------------------------
inline ::rtl::OUString String_to_ustring( ::System::String * str )
{
    OSL_ASSERT( sizeof (wchar_t) == sizeof (sal_Unicode) );
    wchar_t const __pin * chars = PtrToStringChars( str );
    return ::rtl::OUString( chars, str->get_Length() );
}

template< typename T >
inline ::System::Object * to_cli(
    ::com::sun::star::uno::Reference< T > const & x )
{
    ::com::sun::star::uno::Mapping mapping(
        OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME), OUSTR(UNO_LB_CLI) );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw ::com::sun::star::uno::RuntimeException(
            OUSTR("cannot get mapping from C++ to CLI!"),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
    }
    
    intptr_t intptr =
        reinterpret_cast< intptr_t >(
            mapping.mapInterface( x.get(), ::getCppuType( &x ) ) );
    ::System::Runtime::InteropServices::GCHandle handle(
        ::System::Runtime::InteropServices::GCHandle::op_Explicit( intptr ) );
    ::System::Object * ret = handle.get_Target();
    handle.Free();
    return ret;
}

template< typename T >
inline void to_uno(
    ::com::sun::star::uno::Reference< T > * pRet, ::System::Object * x )
{
    ::com::sun::star::uno::Mapping mapping(
        OUSTR(UNO_LB_CLI), OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
    {
        throw ::com::sun::star::uno::RuntimeException(
            OUSTR("cannot get mapping from CLI to C++!"),
            ::com::sun::star::uno::Reference<
              ::com::sun::star::uno::XInterface >() );
    }
    
    ::System::Runtime::InteropServices::GCHandle handle(
        ::System::Runtime::InteropServices::GCHandle::Alloc( x ) );
    T * ret = 0;
    mapping.mapInterface(
        reinterpret_cast< void ** >( &ret ),
        reinterpret_cast< void * >(
            ::System::Runtime::InteropServices::GCHandle::op_Explicit( handle )
#if defined _WIN32
            .ToInt32()
#elif defined _WIN64
            .ToInt64()
#else
#error ERROR: either _WIN64 or _WIN32 must be defined
            ERROR: either _WIN64 or _WIN32 must be defined
#endif
            ),
        ::getCppuType( pRet ) );
    handle.Free();
    pRet->set( ret, SAL_NO_ACQUIRE /* takeover ownership */ );
}

}
}
