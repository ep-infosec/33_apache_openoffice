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


#ifndef _SFXFRAME_HXX
#define _SFXFRAME_HXX

#ifndef TF_NEWDESKTOP
#define TF_NEWDESKTOP 1
#endif

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace awt
            {
                class XWindow;
            }
            namespace frame
            {
                class XFrame;
                class XFrameLoader;
                class XController;
                class XDispatchProviderInterceptor;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}
#include <tools/link.hxx>
#include <tools/ref.hxx>
#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <tools/list.hxx>
#include <svl/poolitem.hxx>
#include <comphelper/namedvaluecollection.hxx>

class SvBorder;
class SfxWorkWindow;
class Window;
class SfxFrame;
class SfxFrame_Impl;
class SfxObjectShell;
class SfxObjectFactory;
class SfxViewFrame;
class SfxFrameDescriptor;
class SfxFrameSetDescriptor;
class SfxMedium;
class SfxFrameHistory_Impl;
class SfxDispatcher;
class SfxMedium;
class Rectangle;
class SfxRequest;
class SfxUnoControllerItem;
class SvCompatWeakHdl;
class SystemWindow;

typedef SfxFrame* SfxFramePtr;
class SfxFrameArr_Impl;

DECLARE_LIST( TargetList, String* )

#define SFXFRAME_HASTITLE     0x0001

//==========================================================================
// Ein SfxFrame ist eine Verwaltungsklasse f"ur Fenster und deren Inhalte.
// Eine SfxApplication pr"asentiert sich als Hierarchie von SfxFrames, wobei
// die konkreten Inhalte in den abgeleiteten Klassen festgelegt werden.
// Die Basisklasse SfxFrame implementiert 2 Aspekte der Frames: Benennung und
// Kontrolle der Lebensdauer.
// Innerhalb einer Frames-Hierarchie kontrolliert immer der ParentFrame die
// Lebensdauer seiner ChildFrames, auch wenn sie in der Regel gar nicht von
// ihm selbst erzeugt wurden. Durch Aufruf vonn DoClose() an irgendeinem
// Frame in der Hierarchie kann ein Teil des "Frameworks" entfernt werden,
// wobei sich Frames an ihren ParentFrames selbst abmelden.
//==========================================================================

class SfxFrameArr_Impl;
struct SfxFramePickEntry_Impl;
class SFX2_DLLPUBLIC SfxFrame
{
	friend class SfxFrameIterator;
    friend class SfxFrameWindow_Impl;

private:
	SfxFrame*			pParentFrame;
	SfxFrameArr_Impl*	pChildArr;
	SfxFrame_Impl*		pImp;
    Window*             pWindow;

protected:
    sal_Bool            Close();
    virtual             ~SfxFrame();

	SAL_DLLPRIVATE void RemoveChildFrame_Impl( SfxFrame* );

						SfxFrame( );    // not implemented
    SAL_DLLPRIVATE      SfxFrame( Window& i_rContainerWindow, bool bHidden );

public:
						TYPEINFO();

    static SfxFrame*    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        CreateBlankFrame();
    static SfxFrame*    Create( SfxObjectShell& rDoc, Window& rWindow, sal_uInt16 nViewId, bool bHidden );

	SvCompatWeakHdl*    GetHdl();
	Window&             GetWindow() const;
	void                CancelTransfers( sal_Bool bCancelLoadEnv = sal_True );
    sal_Bool            DoClose();
    sal_uInt16          GetChildFrameCount() const;
	SfxFrame*			GetChildFrame( sal_uInt16 nPos ) const;
	SfxFrame*	  		GetParentFrame() const
						{ return pParentFrame; }

    void                SetPresentationMode( sal_Bool bSet );
    SystemWindow*       GetSystemWindow() const;

    static SfxFrame*    GetFirst();
    static SfxFrame*    GetNext( SfxFrame& );

    static const SfxPoolItem*
                        OpenDocumentSynchron( SfxItemSet& aSet, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rTargetFrame );

    SfxBroadcaster&     GetBroadcaster() const;
	SfxObjectShell*		GetCurrentDocument() const;
	SfxViewFrame*		GetCurrentViewFrame() const;
	SfxFrame&			GetTopFrame() const;
    sal_Bool            IsParent( SfxFrame* ) const;

    sal_uInt32          GetFrameType() const;
	void				GetTargetList( TargetList& ) const;
	SAL_DLLPRIVATE SfxFrame* GetContainingDocFrame_Impl( SfxFrame* pSelf );
    sal_Bool            IsTop() const;
	void				UpdateDescriptor( SfxObjectShell *pDoc );
	void				Resize();
    sal_Bool            HasComponent() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                            GetComponent() const;
	void				ReleaseComponent();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                            GetFrameInterface() const;
	void 				Appear();
	void 				AppearWithUpdate();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                            GetController() const;

