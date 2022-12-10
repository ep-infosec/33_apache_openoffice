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


#ifndef _SVX_KERNITEM_HXX
#define _SVX_KERNITEM_HXX

// include ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
	class OUString;
}

// class SvxKerningItem --------------------------------------------------

// Achtung: Twips-Werte
// Twips: 0 = kein Kerning

/*	[Beschreibung]

	Dieses Item beschreibt die Schrift-Laufweite.
*/

class EDITENG_DLLPUBLIC SvxKerningItem : public SfxInt16Item
{
public:
	TYPEINFO();

    SvxKerningItem( const short nKern /*= 0*/, const sal_uInt16 nId  );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
	virtual SvStream&		Store(SvStream &, sal_uInt16 nItemVersion) const;
	virtual int				ScaleMetrics( long nMult, long nDiv );
	virtual	int				HasMetrics() const;

	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	inline SvxKerningItem& operator=(const SvxKerningItem& rKern) {
			SetValue( rKern.GetValue() );
			return *this;
		}

	virtual	sal_Bool        	QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif

