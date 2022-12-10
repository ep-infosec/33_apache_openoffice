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



#ifndef _WW8SCAN_HXX
#define _WW8SCAN_HXX


#ifndef LONG_MAX
#include <limits.h>
#endif
#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif
#include <tools/solar.h>		// UINTXX
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <tools/datetime.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>


#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>

#ifndef WW8STRUC_HXX
#include <ww8struc.hxx>		// FIB, STSHI, STD
#endif

#include <ww8scan.hxx>

using namespace nsHdFtFlags;


#ifndef DELETEZ
#define DELETEZ( p ) ( delete( p ), p = 0 )
#endif


//class SvStream;
class  UShortStk;
class  WW8Fib;
class  WW8ScannerBase;
class  WW8PLCFspecial;
struct WW8PLCFxDesc;
class  WW8PLCFx_PCD;
class  SvUShortsSort;


String WW8ReadPString( SvStream& rStrm, rtl_TextEncoding eEnc,
					   sal_Bool bAtEndSeekRel1 = sal_True );

// Folgende Methode liest einen 2-byter-UNICODE-String ein:
// - falls bAtEndSeekRel1 gesetzt ist, wird am Ende genau EIN Byte uebersprungen,
// - falls nChars (die Zeichen-Anzahl, NICHT die Byte-Anzahl) nicht gesetzt ist,
//   wird das erste sal_uInt16 als Lannengenangabe interpretiert,
//   ansonsten als erstes String-Zeichen genommen.
String WW8Read_xstz(SvStream& rStrm,
					sal_uInt16 nChars,
					sal_Bool   bAtEndSeekRel1);


// read array of strings (see MS documentation: STring TaBle stored in File)
// returns NOT the original pascal strings but an array of converted char*
//
// attention: the *extra data* of each string are SKIPPED and ignored

/*  to be optimized like this:    */
void WW8ReadSTTBF(	sal_Bool bVer8, SvStream& rStrm,
					sal_uInt32 nStart, sal_Int32 nLen, sal_uInt16 nSkip,
					rtl_TextEncoding eCS,
					SvStrings &rArray, SvStrings* pExtraArray = 0 );



sal_uInt16 WW8GetSprmId(        sal_uInt8 nVersion, sal_uInt8* pSp,   sal_uInt8*   pDelta = 0 );
short WW8GetSprmSizeNetto(  sal_uInt8 nVersion, sal_uInt8* pSprm, sal_uInt16* pId );
short WW8GetSprmSizeBrutto( sal_uInt8 nVersion, sal_uInt8* pSprm, sal_uInt16* pId );
sal_uInt8 WW8SprmDataOfs( sal_uInt16 nId );

struct WW8FieldDesc
{
	long nLen;				// Gesamtlaenge ( zum Text ueberlesen )
	WW8_CP nSCode;           // Anfang Befehlscode
	long nLCode;			// Laenge
	WW8_CP nSRes;			// Anfang Ergebnis
	long nLRes;				// Laenge ( == 0, falls kein Ergebnis )
	sal_uInt16 nId;				// WW-Id fuer Felder
	sal_uInt8 nOpt;				// WW-Flags ( z.B.: vom User geaendert )
	sal_Bool bCodeNest:1;		// Befehl rekursiv verwendet
	sal_Bool bResNest:1;		// Befehl in Resultat eingefuegt
};

sal_Bool WW8GetFieldPara( sal_uInt8 nVersion, WW8PLCFspecial& rPLCF, WW8FieldDesc& rF );


struct WW8PLCFxSave1
{
	sal_uLong nPLCFxPos;
	sal_uLong nPLCFxPos2;		// fuer PLCF_Cp_Fkp: PieceIter-Pos
	long nPLCFxMemOfs;
	WW8_CP nAttrStart;
	WW8_CP nAttrEnd;
	sal_Bool   bLineEnd;
};


/*
	u.a. fuer Felder, also genausoviele Attr wie Positionen,
	falls Ctor-Param bNoEnd = false
*/
class WW8PLCFspecial		// Iterator fuer PLCFs
{
	sal_Int32* pPLCF_PosArray;	// Pointer auf Pos-Array und auf ganze Struktur
	sal_uInt8*  pPLCF_Contents;	// Pointer auf Inhalts-Array-Teil des Pos-Array
	long nIMax;			// Anzahl der Elemente
	long nIdx;			// Merker, wo wir gerade sind
	long nStru;

public:
	WW8PLCFspecial( SvStream* pSt, long nFilePos, long nPLCF,
		  long nStruct, long nStartPos = -1, sal_Bool bNoEnd = sal_False );
	~WW8PLCFspecial(){ delete( pPLCF_PosArray ); pPLCF_PosArray = 0; }
	sal_uLong GetIdx() const { return (sal_uLong)nIdx; }
	void SetIdx( sal_uLong nI ) { nIdx = (long)nI; }	// geht ueber Nummer
	long GetIMax(){ return nIMax; }
	sal_Bool SeekPos( long nPos );			// geht ueber FC- bzw. CP-Wert
																	// bzw. naechste groesseren Wert
	sal_Bool SeekPosExact( long nPos );
	long Where() { return ( nIdx >= nIMax ) ? LONG_MAX : pPLCF_PosArray[nIdx]; }
	sal_Bool Get( long& rStart, void*& rpValue );
	sal_Bool GetData( long nIdx, long& rPos, void*& rpValue );

	const void* GetData( long nIdx ) const {
		return ( nIdx >= nIMax ) ? 0 : (const void*)&pPLCF_Contents[nIdx * nStru]; }
	long GetPos( long nIdx ){
		return ( nIdx >= nIMax ) ? LONG_MAX : pPLCF_PosArray[nIdx]; }

	WW8PLCFspecial& operator ++( int ) { nIdx++; return *this; }
	WW8PLCFspecial& operator --( int ) { nIdx--; return *this; }
};




/*
	simple Iterator for SPRMs
*/
class WW8SprmIter
{
	// these members will be updated
	sal_uInt8*	pSprms;		// remaining part of the SPRMs ( == start of akt. SPRM)
	sal_uInt8*	pAktParams; // start of akt. SPRM's parameters
	sal_uInt16  nAktId;
	short   nAktSizeBrutto;

	// these members will *not* be updated by UpdateMyMembers()
	sal_uInt8    nVersion;
	sal_uInt8    nDelta;
	short   nRemLen;	// length of remaining SPRMs (including akt. SPRM)

	void UpdateMyMembers();
public:
	WW8SprmIter( sal_uInt8* pSprms_, short nLen_, sal_uInt8 nVersion_ );
	void   SetSprms( sal_uInt8* pSprms_, short nLen_ );
	sal_uInt8*  operator ++( int );
	sal_uInt8*  GetSprms()     const { return ( pSprms && (0 < nRemLen) )
								? pSprms
								: 0; }
	sal_uInt8*  GetAktParams() const { return pAktParams; }
	sal_uInt16 GetAktId()     const { return nAktId; }
};



/*
	u.a. fuer FKPs auf normale Attr., also ein Attr weniger als Positionen
*/
class WW8PLCF   					// Iterator fuer PLCFs
{
	sal_Int32* pPLCF_PosArray;	// Pointer auf Pos-Array und auf ganze Struktur
	sal_uInt8* pPLCF_Contents;		// Pointer auf Inhalts-Array-Teil des Pos-Array
	long nIMax;							// Anzahl der Elemente
	long nIdx;
	long nStru;

	void ReadPLCF( SvStream* pSt, long nFilePos, long nPLCF );

	/*
		Falls im Dok ein PLC fehlt und die FKPs solo dastehen,
		machen wir uns hiermit einen PLC:
	*/
	void GeneratePLCF( SvStream* pSt, long nPN, long ncpN );

public:
	WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
		long nStruct, long nStartPos = -1 );

	/*
		folgender Ctor generiert ggfs. einen PLC aus nPN und ncpN
	*/
	WW8PLCF( SvStream* pSt, long nFilePos, long nPLCF,
		long nStruct, long nStartPos, long nPN, long ncpN );

	~WW8PLCF(){ delete( pPLCF_PosArray ); pPLCF_PosArray = 0; }
	sal_uLong GetIdx() const { return (sal_uLong)nIdx; }
	void SetIdx( sal_uLong nI ) { nIdx = (long)nI; }
	long GetIMax(){ return nIMax; }
	sal_Bool SeekPos( long nPos );
	long Where();
	sal_Bool Get( long& rStart, long& rEnd, void*& rpValue );
	WW8PLCF& operator ++( int ) { if( nIdx < nIMax ) nIdx++; return *this; }

	const void* GetData( long nIdx ) const {
		return ( nIdx >= nIMax ) ? 0 : (const void*)&pPLCF_Contents[nIdx * nStru]; }
};


DECLARE_TABLE( WW8Pcd_FC_sortArr, sal_Int32 )


/*
	fuer Piece Table (bei Schnellspeicherung)
*/
class WW8PLCFpcd
{
friend class WW8PLCFpcd_Iter;
	WW8Pcd_FC_sortArr aFC_sort; // sorted PCD entries by FC
	sal_Int32* pPLCF_PosArray;	// Pointer auf Pos-Array und auf ganze Struktur
	sal_uInt8*  pPLCF_Contents;	// Pointer auf Inhalts-Array-Teil des Pos-Array
	long nIMax;
	long nStru;

public:
	WW8PLCFpcd( SvStream* pSt, long nFilePos, long nPLCF, long nStruct );
	~WW8PLCFpcd(){ delete( pPLCF_PosArray ); }
	sal_uLong FindIdx( WW8_FC nFC ) const;
};

