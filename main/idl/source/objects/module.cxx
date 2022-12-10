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
#include "precompiled_idl.hxx"

#include <ctype.h>
#include <stdio.h>

#include <attrib.hxx>
#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <tools/fsys.hxx>
#include <tools/debug.hxx>

/****************** SvMetaModule ******************************************/
SV_IMPL_META_FACTORY1( SvMetaModule, SvMetaExtern );

/*************************************************************************
|*
|*    SvMetaModule::SvMetaModule()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
SvMetaModule::SvMetaModule()
#ifdef IDL_COMPILER
    : bImported( sal_False )
    , bIsModified( sal_False )
#endif
{
}

#ifdef IDL_COMPILER
SvMetaModule::SvMetaModule( const String & rIdlFileName, sal_Bool bImp )
    : aIdlFileName( rIdlFileName )
    , bImported( bImp ), bIsModified( sal_False )
{
}
#endif

/*************************************************************************
|*
|*    SvMetaModule::Load()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
#define MODULE_VER      0x0001
void SvMetaModule::Load( SvPersistStream & rStm )
{
    bImported = sal_True; // immer importiert
    SvMetaExtern::Load( rStm );

    sal_uInt16 nVer;

    rStm >> nVer; // Version
    DBG_ASSERT( (nVer & ~IDL_WRITE_MASK) == MODULE_VER, "false version" );

    rStm >> aClassList;
    rStm >> aTypeList;
    rStm >> aAttrList;
    // Browser
    rStm.ReadByteString( aIdlFileName );
    rStm.ReadByteString( aHelpFileName );
    rStm.ReadByteString( aSlotIdFile );
    rStm.ReadByteString( aModulePrefix );

    // Compiler Daten lesen
    sal_uInt16 nCmpLen;
    rStm >> nCmpLen;
#ifdef IDL_COMPILER
    DBG_ASSERT( (nVer & IDL_WRITE_MASK) == IDL_WRITE_COMPILER,
                "no idl compiler format" );
    rStm >> aBeginName;
    rStm >> aEndName;
    rStm >> aNextName;
#else
    rStm->SeekRel( nCmpLen );
#endif
}

/*************************************************************************
|*
|*    SvMetaModule::Save()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
void SvMetaModule::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    rStm << (sal_uInt16)(MODULE_VER | IDL_WRITE_COMPILER); // Version

    rStm << aClassList;
    rStm << aTypeList;
    rStm << aAttrList;
    // Browser
    rStm.WriteByteString( aIdlFileName );
    rStm.WriteByteString( aHelpFileName );
    rStm.WriteByteString( aSlotIdFile );
    rStm.WriteByteString( aModulePrefix );

    // Compiler Daten schreiben
    sal_uInt16 nCmpLen = 0;
    sal_uLong nLenPos = rStm.Tell();
    rStm << nCmpLen;
#ifdef IDL_COMPILER
    rStm << aBeginName;
    rStm << aEndName;
    rStm << aNextName;
    // Laenge der Compiler Daten schreiben
    sal_uLong nPos = rStm.Tell();
    rStm.Seek( nLenPos );
    rStm << (sal_uInt16)( nPos - nLenPos - sizeof( sal_uInt16 ) );
    rStm.Seek( nPos );
#endif
}

/*************************************************************************
|*
|*    SvMetaModule::SetName()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
sal_Bool SvMetaModule::SetName( const ByteString & rName, SvIdlDataBase * pBase )
{
    if( pBase )
    {
        if( pBase->GetModule( rName ) )
            return sal_False;
    }
    return SvMetaExtern::SetName( rName );
}

#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaModule::GetNextName()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvMetaModule::FillNextName( SvGlobalName * pName )
{
    *pName = aNextName;

    if( aNextName < aEndName )
    {
        ++aNextName;
        bIsModified = sal_True;
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
|*    SvMetaModule::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );

    aHelpFileName.ReadSvIdl( SvHash_HelpFile(), rInStm );
    if( aSlotIdFile.ReadSvIdl( SvHash_SlotIdFile(), rInStm ) )
    {
	    sal_uInt32 nTokPos = rInStm.Tell();
        if( !rBase.ReadIdFile( String::CreateFromAscii( aSlotIdFile.GetBuffer() ) ) )
        {
            ByteString aStr = "cannot read file: ";
            aStr += aSlotIdFile;
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );

            rInStm.Seek( nTokPos );
        }
    }
    aTypeLibFile.ReadSvIdl( SvHash_TypeLibFile(), rInStm );
	aModulePrefix.ReadSvIdl( SvHash_ModulePrefix(), rInStm );
}

/*************************************************************************
|*    SvMetaModule::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                         SvStream & rOutStm,
                                         sal_uInt16 nTab )
{
    SvMetaExtern::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    if( aTypeLibFile.Len() || aSlotIdFile.Len() || aTypeLibFile.Len() )
	{
	    if( aHelpFileName.Len() )
	    {
		    WriteTab( rOutStm, nTab );
	        aHelpFileName.WriteSvIdl( SvHash_HelpFile(), rOutStm, nTab +1 );
	        rOutStm << ';' << endl;
	    }
	    if( aSlotIdFile.Len() )
	    {
	        WriteTab( rOutStm, nTab );
	        aSlotIdFile.WriteSvIdl( SvHash_SlotIdFile(), rOutStm, nTab +1 );
	        rOutStm << ';' << endl;
	    }
	    if( aTypeLibFile.Len() )
	    {
	        WriteTab( rOutStm, nTab );
	        aTypeLibFile.WriteSvIdl( SvHash_TypeLibFile(), rOutStm, nTab +1 );
	        rOutStm << ';' << endl;
	    }
	}
}

/*************************************************************************
|*    SvMetaModule::ReadContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::ReadContextSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.GetToken()->Is( SvHash_interface() )
      || rInStm.GetToken()->Is( SvHash_shell() ) )
    {
        SvMetaClassRef aClass = new SvMetaClass();
        if( aClass->ReadSvIdl( rBase, rInStm ) )
        {
            aClassList.Append( aClass );
            // Global bekanntgeben
            rBase.GetClassList().Append( aClass );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_enum() ) )
    {
        SvMetaTypeEnumRef aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // Im Modul deklariert
            aTypeList.Append( aEnum );
            // Global bekanntgeben
            rBase.GetTypeList().Append( aEnum );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_item() )
      || rInStm.GetToken()->Is( SvHash_struct() )
      || rInStm.GetToken()->Is( SvHash_typedef() ) )
    {
        SvMetaTypeRef xItem = new SvMetaType();

        if( xItem->ReadSvIdl( rBase, rInStm ) )
        {
            // Im Modul deklariert
            aTypeList.Append( xItem );
            // Global bekanntgeben
            rBase.GetTypeList().Append( xItem );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_include() ) )
    {
        sal_Bool bOk = sal_False;
        rInStm.GetToken_Next();
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
        {
            DirEntry aFullName( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
			rBase.StartNewFile( aFullName.GetFull() );
            if( aFullName.Find( rBase.GetPath() ) )
            {
                SvTokenStream aTokStm( aFullName.GetFull() );
                if( SVSTREAM_OK == aTokStm.GetStream().GetError() )
                {
					// Fehler aus alter Datei retten
					SvIdlError aOldErr = rBase.GetError();
					// Fehler zuruecksetzen
					rBase.SetError( SvIdlError() );

                    sal_uInt32 nBeginPos = 0xFFFFFFFF; // kann mit Tell nicht vorkommen
                    while( nBeginPos != aTokStm.Tell() )
                    {
                        nBeginPos = aTokStm.Tell();
                        ReadContextSvIdl( rBase, aTokStm );
                        aTokStm.ReadDelemiter();
                    }
                    bOk = aTokStm.GetToken()->IsEof();
                    if( !bOk )
                    {
                        rBase.WriteError( aTokStm );
                    }
					// Fehler aus alter Datei wieder herstellen
					rBase.SetError( aOldErr );
                }
                else
                {
                    ByteString aStr = "cannot open file: ";
                    aStr += ByteString( aFullName.GetFull(), RTL_TEXTENCODING_UTF8 );
                    rBase.SetError( aStr, pTok );
                }
            }
            else
            {
                ByteString aStr = "cannot find file: ";
                aStr += ByteString( aFullName.GetFull(), RTL_TEXTENCODING_UTF8 );
                rBase.SetError( aStr, pTok );
            }
        }
        if( !bOk )
            rInStm.Seek( nTokPos );
    }
    else
    {
        SvMetaSlotRef xSlot = new SvMetaSlot();

        if( xSlot->ReadSvIdl( rBase, rInStm ) )
        {
	    	if( xSlot->Test( rBase, rInStm ) )
			{
	            // Im Modul deklariert
	            aAttrList.Append( xSlot );
	            // Global bekanntgeben
	            rBase.AppendAttr( xSlot );
			}
        }
    }
}

/*************************************************************************
|*    SvMetaModule::WriteContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::WriteContextSvIdl( SvIdlDataBase & rBase,
                                      SvStream & rOutStm,
                                      sal_uInt16 nTab )
{
    SvMetaExtern::WriteContextSvIdl( rBase, rOutStm, nTab );
	sal_uLong n;
    for( n = 0; n < aTypeList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aTypeList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
	rOutStm << endl;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
	rOutStm << endl;
    for( n = 0; n < aClassList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aClassList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
}

/*************************************************************************
|*
|*    SvMetaModule::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvMetaModule::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bIsModified = sal_True; // bisher immer wenn Compiler laueft

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken_Next();
    sal_Bool bOk        = sal_False;
    bOk = pTok->Is( SvHash_module() );
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aBeginName.MakeId( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aEndName.MakeId( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        aNextName = aBeginName;

        rBase.Push( this ); // auf den Context Stack

        if( ReadNameSvIdl( rBase, rInStm ) )
        {
            // Zeiger auf sich selbst setzen
			SetModule( rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        rBase.GetStack().Pop(); // und runter
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

/*************************************************************************
|*
|*    SvMetaModule::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaModule::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               sal_uInt16 nTab )
{
    rOutStm << SvHash_module()->GetName().GetBuffer() << endl
            << '\"';
    rOutStm.WriteByteString( aBeginName.GetHexName() );
    rOutStm << '\"' << endl << '\"';
    rOutStm.WriteByteString( aEndName.GetHexName() );
    rOutStm << '\"' << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
}

/*************************************************************************
|*    SvMetaModule::WriteSfx()
*************************************************************************/
void SvMetaModule::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	{
        SvMetaClass * pClass = aClassList.GetObject( n );
		pClass->WriteSfx( rBase, rOutStm );
	}
}

