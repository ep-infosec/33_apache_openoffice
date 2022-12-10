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
#include "precompiled_sdext.hxx"

#include "graphiccollector.hxx"
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>

#include "impoptimizer.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::presentation;

using ::rtl::OUString;

const DeviceInfo& GraphicCollector::GetDeviceInfo( const Reference< XComponentContext >& rxFact )
{
	static DeviceInfo aDeviceInfo;
	if( !aDeviceInfo.Width )
	{
		try
		{
			Reference< XFramesSupplier > xDesktop( rxFact->getServiceManager()->createInstanceWithContext(
					OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ), rxFact ), UNO_QUERY_THROW );
			Reference< XFrame > xFrame( xDesktop->getActiveFrame() );
			Reference< XWindow > xWindow( xFrame->getContainerWindow() );
			Reference< XDevice > xDevice( xWindow, UNO_QUERY_THROW );
			aDeviceInfo = xDevice->getInfo();
		}
		catch( Exception& )
		{
		}
	}
	return aDeviceInfo;
}

void ImpAddEntity( std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities, const GraphicSettings& rGraphicSettings, const GraphicCollector::GraphicUser& rUser )
{
	const rtl::OUString aGraphicURL( rUser.maGraphicURL );
	const rtl::OUString sPackageURL( OUString::createFromAscii( "vnd.sun.star.GraphicObject:" ) );

	if ( rGraphicSettings.mbEmbedLinkedGraphics || ( !aGraphicURL.getLength() || aGraphicURL.match( sPackageURL, 0 ) ) )
	{
		std::vector< GraphicCollector::GraphicEntity >::iterator aIter( rGraphicEntities.begin() );
		while( aIter != rGraphicEntities.end() )
		{
			if ( aIter->maUser[ 0 ].maGraphicURL == aGraphicURL )
			{
				if ( rUser.maLogicalSize.Width > aIter->maLogicalSize.Width )
					aIter->maLogicalSize.Width = rUser.maLogicalSize.Width;
				if ( rUser.maLogicalSize.Height > aIter->maLogicalSize.Height )
					aIter->maLogicalSize.Height = rUser.maLogicalSize.Height;
				aIter->maUser.push_back( rUser );
				break;
			}
			aIter++;
		}
		if ( aIter == rGraphicEntities.end() )
		{
			GraphicCollector::GraphicEntity aEntity( rUser );
			rGraphicEntities.push_back( aEntity );
		}
	}
}

void ImpAddGraphicEntity( const Reference< XComponentContext >& rxContext, Reference< XShape >& rxShape, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
	Reference< XGraphic > xGraphic;
	Reference< XPropertySet > xShapePropertySet( rxShape, UNO_QUERY_THROW );
	if ( xShapePropertySet->getPropertyValue( TKGet( TK_Graphic ) ) >>= xGraphic )
	{
		text::GraphicCrop aGraphicCropLogic( 0, 0, 0, 0 );

		GraphicCollector::GraphicUser aUser;
		aUser.mxShape = rxShape;
		aUser.mbFillBitmap = sal_False;
		xShapePropertySet->getPropertyValue( TKGet( TK_GraphicURL ) ) >>= aUser.maGraphicURL;
		xShapePropertySet->getPropertyValue( TKGet( TK_GraphicStreamURL ) ) >>= aUser.maGraphicStreamURL;
		xShapePropertySet->getPropertyValue( TKGet( TK_GraphicCrop ) ) >>= aGraphicCropLogic;
		awt::Size aLogicalSize( rxShape->getSize() );

		// calculating the logical size, as if there were no cropping
		if ( aGraphicCropLogic.Left || aGraphicCropLogic.Right || aGraphicCropLogic.Top || aGraphicCropLogic.Bottom )
		{
			awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxContext, xGraphic ) );
			if ( aSize100thMM.Width && aSize100thMM.Height )
			{
				awt::Size aCropSize( aSize100thMM.Width - ( aGraphicCropLogic.Left + aGraphicCropLogic.Right ),
									 aSize100thMM.Height - ( aGraphicCropLogic.Top + aGraphicCropLogic.Bottom ));
				if ( aCropSize.Width && aCropSize.Height )
				{
					awt::Size aNewLogSize( static_cast< sal_Int32 >( static_cast< double >( aSize100thMM.Width * aLogicalSize.Width ) / aCropSize.Width ),
						static_cast< sal_Int32 >( static_cast< double >( aSize100thMM.Height * aLogicalSize.Height ) / aCropSize.Height ) );
					aLogicalSize = aNewLogSize;
				}
			}
		}
		aUser.maGraphicCropLogic = aGraphicCropLogic;
		aUser.maLogicalSize = aLogicalSize;
		ImpAddEntity( rGraphicEntities, rGraphicSettings, aUser );
	}
}

