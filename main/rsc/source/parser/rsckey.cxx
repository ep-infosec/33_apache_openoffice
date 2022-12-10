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
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <rscall.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rsckey.hxx>

#if defined(_MSC_VER) && (_MSC_VER >= 1200 )
#define _cdecl __cdecl
#endif

/****************** C o d e **********************************************/
/****************** keyword sort function ********************************/
extern "C" {
#if defined( ZTC ) && defined( PM2 )
    int __CLIB KeyCompare( const void * pFirst, const void * pSecond );
#else
#if defined( WNT ) && !defined( WTC ) && !defined (ICC)
    int _cdecl KeyCompare( const void * pFirst, const void * pSecond );
#else
    int KeyCompare( const void * pFirst, const void * pSecond );
#endif
#endif
}

#if defined( WNT ) && !defined( WTC ) && !defined(ICC)
int _cdecl KeyCompare( const void * pFirst, const void * pSecond ){
#else
int KeyCompare( const void * pFirst, const void * pSecond ){
#endif
    if( ((KEY_STRUCT *)pFirst)->nName > ((KEY_STRUCT *)pSecond)->nName )
        return( 1 );
    else if( ((KEY_STRUCT *)pFirst)->nName < ((KEY_STRUCT *)pSecond)->nName )
        return( -1 );
    else
        return( 0 );
}

/*************************************************************************
|*
|*    RscNameTable::RscNameTable()
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
|*
*************************************************************************/
RscNameTable::RscNameTable() {
    bSort    = sal_True;
    nEntries = 0;
    pTable   = NULL;
};

/*************************************************************************
|*
|*    RscNameTable::~RscNameTable()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
RscNameTable::~RscNameTable() {
    if( pTable )
        rtl_freeMemory( pTable );
};


/*************************************************************************
|*
|*    RscNameTable::SetSort()
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
|*
*************************************************************************/
void RscNameTable::SetSort( sal_Bool bSorted ){
    bSort = bSorted;
    if( bSort && pTable){
        // Schluesselwort Feld sortieren
        qsort( (void *)pTable, nEntries,
               sizeof( KEY_STRUCT ), KeyCompare );
    };
};

/*************************************************************************
|*
|*    RscNameTable::Put()
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
|*
*************************************************************************/
Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, long nValue ){
    if( pTable )
        pTable = (KEY_STRUCT *)
                 rtl_reallocateMemory( (void *)pTable,
                 ((nEntries +1) * sizeof( KEY_STRUCT )) );
    else
        pTable = (KEY_STRUCT *)
                 rtl_allocateMemory( ((nEntries +1)
                                 * sizeof( KEY_STRUCT )) );
    pTable[ nEntries ].nName  = nName;
    pTable[ nEntries ].nTyp   = nTyp;
    pTable[ nEntries ].yylval = nValue;
    nEntries++;
    if( bSort )
        SetSort();
    return( nName );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp, long nValue )
{
    return( Put( pHS->getID( pName ), nTyp, nValue ) );
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp )
{
    return( Put( nName, nTyp, (long)nName ) );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp )
{
    Atom  nId;

    nId = pHS->getID( pName );
    return( Put( nId, nTyp, (long)nId ) );
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass )
{
    return( Put( nName, nTyp, (long)pClass ) );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp, RscTop * pClass )
{
    return( Put( pHS->getID( pName ), nTyp, (long)pClass ) );
};

/*************************************************************************
|*
|*    RscNameTable::Get()
|*
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
|*
*************************************************************************/
sal_Bool RscNameTable::Get( Atom nName, KEY_STRUCT * pEle ){
    KEY_STRUCT * pKey = NULL;
    KEY_STRUCT  aSearchName;
    sal_uInt32  i;

    if( bSort ){
        // Suche nach dem Schluesselwort
        aSearchName.nName = nName;
        pKey = (KEY_STRUCT *)bsearch(
#ifdef UNX
                   (const char *) &aSearchName, (char *)pTable,
#else
                   (const void *) &aSearchName, (const void *)pTable,
#endif
                   nEntries, sizeof( KEY_STRUCT ), KeyCompare );
    }
    else{
        i = 0;
        while( i < nEntries && !pKey ){
            if( pTable[ i ].nName == nName )
                pKey = &pTable[ i ];
            i++;
        };
    };

    if( pKey ){ // Schluesselwort gefunden
        *pEle = *pKey;
        return( sal_True );
    };
    return( sal_False );
};