/*
	mehrere WW8PLCFpcd_Iter koennen auf die gleiche WW8PLCFpcd zeigen !!!
*/
class WW8PLCFpcd_Iter
{
	WW8PLCFpcd& rPLCF;
	long nIdx;

public:
	WW8PLCFpcd_Iter( WW8PLCFpcd& rPLCFpcd, long nStartPos = -1 );
//	~WW8PLCFpcd_Iter() {}
	sal_uLong GetIdx() const { return (sal_uLong)nIdx; }
	void SetIdx( sal_uLong nI ) { nIdx = (long)nI; }
	long GetIMax(){ return rPLCF.nIMax; }
	sal_Bool SeekPos( long nPos );
//	sal_Bool SeekMaxMainFC( WW8Fib& rWwF, long& rMaxPosData );
	long Where();
	sal_Bool Get( long& rStart, long& rEnd, void*& rpValue );
	sal_uLong FindIdx( WW8_FC nFC ) const { return rPLCF.FindIdx( nFC ); }
	WW8PLCFpcd_Iter& operator ++( int ) { if( nIdx < rPLCF.nIMax ) nIdx++; return *this; }
};

// PLCF-Typ:
enum ePLCFT{ CHP=0, PAP, SEP, /*HED, FNR, ENR,*/ PLCF_END };

/*
	pure virtual:
*/
class WW8PLCFx   			// virtueller Iterator fuer Piece Table Exceptions
{
	sal_uInt8 nVersion;				// Versionsnummer des FIB
	sal_Bool bIsSprm; 				// PLCF von Sprms oder von anderem ( Footnote, ... )

public:
	WW8PLCFx( sal_uInt8 nFibVersion, sal_Bool bSprm )
		{ bIsSprm = bSprm; nVersion = nFibVersion; }
	virtual ~WW8PLCFx() {}
	sal_Bool IsSprm() { return bIsSprm; }
	virtual sal_uLong GetIdx() const = 0;
	virtual void SetIdx( sal_uLong nIdx ) = 0;
	virtual sal_uLong GetIdx2() const;
	virtual void SetIdx2( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_CP nCpPos ) = 0;
	virtual long Where() = 0;
//	virtual sal_uInt8* GetSprms( long& rStart, long& rEnd, long& rLen );
	virtual void GetSprms( WW8PLCFxDesc* p );
	virtual long GetNoSprms( long& rStart, long&, long& rLen );
	virtual WW8PLCFx& operator ++( int ) = 0;
	virtual sal_uInt16 GetIstd() const { return 0xffff; }
	virtual void Save(          WW8PLCFxSave1& rSave ) const;
	virtual void Restore( const WW8PLCFxSave1& rSave );
	sal_uInt8 GetVersion() const { return nVersion; }
};

enum eCutT { CUT_NONE = 0, CUT_START, CUT_END, CUT_BOTH };

class WW8PLCFx_PCDAttrs : public WW8PLCFx
{
	WW8PLCFpcd_Iter* pPcdI;
	WW8PLCFx_PCD* pPcd;
	sal_uInt8** pGrpprls;			// Attribute an Piece-Table
	SVBT32 aShortSprm;			// mini storage: can contain ONE sprm with
								// 1 byte param
	sal_uInt16 nGrpprls;			// Attribut Anzahl davon

public:
	WW8PLCFx_PCDAttrs( sal_uInt8 nVersion, WW8PLCFx_PCD* pPLCFx_PCD, WW8ScannerBase* pBase );
	virtual ~WW8PLCFx_PCDAttrs();
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nI );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();
	virtual void GetSprms( WW8PLCFxDesc* p );
	virtual WW8PLCFx& operator ++( int );

	WW8PLCFpcd_Iter* GetIter() const { return pPcdI; }
};

class WW8PLCFx_PCD : public WW8PLCFx			// Iterator fuer Piece Table
{
	WW8PLCFpcd_Iter* pPcdI;
	sal_Bool bVer67;

public:
	WW8PLCFx_PCD( sal_uInt8 nVersion, WW8PLCFpcd* pPLCFpcd, WW8_CP nStartCp, sal_Bool bVer67P );
	virtual ~WW8PLCFx_PCD();
	virtual sal_uLong GetIMax() const;
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nI );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();
//	virtual void GetSprms( WW8PLCFxDesc* p );
	virtual long GetNoSprms( long& rStart, long&, long& rLen );
	virtual WW8PLCFx& operator ++( int );
	WW8_CP AktPieceStartFc2Cp( WW8_FC nStartPos );
	WW8_FC AktPieceStartCp2Fc( WW8_CP nCp );
	eCutT AktPieceFc2Cp( long& rStartPos, long& rEndPos );
	WW8PLCFpcd_Iter* GetPLCFIter() { return pPcdI; }

	static sal_Int32 TransformPieceAddress(long nfc, sal_Bool& bIsUnicodeAddress)
	{
		bIsUnicodeAddress = 0 == (0x40000000 & nfc);
		return bIsUnicodeAddress
			?  nfc
			: (nfc & 0x3fffFFFF) / 2;
	}
	sal_Bool IsVersion67() const { return bVer67; }
};


class WW8PLCFx_Fc_FKP : public WW8PLCFx 	// Iterator fuer Piece Table Exceptions of Fkps
								// arbeitet nur auf FCs, nicht auf CPs !
								// ( Low-Level )
{


		class WW8Fkp   		// Iterator fuer Formatted Disk Page
		{
			struct WW8Grpprl
			{
				sal_uInt8*  pData;
				sal_uInt16 nLen;
				sal_uInt16 nIStd; // nur bei Fkp.Papx gueltig (aktuelle Style-Nr)
				sal_Bool   bMustDelete;
			};

			WW8Grpprl* pGrpprl;		// Pointer of Meta Array (pointing

			sal_uInt8* pFkp;	    	// gesamter Fkp

			long nItemSize;		// entweder 1 Byte oder ein komplettes BX
			long nFilePos;		// Offset in Stream where last read of 52 bytes took place
			short nIdx;			// Pos-Merker
			ePLCFT ePLCF;
			sal_uInt8 nIMax;     	// Anzahl der Eintraege
			sal_uInt8 nVersion;

		public:
			WW8Fkp( sal_uInt8 nFibVer, SvStream* pFKPStrm, SvStream* pDataStrm,
					long _nFilePos, long nItemSiz,
					ePLCFT ePl, WW8_FC nStartFc = -1 );
			~WW8Fkp();
			long GetFilePos() const { return nFilePos; }
			sal_uLong GetIdx() const { return (sal_uLong)nIdx; }
			void SetIdx( sal_uLong nI );
			sal_Bool SeekPos( long nPos );
			WW8_FC Where() { return (  nIdx<nIMax)
									 ? ((WW8_FC*)pFkp)[nIdx]
									 : LONG_MAX; }
			WW8Fkp& operator ++( int ) { if( nIdx < nIMax ) nIdx++;
										 return *this; }
			sal_uInt8* Get( WW8_FC& rStart, WW8_FC& rEnd, short& rLen );
			sal_uInt16 GetIstd() const { return pGrpprl[ nIdx ].nIStd; }

			/*
				liefert einen echten Pointer auf das Sprm vom Typ nId,
				falls ein solches im Fkp drin ist.
			*/
			sal_uInt8* GetLenAndIStdAndSprms(short& rLen);

			/*
				ruft GetLenAndIStdAndSprms() auf...
			*/
			sal_uInt8* HasSprm( sal_uInt16 nId );

			sal_uLong GetParaHeight() const;	// fuer Header/Footer bei Papx-Fkps

			sal_uInt8 GetVersion() const { return nVersion; }
		};


	WW8PLCFx_PCDAttrs* pPCDAttrs;
	SvStream* pFKPStrm;			// Input-File
	SvStream* pDataStrm;		// Input-File
	WW8PLCF* pPLCF;
	WW8Fkp* pFkp;
	WW8_FC nStartFc;

	sal_Bool NewFkp();

protected:
	ePLCFT ePLCF;

public:
	WW8PLCFx_Fc_FKP( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
					 WW8Fib& rFib, ePLCFT ePl, WW8_FC nStartFcL,
					 WW8PLCFx_PCDAttrs* pPLCFx_PCD );
	virtual ~WW8PLCFx_Fc_FKP();
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_FC nFcPos );
	virtual WW8_FC Where();
	sal_uInt8* GetSprms( WW8_FC& rStart, WW8_FC& rEnd, long& rLen );
	virtual WW8PLCFx& operator ++( int );
	virtual sal_uInt16 GetIstd() const;
	void GetPCDSprms( WW8PLCFxDesc& rDesc );
	sal_uInt8* HasSprm( sal_uInt16 nId );
	sal_uLong GetParaHeight() const;
	sal_Bool HasFkp() { return (0 != pFkp); }
};

// Iterator fuer Piece Table Exceptions of Fkps arbeitet auf CPs (High-Level)
class WW8PLCFx_Cp_FKP : public WW8PLCFx_Fc_FKP
{
	const WW8ScannerBase& rSBase;
	WW8PLCFx_PCD* pPcd;
	WW8_CP nAttrStart, nAttrEnd;
	sal_Bool bLineEnd : 1;
	sal_Bool bComplex : 1;

