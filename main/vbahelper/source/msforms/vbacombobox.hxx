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


#ifndef SC_VBA_COMBOBOX_HXX
#define SC_VBA_COMBOBOX_HXX
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <ooo/vba/msforms/XComboBox.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "vbacontrol.hxx"
#include "vbalistcontrolhelper.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper2<ScVbaControl, ov::msforms::XComboBox, css::script::XDefaultProperty > ComboBoxImpl_BASE;
class ScVbaComboBox : public ComboBoxImpl_BASE
{		
	std::auto_ptr< ListControlHelper > mpListHelper;
	rtl::OUString sSourceName; 
	rtl::OUString msDftPropName;
	bool mbDialogType;
	
public:
	ScVbaComboBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper, bool bDialogType = false );

	// Attributes
	virtual css::uno::Any SAL_CALL getListIndex() throw (css::uno::RuntimeException);
	virtual ::sal_Int32 SAL_CALL getListCount() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setListIndex( const css::uno::Any& _value ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStyle() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setStyle( sal_Int32 nStyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDropButtonStyle() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setDropButtonStyle( sal_Int32 nDropButtonStyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getDragBehavior() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setDragBehavior( sal_Int32 nDragBehavior ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEnterFieldBehavior() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setEnterFieldBehavior( sal_Int32 nEnterFieldBehavior ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getListStyle() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setListStyle( sal_Int32 nListStyle ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTextAlign() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setTextAlign( sal_Int32 nTextAlign ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getTextLength() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException);

	// Methods
	virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL removeItem( const css::uno::Any& index ) throw (css::uno::RuntimeException);
	virtual void SAL_CALL Clear(  ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn ) throw (css::uno::RuntimeException);
	// XControl
    virtual void SAL_CALL setRowSource( const rtl::OUString& _rowsource ) throw (css::uno::RuntimeException);

	// XDefaultProperty
        ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }
	//XHelperInterface
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //
