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


#ifndef SC_VBA_BORDERS_HXX
#define SC_VBA_BORDERS_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XBorders.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

 
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XBorders > ScVbaBorders_BASE;
class ScVbaPalette;
class ScVbaBorders : public ScVbaBorders_BASE
{
	// XEnumerationAccess
	virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) throw (css::uno::RuntimeException);
	bool bRangeIsSingleCell;
	css::uno::Reference< css::beans::XPropertySet > m_xProps;
public:
	ScVbaBorders( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::table::XCellRange >& xRange, ScVbaPalette& rPalette );
	virtual ~ScVbaBorders() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);


	// XBorders

	// ScVbaCollectionBaseImpl	
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 
	
	virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getLineStyle() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setLineStyle( const css::uno::Any& _linestyle ) throw (css::uno::RuntimeException);
	virtual css::uno::Any SAL_CALL getWeight() throw (css::uno::RuntimeException);
	virtual void SAL_CALL setWeight( const  css::uno::Any& ) throw (css::uno::RuntimeException);
	// xxxxBASE 
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_BORDERS_HXX

