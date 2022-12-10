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





#define AVER( pFirst, pSecond, Membername ) (( pFirst->Membername + pSecond->Membername ) / 2 )
#define DIFF( pFirst, pSecond, Membername ) ( pSecond->Membername - pFirst->Membername )
#define S_SAFEDIV( a,b ) ((b)==0?CUniString("#DIV"):UniString::CreateFromInt32( (ULONG) ((a)/(b))))
#define S_SAFEDIV_DEC( a,b ) ((b)==0?CUniString("#DIV"):Dec((ULONG) ((a)/(b))))

#include <tools/time.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>

#define PROFILE_START	0x01
#define PROFILE_END 	0x02


struct SysdepProfileSnapshot;
struct SysdepStaticData;	// Nicht wirklich statisch, sondern statisch �ber mehrere Snapshots

struct ProfileSnapshot
{
	Time aTime;
	SysdepProfileSnapshot *pSysdepProfileSnapshot;
	sal_uLong nProcessTicks;
	sal_uLong nSystemTicks;
};


class TTProfiler : private Timer
{
public:
	TTProfiler();
	~TTProfiler();

	String GetProfileHeader();	// Titelzeile f�r Logdatei
	void StartProfileInterval( sal_Bool bReadAnyway = sal_False );	// Zustand merken
	void EndProfileInterval();	// Informationszeile zusammenbauen
	String GetProfileLine( String &aPrefix );


	void StartProfilingPerCommand();	// Jeden Befehl mitschneiden
	void StopProfilingPerCommand();
	sal_Bool IsProfilingPerCommand() { return bIsProfilingPerCommand; }

	void StartPartitioning();
	void StopPartitioning();
	sal_Bool IsPartitioning() { return bIsPartitioning; }
	sal_uLong GetPartitioningTime();

	void StartAutoProfiling( sal_uLong nMSec );	// Automatisch alle nMSec Milisekunden sampeln
	String GetAutoProfiling();	// Aktuelle `Sammlung` abholen
	void StopAutoProfiling();	// Sampeln beenden
	sal_Bool IsAutoProfiling() { return bIsAutoProfiling; }

private:

	void GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot );

	// Informationszeile zusammenbauen
	String GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pStop );


	ProfileSnapshot *mpStart;
	ProfileSnapshot *mpEnd;
	sal_Bool bIsProfileIntervalStarted;



//
	sal_Bool bIsProfilingPerCommand;
	sal_Bool bIsPartitioning;


//	F�r das Automatische Profiling in festen Intervallen

	ProfileSnapshot *pAutoStart;
	ProfileSnapshot *pAutoEnd;
	sal_Bool bIsAutoProfiling;
	String aAutoProfileBuffer;

	virtual void Timeout();


// Einige Hilfsfunktionen

//	String Hex( sal_uLong nNr );
	String Dec( sal_uLong nNr );	// Ergebnis = nNr / 100 mit 2 Dezimalen
	String Pad( const String aS, xub_StrLen nLen );		// F�gt blanks links an den String an

/*	Ab hier werden die Methoden Systemabh�ngig in den entsprechenden cxx implementiert
	Sie werden von den oberen Methoden gerufen.
*/

	SysdepStaticData *pSysDepStatic;

	void InitSysdepProfiler();
	void DeinitSysdepProfiler();

	SysdepProfileSnapshot *NewSysdepSnapshotData();
	void DeleteSysdepSnapshotData( SysdepProfileSnapshot *pSysdepProfileSnapshot );

	// Titelzeile f�r Logdatei
	String GetSysdepProfileHeader();

	// Zustand merken
	void GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, sal_uInt16 nMode = PROFILE_START | PROFILE_END );

	// Informationszeile zusammenbauen
	String GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop );
};

