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



#ifndef DBAUI_ICONTROLLER_HXX
#define DBAUI_ICONTROLLER_HXX

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef DBAUI_IREFERENCE_HXX
#include "IReference.hxx"
#endif
#include "dbaccessdllapi.h"

namespace com { namespace sun { namespace star {
    namespace util {
	    struct URL;
    }
    namespace frame {
	    class XController;
    }
} } }

class NotifyEvent;

namespace dbaui
{
	// interface for controller depended calls like commands
	class DBACCESS_DLLPUBLIC IController : public IReference
	{
	public:
		/** executes the given command without checking if it is allowed
			@param	_rCommand	the URL of the command
		*/
		virtual void executeUnChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

		/** executes the given command only when it is allowed
			@param	_rCommand	
				the URL of the command
		*/
		virtual void executeChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

		/** executes the given command without checking if it is allowed
			@param	_nCommandId
				the id of the command URL 
		*/
		virtual void executeUnChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;

		/** executes the given command only when it is allowed
			@param	_nCommandId
				the id of the command URL 
		*/
		virtual void executeChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) = 0;


		/** checks if the given Command is enabled
			@param	_nCommandId
				the id of the command URL 
			
			@return	
				<TRUE/> if the command is allowed, otherwise <FALSE/>.
		*/
		virtual sal_Bool isCommandEnabled(sal_uInt16 _nCommandId) const = 0;

		/** checks if the given Command is enabled
			@param	_rCompleteCommandURL
				the URL of the command
			
			@return	
				<TRUE/> if the command is allowed, otherwise <FALSE/>.
		*/
        virtual sal_Bool isCommandEnabled( const ::rtl::OUString& _rCompleteCommandURL ) const = 0;

        /** registers a command URL, giving it a unique name

            If you call this with a command URL which is supported by the controller, then
            you will simply get the controller's internal numeric shortcut to this command.

            If you call this with a command URL which is not supported by the controller, then
            you will get a new ID, which is unique during the lifetime of the controller.

            If the command URL is invalid, or the controller cannot register new commands anymore,
            then 0 is returned.
        */
        virtual sal_uInt16
                        registerCommandURL( const ::rtl::OUString& _rCompleteCommandURL ) = 0;

		/** notifyHiContrastChanged will be called when the hicontrast mode changed.
			@param	_bHiContrast
				<TRUE/> when in hicontrast mode.
		*/
		virtual void notifyHiContrastChanged() = 0;

		/** checks if the selected data source is read only
			@return 
				<TRUE/> if read only, otherwise <FALSE/>
		*/
		virtual sal_Bool isDataSourceReadOnly() const = 0;

        /** provides access to the model of the controller

            This must be the same model as returned by XController::getModel, and might be <NULL/> when
            the controller does not have an own model.
        */
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                getXController(void) throw( ::com::sun::star::uno::RuntimeException ) = 0;

        /** allows interception of user input, aka mouse clicks and key events
        */
        virtual bool interceptUserInput( const NotifyEvent& _rEvent ) = 0;
	};
}
#endif // DBAUI_ICONTROLLER_HXX
