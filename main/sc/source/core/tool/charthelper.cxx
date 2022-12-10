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

#include "charthelper.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "rangelst.hxx"
#include "chartlis.hxx"
#include "docuno.hxx"

//#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/util/XModifiable.hpp>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;


// ====================================================================

namespace
{


sal_uInt16 lcl_DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc, sal_Bool bAllCharts )
{
	ScDrawLayer* pModel = pDoc->GetDrawLayer();
	if (!pModel)
		return 0;

	sal_uInt16 nFound = 0;

	sal_uInt16 nPageCount = pModel->GetPageCount();
	for (sal_uInt16 nPageNo=0; nPageNo<nPageCount; nPageNo++)
	{
		SdrPage* pPage = pModel->GetPage(nPageNo);
		DBG_ASSERT(pPage,"Page ?");

		SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
		SdrObject* pObject = aIter.Next();
		while (pObject)
		{
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 && pDoc->IsChart( pObject ) )
			{
                String aName = ((SdrOle2Obj*)pObject)->GetPersistName();
                sal_Bool bHit = sal_True;
                if ( !bAllCharts )
                {
                    ScRangeList aRanges;
                    sal_Bool bColHeaders = sal_False;
                    sal_Bool bRowHeaders = sal_False;
                    pDoc->GetOldChartParameters( aName, aRanges, bColHeaders, bRowHeaders );
                    bHit = aRanges.In( rPos );
                }
                if ( bHit )
                {
                    pDoc->UpdateChart( aName );
                    ++nFound;
                }
			}
			pObject = aIter.Next();
		}
	}
	return nFound;
}

sal_Bool lcl_AdjustRanges( ScRangeList& rRanges, SCTAB nSourceTab, SCTAB nDestTab, SCTAB nTabCount )
{
	//!	if multiple sheets are copied, update references into the other copied sheets?

	sal_Bool bChanged = sal_False;

	sal_uLong nCount = rRanges.Count();
	for (sal_uLong i=0; i<nCount; i++)
	{
		ScRange* pRange = rRanges.GetObject(i);
		if ( pRange->aStart.Tab() == nSourceTab && pRange->aEnd.Tab() == nSourceTab )
		{
			pRange->aStart.SetTab( nDestTab );
			pRange->aEnd.SetTab( nDestTab );
			bChanged = sal_True;
		}
		if ( pRange->aStart.Tab() >= nTabCount )
		{
			pRange->aStart.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
			bChanged = sal_True;
		}
		if ( pRange->aEnd.Tab() >= nTabCount )
		{
			pRange->aEnd.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
			bChanged = sal_True;
		}
	}

	return bChanged;
}

}//end anonymous namespace

// === ScChartHelper ======================================

//static
sal_uInt16 ScChartHelper::DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc )
{
    return lcl_DoUpdateCharts( rPos, pDoc, sal_False );
}

//static
sal_uInt16 ScChartHelper::DoUpdateAllCharts( ScDocument* pDoc )
{
    return lcl_DoUpdateCharts( ScAddress(), pDoc, sal_True );
}

//static
void ScChartHelper::AdjustRangesOfChartsOnDestinationPage( ScDocument* pSrcDoc, ScDocument* pDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab )
{
    if( !pSrcDoc || !pDestDoc )
        return;
    ScDrawLayer* pDrawLayer = pDestDoc->GetDrawLayer();
    if( !pDrawLayer )
        return;

    SdrPage* pDestPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestTab));
    if( pDestPage )
    {
        SdrObjListIter aIter( *pDestPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while( pObject )
        {
            if( pObject->GetObjIdentifier() == OBJ_OLE2 && ((SdrOle2Obj*)pObject)->IsChart() )
            {
                String aChartName = ((SdrOle2Obj*)pObject)->GetPersistName();

                Reference< chart2::XChartDocument > xChartDoc( pDestDoc->GetChartByName( aChartName ) );
                Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
                if( xChartDoc.is() && xReceiver.is() && !xChartDoc->hasInternalDataProvider() )
                {
                    ::std::vector< ScRangeList > aRangesVector;
                    pDestDoc->GetChartRanges( aChartName, aRangesVector, pSrcDoc );

                    ::std::vector< ScRangeList >::iterator aIt( aRangesVector.begin() );
                    for( ; aIt!=aRangesVector.end(); aIt++ )
                    {
                        ScRangeList& rScRangeList( *aIt );
                        lcl_AdjustRanges( rScRangeList, nSrcTab, nDestTab, pDestDoc->GetTableCount() );
                    }
                    pDestDoc->SetChartRanges( aChartName, aRangesVector );
                }
            }
            pObject = aIter.Next();
        }
    }
}

