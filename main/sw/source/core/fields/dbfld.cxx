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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <float.h>
#include <sfx2/app.hxx>
#include <svl/zforlist.hxx>
#include <svx/pageitem.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <frame.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <docfld.hxx>
#include <expfld.hxx>
#include <txtatr.hxx>
#include <unofldmid.h>
#include <switerator.hxx>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;
using ::rtl::OUString;

/*--------------------------------------------------------------------
	Beschreibung: Datenbanktrenner durch Punkte fuer Anzeige ersetzen
 --------------------------------------------------------------------*/

String lcl_DBTrennConv(const String& aContent)
{
	String sTmp(aContent);
	sal_Unicode* pStr = sTmp.GetBufferAccess();
	for( sal_uInt16 i = sTmp.Len(); i; --i, ++pStr )
		if( DB_DELIM == *pStr )
			*pStr = '.';
	return sTmp;
}

/*--------------------------------------------------------------------
	Beschreibung: DatenbankFeldTyp
 --------------------------------------------------------------------*/

SwDBFieldType::SwDBFieldType(SwDoc* pDocPtr, const String& rNam, const SwDBData& rDBData ) :
	SwValueFieldType( pDocPtr, RES_DBFLD ),
	aDBData(rDBData),
	sColumn(rNam),
	nRefCnt(0)
{
    if(aDBData.sDataSource.getLength() || aDBData.sCommand.getLength())
    {
        sName =  aDBData.sDataSource;
        sName += DB_DELIM;
        sName += (String)aDBData.sCommand;
        sName += DB_DELIM;
    }
	sName += GetColumnName();
}
//------------------------------------------------------------------------------
SwDBFieldType::~SwDBFieldType()
{
}
//------------------------------------------------------------------------------

SwFieldType* SwDBFieldType::Copy() const
{
	SwDBFieldType* pTmp = new SwDBFieldType(GetDoc(), sColumn, aDBData);
	return pTmp;
}

//------------------------------------------------------------------------------
const String& SwDBFieldType::GetName() const
{
	return sName;
}

//------------------------------------------------------------------------------

void SwDBFieldType::ReleaseRef()
{
	ASSERT(nRefCnt > 0, "RefCount kleiner 0!");

	if (--nRefCnt <= 0)
	{
		sal_uInt16 nPos = GetDoc()->GetFldTypes()->GetPos(this);

		if (nPos != USHRT_MAX)
		{
			GetDoc()->RemoveFldType(nPos);
			delete this;
		}
	}
}

sal_Bool SwDBFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
	{
    case FIELD_PROP_PAR1:
		rAny <<= aDBData.sDataSource;
		break;
    case FIELD_PROP_PAR2:
		rAny <<= aDBData.sCommand;
		break;
    case FIELD_PROP_PAR3:
		rAny <<= OUString(sColumn);
		break;
	case FIELD_PROP_SHORT1:
		rAny <<= aDBData.nCommandType;
		break;
    default:
		DBG_ERROR("illegal property");
	}
	return sal_True;
}

sal_Bool SwDBFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
	{
    case FIELD_PROP_PAR1:
		rAny >>= aDBData.sDataSource;
		break;
    case FIELD_PROP_PAR2:
		rAny >>= aDBData.sCommand;
		break;
    case FIELD_PROP_PAR3:
		{
			String sTmp;
			::GetString( rAny, sTmp );
			if( sTmp != sColumn )
			{
				sColumn = sTmp;
				SwIterator<SwFmtFld,SwFieldType> aIter( *this );
				SwFmtFld* pFmtFld = aIter.First();
				while(pFmtFld)
				{
					// Feld im Undo?
					SwTxtFld *pTxtFld = pFmtFld->GetTxtFld();
					if(pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
					{
						SwDBField* pDBField = (SwDBField*)pFmtFld->GetField();
						pDBField->ClearInitialized();
						pDBField->InitContent();
 					}
					pFmtFld = aIter.Next();
				}
			}
		}
		break;
	case FIELD_PROP_SHORT1:
		rAny >>= aDBData.nCommandType;
		break;
	default:
		DBG_ERROR("illegal property");
	}
	return sal_True;
}
/*--------------------------------------------------------------------
	Beschreibung: SwDBField
 --------------------------------------------------------------------*/

