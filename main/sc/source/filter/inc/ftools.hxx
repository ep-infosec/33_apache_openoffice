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



#ifndef SC_FTOOLS_HXX
#define SC_FTOOLS_HXX

#include <vector>
#include <map>
#include <limits>
#include <memory>
#include <tools/string.hxx>
#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <oox/helper/helper.hxx>
#include "filter.hxx"
#include "scdllapi.h"

// Common macros ==============================================================

/** Expands to a temporary String, created from an ASCII character array. */
#define CREATE_STRING( ascii )      String( RTL_CONSTASCII_USTRINGPARAM( ascii ) )

// items and item sets --------------------------------------------------------

/** Expands to the item (with type 'itemtype') with Which-ID 'which'. */
#define GETITEM( itemset, itemtype, which ) \
    static_cast< const itemtype & >( (itemset).Get( which ) )

/** Expands to the value (with type 'valuetype') of the item with Which-ID 'which'. */
#define GETITEMVALUE( itemset, itemtype, which, valuetype ) \
    static_cast< valuetype >( GETITEM( itemset, itemtype, which ).GetValue() )

/** Expands to the value of the SfxBoolItem with Which-ID 'which'. */
#define GETITEMBOOL( itemset, which ) \
    GETITEMVALUE( itemset, SfxBoolItem, which, bool )

// Global static helpers ======================================================

// Value range limit helpers --------------------------------------------------

/** Returns the value, if it is not less than nMin, otherwise nMin. */
template< typename ReturnType, typename Type >
inline ReturnType llimit_cast( Type nValue, ReturnType nMin )
{ return static_cast< ReturnType >( ::std::max< Type >( nValue, nMin ) ); }

/** Returns the value, if it fits into ReturnType, otherwise the minimum value of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType llimit_cast( Type nValue )
{ return llimit_cast( nValue, ::std::numeric_limits< ReturnType >::min() ); }

/** Returns the value, if it is not greater than nMax, otherwise nMax. */
template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue, ReturnType nMax )
{ return static_cast< ReturnType >( ::std::min< Type >( nValue, nMax ) ); }

/** Returns the value, if it fits into ReturnType, otherwise the maximum value of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType ulimit_cast( Type nValue )
{ return ulimit_cast( nValue, ::std::numeric_limits< ReturnType >::max() ); }

/** Returns the value, if it is not less than nMin and not greater than nMax, otherwise one of the limits. */
template< typename ReturnType, typename Type >
inline ReturnType limit_cast( Type nValue, ReturnType nMin, ReturnType nMax )
{ return static_cast< ReturnType >( ::std::max< Type >( ::std::min< Type >( nValue, nMax ), nMin ) ); }

/** Returns the value, if it fits into ReturnType, otherwise one of the limits of ReturnType. */
template< typename ReturnType, typename Type >
inline ReturnType limit_cast( Type nValue )
{ return limit_cast( nValue, ::std::numeric_limits< ReturnType >::min(), ::std::numeric_limits< ReturnType >::max() ); }

// Read from bitfields --------------------------------------------------------

/** Returns true, if at least one of the bits set in nMask is set in nBitField. */
template< typename Type >
inline bool get_flag( Type nBitField, Type nMask )
{ return (nBitField & nMask) != 0; }

/** Returns nSet, if at least one bit of nMask is set in nBitField, otherwise nUnset. */
template< typename ReturnType, typename Type >
inline ReturnType get_flagvalue( Type nBitField, Type nMask, ReturnType nSet, ReturnType nUnset )
{ return ::get_flag( nBitField, nMask ) ? nSet : nUnset; }

/** Extracts a value from a bit field.
    @descr  Returns in rnRet the data fragment from nBitField, that starts at bit nStartBit
    (0-based, bit 0 is rightmost) with the width of nBitCount. rnRet will be right-aligned (normalized).
    For instance: extract_value( n, 0x4321, 8, 4 ) stores 3 in n (value in bits 8-11). */
