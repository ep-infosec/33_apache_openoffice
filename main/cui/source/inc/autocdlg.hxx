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


#ifndef _OFA_AUTOCDLG_HXX
#define _OFA_AUTOCDLG_HXX

#include <svtools/langtab.hxx>
#include <sfx2/tabdlg.hxx>
#include <tools/table.hxx>
#include <svx/checklbx.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/metric.hxx>
#include <svx/langbox.hxx>

class SvxAutoCorrect;
class CharClass;
class CollatorWrapper;
class SmartTagMgr;

// class OfaAutoCorrDlg --------------------------------------------------

class OfaAutoCorrDlg : public SfxTabDialog
{
	FixedText		aLanguageFT;
	SvxLanguageBox	aLanguageLB;

	DECL_LINK(SelectLanguageHdl, ListBox*);
public:

	OfaAutoCorrDlg(Window* pParent, const SfxItemSet *pSet );

	void	EnableLanguage(sal_Bool bEnable)
			{	aLanguageFT.Enable(bEnable);
				aLanguageLB.Enable(bEnable);}

};

#ifdef _OFA_AUTOCDLG_CXX
#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <svtools/svtabbx.hxx>
#include <svx/simptabl.hxx>

class SvStringsISortDtor;

// class OfaACorrCheckListBox ------------------------------------------

class OfaACorrCheckListBox : public SvxSimpleTable
{
	using SvxSimpleTable::SetTabs;
	using SvTreeListBox::GetCheckButtonState;
	using SvTreeListBox::SetCheckButtonState;

	protected:
		virtual void	SetTabs();
		virtual void	HBarClick();
        virtual void    KeyInput( const KeyEvent& rKEvt );

	public:
		OfaACorrCheckListBox(Window* pParent, const ResId& rResId ) :
			SvxSimpleTable( pParent, rResId ){}

		inline void *GetUserData(sal_uLong nPos) { return GetEntry(nPos)->GetUserData(); }
		inline void SetUserData(sal_uLong nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }
		inline sal_uLong GetSelectEntryPos() { return GetModel()->GetAbsPos(FirstSelected()); }

		sal_Bool			IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
		void			CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked);
		SvButtonState	GetCheckButtonState( SvLBoxEntry*, sal_uInt16 nCol ) const;
		void			SetCheckButtonState( SvLBoxEntry*, sal_uInt16 nCol, SvButtonState );
};

// class OfaAutocorrOptionsPage ------------------------------------------


class OfaAutocorrOptionsPage : public SfxTabPage
{
	using TabPage::ActivatePage;

private:
	SvxCheckListBox	aCheckLB;

	String		sInput;
	String		sDoubleCaps;
	String		sStartCap;
	String		sBoldUnderline;
	String		sURL;
	String		sNoDblSpaces;
	String		sDash;
    String      sNonBrkSpace;
	String		sFirst;

public:
						OfaAutocorrOptionsPage( Window* pParent, const SfxItemSet& rSet );
						~OfaAutocorrOptionsPage();

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );

};

// class OfaSwAutoFmtOptionsPage ----------------------------------------------------

class OfaSwAutoFmtOptionsPage : public SfxTabPage
{
	using TabPage::ActivatePage;

	OfaACorrCheckListBox	aCheckLB;
	PushButton		aEditPB;
	FixedText		aHeader1Expl;
	FixedText		aHeader2Expl;

	String			sHeader1;
	String			sHeader2;

	String			sDeleteEmptyPara;
	String			sUseReplaceTbl;
	String			sCptlSttWord;
	String			sCptlSttSent;
	String			sUserStyle;
	String			sBullet;
	String			sByInputBullet;
	String			sBoldUnder;
	String			sNoDblSpaces;
	String			sDetectURL;
	String          sDash;
    String          sNonBrkSpace;
	String			sOrdinal;
	String			sRightMargin;
	String			sNum;
	String			sBorder;
	String			sTable;
	String			sReplaceTemplates;
	String			sDelSpaceAtSttEnd;
	String			sDelSpaceBetweenLines;

	String			sMargin;
	String			sBulletChar;
	String			sByInputBulletChar;

	Font			aBulletFont;
	Font 			aByInputBulletFont;
	sal_uInt16			nPercent;

	SvLBoxButtonData*	pCheckButtonData;

		DECL_LINK(SelectHdl, OfaACorrCheckListBox*);
		DECL_LINK(EditHdl, PushButton*);
		SvLBoxEntry* CreateEntry(String& rTxt, sal_uInt16 nCol);


		OfaSwAutoFmtOptionsPage( Window* pParent,
							const SfxItemSet& rSet );
		~OfaSwAutoFmtOptionsPage();