SwDBField::SwDBField(SwDBFieldType* pTyp, sal_uLong nFmt)
	: 	SwValueField(pTyp, nFmt),
		nSubType(0),
		bIsInBodyTxt(sal_True),
		bValidValue(sal_False),
		bInitialized(sal_False)
{
	if (GetTyp())
		((SwDBFieldType*)GetTyp())->AddRef();
	InitContent();
}

//------------------------------------------------------------------------------

SwDBField::~SwDBField()
{
	if (GetTyp())
		((SwDBFieldType*)GetTyp())->ReleaseRef();
}

//------------------------------------------------------------------------------

void SwDBField::InitContent()
{
	if (!IsInitialized())
	{
		aContent = '<';
		aContent += ((SwDBFieldType*)GetTyp())->GetColumnName();
		aContent += '>';
	}
}

//------------------------------------------------------------------------------

void SwDBField::InitContent(const String& rExpansion)
{
	if (rExpansion.Len() > 2)
	{
		if (rExpansion.GetChar(0) == '<' &&
			rExpansion.GetChar(rExpansion.Len() - 1) == '>')
		{
			String sColumn( rExpansion.Copy( 1, rExpansion.Len() - 2 ) );
			if( ::GetAppCmpStrIgnore().isEqual( sColumn,
							((SwDBFieldType *)GetTyp())->GetColumnName() ))
			{
				InitContent();
				return;
			}
		}
	}
	SetExpansion( rExpansion );
}

//------------------------------------------------------------------------------

String SwDBField::Expand() const
{
    String sRet;

    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        sRet = lcl_DBTrennConv(aContent);
    return sRet;
}

//------------------------------------------------------------------------------

SwField* SwDBField::Copy() const
{
	SwDBField *pTmp = new SwDBField((SwDBFieldType*)GetTyp(), GetFormat());
	pTmp->aContent 		= aContent;
	pTmp->bIsInBodyTxt 	= bIsInBodyTxt;
	pTmp->bValidValue 	= bValidValue;
	pTmp->bInitialized	= bInitialized;
	pTmp->nSubType		= nSubType;
	pTmp->SetValue(GetValue());
    pTmp->sFieldCode = sFieldCode;

	return pTmp;
}

String SwDBField::GetFieldName() const
{
    const String& rDBName = static_cast<SwDBFieldType*>(GetTyp())->GetName();
        //TODO/CLEANUP
        //Funktion tut nichts!
        //String sContent( SFX_APP()->LocalizeDBName(INI2NATIONAL,
        //                                    rDBName.GetToken(0, DB_DELIM)));
    String sContent( rDBName.GetToken(0, DB_DELIM) );

    if (sContent.Len() > 1)
    {
        sContent += DB_DELIM;
        sContent += rDBName.GetToken(1, DB_DELIM);
        sContent += DB_DELIM;
        sContent += rDBName.GetToken(2, DB_DELIM);
    }
    return lcl_DBTrennConv(sContent);
}

//------------------------------------------------------------------------------

