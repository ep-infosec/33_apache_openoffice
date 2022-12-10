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

#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>


#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

//REMOVE	#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE	#define SO2_DECL_SVINPLACEOBJECT_DEFINED
//REMOVE	SO2_DECL_REF(SvInPlaceObject)
//REMOVE	#endif

#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"
#include "chartlis.hxx"
#include "chartlock.hxx"
#include "refupdat.hxx"
#include <tools/globname.hxx>
#include <sot/exchange.hxx>

#include "miscuno.hxx"
#include "chart2uno.hxx"
#include "charthelper.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

void lcl_GetChartParameters( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            rtl::OUString& rRanges, chart::ChartDataRowSource& rDataRowSource,
            bool& rHasCategories, bool& rFirstCellAsLabel )
{
    rHasCategories = rFirstCellAsLabel = false;     // default if not in sequence

    uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );

    uno::Reference< chart2::data::XDataSource > xDataSource = xReceiver->getUsedData();
    uno::Reference< chart2::data::XDataProvider > xProvider = xChartDoc->getDataProvider();

    if ( xProvider.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs( xProvider->detectArguments( xDataSource ) );

        const beans::PropertyValue* pPropArray = aArgs.getConstArray();
        long nPropCount = aArgs.getLength();
        for (long i = 0; i < nPropCount; i++)
        {
            const beans::PropertyValue& rProp = pPropArray[i];
            String aPropName(rProp.Name);

            if (aPropName.EqualsAscii( "CellRangeRepresentation" ))
                rProp.Value >>= rRanges;
            else if (aPropName.EqualsAscii( "DataRowSource" ))
                rDataRowSource = (chart::ChartDataRowSource)ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( "HasCategories" ))
                rHasCategories = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
            else if (aPropName.EqualsAscii( "FirstCellAsLabel" ))
                rFirstCellAsLabel = ScUnoHelpFunctions::GetBoolFromAny( rProp.Value );
        }
    }
}

void lcl_SetChartParameters( const uno::Reference< chart2::data::XDataReceiver >& xReceiver,
            const rtl::OUString& rRanges, chart::ChartDataRowSource eDataRowSource,
            bool bHasCategories, bool bFirstCellAsLabel )
{
    if ( xReceiver.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs( 4 );
        aArgs[0] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
            uno::makeAny( rRanges ), beans::PropertyState_DIRECT_VALUE );
        aArgs[1] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("HasCategories"), -1,
            uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
        aArgs[2] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
            uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
        aArgs[3] = beans::PropertyValue(
            ::rtl::OUString::createFromAscii("DataRowSource"), -1,
            uno::makeAny( eDataRowSource ), beans::PropertyState_DIRECT_VALUE );
        xReceiver->setArguments( aArgs );
    }
}

// update charts after loading old document

void ScDocument::UpdateAllCharts()
{
    if ( !pDrawLayer || !pShell )
		return;

	sal_uInt16 nDataCount = pChartCollection->GetCount();
	if ( !nDataCount )
		return ;		// nothing to do

	sal_uInt16 nPos;

	for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
	{
		if (pTab[nTab])
		{
			SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
			DBG_ASSERT(pPage,"Page ?");

			ScRange aRange;
			SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
			SdrObject* pObject = aIter.Next();
			while (pObject)
			{
				if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
				{
					uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
					if ( xIPObj.is() )
					{
						String aIPName = ((SdrOle2Obj*)pObject)->GetPersistName();

						for (nPos=0; nPos<nDataCount; nPos++)
						{
							ScChartArray* pChartObj = (*pChartCollection)[nPos];
							if (pChartObj->GetName() == aIPName)
							{
                                ScRangeListRef aRanges = pChartObj->GetRangeList();
                                String sRangeStr;
                                aRanges->Format( sRangeStr, SCR_ABS_3D, this, GetAddressConvention() );

                                chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                                bool bHasCategories = pChartObj->HasRowHeaders();
                                bool bFirstCellAsLabel = pChartObj->HasColHeaders();

                                // Calc -> DataProvider
                                uno::Reference< chart2::data::XDataProvider > xDataProvider =
                                        new ScChart2DataProvider( this );
                                // Chart -> DataReceiver
                                uno::Reference< chart2::data::XDataReceiver > xReceiver;
                                uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                                if( xCompSupp.is())
                                    xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );
                                if( xReceiver.is())
                                {
                                    // connect
                                    xReceiver->attachDataProvider( xDataProvider );
                                    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier(
                                            pShell->GetModel(), uno::UNO_QUERY );
                                    xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

                                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource,
                                                            bHasCategories, bFirstCellAsLabel );
                                }

								ScChartListener* pCL = new ScChartListener(
									aIPName, this, pChartObj->GetRangeList() );
								pChartListenerCollection->Insert( pCL );
								pCL->StartListeningTo();
							}
						}
					}
				}
				pObject = aIter.Next();
			}
		}
	}

	pChartCollection->FreeAll();
}

