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

#include "precompiled_sc.hxx"

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Layouter.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <NumberFormatPropertyPanel.hxx>
#include <NumberFormatPropertyPanel.hrc>
#include "sc.hrc"
#include "scresid.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Layouter;
using ::sfx2::sidebar::Theme;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

//////////////////////////////////////////////////////////////////////////////
// namespace open

namespace sc { namespace sidebar {

//////////////////////////////////////////////////////////////////////////////

NumberFormatPropertyPanel::NumberFormatPropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   Control(
        pParent, 
        ScResId(RID_PROPERTYPANEL_SC_NUMBERFORMAT)),
    mpFtCategory(new FixedText(this, ScResId(FT_CATEGORY))),
    mpLbCategory(new ListBox(this, ScResId(LB_CATEGORY))),
    mpTBCategoryBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBCategory(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBCategoryBackground.get(), ScResId(TBX_CATEGORY))),
    mpFtDecimals(new FixedText(this, ScResId(FT_DECIMALS))),
    mpEdDecimals(new NumericField(this, ScResId(ED_DECIMALS))),
    mpFtLeadZeroes(new FixedText(this, ScResId(FT_LEADZEROES))),
    mpEdLeadZeroes(new NumericField(this, ScResId(ED_LEADZEROES))),
    mpBtnNegRed(new CheckBox(this, ScResId(BTN_NEGRED))),
    mpBtnThousand(new CheckBox(this, ScResId(BTN_THOUSAND))),
    maNumFormatControl(SID_NUMBER_TYPE_FORMAT, *pBindings, *this),

    // Caution! SID_NUMBER_FORMAT is reworked in symphony code, may be needed (!) If
    // yes, grep for it in SC and symphony (!)
    maFormatControl(SID_NUMBER_FORMAT, *pBindings, *this),

    mnCategorySelected(0),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings)
{
    Initialize();
    FreeResource();

    Layouter::PrepareForLayouting(*mpFtCategory);
    Layouter::PrepareForLayouting(*mpFtDecimals);
    Layouter::PrepareForLayouting(*mpFtLeadZeroes);
}

//////////////////////////////////////////////////////////////////////////////

NumberFormatPropertyPanel::~NumberFormatPropertyPanel()
{
    // Destroy the toolboxes, then their background windows.
    mpTBCategory.reset();
    mpTBCategoryBackground.reset();
}

//////////////////////////////////////////////////////////////////////////////

