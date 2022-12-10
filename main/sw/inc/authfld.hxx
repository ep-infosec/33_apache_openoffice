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


#ifndef SW_AUTHFLD_HXX
#define SW_AUTHFLD_HXX

#include "swdllapi.h"
#include <fldbas.hxx>
#include <toxe.hxx>

#define _SVSTDARR_LONGS
#include <svl/svstdarr.hxx>

class SwAuthDataArr;
/* -----------------21.09.99 13:32-------------------

 --------------------------------------------------*/
class SwAuthEntry
{
	String 		aAuthFields[AUTH_FIELD_END];
	sal_uInt16 		nRefCount;
public:
	SwAuthEntry() : nRefCount(0){}
	SwAuthEntry( const SwAuthEntry& rCopy );
	sal_Bool 			operator==(const SwAuthEntry& rComp);

	inline const String& 	GetAuthorField(ToxAuthorityField ePos)const;
	inline void				SetAuthorField(ToxAuthorityField ePos,
											const String& rField);

	void			AddRef()				{ ++nRefCount; }
	void			RemoveRef()				{ --nRefCount; }
	sal_uInt16			GetRefCount() 			{ return nRefCount; }
};
/* -----------------20.10.99 16:49-------------------

 --------------------------------------------------*/
struct SwTOXSortKey
{
	ToxAuthorityField	eField;
	sal_Bool				bSortAscending;
	SwTOXSortKey() :
		eField(AUTH_FIELD_END),
		bSortAscending(sal_True){}
};

/* -----------------14.09.99 16:15-------------------

 --------------------------------------------------*/
class SwAuthorityField;
class SortKeyArr;

class SW_DLLPUBLIC SwAuthorityFieldType : public SwFieldType
{
	SwDoc*			m_pDoc;
	SwAuthDataArr*	m_pDataArr;
	SvLongs*		m_pSequArr;
	SortKeyArr*		m_pSortKeyArr;
	sal_Unicode 	m_cPrefix;
	sal_Unicode 	m_cSuffix;
	sal_Bool			m_bIsSequence :1;
	sal_Bool			m_bSortByDocument :1;
    LanguageType    m_eLanguage;
    String          m_sSortAlgorithm;

	// @@@ private copy assignment, but public copy ctor? @@@
	const SwAuthorityFieldType& operator=( const SwAuthorityFieldType& );

protected:
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

public:
	SwAuthorityFieldType(SwDoc* pDoc);
	SwAuthorityFieldType( const SwAuthorityFieldType& );
	~SwAuthorityFieldType();

	virtual SwFieldType* Copy()    const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    inline void     SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }
    SwDoc*          GetDoc(){ return m_pDoc; }
	void				RemoveField(long nHandle);
	long				AddField(const String& rFieldContents);
	sal_Bool				AddField(long nHandle);
	void				DelSequenceArray()
						{
							m_pSequArr->Remove(0, m_pSequArr->Count());
						}

	const SwAuthEntry*	GetEntryByHandle(long nHandle) const;

	void 				GetAllEntryIdentifiers( SvStringsDtor& rToFill )const;
	const SwAuthEntry* 	GetEntryByIdentifier(const String& rIdentifier)const;

    bool                ChangeEntryContent(const SwAuthEntry* pNewEntry);
	// import interface
	sal_uInt16				AppendField(const SwAuthEntry& rInsert);
	long				GetHandle(sal_uInt16 nPos);

	sal_uInt16				GetSequencePos(long nHandle);

	sal_Bool				IsSequence() const 		{return m_bIsSequence;}
	void				SetSequence(sal_Bool bSet)
							{
								DelSequenceArray();
								m_bIsSequence = bSet;
							}

	void				SetPreSuffix( sal_Unicode cPre, sal_Unicode cSuf)
							{
								m_cPrefix = cPre;
								m_cSuffix = cSuf;
							}
	sal_Unicode			GetPrefix() const { return m_cPrefix;}
	sal_Unicode			GetSuffix() const { return m_cSuffix;}

	sal_Bool				IsSortByDocument() const {return m_bSortByDocument;}
	void				SetSortByDocument(sal_Bool bSet)
							{
								DelSequenceArray();
								m_bSortByDocument = bSet;
							}

	sal_uInt16				GetSortKeyCount() const ;
	const SwTOXSortKey*	GetSortKey(sal_uInt16 nIdx) const ;
	void				SetSortKeys(sal_uInt16 nKeyCount, SwTOXSortKey nKeys[]);

	//initui.cxx
	static const String& 	GetAuthFieldName(ToxAuthorityField eType);
	static const String& 	GetAuthTypeName(ToxAuthorityType eType);

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const String&   GetSortAlgorithm()const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const String& rSet) {m_sSortAlgorithm = rSet;}

};
/* -----------------14.09.99 16:15-------------------

 --------------------------------------------------*/
class SwAuthorityField : public SwField
{
    long            m_nHandle;
    mutable long    m_nTempSequencePos;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

public:
	SwAuthorityField(SwAuthorityFieldType* pType, const String& rFieldContents);
	SwAuthorityField(SwAuthorityFieldType* pType, long nHandle);
	~SwAuthorityField();

	const String&		GetFieldText(ToxAuthorityField eField) const;

	virtual void		SetPar1(const String& rStr);
	virtual SwFieldType* ChgTyp( SwFieldType* );

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    long                GetHandle() const       { return m_nHandle; }

    virtual String GetDescription() const;
};

// --- inlines -----------------------------------------------------------
inline const String& 	SwAuthEntry::GetAuthorField(ToxAuthorityField ePos)const
{
	DBG_ASSERT(AUTH_FIELD_END > ePos, "wrong index");
	return aAuthFields[ePos];
}
inline void	SwAuthEntry::SetAuthorField(ToxAuthorityField ePos, const String& rField)
{
	DBG_ASSERT(AUTH_FIELD_END > ePos, "wrong index");
	if(AUTH_FIELD_END > ePos)
		aAuthFields[ePos] = rField;
}

#endif

