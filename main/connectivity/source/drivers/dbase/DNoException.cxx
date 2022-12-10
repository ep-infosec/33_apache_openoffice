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
#include "precompiled_connectivity.hxx"
#include "dbase/DTable.hxx"
#include "dbase/DIndex.hxx"
#include "dbase/dindexnode.hxx"
#include <tools/debug.hxx>
#include "diagnose_ex.h"

#include <sal/types.h>
#include <algorithm>
#include <rtl/logfile.hxx>

using namespace connectivity;
using namespace connectivity::dbase;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
//------------------------------------------------------------------
sal_Bool ODbaseTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseTable::seekRow" );
	// ----------------------------------------------------------
	// Positionierung vorbereiten:
	OSL_ENSURE(m_pFileStream,"ODbaseTable::seekRow: FileStream is NULL!");

	sal_uInt32  nNumberOfRecords = (sal_uInt32)m_aHeader.db_anz;
	sal_uInt32 nTempPos = m_nFilePos;
	m_nFilePos = nCurPos;

	switch(eCursorPosition)
	{
		case IResultSetHelper::NEXT:
			++m_nFilePos;
			break;
		case IResultSetHelper::PRIOR:
			if (m_nFilePos > 0)
				--m_nFilePos;
			break;
		case IResultSetHelper::FIRST:
			m_nFilePos = 1;
			break;
		case IResultSetHelper::LAST:
			m_nFilePos = nNumberOfRecords;
			break;
		case IResultSetHelper::RELATIVE:
			m_nFilePos = (((sal_Int32)m_nFilePos) + nOffset < 0) ? 0L
							: (sal_uInt32)(((sal_Int32)m_nFilePos) + nOffset);
			break;
		case IResultSetHelper::ABSOLUTE:
		case IResultSetHelper::BOOKMARK:
			m_nFilePos = (sal_uInt32)nOffset;
			break;
	}

	if (m_nFilePos > (sal_Int32)nNumberOfRecords)
		m_nFilePos = (sal_Int32)nNumberOfRecords + 1;

	if (m_nFilePos == 0 || m_nFilePos == (sal_Int32)nNumberOfRecords + 1)
		goto Error;
	else
	{
		sal_uInt16 nEntryLen = m_aHeader.db_slng;

		OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");
		sal_Int32 nPos = m_aHeader.db_kopf + (sal_Int32)(m_nFilePos-1) * nEntryLen;

		sal_uIntPtr nLen = m_pFileStream->Seek(nPos);
		if (m_pFileStream->GetError() != ERRCODE_NONE)
			goto Error;

		nLen = m_pFileStream->Read((char*)m_pBuffer, nEntryLen);
		if (m_pFileStream->GetError() != ERRCODE_NONE)
			goto Error;
	}
	goto End;

Error:
	switch(eCursorPosition)
	{
		case IResultSetHelper::PRIOR:
		case IResultSetHelper::FIRST:
			m_nFilePos = 0;
			break;
		case IResultSetHelper::LAST:
		case IResultSetHelper::NEXT:
		case IResultSetHelper::ABSOLUTE:
		case IResultSetHelper::RELATIVE:
			if (nOffset > 0)
				m_nFilePos = nNumberOfRecords + 1;
			else if (nOffset < 0)
				m_nFilePos = 0;
			break;
		case IResultSetHelper::BOOKMARK:
			m_nFilePos = nTempPos;	 // vorherige Position
	}
	//	aStatus.Set(SDB_STAT_NO_DATA_FOUND);
	return sal_False;