sal_Bool ScDocument::HasChartAtPoint( SCTAB nTab, const Point& rPos, String* pName )
{
	if (pDrawLayer && pTab[nTab])
	{
		SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
		DBG_ASSERT(pPage,"Page ?");

		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
		SdrObject* pObject = aIter.Next();
		while (pObject)
		{
			if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
				 pObject->GetCurrentBoundRect().IsInside(rPos) )
			{
						// auch Chart-Objekte die nicht in der Collection sind

				if (IsChart(pObject))
				{
					if (pName)
						*pName = ((SdrOle2Obj*)pObject)->GetPersistName();
					return sal_True;
				}
			}
			pObject = aIter.Next();
		}
	}

	if (pName)
		pName->Erase();
	return sal_False;					// nix gefunden
}

void ScDocument::UpdateChartArea( const String& rChartName,
			const ScRange& rNewArea, sal_Bool bColHeaders, sal_Bool bRowHeaders,
			sal_Bool bAdd )
{
	ScRangeListRef aRLR( new ScRangeList );
	aRLR->Append( rNewArea );
	UpdateChartArea( rChartName, aRLR, bColHeaders, bRowHeaders, bAdd );
}

uno::Reference< chart2::XChartDocument > ScDocument::GetChartByName( const String& rChartName )
{
    uno::Reference< chart2::XChartDocument > xReturn;

    if (pDrawLayer)
    {
        sal_uInt16 nCount = pDrawLayer->GetPageCount();
        for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");

            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                        ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
                {
                    xReturn.set( ScChartHelper::GetChartFromSdrObject( pObject ) );
                    return xReturn;
                }
                pObject = aIter.Next();
            }
        }
    }
    return xReturn;
}
void ScDocument::GetChartRanges( const String& rChartName, ::std::vector< ScRangeList >& rRangesVector, ScDocument* pSheetNameDoc )
{
    rRangesVector.clear();
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if ( xChartDoc.is() )
    {
        uno::Sequence< rtl::OUString > aRangeStrings;
        ScChartHelper::GetChartRanges( xChartDoc, aRangeStrings );
        for( sal_Int32 nN=0; nN<aRangeStrings.getLength(); nN++ )
        {
            ScRangeList aRanges;
            aRanges.Parse( aRangeStrings[nN], pSheetNameDoc, SCA_VALID, pSheetNameDoc->GetAddressConvention() );
            rRangesVector.push_back(aRanges);
        }
    }
}

void ScDocument::SetChartRanges( const String& rChartName, const ::std::vector< ScRangeList >& rRangesVector )
{
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if ( xChartDoc.is() )
    {
        sal_Int32 nCount = static_cast<sal_Int32>( rRangesVector.size() );
        uno::Sequence< rtl::OUString > aRangeStrings(nCount);
        for( sal_Int32 nN=0; nN<nCount; nN++ )
        {
            ScRangeList aScRangeList( rRangesVector[nN] );
            String sRangeStr; // This range must be in Calc A1 format.
            aScRangeList.Format( sRangeStr, SCR_ABS_3D, this );
            aRangeStrings[nN]=sRangeStr;
        }
        ScChartHelper::SetChartRanges( xChartDoc, aRangeStrings );
    }
}

