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



#ifndef _TAGTEST_HXX_
#define _TAGTEST_HXX_

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <hash_map> /* std::hashmap*/
#include <rtl/string.h>

class GSILine;

typedef sal_uInt16 TokenId;

#define TOK_INVALIDPOS  sal_uInt16( 0xFFFF )

class ParserMessage;

DECLARE_LIST( Impl_ParserMessageList, ParserMessage* )
class ParserMessageList;


struct equalByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
    }
};
struct lessByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
    }
};

struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
        return rtl_str_hashCode_WithLength( rName.GetBuffer(), rName.Len());
    }
};



typedef std::hash_map<ByteString , String , hashByteString,equalByteString>
                                StringHashMap;

class TokenInfo
{
private:
    void SplitTag( ParserMessageList &rErrorList );

    String aTagName;
    StringHashMap aProperties;
    sal_Bool bClosed;    // tag is closed  <sdnf/>
    sal_Bool bCloseTag;  // tag is close Tag  </sdnf>


    sal_Bool bIsBroken;
    sal_Bool bHasBeenFixed;
    sal_Bool bDone;

public:

	String aTokenString;
	TokenId nId;
    sal_uInt16 nPos;            // Position in String

    TokenInfo():bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),nId( 0 ){;}
explicit    TokenInfo( TokenId pnId, sal_uInt16 nP ):bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),nId( pnId ),nPos(nP){;}
explicit    TokenInfo( TokenId pnId, sal_uInt16 nP, String paStr ):bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),aTokenString( paStr ),nId( pnId ),nPos(nP) {;}
explicit    TokenInfo( TokenId pnId, sal_uInt16 nP, String paStr, ParserMessageList &rErrorList );

	String GetTagName() const;

    String MakeTag() const;

    /**
        Is the property to be ignored or does it have the default value anyways
    **/
    sal_Bool IsPropertyRelevant( const ByteString &aName, const String &aValue ) const;
    sal_Bool IsPropertyValueValid( const ByteString &aName, const String &aValue ) const;
    /**
        Does the property contain the same value for all languages
        e.g.: the href in a link tag
    **/
    sal_Bool IsPropertyInvariant( const ByteString &aName, const String &aValue ) const;
    /**
        a subset of IsPropertyInvariant but containing only those that are fixable
        we don't want to fix e.g.: ahelp :: visibility
    **/
    sal_Bool IsPropertyFixable( const ByteString &aName ) const;
    sal_Bool MatchesTranslation( TokenInfo& rInfo, sal_Bool bGenErrors, ParserMessageList &rErrorList, sal_Bool bFixTags = sal_False ) const;

    sal_Bool IsDone() const { return bDone; }
    void SetDone( sal_Bool bNew = sal_True ) { bDone = bNew; }

    sal_Bool HasBeenFixed() const { return bHasBeenFixed; }
    void SetHasBeenFixed( sal_Bool bNew = sal_True ) { bHasBeenFixed = bNew; }
};


class ParserMessageList : public Impl_ParserMessageList
{
public:
    void AddError( sal_uInt16 nErrorNr, ByteString aErrorText, const TokenInfo &rTag );
    void AddWarning( sal_uInt16 nErrorNr, ByteString aErrorText, const TokenInfo &rTag );

    sal_Bool HasErrors();
};


#define TAG_GROUPMASK				0xF000
#define TAG_GROUPSHIFT				12

#define TAG_GROUP( nTag )			(( nTag & TAG_GROUPMASK ) >> TAG_GROUPSHIFT )
#define TAG_NOGROUP( nTag )			( nTag & ~TAG_GROUPMASK )	// ~ = Bitweises NOT

#define TAG_NOMORETAGS				0x0

#define TAG_GROUP_FORMAT			0x1
#define TAG_ON						0x100
#define TAG_BOLDON					( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x001 )
#define TAG_BOLDOFF					( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x001 )
#define TAG_ITALICON				( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x002 )
#define TAG_ITALICOFF				( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x002 )
#define TAG_UNDERLINEON				( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x004 )
#define TAG_UNDERLINEOFF			( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x004 )

#define TAG_GROUP_NOTALLOWED		0x2
#define TAG_HELPID					( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x001 )
#define TAG_MODIFY					( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x002 )
#define TAG_REFNR					( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x004 )

#define TAG_GROUP_STRUCTURE			0x3
#define TAG_NAME					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x001 )
#define TAG_HREF					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x002 )
#define TAG_AVIS					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x004 )
#define TAG_AHID					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x008 )