	void SearchParaEnd( long nOldEndCp );

public:
	WW8PLCFx_Cp_FKP( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt, const WW8ScannerBase&
						rBase,  ePLCFT ePl );
	virtual ~WW8PLCFx_Cp_FKP();
	void ResetAttrStartEnd();
	sal_uLong GetPCDIMax() const;
	sal_uLong GetPCDIdx() const;
	void SetPCDIdx( sal_uLong nIdx );
	virtual sal_uLong GetIdx2() const;
	virtual void  SetIdx2( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual WW8_CP Where();
	virtual void GetSprms( WW8PLCFxDesc* p );
	virtual WW8PLCFx& operator ++( int );
	virtual void Save(          WW8PLCFxSave1& rSave ) const;
	virtual void Restore( const WW8PLCFxSave1& rSave );
};



class WW8PLCFx_SEPX : public WW8PLCFx   		// Iterator fuer Piece Table Exceptions of Sepx
{
	SvStream* pStrm;
	WW8PLCF* pPLCF;
	sal_uInt8* pSprms;
	sal_uInt16 nArrMax;
	sal_uInt16 nSprmSiz;

public:
	WW8PLCFx_SEPX( SvStream* pSt, SvStream* pTblxySt, WW8Fib& rFib, WW8_CP nStartCp );
	virtual ~WW8PLCFx_SEPX();
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nIdx );
	long GetIMax() const { return ( pPLCF ) ? pPLCF->GetIMax() : 0; }
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();
	virtual void GetSprms( WW8PLCFxDesc* p );
	virtual WW8PLCFx& operator ++( int );
	sal_uInt8* HasSprm( sal_uInt16 nId ) const;
	sal_uInt8* HasSprm( sal_uInt16 nId, sal_uInt8 n2nd ) const;
	sal_uInt8* HasSprm( sal_uInt16 nId, sal_uInt8* pOtherSprms, long nOtherSprmSiz ) const;
	sal_Bool Find4Sprms(sal_uInt16 nId1, sal_uInt16 nId2, sal_uInt16 nId3, sal_uInt16 nId4,
					sal_uInt8*& p1,   sal_uInt8*& p2,   sal_uInt8*& p3,   sal_uInt8*& p4 ) const;
	sal_Bool CompareSprms( sal_uInt8* pOtherSprms,
					   long nOtherSprmSiz,
					   const SvUShortsSort* pIgnoreSprms = 0 ) const;
};

// Iterator fuer Fuss-/Endnoten und Anmerkungen
class WW8PLCFx_SubDoc : public WW8PLCFx
{
	WW8PLCF* pRef;
	WW8PLCF* pTxt;

public:
	WW8PLCFx_SubDoc( SvStream* pSt, sal_uInt8 nVersion, WW8_CP nStartCp,
					long nFcRef, long nLenRef,
					long nFcTxt, long nLenTxt, long nStruc = 0 );
	virtual ~WW8PLCFx_SubDoc();
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();

	// liefert Reference Descriptoren
	const void* GetData( long nIdx = -1 ) const
		{
			return pRef ? pRef->GetData( -1L == nIdx ? pRef->GetIdx()
													 : nIdx ) : 0;
		}

	/*
		liefert Angabe, wo Kopf und Fusszeilen-Text zu finden ist
	*/
	sal_Bool Get( long& rStart, void*& rpValue );
	virtual long GetNoSprms( long& rStart, long&, long& rLen );
	virtual WW8PLCFx& operator ++( int );
	long Count() { return ( pRef ) ? pRef->GetIMax() : 0; }
};

class WW8PLCFx_FLD : public WW8PLCFx			// Iterator fuer Fuss- und Endnoten
{
	WW8PLCFspecial* pPLCF;
	WW8Fib& rFib;

public:
	WW8PLCFx_FLD( SvStream* pSt, WW8Fib& rMyFib, short nType, WW8_CP nStartCp );
	virtual ~WW8PLCFx_FLD();
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();
	virtual long GetNoSprms( long& rStart, long&, long& rLen );
	virtual WW8PLCFx& operator ++( int );
	sal_Bool GetPara( long nIdx, WW8FieldDesc& rF );
};

enum eBookStatus { BOOK_NORMAL = 0, BOOK_IGNORE = 0x1, BOOK_ONLY_REF = 0x2 };

class WW8PLCFx_Book : public WW8PLCFx			// Iterator fuer Booknotes
{
	WW8PLCFspecial* pBook[2];			// Start- und EndPosition
	SvStrings aBookNames;				// Name
	eBookStatus* pStatus;
	long nIMax;							// Anzahl der Booknotes
	sal_uInt16 nIsEnd;
public:
	WW8PLCFx_Book( SvStream* pSt, SvStream* pTblSt, WW8Fib& rFib, WW8_CP nStartCp );
	virtual ~WW8PLCFx_Book();
	long GetIMax(){ return nIMax; }
	virtual sal_uLong GetIdx() const;
	virtual void SetIdx( sal_uLong nI );
	virtual sal_uLong GetIdx2() const;
	virtual void SetIdx2( sal_uLong nIdx );
	virtual sal_Bool SeekPos( WW8_CP nCpPos );
	virtual long Where();
	virtual long GetNoSprms( long& rStart, long& rEnd, long& rLen );
	virtual WW8PLCFx& operator ++( int );
	const String* GetName() const
		{ return ( !nIsEnd && ( (long)(pBook[0]->GetIdx()) < nIMax ) )
						? aBookNames[ pBook[0]->GetIdx() ]
						: 0;
		}
	WW8_CP GetStartPos() const
		{ return ( nIsEnd ) ? LONG_MAX : pBook[0]->Where(); }
	long GetLen() const;
	sal_Bool GetIsEnd() const { return ( nIsEnd ) ? sal_True : sal_False; }
	long GetHandle() const;
	sal_Bool SetStatus( WW8_CP nStartRegion, WW8_CP nEndRegion, const String& rName,
					eBookStatus eStat );
	eBookStatus GetStatus() const;
};

/*
	hiermit arbeiten wir draussen:
*/
struct WW8PLCFManResult
{
	WW8_CP nCpPos;		// Attribut-Anfangsposition
	long nMemLen;		// Laenge dazu
	long nCp2OrIdx;		// footnote-textpos oder Index in PLCF
	WW8_CP nAktCp;		// wird nur vom Aufrufer benutzt
	sal_uInt8* pMemPos;		// Mem-Pos fuer Sprms
	sal_uInt16 nSprmId;		// Sprm-Id ( 0 = ungueltige Id -> ueberspringen! )
										// (2..255) oder Pseudo-Sprm-Id (256..260)
										// bzw. ab Winword-Ver8 die Sprm-Id (800..)
	sal_uInt8 nFlags;		// Absatz- oder Section-Anfang
};

#define MAN_ANZ_PLCF 12

#define MAN_MASK_NEW_PAP 1		// neue Zeile
#define MAN_MASK_NEW_SEP 2		// neue Section


#define MAN_MAINTEXT 0			// Defines fuer PLCFMan-ctor
#define MAN_FTN 1
#define MAN_EDN 2
#define MAN_HDFT 3
#define MAN_AND 4
#define MAN_TXBX 5
#define MAN_TXBX_HDFT 6

struct WW8PLCFxSaveAll
{
	WW8PLCFxSave1 aS[MAN_ANZ_PLCF];
};

/*
	hiermit arbeitet der Manager drinnen:
*/
struct WW8PLCFxDesc
{
	WW8PLCFx* pPLCFx;
	UShortStk* pIdStk;// Speicher fuer Attr-Id fuer Attr-Ende(n)
	sal_uInt8* pMemPos;		// wo liegen die Sprm(s)
	long nStartPos;
	long nEndPos;
	long nCp2OrIdx;		// wo liegen die NoSprm(s)
	long nSprmsLen;		// wie viele Bytes fuer weitere Sprms / Laenge Fussnote
	long nCpOfs;		// fuer Offset Header .. Footnote
	sal_Bool bFirstSprm;	// fuer Erkennung erster Sprm einer Gruppe
	sal_Bool bRealLineEnd;	// sal_False bei Pap-Piece-Ende
	void Save(          WW8PLCFxSave1& rSave ) const;
	void Restore( const WW8PLCFxSave1& rSave );
};


#ifndef DUMP

class WW8PLCFMan
{
	long nCpO;		// Origin Cp -- the basis for nNewCp

	long nLineEnd;					// zeigt *hinter* das <CR>
	long nLastWhereIdxCp;			// last result of WhereIdx()
	sal_uInt16 nPLCF;				    // so viele PLCFe werden verwaltet
	short nManType;

	WW8PLCFxDesc aD[MAN_ANZ_PLCF];
	WW8PLCFxDesc *pChp, *pPap, *pSep, *pFld, *pFldTxbx, *pFldTxbxHdft,
				 *pFtn, *pEdn,
				 *pBkm, *pPcd, *pPcdA, *pAnd;
	WW8PLCFspecial *pFdoa, *pTxbx, *pTxbxBkd;

	WW8Fib* pWwFib;
	sal_uInt16* pNoAttrScan; // Attribute komplett(!) ignorieren, die ueber n CPs
						// aufgespannt sind; z.B. bei Char #7 (Zellen-/Zeilenende)

	short WhereIdx( sal_Bool* pbStart, long* pPos );
	void AdjustEnds(    WW8PLCFxDesc& rDesc );
	void GetNewSprms(   WW8PLCFxDesc& rDesc );
	void GetNewNoSprms( WW8PLCFxDesc& rDesc );
	void GetSprmStart( short nIdx, WW8PLCFManResult* pRes );
	void GetSprmEnd( short nIdx, WW8PLCFManResult* pRes );
	void GetNoSprmStart( short nIdx, WW8PLCFManResult* pRes );
	void GetNoSprmEnd( short nIdx, WW8PLCFManResult* pRes );
	void AdvSprm(   short nIdx, sal_Bool bStart );
	void AdvNoSprm( short nIdx, sal_Bool bStart );

public:
	WW8PLCFMan( WW8ScannerBase* pBase, short nType, long nStartCp );
	~WW8PLCFMan();