void ImpAddFillBitmapEntity( const Reference< XComponentContext >& rxContext, const Reference< XPropertySet >& rxPropertySet, const awt::Size& rLogicalSize,
	std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities, const GraphicSettings& rGraphicSettings, const Reference< XPropertySet >& rxPagePropertySet )
{
	try
	{
		FillStyle eFillStyle;
		if ( rxPropertySet->getPropertyValue( TKGet( TK_FillStyle ) ) >>= eFillStyle )
		{
			if ( eFillStyle == FillStyle_BITMAP )
			{
				rtl::OUString aFillBitmapURL;
				Reference< XBitmap > xFillBitmap;
				if ( rxPropertySet->getPropertyValue( TKGet( TK_FillBitmap ) ) >>= xFillBitmap )
				{
					Reference< XGraphic > xGraphic( xFillBitmap, UNO_QUERY_THROW );
					if ( xGraphic.is() )
					{
						awt::Size aLogicalSize( rLogicalSize );
						Reference< XPropertySetInfo > axPropSetInfo( rxPropertySet->getPropertySetInfo() );
						if ( axPropSetInfo.is() )
						{
							if ( axPropSetInfo->hasPropertyByName( TKGet( TK_FillBitmapMode ) ) )
							{
								BitmapMode eBitmapMode;
								if ( rxPropertySet->getPropertyValue( TKGet( TK_FillBitmapMode ) ) >>= eBitmapMode )
								{
									if ( ( eBitmapMode == BitmapMode_REPEAT ) || ( eBitmapMode == BitmapMode_NO_REPEAT ) )
									{
										sal_Bool bLogicalSize = sal_False;
										awt::Size aSize( 0, 0 );
										if ( ( rxPropertySet->getPropertyValue( TKGet( TK_FillBitmapLogicalSize ) ) >>= bLogicalSize )
										  && ( rxPropertySet->getPropertyValue( TKGet( TK_FillBitmapSizeX ) ) >>= aSize.Width )
										  && ( rxPropertySet->getPropertyValue( TKGet( TK_FillBitmapSizeY ) ) >>= aSize.Height ) )
										{
											if ( bLogicalSize )
											{
												if ( !aSize.Width || !aSize.Height )
												{
													awt::Size aSize100thMM( GraphicCollector::GetOriginalSize( rxContext, xGraphic ) );
													if ( aSize100thMM.Width && aSize100thMM.Height )
														aLogicalSize = aSize100thMM;
												}
												else
													aLogicalSize = aSize;
											}
											else
											{
												aLogicalSize.Width = sal::static_int_cast< sal_Int32 >( ( static_cast< double >( aLogicalSize.Width ) * aSize.Width ) / -100.0 );
												aLogicalSize.Height = sal::static_int_cast< sal_Int32 >( ( static_cast< double >( aLogicalSize.Height ) * aSize.Height ) / -100.0 );
											}
										}
									}
								}
							}
						}
						GraphicCollector::GraphicUser aUser;
						aUser.mxPropertySet = rxPropertySet;
						rxPropertySet->getPropertyValue( TKGet( TK_FillBitmapURL ) ) >>= aUser.maGraphicURL;
						aUser.mbFillBitmap = sal_True;
						aUser.maLogicalSize = aLogicalSize;
						aUser.mxPagePropertySet = rxPagePropertySet;
						ImpAddEntity( rGraphicEntities, rGraphicSettings, aUser );
					}
				}
			}
		}
	}
	catch( Exception& )
	{
	}
}

