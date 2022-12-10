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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <svgio/svgreader/svgpathnode.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgTextPathNode : public SvgNode
        {
        private:
            /// use styles
            SvgStyleAttributes      maSvgStyleAttributes;

            /// link to path content. If maXLink
            /// is set, the node can be fetched on demand
            rtl::OUString           maXLink;

            /// variable scan values, dependent of given XAttributeList
            SvgNumber               maStartOffset;

            /// bitfield
            bool                    mbMethod : 1; // true = align, false = stretch
            bool                    mbSpacing : 1; // true = auto, false = exact

        public:
            SvgTextPathNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgTextPathNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            void decomposePathNode(
                const drawinglayer::primitive2d::Primitive2DSequence& rPathContent, 
                drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
                const basegfx::B2DPoint& rTextStart) const;
            bool isValid() const;

            /// StartOffset content
            const SvgNumber& getStartOffset() const { return maStartOffset; }
            void setStartOffset(const SvgNumber& rStartOffset = SvgNumber()) { maStartOffset = rStartOffset; }

            /// Method content
            bool getMethod() const { return mbMethod; }
            void setMethod(bool bNew) { mbMethod = bNew; }

            /// Spacing content
            bool getSpacing() const { return mbSpacing; }
            void setSpacing(bool bNew) { mbSpacing = bNew; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGTEXTPATHNODE_HXX

// eof