template< typename ReturnType, typename Type >
inline ReturnType extract_value( Type nBitField, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{ return static_cast< ReturnType >( ((1UL << nBitCount) - 1) & (nBitField >> nStartBit) ); }

// Write to bitfields ---------------------------------------------------------

/** Sets or clears (according to bSet) all set bits of nMask in rnBitField. */
template< typename Type >
inline void set_flag( Type& rnBitField, Type nMask, bool bSet = true )
{ if( bSet ) rnBitField |= nMask; else rnBitField &= ~nMask; }

/** Inserts a value into a bitfield.
    @descr  Inserts the lower nBitCount bits of nValue into rnBitField, starting
    there at bit nStartBit. Other contents of rnBitField keep unchanged. */
template< typename Type, typename InsertType >
void insert_value( Type& rnBitField, InsertType nValue, sal_uInt8 nStartBit, sal_uInt8 nBitCount )
{
    unsigned long nMask = ((1UL << nBitCount) - 1);
    Type nNewValue = static_cast< Type >( nValue & nMask );
    (rnBitField &= ~(nMask << nStartBit)) |= (nNewValue << nStartBit);
}

// ============================================================================

/** Deriving from this class prevents copy construction. */
class ScfNoCopy
{
private:
                        ScfNoCopy( const ScfNoCopy& );
    ScfNoCopy&          operator=( const ScfNoCopy& );
protected:
    inline              ScfNoCopy() {}
};

// ----------------------------------------------------------------------------

/** Deriving from this class prevents construction in general. */
class ScfNoInstance : private ScfNoCopy {};

// ============================================================================

/** Simple shared pointer (NOT thread-save, but faster than boost::shared_ptr). */
template< typename Type >
class ScfRef
{
    template< typename > friend class ScfRef;

public:
    typedef Type        element_type;
    typedef ScfRef      this_type;

    inline explicit     ScfRef( element_type* pObj = 0 ) { eat( pObj ); }
    inline /*implicit*/ ScfRef( const this_type& rRef ) { eat( rRef.mpObj, rRef.mpnCount ); }
    template< typename Type2 >
    inline /*implicit*/ ScfRef( const ScfRef< Type2 >& rRef ) { eat( rRef.mpObj, rRef.mpnCount ); }
    inline              ~ScfRef() { rel(); }

    inline void         reset( element_type* pObj = 0 ) { rel(); eat( pObj ); }
    inline this_type&   operator=( const this_type& rRef ) { if( this != &rRef ) { rel(); eat( rRef.mpObj, rRef.mpnCount ); } return *this; }
    template< typename Type2 >
    inline this_type&   operator=( const ScfRef< Type2 >& rRef ) { rel(); eat( rRef.mpObj, rRef.mpnCount ); return *this; }

    inline element_type* get() const { return mpObj; }
    inline bool         is() const { return mpObj != 0; }

    inline element_type* operator->() const { return mpObj; }
    inline element_type& operator*() const { return *mpObj; }

    inline bool         operator!() const { return mpObj == 0; }

private:
    inline void         eat( element_type* pObj, size_t* pnCount = 0 ) { mpObj = pObj; mpnCount = mpObj ? (pnCount ? pnCount : new size_t( 0 )) : 0; if( mpnCount ) ++*mpnCount; }
    inline void         rel() { if( mpnCount && !--*mpnCount ) { DELETEZ( mpObj ); DELETEZ( mpnCount ); } }

private:
    Type*               mpObj;
    size_t*             mpnCount;
};

template< typename Type >
inline bool operator==( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() == rxRef2.get();
}

template< typename Type >
inline bool operator!=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() != rxRef2.get();
}

template< typename Type >
inline bool operator<( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() < rxRef2.get();
}

template< typename Type >
inline bool operator>( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() > rxRef2.get();
}

