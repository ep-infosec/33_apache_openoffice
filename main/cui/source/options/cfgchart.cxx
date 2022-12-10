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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

#include <com/sun/star/uno/Sequence.hxx>
// header for SvStream
#include <tools/stream.hxx>
// header for SAL_STATIC_CAST
#include <sal/types.h>
#include "cfgchart.hxx"
#include <dialmgr.hxx>
#include <cuires.hrc>

#define ROW_COLOR_COUNT 12

using namespace com::sun::star;

TYPEINIT1( SvxChartColorTableItem, SfxPoolItem );

SvxChartColorTable::SvxChartColorTable()
{}

SvxChartColorTable::SvxChartColorTable( const SvxChartColorTable & _rSource ) :
        m_aColorEntries( _rSource.m_aColorEntries )
{}

// accessors
size_t SvxChartColorTable::size() const
{
    return m_aColorEntries.size();
}

const XColorEntry & SvxChartColorTable::operator[]( size_t _nIndex ) const
{
    if ( _nIndex >= m_aColorEntries.size() )
    {
        DBG_ERRORFILE( "SvxChartColorTable::[] invalid index" );
        return m_aColorEntries[ 0 ];
    }

    return m_aColorEntries[ _nIndex ];
}

ColorData SvxChartColorTable::getColorData( size_t _nIndex ) const
{
    if ( _nIndex >= m_aColorEntries.size() )
    {
        DBG_ERRORFILE( "SvxChartColorTable::getColorData invalid index" );
        return COL_BLACK;
    }

    // GetColor should be const but unfortunately isn't
    return const_cast< XColorEntry & >( m_aColorEntries[ _nIndex ] ).GetColor().GetRGBColor();
}

// mutators
void SvxChartColorTable::clear()
{
    m_aColorEntries.clear();
}

void SvxChartColorTable::append( const XColorEntry & _rEntry )
{
    m_aColorEntries.push_back( _rEntry );
}

void SvxChartColorTable::replace( size_t _nIndex, const XColorEntry & _rEntry )
{
    DBG_ASSERT( _nIndex <= m_aColorEntries.size(),
                "SvxChartColorTable::replace invalid index" );

	Color aCol1 = m_aColorEntries[ _nIndex ].GetColor(), aCol2;
    m_aColorEntries[ _nIndex ] = _rEntry;
	aCol2 = m_aColorEntries[ _nIndex ].GetColor();
	if ( aCol2 != const_cast< XColorEntry& >( _rEntry ).GetColor() )
	{
		DBG_ERRORFILE( "wrong color" );
	}
}

void SvxChartColorTable::useDefault()
{
	ColorData aColors[] = {
		RGB_COLORDATA( 0x00, 0x45, 0x86 ),
		RGB_COLORDATA( 0xff, 0x42, 0x0e ),
		RGB_COLORDATA( 0xff, 0xd3, 0x20 ),
		RGB_COLORDATA( 0x57, 0x9d, 0x1c ),
		RGB_COLORDATA( 0x7e, 0x00, 0x21 ),
		RGB_COLORDATA( 0x83, 0xca, 0xff ),
		RGB_COLORDATA( 0x31, 0x40, 0x04 ),
		RGB_COLORDATA( 0xae, 0xcf, 0x00 ),
		RGB_COLORDATA( 0x4b, 0x1f, 0x6f ),
		RGB_COLORDATA( 0xff, 0x95, 0x0e ),
		RGB_COLORDATA( 0xc5, 0x00, 0x0b ),
		RGB_COLORDATA( 0x00, 0x84, 0xd1 )
	};

    clear();

	String aResName( CUI_RES( RID_SVXSTR_DIAGRAM_ROW ) );
	String aPrefix, aPostfix, aName;
	xub_StrLen nPos = aResName.SearchAscii( "$(ROW)" );
	if( nPos != STRING_NOTFOUND )
	{
		aPrefix = String( aResName, 0, nPos );
		aPostfix = String( aResName, nPos + sizeof( "$(ROW)" ) - 1, STRING_LEN );
	}
	else
		aPrefix = aResName;

	for( sal_Int32 i=0; i<ROW_COLOR_COUNT; i++ )
	{
		aName = aPrefix;
		aName.Append( String::CreateFromInt32( i + 1 ));
		aName.Append( aPostfix );

        append( XColorEntry( aColors[ i % sizeof( aColors ) ], aName ));
	}
}

// comparison
bool SvxChartColorTable::operator==( const SvxChartColorTable & _rOther ) const
{
    // note: XColorEntry has no operator ==
    bool bEqual = ( this->m_aColorEntries.size() == _rOther.m_aColorEntries.size() );

    if( bEqual )
    {
        for( size_t i = 0; i < m_aColorEntries.size(); ++i )
        {
            if( getColorData( i ) != _rOther.getColorData( i ))
            {
                bEqual = false;
                break;
            }
        }
    }

    return bEqual;
}

