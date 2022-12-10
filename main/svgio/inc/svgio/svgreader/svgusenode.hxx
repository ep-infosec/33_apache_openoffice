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

#ifndef INCLUDED_SVGIO_SVGREADER_USENODE_HXX
#define INCLUDED_SVGIO_SVGREADER_USENODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgUseNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            basegfx::B2DHomMatrix*      mpaTransform;
            SvgNumber                   maX;
            SvgNumber                   maY;
            SvgNumber                   maWidth;
            SvgNumber                   maHeight;

            /// link to content. If maXLink is set, the node can be fetched
            // on demand
            rtl::OUString               maXLink;

        public:
            SvgUseNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgUseNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// transform content
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }

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
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_USENODE_HXX

// eof
