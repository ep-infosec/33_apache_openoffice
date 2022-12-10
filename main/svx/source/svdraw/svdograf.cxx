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

#define _ANIMATION
#include <unotools/streamwrap.hxx>

#include <sfx2/lnkbase.hxx>
#include <math.h>
#include <vcl/salbtype.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <svl/style.hxx>
#include <svtools/filter.hxx>
#include <svl/urihelper.hxx>
#include <svtools/grfmgr.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdetc.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svdviter.hxx"
#include <svx/svdview.hxx>
#include "svtools/filter.hxx"
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/svdundo.hxx>
#include "svdfmtf.hxx"
#include <svx/sdgcpitm.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/properties/graphicproperties.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <osl/thread.hxx>
#include <vos/mutex.hxx>
#include <drawinglayer/processor2d/objectinfoextractor2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <unotools/cacheoptions.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

// -----------
// - Defines -
// -----------

#define GRAFSTREAMPOS_INVALID   0xffffffff
#define SWAPGRAPHIC_TIMEOUT     5000

// #122985# it is not correct to se the swap-timeout to a hard-coded 5000ms as it was before.
// Added code and experimented what to do as a good compromize, see description
sal_uInt32 getCacheTimeInMs()
{
    static bool bSetAtAll(true);

    if(bSetAtAll)
    {
        static bool bSetToPreferenceTime(true);

        if(bSetToPreferenceTime)
        {
            const SvtCacheOptions aCacheOptions;
            const sal_Int32 nSeconds(aCacheOptions.GetGraphicManagerObjectReleaseTime());

            // the default is 10 minutes. The minimum is one minute, thus 60 seconds. When the minimum
            // should match to the former hard-coded 5 seconds, we have a divisor of 12 to use. For the
            // default of 10 minutes this would mean 50 seconds. Compared to before this is ten times
            // more (would allow better navigation by switching through pages) and is controllable
            // by the user by setting the tools/options/memory/Remove_from_memory_after setting. Seems
            // to be a good compromize to me.
            return nSeconds * 1000 / 12;
        }
        else
        {
            return SWAPGRAPHIC_TIMEOUT;
        }
    }

    return 0;
}

// ------------------
// - SdrGraphicLink	-
// ------------------


const Graphic ImpLoadLinkedGraphic( const String aFileName, const String aFilterName )
{
	Graphic aGraphic;

	SfxMedium xMed( aFileName, STREAM_STD_READ, sal_True );
	xMed.DownLoad();

	SvStream* pInStrm = xMed.GetInStream();
	if ( pInStrm )
	{
		pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
		GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();

		const sal_uInt16 nFilter = aFilterName.Len() && pGF->GetImportFormatCount()
							? pGF->GetImportFormatNumber( aFilterName )
							: GRFILTER_FORMAT_DONTKNOW;

		com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aFilterData( 1 );

		// Room for improvment:
		// As this is a linked graphic the GfxLink is not needed if saving/loading our own format.
		// But this link is required by some filters to access the native graphic (pdf export/ms export),
		// there we should create a new service to provide this data if needed
		aFilterData[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
		aFilterData[ 0 ].Value = Any( sal_True );

        // #123042# for e.g SVG the path is needed, so hand it over here. I have no real idea
        // what consequences this may have; maybe this is not handed over by purpose here. Not
        // handing it over means that any GraphicFormat that internallv needs a path as base
        // to interpret included links may fail.
        // Alternatively the path may be set at the result after this call when it is known
        // that it is a SVG graphic, but only because no one yet tried to interpret it.
        pGF->ImportGraphic( aGraphic, aFileName, *pInStrm, nFilter, NULL, 0, &aFilterData );
	}
	return aGraphic;
}

class SdrGraphicUpdater;
class SdrGraphicLink : public sfx2::SvBaseLink
{
	SdrGrafObj*			pGrafObj;
	SdrGraphicUpdater*	pGraphicUpdater;

public:
						SdrGraphicLink(SdrGrafObj* pObj);
	virtual				~SdrGraphicLink();

	virtual void		Closed();
	virtual void		DataChanged( const String& rMimeType,
								const ::com::sun::star::uno::Any & rValue );
	void				DataChanged( const Graphic& rGraphic );

	sal_Bool				Connect() { return 0 != GetRealObject(); }
	void				UpdateAsynchron();
	void				RemoveGraphicUpdater();
};

class SdrGraphicUpdater : public ::osl::Thread
{
public:
    SdrGraphicUpdater( const String& rFileName, const String& rFilterName, SdrGraphicLink& );
    virtual ~SdrGraphicUpdater( void );

	void SAL_CALL Terminate( void );

    sal_Bool GraphicLinkChanged( const String& rFileName ){ return maFileName != rFileName;	};

protected:

	/**	is called from the inherited create method and acts as the
		main function of this thread.
	*/
	virtual void SAL_CALL run(void);

    /**	Called after the thread is terminated via the terminate
    	method.  Used to kill the thread by calling delete on this.
    */	
    virtual void SAL_CALL onTerminated(void);

private:

    const String	maFileName;
	const String	maFilterName;
	SdrGraphicLink& mrGraphicLink;

	volatile bool mbIsTerminated;
};

SdrGraphicUpdater::SdrGraphicUpdater( const String& rFileName, const String& rFilterName, SdrGraphicLink& rGraphicLink )
: maFileName( rFileName )
, maFilterName( rFilterName )
, mrGraphicLink( rGraphicLink )
, mbIsTerminated( sal_False )
{
	create();
}

SdrGraphicUpdater::~SdrGraphicUpdater( void )
{
}

void SdrGraphicUpdater::Terminate()
{
    mbIsTerminated = sal_True;
}

void SAL_CALL SdrGraphicUpdater::onTerminated(void)
{
	delete this;
}

void SAL_CALL SdrGraphicUpdater::run(void)
{
	Graphic aGraphic( ImpLoadLinkedGraphic( maFileName, maFilterName ) );
	vos::OGuard aSolarGuard( Application::GetSolarMutex() );
	if ( !mbIsTerminated )
	{
		mrGraphicLink.DataChanged( aGraphic );
		mrGraphicLink.RemoveGraphicUpdater();
	}
}

// -----------------------------------------------------------------------------

SdrGraphicLink::SdrGraphicLink(SdrGrafObj* pObj)
: ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB )
, pGrafObj( pObj )
, pGraphicUpdater( NULL )
{
	SetSynchron( sal_False );
}

// -----------------------------------------------------------------------------

