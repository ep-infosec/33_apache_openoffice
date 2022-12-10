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

#include <cmdid.h>
#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svl/srchitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgcoitm.hxx>
#include <svx/sdggaitm.hxx>
#include <svx/sdgtritm.hxx>
#include <svx/sdginitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <editeng/brshitem.hxx>
#include <svx/grfflt.hxx>
#include <svx/tbxcolor.hxx>
#include <fmturl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <frmatr.hxx>
#include <swundo.hxx>
#include <uitool.hxx>
#include <docsh.hxx>
#include <grfsh.hxx>
#include <frmmgr.hxx>
#include <frmdlg.hxx>
#include <frmfmt.hxx>
#include <grfatr.hxx>
#include <usrpref.hxx>
#include <edtwin.hxx>
#include <swwait.hxx>
#include <shells.hrc>
#include <popup.hrc>
#include <doc.hxx>
#include <docsh.hxx>
#include <svx/drawitem.hxx>
#define SwGrfShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include "swabstdlg.hxx"
#include <drawdoc.hxx>

#define TOOLBOX_NAME    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "colorbar" ) )

SFX_IMPL_INTERFACE(SwGrfShell, SwBaseShell, SW_RES(STR_SHELLNAME_GRAPHIC))
{
	SFX_POPUPMENU_REGISTRATION(SW_RES(MN_GRF_POPUPMENU));
	SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_GRAFIK_TOOLBOX));
}