void ScDocument::GetOldChartParameters( const String& rName,
            ScRangeList& rRanges, sal_Bool& rColHeaders, sal_Bool& rRowHeaders )
{
    // used for undo of changing chart source area

    if (!pDrawLayer)
        return;

    sal_uInt16 nCount = pDrawLayer->GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rName )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                if ( xChartDoc.is() )
                {
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    rtl::OUString aRangesStr;
                    lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    rRanges.Parse( aRangesStr, this );
                    if ( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                    {
                        rRowHeaders = bHasCategories;
                        rColHeaders = bFirstCellAsLabel;
                    }
                    else
                    {
                        rColHeaders = bHasCategories;
                        rRowHeaders = bFirstCellAsLabel;
                    }
                }
                return;
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::UpdateChartArea( const String& rChartName,
			const ScRangeListRef& rNewList, sal_Bool bColHeaders, sal_Bool bRowHeaders,
			sal_Bool bAdd )
{
	if (!pDrawLayer)
		return;

	for (SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
	{
		SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
		DBG_ASSERT(pPage,"Page ?");

		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
		SdrObject* pObject = aIter.Next();
		while (pObject)
		{
			if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
					((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
			{
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if ( xChartDoc.is() && xReceiver.is() )
                {
                    ScRangeListRef aNewRanges;
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    rtl::OUString aRangesStr;
                    lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    sal_Bool bInternalData = xChartDoc->hasInternalDataProvider();

                    if ( bAdd && !bInternalData )
                    {
                        // append to old ranges, keep other settings

                        aNewRanges = new ScRangeList;
                        aNewRanges->Parse( aRangesStr, this );

                        sal_uLong nAddCount = rNewList->Count();
                        for ( sal_uLong nAdd=0; nAdd<nAddCount; nAdd++ )
                            aNewRanges->Append( *rNewList->GetObject(nAdd) );
                    }
                    else
                    {
                        // directly use new ranges (only eDataRowSource is used from old settings)

                        if ( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                        {
                            bHasCategories = bRowHeaders;
                            bFirstCellAsLabel = bColHeaders;
                        }
                        else
                        {
                            bHasCategories = bColHeaders;
                            bFirstCellAsLabel = bRowHeaders;
                        }
                        aNewRanges = rNewList;
                    }

                    if ( bInternalData && pShell )
                    {
                        // Calc -> DataProvider
                        uno::Reference< chart2::data::XDataProvider > xDataProvider = new ScChart2DataProvider( this );
                        xReceiver->attachDataProvider( xDataProvider );
                        uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier(
                                pShell->GetModel(), uno::UNO_QUERY );
                        xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );
                    }

                    String sRangeStr;
                    aNewRanges->Format( sRangeStr, SCR_ABS_3D, this, GetAddressConvention() );

                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    pChartListenerCollection->ChangeListening( rChartName, aNewRanges );

                    // ((SdrOle2Obj*)pObject)->GetNewReplacement();
                    // pObject->ActionChanged();

					return;			// nicht weitersuchen
				}
			}
			pObject = aIter.Next();
		}
	}
}

void ScDocument::UpdateChart( const String& rChartName )
{
	if (!pDrawLayer || bInDtorClear)
		return;
    uno::Reference< chart2::XChartDocument > xChartDoc( GetChartByName( rChartName ) );
    if( xChartDoc.is() )
    {
        try
        {
            uno::Reference< util::XModifiable > xModif( xChartDoc, uno::UNO_QUERY_THROW );
            if( apTemporaryChartLock.get() )
                apTemporaryChartLock->AlsoLockThisChart( uno::Reference< frame::XModel >( xModif, uno::UNO_QUERY ) );
            xModif->setModified( sal_True );
        }
        catch ( uno::Exception& )
        {
        }
    }

    // After the update, chart keeps track of its own data source ranges,
    // the listener doesn't need to listen anymore, except the chart has
    // an internal data provider.
    if ( !( xChartDoc.is() && xChartDoc->hasInternalDataProvider() ) && pChartListenerCollection )
    {
        pChartListenerCollection->ChangeListening( rChartName, new ScRangeList );
	}
}

void ScDocument::RestoreChartListener( const String& rName )
{
    // Read the data ranges from the chart object, and start listening to those ranges again
    // (called when a chart is saved, because then it might be swapped out and stop listening itself).

    uno::Reference< embed::XEmbeddedObject > xObject = FindOleObjectByName( rName );
    if ( xObject.is() )
    {
        uno::Reference< util::XCloseable > xComponent = xObject->getComponent();
        uno::Reference< chart2::XChartDocument > xChartDoc( xComponent, uno::UNO_QUERY );
        uno::Reference< chart2::data::XDataReceiver > xReceiver( xComponent, uno::UNO_QUERY );
        if ( xChartDoc.is() && xReceiver.is() && !xChartDoc->hasInternalDataProvider())
        {
            uno::Sequence<rtl::OUString> aRepresentations( xReceiver->getUsedRangeRepresentations() );
            ScRangeListRef aRanges = new ScRangeList;
            sal_Int32 nRangeCount = aRepresentations.getLength();
            for ( sal_Int32 i=0; i<nRangeCount; i++ )
            {
                ScRange aRange;
                ScAddress::Details aDetails(GetAddressConvention(), 0, 0);
                if ( aRange.ParseAny( aRepresentations[i], this, aDetails ) & SCA_VALID )
                    aRanges->Append( aRange );
            }

            pChartListenerCollection->ChangeListening( rName, aRanges );
        }
    }
}

void ScDocument::UpdateChartRef( UpdateRefMode eUpdateRefMode,
									SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
									SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
									SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
	if (!pDrawLayer)
		return;

	sal_uInt16 nChartCount = pChartListenerCollection->GetCount();
	for ( sal_uInt16 nIndex = 0; nIndex < nChartCount; nIndex++ )
	{
		ScChartListener* pChartListener =
			(ScChartListener*) (pChartListenerCollection->At(nIndex));
		ScRangeListRef aRLR( pChartListener->GetRangeList() );
		ScRangeListRef aNewRLR( new ScRangeList );
		sal_Bool bChanged = sal_False;
		sal_Bool bDataChanged = sal_False;
		for ( ScRangePtr pR = aRLR->First(); pR; pR = aRLR->Next() )
		{
			SCCOL theCol1 = pR->aStart.Col();
			SCROW theRow1 = pR->aStart.Row();
			SCTAB theTab1 = pR->aStart.Tab();
			SCCOL theCol2 = pR->aEnd.Col();
			SCROW theRow2 = pR->aEnd.Row();
			SCTAB theTab2 = pR->aEnd.Tab();
			ScRefUpdateRes eRes = ScRefUpdate::Update(
				this, eUpdateRefMode,
				nCol1,nRow1,nTab1, nCol2,nRow2,nTab2,
				nDx,nDy,nDz,
				theCol1,theRow1,theTab1,
				theCol2,theRow2,theTab2 );
			if ( eRes != UR_NOTHING )
			{
				bChanged = sal_True;
				aNewRLR->Append( ScRange(
					theCol1, theRow1, theTab1,
					theCol2, theRow2, theTab2 ));
				if ( eUpdateRefMode == URM_INSDEL
					&& !bDataChanged
					&& (eRes == UR_INVALID ||
						((pR->aEnd.Col() - pR->aStart.Col()
						!= theCol2 - theCol1)
					|| (pR->aEnd.Row() - pR->aStart.Row()
						!= theRow2 - theRow1)
					|| (pR->aEnd.Tab() - pR->aStart.Tab()
						!= theTab2 - theTab1))) )
				{
					bDataChanged = sal_True;
				}
			}
			else
				aNewRLR->Append( *pR );
		}
		if ( bChanged )
		{
#if 0
			if ( nDz != 0 )
			{	// #81844# sheet to be deleted or inserted or moved
				// => no valid sheet names for references right now
				pChartListener->ChangeListening( aNewRLR, bDataChanged );
				pChartListener->ScheduleSeriesRanges();
			}
			else
#endif
			{
//				SetChartRangeList( pChartListener->GetString(), aNewRLR );
//				pChartListener->ChangeListening( aNewRLR, bDataChanged );

                // Force the chart to be loaded now, so it registers itself for UNO events.
                // UNO broadcasts are done after UpdateChartRef, so the chart will get this
                // reference change.

                uno::Reference< embed::XEmbeddedObject > xIPObj = FindOleObjectByName( pChartListener->GetString() );
                svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                // After the change, chart keeps track of its own data source ranges,
                // the listener doesn't need to listen anymore, except the chart has
                // an internal data provider.
                bool bInternalDataProvider = false;
                if ( xIPObj.is() )
                {
                    try
                    {
                        uno::Reference< chart2::XChartDocument > xChartDoc( xIPObj->getComponent(), uno::UNO_QUERY_THROW );
                        bInternalDataProvider = xChartDoc->hasInternalDataProvider();
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
                if ( bInternalDataProvider )
                {
                    pChartListener->ChangeListening( aNewRLR, bDataChanged );
                }
                else
                {
                    pChartListener->ChangeListening( new ScRangeList, bDataChanged );
                }
			}
		}
	}
}


void ScDocument::SetChartRangeList( const String& rChartName,
			const ScRangeListRef& rNewRangeListRef )
{
    // called from ChartListener

    if (!pDrawLayer)
        return;

    for (SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");

        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                    ((SdrOle2Obj*)pObject)->GetPersistName() == rChartName )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( ScChartHelper::GetChartFromSdrObject( pObject ) );
                uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if ( xChartDoc.is() && xReceiver.is() )
                {
                    ScRangeListRef aNewRanges;
                    chart::ChartDataRowSource eDataRowSource = chart::ChartDataRowSource_COLUMNS;
                    bool bHasCategories = false;
                    bool bFirstCellAsLabel = false;
                    rtl::OUString aRangesStr;
                    lcl_GetChartParameters( xChartDoc, aRangesStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    String sRangeStr;
                    rNewRangeListRef->Format( sRangeStr, SCR_ABS_3D, this, GetAddressConvention() );

                    lcl_SetChartParameters( xReceiver, sRangeStr, eDataRowSource, bHasCategories, bFirstCellAsLabel );

                    // don't modify pChartListenerCollection here, called from there
                    return;
                }
            }
            pObject = aIter.Next();
        }
    }
}


sal_Bool ScDocument::HasData( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
	if (pTab[nTab])
		return pTab[nTab]->HasData( nCol, nRow );
	else
		return sal_False;
}

uno::Reference< embed::XEmbeddedObject >
    ScDocument::FindOleObjectByName( const String& rName )
{
    if (!pDrawLayer)
		return uno::Reference< embed::XEmbeddedObject >();

	//	die Seiten hier vom Draw-Layer nehmen,
	//	weil sie evtl. nicht mit den Tabellen uebereinstimmen
	//	(z.B. Redo von Tabelle loeschen, Draw-Redo passiert vor DeleteTab).

	sal_uInt16 nCount = pDrawLayer->GetPageCount();
	for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
	{
		SdrPage* pPage = pDrawLayer->GetPage(nTab);
		DBG_ASSERT(pPage,"Page ?");

		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
		SdrObject* pObject = aIter.Next();
		while (pObject)
		{
			if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj * pOleObject ( dynamic_cast< SdrOle2Obj * >( pObject ));
                if( pOleObject &&
					pOleObject->GetPersistName() == rName )
                {
                    return pOleObject->GetObjRef();
                }
            }
			pObject = aIter.Next();
        }
    }

    return uno::Reference< embed::XEmbeddedObject >();
}

sal_Bool lcl_StringInCollection( const ScStrCollection* pColl, const String& rStr )
{
	if ( !pColl )
		return sal_False;

	StrData aData( rStr );
	sal_uInt16 nDummy;
	return pColl->Search( &aData, nDummy );
}

void ScDocument::UpdateChartListenerCollection()
{
	bChartListenerCollectionNeedsUpdate = sal_False;
	if (!pDrawLayer)
		return;
	else
	{
		ScRange aRange;
		// Range fuer Suche unwichtig
		ScChartListener aCLSearcher( EMPTY_STRING, this, aRange );
		for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
		{
			if (pTab[nTab])
			{
				SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
				DBG_ASSERT(pPage,"Page ?");

                if (!pPage)
                    continue;

				SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
				SdrObject* pObject = aIter.Next();
				while (pObject)
				{
					if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
					{
						String aObjName = ((SdrOle2Obj*)pObject)->GetPersistName();
						aCLSearcher.SetString( aObjName );
						sal_uInt16 nIndex;
						if ( pChartListenerCollection->Search( &aCLSearcher, nIndex ) )
						{
							((ScChartListener*) (pChartListenerCollection->
								At( nIndex )))->SetUsed( sal_True );
						}
						else if ( lcl_StringInCollection( pOtherObjects, aObjName ) )
						{
							// non-chart OLE object -> don't touch
						}
						else
						{
						    bool bIsChart = false;

							uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
                            DBG_ASSERT( xIPObj.is(), "No embedded object is given!");
                            uno::Reference< ::com::sun::star::chart2::data::XDataReceiver > xReceiver;
                            uno::Reference< embed::XComponentSupplier > xCompSupp( xIPObj, uno::UNO_QUERY );
                            if( xCompSupp.is())
                                xReceiver.set( xCompSupp->getComponent(), uno::UNO_QUERY );

                            // if the object is a chart2::XDataReceiver, we must attach as XDataProvider
                            if( xReceiver.is() &&
                                !PastingDrawFromOtherDoc())
                            {
                                // NOTE: this currently does not work as we are
                                // unable to set the data. So a chart from the
                                // same document is treated like a chart with
                                // own data for the time being.
#if 0
                                // data provider
                                uno::Reference< chart2::data::XDataProvider > xDataProvider = new
                                    ScChart2DataProvider( this );
                                xReceiver->attachDataProvider( xDataProvider );
                                // number formats supplier
                                uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( pShell->GetModel(), uno::UNO_QUERY );
                                xReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );
                                // data ?
                                // how to set?? Defined in XML-file, which is already loaded!!!
                                // => we have to do this stuff here, BEFORE the chart is actually loaded

                                bIsChart = true;
#endif
                            }

						    if (!bIsChart)
						    {
							    //	put into list of other ole objects, so the object doesn't have to
							    //	be swapped in the next time UpdateChartListenerCollection is called
							    //!	remove names when objects are no longer there?
							    //	(object names aren't used again before reloading the document)

							    if (!pOtherObjects)
								    pOtherObjects = new ScStrCollection;
							    pOtherObjects->Insert( new StrData( aObjName ) );
						    }
						}
					}
					pObject = aIter.Next();
				}
			}
		}
		// alle nicht auf SetUsed gesetzten loeschen
		pChartListenerCollection->FreeUnused();
	}
}

void ScDocument::AddOLEObjectToCollection(const String& rName)
{
	if (!pOtherObjects)
		pOtherObjects = new ScStrCollection;
	pOtherObjects->Insert( new StrData( rName ) );
}



