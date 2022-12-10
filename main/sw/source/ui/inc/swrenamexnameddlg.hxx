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


#ifndef _SWRENAMEXNAMEDDLG_HXX
#define _SWRENAMEXNAMEDDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <actctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <tools/resary.hxx>
#include <swunodef.hxx>



/* -----------------09.06.99 14:36-------------------
 *
 * --------------------------------------------------*/
class SwRenameXNamedDlg : public ModalDialog
{
	FixedLine       aNameFL;
	FixedText		aNewNameFT;
	NoSpaceEdit 	aNewNameED;
	OKButton 		aOk;
	CancelButton 	aCancel;
	HelpButton		aHelp;

	String 			sRemoveWarning;

	STAR_REFERENCE( container::XNamed ) & 	xNamed;
	STAR_REFERENCE( container::XNameAccess ) & xNameAccess;
	STAR_REFERENCE( container::XNameAccess )   xSecondAccess;
	STAR_REFERENCE( container::XNameAccess )   xThirdAccess;

	DECL_LINK(OkHdl, OKButton*);
	DECL_LINK(ModifyHdl, NoSpaceEdit*);

public:
	SwRenameXNamedDlg( Window* pParent,
					STAR_REFERENCE( container::XNamed ) & xNamed,
					STAR_REFERENCE( container::XNameAccess ) & xNameAccess );

	void	SetForbiddenChars( const String& rSet )
		{ aNewNameED.SetForbiddenChars( rSet ); }

	void SetAlternativeAccess(
			STAR_REFERENCE( container::XNameAccess ) & xSecond,
			STAR_REFERENCE( container::XNameAccess ) & xThird )
	{
		xSecondAccess = xSecond;
		xThirdAccess = xThird;
	}
};

#endif
