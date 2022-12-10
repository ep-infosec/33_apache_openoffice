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

#ifndef _SFXMACRO_HXX
#define _SFXMACRO_HXX

//====================================================================
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/string.hxx>

class SfxSlot;
class SfxShell;
struct SfxMacro_Impl;
class SfxMacro;

//====================================================================

class SfxMacroStatement
{
	sal_uInt16				nSlotId;	// ausgef"uhrte Slot-Id oder 0, wenn manuell
    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > aArgs;      // aktuelle Parameter, falls nSlotId != 0
	String				aStatement; // Statement in BASIC-Syntax (ggf. mit CR/LF)
	sal_Bool				bDone;  	// auskommentieren wenn kein Done() gerufen
	void*				pDummy;		// f"ur alle F"alle zum kompatibel bleiben

#ifdef _SFXMACRO_HXX
private:
	void				GenerateNameAndArgs_Impl( SfxMacro *pMacro,
												  const SfxSlot &rSlot,
												  sal_Bool bRequestDone,
                                                  ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );
#endif

public:
						SfxMacroStatement( const SfxMacroStatement &rOrig );

						SfxMacroStatement( const String &rTarget,
										   const SfxSlot &rSlot,
										   sal_Bool bRequestDone,
                                           ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );

						SfxMacroStatement( const SfxShell &rShell,
										   const String &rTarget,
										   sal_Bool bAbsolute,
										   const SfxSlot &rSlot,
										   sal_Bool bRequestDone,
                                           ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aArgs );

						SfxMacroStatement( const String &rStatment );
						~SfxMacroStatement();

	sal_uInt16				GetSlotId() const;
    const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& GetArgs() const;
	sal_Bool				IsDone() const;
	const String&		GetStatement() const;
};

//--------------------------------------------------------------------

inline sal_uInt16 SfxMacroStatement::GetSlotId() const

/*  [Beschreibung]

	Liefert die Slot-Id die das Statement beim Abspielen wieder ausf"uhren
	soll oder 0, falls das Statement manuell (<SFX_SLOT_RECORDMANUAL>)
	aufgezeichnet wurde.
*/

{
	return nSlotId;
}

//--------------------------------------------------------------------

inline const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& SfxMacroStatement::GetArgs() const

/*  [Beschreibung]

	Liefert die Parameter mit denen Statement ausgef"uhrt wurde oder 0,
	falls das Statement	manuell (<SFX_SLOT_RECORDMANUAL>) aufgezeichnet
	wurde.

	Der R"uckgabewert geh"ort dem SfxMacroStatement und ist nur im
	unmittelbar aufrufenden Stackframe g"ultig.
*/

{
    return aArgs;
}

//--------------------------------------------------------------------

inline sal_Bool	SfxMacroStatement::IsDone() const

/*  [Beschreibung]

	Liefert TRUE, wenn das Statement wirklich ausgef"uhrt wurde,
	also z.B. nicht vom Benutzer abgebrochen wurde. Wurde es nicht
	wirklich ausgef"uhrt, dann wird es im BASIC-Source auskommentiert.
*/

{
	return bDone;
}

//--------------------------------------------------------------------

/*  [Beschreibung]

	Liefert das Statement in BASIC-Syntax. Wurde das Makro manuell erzeugt,
	wird genau der im Konstruktor angegebene String zur"uckgegeben, sonst
	der generierte Source-Code.

	Bei beiden Arten ist es m"oglich, da\s mehrere BASIC-Statements, jeweils
	mit CR/LF getrennt in dem String enthalten sind, da ein SFx-Statement
	ggf. in mehrere BASIC-Statements "ubersetzt wird.

	Statements f"ur die nicht <SfxRequest::Done()> gerufen wurde, werden
	mit einem vorangestellten 'rem' gekennzeichnet.
*/

inline const String& SfxMacroStatement::GetStatement() const
{
	return aStatement;
}

//====================================================================

enum SfxMacroMode

/*	[Beschreibung]

	Mit diesem enum wird bestimmt, ob eine <SfxMacro>-Instanz zum
	absoluten oder relativen Recorden erzeugt wurde, oder um ein
	existierendendes Makro zu Referenzieren.
*/

{
	SFX_MACRO_EXISTING,			/*	es handelt sich um ein bereits
									exitistierendes Makro, welches lediglich
									referenziert wird */

	SFX_MACRO_RECORDINGABSOLUTE,/*	dieses Makro soll aufgezeichnet werden,
									wobei die betroffenen Objekte m"oglichst
									direkt angesprochen werden sollen
									(Beispiel: "[doc.sdc]") */

	SFX_MACRO_RECORDINGRELATIVE/*   dieses Makro soll aufgezeichnet werden,
									wobei die betroffenen Objekte so
									angesprochen werden sollen, da\s sich das
									Abspielen auf die dann g"ultige Selektion
									bezieht (Beispiel: "ActiveDocument()") */
};

//====================================================================

class SfxMacro

/*	[Beschreibung]

	"Uber diese Klasse (bzw. genaugenommen ihre Subklassen) wird zum
	einen die Lokation einer BASIC-Funktion (also in welcher Library,
	in welchem Modul sowie der Funktions-Name) beschrieben, als auch
	ein aufzuzeichnendes Makro w"ahrend der Aufzeichnung zwischen-
	gespeichert.
*/

{
	SfxMacro_Impl*			pImp;

public:
                            SfxMacro( SfxMacroMode eMode = SFX_MACRO_RECORDINGRELATIVE );
	virtual 				~SfxMacro();

	SfxMacroMode			GetMode() const;
	void					Record( SfxMacroStatement *pStatement );
	void					Replace( SfxMacroStatement *pStatement );
	void					Remove();
	const SfxMacroStatement*GetLastStatement() const;

	String					GenerateSource() const;
};

#endif