void SwGrfShell::Execute(SfxRequest &rReq)
{
	SwWrtShell &rSh = GetShell();

	sal_uInt16 nSlot = rReq.GetSlot();
	switch(nSlot)
	{
		case SID_TWAIN_TRANSFER:
		{
            GetView().ExecuteScan( rReq );
			break;
		}
        case FN_SAVE_GRAPHIC:
        {
            const Graphic *pGraphic;
            if(0 != (pGraphic = rSh.GetGraphic()))
            {
			    String sGrfNm, sFilterNm;
			    rSh.GetGrfNms( &sGrfNm, &sFilterNm );
                ExportGraphic( *pGraphic, sGrfNm );
            }
        }
        break;
        case SID_INSERT_GRAPHIC:
        {
            // #123922# implement slot independent from the two below to
            // bring up the insert graphic dialog and associated actions
            SwView& rView = GetView();
            rReq.SetReturnValue(SfxBoolItem(nSlot, rView.InsertGraphicDlg( rReq )));
            break;
        }
		case FN_FORMAT_GRAFIC_DLG:
		case FN_DRAW_WRAP_DLG:
		{
			SwFlyFrmAttrMgr aMgr( sal_False, &rSh, rSh.IsFrmSelected() ?
											   FRMMGR_TYPE_NONE : FRMMGR_TYPE_GRF);
			const SwViewOption* pVOpt = rSh.GetViewOptions();
			SwViewOption aUsrPref( *pVOpt );

            SfxItemSet aSet(GetPool(), //UUUU sorted by indices

                RES_FRMATR_BEGIN,RES_FRMATR_END - 1,                            // [   82
                RES_GRFATR_MIRRORGRF,RES_GRFATR_CROPGRF,                        // [  123

                //UUUU FillAttribute support
                XATTR_FILL_FIRST,       XATTR_FILL_LAST,                        // [ 1014

                SID_DOCFRAME,SID_DOCFRAME,                                      // [ 5598
                SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,                    // [10023
                SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,                          // [10051
                SID_ATTR_GRAF_KEEP_ZOOM,SID_ATTR_GRAF_KEEP_ZOOM,                // [10882
                SID_ATTR_GRAF_FRMSIZE,SID_ATTR_GRAF_GRAPHIC,                    // [10884, contains SID_ATTR_GRAF_FRMSIZE_PERCENT

                //UUUU items to hand over XPropertyList things like
                // XColorList, XHatchList, XGradientList and XBitmapList
                // to the Area TabPage
                SID_COLOR_TABLE,        SID_BITMAP_LIST,                        // [10179

                SID_HTML_MODE,SID_HTML_MODE,                                    // [10414
                FN_GET_PRINT_AREA,FN_GET_PRINT_AREA,                            // [21032
                FN_PARAM_GRF_CONNECT,FN_PARAM_GRF_CONNECT,                      // [21153
                FN_PARAM_GRF_DIALOG,FN_PARAM_GRF_DIALOG,                        // [21171
                FN_SET_FRM_NAME,FN_KEEP_ASPECT_RATIO,                           // [21306
                FN_SET_FRM_ALT_NAME,FN_SET_FRM_ALT_NAME,                        // [21318
                0);

            //UUUU create needed items for XPropertyList entries from the DrawModel so that
            // the Area TabPage can access them
            const SwDrawModel* pDrawModel = rSh.GetView().GetDocShell()->GetDoc()->GetDrawModel();

            aSet.Put(SvxColorTableItem(pDrawModel->GetColorTableFromSdrModel(), SID_COLOR_TABLE));
            aSet.Put(SvxGradientListItem(pDrawModel->GetGradientListFromSdrModel(), SID_GRADIENT_LIST));
            aSet.Put(SvxHatchListItem(pDrawModel->GetHatchListFromSdrModel(), SID_HATCH_LIST));
            aSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapListFromSdrModel(), SID_BITMAP_LIST));

			sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
			aSet.Put(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
			FieldUnit eMetric = ::GetDfltMetric((0 != (nHtmlMode&HTMLMODE_ON)));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );

			const SwRect* pRect = &rSh.GetAnyCurRect(RECT_PAGE);
			SwFmtFrmSize aFrmSize( ATT_VAR_SIZE, pRect->Width(), pRect->Height());
			aFrmSize.SetWhich( GetPool().GetWhich( SID_ATTR_PAGE_SIZE ) );
			aSet.Put( aFrmSize );

			aSet.Put(SfxStringItem(FN_SET_FRM_NAME, rSh.GetFlyName()));
            if ( nSlot == FN_FORMAT_GRAFIC_DLG )
            {
                aSet.Put( SfxStringItem( FN_SET_FRM_ALT_NAME, rSh.GetObjTitle() ) );
            }

			pRect = &rSh.GetAnyCurRect(RECT_PAGE_PRT);
			aFrmSize.SetWidth( pRect->Width() );
			aFrmSize.SetHeight( pRect->Height() );
			aFrmSize.SetWhich( GetPool().GetWhich(FN_GET_PRINT_AREA) );
			aSet.Put( aFrmSize );

			aSet.Put( aMgr.GetAttrSet() );
			aSet.SetParent( aMgr.GetAttrSet().GetParent() );

			// Bei %-Werten Groesse initialisieren
			SwFmtFrmSize aSizeCopy = (const SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE);
			if (aSizeCopy.GetWidthPercent() && aSizeCopy.GetWidthPercent() != 0xff)
				aSizeCopy.SetWidth(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Width());
			if (aSizeCopy.GetHeightPercent() && aSizeCopy.GetHeightPercent() != 0xff)
				aSizeCopy.SetHeight(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Height());

			// and now set the size for "external" tabpages
			{
				SvxSizeItem aSzItm( SID_ATTR_GRAF_FRMSIZE, aSizeCopy.GetSize() );
				aSet.Put( aSzItm );

				Size aSz( aSizeCopy.GetWidthPercent(), aSizeCopy.GetHeightPercent() );
				if( 0xff == aSz.Width() ) 	aSz.Width() = 0;
				if( 0xff == aSz.Height() )  aSz.Height() = 0;

				aSzItm.SetSize( aSz );
				aSzItm.SetWhich( SID_ATTR_GRAF_FRMSIZE_PERCENT );
				aSet.Put( aSzItm );
			}

			String sGrfNm, sFilterNm;
			rSh.GetGrfNms( &sGrfNm, &sFilterNm );
			if( sGrfNm.Len() )
			{
				aSet.Put( SvxBrushItem( INetURLObject::decode( sGrfNm,
										INET_HEX_ESCAPE,
						   				INetURLObject::DECODE_UNAMBIGUOUS,
										RTL_TEXTENCODING_UTF8 ),
										sFilterNm, GPOS_LT,
										SID_ATTR_GRAF_GRAPHIC ));
			}
			else
			{
                // --> OD 2005-02-09 #119353# - robust
                const GraphicObject* pGrfObj = rSh.GetGraphicObj();
                if ( pGrfObj )
                {
                    aSet.Put( SvxBrushItem( *pGrfObj, GPOS_LT,
                                            SID_ATTR_GRAF_GRAPHIC ) );
                }
                // <--
			}
            aSet.Put( SfxBoolItem( FN_PARAM_GRF_CONNECT, sGrfNm.Len() > 0 ) );

			// get Mirror and Crop
			{
				SfxItemSet aTmpSet( rSh.GetAttrPool(),
								RES_GRFATR_MIRRORGRF, RES_GRFATR_CROPGRF );

                rSh.GetCurAttr( aTmpSet );
				aSet.Put( aTmpSet );
			}

			aSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, aUsrPref.IsKeepRatio()));
			aSet.Put(SfxBoolItem( SID_ATTR_GRAF_KEEP_ZOOM, aUsrPref.IsGrfKeepZoom()));

			aSet.Put(SfxFrameItem( SID_DOCFRAME, &GetView().GetViewFrame()->GetTopFrame()));

			SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateFrmTabDialog( DLG_FRM_GRF,
													GetView().GetViewFrame(),
													GetView().GetWindow(),
													aSet, sal_False, DLG_FRM_GRF);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");

            if (nSlot == FN_DRAW_WRAP_DLG)
                pDlg->SetCurPageId(TP_FRM_WRAP);
            
			if( pDlg->Execute() )
			{
				rSh.StartAllAction();
				rSh.StartUndo(UNDO_START);
				const SfxPoolItem* pItem;
				SfxItemSet* pSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                rReq.Done(*pSet);
				// change the 2 frmsize SizeItems to the correct SwFrmSizeItem
				if( SFX_ITEM_SET == pSet->GetItemState(
								SID_ATTR_GRAF_FRMSIZE, sal_False, &pItem ))
				{
					SwFmtFrmSize aSize;
					const Size& rSz = ((SvxSizeItem*)pItem)->GetSize();
					aSize.SetWidth( rSz.Width() );
					aSize.SetHeight( rSz.Height() );

					if( SFX_ITEM_SET == pSet->GetItemState(
							SID_ATTR_GRAF_FRMSIZE_PERCENT, sal_False, &pItem ))
					{
                        const Size& rRelativeSize = ((SvxSizeItem*)pItem)->GetSize();
                        aSize.SetWidthPercent( static_cast< sal_uInt8 >( rRelativeSize.Width() ) );
                        aSize.SetHeightPercent( static_cast< sal_uInt8 >( rRelativeSize.Height() ) );
					}
					pSet->Put( aSize );
				}

				// Vorlagen-AutoUpdate
				SwFrmFmt* pFmt = rSh.GetCurFrmFmt();
				if(pFmt && pFmt->IsAutoUpdateFmt())
				{
                    pFmt->SetFmtAttr(*pSet);
					SfxItemSet aShellSet(GetPool(), RES_FRM_SIZE,	RES_FRM_SIZE,
													RES_SURROUND, 	RES_SURROUND,
													RES_ANCHOR,		RES_ANCHOR,
													RES_VERT_ORIENT,RES_HORI_ORIENT,
													0);
					aShellSet.Put(*pSet);
					aMgr.SetAttrSet(aShellSet);
				}
				else
				{
					aMgr.SetAttrSet(*pSet);
				}
				aMgr.UpdateFlyFrm();

				sal_Bool bApplyUsrPref = sal_False;
				if (SFX_ITEM_SET == pSet->GetItemState(
					FN_KEEP_ASPECT_RATIO, sal_True, &pItem ))
				{
					aUsrPref.SetKeepRatio(
									((const SfxBoolItem*)pItem)->GetValue() );
					bApplyUsrPref = sal_True;
				}
				if( SFX_ITEM_SET == pSet->GetItemState(
					SID_ATTR_GRAF_KEEP_ZOOM, sal_True, &pItem ))
				{
					aUsrPref.SetGrfKeepZoom(
									((const SfxBoolItem*)pItem)->GetValue() );
					bApplyUsrPref = sal_True;
				}

				if( bApplyUsrPref )
					SW_MOD()->ApplyUsrPref(aUsrPref, &GetView());

				// and now set all the graphic attributes and other stuff
				if( SFX_ITEM_SET == pSet->GetItemState(
										SID_ATTR_GRAF_GRAPHIC, sal_True, &pItem ))
				{
					if( ((SvxBrushItem*)pItem)->GetGraphicLink() )
						sGrfNm = *((SvxBrushItem*)pItem)->GetGraphicLink();
					else
						sGrfNm.Erase();

					if( ((SvxBrushItem*)pItem)->GetGraphicFilter() )
						sFilterNm = *((SvxBrushItem*)pItem)->GetGraphicFilter();
					else
						sFilterNm.Erase();

					if( sGrfNm.Len() )
					{
                        SwDocShell* pDocSh = GetView().GetDocShell();
                        SwWait aWait( *pDocSh, true );
                        SfxMedium* pMedium = pDocSh->GetMedium();
                        INetURLObject aAbs;
                        if( pMedium )
                            aAbs = pMedium->GetURLObject();
                        rSh.ReRead( URIHelper::SmartRel2Abs(
                                        aAbs, sGrfNm,
                                        URIHelper::GetMaybeFileHdl() ),
								 	sFilterNm, 0 );
					}
				}
                if ( SFX_ITEM_SET == pSet->GetItemState(
										FN_SET_FRM_ALT_NAME, sal_True, &pItem ))
                {
                    // --> OD 2009-07-13 #i73249#
//                    rSh.SetAlternateText(
//                                ((const SfxStringItem*)pItem)->GetValue() );
                    rSh.SetObjTitle( ((const SfxStringItem*)pItem)->GetValue() );
                    // <--
                }

				SfxItemSet aGrfSet( rSh.GetAttrPool(), RES_GRFATR_BEGIN,
													   RES_GRFATR_END-1 );
				aGrfSet.Put( *pSet );
				if( aGrfSet.Count() )
					rSh.SetAttrSet( aGrfSet );

				rSh.EndUndo(UNDO_END);
				rSh.EndAllAction();
			}
			delete pDlg;
		}
		break;

		case FN_GRAPHIC_MIRROR_ON_EVEN_PAGES:
		{
			SfxItemSet aSet(rSh.GetAttrPool(), RES_GRFATR_MIRRORGRF, RES_GRFATR_MIRRORGRF);
            rSh.GetCurAttr( aSet );
			SwMirrorGrf aGrf((const SwMirrorGrf &)aSet.Get(RES_GRFATR_MIRRORGRF));
			aGrf.SetGrfToggle(!aGrf.IsGrfToggle());
			rSh.SetAttrItem(aGrf);
		}
		break;

		default:
			ASSERT(sal_False, "wrong dispatcher");
			return;
	}
}


