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


#include <bookmrk.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <doc.hxx>
#include <errhdl.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <swserv.hxx>
#include <sfx2/linkmgr.hxx>
#include <swtypes.hxx>
#include <UndoBookmark.hxx>
#include <unobookmark.hxx>
#include <rtl/random.h>
#include <xmloff/odffields.hxx>


SV_IMPL_REF( SwServerObject )

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    static void lcl_FixPosition(SwPosition& rPos)
    {
        // make sure the position has 1) the proper node, and 2) a proper index
        SwTxtNode* pTxtNode = rPos.nNode.GetNode().GetTxtNode();
        if(pTxtNode == NULL && rPos.nContent.GetIndex() > 0)
        {
            OSL_TRACE(
                "bookmrk.cxx::lcl_FixPosition"
                " - illegal position: %d without proper TxtNode", rPos.nContent.GetIndex());
            rPos.nContent.Assign(NULL, 0);
        }
        else if(pTxtNode != NULL && rPos.nContent.GetIndex() > pTxtNode->Len())
        {
            OSL_TRACE(
                "bookmrk.cxx::lcl_FixPosition"
                " - illegal position: %d is beyond %d", rPos.nContent.GetIndex(), pTxtNode->Len());
            rPos.nContent.Assign(pTxtNode, pTxtNode->Len());
        }
    };

    static void lcl_AssureFieldMarksSet(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, NULL);

        SwPosition rStart = pField->GetMarkStart();
        SwTxtNode const*const pStartTxtNode = rStart.nNode.GetNode().GetTxtNode();
        const sal_Unicode ch_start=pStartTxtNode->GetTxt().GetChar(rStart.nContent.GetIndex());
        if(ch_start != aStartMark)
        {
            SwPaM aStartPaM(rStart);
            io_pDoc->InsertString(aStartPaM, aStartMark);
            rStart.nContent--;
            pField->SetMarkStartPos( rStart );
        }

        const SwPosition& rEnd = pField->GetMarkEnd();
        SwTxtNode const*const pEndTxtNode = rEnd.nNode.GetNode().GetTxtNode();
        const sal_Unicode ch_end=pEndTxtNode->GetTxt().GetChar(rEnd.nContent.GetIndex()-1);
        if ( aEndMark && ( ch_end != aEndMark ) && ( rStart != rEnd ) )
        {
            SwPaM aEndPaM(rEnd);
            io_pDoc->InsertString(aEndPaM, aEndMark);
        }


        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, NULL);
    };

    static void lcl_RemoveFieldMarks(Fieldmark* const pField,
        SwDoc* const io_pDoc,
        const sal_Unicode aStartMark,
        const sal_Unicode aEndMark)
    {
        io_pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_UI_REPLACE, NULL);

        const SwPosition& rStart = pField->GetMarkStart();
        SwTxtNode const*const pStartTxtNode = rStart.nNode.GetNode().GetTxtNode();
        const sal_Unicode ch_start=pStartTxtNode->GetTxt().GetChar(rStart.nContent.GetIndex());
        if( ch_start == aStartMark )
        {
            SwPaM aStart(rStart, rStart);
            aStart.End()->nContent++;
            io_pDoc->DeleteRange(aStart);
        }

        const SwPosition& rEnd = pField->GetMarkEnd();
        SwTxtNode const*const pEndTxtNode = rEnd.nNode.GetNode().GetTxtNode();
        const xub_StrLen nEndPos = ( rEnd == rStart ||  rEnd.nContent.GetIndex() == 0 )
                                   ? rEnd.nContent.GetIndex()
                                   : rEnd.nContent.GetIndex() - 1;
        const sal_Unicode ch_end=pEndTxtNode->GetTxt().GetChar( nEndPos );
        if ( ch_end == aEndMark )
        {
            SwPaM aEnd(rEnd, rEnd);
            aEnd.Start()->nContent--;
            io_pDoc->DeleteRange(aEnd);
        }

        io_pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_UI_REPLACE, NULL);
    };
}

namespace sw { namespace mark
{
    MarkBase::MarkBase(const SwPaM& aPaM,
        const ::rtl::OUString& rName)
        : SwModify(0)
        , m_pPos1(new SwPosition(*(aPaM.GetPoint())))
        , m_aName(rName)
    {
        lcl_FixPosition(*m_pPos1);
        if (aPaM.HasMark() && (*aPaM.GetMark() != *aPaM.GetPoint()))
        {
            MarkBase::SetOtherMarkPos(*(aPaM.GetMark()));
            lcl_FixPosition(*m_pPos2);
        }
    }

