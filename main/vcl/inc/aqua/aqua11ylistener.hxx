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


 
#ifndef _AQUA11YLISTENER_HXX_
#define _AQUA11YLISTENER_HXX_

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/implbase1.hxx>

#include "aqua11yfocustracker.hxx"
#include "aquavcltypes.h"
#include <set>
#include <com/sun/star/awt/Rectangle.hpp>

// -------------------------
// - AquaA11yEventListener -
// -------------------------

class AquaA11yEventListener :  
    public ::cppu::WeakImplHelper1< ::com::sun::star::accessibility::XAccessibleEventListener >
{

public:
    AquaA11yEventListener(id wrapperObject, sal_Int16 role);
    virtual ~AquaA11yEventListener();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) 
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) 
        throw( ::com::sun::star::uno::RuntimeException );
        
private:
    const id m_wrapperObject;
    const sal_Int16 m_role;
    ::com::sun::star::awt::Rectangle m_oldBounds;
};

#endif // _AQUA11YLISTENER_HXX_