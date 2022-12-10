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

#ifndef SD_SIDEBAR_PANELS_ALL_MASTER_PAGES_SELECTOR_HXX
#define SD_SIDEBAR_PANELS_ALL_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesSelector.hxx"

#include <memory>

namespace sd { namespace sidebar {


/** Show a list of all available master pages so that the user can assign
    them to the document.
*/
class AllMasterPagesSelector
    : public MasterPagesSelector
{
public:
    static MasterPagesSelector* Create (
        ::Window* pParent,
        ViewShellBase& rViewShellBase,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    
    /** Scan the set of templates for the ones whose first master pages are
        shown by this control and store them in the MasterPageContainer.
    */
    virtual void Fill (ItemList& rItemList);

    virtual void GetState (SfxItemSet& rItemSet);

protected:
    virtual void NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent);

private:
    /** The list of master pages displayed by this class.
    */
    class SortedMasterPageDescriptorList;
    ::std::auto_ptr<SortedMasterPageDescriptorList> mpSortedMasterPages;

    AllMasterPagesSelector (
        ::Window* pParent, 
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~AllMasterPagesSelector (void);

    void AddTemplate (const TemplateEntry& rEntry);

    /** This filter returns <TRUE/> when the master page specified by the
        given file name belongs to the set of Impress master pages.
    */
    bool FileFilter (const String& sFileName);

    void AddItem (MasterPageContainer::Token aToken);

    /** Add all items in the internal master page list into the given list.
    */
    void UpdatePageSet (ItemList& rItemList);

    /** Update the internal list of master pages that are to show in the
        control.
    */
    void UpdateMasterPageList (void);

	using MasterPagesSelector::Fill;
};

} } // end of namespace sd::sidebar

#endif
