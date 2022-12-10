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

// INCLUDE ---------------------------------------------------------------
#include <svl/rectitem.hxx>






#include <svx/dialogs.hrc>

#include <svx/ruler.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/tstpitem.hxx>
#include "editeng/protitem.hxx"
#include "rlrcitem.hxx"
#include "svx/rulritem.hxx"
#include <svl/eitem.hxx>

// class SvxRulerItem ----------------------------------------------------

SvxRulerItem::SvxRulerItem(sal_uInt16 _nId, SvxRuler &rRul, SfxBindings &rBindings)
: SfxControllerItem(_nId, rBindings),
  rRuler(rRul)
{
}

// -----------------------------------------------------------------------

void SvxRulerItem::StateChanged( sal_uInt16 nSID, SfxItemState eState,
								 const SfxPoolItem* pState)
{
	// SFX_ITEM_DONTCARE => pState == -1 => PTR_CAST buff
	if ( eState != SFX_ITEM_AVAILABLE )
		pState = 0;

	switch(nSID)
	{
		// Linker / rechter Seitenrand
		case SID_RULER_LR_MIN_MAX:
		{
			const SfxRectangleItem *pItem = PTR_CAST(SfxRectangleItem, pState);
			rRuler.UpdateFrameMinMax(pItem);
			break;
		}
        case SID_ATTR_LONG_LRSPACE:
		{
			const SvxLongLRSpaceItem *pItem = PTR_CAST(SvxLongLRSpaceItem, pState);
			DBG_ASSERT(pState? 0 != pItem: sal_True, "SvxLRSpaceItem erwartet");
			rRuler.UpdateFrame(pItem);
			break;
		}
        case SID_ATTR_LONG_ULSPACE:
		{
			const SvxLongULSpaceItem *pItem = PTR_CAST(SvxLongULSpaceItem, pState);
			DBG_ASSERT(pState? 0 != pItem: sal_True, "SvxULSpaceItem erwartet");
			rRuler.UpdateFrame(pItem);
			break;
		}
        case SID_ATTR_TABSTOP_VERTICAL:
        case SID_ATTR_TABSTOP:
		{
			const SvxTabStopItem *pItem = PTR_CAST(SvxTabStopItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxTabStopItem erwartet");
			rRuler.Update(pItem);
			break;
		}
        case SID_ATTR_PARA_LRSPACE_VERTICAL:
        case SID_ATTR_PARA_LRSPACE:
		{
			const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxLRSpaceItem erwartet");
			rRuler.UpdatePara(pItem);
			break;
		}
        case SID_RULER_BORDERS_VERTICAL:
        case SID_RULER_BORDERS:
		case SID_RULER_ROWS:
		case SID_RULER_ROWS_VERTICAL:
		{
			const SvxColumnItem *pItem = PTR_CAST(SvxColumnItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxColumnItem erwartet");
#ifdef DBG_UTIL
			if(pItem)
			{
				if(pItem->IsConsistent())
                    rRuler.Update(pItem, nSID);
				else
					DBG_ERROR("Spaltenitem corrupted");
			}
			else
                rRuler.Update(pItem, nSID);
#else
            rRuler.Update(pItem, nSID);
#endif
			break;
		}
		case SID_RULER_PAGE_POS:
		{	// Position Seite, Seitenbreite
			const SvxPagePosSizeItem *pItem = PTR_CAST(SvxPagePosSizeItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxPagePosSizeItem erwartet");
			rRuler.Update(pItem);
			break;
		}
		case SID_RULER_OBJECT:
		{	// Object-Selektion
			const SvxObjectItem *pItem = PTR_CAST(SvxObjectItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxObjectItem erwartet");
			rRuler.Update(pItem);
			break;
		}
		case SID_RULER_PROTECT:
		{
			const SvxProtectItem *pItem = PTR_CAST(SvxProtectItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxProtectItem erwartet");
			rRuler.Update(pItem);
			break;
		}
        case SID_RULER_BORDER_DISTANCE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
			DBG_ASSERT(pState?  0 != pItem: sal_True, "SvxLRSpaceItem erwartet");
            rRuler.UpdateParaBorder(pItem);
        }
        break;
        case SID_RULER_TEXT_RIGHT_TO_LEFT :
        {
            const SfxBoolItem *pItem = PTR_CAST(SfxBoolItem, pState);
            DBG_ASSERT(pState?  0 != pItem: sal_True, "SfxBoolItem erwartet");
            rRuler.UpdateTextRTL(pItem);
        }
        break;
	}
}


