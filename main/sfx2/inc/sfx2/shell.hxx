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


#ifndef _SFX_SHELL_HXX
#define _SFX_SHELL_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <tools/debug.hxx>
#include <tools/rtti.hxx>
#include <svl/brdcst.hxx>

#include <tools/ownlist.hxx>
#include <tools/unqid.hxx>
#include <tools/string.hxx>

#include <sfx2/sfxuno.hxx>

class ResMgr;
class Window;
class ToolBox;
class SfxItemPool;
class SfxPoolItem;
class SfxRequest;
class SfxItemSet;
struct SfxFormalArgument;
class StatusBar;
class SfxInterface;
class SfxViewShell;
class SfxObjectShell;
class SfxSlotPool;
class SvGlobalName;

class SfxShellObject;
class SfxShell;
struct SfxShell_Impl;
struct SfxTypeLibImpl;
class SfxShellObject;
class SfxShellSubObject;
class SfxDispatcher;
class SfxViewFrame;
class SfxSlot;
class SfxRepeatTarget;
class SbxVariable;
class SbxBase;
class SfxBindings;

namespace svl
{
    class IUndoManager;
}

//====================================================================

enum SfxInterfaceId

/*	[Beschreibung]

	Id f"ur die <SfxInterface>s, damit wird "uber ein Array an der
	<SfxApplication> ein quasi-statischer Zugriff auf die Interfaces
	erlaubt.
*/

{
	SFX_INTERFACE_NONE,
	SFX_INTERFACE_SFXAPP,
	SFX_INTERFACE_SFXDOCSH,
	SFX_INTERFACE_SFXIPFRM,
	SFX_INTERFACE_SFXVIEWSH,
	SFX_INTERFACE_SFXVIEWFRM,
	SFX_INTERFACE_SFXPLUGINFRM,
	SFX_INTERFACE_SFXPLUGINOBJ,
	SFX_INTERFACE_SFXPLUGINVIEWSH,
	SFX_INTERFACE_SFXFRAMESETOBJ,
	SFX_INTERFACE_SFXFRAMESETVIEWSH,
	SFX_INTERFACE_SFXINTERNALFRM,
	SFX_INTERFACE_SFXCOMPONENTDOCSH,
	SFX_INTERFACE_SFXGENERICOBJ,
	SFX_INTERFACE_SFXGENERICVIEWSH,
	SFX_INTERFACE_SFXEXPLOBJ,
	SFX_INTERFACE_SFXEXPLVIEWSH,
	SFX_INTERFACE_SFXPLUGINVIEWSHDYNAMIC,
	SFX_INTERFACE_SFXEXTERNALVIEWFRM,
	SFX_INTERFACE_SFXMODULE,
	SFX_INTERFACE_SFXFRAMESETVIEW,
	SFX_INTERFACE_SFXFRAMESETSOURCEVIEW,
	SFX_INTERFACE_SFXHELP_DOCSH,
	SFX_INTERFACE_SFXHELP_VIEWSH,
	SFX_INTERFACE_SFXTASK,
	SFX_INTERFACE_OFA_START			=  100,
	SFX_INTERFACE_OFA_END			=  100,
	SFX_INTERFACE_SC_START			=  150,
	SFX_INTERFACE_SC_END			=  199,
	SFX_INTERFACE_SD_START			=  200,
	SFX_INTERFACE_SD_END			=  249,
	SFX_INTERFACE_SW_START			=  250,
	SFX_INTERFACE_SW_END			=  299,
	SFX_INTERFACE_SIM_START   		=  300,
	SFX_INTERFACE_SIM_END		    =  319,
	SFX_INTERFACE_SCH_START		    =  320,
	SFX_INTERFACE_SCH_END   	    =  339,
	SFX_INTERFACE_SMA_START   		=  340,
	SFX_INTERFACE_SMA_END   		=  359,
	SFX_INTERFACE_SBA_START   		=  360,
	SFX_INTERFACE_SBA_END   		=  399,
	SFX_INTERFACE_IDE_START   		=  400,
	SFX_INTERFACE_IDE_END   		=  409,
	//-falls die noch einer braucht
	SFX_INTERFACE_APP				=  SFX_INTERFACE_SW_START,
	SFX_INTERFACE_LIB				=  450
};