    sal_Bool            IsInPlace() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE sal_Bool DoClose_Impl();
    SAL_DLLPRIVATE void SetFrameInterface_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
	SAL_DLLPRIVATE void ReleasingComponent_Impl( sal_Bool bSet );
	SAL_DLLPRIVATE void GetViewData_Impl();
	SAL_DLLPRIVATE void SetFrameType_Impl( sal_uInt32 );
    SAL_DLLPRIVATE sal_uInt16 PrepareClose_Impl( sal_Bool bUI, sal_Bool bForBrowsing=sal_False );
    SAL_DLLPRIVATE sal_Bool DocIsModified_Impl();
	SAL_DLLPRIVATE void SetCurrentViewFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE sal_Bool IsClosing_Impl() const;
	SAL_DLLPRIVATE void SetIsClosing_Impl();

						// Methoden f"ur den Zugriff auf das aktuelle Set
	SAL_DLLPRIVATE void SetDescriptor( SfxFrameDescriptor* );
	SAL_DLLPRIVATE SfxFrameDescriptor* GetDescriptor() const;

	SAL_DLLPRIVATE void Lock_Impl( sal_Bool bLock );
	SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl() const;
    SAL_DLLPRIVATE sal_Bool IsAutoLoadLocked_Impl() const;

	SAL_DLLPRIVATE static void InsertTopFrame_Impl( SfxFrame* pFrame );
	SAL_DLLPRIVATE static void RemoveTopFrame_Impl( SfxFrame* pFrame );
	SAL_DLLPRIVATE void SetOwnsBindings_Impl( sal_Bool bSet );
    SAL_DLLPRIVATE sal_Bool OwnsBindings_Impl() const;
	SAL_DLLPRIVATE void InvalidateUnoControllers_Impl();
	SAL_DLLPRIVATE void RegisterUnoController_Impl( SfxUnoControllerItem* );
	SAL_DLLPRIVATE void ReleaseUnoController_Impl( SfxUnoControllerItem* );
	SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
	SAL_DLLPRIVATE void SetToolSpaceBorderPixel_Impl( const SvBorder& );
	SAL_DLLPRIVATE Rectangle GetTopOuterRectPixel_Impl() const;
	SAL_DLLPRIVATE void CreateWorkWindow_Impl();
    SAL_DLLPRIVATE void SetWorkWindow_Impl( SfxWorkWindow* pWorkwin );
	SAL_DLLPRIVATE void GrabFocusOnComponent_Impl();
    SAL_DLLPRIVATE void SetInPlace_Impl( sal_Bool );

    SAL_DLLPRIVATE void PrepareForDoc_Impl( SfxObjectShell& i_rDoc );
    SAL_DLLPRIVATE void LockResize_Impl( sal_Bool bLock );
    SAL_DLLPRIVATE void SetMenuBarOn_Impl( sal_Bool bOn );
    SAL_DLLPRIVATE sal_Bool IsMenuBarOn_Impl() const;
    SAL_DLLPRIVATE SystemWindow* GetTopWindow_Impl() const;
    SAL_DLLPRIVATE void PositionWindow_Impl( const Rectangle& rWinArea ) const;
    SAL_DLLPRIVATE bool IsMarkedHidden_Impl() const;
//#endif
private:
    SAL_DLLPRIVATE void Construct_Impl();
};

SV_DECL_COMPAT_WEAK( SfxFrame )

class SfxFrameIterator
{
	const SfxFrame*			pFrame;
    sal_Bool                bRecursive;

//#if 0 // _SOLAR__PRIVATE
	SfxFrame*				NextSibling_Impl( SfxFrame& rPrev );
//#endif

public:
							SfxFrameIterator( const SfxFrame& rFrame, sal_Bool bRecursive=sal_True );
	SfxFrame*				FirstFrame();
	SfxFrame*				NextFrame( SfxFrame& rPrev );
};

//--------------------------------------------------------------------

class SfxFrameItem_Impl;
class SFX2_DLLPUBLIC SfxFrameItem: public SfxPoolItem
{
	SfxFrame*               pFrame;
	SfxFrameWeak  		    wFrame;
	SAL_DLLPRIVATE void SetFramePtr_Impl( SfxFrame* /*pFrameP*/ ) { pFrame = wFrame; }

public:
							TYPEINFO();

							SfxFrameItem( sal_uInt16 nWhich, SfxViewFrame *p );
							SfxFrameItem( SfxFrame *p=0 );
							SfxFrameItem( sal_uInt16 nWhich, SfxFrame *p );

	virtual int 			operator==( const SfxPoolItem& ) const;
	virtual String			GetValueText() const;
	virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;

	virtual	sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool		PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    sal_Bool                FrameKilled() const { return &wFrame != pFrame; }

	SfxFrame*				GetFrame() const
							{ return wFrame; }
};

class SFX2_DLLPUBLIC SfxUsrAnyItem : public SfxPoolItem
{
    ::com::sun::star::uno::Any  aValue;
public:
                                TYPEINFO();
                                SfxUsrAnyItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Any& rAny );
    ::com::sun::star::uno::Any  GetValue() const
                                { return aValue; }
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

class SFX2_DLLPUBLIC SfxUnoFrameItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                                m_xFrame;

public:
                                TYPEINFO();
                                SfxUnoFrameItem();
                                SfxUnoFrameItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rFrame );
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&
                                GetFrame() const
                                { return m_xFrame; }
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

typedef SfxUsrAnyItem SfxUnoAnyItem;

#endif
