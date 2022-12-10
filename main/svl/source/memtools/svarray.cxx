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
#include "precompiled_svl.hxx"

#define _SVARRAY_CXX

#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGSSORT
#define _SVSTDARR_USHORTS
#define _SVSTDARR_LONGS
#define _SVSTDARR_LONGSSORT
#define _SVSTDARR_SHORTS
#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSSORT
#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSISORT
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_USHORTSSORT

#define _SVSTDARR_BYTESTRINGS
#define _SVSTDARR_BYTESTRINGSDTOR
#define _SVSTDARR_BYTESTRINGSSORT
#define _SVSTDARR_BYTESTRINGSSORTDTOR
#define _SVSTDARR_BYTESTRINGSISORT
#define _SVSTDARR_BYTESTRINGSISORTDTOR

#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_XUB_STRLENSORT

#include <svl/svstdarr.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

SV_IMPL_VARARR(SvPtrarr,VoidPtr)

sal_uInt16 SvPtrarr::GetPos( const VoidPtr& aElement ) const
{	sal_uInt16 n;
	for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
	return ( n >= nA ? USHRT_MAX : n );
}

SV_IMPL_VARARR( SvULongs, sal_uLong )
SV_IMPL_VARARR( SvUShorts, sal_uInt16 )
SV_IMPL_VARARR( SvLongs, long)

SV_IMPL_VARARR_SORT( SvULongsSort, sal_uLong )
SV_IMPL_VARARR_SORT( SvLongsSort, long )

SV_IMPL_PTRARR( SvStrings, StringPtr )
SV_IMPL_PTRARR( SvStringsDtor, StringPtr )
SV_IMPL_OP_PTRARR_SORT( SvStringsSort, StringPtr )
SV_IMPL_OP_PTRARR_SORT( SvStringsSortDtor, StringPtr )

SV_IMPL_PTRARR( SvByteStrings, ByteStringPtr )
SV_IMPL_PTRARR( SvByteStringsDtor, ByteStringPtr )
SV_IMPL_OP_PTRARR_SORT( SvByteStringsSort, ByteStringPtr )
SV_IMPL_OP_PTRARR_SORT( SvByteStringsSortDtor, ByteStringPtr )



// ---------------- strings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvStringsISort, StringPtr )
void SvStringsISort::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
	if( nL )
	{
		DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
		for( sal_uInt16 n=nP; n < nP + nL; n++ )
			delete *((StringPtr*)pData+n);
		SvPtrarr::Remove( nP, nL );
	}
}
sal_Bool SvStringsISort::Seek_Entry( const StringPtr aE, sal_uInt16* pP ) const
{
	sal_uInt16 nO  = SvStringsISort_SAR::Count(),
			nM,
			nU = 0;
	if( nO > 0 )
	{
		nO--;
		while( nU <= nO )
		{
			nM = nU + ( nO - nU ) / 2;
			StringCompare eCmp = (*((StringPtr*)pData + nM))->
										CompareIgnoreCaseToAscii( *(aE) );
			if( COMPARE_EQUAL == eCmp )
			{
				if( pP ) *pP = nM;
				return sal_True;
			}
			else if( COMPARE_LESS == eCmp )
				nU = nM + 1;
			else if( nM == 0 )
			{
				if( pP ) *pP = nU;
				return sal_False;
			}
			else
				nO = nM - 1;
		}
	}
	if( pP ) *pP = nU;
	return sal_False;
}

// ---------------- strings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvStringsISortDtor, StringPtr )
void SvStringsISortDtor::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
	if( nL )
	{
		DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
		for( sal_uInt16 n=nP; n < nP + nL; n++ )
			delete *((StringPtr*)pData+n);
		SvPtrarr::Remove( nP, nL );
	}
}
sal_Bool SvStringsISortDtor::Seek_Entry( const StringPtr aE, sal_uInt16* pP ) const
{
	sal_uInt16 nO  = SvStringsISortDtor_SAR::Count(),
			nM,
			nU = 0;
	if( nO > 0 )
	{
		nO--;
		while( nU <= nO )
		{
			nM = nU + ( nO - nU ) / 2;
			StringCompare eCmp = (*((StringPtr*)pData + nM))->
									CompareIgnoreCaseToAscii( *(aE) );
			if( COMPARE_EQUAL == eCmp )
			{
				if( pP ) *pP = nM;
				return sal_True;
			}
			else if( COMPARE_LESS == eCmp )
				nU = nM + 1;
			else if( nM == 0 )
			{
				if( pP ) *pP = nU;
				return sal_False;
			}
			else
				nO = nM - 1;
		}
	}
	if( pP ) *pP = nU;
	return sal_False;
}

// ---------------- Ushorts -------------------------------------