//TODO/CLEANUP: replace by UNO constant
#define SVVERB_SHOW -1

//====================================================================

typedef void (*SfxExecFunc)(SfxShell *, SfxRequest &rReq);
typedef void (*SfxStateFunc)(SfxShell *, SfxItemSet &rSet);

class SFX2_DLLPUBLIC SfxShell: public SfxBroadcaster

/*	[Beschreibung]

	Die Klasse SfxShell ist Basisklasse f"ur alle Schichten, die
	Funktionalit"at Form von <Slot>s bereitstellen wollen.

	Jede Instanz hat einen Verweis auf eine Interface-Beschreibung, der
	mit <SfxShell::GetInterface()const> erh"altlich ist. Dieses Interface
	stellt die Verbindung zu konkreten Methoden her und enth"alt einige
	weitere beschreibende Daten f"ur Controller wie Menus und Toolboxen, aber
	auch f"ur die diversen APIs. Der Hautpteil der Interface-Beschreibung
	liegt in Form einer <Type-Library> vor, die mit dem <SVIDL-Compiler>
	aus einem IDL-File generiert wird. F"ur jede SfxShell-Subclass ist ein
	solches IDL-File zu schreiben.

*/

{
	friend class SfxObjectItem;

	SfxShell_Impl*				pImp;
	SfxItemPool*				pPool;
    ::svl::IUndoManager*        pUndoMgr;

private:
								SfxShell( const SfxShell & ); // n.i.
	SfxShell&					operator = ( const SfxShell & ); // n.i.

protected:
								SfxShell();
								SfxShell( SfxViewShell *pViewSh );

#ifndef _SFXSH_HXX
    SAL_DLLPRIVATE void SetViewShell_Impl( SfxViewShell* pView );
    SAL_DLLPRIVATE void Invalidate_Impl( SfxBindings& rBindings, sal_uInt16 nId );
	SAL_DLLPRIVATE SfxShellObject* GetShellObj_Impl() const;
	SAL_DLLPRIVATE void SetShellObj_Impl( SfxShellObject* pObj );
#endif

public:
								TYPEINFO();
	virtual                     ~SfxShell();

	SfxBroadcaster*				GetBroadcaster();

    // TODO/CLEANUP: still needed?!
    virtual SvGlobalName        GetGlobalName() const;

    virtual SfxInterface*       GetInterface() const;
    static SfxInterface*        GetStaticInterface() { return 0; }

	void						SetName( const String &rName );
	const String&   			GetName() const;

	SfxViewShell*				GetViewShell() const;

	void                        CallExec( SfxExecFunc pFunc, SfxRequest &rReq )
                                { (*pFunc)(this, rReq); }
	void                        CallState( SfxStateFunc pFunc, SfxItemSet &rSet )
                                { (*pFunc)(this, rSet); }

	static void                 EmptyExecStub(SfxShell *pShell, SfxRequest &);
	static void                 EmptyStateStub(SfxShell *pShell, SfxItemSet &);

    const SfxPoolItem*          GetSlotState( sal_uInt16 nSlotId, const SfxInterface *pIF = 0, SfxItemSet *pStateSet = 0 );
    const SfxPoolItem*          ExecuteSlot( SfxRequest &rReq, const SfxInterface *pIF = 0 );
	const SfxPoolItem*			ExecuteSlot( SfxRequest &rReq, sal_Bool bAsync );
    sal_uIntPtr                       ExecuteSlot( sal_uInt16 nSlot, sal_uInt16 nMemberId, SbxVariable& rRet, SbxBase* pArgs = 0 );

    inline SfxItemPool&         GetPool() const;
	inline void					SetPool( SfxItemPool *pNewPool ) ;

    virtual ::svl::IUndoManager*
                                GetUndoManager();
	void						SetUndoManager( ::svl::IUndoManager *pNewUndoMgr );

	SfxRepeatTarget*			GetRepeatTarget() const;
	void					    SetRepeatTarget( SfxRepeatTarget *pTarget );

    virtual void                Invalidate(sal_uInt16 nId = 0);

	sal_Bool						IsActive() const;
	virtual void                Activate(sal_Bool bMDI);
	virtual void                Deactivate(sal_Bool bMDI);
	virtual void           		ParentActivate();
	virtual	void           		ParentDeactivate();

	SfxDispatcher*				GetDispatcher() const;
	SfxViewFrame*				GetFrame() const;
	ResMgr* 					GetResMgr() const;
	virtual	sal_Bool			HasUIFeature( sal_uInt32 nFeature );
	void						UIFeatureChanged();

	// Items
	const SfxPoolItem*			GetItem( sal_uInt16 nSlotId ) const;
	void						PutItem( const SfxPoolItem& rItem );
	void						RemoveItem( sal_uInt16 nSlotId );

    // TODO/CLEANUP: still needed?!
    void SetVerbs(const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& aVerbs);
    const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& GetVerbs() const;
	void						VerbExec (SfxRequest&);
	void						VerbState (SfxItemSet&);
	SAL_DLLPRIVATE const SfxSlot* GetVerbSlot_Impl(sal_uInt16 nId) const;

	void						SetHelpId(sal_uIntPtr nId);
	sal_uIntPtr						GetHelpId() const;
	virtual	SfxObjectShell*		GetObjectShell();
	void						SetDisableFlags( sal_uIntPtr nFlags );
	sal_uIntPtr						GetDisableFlags() const;

    virtual SfxItemSet*         CreateItemSet( sal_uInt16 nId );
    virtual void                ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );

    /** Set the name of the sidebar context that is broadcast on calls
        to Activation().
    */
    void SetContextName (const ::rtl::OUString& rsContextName);

    /** Broadcast a sidebar context change.
        This method is typically called from Activate() or
        Deactivate().
        @param bIsActivated
            When <TRUE/> then broadcast the context name that was
            defined with an earlier call to SetContextName().
            When <FALSE/> then broadcast the 'default' context.
    */
    void BroadcastContextForActivation (const bool bIsActivated);

    /** Enabled or disable the context broadcaster.  Returns the old state.
    */
    bool SetContextBroadcasterEnabled (const bool bIsEnabled);