//static
void ScChartHelper::UpdateChartsOnDestinationPage( ScDocument* pDestDoc, const SCTAB nDestTab )
{
	if( !pDestDoc )
        return;
    ScDrawLayer* pDrawLayer = pDestDoc->GetDrawLayer();
    if( !pDrawLayer )
        return;

    SdrPage* pDestPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestTab));
    if( pDestPage )
    {
        SdrObjListIter aIter( *pDestPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while( pObject )
        {
            if( pObject->GetObjIdentifier() == OBJ_OLE2 && ((SdrOle2Obj*)pObject)->IsChart() )
            {
                String aChartName = ((SdrOle2Obj*)pObject)->GetPersistName();
                Reference< chart2::XChartDocument > xChartDoc( pDestDoc->GetChartByName( aChartName ) );			
				Reference< util::XModifiable > xModif(xChartDoc, uno::UNO_QUERY_THROW);
				xModif->setModified( sal_True);		
			}
			pObject = aIter.Next();
		}
	}
}

//static
uno::Reference< chart2::XChartDocument > ScChartHelper::GetChartFromSdrObject( SdrObject* pObject )
{
    uno::Reference< chart2::XChartDocument > xReturn;
    if( pObject )
    {
        if( pObject->GetObjIdentifier() == OBJ_OLE2 && ((SdrOle2Obj*)pObject)->IsChart() )
        {
            uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
            if( xIPObj.is() )
            {
                svt::EmbeddedObjectRef::TryRunningState( xIPObj );
                uno::Reference< util::XCloseable > xComponent = xIPObj->getComponent();
                xReturn.set( uno::Reference< chart2::XChartDocument >( xComponent, uno::UNO_QUERY ) );
            }
        }
    }
    return xReturn;
}

void ScChartHelper::GetChartRanges( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            uno::Sequence< rtl::OUString >& rRanges )
{
    rRanges.realloc(0);
    uno::Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
    if( !xDataSource.is() )
        return;
    //uno::Reference< chart2::data::XDataProvider > xProvider = xChartDoc->getDataProvider();

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledDataSequences( xDataSource->getDataSequences() );
    rRanges.realloc(2*aLabeledDataSequences.getLength());
    sal_Int32 nRealCount=0;
    for( sal_Int32 nN=0;nN<aLabeledDataSequences.getLength();nN++)
    {
        uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aLabeledDataSequences[nN] );
        if(!xLabeledSequence.is())
            continue;
        uno::Reference< chart2::data::XDataSequence > xLabel( xLabeledSequence->getLabel());
        uno::Reference< chart2::data::XDataSequence > xValues( xLabeledSequence->getValues());

        if( xLabel.is())
            rRanges[nRealCount++] = xLabel->getSourceRangeRepresentation();
        if( xValues.is())
            rRanges[nRealCount++] = xValues->getSourceRangeRepresentation();
    }
    rRanges.realloc(nRealCount);
}