void SwDBField::ChgValue( double d, sal_Bool bVal )
{
	bValidValue = bVal;
	SetValue(d);

	if( bValidValue )
		aContent = ((SwValueFieldType*)GetTyp())->ExpandValue(d, GetFormat(), GetLanguage());
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

SwFieldType* SwDBField::ChgTyp( SwFieldType* pNewType )
{
	SwFieldType* pOld = SwValueField::ChgTyp( pNewType );

	((SwDBFieldType*)pNewType)->AddRef();
	((SwDBFieldType*)pOld)->ReleaseRef();

	return pOld;
}

/*--------------------------------------------------------------------
	Beschreibung: Aktuellen Field-Value holen und chachen
 --------------------------------------------------------------------*/

void SwDBField::Evaluate()
{
	SwNewDBMgr* pMgr = GetDoc()->GetNewDBMgr();

	// erstmal loeschen
	bValidValue = sal_False;
	double nValue = DBL_MAX;
	const SwDBData& aTmpData = GetDBData();

    if(!pMgr || !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_True))
		return ;

	sal_uInt32 nFmt;

	// Passenden Spaltennamen suchen
	String aColNm( ((SwDBFieldType*)GetTyp())->GetColumnName() );

	SvNumberFormatter* pDocFormatter = GetDoc()->GetNumberFormatter();
	pMgr->GetMergeColumnCnt(aColNm, GetLanguage(), aContent, &nValue, &nFmt);
	if( !( nSubType & nsSwExtendedSubType::SUB_OWN_FMT ) )
		SetFormat( nFmt = pMgr->GetColumnFmt( aTmpData.sDataSource, aTmpData.sCommand,
										aColNm, pDocFormatter, GetLanguage() ));

	if( DBL_MAX != nValue )
	{
		sal_Int32 nColumnType = pMgr->GetColumnType(aTmpData.sDataSource, aTmpData.sCommand, aColNm);
		if(	DataType::DATE == nColumnType  || DataType::TIME == nColumnType  ||
                 DataType::TIMESTAMP  == nColumnType)

		{
			Date aStandard(1,1,1900);
			if (*pDocFormatter->GetNullDate() != aStandard)
				nValue += (aStandard - *pDocFormatter->GetNullDate());
		}
		bValidValue = sal_True;
		SetValue(nValue);
		aContent = ((SwValueFieldType*)GetTyp())->ExpandValue(nValue, GetFormat(), GetLanguage());
	}
	else
	{
		SwSbxValue aVal;
		aVal.PutString( aContent );

		if (aVal.IsNumeric())
		{
			SetValue(aVal.GetDouble());

			SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
            if (nFmt && nFmt != SAL_MAX_UINT32 && !pFormatter->IsTextFormat(nFmt))
				bValidValue = sal_True;	// Wegen Bug #60339 nicht mehr bei allen Strings
		}
		else
		{
			// Bei Strings sal_True wenn Laenge > 0 sonst sal_False
			SetValue(aContent.Len() ? 1 : 0);
		}
	}
	bInitialized = sal_True;
}

/*--------------------------------------------------------------------
	Beschreibung: Namen erfragen
 --------------------------------------------------------------------*/

const String& SwDBField::GetPar1() const
{
	return ((SwDBFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

sal_uInt16 SwDBField::GetSubType() const
{
	return nSubType;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDBField::SetSubType(sal_uInt16 nType)
{
	nSubType = nType;
}


sal_Bool SwDBField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
	{
	case FIELD_PROP_BOOL1:
		{
			sal_Bool bTemp = 0 == (GetSubType()&nsSwExtendedSubType::SUB_OWN_FMT);
			rAny.setValue(&bTemp, ::getBooleanCppuType());
		}
		break;
    case FIELD_PROP_BOOL2:
    {
        sal_Bool bVal = 0 == (GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE);
        rAny.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    case FIELD_PROP_FORMAT:
		rAny <<= (sal_Int32)GetFormat();
		break;
	case FIELD_PROP_PAR1:
		rAny <<= OUString(aContent);
		break;
    case FIELD_PROP_PAR2:
        rAny <<= OUString(sFieldCode);
        break;
    default:
		DBG_ERROR("illegal property");
	}
	return sal_True;

}

sal_Bool SwDBField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
	{
	case FIELD_PROP_BOOL1:
		if( *(sal_Bool*)rAny.getValue() )
			SetSubType(GetSubType()&~nsSwExtendedSubType::SUB_OWN_FMT);
		else
			SetSubType(GetSubType()|nsSwExtendedSubType::SUB_OWN_FMT);
		break;
    case FIELD_PROP_BOOL2:
    {
        sal_uInt16 nSubTyp = GetSubType();
        sal_Bool bVisible = sal_False;
        if(!(rAny >>= bVisible))
            return sal_False;
        if(bVisible)
            nSubTyp &= ~nsSwExtendedSubType::SUB_INVISIBLE;
        else
            nSubTyp |= nsSwExtendedSubType::SUB_INVISIBLE;
        SetSubType(nSubTyp);
        //invalidate text node
        if(GetTyp())
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *GetTyp() );
            SwFmtFld* pFmtFld = aIter.First();
            while(pFmtFld)
            {
                SwTxtFld *pTxtFld = pFmtFld->GetTxtFld();
                if(pTxtFld && (SwDBField*)pFmtFld->GetField() == this )
                {
                    //notify the change
                    pTxtFld->NotifyContentChange(*pFmtFld);
                    break;
                }
                pFmtFld = aIter.Next();
            }
        }
    }
    break;
    case FIELD_PROP_FORMAT:
		{
			sal_Int32 nTemp = 0;
			rAny >>= nTemp;
			SetFormat(nTemp);
		}
		break;
	case FIELD_PROP_PAR1:
		::GetString( rAny, aContent );
		break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, sFieldCode );
    break;
	default:
		DBG_ERROR("illegal property");
	}
	return sal_True;
}