/* SortArray fuer UShorts */
sal_Bool SvUShortsSort::Seek_Entry( const sal_uInt16 aE, sal_uInt16* pP ) const
{
	sal_uInt16 nO  = SvUShorts::Count(),
			nM,
			nU = 0;
	if( nO > 0 )
	{
		nO--;
		while( nU <= nO )
		{
			nM = nU + ( nO - nU ) / 2;
			if( *(pData + nM) == aE )
			{
				if( pP ) *pP = nM;
				return sal_True;
			}
			else if( *(pData + nM) < aE )
				nU = nM + 1;
			else if( nM == 0 )
			{
				if( pP ) *pP = nU;
				return sal_False;
			}
			else
				nO = nM - 1;
		}
	}
	if( pP ) *pP = nU;
	return sal_False;
}

void SvUShortsSort::Insert( const SvUShortsSort * pI, sal_uInt16 nS, sal_uInt16 nE )
{
	if( USHRT_MAX == nE )
		nE = pI->Count();
	sal_uInt16 nP;
	const sal_uInt16 * pIArr = pI->GetData();
	for( ; nS < nE; ++nS )
	{
		if( ! Seek_Entry( *(pIArr+nS), &nP) )
				SvUShorts::Insert( *(pIArr+nS), nP );
		if( ++nP >= Count() )
		{
			SvUShorts::Insert( pI, nP, nS+1, nE );
			nS = nE;
		}
	}
}

sal_Bool SvUShortsSort::Insert( const sal_uInt16 aE )
{
	sal_uInt16 nP;
	sal_Bool bExist = Seek_Entry( aE, &nP );
	if( !bExist )
		SvUShorts::Insert( aE, nP );
	return !bExist;
}

sal_Bool SvUShortsSort::Insert( const sal_uInt16 aE, sal_uInt16& rP )
{
	sal_Bool bExist = Seek_Entry( aE, &rP );
	if( !bExist )
		SvUShorts::Insert( aE, rP );
	return !bExist;
}

void SvUShortsSort::Insert( const sal_uInt16* pE, sal_uInt16 nL)
{
	sal_uInt16 nP;
	for( sal_uInt16 n = 0; n < nL; ++n )
		if( ! Seek_Entry( *(pE+n), &nP ))
			SvUShorts::Insert( *(pE+n), nP );
}

// remove ab Pos
void SvUShortsSort::RemoveAt( const sal_uInt16 nP, sal_uInt16 nL )
{
	if( nL )
		SvUShorts::Remove( nP, nL);
}

// remove ab dem Eintrag
void SvUShortsSort::Remove( const sal_uInt16 aE, sal_uInt16 nL )
{
	sal_uInt16 nP;
	if( nL && Seek_Entry( aE, &nP ) )
		SvUShorts::Remove( nP, nL);
}

// ---------------- bytestrings -------------------------------------

// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvByteStringsISort, ByteStringPtr )
void SvByteStringsISort::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
	if( nL )
	{
		DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
		for( sal_uInt16 n=nP; n < nP + nL; n++ )
			delete *((ByteStringPtr*)pData+n);
		SvPtrarr::Remove( nP, nL );
	}
}
sal_Bool SvByteStringsISort::Seek_Entry( const ByteStringPtr aE, sal_uInt16* pP ) const
{
	sal_uInt16 nO  = SvByteStringsISort_SAR::Count(),
			nM,
			nU = 0;
	if( nO > 0 )
	{
		nO--;
		while( nU <= nO )
		{
			nM = nU + ( nO - nU ) / 2;
			StringCompare eCmp = (*((ByteStringPtr*)pData + nM))->
						CompareIgnoreCaseToAscii( *(aE) );
			if( COMPARE_EQUAL == eCmp )
			{
				if( pP ) *pP = nM;
				return sal_True;
			}
			else if( COMPARE_LESS == eCmp )
				nU = nM + 1;
			else if( nM == 0 )
			{
				if( pP ) *pP = nU;
				return sal_False;
			}
			else
				nO = nM - 1;
		}
	}
	if( pP ) *pP = nU;
	return sal_False;
}


// Array mit anderer Seek-Methode!
_SV_IMPL_SORTAR_ALG( SvByteStringsISortDtor, ByteStringPtr )
void SvByteStringsISortDtor::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
	if( nL )
	{
		DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
		for( sal_uInt16 n=nP; n < nP + nL; n++ )
			delete *((ByteStringPtr*)pData+n);
		SvPtrarr::Remove( nP, nL );
	}
}
sal_Bool SvByteStringsISortDtor::Seek_Entry( const ByteStringPtr aE, sal_uInt16* pP ) const
{
	sal_uInt16 nO  = SvByteStringsISortDtor_SAR::Count(),
			nM,
			nU = 0;
	if( nO > 0 )
	{
		nO--;
		while( nU <= nO )
		{
			nM = nU + ( nO - nU ) / 2;
			StringCompare eCmp = (*((ByteStringPtr*)pData + nM))->
									CompareIgnoreCaseToAscii( *(aE) );
			if( COMPARE_EQUAL == eCmp )
			{
				if( pP ) *pP = nM;
				return sal_True;
			}
			else if( COMPARE_LESS == eCmp )
				nU = nM + 1;
			else if( nM == 0 )
			{
				if( pP ) *pP = nU;
				return sal_False;
			}
			else
				nO = nM - 1;
		}
	}
	if( pP ) *pP = nU;
	return sal_False;
}

