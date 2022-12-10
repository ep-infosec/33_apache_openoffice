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
#include "precompiled_sc.hxx"



#include <svl/smplhint.hxx>

#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;


#include "nameuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "convuno.hxx"
#include "targuno.hxx"
#include "tokenuno.hxx"
#include "tokenarray.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "rangenam.hxx"
//CHINA001 #include "namecrea.hxx"		// NAME_TOP etc.
#include "unoguard.hxx"
#include "unonames.hxx"

#include "scui_def.hxx" //CHINA001

//------------------------------------------------------------------------

const SfxItemPropertyMapEntry* lcl_GetNamedRangeMap()
{
    static SfxItemPropertyMapEntry aNamedRangeMap_Impl[] =
	{
        {MAP_CHAR_LEN(SC_UNO_LINKDISPBIT),      0,  &getCppuType((uno::Reference<awt::XBitmap>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNO_LINKDISPNAME),     0,  &getCppuType((rtl::OUString*)0),                beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TOKENINDEX),   0,  &getCppuType((sal_Int32*)0),                    beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ISSHAREDFMLA), 0,  &getBooleanCppuType(),                          0, 0 },
        {0,0,0,0,0,0}
	};
	return aNamedRangeMap_Impl;
}

//------------------------------------------------------------------------

#define SCNAMEDRANGEOBJ_SERVICE		"com.sun.star.sheet.NamedRange"

SC_SIMPLE_SERVICE_INFO( ScLabelRangeObj, "ScLabelRangeObj", "com.sun.star.sheet.LabelRange" )
SC_SIMPLE_SERVICE_INFO( ScLabelRangesObj, "ScLabelRangesObj", "com.sun.star.sheet.LabelRanges" )
SC_SIMPLE_SERVICE_INFO( ScNamedRangesObj, "ScNamedRangesObj", "com.sun.star.sheet.NamedRanges" )

//------------------------------------------------------------------------

sal_Bool lcl_UserVisibleName( const ScRangeData* pData )
{
	//!	als Methode an ScRangeData

	return ( pData && !pData->HasType( RT_DATABASE ) && !pData->HasType( RT_SHARED ) );
}

//------------------------------------------------------------------------

ScNamedRangeObj::ScNamedRangeObj(ScDocShell* pDocSh, const String& rNm, const String& rScopeName) :
	pDocShell( pDocSh ),
	aName( rNm ),
	aScopeName(rScopeName)
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScNamedRangeObj::~ScNamedRangeObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScNamedRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Ref-Update interessiert nicht

	if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
		pDocShell = NULL;		// ungueltig geworden
}

// Hilfsfuntionen

ScRangeData* ScNamedRangeObj::GetRangeData_Impl()
{
	ScRangeData* pRet = NULL;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nPos = 0;
			SCTAB nameScope = MAXTABCOUNT;
			if ( aScopeName != EMPTY_STRING )
			{
				pDocShell->GetDocument()->GetTable( aScopeName, nameScope );
			}
			if (pNames->SearchName( aName, nPos, nameScope ))
			{
				pRet = (*pNames)[nPos];
				pRet->ValidateTabRefs();		// adjust relative tab refs to valid tables
			}
		}
	}
	return pRet;
}

// sheet::XNamedRange

