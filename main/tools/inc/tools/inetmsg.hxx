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


#ifndef _TOOLS_INETMSG_HXX
#define _TOOLS_INETMSG_HXX

#include "tools/toolsdllapi.h"
#include <sal/types.h>

#ifndef _RTL_TEXTENC_H_
#include <rtl/textenc.h>
#endif

#ifndef _TOOLS_INETMIME_HXX
#include <tools/inetmime.hxx>
#endif
#include <tools/list.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>

class DateTime;

/*=======================================================================
 *
 * INetMessageHeader Interface.
 *
 *=====================================================================*/
class INetMessageHeader
{
	ByteString m_aName;
	ByteString m_aValue;

public:
	INetMessageHeader (void)
	{}

	INetMessageHeader (
		const ByteString& rName, const ByteString& rValue)
		: m_aName (rName), m_aValue (rValue)
	{}

	INetMessageHeader (
		const INetMessageHeader& rHdr)
		: m_aName (rHdr.m_aName), m_aValue (rHdr.m_aValue)
	{}

	~INetMessageHeader (void)
	{}

	INetMessageHeader& operator= (const INetMessageHeader& rHdr)
	{
		m_aName  = rHdr.m_aName;
		m_aValue = rHdr.m_aValue;
		return *this;
	}

	const ByteString& GetName  (void) const { return m_aName; }
	const ByteString& GetValue (void) const { return m_aValue; }

	friend SvStream& operator<< (
		SvStream& rStrm, const INetMessageHeader& rHdr)
	{
#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
		rStrm << rHdr.m_aName;
		rStrm << rHdr.m_aValue;
#else
		rStrm.WriteByteString (rHdr.m_aName);
		rStrm.WriteByteString (rHdr.m_aValue);
#endif
		return rStrm;
	}

	friend SvStream& operator>> (
		SvStream& rStrm, INetMessageHeader& rHdr)
	{
#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
		rStrm >> rHdr.m_aName;
		rStrm >> rHdr.m_aValue;
#else
		rStrm.ReadByteString (rHdr.m_aName);
		rStrm.ReadByteString (rHdr.m_aValue);
#endif
		return rStrm;
	}
};

/*=======================================================================
 *
 * INetMessage Interface.
 *
 *=====================================================================*/
class INetMessage
{
	List           m_aHeaderList;

	sal_uIntPtr          m_nDocSize;
	UniString      m_aDocName;
	SvLockBytesRef m_xDocLB;

	void ListCleanup_Impl (void);
	void ListCopy (const INetMessage& rMsg);

protected:
	UniString GetHeaderName_Impl (
		sal_uIntPtr nIndex, rtl_TextEncoding eEncoding) const
	{
		INetMessageHeader *p =
			(INetMessageHeader*)(m_aHeaderList.GetObject(nIndex));
		if (p)
			return UniString(p->GetName(), eEncoding);
		else
			return UniString();
	}

	UniString GetHeaderValue_Impl (
		sal_uIntPtr nIndex, INetMIME::HeaderFieldType eType) const
	{
		INetMessageHeader *p =
			(INetMessageHeader*)(m_aHeaderList.GetObject(nIndex));
		if (p)
			return INetMIME::decodeHeaderFieldBody (eType, p->GetValue());
		else
			return UniString();
	}

	void SetHeaderField_Impl (
		const INetMessageHeader &rHeader, sal_uIntPtr &rnIndex)
	{
		INetMessageHeader *p = new INetMessageHeader (rHeader);
		if (m_aHeaderList.Count() <= rnIndex)
		{
			m_aHeaderList.Insert (p, LIST_APPEND);
			rnIndex = m_aHeaderList.Count() - 1;
		}
		else
		{
			p = (INetMessageHeader*)(m_aHeaderList.Replace(p, rnIndex));
			delete p;
		}
	}

	void SetHeaderField_Impl (
		INetMIME::HeaderFieldType  eType,
		const ByteString          &rName,
		const UniString           &rValue,
		sal_uIntPtr                     &rnIndex);

	virtual SvStream& operator<< (SvStream& rStrm) const;
	virtual SvStream& operator>> (SvStream& rStrm);

public:
	INetMessage (void) : m_nDocSize(0) {}
	virtual ~INetMessage (void);

