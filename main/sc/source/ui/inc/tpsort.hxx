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



#ifndef SC_TPSORT_HXX
#define SC_TPSORT_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/langbox.hxx>


#include "global.hxx"
#include "address.hxx"

//------------------------------------------------------------------------

// +1 because one field is reserved for the "- undefined -" entry
#define SC_MAXFIELDS	MAXCOLCOUNT+1

class ScViewData;
class ScSortDlg;
struct ScSortParam;

//========================================================================
// Kriterien

class ScTabPageSortFields : public SfxTabPage
{
public:
				ScTabPageSortFields( Window*			 pParent,
									 const SfxItemSet&	 rArgSet );
				~ScTabPageSortFields();

	static	SfxTabPage*	Create		( Window*				pParent,
									  const SfxItemSet& 	rArgSet );
	static	sal_uInt16*		GetRanges	();
	virtual	sal_Bool		FillItemSet	( SfxItemSet& rArgSet );
	virtual	void		Reset		( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//	virtual void		ActivatePage	( const SfxItemSet& rSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
	virtual void		ActivatePage	();
	virtual int			DeactivatePage	( SfxItemSet* pSet = 0);

private:
    FixedLine       aFlSort1;
	ListBox			aLbSort1;
	RadioButton		aBtnUp1;
	RadioButton 	aBtnDown1;

    FixedLine       aFlSort2;
	ListBox			aLbSort2;
	RadioButton		aBtnUp2;
	RadioButton 	aBtnDown2;

    FixedLine       aFlSort3;
	ListBox			aLbSort3;
	RadioButton		aBtnUp3;
	RadioButton 	aBtnDown3;

	String			aStrUndefined;
	String			aStrColumn;
	String			aStrRow;

	const sal_uInt16		nWhichSort;
	ScSortDlg*			pDlg;
	ScViewData*			pViewData;
	const ScSortParam&	rSortData;
	SCCOLROW			nFieldArr[SC_MAXFIELDS];
	sal_uInt16				nFieldCount;
	SCCOL				nFirstCol;
	SCROW				nFirstRow;
	sal_Bool				bHasHeader;
	sal_Bool				bSortByRows;

	ListBox*			aSortLbArr[3];
	RadioButton*		aDirBtnArr[3][2];
    FixedLine*          aFlArr[3];

#ifdef _TPSORT_CXX
private:
	void	Init			();
	void	DisableField	( sal_uInt16 nField );
	void	EnableField		( sal_uInt16 nField );
	void	FillFieldLists	();
	sal_uInt16	GetFieldSelPos	( SCCOLROW nField );

	// Handler ------------------------
	DECL_LINK( SelectHdl, ListBox * );
#endif
};

//========================================================================
// Sortieroptionen:

class ScDocument;
class ScRangeData;
class CollatorRessource;
class CollatorWrapper;

#if ENABLE_LAYOUT_EXPERIMENTAL
#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>
#else /* !ENABLE_LAYOUT_EXPERIMENTAL */
#define LocalizedString String
#endif /* !ENABLE_LAYOUT_EXPERIMENTAL */

class ScTabPageSortOptions : public SfxTabPage
{
public:
				ScTabPageSortOptions( Window*			 pParent,
									  const SfxItemSet&	 rArgSet );
				~ScTabPageSortOptions();

#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
	static	SfxTabPage*	Create		( Window*				pParent,
									  const SfxItemSet& 	rArgSet );
	static	sal_uInt16*		GetRanges	();
	virtual	sal_Bool		FillItemSet	( SfxItemSet& rArgSet );
	virtual	void		Reset		( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//	virtual void		ActivatePage	( const SfxItemSet& rSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
	virtual void		ActivatePage	();
	virtual int			DeactivatePage	( SfxItemSet* pSet = 0);

private:

    CheckBox            aBtnCase;
    CheckBox            aBtnHeader;
    CheckBox            aBtnFormats;

    CheckBox            aBtnCopyResult;
    ListBox             aLbOutPos;
    Edit                aEdOutPos;

    CheckBox            aBtnSortUser;
    ListBox             aLbSortUser;

    FixedText           aFtLanguage;
    SvxLanguageBox      aLbLanguage;
    FixedText           aFtAlgorithm;
    ListBox             aLbAlgorithm;

    FixedLine           aLineDirection;
    RadioButton         aBtnTopDown;
    RadioButton         aBtnLeftRight;

    FixedText           aFtAreaLabel;
//  FixedInfo           aFtArea;
    LocalizedString aStrRowLabel;
    LocalizedString aStrColLabel;
    LocalizedString aStrUndefined;
    String              aStrNoName;
    String              aStrAreaLabel;

	const sal_uInt16		nWhichSort;
	const ScSortParam&	rSortData;
	ScViewData*			pViewData;
	ScDocument*			pDoc;
	ScSortDlg*			pDlg;
	ScAddress			theOutPos;

	CollatorRessource*	pColRes;
	CollatorWrapper*	pColWrap;

#ifdef _TPSORT_CXX
private:
	void Init					();
	void FillUserSortListBox	();
	void FillOutPosList 		();

	// Handler ------------------------
	DECL_LINK( EnableHdl, CheckBox * );
	DECL_LINK( SelOutPosHdl, ListBox * );
	void EdOutPosModHdl	( Edit* pEd );
	DECL_LINK( SortDirHdl, RadioButton * );
	DECL_LINK( FillAlgorHdl, void * );
#endif
};

#if ENABLE_LAYOUT_EXPERIMENTAL
#include <layout/layout-post.hxx>
#endif /* ENABLE_LAYOUT_EXPERIMENTAL */

#endif // SC_TPSORT_HXX

