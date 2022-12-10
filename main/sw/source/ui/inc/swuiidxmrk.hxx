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


#ifndef _SWUI_IDXMRK_HXX
#define _SWUI_IDXMRK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <sfx2/basedlgs.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <vcl/lstbox.hxx>
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include <svx/stddlg.hxx>

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <sfx2/childwin.hxx>
#include "toxe.hxx"
#include <svtools/stdctrl.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>

class SwWrtShell;
class SwTOXMgr;
class SwTOXMark;
/*--------------------------------------------------------------------
	 Beschreibung:	Markierung fuer Verzeichniseintrag einfuegen
 --------------------------------------------------------------------*/
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;
class SwIndexMarkDlg : public Window
{
	friend class SwIndexMarkFloatDlg;
	friend class SwIndexMarkModalDlg;
	FixedLine       aIndexFL;
	FixedText 		aTypeFT;
	ListBox			aTypeDCB;
	ImageButton 	aNewBT;

	FixedText 		aEntryFT;
	Edit 			aEntryED;
	FixedText 		aPhoneticFT0;
	Edit			aPhoneticED0;

	FixedText 		aKeyFT;
	ComboBox		aKeyDCB;
	FixedText 		aPhoneticFT1;
	Edit			aPhoneticED1;

	FixedText 		aKey2FT;
	ComboBox 		aKey2DCB;
	FixedText 		aPhoneticFT2;
	Edit			aPhoneticED2;

	FixedText 		aLevelFT;
	NumericField	aLevelED;
 	CheckBox		aMainEntryCB;
 	CheckBox		aApplyToAllCB;
 	CheckBox		aSearchCaseSensitiveCB;
 	CheckBox	   	aSearchCaseWordOnlyCB;


	OKButton 		aOKBT;
	CancelButton 	aCancelBT;
	HelpButton 		aHelpBT;
	PushButton		aDelBT;
	//PushButton		aNewBT;

	ImageButton		aPrevSameBT;
	ImageButton		aNextSameBT;
	ImageButton		aPrevBT;
	ImageButton		aNextBT;

	String			aOrgStr;
	sal_Int32		nOptionsId;
	sal_Bool			bDel;
	sal_Bool			bNewMark;
	sal_Bool			bSelected;

	sal_Bool			bPhoneticED0_ChangedByUser;
	sal_Bool			bPhoneticED1_ChangedByUser;
	sal_Bool			bPhoneticED2_ChangedByUser;
	LanguageType	nLangForPhoneticReading; //Language of current text used for phonetic reading proposal
	sal_Bool			bIsPhoneticReadingEnabled; //this value states whether phopentic reading is enabled in principle dependend of global cjk settings and language of current entry
	com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier >
					xExtendedIndexEntrySupplier;

	SwTOXMgr*	   	pTOXMgr;
	SwWrtShell*	   	pSh;

	void			Apply();
	void 			InitControls();
	void			InsertMark();
	void 			UpdateMark();

	DECL_LINK( InsertHdl, Button * );
	DECL_LINK( CloseHdl, Button * );
	DECL_LINK( DelHdl, Button * );
	DECL_LINK( NextHdl, Button * );
	DECL_LINK( NextSameHdl, Button * );
	DECL_LINK( PrevHdl, Button * );
	DECL_LINK( PrevSameHdl, Button * );
	DECL_LINK( ModifyHdl, ListBox* pBox = 0 );
	DECL_LINK( KeyDCBModifyHdl, ComboBox * );
	DECL_LINK( NewUserIdxHdl, Button*);
	DECL_LINK( SearchTypeHdl, CheckBox*);
	DECL_LINK( PhoneticEDModifyHdl, Edit * );

	//this method updates the values from 'nLangForPhoneticReading' and 'bIsPhoneticReadingEnabled'
	//it needs to be called ones if this dialog is opened to create a new entry (in InitControls),
	//or otherwise it has to be called for each changed TOXMark (in UpdateDialog)
	void			UpdateLanguageDependenciesForPhoneticReading();
	String			GetDefaultPhoneticReading( const String& rText );

	void 			UpdateKeyBoxes();

	void			UpdateDialog();
	void			InsertUpdate();

	virtual void	Activate();

public:

