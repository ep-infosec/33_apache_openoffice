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
#define _SVSTDARR_STRINGS

#include <string.h>
#include <stdio.h>                      // sscanf
#include <hintids.hxx>
#include <i18npool/lang.h>
#include <i18npool/mslangid.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/parhtml.hxx>
#include <svl/svstdarr.hxx>
#include <sot/storage.hxx>
#include <sot/clsids.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <errhdl.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <fltini.hxx>
#include <hints.hxx>
#include <frmatr.hxx>
#include <fmtfsize.hxx>
#include <swtable.hxx>
#include <fmtcntnt.hxx>
#include <editeng/boxitem.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <numrule.hxx>
#include <ndtxt.hxx>
#include <swfltopt.hxx>
#include <swerror.h>
#include <osl/module.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <rtl/uri.hxx>
#include <tools/svlibrary.hxx>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star;

SwRead ReadAscii = 0, /*ReadSwg = 0, ReadSw3 = 0,*/
                ReadHTML = 0, ReadXML = 0;

Reader* GetRTFReader();
Reader* GetWW8Reader();

// Note: if editing, please don't forget to modify also the enum
// ReaderWriterEnum and aFilterDetect in shellio.hxx
SwReaderWriterEntry aReaderWriter[] =
{
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_False ),
    SwReaderWriterEntry( &::GetWW8Reader, &::GetWW8Writer,  sal_True  ),
    SwReaderWriterEntry( &::GetWW8Reader, &::GetWW8Writer,  sal_True  ),
    SwReaderWriterEntry( &::GetRTFReader, &::GetRTFWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetHTMLWriter, sal_True  ),
    SwReaderWriterEntry( 0,               0,                sal_True  ),
    SwReaderWriterEntry( &::GetWW8Reader, 0,                sal_True  ),
    SwReaderWriterEntry( 0,               &::GetXMLWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_True  ),
    SwReaderWriterEntry( 0,               &::GetASCWriter,  sal_True  )
};

Reader* SwReaderWriterEntry::GetReader()
{
    if ( pReader )
        return pReader;
    else if ( fnGetReader )
    {
        pReader = (*fnGetReader)();
        return pReader;
    }
    return NULL;
}

void SwReaderWriterEntry::GetWriter( const String& rNm, const String& rBaseURL, WriterRef& xWrt ) const
{
    if ( fnGetWriter )
        (*fnGetWriter)( rNm, rBaseURL, xWrt );
    else
        xWrt = WriterRef(0);
}

/*SwRead SwGetReaderSw3() // SW_DLLPUBLIC
{
        return ReadSw3;
}
*/
SwRead SwGetReaderXML() // SW_DLLPUBLIC
{
        return ReadXML;
}

bool IsDocShellRegistered()
{
    return 0 != SwDocShell::_GetInterface();
}

inline void _SetFltPtr( sal_uInt16 rPos, SwRead pReader )
{
        aReaderWriter[ rPos ].pReader = pReader;
}

void _InitFilter()
{
    _SetFltPtr( READER_WRITER_BAS, (ReadAscii = new AsciiReader) );
    _SetFltPtr( READER_WRITER_HTML, (ReadHTML = new HTMLReader) );
    _SetFltPtr( READER_WRITER_WW1, new WW1Reader );
    _SetFltPtr( READER_WRITER_XML, (ReadXML = new XMLReader)  );

#ifdef NEW_WW97_EXPORT
    aReaderWriter[ READER_WRITER_WW1 ].fnGetWriter =  &::GetWW8Writer;
    aReaderWriter[ READER_WRITER_WW5 ].fnGetWriter = &::GetWW8Writer;
#endif

    _SetFltPtr( READER_WRITER_TEXT_DLG, ReadAscii );
    _SetFltPtr( READER_WRITER_TEXT, ReadAscii );
}

void _FinitFilter()
{
        // die Reader vernichten
        for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
        {
                SwReaderWriterEntry& rEntry = aReaderWriter[n];
                if( rEntry.bDelReader && rEntry.pReader )
                        delete rEntry.pReader, rEntry.pReader = NULL;
        }
}


/*  */

