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
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <com/sun/star/util/XCloseable.hpp>

#include <doc.hxx>
#	include "writerhelper.hxx"
#   include <msfilter.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <algorithm>                //std::swap
#include <functional>               //std::binary_function
#   include <svl/itemiter.hxx>  //SfxItemIter
#   include <svx/svdobj.hxx>        //SdrObject
#   include <svx/svdoole2.hxx>      //SdrOle2Obj
#   include <svx/fmglob.hxx>        //FmFormInventor
#   include <editeng/brkitem.hxx>       //SvxFmtBreakItem
#   include <editeng/tstpitem.hxx>      //SvxTabStopItem
#   include <ndtxt.hxx>             //SwTxtNode
#    include <ndnotxt.hxx>          //SwNoTxtNode
#    include <fmtcntnt.hxx>         //SwFmtCntnt
#    include <swtable.hxx>          //SwTable
#    include <frmfmt.hxx>           //SwFrmFmt
#    include <flypos.hxx>           //SwPosFlyFrms
#    include <fmtanchr.hxx>         //SwFmtAnchor
#    include <ndgrf.hxx>            //SwGrfNode
#    include <fmtfsize.hxx>         //SwFmtFrmSize
#   include <SwStyleNameMapper.hxx> //SwStyleNameMapper
#   include <docary.hxx>            //SwCharFmts
#   include <charfmt.hxx>           //SwCharFmt
#   include <fchrfmt.hxx>           //SwFmtCharFmt
#ifndef _UNOTOOLS_STREAMWRAP_HXX
#   include <unotools/streamwrap.hxx>
#endif
#include <numrule.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>//For i120928
#endif
#ifdef DEBUGDUMP
#       include <vcl/svapp.hxx>
#   ifndef _TOOLS_URLOBJ_HXX
#       include <tools/urlobj.hxx>
#   endif
#   ifndef _UNOTOOLS_UCBSTREAMHELPER_HXX
#       include <unotools/ucbstreamhelper.hxx>
#   endif
#       include <unotools/localfilehelper.hxx>
#endif

using namespace com::sun::star;
using namespace nsSwGetPoolIdFromName;


namespace
{
    /*
     Stroustroup forgets copy_if, See C++ Programming language Chp 18, pg 530
    */
    template <typename In , typename Out , typename Pred>
        Out my_copy_if(In first, In last, Out res, Pred p)
    {
        while (first != last)
        {
            if (p(*first))
                *res = *first;
            ++first;
        }
        return res;
    }

    // --> OD 2009-02-04 #i98791# - adjust sorting
    //Utility to sort SwTxtFmtColl's by their assigned outline style list level
    class outlinecmp : public
        std::binary_function<const SwTxtFmtColl*, const SwTxtFmtColl*, bool>
    {
    public:
        bool operator()(const SwTxtFmtColl *pA, const SwTxtFmtColl *pB) const
        {
            // --> OD 2009-02-04 #i98791#
//            return pA->GetAttrOutlineLevel() < pB->GetAttrOutlineLevel();   //<-end,zhaojianwei
            bool bResult( false );
            const bool bIsAAssignedToOutlineStyle( pA->IsAssignedToListLevelOfOutlineStyle() );
            const bool bIsBAssignedToOutlineStyle( pB->IsAssignedToListLevelOfOutlineStyle() );
            if ( bIsAAssignedToOutlineStyle != bIsBAssignedToOutlineStyle )
            {
                bResult = bIsBAssignedToOutlineStyle;
            }
            else if ( !bIsAAssignedToOutlineStyle )
            {
                // pA and pB are equal regarding the sorting criteria.
                // Thus return value does not matter.
                bResult = false;
            }
            else
            {
                bResult = pA->GetAssignedOutlineStyleLevel() < pB->GetAssignedOutlineStyleLevel();
            }

            return bResult;
            // <--
       }
    };
    // <--

    bool IsValidSlotWhich(sal_uInt16 nSlotId, sal_uInt16 nWhichId)
    {
        return (nSlotId != 0 && nWhichId != 0 && nSlotId != nWhichId);
    }