void ImpCollectBackgroundGraphic( const Reference< XComponentContext >& rxContext, const Reference< XDrawPage >& rxDrawPage, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
	try
	{
		awt::Size aLogicalSize( 28000, 21000 );
		Reference< XPropertySet > xPropertySet( rxDrawPage, UNO_QUERY_THROW );
		xPropertySet->getPropertyValue( TKGet( TK_Width ) ) >>= aLogicalSize.Width;
		xPropertySet->getPropertyValue( TKGet( TK_Height ) ) >>= aLogicalSize.Height;
		
		Reference< XPropertySet > xBackgroundPropSet;
		if ( xPropertySet->getPropertyValue( TKGet( TK_Background ) ) >>= xBackgroundPropSet )
			ImpAddFillBitmapEntity( rxContext, xBackgroundPropSet, aLogicalSize, rGraphicEntities, rGraphicSettings, xPropertySet );
	}
	catch( Exception& )
	{
	}
}

void ImpCollectGraphicObjects( const Reference< XComponentContext >& rxContext, const Reference< XShapes >& rxShapes, const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicEntities )
{
	for ( sal_Int32 i = 0; i < rxShapes->getCount(); i++ )
	{
		try
		{
			const OUString sGraphicObjectShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GraphicObjectShape" ) );
			const OUString sGroupShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GroupShape" ) );
			Reference< XShape > xShape( rxShapes->getByIndex( i ), UNO_QUERY_THROW );
			const OUString sShapeType( xShape->getShapeType() );
			if ( sShapeType == sGroupShape )
			{
				Reference< XShapes > xShapes( xShape, UNO_QUERY_THROW );
				ImpCollectGraphicObjects( rxContext, xShapes, rGraphicSettings, rGraphicEntities );
				continue;
			}

			if ( sShapeType == sGraphicObjectShape )
				ImpAddGraphicEntity( rxContext, xShape, rGraphicSettings, rGraphicEntities );

			// now check for a fillstyle
			Reference< XPropertySet > xEmptyPagePropSet;
			Reference< XPropertySet > xShapePropertySet( xShape, UNO_QUERY_THROW );
			awt::Size aLogicalSize( xShape->getSize() );
			ImpAddFillBitmapEntity( rxContext, xShapePropertySet, aLogicalSize, rGraphicEntities, rGraphicSettings, xEmptyPagePropSet );
		}
		catch( Exception& )
		{
		}
	}
}

awt::Size GraphicCollector::GetOriginalSize( const Reference< XComponentContext >& rxContext, const Reference< XGraphic >& rxGraphic )
{
	awt::Size aSize100thMM( 0, 0 );
	Reference< XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
	if ( xGraphicPropertySet->getPropertyValue( TKGet( TK_Size100thMM ) ) >>= aSize100thMM )
	{
		if ( !aSize100thMM.Width && !aSize100thMM.Height )
		{	// MAPMODE_PIXEL USED :-(
			awt::Size aSourceSizePixel( 0, 0 );
			if ( xGraphicPropertySet->getPropertyValue( TKGet( TK_SizePixel ) ) >>= aSourceSizePixel )
			{
				const DeviceInfo& rDeviceInfo( GraphicCollector::GetDeviceInfo( rxContext ) );
				if ( rDeviceInfo.PixelPerMeterX && rDeviceInfo.PixelPerMeterY )
				{
					aSize100thMM.Width = static_cast< sal_Int32 >( ( aSourceSizePixel.Width * 100000.0 ) / rDeviceInfo.PixelPerMeterX );
					aSize100thMM.Height = static_cast< sal_Int32 >( ( aSourceSizePixel.Height * 100000.0 ) / rDeviceInfo.PixelPerMeterY );
				}
			}
		}
	}
	return aSize100thMM;
}