namespace SwReaderWriter {

Reader* GetReader( ReaderWriterEnum eReader )
{
    return aReaderWriter[eReader].GetReader();
}

void GetWriter( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
        for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
                if( aFilterDetect[n].IsFilter( rFltName ) )
                {
            aReaderWriter[n].GetWriter( rFltName, rBaseURL, xRet );
                        break;
                }
}

SwRead GetReader( const String& rFltName )
{
        SwRead pRead = 0;
        for( sal_uInt16 n = 0; n < MAXFILTER; ++n )
                if( aFilterDetect[n].IsFilter( rFltName ) )
                {
                        pRead = aReaderWriter[n].GetReader();
                        // fuer einige Reader noch eine Sonderbehandlung:
                        if ( pRead )
                                pRead->SetFltName( rFltName );
                        break;
                }
        return pRead;
}

} // namespace SwReaderWriter

/*  */

/////////////// die Storage Reader/Writer ////////////////////////////////

/*void GetSw3Writer( const String&, const String& rBaseURL, WriterRef& xRet )
{
    DBG_ERROR( "Shouldn't happen!");
        xRet = new Sw3Writer;
}
*/

sal_uLong StgReader::OpenMainStream( SvStorageStreamRef& rRef, sal_uInt16& rBuffSize )
{
        sal_uLong nRet = ERR_SWG_READ_ERROR;
        ASSERT( pStg, "wo ist mein Storage?" );
        const SfxFilter* pFltr = SwIoSystem::GetFilterOfFormat( aFltName );
        if( pFltr )
        {
        rRef = pStg->OpenSotStream( SwIoSystem::GetSubStorageName( *pFltr ),
                                                                        STREAM_READ | STREAM_SHARE_DENYALL );

                if( rRef.Is() )
                {
                        if( SVSTREAM_OK == rRef->GetError() )
                        {
                                sal_uInt16 nOld = rRef->GetBufferSize();
                                rRef->SetBufferSize( rBuffSize );
                                rBuffSize = nOld;
                                nRet = 0;
                        }
                        else
                                nRet = rRef->GetError();
                }
        }
        return nRet;
}

/*  */
/*
sal_uLong Sw3Reader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
{
        sal_uLong nRet;
        if( pStg && pIO )
        {
                // sal_True: Vorlagen ueberschreiben
                pIO->SetReadOptions( aOpt,sal_True );
                if( !bInsertMode )
                {
                        // Im Laden-Modus darf der PaM-Content-Teil nicht
                        // in den Textbereich zeigen (Nodes koennen geloescht werden)
                        rPam.GetBound( sal_True ).nContent.Assign( 0, 0 );
                        rPam.GetBound( sal_False ).nContent.Assign( 0, 0 );
                }
                nRet = pIO->Load( pStg, bInsertMode ? &rPam : 0 );
                aOpt.ResetAllFmtsOnly();
                pIO->SetReadOptions( aOpt, sal_True );
        }
        else
        {
                ASSERT( sal_False, "Sw3-Read without storage and/or IO system" );
                nRet = ERR_SWG_READ_ERROR;
        }
        return nRet;
}

        // read the sections of the document, which is equal to the medium.
        // returns the count of it
sal_uInt16 Sw3Reader::GetSectionList( SfxMedium& rMedium,
                                                                        SvStrings& rStrings ) const
{
        SvStorageRef aStg( rMedium.GetStorage() );
        const SfxFilter* pFlt = rMedium.GetFilter();
        ASSERT( pFlt && pFlt->GetVersion(),
                                                                "Kein Filter oder Filter ohne FF-Version" );
        if( pFlt && pFlt->GetVersion() )
                aStg->SetVersion( (long)pFlt->GetVersion() );

        if( pIO )
                pIO->GetSectionList( &aStg, rStrings );
        return rStrings.Count();
    return 0;
}
*/

/*sal_uLong Sw3Writer::WriteStorage()
{
    sal_uLong nRet;
        if( pIO )
        {
                // der gleiche Storage -> Save, sonst SaveAs aufrufen
                if( !bSaveAs )
                        nRet = pIO->Save( pOrigPam, bWriteAll );
                else
                        nRet = pIO->SaveAs( pStg, pOrigPam, bWriteAll );

                pIO = 0;                // nach dem Schreiben ist der Pointer ungueltig !!
        }
        else
        {
                ASSERT( sal_False, "Sw3-Writer without IO-System" )
                nRet = ERR_SWG_WRITE_ERROR;
    }
    return nRet;
}

sal_uLong Sw3Writer::WriteMedium( SfxMedium& )
{
    DBG_ERROR( "Shouldn't be used currently!");
        return WriteStorage();
}

sal_Bool Sw3Writer::IsSw3Writer() const { return sal_True; }
*/

void Writer::SetPasswd( const String& ) {}


void Writer::SetVersion( const String&, long ) {}