	public:
		static SfxTabPage*  Create( Window* pParent,
								const SfxItemSet& rAttrSet);
		virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
		virtual void        Reset( const SfxItemSet& rSet );
		virtual void		ActivatePage( const SfxItemSet& );
};

// class AutoCorrEdit ----------------------------------------------------

class AutoCorrEdit : public Edit
{
	Link 	aActionLink;
	sal_Bool 	bSpaces;

	public:
					AutoCorrEdit(Window* pParent, const ResId& rResId) :
						Edit(pParent, rResId), bSpaces(sal_False){}

	void 			SetActionHdl( const Link& rLink )
								{ aActionLink = rLink;}

	void 			SetSpaces(sal_Bool bSet)
								{bSpaces = bSet;}

	virtual void	KeyInput( const KeyEvent& rKEvent );
};

// class OfaAutocorrReplacePage ------------------------------------------

class DoubleStringArray;
typedef DoubleStringArray* DoubleStringArrayPtr;
DECLARE_TABLE(DoubleStringTable, DoubleStringArrayPtr)

class OfaAutocorrReplacePage : public SfxTabPage
{
		using TabPage::ActivatePage;
		using TabPage::DeactivatePage;

private:
		CheckBox		aTextOnlyCB;
		FixedText       aShortFT;
		AutoCorrEdit 	aShortED;
		FixedText       aReplaceFT;
		AutoCorrEdit 	aReplaceED;
		SvTabListBox 	aReplaceTLB;
		PushButton 		aNewReplacePB;
		PushButton 		aDeleteReplacePB;

		String			sModify;
		String			sNew;

		SvStringsISortDtor* 	pFormatText;
		DoubleStringTable		aDoubleStringTable;
		CollatorWrapper* 		pCompareClass;
        CharClass*              pCharClass;
		LanguageType 			eLang;

		sal_Bool			bHasSelectionText;
		sal_Bool			bFirstSelect:1;
		sal_Bool			bReplaceEditChanged:1;
		sal_Bool			bSWriter:1;

		DECL_LINK(SelectHdl, SvTabListBox*);
		DECL_LINK(NewDelHdl, PushButton*);
		DECL_LINK(ModifyHdl, Edit*);

		void 			RefillReplaceBox(sal_Bool bFromReset, //Box mit neuer Sprache fuellen
										LanguageType eOldLanguage,
										LanguageType eNewLanguage);

public:
						OfaAutocorrReplacePage( Window* pParent, const SfxItemSet& rSet );
						~OfaAutocorrReplacePage();

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );
	virtual int			DeactivatePage( SfxItemSet* pSet = 0 );

	void				SetLanguage(LanguageType eSet);
};

// class OfaAutocorrExceptPage ---------------------------------------------

struct StringsArrays;
typedef StringsArrays* StringsArraysPtr;
DECLARE_TABLE(StringsTable, StringsArraysPtr)

class OfaAutocorrExceptPage : public SfxTabPage
{
	using TabPage::ActivatePage;
	using TabPage::DeactivatePage;

private:
		FixedLine		aAbbrevFL;
		AutoCorrEdit	aAbbrevED;
		ListBox			aAbbrevLB;
		PushButton		aNewAbbrevPB;
		PushButton		aDelAbbrevPB;
		CheckBox        aAutoAbbrevCB;

		FixedLine		aDoubleCapsFL;
		AutoCorrEdit	aDoubleCapsED;
		ListBox			aDoubleCapsLB;
		PushButton		aNewDoublePB;
		PushButton		aDelDoublePB;
		CheckBox        aAutoCapsCB;

		StringsTable	aStringsTable;
		CollatorWrapper* pCompareClass;
		LanguageType 	eLang;

	DECL_LINK(NewDelHdl, PushButton*);
	DECL_LINK(SelectHdl, ListBox*);
	DECL_LINK(ModifyHdl, Edit*);

	void 			RefillReplaceBoxes(sal_Bool bFromReset, //Box mit neuer Sprache fuellen
										LanguageType eOldLanguage,
										LanguageType eNewLanguage);
public:
						OfaAutocorrExceptPage( Window* pParent, const SfxItemSet& rSet );
						~OfaAutocorrExceptPage();

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );
	virtual int			DeactivatePage( SfxItemSet* pSet = 0 );
	void				SetLanguage(LanguageType eSet);

};

// class OfaQuoteTabPage -------------------------------------------------

class OfaQuoteTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;

