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


#ifndef _SVX_CHARHIDDENITEM_HXX
#define _SVX_CHARHIDDENITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include "editeng/editengdllapi.h"

// class SvxCharHiddenItem -------------------------------------------------
/*
    [Description]
    This item marks text as hidden
*/

class EDITENG_DLLPUBLIC SvxCharHiddenItem : public SfxBoolItem
{
public:
    TYPEINFO();

    SvxCharHiddenItem( const sal_Bool bHidden /*= sal_False*/, const sal_uInt16 nId );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    inline SvxCharHiddenItem& operator=(const SvxCharHiddenItem& rHidden) {
            SetValue(rHidden.GetValue());
            return *this;
        }
};

#endif