	INetMessage (const INetMessage& rMsg)
		: m_nDocSize (rMsg.m_nDocSize),
		  m_aDocName (rMsg.m_aDocName),
		  m_xDocLB   (rMsg.m_xDocLB)
	{
		ListCopy (rMsg);
	}

	INetMessage& operator= (const INetMessage& rMsg)
	{
		m_nDocSize = rMsg.m_nDocSize;
		m_aDocName = rMsg.m_aDocName;
		m_xDocLB   = rMsg.m_xDocLB;
		ListCopy (rMsg);
		return *this;
	}

	sal_uIntPtr GetHeaderCount (void) const { return m_aHeaderList.Count(); }

	UniString GetHeaderName (sal_uIntPtr nIndex) const
	{
		return GetHeaderName_Impl (nIndex, RTL_TEXTENCODING_ASCII_US);
	}

	UniString GetHeaderValue (sal_uIntPtr nIndex) const
	{
		return GetHeaderValue_Impl (nIndex, INetMIME::HEADER_FIELD_TEXT);
	}

	INetMessageHeader GetHeaderField (sal_uIntPtr nIndex) const
	{
		INetMessageHeader *p =
			(INetMessageHeader*)(m_aHeaderList.GetObject(nIndex));
		if (p)
			return INetMessageHeader(*p);
		else
			return INetMessageHeader();
	}

	sal_uIntPtr SetHeaderField (
		const UniString& rName,
		const UniString& rValue,
		sal_uIntPtr            nIndex = LIST_APPEND);

	virtual sal_uIntPtr SetHeaderField (
		const INetMessageHeader &rField, sal_uIntPtr nIndex = LIST_APPEND);

	sal_uIntPtr GetDocumentSize (void) const { return m_nDocSize; }
	void  SetDocumentSize (sal_uIntPtr nSize) { m_nDocSize = nSize; }

	const UniString& GetDocumentName (void) const { return m_aDocName; }
	void  SetDocumentName (const UniString& rName) { m_aDocName = rName; }

	SvLockBytes* GetDocumentLB (void) const { return m_xDocLB; }
	void         SetDocumentLB (SvLockBytes *pDocLB) { m_xDocLB = pDocLB; }

	friend SvStream& operator<< (
		SvStream& rStrm, const INetMessage& rMsg)
	{
		return rMsg.operator<< (rStrm);
	}

	friend SvStream& operator>> (
		SvStream& rStrm, INetMessage& rMsg)
	{
		return rMsg.operator>> (rStrm);
	}
};

/*=======================================================================
 *
 * INetMessageHeaderIterator Interface.
 *
 *=====================================================================*/
class INetMessageHeaderIterator
{
	sal_uIntPtr     nValueCount;
	List      aValueList;
	UniString aEmptyString;

public:
	INetMessageHeaderIterator (
		const INetMessage& rMsg, const UniString& rHdrName);
	virtual ~INetMessageHeaderIterator (void);

	sal_uIntPtr GetValueCount (void) const { return nValueCount; }
	const UniString& GetValue (sal_uIntPtr nIndex) const
	{
		if (nIndex < nValueCount)
		{
			return *((UniString*)(aValueList.GetObject(nIndex)));
		}
		else
		{
			return aEmptyString;
		}
	}
};

/*=======================================================================
 *
 * INetRFC822Message Interface.
 *
 *=====================================================================*/
#define INETMSG_RFC822_BCC                 0
#define INETMSG_RFC822_CC                  1
#define INETMSG_RFC822_COMMENTS            2
#define INETMSG_RFC822_DATE                3
#define INETMSG_RFC822_FROM                4
#define INETMSG_RFC822_IN_REPLY_TO         5
#define INETMSG_RFC822_KEYWORDS            6
#define INETMSG_RFC822_MESSAGE_ID          7
#define INETMSG_RFC822_REFERENCES          8
#define INETMSG_RFC822_REPLY_TO            9
#define INETMSG_RFC822_RETURN_PATH        10
#define INETMSG_RFC822_SENDER             11
#define INETMSG_RFC822_SUBJECT            12
#define INETMSG_RFC822_TO                 13

#define INETMSG_RFC822_X_MAILER           14
#define INETMSG_RFC822_RETURN_RECEIPT_TO  15

#define INETMSG_RFC822_NUMHDR             16

