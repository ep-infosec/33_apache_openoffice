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


#ifndef _SDTAAITM_HXX
#define _SDTAAITM_HXX

#include <svl/intitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SdrTextAniAmountItem: public SfxInt16Item {
public:
	TYPEINFO();
	SdrTextAniAmountItem(sal_Int16 nVal=0): SfxInt16Item(SDRATTR_TEXT_ANIAMOUNT,nVal) {}
	SdrTextAniAmountItem(SvStream& rIn): SfxInt16Item(SDRATTR_TEXT_ANIAMOUNT,rIn) {}
	virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
	virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;
	virtual FASTBOOL HasMetrics() const;
	virtual FASTBOOL ScaleMetrics(long nMul, long nDiv);

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
