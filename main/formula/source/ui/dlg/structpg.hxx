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



#ifndef SC_STRUCTPG_HXX
#define SC_STRUCTPG_HXX

#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <svtools/svtreebx.hxx>
#include "formula/IFunctionDescription.hxx"
#include "ModuleHelper.hxx"

//============================================================================

//============================================================================
namespace formula
{

class IFormulaToken;
class	StructListBox : public SvTreeListBox
{
private:

	sal_Bool			bActiveFlag;

protected:
					virtual void MouseButtonDown( const MouseEvent& rMEvt );

public:

					StructListBox(Window* pParent, const ResId& rResId );

    /** Inserts an entry with static image (no difference between collapsed/expanded). */
    SvLBoxEntry*    InsertStaticEntry(
                        const XubString& rText,
                        const Image& rEntryImg,
                        const Image& rEntryImgHC,
                        SvLBoxEntry* pParent = NULL,
                        sal_uLong nPos = LIST_APPEND,
                        IFormulaToken* pToken = NULL );

	void			SetActiveFlag(sal_Bool bFlag=sal_True);
	sal_Bool			GetActiveFlag();
	void			GetFocus();
	void			LoseFocus();
};

//============================================================================

class StructPage : public TabPage
                    , public IStructHelper
{
private:
    OModuleClient   m_aModuleClient;
	Link			aSelLink;

	FixedText		aFtStruct;
	StructListBox	aTlbStruct;
    Image           maImgEnd;
    Image           maImgError;
    Image           maImgEndHC;
    Image           maImgErrorHC;

	IFormulaToken*	pSelectedToken;

	DECL_LINK( SelectHdl, SvTreeListBox* );

    using Window::GetParent;

protected:

	IFormulaToken*		GetFunctionEntry(SvLBoxEntry* pEntry);

public:

					StructPage( Window* pParent);

	void			ClearStruct();
	virtual SvLBoxEntry*	InsertEntry(const XubString& rText, SvLBoxEntry* pParent,
								sal_uInt16 nFlag,sal_uLong nPos=0,IFormulaToken* pScToken=NULL);

	virtual String	        GetEntryText(SvLBoxEntry* pEntry) const;
    virtual SvLBoxEntry*    GetParent(SvLBoxEntry* pEntry) const;

	String			GetSelectedEntryText();
	IFormulaToken*	GetSelectedToken();

	void            SetSelectionHdl( const Link& rLink ) { aSelLink = rLink; }
	const Link&     GetSelectionHdl() const { return aSelLink; }
};

} // formula

#endif