sal_Bool Writer::IsStgWriter() const { return sal_False; }
//sal_Bool Writer::IsSw3Writer() const { return sal_False; }

sal_Bool StgWriter::IsStgWriter() const { return sal_True; }

/*  */



sal_Bool SwReader::NeedsPasswd( const Reader& /*rOptions*/ )
{
        sal_Bool bRes = sal_False;
    return bRes;
}


sal_Bool SwReader::CheckPasswd( const String& /*rPasswd*/, const Reader& /*rOptions*/ )
{
    return sal_True;
}


/*  */

//-----------------------------------------------------------------------
// Filter Flags lesen, wird von WW8 / W4W / EXCEL / LOTUS benutzt.
//-----------------------------------------------------------------------

/*
<FilterFlags>
        <Excel_Lotus>
                <MinRow cfg:type="long">0</MinRow>
                <MaxRow cfg:type="long">0</MaxRow>
                <MinCol cfg:type="long">0</MinCol>
                <MaxCol cfg:type="long">0</MaxCol>
        </Excel_Lotus>
        <W4W>
                <W4WHD cfg:type="long">0</W4WHD>
                <W4WFT cfg:type="long">0</W4WFT>
                <W4W000 cfg:type="long">0</W4W000>
        </W4W>
        <WinWord>
                <WW1F cfg:type="long">0</WW1F>
                <WW cfg:type="long">0</WW>
                <WW8 cfg:type="long">0</WW8>
                <WWF cfg:type="long">0</WWF>
                <WWFA0 cfg:type="long">0</WWFA0>
                <WWFA1 cfg:type="long">0</WWFA1>
                <WWFA2 cfg:type="long">0</WWFA2>
                <WWFB0 cfg:type="long">0</WWFB0>
                <WWFB1 cfg:type="long">0</WWFB1>
                <WWFB2 cfg:type="long">0</WWFB2>
                <WWFLX cfg:type="long">0</WWFLX>
                <WWFLY cfg:type="long">0</WWFLY>
                <WWFT cfg:type="long">0</WWFT>
                <WWWR cfg:type="long">0</WWWR>
        </WinWord>
        <Writer>
                <SW3Imp cfg:type="long">0</SW3Imp>
        </Writer>
</FilterFlags>
*/

#define FILTER_OPTION_ROOT              String::CreateFromAscii( \
                                RTL_CONSTASCII_STRINGPARAM( "Office.Writer/FilterFlags" ) )

SwFilterOptions::SwFilterOptions( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                sal_uInt32* pValues )
        : ConfigItem( FILTER_OPTION_ROOT )
{
        GetValues( nCnt, ppNames, pValues );
}

void SwFilterOptions::GetValues( sal_uInt16 nCnt, const sal_Char** ppNames,
                                                                        sal_uInt32* pValues )
{
        Sequence<OUString> aNames( nCnt );
        OUString* pNames = aNames.getArray();
        sal_uInt16 n;

        for( n = 0; n < nCnt; ++n )
                pNames[ n ] = OUString::createFromAscii( ppNames[ n ] );
        Sequence<Any> aValues = GetProperties( aNames );

        if( nCnt == aValues.getLength() )
        {
                const Any* pAnyValues = aValues.getConstArray();
                for( n = 0; n < nCnt; ++n )
                        pValues[ n ] = pAnyValues[ n ].hasValue()
                                                        ? *(sal_uInt32*)pAnyValues[ n ].getValue()
                                                        : 0;
        }
        else
                for( n = 0; n < nCnt; ++n )
                        pValues[ n ] = 0;
}

void SwFilterOptions::Commit() {}
void SwFilterOptions::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

/*  */


void StgReader::SetFltName( const String& rFltNm )
{
        if( SW_STORAGE_READER & GetReaderType() )
                aFltName = rFltNm;
}


/*  */

SwRelNumRuleSpaces::SwRelNumRuleSpaces( SwDoc& rDoc, sal_Bool bNDoc )
        : bNewDoc( bNDoc )
{
        pNumRuleTbl = new SwNumRuleTbl( 8, 8 );
        if( !bNDoc )
                pNumRuleTbl->Insert( &rDoc.GetNumRuleTbl(), 0 );
}

SwRelNumRuleSpaces::~SwRelNumRuleSpaces()
{
        if( pNumRuleTbl )
        {
                pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
                delete pNumRuleTbl;
        }
}

