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



#ifndef _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_
#define _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_


#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/broadcasthelper.hxx> 
namespace func_provider
{
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================
//typedef ::cppu::WeakImplHelper1< css::beans::XPropertySet > ScriptingContextImpl_BASE;

class ScriptingContext : public ::comphelper::OMutexAndBroadcastHelper, public ::comphelper::OPropertyContainer, 
                         public ::comphelper::OPropertyArrayUsageHelper< ScriptingContext >,  public css::lang::XTypeProvider, public ::cppu::OWeakObject
{

public:
    ScriptingContext( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptingContext();
    // XInterface

    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) 
        throw( css::uno::RuntimeException )
    { 
        css::uno::Any aRet( OPropertySetHelper::queryInterface( rType ) );
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
    }
    void SAL_CALL acquire() throw() { ::cppu::OWeakObject::acquire(); }
    void SAL_CALL release() throw() { ::cppu::OWeakObject::release(); } 
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw ( css::uno::RuntimeException );
    //XTypeProvider
    DECLARE_XTYPEPROVIDER( )

protected:

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  );

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const;
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;


};
} // func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_
