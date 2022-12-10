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


#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#include <vos/refernce.hxx>
#include <vcl/font.hxx>
#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <tools/dynary.hxx>
#include <svl/lstner.hxx>
#include <svl/svarray.hxx>

#include <map>
#include <vector>
#include <set>
#include <functional>
#include <algorithm>

#include "unomodel.hxx"
#include "utility.hxx"
#include "smmod.hxx"


#define SYMBOLSET_NONE  0xFFFF
#define SYMBOL_NONE		0xFFFF


////////////////////////////////////////////////////////////////////////////////

inline const String GetExportSymbolName( const String &rUiSymbolName )
{
    return SM_MOD()->GetLocSymbolData().GetExportSymbolName( rUiSymbolName );
}


inline const String GetUiSymbolName( const String &rExportSymbolName )
{
    return SM_MOD()->GetLocSymbolData().GetUiSymbolName( rExportSymbolName );
}

inline const String GetExportSymbolSetName( const String &rUiSymbolSetName )
{
    return SM_MOD()->GetLocSymbolData().GetExportSymbolSetName( rUiSymbolSetName );
}


inline const String GetUiSymbolSetName( const String &rExportSymbolSetName )
{
    return SM_MOD()->GetLocSymbolData().GetUiSymbolSetName( rExportSymbolSetName );
}

////////////////////////////////////////////////////////////////////////////////

class SmSym
{
    SmFace              m_aFace;
    String              m_aName;
    String              m_aExportName;
    String              m_aSetName;
    sal_UCS4            m_cChar;
    sal_Bool                m_bPredefined;
    sal_Bool                m_bDocSymbol;

public:
    SmSym();
	SmSym(const String& rName, const Font& rFont, sal_UCS4 cChar,
		  const String& rSet, sal_Bool bIsPredefined = sal_False);
    SmSym(const SmSym& rSymbol);

	SmSym&	   	operator = (const SmSym& rSymbol);

    const Font&     GetFace() const { return m_aFace; }
    sal_UCS4        GetCharacter() const { return m_cChar; }
    const String&   GetName() const { return m_aName; }

    void            SetFace( const Font& rFont )        { m_aFace = rFont; }
    void            SetCharacter( sal_UCS4 cChar )   { m_cChar = cChar; }

//! since the symbol name is also used as key in the map it should not be possible to change the name
//! because ten the key would not be the same as its supposed copy here
//    void            SetName( const String &rTxt )       { m_aName = rTxt; }

    sal_Bool            IsPredefined() const        { return m_bPredefined; }
    const String &  GetSymbolSetName() const    { return m_aSetName; }
    void            SetSymbolSetName( const String &rName )     { m_aSetName = rName; }
    const String &  GetExportName() const       { return m_aExportName; }
    void            SetExportName( const String &rName )        { m_aExportName = rName; }

    sal_Bool            IsDocSymbol() const         { return m_bDocSymbol; }
    void            SetDocSymbol( sal_Bool bVal )   { m_bDocSymbol = bVal; }

    // true if rSymbol has the same name, font and character
    bool            IsEqualInUI( const SmSym& rSymbol ) const;
};

/**************************************************************************/

struct lt_String
{
    bool operator()( const String &r1, const String &r2 ) const
    {
        // r1 < r2 ?
        return r1.CompareTo( r2 ) == COMPARE_LESS;
    }
};


// type of the actual container to hold the symbols
typedef std::map< String, SmSym, lt_String >    SymbolMap_t;

// vector of pointers to the actual symbols in the above container
typedef std::vector< const SmSym * >            SymbolPtrVec_t;

struct lt_SmSymPtr : public std::binary_function< const SmSym *, const SmSym *, bool >
{
    bool operator() ( const SmSym *pSym1, const SmSym *pSym2 ) 
    { 
        return pSym1->GetCharacter() < pSym2->GetCharacter();
    }
};


class SmSymbolManager : public SfxListener
{
    SymbolMap_t         m_aSymbols;
    bool                m_bModified;

	virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
						const SfxHint& rHint, const TypeId& rHintType);

    void        Init();
    void        Exit();

public:
    SmSymbolManager();
    SmSymbolManager(const SmSymbolManager& rSymbolSetManager);
    ~SmSymbolManager();

    SmSymbolManager &   operator = (const SmSymbolManager& rSymbolSetManager);

    // symbol sets are for UI purpose only, thus we assemble them here
    std::set< String >      GetSymbolSetNames() const;
    const SymbolPtrVec_t    GetSymbolSet(  const String& rSymbolSetName );

    sal_uInt16                  GetSymbolCount() const  { return static_cast< sal_uInt16 >(m_aSymbols.size()); }
    const SymbolPtrVec_t    GetSymbols() const;
    bool                    AddOrReplaceSymbol( const SmSym & rSymbol, bool bForceChange = false );
    void                    RemoveSymbol( const String & rSymbolName );

	SmSym	    *	GetSymbolByName(const String& rSymbolName);
	const SmSym *	GetSymbolByName(const String& rSymbolName) const
	{
        return ((SmSymbolManager *) this)->GetSymbolByName(rSymbolName);
	}
    
    bool        IsModified() const          { return m_bModified; }
    void        SetModified(bool bModify)   { m_bModified = bModify; }

    void        Load();
    void        Save();
};

#endif