	/*
		Where fragt, an welcher naechsten Position sich irgendein
		Attr aendert...
	*/
	long Where();

	sal_Bool Get( WW8PLCFManResult* pResult );
	WW8PLCFMan& operator ++( int );
	sal_uInt16 GetColl() const;	// liefert aktuellen Style
	WW8PLCFx_FLD* GetFld() const;
	WW8PLCFx_SubDoc* GetEdn() const { return (WW8PLCFx_SubDoc*)pEdn->pPLCFx; }
	WW8PLCFx_SubDoc* GetFtn() const { return (WW8PLCFx_SubDoc*)pFtn->pPLCFx; }
	WW8PLCFx_SubDoc* GetAtn() const { return (WW8PLCFx_SubDoc*)pAnd->pPLCFx; }
	WW8PLCFx_Book* GetBook() const { return (WW8PLCFx_Book*)pBkm->pPLCFx; }
	long GetCpOfs() const { return pChp->nCpOfs; }	// fuer Header/Footer...

	/*
		fragt, ob *aktueller Absatz* einen Sprm diesen Typs hat
	*/
	sal_uInt8* HasParaSprm( sal_uInt16 nId ) const;

	/*
		fragt, ob *aktueller Textrun* einen Sprm diesen Typs hat
	*/
	sal_uInt8* HasCharSprm( sal_uInt16 nId ) const;

	WW8PLCFx_Cp_FKP* GetChpPLCF(){ return (WW8PLCFx_Cp_FKP*)pChp->pPLCFx; }
	WW8PLCFx_Cp_FKP* GetPapPLCF(){ return (WW8PLCFx_Cp_FKP*)pPap->pPLCFx; }
	WW8PLCFx_SEPX* GetSepPLCF(){   return (WW8PLCFx_SEPX*)pSep->pPLCFx; }
	WW8PLCFxDesc* GetPap(){ return pPap; }
	void SeekPos( long nNewCp );
	void SaveAllPLCFx(                   WW8PLCFxSaveAll& rSave ) const;
	void RestoreAllPLCFx(          const WW8PLCFxSaveAll& rSave );
	WW8PLCFspecial* GetFdoa()	{ return pFdoa;		}
	WW8PLCFspecial* GetTxbx()	{ return pTxbx;		}
	WW8PLCFspecial* GetTxbxBkd(){ return pTxbxBkd;	}
	short GetManType() { return nManType; }
};

#endif // !DUMP

class WW8ScannerBase
{
friend WW8PLCFx_PCDAttrs::WW8PLCFx_PCDAttrs( sal_uInt8 nVersion, WW8PLCFx_PCD* pPLCFx_PCD, WW8ScannerBase* pBase );
friend WW8PLCFx_Cp_FKP::WW8PLCFx_Cp_FKP( SvStream*, SvStream*, SvStream*,
										const WW8ScannerBase&, ePLCFT );

#ifdef DUMP
friend static void DumpPLCFText( WW8_FC nPos, long nLen, long nOfs, sal_Char* pName,
								WW8ScannerBase* pBase );
friend static void DumpFtnShort( short nId, long nPos, long nFieldLen );
#else
friend WW8PLCFMan::WW8PLCFMan( WW8ScannerBase*, short, long );
friend class SwWw8ImplReader;
#endif

	WW8Fib* pWw8Fib;
	WW8PLCFx_Cp_FKP*  pChpPLCF;			// Character-Attrs
	WW8PLCFx_Cp_FKP*  pPapPLCF;			// Para-Attrs
	WW8PLCFx_SEPX*    pSepPLCF;			// Section-Attrs
	WW8PLCFx_SubDoc*  pFtnPLCF;			// Footnotes
	WW8PLCFx_SubDoc*  pEdnPLCF;			// EndNotes
	WW8PLCFx_SubDoc*  pAndPLCF;			// Anmerkungen
	WW8PLCFx_FLD*     pFldPLCF;			// Fields in Main Text
	WW8PLCFx_FLD*     pFldHdFtPLCF;		// Fields in Header / Footer
	WW8PLCFx_FLD*     pFldTxbxPLCF;		// Fields in Textboxes in Main Text
	WW8PLCFx_FLD*     pFldTxbxHdFtPLCF;	// Fields in Textboxes in Header / Footer
	WW8PLCFx_FLD*     pFldFtnPLCF;		// Fields in Footnotes
	WW8PLCFx_FLD*     pFldEdnPLCF;		// Fields in Endnotes
	WW8PLCFx_FLD*     pFldAndPLCF;		// Fields in Anmerkungen
	WW8PLCFspecial*   pMainFdoa;		// Graphic Primitives in Main Text
	WW8PLCFspecial*   pHdFtFdoa;		// Graphic Primitives in Header / Footer
	WW8PLCFspecial*   pMainTxbx;		// Textboxen in Main Text
	WW8PLCFspecial*   pMainTxbxBkd;		// Break-Deskriptoren fuer diese
	WW8PLCFspecial*   pHdFtTxbx;		// TextBoxen in Header / Footer
	WW8PLCFspecial*   pHdFtTxbxBkd;		// Break-Deskriptoren fuer diese
	WW8PLCFx_Book*    pBook;			// Bookmarks

	WW8PLCFpcd*			pPiecePLCF;	// fuer FastSave ( Basis-PLCF ohne Iterator )
	WW8PLCFpcd_Iter*	pPieceIter;	// fuer FastSave ( Iterator dazu )
	WW8PLCFx_PCD*		pPLCFx_PCD;		// dito
	WW8PLCFx_PCDAttrs*	pPLCFx_PCDAttrs;
	sal_uInt8** 				pPieceGrpprls;	// Attribute an Piece-Table
	sal_uInt16 				nPieceGrpprls;	// Anzahl davon
	sal_uInt16				nNoAttrScan;	// Attribute komplett(!) ignorieren, die ueber n CPs
										// aufgespannt sind; z.B. bei Char #7 (Zellen-/Zeilenende)

	WW8PLCFpcd* OpenPieceTable( SvStream* pStr, WW8Fib* pWwF );
	void DeletePieceTable();

public:
	WW8ScannerBase( SvStream* pSt, SvStream* pTblSt, SvStream* pDataSt,
					WW8Fib* pWwF );
	~WW8ScannerBase();
	sal_Bool AreThereFootnotes() { return pFtnPLCF->Count() > 0; };
	sal_Bool AreThereEndnotes()  { return pEdnPLCF->Count() > 0; };
	static DateTime WW8DTTM2DateTime(long lDTTM);

	WW8_CP WW8Fc2Cp( WW8_FC nFcPos ) const ;
	WW8_FC WW8Cp2Fc( WW8_CP nCpPos, sal_Bool* pIsUnicode = 0,
					 WW8_CP* pNextPieceCp = 0, sal_Bool* pTestFlag = 0 ) const;
	void SetNoAttrScan( sal_uInt16 nValue ) { nNoAttrScan = nValue; };

	sal_uInt16 WW8ReadString( SvStream& rStrm, String& rStr,
							WW8_CP nAktStartCp, long nTotalLen,
							rtl_TextEncoding eEnc ) const;
};



/** FIB - File Information Block
*/
class WW8Fib
{
public:
	/*
		von uns verlangte Programm-Version:
		in Ctor wird geprueft, ob sie zu nFib passt
	*/
	sal_uInt8 nVersion;	// 6 steht fuer "WinWord 6 oder WinWord 95",
									// 7 steht fuer "nur WinWord 95"
									// 8 steht fuer "nur WinWord 97"
	/*
		Fehlerstatus
	*/
	sal_uLong nFibError;
	/*
		vom Ctor aus dem FIB gelesene Daten
		(entspricht nur ungefaehr der tatsaechlichen Struktur
		 des Winword-FIB)
	*/
	sal_uInt16 wIdent;		// 0x0 int magic number
	sal_uInt16 nFib; 		// 0x2 FIB version written
	sal_uInt16 nProduct; 	// 0x4 product version written by
	sal_Int16 lid; 			// 0x6 language stamp---localized version;
	WW8_PN pnNext; 			// 0x8

	sal_uInt16 fDot :1; 	// 0xa 0001
	sal_uInt16 fGlsy :1;
	sal_uInt16 fComplex :1; // 0004 when 1, file is in complex, fast-saved format.
	sal_uInt16 fHasPic :1; 	// 0008 file contains 1 or more pictures
	sal_uInt16 cQuickSaves :4; // 00F0 count of times file was quicksaved
	sal_uInt16 fEncrypted :1; //0100 1 if file is encrypted, 0 if not
	sal_uInt16 fWhichTblStm :1; //0200 When 0, this fib refers to the table stream
													// named "0Table", when 1, this fib refers to the
													// table stream named "1Table". Normally, a file
													// will have only one table stream, but under unusual
													// circumstances a file may have table streams with
													// both names. In that case, this flag must be used
													// to decide which table stream is valid.
	// sal_uInt16 u1 :2;
	sal_uInt16 fExtChar :1; // 1000 =1, when using extended character set in file
	// sal_uInt16 u2 :3;