#ifndef _SFXSH_HXX
    SAL_DLLPRIVATE bool     CanExecuteSlot_Impl( const SfxSlot &rSlot );
    SAL_DLLPRIVATE void DoActivate_Impl( SfxViewFrame *pFrame, sal_Bool bMDI);
    SAL_DLLPRIVATE void DoDeactivate_Impl( SfxViewFrame *pFrame, sal_Bool bMDI);
#endif
};

//--------------------------------------------------------------------
SfxItemPool& SfxShell::GetPool() const
/*
  [Beschreibung]

	Jede Subclass von SfxShell mu"s einen Pool referenzieren. Dieser
	wird teilweise von SFx-eigenen Subklassen gesetzt (z.B. <SfxViewShell>),
	mu"s aber insbesondere bei direkt von SfxShell abgeleiteten Klassen
	und bei Ableitungen von SfxObjectShell selbst gesetzt werden.

	Die Klasse SfxShell selbst hat noch keinen SfxItemPool, es wird
	daher ein 0-Pointer zur"uckgeliefert.
*/

{
	DBG_ASSERT( pPool, "no pool" );
	return *pPool;
}
//-------------------------------------------------------------------
inline void SfxShell::SetPool
(
	SfxItemPool*	pNewPool	// Pointer auf den neuen Pool oder 0
)

/*  [Beschreibung]

	Mit dieser Methode melden die Subklassen ihren speziellen <SfxItemPool>
	an der SfxShell an. Jede SfxShell Instanz mu\s Zugriff auf einen
	SfxItemPool haben. In der Regel ist dies der SfxItemPool der
	SfxDocumentShell. Die SfxShell Subklasse "ubernimmt nicht die
	Eigent"umerschaft "uber den "ubergebenen Pool. Bevor er gel"oscht
	wirde, mu\s er mit SetPool(0) abgemeldet werden.
*/

{
	pPool = pNewPool;
}

//=====================================================================

#define SFX_ARGUMENTMAP(ShellClass) static SfxFormalArgument __FAR_DATA a##ShellClass##Args_Impl[] =

#define SFX_SLOTMAP(ShellClass) static SfxFormalArgument __FAR_DATA a##ShellClass##Args_Impl[1]; \
								static SfxSlot __FAR_DATA a##ShellClass##Slots_Impl[] =

#define SFX_SLOTMAP_ARG(ShellClass) static SfxSlot __FAR_DATA a##ShellClass##Slots_Impl[] =

