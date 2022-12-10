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

#include "precompiled_sd.hxx"

#include "RecentMasterPagesSelector.hxx"

#include "ViewShellBase.hxx"
#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageContainerProviders.hxx"
#include "MasterPageObserver.hxx"
#include "SidebarShellManager.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "helpids.h"

#include <vcl/bitmap.hxx>
#include <tools/color.hxx>

namespace sd { namespace sidebar {


MasterPagesSelector* RecentMasterPagesSelector::Create (
    ::Window* pParent,
    ViewShellBase& rViewShellBase,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    SdDrawDocument* pDocument = rViewShellBase.GetDocument();
    if (pDocument == NULL)
        return NULL;

    ::boost::shared_ptr<MasterPageContainer> pContainer (new MasterPageContainer());
    
    MasterPagesSelector* pSelector(
        new RecentMasterPagesSelector (
            pParent, 
            *pDocument,
            rViewShellBase,
            pContainer,
            rxSidebar));
    pSelector->LateInit();
    pSelector->SetHelpId(HID_SD_TASK_PANE_PREVIEW_RECENT);

    return pSelector;
}




RecentMasterPagesSelector::RecentMasterPagesSelector (
    ::Window* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
    const cssu::Reference<css::ui::XSidebar>& rxSidebar)
    : MasterPagesSelector (pParent, rDocument, rBase, rpContainer, rxSidebar)
{
}




RecentMasterPagesSelector::~RecentMasterPagesSelector (void)
{
    RecentlyUsedMasterPages::Instance().RemoveEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




void RecentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit();

    MasterPagesSelector::Fill();
    RecentlyUsedMasterPages::Instance().AddEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




IMPL_LINK(RecentMasterPagesSelector,MasterPageListListener, void*, EMPTYARG)
{
    MasterPagesSelector::Fill();
    return 0;
}




void RecentMasterPagesSelector::Fill (ItemList& rItemList)
{
    // Create a set of names of the master pages used by the document.
    MasterPageObserver::MasterPageNameSet aCurrentNames;
    sal_uInt16 nMasterPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    sal_uInt16 nIndex;
    for (nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != NULL)
            aCurrentNames.insert (pMasterPage->GetName());
    }
    MasterPageObserver::MasterPageNameSet::iterator aI;

    // Insert the recently used master pages that are currently not used.
    RecentlyUsedMasterPages& rInstance (RecentlyUsedMasterPages::Instance());
    int nPageCount = rInstance.GetMasterPageCount();
    for (nIndex=0; nIndex<nPageCount; nIndex++)
    {
        // Add an entry when a) the page is already known to the
        // MasterPageContainer, b) the style name is empty, i.e. it has not yet
        // been loaded (and thus can not be in use) or otherwise c) the
        // style name is not currently in use.
        MasterPageContainer::Token aToken (rInstance.GetTokenForIndex(nIndex));
        if (aToken != MasterPageContainer::NIL_TOKEN)
        {
            String sStyleName (mpContainer->GetStyleNameForToken(aToken));
            if (sStyleName.Len()==0 
                || aCurrentNames.find(sStyleName) == aCurrentNames.end())
            {
                rItemList.push_back(aToken);
            }
        }
    }
}




void RecentMasterPagesSelector::AssignMasterPageToPageList (
    SdPage* pMasterPage,
    const ::boost::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
	sal_uInt16 nSelectedItemId = PreviewValueSet::GetSelectItemId();
    
    MasterPagesSelector::AssignMasterPageToPageList(pMasterPage, rpPageList);

    // Restore the selection.
    if (PreviewValueSet::GetItemCount() > 0)
    {
        if (PreviewValueSet::GetItemCount() >= nSelectedItemId)
            PreviewValueSet::SelectItem(nSelectedItemId);
        else
            PreviewValueSet::SelectItem(PreviewValueSet::GetItemCount());
    }
}




void RecentMasterPagesSelector::ProcessPopupMenu (Menu& rMenu)
{
    if (rMenu.GetItemPos(SID_TP_EDIT_MASTER) != MENU_ITEM_NOTFOUND)
        rMenu.EnableItem(SID_TP_EDIT_MASTER, sal_False);
}




} } // end of namespace sd::sidebar
