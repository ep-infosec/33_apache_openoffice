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
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <tools/rc.h>
#include <rscerror.h>
#include <rsctools.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rsctree.hxx>
#include <rscdb.hxx>
#include <rscdef.hxx>
#include <rscpar.hxx>

#include "rsclex.hxx"

/************** V a r i a b l e n ****************************************/
ObjectStack 					S;
RscTop *						pCurClass;
sal_uInt32						nCurMask;
char							szErrBuf[ 100 ];

/************** H i l f s F u n k t i o n e n ****************************/
RSCINST GetVarInst( const RSCINST & rInst, const char * pVarName )
{
	RSCINST aInst;

	aInst = rInst.pClass->GetVariable( rInst, pHS->getID( pVarName ),
									   RSCINST() );

	if( !aInst.pData )
		pTC->pEH->Error( ERR_NOVARIABLENAME, rInst.pClass, RscId() );

	return( aInst );
}

void SetNumber( const RSCINST & rInst, const char * pVarName, sal_Int32 lValue )
{
	RSCINST aInst;

	aInst = GetVarInst( rInst, pVarName );

	if( aInst.pData ){
		ERRTYPE aError;
		aError = aInst.pClass->SetNumber( aInst, lValue );

		if( aError.IsError() )
			pTC->pEH->Error( aError, aInst.pClass, RscId() );
	}
}

void SetConst( const RSCINST & rInst, const char * pVarName,
			   Atom nValueId, sal_Int32 nVal )
{
	RSCINST aInst;

	aInst = GetVarInst( rInst, pVarName );
	if( aInst.pData )
	{
		ERRTYPE aError;
		aError = aInst.pClass->SetConst( aInst, nValueId, nVal );

		if( aError.IsError() )
			pTC->pEH->Error( aError, aInst.pClass, RscId() );
	}
}

void SetString( const RSCINST & rInst, const char * pVarName, const char * pStr )
{
	RSCINST aInst;

	aInst = GetVarInst( rInst, pVarName );
	if( aInst.pData ){
		ERRTYPE aError;
		aError = aInst.pClass->SetString( aInst, pStr );

		if( aError.IsError() )
			pTC->pEH->Error( aError, aInst.pClass, RscId() );
	}
}

RscId MakeRscId( RscExpType aExpType )
{
	if( !aExpType.IsNothing() ){
		sal_Int32		lValue;

		if( !aExpType.Evaluate( &lValue ) )
			pTC->pEH->Error( ERR_ZERODIVISION, NULL, RscId() );
		if( lValue < 1 || lValue > (sal_Int32)0x7FFF )
		{
			pTC->pEH->Error( ERR_IDRANGE, NULL, RscId(),
							 ByteString::CreateFromInt32( lValue ).GetBuffer() );
		}

		if( aExpType.IsDefinition() )
			return RscId( aExpType.aExp.pDef );
		else
			return RscId( lValue );
	}
	return RscId();
}