	sal_uInt16 nFibBack; 	// 0xc
	sal_Int16 lKey1; 			// 0xe  file encrypted key, only valid if fEncrypted.
	sal_Int16 lKey2; 			// 0x10  key in 2 Portionen wg. Misalignment
	sal_uInt8 envr;			// 0x12 environment in which file was created
									//      0 created by Win Word / 1 created by Mac Word
	sal_uInt8 fMac              :1;			// 0x13 when 1, this file was last saved in the Mac environment
	sal_uInt8 fEmptySpecial     :1;
	sal_uInt8 fLoadOverridePage :1;
	sal_uInt8 fFuturesavedUndo  :1;
	sal_uInt8 fWord97Saved      :1;
	sal_uInt8 :3;

	sal_uInt16 chse;		// 0x14 default extended character set id for text in document stream. (overidden by chp.chse)
						//      0 = ANSI  / 256 Macintosh character set.
	sal_uInt16 chseTables;	// 0x16 default extended character set id for text in
						//      internal data structures: 0 = ANSI, 256 = Macintosh
	WW8_FC fcMin;			// 0x18 file offset of first character of text
	WW8_FC fcMac;			// 0x1c file offset of last character of text + 1

	// Einschub fuer WW8 *****************************************************
	sal_uInt16 csw;				// Count of fields in the array of "shorts"

	// Marke: "rgsw" Beginning of the array of shorts
	sal_uInt16 wMagicCreated;					// unique number Identifying the File's creator
																// 0x6A62 is the creator ID for Word and is reserved.
																// Other creators should choose a different value.
	sal_uInt16 wMagicRevised;					// identifies the File's last modifier
  sal_uInt16 wMagicCreatedPrivate;	// private data
	sal_uInt16 wMagicRevisedPrivate;	// private data
	/*
	sal_Int16  pnFbpChpFirst_W6;			// not used
	sal_Int16  pnChpFirst_W6;					// not used
	sal_Int16  cpnBteChp_W6;					// not used
	sal_Int16  pnFbpPapFirst_W6;			// not used
	sal_Int16  pnPapFirst_W6;					// not used
	sal_Int16  cpnBtePap_W6;					// not used
	sal_Int16  pnFbpLvcFirst_W6;			// not used
	sal_Int16  pnLvcFirst_W6;					// not used
	sal_Int16  cpnBteLvc_W6;					// not used
	*/
	sal_Int16  lidFE;									// Language id if document was written by Far East version
																// of Word (i.e. FIB.fFarEast is on)
	sal_uInt16 clw;										// Number of fields in the array of longs

	// Ende des Einschubs fuer WW8 *******************************************

	// Marke: "rglw" Beginning of the array of longs
	WW8_FC cbMac;			// 0x20 file offset of last byte written to file + 1.

	// WW8_FC u4[4];			// 0x24
	WW8_CP ccpText;			// 0x34 length of main document text stream
	WW8_CP ccpFtn;			// 0x38 length of footnote subdocument text stream
	WW8_CP ccpHdr;			// 0x3c length of header subdocument text stream
	WW8_CP ccpMcr;			// 0x40 length of macro subdocument text stream
	WW8_CP ccpAtn;			// 0x44 length of annotation subdocument text stream
	WW8_CP ccpEdn;			// 0x48 length of endnote subdocument text stream
	WW8_CP ccpTxbx;			// 0x4c length of textbox subdocument text stream
	WW8_CP ccpHdrTxbx;		// 0x50 length of header textbox subdocument text stream

	// Einschub fuer WW8 *****************************************************
	sal_Int32  pnFbpChpFirst;	// when there was insufficient memory for Word to expand
												// the PLCFbte at save time, the PLCFbte is written
												// to the file in a linked list of 512-byte pieces
												// starting with this pn.
	/*
	// folgende Felder existieren zwar so in der Datei,
	// wir benutzen jedoch unten deklarierte General-Variablen
	// fuer Ver67 und Ver8 gemeinsam.
	sal_Int32  pnChpFirst;		// the page number of the lowest numbered page in the
														// document that records CHPX FKP information
	sal_Int32  cpnBteChp;			// count of CHPX FKPs recorded in file. In non-complex
														// files if the number of entries in the PLCFbteChpx
														// is less than this, the PLCFbteChpx is incomplete.
	*/
	sal_Int32  pnFbpPapFirst;	// when there was insufficient memory for Word to expand
												// the PLCFbte at save time, the PLCFbte is written to
												// the file in a linked list of 512-byte pieces
												// starting with this pn
	/*
	// folgende Felder existieren zwar so in der Datei,
	// wir benutzen jedoch unten deklarierte General-Variablen
	// fuer Ver67 und Ver8 gemeinsam.
	sal_Int32  pnPapFirst;		// the page number of the lowest numbered page in the
														// document that records PAPX FKP information
	sal_Int32  cpnBtePap;		// count of PAPX FKPs recorded in file. In non-complex
														// files if the number of entries in the PLCFbtePapx is
														// less than this, the PLCFbtePapx is incomplete.
	*/
	sal_Int32  pnFbpLvcFirst;	// when there was insufficient memory for Word to expand
												// the PLCFbte at save time, the PLCFbte is written to
												// the file in a linked list of 512-byte pieces
												// starting with this pn
	sal_Int32  pnLvcFirst;			// the page number of the lowest numbered page in the
												// document that records LVC FKP information
	sal_Int32  cpnBteLvc;			// count of LVC FKPs recorded in file. In non-complex
												// files if the number of entries in the PLCFbtePapx is
												// less than this, the PLCFbtePapx is incomplete.
	sal_Int32  fcIslandFirst;	// ?
	sal_Int32  fcIslandLim;		// ?
	sal_uInt16 cfclcb;				// Number of fields in the array of FC/LCB pairs.

	// Ende des Einschubs fuer WW8 *******************************************

	// Marke: "rgfclcb" Beginning of array of FC/LCB pairs.
	WW8_FC fcStshfOrig;		// file offset of original allocation for STSH in table
												// stream. During fast save Word will attempt to reuse
												// this allocation if STSH is small enough to fit.
	sal_Int32 lcbStshfOrig;	// 0x5c count of bytes of original STSH allocation
	WW8_FC fcStshf;			// 0x60 file offset of STSH in file.
	sal_Int32 lcbStshf;		// 0x64 count of bytes of current STSH allocation
	WW8_FC fcPlcffndRef;	// 0x68 file offset of footnote reference PLCF.
	sal_Int32 lcbPlcffndRef;	// 0x6c count of bytes of footnote reference PLCF
						//      == 0 if no footnotes defined in document.

	WW8_FC fcPlcffndTxt;	// 0x70 file offset of footnote text PLCF.
	sal_Int32 lcbPlcffndTxt;	// 0x74 count of bytes of footnote text PLCF.
						//      == 0 if no footnotes defined in document

	WW8_FC fcPlcfandRef;	// 0x78 file offset of annotation reference PLCF.
	sal_Int32 lcbPlcfandRef;	// 0x7c count of bytes of annotation reference PLCF.

	WW8_FC fcPlcfandTxt;	// 0x80 file offset of annotation text PLCF.
	sal_Int32 lcbPlcfandTxt;	// 0x84 count of bytes of the annotation text PLCF

	WW8_FC fcPlcfsed;		// 8x88 file offset of section descriptor PLCF.
	sal_Int32 lcbPlcfsed;	// 0x8c count of bytes of section descriptor PLCF.

	WW8_FC fcPlcfpad;		// 0x90 file offset of paragraph descriptor PLCF
	sal_Int32 lcbPlcfpad;	// 0x94 count of bytes of paragraph descriptor PLCF.
						// ==0 if file was never viewed in Outline view.
						// Should not be written by third party creators

	WW8_FC fcPlcfphe;		// 0x98 file offset of PLCF of paragraph heights.
	sal_Int32 lcbPlcfphe;	// 0x9c count of bytes of paragraph height PLCF.
						// ==0 when file is non-complex.

	WW8_FC fcSttbfglsy;		// 0xa0 file offset of glossary string table.
	sal_Int32 lcbSttbfglsy;	// 0xa4 count of bytes of glossary string table.
						//		== 0 for non-glossary documents.
						//		!=0 for glossary documents.

	WW8_FC fcPlcfglsy;		// 0xa8 file offset of glossary PLCF.
	sal_Int32 lcbPlcfglsy;	// 0xac count of bytes of glossary PLCF.
						//		== 0 for non-glossary documents.
						//		!=0 for glossary documents.

	WW8_FC fcPlcfhdd;		// 0xb0 byte offset of header PLCF.
	sal_Int32 lcbPlcfhdd;	// 0xb4 count of bytes of header PLCF.
						// 		== 0 if document contains no headers

	WW8_FC fcPlcfbteChpx;	// 0xb8 file offset of character property bin table.PLCF.
	sal_Int32 lcbPlcfbteChpx;// 0xbc count of bytes of character property bin table PLCF.

	WW8_FC fcPlcfbtePapx; 	// 0xc0 file offset of paragraph property bin table.PLCF.
	sal_Int32 lcbPlcfbtePapx;// 0xc4 count of bytes of paragraph  property bin table PLCF.

	WW8_FC fcPlcfsea;		// 0xc8 file offset of PLCF reserved for private use. The SEA is 6 bytes long.
	sal_Int32 lcbPlcfsea;	// 0xcc	count of bytes of private use PLCF.

	WW8_FC fcSttbfffn;		// 0xd0	file offset of font information STTBF. See the FFN file structure definition.
	sal_Int32 lcbSttbfffn;	// 0xd4	count of bytes in sttbfffn.

