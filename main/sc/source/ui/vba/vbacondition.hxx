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


#ifndef SC_VBA_CONDITION_HXX
#define SC_VBA_CONDITION_HXX
#include <com/sun/star/sheet/XSheetCondition.hpp> 
#include <com/sun/star/sheet/XCellRangeAddressable.hpp> 
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/sheet/ConditionOperator.hpp>

template< typename Ifc1 >
class ScVbaCondition : public InheritedHelperInterfaceImpl1< Ifc1 >
{
typedef InheritedHelperInterfaceImpl1< Ifc1 > ScVbaCondition_BASE;
protected:
	css::uno::Reference< css::sheet::XCellRangeAddressable > mxAddressable;
	css::uno::Reference< css::sheet::XSheetCondition > mxSheetCondition;
public:
	ScVbaCondition(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::sheet::XSheetCondition >& _xSheetCondition );

	static css::sheet::ConditionOperator retrieveAPIOperator( const css::uno::Any& _aOperator) throw ( css::script::BasicErrorException );

	virtual rtl::OUString SAL_CALL Formula1( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
	virtual rtl::OUString SAL_CALL Formula2( ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
	virtual void setFormula1( const css::uno::Any& _aFormula1) throw ( css::script::BasicErrorException );
	virtual void setFormula2( const css::uno::Any& _aFormula2) throw ( css::script::BasicErrorException );
	virtual sal_Int32 Operator(sal_Bool _bIncludeFormulaValue) throw ( css::script::BasicErrorException );
	virtual sal_Int32 SAL_CALL Operator() throw ( css::script::BasicErrorException, css::uno::RuntimeException ) = 0;
	
};
#endif 
