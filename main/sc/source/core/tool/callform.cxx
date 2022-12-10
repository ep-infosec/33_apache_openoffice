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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include <vcl/svapp.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "callform.hxx"
#include "global.hxx"
#include "adiasync.hxx"

//------------------------------------------------------------------------

extern "C" {

typedef void (CALLTYPE* ExFuncPtr1)(void*);
typedef void (CALLTYPE* ExFuncPtr2)(void*, void*);
typedef void (CALLTYPE* ExFuncPtr3)(void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr4)(void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr5)(void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr6)(void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr7)(void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr8)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr9)(void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr10)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr11)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr12)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr13)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr14)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr15)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void (CALLTYPE* ExFuncPtr16)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

typedef void (CALLTYPE* GetFuncCountPtr)(sal_uInt16& nCount);
typedef void (CALLTYPE* GetFuncDataPtr)
	(sal_uInt16& nNo, sal_Char* pFuncName, sal_uInt16& nParamCount, ParamType* peType, sal_Char* pInternalName);

typedef void (CALLTYPE* SetLanguagePtr)( sal_uInt16& nLanguage );
typedef void (CALLTYPE* GetParamDesc)
	(sal_uInt16& nNo, sal_uInt16& nParam, sal_Char* pName, sal_Char* pDesc );

typedef void (CALLTYPE* IsAsync) ( sal_uInt16&		nNo,
								   ParamType*	peType );
typedef void (CALLTYPE* Advice)  ( sal_uInt16&		nNo,
								   AdvData&		pfCallback );
typedef void (CALLTYPE* Unadvice)( double&		nHandle );

typedef void (CALLTYPE* FARPROC) ( void );

}

#if defined(OS2) && defined(BLC)
#define GETFUNCTIONCOUNT		"_GetFunctionCount"
#define GETFUNCTIONDATA			"_GetFunctionData"
#define SETLANGUAGE				"_SetLanguage"
#define GETPARAMDESC			"_GetParameterDescription"
#define ISASYNC					"_IsAsync"
#define ADVICE					"_Advice"
#define UNADVICE				"_Unadvice"
#else // Pascal oder extern "C"
#define GETFUNCTIONCOUNT		"GetFunctionCount"
#define GETFUNCTIONDATA			"GetFunctionData"
#define SETLANGUAGE				"SetLanguage"
#define GETPARAMDESC			"GetParameterDescription"
#define ISASYNC					"IsAsync"
#define ADVICE					"Advice"
#define UNADVICE				"Unadvice"
#endif

#define LIBFUNCNAME( name ) \
	(String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( name ) ))

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

FuncData::FuncData(const String& rIName) :
	pModuleData		(NULL),
	aInternalName   (rIName),
//  aFuncName		(""),
	nNumber			(0),
	nParamCount		(0),
	eAsyncType		(NONE)
{
	for (sal_uInt16 i = 0; i < MAXFUNCPARAM; i++)
		eParamType[i] = PTR_DOUBLE;
}

//------------------------------------------------------------------------

FuncData::FuncData(const ModuleData*pModule,
				   const String&	rIName,
				   const String&	rFName,
						 sal_uInt16	nNo,
					sal_uInt16	nCount,
				   const ParamType* peType,
					ParamType  eType) :
	pModuleData		(pModule),
	aInternalName   (rIName),
	aFuncName		(rFName),
	nNumber			(nNo),
	nParamCount		(nCount),
	eAsyncType		(eType)
{
	for (sal_uInt16 i = 0; i < MAXFUNCPARAM; i++)
		eParamType[i] = peType[i];
}

//------------------------------------------------------------------------

FuncData::FuncData(const FuncData& rData) :
    ScDataObject(),
	pModuleData		(rData.pModuleData),
	aInternalName   (rData.aInternalName),
	aFuncName		(rData.aFuncName),
	nNumber			(rData.nNumber),
	nParamCount		(rData.nParamCount),
	eAsyncType		(rData.eAsyncType)
{
	for (sal_uInt16 i = 0; i < MAXFUNCPARAM; i++)
		eParamType[i] = rData.eParamType[i];
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

short FuncCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return (short) ScGlobal::GetpTransliteration()->compareString(
		((FuncData*)pKey1)->aInternalName, ((FuncData*)pKey2)->aInternalName );
}

//------------------------------------------------------------------------