/*--------------------------------------------------------------------
	Beschreibung: Basisklasse fuer alle weiteren Datenbankfelder
 --------------------------------------------------------------------*/

SwDBNameInfField::SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFmt) :
	SwField(pTyp, nFmt),
    aDBData(rDBData),
    nSubType(0)
{
}

//------------------------------------------------------------------------------

SwDBData SwDBNameInfField::GetDBData(SwDoc* pDoc)
{
	SwDBData aRet;
	if(aDBData.sDataSource.getLength())
		aRet = aDBData;
	else
		aRet = pDoc->GetDBData();
	return aRet;
}

// #111840#
void SwDBNameInfField::SetDBData(const SwDBData & rDBData)
{
    aDBData = rDBData;
}

//------------------------------------------------------------------------------

String SwDBNameInfField::GetFieldName() const
{
    String sStr( SwField::GetFieldName() );
    if (aDBData.sDataSource.getLength())
    {
        sStr += ':';
        sStr += String(aDBData.sDataSource);
        sStr += DB_DELIM;
        sStr += String(aDBData.sCommand);
    }
	return lcl_DBTrennConv(sStr);
}

sal_Bool SwDBNameInfField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
	{
	case FIELD_PROP_PAR1:
		rAny <<= aDBData.sDataSource;
		break;
	case FIELD_PROP_PAR2:
		rAny <<= aDBData.sCommand;
		break;
	case FIELD_PROP_SHORT1:
		rAny <<= aDBData.nCommandType;
		break;
    case FIELD_PROP_BOOL2:
    {
        sal_Bool bVal = 0 == (GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE);
        rAny.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    default:
		DBG_ERROR("illegal property");
	}
	return sal_True;
}

sal_Bool SwDBNameInfField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
	{
	case FIELD_PROP_PAR1:
		rAny >>= aDBData.sDataSource;
		break;
	case FIELD_PROP_PAR2:
		rAny >>= aDBData.sCommand;
		break;
	case FIELD_PROP_SHORT1:
		rAny >>= aDBData.nCommandType;
		break;
    case FIELD_PROP_BOOL2:
    {
        sal_uInt16 nSubTyp = GetSubType();
        sal_Bool bVisible = sal_False;
        if(!(rAny >>= bVisible))
            return sal_False;
        if(bVisible)
            nSubTyp &= ~nsSwExtendedSubType::SUB_INVISIBLE;
        else
            nSubTyp |= nsSwExtendedSubType::SUB_INVISIBLE;
        SetSubType(nSubTyp);
    }
    break;
    default:
		DBG_ERROR("illegal property");
	}
	return sal_True;
}

sal_uInt16 SwDBNameInfField::GetSubType() const
{
    return nSubType;
}

void SwDBNameInfField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}

/*--------------------------------------------------------------------
	Beschreibung: NaechsterDatensatz
 --------------------------------------------------------------------*/

SwDBNextSetFieldType::SwDBNextSetFieldType()
	: SwFieldType( RES_DBNEXTSETFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBNextSetFieldType::Copy() const
{
	SwDBNextSetFieldType* pTmp = new SwDBNextSetFieldType();
    return pTmp;
}
/*--------------------------------------------------------------------
	Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

SwDBNextSetField::SwDBNextSetField(SwDBNextSetFieldType* pTyp,
								   const String& rCond,
								   const String& ,
								   const SwDBData& rDBData) :
	SwDBNameInfField(pTyp, rDBData), aCond(rCond), bCondValid(sal_True)
{}

//------------------------------------------------------------------------------

String SwDBNextSetField::Expand() const
{
	return aEmptyStr;
}

//------------------------------------------------------------------------------

SwField* SwDBNextSetField::Copy() const
{
	SwDBNextSetField *pTmp = new SwDBNextSetField((SwDBNextSetFieldType*)GetTyp(),
										 aCond, aEmptyStr, GetDBData());
    pTmp->SetSubType(GetSubType());
    pTmp->bCondValid = bCondValid;
	return pTmp;
}
//------------------------------------------------------------------------------

void SwDBNextSetField::Evaluate(SwDoc* pDoc)
{
	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
	const SwDBData& rData = GetDBData();
	if( !bCondValid ||
            !pMgr || !pMgr->IsDataSourceOpen(rData.sDataSource, rData.sCommand, sal_False))
		return ;
    pMgr->ToNextRecord(rData.sDataSource, rData.sCommand);
}

/*--------------------------------------------------------------------
	Beschreibung: Bedingung
 --------------------------------------------------------------------*/

const String& SwDBNextSetField::GetPar1() const
{
	return aCond;
}

void SwDBNextSetField::SetPar1(const String& rStr)
{
	aCond = rStr;
}

sal_Bool SwDBNextSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_PAR3:
		rAny <<= OUString(aCond);
		break;
	default:
        bRet = SwDBNameInfField::QueryValue( rAny, nWhichId );
	}
	return bRet;
}