End:
	nCurPos = m_nFilePos;
	return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool ODbaseTable::ReadMemo(sal_uIntPtr nBlockNo, ORowSetValue& aVariable)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseTable::ReadMemo" );
	sal_Bool bIsText = sal_True;
	//	SdbConnection* pConnection = GetConnection();

	m_pMemoStream->Seek(nBlockNo * m_aMemoHeader.db_size);
	switch (m_aMemoHeader.db_typ)
	{
		case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
		{
			const char cEOF = (char) 0x1a;
			ByteString aBStr;
			static char aBuf[514];
			aBuf[512] = 0;			// sonst kann der Zufall uebel mitspielen
			sal_Bool bReady = sal_False;

			do
			{
				m_pMemoStream->Read(&aBuf,512);

				sal_uInt16 i = 0;
				while (aBuf[i] != cEOF && ++i < 512)
					;
				bReady = aBuf[i] == cEOF;

				aBuf[i] = 0;
				aBStr += aBuf;

			} while (!bReady && !m_pMemoStream->IsEof() && aBStr.Len() < STRING_MAXLEN);

			::rtl::OUString aStr(aBStr.GetBuffer(), aBStr.Len(),getConnection()->getTextEncoding());
			aVariable = aStr;

		} break;
		case MemoFoxPro:
		case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
		{
			char sHeader[4];
			m_pMemoStream->Read(sHeader,4);
			// Foxpro stores text and binary data
			if (m_aMemoHeader.db_typ == MemoFoxPro)
			{
				if (((sal_uInt8)sHeader[0]) != 0 || ((sal_uInt8)sHeader[1]) != 0 || ((sal_uInt8)sHeader[2]) != 0)
				{
//					String aText = String(SdbResId(STR_STAT_IResultSetHelper::INVALID));
//					aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//					aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//					aStatus.Set(SDB_STAT_ERROR,
//							String::CreateFromAscii("01000"),
//							aStatus.CreateErrorMessage(aText),
//							0, String() );
					return sal_False;
				}

				bIsText = sHeader[3] != 0;
			}
			else if (((sal_uInt8)sHeader[0]) != 0xFF || ((sal_uInt8)sHeader[1]) != 0xFF || ((sal_uInt8)sHeader[2]) != 0x08)
			{
//				String aText = String(SdbResId(STR_STAT_IResultSetHelper::INVALID));
//				aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//				aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//				aStatus.Set(SDB_STAT_ERROR,
//						String::CreateFromAscii("01000"),
//						aStatus.CreateErrorMessage(aText),
//						0, String() );
				return sal_False;
			}

			sal_uInt32 nLength;
			(*m_pMemoStream) >> nLength;

			if (m_aMemoHeader.db_typ == MemodBaseIV)
				nLength -= 8;

			//	char cChar;
			::rtl::OUString aStr;
			while ( nLength > STRING_MAXLEN )
			{
				ByteString aBStr;
				aBStr.Expand(STRING_MAXLEN);
				m_pMemoStream->Read(aBStr.AllocBuffer(STRING_MAXLEN),STRING_MAXLEN);
				aStr += ::rtl::OUString(aBStr.GetBuffer(),aBStr.Len(), getConnection()->getTextEncoding());
				nLength -= STRING_MAXLEN;
			}
			if ( nLength > 0 )
			{
				ByteString aBStr;
				aBStr.Expand(static_cast<xub_StrLen>(nLength));
				m_pMemoStream->Read(aBStr.AllocBuffer(static_cast<xub_StrLen>(nLength)),nLength);
				//	aBStr.ReleaseBufferAccess();

				aStr += ::rtl::OUString(aBStr.GetBuffer(),aBStr.Len(), getConnection()->getTextEncoding());

			}
			if ( aStr.getLength() )
				aVariable = aStr;
		}
	}
	return sal_True;
}
// -----------------------------------------------------------------------------
void ODbaseTable::AllocBuffer()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseTable::AllocBuffer" );
	sal_uInt16 nSize = m_aHeader.db_slng;
	OSL_ENSURE(nSize > 0, "Size too small");

	if (m_nBufferSize != nSize)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	// Falls noch kein Puffer vorhanden: allozieren:
	if (m_pBuffer == NULL && nSize > 0)
	{
		m_nBufferSize = nSize;
		m_pBuffer		= new sal_uInt8[m_nBufferSize+1];
	}
}
// -----------------------------------------------------------------------------
sal_Bool ODbaseTable::WriteBuffer()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseTable::WriteBuffer" );
	OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");

	// Auf gewuenschten Record positionieren:
	long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
	m_pFileStream->Seek(nPos);
	return m_pFileStream->Write((char*) m_pBuffer, m_aHeader.db_slng) > 0;
}
// -----------------------------------------------------------------------------
sal_Int32 ODbaseTable::getCurrentLastPos() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseTable::getCurrentLastPos" );
	return m_aHeader.db_anz;
}
// -----------------------------------------------------------------------------
//==================================================================
// ONDXNode
//==================================================================