#define TAG_TITEL					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x020 )
#define TAG_KEY						( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x040 )
#define TAG_INDEX					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x080 )

#define TAG_REFSTART				( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x100 )

#define TAG_GRAPHIC					( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x200 )
#define TAG_NEXTVERSION				( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x400 )

#define TAG_GROUP_SYSSWITCH			0x4
#define TAG_WIN						( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x001 )
#define TAG_UNIX					( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x002 )
#define TAG_MAC						( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x004 )
#define TAG_OS2						( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x008 )

#define TAG_GROUP_PROGSWITCH		0x5
#define TAG_WRITER					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x001 )
#define TAG_CALC					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x002 )
#define TAG_DRAW					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x004 )
#define TAG_IMPRESS					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x008 )
#define TAG_SCHEDULE				( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x010 )
#define TAG_IMAGE					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x020 )
#define TAG_MATH					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x040 )
#define TAG_CHART					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x080 )
#define TAG_OFFICE					( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x100 )


#define TAG_GROUP_META				0x6
#define TAG_OFFICEFULLNAME			( TAG_GROUP_META << TAG_GROUPSHIFT | 0x001 )
#define TAG_OFFICENAME				( TAG_GROUP_META << TAG_GROUPSHIFT | 0x002 )
#define TAG_OFFICEPATH				( TAG_GROUP_META << TAG_GROUPSHIFT | 0x004 )
#define TAG_OFFICEVERSION			( TAG_GROUP_META << TAG_GROUPSHIFT | 0x008 )
#define TAG_PORTALNAME				( TAG_GROUP_META << TAG_GROUPSHIFT | 0x010 )
#define TAG_PORTALFULLNAME			( TAG_GROUP_META << TAG_GROUPSHIFT | 0x020 )
#define TAG_PORTALPATH				( TAG_GROUP_META << TAG_GROUPSHIFT | 0x040 )
#define TAG_PORTALVERSION			( TAG_GROUP_META << TAG_GROUPSHIFT | 0x080 )
#define TAG_PORTALSHORTNAME			( TAG_GROUP_META << TAG_GROUPSHIFT | 0x100 )


#define TAG_GROUP_SINGLE            0x7
#define TAG_REFINSERT               ( TAG_GROUP_SINGLE << TAG_GROUPSHIFT | 0x001 )


#define TAG_GROUP_MULTI				0x8
#define TAG_END						( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x010 )
#define TAG_ELSE					( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x020 )
#define TAG_AEND					( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x040 )
#define TAG_VERSIONEND				( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x080 )
#define TAG_ENDGRAPHIC				( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x100 )

#define TAG_GROUP_MISC				0x9
#define TAG_COMMONSTART				( TAG_GROUP_MISC << TAG_GROUPSHIFT | 0x001 )
#define TAG_COMMONEND				( TAG_GROUP_MISC << TAG_GROUPSHIFT | 0x002 )

#define TAG_UNKNOWN_TAG				( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x800 )

DECLARE_LIST( TokenListImpl, TokenInfo* )

class TokenList : private TokenListImpl
{
private:

    TokenList&   operator =( const TokenList& rList );
//                { TokenListImpl::operator =( rList ); return *this; }


public:
	using TokenListImpl::Count;


    TokenList() : TokenListImpl(){};
    ~TokenList(){ Clear(); };

	void		Clear()
		{
			for ( sal_uLong i = 0 ; i < Count() ; i++ )
				delete TokenListImpl::GetObject( i );
			TokenListImpl::Clear();
		}
	void		Insert( TokenInfo p, sal_uLong nIndex = LIST_APPEND )
		{ TokenListImpl::Insert( new TokenInfo(p), nIndex ); }
/*    TokenInfo		Remove( sal_uLong nIndex )
		{
			TokenInfo aT = GetObject( nIndex );
			delete TokenListImpl::GetObject( nIndex );
			TokenListImpl::Remove( nIndex );
			return aT;
		}*/
//    TokenInfo		Remove( TokenInfo p ){ return Remove( GetPos( p ) ); }
//    TokenInfo		GetCurObject() const { return *TokenListImpl::GetCurObject(); }
    TokenInfo&		GetObject( sal_uLong nIndex ) const
		{
//			if ( TokenListImpl::GetObject(nIndex) )
				return *TokenListImpl::GetObject(nIndex);
//			else
//				return TokenInfo();
		}
/*    sal_uLong		GetPos( const TokenInfo p ) const
		{
			for ( sal_uLong i = 0 ; i < Count() ; i++ )
				if ( p == GetObject( i ) )
					return i;
			return LIST_ENTRY_NOTFOUND;
		}*/