    /*
     Utility to convert a SwPosFlyFrms into a simple vector of sw::Frames

     The crucial thing is that a sw::Frame always has an anchor which
     points to some content in the document. This is a requirement of exporting
     to Word
    */
    sw::Frames SwPosFlyFrmsToFrames(const SwPosFlyFrms &rFlys)
    {
        sw::Frames aRet;

        for(SwPosFlyFrms::const_iterator aIter(rFlys.begin()); aIter != rFlys.end(); aIter++)
        {
            const SwFrmFmt &rEntry = (*aIter)->GetFmt();

            if (const SwPosition* pAnchor = rEntry.GetAnchor().GetCntntAnchor())
            {
                aRet.push_back(sw::Frame(rEntry, *pAnchor));
            }
            else
            {
                SwPosition aPos((*aIter)->GetNdIndex());
                
                if (SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode())
                {
                    aPos.nContent.Assign(pTxtNd, 0);
                }
                
                aRet.push_back(sw::Frame(rEntry, aPos));
            }
        }
        return aRet;
    }

    //Utility to test if a frame is anchored at a given node index
    class anchoredto: public std::unary_function<const sw::Frame&, bool>
    {
    private:
        sal_uLong mnNode;
    public:
        anchoredto(sal_uLong nNode) : mnNode(nNode) {}
        bool operator()(const sw::Frame &rFrame) const
        {
            return (mnNode == rFrame.GetPosition().nNode.GetNode().GetIndex());
        }
    };
}