void GraphicCollector::CollectGraphics( const Reference< XComponentContext >& rxContext, const Reference< XModel >& rxModel,
		const GraphicSettings& rGraphicSettings, std::vector< GraphicCollector::GraphicEntity >& rGraphicList )
{
	try
	{
		sal_Int32 i;
		Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
		Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
		for ( i = 0; i < xDrawPages->getCount(); i++ )
		{
			Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
			ImpCollectBackgroundGraphic( rxContext, xDrawPage, rGraphicSettings, rGraphicList );
			Reference< XShapes > xDrawShapes( xDrawPage, UNO_QUERY_THROW );
			ImpCollectGraphicObjects( rxContext, xDrawShapes, rGraphicSettings, rGraphicList );

			Reference< XPresentationPage > xPresentationPage( xDrawPage, UNO_QUERY_THROW );
			Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
			ImpCollectBackgroundGraphic( rxContext, xNotesPage, rGraphicSettings, rGraphicList );
			Reference< XShapes > xNotesShapes( xNotesPage, UNO_QUERY_THROW );
			ImpCollectGraphicObjects( rxContext, xNotesShapes, rGraphicSettings, rGraphicList );
		}
		Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
		Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
		for ( i = 0; i < xMasterPages->getCount(); i++ )
		{
			Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
			ImpCollectBackgroundGraphic( rxContext, xMasterPage, rGraphicSettings, rGraphicList );
			Reference< XShapes > xMasterPageShapes( xMasterPage, UNO_QUERY_THROW );
			ImpCollectGraphicObjects( rxContext, xMasterPageShapes, rGraphicSettings, rGraphicList );
		}

		std::vector< GraphicCollector::GraphicEntity >::iterator aGraphicIter( rGraphicList.begin() );
		std::vector< GraphicCollector::GraphicEntity >::iterator aGraphicIEnd( rGraphicList.end() );
		while( aGraphicIter != aGraphicIEnd )
		{
			// check if it is possible to remove the crop area
			aGraphicIter->mbRemoveCropArea = rGraphicSettings.mbRemoveCropArea;
			if ( aGraphicIter->mbRemoveCropArea )
			{
				std::vector< GraphicCollector::GraphicUser >::iterator aGUIter( aGraphicIter->maUser.begin() );
				while( aGraphicIter->mbRemoveCropArea && ( aGUIter != aGraphicIter->maUser.end() ) )
				{
					if ( aGUIter->maGraphicCropLogic.Left || aGUIter->maGraphicCropLogic.Top
						|| aGUIter->maGraphicCropLogic.Right || aGUIter->maGraphicCropLogic.Bottom )
					{
						if ( aGUIter == aGraphicIter->maUser.begin() )
							aGraphicIter->maGraphicCropLogic = aGUIter->maGraphicCropLogic;
						else if ( ( aGraphicIter->maGraphicCropLogic.Left != aGUIter->maGraphicCropLogic.Left )
							|| ( aGraphicIter->maGraphicCropLogic.Top != aGUIter->maGraphicCropLogic.Top )
							|| ( aGraphicIter->maGraphicCropLogic.Right != aGUIter->maGraphicCropLogic.Right )
							|| ( aGraphicIter->maGraphicCropLogic.Bottom != aGUIter->maGraphicCropLogic.Bottom ) )
						{
							aGraphicIter->mbRemoveCropArea = sal_False;
						}
					}
					else
						aGraphicIter->mbRemoveCropArea = sal_False;
					aGUIter++;
				}
			}
			if ( !aGraphicIter->mbRemoveCropArea )
				aGraphicIter->maGraphicCropLogic = text::GraphicCrop( 0, 0, 0, 0 );
			aGraphicIter++;
		}
	}
	catch ( Exception& )
	{
	}
}

