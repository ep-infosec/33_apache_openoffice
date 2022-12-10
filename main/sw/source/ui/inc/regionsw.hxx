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


#ifndef _REGIONSW_HXX
#define _REGIONSW_HXX

#include <hintids.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <svtools/svtreebx.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <editeng/brshitem.hxx>

#include <condedit.hxx>
#include <section.hxx>
#include <fmtclds.hxx>
#include <fmtftntx.hxx>
#include <fmtclbl.hxx>
#include <numberingtypelistbox.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/image.hxx>
#include <svx/paraprev.hxx>
#include <editeng/lrspitem.hxx>


class SwWrtShell;
class EditRegionDlg;

namespace sfx2
{
    class DocumentInserter;
    class FileDialogHelper;
}

/*************************************************************************
	Dialog "Bereiche bearbeiten"
*************************************************************************/

class SectRepr;
typedef SectRepr* SectReprPtr;
SV_DECL_PTRARR_SORT( SectReprArr, SectReprPtr, 0, 4 )

class SwEditRegionDlg : public SfxModalDialog
{
    FixedLine       aNameFL;
    Edit            aCurName;
    SvTreeListBox   aTree;

    FixedLine       aLinkFL;
    TriStateBox     aFileCB;
	CheckBox		aDDECB;
	FixedText		aFileNameFT;
	FixedText		aDDECommandFT;
	Edit 			aFileNameED;
    PushButton      aFilePB;
    FixedText       aSubRegionFT;
    ComboBox        aSubRegionED;
    bool            bSubRegionsFilled;

    FixedLine       aProtectFL;
    TriStateBox     aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    TriStateBox     aHideCB;
    FixedText       aConditionFT;
	ConditionEdit	aConditionED;

    // --> FME 2004-06-22 #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    TriStateBox     aEditInReadonlyCB;
    // <--

    OKButton        aOK;
	CancelButton 	aCancel;
	PushButton 		aOptionsPB;
	PushButton 		aDismiss;
	HelpButton		aHelp;
    ImageList       aImageIL;
    ImageList       aImageILH;

    SwWrtShell&             rSh;
    SectReprArr             aSectReprArr;
    SvLBoxEntry*            pAktEntry;
    const SwSection*        pCurrSect;
    sfx2::DocumentInserter* m_pDocInserter;
    Window*                 m_pOldDefDlgParent;

    sal_Bool            bDontCheckPasswd :1;
	sal_Bool 			bWeb			:1;


    Image  BuildBitmap(sal_Bool bProtect,sal_Bool bHidden,sal_Bool bHighContrast);

    void    RecurseList( const SwSectionFmt* pFmt, SvLBoxEntry* pEntry);
	sal_uInt16 	FindArrPos(const SwSectionFmt* pFmt);

	DECL_LINK( GetFirstEntryHdl, SvTreeListBox * );
	DECL_LINK( DeselectHdl, SvTreeListBox * );

	DECL_LINK( OkHdl, CheckBox * );
	DECL_LINK( NameEditHdl, Edit * );
	DECL_LINK( ConditionEditHdl, Edit * );

    DECL_LINK( ChangePasswdHdl, Button * );
	DECL_LINK( ChangeProtectHdl, TriStateBox * );
	DECL_LINK( ChangeHideHdl, TriStateBox * );
    // --> FME 2004-06-22 #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, TriStateBox * );
    // <--
    DECL_LINK( ChangeDismissHdl, CheckBox * );
	DECL_LINK( UseFileHdl, CheckBox* );
	DECL_LINK( FileSearchHdl, PushButton* );
	DECL_LINK( OptionsHdl, PushButton* );
	DECL_LINK( FileNameHdl, Edit* );
	DECL_LINK( DDEHdl, CheckBox* );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );
    DECL_LINK( SubRegionEventHdl, VclWindowEvent * );

    sal_Bool CheckPasswd(CheckBox* pBox = 0);

public:
	SwEditRegionDlg( Window* pParent, SwWrtShell& rWrtSh );
	virtual ~SwEditRegionDlg();

    void    SelectSection(const String& rSectionName);

};
/*************************************************************************
	Dialog "Bereich einfuegen"
*************************************************************************/

/* -----------------21.05.99 11:05-------------------
 *
 * --------------------------------------------------*/
class SwInsertSectionTabPage : public SfxTabPage
{
    FixedLine       aNameFL;
    ComboBox        aCurName;

    FixedLine       aLinkFL;
    CheckBox        aFileCB;
	CheckBox		aDDECB;
	FixedText		aDDECommandFT;
	FixedText		aFileNameFT;
	Edit 			aFileNameED;
    PushButton      aFilePB;
    FixedText       aSubRegionFT;
	ComboBox		aSubRegionED;

    FixedLine       aProtectFL;
    CheckBox        aProtectCB;
    CheckBox        aPasswdCB;
    PushButton      aPasswdPB;