void SvMetaModule::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
							Table* pTable )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	{
        SvMetaClass * pClass = aClassList.GetObject( n );
		pClass->WriteHelpIds( rBase, rOutStm, pTable );
	}
}

/*************************************************************************
|*    SvMetaModule::WriteAttributes()
*************************************************************************/
void SvMetaModule::WriteAttributes( SvIdlDataBase & rBase,
									SvStream & rOutStm,
                             		sal_uInt16 nTab,
		 						    WriteType nT, WriteAttribute nA )
{
	SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
    if( aHelpFileName.Len() )
	{
		WriteTab( rOutStm, nTab );
		rOutStm << "// class SvMetaModule" << endl;
		WriteTab( rOutStm, nTab );
        rOutStm << "helpfile(\"" << aHelpFileName.GetBuffer() << "\");" << endl;
	}
}

/*************************************************************************
|*    SvMetaModule::WriteSbx()
*************************************************************************/
/*
void SvMetaModule::WriteSbx( SvIdlDataBase & rBase, SvStream & rOutStm,
								SvNamePosList & rList )
{
	for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	{
	    SvMetaClass * pClass = aClassList.GetObject( n );
		if( !pClass->IsShell() && pClass->GetAutomation() )
		{
			rList.Insert( new SvNamePos( pClass->GetUUId(), rOutStm.Tell() ),
						LIST_APPEND );
			SbxObjectRef xSbxObj = new SbxObject( pClass->GetName() );
			pClass->FillSbxObject( rBase, xSbxObj );
			xSbxObj->Store( rOutStm );
		}
	}
}
 */