sal_Bool FuncCollection::SearchFunc( const String& rName, sal_uInt16& rIndex ) const
{
	FuncData aDataObj(rName);
	return Search( &aDataObj, rIndex );
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleData : public ScDataObject
{
friend class ModuleCollection;
	String		aName;
    osl::Module* pInstance;
public:
    ModuleData(const String& rStr, osl::Module* pInst) : aName (rStr), pInstance (pInst) {}
    ModuleData(const ModuleData& rData) : ScDataObject(), aName (rData.aName) {pInstance = new osl::Module(aName);}
	~ModuleData() { delete pInstance; }
	virtual ScDataObject*	Clone() const { return new ModuleData(*this); }

	const   String&			GetName() const { return aName; }
            osl::Module*    GetInstance() const { return pInstance; }
			void			FreeInstance() { delete pInstance; pInstance = 0; }
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class ModuleCollection : public ScSortedCollection
{
public:
	ModuleCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False) : ScSortedCollection ( nLim, nDel, bDup ) {}
	ModuleCollection(const ModuleCollection& rModuleCollection) : ScSortedCollection ( rModuleCollection ) {}

	virtual ScDataObject*		Clone() const { return new ModuleCollection(*this); }
			ModuleData*		operator[]( const sal_uInt16 nIndex) const {return (ModuleData*)At(nIndex);}
	virtual short			Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
			sal_Bool			SearchModule( const String& rName,
										  const ModuleData*& rpModule ) const;
};

static ModuleCollection aModuleCollection;

//------------------------------------------------------------------------

short ModuleCollection::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    return (short) ScGlobal::GetpTransliteration()->compareString(
		((ModuleData*)pKey1)->aName, ((ModuleData*)pKey2)->aName );
}

//------------------------------------------------------------------------

sal_Bool ModuleCollection::SearchModule( const String& rName,
									 const ModuleData*& rpModule ) const
{
	sal_uInt16 nIndex;
	ModuleData aSearchModule(rName, 0);
	sal_Bool bFound = Search( &aSearchModule, nIndex );
	if (bFound)
		rpModule = (ModuleData*)At(nIndex);
	else
		rpModule = 0;
	return bFound;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

sal_Bool InitExternalFunc(const rtl::OUString& rModuleName)
{
	String aModuleName( rModuleName );

	// Module schon geladen?
	const ModuleData* pTemp;
	if (aModuleCollection.SearchModule(aModuleName, pTemp))
		return sal_False;

	rtl::OUString aNP;
	aNP = rModuleName;

	sal_Bool bRet = sal_False;
    osl::Module* pLib = new osl::Module( aNP );
    if (pLib->is())
	{
        FARPROC fpGetCount = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(GETFUNCTIONCOUNT));
        FARPROC fpGetData = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(GETFUNCTIONDATA));
		if ((fpGetCount != NULL) && (fpGetData != NULL))
		{
            FARPROC fpIsAsync = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(ISASYNC));
            FARPROC fpAdvice = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(ADVICE));
            FARPROC fpSetLanguage = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(SETLANGUAGE));
			if ( fpSetLanguage )
			{
                LanguageType eLanguage = Application::GetSettings().GetUILanguage();
				sal_uInt16 nLanguage = (sal_uInt16) eLanguage;
				(*((SetLanguagePtr)fpSetLanguage))( nLanguage );
			}

			// Module in die Collection aufnehmen
			ModuleData* pModuleData = new ModuleData(aModuleName, pLib);
			aModuleCollection.Insert(pModuleData);

			// Schnittstelle initialisieren
			AdvData pfCallBack = &ScAddInAsyncCallBack;
			FuncData* pFuncData;
			FuncCollection* pFuncCol = ScGlobal::GetFuncCollection();
			sal_uInt16 nCount;
			(*((GetFuncCountPtr)fpGetCount))(nCount);
			for (sal_uInt16 i=0; i < nCount; i++)
			{
				sal_Char cFuncName[256];
				sal_Char cInternalName[256];
				sal_uInt16 nParamCount;
				ParamType eParamType[MAXFUNCPARAM];
				ParamType eAsyncType = NONE;
				// #62113# alles initialisieren, falls das AddIn sich schlecht verhaelt
				cFuncName[0] = 0;
				cInternalName[0] = 0;
				nParamCount = 0;
				for ( sal_uInt16 j=0; j<MAXFUNCPARAM; j++ )
				{
					eParamType[j] = NONE;
				}
				(*((GetFuncDataPtr)fpGetData))(i, cFuncName, nParamCount,
											   eParamType, cInternalName);
				if( fpIsAsync )
				{
					(*((IsAsync)fpIsAsync))(i, &eAsyncType);
					if ( fpAdvice && eAsyncType != NONE )
						(*((Advice)fpAdvice))( i, pfCallBack );
				}
				String aInternalName( cInternalName, osl_getThreadTextEncoding() );
				String aFuncName( cFuncName, osl_getThreadTextEncoding() );
				pFuncData = new FuncData( pModuleData,
										  aInternalName,
										  aFuncName,
										  i,
										  nParamCount,
										  eParamType,
										  eAsyncType );
				pFuncCol->Insert(pFuncData);
			}
			bRet = sal_True;
		}
		else
			delete pLib;
	}
	else
		delete pLib;
	return bRet;
}