//------------------------------------------------------------------
void ONDXNode::Read(SvStream &rStream, ODbaseIndex& rIndex)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXNode::Read" );
	rStream >> aKey.nRecord; // schluessel

	if (rIndex.getHeader().db_keytype)
	{
		double aDbl;
		rStream >> aDbl;
		aKey = ONDXKey(aDbl,aKey.nRecord);
	}
	else
	{
		ByteString aBuf;
		sal_uInt16 nLen = rIndex.getHeader().db_keylen;
		char* pStr = aBuf.AllocBuffer(nLen+1);

		rStream.Read(pStr,nLen);
		pStr[nLen] = 0;
		aBuf.ReleaseBufferAccess();
		aBuf.EraseTrailingChars();

		//	aKey = ONDXKey((aBuf,rIndex.GetDBFConnection()->GetCharacterSet()) ,aKey.nRecord);
		aKey = ONDXKey(::rtl::OUString(aBuf.GetBuffer(),aBuf.Len(),rIndex.m_pTable->getConnection()->getTextEncoding()) ,aKey.nRecord);
	}
	rStream >> aChild;
}

union
{
	double aDbl;
	char   aData[128];
} aNodeData;
//------------------------------------------------------------------
void ONDXNode::Write(SvStream &rStream, const ONDXPage& rPage) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXNode::Write" );
	const ODbaseIndex& rIndex = rPage.GetIndex();
	if (!rIndex.isUnique() || rPage.IsLeaf())
		rStream << (sal_uInt32)aKey.nRecord; // schluessel
	else
		rStream << (sal_uInt32)0;	// schluessel

	if (rIndex.getHeader().db_keytype) // double
	{
		if (aKey.getValue().isNull())
		{
			memset(aNodeData.aData,0,rIndex.getHeader().db_keylen);
			rStream.Write((sal_uInt8*)aNodeData.aData,rIndex.getHeader().db_keylen);
		}
		else
			rStream << (double) aKey.getValue();
	}
	else
	{
		memset(aNodeData.aData,0x20,rIndex.getHeader().db_keylen);
		if (!aKey.getValue().isNull())
		{
			::rtl::OUString sValue = aKey.getValue();
			ByteString aText(sValue.getStr(), rIndex.m_pTable->getConnection()->getTextEncoding());
			strncpy(aNodeData.aData,aText.GetBuffer(),std::min(rIndex.getHeader().db_keylen, aText.Len()));
		}
		rStream.Write((sal_uInt8*)aNodeData.aData,rIndex.getHeader().db_keylen);
	}
	rStream << aChild;
}


//------------------------------------------------------------------
ONDXPagePtr& ONDXNode::GetChild(ODbaseIndex* pIndex, ONDXPage* pParent)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXNode::GetChild" );
	if (!aChild.Is() && pIndex)
	{
		aChild = pIndex->CreatePage(aChild.GetPagePos(),pParent,aChild.HasPage());
	}
	return aChild;
}

//==================================================================
// ONDXKey
//==================================================================
//------------------------------------------------------------------
sal_Bool ONDXKey::IsText(sal_Int32 eType)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXKey::IsText" );
	return eType == DataType::VARCHAR || eType == DataType::CHAR;
}