SdrGraphicLink::~SdrGraphicLink()
{
	if ( pGraphicUpdater )
		pGraphicUpdater->Terminate();
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::DataChanged( const Graphic& rGraphic )
{
	pGrafObj->ImpSetLinkedGraphic( rGraphic );
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::RemoveGraphicUpdater()
{
	pGraphicUpdater = NULL;
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::DataChanged( const String& rMimeType,
								const ::com::sun::star::uno::Any & rValue )
{
	SdrModel*       pModel      = pGrafObj ? pGrafObj->GetModel() : 0;
	sfx2::LinkManager* pLinkManager= pModel  ? pModel->GetLinkManager() : 0;

	if( pLinkManager && rValue.hasValue() )
	{
		pLinkManager->GetDisplayNames( this, 0, &pGrafObj->aFileName, 0, &pGrafObj->aFilterName );

		Graphic aGraphic;
		if( sfx2::LinkManager::GetGraphicFromAny( rMimeType, rValue, aGraphic ))
		{
   			pGrafObj->NbcSetGraphic( aGraphic );
			pGrafObj->ActionChanged();
		}
		else if( SotExchange::GetFormatIdFromMimeType( rMimeType ) != sfx2::LinkManager::RegisterStatusInfoId() )
		{
			// broadcasting, to update slidesorter
			pGrafObj->BroadcastObjectChange();
		}
	}
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::Closed()
{
	// Die Verbindung wird aufgehoben; pLink des Objekts auf NULL setzen, da die Link-Instanz ja gerade destruiert wird.
	pGrafObj->ForceSwapIn();
	pGrafObj->pGraphicLink=NULL;
	pGrafObj->ReleaseGraphicLink();
	SvBaseLink::Closed();
}

// -----------------------------------------------------------------------------

void SdrGraphicLink::UpdateAsynchron()
{
	if( GetObj() )
	{
		if ( pGraphicUpdater )
		{
			if ( pGraphicUpdater->GraphicLinkChanged( pGrafObj->GetFileName() ) )
			{
				pGraphicUpdater->Terminate();
				pGraphicUpdater = new SdrGraphicUpdater( pGrafObj->GetFileName(), pGrafObj->GetFilterName(), *this );
			}
		}
		else
			pGraphicUpdater = new SdrGraphicUpdater( pGrafObj->GetFileName(), pGrafObj->GetFilterName(), *this );
	}
}

// --------------
// - SdrGrafObj -
// --------------

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrGrafObj::CreateObjectSpecificProperties()
{
	return new sdr::properties::GraphicProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrGrafObj::CreateObjectSpecificViewContact()
{
	return new sdr::contact::ViewContactOfGraphic(*this);
}

//////////////////////////////////////////////////////////////////////////////
// check if SVG and if try to get ObjectInfoPrimitive2D and extract info

void SdrGrafObj::onGraphicChanged()
{
    String aName;
    String aTitle;
    String aDesc;

    if(pGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            const drawinglayer::primitive2d::Primitive2DSequence aSequence(rSvgDataPtr->getPrimitive2DSequence());

            if(aSequence.hasElements())
            {
                drawinglayer::geometry::ViewInformation2D aViewInformation2D;
                drawinglayer::processor2d::ObjectInfoPrimitiveExtractor2D aProcessor(aViewInformation2D);

                aProcessor.process(aSequence);

                const drawinglayer::primitive2d::ObjectInfoPrimitive2D* pResult = aProcessor.getResult();

                if(pResult)
                {
                    aName = pResult->getName();
			        aTitle = pResult->getTitle();
			        aDesc = pResult->getDesc();
                }
            }
        }
    }

    if(aName.Len())
    {
        SetName(aName);
    }

    if(aTitle.Len())
    {
    	SetTitle(aTitle);
    }

    if(aDesc.Len())
    {
    	SetDescription(aDesc);
    }
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrGrafObj,SdrRectObj);

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj()
:	SdrRectObj(),
	pGraphicLink	( NULL ),
	bMirrored		( sal_False )
{
	pGraphic = new GraphicObject;
    mpReplacementGraphic = 0;
	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), getCacheTimeInMs() );
    onGraphicChanged();
	
    // #i118485# Shear allowed and possible now
    bNoShear = false;

	// #111096#
	mbGrafAnimationAllowed = sal_True;

	// #i25616#
	mbLineIsOutsideGeometry = sal_True;
	mbInsidePaint = sal_False;
	mbIsPreview = sal_False;

	// #i25616#
	mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect)
:	SdrRectObj		( rRect ),
	pGraphicLink	( NULL ),
	bMirrored		( sal_False )
{
	pGraphic = new GraphicObject( rGrf );
    mpReplacementGraphic = 0;
	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), getCacheTimeInMs() );
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    bNoShear = false;

	// #111096#
	mbGrafAnimationAllowed = sal_True;

	// #i25616#
	mbLineIsOutsideGeometry = sal_True;
	mbInsidePaint = sal_False;
	mbIsPreview	= sal_False;

	// #i25616#
	mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::SdrGrafObj( const Graphic& rGrf )
:	SdrRectObj(),
	pGraphicLink	( NULL ),
	bMirrored		( sal_False )
{
	pGraphic = new GraphicObject( rGrf );
    mpReplacementGraphic = 0;
	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), getCacheTimeInMs() );
    onGraphicChanged();

    // #i118485# Shear allowed and possible now
    bNoShear = false;

	// #111096#
	mbGrafAnimationAllowed = sal_True;

	// #i25616#
	mbLineIsOutsideGeometry = sal_True;
	mbInsidePaint = sal_False;
	mbIsPreview	= sal_False;

	// #i25616#
	mbSupportTextIndentingOnLineWidthChange = sal_False;
}

// -----------------------------------------------------------------------------

