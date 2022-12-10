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

#include "LineWidthControl.hxx"
#include "LinePropertyPanel.hrc"
#include "LinePropertyPanel.hxx"

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <unotools/viewoptions.hxx>
#include <svx/xlnwtit.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "svx/sidebar/PopupContainer.hxx"


namespace svx { namespace sidebar {

LineWidthControl::LineWidthControl (
    Window* pParent,
    LinePropertyPanel& rPanel)
    : svx::sidebar::PopupControl(pParent,SVX_RES(RID_POPUPPANEL_LINEPAGE_WIDTH)),
      mrLinePropertyPanel(rPanel),
      mpBindings(NULL),
      maVSWidth( this, SVX_RES(VS_WIDTH)),
      maFTCus( this, SVX_RES(FT_CUSTOME)),
      maFTWidth( this, SVX_RES(FT_LINE_WIDTH)),
      maMFWidth( this, SVX_RES(MF_WIDTH)),
      meMapUnit(SFX_MAPUNIT_TWIP),
      rStr(NULL),
      mstrPT(SVX_RES(STR_PT)),
      mnCustomWidth(0),
      mbCustom(false),
      mbCloseByEdit(false),
      mnTmpCustomWidth(0),
      mbVSFocus(true),
      maIMGCus(SVX_RES(IMG_WIDTH_CUSTOM)),
      maIMGCusGray(SVX_RES(IMG_WIDTH_CUSTOM_GRAY))
{
	Initialize();
	FreeResource();
	mpBindings = mrLinePropertyPanel.GetBindings();
}




LineWidthControl::~LineWidthControl (void)
{
	delete[] rStr;
}




void LineWidthControl::Paint(const Rectangle& rect)
{
	svx::sidebar::PopupControl::Paint(rect);
    
	Color aOldLineColor = GetLineColor();
	Color aOldFillColor = GetFillColor();

	Point aPos( LogicToPixel( Point( CUSTOM_X, CUSTOM_Y), MAP_APPFONT ));
	Size aSize( LogicToPixel( Size(  CUSTOM_W, CUSTOM_H ), MAP_APPFONT ));
	Rectangle aRect( aPos, aSize );
	aRect.Left() -= 1;
	aRect.Top() -= 1;
	aRect.Right() += 1;
	aRect.Bottom() += 1;

	Color aLineColor(189,201,219);
	if(!GetSettings().GetStyleSettings().GetHighContrastMode())
		SetLineColor(aLineColor);
	else
		SetLineColor(GetSettings().GetStyleSettings().GetShadowColor());
	SetFillColor(COL_TRANSPARENT);
	DrawRect(aRect);

	SetLineColor(aOldLineColor);
	SetFillColor(aOldFillColor);
}




void LineWidthControl::Initialize()
{
	maVSWidth.SetStyle( maVSWidth.GetStyle()| WB_3DLOOK |  WB_NO_DIRECTSELECT  );// WB_NAMEFIELD | WB_ITEMBORDER |WB_DOUBLEBORDER | WB_NONEFIELD |
	//for high contract wj
	if(GetSettings().GetStyleSettings().GetHighContrastMode())
	{
		maVSWidth.SetColor(GetSettings().GetStyleSettings().GetMenuColor());
	//	maBorder.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
		maFTWidth.SetBackground(GetSettings().GetStyleSettings().GetMenuColor());
	}
	else
	{
		maVSWidth.SetColor(COL_WHITE);
	//	maBorder.SetBackground(Wallpaper(COL_WHITE));
		maFTWidth.SetBackground(Wallpaper(COL_WHITE));
	}

	sal_Int64 nFirst= maMFWidth.Denormalize( maMFWidth.GetFirst( FUNIT_TWIP ) );
	sal_Int64 nLast = maMFWidth.Denormalize( maMFWidth.GetLast( FUNIT_TWIP ) );
	sal_Int64 nMin = maMFWidth.Denormalize( maMFWidth.GetMin( FUNIT_TWIP ) );
	sal_Int64 nMax = maMFWidth.Denormalize( maMFWidth.GetMax( FUNIT_TWIP ) );
	maMFWidth.SetSpinSize( 10 );
	maMFWidth.SetUnit( FUNIT_POINT );
    if( maMFWidth.GetDecimalDigits() > 1 )
		 maMFWidth.SetDecimalDigits( 1 );
	maMFWidth.SetFirst( maMFWidth.Normalize( nFirst ), FUNIT_TWIP );
	maMFWidth.SetLast( maMFWidth.Normalize( nLast ), FUNIT_TWIP );
	maMFWidth.SetMin( maMFWidth.Normalize( nMin ), FUNIT_TWIP );
	maMFWidth.SetMax( maMFWidth.Normalize( nMax ), FUNIT_TWIP );	

	rStr = new XubString[9];
	//modify,
	rStr[0] = String("05", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[1] = String("08", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[2] = String("10", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[3] = String("15", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[4] = String("23", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[5] = String("30", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[6] = String("45", 2, RTL_TEXTENCODING_ASCII_US);
	rStr[7] = String("60", 2, RTL_TEXTENCODING_ASCII_US);	
	rStr[8] = String( SVX_RES(STR_WIDTH_LAST_CUSTOM) );

	LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
	const sal_Unicode cSep = aLocaleWrapper.getNumDecimalSep().GetChar(0);

	for(int i = 0; i <= 7 ; i++)
	{
		rStr[i] = rStr[i].Insert(cSep, 1);//Modify
		rStr[i].Append(mstrPT);
	}
	//end

	for(sal_uInt16 i = 1 ; i <= 9 ; i++)
	{
		maVSWidth.InsertItem(i);
		maVSWidth.SetItemText(i, rStr[i-1]);
	}
	maVSWidth.SetUnit(rStr);
	maVSWidth.SetItemData(1,(void*)5);
	maVSWidth.SetItemData(2,(void*)8);
	maVSWidth.SetItemData(3,(void*)10);
	maVSWidth.SetItemData(4,(void*)15);
	maVSWidth.SetItemData(5,(void*)23);
	maVSWidth.SetItemData(6,(void*)30);
	maVSWidth.SetItemData(7,(void*)45);
	maVSWidth.SetItemData(8,(void*)60);
	maVSWidth.SetImage(maIMGCusGray);

	maVSWidth.SetSelItem(0);
	Link aLink =  LINK( this, LineWidthControl, VSSelectHdl ) ;
    maVSWidth.SetSelectHdl(aLink);
	aLink = LINK(this, LineWidthControl, MFModifyHdl);
	maMFWidth.SetModifyHdl(aLink);

	maVSWidth.StartSelection();
	maVSWidth.Show();
}




void LineWidthControl::GetFocus()
{
	if(!mbVSFocus)
		maMFWidth.GrabFocus();
	else
		maVSWidth.GrabFocus();
}




ValueSet& LineWidthControl::GetValueSet()
{
	return maVSWidth;
}




void LineWidthControl::SetWidthSelect( long lValue, bool bValuable, SfxMapUnit eMapUnit)
{
	mbVSFocus = true;
	maVSWidth.SetSelItem(0);
	mbCloseByEdit = false;
	meMapUnit = eMapUnit;
	SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_LINE_WIDTH_GLOBAL_VALUE );
    if ( aWinOpt.Exists() )
	{
		::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();
		::rtl::OUString aTmp;
		if ( aSeq.getLength())
			aSeq[0].Value >>= aTmp;

		String aWinData( aTmp );
		mnCustomWidth = aWinData.ToInt32();
		mbCustom = true;
		maVSWidth.SetImage(maIMGCus);
		maVSWidth.SetCusEnable(true);

		String aStrTip( String::CreateFromDouble( (double)mnCustomWidth / 10));
		aStrTip.Append(mstrPT);		//		
		maVSWidth.SetItemText(9, aStrTip);
	}
	else
	{
		mbCustom = false;
		maVSWidth.SetImage(maIMGCusGray);
		maVSWidth.SetCusEnable(false);
		//modify 
		//String aStrTip(String(SVX_RES(STR_WIDTH_LAST_CUSTOM)));	
		//maVSWidth.SetItemText(9, aStrTip);
		maVSWidth.SetItemText(9, rStr[8]);
	}
	
	if(bValuable)
    {
        sal_Int64 nVal = OutputDevice::LogicToLogic( lValue, (MapUnit)eMapUnit, MAP_100TH_MM );
        nVal = maMFWidth.Normalize( nVal );
        maMFWidth.SetValue( nVal, FUNIT_100TH_MM );
    }
	else 
    {
		maMFWidth.SetText( String() );
    }

	MapUnit eOrgUnit = (MapUnit)eMapUnit;
	MapUnit ePntUnit( MAP_TWIP );		
	lValue = LogicToLogic( lValue , eOrgUnit, ePntUnit );

	XubString strCurrValue = maMFWidth.GetText();
	sal_uInt16 i = 0;
	for(; i < 8; i++)
		if(strCurrValue == rStr[i])
		{
			maVSWidth.SetSelItem(i+1);
			break;
		}
	if (i>=8)
	{
		mbVSFocus = false;
		maVSWidth.SetSelItem(0);
	}
	maVSWidth.Format();
	maVSWidth.StartSelection();
}




IMPL_LINK(LineWidthControl, VSSelectHdl, void *, pControl)
{
	if(pControl == &maVSWidth)  
	{		
		sal_uInt16 iPos = maVSWidth.GetSelectItemId();
		if(iPos >= 1 && iPos <= 8)
		{
			long nVal = LogicToLogic((long)(unsigned long)maVSWidth.GetItemData( iPos ) , MAP_POINT, (MapUnit)meMapUnit);
			nVal = maMFWidth.Denormalize(nVal);
			XLineWidthItem aWidthItem( nVal );
			mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);
			mrLinePropertyPanel.SetWidthIcon(iPos);
			mrLinePropertyPanel.SetWidth(nVal);
			mbCloseByEdit = false;
			mnTmpCustomWidth = 0;
		}
		else if(iPos == 9)
		{//last custom
			//modified 
			if(mbCustom)
			{
				long nVal = LogicToLogic(mnCustomWidth , MAP_POINT, (MapUnit)meMapUnit);
				nVal = maMFWidth.Denormalize(nVal);
				XLineWidthItem aWidthItem( nVal );
				mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);
				mrLinePropertyPanel.SetWidth(nVal);
				mbCloseByEdit = false;
				mnTmpCustomWidth = 0;
			}
			else
			{
				maVSWidth.SetNoSelection();		//add , set no selection and keep the last select item 
				maVSWidth.Format();
				Invalidate();
				maVSWidth.StartSelection();
			}
			//modify end
		}
		if((iPos >= 1 && iPos <= 8) || (iPos == 9 && mbCustom)) //add 
			mrLinePropertyPanel.EndLineWidthPopupMode();
	}
	return( 0L );
}




IMPL_LINK(LineWidthControl, MFModifyHdl, void *, pControl)
{
	if(pControl == &maMFWidth)  
	{
		if(maVSWidth.GetSelItem())
		{
			maVSWidth.SetSelItem(0);
			maVSWidth.Format();
			Invalidate();
			maVSWidth.StartSelection();
		}
		long nTmp = static_cast<long>(maMFWidth.GetValue());
		long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)meMapUnit );
		sal_Int32 nNewWidth = (short)maMFWidth.Denormalize( nVal );
		XLineWidthItem aWidthItem(nNewWidth);
		mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aWidthItem, 0L);
		
		mbCloseByEdit = true;
		mnTmpCustomWidth = nTmp;
		/*for(sal_uInt16 i = 0; i < 8; i++)
		{
			if(nTmp == (sal_Int32)maVSWidth.GetItemData(i))
			{
				mbCloseByEdit = false;
				break;
			}
		}*/

	}
	return( 0L );
}




bool LineWidthControl::IsCloseByEdit()
{
	return mbCloseByEdit;
}




long LineWidthControl::GetTmpCustomWidth()
{
	return mnTmpCustomWidth;
}



} } // end of namespace svx::sidebar

// eof
