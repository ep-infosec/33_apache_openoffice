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



#include "HistoryOptTest.hxx"
// #include "AccessibilityOptTest.hxx"
// #include "PrintOptTest.hxx"
#include "UserOptTest.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

//=============================================================================
namespace css = ::com::sun::star;

namespace svl{

//=============================================================================
static const ::rtl::OUString PROP_TEST                  = ::rtl::OUString::createFromAscii("Test");
static const ::rtl::OUString TEST_PICKLIST              = ::rtl::OUString::createFromAscii("checkPicklist");
static const ::rtl::OUString TEST_URLHISTORY            = ::rtl::OUString::createFromAscii("checkURLHistory");
static const ::rtl::OUString TEST_HELPBOOKMARKS         = ::rtl::OUString::createFromAscii("checkHelpBookmarks");
// static const ::rtl::OUString TEST_ACCESSIBILITYOPTIONS  = ::rtl::OUString::createFromAscii("checkAccessibilityOptions");
// static const ::rtl::OUString TEST_PRINTOPTIONS          = ::rtl::OUString::createFromAscii("checkPrintOptions");
static const ::rtl::OUString TEST_USEROPTIONS           = ::rtl::OUString::createFromAscii("checkUserOptions");
    
//=============================================================================
class ConfigItemTest : public ::cppu::WeakImplHelper2< css::task::XJob         ,
                                                        css::lang::XServiceInfo >
{
    //-------------------------------------------------------------------------
    // interface
    public:
        explicit ConfigItemTest(const css::uno::Reference< css::uno::XComponentContext >& xContext);
    
        // css::task::XJob
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
            throw (css::uno::RuntimeException         ,
                   css::lang::IllegalArgumentException,
                   css::uno::Exception                );

        // css::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException);
    
        virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
            throw (css::uno::RuntimeException);
    
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException);
    
    //-------------------------------------------------------------------------
    // internal
    private:
        ConfigItemTest(ConfigItemTest &); // not defined
        virtual ~ConfigItemTest() {}
        void operator=(ConfigItemTest &); // not defined
    
    //-------------------------------------------------------------------------
    // helper for registration !
    public:
        static ::rtl::OUString SAL_CALL st_getImplementationName();
        static css::uno::Sequence< ::rtl::OUString > SAL_CALL st_getSupportedServiceNames();
        static css::uno::Reference< css::uno::XInterface > SAL_CALL st_create(const css::uno::Reference< css::uno::XComponentContext >& XContext);
        
    //-------------------------------------------------------------------------
    // member
    private:
        css::uno::Reference< css::uno::XComponentContext >  m_xContext;
};

//=============================================================================
ConfigItemTest::ConfigItemTest(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext(xContext)
{}

//=============================================================================
// css::task::XJob
css::uno::Any SAL_CALL ConfigItemTest::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw (css::uno::RuntimeException         ,
           css::lang::IllegalArgumentException,
           css::uno::Exception                )
{
    ::rtl::OUString sTest;
    ::sal_Int32     i    = 0;
    ::sal_Int32     c    = lArguments.getLength();
    for (i=0; i<c; ++i)
    {
        const css::beans::NamedValue& rArg = lArguments[0];
        if (rArg.Name.equals(PROP_TEST))
            rArg.Value >>= sTest;
    }

	if (sTest.equals(TEST_PICKLIST))
	{
		HistoryOptTest aOptTest;
		aOptTest.checkPicklist();
	}
	else if (sTest.equals(TEST_URLHISTORY))
	{
		HistoryOptTest aOptTest;
		aOptTest.checkURLHistory();
	}
	else if (sTest.equals(TEST_HELPBOOKMARKS))
	{
		HistoryOptTest aOptTest;
		aOptTest.checkHelpBookmarks();
	}
// 	else if (sTest.equals(TEST_ACCESSIBILITYOPTIONS))
// 	{
// 		AccessibilityOptTest aOptTest;
// 		aOptTest.impl_checkAccessibilityOptions();
// 	}
// 	else if (sTest.equals(TEST_PRINTOPTIONS))
// 	{
//         PrintOptTest aOptTest;
//         aOptTest.impl_checkPrint();
// 	}
	else if (sTest.equals(TEST_USEROPTIONS))
	{
		UserOptTest aOptTest;
		aOptTest.impl_checkUserData();
	}

    return css::uno::Any();
}

//=============================================================================
// com::sun::star::uno::XServiceInfo
::rtl::OUString SAL_CALL ConfigItemTest::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ConfigItemTest::st_getImplementationName();
}

//=============================================================================
// com::sun::star::uno::XServiceInfo
::sal_Bool SAL_CALL ConfigItemTest::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames = ConfigItemTest::st_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < lServiceNames.getLength(); ++i)
    {
        if (lServiceNames[i].equals(sServiceName))
            return sal_True;
    }
    return sal_False;
}

//=============================================================================
// com::sun::star::uno::XServiceInfo
css::uno::Sequence< ::rtl::OUString > SAL_CALL ConfigItemTest::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return ConfigItemTest::st_getSupportedServiceNames();
}

//=============================================================================
::rtl::OUString SAL_CALL ConfigItemTest::st_getImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.svl.ConfigItemTest");
}

//=============================================================================
css::uno::Sequence< ::rtl::OUString > SAL_CALL ConfigItemTest::st_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServices(1);
    lServices[0] = ::rtl::OUString::createFromAscii("com.sun.star.test.ConfigItems");
    return lServices;
}

//=============================================================================
css::uno::Reference< css::uno::XInterface > SAL_CALL ConfigItemTest::st_create(const css::uno::Reference< css::uno::XComponentContext >& xContext)
{
    ConfigItemTest*                            pObject = new ConfigItemTest(xContext);
    css::uno::Reference< css::uno::XInterface > xObject (static_cast< ::cppu::OWeakObject* >(pObject));
    return xObject;
}

} // namespace svl

//=============================================================================
static ::cppu::ImplementationEntry const lRegEntries[] =
{
    {
        &::svl::ConfigItemTest::st_create,
        &::svl::ConfigItemTest::st_getImplementationName,
        &::svl::ConfigItemTest::st_getSupportedServiceNames,
        &::cppu::createSingleComponentFactory, 0, 0
    },

    { 0, 0, 0, 0, 0, 0 }
};

//=============================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(const char**            pEnvTypeName,
                                                                      uno_Environment**             )
{
    *pEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=============================================================================
extern "C" void * SAL_CALL component_getFactory(const char* sImplName      ,
                                                      void* pServiceManager,
                                                      void* pRegistryKey   )
{
    return ::cppu::component_getFactoryHelper(sImplName, pServiceManager, pRegistryKey, lRegEntries);
}

//=============================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo(void* pServiceManager,
                                                 void* pRegistryKey   )
{
    return ::cppu::component_writeInfoHelper(pServiceManager, pRegistryKey, lRegEntries);
}