sal_Bool DoClassHeader( RSCHEADER * pHeader, sal_Bool bMember )
{
	RSCINST aCopyInst;
	RscId aName1 = MakeRscId( pHeader->nName1 );
	RscId aName2 = MakeRscId( pHeader->nName2 );

	if( pHeader->pRefClass )
		aCopyInst.pClass = pHeader->pRefClass;
	else
		aCopyInst.pClass = pHeader->pClass;

	if( TYPE_COPY == pHeader->nTyp )
	{
		ObjNode * pCopyObj = aCopyInst.pClass->GetObjNode( aName2 );

		if( !pCopyObj )
		{
			ByteString aMsg( pHS->getString( aCopyInst.pClass->GetId() ) );
			aMsg += ' ';
			aMsg += aName2.GetName();
			pTC->pEH->Error( ERR_NOCOPYOBJ, pHeader->pClass, aName1,
							 aMsg.GetBuffer() );
		}
		else
			aCopyInst.pData = pCopyObj->GetRscObj();
	}

	if( bMember )
	{
		// Angabe von Superklassen oder abgeleiteten Klassen ist jetzt erlaubt
		if( S.Top().pClass->InHierarchy( pHeader->pClass )
		  ||  pHeader->pClass->InHierarchy( S.Top().pClass) )
		{
			if( aCopyInst.IsInst() )
			{
				RSCINST aTmpI( S.Top() );
				aTmpI.pClass->Destroy( aTmpI );
				aTmpI.pClass->Create( &aTmpI, aCopyInst );
			};
		}
		else
			pTC->pEH->Error( ERR_FALSETYPE, S.Top().pClass, aName1,
							 pHS->getString( pHeader->pClass->GetId() ) );
	}
	else
	{
		if( S.IsEmpty() )
		{
			if( (sal_Int32)aName1 < 256 )
				pTC->pEH->Error( WRN_GLOBALID, pHeader->pClass, aName1 );

			if( aCopyInst.IsInst() )
				S.Push( pHeader->pClass->Create( NULL, aCopyInst ) );
			else
				S.Push( pHeader->pClass->Create( NULL, RSCINST() ) );

			ObjNode * pNode = new ObjNode( aName1, S.Top().pData,
										   pFI->GetFileIndex() );
			pTC->pEH->StdOut( ".", RscVerbosityVerbose );

			if( !aName1.IsId() )
				pTC->pEH->Error( ERR_IDEXPECTED, pHeader->pClass, aName1 );
			else if( !pHeader->pClass->PutObjNode( pNode ) )
				pTC->pEH->Error( ERR_DOUBLEID, pHeader->pClass, aName1 );
		}
		else
		{
			RSCINST aTmpI;
			ERRTYPE aError;

			if( (sal_Int32)aName1 >= 256 && aName1.IsId() )
				pTC->pEH->Error( WRN_LOCALID, pHeader->pClass, aName1 );
			aError = S.Top().pClass->GetElement( S.Top(), aName1,
												 pHeader->pClass, aCopyInst, &aTmpI );

			if( aError.IsWarning() )
				pTC->pEH->Error( aError, pHeader->pClass, aName1 );
			else if( aError.IsError() )
			{
				if( ERR_CONT_INVALIDTYPE == aError )
					pTC->pEH->Error( aError, S.Top().pClass, aName1,
									 pHS->getString( pHeader->pClass->GetId() ) );
				else
					pTC->pEH->Error( aError, S.Top().pClass, aName1 );
				S.Top().pClass->GetElement( S.Top(), RscId(),
											pHeader->pClass, RSCINST(), &aTmpI );

				if( !aTmpI.IsInst() )
					return( sal_False );
			}
			S.Push( aTmpI );
		};
	};
	if( TYPE_REF == pHeader->nTyp )
	{
		ERRTYPE aError;

		aError = S.Top().pClass->SetRef( S.Top(), aName2 );
		pTC->pEH->Error( aError, S.Top().pClass, aName1 );
	}

	return( sal_True );
}

RSCINST GetFirstTupelEle( const RSCINST & rTop )
{ // Aufwaertskompatible, Tupel probieren
	RSCINST aInst;
	ERRTYPE aErr;

	aErr = rTop.pClass->GetElement( rTop, RscId(), NULL, RSCINST(), &aInst );
	if( !aErr.IsError() )
		aInst = aInst.pClass->GetTupelVar( aInst, 0, RSCINST() );
	return aInst;
}

/************** Y a c c   C o d e ****************************************/
//#define YYDEBUG 1

#define TYPE_Atom 			  0
#define TYPE_RESID				  1

#ifdef UNX
#define YYMAXDEPTH				2000
#else
#define YYMAXDEPTH				800
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4129 4273 4701)
#endif
#include "yyrscyacc.cxx"
#if defined _MSC_VER
#pragma warning(pop)
#endif