#define SFX_DECL_INTERFACE(nId) 											\
            static SfxInterface*                pInterface;                 \
        private:                                                            \
			static void 						InitInterface_Impl();		\
        public:                                                             \
            static const SfxFormalArgument*     pSfxFormalArgs_Impl;        \
            static SfxInterface*                GetStaticInterface();       \
            static SfxInterfaceId               GetInterfaceId() {return SfxInterfaceId(nId);} \
            static void                         RegisterInterface(SfxModule* pMod=NULL); \
            virtual SfxInterface*       GetInterface() const;

#define SFX_IMPL_INTERFACE(Class,SuperClass,NameResId)                      \
																			\
    SfxInterface* Class::pInterface = 0;                                    \
    const SfxFormalArgument* Class::pSfxFormalArgs_Impl = a##Class##Args_Impl;\
    SfxInterface* __EXPORT Class::GetStaticInterface()                      \
    {                                                                       \
        if ( !pInterface )                                                  \
        {                                                                   \
            pInterface =                                                    \
                new SfxInterface(                                           \
            #Class, NameResId, GetInterfaceId(),                            \
            SuperClass::GetStaticInterface(),                               \
            a##Class##Slots_Impl[0],                                        \
            (sal_uInt16) (sizeof(a##Class##Slots_Impl) / sizeof(SfxSlot) ) );   \
            InitInterface_Impl();                                           \
        }                                                                   \
        return pInterface;                                                  \
    }                                                                       \
																			\
    SfxInterface* Class::GetInterface() const                               \
    {                                                                       \
        return GetStaticInterface();                                        \
    }                                                                       \
                                                                            \
    void Class::RegisterInterface(SfxModule* pMod)                          \
    {                                                                       \
        GetStaticInterface()->Register(pMod);                               \
    }                                                                       \
																			\
	void Class::InitInterface_Impl()

#define SFX_POSITION_MASK				0x000F
#define SFX_VISIBILITY_MASK 			0xFFF0
#define SFX_VISIBILITY_UNVISIBLE		0x0000	// nie sichtbar
#define SFX_VISIBILITY_PLUGSERVER       0x0010
#define SFX_VISIBILITY_PLUGCLIENT       0x0020
#define SFX_VISIBILITY_VIEWER	        0x0040
												// noch 1 sind frei!
#define SFX_VISIBILITY_RECORDING		0x0200
#define SFX_VISIBILITY_READONLYDOC		0x0400
#define SFX_VISIBILITY_DESKTOP    		0x0800
#define SFX_VISIBILITY_STANDARD 		0x1000
#define SFX_VISIBILITY_FULLSCREEN		0x2000
#define SFX_VISIBILITY_CLIENT			0x4000
#define SFX_VISIBILITY_SERVER			0x8000
#define SFX_VISIBILITY_NOCONTEXT		0xFFFF	// immer sichtbar

#define SFX_OBJECTBAR_REGISTRATION(nPos,rResId) \
        GetStaticInterface()->RegisterObjectBar( nPos, rResId )

#define SFX_FEATURED_OBJECTBAR_REGISTRATION(nPos,rResId,nFeature) \
        GetStaticInterface()->RegisterObjectBar( nPos, rResId, nFeature )

#define SFX_CHILDWINDOW_REGISTRATION(nId) \
        GetStaticInterface()->RegisterChildWindow( nId, (sal_Bool) sal_False )

#define SFX_FEATURED_CHILDWINDOW_REGISTRATION(nId,nFeature) \
        GetStaticInterface()->RegisterChildWindow( nId, (sal_Bool) sal_False, nFeature )

#define SFX_CHILDWINDOW_CONTEXT_REGISTRATION(nId) \
        GetStaticInterface()->RegisterChildWindow( nId, (sal_Bool) sal_True )

#define SFX_POPUPMENU_REGISTRATION(rResId) \
        GetStaticInterface()->RegisterPopupMenu( rResId )

#define SFX_OBJECTMENU_REGISTRATION(nPos,rResId) \
        GetStaticInterface()->RegisterObjectMenu( nPos, rResId )

#define SFX_STATUSBAR_REGISTRATION(rResId) \
        GetStaticInterface()->RegisterStatusBar( rResId )

#endif

