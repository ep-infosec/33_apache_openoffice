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



#ifndef INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX
#define INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX

#include <boost/shared_ptr.hpp>

/* Definition of IntrinsicAnimationEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling intrinsic animation display modes.

        	Classes implementing this interface can be added to an
        	EventMultiplexer object, and are called from there to
        	handle intrinsic animation events.
         */
        class IntrinsicAnimationEventHandler
        {
        public:
            virtual ~IntrinsicAnimationEventHandler() {}

            virtual bool enableAnimations() = 0;
            virtual bool disableAnimations() = 0;
        };

        typedef ::boost::shared_ptr< IntrinsicAnimationEventHandler > IntrinsicAnimationEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_INTRINSICANIMATIONEVENTHANDLER_HXX */