class TOOLS_DLLPUBLIC INetRFC822Message : public INetMessage
{
	sal_uIntPtr m_nIndex[INETMSG_RFC822_NUMHDR];

protected:
	virtual SvStream& operator<< (SvStream& rStrm) const;
	virtual SvStream& operator>> (SvStream& rStrm);

public:
	INetRFC822Message (void);
	INetRFC822Message (const INetRFC822Message& rMsg);
	virtual ~INetRFC822Message (void);

	INetRFC822Message& operator= (const INetRFC822Message& rMsg);

	static sal_Bool GenerateDateField (
		const DateTime& rDateTime, UniString& rDateField);
	static sal_Bool ParseDateField (
		const UniString& rDateField, DateTime& rDateTime);

    using INetMessage::SetHeaderField;
	virtual sal_uIntPtr SetHeaderField (
		const INetMessageHeader &rHeader, sal_uIntPtr nIndex = LIST_APPEND);

	/** Header fields.
	 */
	void      SetBCC (const UniString& rBCC);
	UniString GetBCC (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_BCC],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetCC (const UniString& rCC);
	UniString GetCC (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_CC],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetComments (const UniString& rComments);
	UniString GetComments (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_COMMENTS],
			INetMIME::HEADER_FIELD_TEXT);
	}

	void      SetDate (const UniString& rDate);
	UniString GetDate (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_DATE],
			INetMIME::HEADER_FIELD_STRUCTURED);
	}

	void      SetFrom (const UniString& rFrom);
	UniString GetFrom (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_FROM],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetInReplyTo (const UniString& rInReplyTo);
	UniString GetInReplyTo (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_IN_REPLY_TO],
			INetMIME::HEADER_FIELD_ADDRESS); // ??? MESSAGE_ID ???
	}

	void      SetKeywords (const UniString& rKeywords);
	UniString GetKeywords (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_KEYWORDS],
			INetMIME::HEADER_FIELD_PHRASE);
	}

	void      SetMessageID (const UniString& rMessageID);
	UniString GetMessageID (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_MESSAGE_ID],
			INetMIME::HEADER_FIELD_MESSAGE_ID);
	}

	void      SetReferences (const UniString& rReferences);
	UniString GetReferences (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_REFERENCES],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetReplyTo (const UniString& rReplyTo);
	UniString GetReplyTo (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_REPLY_TO],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetReturnPath (const UniString& rReturnPath);
	UniString GetReturnPath (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_RETURN_PATH],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetReturnReceiptTo (const UniString& rReturnReceiptTo);
	UniString GetReturnReceiptTo (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_RETURN_RECEIPT_TO],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetSender (const UniString& rSender);
	UniString GetSender (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_SENDER],
			INetMIME::HEADER_FIELD_ADDRESS);
	}

	void      SetSubject (const UniString& rSubject);
	UniString GetSubject (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_SUBJECT],
			INetMIME::HEADER_FIELD_TEXT);
	}

	void      SetTo (const UniString& rTo);
	UniString GetTo (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_TO],
			INetMIME::HEADER_FIELD_TEXT);
	}

	void      SetXMailer (const UniString& rXMailer);
	UniString GetXMailer (void) const
	{
		return GetHeaderValue_Impl (
			m_nIndex[INETMSG_RFC822_X_MAILER],
			INetMIME::HEADER_FIELD_TEXT);
	}

	/** Stream operators.
	 */
	friend SvStream& operator<< (
		SvStream& rStrm, const INetRFC822Message& rMsg)
	{
		return rMsg.operator<< (rStrm);
	}

	friend SvStream& operator>> (
		SvStream& rStrm, INetRFC822Message& rMsg)
	{
		return rMsg.operator>> (rStrm);
	}
};

/*=======================================================================
 *
 * INetMIMEMessage Interface.
 *
 *=====================================================================*/
#define INETMSG_MIME_VERSION                    0
#define INETMSG_MIME_CONTENT_DESCRIPTION        1
#define INETMSG_MIME_CONTENT_DISPOSITION        2
#define INETMSG_MIME_CONTENT_ID                 3
#define INETMSG_MIME_CONTENT_TYPE               4
#define INETMSG_MIME_CONTENT_TRANSFER_ENCODING  5

#define INETMSG_MIME_NUMHDR                     6

enum INetMessageContainerType
{
	INETMSG_MESSAGE_RFC822,
	INETMSG_MULTIPART_MIXED,
	INETMSG_MULTIPART_ALTERNATIVE,
	INETMSG_MULTIPART_DIGEST,
	INETMSG_MULTIPART_PARALLEL,
	INETMSG_MULTIPART_RELATED,
	INETMSG_MULTIPART_FORM_DATA
};

