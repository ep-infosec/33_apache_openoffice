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



#ifndef SC_INSCLDLG_HXX
#define SC_INSCLDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>


#include "global.hxx"

//------------------------------------------------------------------------

class ScInsertCellDlg : public ModalDialog
{
private:
    FixedLine       aFlFrame;
	RadioButton		aBtnCellsDown;
	RadioButton		aBtnCellsRight;
	RadioButton		aBtnInsRows;
	RadioButton		aBtnInsCols;
	OKButton		aBtnOk;
	CancelButton	aBtnCancel;
	HelpButton		aBtnHelp;

public:
			ScInsertCellDlg( Window* pParent,sal_Bool bDisallowCellMove = sal_False );
			~ScInsertCellDlg();

	InsCellCmd GetInsCellCmd() const;
};


#endif // SC_INSCLDLG_HXX


