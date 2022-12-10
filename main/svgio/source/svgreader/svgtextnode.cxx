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
#include "precompiled_svgio.hxx"

#include <svgio/svgreader/svgtextnode.hxx>
#include <svgio/svgreader/svgcharacternode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>
#include <svgio/svgreader/svgtrefnode.hxx>
#include <svgio/svgreader/svgtextpathnode.hxx>
#include <svgio/svgreader/svgtspannode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgTextNode::SvgTextNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenText, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpaTransform(0),
            maSvgTextPositions()
        {
        }

        SvgTextNode::~SvgTextNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgTextNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("text"));

            return checkForCssStyle(aClassStr, maSvgStyleAttributes);
        }

        void SvgTextNode::parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent, false);

            // read text position attributes
            maSvgTextPositions.parseTextPositionAttributes(rTokenName, aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    readLocalCssStyle(aContent);
                    break;
                }
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        void SvgTextNode::addTextPrimitives(
            const SvgNode& rCandidate, 
            drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
            drawinglayer::primitive2d::Primitive2DSequence& rSource) const
        {
            if(rSource.hasElements())
            {
                const SvgStyleAttributes* pAttributes = rCandidate.getSvgStyleAttributes();

                if(pAttributes)
                {
                    // add text with taking all Fill/Stroke attributes into account
                    pAttributes->add_text(rTarget, rSource);
                }
                else
                {
                    // should not happen, every subnode from SvgTextNode will at least
                    // return the attributes from SvgTextNode. Nonetheless, add text
                    drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(rTarget, rSource);
                }
            }
        }

        void SvgTextNode::DecomposeChild(const SvgNode& rCandidate, drawinglayer::primitive2d::Primitive2DSequence& rTarget, SvgTextPosition& rSvgTextPosition) const
        {
            switch(rCandidate.getType())
            {
                case SVGTokenCharacter:
                {
                    // direct SvgTextPathNode derivates, decompose them
                    const SvgCharacterNode& rSvgCharacterNode = static_cast< const SvgCharacterNode& >(rCandidate);
                    rSvgCharacterNode.decomposeText(rTarget, rSvgTextPosition);
                    break;
                }
                case SVGTokenTextPath:
                {
                    // direct TextPath decompose
                    const SvgTextPathNode& rSvgTextPathNode = static_cast< const SvgTextPathNode& >(rCandidate);
                    const SvgNodeVector& rChildren = rSvgTextPathNode.getChildren();
                    const sal_uInt32 nCount(rChildren.size());

                    if(nCount && rSvgTextPathNode.isValid())
                    {
                        // remember original TextStart to later detect hor/ver offsets
                        const basegfx::B2DPoint aTextStart(rSvgTextPosition.getPosition());
                        drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                        // decompose to regular TextPrimitives
                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            DecomposeChild(*rChildren[a], aNewTarget, rSvgTextPosition);
                        }

                        if(aNewTarget.hasElements())
                        {
                            const drawinglayer::primitive2d::Primitive2DSequence aPathContent(aNewTarget);
                            aNewTarget.realloc(0);

                            // dismantle TextPrimitives and map them on curve/path
                            rSvgTextPathNode.decomposePathNode(aPathContent, aNewTarget, aTextStart);
                        }

                        if(aNewTarget.hasElements())
                        {
                            addTextPrimitives(rCandidate, rTarget, aNewTarget);
                        }
                    }

                    break;
                }
                case SVGTokenTspan:
                {
                    // Tspan may have children, call recursively
                    const SvgTspanNode& rSvgTspanNode = static_cast< const SvgTspanNode& >(rCandidate);
                    const SvgNodeVector& rChildren = rSvgTspanNode.getChildren();
                    const sal_uInt32 nCount(rChildren.size());

                    if(nCount)
                    {
                        SvgTextPosition aSvgTextPosition(&rSvgTextPosition, rSvgTspanNode, rSvgTspanNode.getSvgTextPositions());
                        drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            DecomposeChild(*rChildren[a], aNewTarget, aSvgTextPosition);
                        }

                        rSvgTextPosition.setPosition(aSvgTextPosition.getPosition());
                        
                        if(aNewTarget.hasElements())
                        {
                            addTextPrimitives(rCandidate, rTarget, aNewTarget);
                        }
                    }
                    break;
                }
                case SVGTokenTref:
                {
                    const SvgTrefNode& rSvgTrefNode = static_cast< const SvgTrefNode& >(rCandidate);
                    const SvgTextNode* pRefText = rSvgTrefNode.getReferencedSvgTextNode();

                    if(pRefText)
                    {
                        const SvgNodeVector& rChildren = pRefText->getChildren();
                        const sal_uInt32 nCount(rChildren.size());
                        drawinglayer::primitive2d::Primitive2DSequence aNewTarget;

                        if(nCount)
                        {
                            for(sal_uInt32 a(0); a < nCount; a++)
                            {
                                const SvgNode& rChildCandidate = *rChildren[a];
                                const_cast< SvgNode& >(rChildCandidate).setAlternativeParent(this);
    
                                DecomposeChild(rChildCandidate, aNewTarget, rSvgTextPosition);
                                const_cast< SvgNode& >(rChildCandidate).setAlternativeParent(0);
                            }
                            
                            if(aNewTarget.hasElements())
                            {
                                addTextPrimitives(rCandidate, rTarget, aNewTarget);
                            }
                        }
                    }

                    break;
                }
                default:
                {
                    OSL_ENSURE(false, "Unexpected node in text token (!)");
                    break;
                }
            }
        }

        void SvgTextNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool /*bReferenced`*/) const
        {
            // text has a group of child nodes, allowed are SVGTokenCharacter, SVGTokenTspan,
            // SVGTokenTref and SVGTokenTextPath. These increase a given current text position
            const SvgStyleAttributes* pStyle = getSvgStyleAttributes();

            if(pStyle && !getChildren().empty())
            {
                const double fOpacity(pStyle->getOpacity().getNumber());

                if(fOpacity > 0.0)
                {
                    SvgTextPosition aSvgTextPosition(0, *this, getSvgTextPositions());
                    drawinglayer::primitive2d::Primitive2DSequence aNewTarget;
                    const SvgNodeVector& rChildren = getChildren();
                    const sal_uInt32 nCount(rChildren.size());

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        const SvgNode& rCandidate = *rChildren[a];
    
                        DecomposeChild(rCandidate, aNewTarget, aSvgTextPosition);
                    }
                
                    if(aNewTarget.hasElements())
                    {
                        drawinglayer::primitive2d::Primitive2DSequence aNewTarget2;

                        addTextPrimitives(*this, aNewTarget2, aNewTarget);
                        aNewTarget = aNewTarget2;
                    }

                    if(aNewTarget.hasElements())
                    {
                        pStyle->add_postProcess(rTarget, aNewTarget, getTransform());
                    }
                }
            }
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof
