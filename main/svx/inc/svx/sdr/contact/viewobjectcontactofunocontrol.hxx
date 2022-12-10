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



#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#define SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/uno/Reference.hxx>
/** === end UNO includes === **/
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>

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

    class ViewContactOfUnoControl;
    class ObjectContactOfPageView;
	//====================================================================
	//= ViewObjectContactOfUnoControl
	//====================================================================
    class ViewObjectContactOfUnoControl_Impl;
	class SVX_DLLPRIVATE ViewObjectContactOfUnoControl : public ViewObjectContactOfSdrObj
	{
    protected:
        ::rtl::Reference< ViewObjectContactOfUnoControl_Impl >    m_pImpl;

	public:
        ViewObjectContactOfUnoControl( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        /// determines whether an XControl already exists, and is currently visible
        bool    isControlVisible() const;

        /// returns the ->XControl instance belonging to the instance, creates it if necessary
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
                getControl();

        /** retrieves a temporary XControl instance, whose parent is the given device
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
            getTemporaryControlForWindow(
                const Window& _rWindow,
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer,
                const SdrUnoObj& _rUnoObject
            );

        /// ensures that the control belonging to this instances has a given visibility
        void    ensureControlVisibility( bool _bVisible ) const;

        /** sets the design/alive mode of the control
        */
        void    setControlDesignMode( bool _bDesignMode ) const;

        /** callback from impl class to react on changes of properties form the XControlModel
        */
        void propertyChange();

        /** React on changes of the object of this ViewContact
        */
		virtual void ActionChanged();

        /** to be called when any aspect of the control which requires view updates changed
        */
        struct ImplAccess { friend class ViewObjectContactOfUnoControl_Impl; friend class ViewObjectContactOfUnoControl; private: ImplAccess() { } };
        void onControlChangedOrModified( ImplAccess ) { impl_onControlChangedOrModified(); }

    protected:
        ~ViewObjectContactOfUnoControl();

		// support for Primitive2D
		virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

        // visibility check
	    virtual bool isPrimitiveVisible( const DisplayInfo& _rDisplayInfo ) const;
        /// to be called when any aspect of the control which requires view updates changed
        void impl_onControlChangedOrModified();

    private:
        ViewObjectContactOfUnoControl();                                                    // never implemented
        ViewObjectContactOfUnoControl( const ViewObjectContactOfUnoControl& );              // never implemented
        ViewObjectContactOfUnoControl& operator=( const ViewObjectContactOfUnoControl& );   // never implemented
	};

    //====================================================================
	//= UnoControlPrintOrPreviewContact
	//====================================================================
    class SVX_DLLPRIVATE UnoControlPrintOrPreviewContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlPrintOrPreviewContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        ~UnoControlPrintOrPreviewContact();

    private:
        UnoControlPrintOrPreviewContact();                                                 // never implemented
        UnoControlPrintOrPreviewContact( const UnoControlPrintOrPreviewContact& );            // never implemented
        UnoControlPrintOrPreviewContact& operator=( const UnoControlPrintOrPreviewContact& ); // never implemented

        virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo ) const;
    };

//........................................................................
} } // namespace sdr::contact
//........................................................................

#endif // SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

