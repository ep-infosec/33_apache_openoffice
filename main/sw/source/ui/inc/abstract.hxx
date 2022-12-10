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


#ifndef _ABSTRACT_HXX
#define _ABSTRACT_HXX

#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif


/*-----------------22.02.97 15.06-------------------

--------------------------------------------------*/
class SwInsertAbstractDlg  : public SfxModalDialog
{
    FixedLine       aFL;
	FixedText		aLevelFT;
	NumericField	aLevelNF;
	FixedText		aParaFT;
	NumericField	aParaNF;
	FixedText		aDescFT;
	OKButton		aOkPB;
	CancelButton	aCancelPB;
	HelpButton		aHelpPB;

protected:

public:
	SwInsertAbstractDlg( Window* pParent );
	~SwInsertAbstractDlg();

	sal_uInt8	GetLevel() const;
	sal_uInt8	GetPara() const;
};

#endif



