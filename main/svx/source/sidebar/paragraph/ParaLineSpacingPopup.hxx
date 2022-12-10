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

#ifndef _SVX_SIDEBAR_LINE_SPACING_POPUP_HXX_
#define _SVX_SIDEBAR_LINE_SPACING_POPUP_HXX_

#include "svx/sidebar/Popup.hxx"

#include <boost/function.hpp>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

namespace svx { namespace sidebar {

class ParaLineSpacingPopup
    : public Popup
{
public :
    ParaLineSpacingPopup (
        Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator);
    ~ParaLineSpacingPopup (void);

    void Rearrange (SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem ,const ::sfx2::sidebar::EnumContext currentContext);
private:
    void PopupModeEndCallback (void);
};

} } // end of namespace svx::sidebar

#endif