void ScNamedRangeObj::Modify_Impl( const String* pNewRangeName, const ScTokenArray* pNewTokens, const String* pNewContent,
									const ScAddress* pNewPos, const sal_uInt16* pNewType,
                                    const formula::FormulaGrammar::Grammar eGrammar, const String* pNewScopeName )
{
       if (pDocShell)
       {
              ScDocument* pDoc = pDocShell->GetDocument();
              ScRangeName* pNames = pDoc->GetRangeName();
              if (pNames)
              {
                     sal_uInt16 nPos = 0;
                     SCTAB nameScope = MAXTABCOUNT;
                     if (aScopeName != EMPTY_STRING ) 
                         pDoc->GetTable(aScopeName, nameScope);
					 
                     if (pNames->SearchName( aName, nPos, nameScope ))
                     {
                            SCTAB newNameScope = MAXTABCOUNT;
                            if (pNewScopeName && *pNewScopeName != EMPTY_STRING && !pDoc->GetTable(*pNewScopeName, newNameScope))
                                return;
							//added for namerange renew
							else if (!pNewScopeName || *pNewScopeName == EMPTY_STRING )
								newNameScope = nameScope;
							//end of add
                            
                            ScRangeData* pOld = (*pNames)[nPos];

                            String aInsName(pOld->GetName());
                            if (pNewRangeName)
                                aInsName = *pNewRangeName;
                            String aContent;                            // Content string based =>
                            pOld->GetSymbol( aContent, eGrammar);   // no problems with changed positions and such.
                            if (pNewContent)
                                aContent = *pNewContent;
                            ScAddress aPos(pOld->GetPos());
                            if (pNewPos)
                                aPos = *pNewPos;
                            sal_uInt16 nType = pOld->GetType();
                            if (pNewType)
                                nType = *pNewType;

                            ScRangeData* pNew = NULL;
                            if ( pNewTokens )
                                pNew = new ScRangeData( pDoc, aInsName, *pNewTokens, aPos, nType );
                            else
                                pNew = new ScRangeData( pDoc, aInsName, aContent, aPos, nType, eGrammar );
                            pNew->SetIndex( pOld->GetIndex() );
                            pNew->SetRangeScope(newNameScope);

							const bool bSupportUndo(!pDoc->IsImportingXML());
							if ( bSupportUndo )
							{
								ScRangeName* pNewRanges = new ScRangeName( *pNames );
								pNewRanges->AtFree( nPos );
								if ( pNewRanges->Insert(pNew) )
								{
									 ScDocFunc aFunc(*pDocShell);
									 aFunc.SetNewRangeNames( pNewRanges, sal_True );
									 aName = aInsName;	//! broadcast?
									 aScopeName = pNewScopeName ? *pNewScopeName : aScopeName;
								}
								else
								{
									 delete pNew;		//! uno::Exception/Fehler oder so
									 delete pNewRanges;
								}
							}
							else
							{
								pNames->AtFree( nPos );
								if ( pNames->Insert(pNew) )
								{
									 ScDocFunc aFunc(*pDocShell);
									 aFunc.SetNewRangeNames( pNames, sal_True );
									 aName = aInsName;	//! broadcast?
									 aScopeName = pNewScopeName ? *pNewScopeName : aScopeName;
								}
								else
								{
									 delete pNew;		//! uno::Exception/Fehler oder so									 
								}
							}
                       }
              }
       }
}


rtl::OUString SAL_CALL ScNamedRangeObj::getName() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return aName;
}

void SAL_CALL ScNamedRangeObj::setName( const rtl::OUString& aNewName )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	//!	Formeln anpassen ?????

	String aNewStr(aNewName);
    // GRAM_PODF_A1 for API compatibility.
	Modify_Impl( &aNewStr, NULL, NULL, NULL, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );

	if ( aName != aNewStr )					// some error occurred...
		throw uno::RuntimeException();		// no other exceptions specified
}
rtl::OUString SAL_CALL ScNamedRangeObj::getScopeName() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return aScopeName;
}

void SAL_CALL ScNamedRangeObj::setScopeAndRangeName( const rtl::OUString& aNewScopeName, const rtl::OUString& aNewRangeName )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	//!	Formeln anpassen ?????

	String aNewRangeStr(aNewRangeName);
	String aNewScopeStr(aNewScopeName);
    // GRAM_PODF_A1 for API compatibility.
	Modify_Impl( &aNewRangeStr, NULL, NULL, NULL, NULL,formula::FormulaGrammar::GRAM_PODF_A1,  aNewScopeName.getLength() == 0 ? NULL : &aNewScopeStr);

	if ( aScopeName != aNewScopeStr || aName != aNewRangeStr  )					// some error occurred...
		throw uno::RuntimeException();		// no other exceptions specified
}


rtl::OUString SAL_CALL ScNamedRangeObj::getContent() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
   	String aContent;
	ScRangeData* pData = GetRangeData_Impl();
	if (pData)
        // GRAM_PODF_A1 for API compatibility.
        pData->GetSymbol( aContent,formula::FormulaGrammar::GRAM_PODF_A1);
	return aContent;
}

void SAL_CALL ScNamedRangeObj::setContent( const rtl::OUString& aContent )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	String aContStr(aContent);
    // GRAM_PODF_A1 for API compatibility.
	Modify_Impl( NULL, NULL, &aContStr, NULL, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );
}

void ScNamedRangeObj::SetContentWithGrammar( const ::rtl::OUString& aContent,
                                    const formula::FormulaGrammar::Grammar eGrammar )
                                throw(::com::sun::star::uno::RuntimeException)
{
    String aContStr(aContent);
    Modify_Impl( NULL, NULL, &aContStr, NULL, NULL, eGrammar );
}

table::CellAddress SAL_CALL ScNamedRangeObj::getReferencePosition()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScAddress aPos;
	ScRangeData* pData = GetRangeData_Impl();
	if (pData)
		aPos = pData->GetPos();
	table::CellAddress aAddress;
	aAddress.Column	= aPos.Col();
	aAddress.Row	= aPos.Row();
	aAddress.Sheet	= aPos.Tab();
	if (pDocShell)
	{
		SCTAB nDocTabs = pDocShell->GetDocument()->GetTableCount();
		if ( aAddress.Sheet >= nDocTabs && nDocTabs > 0 )
		{
			//	Even after ValidateTabRefs, the position can be invalid if
			//	the content points to preceding tables. The resulting string
			//	is invalid in any case, so the position is just shifted.
			aAddress.Sheet = nDocTabs - 1;
		}
	}
	return aAddress;
}