//------------------------------------------------------------------
StringCompare ONDXKey::Compare(const ONDXKey& rKey) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXKey::Compare" );
	//	DBG_ASSERT(is(), "Falscher Indexzugriff");
	StringCompare eResult;

	if (getValue().isNull())
	{
		if (rKey.getValue().isNull() || (rKey.IsText(getDBType()) && !rKey.getValue().getString().getLength()))
			eResult = COMPARE_EQUAL;
		else
			eResult = COMPARE_LESS;
	}
	else if (rKey.getValue().isNull())
	{
		if (getValue().isNull() || (IsText(getDBType()) && !getValue().getString().getLength()))
			eResult = COMPARE_EQUAL;
		else
			eResult = COMPARE_GREATER;
	}
	else if (IsText(getDBType()))
	{
		sal_Int32 nRes = getValue().getString().compareTo(rKey.getValue());
		eResult = (nRes > 0) ? COMPARE_GREATER : (nRes == 0) ? COMPARE_EQUAL : COMPARE_LESS;
	}
	else
	{
		double m = getValue(),n = rKey.getValue();
		eResult = (m > n) ? COMPARE_GREATER : (n == m) ? COMPARE_EQUAL : COMPARE_LESS;
	}

	// Record vergleich, wenn Index !Unique
	if (eResult == COMPARE_EQUAL && nRecord && rKey.nRecord)
		eResult = (nRecord > rKey.nRecord) ? COMPARE_GREATER :
				  (nRecord == rKey.nRecord) ? COMPARE_EQUAL : COMPARE_LESS;

	return eResult;
}
// -----------------------------------------------------------------------------
void ONDXKey::setValue(const ORowSetValue& _rVal)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXKey::setValue" );
	xValue = _rVal;
}
// -----------------------------------------------------------------------------
const ORowSetValue& ONDXKey::getValue() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXKey::getValue" );
	return xValue;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPagePtr& rPage)
{
	rStream >> rPage.nPagePos;
	return rStream;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPagePtr& rPage)
{
	rStream << rPage.nPagePos;
	return rStream;
}
// -----------------------------------------------------------------------------
//==================================================================
// ONDXPagePtr
//==================================================================
//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(const ONDXPagePtr& rRef)
			  :ONDXPageRef(rRef)
			  ,nPagePos(rRef.nPagePos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPagePtr::ONDXPagePtr" );
}

//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(ONDXPage* pRefPage)
			  :ONDXPageRef(pRefPage)
			  ,nPagePos(0)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPagePtr::ONDXPagePtr" );
	if (pRefPage)
		nPagePos = pRefPage->GetPagePos();
}
//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator=(const ONDXPagePtr& rRef)
{
	ONDXPageRef::operator=(rRef);
	nPagePos = rRef.nPagePos;
	return *this;
}

