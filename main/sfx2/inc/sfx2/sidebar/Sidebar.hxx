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

#ifndef SFX_SIDEBAR_SIDEBAR_HXX
#define SFX_SIDEBAR_SIDEBAR_HXX

#include "sfx2/dllapi.h"
#include <com/sun/star/frame/XFrame.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {

/** Accept requests for switching to certain panels or decks.
*/
class SFX2_DLLPUBLIC Sidebar
{
public:
    /** Switch to the deck that contains the specified panel and make
        sure that the panel is visible (expanded and scrolled into the
        visible area.)
        Note that most of the work is done asynchronously and that
        this function probably returns before the requested panel is visible.
    */        
    static void ShowPanel (
        const ::rtl::OUString& rsPanelId,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    /** Switch to the specified deck.
        Note that most of the work is done asynchronously and that
        this function probably returns before the requested deck is visible.
    */        
    static void ShowDeck (
        const ::rtl::OUString& rsDeckId,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
};

} } // end of namespace sfx2::sidebar

#endif