void ImpCountGraphicObjects( const Reference< XComponentContext >& rxContext, const Reference< XShapes >& rxShapes, const GraphicSettings& rGraphicSettings, sal_Int32& rnGraphics )
{
	for ( sal_Int32 i = 0; i < rxShapes->getCount(); i++ )
	{
		try
		{
			const OUString sGraphicObjectShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GraphicObjectShape" ) );
			const OUString sGroupShape( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GroupShape" ) );
			Reference< XShape > xShape( rxShapes->getByIndex( i ), UNO_QUERY_THROW );
			const OUString sShapeType( xShape->getShapeType() );
			if ( sShapeType == sGroupShape )
			{
				Reference< XShapes > xShapes( xShape, UNO_QUERY_THROW );
				ImpCountGraphicObjects( rxContext, xShapes, rGraphicSettings, rnGraphics );
				continue;
			}

			if ( sShapeType == sGraphicObjectShape )
			{
				rnGraphics++;
			}

			// now check for a fillstyle
			Reference< XPropertySet > xEmptyPagePropSet;
			Reference< XPropertySet > xShapePropertySet( xShape, UNO_QUERY_THROW );
			awt::Size aLogicalSize( xShape->getSize() );

			FillStyle eFillStyle;
			if ( xShapePropertySet->getPropertyValue( TKGet( TK_FillStyle ) ) >>= eFillStyle )
			{
				if ( eFillStyle == FillStyle_BITMAP )
				{
					rnGraphics++;
				}
			}
		}
		catch( Exception& )
		{
		}
	}
}

void ImpCountBackgroundGraphic( const Reference< XComponentContext >& /* rxContext */, const Reference< XDrawPage >& rxDrawPage,
							   const GraphicSettings& /* rGraphicSettings */, sal_Int32& rnGraphics )
{
	try
	{
		awt::Size aLogicalSize( 28000, 21000 );
		Reference< XPropertySet > xPropertySet( rxDrawPage, UNO_QUERY_THROW );
		xPropertySet->getPropertyValue( TKGet( TK_Width ) ) >>= aLogicalSize.Width;
		xPropertySet->getPropertyValue( TKGet( TK_Height ) ) >>= aLogicalSize.Height;
		
		Reference< XPropertySet > xBackgroundPropSet;
		if ( xPropertySet->getPropertyValue( TKGet( TK_Background ) ) >>= xBackgroundPropSet )
		{
			FillStyle eFillStyle;
			if ( xBackgroundPropSet->getPropertyValue( TKGet( TK_FillStyle ) ) >>= eFillStyle )
			{
				if ( eFillStyle == FillStyle_BITMAP )
				{
					rnGraphics++;
				}
			}
		}
	}
	catch( Exception& )
	{
	}
}

void GraphicCollector::CountGraphics( const Reference< XComponentContext >& rxContext, const Reference< XModel >& rxModel,
		const GraphicSettings& rGraphicSettings, sal_Int32& rnGraphics )
{
	try
	{
		sal_Int32 i;
		Reference< XDrawPagesSupplier > xDrawPagesSupplier( rxModel, UNO_QUERY_THROW );
		Reference< XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY_THROW );
		for ( i = 0; i < xDrawPages->getCount(); i++ )
		{
			Reference< XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), UNO_QUERY_THROW );
			ImpCountBackgroundGraphic( rxContext, xDrawPage, rGraphicSettings, rnGraphics );
			Reference< XShapes > xDrawShapes( xDrawPage, UNO_QUERY_THROW );
			ImpCountGraphicObjects( rxContext, xDrawShapes, rGraphicSettings, rnGraphics );

			Reference< XPresentationPage > xPresentationPage( xDrawPage, UNO_QUERY_THROW );
			Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
			ImpCountBackgroundGraphic( rxContext, xNotesPage, rGraphicSettings, rnGraphics );
			Reference< XShapes > xNotesShapes( xNotesPage, UNO_QUERY_THROW );
			ImpCountGraphicObjects( rxContext, xNotesShapes, rGraphicSettings, rnGraphics );
		}
		Reference< XMasterPagesSupplier > xMasterPagesSupplier( rxModel, UNO_QUERY_THROW );
		Reference< XDrawPages > xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY_THROW );
		for ( i = 0; i < xMasterPages->getCount(); i++ )
		{
			Reference< XDrawPage > xMasterPage( xMasterPages->getByIndex( i ), UNO_QUERY_THROW );
			ImpCountBackgroundGraphic( rxContext, xMasterPage, rGraphicSettings, rnGraphics );
			Reference< XShapes > xMasterPageShapes( xMasterPage, UNO_QUERY_THROW );
			ImpCountGraphicObjects( rxContext, xMasterPageShapes, rGraphicSettings, rnGraphics );
		}
	}
	catch ( Exception& )
	{
	}
}

