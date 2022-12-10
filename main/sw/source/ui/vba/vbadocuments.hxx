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


#ifndef SW_VBA_DOCUMENTS_HXX
#define SW_VBA_DOCUMENTS_HXX


#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XDocuments.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <vbahelper/vbadocumentsbase.hxx>
#include "wordvbahelper.hxx"


typedef cppu::ImplInheritanceHelper1< VbaDocumentsBase, ov::word::XDocuments > SwVbaDocuments_BASE;

class SwVbaDocuments : public SwVbaDocuments_BASE
{
public:
	SwVbaDocuments( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );
	virtual ~SwVbaDocuments() {}

	// XEnumerationAccess
	virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
	virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

	// SwVbaDocuments_BASE
	virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ); 
	virtual rtl::OUString& getServiceImplName();
	virtual css::uno::Sequence<rtl::OUString> getServiceNames();

	// Methods
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Template, const css::uno::Any& NewTemplate, const css::uno::Any& DocumentType, const css::uno::Any& Visible ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Open( const ::rtl::OUString& Filename, const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly, const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Format, const css::uno::Any& Encoding, const css::uno::Any& Visible, const css::uno::Any& OpenAndRepair, const css::uno::Any& DocumentDirection, const css::uno::Any& NoEncodingDialog, const css::uno::Any& XMLTransform ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& OriginalFormat, const css::uno::Any& RouteDocument ) throw (css::uno::RuntimeException);
};

#endif /* SW_VBA_DOCUMENTS_HXX */
