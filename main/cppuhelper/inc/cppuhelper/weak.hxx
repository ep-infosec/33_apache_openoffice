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


#ifndef _CPPUHELPER_WEAK_HXX_
#define _CPPUHELPER_WEAK_HXX_

#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/queryinterface.hxx>
#include "cppuhelper/cppuhelperdllapi.h"
#ifndef _COM_SUN_STAR_UNO_WEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif


namespace cppu
{    

/** @internal */
class OWeakConnectionPoint;

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.
    
    @derive
    Inherit from this class and delegate acquire()/ release() calls.
*/
class CPPUHELPER_DLLPUBLIC OWeakObject : public ::com::sun::star::uno::XWeak
{
    /** @internal */
	friend class OWeakConnectionPoint;
    
protected:
	/** Virtual dtor.
        
        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
	*/
    virtual ~OWeakObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );
    
    /** disposes and resets m_pWeakConnectionPoint
        @precond
            m_refCount equals 0
    */
    void    disposeWeakConnectionPoint();
    
	/** reference count.
        
        @attention
        Don't modify manually!  Use acquire() and release().
    */
    oslInterlockedCount m_refCount;
	
	/** Container of all weak reference listeners and the connection point from the weak reference.
        @internal
    */
	OWeakConnectionPoint * m_pWeakConnectionPoint;
    
    /** reserved for future use. do not use.
        @internal
    */
    void * m_pReserved;
    
public:
	// these are here to force memory de/allocation to sal lib.
    /** @internal */
	inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
		{ return ::rtl_allocateMemory( nSize ); }
    /** @internal */
	inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
		{ ::rtl_freeMemory( pMem ); }
    /** @internal */
	inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW( () )
		{ return pMem; }
    /** @internal */
	inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW( () )
		{}
	
#ifdef _MSC_VER
	/** Default Constructor.  Sets the reference count to zero.
        Accidentally occurs in msvc mapfile = > had to be outlined.
    */
	OWeakObject() SAL_THROW( () );
#else
	/** Default Constructor.  Sets the reference count to zero.
    */
	inline OWeakObject() SAL_THROW( () )
		: m_refCount( 0 )
		, m_pWeakConnectionPoint( 0 )
		{}
#endif
	/** Dummy copy constructor.  Set the reference count to zero.
        
        @param rObj dummy param
    */
    inline OWeakObject( const OWeakObject & ) SAL_THROW( () )
		: com::sun::star::uno::XWeak()
        , m_refCount( 0 )
		, m_pWeakConnectionPoint( 0 )
		{}
	/** Dummy assignment operator. Does not affect reference count.
        
        @return this OWeakObject
    */
    inline OWeakObject & SAL_CALL operator = ( const OWeakObject &)
        SAL_THROW( () )
    	{ return *this; }
	
	/** Basic queryInterface() implementation supporting ::com::sun::star::uno::XWeak and
        ::com::sun::star::uno::XInterface.
        
        @param rType demanded type
        @return demanded type or empty any
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
		const ::com::sun::star::uno::Type & rType )
		throw (::com::sun::star::uno::RuntimeException);
    /** increasing m_refCount
    */
    virtual void SAL_CALL acquire()
		throw ();
    /** decreasing m_refCount
    */
    virtual void SAL_CALL release()
		throw ();
	
	/** XWeak::queryAdapter() implementation

        @return a ::com::sun::star::uno::XAdapter reference
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAdapter > SAL_CALL queryAdapter()
		throw (::com::sun::star::uno::RuntimeException);
	
	/** Cast operator to XInterface reference.
        
        @return XInterface reference
    */
	inline SAL_CALL operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () SAL_THROW( () )
		{ return this; }
};

}

#endif