void SAL_CALL ScNamedRangeObj::setReferencePosition( const table::CellAddress& aReferencePosition )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScAddress aPos( (SCCOL)aReferencePosition.Column, (SCROW)aReferencePosition.Row, aReferencePosition.Sheet );
    // GRAM_PODF_A1 for API compatibility.
	Modify_Impl( NULL, NULL, NULL, &aPos, NULL,formula::FormulaGrammar::GRAM_PODF_A1 );
}

sal_Int32 SAL_CALL ScNamedRangeObj::getType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Int32 nType=0;
	ScRangeData* pData = GetRangeData_Impl();
	if (pData)
	{
        // do not return internal RT_* flags
        // see property 'IsSharedFormula' for RT_SHARED
		if ( pData->HasType(RT_CRITERIA) )	nType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
		if ( pData->HasType(RT_PRINTAREA) )	nType |= sheet::NamedRangeFlag::PRINT_AREA;
		if ( pData->HasType(RT_COLHEADER) )	nType |= sheet::NamedRangeFlag::COLUMN_HEADER;
		if ( pData->HasType(RT_ROWHEADER) )	nType |= sheet::NamedRangeFlag::ROW_HEADER;
	}
	return nType;
}

void SAL_CALL ScNamedRangeObj::setType( sal_Int32 nUnoType ) throw(uno::RuntimeException)
{
    // see property 'IsSharedFormula' for RT_SHARED
	ScUnoGuard aGuard;
	sal_uInt16 nNewType = RT_NAME;
	if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )	nNewType |= RT_CRITERIA;
	if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )			nNewType |= RT_PRINTAREA;
	if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )		nNewType |= RT_COLHEADER;
	if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )			nNewType |= RT_ROWHEADER;

    // GRAM_PODF_A1 for API compatibility.
	Modify_Impl( NULL, NULL, NULL, NULL, &nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
}

// XFormulaTokens

uno::Sequence<sheet::FormulaToken> SAL_CALL ScNamedRangeObj::getTokens() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence<sheet::FormulaToken> aSequence;
    ScRangeData* pData = GetRangeData_Impl();
    if (pData && pDocShell)
    {
        ScTokenArray* pTokenArray = pData->GetCode();
        if ( pTokenArray )
            (void)ScTokenConversion::ConvertToTokenSequence( *pDocShell->GetDocument(), aSequence, *pTokenArray );
    }
    return aSequence;
}

void SAL_CALL ScNamedRangeObj::setTokens( const uno::Sequence<sheet::FormulaToken>& rTokens ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if( pDocShell )
    {
        ScTokenArray aTokenArray;
        (void)ScTokenConversion::ConvertToTokenArray( *pDocShell->GetDocument(), aTokenArray, rTokens );
        // GRAM_PODF_A1 for API compatibility.
        Modify_Impl( NULL, &aTokenArray, NULL, NULL, NULL, formula::FormulaGrammar::GRAM_PODF_A1 );
    }
}


// XCellRangeSource

uno::Reference<table::XCellRange> SAL_CALL ScNamedRangeObj::getReferredCells()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScRange aRange;
	ScRangeData* pData = GetRangeData_Impl();
    if ( pData && pData->IsValidReference( aRange ) )
	{
		//!	static Funktion um ScCellObj/ScCellRangeObj zu erzeugen am ScCellRangeObj ???

		if ( aRange.aStart == aRange.aEnd )
			return new ScCellObj( pDocShell, aRange.aStart );
		else
			return new ScCellRangeObj( pDocShell, aRange );
	}
	return NULL;
}

// beans::XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScNamedRangeObj::getPropertySetInfo()
														throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	static uno::Reference< beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( lcl_GetNamedRangeMap() ));
	return aRef;
}

void SAL_CALL ScNamedRangeObj::setPropertyValue(
                        const rtl::OUString& rPropertyName, const uno::Any& aValue )
				throw(beans::UnknownPropertyException, beans::PropertyVetoException,
						lang::IllegalArgumentException, lang::WrappedTargetException,
						uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( rPropertyName.equalsAscii( SC_UNONAME_ISSHAREDFMLA ) )
    {
        bool bIsShared = false;
        if( aValue >>= bIsShared )
        {
            sal_uInt16 nNewType = bIsShared ? RT_SHARED : RT_NAME;
            Modify_Impl( NULL, NULL, NULL, NULL, &nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );
        }
    }
}