void ScChartHelper::SetChartRanges( const uno::Reference< chart2::XChartDocument >& xChartDoc,
            const uno::Sequence< rtl::OUString >& rRanges )
{
    uno::Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
    if( !xDataSource.is() )
        return;
    uno::Reference< chart2::data::XDataProvider > xDataProvider = xChartDoc->getDataProvider();
    if( !xDataProvider.is() )
        return;

    uno::Reference< frame::XModel > xModel( xChartDoc, uno::UNO_QUERY );
    if( xModel.is() )
        xModel->lockControllers();

    try
    {
        rtl::OUString aPropertyNameRole( ::rtl::OUString::createFromAscii("Role") );

        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledDataSequences( xDataSource->getDataSequences() );
        sal_Int32 nRange=0;
        for( sal_Int32 nN=0; (nN<aLabeledDataSequences.getLength()) && (nRange<rRanges.getLength()); nN++ )
        {
            uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aLabeledDataSequences[nN] );
            if(!xLabeledSequence.is())
                continue;
            uno::Reference< beans::XPropertySet > xLabel( xLabeledSequence->getLabel(), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xValues( xLabeledSequence->getValues(), uno::UNO_QUERY );

            if( xLabel.is())
            {
                // the range string must be in Calc A1 format.
                uno::Reference< chart2::data::XDataSequence > xNewSeq(
                    xDataProvider->createDataSequenceByRangeRepresentation( rRanges[nRange++] ));
                
                uno::Reference< beans::XPropertySet > xNewProps( xNewSeq, uno::UNO_QUERY );
                if( xNewProps.is() )
                    xNewProps->setPropertyValue( aPropertyNameRole, xLabel->getPropertyValue( aPropertyNameRole ) );

                xLabeledSequence->setLabel( xNewSeq );
            }

            if( !(nRange<rRanges.getLength()) )
                break;

            if( xValues.is())
            {
                // the range string must be in Calc A1 format.
                uno::Reference< chart2::data::XDataSequence > xNewSeq(
                    xDataProvider->createDataSequenceByRangeRepresentation( rRanges[nRange++] ));
                
                uno::Reference< beans::XPropertySet > xNewProps( xNewSeq, uno::UNO_QUERY );
                if( xNewProps.is() )
                    xNewProps->setPropertyValue( aPropertyNameRole, xValues->getPropertyValue( aPropertyNameRole ) );

                xLabeledSequence->setValues( xNewSeq );
            }
        }
    }
    catch ( uno::Exception& ex )
    {
        (void)ex;
        DBG_ERROR("Exception in ScChartHelper::SetChartRanges - invalid range string?");
    }

    if( xModel.is() )
        xModel->unlockControllers();
}

void ScChartHelper::AddRangesIfProtectedChart( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrObject* pObject )
{
    if ( pDocument && pObject && ( pObject->GetObjIdentifier() == OBJ_OLE2 ) )
    {
        SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
        if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
        {
            uno::Reference< embed::XEmbeddedObject > xEmbeddedObj = pSdrOle2Obj->GetObjRef();
            if ( xEmbeddedObj.is() )
            {
                bool bDisableDataTableDialog = false;
                sal_Int32 nOldState = xEmbeddedObj->getCurrentState();
                svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
                uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
                if ( xProps.is() &&
                     ( xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableDataTableDialog" ) ) ) >>= bDisableDataTableDialog ) &&
                     bDisableDataTableDialog )
                {
                    ::rtl::OUString aChartName = pSdrOle2Obj->GetPersistName();
                    ScRange aEmptyRange;
                    ScChartListener aSearcher( aChartName, pDocument, aEmptyRange );
                    sal_uInt16 nIndex = 0;
                    ScChartListenerCollection* pCollection = pDocument->GetChartListenerCollection();
                    if ( pCollection && pCollection->Search( &aSearcher, nIndex ) )
                    {
                        ScChartListener* pListener = static_cast< ScChartListener* >( pCollection->At( nIndex ) );
                        if ( pListener )
                        {
                            const ScRangeListRef& rRangeList = pListener->GetRangeList();
                            if ( rRangeList.Is() )
                            {
                                rRangesVector.push_back( *rRangeList );
                            }
                        }
                    }
                }
                if ( xEmbeddedObj->getCurrentState() != nOldState )
                {
                    xEmbeddedObj->changeState( nOldState );
                }
            }
        }
    }
}

void ScChartHelper::FillProtectedChartRangesVector( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrPage* pPage )
{
    if ( pDocument && pPage )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            AddRangesIfProtectedChart( rRangesVector, pDocument, pObject );
            pObject = aIter.Next();
        }
    }
}

