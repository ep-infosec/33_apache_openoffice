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



#ifndef SVX_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX
#define SVX_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Reference.hxx>
/** === end UNO includes === **/
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svxdllapi.h>

#include <memory>

class OutputDevice;
class Window;
class SdrUnoObj;
namespace com { namespace sun { namespace star {
    namespace awt {
        class XControl;
        class XControlContainer;
    }
} } }

//........................................................................
namespace sdr { namespace contact {
//........................................................................

	//====================================================================
	//= ViewContactOfUnoControl
	//====================================================================
    class ViewContactOfUnoControl_Impl;
    class SVX_DLLPRIVATE ViewContactOfUnoControl : public ViewContactOfSdrObj
	{
    private:
        ::std::auto_ptr< ViewContactOfUnoControl_Impl >   m_pImpl;

    public:
		// access to SdrObject
		SdrUnoObj& GetSdrUnoObj() const
		{
			return ((SdrUnoObj&)GetSdrObject());
		}

		ViewContactOfUnoControl( SdrUnoObj& _rUnoObject );
        virtual ~ViewContactOfUnoControl();

        /** access control to selected members
        */
		struct SdrUnoObjAccessControl { friend class ::SdrUnoObj; private: SdrUnoObjAccessControl() { } };

        /** retrieves a temporary XControl instance, whose parent is the given window
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
            getTemporaryControlForWindow( const Window& _rWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer ) const;

    protected:
		virtual ViewObjectContact& CreateObjectSpecificViewObjectContact( ObjectContact& _rObjectContact );

    private:
        ViewContactOfUnoControl();                                            // never implemented
        ViewContactOfUnoControl( const ViewContactOfUnoControl& );              // never implemented
        ViewContactOfUnoControl& operator=( const ViewContactOfUnoControl& );   // never implemented

	protected:
		// This method is responsible for creating the graphical visualisation data
        // ONLY based on model data
		virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
	};

//........................................................................
} } // namespace sdr::contact
//........................................................................

#endif // SVX_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX
// eof