SdrGrafObj::~SdrGrafObj()
{
	delete pGraphic;
    delete mpReplacementGraphic;
	ImpLinkAbmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicObject( const GraphicObject& rGrfObj )
{
	*pGraphic = rGrfObj;
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;
	pGraphic->SetSwapStreamHdl( LINK( this, SdrGrafObj, ImpSwapHdl ), getCacheTimeInMs() );
	pGraphic->SetUserData();
	mbIsPreview = sal_False;
	SetChanged();
	BroadcastObjectChange();
    onGraphicChanged();
}

// -----------------------------------------------------------------------------

const GraphicObject& SdrGrafObj::GetGraphicObject(bool bForceSwapIn) const
{
	if(bForceSwapIn)
	{
		ForceSwapIn();
	}

	return *pGraphic;
}

const GraphicObject* SdrGrafObj::GetReplacementGraphicObject() const
{
    if(!mpReplacementGraphic && pGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            const_cast< SdrGrafObj* >(this)->mpReplacementGraphic = new GraphicObject(rSvgDataPtr->getReplacement());
        }
    }

    return mpReplacementGraphic;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetGraphic( const Graphic& rGrf )
{
	pGraphic->SetGraphic( rGrf );
    delete mpReplacementGraphic;
    mpReplacementGraphic = 0;
	pGraphic->SetUserData();
	mbIsPreview = sal_False;
    onGraphicChanged();
}

void SdrGrafObj::SetGraphic( const Graphic& rGrf )
{
    NbcSetGraphic(rGrf);
	SetChanged();
	BroadcastObjectChange();
}

// -----------------------------------------------------------------------------

const Graphic& SdrGrafObj::GetGraphic() const
{
	ForceSwapIn();
	return pGraphic->GetGraphic();
}

// -----------------------------------------------------------------------------

Graphic SdrGrafObj::GetTransformedGraphic( sal_uIntPtr nTransformFlags ) const
{
    // #107947# Refactored most of the code to GraphicObject, where
    // everybody can use e.g. the cropping functionality

	GraphicType	    eType = GetGraphicType();
    MapMode   		aDestMap( pModel->GetScaleUnit(), Point(), pModel->GetScaleFraction(), pModel->GetScaleFraction() );
    const Size      aDestSize( GetLogicRect().GetSize() );
    const sal_Bool      bMirror = ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_MIRROR ) != 0;
    const sal_Bool      bRotate = ( ( nTransformFlags & SDRGRAFOBJ_TRANSFORMATTR_ROTATE ) != 0 ) &&
        ( aGeo.nDrehWink && aGeo.nDrehWink != 18000 ) && ( GRAPHIC_NONE != eType );

    // #104115# Need cropping info earlier
    ( (SdrGrafObj*) this )->ImpSetAttrToGrafInfo();
    GraphicAttr aActAttr;

	if( SDRGRAFOBJ_TRANSFORMATTR_NONE != nTransformFlags &&
        GRAPHIC_NONE != eType )
	{
        // actually transform the graphic only in this case. On the
        // other hand, cropping will always happen
        aActAttr = aGrafInfo;

        if( bMirror )
		{
			sal_uInt16		nMirrorCase = ( aGeo.nDrehWink == 18000 ) ? ( bMirrored ? 3 : 4 ) : ( bMirrored ? 2 : 1 );
			FASTBOOL	bHMirr = nMirrorCase == 2 || nMirrorCase == 4;
			FASTBOOL	bVMirr = nMirrorCase == 3 || nMirrorCase == 4;

			aActAttr.SetMirrorFlags( ( bHMirr ? BMP_MIRROR_HORZ : 0 ) | ( bVMirr ? BMP_MIRROR_VERT : 0 ) );
		}

		if( bRotate )
			aActAttr.SetRotation( sal_uInt16(aGeo.nDrehWink / 10) );
	}

    // #107947# Delegate to moved code in GraphicObject
    return GetGraphicObject().GetTransformedGraphic( aDestSize, aDestMap, aActAttr );
}

// -----------------------------------------------------------------------------

GraphicType SdrGrafObj::GetGraphicType() const
{
	return pGraphic->GetType();
}

sal_Bool SdrGrafObj::IsAnimated() const
{
	return pGraphic->IsAnimated();
}

sal_Bool SdrGrafObj::IsEPS() const
{
	return pGraphic->IsEPS();
}

sal_Bool SdrGrafObj::IsSwappedOut() const
{
	return mbIsPreview ? sal_True : pGraphic->IsSwappedOut();
}

const MapMode& SdrGrafObj::GetGrafPrefMapMode() const
{
	return pGraphic->GetPrefMapMode();
}