void SwRelNumRuleSpaces::SetNumRelSpaces( SwDoc& rDoc )
{
        const SwNumRuleTbl* pRuleTbl = NULL;

        if( !bNewDoc )
        {
                // jetzt alle schon vorhanden NumRules aus dem Array entfernen,
                // damit nur die neuen angepasst werden
                SwNumRuleTbl aNumRuleTbl;
                aNumRuleTbl.Insert( pNumRuleTbl, 0 );
                pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
                const SwNumRuleTbl& rRuleTbl = rDoc.GetNumRuleTbl();
                SwNumRule* pRule;

                for( sal_uInt16 n = 0; n < rRuleTbl.Count(); ++n )
                        if( USHRT_MAX == aNumRuleTbl.GetPos( ( pRule = rRuleTbl[ n ] )))
                                // war noch nicht vorhanden, also neu
                                pNumRuleTbl->Insert( pRule, pNumRuleTbl->Count() );

                aNumRuleTbl.Remove( 0, aNumRuleTbl.Count() );

        pRuleTbl = pNumRuleTbl;
        }
    else
    {
        pRuleTbl = &rDoc.GetNumRuleTbl();
    }

        if( pRuleTbl )
        {
                for( sal_uInt16 n = pRuleTbl->Count(); n; )
                {
                        SwNumRule* pRule = (*pRuleTbl)[ --n ];
                        // Rule noch gueltig und am Doc vorhanden?
                        if( USHRT_MAX != rDoc.GetNumRuleTbl().GetPos( pRule ))
                        {
                // --> OD 2008-02-19 #refactorlists#
//                SwNumRuleInfo aUpd( pRule->GetName() );
//                aUpd.MakeList( rDoc );

//                // bei allen nmumerierten Absaetzen vom linken Rand
//                // den absoluten Wert des NumFormates abziehen
//                for( sal_uLong nUpdPos = 0; nUpdPos < aUpd.GetList().Count();
//                    ++nUpdPos )
//                {
//                    SwTxtNode* pNd = aUpd.GetList().GetObject( nUpdPos );
//                    SetNumLSpace( *pNd, *pRule );
//                }
                SwNumRule::tTxtNodeList aTxtNodeList;
                pRule->GetTxtNodeList( aTxtNodeList );
                for ( SwNumRule::tTxtNodeList::iterator aIter = aTxtNodeList.begin();
                      aIter != aTxtNodeList.end(); ++aIter )
                {
                    SwTxtNode* pNd = *aIter;
                    SetNumLSpace( *pNd, *pRule );
                }
                // <--
                        }
                }
        }

        if( pNumRuleTbl )
        {
                pNumRuleTbl->Remove( 0, pNumRuleTbl->Count() );
                delete pNumRuleTbl, pNumRuleTbl = 0;
        }

        if( bNewDoc )
        {
                SetOultineRelSpaces( SwNodeIndex( rDoc.GetNodes() ),
                                                        SwNodeIndex( rDoc.GetNodes().GetEndOfContent()));
        }
}

void SwRelNumRuleSpaces::SetOultineRelSpaces( const SwNodeIndex& rStt,
                                                                                        const SwNodeIndex& rEnd )
{
        SwDoc* pDoc = rStt.GetNode().GetDoc();
        const SwOutlineNodes& rOutlNds = pDoc->GetNodes().GetOutLineNds();
        if( rOutlNds.Count() )
        {
                sal_uInt16 nPos;
                rOutlNds.Seek_Entry( &rStt.GetNode(), &nPos );
                for( ; nPos < rOutlNds.Count() &&
                                rOutlNds[ nPos ]->GetIndex() < rEnd.GetIndex(); ++nPos )
                {
                        SwTxtNode* pNd = rOutlNds[ nPos ]->GetTxtNode();
                        if( pNd->IsOutline() && !pNd->GetNumRule() )
                                SetNumLSpace( *pNd, *pDoc->GetOutlineNumRule() );
                }
        }
}

