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


#ifndef SC_VBA_PIVOTTABLES_HXX
#define SC_VBA_PIVOTTABLES_HXX

#include <cppuhelper/implbase1.hxx>

#include <ooo/vba/excel/XPivotTables.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
 
#include "excelvbahelper.hxx"
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>


typedef CollTestImplHelper< ov::excel::XPivotTables > ScVbaPivotTables_BASE;

class ScVbaPivotTables : public ScVbaPivotTables_BASE
{
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();

public:
	ScVbaPivotTables( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );
	virtual ~ScVbaPivotTables() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);


	// XPivotTables

	// ScVbaPivotTables_BASE
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 
	
	virtual rtl::OUString& getServiceImplName();

};

#endif //SC_VBA_PIVOTTABLES