const Size& SdrGrafObj::GetGrafPrefSize() const
{
	return pGraphic->GetPrefSize();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGrafStreamURL( const String& rGraphicStreamURL )
{
	mbIsPreview = sal_False;
	if( !rGraphicStreamURL.Len() )
	{
		pGraphic->SetUserData();
	}
	else if( pModel->IsSwapGraphics() )
	{
		pGraphic->SetUserData( rGraphicStreamURL );

		// set state of graphic object to 'swapped out'
		if( pGraphic->GetType() == GRAPHIC_NONE )
			pGraphic->SetSwapState();
	}
}

// -----------------------------------------------------------------------------

String SdrGrafObj::GetGrafStreamURL() const
{
	return pGraphic->GetUserData();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetFileName(const String& rFileName)
{
	aFileName = rFileName;
	SetChanged();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetFilterName(const String& rFilterName)
{
	aFilterName = rFilterName;
	SetChanged();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapIn() const
{
	if( mbIsPreview )
	{
		// removing preview graphic
		const String aUserData( pGraphic->GetUserData() );

		Graphic aEmpty;
		pGraphic->SetGraphic( aEmpty );
		pGraphic->SetUserData( aUserData );
		pGraphic->SetSwapState();

		const_cast< SdrGrafObj* >( this )->mbIsPreview = sal_False;
	}
	if ( pGraphicLink && pGraphic->IsSwappedOut() )
		ImpUpdateGraphicLink( sal_False );
	else
		pGraphic->FireSwapInRequest();

	if( pGraphic->IsSwappedOut() ||
	    ( pGraphic->GetType() == GRAPHIC_NONE ) ||
		( pGraphic->GetType() == GRAPHIC_DEFAULT ) )
	{
		Graphic aDefaultGraphic;
		aDefaultGraphic.SetDefaultType();
		pGraphic->SetGraphic( aDefaultGraphic );
	}
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ForceSwapOut() const
{
	pGraphic->FireSwapOutRequest();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAnmeldung()
{
	sfx2::LinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

	if( pLinkManager != NULL && pGraphicLink == NULL )
	{
		if( aFileName.Len() )
		{
			pGraphicLink = new SdrGraphicLink( this );
			pLinkManager->InsertFileLink( *pGraphicLink, OBJECT_CLIENT_GRF, aFileName, ( aFilterName.Len() ? &aFilterName : NULL ), NULL );
			pGraphicLink->Connect();
		}
	}
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpLinkAbmeldung()
{
	sfx2::LinkManager* pLinkManager = pModel != NULL ? pModel->GetLinkManager() : NULL;

	if( pLinkManager != NULL && pGraphicLink!=NULL)
	{
		// Bei Remove wird *pGraphicLink implizit deleted
		pLinkManager->Remove( pGraphicLink );
		pGraphicLink=NULL;
	}
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetGraphicLink( const String& rFileName, const String& rFilterName )
{
	ImpLinkAbmeldung();
	aFileName = rFileName;
	aFilterName = rFilterName;
	ImpLinkAnmeldung();
	pGraphic->SetUserData();

    // #92205# A linked graphic is per definition swapped out (has to be loaded)
    pGraphic->SetSwapState();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ReleaseGraphicLink()
{
	ImpLinkAbmeldung();
	aFileName = String();
	aFilterName = String();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
	FASTBOOL bNoPresGrf = ( pGraphic->GetType() != GRAPHIC_NONE ) && !bEmptyPresObj;

	rInfo.bResizeFreeAllowed = aGeo.nDrehWink % 9000 == 0 ||
							   aGeo.nDrehWink % 18000 == 0 ||
							   aGeo.nDrehWink % 27000 == 0;

	rInfo.bResizePropAllowed = sal_True;
	rInfo.bRotateFreeAllowed = bNoPresGrf;
	rInfo.bRotate90Allowed = bNoPresGrf;
	rInfo.bMirrorFreeAllowed = bNoPresGrf;
	rInfo.bMirror45Allowed = bNoPresGrf;
	rInfo.bMirror90Allowed = !bEmptyPresObj;
	rInfo.bTransparenceAllowed = sal_False;
	rInfo.bGradientAllowed = sal_False;

    // #i118485# Shear allowed and possible now
	rInfo.bShearAllowed = true;

    rInfo.bEdgeRadiusAllowed=sal_False;
	rInfo.bCanConvToPath = !IsEPS();
	rInfo.bCanConvToPathLineToArea = sal_False;
	rInfo.bCanConvToPolyLineToArea = sal_False;
	rInfo.bCanConvToPoly = !IsEPS();
	rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

// -----------------------------------------------------------------------------

sal_uInt16 SdrGrafObj::GetObjIdentifier() const
{
	return sal_uInt16( OBJ_GRAF );
}

// -----------------------------------------------------------------------------

/* The graphic of the GraphicLink will be loaded. If it is called with
   bAsynchron = true then the graphic will be set later via DataChanged
*/ 
sal_Bool SdrGrafObj::ImpUpdateGraphicLink( sal_Bool bAsynchron ) const
{
    sal_Bool bRet = sal_False;
    if( pGraphicLink )
	{
		if ( bAsynchron )
			pGraphicLink->UpdateAsynchron();
		else
			pGraphicLink->DataChanged( ImpLoadLinkedGraphic( aFileName, aFilterName ) );
        bRet = sal_True;
    }
	return bRet;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetLinkedGraphic( const Graphic& rGraphic )
{
	const sal_Bool bIsChanged = GetModel()->IsChanged();
	NbcSetGraphic( rGraphic );
	ActionChanged();
	BroadcastObjectChange();
	GetModel()->SetChanged( bIsChanged );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNameSingul(XubString& rName) const
{
    if(pGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            rName = ImpGetResStr(STR_ObjNameSingulGRAFSVG);
        }
        else
        {
	        switch( pGraphic->GetType() )
	        {
		        case GRAPHIC_BITMAP:
                {
                    const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                         ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPTRANSLNK : STR_ObjNameSingulGRAFBMPTRANS ) :
                                         ( IsLinkedGraphic() ? STR_ObjNameSingulGRAFBMPLNK : STR_ObjNameSingulGRAFBMP ) );

                    rName=ImpGetResStr( nId );
                }
                break;

		        case GRAPHIC_GDIMETAFILE:
                    rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFMTFLNK : STR_ObjNameSingulGRAFMTF );
                break;

                case GRAPHIC_NONE:
                    rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNameSingulGRAFNONELNK : STR_ObjNameSingulGRAFNONE );
                break;

                default:
                    rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNameSingulGRAFLNK : STR_ObjNameSingulGRAF );
                break;
	        }
        }

	    const String aName(GetName());

	    if( aName.Len() )
	    {
		    rName.AppendAscii( " '" );
		    rName += aName;
		    rName += sal_Unicode( '\'' );
	    }
    }
}

// -----------------------------------------------------------------------------

void SdrGrafObj::TakeObjNamePlural( XubString& rName ) const
{
    if(pGraphic)
    {
        const SvgDataPtr& rSvgDataPtr = pGraphic->GetGraphic().getSvgData();

        if(rSvgDataPtr.get())
        {
            rName = ImpGetResStr(STR_ObjNamePluralGRAFSVG);
        }
        else
        {
	        switch( pGraphic->GetType() )
	        {
		        case GRAPHIC_BITMAP:
                {
                    const sal_uInt16 nId = ( ( pGraphic->IsTransparent() || ( (const SdrGrafTransparenceItem&) GetObjectItem( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ) ?
                                         ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPTRANSLNK : STR_ObjNamePluralGRAFBMPTRANS ) :
                                         ( IsLinkedGraphic() ? STR_ObjNamePluralGRAFBMPLNK : STR_ObjNamePluralGRAFBMP ) );

                    rName=ImpGetResStr( nId );
                }
                break;

		        case GRAPHIC_GDIMETAFILE:
                    rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFMTFLNK : STR_ObjNamePluralGRAFMTF );
                break;

                case GRAPHIC_NONE:
                    rName=ImpGetResStr( IsLinkedGraphic() ? STR_ObjNamePluralGRAFNONELNK : STR_ObjNamePluralGRAFNONE );
                break;

                default:
                    rName=ImpGetResStr(  IsLinkedGraphic() ? STR_ObjNamePluralGRAFLNK : STR_ObjNamePluralGRAF );
                break;
	        }
        }

	    const String aName(GetName());

	    if( aName.Len() )
	    {
		    rName.AppendAscii( " '" );
		    rName += aName;
		    rName += sal_Unicode( '\'' );
	    }
    }
}

// -----------------------------------------------------------------------------

SdrObject* SdrGrafObj::getFullDragClone() const
{
    // call parent
    SdrGrafObj* pRetval = static_cast< SdrGrafObj* >(SdrRectObj::getFullDragClone());

    // #i103116# the full drag clone leads to problems
    // with linked graphics, so reset the link in this
    // temporary interaction object and load graphic
    if(pRetval && IsLinkedGraphic())
    {
        pRetval->ForceSwapIn();
        pRetval->ReleaseGraphicLink();
    }

    return pRetval;
}

void SdrGrafObj::operator=( const SdrObject& rObj )
{
	SdrRectObj::operator=( rObj );

	const SdrGrafObj& rGraf = (SdrGrafObj&) rObj;

	pGraphic->SetGraphic( rGraf.GetGraphic(), &rGraf.GetGraphicObject() );
	aCropRect = rGraf.aCropRect;
	aFileName = rGraf.aFileName;
	aFilterName = rGraf.aFilterName;
	bMirrored = rGraf.bMirrored;

	if( rGraf.pGraphicLink != NULL)
	{
		SetGraphicLink( aFileName, aFilterName );
	}

	ImpSetAttrToGrafInfo();
}

// -----------------------------------------------------------------------------
// #i25616#

basegfx::B2DPolyPolygon SdrGrafObj::TakeXorPoly() const
{
	if(mbInsidePaint)
	{
		basegfx::B2DPolyPolygon aRetval;

		// take grown rectangle
		const sal_Int32 nHalfLineWidth(ImpGetLineWdt() / 2);
		const Rectangle aGrownRect(
			aRect.Left() - nHalfLineWidth,
			aRect.Top() - nHalfLineWidth,
			aRect.Right() + nHalfLineWidth,
			aRect.Bottom() + nHalfLineWidth);

		XPolygon aXPoly(ImpCalcXPoly(aGrownRect, GetEckenradius()));
		aRetval.append(aXPoly.getB2DPolygon());

		return aRetval;
	}
	else
	{
		// call parent
		return SdrRectObj::TakeXorPoly();
	}
}

// -----------------------------------------------------------------------------

sal_uInt32 SdrGrafObj::GetHdlCount() const
{
	return 8L;
}

// -----------------------------------------------------------------------------

SdrHdl* SdrGrafObj::GetHdl(sal_uInt32 nHdlNum) const
{
	return SdrRectObj::GetHdl( nHdlNum + 1L );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
	SdrRectObj::NbcResize( rRef, xFact, yFact );

	FASTBOOL bMirrX = xFact.GetNumerator() < 0;
	FASTBOOL bMirrY = yFact.GetNumerator() < 0;

	if( bMirrX != bMirrY )
		bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
	SdrRectObj::NbcRotate(rRef,nWink,sn,cs);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
	SdrRectObj::NbcMirror(rRef1,rRef2);
	bMirrored = !bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    // #i118485# Call Shear now, old version redirected to rotate
	SdrRectObj::NbcShear(rRef, nWink, tn, bVShear);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetSnapRect(const Rectangle& rRect)
{
	SdrRectObj::NbcSetSnapRect(rRect);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::NbcSetLogicRect( const Rectangle& rRect)
{
	//int bChg=rRect.GetSize()!=aRect.GetSize();
	SdrRectObj::NbcSetLogicRect(rRect);
}

// -----------------------------------------------------------------------------

SdrObjGeoData* SdrGrafObj::NewGeoData() const
{
	return new SdrGrafObjGeoData;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
	SdrRectObj::SaveGeoData(rGeo);
	SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
	rGGeo.bMirrored=bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::RestGeoData(const SdrObjGeoData& rGeo)
{
	//long		nDrehMerk = aGeo.nDrehWink;
	//long		nShearMerk = aGeo.nShearWink;
	//int	bMirrMerk = bMirrored;
	Size		aSizMerk( aRect.GetSize() );

	SdrRectObj::RestGeoData(rGeo);
	SdrGrafObjGeoData& rGGeo=(SdrGrafObjGeoData&)rGeo;
	bMirrored=rGGeo.bMirrored;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetPage( SdrPage* pNewPage )
{
	FASTBOOL bRemove = pNewPage == NULL && pPage != NULL;
	FASTBOOL bInsert = pNewPage != NULL && pPage == NULL;

	if( bRemove )
	{
		// hier kein SwapIn noetig, weil wenn nicht geladen, dann auch nicht animiert.
		if( pGraphic->IsAnimated())
			pGraphic->StopAnimation();

		if( pGraphicLink != NULL )
			ImpLinkAbmeldung();
	}

    if(!pModel && !GetStyleSheet() && pNewPage->GetModel())
    {
        // #119287# Set default StyleSheet for SdrGrafObj here, it is different from 'Default'. This
        // needs to be done before the style 'Default' is set from the :SetModel() call which is triggered
        // from the following :SetPage().
        // TTTT: Needs to be moved in branch aw080 due to having a SdrModel from the beginning, is at this
        // place for convenience currently (works in both versions, is not in the way)
		SfxStyleSheet* pSheet = pNewPage->GetModel()->GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj();

		if(pSheet)
		{
			SetStyleSheet(pSheet, false);
		}
        else
        {
		    SetMergedItem(XFillStyleItem(XFILL_NONE));
		    SetMergedItem(XLineStyleItem(XLINE_NONE));
        }
    }

	SdrRectObj::SetPage( pNewPage );

	if(aFileName.Len() && bInsert)
		ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::SetModel( SdrModel* pNewModel )
{
	FASTBOOL bChg = pNewModel != pModel;

	if( bChg )
	{
		if( pGraphic->HasUserData() )
		{
			ForceSwapIn();
			pGraphic->SetUserData();
		}

		if( pGraphicLink != NULL )
			ImpLinkAbmeldung();
	}

	// Model umsetzen
	SdrRectObj::SetModel(pNewModel);

	if( bChg && aFileName.Len() )
		ImpLinkAnmeldung();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StartAnimation( OutputDevice* /*pOutDev*/, const Point& /*rPoint*/, const Size& /*rSize*/, long /*nExtraData*/)
{
	// #111096#
	// use new graf animation
	SetGrafAnimationAllowed(sal_True);
}

// -----------------------------------------------------------------------------

void SdrGrafObj::StopAnimation(OutputDevice* /*pOutDev*/, long /*nExtraData*/)
{
	// #111096#
	// use new graf animation
	SetGrafAnimationAllowed(sal_False);
}

// -----------------------------------------------------------------------------

FASTBOOL SdrGrafObj::HasGDIMetaFile() const
{
	return( pGraphic->GetType() == GRAPHIC_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

const GDIMetaFile* SdrGrafObj::GetGDIMetaFile() const
{
	DBG_ERROR( "Invalid return value! Don't use it! (KA)" );
	return &GetGraphic().GetGDIMetaFile();
}

// -----------------------------------------------------------------------------

bool SdrGrafObj::isEmbeddedSvg() const
{
    return GRAPHIC_BITMAP == GetGraphicType() && GetGraphic().getSvgData().get();
}

GDIMetaFile SdrGrafObj::getMetafileFromEmbeddedSvg() const
{
    GDIMetaFile aRetval;

    if(isEmbeddedSvg() && GetModel())
    {
        VirtualDevice aOut;
        const Rectangle aBoundRect(GetCurrentBoundRect());
        const MapMode aMap(GetModel()->GetScaleUnit(), Point(), GetModel()->GetScaleFraction(), GetModel()->GetScaleFraction());

        aOut.EnableOutput(false);
        aOut.SetMapMode(aMap);
        aRetval.Record(&aOut);
        SingleObjectPainter(aOut);
        aRetval.Stop();
        aRetval.WindStart();
        aRetval.Move(-aBoundRect.Left(), -aBoundRect.Top());
        aRetval.SetPrefMapMode(aMap);
        aRetval.SetPrefSize(aBoundRect.GetSize());
    }

    return aRetval;
}

SdrObject* SdrGrafObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
	SdrObject* pRetval = NULL;
    GraphicType aGraphicType(GetGraphicType());
    GDIMetaFile aMtf;

    if(isEmbeddedSvg())
    {
        // Embedded Svg
        // There is currently no helper to create SdrObjects from primitives (even if I'm thinking
        // about writing one for some time). To get the roundtrip to SdrObjects it is necessary to
        // use the old converter path over the MetaFile mechanism. Create Metafile from Svg 
        // primitives here pretty directly
        aMtf = getMetafileFromEmbeddedSvg();
        aGraphicType = GRAPHIC_GDIMETAFILE;
    }
    else if(GRAPHIC_GDIMETAFILE == aGraphicType)
    {
        aMtf = GetTransformedGraphic(SDRGRAFOBJ_TRANSFORMATTR_COLOR|SDRGRAFOBJ_TRANSFORMATTR_MIRROR).GetGDIMetaFile();
    }

	switch(aGraphicType)
	{
		case GRAPHIC_GDIMETAFILE:
		{
			// NUR die aus dem MetaFile erzeugbaren Objekte in eine Gruppe packen und zurueckliefern
			ImpSdrGDIMetaFileImport aFilter(*GetModel(), GetLayer(), aRect);
			SdrObjGroup* pGrp = new SdrObjGroup();
			sal_uInt32 nInsAnz = aFilter.DoImport(aMtf, *pGrp->GetSubList(), 0);

            if(nInsAnz)
			{
                {
                        // copy transformation
                	GeoStat aGeoStat(GetGeoStat());

	                if(aGeoStat.nShearWink) 
                    {
                        aGeoStat.RecalcTan();
                        pGrp->NbcShear(aRect.TopLeft(), aGeoStat.nShearWink, aGeoStat.nTan, false);
                    }

	                if(aGeoStat.nDrehWink) 
                    {
	                    aGeoStat.RecalcSinCos();
                        pGrp->NbcRotate(aRect.TopLeft(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
                    }
                }

                pRetval = pGrp;
				pGrp->NbcSetLayer(GetLayer());
				pGrp->SetModel(GetModel());
                
                if(bAddText)
                {
				    pRetval = ImpConvertAddText(pRetval, bBezier);
                }

                // convert all children
                if( pRetval )
                {
                    SdrObject* pHalfDone = pRetval;
                    pRetval = pHalfDone->DoConvertToPolyObj(bBezier, bAddText);
                    SdrObject::Free( pHalfDone ); // resulting object is newly created

                    if( pRetval )
                    {
                        // flatten subgroups. As we call
                        // DoConvertToPolyObj() on the resulting group
                        // objects, subgroups can exist (e.g. text is
                        // a group object for every line).
                        SdrObjList* pList = pRetval->GetSubList();
                        if( pList )
                            pList->FlattenGroups();
                    }
                }
			}
			else
            {
				delete pGrp;
            }

            // #i118485# convert line and fill
            SdrObject* pLineFill = SdrRectObj::DoConvertToPolyObj(bBezier, false);

            if(pLineFill)
            {
                if(pRetval)
                {
                    pGrp = dynamic_cast< SdrObjGroup* >(pRetval);

                    if(!pGrp)
                    {
            			pGrp = new SdrObjGroup();

                        pGrp->NbcSetLayer(GetLayer());
				        pGrp->SetModel(GetModel());
                        pGrp->GetSubList()->NbcInsertObject(pRetval);
                    }

                    pGrp->GetSubList()->NbcInsertObject(pLineFill, 0);
                }
                else
                {
                    pRetval = pLineFill;
                }
            }

			break;
		}
		case GRAPHIC_BITMAP:
		{
			// Grundobjekt kreieren und Fuellung ergaenzen
			pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);

			// Bitmap als Attribut retten
			if(pRetval)
			{
				// Bitmap als Fuellung holen
				SfxItemSet aSet(GetObjectItemSet());

				aSet.Put(XFillStyleItem(XFILL_BITMAP));
				const BitmapEx aBitmapEx(GetTransformedGraphic().GetBitmapEx());
				aSet.Put(XFillBitmapItem(String(), Graphic(aBitmapEx)));
				aSet.Put(XFillBmpTileItem(false));

				pRetval->SetMergedItemSet(aSet);
			}
			break;
		}
		case GRAPHIC_NONE:
		case GRAPHIC_DEFAULT:
		{
			pRetval = SdrRectObj::DoConvertToPolyObj(bBezier, bAddText);
			break;
		}
	}

	return pRetval;
}

// -----------------------------------------------------------------------------

void SdrGrafObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
	SetXPolyDirty();
	SdrRectObj::Notify( rBC, rHint );
	ImpSetAttrToGrafInfo();
}

void SdrGrafObj::ImpSetAttrToGrafInfo()
{
	const SfxItemSet& rSet = GetObjectItemSet();
	const sal_uInt16 nTrans = ( (SdrGrafTransparenceItem&) rSet.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue();
	const SdrGrafCropItem&	rCrop = (const SdrGrafCropItem&) rSet.Get( SDRATTR_GRAFCROP );

	aGrafInfo.SetLuminance( ( (SdrGrafLuminanceItem&) rSet.Get( SDRATTR_GRAFLUMINANCE ) ).GetValue() );
	aGrafInfo.SetContrast( ( (SdrGrafContrastItem&) rSet.Get( SDRATTR_GRAFCONTRAST ) ).GetValue() );
	aGrafInfo.SetChannelR( ( (SdrGrafRedItem&) rSet.Get( SDRATTR_GRAFRED ) ).GetValue() );
	aGrafInfo.SetChannelG( ( (SdrGrafGreenItem&) rSet.Get( SDRATTR_GRAFGREEN ) ).GetValue() );
	aGrafInfo.SetChannelB( ( (SdrGrafBlueItem&) rSet.Get( SDRATTR_GRAFBLUE ) ).GetValue() );
	aGrafInfo.SetGamma( ( (SdrGrafGamma100Item&) rSet.Get( SDRATTR_GRAFGAMMA ) ).GetValue() * 0.01 );
	aGrafInfo.SetTransparency( (sal_uInt8) FRound( Min( nTrans, (sal_uInt16) 100 )  * 2.55 ) );
	aGrafInfo.SetInvert( ( (SdrGrafInvertItem&) rSet.Get( SDRATTR_GRAFINVERT ) ).GetValue() );
	aGrafInfo.SetDrawMode( ( (SdrGrafModeItem&) rSet.Get( SDRATTR_GRAFMODE ) ).GetValue() );
	aGrafInfo.SetCrop( rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom() );

	SetXPolyDirty();
	SetRectsDirty();
}

// -----------------------------------------------------------------------------

void SdrGrafObj::ImpSetGrafInfoToAttr()
{
	SetObjectItem( SdrGrafLuminanceItem( aGrafInfo.GetLuminance() ) );
	SetObjectItem( SdrGrafContrastItem( aGrafInfo.GetContrast() ) );
	SetObjectItem( SdrGrafRedItem( aGrafInfo.GetChannelR() ) );
	SetObjectItem( SdrGrafGreenItem( aGrafInfo.GetChannelG() ) );
	SetObjectItem( SdrGrafBlueItem( aGrafInfo.GetChannelB() ) );
	SetObjectItem( SdrGrafGamma100Item( FRound( aGrafInfo.GetGamma() * 100.0 ) ) );
	SetObjectItem( SdrGrafTransparenceItem( (sal_uInt16) FRound( aGrafInfo.GetTransparency() / 2.55 ) ) );
	SetObjectItem( SdrGrafInvertItem( aGrafInfo.IsInvert() ) );
	SetObjectItem( SdrGrafModeItem( aGrafInfo.GetDrawMode() ) );
	SetObjectItem( SdrGrafCropItem( aGrafInfo.GetLeftCrop(), aGrafInfo.GetTopCrop(), aGrafInfo.GetRightCrop(), aGrafInfo.GetBottomCrop() ) );
}

// -----------------------------------------------------------------------------

void SdrGrafObj::AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly )
{
	Size aSize;
	Size aMaxSize( rMaxRect.GetSize() );
	if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
		aSize = Application::GetDefaultDevice()->PixelToLogic( pGraphic->GetPrefSize(), MAP_100TH_MM );
	else
		aSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
										    pGraphic->GetPrefMapMode(),
										    MapMode( MAP_100TH_MM ) );

	if( aSize.Height() != 0 && aSize.Width() != 0 )
	{
		Point aPos( rMaxRect.TopLeft() );

		// Falls Grafik zu gross, wird die Grafik
		// in die Seite eingepasst
		if ( (!bShrinkOnly                          ||
	    	 ( aSize.Height() > aMaxSize.Height() ) ||
		 	( aSize.Width()  > aMaxSize.Width()  ) )&&
		 	aSize.Height() && aMaxSize.Height() )
		{
			float fGrfWH =	(float)aSize.Width() /
							(float)aSize.Height();
			float fWinWH =	(float)aMaxSize.Width() /
							(float)aMaxSize.Height();

			// Grafik an Pagesize anpassen (skaliert)
			if ( fGrfWH < fWinWH )
			{
				aSize.Width() = (long)(aMaxSize.Height() * fGrfWH);
				aSize.Height()= aMaxSize.Height();
			}
			else if ( fGrfWH > 0.F )
			{
				aSize.Width() = aMaxSize.Width();
				aSize.Height()= (long)(aMaxSize.Width() / fGrfWH);
			}

			aPos = rMaxRect.Center();
		}

		if( bShrinkOnly )
			aPos = aRect.TopLeft();

		aPos.X() -= aSize.Width() / 2;
		aPos.Y() -= aSize.Height() / 2;
		SetLogicRect( Rectangle( aPos, aSize ) );
	}
}

// -----------------------------------------------------------------------------

IMPL_LINK( SdrGrafObj, ImpSwapHdl, GraphicObject*, pO )
{
	SvStream* pRet = GRFMGR_AUTOSWAPSTREAM_NONE;

	if( pO->IsInSwapOut() )
	{
		if( pModel && !mbIsPreview && pModel->IsSwapGraphics() && pGraphic->GetSizeBytes() > 20480 )
		{
			// test if this object is visualized from someone
            // ## test only if there are VOCs other than the preview renderer
			if(!GetViewContact().HasViewObjectContacts(true))
			{
				const sal_uIntPtr	nSwapMode = pModel->GetSwapGraphicsMode();

				if( ( pGraphic->HasUserData() || pGraphicLink ) &&
					( nSwapMode & SDR_SWAPGRAPHICSMODE_PURGE ) )
				{
					pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
				}
				else if( nSwapMode & SDR_SWAPGRAPHICSMODE_TEMP )
				{
					pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
					pGraphic->SetUserData();
				}

				// #i102380#
				sdr::contact::ViewContactOfGraphic* pVC = dynamic_cast< sdr::contact::ViewContactOfGraphic* >(&GetViewContact());

				if(pVC)
				{
					pVC->flushGraphicObjects();
				}
			}
		}
	}
	else if( pO->IsInSwapIn() )
	{
		// kann aus dem original Doc-Stream nachgeladen werden...
		if( pModel != NULL )
		{
			if( pGraphic->HasUserData() )
			{
				SdrDocumentStreamInfo aStreamInfo;

				aStreamInfo.mbDeleteAfterUse = sal_False;
				aStreamInfo.maUserData = pGraphic->GetUserData();

				SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );

				if( pStream != NULL )
				{
					Graphic aGraphic;

                    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData = NULL;
                    
					if(mbInsidePaint && !GetViewContact().HasViewObjectContacts(true))
                    {
                        pFilterData = new com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >( 3 );

                        const com::sun::star::awt::Size aPreviewSizeHint( 64, 64 );
                        const sal_Bool bAllowPartialStreamRead = sal_True;
                        // create <GfxLink> instance also for previews in order to avoid that its corresponding
                        // data is cleared in the graphic cache entry in case that the preview data equals the complete graphic data
                        const sal_Bool bCreateNativeLink = sal_True;
                        (*pFilterData)[ 0 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "PreviewSizeHint" ) );
                        (*pFilterData)[ 0 ].Value <<= aPreviewSizeHint;
                        (*pFilterData)[ 1 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "AllowPartialStreamRead" ) );
                        (*pFilterData)[ 1 ].Value <<= bAllowPartialStreamRead;
                        (*pFilterData)[ 2 ].Name = String( RTL_CONSTASCII_USTRINGPARAM( "CreateNativeLink" ) );
                        (*pFilterData)[ 2 ].Value <<= bCreateNativeLink;

                        mbIsPreview = sal_True;
                    }

                    if(!GraphicFilter::GetGraphicFilter()->ImportGraphic(
                        aGraphic, aStreamInfo.maUserData, *pStream,
                        GRFILTER_FORMAT_DONTKNOW, NULL, 0, pFilterData))
                    {
                        const String aUserData( pGraphic->GetUserData() );

                        pGraphic->SetGraphic( aGraphic );
                        pGraphic->SetUserData( aUserData );

                        // #142146# Graphic successfully swapped in.
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    delete pFilterData;

                    pStream->ResetError();

                    if( aStreamInfo.mbDeleteAfterUse || aStreamInfo.mxStorageRef.is() )
                    {
                        if ( aStreamInfo.mxStorageRef.is() )
                        {
                            aStreamInfo.mxStorageRef->dispose();
                            aStreamInfo.mxStorageRef = 0;
                        }

                        delete pStream;
                    }
                }
			}
			else if( !ImpUpdateGraphicLink( sal_False ) )
            {
				pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
            }
			else
            {
                pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
            }
		}
		else
			pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;
	}

	return (long)(void*) pRet;
}

// -----------------------------------------------------------------------------

// #111096#
// Access to GrafAnimationAllowed flag
sal_Bool SdrGrafObj::IsGrafAnimationAllowed() const
{
	return mbGrafAnimationAllowed;
}

void SdrGrafObj::SetGrafAnimationAllowed(sal_Bool bNew)
{
	if(mbGrafAnimationAllowed != bNew)
	{
		mbGrafAnimationAllowed = bNew;
		ActionChanged();
	}
}

// #i25616#
sal_Bool SdrGrafObj::IsObjectTransparent() const
{
	if(((const SdrGrafTransparenceItem&)GetObjectItem(SDRATTR_GRAFTRANSPARENCE)).GetValue()
		|| pGraphic->IsTransparent())
	{
		return sal_True;
	}

	return sal_False;
}

Reference< XInputStream > SdrGrafObj::getInputStream()
{
	Reference< XInputStream > xStream;

	if( pModel )
	{
//		if( !pGraphic->HasUserData() )
//			pGraphic->SwapOut();

		// kann aus dem original Doc-Stream nachgeladen werden...
		if( pGraphic->HasUserData() )
		{
			SdrDocumentStreamInfo aStreamInfo;

			aStreamInfo.mbDeleteAfterUse = sal_False;
			aStreamInfo.maUserData = pGraphic->GetUserData();

			SvStream* pStream = pModel->GetDocumentStream( aStreamInfo );

			if( pStream )
				xStream.set( new utl::OInputStreamWrapper( pStream, sal_True ) );
		}
		else if( pGraphic && GetGraphic().IsLink() )
		{
			Graphic aGraphic( GetGraphic() );
			GfxLink aLink( aGraphic.GetLink() );
			sal_uInt32 nSize = aLink.GetDataSize();
			const void* pSourceData = (const void*)aLink.GetData();
			if( nSize && pSourceData )
			{
				sal_uInt8 * pBuffer = new sal_uInt8[ nSize ];
				if( pBuffer )
				{
					memcpy( pBuffer, pSourceData, nSize );

					SvMemoryStream* pStream = new SvMemoryStream( (void*)pBuffer, (sal_Size)nSize, STREAM_READ );
					pStream->ObjectOwnsMemory( sal_True );
					xStream.set( new utl::OInputStreamWrapper( pStream, sal_True ) );
				}
			}
		}

		if( !xStream.is() && aFileName.Len() )
		{
			SvFileStream* pStream = new SvFileStream( aFileName, STREAM_READ );
			if( pStream )
				xStream.set( new utl::OInputStreamWrapper( pStream ) );
		}
	}

	return xStream;
}

// moved crop handle creation here; this is the object type using them
void SdrGrafObj::addCropHandles(SdrHdlList& rTarget) const
{
    basegfx::B2DHomMatrix aMatrix;
    basegfx::B2DPolyPolygon aPolyPolygon;

    // get object transformation
    TRGetBaseGeometry(aMatrix, aPolyPolygon);

    // part of object transformation correction, but used later, so defined outside next scope
    double fShearX(0.0), fRotate(0.0);

    {   // TTTT correct shear, it comes currently mirrored from TRGetBaseGeometry, can be removed with aw080
        basegfx::B2DTuple aScale;
        basegfx::B2DTuple aTranslate;

        aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

        if(!basegfx::fTools::equalZero(fShearX))
        {
            // shearX is used, correct it
            fShearX = -fShearX;
        }

        aMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
            aScale,
            fShearX,
            fRotate,
            aTranslate);
    }

    // get crop values
    const SdrGrafCropItem& rCrop = static_cast< const SdrGrafCropItem& >(GetMergedItem(SDRATTR_GRAFCROP));

    if(rCrop.GetLeft() || rCrop.GetTop() || rCrop.GetRight() ||rCrop.GetBottom())
    {
        // decompose object transformation to have current translate and scale
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        aMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

        if(!aScale.equalZero())
        {
            // get crop scale
            const basegfx::B2DVector aCropScaleFactor(
                GetGraphicObject().calculateCropScaling(
                    aScale.getX(),
                    aScale.getY(),
                    rCrop.GetLeft(),
                    rCrop.GetTop(),
                    rCrop.GetRight(),
                    rCrop.GetBottom()));

            // apply crop scale
            const double fCropLeft(rCrop.GetLeft() * aCropScaleFactor.getX());
            const double fCropTop(rCrop.GetTop() * aCropScaleFactor.getY());
            const double fCropRight(rCrop.GetRight() * aCropScaleFactor.getX());
            const double fCropBottom(rCrop.GetBottom() * aCropScaleFactor.getY());
            basegfx::B2DHomMatrix aMatrixForCropViewHdl(aMatrix);

            if(IsMirrored())
            {
                // create corrected new matrix, TTTT can be removed with aw080
                // the old mirror only can mirror horizontally; the vertical mirror
                // is faked by using the horizontal and 180 degree rotation. Since
                // the object can be rotated differently from 180 degree, this is
                // not safe to detect. Just correct horizontal mirror (which is
                // in IsMirrored()) and keep the rotation angle
                // caution: Do not modify aMatrix, it is used below to calculate
                // the exact handle positions
                basegfx::B2DHomMatrix aPreMultiply;

                // mirrored X, apply
                aPreMultiply.translate(-0.5, 0.0);
                aPreMultiply.scale(-1.0, 1.0);
                aPreMultiply.translate(0.5, 0.0);

                aMatrixForCropViewHdl = aMatrixForCropViewHdl * aPreMultiply;
            }

            rTarget.AddHdl(
                new SdrCropViewHdl(
                    aMatrixForCropViewHdl,
                    GetGraphicObject().GetGraphic(),
                    fCropLeft,
                    fCropTop,
                    fCropRight,
                    fCropBottom));
        }
    }

    basegfx::B2DPoint aPos;

    aPos = aMatrix * basegfx::B2DPoint(0.0, 0.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPLFT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 0.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPPER, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_UPRGT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 0.5); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LEFT , fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 0.5); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_RIGHT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.0, 1.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LWLFT, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(0.5, 1.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LOWER, fShearX, fRotate));
    aPos = aMatrix * basegfx::B2DPoint(1.0, 1.0); 
    rTarget.AddHdl(new SdrCropHdl(Point(basegfx::fround(aPos.getX()), basegfx::fround(aPos.getY())), HDL_LWRGT, fShearX, fRotate));
}

// eof