sal_Bool SwDBNextSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_PAR3:
		::GetString( rAny, aCond );
		break;
	default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
	}
	return bRet;
}

/*--------------------------------------------------------------------
	Beschreibung: Datensatz mit bestimmter ID
 --------------------------------------------------------------------*/

SwDBNumSetFieldType::SwDBNumSetFieldType() :
	SwFieldType( RES_DBNUMSETFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBNumSetFieldType::Copy() const
{
	SwDBNumSetFieldType* pTmp = new SwDBNumSetFieldType();
	return pTmp;
}

/*--------------------------------------------------------------------
	Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

SwDBNumSetField::SwDBNumSetField(SwDBNumSetFieldType* pTyp,
								 const String& rCond,
								 const String& rDBNum,
								 const SwDBData& rDBData) :
	SwDBNameInfField(pTyp, rDBData),
	aCond(rCond),
	aPar2(rDBNum),
	bCondValid(sal_True)
{}

//------------------------------------------------------------------------------

String SwDBNumSetField::Expand() const
{
	return aEmptyStr;
}

//------------------------------------------------------------------------------

SwField* SwDBNumSetField::Copy() const
{
	SwDBNumSetField *pTmp = new SwDBNumSetField((SwDBNumSetFieldType*)GetTyp(),
										 aCond, aPar2, GetDBData());
	pTmp->bCondValid = bCondValid;
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

void SwDBNumSetField::Evaluate(SwDoc* pDoc)
{
	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
	const SwDBData& aTmpData = GetDBData();

	if( bCondValid && pMgr && pMgr->IsInMerge() &&
                        pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_True))
	{	// Bedingug OK -> aktuellen Set einstellen
		pMgr->ToRecordId(Max((sal_uInt16)aPar2.ToInt32(), sal_uInt16(1))-1);
	}
}

/*--------------------------------------------------------------------
	Beschreibung: LogDBName
 --------------------------------------------------------------------*/

const String& SwDBNumSetField::GetPar1() const
{
	return aCond;
}

void SwDBNumSetField::SetPar1(const String& rStr)
{
	aCond = rStr;
}

/*--------------------------------------------------------------------
	Beschreibung: Bedingung
 --------------------------------------------------------------------*/

String SwDBNumSetField::GetPar2() const
{
	return aPar2;
}

void SwDBNumSetField::SetPar2(const String& rStr)
{
	aPar2 = rStr;
}

sal_Bool SwDBNumSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_PAR3:
		rAny <<= OUString(aCond);
		break;
	case FIELD_PROP_FORMAT:
		rAny <<= (sal_Int32)aPar2.ToInt32();
		break;
	default:
        bRet = SwDBNameInfField::QueryValue(rAny, nWhichId );
	}
	return bRet;
}

sal_Bool    SwDBNumSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_PAR3:
		::GetString( rAny, aCond );
		break;
	case FIELD_PROP_FORMAT:
		{
			sal_Int32 nVal = 0;
			rAny >>= nVal;
			aPar2 = String::CreateFromInt32(nVal);
		}
		break;
	default:
        bRet = SwDBNameInfField::PutValue(rAny, nWhichId );
	}
	return bRet;
}

