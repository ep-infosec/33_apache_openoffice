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
#include "precompiled_svx.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;

sal_Unicode const pszExtColor[]	 = {'s','o','c'};

static char const aChckColor[]  = { 0x04, 0x00, 'S','O','C','L'};	// < 5.2
static char const aChckColor0[] = { 0x04, 0x00, 'S','O','C','0'};	// = 5.2
static char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };		// = 6.0

/*************************************************************************
|*
|* XColorList::XColorList()
|*
*************************************************************************/

static XColorListSharedPtr aStaticGlobalColorList;

XColorList::XColorList( const String& rPath ) :
				XPropertyList( rPath )
{
}

/************************************************************************/

XColorList::~XColorList()
{
}

/************************************************************************/

XColorEntry* XColorList::Replace(XColorEntry* pEntry, long nIndex )
{
	return (XColorEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XColorEntry* XColorList::Remove(long nIndex)
{
	return (XColorEntry*) XPropertyList::Remove(nIndex);
}

/************************************************************************/

XColorEntry* XColorList::GetColor(long nIndex) const
{
	return (XColorEntry*) XPropertyList::Get(nIndex);
}

/************************************************************************/

bool XColorList::Load()
{
	if( mbListDirty )
	{
		mbListDirty = false;

		INetURLObject aURL( maPath );

		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
		{
			OSL_ENSURE( !maPath.Len(), "invalid URL" );
			return false;
		}

		aURL.Append( maName );

		if( !aURL.getExtension().getLength() )
			aURL.setExtension( rtl::OUString( pszExtColor, 3 ) );

		uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
	}

	return false;
}

/************************************************************************/

bool XColorList::Save()
{
	INetURLObject aURL( maPath );

	if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
	{
		OSL_ENSURE( !maPath.Len(), "invalid URL" );
		return false;
	}

	aURL.Append( maName );

	if( !aURL.getExtension().getLength() )
		aURL.setExtension( rtl::OUString( pszExtColor, 3 ) );

	uno::Reference< container::XNameContainer > xTable( SvxUnoXColorTable_createInstance( this ), uno::UNO_QUERY );
	return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

/************************************************************************/

bool XColorList::Create()
{
    sal_uInt32 a(0);
    sal_uInt32 b(0);

    // <!-- Gray palette from white to black -->
    XubString aStrGrey( SVX_RESSTR( RID_SVXSTR_COLOR_GREY ) );

    Insert( new XColorEntry( Color( 0xff, 0xff, 0xff ), SVX_RESSTR( RID_SVXSTR_COLOR_WHITE ) ) );
    aStrGrey.AppendAscii(" 1"); Insert( new XColorEntry( Color( 0xee, 0xee, 0xee ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('2')); Insert( new XColorEntry( Color( 0xdd, 0xdd, 0xdd ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('3')); Insert( new XColorEntry( Color( 0xcc, 0xcc, 0xcc ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('4')); Insert( new XColorEntry( Color( 0xb2, 0xb2, 0xb2 ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('5')); Insert( new XColorEntry( Color( 0x99, 0x99, 0x99 ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('6')); Insert( new XColorEntry( Color( 0x80, 0x80, 0x80 ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('7')); Insert( new XColorEntry( Color( 0x66, 0x66, 0x66 ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('8')); Insert( new XColorEntry( Color( 0x33, 0x33, 0x33 ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('9')); Insert( new XColorEntry( Color( 0x1c, 0x1c, 0x1c  ), aStrGrey ) );
    aStrGrey.SetChar(aStrGrey.Len() - 1, sal_Unicode('1')); aStrGrey.AppendAscii("0"); Insert( new XColorEntry( Color( 0x11, 0x11, 0x11 ), aStrGrey ) );
    Insert( new XColorEntry( Color( 0x00, 0x00, 0x00 ), SVX_RESSTR( RID_SVXSTR_COLOR_BLACK ) ) );

    // <!-- Base colors step 0 to 10 -->
    const sal_uInt32 nNumColorsInGroup(12);
    const sal_uInt32 nNumGroups(11);
    XubString aStrCol[nNumColorsInGroup];
    const sal_uInt32 aStdCol[nNumColorsInGroup * nNumGroups] = {
        0xffff99, 0xff6600, 0xff3333, 0xff00cc, 0xff33ff, 0x9900ff, 0x6666ff, 0x00ccff, 0x66ffff, 0x33ff99, 0x99ff66, 0xccff00, 
        0xffff66, 0xffcc00, 0xff9999, 0xff66cc, 0xff99ff, 0xcc66ff, 0x9999ff, 0x9999ff, 0x99ffff, 0x66ff99, 0x99ff99, 0xccff66,
        0xffff00, 0xff9900, 0xff6666, 0xff3399, 0xff66ff, 0x9933ff, 0x3333ff, 0x3399ff, 0x00ffff, 0x00ff66, 0x66ff66, 0x99ff33, 
        0xcc9900, 0xff3300, 0xff0000, 0xff0066, 0xff00ff, 0x6600ff, 0x0000ff, 0x0066ff, 0x00cccc, 0x00cc33, 0x00cc00, 0x66ff00,
        0x996600, 0xcc3300, 0xcc0000, 0xcc0066, 0xcc00cc, 0x6600cc, 0x0000cc, 0x0066cc, 0x009999, 0x009933, 0x009900, 0x66cc00, 
        0x663300, 0x801900, 0x990000, 0x990066, 0x990099, 0x330099, 0x000099, 0x006699, 0x006666, 0x007826, 0x006600, 0x669900,
        0x333300, 0x461900, 0x330000, 0x330033, 0x660066, 0x000033, 0x000066, 0x000080, 0x003333, 0x00331a, 0x003300, 0x193300, 
        0x666633, 0x661900, 0x663333, 0x660033, 0x663366, 0x330066, 0x333366, 0x003366, 0x336666, 0x006633, 0x336633, 0x336600,
        0x999966, 0x996633, 0x996666, 0x993366, 0x996699, 0x663399, 0x666699, 0x336699, 0x669999, 0x339966, 0x669966, 0x669933, 
        0xcccc99, 0xcc9966, 0xcc9999, 0xcc6699, 0xcc99cc, 0x9966cc, 0x9999cc, 0x6699cc, 0x99cccc, 0x66cc99, 0x99cc99, 0x99cc66,
        0xffffcc, 0xffcc99, 0xffcccc, 0xff99cc, 0xffccff, 0xcc99ff, 0xccccff, 0x99ccff, 0xccffff, 0x99ffcc, 0xccffcc, 0xccff99 };

    for(a = 0; a < nNumGroups; a++)
    {
        switch(a)
        {
            case 0:
            {
                aStrCol[0] = SVX_RESSTR(RID_SVXSTR_COLOR_YELLOW);
                aStrCol[1] = SVX_RESSTR(RID_SVXSTR_COLOR_ORANGE);
                aStrCol[2] = SVX_RESSTR(RID_SVXSTR_COLOR_RED);
                aStrCol[3] = SVX_RESSTR(RID_SVXSTR_COLOR_PINK);
                aStrCol[4] = SVX_RESSTR(RID_SVXSTR_COLOR_MAGENTA);
                aStrCol[5] = SVX_RESSTR(RID_SVXSTR_COLOR_PURPLE);
                aStrCol[6] = SVX_RESSTR(RID_SVXSTR_COLOR_BLUE);
                aStrCol[7] = SVX_RESSTR(RID_SVXSTR_COLOR_SKYBLUE);
                aStrCol[8] = SVX_RESSTR(RID_SVXSTR_COLOR_CYAN);
                aStrCol[9] = SVX_RESSTR(RID_SVXSTR_COLOR_TURQUOISE);
                aStrCol[10] = SVX_RESSTR(RID_SVXSTR_COLOR_GREEN);
                aStrCol[11] = SVX_RESSTR(RID_SVXSTR_COLOR_YELLOWGREEN);
                break;
            }
            case 1:
            {
                for(b = 0; b < nNumColorsInGroup; b++)
                {
                    aStrCol[b].AppendAscii(" 1");
                }
                break;
            }
            case 10:
            {
                for(b = 0; b < nNumColorsInGroup; b++)
                {
                    aStrCol[b].SetChar(aStrCol[b].Len() - 1, sal_Unicode('1'));
                    aStrCol[b].AppendAscii("0");
                }
                break;
            }
            default: // 2 .. 9, 11
            {
                for(b = 0; b < nNumColorsInGroup; b++)
                {
                    aStrCol[b].SetChar(aStrCol[b].Len() - 1, sal_Unicode('0') + sal_Unicode(a % 10));
                }
                break;
            }
        }

        const sal_uInt32 nOffset(a * nNumColorsInGroup);

        for(b = 0; b < nNumColorsInGroup; b++)
        {
            Insert( new XColorEntry( aStdCol[nOffset + b], aStrCol[b] ) );
        }
    }

    // <!-- use some 'nice' colors from original palette -->
    Insert( new XColorEntry( Color( 0xe6, 0xe6, 0xff ), SVX_RESSTR( RID_SVXSTR_COLOR_BLUEGREY) ) );
    Insert( new XColorEntry( Color( 0xCF, 0xE7, 0xF5  ), SVX_RESSTR( RID_SVXSTR_COLOR_BLUE_CLASSIC ) ) );

    // <!-- add 'pale' colors from original palette -->
    Insert( new XColorEntry( Color( 0x99, 0x99, 0xff ), SVX_RESSTR( RID_SVXSTR_COLOR_VIOLET ) ) );
    Insert( new XColorEntry( Color( 0x99, 0x33, 0x66 ), SVX_RESSTR( RID_SVXSTR_COLOR_BORDEAUX ) ) );
    Insert( new XColorEntry( Color( 0xff, 0xff, 0xcc ), SVX_RESSTR( RID_SVXSTR_COLOR_PALE_YELLOW ) ) );
    Insert( new XColorEntry( Color( 0xcc, 0xff, 0xff ), SVX_RESSTR( RID_SVXSTR_COLOR_PALE_GREEN ) ) );
    Insert( new XColorEntry( Color( 0x66, 0x00, 0x66 ), SVX_RESSTR( RID_SVXSTR_COLOR_DARKVIOLET ) ) );
    Insert( new XColorEntry( Color( 0xff, 0x80, 0x80 ), SVX_RESSTR( RID_SVXSTR_COLOR_SALMON ) ) );
    Insert( new XColorEntry( Color( 0x00, 0x66, 0xcc ), SVX_RESSTR( RID_SVXSTR_COLOR_SEABLUE ) ) );

    // <!-- add Chart colors from original palette (also 12, coincidence?) -->
    XubString aStrChart( SVX_RESSTR( RID_SVXSTR_COLOR_CHART ) );
    aStrChart.AppendAscii(" 1");
    Insert( new XColorEntry( Color( 0x00, 0x45, 0x86 ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 0xff, 0x42, 0x0e ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('3'));
    Insert( new XColorEntry( Color( 0xff, 0xd3, 0x20 ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('4'));
    Insert( new XColorEntry( Color( 0x57, 0x9d, 0x1c ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('5'));
    Insert( new XColorEntry( Color( 0x7e, 0x00, 0x21 ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('6'));
    Insert( new XColorEntry( Color( 0x83, 0xca, 0xff ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('7'));
    Insert( new XColorEntry( Color( 0x31, 0x40, 0x04 ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('8'));
    Insert( new XColorEntry( Color( 0xae, 0xcf, 0x00 ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('9'));
    Insert( new XColorEntry( Color( 0x4b, 0x1f, 0x6f ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('1'));
    aStrChart.AppendAscii("0");
    Insert( new XColorEntry( Color( 0xff, 0x95, 0x0e ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('1'));
    Insert( new XColorEntry( Color( 0xc5, 0x00, 0x0b ), aStrChart ) );
    aStrChart.SetChar(aStrChart.Len() - 1, sal_Unicode('2'));
    Insert( new XColorEntry( Color( 0x00, 0x84, 0xd1 ), aStrChart ) );

    return(165 == Count());
}

/************************************************************************/

Bitmap XColorList::CreateBitmapForUI( long /*nIndex*/ )
{
	return Bitmap();
}

/************************************************************************/

XColorListSharedPtr XColorList::GetStdColorList()
{
    if ( !aStaticGlobalColorList.get() )
    {
        aStaticGlobalColorList = XPropertyListFactory::CreateSharedXColorList(SvtPathOptions().GetPalettePath());
    }

    return aStaticGlobalColorList;
}

// eof
