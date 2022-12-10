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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGELLIPSENODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGELLIPSENODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgEllipseNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes          maSvgStyleAttributes;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber               maCx;
            SvgNumber               maCy;
            SvgNumber               maRx;
            SvgNumber               maRy;
            basegfx::B2DHomMatrix*  mpaTransform;

        public:
            SvgEllipseNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgEllipseNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// Cx content, set if found in current context
            const SvgNumber& getCx() const { return maCx; }
            void setCx(const SvgNumber& rCx = SvgNumber()) { maCx = rCx; }

            /// Cy content, set if found in current context
            const SvgNumber& getCy() const { return maCy; }
            void setCy(const SvgNumber& rCy = SvgNumber()) { maCy = rCy; }

            /// Rx content, set if found in current context
            const SvgNumber& getRx() const { return maRx; }
            void setRx(const SvgNumber& rRx = SvgNumber()) { maRx = rRx; }

            /// Ry content, set if found in current context
            const SvgNumber& getRy() const { return maRy; }
            void setRy(const SvgNumber& rRy = SvgNumber()) { maRy = rRy; }

            /// transform content, set if found in current context
            const basegfx::B2DHomMatrix* getTransform() const { return mpaTransform; }
            void setTransform(const basegfx::B2DHomMatrix* pMatrix = 0) { if(mpaTransform) delete mpaTransform; mpaTransform = 0; if(pMatrix) mpaTransform = new basegfx::B2DHomMatrix(*pMatrix); }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGELLIPSENODE_HXX

// eof