/*--------------------------------------------------------------------
	Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

SwDBNameFieldType::SwDBNameFieldType(SwDoc* pDocument)
	: SwFieldType( RES_DBNAMEFLD )
{
	pDoc = pDocument;
}
//------------------------------------------------------------------------------

String SwDBNameFieldType::Expand(sal_uLong ) const
{
	const SwDBData aData = pDoc->GetDBData();
	String sRet(aData.sDataSource);
	sRet += '.';
	sRet += (String)aData.sCommand;
	return sRet;
}
//------------------------------------------------------------------------------

SwFieldType* SwDBNameFieldType::Copy() const
{
	SwDBNameFieldType *pTmp = new SwDBNameFieldType(pDoc);
	return pTmp;
}

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
	Beschreibung: Name der angedockten DB
 --------------------------------------------------------------------*/

SwDBNameField::SwDBNameField(SwDBNameFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFmt)
	: SwDBNameInfField(pTyp, rDBData, nFmt)
{}

//------------------------------------------------------------------------------

String SwDBNameField::Expand() const
{
    String sRet;
    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        sRet = ((SwDBNameFieldType*)GetTyp())->Expand(GetFormat());
    return sRet;
}

//------------------------------------------------------------------------------

SwField* SwDBNameField::Copy() const
{
	SwDBNameField *pTmp = new SwDBNameField((SwDBNameFieldType*)GetTyp(), GetDBData());
	pTmp->ChangeFormat(GetFormat());
	pTmp->SetLanguage(GetLanguage());
    pTmp->SetSubType(GetSubType());
	return pTmp;
}


sal_Bool SwDBNameField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    return SwDBNameInfField::QueryValue(rAny, nWhichId );
}

sal_Bool SwDBNameField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    return SwDBNameInfField::PutValue(rAny, nWhichId );
}
/*--------------------------------------------------------------------
	Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

SwDBSetNumberFieldType::SwDBSetNumberFieldType()
	: SwFieldType( RES_DBSETNUMBERFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBSetNumberFieldType::Copy() const
{
	SwDBSetNumberFieldType *pTmp = new SwDBSetNumberFieldType;
	return pTmp;
}

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
	Beschreibung: SetNumber der angedockten DB
 --------------------------------------------------------------------*/

SwDBSetNumberField::SwDBSetNumberField(SwDBSetNumberFieldType* pTyp,
									   const SwDBData& rDBData,
									   sal_uLong nFmt)
	: SwDBNameInfField(pTyp, rDBData, nFmt), nNumber(0)
{}

//------------------------------------------------------------------------------

String SwDBSetNumberField::Expand() const
{
    if(0 !=(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE) || nNumber == 0)
		return aEmptyStr;
	else
		return FormatNumber((sal_uInt16)nNumber, GetFormat());
}

//------------------------------------------------------------------------------

void SwDBSetNumberField::Evaluate(SwDoc* pDoc)
{
	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();

    const SwDBData& aTmpData = GetDBData();
    if (!pMgr || !pMgr->IsInMerge() ||
        !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_False))
        return;
    nNumber = pMgr->GetSelectedRecordId();
}


//------------------------------------------------------------------------------

SwField* SwDBSetNumberField::Copy() const
{
	SwDBSetNumberField *pTmp =
		new SwDBSetNumberField((SwDBSetNumberFieldType*)GetTyp(), GetDBData(), GetFormat());
	pTmp->SetLanguage(GetLanguage());
	pTmp->SetSetNumber(nNumber);
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

sal_Bool SwDBSetNumberField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_USHORT1:
		rAny <<= (sal_Int16)GetFormat();
		break;
	case FIELD_PROP_FORMAT:
		rAny <<= nNumber;
		break;
	default:
        bRet = SwDBNameInfField::QueryValue( rAny, nWhichId );
	}
	return bRet;
}

sal_Bool SwDBSetNumberField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
	sal_Bool bRet = sal_True;
    switch( nWhichId )
	{
	case FIELD_PROP_USHORT1:
		{
			sal_Int16 nSet = 0;
			rAny >>= nSet;
			if(nSet < (sal_Int16) SVX_NUMBER_NONE )
				SetFormat(nSet);
			else {
				//exception(wrong_value)
				;
            }
		}
		break;
	case FIELD_PROP_FORMAT:
		rAny >>= nNumber;
		break;
	default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
	}
	return bRet;
}