	WW8_FC fcPlcffldMom;	// 0xd8	offset in doc stream to the PLCF of field positions in the main document.
	sal_Int32 lcbPlcffldMom;	// 0xdc

	WW8_FC fcPlcffldHdr;	// 0xe0	offset in doc stream to the PLCF of field positions in the header subdocument.
	sal_Int32 lcbPlcffldHdr;	// 0xe4

	WW8_FC fcPlcffldFtn;	// 0xe8	offset in doc stream to the PLCF of field positions in the footnote subdocument.
	sal_Int32 lcbPlcffldFtn;	// 0xec

	WW8_FC fcPlcffldAtn;	// 0xf0	offset in doc stream to the PLCF of field positions in the annotation subdocument.
	sal_Int32 lcbPlcffldAtn;	// 0xf4

	WW8_FC fcPlcffldMcr;	// 0xf8	offset in doc stream to the PLCF of field positions in the macro subdocument.
	sal_Int32 lcbPlcffldMcr;	// 9xfc

	WW8_FC fcSttbfbkmk;	// 0x100 offset in document stream of the STTBF that records bookmark names in the main document
	sal_Int32 lcbSttbfbkmk;	// 0x104

	WW8_FC fcPlcfbkf;	// 0x108 offset in document stream of the PLCF that records the beginning CP offsets of bookmarks in the main document. See BKF
	sal_Int32 lcbPlcfbkf;	// 0x10c

	WW8_FC fcPlcfbkl;	// 0x110 offset in document stream of the PLCF that records the ending CP offsets of bookmarks recorded in the main document. See the BKL structure definition.
	sal_Int32 lcbPlcfbkl;	// 0x114 sal_Int32

	WW8_FC fcCmds;		// 0x118 FC
	sal_Int32 lcbCmds;		// 0x11c

	WW8_FC fcPlcfmcr;		// 0x120 FC
	sal_Int32 lcbPlcfmcr;		// 0x124

	WW8_FC fcSttbfmcr;	// 0x128 FC
	sal_Int32 lcbSttbfmcr;	// 0x12c

	WW8_FC fcPrDrvr;		// 0x130 file offset of the printer driver information (names of drivers, port etc...)
	sal_Int32 lcbPrDrvr;		// 0x134 count of bytes of the printer driver information (names of drivers, port etc...)

	WW8_FC fcPrEnvPort;	// 0x138 file offset of the print environment in portrait mode.
	sal_Int32 lcbPrEnvPort;	// 0x13c count of bytes of the print environment in portrait mode.

	WW8_FC fcPrEnvLand;	// 0x140 file offset of the print environment in landscape mode.
	sal_Int32 lcbPrEnvLand;	// 0x144 count of bytes of the print environment in landscape mode.

	WW8_FC fcWss;		// 0x148 file offset of Window Save State data structure. See WSS.
	sal_Int32 lcbWss;		// 0x14c count of bytes of WSS. ==0 if unable to store the window state.

	WW8_FC fcDop;		// 0x150 file offset of document property data structure.
	sal_Int32 lcbDop;		// 0x154 count of bytes of document properties.
		// cbDOP is 84 when nFib < 103


	WW8_FC fcSttbfAssoc;	// 0x158 offset to STTBF of associated strings. See STTBFASSOC.
	sal_Int32 cbSttbfAssoc;	// 0x15C

	WW8_FC fcClx;			// 0x160 file  offset of beginning of information for complex files.
	sal_Int32 lcbClx;		// 0x164 count of bytes of complex file information. 0 if file is non-complex.

	WW8_FC fcPlcfpgdFtn;	// 0x168 file offset of page descriptor PLCF for footnote subdocument.
	sal_Int32 lcbPlcfpgdFtn;	// 0x16C count of bytes of page descriptor PLCF for footnote subdocument.
						//	==0 if document has not been paginated. The length of the PGD is 8 bytes.

	WW8_FC fcAutosaveSource;	// 0x170 file offset of the name of the original file.
	sal_Int32 lcbAutosaveSource;	// 0x174 count of bytes of the name of the original file.

	WW8_FC fcGrpStAtnOwners;	// 0x178 group of strings recording the names of the owners of annotations
	sal_Int32 lcbGrpStAtnOwners;	// 0x17C count of bytes of the group of strings

	WW8_FC fcSttbfAtnbkmk;	// 0x180 file offset of the sttbf that records names of bookmarks in the annotation subdocument
	sal_Int32 lcbSttbfAtnbkmk;	// 0x184 length in bytes of the sttbf that records names of bookmarks in the annotation subdocument

	// Einschubs fuer WW67 ***************************************************

	// sal_Int16 wSpare4Fib;	// Reserve, muss hier nicht deklariert werden

	/*
	// folgende Felder existieren zwar so in der Datei,
	// wir benutzen jedoch unten deklarierte General-Variablen
	// fuer Ver67 und Ver8 gemeinsam.
	WW8_PN pnChpFirst;	// the page number of the lowest numbered page in
														// the document that records CHPX FKP information
	WW8_PN pnPapFirst;	// the page number of the lowest numbered page in
														// the document that records PAPX FKP information

	WW8_PN cpnBteChp;		// count of CHPX FKPs recorded in file. In non-complex
														// files if the number of entries in the PLCFbteChpx is
														// less than this, the PLCFbteChpx  is incomplete.
	WW8_PN cpnBtePap;		// count of PAPX FKPs recorded in file. In non-complex
														// files if the number of entries in the PLCFbtePapx is
														// less than this, the PLCFbtePapx  is incomplete.
	*/

	// Ende des Einschubs fuer WW67 ******************************************

	WW8_FC fcPlcfdoaMom;	// 0x192 file offset of the  FDOA (drawn object) PLCF for main document.
						//	==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
						// ab Ver8 unused
	sal_Int32 lcbPlcfdoaMom;	// 0x196 length in bytes of the FDOA PLCF of the main document
												// ab Ver8 unused
	WW8_FC fcPlcfdoaHdr;	// 0x19A file offset of the  FDOA (drawn object) PLCF for the header document.
						//	==0 if document has no drawn objects. The length of the FDOA is 6 bytes.
						// ab Ver8 unused
	sal_Int32 lcbPlcfdoaHdr;	// 0x19E length in bytes of the FDOA PLCF of the header document
												// ab Ver8 unused

	WW8_FC fcPlcfspaMom;		// offset in table stream of the FSPA PLCF for main document.
												// == 0 if document has no office art objects
														// war in Ver67 nur leere Reserve
	sal_Int32 lcbPlcfspaMom;		// length in bytes of the FSPA PLCF of the main document
														// war in Ver67 nur leere Reserve
	WW8_FC fcPlcfspaHdr;		// offset in table stream of the FSPA PLCF for header document.
												// == 0 if document has no office art objects
														// war in Ver67 nur leere Reserve
	sal_Int32 lcbPlcfspaHdr;		// length in bytes of the FSPA PLCF of the header document
														// war in Ver67 nur leere Reserve

	WW8_FC fcPlcfAtnbkf;	// 0x1B2 file offset of BKF (bookmark first) PLCF of the annotation subdocument
	sal_Int32 lcbPlcfAtnbkf;	// 0x1B6 length in bytes of BKF (bookmark first) PLCF of the annotation subdocument

	WW8_FC fcPlcfAtnbkl;	// 0x1BA file offset of BKL (bookmark last) PLCF of the annotation subdocument
	sal_Int32 lcbPlcfAtnbkl;	// 0x1BE length in bytes of BKL (bookmark first) PLCF of the annotation subdocument

	WW8_FC fcPms;		// 0x1C2 file offset of PMS (Print Merge State) information block
	sal_Int32 lcbPMS;		// 0x1C6 length in bytes of PMS

	WW8_FC fcFormFldSttbf;	// 0x1CA file offset of form field Sttbf which contains strings used in form field dropdown controls
	sal_Int32 lcbFormFldSttbf;	// 0x1CE length in bytes of form field Sttbf

	WW8_FC fcPlcfendRef;	// 0x1D2 file offset of PLCFendRef which points to endnote references in the main document stream
	sal_Int32 lcbPlcfendRef;	// 0x1D6

	WW8_FC fcPlcfendTxt;	// 0x1DA file offset of PLCFendRef which points to endnote text  in the endnote document
						//		 stream which corresponds with the PLCFendRef
	sal_Int32 lcbPlcfendTxt;	// 0x1DE

	WW8_FC fcPlcffldEdn;	// 0x1E2 offset to PLCF of field positions in the endnote subdoc
	sal_Int32 lcbPlcffldEdn;	// 0x1E6

	WW8_FC  fcPlcfpgdEdn; 	// 0x1EA offset to PLCF of page boundaries in the endnote subdoc.
	sal_Int32 lcbPlcfpgdEdn;		// 0x1EE


	WW8_FC fcDggInfo;			// offset in table stream of the office art object table data.
												// The format of office art object table data is found in a separate document.
														// war in Ver67 nur leere Reserve
	sal_Int32 lcbDggInfo;			// length in bytes of the office art object table data
														// war in Ver67 nur leere Reserve

	WW8_FC fcSttbfRMark;		// 0x1fa offset to STTBF that records the author abbreviations...
	sal_Int32 lcbSttbfRMark;		// 0x1fe
	WW8_FC fcSttbfCaption;	// 0x202 offset to STTBF that records caption titles...
	sal_Int32 lcbSttbfCaption;	// 0x206
	WW8_FC fcSttbAutoCaption;	// offset in table stream to the STTBF that records the object names and
														// indices into the caption STTBF for objects which get auto captions.
	sal_Int32 lcbSttbAutoCaption;	// 0x20e