template< typename Type >
inline bool operator<=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() <= rxRef2.get();
}

template< typename Type >
inline bool operator>=( const ScfRef< Type >& rxRef1, const ScfRef< Type >& rxRef2 )
{
    return rxRef1.get() >= rxRef2.get();
}

// ----------------------------------------------------------------------------

/** Template for a map of ref-counted objects with additional accessor functions. */
template< typename KeyType, typename ObjType >
class ScfRefMap : public ::std::map< KeyType, ScfRef< ObjType > >
{
public:
    typedef KeyType                             key_type;
    typedef ScfRef< ObjType >                   ref_type;
    typedef ::std::map< key_type, ref_type >    map_type;

    /** Returns true, if the object accossiated to the passed key exists. */
    inline bool         has( key_type nKey ) const
                        {
                            typename map_type::const_iterator aIt = this->find( nKey );
                            return (aIt != this->end()) && aIt->second.is();
                        }

    /** Returns a reference to the object accossiated to the passed key, or 0 on error. */
    inline ref_type     get( key_type nKey ) const
                        {
                            typename map_type::const_iterator aIt = this->find( nKey );
                            if( aIt != this->end() ) return aIt->second;
                            return ref_type();
                        }
};

// ============================================================================

class Color;
class SfxPoolItem;
class SfxItemSet;
class ScStyleSheet;
class ScStyleSheetPool;
class SotStorage;
class SotStorageRef;
class SotStorageStreamRef;
class SvStream;

/** Contains static methods used anywhere in the filters. */
class ScfTools : ScfNoInstance
{
public:

// *** common methods *** -----------------------------------------------------

    /** Reads a 10-byte-long-double and converts it to double. */
    static double       ReadLongDouble( SvStream& rStrm );
    /** Returns system text encoding for byte string conversion. */
    static rtl_TextEncoding GetSystemTextEncoding();
    /** Returns a string representing the hexadecimal value of nValue. */
    static String       GetHexStr( sal_uInt16 nValue );

    /** Mixes RGB components with given transparence.
        @param nTrans  Foreground transparence (0x00 == full nFore ... 0x80 = full nBack). */
    static sal_uInt8    GetMixedColorComp( sal_uInt8 nFore, sal_uInt8 nBack, sal_uInt8 nTrans );
    /** Mixes colors with given transparence.
        @param nTrans  Foreground transparence (0x00 == full rFore ... 0x80 = full rBack). */
    static Color        GetMixedColor( const Color& rFore, const Color& rBack, sal_uInt8 nTrans );

// *** conversion of names *** ------------------------------------------------

    /** Converts a string to a valid Calc defined name or database range name.
        @descr  Defined names in Calc may contain letters, digits (*), underscores, periods (*),
        colons (*), question marks, and dollar signs.
        (*) = not allowed at first position. */
    static void         ConvertToScDefinedName( String& rName );

// *** streams and storages *** -----------------------------------------------

    /** Tries to open an existing storage with the specified name in the passed storage (read-only). */
    static SotStorageRef OpenStorageRead( SotStorageRef xStrg, const String& rStrgName );
    /** Creates and opens a storage with the specified name in the passed storage (read/write). */
    static SotStorageRef OpenStorageWrite( SotStorageRef xStrg, const String& rStrgName );

    /** Tries to open an existing stream with the specified name in the passed storage (read-only). */
    static SotStorageStreamRef OpenStorageStreamRead( SotStorageRef xStrg, const String& rStrmName );
    /** Creates and opens a stream with the specified name in the passed storage (read/write). */
    static SotStorageStreamRef OpenStorageStreamWrite( SotStorageRef xStrg, const String& rStrmName );

// *** item handling *** ------------------------------------------------------

