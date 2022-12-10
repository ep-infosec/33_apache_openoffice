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



#ifndef _DDEIMP_HXX
#define _DDEIMP_HXX

#ifdef OS2

#include "ddemlos2.h"

#define WORD sal_uInt16
#define DWORD sal_uLong
#define LPBYTE sal_uInt8*
#define LPWORD sal_uInt16*
#define LPDWORD sal_uLong*
#define LPCTSTR PCSZ

#else

#include <tools/prewin.h>
#include <ddeml.h>
#include <tools/postwin.h>
#include "ddewrap.hxx"

/*
extern "C"
{
#define sal_Bool WIN_BOOL
#define sal_uInt8 WIN_BYTE
#undef sal_Bool
#undef sal_uInt8
};
*/

#endif
#include <tools/string.hxx>
#include <tools/list.hxx>
#include <tools/shl.hxx>

class DdeService;
class DdeTopic;
class DdeItem;
class DdeTopics;
class DdeItems;

// ----------------
// - Conversation -
// ----------------

struct Conversation
{
	HCONV       hConv;
	DdeTopic*   pTopic;
};

DECLARE_LIST( ConvList, Conversation* );

// ---------------
// - DdeInternal -
// ---------------

class DdeInternal
{
public:
#ifdef WNT
	static HDDEDATA CALLBACK CliCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK SvrCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK InfCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#else
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
	static HDDEDATA CALLBACK __EXPORT CliCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK __EXPORT SvrCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK __EXPORT InfCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#else
	static HDDEDATA CALLBACK _export CliCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK _export SvrCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
	static HDDEDATA CALLBACK _export InfCallback
		   ( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
#endif
#endif
	static DdeService*      FindService( HSZ );
	static DdeTopic*        FindTopic( DdeService&, HSZ );
	static DdeItem*         FindItem( DdeTopic&, HSZ );
};

// -------------
// - DdeString -
// -------------

class DdeString : public String
{
protected:
	HSZ         hString;
	DWORD       hInst;

public:
				DdeString( DWORD, const sal_Unicode* );
				DdeString( DWORD, const String& );
				~DdeString();

	int         operator==( HSZ );
				operator HSZ();
};

// --------------
// - DdeDataImp -
// --------------

struct DdeDataImp
{
	HDDEDATA		hData;
	LPBYTE			pData;
	long			nData;
	sal_uLong 			nFmt;
};

class DdeConnections;
class DdeServices;

struct DdeInstData
{
	sal_uInt16			nRefCount;
	DdeConnections*	pConnections;
	// Server
	HCONV 			hCurConvSvr;
	DWORD			hDdeInstSvr;
	short			nInstanceSvr;
	DdeServices*	pServicesSvr;
	// Client
	DWORD			hDdeInstCli;
	short			nInstanceCli;
};

#ifndef SHL_SVDDE
#define SHL_SVDDE	SHL_SHL2
#endif

inline DdeInstData* ImpGetInstData()
{
	return (DdeInstData*)(*GetAppData( SHL_SVDDE ));
}
DdeInstData* ImpInitInstData();
void ImpDeinitInstData();

#endif // _DDEIMP_HXX
