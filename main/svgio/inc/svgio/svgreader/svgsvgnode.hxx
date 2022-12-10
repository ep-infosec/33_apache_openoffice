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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGSVGNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGSVGNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgSvgNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DRange*      mpViewBox;
            SvgAspectRatio          maSvgAspectRatio;
            SvgNumber               maX;
            SvgNumber               maY;
            SvgNumber               maWidth;
            SvgNumber               maHeight;
            SvgNumber               maVersion;

            /// #125258# bitfield
            bool                    mbStyleAttributesInitialized : 1;

            // #125258# on-demand init hard attributes when this is the outmost svg element
            // and more (see implementation)
            void initializeStyleAttributes();

        public:
            SvgSvgNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgSvgNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// Seeks width and height of viewport, which is current before the new viewport is set.
            // needed for percentage unit in x, y, width or height
            virtual void seekReferenceWidth(double& fWidth, bool& bHasFound) const;
            virtual void seekReferenceHeight(double& fHeight, bool& bHasFound) const;

            /// InfoProvider support for % values in childs
            // The returned 'CurrentViewPort' is the viewport as it is set by this svg element
            // and as it is needed to resolve relative values in childs
            // The method does not check for invalid width and height
            virtual const basegfx::B2DRange getCurrentViewPort() const;

            /// viewBox content
            const basegfx::B2DRange* getViewBox() const { return mpViewBox; }
            void setViewBox(const basegfx::B2DRange* pViewBox = 0) { if(mpViewBox) delete mpViewBox; mpViewBox = 0; if(pViewBox) mpViewBox = new basegfx::B2DRange(*pViewBox); }

            /// SvgAspectRatio content
            const SvgAspectRatio& getSvgAspectRatio() const { return maSvgAspectRatio; }
            void setSvgAspectRatio(const SvgAspectRatio& rSvgAspectRatio = SvgAspectRatio()) { maSvgAspectRatio = rSvgAspectRatio; }

            /// x content
            const SvgNumber& getX() const { return maX; }
            void setX(const SvgNumber& rX = SvgNumber()) { maX = rX; }

            /// y content
            const SvgNumber& getY() const { return maY; }
            void setY(const SvgNumber& rY = SvgNumber()) { maY = rY; }

            /// width content
            const SvgNumber& getWidth() const { return maWidth; }
            void setWidth(const SvgNumber& rWidth = SvgNumber()) { maWidth = rWidth; }

            /// height content
            const SvgNumber& getHeight() const { return maHeight; }
            void setHeight(const SvgNumber& rHeight = SvgNumber()) { maHeight = rHeight; }

            /// version content
            const SvgNumber& getVersion() const { return maVersion; }
            void setVersion(const SvgNumber& rVersion = SvgNumber()) { maVersion = rVersion; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGSVGNODE_HXX

// eof