    bool MarkBase::IsCoveringPosition(const SwPosition& rPos) const
    {
        return GetMarkStart() <= rPos && rPos <= GetMarkEnd();
    }

    void MarkBase::SetMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos1);
        //lcl_FixPosition(*m_pPos1);
    }

    void MarkBase::SetOtherMarkPos(const SwPosition& rNewPos)
    {
        ::boost::scoped_ptr<SwPosition>(new SwPosition(rNewPos)).swap(m_pPos2);
        //lcl_FixPosition(*m_pPos2);
    }

    rtl::OUString MarkBase::ToString( ) const
    {
        rtl::OUStringBuffer buf;
        buf.appendAscii( "Mark: ( Name, [ Node1, Index1 ] ): ( " );
        buf.append( m_aName ).appendAscii( ", [ " );
        buf.append( sal_Int32( GetMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetMarkPos().nContent.GetIndex( ) ) ).appendAscii( " ] )" );

        return buf.makeStringAndClear( );
    }

    MarkBase::~MarkBase()
    { }

    ::rtl::OUString MarkBase::GenerateNewName(const ::rtl::OUString& rPrefix)
    {
        static rtlRandomPool aPool = rtl_random_createPool();
        static ::rtl::OUString sUniquePostfix;
        static sal_Int32 nCount = SAL_MAX_INT32;
        ::rtl::OUStringBuffer aResult(rPrefix);
        if(nCount == SAL_MAX_INT32)
        {
            sal_Int32 nRandom;
            rtl_random_getBytes(aPool, &nRandom, sizeof(nRandom));
            sUniquePostfix = ::rtl::OUStringBuffer(13).appendAscii("_").append(static_cast<sal_Int32>(abs(nRandom))).makeStringAndClear();
            nCount = 0;
        }
        // putting the counter in front of the random parts will speed up string comparisons
        return aResult.append(nCount++).append(sUniquePostfix).makeStringAndClear();
    }


    void MarkBase::Modify( const SfxPoolItem *pOld, const SfxPoolItem *pNew )
    {
        NotifyClients(pOld, pNew);
        if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
        {   // invalidate cached uno object
            SetXBookmark(uno::Reference<text::XTextContent>(0));
        }
    }


    NavigatorReminder::NavigatorReminder(const SwPaM& rPaM)
        : MarkBase(rPaM, our_sNamePrefix)
    { }

    const ::rtl::OUString NavigatorReminder::our_sNamePrefix = ::rtl::OUString::createFromAscii("__NavigatorReminder__");

    UnoMark::UnoMark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    { }

    const ::rtl::OUString UnoMark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__UnoMark__");

    DdeBookmark::DdeBookmark(const SwPaM& aPaM)
        : MarkBase(aPaM, MarkBase::GenerateNewName(our_sNamePrefix))
        , m_aRefObj(NULL)
        , mbInDestruction( false )
    { }

    void DdeBookmark::SetRefObject(SwServerObject* pObj)
    {
        m_aRefObj = pObj;
    }

    const ::rtl::OUString DdeBookmark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__DdeLink__");

    void DdeBookmark::DeregisterFromDoc(SwDoc* const pDoc)
    {
        if(m_aRefObj.Is())
            pDoc->GetLinkManager().RemoveServer(m_aRefObj);
    }

    DdeBookmark::~DdeBookmark()
    {
        mbInDestruction = true;
        if( m_aRefObj.Is() )
        {
            if(m_aRefObj->HasDataLinks())
            {
                ::sfx2::SvLinkSource* p = &m_aRefObj;
                p->SendDataChanged();
            }
            m_aRefObj->SetNoServer();
        }
    }

    Bookmark::Bookmark(const SwPaM& aPaM,
        const KeyCode& rCode,
        const ::rtl::OUString& rName,
        const ::rtl::OUString& rShortName)
        : DdeBookmark(aPaM)
        , ::sfx2::Metadatable()
        , m_aCode(rCode)
        , m_sShortName(rShortName)
    {
        m_aName = rName;
    }

    void Bookmark::InitDoc(SwDoc* const io_pDoc)
    {
        if (io_pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            io_pDoc->GetIDocumentUndoRedo().AppendUndo(
                    new SwUndoInsBookmark(*this));
        }
        io_pDoc->SetModified();
    }

    // ::sfx2::Metadatable
    ::sfx2::IXmlIdRegistry& Bookmark::GetRegistry()
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::MakeUnoObject: no doc?");
        return pDoc->GetXmlIdRegistry();
    }

    bool Bookmark::IsInClipboard() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::IsInClipboard: no doc?");
        return pDoc->IsClipBoard();
    }

    bool Bookmark::IsInUndo() const
    {
        return false;
    }

    bool Bookmark::IsInContent() const
    {
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::IsInContent: no doc?");
        return !pDoc->IsInHeaderFooter( SwNodeIndex(GetMarkPos().nNode) );
    }

    uno::Reference< rdf::XMetadatable > Bookmark::MakeUnoObject()
    {
        // create new SwXBookmark
        SwDoc *const pDoc( GetMarkPos().GetDoc() );
        OSL_ENSURE(pDoc, "Bookmark::MakeUnoObject: no doc?");
        const uno::Reference< rdf::XMetadatable> xMeta(
                SwXBookmark::CreateXBookmark(*pDoc, *this), uno::UNO_QUERY);
        return xMeta;
    }


    Fieldmark::Fieldmark(const SwPaM& rPaM)
        : MarkBase(rPaM, MarkBase::GenerateNewName(our_sNamePrefix))
    {
        if(!IsExpanded())
            SetOtherMarkPos(GetMarkPos());
    }

    void Fieldmark::SetMarkStartPos( const SwPosition& rNewStartPos )
    {
        if ( GetMarkPos( ) <= GetOtherMarkPos( ) )
            return SetMarkPos( rNewStartPos );
        else
            return SetOtherMarkPos( rNewStartPos );
    }

    void Fieldmark::SetMarkEndPos( const SwPosition& rNewEndPos )
    {
        if ( GetMarkPos( ) <= GetOtherMarkPos( ) )
            return SetOtherMarkPos( rNewEndPos );
        else
            return SetMarkPos( rNewEndPos );
    }

    rtl::OUString Fieldmark::ToString( ) const
    {
        rtl::OUStringBuffer buf;
        buf.appendAscii( "Fieldmark: ( Name, Type, [ Nd1, Id1 ], [ Nd2, Id2 ] ): ( " );
        buf.append( m_aName ).appendAscii( ", " );
        buf.append( m_aFieldname ).appendAscii( ", [ " );
        buf.append( sal_Int32( GetMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ], [" );
        buf.append( sal_Int32( GetOtherMarkPos().nNode.GetIndex( ) ) ).appendAscii( ", " );
        buf.append( sal_Int32( GetOtherMarkPos( ).nContent.GetIndex( ) ) ).appendAscii( " ] ) " );

        return buf.makeStringAndClear( );
    }

    void Fieldmark::Invalidate( )
    {
        // @TODO: Does exist a better solution to trigger a format of the
        //        fieldmark portion? If yes, please use it.
        SwPaM aPaM( this->GetMarkPos(), this->GetOtherMarkPos() );
        aPaM.InvalidatePaM();
    }

    const ::rtl::OUString Fieldmark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__Fieldmark__");

    TextFieldmark::TextFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void TextFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    void TextFieldmark::ReleaseDoc(SwDoc* const pDoc)
    {
        lcl_RemoveFieldMarks(this, pDoc, CH_TXT_ATR_FIELDSTART, CH_TXT_ATR_FIELDEND);
    }

    CheckboxFieldmark::CheckboxFieldmark(const SwPaM& rPaM)
        : Fieldmark(rPaM)
    { }

    void CheckboxFieldmark::InitDoc(SwDoc* const io_pDoc)
    {
        lcl_AssureFieldMarksSet(this, io_pDoc, CH_TXT_ATR_FORMELEMENT, CH_TXT_ATR_FIELDEND);

        // For some reason the end mark is moved from 1 by the Insert: we don't
        // want this for checkboxes
        SwPosition aNewEndPos = this->GetMarkEnd();
        aNewEndPos.nContent--;
        SetMarkEndPos( aNewEndPos );
    }
    void CheckboxFieldmark::SetChecked(bool checked)
    {
        (*GetParameters())[::rtl::OUString::createFromAscii(ODF_FORMCHECKBOX_RESULT)] = makeAny(checked);
    }

    bool CheckboxFieldmark::IsChecked() const
    {
        bool bResult = false;
        parameter_map_t::const_iterator pResult = GetParameters()->find(::rtl::OUString::createFromAscii(ODF_FORMCHECKBOX_RESULT));
        if(pResult != GetParameters()->end())
            pResult->second >>= bResult;
        return bResult;
    }

}}
