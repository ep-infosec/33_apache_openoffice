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
#include "precompiled_vcl.hxx"

#include <vcl/svgdata.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XSvgParser.hpp>
#include <com/sun/star/graphic/XPrimitive2DRenderer.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <vcl/canvastools.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

BitmapEx VCL_DLLPUBLIC convertPrimitive2DSequenceToBitmapEx(
    const Primitive2DSequence& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels)
{
    BitmapEx aRetval;

    if(rSequence.hasElements())
    {
        // create replacement graphic from maSequence
        // create XPrimitive2DRenderer
        uno::Reference< lang::XMultiServiceFactory > xFactory(::comphelper::getProcessServiceFactory());
        const rtl::OUString aServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.Primitive2DTools"));

        try
        {
            const uno::Reference< graphic::XPrimitive2DRenderer > xPrimitive2DRenderer(xFactory->createInstance(aServiceName), uno::UNO_QUERY_THROW);

            if(xPrimitive2DRenderer.is())
            {
                uno::Sequence< beans::PropertyValue > aViewParameters;
                geometry::RealRectangle2D aRealRect;

                aRealRect.X1 = rTargetRange.getMinX();
                aRealRect.Y1 = rTargetRange.getMinY();
                aRealRect.X2 = rTargetRange.getMaxX();
                aRealRect.Y2 = rTargetRange.getMaxY();

                // get system DPI
                const Size aDPI(Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MAP_INCH));

                const uno::Reference< rendering::XBitmap > xBitmap(
                    xPrimitive2DRenderer->rasterize( 
                        rSequence,
                        aViewParameters, 
                        aDPI.getWidth(), 
                        aDPI.getHeight(), 
                        aRealRect, 
                        nMaximumQuadraticPixels));

                if(xBitmap.is())
                {
                    const uno::Reference< rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

                    if(xIntBmp.is())
                    {
                        aRetval = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
                    }
                }
            }
        }
        catch(const uno::Exception&)
        {
            OSL_ENSURE(sal_False, "Got no graphic::XPrimitive2DRenderer (!)" );
        }
    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////

void SvgData::ensureReplacement()
{
    ensureSequenceAndRange();

    if(maReplacement.IsEmpty() && maSequence.hasElements())
    {
        maReplacement = convertPrimitive2DSequenceToBitmapEx(maSequence, getRange());
    }
}

//////////////////////////////////////////////////////////////////////////////

void SvgData::ensureSequenceAndRange()
{
    if(!maSequence.hasElements() && mnSvgDataArrayLength)
    {
        // import SVG to maSequence, also set maRange
        maRange.reset();

        // create stream
        const uno::Sequence< sal_Int8 > aPostData((sal_Int8*)maSvgDataArray.get(), mnSvgDataArrayLength);
        const uno::Reference< io::XInputStream > myInputStream(new comphelper::SequenceInputStream(aPostData));

        if(myInputStream.is())
        {
            // create SVG interpreter
            uno::Reference< lang::XMultiServiceFactory > xFactory(::comphelper::getProcessServiceFactory());
            const rtl::OUString aServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.graphic.SvgTools"));

            try
            {
                const uno::Reference< graphic::XSvgParser > xSvgParser(xFactory->createInstance(aServiceName), uno::UNO_QUERY_THROW);

                if(xSvgParser.is())
                {
                    maSequence = xSvgParser->getDecomposition(myInputStream, maPath);
                }
            }
            catch(const uno::Exception&)
            {
                OSL_ENSURE(sal_False, "Got no graphic::XSvgParser (!)" );
            }
        }
        
        if(maSequence.hasElements())
        {
            const sal_Int32 nCount(maSequence.getLength());
            geometry::RealRectangle2D aRealRect;
            uno::Sequence< beans::PropertyValue > aViewParameters;

            for(sal_Int32 a(0L); a < nCount; a++)
            {
                // get reference
                const Primitive2DReference xReference(maSequence[a]);

                if(xReference.is())
                {
                    aRealRect = xReference->getRange(aViewParameters);

                    maRange.expand(
                        basegfx::B2DRange(
                            aRealRect.X1,
                            aRealRect.Y1,
                            aRealRect.X2,
                            aRealRect.Y2));
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SvgData::SvgData(const SvgDataArray& rSvgDataArray, sal_uInt32 nSvgDataArrayLength, const rtl::OUString& rPath)
:   maSvgDataArray(rSvgDataArray),
    mnSvgDataArrayLength(nSvgDataArrayLength),
    maPath(rPath),
    maRange(),
    maSequence(),
    maReplacement()
{
}

//////////////////////////////////////////////////////////////////////////////

const basegfx::B2DRange& SvgData::getRange() const
{
    const_cast< SvgData* >(this)->ensureSequenceAndRange();

    return maRange;
}

//////////////////////////////////////////////////////////////////////////////

const Primitive2DSequence& SvgData::getPrimitive2DSequence() const
{
    const_cast< SvgData* >(this)->ensureSequenceAndRange();

    return maSequence;
}

//////////////////////////////////////////////////////////////////////////////

const BitmapEx& SvgData::getReplacement() const
{
    const_cast< SvgData* >(this)->ensureReplacement();

    return maReplacement;
}

//////////////////////////////////////////////////////////////////////////////
// eof