//------------------------------------------------------------------------

void ExitExternalFunc()
{
	sal_uInt16 nCount = aModuleCollection.GetCount();
	for (sal_uInt16 i=0; i<nCount; i++)
	{
		ModuleData* pData = aModuleCollection[i];
		pData->FreeInstance();
	}
}

//------------------------------------------------------------------------

sal_Bool FuncData::Call(void** ppParam)
{
	sal_Bool bRet = sal_False;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(aFuncName);
	if (fProc != NULL)
	{
		switch (nParamCount)
		{
			case 1 :
				(*((ExFuncPtr1)fProc))(ppParam[0]);
				bRet = sal_True;
				break;
			case 2 :
				(*((ExFuncPtr2)fProc))(ppParam[0], ppParam[1]);
				bRet = sal_True;
				break;
			case 3 :
				(*((ExFuncPtr3)fProc))(ppParam[0], ppParam[1], ppParam[2]);
				bRet = sal_True;
				break;
			case 4 :
				(*((ExFuncPtr4)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3]);
				bRet = sal_True;
				break;
			case 5 :
				(*((ExFuncPtr5)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4]);
				bRet = sal_True;
				break;
			case 6 :
				(*((ExFuncPtr6)fProc))(ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5]);
				bRet = sal_True;
				break;
			case 7 :
				(*((ExFuncPtr7)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6]);
				bRet = sal_True;
				break;
			case 8 :
				(*((ExFuncPtr8)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7]);
				bRet = sal_True;
				break;
			case 9 :
				(*((ExFuncPtr9)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8]);
				bRet = sal_True;
				break;
			case 10 :
				(*((ExFuncPtr10)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9]);
				bRet = sal_True;
				break;
			case 11 :
				(*((ExFuncPtr11)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10]);
				bRet = sal_True;
				break;
			case 12:
				(*((ExFuncPtr12)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11]);
				bRet = sal_True;
				break;
			case 13:
				(*((ExFuncPtr13)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
										ppParam[12]);
				bRet = sal_True;
				break;
			case 14 :
				(*((ExFuncPtr14)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
										ppParam[12], ppParam[13]);
				bRet = sal_True;
				break;
			case 15 :
				(*((ExFuncPtr15)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
										ppParam[12], ppParam[13], ppParam[14]);
				bRet = sal_True;
				break;
			case 16 :
				(*((ExFuncPtr16)fProc))( ppParam[0], ppParam[1], ppParam[2], ppParam[3], ppParam[4], ppParam[5],
										ppParam[6], ppParam[7], ppParam[8], ppParam[9], ppParam[10], ppParam[11],
										ppParam[12], ppParam[13], ppParam[14], ppParam[15]);
				bRet = sal_True;
				break;
			default : break;
		}
	}
	return bRet;
}

//------------------------------------------------------------------------

sal_Bool FuncData::Unadvice( double nHandle )
{
	sal_Bool bRet = sal_False;
    osl::Module* pLib = pModuleData->GetInstance();
    FARPROC fProc = (FARPROC)pLib->getFunctionSymbol(LIBFUNCNAME(UNADVICE));
	if (fProc != NULL)
	{
		((::Unadvice)fProc)(nHandle);
		bRet = sal_True;
	}
	return bRet;
}

//------------------------------------------------------------------------

const String& FuncData::GetModuleName() const
{
	// DBG_ASSERT( pModuleData, "Keine Arme, keine Kekse" ):
	return pModuleData->GetName();
}

//------------------------------------------------------------------------

sal_Bool FuncData::GetParamDesc( String& aName, String& aDesc, sal_uInt16 nParam )
{
	sal_Bool bRet = sal_False;
	if ( nParam <= nParamCount )
	{
        osl::Module* pLib = pModuleData->GetInstance();
        FARPROC fProc = (FARPROC) pLib->getFunctionSymbol( LIBFUNCNAME(GETPARAMDESC) );
		if ( fProc != NULL )
		{
			sal_Char pcName[256];
			sal_Char pcDesc[256];
			*pcName = *pcDesc = 0;
			sal_uInt16 nFuncNo = nNumber;	// nicht per Reference versauen lassen..
			((::GetParamDesc)fProc)( nFuncNo, nParam, pcName, pcDesc );
			aName = String( pcName, osl_getThreadTextEncoding() );
			aDesc = String( pcDesc, osl_getThreadTextEncoding() );
			bRet = sal_True;
		}
	}
	if ( !bRet )
	{
		aName.Erase();
		aDesc.Erase();
	}
	return bRet;
}