void ScChartHelper::GetChartNames( ::std::vector< ::rtl::OUString >& rChartNames, SdrPage* pPage )
{
    if ( pPage )
    {
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
                if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
                {
                    rChartNames.push_back( pSdrOle2Obj->GetPersistName() );
                }
            }
            pObject = aIter.Next();
        }
    }
}

void ScChartHelper::CreateProtectedChartListenersAndNotify( ScDocument* pDoc, SdrPage* pPage, ScModelObj* pModelObj, SCTAB nTab,
    const ScRangeListVector& rRangesVector, const ::std::vector< ::rtl::OUString >& rExcludedChartNames, bool bSameDoc )
{
    if ( pDoc && pPage && pModelObj )
    {
        size_t nRangeListCount = rRangesVector.size();
        size_t nRangeList = 0;
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while ( pObject )
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >( pObject );
                if ( pSdrOle2Obj && pSdrOle2Obj->IsChart() )
                {
                    ::rtl::OUString aChartName = pSdrOle2Obj->GetPersistName();
                    ::std::vector< ::rtl::OUString >::const_iterator aEnd = rExcludedChartNames.end();
                    ::std::vector< ::rtl::OUString >::const_iterator aFound = ::std::find( rExcludedChartNames.begin(), aEnd, aChartName );
                    if ( aFound == aEnd )
                    {
                        uno::Reference< embed::XEmbeddedObject > xEmbeddedObj = pSdrOle2Obj->GetObjRef();
                        if ( xEmbeddedObj.is() && ( nRangeList < nRangeListCount ) )
                        {
                            bool bDisableDataTableDialog = false;
                            svt::EmbeddedObjectRef::TryRunningState( xEmbeddedObj );
                            uno::Reference< beans::XPropertySet > xProps( xEmbeddedObj->getComponent(), uno::UNO_QUERY );
                            if ( xProps.is() &&
                                 ( xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableDataTableDialog" ) ) ) >>= bDisableDataTableDialog ) &&
                                 bDisableDataTableDialog )
                            {
                                if ( bSameDoc )
                                {
                                    ScRange aEmptyRange;
                                    ScChartListener aSearcher( aChartName, pDoc, aEmptyRange );
                                    sal_uInt16 nIndex = 0;
                                    ScChartListenerCollection* pCollection = pDoc->GetChartListenerCollection();
                                    if ( pCollection && !pCollection->Search( &aSearcher, nIndex ) )
                                    {
                                        ScRangeList aRangeList( rRangesVector[ nRangeList++ ] );
                                        ScRangeListRef rRangeList( new ScRangeList( aRangeList ) );
                                        ScChartListener* pChartListener = new ScChartListener( aChartName, pDoc, rRangeList );
                                        pCollection->Insert( pChartListener );
                                        pChartListener->StartListeningTo();
                                    }
                                }
                                else
                                {
                                    xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableDataTableDialog" ) ),
                                        uno::makeAny( sal_False ) );
                                    xProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisableComplexChartTypes" ) ),
                                        uno::makeAny( sal_False ) );
                                }
                            }
                        }

                        if ( pModelObj && pModelObj->HasChangesListeners() )
                        {
                            Rectangle aRectangle = pSdrOle2Obj->GetSnapRect();
                            ScRange aRange( pDoc->GetRange( nTab, aRectangle ) );
                            ScRangeList aChangeRanges;
                            aChangeRanges.Append( aRange );

                            uno::Sequence< beans::PropertyValue > aProperties( 1 );
                            aProperties[ 0 ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
                            aProperties[ 0 ].Value <<= aChartName;

                            pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert-chart" ) ), aChangeRanges, aProperties );
                        }
                    }
                }
            }
            pObject = aIter.Next();
        }
    }
}
