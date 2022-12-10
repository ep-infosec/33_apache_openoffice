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



#ifndef _SVDLAYER_HXX
#define _SVDLAYER_HXX

#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx> // fuer typedef SdrLayerID
#include "svx/svxdllapi.h"

class SdrModel;

class SVX_DLLPUBLIC SdrLayer 
{
friend class SdrLayerAdmin;
protected:
	String     aName;
	String		maTitle;
	String		maDescription;
	SdrModel*  pModel; // zum Broadcasten
	sal_uInt16     nType;  // 0=Userdefined,1=Standardlayer
	SdrLayerID nID;
protected:
	SdrLayer(SdrLayerID nNewID, const String& rNewName)       { nID=nNewID; aName=rNewName; nType=0; pModel=NULL; }
	void SetID(SdrLayerID nNewID)                             { nID=nNewID; }
public:
	SdrLayer(): pModel(NULL),nType(0),nID(0)                  {}
	bool      operator==(const SdrLayer& rCmpLayer) const;
	bool      operator!=(const SdrLayer& rCmpLayer) const { return !operator==(rCmpLayer); }

	void          SetName(const String& rNewName);
	const String& GetName() const                             { return aName; }

	void          SetTitle(const String& rTitle) { maTitle = rTitle; }
	const String& GetTitle() const { return maTitle; }

	void          SetDescription(const String& rDesc) { maDescription = rDesc; }
	const String& GetDescription() const { return maDescription; }

	SdrLayerID    GetID() const                               { return nID; }
	void          SetModel(SdrModel* pNewModel)               { pModel=pNewModel; }
	SdrModel*     GetModel() const                            { return pModel; }
	// Einem SdrLayer kann man sagen dass er ein (der) Standardlayer sein soll.
	// Es wird dann laenderspeziefisch der passende Name gesetzt. SetName()
	// setzt das Flag "StandardLayer" ggf. zurueck auf "Userdefined".
	void          SetStandardLayer(FASTBOOL bStd=sal_True);
	FASTBOOL      IsStandardLayer() const                     { return nType==1; }
};

// Beim Aendern von Layerdaten muss man derzeit
// noch selbst das Modify-Flag am Model setzen.
#define SDRLAYER_MAXCOUNT 255
class SVX_DLLPUBLIC SdrLayerAdmin {
friend class SdrView;
friend class SdrModel;
friend class SdrPage;

protected:
	Container      aLayer;
	Container      aLSets;
	SdrLayerAdmin* pParent; // Der Admin der Seite kennt den Admin des Docs
	SdrModel*      pModel; // zum Broadcasten
	String         aControlLayerName;
protected:
	// Eine noch nicht verwendete LayerID raussuchen. Sind bereits alle
	// verbraucht, so gibt's 'ne 0. Wer sicher gehen will, muss vorher
	// GetLayerCount()<SDRLAYER_MAXCOUNT abfragen, denn sonst sind alle
	// vergeben.
	SdrLayerID           GetUniqueLayerID() const;
	void                 Broadcast() const;
public:
	SdrLayerAdmin(SdrLayerAdmin* pNewParent=NULL);
	SdrLayerAdmin(const SdrLayerAdmin& rSrcLayerAdmin);
	~SdrLayerAdmin();
	const SdrLayerAdmin& operator=(const SdrLayerAdmin& rSrcLayerAdmin);
	bool             operator==(const SdrLayerAdmin& rCmpLayerAdmin) const;
	bool             operator!=(const SdrLayerAdmin& rCmpLayerAdmin) const       { return !operator==(rCmpLayerAdmin); }
	SdrLayerAdmin*       GetParent() const                                           { return pParent; }
	void                 SetParent(SdrLayerAdmin* pNewParent)                        { pParent=pNewParent; }
	void                 SetModel(SdrModel* pNewModel);
	SdrModel*            GetModel() const                                            { return pModel; }
	void                 InsertLayer(SdrLayer* pLayer, sal_uInt16 nPos=0xFFFF)           { aLayer.Insert(pLayer,nPos); pLayer->SetModel(pModel); Broadcast(); }
	SdrLayer*            RemoveLayer(sal_uInt16 nPos);
	// Alle Layer loeschen
	void               ClearLayer();
	// Neuer Layer wird angelegt und eingefuegt
	SdrLayer*          NewLayer(const String& rName, sal_uInt16 nPos=0xFFFF);
	void               DeleteLayer(SdrLayer* pLayer)                                 { aLayer.Remove(pLayer); delete pLayer; Broadcast(); }
	void               MoveLayer(SdrLayer* pLayer, sal_uInt16 nNewPos=0xFFFF);
	SdrLayer*          MoveLayer(sal_uInt16 nPos, sal_uInt16 nNewPos);
	// Neuer Layer, Name wird aus der Resource geholt
	SdrLayer*          NewStandardLayer(sal_uInt16 nPos=0xFFFF);

	// Iterieren ueber alle Layer
	sal_uInt16             GetLayerCount() const                                         { return sal_uInt16(aLayer.Count()); }
	SdrLayer*          GetLayer(sal_uInt16 i)                                            { return (SdrLayer*)(aLayer.GetObject(i)); }
	const SdrLayer*    GetLayer(sal_uInt16 i) const                                      { return (SdrLayer*)(aLayer.GetObject(i)); }

	sal_uInt16             GetLayerPos(SdrLayer* pLayer) const;

	SdrLayer*          GetLayer(const String& rName, FASTBOOL bInherited)            { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayer(rName,bInherited)); }
	const SdrLayer*    GetLayer(const String& rName, FASTBOOL bInherited) const;
		  SdrLayerID   GetLayerID(const String& rName, FASTBOOL bInherited) const;
		  SdrLayer*    GetLayerPerID(sal_uInt16 nID)                                     { return (SdrLayer*)(((const SdrLayerAdmin*)this)->GetLayerPerID(nID)); }
	const SdrLayer*    GetLayerPerID(sal_uInt16 nID) const;

	void     	       SetControlLayerName(const String& rNewName) { aControlLayerName=rNewName; }
	const String& 	   GetControlLayerName() const                 { return aControlLayerName; }
};

/*
Anmerkung zu den Layer - Gemischt symbolisch/ID-basierendes Interface
	Einen neuen Layer macht man sich mit:
	  pLayerAdmin->NewLayer("Der neue Layer");
	Der Layer wird dann automatisch an das Ende der Liste angehaengt.
	Entsprechdes gilt fuer Layersets gleichermassen.
	Das Interface am SdrLayerSet basiert auf LayerID's. Die App muss sich
	dafuer am SdrLayerAdmin eine ID abholen:
		SdrLayerID nLayerID=pLayerAdmin->GetLayerID("Der neue Layer");
	Wird der Layer nicht gefunden, so liefert die Methode SDRLAYER_NOTFOUND
	zurueck. Die Methoden mit ID-Interface fangen diesen Wert jedoch i.d.R
	sinnvoll ab.
	Hat man nicht nur den Namen, sondern gar einen SdrLayer*, so kann man
	sich die ID natuerlich wesentlich schneller direkt vom Layer abholen.
bInherited:
	TRUE: Wird der Layer/LayerSet nicht gefunden, so wird im Parent-LayerAdmin
		  nachgesehen, ob es dort einen entsprechende Definition gibt.
	FALSE: Es wird nur dieser LayerAdmin durchsucht.
	Jeder LayerAdmin einer Seite hat einen Parent-LayerAdmin, n�mlich den des
	Model. Das Model selbst hat keinen Parent.
*/

#endif //_SVDLAYER_HXX

