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



#ifndef _SFX_PLUGIN_HXX
#define _SFX_PLUGIN_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/plugin/XPlugin.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <cppuhelper/implbase5.hxx>

#include <rtl/ustring.hxx>
#include <svl/ownlist.hxx>
#include <svl/itemprop.hxx>

#include <sfx2/sfxuno.hxx>

namespace sfx2
{

class PluginObject : public ::cppu::WeakImplHelper5 <
        com::sun::star::util::XCloseable,
        com::sun::star::lang::XEventListener,
        com::sun::star::frame::XSynchronousFrameLoader,
        com::sun::star::lang::XInitialization,
        com::sun::star::beans::XPropertySet >
{
    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > mxFact;
    com::sun::star::uno::Reference< com::sun::star::plugin::XPlugin > mxPlugin;
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > mxObj;
    SfxItemPropertyMap  maPropMap;
    SvCommandList       maCmdList;
    ::rtl::OUString     maURL;
    ::rtl::OUString     maMimeType;

                        PluginObject( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rFact );
                        ~PluginObject();

    virtual sal_Bool SAL_CALL load( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& lDescriptor,
            const com::sun::star::uno::Reference < com::sun::star::frame::XFrame >& xFrame ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL cancel() throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException) ;
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
	virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
public:
    SFX_DECL_XSERVICEINFO
};

}
#endif