	WW8_FC fcPlcfwkb;		// 0x212 offset to PLCF that describes the boundaries of contributing documents...
	sal_Int32 lcbPlcfwkb;		// 0x216

	WW8_FC fcPlcfspl;		// offset in table stream of PLCF (of SPLS structures) that records spell check state
														// war in Ver67 nur leere Reserve
	sal_Int32 lcbPlcfspl;					// war in Ver67 nur leere Reserve

	WW8_FC fcPlcftxbxTxt;	// 0x222 ...PLCF of beginning CP in the text box subdoc
	sal_Int32 lcbPlcftxbxTxt;	// 0x226
	WW8_FC fcPlcffldTxbx;	// 0x22a ...PLCF of field boundaries recorded in the textbox subdoc.
	sal_Int32 lcbPlcffldTxbx;	// 0x22e
	WW8_FC fcPlcfHdrtxbxTxt;// 0x232 ...PLCF of beginning CP in the header text box subdoc
	sal_Int32 lcbPlcfHdrtxbxTxt;// 0x236
	WW8_FC fcPlcffldHdrTxbx;// 0x23a ...PLCF of field boundaries recorded in the header textbox subdoc.
	sal_Int32 lcbPlcffldHdrTxbx;// 0x23e

	/*
		spezielle Listenverwaltung fuer WW8
	*/
	WW8_FC fcPlcfLst;		// 0x02e2 offset in the table stream of list format information.
	sal_Int32 lcbPlcfLst;		// 0x02e6 length
	WW8_FC fcPlfLfo;		// 0x02ea offset in the table stream of list format override information.
	sal_Int32 lcbPlfLfo;		// 0x02ee length
	/*
		spezielle Break-Verwaltung fuer Text-Box-Stories in WW8
	*/
	WW8_FC fcPlcftxbxBkd;	// 0x02f2 PLCF fuer TextBox-Break-Deskriptoren im Maintext
	sal_Int32 lcbPlcftxbxBkd;	// 0x02f6
	WW8_FC fcPlcfHdrtxbxBkd;// 0x02fa PLCF fuer TextBox-Break-Deskriptoren im Header-/Footer-Bereich
	sal_Int32 lcbPlcfHdrtxbxBkd;// 0x02fe

	// 0x302 - 372 == ignore
	/*
		ListNames (skip to here!)
	*/
	WW8_FC fcSttbListNames;// 0x0372 PLCF for Listname Table
	sal_Int32 lcbSttbListNames;// 0x0376
	/*
		General-Varaiblen, die fuer Ver67 und Ver8 verwendet werden,
		obwohl sie in der jeweiligen DATEI verschiedene Groesse haben:
	*/
	sal_Int32 pnChpFirst;
	sal_Int32 pnPapFirst;
	sal_Int32 cpnBteChp;
	sal_Int32 cpnBtePap;
	/*
		nun wird lediglich noch ein Ctor benoetigt
	*/
	WW8Fib( SvStream& rStrm, sal_uInt8 nWantedVersion,sal_uInt32 nOffset=0 );

	/* leider falsch, man braucht auch noch einen fuer den Export */
	WW8Fib( sal_uInt8 nVersion = 6 );
	sal_Bool Write( SvStream& rStrm );
};


class WW8Style
{
protected:
	WW8Fib& rFib;
	SvStream& rSt;
	long nStyleStart;
	long nStyleLen;
//	WW8_STSHI* pStishi;

	sal_uInt16	cstd;								// Count of styles in stylesheet
	sal_uInt16	cbSTDBaseInFile;		// Length of STD Base as stored in a file
	sal_uInt16	fStdStylenamesWritten : 1;	// Are built-in stylenames stored?
	sal_uInt16	: 15;					// Spare flags
	sal_uInt16	stiMaxWhenSaved;		// Max sti known when this file was written
	sal_uInt16	istdMaxFixedWhenSaved;	// How many fixed-index istds are there?
	sal_uInt16	nVerBuiltInNamesWhenSaved;	// Current version of built-in stylenames
	sal_uInt16	ftcStandardChpStsh;		// ftc used by StandardChpStsh for this document

public:
	WW8Style( SvStream& rSt, WW8Fib& rFibPara );
//	~WW8Style(){ delete( pStishi ); pStishi = 0; }
	WW8_STD* Read1STDFixed( short& rSkip, short* pcbStd );
	WW8_STD* Read1Style( short& rSkip, String* pString, short* pcbStd );
	const sal_uInt16 GetCount() const { return cstd; }
};


class WW8Fonts
{
protected:
	WW8_FFN* pFontA;		// Array of Pointers to Font Description
	// Hilfs-Vari
	sal_uInt16 nMax;		// Array-Groesse
public:
	WW8Fonts( SvStream& rSt, WW8Fib& rFib );
	~WW8Fonts(){ if( pFontA ) delete [] pFontA; }
	const WW8_FFN* GetFont( sal_uInt16 nNum ) const;
	sal_uInt16 GetMax() const { return nMax; }
};


/** Document Properties
*/
class WW8Dop
{
public:
	/*
		Fehlerstatus
	*/
	sal_uLong nDopError;
	/*
		vom Ctor aus dem FIB gelesene Daten
		(entspricht nur ungefaehr der tatsaechlichen Struktur
		 des Winword-FIB)
	*/
	// Initialisier-Dummy:
	sal_uInt8    nDataStart;
	//-------------------------
	sal_uInt16	fFacingPages : 1;	// 1 when facing pages should be printed
	sal_uInt16	fWidowControl : 1;	// 1 when widow control is in effect. 0 when widow control disabled.
	sal_uInt16	fPMHMainDoc	: 1;	// 1 when doc is a main doc for Print Merge Helper, 0 when not; default=0
	sal_uInt16	grfSuppression : 2;	// 0 Default line suppression storage; 0= form letter line suppression; 1= no line suppression; default=0
	sal_uInt16	fpc : 2;			// 1 footnote position code: 0 as endnotes, 1 at bottom of page, 2 immediately beneath text
	sal_uInt16	: 1;				// 0 unused
	//-------------------------
	sal_uInt16	grpfIhdt : 8;			// 0 specification of document headers and footers. See explanation under Headers and Footers topic.
	//-------------------------
	sal_uInt16	rncFtn : 2;			// 0 restart index for footnotes, 0 don't restart note numbering, 1	section, 2 page
	sal_uInt16	nFtn : 14;			// 1 initial footnote number for document
	sal_uInt16	fOutlineDirtySave : 1; // when 1, indicates that information in the hPLCFpad should be refreshed since outline has been dirtied
	sal_uInt16	: 7;				//	 reserved
	sal_uInt16	fOnlyMacPics : 1;	//   when 1, Word believes all pictures recorded in the document were created on a Macintosh
	sal_uInt16	fOnlyWinPics : 1;	//	 when 1, Word believes all pictures recorded in the document were created in Windows
	sal_uInt16	fLabelDoc : 1;		//	 when 1, document was created as a print merge labels document
	sal_uInt16	fHyphCapitals : 1;	//	 when 1, Word is allowed to hyphenate words that are capitalized. When 0, capitalized may not be hyphenated
	sal_uInt16	fAutoHyphen	: 1;	//	 when 1, Word will hyphenate newly typed text as a background task
	sal_uInt16	fFormNoFields : 1;
	sal_uInt16	fLinkStyles	: 1;	//	 when 1, Word will merge styles from its template
	sal_uInt16	fRevMarking	: 1;	//	 when 1, Word will mark revisions as the document is edited
	sal_uInt16	fBackup : 1;		//	 always make backup when document saved when 1.
	sal_uInt16	fExactCWords : 1;
	sal_uInt16	fPagHidden : 1;		//
	sal_uInt16	fPagResults : 1;
	sal_uInt16	fLockAtn : 1;		//	 when 1, annotations are locked for editing
	sal_uInt16	fMirrorMargins : 1;	//	 swap margins on left/right pages when 1.
	sal_uInt16	fReadOnlyRecommended : 1;// user has recommended that this doc be opened read-only when 1
	sal_uInt16	fDfltTrueType : 1;	//	 when 1, use TrueType fonts by default (flag obeyed only when doc was created by WinWord 2.x)
	sal_uInt16	fPagSuppressTopSpacing : 1;//when 1, file created with SUPPRESSTOPSPACING=YES in win.ini. (flag obeyed only when doc was created by WinWord 2.x).
	sal_uInt16	fProtEnabled : 1;	//	 when 1, document is protected from edit operations
	sal_uInt16	fDispFormFldSel : 1;//	 when 1, restrict selections to occur only within form fields
	sal_uInt16	fRMView : 1;		//	 when 1, show revision markings on screen
	sal_uInt16	fRMPrint : 1;		//	 when 1, print revision marks when document is printed
	sal_uInt16	fWriteReservation : 1;
	sal_uInt16	fLockRev : 1;		//	 when 1, the current revision marking state is locked
	sal_uInt16	fEmbedFonts : 1;	//	 when 1, document contains embedded True Type fonts
	//	  compatibility options
	sal_uInt16 copts_fNoTabForInd : 1;			//	  when 1, don�t add automatic tab stops for hanging indent
	sal_uInt16 copts_fNoSpaceRaiseLower : 1;		//	  when 1, don�t add extra space for raised or lowered characters
	sal_uInt16 copts_fSupressSpbfAfterPgBrk : 1;	// when 1, suppress the paragraph Space Before and Space After options after a page break
	sal_uInt16 copts_fWrapTrailSpaces : 1;		//	  when 1, wrap trailing spaces at the end of a line to the next line
	sal_uInt16 copts_fMapPrintTextColor : 1;		//	  when 1, print colors as black on non-color printers
	sal_uInt16 copts_fNoColumnBalance : 1;		//	  when 1, don�t balance columns for Continuous Section starts
	sal_uInt16 copts_fConvMailMergeEsc : 1;
	sal_uInt16 copts_fSupressTopSpacing : 1;		//	  when 1, suppress extra line spacing at top of page
	sal_uInt16 copts_fOrigWordTableRules : 1;	//	  when 1, combine table borders like Word 5.x for the Macintosh
	sal_uInt16 copts_fTransparentMetafiles : 1;	//	  when 1, don�t blank area between metafile pictures
	sal_uInt16 copts_fShowBreaksInFrames : 1;	//	  when 1, show hard page or column breaks in frames
	sal_uInt16 copts_fSwapBordersFacingPgs : 1;	//	  when 1, swap left and right pages on odd facing pages