//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator= (ONDXPage* pRef)
{
	ONDXPageRef::operator=(pRef);
	nPagePos = (pRef) ? pRef->GetPagePos() : 0;
	return *this;
}
// -----------------------------------------------------------------------------
static sal_uInt32 nValue;
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPage& rPage)
{
	rStream.Seek(rPage.GetPagePos() * 512);
	rStream >> nValue >> rPage.aChild;
	rPage.nCount = sal_uInt16(nValue);

//	DBG_ASSERT(rPage.nCount && rPage.nCount < rPage.GetIndex().GetMaxNodes(), "Falscher Count");
	for (sal_uInt16 i = 0; i < rPage.nCount; i++)
		rPage[i].Read(rStream, rPage.GetIndex());
	return rStream;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPage& rPage)
{
	// Seite existiert noch nicht
	sal_uIntPtr nSize = (rPage.GetPagePos() + 1) * 512;
	if (nSize > rStream.Seek(STREAM_SEEK_TO_END))
	{
		rStream.SetStreamSize(nSize);
		rStream.Seek(rPage.GetPagePos() * 512);

		char aEmptyData[512];
		memset(aEmptyData,0x00,512);
		rStream.Write((sal_uInt8*)aEmptyData,512);
	}
	sal_uIntPtr nCurrentPos = rStream.Seek(rPage.GetPagePos() * 512);
    OSL_UNUSED( nCurrentPos );

	nValue = rPage.nCount;
	rStream << nValue << rPage.aChild;

	sal_uInt16 i = 0;
	for (; i < rPage.nCount; i++)
		rPage[i].Write(rStream, rPage);

	// check if we have to fill the stream with '\0'
	if(i < rPage.rIndex.getHeader().db_maxkeys)
	{
		sal_uIntPtr nTell = rStream.Tell() % 512;
		sal_uInt16 nBufferSize = rStream.GetBufferSize();
		sal_uIntPtr nSize = nBufferSize - nTell;
		if ( nSize <= nBufferSize )
		{
			char* pEmptyData = new char[nSize];
			memset(pEmptyData,0x00,nSize);
			rStream.Write((sal_uInt8*)pEmptyData,nSize);
			rStream.Seek(nTell);
			delete [] pEmptyData;
		}
	}
	return rStream;
}
// -----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
//------------------------------------------------------------------
void ONDXPage::PrintPage()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::PrintPage" );
	DBG_TRACE4("\nSDB: -----------Page: %d  Parent: %d  Count: %d  Child: %d-----",
		nPagePos, HasParent() ? aParent->GetPagePos() : 0 ,nCount, aChild.GetPagePos());

	for (sal_uInt16 i = 0; i < nCount; i++)
	{
		ONDXNode rNode = (*this)[i];
		ONDXKey&  rKey = rNode.GetKey();
		if (!IsLeaf())
			rNode.GetChild(&rIndex, this);

		if (rKey.getValue().isNull())
		{
			DBG_TRACE2("SDB: [%d,NULL,%d]",rKey.GetRecord(), rNode.GetChild().GetPagePos());
		}
		else if (rIndex.getHeader().db_keytype)
		{
			DBG_TRACE3("SDB: [%d,%f,%d]",rKey.GetRecord(), rKey.getValue().getDouble(),rNode.GetChild().GetPagePos());
		}
		else
		{
			DBG_TRACE3("SDB: [%d,%s,%d]",rKey.GetRecord(), (const char* )ByteString(rKey.getValue().getString().getStr(), rIndex.m_pTable->getConnection()->getTextEncoding()).GetBuffer(),rNode.GetChild().GetPagePos());
		}
	}
	DBG_TRACE("SDB: -----------------------------------------------\n");
	if (!IsLeaf())
	{
#if OSL_DEBUG_LEVEL > 1
		GetChild(&rIndex)->PrintPage();
		for (sal_uInt16 i = 0; i < nCount; i++)
		{
			ONDXNode rNode = (*this)[i];
			rNode.GetChild(&rIndex,this)->PrintPage();
		}
#endif
	}
	DBG_TRACE("SDB: ===============================================\n");
}
#endif
// -----------------------------------------------------------------------------
sal_Bool ONDXPage::IsFull() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::IsFull" );
	return Count() == rIndex.getHeader().db_maxkeys;
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
sal_uInt16 ONDXPage::Search(const ONDXKey& rSearch)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::Search" );
	// binare Suche spaeter
	sal_uInt16 i = 0xFFFF;
	while (++i < Count())
		if ((*this)[i].GetKey() == rSearch)
			break;

	return (i < Count()) ? i : NODE_NOTFOUND;
}

//------------------------------------------------------------------
sal_uInt16 ONDXPage::Search(const ONDXPage* pPage)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::Search" );
	sal_uInt16 i = 0xFFFF;
	while (++i < Count())
		if (((*this)[i]).GetChild() == pPage)
			break;

	// wenn nicht gefunden, dann wird davon ausgegangen, dass die Seite selbst
	// auf die Page zeigt
	return (i < Count()) ? i : NODE_NOTFOUND;
}
// -----------------------------------------------------------------------------
// laeuft rekursiv
void ONDXPage::SearchAndReplace(const ONDXKey& rSearch,
								  ONDXKey& rReplace)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::SearchAndReplace" );
	OSL_ENSURE(rSearch != rReplace,"Invalid here:rSearch == rReplace");
	if (rSearch != rReplace)
	{
		sal_uInt16 nPos = NODE_NOTFOUND;
		ONDXPage* pPage = this;

		while (pPage && (nPos = pPage->Search(rSearch)) == NODE_NOTFOUND)
			pPage = pPage->aParent;

		if (pPage)
		{
			(*pPage)[nPos].GetKey() = rReplace;
			pPage->SetModified(sal_True);
		}
	}
}
// -----------------------------------------------------------------------------
ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos)
{
	DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
	return ppNodes[nPos];
}

//------------------------------------------------------------------
const ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos) const
{
	DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
	return ppNodes[nPos];
}
// -----------------------------------------------------------------------------
void ONDXPage::Remove(sal_uInt16 nPos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ONDXPage::Remove" );
	DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");

	for (sal_uInt16 i = nPos; i < (nCount-1); i++)
		(*this)[i] = (*this)[i+1];

	nCount--;
	bModified = sal_True;
}
// -----------------------------------------------------------------------------











