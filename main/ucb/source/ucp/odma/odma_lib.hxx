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


#ifndef ODMA_LIB_HXX
#define ODMA_LIB_HXX

typedef void	            *LPVOID;
typedef char				*LPSTR, 
							*PSTR;
typedef const char			*LPCSTR;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef unsigned short      *LPWORD;
typedef DWORD		        *LPDWORD;

#define WINAPI				__stdcall
#define FAR

#include "odma.h"

namespace odma
{
	typedef ODMSTATUS (WINAPI *TODMRegisterApp) (	ODMHANDLE FAR *pOdmHandle, 
													WORD version,
													LPSTR lpszAppId, 
													DWORD dwEnvData, 
													LPVOID pReserved);
	
	typedef void (WINAPI *TODMUnRegisterApp)(ODMHANDLE odmHandle);
	

	typedef ODMSTATUS (WINAPI *TODMSelectDoc)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags);
	

	typedef ODMSTATUS (WINAPI *TODMOpenDoc)(		ODMHANDLE odmHandle,
													DWORD flags,
													LPSTR lpszDocId, 
													LPSTR lpszDocLocation);

	typedef ODMSTATUS (WINAPI *TODMSaveDoc)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPSTR lpszNewDocId);

	typedef ODMSTATUS (WINAPI *TODMCloseDoc)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													DWORD activeTime, 
													DWORD pagesPrinted, 
													LPVOID sessionData, 
													WORD dataLen);

	typedef ODMSTATUS (WINAPI *TODMNewDoc)(			ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													DWORD dwFlags, 
													LPSTR lpszFormat, 
													LPSTR lpszDocLocation);

	typedef ODMSTATUS (WINAPI *TODMSaveAs)(			ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPSTR lpszNewDocId, 
													LPSTR lpszFormat, 
													ODMSAVEASCALLBACK pcbCallBack,
													LPVOID pInstanceData);

	typedef ODMSTATUS (WINAPI *TODMActivate)(		ODMHANDLE odmHandle, 
													WORD action,
													LPSTR lpszDocId);

	typedef ODMSTATUS (WINAPI *TODMGetDocInfo)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													WORD item, 
													LPSTR lpszData, 
													WORD dataLen);

	typedef ODMSTATUS (WINAPI *TODMSetDocInfo)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													WORD item, 
													LPSTR lpszData);

	typedef ODMSTATUS (WINAPI *TODMGetDMSInfo)(		ODMHANDLE odmHandle, 
													LPSTR lpszDmsId,
													LPWORD pwVerNo, 
													LPDWORD pdwExtensions);

	/* Query Enhancements */
	typedef WORD (WINAPI *TODMGetDMSCount)();

	typedef WORD (WINAPI *TODMGetDMSList)(			LPSTR buffer, 
													WORD buffer_size );

	typedef ODMSTATUS (WINAPI *TODMGetDMS)(			LPCSTR lpszAppId, 
													LPSTR lpszDMSId );

	typedef ODMSTATUS (WINAPI *TODMSetDMS)(			LPCSTR lpszAppId, 
													LPCSTR lpszDMSId );

	typedef ODMSTATUS (WINAPI *TODMQueryExecute)(	ODMHANDLE odmHandle, 
													LPCSTR lpszQuery,
													DWORD flags, 
													LPCSTR lpszDMSList, 
													LPSTR queryId );

	typedef ODMSTATUS (WINAPI *TODMQueryGetResults)(ODMHANDLE odmHandle, 
													LPCSTR queryId,
													LPSTR lpszDocId, 
													LPSTR lpszDocName, 
													WORD docNameLen,
													WORD *docCount );

	typedef ODMSTATUS (WINAPI *TODMQueryClose)(		ODMHANDLE odmHandle, 
													LPCSTR queryId );

	/* ODMA 2.0 Enhancements */
	typedef ODMSTATUS (WINAPI *TODMCloseDocEx)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags, 
													DWORD activeTime, 
													DWORD pagesPrinted,
													LPVOID sessionData, 
													WORD dataLen);

	typedef ODMSTATUS (WINAPI *TODMSaveAsEx)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPSTR lpszNewDocId, 
													LPSTR lpszFormat, 
													ODMSAVEASCALLBACK pcbCallBack,
													LPVOID pInstanceData, 
													LPDWORD pdwFlags);

	typedef ODMSTATUS (WINAPI *TODMSaveDocEx)(		ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPSTR lpszNewDocId,	
													LPDWORD pdwFlags);

	typedef ODMSTATUS (WINAPI *TODMSelectDocEx)(	ODMHANDLE odmHandle, 
													LPSTR lpszDocIds,
													LPWORD pwDocIdsLen, 
													LPWORD pwDocCount, 
													LPDWORD pdwFlags,
													LPSTR lpszFormatFilter);

	typedef ODMSTATUS (WINAPI *TODMQueryCapability)(ODMHANDLE odmHandle, 
													LPCSTR lpszDmsId,
													DWORD function, 
													DWORD item, 
													DWORD flags);

	typedef ODMSTATUS (WINAPI *TODMSetDocEvent)(	ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													DWORD flags, 
													DWORD event, 
													LPVOID lpData, 
													DWORD dwDataLen,
													LPSTR lpszComment);

	typedef ODMSTATUS (WINAPI *TODMGetAlternateContent)(ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags, 
													LPSTR lpszFormat, 
													LPSTR lpszDocLocation);

	typedef ODMSTATUS (WINAPI *TODMSetAlternateContent)(ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags, 
													LPSTR lpszFormat, 
													LPSTR lpszDocLocation);

	typedef ODMSTATUS (WINAPI *TODMGetDocRelation)(	ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags, 
													LPSTR lpszLinkedId, 
													LPSTR lpszFormat,
													LPSTR lpszPreviousId);

	typedef ODMSTATUS (WINAPI *TODMSetDocRelation)(	ODMHANDLE odmHandle, 
													LPSTR lpszDocId,
													LPDWORD pdwFlags, 
													LPSTR lpszLinkedId, 
													LPSTR lpszFormat,
													LPSTR lpszPreviousId);

	// now we define some macros

	#define NODMRegisterApp(a,b,c,d,e)			(*(pODMRegisterApp))(a,b,c,d,e)
	#define NODMUnRegisterApp(a)				(*(pODMUnRegisterApp))(a)
	#define NODMSelectDoc(a,b,c)				(*(pODMSelectDoc))(a,b,c)
	#define NODMOpenDoc(a,b,c,d)				(*(pODMOpenDoc))(a,b,c,d)
	#define NODMSaveDoc(a,b,c)					(*(pODMSaveDoc))(a,b,c)
	#define NODMCloseDoc(a,b,c,d,e,f)			(*(pODMCloseDoc))(a,b,c,d,e,f)
	#define NODMNewDoc(a,b,c,d,e)				(*(pODMNewDoc))(a,b,c,d,e)
	#define NODMSaveAs(a,b,c,d,e,f)				(*(pODMSaveAs))(a,b,c,d,e,f)
	#define NODMActivate(a,b,c)					(*(pODMActivate))(a,b,c)
	#define NODMGetDocInfo(a,b,c,d,e)			(*(pODMGetDocInfo))(a,b,c,d,e)
	#define NODMSetDocInfo(a,b,c,d)				(*(pODMSetDocInfo))(a,b,c,d)
	#define NODMGetDMSInfo(a,b,c,d)				(*(pODMGetDMSInfo))(a,b,c,d)
	#define NODMGetDMSCount()					(*(pODMGetDMSCount))()
	#define NODMGetDMSList(a,b)					(*(pODMGetDMSList))(a,b)
	#define NODMGetDMS(a,b)						(*(pODMGetDMS))(a,b)
	#define NODMSetDMS(a,b)						(*(pODMSetDMS))(a,b)
	#define NODMQueryExecute(a,b,c,d,e)			(*(pODMQueryExecute))(a,b,c,d,e)
	#define NODMQueryGetResults(a,b,c,d,e,f)	(*(pODMQueryGetResults))(a,b,c,d,e,f)
	#define NODMQueryClose(a,b)					(*(pODMQueryClose))(a,b)
	#define NODMCloseDocEx(a,b,c,d,e,f,g)		(*(pODMCloseDocEx))(a,b,c,d,e,f,g)
	#define NODMSaveAsEx(a,b,c,d,e,f,g)			(*(pODMSaveAsEx))(a,b,c,d,e,f,g)
	#define NODMSaveDocEx(a,b,c,d)				(*(pODMSaveDocEx))(a,b,c,d)
	#define NODMSelectDocEx(a,b,c,d,e,f)		(*(pODMSelectDocEx))(a,b,c,d,e,f)
	#define NODMQueryCapability(a,b,c,d,e)		(*(pODMQueryCapability))(a,b,c,d,e)
	#define NODMSetDocEvent(a,b,c,d,e,f,g)		(*(pODMSetDocEvent))(a,b,c,d,e,f,g)
	#define NODMGetAlternateContent(a,b,c,d,e)	(*(pODMGetAlternateContent))(a,b,c,d,e)
	#define NODMSetAlternateContent(a,b,c,d,e)	(*(pODMSetAlternateContent))(a,b,c,d,e)
	#define NODMGetDocRelation(a,b,c,d,e,f)		(*(pODMGetDocRelation))(a,b,c,d,e,f)
	#define NODMSetDocRelation(a,b,c,d,e,f)		(*(pODMSetDocRelation))(a,b,c,d,e,f)

	sal_Bool LoadLibrary();

	extern TODMRegisterApp			pODMRegisterApp;
	extern TODMUnRegisterApp		pODMUnRegisterApp;
	extern TODMSelectDoc			pODMSelectDoc;
	extern TODMOpenDoc				pODMOpenDoc;
	extern TODMSaveDoc				pODMSaveDoc;
	extern TODMCloseDoc				pODMCloseDoc;
	extern TODMNewDoc				pODMNewDoc;
	extern TODMSaveAs				pODMSaveAs;
	extern TODMActivate				pODMActivate;
	extern TODMGetDocInfo			pODMGetDocInfo;
	extern TODMSetDocInfo			pODMSetDocInfo;
	extern TODMGetDMSInfo			pODMGetDMSInfo;
	extern TODMGetDMSCount			pODMGetDMSCount;
	extern TODMGetDMSList			pODMGetDMSList;
	extern TODMGetDMS				pODMGetDMS;
	extern TODMSetDMS				pODMSetDMS;
	extern TODMQueryExecute			pODMQueryExecute;
	extern TODMQueryGetResults		pODMQueryGetResults;
	extern TODMQueryClose			pODMQueryClose;
	extern TODMCloseDocEx			pODMCloseDocEx;
	extern TODMSaveAsEx				pODMSaveAsEx;
	extern TODMSaveDocEx			pODMSaveDocEx;
	extern TODMSelectDocEx			pODMSelectDocEx;
	extern TODMQueryCapability		pODMQueryCapability;
	extern TODMSetDocEvent			pODMSetDocEvent;
	extern TODMGetAlternateContent	pODMGetAlternateContent;
	extern TODMSetAlternateContent	pODMSetAlternateContent;
	extern TODMGetDocRelation		pODMGetDocRelation;
	extern TODMSetDocRelation		pODMSetDocRelation;

}

#endif // ODMA_LIB_HXX
