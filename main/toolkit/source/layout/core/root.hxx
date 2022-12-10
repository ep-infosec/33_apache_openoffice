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



#ifndef LAYOUT_CORE_ROOT_HXX
#define LAYOUT_CORE_ROOT_HXX

#define _BACKWARD_BACKWARD_WARNING_H 1
#include <hash_map>

#include <com/sun/star/awt/XLayoutRoot.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <toolkit/dllapi.h>

#include <layout/core/proplist.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;

css::uno::Reference< css::io::XInputStream > getFileAsStream( const rtl::OUString &rName );

/* Interface generation code -- to hook to a parser. */

/*
  TODO: (ricardo) I think we should cut on LayoutRoot, stripping out its widget
  proxy interface (just make it return the root widget). Would even make it easier
  if there was interest to support multiple toplevel widgets in the same file.

  We also need to make sure the code gets diposed well... There is no need to keep
  these objects around after initialization...
*/


class LayoutWidget;

class TOOLKIT_DLLPUBLIC LayoutRoot : public ::cppu::WeakImplHelper3<
    css::awt::XLayoutRoot,
    css::lang::XInitialization,
    css::lang::XComponent>
{
protected:
    ::osl::Mutex maMutex;

    typedef std::hash_map< rtl::OUString,
                           css::uno::Reference< css::awt::XLayoutConstrains >,
                           ::rtl::OUStringHash > ItemHash;
    ItemHash maItems;

    sal_Bool mbDisposed;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxFactory;
    ::cppu::OInterfaceContainerHelper *mpListeners;

    css::uno::Reference< css::awt::XWindow >          mxWindow;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;

    css::uno::Reference< css::awt::XToolkit > mxToolkit;
    LayoutWidget *mpToplevel;
    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;

    void error( rtl::OUString const& message );

public:
    LayoutRoot( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
    virtual ~LayoutRoot();

    void addItem( const rtl::OUString &rName,
                  const css::uno::Reference< css::awt::XLayoutConstrains > &xRef );

    void setWindow(    css::uno::Reference< css::awt::XLayoutConstrains > xPeer )
    {
        mxWindow = css::uno::Reference< css::awt::XWindow >( xPeer, css::uno::UNO_QUERY );
    }

    // get XLayoutContainer
    virtual css::uno::Reference< css::awt::XLayoutContainer > SAL_CALL getLayoutContainer() throw (css::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException);

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException);
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException);

    // generator
    virtual LayoutWidget *create( rtl::OUString id, const rtl::OUString unoName, long attrbs, css::uno::Reference< css::awt::XLayoutContainer > xParent );
};

class TOOLKIT_DLLPUBLIC LayoutWidget
{
    friend class LayoutRoot;

public:
    LayoutWidget() {}
    LayoutWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                  css::uno::Reference< css::awt::XLayoutContainer > xToplevel,
                  rtl::OUString unoName, long attrbs );
    virtual ~LayoutWidget();

    virtual void setProperties( const PropList &rProps );
    virtual void setProperty( rtl::OUString const& attr, rtl::OUString const& value );

    virtual bool addChild( LayoutWidget *pChild );
    virtual void setChildProperties( LayoutWidget *pChild, const PropList &rProps );

    inline css::uno::Reference< css::awt::XLayoutConstrains > getPeer()
    { return mxWidget; }
    inline css::uno::Reference< css::awt::XLayoutContainer > getContainer()
    { return mxContainer; }

protected:
    css::uno::Reference< css::awt::XLayoutConstrains > mxWidget;
    css::uno::Reference< css::awt::XLayoutContainer > mxContainer;
};

} // namespace layoutimpl

#endif /* LAYOUT_CORE_ROOT_HXX */
