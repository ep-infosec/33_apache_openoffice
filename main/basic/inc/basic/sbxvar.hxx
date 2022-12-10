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



#ifndef _SBXVAR_HXX
#define _SBXVAR_HXX

#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <basic/sbxcore.hxx>
#include "basic/basicdllapi.h"

#ifndef __SBX_64
#define __SBX_64

struct BASIC_DLLPUBLIC SbxINT64
{
	sal_Int32 nHigh; sal_uInt32 nLow;

#if FALSE
	SbxINT64()           : nHigh( 0 ), nLow( 0 ) {}
	SbxINT64( sal_uInt8  n ) : nHigh( 0 ), nLow( n ) {}
	SbxINT64( sal_uInt16 n ) : nHigh( 0 ), nLow( n ) {}
	SbxINT64( sal_uInt32 n ) : nHigh( 0 ), nLow( n ) {}
	SbxINT64( unsigned int n ) : nHigh( 0 ), nLow( n ) {}
	SbxINT64( sal_Int8   n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
	SbxINT64( sal_Int16  n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
	SbxINT64( sal_Int32  n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
	SbxINT64( int    n ) : nHigh( n < 0 ? -1 : 0 ), nLow( n ) {}
	SbxINT64( SbxINT64 &r ) : nHigh( r.nHigh ), nLow( r.nLow ) {}

	SbxINT64( BigInt &r );
	SbxINT64( double n );
#endif
	void CHS()
	{
		nLow  ^= (sal_uInt32)-1;
		nHigh ^= -1;
		nLow++;
		if( !nLow )
			nHigh++;
	}

	// blc/os2i do not like operator =
	void Set(double n)
	{
		if( n >= 0 )
		{
			nHigh = (sal_Int32)(n / (double)4294967296.0);
			nLow  = (sal_uInt32)(n - ((double)nHigh * (double)4294967296.0) + 0.5);
		}
		else {
			nHigh = (sal_Int32)(-n / (double)4294967296.0);
			nLow  = (sal_uInt32)(-n - ((double)nHigh * (double)4294967296.0) + 0.5);
			CHS();
		}
	}
	void Set(sal_Int32 n) { nHigh = n < 0 ? -1 : 0; nLow = n; }

	void SetMax()  { nHigh = 0x7FFFFFFF; nLow = 0xFFFFFFFF; }
	void SetMin()  { nHigh = 0x80000000; nLow = 0x00000000; }
	void SetNull() { nHigh = 0x00000000; nLow = 0x00000000; }

	int operator ! () const { return !nHigh && !nLow; }

	SbxINT64 &operator -= ( const SbxINT64 &r );
	SbxINT64 &operator += ( const SbxINT64 &r );
	SbxINT64 &operator /= ( const SbxINT64 &r );
	SbxINT64 &operator %= ( const SbxINT64 &r );
	SbxINT64 &operator *= ( const SbxINT64 &r );
	SbxINT64 &operator &= ( const SbxINT64 &r );
	SbxINT64 &operator |= ( const SbxINT64 &r );
	SbxINT64 &operator ^= ( const SbxINT64 &r );

	friend SbxINT64 operator - ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator + ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator / ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator % ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator * ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator & ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator | ( const SbxINT64 &l, const SbxINT64 &r );
	friend SbxINT64 operator ^ ( const SbxINT64 &l, const SbxINT64 &r );

	friend SbxINT64 operator - ( const SbxINT64 &r );
	friend SbxINT64 operator ~ ( const SbxINT64 &r );

	static double GetMin() { return ((double)0x7FFFFFFF*(double)4294967296.0
									 + (double)0xFFFFFFFF)
									/ CURRENCY_FACTOR; }
	static double GetMax() { return ((double)0x80000000*(double)4294967296.0
									 + (double)0xFFFFFFFF)
									/ CURRENCY_FACTOR; }
};

struct BASIC_DLLPUBLIC SbxUINT64
{
	sal_uInt32 nHigh; sal_uInt32 nLow;
	void Set(double n)
	{
		nHigh = (sal_uInt32)(n / (double)4294967296.0);
		nLow  = (sal_uInt32)(n - ((double)nHigh * (double)4294967296.0));
	}

	void Set(sal_uInt32 n) { nHigh = 0; nLow = n; }

	void SetMax()  { nHigh = 0xFFFFFFFF; nLow = 0xFFFFFFFF; }
	void SetMin()  { nHigh = 0x00000000; nLow = 0x00000000; }
	void SetNull() { nHigh = 0x00000000; nLow = 0x00000000; }

	int operator ! () const { return !nHigh && !nLow; }

	SbxUINT64 &operator -= ( const SbxUINT64 &r );
	SbxUINT64 &operator += ( const SbxUINT64 &r );
	SbxUINT64 &operator /= ( const SbxUINT64 &r );
	SbxUINT64 &operator %= ( const SbxUINT64 &r );
	SbxUINT64 &operator *= ( const SbxUINT64 &r );
	SbxUINT64 &operator &= ( const SbxUINT64 &r );
	SbxUINT64 &operator |= ( const SbxUINT64 &r );
	SbxUINT64 &operator ^= ( const SbxUINT64 &r );

	friend SbxUINT64 operator - ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator + ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator / ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator % ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator * ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator & ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator | ( const SbxUINT64 &l, const SbxUINT64 &r );
	friend SbxUINT64 operator ^ ( const SbxUINT64 &l, const SbxUINT64 &r );

	friend SbxUINT64 operator ~ ( const SbxUINT64 &r );
};

#endif

#ifndef __SBX_SBXVALUES_HXX
#define __SBX_SBXVALUES_HXX

class BigInt;
class SbxDecimal;

struct BASIC_DLLPUBLIC SbxValues
{
	union {
		sal_Unicode		nChar;
		sal_uInt8			nByte;
		sal_Int16			nInteger;
		sal_Int32           nLong;
		sal_uInt16          nUShort;
		sal_uInt32          nULong;
		float           nSingle;
		double          nDouble;
		SbxINT64        nLong64;
		SbxUINT64       nULong64;
		sal_Int64       nInt64;
		sal_uInt64      uInt64;
		int             nInt;
		unsigned int    nUInt;
		::rtl::OUString* pOUString;
		SbxDecimal*		pDecimal;

		SbxBase*        pObj;
		sal_Unicode*    pChar;
		sal_uInt8*	        pByte;
		sal_Int16*	        pInteger;
		sal_Int32*	        pLong;
		sal_uInt16*	        pUShort;
		sal_uInt32*	        pULong;
		float*	        pSingle;
		double*	        pDouble;
		SbxINT64*       pLong64;
		SbxUINT64*      pULong64;
		sal_Int64*      pnInt64;
		sal_uInt64*     puInt64;
		int*	        pInt;
		unsigned int*   pUInt;
		void*	        pData;
	};
	SbxDataType	 eType;

	SbxValues(): pData( NULL ), eType(SbxEMPTY) {}
	SbxValues( SbxDataType e ): eType(e) {}
	SbxValues( char _nChar ): nChar( _nChar ), eType(SbxCHAR) {}
	SbxValues( sal_uInt8 _nByte ): nByte( _nByte ), eType(SbxBYTE) {}
	SbxValues( short _nInteger ): nInteger( _nInteger ), eType(SbxINTEGER ) {}
	SbxValues( long _nLong ): nLong( _nLong ), eType(SbxLONG) {}
	SbxValues( sal_uInt16 _nUShort ): nUShort( _nUShort ), eType(SbxUSHORT) {}
	SbxValues( sal_uIntPtr _nULong ): nULong( _nULong ), eType(SbxULONG) {}
	SbxValues( float _nSingle ): nSingle( _nSingle ), eType(SbxSINGLE) {}
	SbxValues( double _nDouble ): nDouble( _nDouble ), eType(SbxDOUBLE) {}
	SbxValues( int _nInt ): nInt( _nInt ), eType(SbxINT) {}
	SbxValues( unsigned int _nUInt ): nUInt( _nUInt ), eType(SbxUINT) {}
	SbxValues( const ::rtl::OUString* _pString ): pOUString( (::rtl::OUString*)_pString ), eType(SbxSTRING) {}
	SbxValues( SbxBase* _pObj ): pObj( _pObj ), eType(SbxOBJECT) {}
	SbxValues( sal_Unicode* _pChar ): pChar( _pChar ), eType(SbxLPSTR) {}
	SbxValues( void* _pData ): pData( _pData ), eType(SbxPOINTER) {}
	SbxValues( const BigInt &rBig );
};

#endif

#ifndef __SBX_SBXVALUE
#define __SBX_SBXVALUE

struct SbxValues;

class SbxValueImpl;

class BASIC_DLLPUBLIC SbxValue : public SbxBase
{
	SbxValueImpl* mpSbxValueImplImpl;	// Impl data

	// #55226 Transport additional infos
	SbxValue* TheRealValue( sal_Bool bObjInObjError ) const;
	SbxValue* TheRealValue() const;
protected:
	SbxValues aData; // Data
	::rtl::OUString aPic;  // Picture-String
	String          aToolString;  // tool string copy

	virtual void Broadcast( sal_uIntPtr );   	// Broadcast-Call
	virtual ~SbxValue();
	virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
	virtual sal_Bool StoreData( SvStream& ) const;
public:
	SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VALUE,1);
	TYPEINFO();
	SbxValue();
	SbxValue( SbxDataType, void* = NULL );
	SbxValue( const SbxValue& );
	SbxValue& operator=( const SbxValue& );
	virtual void Clear();
	virtual sal_Bool IsFixed() const;

	sal_Bool IsInteger() const { return sal_Bool( GetType() == SbxINTEGER  ); }
	sal_Bool IsLong()    const { return sal_Bool( GetType() == SbxLONG     ); }
	sal_Bool IsSingle()  const { return sal_Bool( GetType() == SbxSINGLE   ); }
	sal_Bool IsDouble()  const { return sal_Bool( GetType() == SbxDOUBLE   ); }
	sal_Bool IsString()  const { return sal_Bool( GetType() == SbxSTRING   ); }
	sal_Bool IsDate()    const { return sal_Bool( GetType() == SbxDATE     ); }
	sal_Bool IsCurrency()const { return sal_Bool( GetType() == SbxCURRENCY ); }
	sal_Bool IsObject()  const { return sal_Bool( GetType() == SbxOBJECT   ); }
	sal_Bool IsDataObject()const{return sal_Bool( GetType() == SbxDATAOBJECT);}
	sal_Bool IsBool()    const { return sal_Bool( GetType() == SbxBOOL     ); }
	sal_Bool IsErr()     const { return sal_Bool( GetType() == SbxERROR    ); }
	sal_Bool IsEmpty()   const { return sal_Bool( GetType() == SbxEMPTY    ); }
	sal_Bool IsNull()    const { return sal_Bool( GetType() == SbxNULL     ); }
	sal_Bool IsChar()    const { return sal_Bool( GetType() == SbxCHAR     ); }
	sal_Bool IsByte()    const { return sal_Bool( GetType() == SbxBYTE     ); }
	sal_Bool IsUShort()  const { return sal_Bool( GetType() == SbxUSHORT   ); }
	sal_Bool IsULong()   const { return sal_Bool( GetType() == SbxULONG    ); }
	sal_Bool IsInt()     const { return sal_Bool( GetType() == SbxINT      ); }
	sal_Bool IsUInt()    const { return sal_Bool( GetType() == SbxUINT     ); }
	sal_Bool IspChar()   const { return sal_Bool( GetType() == SbxLPSTR    ); }
	sal_Bool IsNumeric() const;
	sal_Bool IsNumericRTL() const;	// #41692 Interface for Basic
	sal_Bool ImpIsNumeric( sal_Bool bOnlyIntntl ) const;	// Implementation

	virtual SbxClassType GetClass() const;
	virtual SbxDataType GetType() const;
	SbxDataType GetFullType() const;
	sal_Bool SetType( SbxDataType );

	virtual sal_Bool Get( SbxValues& ) const;
	sal_Bool GetNoBroadcast( SbxValues& );
	const SbxValues& GetValues_Impl() const { return aData; }
	virtual sal_Bool Put( const SbxValues& );

    inline SbxValues * data() { return &aData; }

	SbxINT64 GetCurrency() const;
	SbxINT64 GetLong64() const;
	SbxUINT64 GetULong64() const;
	sal_Int64  GetInt64() const;
	sal_uInt64 GetUInt64() const;
	sal_Int16  GetInteger() const;
	sal_Int32  GetLong() const;
	float  GetSingle() const;
	double GetDouble() const;
	double GetDate() const;
	sal_Bool   GetBool() const;
	sal_uInt16 GetErr() const;
	const  String& GetString() const;
	const  String& GetCoreString() const;
    ::rtl::OUString GetOUString() const;
	SbxDecimal* GetDecimal() const;
	SbxBase* GetObject() const;
	sal_Bool	 HasObject() const;
	void*  GetData() const;
	sal_Unicode GetChar() const;
	sal_uInt8   GetByte() const;
	sal_uInt16 GetUShort() const;
	sal_uInt32 GetULong() const;
	int	   GetInt() const;

	sal_Bool PutCurrency( const SbxINT64& );
	sal_Bool PutLong64( const SbxINT64& );
	sal_Bool PutULong64( const SbxUINT64& );
	sal_Bool PutInt64( sal_Int64 );
	sal_Bool PutUInt64( sal_uInt64 );
	sal_Bool PutInteger( sal_Int16 );
	sal_Bool PutLong( sal_Int32 );
	sal_Bool PutSingle( float );
	sal_Bool PutDouble( double );
	sal_Bool PutDate( double );
	sal_Bool PutBool( sal_Bool );
	sal_Bool PutErr( sal_uInt16 );
	sal_Bool PutStringExt( const ::rtl::OUString& );     // with extended analysis (International, "sal_True"/"sal_False")
	sal_Bool PutString( const ::rtl::OUString& );
	sal_Bool PutString( const sal_Unicode* );   // Type = SbxSTRING
	sal_Bool PutpChar( const sal_Unicode* );    // Type = SbxLPSTR
	sal_Bool PutDecimal( SbxDecimal* pDecimal );
	sal_Bool PutObject( SbxBase* );
	sal_Bool PutData( void* );
	sal_Bool PutChar( sal_Unicode );
	sal_Bool PutByte( sal_uInt8 );
	sal_Bool PutUShort( sal_uInt16 );
	sal_Bool PutULong( sal_uInt32 );
	sal_Bool PutInt( int );
	sal_Bool PutEmpty();
	sal_Bool PutNull();

	// Special decimal methods
	sal_Bool PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );
	sal_Bool fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

	virtual sal_Bool Convert( SbxDataType );
	virtual sal_Bool Compute( SbxOperator, const SbxValue& );
	virtual sal_Bool Compare( SbxOperator, const SbxValue& ) const;
	sal_Bool Scan( const String&, sal_uInt16* = NULL );
	void Format( String&, const String* = NULL ) const;

	// Interface for CDbl in Basic
	static SbxError ScanNumIntnl( const String& rSrc, double& nVal, sal_Bool bSingle=sal_False );

	// The following operators are definied for easier handling.
	// Error conditions (overflow, conversions) are not
	// taken into consideration.

	inline int operator ==( const SbxValue& ) const;
	inline int operator !=( const SbxValue& ) const;
	inline int operator <( const SbxValue& ) const;
	inline int operator >( const SbxValue& ) const;
	inline int operator <=( const SbxValue& ) const;
	inline int operator >=( const SbxValue& ) const;

	inline SbxValue& operator *=( const SbxValue& );
	inline SbxValue& operator /=( const SbxValue& );
	inline SbxValue& operator %=( const SbxValue& );
	inline SbxValue& operator +=( const SbxValue& );
	inline SbxValue& operator -=( const SbxValue& );
	inline SbxValue& operator &=( const SbxValue& );
	inline SbxValue& operator |=( const SbxValue& );
	inline SbxValue& operator ^=( const SbxValue& );
};

inline int SbxValue::operator==( const SbxValue& r ) const
{ return Compare( SbxEQ, r ); }

inline int SbxValue::operator!=( const SbxValue& r ) const
{ return Compare( SbxNE, r ); }

inline int SbxValue::operator<( const SbxValue& r ) const
{ return Compare( SbxLT, r ); }

inline int SbxValue::operator>( const SbxValue& r ) const
{ return Compare( SbxGT, r ); }

inline int SbxValue::operator<=( const SbxValue& r ) const
{ return Compare( SbxLE, r ); }

inline int SbxValue::operator>=( const SbxValue& r ) const
{ return Compare( SbxGE, r ); }

inline SbxValue& SbxValue::operator*=( const SbxValue& r )
{ Compute( SbxMUL, r ); return *this; }

inline SbxValue& SbxValue::operator/=( const SbxValue& r )
{ Compute( SbxDIV, r ); return *this; }

inline SbxValue& SbxValue::operator%=( const SbxValue& r )
{ Compute( SbxMOD, r ); return *this; }

inline SbxValue& SbxValue::operator+=( const SbxValue& r )
{ Compute( SbxPLUS, r ); return *this; }

inline SbxValue& SbxValue::operator-=( const SbxValue& r )
{ Compute( SbxMINUS, r ); return *this; }

inline SbxValue& SbxValue::operator&=( const SbxValue& r )
{ Compute( SbxAND, r ); return *this; }

inline SbxValue& SbxValue::operator|=( const SbxValue& r )
{ Compute( SbxOR, r ); return *this; }

inline SbxValue& SbxValue::operator^=( const SbxValue& r )
{ Compute( SbxXOR, r ); return *this; }

#endif

#ifndef __SBX_SBXVARIABLE_HXX
#define __SBX_SBXVARIABLE_HXX

class SbxArray;
class SbxInfo;

#ifndef SBX_ARRAY_DECL_DEFINED
#define SBX_ARRAY_DECL_DEFINED
SV_DECL_REF(SbxArray)
#endif

#ifndef SBX_INFO_DECL_DEFINED
#define SBX_INFO_DECL_DEFINED
SV_DECL_REF(SbxInfo)
#endif

class SfxBroadcaster;

class SbxVariableImpl;
class StarBASIC;

class BASIC_DLLPUBLIC SbxVariable : public SbxValue
{
    friend class SbMethod;

