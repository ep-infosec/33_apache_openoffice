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
#include "precompiled_scfilt.hxx"

#include <rtl/random.h>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <unotools/saveopt.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "xecontent.hxx"
#include "xltracer.hxx"
#include "xeescher.hxx"
#include "xeformula.hxx"
#include "xehelper.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xepivot.hxx"
#include "xestyle.hxx"
#include "xeroot.hxx"

#include "excrecds.hxx"  // for filter manager
#include "tabprotection.hxx"
#include "document.hxx"
#include "scextopt.hxx"

using namespace ::com::sun::star;

// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eTextEnc, true )
{
    SvtSaveOptions aSaveOpt;
    mbRelUrl = mrMedium.IsRemote() ? aSaveOpt.IsSaveRelINet() : aSaveOpt.IsSaveRelFSys();
}

XclExpRootData::~XclExpRootData()
{
}

// ----------------------------------------------------------------------------

XclExpRoot::XclExpRoot( XclExpRootData& rExpRootData ) :
    XclRoot( rExpRootData ),
    mrExpData( rExpRootData )
{
}

XclExpTabInfo& XclExpRoot::GetTabInfo() const
{
    DBG_ASSERT( mrExpData.mxTabInfo.is(), "XclExpRoot::GetTabInfo - missing object (wrong BIFF?)" );
    return *mrExpData.mxTabInfo;
}

XclExpAddressConverter& XclExpRoot::GetAddressConverter() const
{
    DBG_ASSERT( mrExpData.mxAddrConv.is(), "XclExpRoot::GetAddressConverter - missing object (wrong BIFF?)" );
    return *mrExpData.mxAddrConv;
}

XclExpFormulaCompiler& XclExpRoot::GetFormulaCompiler() const
{
    DBG_ASSERT( mrExpData.mxFmlaComp.is(), "XclExpRoot::GetFormulaCompiler - missing object (wrong BIFF?)" );
    return *mrExpData.mxFmlaComp;
}

XclExpProgressBar& XclExpRoot::GetProgressBar() const
{
    DBG_ASSERT( mrExpData.mxProgress.is(), "XclExpRoot::GetProgressBar - missing object (wrong BIFF?)" );
    return *mrExpData.mxProgress;
}