private:
    // For anything but writer
	SvxCheckListBox	aCheckLB;

    // Just for writer
	OfaACorrCheckListBox	aSwCheckLB;	
    String			sHeader1;
	String			sHeader2;
    
    String          sNonBrkSpace;
    String          sOrdinal;
	
    SvLBoxButtonData*	pCheckButtonData;

	FixedLine	aSingleFL;
	CheckBox    aSingleTypoCB;
	FixedText	aSglStartQuoteFT;
	PushButton  aSglStartQuotePB;
	FixedText	aSglStartExFT;
	FixedText	aSglEndQuoteFT;
	PushButton	aSglEndQuotePB;
	FixedText	aSglEndExFT;
	PushButton	aSglStandardPB;

	FixedLine	aDoubleFL;
	CheckBox    aTypoCB;
	FixedText	aStartQuoteFT;
	PushButton  aStartQuotePB;
	FixedText	aDblStartExFT;
	FixedText	aEndQuoteFT;
	PushButton	aEndQuotePB;
	FixedText	aDblEndExFT;
	PushButton	aDblStandardPB;

	String		sStartQuoteDlg;
	String		sEndQuoteDlg;

	String 		sStandard;


	sal_UCS4	cSglStartQuote;
	sal_UCS4	cSglEndQuote;

	sal_UCS4	cStartQuote;
	sal_UCS4	cEndQuote;

	DECL_LINK( QuoteHdl, PushButton* );
	DECL_LINK( StdQuoteHdl, PushButton* );

	String 				ChangeStringExt_Impl( sal_UCS4 );

    SvLBoxEntry* CreateEntry(String& rTxt, sal_uInt16 nCol);

						OfaQuoteTabPage( Window* pParent, const SfxItemSet& rSet );
public:
						~OfaQuoteTabPage();

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );
};

// class OfaAutoCompleteTabPage ---------------------------------------------

class OfaAutoCompleteTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;

	class AutoCompleteMultiListBox : public MultiListBox
	{
		OfaAutoCompleteTabPage& rPage;
	public:
		AutoCompleteMultiListBox( OfaAutoCompleteTabPage& rPg,
									const ResId& rResId )
			: MultiListBox( &rPg, rResId ), rPage( rPg ) {}

		virtual long PreNotify( NotifyEvent& rNEvt );
	};

    CheckBox        aCBActiv; //Enable word completion
    CheckBox        aCBAppendSpace;//Append space
    CheckBox        aCBAsTip; //Show as tip

    CheckBox        aCBCollect;//Collect words
    CheckBox        aCBRemoveList;//...save the list for later use...
    //--removed--CheckBox        aCBEndless;//

    FixedText       aFTExpandKey;
	ListBox 		aDCBExpandKey;
    FixedText       aFTMinWordlen;
    NumericField    aNFMinWordlen;
    FixedText       aFTMaxEntries;
    NumericField    aNFMaxEntries;
    AutoCompleteMultiListBox    aLBEntries;
	PushButton					aPBEntries;
	SvStringsISortDtor* 		pAutoCmpltList;
	sal_uInt16 						nAutoCmpltListCnt;

	DECL_LINK( CheckHdl, CheckBox* );

						OfaAutoCompleteTabPage( Window* pParent,
												const SfxItemSet& rSet );
public:
						virtual ~OfaAutoCompleteTabPage();

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );

	void CopyToClipboard() const;
	DECL_LINK( DeleteHdl, PushButton* );
};

// class OfaSmartTagOptionsTabPage ---------------------------------------------

/** Smart tag options tab page

    This tab page is used to enable/disable smart tag types
*/
class OfaSmartTagOptionsTabPage : public SfxTabPage
{
	using TabPage::ActivatePage;

private:
    
	// controls
    CheckBox				m_aMainCB;
	SvxCheckListBox			m_aSmartTagTypesLB;
	PushButton				m_aPropertiesPB;
	FixedText				m_aTitleFT;

    // construction via Create()
    OfaSmartTagOptionsTabPage( Window* pParent,	const SfxItemSet& rSet );
    
    /** Inserts items into m_aSmartTagTypesLB
    
        Reads out the smart tag types supported by the SmartTagMgr and
        inserts the associated strings into the list box.
    */
    void FillListBox( const SmartTagMgr& rSmartTagMgr );
    
    /** Clears the m_aSmartTagTypesLB
    */
    void ClearListBox();

    /** Handler for the check box

        Enables/disables all controls in the tab page (except from the
        check box.
    */
	DECL_LINK( CheckHdl, CheckBox* );
    
    /** Handler for the push button

        Calls the displayPropertyPage function of the smart tag recognizer
        associated with the currently selected smart tag type.
    */
	DECL_LINK( ClickHdl, PushButton* );

    /** Handler for the list box

        Enables/disables the properties push button if selection in the
        smart tag types list box changes.
    */
    DECL_LINK( SelectHdl, SvxCheckListBox* );
    
public:

	virtual ~OfaSmartTagOptionsTabPage();

	static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rAttrSet);

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
	virtual void		ActivatePage( const SfxItemSet& );
};

#endif // _OFA_AUTOCDLG_CXX

#endif //