    FixedLine       aHideFL;
    CheckBox        aHideCB;
    FixedText       aConditionFT;
	ConditionEdit	aConditionED;

    // --> FME 2004-06-22 #114856# edit in readonly sections
    FixedLine       aPropertiesFL;
    CheckBox        aEditInReadonlyCB;
    // <--

    String          m_sFileName;
    String          m_sFilterName;
    String          m_sFilePasswd;

//	SwFmtCol*		pCols;
    ::com::sun::star::uno::Sequence <sal_Int8 > m_aNewPasswd;
    SwWrtShell*             m_pWrtSh;
    sfx2::DocumentInserter* m_pDocInserter;
    Window*                 m_pOldDefDlgParent;

	DECL_LINK( ChangeHideHdl, CheckBox * );
    // --> FME 2004-06-22 #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, CheckBox * );
    // <--
	DECL_LINK( ChangeProtectHdl, CheckBox * );
    DECL_LINK( ChangePasswdHdl, Button * );
    DECL_LINK( NameEditHdl, Edit * );
	DECL_LINK( UseFileHdl, CheckBox* );
	DECL_LINK( FileSearchHdl, PushButton* );
	DECL_LINK( DDEHdl, CheckBox* );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper* );

public:
	SwInsertSectionTabPage(Window *pParent, const SfxItemSet &rAttrSet);
	virtual ~SwInsertSectionTabPage();

	void	SetWrtShell(SwWrtShell& rSh);

	virtual sal_Bool		FillItemSet( SfxItemSet& );
	virtual void		Reset( const SfxItemSet& );

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);
};

class SwSectionFtnEndTabPage : public SfxTabPage
{
    FixedLine       aFtnFL;
	CheckBox 		aFtnNtAtTextEndCB;

	CheckBox 		aFtnNtNumCB;
	FixedText 		aFtnOffsetLbl;
	NumericField 	aFtnOffsetFld;

	CheckBox 		aFtnNtNumFmtCB;
	FixedText		aFtnPrefixFT;
	Edit			aFtnPrefixED;
	SwNumberingTypeListBox	 aFtnNumViewBox;
	FixedText		aFtnSuffixFT;
	Edit			aFtnSuffixED;

    FixedLine       aEndFL;
	CheckBox 		aEndNtAtTextEndCB;

	CheckBox 		aEndNtNumCB;
	FixedText 		aEndOffsetLbl;
	NumericField 	aEndOffsetFld;

	CheckBox 		aEndNtNumFmtCB;
	FixedText		aEndPrefixFT;
	Edit			aEndPrefixED;
	SwNumberingTypeListBox	 aEndNumViewBox;
	FixedText		aEndSuffixFT;
	Edit			aEndSuffixED;


	DECL_LINK( FootEndHdl, CheckBox * );
	void ResetState( sal_Bool bFtn, const SwFmtFtnEndAtTxtEnd& );

public:
	SwSectionFtnEndTabPage( Window *pParent, const SfxItemSet &rAttrSet );
	virtual ~SwSectionFtnEndTabPage();

	virtual sal_Bool		FillItemSet( SfxItemSet& );
	virtual void		Reset( const SfxItemSet& );

	static SfxTabPage*	Create( Window* pParent,
								const SfxItemSet& rAttrSet);
};
/* -----------------13.06.2003 09:51-----------------

 --------------------------------------------------*/
class SwSectionIndentTabPage : public SfxTabPage
{
    FixedLine           aIndentFL;
    FixedText           aBeforeFT;
    MetricField         aBeforeMF;
    FixedText           aAfterFT;
    MetricField         aAfterMF;

    SvxParaPrevWindow   aPreviewWin;

    DECL_LINK(IndentModifyHdl, MetricField*);
public:
    SwSectionIndentTabPage( Window *pParent, const SfxItemSet &rAttrSet );
    virtual ~SwSectionIndentTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void    SetWrtShell(SwWrtShell& rSh);
};

/* -----------------21.05.99 13:07-------------------
 *
 * --------------------------------------------------*/
class SwInsertSectionTabDialog : public SfxTabDialog
{
	SwWrtShell& 	rWrtSh;
    ::std::auto_ptr<SwSectionData> m_pSectionData;

protected:
	virtual void 	PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
	virtual short	Ok();
public:
	SwInsertSectionTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
	virtual ~SwInsertSectionTabDialog();

    void        SetSectionData(SwSectionData const& rSect);
    SwSectionData * GetSectionData() { return m_pSectionData.get(); }
};

/* -----------------21.05.99 13:07-------------------
 *
 * --------------------------------------------------*/
class SwSectionPropertyTabDialog : public SfxTabDialog
{
    SwWrtShell& rWrtSh;
protected:
	virtual void 	PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:
	SwSectionPropertyTabDialog(Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
	virtual ~SwSectionPropertyTabDialog();
};

#endif