XclExpSst& XclExpRoot::GetSst() const
{
    DBG_ASSERT( mrExpData.mxSst.is(), "XclExpRoot::GetSst - missing object (wrong BIFF?)" );
    return *mrExpData.mxSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    DBG_ASSERT( mrExpData.mxPalette.is(), "XclExpRoot::GetPalette - missing object (wrong BIFF?)" );
    return *mrExpData.mxPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    DBG_ASSERT( mrExpData.mxFontBfr.is(), "XclExpRoot::GetFontBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxFontBfr;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    DBG_ASSERT( mrExpData.mxNumFmtBfr.is(), "XclExpRoot::GetNumFmtBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxNumFmtBfr;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    DBG_ASSERT( mrExpData.mxXFBfr.is(), "XclExpRoot::GetXFBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxXFBfr;
}

XclExpLinkManager& XclExpRoot::GetGlobalLinkManager() const
{
    DBG_ASSERT( mrExpData.mxGlobLinkMgr.is(), "XclExpRoot::GetGlobalLinkManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxGlobLinkMgr;
}

XclExpLinkManager& XclExpRoot::GetLocalLinkManager() const
{
    DBG_ASSERT( GetLocalLinkMgrRef().is(), "XclExpRoot::GetLocalLinkManager - missing object (wrong BIFF?)" );
    return *GetLocalLinkMgrRef();
}

XclExpNameManager& XclExpRoot::GetNameManager() const
{
    DBG_ASSERT( mrExpData.mxNameMgr.is(), "XclExpRoot::GetNameManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxNameMgr;
}

XclExpObjectManager& XclExpRoot::GetObjectManager() const
{
    DBG_ASSERT( mrExpData.mxObjMgr.is(), "XclExpRoot::GetObjectManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxObjMgr;
}

XclExpFilterManager& XclExpRoot::GetFilterManager() const
{
    DBG_ASSERT( mrExpData.mxFilterMgr.is(), "XclExpRoot::GetFilterManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxFilterMgr;
}

XclExpPivotTableManager& XclExpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrExpData.mxPTableMgr.is(), "XclExpRoot::GetPivotTableManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxPTableMgr;
}

void XclExpRoot::InitializeConvert()
{
    mrExpData.mxTabInfo.reset( new XclExpTabInfo( GetRoot() ) );
    mrExpData.mxAddrConv.reset( new XclExpAddressConverter( GetRoot() ) );
    mrExpData.mxFmlaComp.reset( new XclExpFormulaCompiler( GetRoot() ) );
    mrExpData.mxProgress.reset( new XclExpProgressBar( GetRoot() ) );

    GetProgressBar().Initialize();
}

void XclExpRoot::InitializeGlobals()
{
    SetCurrScTab( SCTAB_GLOBAL );

    if( GetBiff() >= EXC_BIFF5 )
    {
        mrExpData.mxPalette.reset( new XclExpPalette( GetRoot() ) );
        mrExpData.mxFontBfr.reset( new XclExpFontBuffer( GetRoot() ) );
        mrExpData.mxNumFmtBfr.reset( new XclExpNumFmtBuffer( GetRoot() ) );
        mrExpData.mxXFBfr.reset( new XclExpXFBuffer( GetRoot() ) );
        mrExpData.mxGlobLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
        mrExpData.mxNameMgr.reset( new XclExpNameManager( GetRoot() ) );
    }

    if( GetBiff() == EXC_BIFF8 )
    {
        mrExpData.mxSst.reset( new XclExpSst );
        mrExpData.mxObjMgr.reset( new XclExpObjectManager( GetRoot() ) );
        mrExpData.mxFilterMgr.reset( new XclExpFilterManager( GetRoot() ) );
        mrExpData.mxPTableMgr.reset( new XclExpPivotTableManager( GetRoot() ) );
        // BIFF8: only one link manager for all sheets
        mrExpData.mxLocLinkMgr = mrExpData.mxGlobLinkMgr;
    }

    GetXFBuffer().Initialize();
    GetNameManager().Initialize();
}

void XclExpRoot::InitializeTable( SCTAB nScTab )
{
    SetCurrScTab( nScTab );
    if( GetBiff() == EXC_BIFF5 )
    {
        // local link manager per sheet
        mrExpData.mxLocLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
    }
}

void XclExpRoot::InitializeSave()
{
    GetPalette().Finalize();
    GetXFBuffer().Finalize();
}

XclExpRecordRef XclExpRoot::CreateRecord( sal_uInt16 nRecId ) const
{
    XclExpRecordRef xRec;
    switch( nRecId )
    {
        case EXC_ID_PALETTE:        xRec = mrExpData.mxPalette;     break;
        case EXC_ID_FONTLIST:       xRec = mrExpData.mxFontBfr;     break;
        case EXC_ID_FORMATLIST:     xRec = mrExpData.mxNumFmtBfr;   break;
        case EXC_ID_XFLIST:         xRec = mrExpData.mxXFBfr;       break;
        case EXC_ID_SST:            xRec = mrExpData.mxSst;         break;
        case EXC_ID_EXTERNSHEET:    xRec = GetLocalLinkMgrRef();    break;
        case EXC_ID_NAME:           xRec = mrExpData.mxNameMgr;     break;
    }
    DBG_ASSERT( xRec.is(), "XclExpRoot::CreateRecord - unknown record ID or missing object" );
    return xRec;
}

bool XclExpRoot::IsDocumentEncrypted() const
{
    // We need to encrypt the content when the document structure is protected.
    const ScDocProtection* pDocProt = GetDoc().GetDocProtection();
    if (pDocProt && pDocProt->isProtected() && pDocProt->isOptionEnabled(ScDocProtection::STRUCTURE))
        return true;

    if ( GetEncryptionData().getLength() > 0 )
        // Password is entered directly into the save dialog.
        return true;

    return false;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateEncryptionData( const ::rtl::OUString& aPass ) const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;

    if ( aPass.getLength() > 0 && aPass.getLength() < 16 )
    {
        TimeValue aTime;
        osl_getSystemTime( &aTime );
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_addBytes ( aRandomPool, &aTime, 8 ); 

        sal_uInt8 pnDocId[16];
        rtl_random_getBytes( aRandomPool, pnDocId, 16 );

        rtl_random_destroyPool( aRandomPool );

        sal_uInt16 pnPasswd[16];
        memset( pnPasswd, 0, sizeof( pnPasswd ) );
        for (xub_StrLen nChar = 0; nChar < aPass.getLength(); ++nChar )
            pnPasswd[nChar] = aPass.getStr()[nChar];

        ::msfilter::MSCodec_Std97 aCodec;
        aCodec.InitKey( pnPasswd, pnDocId );
        aEncryptionData = aCodec.GetEncryptionData();
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GetEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    SFX_ITEMSET_ARG( GetMedium().GetItemSet(), pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False );
    if ( pEncryptionDataItem )
        pEncryptionDataItem->GetValue() >>= aEncryptionData;
    else
    {
        // try to get the encryption data from the password
        SFX_ITEMSET_ARG( GetMedium().GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
        if ( pPasswordItem && pPasswordItem->GetValue().Len() )
            aEncryptionData = GenerateEncryptionData( pPasswordItem->GetValue() );
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > XclExpRoot::GenerateDefaultEncryptionData() const
{
    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( GetDefaultPassword().Len() > 0 )
        aEncryptionData = GenerateEncryptionData( GetDefaultPassword() );

    return aEncryptionData;
}

XclExpRootData::XclExpLinkMgrRef XclExpRoot::GetLocalLinkMgrRef() const
{
    return IsInGlobals() ? mrExpData.mxGlobLinkMgr : mrExpData.mxLocLinkMgr;
}

// ============================================================================

