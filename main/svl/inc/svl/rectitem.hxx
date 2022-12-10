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


#ifndef _SFXRECTITEM_HXX
#define _SFXRECTITEM_HXX

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <svl/poolitem.hxx>

class SvStream;

DBG_NAMEEX_VISIBILITY(SfxRectangleItem, SVL_DLLPUBLIC)

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxRectangleItem: public SfxPoolItem
{
	Rectangle				 aVal;

public:
							 TYPEINFO();
							 SfxRectangleItem();
							 SfxRectangleItem( sal_uInt16 nWhich, const Rectangle& rVal );
							 SfxRectangleItem( sal_uInt16 nWhich, SvStream & );
							 SfxRectangleItem( const SfxRectangleItem& );
							 ~SfxRectangleItem() {
								 DBG_DTOR(SfxRectangleItem, 0); }

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
									XubString &rText,
                                    const IntlWrapper * = 0 ) const;

	virtual int 			 operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*	 Create(SvStream &, sal_uInt16 nItemVersion) const;
	virtual SvStream&		 Store(SvStream &, sal_uInt16 nItemVersion) const;

	const Rectangle&    	 GetValue() const { return aVal; }
			void			 SetValue( const Rectangle& rNewVal ) {
								 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
								 aVal = rNewVal;
							 }
	virtual	sal_Bool 			 QueryValue( com::sun::star::uno::Any& rVal,
							 			 sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool 			 PutValue( const com::sun::star::uno::Any& rVal,
						   			   sal_uInt8 nMemberId = 0 );
};

#endif

