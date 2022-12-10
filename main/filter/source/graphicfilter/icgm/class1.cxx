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
#include "precompiled_filter.hxx"

#include <main.hxx>

// ---------------------------------------------------------------

void CGM::ImplDoClass1()
{
	long	nInteger, nI0, nI1;
	sal_uInt32	nUInteger;

	switch ( mnElementID )
	{
		case 0x01 : ComOut( CGM_LEVEL1, "Metafile Version" )
			pElement->nMetaFileVersion = ImplGetI( pElement->nIntegerPrecision );
		break;
		case 0x02 : ComOut( CGM_LEVEL1, "Metafile Description" ) break;
		case 0x03 : ComOut( CGM_LEVEL1, "VDC Type" )
		{
			nUInteger = ImplGetUI16();
			switch( nUInteger )
			{
				case 0 : pElement->eVDCType = VDC_INTEGER; break;
				case 1 : pElement->eVDCType = VDC_REAL; break;
				default: mbStatus = sal_False; break;
			}
		}
		break;
		case 0x04 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Integer Precision" )
		{
			nInteger = ImplGetI( pElement->nIntegerPrecision );
			switch ( nInteger )
			{
				case 32 :
				case 24 :
				case 16 :
				case 8 : pElement->nIntegerPrecision = nInteger >> 3; break;
				default : mbStatus = sal_False; break;
			}
		}
		break;
		case 0x05 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Real Precision" )
		{
			nUInteger = ImplGetUI16( 4 );
			nI0 = ImplGetI( pElement->nIntegerPrecision );	// exponent
			nI1 = ImplGetI( pElement->nIntegerPrecision );	// mantisse
			switch( nUInteger )
			{
				case 0 :
					pElement->eRealPrecision = RP_FLOAT;
					switch ( nI0 )
					{
						case 9 :
							if ( nI1 != 23 )
								mbStatus = sal_False;
							pElement->nRealSize = 4;
							break;
						case 12 :
							if ( nI1 != 52 )
								mbStatus =sal_False;
							pElement->nRealSize = 8;
							break;
						default:
							mbStatus = sal_False;
							break;
					}
					break;
				case 1 :
					pElement->eRealPrecision = RP_FIXED;
					if ( nI0 != nI1 )
						mbStatus = sal_False;
					if ( nI0 == 16 )
						pElement->nRealSize = 4;
					else if ( nI0 == 32 )
						pElement->nRealSize = 8;
					else
						mbStatus = sal_False;
					break;
				default :
					mbStatus = sal_False; break;
			}
		}
		break;
		case 0x06 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Index Precision" )
		{
			nInteger = ImplGetI( pElement->nIntegerPrecision );
			switch ( nInteger )
			{
				case 32 :
				case 24 :
				case 16 :
				case 8 : pElement->nIndexPrecision = nInteger >> 3; break;
				default : mbStatus = sal_False; break;
			}
		}
		break;
		case 0x07 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Color Precision" )
		{
			nInteger = ImplGetI( pElement->nIntegerPrecision );
			switch ( nInteger )
			{
				case 32 :
				case 24 :
				case 16 :
				case 8 : pElement->nColorPrecision = nInteger >> 3; break;
				default : mbStatus = sal_False; break;
			}
		}
		break;
		case 0x08 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Color Index Precision" )
		{
			nInteger = ImplGetI( pElement->nIntegerPrecision );
			switch ( nInteger )
			{
				case 32 :
				case 24 :
				case 16 :
				case 8 : pElement->nColorIndexPrecision = nInteger >> 3; break;
				default : mbStatus = sal_False; break;
			}
		}
		break;
		case 0x09 : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Maximum Colour Index" )
		{
			pElement->nColorMaximumIndex = ImplGetUI( pElement->nColorIndexPrecision );
			if ( ( pElement->nColorMaximumIndex > 256 /*255*/ ) || ( pElement->nColorMaximumIndex == 0 ) )
				mbStatus = sal_False;
		}
		break;
		case 0x0a : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Color Value Extent" )
		{
			if ( pElement->eColorModel == CM_RGB )
				nI1 = 6;
			else
			{
				nI1 = 8;
				mbStatus = sal_False;								// CMYK is not supported
			}
			for ( nI0 = 0; nI0 < nI1; nI0++ )
			{
				pElement->nColorValueExtent[ nI0 ] = (sal_uInt8)ImplGetUI( pElement->nColorPrecision );
			}
		}
		break;
		case 0x0b : ComOut( CGM_LEVEL1, "MetaFile Element List" ) break;
		case 0x0c : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "MetaFile Default Replacement" )
		{
			if ( mnElementSize > 1 )
			{
				sal_Int8* pBuf = new sal_Int8[ mnElementSize ];
				if ( pBuf )
				{
					memcpy( pBuf, mpSource, mnElementSize );
					maDefRepList.Insert( pBuf, LIST_APPEND );
					maDefRepSizeList.Insert( (void*)mnElementSize, LIST_APPEND );
				}
			}
			mnParaSize = mnElementSize;
		}
		break;
		case 0x0d : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Font List" )
		{
			while ( mnParaSize < mnElementSize )
			{
				sal_uInt32 nSize;
				nSize = ImplGetUI( 1 );
				pElement->aFontList.InsertName( mpSource + mnParaSize, nSize );
				mnParaSize += nSize;
			}
		}
		break;
		case 0x0e : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Character Set List" )
		{
			while ( mnParaSize < mnElementSize )
			{
				sal_uInt32 nCharSetType;
				sal_uInt32 nSize;
				nCharSetType = ImplGetUI16();
				nSize = ImplGetUI( 1 );
				pElement->aFontList.InsertCharSet( (CharSetType)nCharSetType, mpSource + mnParaSize, nSize );
				mnParaSize += nSize;
			}
		}
		break;
		case 0x0f : ComOut( CGM_LEVEL1 | CGM_DRAWING_PLUS_CONTROL_SET, "Character Coding Announcer" )
			pElement->eCharacterCodingA = (CharacterCodingA)ImplGetUI16();
		break;
		case 0x10 : ComOut( CGM_LEVEL2, "Name Precision" ) break;					// NS
		case 0x11 : ComOut( CGM_LEVEL2, "Maximum VDC Extent" ) break;				// NS
		case 0x12 : ComOut( CGM_LEVEL2, "Segment Priority Extent" ) break;			// NS
		case 0x13 : ComOut( CGM_LEVEL3, "Color Model" ) break;						// NS
		case 0x14 : ComOut( CGM_LEVEL3, "Color Calibration" ) break;				// NS
		case 0x15 : ComOut( CGM_LEVEL3, "Font Properties" ) break;					// NS
		case 0x16 : ComOut( CGM_LEVEL3, "Glyph Mapping" ) break;					// NS
		case 0x17 : ComOut( CGM_LEVEL3, "Symbol Library List" ) break;				// NS
		case 0xfc : ComOut( CGM_GDSF_ONLY, "Inquire Function Support" ) break;
		case 0xfa : ComOut( CGM_GDSF_ONLY, "End Metafile Defaults Replacement" ) break;
		case 0xf8 : ComOut( CGM_GDSF_ONLY, "Set Color Value Desc Extent" ) break;
		default: ComOut( CGM_UNKNOWN_COMMAND, "" ) break;
	}
};