uno::Any SAL_CALL ScNamedRangeObj::getPropertyValue( const rtl::OUString& rPropertyName )
				throw(beans::UnknownPropertyException, lang::WrappedTargetException,
						uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Any aRet;
    if ( rPropertyName.equalsAscii( SC_UNO_LINKDISPBIT ) )
	{
		//	no target bitmaps for individual entries (would be all equal)
		// ScLinkTargetTypeObj::SetLinkTargetBitmap( aRet, SC_LINKTARGETTYPE_RANGENAME );
	}
    else if ( rPropertyName.equalsAscii( SC_UNO_LINKDISPNAME ) )
		aRet <<= rtl::OUString( aName );
    else if ( rPropertyName.equalsAscii( SC_UNONAME_TOKENINDEX ) )
    {
        // get index for use in formula tokens (read-only)
        ScRangeData* pData = GetRangeData_Impl();
        if (pData)
            aRet <<= static_cast<sal_Int32>(pData->GetIndex());
    }
    else if ( rPropertyName.equalsAscii( SC_UNONAME_ISSHAREDFMLA ) )
    {
        if( ScRangeData* pData = GetRangeData_Impl() )
            aRet <<= static_cast< bool >( pData->HasType( RT_SHARED ) );
    }
	return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScNamedRangeObj )

// lang::XServiceInfo

rtl::OUString SAL_CALL ScNamedRangeObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScNamedRangeObj" ) );
}

sal_Bool SAL_CALL ScNamedRangeObj::supportsService( const rtl::OUString& rServiceName )
													throw(uno::RuntimeException)
{
    return rServiceName.equalsAscii( SCNAMEDRANGEOBJ_SERVICE ) ||
           rServiceName.equalsAscii( SCLINKTARGET_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScNamedRangeObj::getSupportedServiceNames()
													throw(uno::RuntimeException)
{
	uno::Sequence<rtl::OUString> aRet(2);
    aRet[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SCNAMEDRANGEOBJ_SERVICE ) );
    aRet[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SCLINKTARGET_SERVICE ) );
	return aRet;
}


// XUnoTunnel

sal_Int64 SAL_CALL ScNamedRangeObj::getSomething(
				const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
	if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
									rId.getConstArray(), 16 ) )
	{
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
	}
	return 0;
}

// static
const uno::Sequence<sal_Int8>& ScNamedRangeObj::getUnoTunnelId()
{
	static uno::Sequence<sal_Int8> * pSeq = 0;
	if( !pSeq )
	{
		osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
		if( !pSeq )
		{
			static uno::Sequence< sal_Int8 > aSeq( 16 );
			rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
			pSeq = &aSeq;
		}
	}
	return *pSeq;
}

// static
ScNamedRangeObj* ScNamedRangeObj::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
	ScNamedRangeObj* pRet = NULL;
	uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
	if (xUT.is())
        pRet = reinterpret_cast<ScNamedRangeObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
	return pRet;
}

//------------------------------------------------------------------------

ScNamedRangesObj::ScNamedRangesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScNamedRangesObj::~ScNamedRangesObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScNamedRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Referenz-Update interessiert hier nicht

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// sheet::XNamedRanges

ScNamedRangeObj* ScNamedRangesObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nCount = pNames->GetCount();
			sal_uInt16 nPos = 0;
			for (sal_uInt16 i=0; i<nCount; i++)
			{
				ScRangeData* pData = (*pNames)[i];
				if (lcl_UserVisibleName(pData))			// interne weglassen
				{
					if ( nPos == nIndex )
						return new ScNamedRangeObj( pDocShell, pData->GetName(), pData->GetScopeSheetName() );
					++nPos;
				}
			}
		}
	}
	return NULL;
}

ScNamedRangeObj* ScNamedRangesObj::GetObjectByName_Impl(const rtl::OUString& aName)
{
	if ( pDocShell && hasByName(aName) )
		return new ScNamedRangeObj( pDocShell, String(aName) );
	return NULL;
}

