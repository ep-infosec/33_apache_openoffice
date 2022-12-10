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



#ifndef TOOLS_DIAGNOSE_EX_H
#define TOOLS_DIAGNOSE_EX_H

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <boost/current_function.hpp>


#define OSL_UNUSED( expression ) \
    (void)(expression)

#if OSL_DEBUG_LEVEL > 0

    #include <cppuhelper/exc_hlp.hxx>
    #include <osl/diagnose.h>
    #include <osl/thread.h>
    #include <boost/current_function.hpp>
    #include <typeinfo>

    /** reports a caught UNO exception via OSL diagnostics

        Note that whenever you use this, it might be an indicator that your error
        handling is not correct ....
    */
    #define DBG_UNHANDLED_EXCEPTION()   \
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() ); \
	    ::rtl::OString sMessage( "caught an exception!" ); \
        sMessage += "\nin function:"; \
        sMessage += BOOST_CURRENT_FUNCTION; \
	    sMessage += "\ntype: "; \
	    sMessage += ::rtl::OString( caught.getValueTypeName().getStr(), caught.getValueTypeName().getLength(), osl_getThreadTextEncoding() ); \
        ::com::sun::star::uno::Exception exception; \
        caught >>= exception; \
        if ( exception.Message.getLength() ) \
        { \
	        sMessage += "\nmessage: "; \
	        sMessage += ::rtl::OString( exception.Message.getStr(), exception.Message.getLength(), osl_getThreadTextEncoding() ); \
        } \
        if ( exception.Context.is() ) \
        { \
            const char* pContext = typeid( *exception.Context.get() ).name(); \
            sMessage += "\ncontext: "; \
            sMessage += pContext; \
        } \
        sMessage += "\n"; \
	    OSL_ENSURE( false, sMessage )

#else   // OSL_DEBUG_LEVEL

    #define DBG_UNHANDLED_EXCEPTION()

#endif  // OSL_DEBUG_LEVEL


/** This macro asserts the given condition (in debug mode), and throws
    an IllegalArgumentException afterwards.
 */
#define ENSURE_ARG_OR_THROW(c, m) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     throw ::com::sun::star::lang::IllegalArgumentException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii( ",\n" m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(), \
                                     0 ); }
#define ENSURE_ARG_OR_THROW2(c, m, ifc, arg) if( !(c) ) { \
                                               OSL_ENSURE(c, m); \
                                               throw ::com::sun::star::lang::IllegalArgumentException( \
                                               ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                               ::rtl::OUString::createFromAscii( ",\n" m), \
                                               ifc, \
                                               arg ); }

/** This macro asserts the given condition (in debug mode), and throws
    an RuntimeException afterwards.
 */
#define ENSURE_OR_THROW(c, m) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     throw ::com::sun::star::uno::RuntimeException( \
                                     ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                     ::rtl::OUString::createFromAscii( ",\n" m), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() ); }
#define ENSURE_OR_THROW2(c, m, ifc) if( !(c) ) { \
                                          OSL_ENSURE(c, m); \
                                          throw ::com::sun::star::uno::RuntimeException( \
                                          ::rtl::OUString::createFromAscii(BOOST_CURRENT_FUNCTION) + \
                                          ::rtl::OUString::createFromAscii( ",\n" m), \
                                          ifc ); }

/** This macro asserts the given condition (in debug mode), and
    returns the given value afterwards.
 */
#define ENSURE_OR_RETURN(c, m, r) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     return r; }

/** This macro asserts the given condition (in debug mode), and
    returns false afterwards.
 */
#define ENSURE_OR_RETURN_FALSE(c, m) \
    ENSURE_OR_RETURN(c, m, false)

/** This macro asserts the given condition (in debug mode), and
    returns afterwards, without return value "void".
 */
#define ENSURE_OR_RETURN_VOID( c, m ) \
    if( !(c) )  \
    { \
        OSL_ENSURE( c, m ); \
        return;   \
    }



/** This macro asserts the given condition (in debug mode), and
    returns afterwards, without return value "void".
 */
#define ENSURE_OR_RETURN_VOID( c, m ) \
    if( !(c) )  \
    { \
        OSL_ENSURE( c, m ); \
        return;   \
    }

/** asserts a given condition (in debug mode), and continues the most-inner
    loop if the condition is not met
*/
#define ENSURE_OR_CONTINUE( c, m ) \
    if ( !(c) ) \
    {   \
        OSL_ENSURE( false, m ); \
        continue;   \
    }

/** asserts a given condition (in debug mode), and continues the most-inner
    loop if the condition is not met
*/
#define ENSURE_OR_BREAK( c, m ) \
    if ( !(c) ) \
    {   \
        OSL_ENSURE( false, m ); \
        break;  \
    }

#endif // TOOLS_DIAGNOSE_EX_H
