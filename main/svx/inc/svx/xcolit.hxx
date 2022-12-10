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



#ifndef _SVX_XCOLORITEM_HXX
#define _SVX_XCOLORITEM_HXX

#include <tools/color.hxx>
#include "svx/svxdllapi.h"
#include <svx/xit.hxx>
#include <boost/shared_ptr.hpp>

//-----------------
// class XColorItem
//-----------------

class XColorList;
typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;

class SVX_DLLPUBLIC XColorItem : public NameOrIndex
{
	Color   aColor;

public:
			TYPEINFO();
			XColorItem() {}
			XColorItem(sal_uInt16 nWhich, sal_Int32 nIndex, const Color& rTheColor);

	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

			XColorItem(sal_uInt16 nWhich, const String& rName, const Color& rTheColor);
			XColorItem(sal_uInt16 nWhich, SvStream& rIn);
			XColorItem(const XColorItem& rItem);

	virtual int             operator==(const SfxPoolItem& rItem) const;
	virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
	virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
	virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

	const Color&    GetColorValue() const;
	const Color&    GetColorValue(const XColorListSharedPtr aTable) const;
	void            SetColorValue(const Color& rNew) { aColor = rNew; Detach(); }
};

#endif