	sal_Int16  dxaTab;				// 720 twips	default tab width
	sal_uInt16 wSpare;				//
	sal_uInt16 dxaHotZ;			//		width of hyphenation hot zone measured in twips
	sal_uInt16 cConsecHypLim;		//		number of lines allowed to have consecutive hyphens
	sal_uInt16 wSpare2;			//		reserved
	sal_Int32	dttmCreated;		// DTTM date and time document was created
	sal_Int32	dttmRevised;		// DTTM date and time document was last revised
	sal_Int32	dttmLastPrint;		// DTTM date and time document was last printed
	sal_Int16	nRevision;			//		number of times document has been revised since its creation
	sal_Int32	tmEdited;			//		time document was last edited
	sal_Int32	cWords;				//		count of words tallied by last Word Count execution
	sal_Int32	cCh;				//		count of characters tallied by last Word Count execution
	sal_Int16	cPg;				//		count of pages tallied by last Word Count execution
	sal_Int32	cParas;				//		count of paragraphs tallied by last Word Count execution
	sal_uInt16 rncEdn : 2;			//		restart endnote number code: 0 don�t restart endnote numbering, 1 section, 2 page
	sal_uInt16 nEdn : 14;			//		beginning endnote number
	sal_uInt16 epc : 2;			//		endnote position code: 0 at end of section, 3 at end of	document
	// sal_uInt16 nfcFtnRef : 4;		//		number format code for auto footnotes: 0 Arabic, 1 Upper case Roman, 2 Lower case Roman
								//		3 Upper case Letter, 4 Lower case Letter
								// ersetzt durch gleichlautendes Feld unten
	// sal_uInt16 nfcEdnRef : 4;		//		number format code for auto endnotes: 0 Arabic, 1 Upper case Roman, 2 Lower case Roman
								//		3 Upper case Letter, 4 Lower case Letter
								// ersetzt durch gleichlautendes Feld unten
	sal_uInt16 fPrintFormData : 1;	//		only print data inside of form fields
	sal_uInt16 fSaveFormData : 1;	//		only save document data that is inside of a form field.
	sal_uInt16 fShadeFormData : 1;	//		shade form fields
	sal_uInt16 : 2;				//		reserved
	sal_uInt16 fWCFtnEdn : 1;		//		when 1, include footnotes and endnotes in word count
	sal_Int32	cLines;				//		count of lines tallied by last Word Count operation
	sal_Int32	cWordsFtnEnd;		//		count of words in footnotes and endnotes tallied by last Word Count operation
	sal_Int32	cChFtnEdn;			//		count of characters in footnotes and endnotes tallied by last Word Count operation
	sal_Int16	cPgFtnEdn;			//		count of pages in footnotes and endnotes tallied by last Word Count operation
	sal_Int32	cParasFtnEdn;		//		count of paragraphs in footnotes and endnotes tallied by last Word Count operation
	sal_Int32	cLinesFtnEdn;		//		count of paragraphs in footnotes and endnotes tallied by last Word Count operation
	sal_Int32	lKeyProtDoc;		//		document protection password key, only valid if dop.fProtEnabled, dop.fLockAtn or dop.fLockRev are 1.
	sal_uInt16	wvkSaved : 3;		//		document view kind: 0 Normal view, 1 Outline view, 2 Page View
	sal_uInt16	wScaleSaved : 9;	//
	sal_uInt16	zkSaved : 2;

	// hier sollte bei nFib < 103	Schluss sein, sonst ist Datei fehlerhaft!

	/*
		bei nFib >= 103 gehts weiter:
	*/
	sal_uInt32 fNoTabForInd								:1; // see above in compatibility options
	sal_uInt32 fNoSpaceRaiseLower					:1;	// see above
	sal_uInt32 fSupressSpbfAfterPageBreak	:1;	// see above
	sal_uInt32 fWrapTrailSpaces						:1;	// see above
	sal_uInt32 fMapPrintTextColor					:1;	// see above
	sal_uInt32 fNoColumnBalance						:1;	// see above
	sal_uInt32 fConvMailMergeEsc					:1;	// see above
	sal_uInt32 fSupressTopSpacing					:1;	// see above
	sal_uInt32 fOrigWordTableRules				:1;	// see above
	sal_uInt32 fTransparentMetafiles			:1;	// see above
	sal_uInt32 fShowBreaksInFrames				:1;	// see above
	sal_uInt32 fSwapBordersFacingPgs			:1;	// see above
	sal_uInt32														:4;	// reserved
	sal_uInt32 fSuppressTopSpacingMac5		:1;	// Suppress extra line spacing at top
																				// of page like MacWord 5.x
	sal_uInt32 fTruncDxaExpand						:1;	// Expand/Condense by whole number of points
	sal_uInt32 fPrintBodyBeforeHdr				:1;	// Print body text before header/footer
	sal_uInt32 fNoLeading									:1;	// Don't add extra spacebetween rows of text
	sal_uInt32														:1;	// reserved
	sal_uInt32 fMWSmallCaps								:1;	// Use larger small caps like MacWord 5.x
	sal_uInt32													 :10;// reserved

	// hier sollte bei nFib <= 105	Schluss sein, sonst ist Datei fehlerhaft!

	/*
		bei nFib > 105 gehts weiter:
	*/
	sal_Int16	adt;							// Autoformat Document Type:
													// 0 for normal. 1 for letter, and 2 for email.
	WW8_DOPTYPOGRAPHY doptypography;	// siehe WW8STRUC.HXX
	WW8_DOGRID        dogrid;					// siehe WW8STRUC.HXX
	sal_uInt16 						:1; // reserved
	sal_uInt16 lvl					:4; // Which outline levels are showing in outline view
	sal_uInt16 						:4; // reserved
	sal_uInt16 fHtmlDoc				:1; // This file is based upon an HTML file
	sal_uInt16						:1;	// reserved
	sal_uInt16 fSnapBorder			:1;	// Snap table and page borders to page border
	sal_uInt16 fIncludeHeader		:1;	// Place header inside page border
	sal_uInt16 fIncludeFooter		:1;	// Place footer inside page border
	sal_uInt16 fForcePageSizePag	:1;	// Are we in online view
	sal_uInt16 fMinFontSizePag		:1;	// Are we auto-promoting
																// fonts to >= hpsZoonFontPag?
	sal_uInt16 fHaveVersions			:1; // versioning is turned on
	sal_uInt16 fAutoVersion				:1;	// autoversioning is enabled
	sal_uInt16									 :14;	// reserved
	// hier 12 Byte ueberspringen: ASUMI
	sal_Int32 cChWS;
	sal_Int32 cChWSFtnEdn;
	sal_Int32 grfDocEvents;
	// hier 4+30+8 Bytes ueberspringen
	sal_Int32 cDBC;
	sal_Int32 cDBCFtnEdn;
	// hier 4 Bytes ueberspringen
	sal_Int16 nfcFtnRef;
	sal_Int16 nfcEdnRef;
	sal_Int16 hpsZoonFontPag;
	sal_Int16 dywDispPag;

	// 2. Initialisier-Dummy:
	sal_uInt8    nDataEnd;

	/*
		nun wird lediglich noch ein Ctor benoetigt,
		dem die FIB-Nummer uebergeben werden muss
	*/
	WW8Dop( SvStream& rSt, sal_Int16 nFib, sal_Int32 nPos, sal_Int32 nSize );

	/* leider falsch, man braucht auch noch einen fuer den Export */
	WW8Dop();
	sal_Bool Write( SvStream& rStrm, WW8Fib& rFib );
};


inline short WW8SkipOdd(SvStream* pSt )
{
#if defined HP9000 || defined SINIX
	short bRet = pSt->Tell() & 0x1;
	if (bRet) {
		sal_uInt8 c;
		pSt->Read( &c, 1 );
	}
	return bRet;
#else
	if ( pSt->Tell() & 0x1 ){
		sal_uInt8 c;
		pSt->Read( &c, 1 );
		return 1;
	}
	return 0;
#endif
}


class WW8PLCF_HdFt
{
	WW8PLCF aPLCF;
	long nTextOfs;
	short nIdxOffset;

public:
	WW8PLCF_HdFt( SvStream* pSt, WW8Fib& rFib, WW8Dop& rDop );
//	~WW8PLCF_HdFt() {}
	sal_Bool GetTextPos( sal_uInt8 grpfIhdt, sal_uInt8 nWhich, WW8_CP& rStart, long& rLen );
	sal_Bool GetTextPosExact( short nIdx, WW8_CP& rStart, long& rLen );
	void UpdateIndex( sal_uInt8 grpfIhdt );
};


#endif