void SwRelNumRuleSpaces::SetNumLSpace( SwTxtNode& rNd, const SwNumRule& rRule )
{
        sal_Bool bOutlineRule = OUTLINE_RULE == rRule.GetRuleType();
    // --> OD 2005-11-18 #128056#
    // correction of refactoring done by cws swnumtree:
    // - assure a correct level for retrieving numbering format.
//    sal_uInt8 nLvl = rNd.GetLevel();
    sal_uInt8 nLvl = 0;
    if ( rNd.GetActualListLevel() >= 0 && rNd.GetActualListLevel() < MAXLEVEL )
    {
        nLvl = static_cast< sal_uInt8 >(rNd.GetActualListLevel());
    }
    // <--
        const SwNumFmt& rFmt = rRule.Get( nLvl );
        const SvxLRSpaceItem& rLR = rNd.GetSwAttrSet().GetLRSpace();

        SvxLRSpaceItem aLR( rLR );
        aLR.SetTxtFirstLineOfst( 0 );

        // sagt der Node, das die Numerierung den Wert vorgibt?
        if( !bOutlineRule && rNd.IsSetNumLSpace() )
                aLR.SetTxtLeft( 0 );
        else
        {
                long nLeft = rFmt.GetAbsLSpace(), nParaLeft = rLR.GetTxtLeft();
                if( 0 < rLR.GetTxtFirstLineOfst() )
                        nParaLeft += rLR.GetTxtFirstLineOfst();
                else if( nParaLeft >= nLeft )
                        // #82963#/#82962#: set correct paragraph indent
                        nParaLeft -= nLeft;
                else
                        //#83154#, Don't think any of the older #80856# bugfix code is
                        //relevant anymore.
                        nParaLeft = rLR.GetTxtLeft()+rLR.GetTxtFirstLineOfst();
                aLR.SetTxtLeft( nParaLeft );
        }

        if( aLR.GetTxtLeft() != rLR.GetTxtLeft() )
        {
                //bevor rLR geloescht wird!
                long nOffset = rLR.GetTxtLeft() - aLR.GetTxtLeft();
        rNd.SetAttr( aLR );

                // Tabs anpassen !!
                const SfxPoolItem* pItem;
                if( SFX_ITEM_SET == rNd.GetSwAttrSet().GetItemState(
                                RES_PARATR_TABSTOP, sal_True, &pItem ))
                {
                        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
                        for( sal_uInt16 n = 0; n < aTStop.Count(); ++n )
                        {
                                SvxTabStop& rTab = (SvxTabStop&)aTStop[ n ];
                                if( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() )
                                {
                                        if( !rTab.GetTabPos() )
                                        {
                                                aTStop.Remove( n );
                                                --n;
                                        }
                                        else
                                                rTab.GetTabPos() += nOffset;
                                }
                        }
            rNd.SetAttr( aTStop );
                }
        }
}

/*  */


