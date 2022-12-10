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



#ifndef _SFXHTML_HXX
#define _SFXHTML_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <i18npool/lang.h>
#include <svtools/parhtml.hxx>
#include <svl/macitem.hxx>


class ImageMap;
class SfxMedium;
class SfxObjectShell;

class SFX2_DLLPUBLIC SfxHTMLParser : public HTMLParser
{
	DECL_DLLPRIVATE_STATIC_LINK( SfxHTMLParser, FileDownloadDone, void* );

	String					aScriptType;

	SfxMedium*				pMedium;
	SfxMedium *pDLMedium;	// Medium fuer Download von Files

	sal_uInt16 nMetaTags;		// Anzahl der bisher gelesenen Meta-Tags
	ScriptType eScriptType;

	SAL_DLLPRIVATE void GetScriptType_Impl( SvKeyValueIterator* );

protected:

	SfxHTMLParser( SvStream& rStream, sal_Bool bNewDoc=sal_True, SfxMedium *pMedium=0 );

    virtual ~SfxHTMLParser();

public:
	// Lesen der Optionen einer Image-Map
	// <MAP>: sal_True = Image-Map hat einen Namen
	// <AREA>: sal_True = Image-Map hat jetzt einen Bereich mehr
	static sal_Bool ParseMapOptions(ImageMap * pImageMap,
								const HTMLOptions * pOptions );
	sal_Bool ParseMapOptions(ImageMap * pImageMap)
	{ return ParseMapOptions(pImageMap, GetOptions()); }
    static sal_Bool ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
								 const HTMLOptions * pOptions,
								 sal_uInt16 nEventMouseOver = 0,
								 sal_uInt16 nEventMouseOut = 0 );
    inline sal_Bool ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
								 sal_uInt16 nEventMouseOver = 0,
								 sal_uInt16 nEventMouseOut = 0);

	// <TD SDVAL="..." SDNUM="...">
	static double GetTableDataOptionsValNum( sal_uInt32& nNumForm,
			LanguageType& eNumLang, const String& aValStr,
			const String& aNumStr, SvNumberFormatter& rFormatter );

protected:

	// Start eines File-Downloads. Dieser erfolgt synchron oder asynchron.
	// Im synchronen Fall befindet sich der Parser nach dem Aufruf im
	// Working-Zustand. Die gelesene Datei kann dann direkt mit
	// FinishFileDownload abgeholt werden.
	// Im asynchronen Fall befindet sich der Parser nach dem Aufruf im
	// Pending-Zustand. Der Parser muss dann ueber das Continue verlassen
	// werden (ohne Reschedule!). Wenn die Datei geladen ist, wird
	// ein Continue mit dem uebergebenen Token aufgerufen. Die Datei kann
	// dann wiederum mit FinishFileDownload abgeholt werden.
	// Zum Abbrechen des Dwonloads sollte eine Shell uebergeben werden.
	// Es kann nur ein einziger Download gleichzeitig existieren. Fuer jeden
	// gestarteten Download muss FinshFileDownload aufgerufen werden.
	void StartFileDownload( const String& rURL, int nToken,
							SfxObjectShell *pSh=0 );

	// Ermittelnd des MIME-Types eines zuvor downloadeten Files. Kann nur
	// unmittelbar vor FinishFileDownload aufgerufen werden, nie aber
	// danach.

	sal_Bool GetFileDownloadMIME( String& rMime );

	// Beenden eines asynchronen File-Downloads. Gibt sal_True zurueck, wenn
	// der Download geklappt hat. Das gelesene File befindet sich dann in
	// dem uebergeben String.
	sal_Bool FinishFileDownload( String& rStr );

	// Gibt sal_True zurueck, wenn ein File downloaded wurde und
	// FileDownloadFinished noch nicht gerufen wurde.
	sal_Bool ShouldFinishFileDownload() const { return pDLMedium != 0; }

	SfxMedium *GetMedium() { return pMedium; }
	const SfxMedium *GetMedium() const { return pMedium; }

	// Default (auch ohne Iterator) ist JavaScript
	ScriptType GetScriptType( SvKeyValueIterator* ) const;
	const String& GetScriptTypeString( SvKeyValueIterator* ) const;
};

inline sal_Bool SfxHTMLParser::ParseAreaOptions(ImageMap * pImageMap, const String& rBaseURL,
											sal_uInt16 nEventMouseOver,
											sal_uInt16 nEventMouseOut)
{
    return ParseAreaOptions( pImageMap, rBaseURL, GetOptions(),
							 nEventMouseOver, nEventMouseOut );
}


#endif
