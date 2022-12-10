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

#ifndef __ACCPARAGRAPHEVENTLISTENER_HXX
#define __ACCPARAGRAPHEVENTLISTENER_HXX

#include <stdio.h>
#include "AccContainerEventListener.hxx"
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

using namespace ::com::sun::star::uno;

/**
 * AccParagraphEventListener is inherited from AccContainerEventListener. It handles the evnets
 * generated by container controls. The accessible roles are: PARAGRAPH and HEADING.
 * It defines the procedure of specific event handling related with text containsers and provides 
 * the detailed support for some related methods.  
 */
class AccParagraphEventListener: public AccContainerEventListener
{
public:
    AccParagraphEventListener(com::sun::star::accessibility::XAccessible* pAcc, AccObjectManagerAgent* Agent);
    virtual ~AccParagraphEventListener();

    //AccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    //for caret changed event
    virtual void SAL_CALL handleCaretChangedEvent(Any oldValue, Any newValue);

    //for visible data changed event
    virtual void SAL_CALL handleVisibleDataChangedEvent();

    //for bound rect changed event
    virtual void SAL_CALL handleBoundrectChangedEvent();

    virtual void SAL_CALL setComponentState(short state, bool enable);

    //add TEXT_SELECTION_CHANGED event
    void handleTextSelectionChangedEvent();
};

#endif
