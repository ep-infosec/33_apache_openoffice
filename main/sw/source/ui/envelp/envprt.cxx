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


#include <vcl/print.hxx>
#include <svtools/prnsetup.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "envprt.hxx"
#include "envlop.hxx"
#include "uitool.hxx"

#include "envprt.hrc"





SwEnvPrtPage::SwEnvPrtPage(Window* pParent, const SfxItemSet& rSet) :

	SfxTabPage(pParent, SW_RES(TP_ENV_PRT), rSet),

	aAlignBox    (this, SW_RES(BOX_ALIGN  )),
	aTopButton   (this, SW_RES(BTN_TOP    )),
	aBottomButton(this, SW_RES(BTN_BOTTOM )),
	aRightText   (this, SW_RES(TXT_RIGHT  )),
	aRightField  (this, SW_RES(FLD_RIGHT  )),
	aDownText    (this, SW_RES(TXT_DOWN   )),
	aDownField   (this, SW_RES(FLD_DOWN   )),
	aPrinterInfo (this, SW_RES(TXT_PRINTER)),
    aNoNameFL    (this, SW_RES(FL_NONAME )),
    aPrinterFL   (this, SW_RES(FL_PRINTER )),
	aPrtSetup    (this, SW_RES(BTN_PRTSETUP))

{
	FreeResource();
	SetExchangeSupport();

	// Metriken
	FieldUnit eUnit = ::GetDfltMetric(sal_False);
	SetMetric(aRightField, eUnit);
	SetMetric(aDownField , eUnit);

	// Handler installieren
	aTopButton   .SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));
	aBottomButton.SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));

	aPrtSetup    .SetClickHdl(LINK(this, SwEnvPrtPage, ButtonHdl));

	// Bitmaps
	aBottomButton.GetClickHdl().Call(&aBottomButton);

	// ToolBox
	Size aSz = aAlignBox.CalcWindowSizePixel();
	aAlignBox.SetSizePixel(aSz);
//    aAlignBox.SetPosPixel(Point(aNoNameFL.GetPosPixel().X() + (aNoNameFL.GetSizePixel().Width() - aSz.Width()) / 2, aAlignBox.GetPosPixel().Y()));
	aAlignBox.SetClickHdl(LINK(this, SwEnvPrtPage, AlignHdl));

}

// --------------------------------------------------------------------------



SwEnvPrtPage::~SwEnvPrtPage()
{
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, ClickHdl, Button *, EMPTYARG )
{
	sal_Bool bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
	if (aBottomButton.IsChecked())
	{
		// Briefumschlaege von unten
		aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(bHC ? BMP_HOR_LEFT_LOWER_H : BMP_HOR_LEFT_LOWER)));
		aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(bHC ? BMP_HOR_CNTR_LOWER_H : BMP_HOR_CNTR_LOWER)));
		aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(bHC ? BMP_HOR_RGHT_LOWER_H : BMP_HOR_RGHT_LOWER)));
		aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(bHC ? BMP_VER_LEFT_LOWER_H : BMP_VER_LEFT_LOWER)));
		aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(bHC ? BMP_VER_CNTR_LOWER_H : BMP_VER_CNTR_LOWER)));
		aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(bHC ? BMP_VER_RGHT_LOWER_H : BMP_VER_RGHT_LOWER)));
	}
	else
	{
		// Briefumschlaege von oben
		aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(bHC ? BMP_HOR_LEFT_UPPER_H : BMP_HOR_LEFT_UPPER)));
		aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(bHC ? BMP_HOR_CNTR_UPPER_H : BMP_HOR_CNTR_UPPER)));
		aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(bHC ? BMP_HOR_RGHT_UPPER_H : BMP_HOR_RGHT_UPPER)));
		aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(bHC ? BMP_VER_LEFT_UPPER_H : BMP_VER_LEFT_UPPER)));
		aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(bHC ? BMP_VER_CNTR_UPPER_H : BMP_VER_CNTR_UPPER)));
		aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(bHC ? BMP_VER_RGHT_UPPER_H : BMP_VER_RGHT_UPPER)));
	}
	return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, ButtonHdl, Button *, pBtn )
{
	if (pBtn == &aPrtSetup)
	{
		// Druck-Setup aufrufen
		if (pPrt)
		{
			PrinterSetupDialog* pDlg = new PrinterSetupDialog(this );
			pDlg->SetPrinter(pPrt);
			pDlg->Execute();
			delete pDlg;
			GrabFocus();
			aPrinterInfo.SetText(pPrt->GetName());
		}
	}
	return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, AlignHdl, ToolBox *, EMPTYARG )
{
	if (aAlignBox.GetCurItemId())
	{
		for (sal_uInt16 i = ITM_HOR_LEFT; i <= ITM_VER_RGHT; i++)
			aAlignBox.CheckItem(i, sal_False);
		aAlignBox.CheckItem(aAlignBox.GetCurItemId(), sal_True);
	}
	else
	{
		// GetCurItemId() == 0 ist moeglich!
		const SwEnvItem& rItem = (const SwEnvItem&) GetItemSet().Get(FN_ENVELOP);
		aAlignBox.CheckItem((sal_uInt16) rItem.eAlign + ITM_HOR_LEFT, sal_True);
	}
	return 0;
}

// --------------------------------------------------------------------------



SfxTabPage* SwEnvPrtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
	return new SwEnvPrtPage(pParent, rSet);
}

// --------------------------------------------------------------------------



void SwEnvPrtPage::ActivatePage(const SfxItemSet&)
{
	if (pPrt)
		aPrinterInfo.SetText(pPrt->GetName());
}

// --------------------------------------------------------------------------



int SwEnvPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(*_pSet);
	return SfxTabPage::LEAVE_PAGE;
}

// --------------------------------------------------------------------------



void SwEnvPrtPage::FillItem(SwEnvItem& rItem)
{
	sal_uInt16 nID = 0;
	for (sal_uInt16 i = ITM_HOR_LEFT; i <= ITM_VER_RGHT && !nID; i++)
		if (aAlignBox.IsItemChecked(i))
			nID = i;

	rItem.eAlign          = (SwEnvAlign) (nID - ITM_HOR_LEFT);
	rItem.bPrintFromAbove = aTopButton.IsChecked();
    rItem.lShiftRight     = static_cast< sal_Int32 >(GetFldVal(aRightField));
    rItem.lShiftDown      = static_cast< sal_Int32 >(GetFldVal(aDownField ));
}

// --------------------------------------------------------------------------



sal_Bool SwEnvPrtPage::FillItemSet(SfxItemSet& rSet)
{
	FillItem(GetParent()->aEnvItem);
	rSet.Put(GetParent()->aEnvItem);
	return sal_True;
}

// ----------------------------------------------------------------------------



void SwEnvPrtPage::Reset(const SfxItemSet& rSet)
{
//    SfxItemSet aSet(rSet);
//    aSet.Put(GetParent()->aEnvItem);

	// Item auslesen
	const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
	aAlignBox.CheckItem((sal_uInt16) rItem.eAlign + ITM_HOR_LEFT);

	if (rItem.bPrintFromAbove)
		aTopButton   .Check();
	else
		aBottomButton.Check();

	SetFldVal(aRightField, rItem.lShiftRight);
	SetFldVal(aDownField , rItem.lShiftDown );

	ActivatePage(rSet);
	ClickHdl(&aTopButton);
}




