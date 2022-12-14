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



#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>
#include <uielement/commandinfo.hxx>

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

//shizhoubo
#include <com/sun/star/frame/XToolbarController.hpp>
//end
//_________________________________________________________________________________________________________________
//	other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/accel.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XLayoutManager;
            }
        }
    }
}

namespace framework
{

class ToolBar;
class ToolBarManager : public ::com::sun::star::frame::XFrameActionListener         ,
                       public ::com::sun::star::frame::XStatusListener              ,
                       public ::com::sun::star::lang::XComponent                    ,
                       public ::com::sun::star::lang::XTypeProvider                 ,
                       public ::com::sun::star::ui::XUIConfigurationListener,
					   public ThreadHelpBase		                                ,
					   public ::cppu::OWeakObject
{
    public:
        ToolBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                        const rtl::OUString& rResourceName,
                        ToolBar* pToolBar );
        virtual ~ToolBarManager();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        ToolBox* GetToolBar() const;

        // XFrameActionListener
		virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
		virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
		virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

        void CheckAndUpdateImages();
        virtual void RefreshImages();
        void FillToolbar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rToolBarData );
        void notifyRegisteredControllers( const rtl::OUString& aUIElementName, const rtl::OUString& aCommand );
        void Destroy();

        enum ExecuteCommand
        {
            EXEC_CMD_CLOSETOOLBAR,
            EXEC_CMD_DOCKTOOLBAR,
            EXEC_CMD_DOCKALLTOOLBARS,
            EXEC_CMD_NONE,
            EXEC_CMD_COUNT
        };

        struct ExecuteInfo
        {
            rtl::OUString   aToolbarResName;
            ExecuteCommand  nCmd;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xWindow;
        };
        struct ControllerParams
        {
            sal_Int16 nWidth;
        };
        typedef std::vector< ControllerParams > ControllerParamsVector;

    protected:
		//added for 33668 by shizhoubo : 2008:04
		DECL_LINK( Command, CommandEvent * );
		PopupMenu * GetToolBarCustomMenu(ToolBox* pToolBar);
		//end
        DECL_LINK( Click, ToolBox * );
        DECL_LINK( DropdownClick, ToolBox * );
        DECL_LINK( DoubleClick, ToolBox * );
        DECL_LINK( Select, ToolBox * );
		DECL_LINK( Highlight, ToolBox * );
		DECL_LINK( Activate, ToolBox * );
		DECL_LINK( Deactivate, ToolBox * );
        DECL_LINK( StateChanged, StateChangedType* );
        DECL_LINK( DataChanged, DataChangedEvent* );

        DECL_LINK( MenuButton, ToolBox * );
        DECL_LINK( MenuSelect, Menu * );
        DECL_LINK( MenuDeactivate, Menu * );
        DECL_LINK( AsyncUpdateControllersHdl, Timer * );
        DECL_STATIC_LINK( ToolBarManager, ExecuteHdl_Impl, ExecuteInfo* );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;

        void RemoveControllers();
        rtl::OUString RetrieveLabelFromCommand( const rtl::OUString& aCmdURL );
        sal_Int32 RetrievePropertiesFromCommand( const rtl::OUString& aCmdURL );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropsForCommand( const ::rtl::OUString& rCmdURL );
        void CreateControllers();
        void UpdateControllers();
		//for update controller via Support Visiable by shizhoubo
		void UpdateController( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XToolbarController > xController);
		//end
        void AddFrameActionListener();
        void AddImageOrientationListener();
        void UpdateImageOrientation();
        void ImplClearPopupMenu( ToolBox *pToolBar );
        void RequestImages();
        sal_uInt16 ConvertStyleToToolboxItemBits( sal_Int32 nStyle );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetModelFromFrame() const;
        sal_Bool IsPluginMode() const;
		Image QueryAddonsImage( const ::rtl::OUString& aCommandURL, bool bBigImages, bool bHiContrast );
        long HandleClick(void ( SAL_CALL ::com::sun::star::frame::XToolbarController::*_pClick )(  ));
        void setToolBarImage(const Image& _aImage,const CommandToInfoMap::const_iterator& _pIter);
        void impl_elementChanged(bool _bRemove,const ::com::sun::star::ui::ConfigurationEvent& Event );

		static bool impl_RetrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg, const rtl::OUString& rCommand, rtl::OUString& rShortCut );
		bool RetrieveShortcut( const rtl::OUString& rCommandURL, rtl::OUString& rShortCut );

    protected:
        typedef ::std::hash_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > ToolBarControllerMap;
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XSubToolbarController > > SubToolBarControllerVector;
        typedef BaseHash< SubToolBarControllerVector >                                                              SubToolBarToSubToolBarControllerMap;

        typedef ::std::hash_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > > MenuDescriptionMap;
        sal_Bool                                                                               m_bDisposed : 1,
                                                                                               m_bIsHiContrast : 1,
                                                                                               m_bSmallSymbols : 1,
                                                                                               m_bModuleIdentified : 1,
                                                                                               m_bAddedToTaskPaneList : 1,
                                                                                               m_bVerticalTextEnabled : 1,
                                                                                               m_bFrameActionRegistered : 1,
                                                                                               m_bUpdateControllers : 1;
        sal_Bool                                                                               m_bImageOrientationRegistered : 1,
                                                                                               m_bImageMirrored : 1,
                                                                                               m_bCanBeCustomized : 1;
        long                                                                                   m_lImageRotation;
        ToolBar*                                                                               m_pToolBar;
        rtl::OUString                                                                          m_aModuleIdentifier;
        rtl::OUString                                                                          m_aResourceName;
        com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >              m_xURLTransformer;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                        m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >               m_xUICommandLabels;
        ToolBarControllerMap                                                                   m_aControllerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                             m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >         m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerFactory > m_xToolbarControllerFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xModuleImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xDocImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >                 m_xImageOrientationListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >                m_xUICfgMgr;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >                m_xDocUICfgMgr;

        CommandToInfoMap                                                                       m_aCommandMap;
        SubToolBarToSubToolBarControllerMap                                                    m_aSubToolBarControllerMap;
        Timer				                                                                   m_aAsyncUpdateControllersTimer;
        sal_Int16                                                                              m_nSymbolsStyle;
        MenuDescriptionMap m_aMenuMap;
        sal_Bool																			   m_bAcceleratorCfg;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
