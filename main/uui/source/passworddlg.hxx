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



#ifndef PASSWORDDLG_HXX
#define PASSWORDDLG_HXX

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>

//============================================================================

class PasswordDialog : public ModalDialog
{
	FixedText		aFTPassword;
	Edit			aEDPassword;
    FixedText       aFTConfirmPassword;
    Edit            aEDConfirmPassword;
	OKButton		aOKBtn;
	CancelButton	aCancelBtn;
	HelpButton		aHelpBtn;
    FixedLine       aFixedLine1;
    sal_uInt16          nMinLen;
    String          aPasswdMismatch;


	DECL_LINK( OKHdl_Impl, OKButton * );

public:
    PasswordDialog( Window* pParent, ::com::sun::star::task::PasswordRequestMode nDlgMode, ResMgr * pResMgr, ::rtl::OUString& aDocURL, 
            bool bOpenToModify = false, bool bIsSimplePasswordRequest = false );

    void            SetMinLen( sal_uInt16 nMin ) { nMinLen = nMin; }
	String			GetPassword() const { return aEDPassword.GetText(); }

private:
	::com::sun::star::task::PasswordRequestMode		nDialogMode;
	ResMgr*											pResourceMgr;
};

#endif // PASSWORDDLG_HXX