void SwGrfShell::ExecAttr( SfxRequest &rReq )
{
	sal_uInt16 nGrfType;
	if( CNT_GRF == GetShell().GetCntType() &&
		( GRAPHIC_BITMAP == ( nGrfType = GetShell().GetGraphicType()) ||
		  GRAPHIC_GDIMETAFILE == nGrfType ))
	{
		SfxItemSet aGrfSet( GetShell().GetAttrPool(), RES_GRFATR_BEGIN,
													  RES_GRFATR_END -1 );
    	const SfxItemSet *pArgs = rReq.GetArgs();
    	const SfxPoolItem* pItem;
    	sal_uInt16 nSlot = rReq.GetSlot();
	    if( !pArgs || SFX_ITEM_SET != pArgs->GetItemState( nSlot, sal_False, &pItem ))
			pItem = 0;

        switch( nSlot )
        {
            case SID_FLIP_VERTICAL:
            case SID_FLIP_HORIZONTAL:
            {
                GetShell().GetCurAttr( aGrfSet );
                SwMirrorGrf aMirror( (SwMirrorGrf&)aGrfSet.Get( RES_GRFATR_MIRRORGRF ) );
                sal_uInt16 nMirror = aMirror.GetValue();
                if ( nSlot==SID_FLIP_HORIZONTAL )
                    switch( nMirror )
                    {
                    case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_VERT;
                                                break;
                    case RES_MIRROR_GRAPH_HOR:  nMirror = RES_MIRROR_GRAPH_BOTH;
                                                break;
                    case RES_MIRROR_GRAPH_VERT:   nMirror = RES_MIRROR_GRAPH_DONT;
                                                break;
                    case RES_MIRROR_GRAPH_BOTH: nMirror = RES_MIRROR_GRAPH_HOR;
                                                break;
                    }
                else
                    switch( nMirror )
                    {
                    case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_HOR;
                                                break;
                    case RES_MIRROR_GRAPH_VERT: nMirror = RES_MIRROR_GRAPH_BOTH;
                                                break;
                    case RES_MIRROR_GRAPH_HOR:    nMirror = RES_MIRROR_GRAPH_DONT;
                                                break;
                    case RES_MIRROR_GRAPH_BOTH: nMirror = RES_MIRROR_GRAPH_VERT;
                                                break;
                    }
                aMirror.SetValue( nMirror );
                aGrfSet.ClearItem();
                aGrfSet.Put( aMirror );
            }
            break;

		case SID_ATTR_GRAF_LUMINANCE:
			if( pItem )
				aGrfSet.Put( SwLuminanceGrf(
							((SfxInt16Item*)pItem)->GetValue() ));
			break;
		case SID_ATTR_GRAF_CONTRAST:
			if( pItem )
				aGrfSet.Put( SwContrastGrf(
							((SfxInt16Item*)pItem)->GetValue() ));
			break;
		case SID_ATTR_GRAF_RED:
			if( pItem )
				aGrfSet.Put( SwChannelRGrf(
							((SfxInt16Item*)pItem)->GetValue() ));
			break;
		case SID_ATTR_GRAF_GREEN:
			if( pItem )
				aGrfSet.Put( SwChannelGGrf(
							((SfxInt16Item*)pItem)->GetValue() ));
			break;
		case SID_ATTR_GRAF_BLUE:
			if( pItem )
				aGrfSet.Put( SwChannelBGrf(
							((SfxInt16Item*)pItem)->GetValue() ));
			break;
		case SID_ATTR_GRAF_GAMMA:
			if( pItem )
			{
				double fVal = ((SfxUInt32Item*)pItem)->GetValue();
				aGrfSet.Put( SwGammaGrf(fVal/100. ));
			}
			break;
		case SID_ATTR_GRAF_TRANSPARENCE:
			if( pItem )
                aGrfSet.Put( SwTransparencyGrf(
                    static_cast< sal_Int8 >( ( (SfxUInt16Item*)pItem )->GetValue() ) ) );
			break;
		case SID_ATTR_GRAF_INVERT:
			if( pItem )
				aGrfSet.Put( SwInvertGrf(
							((SfxBoolItem*)pItem)->GetValue() ));
			break;

		case SID_ATTR_GRAF_MODE:
			if( pItem )
				aGrfSet.Put( SwDrawModeGrf(
							((SfxUInt16Item*)pItem)->GetValue() ));
			break;

        case SID_COLOR_SETTINGS:
        {
            svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
            aToolboxAccess.toggleToolbox();
            break;
        }
        case SID_GRFFILTER:
        case SID_GRFFILTER_INVERT:
		case SID_GRFFILTER_SMOOTH:
		case SID_GRFFILTER_SHARPEN:
		case SID_GRFFILTER_REMOVENOISE:
		case SID_GRFFILTER_SOBEL:
		case SID_GRFFILTER_MOSAIC:
		case SID_GRFFILTER_EMBOSS:
		case SID_GRFFILTER_POSTER:
		case SID_GRFFILTER_POPART:
		case SID_GRFFILTER_SEPIA:
		case SID_GRFFILTER_SOLARIZE:
			if( GRAPHIC_BITMAP == nGrfType )
			{
                // --> OD 2005-02-09 #119353# - robust
                const GraphicObject* pFilterObj( GetShell().GetGraphicObj() );
                if ( pFilterObj )
                {
                    GraphicObject aFilterObj( *pFilterObj );
                    if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                        SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ))
                        GetShell().ReRead( aEmptyStr, aEmptyStr,
                                           &aFilterObj.GetGraphic() );
                }
                // <--
			}
			break;

		default:
			ASSERT(sal_False, "wrong dispatcher");
		}
		if( aGrfSet.Count() )
			GetShell().SetAttrSet( aGrfSet );
	}
	GetView().GetViewFrame()->GetBindings().Invalidate(rReq.GetSlot());
}

