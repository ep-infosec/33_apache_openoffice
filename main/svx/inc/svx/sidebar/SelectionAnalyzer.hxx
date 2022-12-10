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

#ifndef _SVX_SIDEBAR_SELECTION_ANALYZER_HXX_
#define _SVX_SIDEBAR_SELECTION_ANALYZER_HXX_

#include <sfx2/sidebar/EnumContext.hxx>
#include "svx/svxdllapi.h"

class SdrMarkList;
class SdrObject;


namespace svx { namespace sidebar {

/** Analyze the current selection of Calc or Draw/Impress documents
    and return the associated sidebar context.

    The decision is based on heuristics.  Do not expect pretty code.
*/
class SVX_DLLPUBLIC SelectionAnalyzer
{
public :
    static sfx2::sidebar::EnumContext::Context GetContextForSelection_SC (
        const SdrMarkList& rMarkList);

    enum ViewType
    {
        VT_Standard,
        VT_Master,
        VT_Handout,
        VT_Notes,
        VT_Outline
    };
    static sfx2::sidebar::EnumContext::Context GetContextForSelection_SD (
        const SdrMarkList& rMarkList,
        const ViewType eViewType);

private:
    static sfx2::sidebar::EnumContext::Context GetContextForObjectId_SC (
        const sal_uInt16 nObjectId);
    static sfx2::sidebar::EnumContext::Context GetContextForObjectId_SD (
        const sal_uInt16 nObjectId,
        const ViewType eViewType);
    static sal_uInt32 GetInventorTypeFromMark (
        const SdrMarkList& rMarkList);
    static sal_uInt16 GetObjectTypeFromMark (
        const SdrMarkList& rMarkList);
    static sal_uInt16 GetObjectTypeFromGroup (
        const SdrObject* pObj);
    static bool IsShapeType (
        const sal_uInt16 nType);
    static bool IsTextObjType (
        const sal_uInt16 nType);
};

} } // end of namespace svx::sidebar

#endif