class TOOLS_DLLPUBLIC INetMIMEMessage : public INetRFC822Message
{
	sal_uIntPtr           m_nIndex[INETMSG_MIME_NUMHDR];

	INetMIMEMessage *pParent;
	sal_uIntPtr           nNumChildren;
	List            aChildren;
	ByteString      m_aBoundary;
	sal_Bool            bHeaderParsed;

	friend class INetMIMEMessageStream;

	void SetChildCount (sal_uIntPtr nCount) { nNumChildren = nCount; }
	const ByteString& GetMultipartBoundary (void) const { return m_aBoundary; }
	void SetMultipartBoundary (const ByteString& rBnd) { m_aBoundary = rBnd; }

	void CleanupImp (void);
	void CopyImp    (const INetMIMEMessage& rMsg);
	void SetHeaderParsed() { bHeaderParsed = sal_True; }

protected:
	virtual SvStream& operator<< (SvStream& rStrm) const;
	virtual SvStream& operator>> (SvStream& rStrm);

public:
	INetMIMEMessage (void);
	INetMIMEMessage (const INetMIMEMessage& rMsg);
	virtual ~INetMIMEMessage (void);

	INetMIMEMessage& operator= (const INetMIMEMessage& rMsg);

	sal_Bool HeaderParsed() const { return bHeaderParsed; }

	virtual INetMIMEMessage* CreateMessage (
		const INetMIMEMessage& rMsg) const;

    using INetRFC822Message::SetHeaderField;
	virtual sal_uIntPtr SetHeaderField (
		const INetMessageHeader &rHeader, sal_uIntPtr nIndex = LIST_APPEND);

	/** Header fields.
	 */
	void      SetMIMEVersion (const UniString& rVersion);
	UniString GetMIMEVersion (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_VERSION]);
	}

	void      SetContentDescription (const UniString& rDescription);
	UniString GetContentDescription (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_DESCRIPTION]);
	}

	void      SetContentDisposition (const UniString& rDisposition);
	UniString GetContentDisposition (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_DISPOSITION]);
	}

	void      SetContentID (const UniString& rID);
	UniString GetContentID (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_ID]);
	}

	void      SetContentType (const UniString& rType);
	UniString GetContentType (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_TYPE]);
	}

	void      SetContentTransferEncoding (const UniString& rEncoding);
	UniString GetContentTransferEncoding (void) const
	{
		return GetHeaderValue (m_nIndex[INETMSG_MIME_CONTENT_TRANSFER_ENCODING]);
	}

	virtual void GetDefaultContentType (UniString& rContentType);

	/** Message container methods.
	 */
	sal_Bool IsContainer (void) const
	{
		return (IsMessage() || IsMultipart());
	}
	sal_Bool IsMessage (void) const
	{
		UniString aType (GetContentType());
		return (aType.CompareIgnoreCaseToAscii("message/", 8) == 0);
	}
	sal_Bool IsMultipart (void) const
	{
		UniString aType (GetContentType());
		return (aType.CompareIgnoreCaseToAscii("multipart/", 10) == 0);
	}

	sal_uIntPtr GetChildCount (void) const { return nNumChildren; }
	INetMIMEMessage* GetChild (sal_uIntPtr nIndex) const
	{
		return ((INetMIMEMessage *)(aChildren.GetObject (nIndex)));
	}
	INetMIMEMessage* GetParent (void) const { return pParent; }

	sal_Bool EnableAttachChild (
		INetMessageContainerType eType = INETMSG_MULTIPART_MIXED);
	sal_Bool AttachChild (
		INetMIMEMessage& rChildMsg, sal_Bool bOwner = sal_True);
	sal_Bool DetachChild (
		sal_uIntPtr nIndex, INetMIMEMessage& rChildMsg) const;

	/** Stream operators.
	 */
	friend SvStream& operator<< (
		SvStream& rStrm, const INetMIMEMessage& rMsg)
	{
		return rMsg.operator<< (rStrm);
	}

	friend SvStream& operator>> (
		SvStream& rStrm, INetMIMEMessage& rMsg)
	{
		return rMsg.operator>> (rStrm);
	}
};

#endif /* !_TOOLS_INETMSG_HXX */

