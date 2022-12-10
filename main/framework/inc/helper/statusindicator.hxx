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



#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_

//_______________________________________________
// include files of own module

#include <helper/statusindicatorfactory.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>

//_______________________________________________
// include UNO interfaces
#include <com/sun/star/task/XStatusIndicator.hpp>

//_______________________________________________
// include all others
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// definitions

//_______________________________________________
/**
	@short          implement a status indicator object
    
	@descr			With this indicator you can show a message and a progress ...
					but you share the output device with other indicator objects,
                    if this instances was created by the same factory.
					Then the last created object has full access to device.
                    All others change her internal data structure only.
                    
                    All objects of this StatusIndicator class calls a c++ interface
                    on the StatusIndicatorFactory (where they was created).
                    The factory holds all data structures and paints the progress.

	@devstatus		ready to use
    @threadsafe     yes
*/
class StatusIndicator : public  css::lang::XTypeProvider
                      , public  css::task::XStatusIndicator
                      , private ThreadHelpBase                  // Order of baseclasses is necessary for right initializaton!
                      , public  ::cppu::OWeakObject             // => XInterface
{
    //-------------------------------------------
    // member
    private:
    
        /** @short  weak reference to our factory
            @descr  All our interface calls will be forwarded
                    to a suitable c++ interface on this factory.
                    But we dont hold our factory alive. They
                    correspond with e.g. with a Frame service and
                    will be owned by him. If the frame will be closed
                    he close our factory too ...
         */
        css::uno::WeakReference< css::task::XStatusIndicatorFactory > m_xFactory;
    
    //-------------------------------------------
    // c++ interface
    public:

        //----------------------------------------
        /** @short  initialize new instance of this class.
        
            @param  pFactory
                    pointer to our factory
         */
        StatusIndicator(StatusIndicatorFactory* pFactory);
        
        //----------------------------------------
        /** @short  does nothing real ....
         */
        virtual ~StatusIndicator();

    //-------------------------------------------
    // uno interface
    public:
    
        //---------------------------------------
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //---------------------------------------
        // XStatusIndicator
        virtual void SAL_CALL start(const ::rtl::OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);
            
        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);
            
        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);
            
        virtual void SAL_CALL setText(const ::rtl::OUString& sText)
            throw(css::uno::RuntimeException);
            
        virtual void SAL_CALL setValue(sal_Int32 nValue)
            throw(css::uno::RuntimeException);

}; // class StatusIndicator

} // namespace framework

#endif // __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
