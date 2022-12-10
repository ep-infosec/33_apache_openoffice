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
#include "precompiled_basic.hxx"
#include <tools/errcode.hxx>
#include <basic/sbx.hxx>
#include "sbxconv.hxx"

// AB 29.10.99 Unicode
#ifndef _USE_NO_NAMESPACE
using namespace rtl;
#endif

xub_Unicode ImpGetChar( const SbxValues* p )
{
	SbxValues aTmp;
	xub_Unicode nRes = 0;
start:
	switch( +p->eType )
	{
		case SbxNULL:
			SbxBase::SetError( SbxERR_CONVERSION );
		case SbxEMPTY:
			nRes = 0; break;
		case SbxCHAR:
			nRes = p->nChar; break;
		case SbxBYTE:
			nRes = (xub_Unicode) p->nByte;
			break;
		case SbxINTEGER:
		case SbxBOOL:
			if( p->nInteger < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
			}
			else
				nRes = (xub_Unicode) p->nInteger;
			break;
		case SbxERROR:
		case SbxUSHORT:
			nRes = (xub_Unicode) p->nUShort;
			break;
		case SbxLONG:
			if( p->nLong > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else if( p->nLong < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
			}
			else
				nRes = (xub_Unicode) p->nLong;
			break;
		case SbxULONG:
			if( p->nULong > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else
				nRes = (xub_Unicode) p->nULong;
			break;
		case SbxSALINT64:
			if( p->nInt64 > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else if( p->nInt64 < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
			}
			else
				nRes = (xub_Unicode) p->nInt64;
			break;
		case SbxSALUINT64:
			if( p->uInt64 > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else
				nRes = (xub_Unicode) p->uInt64;
			break;
		case SbxSINGLE:
			if( p->nSingle > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else if( p->nSingle < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
			}
			else
				nRes = (xub_Unicode) ImpRound( p->nSingle );
			break;
		case SbxDATE:
		case SbxDOUBLE:
		case SbxLONG64:
		case SbxULONG64:
		case SbxCURRENCY:
		case SbxDECIMAL:
		case SbxBYREF | SbxDECIMAL:
			{
			double dVal;
			if( p->eType ==	SbxCURRENCY )
				dVal = ImpCurrencyToDouble( p->nLong64 );
			else if( p->eType == SbxLONG64 )
				dVal = ImpINT64ToDouble( p->nLong64 );
			else if( p->eType == SbxULONG64 )
				dVal = ImpUINT64ToDouble( p->nULong64 );
			else if( p->eType == SbxDECIMAL )
			{
				dVal = 0.0;
				if( p->pDecimal )
					p->pDecimal->getDouble( dVal );
			}
			else
				dVal = p->nDouble;

			if( dVal > SbxMAXCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
			}
			else if( dVal < SbxMINCHAR )
			{
				SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
			}
			else
				nRes = (sal_uInt8) ImpRound( dVal );
			break;
			}
		case SbxBYREF | SbxSTRING:
		case SbxSTRING:
		case SbxLPSTR:
            if ( p->pOUString )
            {
                double d;
                SbxDataType t;
                if( ImpScan( *p->pOUString, d, t, NULL ) != SbxERR_OK )
                    nRes = 0;
                else if( d > SbxMAXCHAR )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMAXCHAR;
                }
                else if( d < SbxMINCHAR )
                {
                    SbxBase::SetError( SbxERR_OVERFLOW ); nRes = SbxMINCHAR;
                }
                else
                    nRes = (xub_Unicode) ImpRound( d );
            }
			break;
		case SbxOBJECT:
		{
			SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
			if( pVal )
				nRes = pVal->GetChar();
			else
			{
				SbxBase::SetError( SbxERR_NO_OBJECT ); nRes = 0;
			}
			break;
		}

		case SbxBYREF | SbxCHAR:
			nRes = *p->pChar; break;
		// ab hier wird getestet
		case SbxBYREF | SbxBYTE:
			aTmp.nByte = *p->pByte; goto ref;
		case SbxBYREF | SbxINTEGER:
		case SbxBYREF | SbxBOOL:
			aTmp.nInteger = *p->pInteger; goto ref;
		case SbxBYREF | SbxLONG:
			aTmp.nLong = *p->pLong; goto ref;
		case SbxBYREF | SbxULONG:
			aTmp.nULong = *p->pULong; goto ref;
		case SbxBYREF | SbxERROR:
		case SbxBYREF | SbxUSHORT:
			aTmp.nUShort = *p->pUShort; goto ref;
		case SbxBYREF | SbxSINGLE:
			aTmp.nSingle = *p->pSingle; goto ref;
		case SbxBYREF | SbxDATE:
		case SbxBYREF | SbxDOUBLE:
			aTmp.nDouble = *p->pDouble; goto ref;
		case SbxBYREF | SbxULONG64:
			aTmp.nULong64 = *p->pULong64; goto ref;
		case SbxBYREF | SbxLONG64:
		case SbxBYREF | SbxCURRENCY:
			aTmp.nLong64 = *p->pLong64; goto ref;
		case SbxBYREF | SbxSALINT64:
			aTmp.nInt64 = *p->pnInt64; goto ref;
		case SbxBYREF | SbxSALUINT64:
			aTmp.uInt64 = *p->puInt64; goto ref;
		ref:
			aTmp.eType = SbxDataType( p->eType & 0x0FFF );
			p = &aTmp; goto start;

		default:
			SbxBase::SetError( SbxERR_CONVERSION ); nRes = 0;
	}
	return nRes;
}

void ImpPutChar( SbxValues* p, xub_Unicode n )
{
	SbxValues aTmp;
start:
	switch( +p->eType )
	{
		case SbxCHAR:
			p->nChar = n; break;
		case SbxINTEGER:
		case SbxBOOL:
			p->nInteger = n; break;
		case SbxLONG:
			p->nLong = n; break;
		case SbxSINGLE:
			p->nSingle = n; break;
		case SbxDATE:
		case SbxDOUBLE:
			p->nDouble = n; break;
		case SbxSALINT64:
			p->nInt64 = n; break;
		case SbxSALUINT64:
			p->uInt64 = n; break;
		case SbxULONG64:
			p->nULong64 = ImpDoubleToUINT64( (double)n ); break;
		case SbxLONG64:
			p->nLong64 = ImpDoubleToINT64( (double)n ); break;
		case SbxCURRENCY:
			p->nLong64 = ImpDoubleToCurrency( (double)n ); break;
		case SbxBYREF | SbxDECIMAL:
			ImpCreateDecimal( p )->setChar( n );
			break;

		// ab hier wird getestet
		case SbxBYTE:
			aTmp.pByte = &p->nByte; goto direct;
		case SbxULONG:
			aTmp.pULong = &p->nULong; goto direct;
		case SbxERROR:
		case SbxUSHORT:
			aTmp.pUShort = &p->nUShort; goto direct;
		direct:
			aTmp.eType = SbxDataType( p->eType | SbxBYREF );
			p = &aTmp; goto start;

		case SbxBYREF | SbxSTRING:
		case SbxSTRING:
		case SbxLPSTR:
            if ( !p->pOUString )
                p->pOUString = new ::rtl::OUString( n );
            else
                *p->pOUString = ::rtl::OUString( n );
			break;
		case SbxOBJECT:
		{
			SbxValue* pVal = PTR_CAST(SbxValue,p->pObj);
			if( pVal )
				pVal->PutChar( n );
			else
				SbxBase::SetError( SbxERR_NO_OBJECT );
			break;
		}
		case SbxBYREF | SbxCHAR:
			*p->pChar = n; break;
		case SbxBYREF | SbxBYTE:
			*p->pByte = (sal_uInt8) n; break;
		case SbxBYREF | SbxINTEGER:
		case SbxBYREF | SbxBOOL:
			*p->pInteger = n; break;
		case SbxBYREF | SbxERROR:
		case SbxBYREF | SbxUSHORT:
			*p->pUShort = (sal_uInt16) n; break;
		case SbxBYREF | SbxLONG:
			*p->pLong = (sal_Int32) n; break;
		case SbxBYREF | SbxULONG:
			*p->pULong = (sal_uInt32) n; break;
		case SbxBYREF | SbxSINGLE:
			*p->pSingle = (float) n; break;
		case SbxBYREF | SbxDATE:
		case SbxBYREF | SbxDOUBLE:
			*p->pDouble = (double) n; break;
		case SbxBYREF | SbxSALINT64:
			*p->pnInt64 = n; break;
		case SbxBYREF | SbxSALUINT64:
			*p->puInt64 = n; break;
		case SbxBYREF | SbxULONG64:
			*p->pULong64 = ImpDoubleToUINT64( (double)n ); break;
		case SbxBYREF | SbxLONG64:
			*p->pLong64 = ImpDoubleToINT64( (double)n ); break;
		case SbxBYREF | SbxCURRENCY:
			*p->pLong64 = ImpDoubleToCurrency( (double)n ); break;

		default:
			SbxBase::SetError( SbxERR_CONVERSION );
	}
}


