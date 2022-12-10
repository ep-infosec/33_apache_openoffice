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


#ifndef _SVX_SMARTTAGITEM_HXX
#define _SVX_SMARTTAGITEM_HXX

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include "svx/svxdllapi.h"

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
} } } }

namespace com { namespace sun { namespace star { namespace frame {
    class XController;
} } } }

// class SvxSmartTagItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxSmartTagItem : public SfxPoolItem
{
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > > maActionComponentsSequence;
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > > maActionIndicesSequence;
    const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > > maStringKeyMaps;
    const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> mxRange;
    const com::sun::star::uno::Reference<com::sun::star::frame::XController> mxController;
    const com::sun::star::lang::Locale maLocale;
    const rtl::OUString maApplicationName;
    const rtl::OUString maRangeText;

public:
	TYPEINFO();

	SvxSmartTagItem( const sal_uInt16 nId,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                     const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                     const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                     const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> rRange,
                     const com::sun::star::uno::Reference<com::sun::star::frame::XController> rController,
                     const com::sun::star::lang::Locale rLocale,
                     const rtl::OUString& rApplicationName,
                     const rtl::OUString& rRangeText );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 			 operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*	 Create(SvStream &, sal_uInt16) const; // leer
	virtual SvStream&		 Store(SvStream &, sal_uInt16 nItemVersion) const; // leer 
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const; // leer
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ); // leer

    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& GetActionComponentsSequence() const { return maActionComponentsSequence; }
    const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& GetActionIndicesSequence() const { return maActionIndicesSequence; }
    const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& GetStringKeyMaps() const { return maStringKeyMaps; }
    const com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& GetTextRange() const { return mxRange; }
    const com::sun::star::uno::Reference<com::sun::star::frame::XController>& GetController() const { return mxController; }
    const com::sun::star::lang::Locale& GetLocale() const { return maLocale; }
    const rtl::OUString GetApplicationName() const { return maApplicationName; }
    const rtl::OUString GetRangeText() const { return maRangeText; }
};

#endif // #ifndef _SVX_SMARTTAGITEM_HXX