void CalculateFlySize(SfxItemSet& rFlySet, const SwNodeIndex& rAnchor,
        SwTwips nPageWidth)
{
        const SfxPoolItem* pItem = 0;
        if( SFX_ITEM_SET != rFlySet.GetItemState( RES_FRM_SIZE, sal_True, &pItem ) ||
                MINFLY > ((SwFmtFrmSize*)pItem)->GetWidth() )
        {
                SwFmtFrmSize aSz((SwFmtFrmSize&)rFlySet.Get(RES_FRM_SIZE, sal_True));
                if (pItem)
                        aSz = (SwFmtFrmSize&)(*pItem);

                SwTwips nWidth;
                // dann die Breite des Flys selbst bestimmen. Ist eine Tabelle
                // defininiert, dann benutze deren Breite, sonst die Breite der
                // Seite
                const SwTableNode* pTblNd = rAnchor.GetNode().FindTableNode();
                if( pTblNd )
                        nWidth = pTblNd->GetTable().GetFrmFmt()->GetFrmSize().GetWidth();
                else
                        nWidth = nPageWidth;

                const SwNodeIndex* pSttNd = ((SwFmtCntnt&)rFlySet.Get( RES_CNTNT )).
                                                                        GetCntntIdx();
                if( pSttNd )
                {
                        sal_Bool bOnlyOneNode = sal_True;
                        sal_uLong nMinFrm = 0;
                        sal_uLong nMaxFrm = 0;
                        SwTxtNode* pFirstTxtNd = 0;
                        SwNodeIndex aIdx( *pSttNd, 1 );
                        SwNodeIndex aEnd( *pSttNd->GetNode().EndOfSectionNode() );
                        while( aIdx < aEnd )
                        {
                                SwTxtNode *pTxtNd = aIdx.GetNode().GetTxtNode();
                                if( pTxtNd )
                                {
                                        if( !pFirstTxtNd )
                                                pFirstTxtNd = pTxtNd;
                                        else if( pFirstTxtNd != pTxtNd )
                                        {
                                                // forget it
                                                bOnlyOneNode = sal_False;
                                                break;
                                        }

                                        sal_uLong nAbsMinCnts;
                                        pTxtNd->GetMinMaxSize( aIdx.GetIndex(), nMinFrm,
                                                                                        nMaxFrm, nAbsMinCnts );
                                }
                                aIdx++;
                        }

                        if( bOnlyOneNode )
                        {
                                if( nMinFrm < MINLAY && pFirstTxtNd )
                                {
                                        // if the first node dont contained any content, then
                                        // insert one char in it calc again and delete once again
                                        SwIndex aNdIdx( pFirstTxtNd );
                    pFirstTxtNd->InsertText( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "MM" )), aNdIdx );
                                        sal_uLong nAbsMinCnts;
                                        pFirstTxtNd->GetMinMaxSize( pFirstTxtNd->GetIndex(),
                                                                                        nMinFrm, nMaxFrm, nAbsMinCnts );
                                        aNdIdx -= 2;
                    pFirstTxtNd->EraseText( aNdIdx, 2 );
                }

                                // Umrandung und Abstand zum Inhalt beachten
                                const SvxBoxItem& rBoxItem = (SvxBoxItem&)rFlySet.Get( RES_BOX );
                                sal_uInt16 nLine = BOX_LINE_LEFT;
                                for( int i = 0; i < 2; ++i )
                                {
                                        const SvxBorderLine* pLn = rBoxItem.GetLine( nLine );
                                        if( pLn )
                                        {
                                                sal_uInt16 nWidthTmp = pLn->GetOutWidth() + pLn->GetInWidth();
                        nWidthTmp = nWidthTmp + rBoxItem.GetDistance( nLine );
                                                nMinFrm += nWidthTmp;
                                                nMaxFrm += nWidthTmp;
                                        }
                                        nLine = BOX_LINE_RIGHT;
                                }

                                // Mindestbreite fuer Inhalt einhalten
                                if( nMinFrm < MINLAY )
                                        nMinFrm = MINLAY;
                                if( nMaxFrm < MINLAY )
                                        nMaxFrm = MINLAY;

                                if( nWidth > (sal_uInt16)nMaxFrm )
                                        nWidth = nMaxFrm;
                                else if( nWidth > (sal_uInt16)nMinFrm )
                                        nWidth = nMinFrm;
                        }
                }

                if( MINFLY > nWidth )
                        nWidth = MINFLY;

                aSz.SetWidth( nWidth );
                if( MINFLY > aSz.GetHeight() )
                        aSz.SetHeight( MINFLY );
                rFlySet.Put( aSz );
        }
        else if( MINFLY > ((SwFmtFrmSize*)pItem)->GetHeight() )
        {
                SwFmtFrmSize aSz( *(SwFmtFrmSize*)pItem );
                aSz.SetHeight( MINFLY );
                rFlySet.Put( aSz );
        }
}

/*  */
struct CharSetNameMap
{
    rtl_TextEncoding eCode;
    const sal_Char* pName;
};

