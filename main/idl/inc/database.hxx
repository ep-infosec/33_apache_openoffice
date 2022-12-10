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



#ifndef _DATABASE_HXX
#define _DATABASE_HXX

#include <module.hxx>
#include <hash.hxx>
#include <lex.hxx>
#include <tools/pstm.hxx>

/*************************************************************************
*************************************************************************/

class SvCommand;

#ifdef IDL_COMPILER
/******************** class SvIdlError ***********************************/
class SvIdlError
{
    ByteString  aText;
public:
    sal_uInt32  nLine, nColumn;

            SvIdlError() : nLine(0), nColumn(0) {}
            SvIdlError( sal_uInt32 nL, sal_uInt32 nC )
                : nLine(nL), nColumn(nC) {}

    const ByteString &  GetText() const { return aText; }
    void            SetText( const ByteString & rT ) { aText = rT; }
    sal_Bool            IsError() const { return nLine != 0; }
    void            Clear() { nLine = nColumn = 0; }
	SvIdlError &	operator = ( const SvIdlError & rRef )
	{ aText   = rRef.aText;
	  nLine   = rRef.nLine;
	  nColumn = rRef.nColumn;
	  return *this;
	}
};
#endif

/******************** class SvIdlDataBase ********************************/
class SvIdlDataBase
{
	sal_Bool  						bExport;
    String                      aExportFile;
	sal_uInt32					nUniqueId;
    sal_uInt32                  nVerbosity;
    String                      aDataBaseFile;
    SvFileStream *              pStm;
    sal_Bool                        bIsModified;
    SvPersistStream             aPersStream;
    StringList                  aIdFileList;
    SvStringHashTable *         pIdTable;

    SvMetaTypeMemberList        aTypeList;
    SvMetaClassMemberList       aClassList;
    SvMetaModuleMemberList      aModuleList;
	SvMetaAttributeMemberList	aAttrList;
    SvMetaTypeMemberList        aTmpTypeList; // nicht Persistent

protected:
#ifdef IDL_COMPILER
	ByteString						aModulePrefix;
    SvMetaObjectMemberStack     aContextStack;
    String                      aPath;
    SvIdlError                  aError;
	void						WriteReset()
								{
									aUsedTypes.Clear();
									aIFaceName.Erase();
								}
#endif
public:
                explicit SvIdlDataBase( const SvCommand& rCmd );
                ~SvIdlDataBase();
    static sal_Bool IsBinaryFormat( SvStream & rInStm );

    void        Load( SvStream & rInStm );
    void        Save( SvStream & rInStm, sal_uInt32 nContextFlags );

    SvMetaAttributeMemberList&  GetAttrList() { return aAttrList; }
    SvStringHashTable *       GetIdTable() { return pIdTable; }
    SvMetaTypeMemberList &    GetTypeList();
    SvMetaClassMemberList &   GetClassList()  { return aClassList; }
    SvMetaModuleMemberList &  GetModuleList() { return aModuleList; }
    SvMetaModule *            GetModule( const ByteString & rName );

	// Liste der benutzten Typen beim Schreiben
    SvMetaTypeMemberList	aUsedTypes;
	ByteString					aIFaceName;
	SvNumberIdentifier		aStructSlotId;

#ifdef IDL_COMPILER
    void                    StartNewFile( const String& rName );
    void                    SetExportFile( const String& rName )
							{ aExportFile = rName; }
	void					AppendAttr( SvMetaAttribute *pSlot );
	const ByteString&			GetActModulePrefix() const { return aModulePrefix; }
	const SvIdlError &		GetError() const { return aError; }
	void					SetError( const SvIdlError & r )
							{ aError = r; }

    const String &            GetPath() const { return aPath; }
    SvMetaObjectMemberStack & GetStack()      { return aContextStack; }

	void					Write( const ByteString & rText );
	void 					WriteError( const ByteString & rErrWrn,
									const ByteString & rFileName,
									const ByteString & rErrorText,
									sal_uLong nRow = 0, sal_uLong nColumn = 0 ) const;
    void                    WriteError( SvTokenStream & rInStm );
    void                    SetError( const ByteString & rError, SvToken * pTok );
    void                    Push( SvMetaObject * pObj );
    sal_Bool                    Pop( sal_Bool bOk, SvTokenStream & rInStm, sal_uInt32 nTokPos )
                            {
                                GetStack().Pop();
                                if( bOk )
                                    aError.Clear();
                                else
                                    rInStm.Seek( nTokPos );
                                return bOk;
                            }
	sal_uInt32				GetUniqueId() { return ++nUniqueId; }
    sal_Bool                    FindId( const ByteString & rIdName, sal_uLong * pVal );
    sal_Bool                    InsertId( const ByteString & rIdName, sal_uLong nVal );
    sal_Bool                    ReadIdFile( const String & rFileName );

	SvMetaType *			FindType( const ByteString & rName );
	static SvMetaType *		FindType( const SvMetaType *, SvMetaTypeMemberList & );

    SvMetaType *            ReadKnownType( SvTokenStream & rInStm );
    SvMetaAttribute *       ReadKnownAttr( SvTokenStream & rInStm,
											SvMetaType * pType = NULL );
    SvMetaAttribute *       SearchKnownAttr( const SvNumberIdentifier& );
    SvMetaClass *           ReadKnownClass( SvTokenStream & rInStm );
#endif
};

#ifdef IDL_COMPILER
class SvIdlWorkingBase : public SvIdlDataBase
{
public:
                explicit SvIdlWorkingBase( const SvCommand& rCmd );

    sal_Bool        ReadSvIdl( SvTokenStream &, sal_Bool bImported, const String & rPath );
    sal_Bool        WriteSvIdl( SvStream & );

    sal_Bool        WriteSfx( SvStream & );
	sal_Bool		WriteHelpIds( SvStream & );
    sal_Bool        WriteSfxItem( SvStream & );
	sal_Bool 		WriteCSV( SvStream& );
	sal_Bool		WriteDocumentation( SvStream& );
};
#endif

#endif // _DATABASE_HXX