	SwIndexMarkDlg( Window *pParent,
				   	sal_Bool bNewDlg,
					const ResId& rResId,
                    sal_Int32 _nOptionsId,
                    SwWrtShell& rWrtShell );


	~SwIndexMarkDlg();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark = 0);
	sal_Bool	IsTOXType(const String& rName)
				{return LISTBOX_ENTRY_NOTFOUND != aTypeDCB.GetEntryPos(rName);}
};
/* -----------------06.10.99 10:11-------------------

 --------------------------------------------------*/
class SwIndexMarkFloatDlg : public SfxModelessDialog
{
	SwIndexMarkDlg		aDlg;
	virtual void	Activate();
	public:
		SwIndexMarkFloatDlg( 	SfxBindings* pBindings,
				   				SfxChildWindow* pChild,
				   				Window *pParent,
								SfxChildWinInfo* pInfo,
				   				sal_Bool bNew=sal_True);
	void	ReInitDlg(SwWrtShell& rWrtShell);
};
/* -----------------06.10.99 10:33-------------------

 --------------------------------------------------*/
class SwIndexMarkModalDlg : public SvxStandardDialog
{
	SwIndexMarkDlg		aDlg;
public:
    SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark);

	virtual void		Apply();
	void	ReInitDlg(SwWrtShell& rWrtShell);
};

/* -----------------15.09.99 08:39-------------------

 --------------------------------------------------*/
class SwAuthMarkModalDlg;
class SwAuthMarkDlg : public Window
{
	static sal_Bool 	bIsFromComponent;

	friend class SwAuthMarkModalDlg;
    friend class SwAuthMarkFloatDlg;

    RadioButton     aFromComponentRB;
	RadioButton		aFromDocContentRB;
	FixedText		aAuthorFT;
	FixedInfo		aAuthorFI;
	FixedText		aTitleFT;
	FixedInfo		aTitleFI;
	FixedText		aEntryFT;
	Edit			aEntryED;
	ListBox 		aEntryLB;

    FixedLine       aEntryFL;

	OKButton 		aOKBT;
	CancelButton 	aCancelBT;
	HelpButton 		aHelpBT;
	PushButton 		aCreateEntryPB;
	PushButton 		aEditEntryPB;

	String			sChangeST;
	sal_Bool 			bNewEntry;
	sal_Bool			bBibAccessInitialized;

	SwWrtShell*	   	pSh;

	String			m_sColumnTitles[AUTH_FIELD_END];
	String			m_sFields[AUTH_FIELD_END];

	String 			m_sCreatedEntry[AUTH_FIELD_END];

	::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  	xBibAccess;

	DECL_LINK(InsertHdl, PushButton*);
	DECL_LINK(CloseHdl, PushButton*);
	DECL_LINK(CreateEntryHdl, PushButton*);
	DECL_LINK(CompEntryHdl, ListBox*);
	DECL_LINK(ChangeSourceHdl, RadioButton*);
	DECL_LINK(IsEntryAllowedHdl, Edit*);
    DECL_LINK(EditModifyHdl, Edit*);

	void InitControls();
    virtual void    Activate();
public:

	SwAuthMarkDlg( Window *pParent,
					const ResId& rResId,
				   	sal_Bool bNew=sal_True);
	~SwAuthMarkDlg();

	void	ReInitDlg(SwWrtShell& rWrtShell);
};
/* -----------------06.10.99 10:11-------------------

 --------------------------------------------------*/
class SwAuthMarkFloatDlg : public SfxModelessDialog
{
	SwAuthMarkDlg		aDlg;
	virtual void	Activate();
	public:
		SwAuthMarkFloatDlg( 	SfxBindings* pBindings,
				   				SfxChildWindow* pChild,
				   				Window *pParent,
								SfxChildWinInfo* pInfo,
				   				sal_Bool bNew=sal_True);
	void	ReInitDlg(SwWrtShell& rWrtShell);
};
/* -----------------06.10.99 10:33-------------------

 --------------------------------------------------*/
class SwAuthMarkModalDlg : public SvxStandardDialog
{
	SwAuthMarkDlg		aDlg;
public:
	SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh);

	virtual void		Apply();
	void	ReInitDlg(SwWrtShell& rWrtShell);
};


#endif // _SWUI_IDXMRK_HXX