const CharSetNameMap *GetCharSetNameMap()
{
    static const CharSetNameMap aMapArr[] =
    {
#   define IMPLENTRY(X) { RTL_TEXTENCODING_##X, "" #X "" }
        IMPLENTRY(DONTKNOW),
        IMPLENTRY(MS_1252),
        IMPLENTRY(APPLE_ROMAN),
        IMPLENTRY(IBM_437),
        IMPLENTRY(IBM_850),
        IMPLENTRY(IBM_860),
        IMPLENTRY(IBM_861),
        IMPLENTRY(IBM_863),
        IMPLENTRY(IBM_865),
        IMPLENTRY(SYMBOL),
        IMPLENTRY(ASCII_US),
        IMPLENTRY(ISO_8859_1),
        IMPLENTRY(ISO_8859_2),
        IMPLENTRY(ISO_8859_3),
        IMPLENTRY(ISO_8859_4),
        IMPLENTRY(ISO_8859_5),
        IMPLENTRY(ISO_8859_6),
        IMPLENTRY(ISO_8859_7),
        IMPLENTRY(ISO_8859_8),
        IMPLENTRY(ISO_8859_9),
        IMPLENTRY(ISO_8859_14),
        IMPLENTRY(ISO_8859_15),
        IMPLENTRY(IBM_737),
        IMPLENTRY(IBM_775),
        IMPLENTRY(IBM_852),
        IMPLENTRY(IBM_855),
        IMPLENTRY(IBM_857),
        IMPLENTRY(IBM_862),
        IMPLENTRY(IBM_864),
        IMPLENTRY(IBM_866),
        IMPLENTRY(IBM_869),
        IMPLENTRY(MS_874),
        IMPLENTRY(MS_1250),
        IMPLENTRY(MS_1251),
        IMPLENTRY(MS_1253),
        IMPLENTRY(MS_1254),
        IMPLENTRY(MS_1255),
        IMPLENTRY(MS_1256),
        IMPLENTRY(MS_1257),
        IMPLENTRY(MS_1258),
        IMPLENTRY(APPLE_ARABIC),
        IMPLENTRY(APPLE_CENTEURO),
        IMPLENTRY(APPLE_CROATIAN),
        IMPLENTRY(APPLE_CYRILLIC),
        IMPLENTRY(APPLE_DEVANAGARI),
        IMPLENTRY(APPLE_FARSI),
        IMPLENTRY(APPLE_GREEK),
        IMPLENTRY(APPLE_GUJARATI),
        IMPLENTRY(APPLE_GURMUKHI),
        IMPLENTRY(APPLE_HEBREW),
        IMPLENTRY(APPLE_ICELAND),
        IMPLENTRY(APPLE_ROMANIAN),
        IMPLENTRY(APPLE_THAI),
        IMPLENTRY(APPLE_TURKISH),
        IMPLENTRY(APPLE_UKRAINIAN),
        IMPLENTRY(APPLE_CHINSIMP),
        IMPLENTRY(APPLE_CHINTRAD),
        IMPLENTRY(APPLE_JAPANESE),
        IMPLENTRY(APPLE_KOREAN),
        IMPLENTRY(MS_932),
        IMPLENTRY(MS_936),
        IMPLENTRY(MS_949),
        IMPLENTRY(MS_950),
        IMPLENTRY(SHIFT_JIS),
        IMPLENTRY(GB_2312),
        IMPLENTRY(GBT_12345),
        IMPLENTRY(GBK),
        IMPLENTRY(BIG5),
        IMPLENTRY(EUC_JP),
        IMPLENTRY(EUC_CN),
        IMPLENTRY(EUC_TW),
        IMPLENTRY(ISO_2022_JP),
        IMPLENTRY(ISO_2022_CN),
        IMPLENTRY(KOI8_R),
        IMPLENTRY(KOI8_U),
        IMPLENTRY(UTF7),
        IMPLENTRY(UTF8),
        IMPLENTRY(ISO_8859_10),
        IMPLENTRY(ISO_8859_13),
        IMPLENTRY(EUC_KR),
        IMPLENTRY(ISO_2022_KR),
        IMPLENTRY(JIS_X_0201),
        IMPLENTRY(JIS_X_0208),
        IMPLENTRY(JIS_X_0212),
        IMPLENTRY(MS_1361),
        IMPLENTRY(GB_18030),
        IMPLENTRY(BIG5_HKSCS),
        IMPLENTRY(TIS_620),
        IMPLENTRY(PT154),
        IMPLENTRY(UCS4),
        IMPLENTRY(UCS2),
        IMPLENTRY(UNICODE),
        {0,0}       //Last
    };
    return &aMapArr[0];
}
/*
 Get a rtl_TextEncoding from its name
 */
rtl_TextEncoding CharSetFromName(const String& rChrSetStr)
{
    const CharSetNameMap *pStart = GetCharSetNameMap();
    rtl_TextEncoding nRet = pStart->eCode;

    for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
    {
        if(rChrSetStr.EqualsIgnoreCaseAscii(pMap->pName))
        {
            nRet = pMap->eCode;
            break;
        }
    }

    ASSERT(nRet != pStart->eCode, "TXT: That was an unknown language!");

        return nRet;
}


/*
 Get the String name of an rtl_TextEncoding
 */
String NameFromCharSet(rtl_TextEncoding nChrSet)
{
    const CharSetNameMap *pStart = GetCharSetNameMap();
    const char *pRet = pStart->pName;

    for(const CharSetNameMap *pMap = pStart; pMap->pName; ++pMap)
    {
        if (nChrSet == pMap->eCode)
        {
            pRet = pMap->pName;
            break;
        }
    }

    ASSERT(pRet != pStart->pName, "TXT: That was an unknown language!");

    return String::CreateFromAscii(pRet);
}

// for the automatic conversion (mail/news/...)
// The user data contains the options for the ascii import/export filter.
// The format is:
//      1. CharSet - as ascii chars
//      2. LineEnd - as CR/LR/CRLF
//      3. Fontname
//      4. Language
// the delimetercharacter is ","
//

