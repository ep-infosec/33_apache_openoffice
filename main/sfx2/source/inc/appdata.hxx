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


#ifndef _SFX_APPDATA_HXX
#define _SFX_APPDATA_HXX

#include <tools/link.hxx>
#include <tools/list.hxx>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>
#include <tools/string.hxx>
#include "rtl/ref.hxx"

#include <com/sun/star/frame/XModel.hpp>

#include "bitset.hxx"

class SfxApplication;
class SvStrings;
class SfxProgress;
class SfxChildWinFactArr_Impl;
class SfxDdeDocTopics_Impl;
class DdeService;
class SfxEventConfiguration;
class SfxMacroConfig;
class SfxItemPool;
class SfxInitLinkList;
class SfxFilterMatcher;
class SvUShorts;
class ISfxTemplateCommon;
class SfxFilterMatcher;
class SfxStatusDispatcher;
class SfxDdeTriggerTopic_Impl;
class SfxDocumentTemplates;
class SfxFrameArr_Impl;
class SvtSaveOptions;
class SvtUndoOptions;
class SvtHelpOptions;
class SfxObjectFactory;
class SfxObjectShell;
class ResMgr;
class Window;
class SfxTbxCtrlFactArr_Impl;
class SfxStbCtrlFactArr_Impl;
class SfxMenuCtrlFactArr_Impl;
class SfxViewFrameArr_Impl;
class SfxViewShellArr_Impl;
class SfxObjectShellArr_Impl;
class ResMgr;
class SimpleResMgr;
class SfxViewFrame;
class SfxSlotPool;
class SfxResourceManager;
class SfxDispatcher;
class SfxInterface;
class BasicManager;
class SfxBasicManagerHolder;
class SfxBasicManagerCreationListener;

namespace sfx2 { namespace appl { class ImeStatusWindow; } }

typedef Link* LinkPtr;
SV_DECL_PTRARR(SfxInitLinkList, LinkPtr, 4, 4)

//=========================================================================
// SfxAppData_Impl
//=========================================================================

class SfxAppData_Impl
{
public:
    IndexBitSet                         aIndexBitSet;           // for counting noname documents
    String                              aLastDir;               // for IO dialog

    // DDE stuff
    DdeService*                         pDdeService;
	SfxDdeDocTopics_Impl*               pDocTopics;
	SfxDdeTriggerTopic_Impl*            pTriggerTopic;
	DdeService*                         pDdeService2;

    // single instance classes
    SfxChildWinFactArr_Impl*            pFactArr;
	SfxFrameArr_Impl*                   pTopFrames;

    // special members
	SfxInitLinkList*                    pInitLinkList;

    // application members
	SfxFilterMatcher*                   pMatcher;
	ResMgr*                             pBasicResMgr;
	ResMgr*                             pSvtResMgr;
	SfxStatusDispatcher*				pAppDispatch;
    SfxDocumentTemplates*               pTemplates;

    // global pointers
	SfxItemPool*                        pPool;
	SvUShorts*                          pDisabledSlotList;
	SvStrings*                          pSecureURLs;
    SvtSaveOptions*                     pSaveOptions;
    SvtUndoOptions*                     pUndoOptions;
    SvtHelpOptions*                     pHelpOptions;

    // "current" functionality
	SfxProgress*                        pProgress;
	ISfxTemplateCommon*                 pTemplateCommon;

    sal_uInt16                              nDocModalMode;              // counts documents in modal mode
	sal_uInt16                              nAutoTabPageId;
	sal_uInt16                              nRescheduleLocks;
	sal_uInt16                              nInReschedule;
	sal_uInt16                              nAsynchronCalls;

    rtl::Reference< sfx2::appl::ImeStatusWindow > m_xImeStatusWindow;

    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxViewFrameArr_Impl*       pViewFrames;
    SfxViewShellArr_Impl*       pViewShells;
    SfxObjectShellArr_Impl*     pObjShells;
    ResMgr*                     pSfxResManager;
    ResMgr*                     pOfaResMgr;
    SimpleResMgr*				pSimpleResManager;
    SfxBasicManagerHolder*      pBasicManager;
    SfxBasicManagerCreationListener*
                                pBasMgrListener;
    SfxViewFrame*               pViewFrame;
    SfxSlotPool*                pSlotPool;
    SfxResourceManager*         pResMgr;
    SfxDispatcher*              pAppDispat;     // Dispatcher falls kein Doc
    SfxInterface**              pInterfaces;

    sal_uInt16                      nDocNo;     		// Laufende Doc-Nummer (AutoName)
    sal_uInt16                      nInterfaces;

    sal_Bool                        bDispatcherLocked:1;    // nichts ausf"uhren
    sal_Bool                        bDowning:1;   // sal_True ab Exit und danach
    sal_Bool                        bInQuit : 1;
    sal_Bool                        bInvalidateOnUnlock : 1;
    sal_Bool                        bODFVersionWarningLater : 1;

                                SfxAppData_Impl( SfxApplication* );
                                ~SfxAppData_Impl();

    void                        UpdateApplicationSettings( sal_Bool bDontHide );
    SfxDocumentTemplates*       GetDocumentTemplates();
    void                        DeInitDDE();

    /** called when the Application's BasicManager has been created. This can happen
        explicitly in SfxApplication::GetBasicManager, or implicitly if a document's
        BasicManager is created before the application's BasicManager exists.
    */
    void                        OnApplicationBasicManagerCreated( BasicManager& _rManager );
};

#endif // #ifndef _SFX_APPDATA_HXX