    TokenList( const TokenList& rList );
/*		{
			for ( sal_uLong i = 0 ; i < rList.Count() ; i++ )
			{
				Insert( rList.GetObject( i ), LIST_APPEND );
			}
		}*/
};

class ParserMessage
{
	sal_uInt16 nErrorNr;
	ByteString aErrorText;
	sal_uInt16 nTagBegin,nTagLength;

protected:
    ParserMessage( sal_uInt16 PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );
public:

	sal_uInt16 GetErrorNr() { return nErrorNr; }
	ByteString GetErrorText() { return aErrorText; }

	sal_uInt16 GetTagBegin() { return nTagBegin; }
	sal_uInt16 GetTagLength() { return nTagLength; }

    virtual ~ParserMessage() {}
    virtual sal_Bool IsError() =0;
    virtual ByteString Prefix() =0;
};

class ParserError : public ParserMessage
{
public:
    ParserError( sal_uInt16 PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );

    virtual sal_Bool IsError() {return sal_True;};
    virtual ByteString Prefix() {return "Error:"; };
};

class ParserWarning : public ParserMessage
{
public:
    ParserWarning( sal_uInt16 PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );

    virtual sal_Bool IsError() {return sal_False;};
    virtual ByteString Prefix() {return "Warning:"; };
};

class SimpleParser
{
private:
	sal_uInt16 nPos;
	String aSource;
	String aLastToken;
	TokenList aTokenList;

    TokenInfo aNextTag;     // to store closetag in case of combined tags like <br/>

	String GetNextTokenString( ParserMessageList &rErrorList, sal_uInt16 &rTokeStartPos );

public:
	SimpleParser();
	void Parse( String PaSource );
	TokenInfo GetNextToken( ParserMessageList &rErrorList );
	static String GetLexem( TokenInfo const &aToken );
	TokenList& GetTokenList(){ return aTokenList; }
};

class TokenParser
{
    sal_Bool match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken );
	sal_Bool match( const TokenInfo &aCurrentToken, const TokenInfo &aExpectedToken );
	void ParseError( sal_uInt16 nErrNr, ByteString aErrMsg, const TokenInfo &rTag );
	void Paragraph();
	void PfCase();
	void PfCaseBegin();
	void AppCase();
	void AppCaseBegin();
	void CaseEnd();
	void SimpleTag();
	void TagPair();
	void TagRef();

	SimpleParser aParser;
	TokenInfo aTag;

	TokenId nPfCaseOptions;
	TokenId nAppCaseOptions;
	sal_Bool bPfCaseActive ,bAppCaseActive;

	TokenId nActiveRefTypes;

	ParserMessageList *pErrorList;

public:
	TokenParser();
	void Parse( const String &aCode, ParserMessageList* pList );
//	ParserMessageList& GetErrors(){ return aErrorList; }
//	sal_Bool HasErrors(){ return ( aErrorList.Count() > 0 ); }
	TokenList& GetTokenList(){ return aParser.GetTokenList(); }
};

class LingTest
{
private:
	TokenParser aReferenceParser;
	TokenParser aTesteeParser;
	ParserMessageList aCompareWarningList;
	void CheckTags( TokenList &aReference, TokenList &aTestee, sal_Bool bFixTags );
    sal_Bool IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens );
    String aFixedTestee;
public:
	void CheckReference( GSILine *aReference );
	void CheckTestee( GSILine *aTestee, sal_Bool bHasSourceLine, sal_Bool bFixTags );

//	ParserMessageList& GetReferenceErrors(){ return aReferenceParser.GetErrors(); }
//	sal_Bool HasReferenceErrors(){ return aReferenceParser.HasErrors(); }

//	ParserMessageList& GetTesteeErrors(){ return aTesteeParser.GetErrors(); }
//	sal_Bool HasTesteeErrors(){ return aTesteeParser.HasErrors(); }

	ParserMessageList& GetCompareWarnings(){ return aCompareWarningList; }
	sal_Bool HasCompareWarnings(){ return ( aCompareWarningList.Count() > 0 ); }

    String GetFixedTestee(){ return aFixedTestee; }
};

#endif