void SwAsciiOptions::ReadUserData( const String& rStr )
{
        xub_StrLen nToken = 0;
        sal_uInt16 nCnt = 0;
        String sToken;
        do {
                if( 0 != (sToken = rStr.GetToken( 0, ',', nToken )).Len() )
                {
                        switch( nCnt )
                        {
                        case 0:         // CharSet
                eCharSet = CharSetFromName(sToken);
                                break;
                        case 1:         // LineEnd
                                if( sToken.EqualsIgnoreCaseAscii( "CRLF" ))
                                        eCRLF_Flag = LINEEND_CRLF;
                                else if( sToken.EqualsIgnoreCaseAscii( "LF" ))
                                        eCRLF_Flag = LINEEND_LF;
                                else
                                        eCRLF_Flag = LINEEND_CR;
                                break;
                        case 2:         // fontname
                                sFont = sToken;
                                break;
                        case 3:         // Language
                nLanguage = MsLangId::convertIsoStringToLanguage( sToken );
                                break;
                        }
                }
                ++nCnt;
        } while( STRING_NOTFOUND != nToken );
}

void SwAsciiOptions::WriteUserData( String& rStr )
{
        // 1. charset
        rStr = NameFromCharSet(eCharSet);
        rStr += ',';

        // 2. LineEnd
        switch(eCRLF_Flag)
        {
        case LINEEND_CRLF:
            rStr.AppendAscii( "CRLF" );
            break;
        case LINEEND_CR:
            rStr.AppendAscii(  "CR" );
            break;
        case LINEEND_LF:
            rStr.AppendAscii(  "LF" );
            break;
        }
        rStr += ',';

        // 3. Fontname
        rStr += sFont;
        rStr += ',';

        // 4. Language
        if (nLanguage)
        {
        rtl::OUString sTmp = MsLangId::convertLanguageToIsoString( nLanguage );
        rStr += (String)sTmp;
        }
        rStr += ',';
}

extern "C" { static void SAL_CALL thisModule() {} }

static oslGenericFunction GetMswordLibSymbol( const char *pSymbol )
{
    static ::osl::Module aModule;
        static sal_Bool bLoaded = sal_False;
    static ::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "msword" ) ) );
        if (!bLoaded)
                bLoaded = SvLibrary::LoadModule( aModule, aLibName, &thisModule );
        if (bLoaded)
        return aModule.getFunctionSymbol( ::rtl::OUString::createFromAscii( pSymbol ) );
    return NULL;
}

Reader* GetRTFReader()
{
    FnGetReader pFunction = reinterpret_cast<FnGetReader>( GetMswordLibSymbol( "ImportRTF" ) );

    if ( pFunction )
        return (*pFunction)();

    return NULL;
}

void GetRTFWriter( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    FnGetWriter pFunction = reinterpret_cast<FnGetWriter>( GetMswordLibSymbol( "ExportRTF" ) );

    if ( pFunction )
        (*pFunction)( rFltName, rBaseURL, xRet );
    else
        xRet = WriterRef(0);
}

Reader* GetWW8Reader()
{
    FnGetReader pFunction = reinterpret_cast<FnGetReader>( GetMswordLibSymbol( "ImportDOC" ) );

    if ( pFunction )
        return (*pFunction)();

    return NULL;
}

void GetWW8Writer( const String& rFltName, const String& rBaseURL, WriterRef& xRet )
{
    FnGetWriter pFunction = reinterpret_cast<FnGetWriter>( GetMswordLibSymbol( "ExportDOC" ) );

    if ( pFunction )
        (*pFunction)( rFltName, rBaseURL, xRet );
    else
        xRet = WriterRef(0);
}

typedef sal_uLong ( __LOADONCALLAPI *SaveOrDel )( SfxObjectShell&, SotStorage&, sal_Bool, const String& );
typedef sal_uLong ( __LOADONCALLAPI *GetSaveWarning )( SfxObjectShell& );

sal_uLong SaveOrDelMSVBAStorage( SfxObjectShell& rDoc, SotStorage& rStor, sal_Bool bSaveInto, const String& rStorageName )
{
    SaveOrDel pFunction = reinterpret_cast<SaveOrDel>( GetMswordLibSymbol( "SaveOrDelMSVBAStorage_ww8" ) );
    if( pFunction )
                return pFunction( rDoc, rStor, bSaveInto, rStorageName );
        return ERRCODE_NONE;
}

sal_uLong GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS )
{
    GetSaveWarning pFunction = reinterpret_cast<GetSaveWarning>( GetMswordLibSymbol( "GetSaveWarningOfMSVBAStorage_ww8" ) );
    if( pFunction )
                        return pFunction( rDocS );
        return ERRCODE_NONE;
}


