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
#include "precompiled_soltools.hxx"

#include <gi_parse.hxx>

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <gilacces.hxx>


using namespace std;


const char * C_sLineEnd = "\r\n";



inline void
WriteStr( ostream & o_rOut, const Simstr & i_rStr )
{
    o_rOut.write( i_rStr.str(), i_rStr.l() );
}

inline void
WriteStr( ostream & o_rOut, const char * i_rStr )
{
    o_rOut.write( i_rStr, strlen(i_rStr) );
}

inline void
GenericInfo_Parser::SetError( E_Error i_eError )
{
 	eErrorCode = i_eError;
    nErrorLine = nCurLine;
}


GenericInfo_Parser::GenericInfo_Parser()
    :   sCurParsePosition(""),
        nCurLine(0),
        nLevel(0),
        bGoon(false),
        // sCurComment,
        eErrorCode(ok),
	    nErrorLine(0),
        pResult(0),
        pResource(0)
{
}

GenericInfo_Parser::~GenericInfo_Parser()
{
}

bool
GenericInfo_Parser::LoadList( GenericInfoList_Builder &  o_rResult,
                              const Simstr &             i_sSourceFileName )
{
    ifstream  aFile( i_sSourceFileName.str() );
    if ( aFile.fail() )
    {
        SetError(cannot_open);
        return false;
    }

    aFile.seekg(0, ios::end);
    UINT32 nTextSize = aFile.tellg();
    if ( nTextSize == 0 || nTextSize == UINT32(-1) )
        return true;
    dpBuffer = new char[nTextSize+2];

    aFile.seekg(0);
    aFile.read( dpBuffer, nTextSize );
    aFile.close();

    sFilePtr = dpBuffer;
    char * sLastChar = dpBuffer + nTextSize - 1;

    while ( sFilePtr != sLastChar && *sFilePtr <= 32 )
        ++sCurParsePosition;
    if ( sFilePtr == sLastChar )
    {
        if ( *sFilePtr <= 32 )
            return true;
    }
    else while ( *sLastChar <= 32 )
    {
        --sLastChar;
    }

    *(sLastChar+1) = '\n';
    *(sLastChar+2) = '\0';

    ResetState(o_rResult);

    for ( ReadLine(); bGoon; ReadLine() )
    {
        bool bOk = InterpretLine();
        if ( !bOk)
        {
            SetError(syntax_error);
            break;
        }
    }

    if ( nLevel > 0 && eErrorCode == ok)
    {
        SetError(unexpected_eof);
    }
    else if ( nLevel < 0 )
    {
        SetError(unexpected_list_end);
    }

    delete [] dpBuffer;
    dpBuffer = 0;
    sFilePtr = 0;

	return eErrorCode == ok;
}

bool
GenericInfo_Parser::SaveList( const Simstr &            i_rOutputFile,
	                          GenericInfoList_Browser & io_rListBrowser )
{
    ofstream  aFile( i_rOutputFile.str() );
    if ( aFile.fail() )
    {
        SetError(cannot_open);
        return false;
    }

    ResetState(io_rListBrowser);

    WriteList(aFile);

	aFile.close();
	return eErrorCode == ok;
}

void
GenericInfo_Parser::ResetState( GenericInfoList_Builder & io_rResult )
{
    sCurParsePosition = "";
    nCurLine = 0;
    nLevel = 0;
    bGoon = true;
    sCurComment = "";
    eErrorCode = ok;
	nErrorLine = 0;
    pResult = &io_rResult;
    pResource = 0;
}

void
GenericInfo_Parser::ResetState( GenericInfoList_Browser & io_rSrc )
{
    sCurParsePosition = "";
    nCurLine = 0;
    nLevel = 0;
    bGoon = false;
    sCurComment = "";
    eErrorCode = ok;
	nErrorLine = 0;
    pResult = 0;
    pResource = &io_rSrc;
}


void
GenericInfo_Parser::ReadLine()
{
    if ( *sFilePtr == '\0' )    // See initialising of dpBuffer and sLastChar in LoadList().
	{
		bGoon = false;
		return;
	}

    sCurParsePosition = sFilePtr;
    while ( *sFilePtr != '\n' )
        ++sFilePtr;
    nCurLine++;

    // Remove leading and trailing whitespace from line:
    while ( sCurParsePosition != sFilePtr && *sCurParsePosition <= 32 )
        ++sCurParsePosition;

    char * sEndOfLine = sFilePtr;
    while ( sEndOfLine != sCurParsePosition && *sEndOfLine <= 32 )
          --sEndOfLine;
    if ( sCurParsePosition != sEndOfLine || *sCurParsePosition > 32 )
	    ++sEndOfLine;
    *sEndOfLine = '\0';

    ++sFilePtr; // Go beyond line end to first character of next line.
}

