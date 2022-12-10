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


#ifndef _FTNINFO_HXX
#define _FTNINFO_HXX

#include <tools/string.hxx>
#include "swdllapi.h"
#include <calbck.hxx>
#include <editeng/numitem.hxx>

class SwTxtFmtColl;
class SwPageDesc;
class SwCharFmt;
class SwDoc;

class SW_DLLPUBLIC SwEndNoteInfo : public SwClient
{
	SwDepend  	aPageDescDep;
	SwDepend 	aCharFmtDep, aAnchorCharFmtDep;
	String 		sPrefix;
	String 		sSuffix;
protected:
    bool        m_bEndNote;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
	SvxNumberType aFmt;
	sal_uInt16 	  nFtnOffset;

	void 		ChgPageDesc( SwPageDesc *pDesc );
	SwPageDesc* GetPageDesc( SwDoc &rDoc ) const;
    bool        KnowsPageDesc() const;
    bool        DependsOn( const SwPageDesc* ) const;

	void SetFtnTxtColl(SwTxtFmtColl& rColl);
	SwTxtFmtColl* GetFtnTxtColl() const { return  (SwTxtFmtColl*) GetRegisteredIn(); } // kann 0 sein

	SwCharFmt* GetCharFmt(SwDoc &rDoc) const;
	void SetCharFmt( SwCharFmt* );
	SwClient   *GetCharFmtDep() const { return (SwClient*)&aCharFmtDep; }

	SwCharFmt* GetAnchorCharFmt(SwDoc &rDoc) const;
	void SetAnchorCharFmt( SwCharFmt* );
	SwClient   *GetAnchorCharFmtDep() const { return (SwClient*)&aAnchorCharFmtDep; }

	SwEndNoteInfo & operator=(const SwEndNoteInfo&);
	sal_Bool operator==( const SwEndNoteInfo &rInf ) const;

	SwEndNoteInfo( SwTxtFmtColl *pTxtColl = 0);
	SwEndNoteInfo(const SwEndNoteInfo&);

	const String& GetPrefix() const 		{ return sPrefix; }
	const String& GetSuffix() const 		{ return sSuffix; }

	void SetPrefix(const String& rSet)		{ sPrefix = rSet; }
	void SetSuffix(const String& rSet)		{ sSuffix = rSet; }
    void ReleaseCollection() { if ( GetRegisteredInNonConst() ) GetRegisteredInNonConst()->Remove( this ); }
};

enum SwFtnPos
{
	//Derzeit nur PAGE und CHAPTER. CHAPTER == Dokumentendenoten.
	FTNPOS_PAGE = 1,
	FTNPOS_CHAPTER = 8
};

enum SwFtnNum
{
	FTNNUM_PAGE, FTNNUM_CHAPTER, FTNNUM_DOC
};

class SW_DLLPUBLIC SwFtnInfo: public SwEndNoteInfo
{
    using SwEndNoteInfo::operator ==;

public:
	String    aQuoVadis;
	String	  aErgoSum;
	SwFtnPos  ePos;
	SwFtnNum  eNum;


	SwFtnInfo& operator=(const SwFtnInfo&);

	sal_Bool operator==( const SwFtnInfo &rInf ) const;

	SwFtnInfo(SwTxtFmtColl* pTxtColl = 0);
	SwFtnInfo(const SwFtnInfo&);
};


#endif
