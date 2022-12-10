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
#include "precompiled_sw.hxx"


#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <modcfg.hxx>
#include <svx/htmlmode.hxx>
#include <viewopt.hxx>
#include "swmodule.hxx"
#include "cmdid.h"
#include "convert.hxx"
#include "tablemgr.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "tblafmt.hxx"

#include "table.hrc"
#include "convert.hrc"
#include "swabstdlg.hxx"

namespace swui
{
	SwAbstractDialogFactory * GetFactory();
}

//keep the state of the buttons on runtime
static int nSaveButtonState = -1; // 0: tab, 1: semicolon, 2: paragraph, 3: other, -1: not yet used
static sal_Bool bIsKeepColumn = sal_True;
static sal_Unicode uOther = ',';

void SwConvertTableDlg::GetValues(  sal_Unicode& rDelim,
									SwInsertTableOptions& rInsTblOpts,
									SwTableAutoFmt *& prTAFmt )
{
	if( aTabBtn.IsChecked() )
    {
        //0x0b mustn't be set when re-converting table into text
        bIsKeepColumn = !aKeepColumn.IsVisible() || aKeepColumn.IsChecked();
        rDelim = bIsKeepColumn ? 0x09 : 0x0b;
        nSaveButtonState = 0;
    }
	else if( aSemiBtn.IsChecked() )
    {
		rDelim = ';';
        nSaveButtonState = 1;
    }
    else if( aOtherBtn.IsChecked() && aOtherEd.GetText().Len() )
    {
        uOther = aOtherEd.GetText().GetChar( 0 );
        rDelim = uOther;
        nSaveButtonState = 3;
    }
	else
    {
        nSaveButtonState = 2;
        rDelim = cParaDelim;
        if(aOtherBtn.IsChecked())
        {
            nSaveButtonState = 3;
            uOther = 0;
        }
    }


	sal_uInt16 nInsMode = 0;
	if (aBorderCB.IsChecked())
        nInsMode |= tabopts::DEFAULT_BORDER;
	if (aHeaderCB.IsChecked())
        nInsMode |= tabopts::HEADLINE;
	if (aRepeatHeaderCB.IsEnabled() && aRepeatHeaderCB.IsChecked())
        rInsTblOpts.mnRowsToRepeat = sal_uInt16( aRepeatHeaderNF.GetValue() );
	else
		rInsTblOpts.mnRowsToRepeat = 0;
	if (!aDontSplitCB.IsChecked())
        nInsMode |= tabopts::SPLIT_LAYOUT;

	if( pTAutoFmt )
		prTAFmt = new SwTableAutoFmt( *pTAutoFmt );

	rInsTblOpts.mnInsMode = nInsMode;
}


