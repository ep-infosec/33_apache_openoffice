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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/abstdlg.hxx>

#include <osl/module.hxx>
#include <tools/string.hxx>

typedef VclAbstractDialogFactory* (__LOADONCALLAPI *FuncPtrCreateDialogFactory)();

extern "C" { static void SAL_CALL thisModule() {} }

VclAbstractDialogFactory* VclAbstractDialogFactory::Create()
{
    FuncPtrCreateDialogFactory fp = 0;
	static ::osl::Module aDialogLibrary;
	if ( aDialogLibrary.is() || aDialogLibrary.loadRelative( &thisModule, String( RTL_CONSTASCII_USTRINGPARAM( CUI_DLL_NAME ) ) ) )
        fp = ( VclAbstractDialogFactory* (__LOADONCALLAPI*)() )
			aDialogLibrary.getFunctionSymbol( ::rtl::OUString::createFromAscii("CreateDialogFactory") );
	if ( fp )
		return fp();
	return 0;
}

VclAbstractDialog::~VclAbstractDialog()
{
}

// virtual
VclAbstractDialog2::~VclAbstractDialog2()
{
}

VclAbstractDialogFactory::~VclAbstractDialogFactory()
{
}