    /** Returns true, if the passed item set contains the item.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItem( const SfxItemSet& rItemSet, sal_uInt16 nWhichId, bool bDeep );
    /** Returns true, if the passed item set contains at least one of the items.
        @param pnWhichIds  Zero-terminated array of Which-IDs.
        @param bDeep  true = Searches in parent item sets too. */
    static bool         CheckItems( const SfxItemSet& rItemSet, const sal_uInt16* pnWhichIds, bool bDeep );

    /** Puts the item into the passed item set.
        @descr  The item will be put into the item set, if bSkipPoolDef is false,
        or if the item differs from the default pool item.
        @param rItemSet  The destination item set.
        @param rItem  The item to put into the item set.
        @param nWhichId  The Which-ID to set with the item.
        @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
    static void         PutItem(
                            SfxItemSet& rItemSet, const SfxPoolItem& rItem,
                            sal_uInt16 nWhichId, bool bSkipPoolDef );

    /** Puts the item into the passed item set.
        @descr  The item will be put into the item set, if bSkipPoolDef is false,
        or if the item differs from the default pool item.
        @param rItemSet  The destination item set.
        @param rItem  The item to put into the item set.
        @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
    static void         PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, bool bSkipPoolDef );

// *** style sheet handling *** -----------------------------------------------

    /** Creates and returns a cell style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet& MakeCellStyleSheet(
                            ScStyleSheetPool& rPool,
                            const String& rStyleName, bool bForceName );
    /** Creates and returns a page style sheet and inserts it into the pool.
        @descr  If the style sheet is already in the pool, another unused style name is used.
        @param bForceName  Controls behaviour, if the style already exists:
        true = Old existing style will be renamed; false = New style gets another name. */
    static ScStyleSheet& MakePageStyleSheet(
                            ScStyleSheetPool& rPool,
                            const String& rStyleName, bool bForceName );

// *** byte string import operations *** --------------------------------------

    /** Reads and returns a zero terminted byte string. */
    static ByteString   ReadCString( SvStream& rStrm );
    /** Reads and returns a zero terminted byte string. */
    inline static String ReadCString( SvStream& rStrm, rtl_TextEncoding eTextEnc )
                            { return String( ReadCString( rStrm ), eTextEnc ); }

    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    static ByteString   ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft );
    /** Reads and returns a zero terminted byte string and decreases a stream counter. */
    inline static String ReadCString( SvStream& rStrm, sal_Int32& rnBytesLeft, rtl_TextEncoding eTextEnc )
                            { return String( ReadCString( rStrm, rnBytesLeft ), eTextEnc ); }

    /** Appends a zero terminted byte string.
        @param nLen
               The previous length of the string, usually rString.Len(), but 
               necessary as this may be called from within AppendCString() 
               where rString is a temporary ByteString to be appended to 
               UniString. */
    static void         AppendCStringWithLen( SvStream& rStrm, ByteString& rString, sal_uInt32 nLen );
    /** Appends a zero terminted byte string. */
    static void         AppendCString( SvStream& rStrm, String& rString, rtl_TextEncoding eTextEnc );

// *** HTML table names <-> named range names *** -----------------------------

    /** Returns the built-in range name for an HTML document. */
    static const String& GetHTMLDocName();
    /** Returns the built-in range name for all HTML tables. */
    static const String& GetHTMLTablesName();
    /** Returns the built-in range name for an HTML table, specified by table index. */
    static String       GetNameFromHTMLIndex( sal_uInt32 nIndex );
    /** Returns the built-in range name for an HTML table, specified by table name. */
    static String       GetNameFromHTMLName( const String& rTabName );

    /** Returns true, if rSource is the built-in range name for an HTML document. */
    static bool         IsHTMLDocName( const String& rSource );
    /** Returns true, if rSource is the built-in range name for all HTML tables. */
    static bool         IsHTMLTablesName( const String& rSource );
    /** Converts a built-in range name to an HTML table name.
        @param rSource  The string to be determined.
        @param rName  The HTML table name.
        @return  true, if conversion was successful. */
    static bool         GetHTMLNameFromName( const String& rSource, String& rName );

