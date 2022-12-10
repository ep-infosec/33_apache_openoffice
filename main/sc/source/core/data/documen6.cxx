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



#include "scitems.hxx"
#include <editeng/scripttypeitem.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "document.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "patattr.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"

using namespace com::sun::star;

#define SC_BREAKITER_SERVICE	"com.sun.star.i18n.BreakIterator"

//
//	this file is compiled with exceptions enabled
//	put functions here that need exceptions!
//

// -----------------------------------------------------------------------

const uno::Reference< i18n::XBreakIterator >& ScDocument::GetBreakIterator()
{
	if ( !pScriptTypeData )
		pScriptTypeData = new ScScriptTypeData;
    if ( !pScriptTypeData->xBreakIter.is() )
    {
        uno::Reference< uno::XInterface > xInterface = xServiceManager->createInstance(
                            ::rtl::OUString::createFromAscii( SC_BREAKITER_SERVICE ) );
        pScriptTypeData->xBreakIter = uno::Reference< i18n::XBreakIterator >( xInterface, uno::UNO_QUERY );
		DBG_ASSERT( pScriptTypeData->xBreakIter.is(), "can't get BreakIterator" );
	}
    return pScriptTypeData->xBreakIter;
}

sal_Bool ScDocument::HasStringWeakCharacters( const String& rString )
{
	if (rString.Len())
	{
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
		if ( xBreakIter.is() )
		{
			rtl::OUString aText = rString;
			sal_Int32 nLen = aText.getLength();

			sal_Int32 nPos = 0;
			do
			{
				sal_Int16 nType = xBreakIter->getScriptType( aText, nPos );
				if ( nType == i18n::ScriptType::WEAK )
					return sal_True;							// found

				nPos = xBreakIter->endOfScript( aText, nPos, nType );
			}
			while ( nPos >= 0 && nPos < nLen );
		}
	}

	return sal_False;		// none found
}

sal_uInt8 ScDocument::GetStringScriptType( const String& rString )
{

	sal_uInt8 nRet = 0;
	if (rString.Len())
	{
        uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
		if ( xBreakIter.is() )
		{
			rtl::OUString aText = rString;
			sal_Int32 nLen = aText.getLength();

			sal_Int32 nPos = 0;
			do
			{
				sal_Int16 nType = xBreakIter->getScriptType( aText, nPos );
				switch ( nType )
				{
					case i18n::ScriptType::LATIN:
						nRet |= SCRIPTTYPE_LATIN;
						break;
					case i18n::ScriptType::ASIAN:
						nRet |= SCRIPTTYPE_ASIAN;
						break;
					case i18n::ScriptType::COMPLEX:
						nRet |= SCRIPTTYPE_COMPLEX;
						break;
					// WEAK is ignored
				}
				nPos = xBreakIter->endOfScript( aText, nPos, nType );
			}
			while ( nPos >= 0 && nPos < nLen );
		}
	}
	return nRet;
}

sal_uInt8 ScDocument::GetCellScriptType( ScBaseCell* pCell, sal_uLong nNumberFormat )
{
	if ( !pCell )
		return 0;		// empty

	sal_uInt8 nStored = pCell->GetScriptType();
	if ( nStored != SC_SCRIPTTYPE_UNKNOWN )			// stored value valid?
		return nStored;								// use stored value

	String aStr;
	Color* pColor;
	ScCellFormat::GetString( pCell, nNumberFormat, aStr, &pColor, *xPoolHelper->GetFormTable() );

	sal_uInt8 nRet = GetStringScriptType( aStr );

	pCell->SetScriptType( nRet );		// store for later calls

	return nRet;
}

sal_uInt8 ScDocument::GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell )
{
	// if cell is not passed, take from document

	if (!pCell)
	{
		pCell = GetCell( ScAddress( nCol, nRow, nTab ) );
		if ( !pCell )
			return 0;		// empty
	}

	// if script type is set, don't have to get number formats

	sal_uInt8 nStored = pCell->GetScriptType();
	if ( nStored != SC_SCRIPTTYPE_UNKNOWN )			// stored value valid?
		return nStored;								// use stored value

	// include number formats from conditional formatting

	const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
	if (!pPattern) return 0;
	const SfxItemSet* pCondSet = NULL;
	if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
		pCondSet = GetCondResult( nCol, nRow, nTab );

	sal_uLong nFormat = pPattern->GetNumberFormat( xPoolHelper->GetFormTable(), pCondSet );
	return GetCellScriptType( pCell, nFormat );
}