bool
GenericInfo_Parser::InterpretLine()
{
    switch ( ClassifyLine() )
    {
        case lt_key:        ReadKey();
                            break;
        case lt_open_list:  PushLevel_Read();
                            break;
        case lt_close_list: PopLevel_Read();
                            break;
        case lt_comment:    AddCurLine2CurComment();
                            break;
        case lt_empty:      AddCurLine2CurComment();
                            break;
        default:
                            return false;
    }
    return true;
}

GenericInfo_Parser::E_LineType
GenericInfo_Parser::ClassifyLine()
{
    switch ( *sCurParsePosition )
    {
     	case '{':   return lt_open_list;
        case '}':   return lt_close_list;
        case '#':   return lt_comment;
        case '\0':   return lt_empty;
    }

    return lt_key;
}

void
GenericInfo_Parser::ReadKey()
{
    const char * pSearch = sCurParsePosition;

    for ( ; *pSearch > 32; ++pSearch ) ;
    UINT32 nKeyLength = pSearch - sCurParsePosition;

    for ( ; *pSearch <= 32 && *pSearch > '\0'; ++pSearch ) ;

    pResult->AddKey( sCurParsePosition,     nKeyLength,
                     pSearch,               strlen(pSearch),
                     sCurComment.str(),     sCurComment.l()
                   );
    sCurComment = "";
}

void
GenericInfo_Parser::PushLevel_Read()
{
    nLevel++;
    pResult->OpenList();
}

void
GenericInfo_Parser::PopLevel_Read()
{
    nLevel--;
    pResult->CloseList();
}

void
GenericInfo_Parser::AddCurLine2CurComment()
{
    sCurComment +=  sCurParsePosition;
    sCurComment +=  C_sLineEnd;
}

void
GenericInfo_Parser::WriteList( ostream & o_rFile )
{
    static char sBuffer[32000];

    for ( bGoon = pResource->Start_CurList();
          bGoon;
          bGoon = pResource->NextOf_CurList() )
    {
        pResource->Get_CurComment(&sBuffer[0]);
        WriteComment(o_rFile,sBuffer);

        pResource->Get_CurKey(&sBuffer[0]);
        WriteKey(o_rFile,sBuffer);

        pResource->Get_CurValue(&sBuffer[0]);
        WriteValue(o_rFile,sBuffer);

        if ( pResource->HasSubList_CurKey() )
        {
            PushLevel_Write();

/*
            WriteIndentation();
            o_rFile.write("{",1);
            o_rFile.write(C_sLineEnd, C_nLineEndLength);
*/
            WriteList(o_rFile);

/*
            WriteIndentation();
            o_rFile.write("}",1);
            o_rFile.write(C_sLineEnd, C_nLineEndLength);
*/
            PopLevel_Write();
        }
    }   // end for
}

void
GenericInfo_Parser::PushLevel_Write()
{
    nLevel++;
    pResource->Push_CurList();
}

void
GenericInfo_Parser::PopLevel_Write()
{
    nLevel--;
    pResource->Pop_CurList();
}

void
GenericInfo_Parser::WriteComment( ostream &    o_rFile,
                                  const char *  i_sStr )
{
    WriteStr( o_rFile, i_sStr );
    if ( i_sStr[ strlen(i_sStr)-1 ] != '\n' )
        WriteStr( o_rFile, C_sLineEnd );
}

void
GenericInfo_Parser::WriteKey( ostream &    o_rFile,
                              const char *  i_sStr )
{
    WriteIndentation(o_rFile);
    WriteStr( o_rFile, i_sStr );
}

void
GenericInfo_Parser::WriteValue( ostream &    o_rFile,
                                const char *  i_sStr )
{
    if ( i_sStr != 0 ? strlen(i_sStr) > 0 : false )
    {
     	WriteStr(o_rFile," ");
     	WriteStr(o_rFile,i_sStr);
    }

    WriteStr(o_rFile,C_sLineEnd);
}

void
GenericInfo_Parser::WriteIndentation( ostream &    o_rFile )
{
    const int nIndentBound = 60;

    static const char sIndentation[nIndentBound+1] =
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
        "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

    if ( nLevel == 0 )
        return;

    if ( nLevel <= nIndentBound )
        o_rFile.write( sIndentation, nLevel );
    else
    {
        INT16 iLevel = nLevel;
        for ( ; iLevel > nIndentBound; iLevel-=nIndentBound )
            o_rFile.write( sIndentation, nIndentBound );
        o_rFile.write( sIndentation, iLevel );
    }
}



