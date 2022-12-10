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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGCHARACTERNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGCHARACTERNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace primitive2d { class TextSimplePortionPrimitive2D; }}

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgTextPositions
        {
        private:
            SvgNumberVector         maX;
            SvgNumberVector         maY;
            SvgNumberVector         maDx;
            SvgNumberVector         maDy;
            SvgNumberVector         maRotate;
            SvgNumber               maTextLength;

            /// bitfield
            bool                    mbLengthAdjust : 1; // true = spacing, false = spacingAndGlyphs

        public:
            SvgTextPositions();

            void parseTextPositionAttributes(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);

            /// X content
            const SvgNumberVector& getX() const { return maX; }
            void setX(const SvgNumberVector& aX) { maX = aX; }

            /// Y content
            const SvgNumberVector& getY() const { return maY; }
            void setY(const SvgNumberVector& aY) { maY = aY; }

            /// Dx content
            const SvgNumberVector& getDx() const { return maDx; }
            void setDx(const SvgNumberVector& aDx) { maDx = aDx; }

            /// Dy content
            const SvgNumberVector& getDy() const { return maDy; }
            void setDy(const SvgNumberVector& aDy) { maDy = aDy; }

            /// Rotate content
            const SvgNumberVector& getRotate() const { return maRotate; }
            void setRotate(const SvgNumberVector& aRotate) { maRotate = aRotate; }

            /// TextLength content
            const SvgNumber& getTextLength() const { return maTextLength; }
            void setTextLength(const SvgNumber& rTextLength = SvgNumber()) { maTextLength = rTextLength; }

            /// LengthAdjust content
            bool getLengthAdjust() const { return mbLengthAdjust; }
            void setLengthAdjust(bool bNew) { mbLengthAdjust = bNew; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgTextPosition
        {
        private:
            SvgTextPosition*            mpParent;
            ::std::vector< double >     maX;
            ::std::vector< double >     maY;
            ::std::vector< double >     maRotate;
            double                      mfTextLength;

            // absolute, current, advancing position
            basegfx::B2DPoint           maPosition;

            // advancing rotation index
            sal_uInt32                  mnRotationIndex;

            /// bitfield
            bool                        mbLengthAdjust : 1; // true = spacing, false = spacingAndGlyphs
            bool                        mbAbsoluteX : 1;
            bool                        mbAbsoluteY : 1;

        public:
            SvgTextPosition(
                SvgTextPosition* pParent,
                const InfoProvider& rInfoProvider,
                const SvgTextPositions& rSvgTextPositions);

            // data read access
            const SvgTextPosition* getParent() const { return mpParent; }
            const ::std::vector< double >& getX() const { return maX; }
            const ::std::vector< double >& getY() const { return maY; }
            double getTextLength() const { return mfTextLength; }
            bool getLengthAdjust() const { return mbLengthAdjust; }
            bool getAbsoluteX() const { return mbAbsoluteX; }
            bool getAbsoluteY() const { return mbAbsoluteY; }

            // get/set absolute, current, advancing position
            const basegfx::B2DPoint& getPosition() const { return maPosition; }
            void setPosition(const basegfx::B2DPoint& rNew) { maPosition = rNew; }

            // rotation handling
            bool isRotated() const;
            double consumeRotation();
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgCharacterNode : public SvgNode
        {
        private:
            /// the string data
            rtl::OUString           maText;

            /// local helpers
            drawinglayer::primitive2d::TextSimplePortionPrimitive2D* createSimpleTextPrimitive(
                SvgTextPosition& rSvgTextPosition,
                const SvgStyleAttributes& rSvgStyleAttributes) const;
            void decomposeTextWithStyle(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
                SvgTextPosition& rSvgTextPosition,
                const SvgStyleAttributes& rSvgStyleAttributes) const;

        public:
            SvgCharacterNode(
                SvgDocument& rDocument,
                SvgNode* pParent,
                const rtl::OUString& rText);
            virtual ~SvgCharacterNode();

            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void decomposeText(drawinglayer::primitive2d::Primitive2DSequence& rTarget, SvgTextPosition& rSvgTextPosition) const;
            void whiteSpaceHandling();
            void addGap();
            void concatenate(const rtl::OUString& rText);

            /// Text content
            const rtl::OUString& getText() const { return maText; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGCHARACTERNODE_HXX

// eof