private:
    /** Returns the prefix for table index names. */
    static const String& GetHTMLIndexPrefix();
    /** Returns the prefix for table names. */
    static const String& GetHTMLNamePrefix();
};

// Containers =================================================================

typedef ::std::vector< sal_uInt8 >                  ScfUInt8Vec;
typedef ::std::vector< sal_Int16 >                  ScfInt16Vec;
typedef ::std::vector< sal_uInt16 >                 ScfUInt16Vec;
typedef ::std::vector< sal_Int32 >                  ScfInt32Vec;
typedef ::std::vector< sal_uInt32 >                 ScfUInt32Vec;
typedef ::std::vector< sal_Int64 >                  ScfInt64Vec;
typedef ::std::vector< sal_uInt64 >                 ScfUInt64Vec;
typedef ::std::vector< String >                     ScfStringVec;

// ----------------------------------------------------------------------------

/** Template for a list that owns the contained objects.
    @descr  This list stores pointers to objects and deletes the objects itself
    on destruction. The Clear() method deletes all objects too. */
template< typename Type > class ScfDelList
{
public:
    inline explicit     ScfDelList( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 ) :
                            maList( nInitSize, nResize ) {}
    /** Creates a deep copy of the passed list (copy-constructs all contained objects). */
    inline explicit     ScfDelList( const ScfDelList& rSrc ) { *this = rSrc; }
    virtual             ~ScfDelList();

    /** Creates a deep copy of the passed list (copy-constructs all contained objects). */
    ScfDelList&         operator=( const ScfDelList& rSrc );

    inline void         Insert( Type* pObj, sal_uLong nIndex )      { if( pObj ) maList.Insert( pObj, nIndex ); }
    inline void         Append( Type* pObj )                    { if( pObj ) maList.Insert( pObj, LIST_APPEND ); }
    /** Removes the object without deletion. */
    inline Type*        Remove( sal_uLong nIndex )                  { return static_cast< Type* >( maList.Remove( nIndex ) ); }
    /** Removes and deletes the object. */
    inline void         Delete( sal_uLong nIndex )                  { delete Remove( nIndex ); }
    /** Exchanges the contained object with the passed, returns the old. */
    inline Type*        Exchange( Type* pObj, sal_uLong nIndex )    { return static_cast< Type* >( maList.Replace( pObj, nIndex ) ); }
    /** Replaces (deletes) the contained object. */
    inline void         Replace( Type* pObj, sal_uLong nIndex )     { delete Exchange( pObj, nIndex ); }

    void                Clear();
    inline sal_uLong        Count() const                           { return maList.Count(); }
    inline bool         Empty() const                           { return maList.Count() == 0; }

    inline Type*        GetCurObject() const                    { return static_cast< Type* >( maList.GetCurObject() ); }
    inline sal_uLong        GetCurPos() const                       { return maList.GetCurPos(); }
    inline Type*        GetObject( sal_uInt32 nIndex ) const    { return static_cast< Type* >( maList.GetObject( nIndex ) ); }

    inline Type*        First() const                           { return static_cast< Type* >( maList.First() ); }
    inline Type*        Last() const                            { return static_cast< Type* >( maList.Last() ); }
    inline Type*        Next() const                            { return static_cast< Type* >( maList.Next() ); }
    inline Type*        Prev() const                            { return static_cast< Type* >( maList.Prev() ); }

private:
    mutable List        maList;     /// The base container object.
};

template< typename Type > ScfDelList< Type >& ScfDelList< Type >::operator=( const ScfDelList& rSrc )
{
    Clear();
    for( const Type* pObj = rSrc.First(); pObj; pObj = rSrc.Next() )
        Append( new Type( *pObj ) );
    return *this;
}

template< typename Type > ScfDelList< Type >::~ScfDelList()
{
    Clear();
}

template< typename Type > void ScfDelList< Type >::Clear()
{
    for( Type* pObj = First(); pObj; pObj = Next() )
        delete pObj;
    maList.Clear();
}

// ----------------------------------------------------------------------------

/** Template for a stack that owns the contained objects.
    @descr  This stack stores pointers to objects and deletes the objects
    itself on destruction. The Clear() method deletes all objects too.
    The Pop() method removes the top object from stack without deletion. */
template< typename Type >
class ScfDelStack : private ScfDelList< Type >
{
public:
    inline              ScfDelStack( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 ) :
                            ScfDelList< Type >( nInitSize, nResize ) {}

    inline void         Push( Type* pObj )      { Append( pObj ); }
    /** Removes the top object without deletion. */
    inline Type*        Pop()                   { return Remove( Count() - 1 ); }

    inline Type*        Top() const             { return GetObject( Count() - 1 ); }

    using               ScfDelList< Type >::Clear;
    using               ScfDelList< Type >::Count;
    using               ScfDelList< Type >::Empty;
};

// ----------------------------------------------------------------------------
class ScFormatFilterPluginImpl : public ScFormatFilterPlugin {
  public:
    ScFormatFilterPluginImpl();
    // various import filters
    virtual FltError ScImportLotus123( SfxMedium&, ScDocument*, CharSet eSrc = RTL_TEXTENCODING_DONTKNOW );
    virtual FltError ScImportQuattroPro( SfxMedium &rMedium, ScDocument *pDoc );
    virtual FltError ScImportExcel( SfxMedium&, ScDocument*, const EXCIMPFORMAT );
        // eFormat == EIF_AUTO	-> passender Filter wird automatisch verwendet
        // eFormat == EIF_BIFF5	-> nur Biff5-Stream fuehrt zum Erfolg (auch wenn in einem Excel97-Doc)
        // eFormat == EIF_BIFF8	-> nur Biff8-Stream fuehrt zum Erfolg (nur in Excel97-Docs)
        // eFormat == EIF_BIFF_LE4 -> nur Nicht-Storage-Dateien _koennen_ zum Erfolg fuehren
    virtual FltError ScImportStarCalc10( SvStream&, ScDocument* );
    virtual FltError ScImportDif( SvStream&, ScDocument*, const ScAddress& rInsPos,
				 const CharSet eSrc = RTL_TEXTENCODING_DONTKNOW, sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScImportRTF( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange );
    virtual FltError ScImportHTML( SvStream&, const String& rBaseURL, ScDocument*, ScRange& rRange, 
                                   double nOutputFactor = 1.0, sal_Bool bCalcWidthHeight = sal_True, 
                                   SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true );

    virtual ScEEAbsImport *CreateRTFImport( ScDocument* pDoc, const ScRange& rRange );
    virtual ScEEAbsImport *CreateHTMLImport( ScDocument* pDocP, const String& rBaseURL, const ScRange& rRange, sal_Bool bCalcWidthHeight );
    virtual String         GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName );

    // various export filters
#if ENABLE_LOTUS123_EXPORT
    virtual FltError ScExportLotus123( SvStream&, ScDocument*, ExportFormatLotus, CharSet eDest );
#endif
    virtual FltError ScExportExcel5( SfxMedium&, ScDocument*, ExportFormatExcel eFormat, CharSet eDest );
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScAddress& rOutPos, const CharSet eDest,
                                 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScExportDif( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest,
				 sal_uInt32 nDifOption = SC_DIFOPT_EXCEL );
    virtual FltError ScExportHTML( SvStream&, const String& rBaseURL, ScDocument*, const ScRange& rRange, const CharSet eDest, sal_Bool bAll,
				  const String& rStreamPath, String& rNonConvertibleChars );
    virtual FltError ScExportRTF( SvStream&, ScDocument*, const ScRange& rRange, const CharSet eDest );
};

// ============================================================================

#endif