namespace sw
{
	//For i120928,size conversion before exporting graphic of bullet
	Frame::Frame(const Graphic&rGrf, const SwPosition &rPos)
		:mpFlyFrm(NULL),
		maPos(rPos),
		maSize(),
		maLayoutSize(),
		meWriterType(eBulletGrf),
		mpStartFrameContent(0),
		mbIsInline(true),
		mbForBullet(true),
		maGrf(rGrf)
	{
		const MapMode aMap100mm( MAP_100TH_MM );
		Size    aSize( rGrf.GetPrefSize() );    
		if ( MAP_PIXEL == rGrf.GetPrefMapMode().GetMapUnit() )    
		{        
			aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMap100mm );   
		}    
		else    
		{        
			aSize = OutputDevice::LogicToLogic( aSize,rGrf.GetPrefMapMode(), aMap100mm );   
		} 
		maSize = aSize;
		maLayoutSize = maSize;
	}
	
    Frame::Frame(const SwFrmFmt &rFmt, const SwPosition &rPos)
        : mpFlyFrm(&rFmt),
          maPos(rPos),
          maSize(),
          // --> OD 2007-04-19 #i43447#
          maLayoutSize(),
          // <--
          meWriterType(eTxtBox),
          mpStartFrameContent(0),
          // --> OD 2007-04-19 #i43447# - move to initialization list
          mbIsInline( (rFmt.GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
          // <--
          //For i120928,handle graphic of bullet within existing implementation
          ,mbForBullet(false)
          ,maGrf()
    {
        switch (rFmt.Which())
        {
            case RES_FLYFRMFMT:
                if (const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx())
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    const SwNode &rNd = aIdx.GetNode();
                    using sw::util::GetSwappedInSize;
                    // --> OD 2007-04-19 #i43447# - determine layout size
                    {
                        SwRect aLayRect( rFmt.FindLayoutRect() );
                        Rectangle aRect( aLayRect.SVRect() );
                        // The Object is not rendered (e.g. something in unused
                        // header/footer) - thus, get the values from the format.
                        if ( aLayRect.IsEmpty() )
                        {
                            aRect.SetSize( rFmt.GetFrmSize().GetSize() );
                        }
                        maLayoutSize = aRect.GetSize();
                    }
                    // <--
                    switch (rNd.GetNodeType())
                    {
                        case ND_GRFNODE:
                            meWriterType = eGraphic;
                            maSize = GetSwappedInSize(*rNd.GetNoTxtNode());
                            break;
                        case ND_OLENODE:
                            meWriterType = eOle;
                            maSize = GetSwappedInSize(*rNd.GetNoTxtNode());
                            break;
                        default:
                            meWriterType = eTxtBox;
                            // --> OD 2007-04-19 #i43447#
                            // Size equals layout size for text boxes
                            maSize = maLayoutSize;
                            // <--
                            break;
                    }
                    mpStartFrameContent = &rNd;
                }
                else
                {
                    ASSERT(sal_False, "Impossible");
                    meWriterType = eTxtBox;
                }
                break;
            default:
                if (const SdrObject* pObj = rFmt.FindRealSdrObject())
                {
                    if (pObj->GetObjInventor() == FmFormInventor)
                        meWriterType = eFormControl;
                    else
                        meWriterType = eDrawing;
                    maSize = pObj->GetSnapRect().GetSize();
                }
                else
                {
                    ASSERT(sal_False, "Impossible");
                    meWriterType = eDrawing;
                }
                break;
        }
    }

    bool Frame::IsInline() const
    {
        return mbIsInline;
    }

    void Frame::ForceTreatAsInline()
    {
        mbIsInline = true;
    }

    namespace hack
    {

        sal_uInt16 TransformWhichBetweenPools(const SfxItemPool &rDestPool,
            const SfxItemPool &rSrcPool, sal_uInt16 nWhich)
        {
            sal_uInt16 nSlotId = rSrcPool.GetSlotId(nWhich);
            if (IsValidSlotWhich(nSlotId, nWhich))
                nWhich = rDestPool.GetWhich(nSlotId);
            else
                nWhich = 0;
            return nWhich;
        }

        sal_uInt16 GetSetWhichFromSwDocWhich(const SfxItemSet &rSet,
            const SwDoc &rDoc, sal_uInt16 nWhich)
        {
            if (RES_WHICHHINT_END < *(rSet.GetRanges()))
            {
                nWhich = TransformWhichBetweenPools(*rSet.GetPool(),
                    rDoc.GetAttrPool(), nWhich);
            }
            return nWhich;
        }

        DrawingOLEAdaptor::DrawingOLEAdaptor(SdrOle2Obj &rObj,
            SfxObjectShell &rPers)
            : msOrigPersistName(rObj.GetPersistName()),
            mxIPRef(rObj.GetObjRef()), mrPers(rPers),
            mpGraphic( rObj.GetGraphic() )
        {
            //rObj.SetPersistName(String());
            //rObj.SetObjRef(0);
            rObj.AbandonObject();
        }

        bool DrawingOLEAdaptor::TransferToDoc( ::rtl::OUString &rName )
        {
            ASSERT(mxIPRef.is(), "Transferring invalid object to doc");
            if (!mxIPRef.is())
                return false;

            uno::Reference < container::XChild > xChild( mxIPRef, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( mrPers.GetModel() );

            bool bSuccess = mrPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( mxIPRef, rName );
            if (bSuccess)
            {
                if ( mpGraphic )
                    ::svt::EmbeddedObjectRef::SetGraphicToContainer( *mpGraphic,
                                                                    mrPers.GetEmbeddedObjectContainer(),
                                                                    rName,
                                                                    ::rtl::OUString() );

                //mxIPRef->changeState( embed::EmbedStates::LOADED );
                mxIPRef = 0;
            }

            return bSuccess;
        }

        DrawingOLEAdaptor::~DrawingOLEAdaptor()
        {
            if (mxIPRef.is())
            {
                DBG_ASSERT( !mrPers.GetEmbeddedObjectContainer().HasEmbeddedObject( mxIPRef ), "Object in adaptor is inserted?!" );
                try
                {
                    uno::Reference < com::sun::star::util::XCloseable > xClose( mxIPRef, uno::UNO_QUERY );
                    if ( xClose.is() )
                        xClose->close(sal_True);
                }
                catch ( com::sun::star::util::CloseVetoException& )
                {
                }

                mxIPRef = 0;
            }
        }

#ifdef DEBUGDUMP
        SvStream *CreateDebuggingStream(const String &rSuffix)
        {
            SvStream* pDbgOut = 0;
            static sal_Int32 nCount;
            String aFileName(String(RTL_CONSTASCII_STRINGPARAM("wwdbg")));
            aFileName.Append(String::CreateFromInt32(++nCount));
            aFileName.Append(rSuffix);
            String aURLStr;
            if (::utl::LocalFileHelper::ConvertPhysicalNameToURL(
                Application::GetAppFileName(), aURLStr))
            {
                INetURLObject aURL(aURLStr);
                aURL.removeSegment();
                aURL.removeFinalSlash();
                aURL.Append(aFileName);

                pDbgOut = ::utl::UcbStreamHelper::CreateStream(
                    aURL.GetMainURL(INetURLObject::NO_DECODE),
                    STREAM_TRUNC | STREAM_WRITE);
            }
            return pDbgOut;
        }

        void DumpStream(const SvStream &rSrc, SvStream &rDest, sal_uInt32 nLen)
        {
            SvStream &rSource = const_cast<SvStream&>(rSrc);
            sal_uLong nOrigPos = rSource.Tell();
            if (nLen == STREAM_SEEK_TO_END)
            {
                rSource.Seek(STREAM_SEEK_TO_END);
                nLen = rSource.Tell();
            }
            if (nLen - nOrigPos)
            {
                rSource.Seek(nOrigPos);
                sal_Char* pDat = new sal_Char[nLen];
                rSource.Read(pDat, nLen);
                rDest.Write(pDat, nLen);
                delete[] pDat;
                rSource.Seek(nOrigPos);
            }
        }
#endif
    }

    namespace util
    {
        SV_IMPL_OP_PTRARR_SORT(AuthorInfos, AuthorInfo_Ptr)

        SwTwips MakeSafePositioningValue(SwTwips nIn)
        {
            if (nIn > SHRT_MAX)
                nIn = SHRT_MAX;
            else if (nIn < SHRT_MIN)
                nIn = SHRT_MIN;
            return nIn;
        }

        void SetLayer::SendObjectToHell(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHell);
        }

        void SetLayer::SendObjectToHeaven(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHeaven);
        }

        void SetLayer::SetObjectLayer(SdrObject &rObject, Layer eLayer) const
        {
            if (FmFormInventor == rObject.GetObjInventor())
                rObject.SetLayer(mnFormLayer);
            else
            {
                switch (eLayer)
                {
                    case eHeaven:
                        rObject.SetLayer(mnHeavenLayer);
                        break;
                    case eHell:
                        rObject.SetLayer(mnHellLayer);
                        break;
                }
            }
        }

        //SetLayer boilerplate begin
        void SetLayer::Swap(SetLayer& rOther) throw()
        {
            std::swap(mnHeavenLayer, rOther.mnHeavenLayer);
            std::swap(mnHellLayer, rOther.mnHellLayer);
            std::swap(mnFormLayer, rOther.mnFormLayer);
        }

        // --> OD 2004-12-13 #i38889# - by default put objects into the invisible
        // layers.
        SetLayer::SetLayer(const SwDoc &rDoc)
            : mnHeavenLayer(rDoc.GetInvisibleHeavenId()),
              mnHellLayer(rDoc.GetInvisibleHellId()),
              mnFormLayer(rDoc.GetInvisibleControlsId())
        {
        }
        // <--

        SetLayer::SetLayer(const SetLayer& rOther) throw()
            : mnHeavenLayer(rOther.mnHeavenLayer),
            mnHellLayer(rOther.mnHellLayer),
            mnFormLayer(rOther.mnFormLayer)
        {
        }

        SetLayer& SetLayer::operator=(const SetLayer& rOther) throw()
        {
            SetLayer aTemp(rOther);
            Swap(aTemp);
            return *this;
        }
        //SetLayer boilerplate end

        void GetPoolItems(const SfxItemSet &rSet, PoolItems &rItems, bool bExportParentItemSet )
        {
            if( bExportParentItemSet )
            {
                sal_uInt16 nTotal = rSet.TotalCount();
                for( sal_uInt16 nItem =0; nItem < nTotal; ++nItem )
                {
                    const SfxPoolItem* pItem = 0;
                    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetWhichByPos( nItem ), true, &pItem ) )
                    {
                        rItems[pItem->Which()] = pItem;
                    }
                }
            }
            else if( rSet.Count())
            {
                SfxItemIter aIter(rSet);
                if (const SfxPoolItem *pItem = aIter.GetCurItem())
                {
                    do
                        rItems[pItem->Which()] = pItem;
                    while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
                }
            }
        }

        const SfxPoolItem *SearchPoolItems(const PoolItems &rItems,
            sal_uInt16 eType)
        {
            sw::cPoolItemIter aIter = rItems.find(eType);
            if (aIter != rItems.end())
                return aIter->second;
            return 0;
        }

        void ClearOverridesFromSet(const SwFmtCharFmt &rFmt, SfxItemSet &rSet)
        {
            if (const SwCharFmt* pCharFmt = rFmt.GetCharFmt())
            {
                if (pCharFmt->GetAttrSet().Count())
                {
                    SfxItemIter aIter(pCharFmt->GetAttrSet());
                    const SfxPoolItem *pItem = aIter.GetCurItem();
                    do
                        rSet.ClearItem(pItem->Which());
                    while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
                }
            }
        }

        ParaStyles GetParaStyles(const SwDoc &rDoc)
        {
            ParaStyles aStyles;
            typedef ParaStyles::size_type mysizet;

            const SwTxtFmtColls *pColls = rDoc.GetTxtFmtColls();
            mysizet nCount = pColls ? pColls->Count() : 0;
            aStyles.reserve(nCount);
            for (mysizet nI = 0; nI < nCount; ++nI)
                aStyles.push_back((*pColls)[ static_cast< sal_uInt16 >(nI) ]);
            return aStyles;
        }

        SwTxtFmtColl* GetParaStyle(SwDoc &rDoc, const String& rName)
        {
            // Search first in the Doc-Styles
            SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName(rName);
            if (!pColl)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pColl = rDoc.GetTxtCollFromPool(n, false);
            }
            return pColl;
        }

        SwCharFmt* GetCharStyle(SwDoc &rDoc, const String& rName)
        {
            SwCharFmt *pFmt = rDoc.FindCharFmtByName(rName);
            if (!pFmt)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pFmt = rDoc.GetCharFmtFromPool(n);
            }
            return pFmt;
        }

        // --> OD 2009-02-04 #i98791# - adjust sorting algorithm
        void SortByAssignedOutlineStyleListLevel(ParaStyles &rStyles)
        {
            std::sort(rStyles.begin(), rStyles.end(), outlinecmp());
        }
        // <--

        /*
           Utility to extract flyfmts from a document, potentially from a
           selection.
           */
        Frames GetFrames(const SwDoc &rDoc, SwPaM *pPaM /*, bool bAll*/)
        {
            SwPosFlyFrms aFlys(rDoc.GetAllFlyFmts(pPaM, true));
            sw::Frames aRet(SwPosFlyFrmsToFrames(aFlys));
            return aRet;
        }