/*************************************************************************
|*    SvMetaModule::Write()
*************************************************************************/
void SvMetaModule::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                          	sal_uInt16 nTab,
		 					WriteType nT, WriteAttribute nA )
{
	switch ( nT )
	{
	case WRITE_ODL:
	{
	    if( aSlotIdFile.Len() )
		{
			WriteTab( rOutStm, nTab );
            rOutStm << "#include \"" << aSlotIdFile.GetBuffer() << '"' << endl << endl;
	    }
		SvMetaExtern::Write( rBase, rOutStm, nTab, nT, nA );
		rOutStm << endl;
		WriteTab( rOutStm, nTab );
        rOutStm << "library " << GetName().GetBuffer() << endl;
		WriteTab( rOutStm, nTab );
	    rOutStm << '{' << endl;
		WriteTab( rOutStm, nTab );
		rOutStm << "importlib(\"STDOLE.TLB\");" << endl;

	/*
	    for( sal_uLong n = 0; n < aTypeList.Count(); n++ )
	    {
	        SvMetaType * pType = aTypeList.GetObject( n );
	        if( !pType ->Write( rBase, rOutStm, nTab +1, nT, nA ) )
	            return sal_False;
	    }
	*/
		/*
	    for( sal_uLong n = 0; n < rBase.GetModuleList().Count(); n++ )
	    {
	        SvMetaModule * pModule = rBase.GetModuleList().GetObject( n );
	        const SvMetaTypeMemberList &rTypeList = pModule->GetTypeList();
	        for( sal_uLong n = 0; n < rTypeList.Count(); n++ )
	        {
	            SvMetaType * pType = rTypeList.GetObject( n );
	            pType->Write( rBase, rOutStm, nTab +1, nT, nA );
	        }
	    }
		*/

	    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	    {
	        SvMetaClass * pClass = aClassList.GetObject( n );
			if( !pClass->IsShell() && pClass->GetAutomation() )
			{
				WriteTab( rOutStm, nTab );
				WriteStars( rOutStm );
		        pClass->Write( rBase, rOutStm, nTab +1, nT, nA );
			    if( n +1 < aClassList.Count() )
					rOutStm << endl;
			}
	    }

	    rOutStm << '}' << endl;
	}
	break;
	case WRITE_DOCU:
	{
		rOutStm << "SvIDL interface documentation" << endl << endl;
        rOutStm << "<MODULE>" << endl << GetName().GetBuffer() << endl;
		WriteDescription( rOutStm );
		rOutStm	<< "</MODULE>" << endl << endl;

		rOutStm << "<CLASSES>" << endl;
	    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	    {
	        SvMetaClass * pClass = aClassList.GetObject( n );
			if( !pClass->IsShell() )
			{
                rOutStm << pClass->GetName().GetBuffer();
				SvMetaClass* pSC = pClass->GetSuperClass();
			    if( pSC )
                    rOutStm << " : " << pSC->GetName().GetBuffer();

				// Importierte Klassen
				const SvClassElementMemberList& rClassList = pClass->GetClassList();
				if ( rClassList.Count() )
				{
					rOutStm << " ( ";

					for( sal_uLong m=0; m<rClassList.Count(); m++ )
					{
						SvClassElement *pEle = rClassList.GetObject(m);
						SvMetaClass *pCl = pEle->GetClass();
                        rOutStm << pCl->GetName().GetBuffer();
						if ( m+1 == rClassList.Count() )
							rOutStm << " )";
						else
							rOutStm << " , ";
					}
				}

				rOutStm << endl;
			}
		}
		rOutStm << "</CLASSES>" << endl << endl;
		// kein Break!
	}

	case WRITE_C_SOURCE:
	case WRITE_C_HEADER:
	{
	    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
	    {
	        SvMetaClass * pClass = aClassList.GetObject( n );
			if( !pClass->IsShell() /* && pClass->GetAutomation() */ )
	        	pClass->Write( rBase, rOutStm, nTab, nT, nA );
	    }
	}
	break;

    default:
        break;
	}
}

/*************************************************************************
|*    SvMetaModule::WriteSrc()
*************************************************************************/
void SvMetaModule::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
//	rOutStm << "#pragma CHARSET IBMPC" << endl;
    if( aSlotIdFile.Len() )
        rOutStm << "//#include <" << aSlotIdFile.GetBuffer() << '>' << endl;
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        aClassList.GetObject( n )->WriteSrc( rBase, rOutStm, pTable );
    }
}

/*************************************************************************
|*    SvMetaModule::WriteHxx()
*************************************************************************/
void SvMetaModule::WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteHxx( rBase, rOutStm, nTab );
    }
}

/*************************************************************************
|*    SvMetaModule::WriteCxx()
*************************************************************************/
void SvMetaModule::WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteCxx( rBase, rOutStm, nTab );
    }
}

#endif // IDL_COMPILER