ScNamedRangeObj* ScNamedRangesObj::GetObjectByScopeName_Impl(const ::rtl::OUString& aScopeName, const ::rtl::OUString& aRangeName)
{
     if ( pDocShell && hasByScopeName(aScopeName, aRangeName) )
		return new ScNamedRangeObj( pDocShell, String(aRangeName),String(aScopeName) );
	return NULL;
}
void ScNamedRangesObj::ImplAddNewByScopeAndName(SCTAB aScope, const ::rtl::OUString& aRangeName, const ::rtl::OUString& aContent,
								const ::com::sun::star::table::CellAddress& aPosition, sal_Int32 nUnoType) throw(uno::RuntimeException)
{
	ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, aPosition.Sheet );

	sal_uInt16 nNewType = RT_NAME;
	if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )	nNewType |= RT_CRITERIA;
	if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )			nNewType |= RT_PRINTAREA;
	if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )		nNewType |= RT_COLHEADER;
	if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )			nNewType |= RT_ROWHEADER;

	bool bDone = false;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangeName* pNames = pDoc->GetRangeName();
              sal_uInt16 nIndex = 0;
              String aNameStr(aRangeName);
	       String aContStr(aContent);
		if (pNames && !pNames->SearchName(aNameStr, nIndex,aScope))
		{
                   
            // GRAM_PODF_A1 for API compatibility.
                   ScRangeData* pNew = new ScRangeData( pDoc, aNameStr, aContStr,
												aPos, nNewType,formula::FormulaGrammar::GRAM_PODF_A1 );//GRAM_ODFF,//

                   pNew->SetRangeScope(aScope);
			
				   const bool bSupportUndo(!pDoc->IsImportingXML());
				   if ( bSupportUndo )
				   {
					   ScRangeName* pNewRanges = new ScRangeName( *pNames );
					   if ( pNewRanges->Insert(pNew) )
					   {
							ScDocFunc aFunc(*pDocShell);
							aFunc.SetNewRangeNames( pNewRanges, sal_True );
							bDone = true;
					   }
					   else
					   {
							delete pNew;
							delete pNewRanges;
					   }
				   }
				   else
				   {					   
					   if ( pNames->Insert(pNew) )
					   {
							ScDocFunc aFunc(*pDocShell);
							aFunc.SetNewRangeNames( pNames, sal_True );
							bDone = true;
					   }
					   else
					   {
							delete pNew;
					   }
				   }
		}
	}

	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScNamedRangesObj::addNewByName( const rtl::OUString& aName,
		const rtl::OUString& aContent, const table::CellAddress& aPosition,
		sal_Int32 nUnoType ) throw(uno::RuntimeException)
{
       ScUnoGuard aGuard;
       ImplAddNewByScopeAndName(MAXTABCOUNT, aName, aContent, aPosition, nUnoType);
}


void SAL_CALL ScNamedRangesObj::addNewByScopeName( const rtl::OUString& aScopeName,const rtl::OUString& aRangeName,
		const rtl::OUString& aContent, const table::CellAddress& aPosition,
		sal_Int32 nUnoType ) throw(uno::RuntimeException)
{
       ScUnoGuard aGuard;
       SCTAB scope = MAXTABCOUNT;
       if (aScopeName.getLength() != 0 && pDocShell && 
		          !pDocShell->GetDocument()->GetTable( String(aScopeName), scope ) )
           throw uno::RuntimeException();
       ImplAddNewByScopeAndName(scope, aRangeName, aContent, aPosition, nUnoType);
		
		
}

void SAL_CALL ScNamedRangesObj::addNewFromTitles( const table::CellRangeAddress& aSource,
									sheet::Border aBorder ) throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	//!	das darf kein enum sein, weil mehrere Bits gesetzt sein koennen !!!

	sal_Bool bTop    = ( aBorder == sheet::Border_TOP );
	sal_Bool bLeft   = ( aBorder == sheet::Border_LEFT );
	sal_Bool bBottom = ( aBorder == sheet::Border_BOTTOM );
	sal_Bool bRight  = ( aBorder == sheet::Border_RIGHT );

	ScRange aRange;
	ScUnoConversion::FillScRange( aRange, aSource );

	sal_uInt16 nFlags = 0;
	if (bTop)	 nFlags |= NAME_TOP;
	if (bLeft)	 nFlags |= NAME_LEFT;
	if (bBottom) nFlags |= NAME_BOTTOM;
	if (bRight)	 nFlags |= NAME_RIGHT;

	if (nFlags)
	{
		ScDocFunc aFunc(*pDocShell);
		aFunc.CreateNames( aRange, nFlags, sal_True );
	}
}

void ScNamedRangesObj::ImplRemoveByScopeAndName(SCTAB aScope, const ::rtl::OUString& aRangeName)
								throw(uno::RuntimeException)
{
	bool bDone = false;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nPos = 0;
			if (pNames->SearchName( String(aRangeName), nPos, aScope ))
				if ( lcl_UserVisibleName((*pNames)[nPos]) )
				{
                                   ScRangeName* pNewRanges = new ScRangeName(*pNames);
					pNewRanges->AtFree(nPos);
					ScDocFunc aFunc(*pDocShell);
                                   aFunc.SetNewRangeNames( pNewRanges, sal_True );
					bDone = true;
				}
		}
	}

	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

void SAL_CALL ScNamedRangesObj::removeByName( const rtl::OUString& aName )
												throw(uno::RuntimeException)
{
     ScUnoGuard aGuard;
     ImplRemoveByScopeAndName(MAXTABCOUNT, aName);	
}


