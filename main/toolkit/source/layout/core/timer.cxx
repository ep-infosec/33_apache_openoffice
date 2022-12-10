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



#include "timer.hxx"

#include <vector>
#include <list>
#include <vcl/timer.hxx>
#include <com/sun/star/awt/XLayoutContainer.hpp>

namespace layoutimpl
{
using namespace ::com::sun::star;

class AllocateTimer : public Timer
{
    typedef std::list< uno::Reference< awt::XLayoutContainer > > ContainerList;
    ContainerList mxContainers;
    uno::Reference< awt::XLayoutContainer > mxLastAdded;

public:
    AllocateTimer()
    {
        // timer set to 0 -- just process it as soon as it gets idle
        SetTimeout( 0 );
    }

    static inline bool isParentOf( uno::Reference< awt::XLayoutContainer > xParent,
                                   uno::Reference< awt::XLayoutContainer > xWidget )
    {
        while ( xWidget.is() )
        {
            if ( xWidget == xParent )
                return true;
            xWidget = uno::Reference< awt::XLayoutContainer >( xWidget->getParent(), uno::UNO_QUERY );
        }
        return false;
    }

    static inline void eraseChildren( ContainerList::iterator &it, ContainerList &list )
    {
        ContainerList::iterator jt = list.begin();
        while ( jt != list.end() )
        {
            if ( it != jt && isParentOf( *it, *jt ) )
                jt = list.erase( jt );
            else
                jt++;
        }
    }

    static inline bool isContainerDamaged( uno::Reference< awt::XLayoutContainer > xContainer )
    {
        uno::Reference< awt::XLayoutConstrains > xConstrains( xContainer, uno::UNO_QUERY );
        awt::Size lastReq( xContainer->getRequestedSize() );
        awt::Size curReq( xConstrains->getMinimumSize() );
        return lastReq.Width != curReq.Width || lastReq.Height != curReq.Height;
    }

    void add( const uno::Reference< awt::XLayoutContainer > &xContainer )
    {
        // small optimization
        if ( mxLastAdded == xContainer )
            return;
        mxLastAdded = xContainer;

        mxContainers.push_back( xContainer );
    }

    virtual void Timeout()
    {
        mxLastAdded = uno::Reference< awt::XLayoutContainer >();

        // 1. remove duplications and children
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); it++ )
            eraseChildren( it, mxContainers );

        // 2. check damage extent
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); it++ )
        {
            uno::Reference< awt::XLayoutContainer > xContainer = *it;
            while ( xContainer->getParent().is() && isContainerDamaged( xContainer ) )
            {
                xContainer = uno::Reference< awt::XLayoutContainer >(
                    xContainer->getParent(), uno::UNO_QUERY );
            }

            if ( *it != xContainer )
            {
                // 2.2 replace it with parent
                *it = xContainer;

                // 2.3 remove children of new parent
                eraseChildren( it, mxContainers );
            }
        }

        // 3. force re-calculations
        for ( ContainerList::iterator it = mxContainers.begin();
             it != mxContainers.end(); it++ )
            (*it)->allocateArea( (*it)->getAllocatedArea() );
    }
};

static void AddResizeTimeout( const uno::Reference< awt::XLayoutContainer > &xCont )
{
    static AllocateTimer timer;
    timer.add( xCont );
    timer.Start();
}

LayoutUnit::LayoutUnit() : LayoutUnit_Base()
{
}

void SAL_CALL LayoutUnit::queueResize( const uno::Reference< awt::XLayoutContainer > &xContainer )
    throw( uno::RuntimeException )
{
    AddResizeTimeout( xContainer );
}

}