void SwGrfShell::GetAttrState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxItemSet aCoreSet( GetPool(), aNoTxtNodeSetRange );
    rSh.GetCurAttr( aCoreSet );
    sal_Bool bParentCntProt = 0 != rSh.IsSelObjProtected( FLYPROTECT_CONTENT|FLYPROTECT_PARENT );
    sal_Bool bIsGrfCntnt = CNT_GRF == GetShell().GetCntType();

    SetGetStateSet( &rSet );

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        sal_Bool bDisable = bParentCntProt;
        switch( nWhich )
        {
        case SID_INSERT_GRAPHIC:
        case FN_FORMAT_GRAFIC_DLG:
        case SID_TWAIN_TRANSFER:
            if( bParentCntProt || !bIsGrfCntnt )
                bDisable = sal_True;
            else if ( nWhich == SID_INSERT_GRAPHIC
                      && rSh.CrsrInsideInputFld() )
            {
                bDisable = sal_True;
            }
            break;

        case FN_SAVE_GRAPHIC:
            if( rSh.GetGraphicType() == GRAPHIC_NONE )
                bDisable = sal_True;
            break;

        case SID_COLOR_SETTINGS:
        {
            if ( bParentCntProt || !bIsGrfCntnt )
                bDisable = sal_True;
            else
            {
                svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
                rSet.Put( SfxBoolItem( nWhich, aToolboxAccess.isToolboxVisible() ) );
            }
            break;
        }

        case SID_FLIP_HORIZONTAL:
            if( !bParentCntProt )
            {
                MirrorGraph nState = static_cast< MirrorGraph >(((const SwMirrorGrf &) aCoreSet.Get(
                                        RES_GRFATR_MIRRORGRF )).GetValue());

                rSet.Put(SfxBoolItem( nWhich, nState == RES_MIRROR_GRAPH_VERT ||
                                              nState == RES_MIRROR_GRAPH_BOTH));
            }
            break;

        case SID_FLIP_VERTICAL:
            if( !bParentCntProt )
            {
                MirrorGraph nState = static_cast< MirrorGraph >(((const SwMirrorGrf &) aCoreSet.Get(
                                        RES_GRFATR_MIRRORGRF )).GetValue());

                rSet.Put(SfxBoolItem( nWhich, nState == RES_MIRROR_GRAPH_HOR ||
                                              nState == RES_MIRROR_GRAPH_BOTH));
            }
            break;


		case SID_ATTR_GRAF_LUMINANCE:
			if( !bParentCntProt )
				rSet.Put( SfxInt16Item( nWhich, ((SwLuminanceGrf&)
						aCoreSet.Get(RES_GRFATR_LUMINANCE)).GetValue() ));
			break;
		case SID_ATTR_GRAF_CONTRAST:
			if( !bParentCntProt )
				rSet.Put( SfxInt16Item( nWhich, ((SwContrastGrf&)
						aCoreSet.Get(RES_GRFATR_CONTRAST)).GetValue() ));
			break;
		case SID_ATTR_GRAF_RED:
			if( !bParentCntProt )
				rSet.Put( SfxInt16Item( nWhich, ((SwChannelRGrf&)
						aCoreSet.Get(RES_GRFATR_CHANNELR)).GetValue() ));
			break;
		case SID_ATTR_GRAF_GREEN:
			if( !bParentCntProt )
				rSet.Put( SfxInt16Item( nWhich, ((SwChannelGGrf&)
						aCoreSet.Get(RES_GRFATR_CHANNELG)).GetValue() ));
			break;
		case SID_ATTR_GRAF_BLUE:
			if( !bParentCntProt )
				rSet.Put( SfxInt16Item( nWhich, ((SwChannelBGrf&)
						aCoreSet.Get(RES_GRFATR_CHANNELB)).GetValue() ));
			break;

		case SID_ATTR_GRAF_GAMMA:
			if( !bParentCntProt )
                rSet.Put( SfxUInt32Item( nWhich, static_cast< sal_uInt32 >(
                    ( (SwGammaGrf&)aCoreSet.Get( RES_GRFATR_GAMMA ) ).GetValue() * 100 ) ) );
			break;
		case SID_ATTR_GRAF_TRANSPARENCE:
			if( !bParentCntProt )
			{
                // --> OD 2005-02-09 #119353# - robust
                const GraphicObject* pGrafObj = rSh.GetGraphicObj();
                if ( pGrafObj )
                {
                    if( pGrafObj->IsAnimated() ||
                        GRAPHIC_GDIMETAFILE == pGrafObj->GetType() )
                        bDisable = sal_True;
                    else
                        rSet.Put( SfxUInt16Item( nWhich, ((SwTransparencyGrf&)
                            aCoreSet.Get(RES_GRFATR_TRANSPARENCY)).GetValue() ));
                }
			}
			break;
		case SID_ATTR_GRAF_INVERT:
			if( !bParentCntProt )
				rSet.Put( SfxBoolItem( nWhich, ((SwInvertGrf&)
						aCoreSet.Get(RES_GRFATR_INVERT)).GetValue() ));
			break;

		case SID_ATTR_GRAF_MODE:
			if( !bParentCntProt )
                rSet.Put( SfxUInt16Item( nWhich, ((SwDrawModeGrf&)
						aCoreSet.Get(RES_GRFATR_DRAWMODE)).GetValue() ));
			break;

        case SID_GRFFILTER:
        case SID_GRFFILTER_INVERT:
        case SID_GRFFILTER_SMOOTH:
        case SID_GRFFILTER_SHARPEN:
        case SID_GRFFILTER_REMOVENOISE:
        case SID_GRFFILTER_SOBEL:
        case SID_GRFFILTER_MOSAIC:
        case SID_GRFFILTER_EMBOSS:
        case SID_GRFFILTER_POSTER:
        case SID_GRFFILTER_POPART:
        case SID_GRFFILTER_SEPIA:
        case SID_GRFFILTER_SOLARIZE:
            {
                if( bParentCntProt || !bIsGrfCntnt )
                    bDisable = sal_True;
                else
                {
                    const sal_uInt16 eGraphicType( rSh.GetGraphicType() );
                    if ( ( eGraphicType == GRAPHIC_NONE ||
                           eGraphicType == GRAPHIC_DEFAULT ) &&
                         rSh.IsGrfSwapOut( sal_True ) )
                    {
                        rSet.DisableItem( nWhich );
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(sal_False);  // start the loading
                    }
                    else
                    {
                        bDisable = eGraphicType != GRAPHIC_BITMAP;
                    }
                }
                // <--
            }
            break;

        default:
            bDisable = sal_False;
        }

        if( bDisable )
            rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
    SetGetStateSet( 0 );
}


SwGrfShell::SwGrfShell(SwView &_rView) :
    SwBaseShell(_rView)

{
	SetName(String::CreateFromAscii("Graphic"));
	SetHelpId(SW_GRFSHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Graphic));
}
