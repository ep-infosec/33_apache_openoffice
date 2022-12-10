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


#ifndef SW_VBA_TEMPLATE_HXX
#define SW_VBA_TEMPLATE_HXX

#include <ooo/vba/word/XTemplate.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XTemplate > SwVbaTemplate_BASE;

class SwVbaTemplate : public SwVbaTemplate_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    rtl::OUString msName;
public:
	SwVbaTemplate( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, 
        const css::uno::Reference< css::frame::XModel >& rModel, const rtl::OUString& );
	virtual ~SwVbaTemplate();

   // XTemplate
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL AutoTextEntries( const css::uno::Any& index ) throw (css::uno::RuntimeException);
	// XHelperInterface
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_TEMPLATE_HXX */