#if 0        
        Frames GetFramesBetweenNodes(const Frames &rFrames,
            const SwNode &rStart, const SwNode &rEnd)
        {
            Frames aRet;
            sal_uLong nEnd = rEnd.GetIndex();
            for (sal_uLong nI = rStart.GetIndex(); nI < nEnd; ++nI)
            {
                my_copy_if(rFrames.begin(), rFrames.end(),
                    std::back_inserter(aRet), anchoredto(nI));
            }
            return aRet;

        }
#endif
        Frames GetFramesInNode(const Frames &rFrames, const SwNode &rNode)
        {
            Frames aRet;
            my_copy_if(rFrames.begin(), rFrames.end(),
                std::back_inserter(aRet), anchoredto(rNode.GetIndex()));
            return aRet;
        }

        const SwNumFmt* GetNumFmtFromTxtNode(const SwTxtNode &rTxtNode)
        {
            const SwNumRule *pRule = 0;
            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetNumRule())
                )
            {
                return &(pRule->Get( static_cast< sal_uInt16 >(rTxtNode.GetActualListLevel()) ));
            }

            ASSERT(rTxtNode.GetDoc(), "No document for node?, suspicious");
            if (!rTxtNode.GetDoc())
                return 0;

            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetDoc()->GetOutlineNumRule())
                )
            {
                return &(pRule->Get( static_cast< sal_uInt16 >(rTxtNode.GetActualListLevel()) ));
            }

            return 0;
        }

        const SwNumRule* GetNumRuleFromTxtNode(const SwTxtNode &rTxtNode)
        {
            return GetNormalNumRuleFromTxtNode(rTxtNode);
        }

        const SwNumRule* GetNormalNumRuleFromTxtNode(const SwTxtNode &rTxtNode)
        {
            const SwNumRule *pRule = 0;

            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetNumRule())
               )
            {
                return pRule;
            }
            return 0;
        }


        SwNoTxtNode *GetNoTxtNodeFromSwFrmFmt(const SwFrmFmt &rFmt)
        {
            const SwNodeIndex *pIndex = rFmt.GetCntnt().GetCntntIdx();
            ASSERT(pIndex, "No NodeIndex in SwFrmFmt ?, suspicious");
            if (!pIndex)
                return 0;
            SwNodeIndex aIdx(*pIndex, 1);
            return aIdx.GetNode().GetNoTxtNode();
        }

        bool HasPageBreak(const SwNode &rNd)
        {
            const SvxFmtBreakItem *pBreak = 0;
            if (rNd.IsTableNode() && rNd.GetTableNode())
            {
                const SwTable& rTable = rNd.GetTableNode()->GetTable();
                const SwFrmFmt* pApply = rTable.GetFrmFmt();
                ASSERT(pApply, "impossible");
                if (pApply)
                    pBreak = &(ItemGet<SvxFmtBreakItem>(*pApply, RES_BREAK));
            }
            else if (const SwCntntNode *pNd = rNd.GetCntntNode())
                pBreak = &(ItemGet<SvxFmtBreakItem>(*pNd, RES_BREAK));

            if (pBreak && pBreak->GetBreak() == SVX_BREAK_PAGE_BEFORE)
                return true;
            return false;
        }

        Polygon PolygonFromPolyPolygon(const PolyPolygon &rPolyPoly)
        {
			if(1 == rPolyPoly.Count())
			{
				return rPolyPoly[0];
			}
			else
			{
				// This method will now just concatenate the polygons contained
				// in the given PolyPolygon. Anything else which might be thought of
				// for reducing to a single polygon will just need nore power and
				// cannot create more correct results.
				sal_uInt32 nPointCount(0L);
				sal_uInt16 a;

				for(a = 0; a < rPolyPoly.Count(); a++)
				{
					nPointCount += (sal_uInt32)rPolyPoly[a].GetSize();
				}

				if(nPointCount > 0x0000ffff)
				{
					DBG_ERROR("PolygonFromPolyPolygon: too many points for a single polygon (!)");
					nPointCount = 0x0000ffff;
				}

				Polygon aRetval((sal_uInt16)nPointCount);
				sal_uInt32 nAppendIndex(0L);

				for(a = 0; a < rPolyPoly.Count(); a++)
				{
					const Polygon& rCandidate = rPolyPoly[a];

					for(sal_uInt16 b(0); nAppendIndex <= nPointCount && b < rCandidate.GetSize(); b++)
					{
						aRetval[(sal_uInt16)nAppendIndex++] = rCandidate[b];
					}
				}

				return aRetval;
			}
        }

        bool IsStarSymbol(const String &rFontName)
        {
            String sFamilyNm(GetFontToken(rFontName, 0));
            return (sFamilyNm.EqualsIgnoreCaseAscii("starsymbol") ||
                sFamilyNm.EqualsIgnoreCaseAscii("opensymbol"));
        }

        Size GetSwappedInSize(const SwNoTxtNode& rNd)
        {
            Size aGrTwipSz(rNd.GetTwipSize());
            if ((!aGrTwipSz.Width() || !aGrTwipSz.Height()))
            {
                SwGrfNode *pGrfNode = const_cast<SwGrfNode*>(rNd.GetGrfNode());
                if (pGrfNode && (GRAPHIC_NONE != pGrfNode->GetGrf().GetType()))
                {
                    bool bWasSwappedOut = pGrfNode->GetGrfObj().IsSwappedOut();
                    pGrfNode->SwapIn();
                    aGrTwipSz = pGrfNode->GetTwipSize();
                    if (bWasSwappedOut)
                        pGrfNode->SwapOut();
                }
            }

            ASSERT(aGrTwipSz.Width() && aGrTwipSz.Height(), "0 x 0 graphic ?");
            return aGrTwipSz;
        }

        void RedlineStack::open(const SwPosition& rPos, const SfxPoolItem& rAttr)
        {
            ASSERT(rAttr.Which() == RES_FLTR_REDLINE, "not a redline");
            maStack.push_back(new SwFltStackEntry(rPos,rAttr.Clone()));
        }


        class SameOpenRedlineType :
            public std::unary_function<const SwFltStackEntry*, bool>
        {
        private:
            RedlineType_t meType;
        public:
            SameOpenRedlineType(RedlineType_t eType) : meType(eType) {}
            bool operator()(const SwFltStackEntry *pEntry) const
            {
                const SwFltRedline *pTest = static_cast<const SwFltRedline *>
                    (pEntry->pAttr);
                return (pEntry->bLocked && (pTest->eType == meType));
            }
        };

        bool RedlineStack::close(const SwPosition& rPos, RedlineType_t eType)
        {
            //Search from end for same type
            myriter aResult = std::find_if(maStack.rbegin(), maStack.rend(),
                SameOpenRedlineType(eType));
            if (aResult != maStack.rend())
            {
                (*aResult)->SetEndPos(rPos);
                return true;
            }
            return false;
        }



        void RedlineStack::closeall(const SwPosition& rPos)
        {
            std::for_each(maStack.begin(), maStack.end(), CloseIfOpen(rPos));
        }


        void SetInDocAndDelete::operator()(SwFltStackEntry *pEntry)
        {
            SwPaM aRegion(pEntry->nMkNode);
            if (
                pEntry->MakeRegion(&mrDoc, aRegion, true) &&
                (*aRegion.GetPoint() != *aRegion.GetMark())
            )
            {
                mrDoc.SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT |
									     nsRedlineMode_t::REDLINE_SHOW_DELETE));
                const SwFltRedline *pFltRedline = static_cast<const SwFltRedline*>
                    (pEntry->pAttr);

                if (USHRT_MAX != pFltRedline->nAutorNoPrev)
                {
                    SwRedlineData aData(pFltRedline->eTypePrev,
                        pFltRedline->nAutorNoPrev, pFltRedline->aStampPrev, aEmptyStr,
                        0);

                    mrDoc.AppendRedline(new SwRedline(aData, aRegion), true);
                }

                SwRedlineData aData(pFltRedline->eType, pFltRedline->nAutorNo,
                        pFltRedline->aStamp, aEmptyStr, 0);

                mrDoc.AppendRedline(new SwRedline(aData, aRegion), true);
                mrDoc.SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_NONE | nsRedlineMode_t::REDLINE_SHOW_INSERT |
				     nsRedlineMode_t::REDLINE_SHOW_DELETE ));
            }
            delete pEntry;
        }


        bool CompareRedlines::operator()(const SwFltStackEntry *pOneE,
            const SwFltStackEntry *pTwoE) const
        {
            const SwFltRedline *pOne= static_cast<const SwFltRedline*>
                (pOneE->pAttr);
            const SwFltRedline *pTwo= static_cast<const SwFltRedline*>
                (pTwoE->pAttr);

            //Return the earlier time, if two have the same time, prioritize
            //inserts over deletes
            if (pOne->aStamp == pTwo->aStamp)
                return (pOne->eType == nsRedlineType_t::REDLINE_INSERT && pTwo->eType != nsRedlineType_t::REDLINE_INSERT);
            else
                return (pOne->aStamp < pTwo->aStamp) ? true : false;
        }


        RedlineStack::~RedlineStack()
        {
            std::sort(maStack.begin(), maStack.end(), CompareRedlines());
            std::for_each(maStack.begin(), maStack.end(), SetInDocAndDelete(mrDoc));
        }

        sal_uInt16 WrtRedlineAuthor::AddName( const String& rNm )
        {
            sal_uInt16 nRet;
            typedef std::vector<String>::iterator myiter;
            myiter aIter = std::find(maAuthors.begin(), maAuthors.end(), rNm);
            if (aIter != maAuthors.end())
                nRet = static_cast< sal_uInt16 >(aIter - maAuthors.begin());
            else
            {
                nRet = static_cast< sal_uInt16 >(maAuthors.size());
                maAuthors.push_back(rNm);
            }
            return nRet;
        }
/*
        std::vector<String> WrtRedlineAuthor::GetNames()
        {
            return maAuthors;
        }
*/
    }
}
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
