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



#ifndef INCLUDED_SLIDESHOW_ANIMATIONTRANSFORMNODE_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONTRANSFORMNODE_HXX

#include "animationbasenode.hxx"
#include "com/sun/star/animations/XAnimateTransform.hpp"

namespace slideshow {
namespace internal {

class AnimationTransformNode : public AnimationBaseNode
{
public:
    AnimationTransformNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode, 
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext )
        : AnimationBaseNode( xNode, pParent, rContext ),
          mxTransformNode( xNode, ::com::sun::star::uno::UNO_QUERY_THROW ) {}
    
#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "AnimationTransformNode"; }
#endif
    
protected:
    virtual void dispose();
    
private:
    virtual AnimationActivitySharedPtr createActivity() const;
    
    ::com::sun::star::uno::Reference< 
        ::com::sun::star::animations::XAnimateTransform >   mxTransformNode;
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_ANIMATIONTRANSFORMNODE_HXX */