void SAL_CALL ScNamedRangesObj::removeByScopeName( const ::rtl::OUString& aScopeName, const ::rtl::OUString& aRangeName )
								throw(uno::RuntimeException)
{
       ScUnoGuard aGuard;
       SCTAB scope = MAXTABCOUNT;
	if (aScopeName.getLength() != 0 && pDocShell && 
		                 !pDocShell->GetDocument()->GetTable( String(aScopeName), scope ))
           throw uno::RuntimeException();
	ImplRemoveByScopeAndName(scope, aRangeName);	
}


void SAL_CALL ScNamedRangesObj::outputList( const table::CellAddress& aOutputPosition )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScAddress aPos( (SCCOL)aOutputPosition.Column, (SCROW)aOutputPosition.Row, aOutputPosition.Sheet );
	if (pDocShell)
	{
		ScDocFunc aFunc(*pDocShell);
		aFunc.InsertNameList( aPos, sal_True );
	}
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScNamedRangesObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.NamedRangesEnumeration")));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScNamedRangesObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	long nRet = 0;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nCount = pNames->GetCount();
			for (sal_uInt16 i=0; i<nCount; i++)
				if (lcl_UserVisibleName( (*pNames)[i] ))	// interne weglassen
					++nRet;
		}
	}
	return nRet;
}

uno::Any SAL_CALL ScNamedRangesObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference< sheet::XNamedRange2 >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
	if ( xRange.is() )
        return uno::makeAny(xRange);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScNamedRangesObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ::getCppuType((const uno::Reference< sheet::XNamedRange2 >*)0);	// muss zu getByIndex passen
}

sal_Bool SAL_CALL ScNamedRangesObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

uno::Any SAL_CALL ScNamedRangesObj::getByName( const rtl::OUString& aName )
			throw(container::NoSuchElementException,
					lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference< sheet::XNamedRange2 >  xRange(GetObjectByName_Impl(aName));
	if ( xRange.is() )
        return uno::makeAny(xRange);
	else
		throw container::NoSuchElementException();
//    return uno::Any();
}

uno::Any SAL_CALL ScNamedRangesObj::getByScopeName( const rtl::OUString& aScopeName, const rtl::OUString& aRangeName )
			throw(container::NoSuchElementException,
					lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference< sheet::XNamedRange2 >  xRange(GetObjectByScopeName_Impl(aScopeName, aRangeName));
	if ( xRange.is() )
        return uno::makeAny(xRange);
	else
		throw container::NoSuchElementException();
//    return uno::Any();
}
uno::Sequence<rtl::OUString> SAL_CALL ScNamedRangesObj::getElementNames()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			long nVisCount = getCount();			// Namen mit lcl_UserVisibleName
			uno::Sequence<rtl::OUString> aSeq(nVisCount);
			rtl::OUString* pAry = aSeq.getArray();

			sal_uInt16 nCount = pNames->GetCount();
			sal_uInt16 nVisPos = 0;
			for (sal_uInt16 i=0; i<nCount; i++)
			{
				ScRangeData* pData = (*pNames)[i];
				if ( lcl_UserVisibleName(pData) )
					pAry[nVisPos++] = pData->GetName();
			}
//			DBG_ASSERT(nVisPos == nVisCount, "huch, verzaehlt?");
			return aSeq;
		}
	}
	return uno::Sequence<rtl::OUString>(0);
}

uno::Sequence< sheet::RangeScopeName > SAL_CALL ScNamedRangesObj::getElementScopeNames()
												throw(uno::RuntimeException)
{
       ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			long nVisCount = getCount();			// Namen mit lcl_UserVisibleName
			uno::Sequence<sheet::RangeScopeName> aSeq(nVisCount);
			sheet::RangeScopeName * pAry = aSeq.getArray();

			sal_uInt16 nCount = pNames->GetCount();
			sal_uInt16 nVisPos = 0;
			for (sal_uInt16 i=0; i<nCount; i++)
			{
				ScRangeData* pData = (*pNames)[i];
				if ( lcl_UserVisibleName(pData) )
				{
					pAry[nVisPos].RangeName = pData->GetName();
					pAry[nVisPos++].ScopeName = pData->GetScopeSheetName();
				}
			}
//			DBG_ASSERT(nVisPos == nVisCount, "huch, verzaehlt?");
			return aSeq;
		}
	}
	return uno::Sequence< sheet::RangeScopeName >(0);
}
sal_Bool SAL_CALL ScNamedRangesObj::hasByName( const rtl::OUString& aName )
										throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nPos = 0;
			if (pNames->SearchName( String(aName), nPos ))
				if ( lcl_UserVisibleName((*pNames)[nPos]) )
					return sal_True;
		}
	}
	return sal_False;
}

sal_Bool SAL_CALL ScNamedRangesObj::hasByScopeName( const ::rtl::OUString& aScopeName, const ::rtl::OUString& aRangeName)
										throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
	       SCTAB scope = MAXTABCOUNT;
	       if (aScopeName.getLength() != 0 && !pDocShell->GetDocument()->GetTable( String(aScopeName), scope ) )
	           return sal_False; 
			
		ScRangeName* pNames = pDocShell->GetDocument()->GetRangeName();
		if (pNames)
		{
			sal_uInt16 nPos = 0;
			if (pNames->SearchName( String(aRangeName), nPos, scope ))
				if ( lcl_UserVisibleName((*pNames)[nPos]) )
					return sal_True;
		}
	}
	return sal_False;
}
/** called from the XActionLockable interface methods on initial locking */
void ScNamedRangesObj::lock()
{
    pDocShell->GetDocument()->CompileNameFormula( sal_True ); // CreateFormulaString
}

/** called from the XActionLockable interface methods on final unlock */
void ScNamedRangesObj::unlock()
{
    pDocShell->GetDocument()->CompileNameFormula( sal_False ); // CompileFormulaString
}

// document::XActionLockable

sal_Bool ScNamedRangesObj::isActionLocked() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pDocShell->GetDocument()->GetNamedRangesLockCount() != 0;
}

void ScNamedRangesObj::addActionLock() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    ++nLockCount;
    if ( nLockCount == 1 )
    {
        lock();
    }
    pDoc->SetNamedRangesLockCount( nLockCount );
}

void ScNamedRangesObj::removeActionLock() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        --nLockCount;
        if ( nLockCount == 0 )
        {
            unlock();
        }
        pDoc->SetNamedRangesLockCount( nLockCount );
    }
}

void ScNamedRangesObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( nLock >= 0 )
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
        if ( nLock == 0 && nLockCount > 0 )
        {
            unlock();
        }
        if ( nLock > 0 && nLockCount == 0 )
        {
            lock();
        }
        pDoc->SetNamedRangesLockCount( nLock );
    }
}

sal_Int16 ScNamedRangesObj::resetActionLocks() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Int16 nLockCount = pDoc->GetNamedRangesLockCount();
    if ( nLockCount > 0 )
    {
        unlock();
    }
    pDoc->SetNamedRangesLockCount( 0 );
    return nLockCount;
}

//------------------------------------------------------------------------

ScLabelRangeObj::ScLabelRangeObj(ScDocShell* pDocSh, sal_Bool bCol, const ScRange& rR) :
	pDocShell( pDocSh ),
	bColumn( bCol ),
	aRange( rR )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScLabelRangeObj::~ScLabelRangeObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLabelRangeObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//!	Ref-Update !!!

	if ( rHint.ISA( SfxSimpleHint ) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
		pDocShell = NULL;		// ungueltig geworden
}

// Hilfsfuntionen

ScRangePair* ScLabelRangeObj::GetData_Impl()
{
	ScRangePair* pRet = NULL;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
		if (pList)
			pRet = pList->Find( aRange );
	}
	return pRet;
}

void ScLabelRangeObj::Modify_Impl( const ScRange* pLabel, const ScRange* pData )
{
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
		if (pOldList)
		{
			ScRangePairListRef xNewList(pOldList->Clone());
			ScRangePair* pEntry = xNewList->Find( aRange );
			if (pEntry)
			{
				xNewList->Remove( pEntry );		// nur aus der Liste entfernt, nicht geloescht

				if ( pLabel )
					pEntry->GetRange(0) = *pLabel;
				if ( pData )
					pEntry->GetRange(1) = *pData;

				xNewList->Join( *pEntry );
				delete pEntry;

				if (bColumn)
					pDoc->GetColNameRangesRef() = xNewList;
				else
					pDoc->GetRowNameRangesRef() = xNewList;

				pDoc->CompileColRowNameFormula();
				pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
				pDocShell->SetDocumentModified();

				//!	Undo ?!?! (hier und aus Dialog)

				if ( pLabel )
					aRange = *pLabel;	// Objekt anpassen, um Range wiederzufinden
			}
		}
	}
}

// sheet::XLabelRange

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getLabelArea()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	table::CellRangeAddress aRet;
	ScRangePair* pData = GetData_Impl();
	if (pData)
		ScUnoConversion::FillApiRange( aRet, pData->GetRange(0) );
	return aRet;
}

void SAL_CALL ScLabelRangeObj::setLabelArea( const table::CellRangeAddress& aLabelArea )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScRange aLabelRange;
	ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
	Modify_Impl( &aLabelRange, NULL );
}

table::CellRangeAddress SAL_CALL ScLabelRangeObj::getDataArea()
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	table::CellRangeAddress aRet;
	ScRangePair* pData = GetData_Impl();
	if (pData)
		ScUnoConversion::FillApiRange( aRet, pData->GetRange(1) );
	return aRet;
}

void SAL_CALL ScLabelRangeObj::setDataArea( const table::CellRangeAddress& aDataArea )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	ScRange aDataRange;
	ScUnoConversion::FillScRange( aDataRange, aDataArea );
	Modify_Impl( NULL, &aDataRange );
}

//------------------------------------------------------------------------

ScLabelRangesObj::ScLabelRangesObj(ScDocShell* pDocSh, sal_Bool bCol) :
	pDocShell( pDocSh ),
	bColumn( bCol )
{
	pDocShell->GetDocument()->AddUnoObject(*this);
}

ScLabelRangesObj::~ScLabelRangesObj()
{
	if (pDocShell)
		pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScLabelRangesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
	//	Referenz-Update interessiert hier nicht

	if ( rHint.ISA( SfxSimpleHint ) &&
			((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
	{
		pDocShell = NULL;		// ungueltig geworden
	}
}

// sheet::XLabelRanges

ScLabelRangeObj* ScLabelRangesObj::GetObjectByIndex_Impl(sal_uInt16 nIndex)
{
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
		if ( pList && nIndex < pList->Count() )
		{
			ScRangePair* pData = pList->GetObject(nIndex);
			if (pData)
				return new ScLabelRangeObj( pDocShell, bColumn, pData->GetRange(0) );
		}
	}
	return NULL;
}

void SAL_CALL ScLabelRangesObj::addNew( const table::CellRangeAddress& aLabelArea,
								const table::CellRangeAddress& aDataArea )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
		if (pOldList)
		{
			ScRangePairListRef xNewList(pOldList->Clone());

			ScRange aLabelRange;
			ScRange aDataRange;
			ScUnoConversion::FillScRange( aLabelRange, aLabelArea );
			ScUnoConversion::FillScRange( aDataRange,  aDataArea );
			xNewList->Join( ScRangePair( aLabelRange, aDataRange ) );

			if (bColumn)
				pDoc->GetColNameRangesRef() = xNewList;
			else
				pDoc->GetRowNameRangesRef() = xNewList;

			pDoc->CompileColRowNameFormula();
			pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
			pDocShell->SetDocumentModified();

			//!	Undo ?!?! (hier und aus Dialog)
		}
	}
}

void SAL_CALL ScLabelRangesObj::removeByIndex( sal_Int32 nIndex )
												throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	sal_Bool bDone = sal_False;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pOldList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();

		if ( pOldList && nIndex >= 0 && nIndex < (sal_Int32)pOldList->Count() )
		{
			ScRangePairListRef xNewList(pOldList->Clone());

			ScRangePair* pEntry = xNewList->GetObject( nIndex );
			if (pEntry)
			{
				xNewList->Remove( pEntry );
				delete pEntry;

				if (bColumn)
					pDoc->GetColNameRangesRef() = xNewList;
				else
					pDoc->GetRowNameRangesRef() = xNewList;

				pDoc->CompileColRowNameFormula();
				pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
				pDocShell->SetDocumentModified();
				bDone = sal_True;

				//!	Undo ?!?! (hier und aus Dialog)
			}
		}
	}
	if (!bDone)
		throw uno::RuntimeException();		// no other exceptions specified
}

// container::XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScLabelRangesObj::createEnumeration()
													throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.LabelRangesEnumeration")));
}

// container::XIndexAccess

sal_Int32 SAL_CALL ScLabelRangesObj::getCount() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	if (pDocShell)
	{
		ScDocument* pDoc = pDocShell->GetDocument();
		ScRangePairList* pList = bColumn ? pDoc->GetColNameRanges() : pDoc->GetRowNameRanges();
		if (pList)
			return pList->Count();
	}
	return 0;
}

uno::Any SAL_CALL ScLabelRangesObj::getByIndex( sal_Int32 nIndex )
							throw(lang::IndexOutOfBoundsException,
									lang::WrappedTargetException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference< sheet::XLabelRange >  xRange(GetObjectByIndex_Impl((sal_uInt16)nIndex));
	if ( xRange.is() )
        return uno::makeAny(xRange);
	else
		throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScLabelRangesObj::getElementType() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ::getCppuType((const uno::Reference< sheet::XLabelRange >*)0);	// muss zu getByIndex passen

}

sal_Bool SAL_CALL ScLabelRangesObj::hasElements() throw(uno::RuntimeException)
{
	ScUnoGuard aGuard;
	return ( getCount() != 0 );
}

//------------------------------------------------------------------------



