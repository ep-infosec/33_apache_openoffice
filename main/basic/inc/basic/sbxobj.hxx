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



#ifndef _SBX_SBXOBJECT_HXX
#define _SBX_SBXOBJECT_HXX

#include <svl/lstner.hxx>
#include "basic/basicdllapi.h"
#include <basic/sbxvar.hxx>

///////////////////////////////////////////////////////////////////////////

class SbxProperty;
class SvDispatch;

class SbxObjectImpl;

class BASIC_DLLPUBLIC SbxObject : public SbxVariable, public SfxListener
{
	SbxObjectImpl* mpSbxObjectImpl;	// Impl data

	SbxArray* FindVar( SbxVariable*, sal_uInt16& );
	// AB 23.3.1997, special method for VCPtrRemove (see below)
	SbxArray* VCPtrFindVar( SbxVariable*, sal_uInt16& );
protected:
	SbxArrayRef  pMethods;                  // Methods
	SbxArrayRef  pProps;                    // Properties
	SbxArrayRef  pObjs;                     // Objects
	SbxProperty* pDfltProp;                 // Default-Property
	String       aClassName;                // Classname
	String       aDfltPropName;
	virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
	virtual sal_Bool StoreData( SvStream& ) const;
	virtual ~SbxObject();
	virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
							 const SfxHint& rHint, const TypeId& rHintType );
public:
	SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_OBJECT,1);
	TYPEINFO();
	SbxObject( const String& rClassname );
	SbxObject( const SbxObject& );
	SbxObject& operator=( const SbxObject& );
	virtual SbxDataType GetType() const;
	virtual SbxClassType GetClass() const;
	virtual void Clear();

	virtual sal_Bool  IsClass( const String& ) const;
	const String& GetClassName() const { return aClassName; }
	void		  SetClassName( const String &rNew ) { aClassName = rNew; }
	// Default-Property
	SbxProperty* GetDfltProperty();
	void SetDfltProperty( const String& r );
	void SetDfltProperty( SbxProperty* );
	// Search for an element
	virtual SbxVariable* FindUserData( sal_uInt32 nUserData );
	virtual SbxVariable* Find( const String&, SbxClassType );
	SbxVariable* FindQualified( const String&, SbxClassType );
	// Quick-Call-Interface for Methods
	virtual sal_Bool Call( const String&, SbxArray* = NULL );
	// Execution of DDE-Commands
	SbxVariable* Execute( const String& );
	// Manage elements
	virtual sal_Bool GetAll( SbxClassType ) { return sal_True; }
	SbxVariable* Make( const String&, SbxClassType, SbxDataType, bool bIsRuntimeFunction = false );
	virtual SbxObject* MakeObject( const String&, const String& );
	virtual void Insert( SbxVariable* );
	// AB 23.4.1997, Optimization, Insertion without check for duplicate Entries and 
	// without Broadcasts, only used in SO2/auto.cxx
	void QuickInsert( SbxVariable* );
	// AB 23.3.1997, Special-Method, allow corresponding controls
	void VCPtrInsert( SbxVariable* );
	virtual void Remove( const String&, SbxClassType );
	virtual void Remove( SbxVariable* );
	// AB 23.3.1997, deletion per pointer for controls (duplicate names!)
	void VCPtrRemove( SbxVariable* );
	void SetPos( SbxVariable*, sal_uInt16 );

	// Macro-Recording
	virtual String GenerateSource( const String &rLinePrefix,
								   const SbxObject *pRelativeTo );
	// Direct access on arrays 
	SbxArray* GetMethods()		{ return pMethods;	}
	SbxArray* GetProperties()	{ return pProps; 	}
	SbxArray* GetObjects()		{ return pObjs; 	}
	// Hooks
	virtual SvDispatch* GetSvDispatch();
	// Debugging
	void Dump( SvStream&, sal_Bool bDumpAll=sal_False );

	static void GarbageCollection( sal_uIntPtr nObjects = 0 /* ::= all */ );
};

#ifndef __SBX_SBXOBJECTREF_HXX

#ifndef SBX_OBJECT_DECL_DEFINED
#define SBX_OBJECT_DECL_DEFINED
SV_DECL_REF(SbxObject)
#endif
SV_IMPL_REF(SbxObject)

#endif /* __SBX_SBXOBJECTREF_HXX */
#endif /* _SBX_SBXOBJECT_HXX */