SwConvertTableDlg::SwConvertTableDlg( SwView& rView, bool bToTable )

	: SfxModalDialog( &rView.GetViewFrame()->GetWindow(), SW_RES(DLG_CONV_TEXT_TABLE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    aTabBtn         (this, SW_RES(CB_TAB)),
	aSemiBtn		(this, SW_RES(CB_SEMI)),
	aParaBtn		(this, SW_RES(CB_PARA)),
    aOtherBtn       (this, SW_RES(RB_OTHER)),
    aOtherEd        (this, SW_RES(ED_OTHER)),
    aKeepColumn     (this, SW_RES(CB_KEEPCOLUMN)),
    aDelimFL       (this, SW_RES(FL_DELIM)),

    aHeaderCB       (this, SW_RES(CB_HEADER)),
	aRepeatHeaderCB	(this, SW_RES(CB_REPEAT_HEADER)),

    aRepeatHeaderFT         (this, SW_RES(FT_REPEAT_HEADER)),
    aRepeatHeaderBeforeFT   (this),
    aRepeatHeaderNF         (this, SW_RES(NF_REPEAT_HEADER)),
    aRepeatHeaderAfterFT    (this),
    aRepeatHeaderCombo      (this, SW_RES(WIN_REPEAT_HEADER), aRepeatHeaderNF, aRepeatHeaderBeforeFT, aRepeatHeaderAfterFT),

    aOptionsFL      (this, SW_RES(FL_OPTIONS)),
	aDontSplitCB	(this, SW_RES(CB_DONT_SPLIT)),
	aBorderCB		(this, SW_RES(CB_BORDER)),
    aAutoFmtBtn(this,SW_RES(BT_AUTOFORMAT)),

	aOkBtn(this,SW_RES(BT_OK)),
	aCancelBtn(this,SW_RES(BT_CANCEL)),
	aHelpBtn(this, SW_RES(BT_HELP)),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    sConvertTextTable(SW_RES(STR_CONVERT_TEXT_TABLE)),
	pTAutoFmt( 0 ),
    pShell( &rView.GetWrtShell() )
{
	aOtherEd.SetAccessibleName(String(SW_RES(STR_SYMBOL)));
	aOtherEd.SetAccessibleRelationLabeledBy(&aOtherBtn);
	FreeResource();
    if(nSaveButtonState > -1)
    {
        switch (nSaveButtonState)
        {
            case 0:
                aTabBtn.Check();
                aKeepColumn.Check(bIsKeepColumn);
            break;
            case 1: aSemiBtn.Check();break;
            case 2: aParaBtn.Check();break;
            case 3:
                aOtherBtn.Check();
                if(uOther)
                    aOtherEd.SetText(uOther);
            break;
        }

    }
    if( bToTable )
	{
		SetText( sConvertTextTable );
		aAutoFmtBtn.SetClickHdl(LINK(this, SwConvertTableDlg, AutoFmtHdl));
		aAutoFmtBtn.Show();
		aKeepColumn.Show();
		aKeepColumn.Enable( aTabBtn.IsChecked() );
        aRepeatHeaderCombo.Arrange( aRepeatHeaderFT );
	}
	else
	{
		//Einfuege-Optionen verstecken
		aHeaderCB		   .Show(sal_False);
		aRepeatHeaderCB	   .Show(sal_False);
		aDontSplitCB	   .Show(sal_False);
		aBorderCB		   .Show(sal_False);
        aOptionsFL         .Show(sal_False);
        aRepeatHeaderCombo.Show(sal_False);

		//Groesse anpassen
        Size aSize(GetSizePixel());
        aSize.Height() = 8 + aHelpBtn.GetSizePixel().Height() + aHelpBtn.GetPosPixel().Y();
        SetOutputSizePixel(aSize);
	}
	aKeepColumn.SaveValue();

	Link aLk( LINK(this, SwConvertTableDlg, BtnHdl) );
	aTabBtn.SetClickHdl( aLk );
	aSemiBtn.SetClickHdl( aLk );
	aParaBtn.SetClickHdl( aLk );
	aOtherBtn.SetClickHdl(aLk );
	aOtherEd.Enable( aOtherBtn.IsChecked() );

	const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

	sal_Bool bHTMLMode = 0 != (::GetHtmlMode(rView.GetDocShell())&HTMLMODE_ON);

	SwInsertTableOptions aInsOpts = pModOpt->GetInsTblFlags(bHTMLMode);
	sal_uInt16 nInsTblFlags = aInsOpts.mnInsMode;

    aHeaderCB.Check( 0 != (nInsTblFlags & tabopts::HEADLINE) );
    aRepeatHeaderCB.Check(aInsOpts.mnRowsToRepeat > 0);
    aDontSplitCB.Check( 0 == (nInsTblFlags & tabopts::SPLIT_LAYOUT));
    aBorderCB.Check( 0!= (nInsTblFlags & tabopts::DEFAULT_BORDER) );

	aHeaderCB.SetClickHdl(LINK(this, SwConvertTableDlg, CheckBoxHdl));
	aRepeatHeaderCB.SetClickHdl(LINK(this, SwConvertTableDlg, ReapeatHeaderCheckBoxHdl));
	ReapeatHeaderCheckBoxHdl();
	CheckBoxHdl();
}

SwConvertTableDlg::	~SwConvertTableDlg()
{
	delete pTAutoFmt;
}

IMPL_LINK( SwConvertTableDlg, AutoFmtHdl, PushButton*, pButton )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwAutoFormatDlg* pDlg = pFact->CreateSwAutoFormatDlg(pButton, pShell, DLG_AUTOFMT_TABLE, sal_False, pTAutoFmt);
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFmtOfIndex( pTAutoFmt );
    delete pDlg;
	return 0;
}

IMPL_LINK( SwConvertTableDlg, BtnHdl, Button*, pButton )
{
	if( pButton == &aTabBtn )
		aKeepColumn.SetState( aKeepColumn.GetSavedValue() );
	else
	{
		if( aKeepColumn.IsEnabled() )
			aKeepColumn.SaveValue();
		aKeepColumn.Check( sal_True );
	}
	aKeepColumn.Enable( aTabBtn.IsChecked() );
	aOtherEd.Enable( aOtherBtn.IsChecked() );
	return 0;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(SwConvertTableDlg, CheckBoxHdl, CheckBox*, EMPTYARG)
{
	aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());
	ReapeatHeaderCheckBoxHdl();

	return 0;
}

IMPL_LINK(SwConvertTableDlg, ReapeatHeaderCheckBoxHdl, void*, EMPTYARG)
{
    sal_Bool bEnable = aHeaderCB.IsChecked() && aRepeatHeaderCB.IsChecked();
    aRepeatHeaderBeforeFT.Enable(bEnable);
    aRepeatHeaderAfterFT.Enable(bEnable);
    aRepeatHeaderNF.Enable(bEnable);

	return 0;
}
