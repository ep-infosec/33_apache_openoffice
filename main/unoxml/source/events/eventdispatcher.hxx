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



#ifndef EVENT_EVENT_DISPATCHER_HXX
#define EVENT_EVENT_DISPATCHER_HXX

#include <map>
#include <vector>

#include <libxml/tree.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/events/EventType.hpp>
#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM {

class CDocument;

namespace events {

typedef std::multimap< xmlNodePtr, Reference< com::sun::star::xml::dom::events::XEventListener> > ListenerMap;
typedef std::map< ::rtl::OUString, ListenerMap*> TypeListenerMap;
typedef std::vector<ListenerMap::value_type> ListenerPairVector;

class CEventDispatcher
{
private:
    TypeListenerMap m_CaptureListeners;
    TypeListenerMap m_TargetListeners;

public:
    void addListener(
        xmlNodePtr pNode,
        ::rtl::OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    void removeListener(
        xmlNodePtr pNode,
        ::rtl::OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    static void callListeners(
        TypeListenerMap const& rTMap,
        xmlNodePtr const pNode,
        ::rtl::OUString aType,
        const Reference< XEvent >& xEvent);

    bool dispatchEvent(
        DOM::CDocument & rDocument,
        ::osl::Mutex & rMutex,
        xmlNodePtr const pNode,
        Reference<XNode> const& xNode,
        Reference< XEvent > const& xEvent) const;
};

}}

#endif