void NumberFormatPropertyPanel::Initialize()
{
	Link aLink = LINK(this, NumberFormatPropertyPanel, NumFormatSelectHdl);
	mpLbCategory->SetSelectHdl ( aLink );
	mpLbCategory->SelectEntryPos(0);
	mpLbCategory->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Category")));		//wj acc
    mpLbCategory->SetDropDownLineCount(mpLbCategory->GetEntryCount());

    // Note that we use icons from UNO commands that are not in every case the commands
    // that are really dispatched.  They just look right.
	mpTBCategory->SetItemImage(
        ID_NUMBER,
        GetImage(mxFrame, A2S(".uno:NumericField"), sal_False, Theme::IsHighContrastMode()));
	mpTBCategory->SetItemImage(
        ID_PERCENT,
        GetImage(mxFrame, A2S(".uno:NumberFormatPercent"), sal_False, Theme::IsHighContrastMode()));
	mpTBCategory->SetItemImage(
        ID_CURRENCY,
        GetImage(mxFrame, A2S(".uno:NumberFormatCurrency"), sal_False, Theme::IsHighContrastMode()));
	mpTBCategory->SetItemImage(
        ID_DATE,
        GetImage(mxFrame, A2S(".uno:NumberFormatDate"), sal_False, Theme::IsHighContrastMode()));
	mpTBCategory->SetItemImage(
        ID_TEXT,
        GetImage(mxFrame, A2S(".uno:InsertFixedText"), sal_False, Theme::IsHighContrastMode()));

	Size aTbxSize( mpTBCategory->CalcWindowSizePixel() );
	mpTBCategory->SetOutputSizePixel( aTbxSize );
	mpTBCategory->SetBackground(Wallpaper());
	mpTBCategory->SetPaintTransparent(true);
	aLink = LINK(this, NumberFormatPropertyPanel, NumFormatHdl);
	mpTBCategory->SetSelectHdl ( aLink );

	aLink = LINK(this, NumberFormatPropertyPanel, NumFormatValueHdl);
	
	mpEdDecimals->SetModifyHdl( aLink );	
	mpEdLeadZeroes->SetModifyHdl( aLink );
	mpEdDecimals->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Decimal Places")));		//wj acc
	mpEdLeadZeroes->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Leading Zeroes")));		//wj acc
	mpBtnNegRed->SetClickHdl( aLink );
	mpBtnThousand->SetClickHdl( aLink );

    mpLbCategory->SetAccessibleRelationLabeledBy(mpFtCategory.get());
	mpTBCategory->SetAccessibleRelationLabeledBy(mpTBCategory.get());
	mpEdDecimals->SetAccessibleRelationLabeledBy(mpFtDecimals.get());
	mpEdLeadZeroes->SetAccessibleRelationLabeledBy(mpFtLeadZeroes.get());
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( NumberFormatPropertyPanel, NumFormatHdl, ToolBox*, pBox )
{
	sal_uInt16 nVal = pBox->GetCurItemId();
	sal_uInt16 nId = 0;
	switch(nVal)
	{
	case ID_NUMBER:
		nId = 1;
		break;
	case ID_PERCENT:
		nId = 2;
		break;
	case ID_CURRENCY:
		nId = 3;
		break;
	case ID_DATE:
		nId = 4;
		break;
	case ID_TEXT:
		nId = 9;
		break;
	default:
		;
	}
	if( nId != mnCategorySelected )
	{
		SfxUInt16Item aItem( SID_NUMBER_TYPE_FORMAT,  nId );
		GetBindings()->GetDispatcher()->Execute(SID_NUMBER_TYPE_FORMAT, SFX_CALLMODE_RECORD, &aItem, 0L);
	}
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( NumberFormatPropertyPanel, NumFormatSelectHdl, ListBox*, pBox )
{
	sal_uInt16 nVal = pBox->GetSelectEntryPos();
	if( nVal != mnCategorySelected )
	{
		SfxUInt16Item aItem( SID_NUMBER_TYPE_FORMAT,  nVal );
		GetBindings()->GetDispatcher()->Execute(SID_NUMBER_TYPE_FORMAT, SFX_CALLMODE_RECORD, &aItem, 0L);
		mnCategorySelected = nVal;
	}
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( NumberFormatPropertyPanel, NumFormatValueHdl, void*, EMPTYARG )
{
	String		  aFormat;
	String		  sBreak = String::CreateFromAscii(",");
	bool		  bThousand 	=    mpBtnThousand->IsEnabled()
		&& mpBtnThousand->IsChecked();
	bool		  bNegRed		=    mpBtnNegRed->IsEnabled()
		&& mpBtnNegRed->IsChecked();
	sal_uInt16		  nPrecision	= (mpEdDecimals->IsEnabled())
		? (sal_uInt16)mpEdDecimals->GetValue()
		: (sal_uInt16)0;
	sal_uInt16		  nLeadZeroes	= (mpEdLeadZeroes->IsEnabled())
		? (sal_uInt16)mpEdLeadZeroes->GetValue()
		: (sal_uInt16)0;

	String sThousand = String::CreateFromInt32(bThousand);
	String sNegRed = String::CreateFromInt32(bNegRed);
	String sPrecision = String::CreateFromInt32(nPrecision);
	String sLeadZeroes = String::CreateFromInt32(nLeadZeroes);

	aFormat += sThousand;
	aFormat += sBreak;
	aFormat += sNegRed;
	aFormat += sBreak;
	aFormat += sPrecision;
	aFormat += sBreak;
	aFormat += sLeadZeroes;
	aFormat += sBreak;

	SfxStringItem aItem( SID_NUMBER_FORMAT,  aFormat );
	GetBindings()->GetDispatcher()->Execute(SID_NUMBER_FORMAT, SFX_CALLMODE_RECORD, &aItem, 0L);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////

NumberFormatPropertyPanel* NumberFormatPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to NumberFormatPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to NumberFormatPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to NumberFormatPropertyPanel::Create"), NULL, 2);
    
    return new NumberFormatPropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}

//////////////////////////////////////////////////////////////////////////////

void NumberFormatPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

//////////////////////////////////////////////////////////////////////////////

void NumberFormatPropertyPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext aContext)
{
    if(maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;



    // todo
}

//////////////////////////////////////////////////////////////////////////////

void NumberFormatPropertyPanel::NotifyItemUpdate( 
    sal_uInt16 nSID, 
    SfxItemState eState, 
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    
	switch(nSID)
	{
	case SID_NUMBER_TYPE_FORMAT:
		{
			if( eState >= SFX_ITEM_AVAILABLE)
			{
				const SfxInt16Item* pItem = (const SfxInt16Item*)pState;
				sal_uInt16 nVal = pItem->GetValue();
				mnCategorySelected = nVal;
				mpLbCategory->SelectEntryPos(nVal);
				if( nVal < 4 )
				{
					mpBtnThousand->Enable();
					mpBtnNegRed->Enable();
					mpEdDecimals->Enable();
					mpEdLeadZeroes->Enable();
				}
				else
				{
					mpBtnThousand->Disable();
					mpBtnNegRed->Disable();
					mpEdDecimals->Disable();
					mpEdLeadZeroes->Disable();
				}
			}
			else
			{
				mpLbCategory->SetNoSelection();
				mnCategorySelected = 0;
				mpBtnThousand->Disable();
				mpBtnNegRed->Disable();
				mpEdDecimals->Disable();
				mpEdLeadZeroes->Disable();
			}
		}
		break;
	case SID_NUMBER_FORMAT:
		{
			bool		  bThousand 	=    0;
			bool		  bNegRed		=    0;
			sal_uInt16		  nPrecision	=	 0;
			sal_uInt16		  nLeadZeroes	=    0;
			if( eState >= SFX_ITEM_AVAILABLE)
			{
				const SfxStringItem* pItem = (const SfxStringItem*)pState;
				String aCode = pItem->GetValue();
			/*	if(aCode.Equals(String::CreateFromAscii("General")))
				{
					mnCategorySelected = 0;
					mpLbCategory->SelectEntryPos(0);
					mpBtnThousand->Check(0);
					mpBtnNegRed->Check(0);
					mpEdDecimals->SetValue(0);
					mpEdLeadZeroes->SetValue(1);
					break;
				}
				else if( mpLbCategory->GetSelectEntryPos() == 0 )
				{
					mnCategorySelected = 1;
					mpLbCategory->SelectEntryPos(1);
				}*/
				sal_uInt16 aLen = aCode.Len();
				String* sFormat = new String[4];
				String  sTmpStr = String::CreateFromAscii(""); 
				sal_uInt16 nCount = 0;
				sal_uInt16 nStrCount = 0;
				while( nCount < aLen )
				{
					sal_Unicode cChar = aCode.GetChar(nCount);
					if(cChar == sal_Unicode(','))
					{
						sFormat[nStrCount] = sTmpStr;
						sTmpStr = String::CreateFromAscii("");
						nStrCount++;
					}
					else
					{
						sTmpStr += cChar;
					}
					nCount++;
				}
				bThousand 	=    sFormat[0].ToInt32();
				bNegRed		=    sFormat[1].ToInt32();
				nPrecision	=	 (sal_uInt16)sFormat[2].ToInt32();
				nLeadZeroes	=    (sal_uInt16)sFormat[3].ToInt32();
				delete[] sFormat;
			}
			else
			{
				bThousand 	=    0;
				bNegRed		=    0;
				nPrecision	=	 0;
				nLeadZeroes	=    1;
			}
			mpBtnThousand->Check(bThousand);
			mpBtnNegRed->Check(bNegRed);
			mpEdDecimals->SetValue(nPrecision);
			mpEdLeadZeroes->SetValue(nLeadZeroes);
		}
	default:
		;
	}
}

//////////////////////////////////////////////////////////////////////////////

SfxBindings* NumberFormatPropertyPanel::GetBindings() 
{ 
    return mpBindings; 
}




void NumberFormatPropertyPanel::Resize (void)
{
    const sal_Int32 nRight (GetSizePixel().Width() - Layouter::MapWidth(*this, TB_BORDER));
    Layouter::SetRight(*mpFtCategory, nRight);
    Layouter::SetRight(*mpFtLeadZeroes, nRight);
    Layouter::SetRight(*mpBtnNegRed, nRight);
    Layouter::SetRight(*mpBtnThousand, nRight);
}

}} // end of namespace ::sc::sidebar

//////////////////////////////////////////////////////////////////////////////
// eof