// ====================
// class SvxChartOptions
// ====================

SvxChartOptions::SvxChartOptions() :
	::utl::ConfigItem( rtl::OUString::createFromAscii( "Office.Chart" )),
	mbIsInitialized( sal_False )
{
	maPropertyNames.realloc( 1 );
	maPropertyNames[ 0 ] = ::rtl::OUString::createFromAscii( "DefaultColor/Series" );
}

SvxChartOptions::~SvxChartOptions()
{
}

const SvxChartColorTable& SvxChartOptions::GetDefaultColors()
{
	if ( !mbIsInitialized )
		mbIsInitialized = RetrieveOptions();
	return maDefColors;
}

void SvxChartOptions::SetDefaultColors( const SvxChartColorTable& aCol )
{
	maDefColors = aCol;
	SetModified();
}

sal_Bool SvxChartOptions::RetrieveOptions()
{
	// get sequence containing all properties

	uno::Sequence< ::rtl::OUString > aNames = GetPropertyNames();
	uno::Sequence< uno::Any > aProperties( aNames.getLength());
	aProperties = GetProperties( aNames );

	if( aProperties.getLength() == aNames.getLength())
	{
		// 1. default colors for series
        maDefColors.clear();
		uno::Sequence< sal_Int64 > aColorSeq;
		aProperties[ 0 ] >>= aColorSeq;

		sal_Int32 nCount = aColorSeq.getLength();
		Color aCol;

		// create strings for entry names
		String aResName( CUI_RES( RID_SVXSTR_DIAGRAM_ROW ) );
		String aPrefix, aPostfix, aName;
		xub_StrLen nPos = aResName.SearchAscii( "$(ROW)" );
		if( nPos != STRING_NOTFOUND )
		{
			aPrefix = String( aResName, 0, nPos );
			aPostfix = String( aResName, nPos + sizeof( "$(ROW)" ) - 1, STRING_LEN );
		}
		else
			aPrefix = aResName;

		// set color values
		for( sal_Int32 i=0; i < nCount; i++ )
		{
			aCol.SetColor( SAL_STATIC_CAST( ColorData, aColorSeq[ i ] ));

			aName = aPrefix;
			aName.Append( String::CreateFromInt32( i + 1 ));
			aName.Append( aPostfix );

            maDefColors.append( XColorEntry( aCol, aName ));
		}
		return sal_True;
	}
	return sal_False;
}

void SvxChartOptions::Commit()
{
	uno::Sequence< ::rtl::OUString > aNames = GetPropertyNames();
	uno::Sequence< uno::Any > aValues( aNames.getLength());

	if( aValues.getLength() >= 1 )
	{
		// 1. default colors for series
		// convert list to sequence
		const size_t nCount = maDefColors.size();
		uno::Sequence< sal_Int64 > aColors( nCount );
		for( size_t i=0; i < nCount; i++ )
		{
			ColorData aData = maDefColors.getColorData( i );
			aColors[ i ] = aData;
		}

		aValues[ 0 ] <<= aColors;
	}

	PutProperties( aNames, aValues );
}

void SvxChartOptions::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

// --------------------
// class SvxChartColorTableItem
// --------------------

SvxChartColorTableItem::SvxChartColorTableItem( sal_uInt16 nWhich_, const SvxChartColorTable& aTable ) :
	SfxPoolItem( nWhich_ ),
	m_aColorTable( aTable )
{
}

SvxChartColorTableItem::SvxChartColorTableItem( const SvxChartColorTableItem& rOther ) :
	SfxPoolItem( rOther ),
	m_aColorTable( rOther.m_aColorTable )
{
}

SfxPoolItem* __EXPORT SvxChartColorTableItem::Clone( SfxItemPool * ) const
{
	return new SvxChartColorTableItem( *this );
}

int __EXPORT SvxChartColorTableItem::operator==( const SfxPoolItem& rAttr ) const
{
	DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "SvxChartColorTableItem::operator== : types differ" );

    const SvxChartColorTableItem * rCTItem( dynamic_cast< const SvxChartColorTableItem * >( & rAttr ));
    if( rCTItem )
    {
        return (this->m_aColorTable == rCTItem->GetColorTable());
    }

    return 0;
}

void __EXPORT SvxChartColorTableItem::SetOptions( SvxChartOptions* pOpts ) const
{
	if ( pOpts )
		pOpts->SetDefaultColors( m_aColorTable );
}


SvxChartColorTable & SvxChartColorTableItem::GetColorTable()
{
    return m_aColorTable;
}

const SvxChartColorTable & SvxChartColorTableItem::GetColorTable() const
{
    return m_aColorTable;
}

void SvxChartColorTableItem::ReplaceColorByIndex( size_t _nIndex, const XColorEntry & _rEntry )
{
    m_aColorTable.replace( _nIndex, _rEntry );
}
