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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/shl.hxx>

#define _SVX_FONTWORK_CXX
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdview.hxx>
#include <svx/svdocirc.hxx>
#include <svx/xtextit.hxx>

#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"

#include <svx/dialogs.hrc>
#include "fontwork.hrc"
#include <svx/fontwork.hxx>
#include <editeng/outlobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>

SFX_IMPL_DOCKINGWINDOW( SvxFontWorkChildWindow, SID_FONTWORK );

/*************************************************************************
|*
|* ControllerItem fuer Fontwork
|*
\************************************************************************/

SvxFontWorkControllerItem::SvxFontWorkControllerItem
(
    sal_uInt16 _nId,
	SvxFontWorkDialog& rDlg,
	SfxBindings& rBindings
) :

    SfxControllerItem( _nId, rBindings ),

	rFontWorkDlg( rDlg )
{
}

/*************************************************************************
|*
|* StateChanged-Methode fuer FontWork-Items
|*
\************************************************************************/

void SvxFontWorkControllerItem::StateChanged( sal_uInt16 /*nSID*/, SfxItemState /*eState*/,
											  const SfxPoolItem* pItem )
{
	switch ( GetId() )
	{
		case SID_FORMTEXT_STYLE:
		{
			const XFormTextStyleItem* pStateItem =
								PTR_CAST(XFormTextStyleItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStyleItem erwartet");
			rFontWorkDlg.SetStyle_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_ADJUST:
		{
			const XFormTextAdjustItem* pStateItem =
								PTR_CAST(XFormTextAdjustItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextAdjustItem erwartet");
			rFontWorkDlg.SetAdjust_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_DISTANCE:
		{
			const XFormTextDistanceItem* pStateItem =
								PTR_CAST(XFormTextDistanceItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextDistanceItem erwartet");
			rFontWorkDlg.SetDistance_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_START:
		{
			const XFormTextStartItem* pStateItem =
								PTR_CAST(XFormTextStartItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStartItem erwartet");
			rFontWorkDlg.SetStart_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_MIRROR:
		{
			const XFormTextMirrorItem* pStateItem =
								PTR_CAST(XFormTextMirrorItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextMirrorItem erwartet");
			rFontWorkDlg.SetMirror_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_HIDEFORM:
		{
			const XFormTextHideFormItem* pStateItem =
								PTR_CAST(XFormTextHideFormItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextHideFormItem erwartet");
			rFontWorkDlg.SetShowForm_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_OUTLINE:
		{
			const XFormTextOutlineItem* pStateItem =
								PTR_CAST(XFormTextOutlineItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextOutlineItem erwartet");
			rFontWorkDlg.SetOutline_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_SHADOW:
		{
			const XFormTextShadowItem* pStateItem =
								PTR_CAST(XFormTextShadowItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowItem erwartet");
			rFontWorkDlg.SetShadow_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_SHDWCOLOR:
		{
			const XFormTextShadowColorItem* pStateItem =
								PTR_CAST(XFormTextShadowColorItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowColorItem erwartet");
			rFontWorkDlg.SetShadowColor_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_SHDWXVAL:
		{
			const XFormTextShadowXValItem* pStateItem =
								PTR_CAST(XFormTextShadowXValItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowXValItem erwartet");
			rFontWorkDlg.SetShadowXVal_Impl(pStateItem);
			break;
		}
		case SID_FORMTEXT_SHDWYVAL:
		{
			const XFormTextShadowYValItem* pStateItem =
								PTR_CAST(XFormTextShadowYValItem, pItem);
			DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowYValItem erwartet");
			rFontWorkDlg.SetShadowYVal_Impl(pStateItem);
			break;
		}
	}
}

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Fontwork-Dialog
|*
\************************************************************************/

SvxFontWorkChildWindow::SvxFontWorkChildWindow
(
    Window* _pParent,
	sal_uInt16 nId,
	SfxBindings* pBindings,
	SfxChildWinInfo* pInfo
) :

    SfxChildWindow( _pParent, nId )

{
    pWindow = new SvxFontWorkDialog( pBindings, this, _pParent,
									 SVX_RES( RID_SVXDLG_FONTWORK ) );
	SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*) pWindow;

	eChildAlignment = SFX_ALIGN_NOALIGNMENT;

	pDlg->Initialize( pInfo );
}

/*************************************************************************
|*
|* Floating Window zur Attributierung von Texteffekten
|*
\************************************************************************/

SvxFontWorkDialog::SvxFontWorkDialog( SfxBindings *pBindinx,
									  SfxChildWindow *pCW,
                                      Window* _pParent,
									  const ResId& rResId ) :
    SfxDockingWindow( pBindinx, pCW, _pParent, rResId ),

	aTbxStyle		(this, ResId(TBX_STYLE,*rResId.GetResMgr())),
	aTbxAdjust		(this, ResId(TBX_ADJUST,*rResId.GetResMgr())),

	aFbDistance 	(this, ResId(FB_DISTANCE,*rResId.GetResMgr())),
	aMtrFldDistance (this, ResId(MTR_FLD_DISTANCE,*rResId.GetResMgr())),
	aFbTextStart	(this, ResId(FB_TEXTSTART,*rResId.GetResMgr())),
	aMtrFldTextStart(this, ResId(MTR_FLD_TEXTSTART,*rResId.GetResMgr())),

	aTbxShadow		(this, ResId(TBX_SHADOW,*rResId.GetResMgr())),

	aFbShadowX		(this, ResId(FB_SHADOW_X,*rResId.GetResMgr())),
	aMtrFldShadowX	(this, ResId(MTR_FLD_SHADOW_X,*rResId.GetResMgr())),
	aFbShadowY		(this, ResId(FB_SHADOW_Y,*rResId.GetResMgr())),
	aMtrFldShadowY	(this, ResId(MTR_FLD_SHADOW_Y,*rResId.GetResMgr())),

	aShadowColorLB	(this, ResId(CLB_SHADOW_COLOR,*rResId.GetResMgr())),
    rBindings       (*pBindinx),

	nLastStyleTbxId(0),
	nLastAdjustTbxId(0),
	nLastShadowTbxId(0),
	nSaveShadowX	(0),
	nSaveShadowY	(0),
	nSaveShadowAngle(450),
	nSaveShadowSize (100),

    maImageList     (ResId(IL_FONTWORK,*rResId.GetResMgr())),
    maImageListH    (ResId(ILH_FONTWORK,*rResId.GetResMgr())),

    maColorTable()
{
	FreeResource();

	ApplyImageList();

	pCtrlItems[0] = new SvxFontWorkControllerItem(SID_FORMTEXT_STYLE, *this, rBindings);
	pCtrlItems[1] = new SvxFontWorkControllerItem(SID_FORMTEXT_ADJUST, *this, rBindings);
	pCtrlItems[2] = new SvxFontWorkControllerItem(SID_FORMTEXT_DISTANCE, *this, rBindings);
	pCtrlItems[3] = new SvxFontWorkControllerItem(SID_FORMTEXT_START, *this, rBindings);
	pCtrlItems[4] = new SvxFontWorkControllerItem(SID_FORMTEXT_MIRROR, *this, rBindings);
	pCtrlItems[5] = new SvxFontWorkControllerItem(SID_FORMTEXT_HIDEFORM, *this, rBindings);
	pCtrlItems[6] = new SvxFontWorkControllerItem(SID_FORMTEXT_OUTLINE, *this, rBindings);
	pCtrlItems[7] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHADOW, *this, rBindings);
	pCtrlItems[8] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWCOLOR, *this, rBindings);
	pCtrlItems[9] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWXVAL, *this, rBindings);
	pCtrlItems[10] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWYVAL, *this, rBindings);

	Size aSize = aTbxStyle.CalcWindowSizePixel();
	aTbxStyle.SetSizePixel(aSize);
	aTbxStyle.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectStyleHdl_Impl) );

	aTbxAdjust.SetSizePixel(aSize);
	aTbxAdjust.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectAdjustHdl_Impl) );

	aTbxShadow.SetSizePixel(aSize);
	aTbxShadow.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectShadowHdl_Impl) );

	Link aLink = LINK(this, SvxFontWorkDialog, ModifyInputHdl_Impl);
	aMtrFldDistance.SetModifyHdl( aLink );
	aMtrFldTextStart.SetModifyHdl( aLink );
	aMtrFldShadowX.SetModifyHdl( aLink );
	aMtrFldShadowY.SetModifyHdl( aLink );

	// System-Metrik setzen
	const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
	SetFieldUnit( aMtrFldDistance, eDlgUnit, sal_True );
	SetFieldUnit( aMtrFldTextStart, eDlgUnit, sal_True );
	SetFieldUnit( aMtrFldShadowX, eDlgUnit, sal_True );
	SetFieldUnit( aMtrFldShadowY, eDlgUnit, sal_True );
	if( eDlgUnit == FUNIT_MM )
	{
		aMtrFldDistance.SetSpinSize( 50 );
		aMtrFldTextStart.SetSpinSize( 50 );
		aMtrFldShadowX.SetSpinSize( 50 );
		aMtrFldShadowY.SetSpinSize( 50 );
	}
	else
	{
		aMtrFldDistance.SetSpinSize( 10 );
		aMtrFldTextStart.SetSpinSize( 10 );
		aMtrFldShadowX.SetSpinSize( 10 );
		aMtrFldShadowY.SetSpinSize( 10 );
	}

	aShadowColorLB.SetSelectHdl( LINK(this, SvxFontWorkDialog, ColorSelectHdl_Impl) );

	aInputTimer.SetTimeout(500);
	aInputTimer.SetTimeoutHdl(LINK(this, SvxFontWorkDialog, InputTimoutHdl_Impl));
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SvxFontWorkDialog::~SvxFontWorkDialog()
{
	for (sal_uInt16 i = 0; i < CONTROLLER_COUNT; i++)
		DELETEZ(pCtrlItems[i]);
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::Zoom()
{
	SfxDockingWindow::Roll();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SfxChildAlignment SvxFontWorkDialog::CheckAlignment( SfxChildAlignment eActAlign,
													 SfxChildAlignment eAlign )
{
	SfxChildAlignment eAlignment;

	switch ( eAlign )
	{
		case SFX_ALIGN_TOP:
		case SFX_ALIGN_HIGHESTTOP:
		case SFX_ALIGN_LOWESTTOP:
		case SFX_ALIGN_BOTTOM:
		case SFX_ALIGN_LOWESTBOTTOM:
		case SFX_ALIGN_HIGHESTBOTTOM:
		{
			eAlignment = eActAlign;
		}
		break;

		case SFX_ALIGN_LEFT:
		case SFX_ALIGN_RIGHT:
		case SFX_ALIGN_FIRSTLEFT:
		case SFX_ALIGN_LASTLEFT:
		case SFX_ALIGN_FIRSTRIGHT:
		case SFX_ALIGN_LASTRIGHT:
		{
			eAlignment = eAlign;
		}
		break;

		default:
		{
			eAlignment = eAlign;
		}
		break;
	}

	return eAlignment;
}


/*************************************************************************
|*
|* Style-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetStyle_Impl(const XFormTextStyleItem* pItem)
{
    if ( pItem )
	{
		sal_uInt16 nId = TBI_STYLE_OFF;

		switch ( pItem->GetValue() )
		{
			case XFT_ROTATE : nId = TBI_STYLE_ROTATE;	break;
			case XFT_UPRIGHT: nId = TBI_STYLE_UPRIGHT;	break;
			case XFT_SLANTX : nId = TBI_STYLE_SLANTX;	break;
			case XFT_SLANTY : nId = TBI_STYLE_SLANTY;	break;
            default: ;//prevent warning
		}
		aTbxStyle.Enable();

        // Make sure that there is allways exactly one checked toolbox item.
		if ( pItem->GetValue() == XFT_NONE )
		{
			aTbxStyle.CheckItem(TBI_STYLE_ROTATE, sal_False);
			aTbxStyle.CheckItem(TBI_STYLE_UPRIGHT, sal_False);
			aTbxStyle.CheckItem(TBI_STYLE_SLANTX, sal_False);
			aTbxStyle.CheckItem(TBI_STYLE_SLANTY, sal_False);

			aTbxStyle.CheckItem(TBI_STYLE_OFF, sal_True);
		}
		else
        {
			aTbxStyle.CheckItem(TBI_STYLE_OFF, sal_False);
            aTbxStyle.CheckItem(nId);
        }

		nLastStyleTbxId = nId;
	}
	else
		aTbxStyle.Disable();
}

/*************************************************************************
|*
|* Adjust-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetAdjust_Impl(const XFormTextAdjustItem* pItem)
{
	if ( pItem )
	{
		sal_uInt16 nId;

		aTbxAdjust.Enable();
		aMtrFldDistance.Enable();

		if ( pItem->GetValue() == XFT_LEFT || pItem->GetValue() == XFT_RIGHT )
		{
			if ( pItem->GetValue() == XFT_LEFT )	nId = TBI_ADJUST_LEFT;
			else									nId = TBI_ADJUST_RIGHT;
			aMtrFldTextStart.Enable();
		}
		else
		{
			if ( pItem->GetValue() == XFT_CENTER )	nId = TBI_ADJUST_CENTER;
			else									nId = TBI_ADJUST_AUTOSIZE;
			aMtrFldTextStart.Disable();
		}

		if ( !aTbxAdjust.IsItemChecked(nId) )
		{
			aTbxAdjust.CheckItem(nId);
		}
		nLastAdjustTbxId = nId;
	}
	else
	{
		aTbxAdjust.Disable();
		aMtrFldTextStart.Disable();
		aMtrFldDistance.Disable();
	}
}

/*************************************************************************
|*
|* Abstand-Wert in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetDistance_Impl(const XFormTextDistanceItem* pItem)
{
	// #104596# Use HasChildPathFocus() instead of HasFocus() at SpinFields
	if ( pItem && !aMtrFldDistance.HasChildPathFocus() )
	{
		SetMetricValue( aMtrFldDistance, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
	}
}

/*************************************************************************
|*
|* Einzug-Wert in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetStart_Impl(const XFormTextStartItem* pItem)
{
	// #104596# Use HasChildPathFocus() instead of HasFocus() at SpinFields
	if ( pItem && !aMtrFldTextStart.HasChildPathFocus() )
	{
		SetMetricValue( aMtrFldTextStart, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
	}
}

/*************************************************************************
|*
|* Button fuer Umkehrung der Textrichtung setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetMirror_Impl(const XFormTextMirrorItem* pItem)
{
	if ( pItem )
		aTbxAdjust.CheckItem(TBI_ADJUST_MIRROR, pItem->GetValue());
}

/*************************************************************************
|*
|* Button fuer Konturanzeige setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShowForm_Impl(const XFormTextHideFormItem* pItem)
{
	if ( pItem )
		aTbxShadow.CheckItem(TBI_SHOWFORM, !pItem->GetValue());
}

/*************************************************************************
|*
|* Button fuer Zeichenumrandung setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetOutline_Impl(const XFormTextOutlineItem* pItem)
{
	if ( pItem )
		aTbxShadow.CheckItem(TBI_OUTLINE, pItem->GetValue());
}

/*************************************************************************
|*
|* Shadow-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadow_Impl(const XFormTextShadowItem* pItem,
										sal_Bool bRestoreValues)
{
	if ( pItem )
	{
		sal_uInt16 nId;

		aTbxShadow.Enable();

		if ( pItem->GetValue() == XFTSHADOW_NONE )
		{
			nId = TBI_SHADOW_OFF;
			aFbShadowX.Hide();
			aFbShadowY.Hide();
			aMtrFldShadowX.Disable();
			aMtrFldShadowY.Disable();
			aShadowColorLB.Disable();
		}
		else
		{
			aFbShadowX.Show();
			aFbShadowY.Show();
			aMtrFldShadowX.Enable();
			aMtrFldShadowY.Enable();
			aShadowColorLB.Enable();

			if ( pItem->GetValue() == XFTSHADOW_NORMAL )
			{
				nId = TBI_SHADOW_NORMAL;
				const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();

//              aFbShadowX.SetBitmap( Bitmap( ResId(RID_SVXBMP_SHADOW_XDIST, _pMgr ) ) );
				//aMtrFldShadowX.SetUnit(FUNIT_MM);
				aMtrFldShadowX.SetUnit( eDlgUnit );
				aMtrFldShadowX.SetDecimalDigits(2);
                aMtrFldShadowX.SetMin(LONG_MIN);
                aMtrFldShadowX.SetMax(LONG_MAX);
				if( eDlgUnit == FUNIT_MM )
					aMtrFldShadowX.SetSpinSize( 50 );
				else
					aMtrFldShadowX.SetSpinSize( 10 );

//              aFbShadowY.SetBitmap( Bitmap( ResId( RID_SVXBMP_SHADOW_YDIST, _pMgr ) ) );
				//aMtrFldShadowY.SetUnit(FUNIT_MM);
				aMtrFldShadowY.SetUnit( eDlgUnit );
				aMtrFldShadowY.SetDecimalDigits(2);
                aMtrFldShadowY.SetMin(LONG_MIN);
                aMtrFldShadowY.SetMax(LONG_MAX);
				if( eDlgUnit == FUNIT_MM )
					aMtrFldShadowY.SetSpinSize( 50 );
				else
					aMtrFldShadowY.SetSpinSize( 10 );

				if ( bRestoreValues )
				{
					SetMetricValue( aMtrFldShadowX, nSaveShadowX, SFX_MAPUNIT_100TH_MM );
					SetMetricValue( aMtrFldShadowY, nSaveShadowY, SFX_MAPUNIT_100TH_MM );

					XFormTextShadowXValItem aXItem( nSaveShadowX );
					XFormTextShadowYValItem aYItem( nSaveShadowY );

					GetBindings().GetDispatcher()->Execute(
						SID_FORMTEXT_SHDWXVAL, SFX_CALLMODE_RECORD, &aXItem, &aYItem, 0L );
				}
			}
			else
			{
				nId = TBI_SHADOW_SLANT;

//              aFbShadowX.SetBitmap( Bitmap( ResId( RID_SVXBMP_SHADOW_ANGLE, _pMgr ) ) );
				aMtrFldShadowX.SetUnit(FUNIT_CUSTOM);
				aMtrFldShadowX.SetDecimalDigits(1);
                aMtrFldShadowX.SetMin(-1800);
                aMtrFldShadowX.SetMax( 1800);
				aMtrFldShadowX.SetSpinSize(10);

//              aFbShadowY.SetBitmap( Bitmap( ResId( RID_SVXBMP_SHADOW_SIZE, _pMgr ) ) );
				aMtrFldShadowY.SetUnit(FUNIT_PERCENT);
				aMtrFldShadowY.SetDecimalDigits(0);
                aMtrFldShadowY.SetMin(-999);
                aMtrFldShadowY.SetMax( 999);
				aMtrFldShadowY.SetSpinSize(10);

				if ( bRestoreValues )
				{
					aMtrFldShadowX.SetValue(nSaveShadowAngle);
					aMtrFldShadowY.SetValue(nSaveShadowSize);
					XFormTextShadowXValItem aXItem(nSaveShadowAngle);
					XFormTextShadowYValItem aYItem(nSaveShadowSize);
					GetBindings().GetDispatcher()->Execute(
						SID_FORMTEXT_SHDWXVAL, SFX_CALLMODE_RECORD, &aXItem, &aYItem, 0L );
				}
			}
		}

		if ( !aTbxShadow.IsItemChecked(nId) )
		{
			aTbxShadow.CheckItem(nId);
		}
		nLastShadowTbxId = nId;

		ApplyImageList();
	}
	else
	{
		aTbxShadow.Disable();
		aMtrFldShadowX.Disable();
		aMtrFldShadowY.Disable();
		aShadowColorLB.Disable();
	}
}

/*************************************************************************
|*
|* Schattenfarbe in Listbox eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowColor_Impl(const XFormTextShadowColorItem* pItem)
{
	if ( pItem )
        aShadowColorLB.SelectEntry(pItem->GetColorValue());
}

/*************************************************************************
|*
|* X-Wert fuer Schatten in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowXVal_Impl(const XFormTextShadowXValItem* pItem)
{
	// #104596# Use HasChildPathFocus() instead of HasFocus() at SpinFields
	if ( pItem && !aMtrFldShadowX.HasChildPathFocus() )
	{
		// #i19251#
		// sal_Int32 nValue = pItem->GetValue();
		
		// #i19251#
		// The two involved fields/items are used double and contain/give different
		// values regarding to the access method. Thus, here we need to separate the access
		// methos regarding to the kind of value accessed.
		if(aTbxShadow.IsItemChecked(TBI_SHADOW_SLANT))
		{
			// #i19251#
			// There is no value correction necessary at all, i think this
			// was only tried to be done without understanding that the two
			// involved fields/items are used double and contain/give different
			// values regarding to the access method.
			// nValue = nValue - ( int( float( nValue ) / 360.0 ) * 360 );
			aMtrFldShadowX.SetValue(pItem->GetValue());
		}
		else
		{
			SetMetricValue( aMtrFldShadowX, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
		}
	}
}

/*************************************************************************
|*
|* Y-Wert fuer Schatten in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowYVal_Impl(const XFormTextShadowYValItem* pItem)
{
	// #104596# Use HasChildPathFocus() instead of HasFocus() at SpinFields
	if ( pItem && !aMtrFldShadowY.HasChildPathFocus() )
	{
		// #i19251#
		// The two involved fields/items are used double and contain/give different
		// values regarding to the access method. Thus, here we need to separate the access
		// methos regarding to the kind of value accessed.
		if(aTbxShadow.IsItemChecked(TBI_SHADOW_SLANT))
		{
			aMtrFldShadowY.SetValue(pItem->GetValue());
		}
		else
		{
			SetMetricValue( aMtrFldShadowY, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
		}
	}
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectStyleHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nId = aTbxStyle.GetCurItemId();

    // Execute this block when a different toolbox item has been clicked or
    // when the off item has been clicked.  The later is necessary to
    // override the toolbox behaviour of unchecking the item after second
    // click on it: One of the items has to be checked at all times (when
    // enabled that is.)
	if (nId == TBI_STYLE_OFF || nId != nLastStyleTbxId )
	{
		XFormTextStyle eStyle = XFT_NONE;

		switch ( nId )
		{
			case TBI_STYLE_ROTATE	: eStyle = XFT_ROTATE;	break;
			case TBI_STYLE_UPRIGHT	: eStyle = XFT_UPRIGHT; break;
			case TBI_STYLE_SLANTX	: eStyle = XFT_SLANTX;	break;
			case TBI_STYLE_SLANTY	: eStyle = XFT_SLANTY;	break;
		}
		XFormTextStyleItem aItem( eStyle );
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_STYLE, SFX_CALLMODE_RECORD, &aItem, 0L );
		SetStyle_Impl( &aItem );
		nLastStyleTbxId = nId;
	}
	return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectAdjustHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nId = aTbxAdjust.GetCurItemId();

	if ( nId == TBI_ADJUST_MIRROR )
	{
		XFormTextMirrorItem aItem(aTbxAdjust.IsItemChecked(nId));
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_MIRROR, SFX_CALLMODE_SLOT, &aItem, 0L );
	}
	else if ( nId != nLastAdjustTbxId )
	{
		XFormTextAdjust eAdjust = XFT_AUTOSIZE;

		switch ( nId )
		{
			case TBI_ADJUST_LEFT	: eAdjust = XFT_LEFT;	break;
			case TBI_ADJUST_CENTER	: eAdjust = XFT_CENTER; break;
			case TBI_ADJUST_RIGHT	: eAdjust = XFT_RIGHT;	break;
		}
		XFormTextAdjustItem aItem(eAdjust);
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_ADJUST, SFX_CALLMODE_RECORD, &aItem, 0L );
		SetAdjust_Impl(&aItem);
		nLastAdjustTbxId = nId;
	}
	return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectShadowHdl_Impl, void *, EMPTYARG )
{
	sal_uInt16 nId = aTbxShadow.GetCurItemId();

	if ( nId == TBI_SHOWFORM )
	{
		XFormTextHideFormItem aItem(!aTbxShadow.IsItemChecked(nId));
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_HIDEFORM, SFX_CALLMODE_RECORD, &aItem, 0L );
	}
	else if ( nId == TBI_OUTLINE )
	{
		XFormTextOutlineItem aItem(aTbxShadow.IsItemChecked(nId));
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_OUTLINE, SFX_CALLMODE_RECORD, &aItem, 0L );
	}
	else if ( nId != nLastShadowTbxId )
	{
		XFormTextShadow eShadow = XFTSHADOW_NONE;

		if ( nLastShadowTbxId == TBI_SHADOW_NORMAL )
		{
			nSaveShadowX = GetCoreValue( aMtrFldShadowX, SFX_MAPUNIT_100TH_MM );
			nSaveShadowY = GetCoreValue( aMtrFldShadowY, SFX_MAPUNIT_100TH_MM );
		}
		else if ( nLastShadowTbxId == TBI_SHADOW_SLANT )
		{
			nSaveShadowAngle = static_cast<long>(aMtrFldShadowX.GetValue());
			nSaveShadowSize  = static_cast<long>(aMtrFldShadowY.GetValue());
		}
		nLastShadowTbxId = nId;

		if ( nId == TBI_SHADOW_NORMAL ) 	eShadow = XFTSHADOW_NORMAL;
		else if ( nId == TBI_SHADOW_SLANT ) eShadow = XFTSHADOW_SLANT;

		XFormTextShadowItem aItem(eShadow);
		GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHADOW, SFX_CALLMODE_RECORD, &aItem, 0L );
		SetShadow_Impl(&aItem, sal_True);
	}
	return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SvxFontWorkDialog, ModifyInputHdl_Impl, void *, EMPTYARG )
{
	aInputTimer.Start();
	return 0;
}
IMPL_LINK_INLINE_END( SvxFontWorkDialog, ModifyInputHdl_Impl, void *, EMPTYARG )

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, InputTimoutHdl_Impl, void *, EMPTYARG )
{
	// System-Metrik evtl. neu setzen
	// Dieses sollte mal als Listener passieren, ist aber aus
	// inkompatibilitaetsgruenden z.Z. nicht moeglich
	const FieldUnit eDlgUnit = rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
	if( eDlgUnit != aMtrFldDistance.GetUnit() )
	{
		SetFieldUnit( aMtrFldDistance, eDlgUnit, sal_True );
		SetFieldUnit( aMtrFldTextStart, eDlgUnit, sal_True );
		aMtrFldDistance.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
		aMtrFldTextStart.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
	}
	if( eDlgUnit != aMtrFldShadowX.GetUnit() &&
		aTbxShadow.IsItemChecked( TBI_SHADOW_NORMAL ) )
	{
		SetFieldUnit( aMtrFldShadowX, eDlgUnit, sal_True );
		SetFieldUnit( aMtrFldShadowY, eDlgUnit, sal_True );
		aMtrFldShadowX.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
		aMtrFldShadowY.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
	}

	long nValue = GetCoreValue( aMtrFldDistance, SFX_MAPUNIT_100TH_MM );
	XFormTextDistanceItem aDistItem( nValue );
	nValue = GetCoreValue( aMtrFldTextStart, SFX_MAPUNIT_100TH_MM );
	XFormTextStartItem aStartItem( nValue );

	sal_Int32 nValueX(0L);
	sal_Int32 nValueY(0L);

	// #i19251#
	// The two involved fields/items are used double and contain/give different
	// values regarding to the access method. Thus, here we need to separate the access
	// methos regarding to the kind of value accessed.
	if(nLastShadowTbxId == TBI_SHADOW_NORMAL)
	{
		nValueX = GetCoreValue( aMtrFldShadowX, SFX_MAPUNIT_100TH_MM );
		nValueY = GetCoreValue( aMtrFldShadowY, SFX_MAPUNIT_100TH_MM );
	}
	else if(nLastShadowTbxId == TBI_SHADOW_SLANT)
	{
		nValueX = static_cast<long>(aMtrFldShadowX.GetValue());
		nValueY = static_cast<long>(aMtrFldShadowY.GetValue());
	}

	XFormTextShadowXValItem aShadowXItem( nValueX );
	XFormTextShadowYValItem aShadowYItem( nValueY );

	// Slot-ID ist egal, die Exec-Methode wertet das gesamte ItemSet aus
	GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_DISTANCE, SFX_CALLMODE_RECORD, &aDistItem,
											&aStartItem, &aShadowXItem, &aShadowYItem, 0L );
	return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, ColorSelectHdl_Impl, void *, EMPTYARG )
{
// Changed by obo. Linux-Compiler can't parse commented lines
	XFormTextShadowColorItem aItem( (const String &) String(),
								    (const Color &) aShadowColorLB.GetSelectEntryColor() );
//	XFormTextShadowColorItem aItem( String(),
//								    aShadowColorLB.GetSelectEntryColor() );
	GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHDWCOLOR, SFX_CALLMODE_RECORD, &aItem, 0L );
	return 0;
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::SetColorTable(XColorListSharedPtr aTable)
{
	if ( aTable != maColorTable )
	{
		maColorTable = aTable;
		aShadowColorLB.Clear();
		aShadowColorLB.Fill(maColorTable);
	}
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::SetActive(sal_Bool /*bActivate*/)
{
}

void SvxFontWorkDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    SfxDockingWindow::DataChanged( rDCEvt );
}
/* -----------------------------08.05.2002 14:28------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontWorkDialog::ApplyImageList()
{
	bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    ImageList& rImgLst = bHighContrast ? maImageListH : maImageList;

	aTbxStyle.SetImageList( rImgLst );
	aTbxAdjust.SetImageList( rImgLst );
	aTbxShadow.SetImageList( rImgLst );

	switch( nLastShadowTbxId )
	{
	case TBI_SHADOW_SLANT:
		aFbShadowX.SetImage( rImgLst.GetImage( TBI_SHADOW_ANGLE ) );
		aFbShadowY.SetImage( rImgLst.GetImage( TBI_SHADOW_SIZE ) );
		break;
//	case TBI_SHADOW_NORMAL:
	default:
		aFbShadowX.SetImage( rImgLst.GetImage( TBI_SHADOW_XDIST ) );
		aFbShadowY.SetImage( rImgLst.GetImage( TBI_SHADOW_YDIST ) );
		break;
	}

	aFbDistance.SetImage( rImgLst.GetImage( TBI_DISTANCE ) );
	aFbTextStart.SetImage( rImgLst.GetImage( TBI_TEXTSTART ) );

}
