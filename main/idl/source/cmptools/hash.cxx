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
#include "precompiled_idl.hxx"

/****************** I N C L U D E S **************************************/
// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// Programmabh�ngige Includes.
#include <hash.hxx>
#include <tools/debug.hxx>

/****************** C O D E **********************************************/
/*************************************************************************
|*
|*    SvStringHashEntry::~SvStringHashEntry()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashEntry::~SvStringHashEntry() { };

/*************************************************************************
|*
|*    SvHashTable::SvHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvHashTable::SvHashTable( sal_uInt32 nMaxEntries )
{
    nMax = nMaxEntries;     // set max entries
    nFill = 0;              // no entries
    lTry = 0;
    lAsk = 0;
}

/*************************************************************************
|*
|*    SvHashTable::~SvHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvHashTable::~SvHashTable()
{
#ifdef DOS_NIE
    printf( "Maximum: %ld, F�llung: %ld\n", (sal_uLong)nMax, (sal_uLong)nFill );
    printf( "Anfragen: %ld, Versuche: %ld", (sal_uLong)lAsk, (sal_uLong)lTry );
    if( lTry != 0 )
        printf( ", V/E = %ld\n", lTry / lAsk );
#endif
}

/*************************************************************************
|*
|*    SvHashTable::Test_Insert()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvHashTable::Test_Insert( const void * pElement, sal_Bool bInsert,
                               sal_uInt32 * pInsertPos )
{
    sal_uInt32    nHash;
    sal_uInt32    nIndex;
    sal_uInt32    nLoop;

    lAsk++;
    lTry++;

    nHash =  HashFunc( pElement );
    nIndex = nHash % nMax;

//  const char* s = ((ByteString*) pElement)->GetStr();
//  fprintf(stderr,"### Hash: %lu , Name: %s\n",nIndex,s );

    nLoop = 0;                                      // divide to range
    while( (nMax != nLoop) && IsEntry( nIndex ) )
    {                     // is place occupied
        if( COMPARE_EQUAL == Compare( pElement, nIndex ) )
        {
            if( pInsertPos )
                *pInsertPos = nIndex;               // place of Element
            return sal_True;
        }
        nLoop++;
        lTry++;
        nIndex = (sal_uInt16)(nIndex + nHash + 7) % nMax;
    }

    if( bInsert )
    {
        DBG_ASSERT( nMax != nLoop, "Hash table full" );
		if( nMax != nLoop )
		{
	        nFill++;
	        *pInsertPos = nIndex;                       // return free place
	        return sal_True;
		}
    }
    return( sal_False );
}

/************************************************************************/
/*************************************************************************
|*
|*    SvStringHashTable::SvStringHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashTable::SvStringHashTable( sal_uInt32 nMaxEntries )
        : SvHashTable( nMaxEntries )
{
    pEntries = new SvStringHashEntry[ nMaxEntries ];

    // RefCount auf eins setzen
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + nMaxEntries;
    while( pPos != pEnd )
    {
        pPos->AddRef();
        pPos++;
    }
}

/*************************************************************************
|*
|*    ~SvStringHashTable::SvStringHashTable()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashTable::~SvStringHashTable()
{
    // RefCount auf eins setzen
    SvStringHashEntry * pPos, *pEnd;
    pPos    = pEntries;
    pEnd    = pEntries + GetMax();
#ifdef DBG_UTIL
    while( pPos != pEnd )
    {
        DBG_ASSERT( pPos->GetRefCount() == 1, "Reference count != 1" );
        pPos++;
    }
#endif

#ifdef MPW
    // der MPW-Compiler ruft sonst keine Dtoren!
    for ( sal_uInt16 n = 0; n < GetMax(); ++n )
        (pEntries+n)->SvStringHashEntry::~SvStringHashEntry();
    delete (void*) pEntries;
#else
    delete [] pEntries;
#endif
}

/*************************************************************************
|*
|*    SvStringHashTable::HashFunc()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_uInt32 SvStringHashTable::HashFunc( const void * pElement ) const
{
    sal_uInt32          nHash = 0;  // hash value
    const char *    pStr = ((const ByteString * )pElement)->GetBuffer();

    int nShift = 0;
    while( *pStr )
    {
        if( isupper( *pStr ) )
            nHash ^= sal_uInt32(*pStr - 'A' + 26) << nShift;
        else
            nHash ^= sal_uInt32(*pStr - 'a') << nShift;
        if( nShift == 28 )
            nShift = 0;
        else
            nShift += 4;
        pStr++;
    }
    return( nHash );
}

/*************************************************************************
|*
|*    SvStringHashTable::GetNearString()
|*
|*    Beschreibung
|*
*************************************************************************/
ByteString SvStringHashTable::GetNearString( const ByteString & rName ) const
{
    for( sal_uInt32 i = 0; i < GetMax(); i++ )
    {
        SvStringHashEntry * pE = Get( i );
        if( pE )
        {
            if( pE->GetName().EqualsIgnoreCaseAscii( rName ) && !pE->GetName().Equals( rName ) )
                return pE->GetName();
        }
    }
    return ByteString();
}

/*************************************************************************
|*
|*    SvStringHashTable::IsEntry()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvStringHashTable::IsEntry( sal_uInt32 nIndex ) const
{
    if( nIndex >= GetMax() )
        return sal_False;
    return pEntries[ nIndex ].HasId();
}

/*************************************************************************
|*
|*    SvStringHashTable::Insert()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvStringHashTable::Insert( const ByteString & rName, sal_uInt32 * pIndex )
{
    sal_uInt32 nIndex;

    if( !pIndex ) pIndex = &nIndex;

    if( !SvHashTable::Test_Insert( &rName, sal_True, pIndex ) )
        return sal_False;

    if( !IsEntry( *pIndex ) )
        pEntries[ *pIndex ] = SvStringHashEntry( rName, *pIndex );
    return sal_True;
}

/*************************************************************************
|*
|*    SvStringHashTable::Test()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvStringHashTable::Test( const ByteString & rName, sal_uInt32 * pPos ) const
{
    return ((SvStringHashTable *)this)->SvHashTable::
                Test_Insert( &rName, sal_False, pPos );
}

/*************************************************************************
|*
|*    SvStringHashTable::Get()
|*
|*    Beschreibung
|*
*************************************************************************/
SvStringHashEntry * SvStringHashTable::Get( sal_uInt32 nIndex ) const
{
    if( IsEntry( nIndex ) )
        return pEntries + nIndex;
    return( NULL );
}

/*************************************************************************
|*
|*    SvStringHashTable::Get()
|*
|*    Beschreibung
|*
*************************************************************************/
StringCompare SvStringHashTable::Compare( const void * pElement,
                                          sal_uInt32 nIndex ) const
{
    return ((const ByteString *)pElement)->CompareTo( pEntries[ nIndex ].GetName() );
}

/*************************************************************************
|*
|*    SvStringHashTable::FillHashList()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvStringHashTable::FillHashList( SvStringHashList * pList ) const
{
    for( sal_uInt32 n = 0; n < GetMax(); n++ )
    {
        if( IsEntry( n ) )
            pList->Insert( Get( n ), LIST_APPEND );
    }
    // Hash Reihenfolge, jetzt sortieren
}
