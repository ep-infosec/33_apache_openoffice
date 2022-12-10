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


#ifndef _SVX_CRSDITEM_HXX
#define _SVX_CRSDITEM_HXX

// include ---------------------------------------------------------------

#include <vcl/vclenum.hxx>
#include <svl/eitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
	class OUString;
}

// class SvxCrossedOutItem -----------------------------------------------

/*
	[Beschreibung]
	Dieses Item beschreibt, ob und wie durchstrichen ist.
*/

class EDITENG_DLLPUBLIC SvxCrossedOutItem : public SfxEnumItem
{
public:
	TYPEINFO();

    SvxCrossedOutItem( const FontStrikeout eSt /*= STRIKEOUT_NONE*/,
                       const sal_uInt16 nId  );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
	virtual SvStream&		Store(SvStream &, sal_uInt16 nItemVersion) const;
	virtual String			GetValueTextByPos( sal_uInt16 nPos ) const;
	virtual sal_uInt16          GetValueCount() const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	// MS VC4.0 kommt durcheinander
	void			 		SetValue( sal_uInt16 nNewVal )
								{SfxEnumItem::SetValue(nNewVal); }

	virtual int 			HasBoolValue() const;
	virtual sal_Bool			GetBoolValue() const;
	virtual void			SetBoolValue( sal_Bool bVal );

	inline SvxCrossedOutItem& operator=(const SvxCrossedOutItem& rCross)
		{
			SetValue( rCross.GetValue() );
			return *this;
		}

	// enum cast
	FontStrikeout 			GetStrikeout() const
								{ return (FontStrikeout)GetValue(); }
	void 					SetStrikeout( FontStrikeout eNew )
								{ SetValue( (sal_uInt16)eNew ); }
};

#endif // #ifndef _SVX_CRSDITEM_HXX