    SbxVariableImpl* mpSbxVariableImpl;	// Impl data
    SfxBroadcaster*  pCst;		// Broadcaster, if needed
    String           maName;            // Name, if available
    SbxArrayRef      mpPar;             // Parameter-Array, if set
    sal_uInt16           nHash;             // Hash-ID for search

	SbxVariableImpl* getImpl( void );

protected:
	SbxInfoRef  pInfo;              // Probably called information
	sal_uIntPtr nUserData;          // User data for Call()
	SbxObject* pParent;             // Currently attached object
	virtual ~SbxVariable();
	virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
	virtual sal_Bool StoreData( SvStream& ) const;
public:
	SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_VARIABLE,2);
	TYPEINFO();
	SbxVariable();
	SbxVariable( SbxDataType, void* = NULL );
	SbxVariable( const SbxVariable& );
	SbxVariable& operator=( const SbxVariable& );

	void Dump( SvStream&, sal_Bool bDumpAll=sal_False );

	virtual void SetName( const String& );
	virtual const String& GetName( SbxNameType = SbxNAME_NONE ) const;
	sal_uInt16 GetHashCode() const			{ return nHash; }

	virtual void SetModified( sal_Bool );

	sal_uIntPtr GetUserData() const 	   { return nUserData; }
	void SetUserData( sal_uIntPtr n ) { nUserData = n;    }

	virtual SbxDataType  GetType()  const;
	virtual SbxClassType GetClass() const;

	// Parameter-Interface
	virtual SbxInfo* GetInfo();
	void SetInfo( SbxInfo* p );
	void SetParameters( SbxArray* p );
	SbxArray* GetParameters() const		{ return mpPar; }

	// Sfx-Broadcasting-Support:
	// Due to data reduction and better DLL-hierarchie currently via casting
	SfxBroadcaster& GetBroadcaster();
	sal_Bool IsBroadcaster() const { return sal_Bool( pCst != NULL ); }
	virtual void Broadcast( sal_uIntPtr nHintId );

	inline const SbxObject* GetParent() const { return pParent; }
	inline SbxObject* GetParent() { return pParent; }
	virtual void SetParent( SbxObject* );

	const String& GetDeclareClassName( void );
	void SetDeclareClassName( const String& );
	void SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener,
		StarBASIC* pParentBasic );
	void ClearComListener( void );

	static sal_uInt16 MakeHashCode( const String& rName );
};

#ifndef SBX_VARIABLE_DECL_DEFINED
#define SBX_VARIABLE_DECL_DEFINED
SV_DECL_REF(SbxVariable)
#endif

#endif

#endif	// _SBXVAR_HXX
