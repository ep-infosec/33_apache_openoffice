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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include "basecontainernode.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "delayevent.hxx"

#include <boost/mem_fn.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

BaseContainerNode::BaseContainerNode(
    const uno::Reference< animations::XAnimationNode >&     xNode, 
    const BaseContainerNodeSharedPtr&                       rParent,
    const NodeContext&                                      rContext )
    : BaseNode( xNode, rParent, rContext ),
      maChildren(),
      mnFinishedChildren(0),
      mbDurationIndefinite( isIndefiniteTiming( xNode->getEnd() ) &&
                            isIndefiniteTiming( xNode->getDuration() ) )
{
}

void BaseContainerNode::dispose()
{
    forEachChildNode( boost::mem_fn(&Disposable::dispose) );
    maChildren.clear();
    BaseNode::dispose();
}

bool BaseContainerNode::init_st()
{
    mnFinishedChildren = 0;
    // initialize all children
    return (std::count_if(
                maChildren.begin(), maChildren.end(),
                boost::mem_fn(&AnimationNode::init) ) ==
            static_cast<VectorOfNodes::difference_type>(maChildren.size()));
}

void BaseContainerNode::deactivate_st( NodeState eDestState )
{
    if (eDestState == FROZEN) {
        // deactivate all children that are not FROZEN or ENDED:
        forEachChildNode( boost::mem_fn(&AnimationNode::deactivate),
                          ~(FROZEN | ENDED) );
    }
    else {
        // end all children that are not ENDED:
        forEachChildNode( boost::mem_fn(&AnimationNode::end), ~ENDED );
    }
}

bool BaseContainerNode::hasPendingAnimation() const
{
    // does any of our children returns "true" on
    // AnimationNode::hasPendingAnimation()? 
    // If yes, we, too, return true
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    return (std::find_if(
                maChildren.begin(), iEnd,
                boost::mem_fn(&AnimationNode::hasPendingAnimation) ) != iEnd);
}

void BaseContainerNode::appendChildNode( AnimationNodeSharedPtr const& pNode )
{
    if (! checkValidNode())
        return;
    
    // register derived classes as end listeners at all children.
    // this is necessary to control the children animation 
    // sequence, and to determine our own end event
    if (pNode->registerDeactivatingListener( getSelf() )) {
        maChildren.push_back( pNode );
    }
}

bool BaseContainerNode::isChildNode( AnimationNodeSharedPtr const& pNode ) const
{
    // find given notifier in child vector
    VectorOfNodes::const_iterator const iBegin( maChildren.begin() );
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    VectorOfNodes::const_iterator const iFind(
        std::find( iBegin, iEnd, pNode ) );
    return (iFind != iEnd);
}

bool BaseContainerNode::notifyDeactivatedChild(
    AnimationNodeSharedPtr const& pChildNode )
{
    OSL_ASSERT( pChildNode->getState() == FROZEN ||
                pChildNode->getState() == ENDED );
    // early exit on invalid nodes
    OSL_ASSERT( getState() != INVALID );
    if( getState() == INVALID )
        return false;

    if (! isChildNode(pChildNode)) {
        OSL_ENSURE( false, "unknown notifier!" );
        return false;
    }
    
    std::size_t const nSize = maChildren.size();
    OSL_ASSERT( mnFinishedChildren < nSize );
    ++mnFinishedChildren;
    bool const bFinished = (mnFinishedChildren >= nSize);
    
    // all children finished, and we've got indefinite duration?
    // think of ParallelTimeContainer::notifyDeactivating()
    // if duration given, we will be deactivated by some end event
    // @see fillCommonParameters()
    if (bFinished && isDurationIndefinite()) {
        deactivate();
    }
    
    return bFinished;
}

#if defined(VERBOSE) && defined(DBG_UTIL)
void BaseContainerNode::showState() const
{
    for( std::size_t i=0; i<maChildren.size(); ++i )
    {
        BaseNodeSharedPtr pNode =
            boost::dynamic_pointer_cast<BaseNode>(maChildren[i]);
        VERBOSE_TRACE(
            "Node connection: n0x%X -> n0x%X", 
            (const char*)this+debugGetCurrentOffset(), 
            (const char*)pNode.get()+debugGetCurrentOffset() );
        pNode->showState();
    }
    
    BaseNode::showState();
}
#endif

} // namespace internal
} // namespace slideshow

