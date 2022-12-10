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



// INCLUDE --------------------------------------------------------------

#include <tools/debug.hxx>
#include <svl/zforlist.hxx>

#include "dpshttab.hxx"
#include "dptabres.hxx"
#include "document.hxx"
#include "collect.hxx"
#include "cell.hxx"
#include "dpcachetable.hxx"
#include "dpobject.hxx"
#include "globstr.hrc"
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include "dpglobal.hxx"
// End Comments
#include <com/sun/star/sheet/DataPilotFieldFilter.hpp>

#include <vector>
#include <set>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::std::vector;
using ::std::hash_map;
using ::std::hash_set;

// -----------------------------------------------------------------------

ScSheetDPData::ScSheetDPData( ScDocument* pD, const ScSheetSourceDesc& rDesc , long nCacheId) :
    ScDPTableData(pD, rDesc.GetCacheId( pD, nCacheId) ), // DataPilot Migration - Cache&&Performance
	aQuery ( rDesc.aQueryParam  ),
	pSpecial(NULL),
	bIgnoreEmptyRows( sal_False ),
	bRepeatIfEmpty(sal_False),
    aCacheTable( pD, GetCacheId() )     // base class ID is initialized with the GetCacheId call above
{
    SCSIZE nEntryCount( aQuery.GetEntryCount());
    pSpecial = new sal_Bool[nEntryCount];
    for (SCSIZE j = 0; j < nEntryCount; ++j )
    {
        ScQueryEntry& rEntry = aQuery.GetEntry(j);
        if (rEntry.bDoQuery)
        {
           pSpecial[j] = false;
            if (!rEntry.bQueryByString)
            {
                if (*rEntry.pStr == EMPTY_STRING &&
                   ((rEntry.nVal == SC_EMPTYFIELDS) || (rEntry.nVal == SC_NONEMPTYFIELDS)))
                    pSpecial[j] = true;
            }
            else
            {
		        sal_uInt32 nIndex = 0;
		        rEntry.bQueryByString =
					        !(pD->GetFormatTable()->
						        IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
            }
    	}
    }
}

ScSheetDPData::~ScSheetDPData()
{
	 delete[] pSpecial;
}

void ScSheetDPData::DisposeData()
{
    aCacheTable.clear();
}

long ScSheetDPData::GetColumnCount()
{
    CreateCacheTable();
    return aCacheTable.getColSize();
}

String ScSheetDPData::getDimensionName(long nColumn)
{
    CreateCacheTable();
	if (getIsDataLayoutDimension(nColumn))
	{
		//!	different internal and display names?
		//return "Data";
		return ScGlobal::GetRscString(STR_PIVOT_DATA);
	}
    else if (nColumn >= aCacheTable.getColSize())
	{
		DBG_ERROR("getDimensionName: invalid dimension");
		return String();
	}
	else
	{
		return  aCacheTable.getFieldName((SCCOL)nColumn);
	}
}

sal_Bool ScSheetDPData::IsDateDimension(long nDim)
{
	CreateCacheTable();
	long nColCount = aCacheTable.getColSize();
	if (getIsDataLayoutDimension(nDim))
	{
		return sal_False;
	}
	else if (nDim >= nColCount)
	{
		DBG_ERROR("IsDateDimension: invalid dimension");
		return sal_False;
	}
	else
	{
		return aCacheTable.GetCache()->IsDateDimension( nDim);
	}
}

sal_uLong ScSheetDPData::GetNumberFormat(long nDim)
{
	CreateCacheTable();
	if (getIsDataLayoutDimension(nDim))
	{
		return 0;
	}
	else if (nDim >= GetCacheTable().getColSize())
	{
		DBG_ERROR("GetNumberFormat: invalid dimension");
		return 0;
	}
	else
	{
		return GetCacheTable().GetCache()->GetNumberFormat( nDim );
	} 
}
sal_uInt32	ScDPTableData::GetNumberFormatByIdx( NfIndexTableOffset eIdx )
{
	if( !mpDoc )
		return 0;

	if ( SvNumberFormatter* pFormatter = mpDoc->GetFormatTable() )
		return pFormatter->GetFormatIndex( eIdx, LANGUAGE_SYSTEM );
	
	return 0;
}

sal_Bool ScSheetDPData::getIsDataLayoutDimension(long nColumn)
{
    CreateCacheTable();
    return (nColumn ==(long)( aCacheTable.getColSize()));
}

void ScSheetDPData::SetEmptyFlags( sal_Bool bIgnoreEmptyRowsP, sal_Bool bRepeatIfEmptyP )
{
    bIgnoreEmptyRows = bIgnoreEmptyRowsP;
    bRepeatIfEmpty   = bRepeatIfEmptyP;
}

bool ScSheetDPData::IsRepeatIfEmpty()
{
    return bRepeatIfEmpty;
}

void ScSheetDPData::CreateCacheTable()
{
    // Scan and store the data from the source range.
    if (!aCacheTable.empty())
        // already cached.
        return;

    aCacheTable.fillTable( aQuery, pSpecial, 
                                bIgnoreEmptyRows, bRepeatIfEmpty );
}

void ScSheetDPData::FilterCacheTable(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims)
{
    CreateCacheTable();
    aCacheTable.filterByPageDimension(
        rCriteria, (IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>()));
}

void ScSheetDPData::GetDrillDownData(const vector<ScDPCacheTable::Criterion>& rCriteria, const hash_set<sal_Int32>& rCatDims, Sequence< Sequence<Any> >& rData)
{
    CreateCacheTable();
    sal_Int32 nRowSize = aCacheTable.getRowSize();
    if (!nRowSize)
        return;

    aCacheTable.filterTable(
        rCriteria, rData, IsRepeatIfEmpty() ? rCatDims : hash_set<sal_Int32>());
}

void ScSheetDPData::CalcResults(CalcInfo& rInfo, bool bAutoShow)
{
    CreateCacheTable();
    CalcResultsFromCacheTable(aCacheTable, rInfo, bAutoShow);
}

const ScDPCacheTable& ScSheetDPData::GetCacheTable() const
{
    return aCacheTable;
}


// Wang Xu Ming -- 2009-8-5
// DataPilot Migration - Cache&&Performance
ScDPTableDataCache* ScSheetSourceDesc::CreateCache( ScDocument* pDoc , long nID ) const
{
    if ( pDoc )
    {
        ScDPTableDataCache* pCache =  GetExistDPObjectCache( pDoc );
        if ( pCache && ( nID < 0 || nID == pCache->GetId() ) )
            return pCache;

        sal_uLong nErrId = CheckValidate( pDoc );
        if ( !nErrId )
        {
            pCache = new ScDPTableDataCache( pDoc );

            pCache->InitFromDoc( pDoc, aSourceRange );
            pCache->SetId( nID );
            pDoc->AddDPObjectCache( pCache );

            DBG_TRACE1("Create a cache id = %d \n", pCache->GetId() );
        }
        else
            DBG_ERROR( "\n Error Create Cache" );
        return pCache;
    }
    return NULL;
}

ScDPTableDataCache* ScSheetSourceDesc::GetExistDPObjectCache ( ScDocument* pDoc  ) const
{
    return pDoc->GetUsedDPObjectCache( aSourceRange );
}
ScDPTableDataCache* ScSheetSourceDesc::GetCache( ScDocument* pDoc, long nID ) const
{
    ScDPTableDataCache* pCache = pDoc->GetDPObjectCache( nID );
    if ( NULL == pCache && pDoc )
        pCache = GetExistDPObjectCache( pDoc );
    if ( NULL == pCache )
        pCache = CreateCache( pDoc );    
    return pCache;
}

long ScSheetSourceDesc:: GetCacheId( ScDocument* pDoc, long nID ) const
{
	ScDPTableDataCache* pCache = GetCache( pDoc,  nID);
	if ( NULL == pCache )
		return -1;
	else 
		return pCache->GetId();
}

sal_uLong ScSheetSourceDesc::CheckValidate( ScDocument* pDoc ) const
{
    ScRange aSrcRange( aSourceRange);
    if ( !pDoc )
        return STR_ERR_DATAPILOTSOURCE;

    // #i116457# Empty column titles were allowed before 3.3, and might be useful for hidden columns with annotations.
    // Be compatible with 3.2: Allow empty titles, create columns with empty names, hide them in the dialogs.

    if( pDoc->IsBlockEmpty( aSrcRange.aStart.Tab(), aSrcRange.aStart.Col(), aSrcRange.aStart.Row()+1, aSrcRange.aEnd.Col(), aSrcRange.aEnd.Row() ) )
    {
        return STR_PIVOT_ONLYONEROWERR; 
    }
    return 0;
}
// End Comments

// -----------------------------------------------------------------------







