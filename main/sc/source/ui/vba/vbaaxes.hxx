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


#ifndef SC_VBA_AXES_HXX
#define SC_VBA_AXES_HXX
#include <ooo/vba/excel/XAxes.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <ooo/vba/excel/XChart.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XAxes > ScVbaAxes_BASE;
class ScVbaAxes : public ScVbaAxes_BASE
{
	css::uno::Reference< ov::excel::XChart > moChartParent; // not the true parent I guess
public:
	ScVbaAxes( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< ov::excel::XChart >& xChart );
	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);	
	css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& aIndex2 ) throw (css::uno::RuntimeException);
	virtual css::uno::Any createCollectionObject(const css::uno::Any&);
	// XHelperInterface
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();
	static css::uno::Reference< ov::excel::XAxis > createAxis( const css::uno::Reference< ov::excel::XChart >& xChart, const css::uno::Reference< css::uno::XComponentContext >& xContext,  sal_Int32 nType, sal_Int32 nAxisGroup ) throw ( css::uno::RuntimeException );
};

#endif //SC_VBA_AXES_HXX
