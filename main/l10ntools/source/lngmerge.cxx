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
#include "precompiled_l10ntools.hxx"
#include <tools/fsys.hxx>

// local includes
#include "lngmerge.hxx"
#include "utf8conv.hxx"
#include <iostream>
using namespace std;
//
// class LngParser
//
/*****************************************************************************/
LngParser::LngParser( const ByteString &rLngFile, sal_Bool bUTF8, sal_Bool bULFFormat )
/*****************************************************************************/
				:
				nError( LNG_OK ),
				pLines( NULL ),
                sSource( rLngFile ),
				bDBIsUTF8( bUTF8 ),
				bULF( bULFFormat )
{
	pLines = new LngLineList( 100, 100 );
	DirEntry aEntry( String( sSource, RTL_TEXTENCODING_ASCII_US ));
	if ( aEntry.Exists()) {
		SvFileStream aStream( String( sSource, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
		if ( aStream.IsOpen()) {
			ByteString sLine;
            bool bFirstLine = true;			
            while ( !aStream.IsEof()) {
				aStream.ReadLine( sLine );
				
                if( bFirstLine ){       // Always remove UTF8 BOM from the first line
                    Export::RemoveUTF8ByteOrderMarker( sLine );
                    bFirstLine = false;
                }
                
                pLines->Insert( new ByteString( sLine ), LIST_APPEND );
			}
		}
		else
			nError = LNG_COULD_NOT_OPEN;
	}
	else
		nError = LNG_FILE_NOTFOUND;
}

/*****************************************************************************/
LngParser::~LngParser()
/*****************************************************************************/
{
	for ( sal_uLong i = 0; i < pLines->Count(); i++ )
		delete pLines->GetObject( i );
	delete pLines;
}

/*****************************************************************************/
void LngParser::FillInFallbacks( ByteStringHashMap Text )
/*****************************************************************************/
{
    ByteString sCur;
    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];
        
        if( Export::isAllowed( sCur ) ){
            ByteString sFallbackLang = Export::GetFallbackLanguage( sCur );
            if( sFallbackLang.Len() ){
                Text[ sCur ] = Text[ sFallbackLang ];
            }
		}
	}
}

/*****************************************************************************/
sal_Bool LngParser::CreateSDF(
	const ByteString &rSDFFile, const ByteString &rPrj,
	const ByteString &rRoot )
/*****************************************************************************/
{

    Export::InitLanguages( false );
    aLanguages = Export::GetLanguages();
    SvFileStream aSDFStream( String( rSDFFile, RTL_TEXTENCODING_ASCII_US ),
		STREAM_STD_WRITE | STREAM_TRUNC );
	if ( !aSDFStream.IsOpen()) {
		nError = SDF_COULD_NOT_OPEN;
	}
    aSDFStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );	
    nError = SDF_OK;
	DirEntry aEntry( String( sSource, RTL_TEXTENCODING_ASCII_US ));
	aEntry.ToAbs();
	String sFullEntry = aEntry.GetFull();
	aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
	aEntry += DirEntry( rRoot );
	ByteString sPrjEntry( aEntry.GetFull(), gsl_getSystemTextEncoding());
	ByteString sActFileName(
		sFullEntry.Copy( sPrjEntry.Len() + 1 ), gsl_getSystemTextEncoding());
	sActFileName.SearchAndReplaceAll( "/", "\\" );

	sal_uLong nPos  = 0;
	sal_Bool bStart = true;
	ByteString sGroup;
  	ByteStringHashMap Text;
    ByteString sID;
    ByteString sLine;

    while( nPos < pLines->Count() ){
        sLine = *pLines->GetObject( nPos++ );    
        while( nPos < pLines->Count() && !isNextGroup( sGroup , sLine ) ){
            ReadLine( sLine , Text );
            sID = sGroup;
            sLine = *pLines->GetObject( nPos++ );    
        };
        if( bStart ){
            bStart = false;
            sID = sGroup;
        }
        else {

            WriteSDF( aSDFStream , Text , rPrj , rRoot , sActFileName , sID );    
        }
    }
	aSDFStream.Close();
	return true;
}

 void LngParser::WriteSDF( SvFileStream &aSDFStream , ByteStringHashMap &rText_inout ,	
     const ByteString &rPrj , const ByteString &rRoot ,
     const ByteString &sActFileName , const ByteString &sID )
 {
     
    sal_Bool bExport = true;
    if ( bExport ) {
   		ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;
		FillInFallbacks( rText_inout );
        for( unsigned int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
            ByteString sAct = rText_inout[ sCur ];	
            if ( !sAct.Len() && sCur.Len() )
                sAct = rText_inout[ ByteString("en-US") ];
		    
            ByteString sOutput( rPrj ); sOutput += "\t";
			if ( rRoot.Len())
			    sOutput += sActFileName;
			sOutput += "\t0\t";
			sOutput += "LngText\t";
			sOutput += sID; sOutput += "\t\t\t\t0\t";
			sOutput += sCur; sOutput += "\t";
			sOutput += sAct; sOutput += "\t\t\t\t";
			sOutput += sTimeStamp;
            //if( !sCur.EqualsIgnoreCaseAscii("de") ||( sCur.EqualsIgnoreCaseAscii("de") && !Export::isMergingGermanAllowed( rPrj ) ) )
            aSDFStream.WriteLine( sOutput );
		}
    }
 }
 bool LngParser::isNextGroup( ByteString &sGroup_out , ByteString &sLine_in ){
	sLine_in.EraseLeadingChars( ' ' );
	sLine_in.EraseTrailingChars( ' ' );
	if (( sLine_in.GetChar( 0 ) == '[' ) &&
			( sLine_in.GetChar( sLine_in.Len() - 1 ) == ']' )){
		sGroup_out = sLine_in.GetToken( 1, '[' ).GetToken( 0, ']' );
		sGroup_out.EraseLeadingChars( ' ' );
		sGroup_out.EraseTrailingChars( ' ' );
		return true;
	}
    return false;
 }
 void LngParser::ReadLine( const ByteString &sLine_in , ByteStringHashMap &rText_inout){
    //printf("sLine -> '%s'\n",sLine_in.GetBuffer());
    ByteString sLang = sLine_in.GetToken( 0, '=' );
	sLang.EraseLeadingChars( ' ' );
	sLang.EraseTrailingChars( ' ' );
	ByteString sText = sLine_in.GetToken( 1, '\"' ).GetToken( 0, '\"' );
	if( sLang.Len() )
        rText_inout[ sLang ] = sText;   
 }

/*****************************************************************************/
sal_Bool LngParser::Merge(
	const ByteString &rSDFFile, const ByteString &rDestinationFile , const ByteString& rPrj )
/*****************************************************************************/
{
    (void) rPrj;
    Export::InitLanguages( true );
    SvFileStream aDestination(
		String( rDestinationFile, RTL_TEXTENCODING_ASCII_US ),
		STREAM_STD_WRITE | STREAM_TRUNC );
	if ( !aDestination.IsOpen()) {
		nError = LNG_COULD_NOT_OPEN;
	}
	nError = LNG_OK;
//    MergeDataFile( const ByteString &rFileName, const ByteString& rFile , sal_Bool bErrLog, CharSet aCharSet, sal_Bool bUTF8 );

	MergeDataFile aMergeDataFile( rSDFFile, sSource , sal_False, RTL_TEXTENCODING_MS_1252);//, bDBIsUTF8 );
    ByteString sTmp( Export::sLanguages );
    if( sTmp.ToUpperAscii().Equals("ALL") ) 
        Export::SetLanguages( aMergeDataFile.GetLanguages() );
    aLanguages = Export::GetLanguages();

	sal_uLong nPos = 0;
	sal_Bool bGroup = sal_False;
	ByteString sGroup;

	// seek to next group
	while ( nPos < pLines->Count() && !bGroup ) {
		ByteString sLine( *pLines->GetObject( nPos ));
		sLine.EraseLeadingChars( ' ' );
		sLine.EraseTrailingChars( ' ' );
		if (( sLine.GetChar( 0 ) == '[' ) &&
			( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
		{
			sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
			sGroup.EraseLeadingChars( ' ' );
			sGroup.EraseTrailingChars( ' ' );
			bGroup = sal_True;
		}
		nPos ++;
	}

	while ( nPos < pLines->Count()) {
        ByteStringHashMap Text;
		ByteString sID( sGroup );
		sal_uLong nLastLangPos = 0;

		ResData  *pResData = new ResData( "", sID , sSource );
        pResData->sResTyp = "LngText";
		PFormEntrys *pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
		// read languages
		bGroup = sal_False;

		ByteString sLanguagesDone;

		while ( nPos < pLines->Count() && !bGroup ) {
			ByteString sLine( *pLines->GetObject( nPos ));
			sLine.EraseLeadingChars( ' ' );
			sLine.EraseTrailingChars( ' ' );
			if (( sLine.GetChar( 0 ) == '[' ) &&
				( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
			{
				sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
				sGroup.EraseLeadingChars( ' ' );
				sGroup.EraseTrailingChars( ' ' );
				bGroup = sal_True;
				nPos ++;
				sLanguagesDone = "";
			}
			else if ( sLine.GetTokenCount( '=' ) > 1 ) {
				ByteString sLang = sLine.GetToken( 0, '=' );
				sLang.EraseLeadingChars( ' ' );
				sLang.EraseTrailingChars( ' ' );

				ByteString sSearch( ";" );
				sSearch += sLang;
				sSearch += ";";

				if (( sLanguagesDone.Search( sSearch ) != STRING_NOTFOUND )) {
					pLines->Remove( nPos );
				}
                if( bULF && pEntrys )
				{
					// this is a valid text line
					ByteString sText = sLine.GetToken( 1, '\"' ).GetToken( 0, '\"' );
                    if( sLang.Len() ){
						ByteString sNewText;
                        pEntrys->GetText( sNewText, STRING_TYP_TEXT, sLang, sal_True );

						if ( sNewText.Len()) {
							ByteString *pLine = pLines->GetObject( nPos );
                            
								ByteString sText1( sLang );
								sText1 += " = \"";
								sText1 += sNewText;
								sText1 += "\"";
								*pLine = sText1;
							//}
                            Text[ sLang ] = sNewText;
						}
					}
					nLastLangPos = nPos;
					nPos ++;
					sLanguagesDone += sSearch;
				}
				else {
					nLastLangPos = nPos;
					nPos ++;
					sLanguagesDone += sSearch;
				}
			}
			else
				nPos++;
		}
		ByteString sCur;
        if ( nLastLangPos ) {
            for( unsigned int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];
                if(   //( !sCur.EqualsIgnoreCaseAscii("de") || 
                      //( sCur.EqualsIgnoreCaseAscii("de") && Export::isMergingGermanAllowed( rPrj ) ) ) 
                    !sCur.EqualsIgnoreCaseAscii("en-US") && !Text[ sCur ].Len() && pEntrys ){
                    
                    ByteString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur, sal_True );
                    if (( sNewText.Len()) &&
                        !(( sCur.Equals("x-comment") ) && ( sNewText == "-" )))
					{
						ByteString sLine;
                        sLine += sCur;
						sLine += " = \"";
						sLine += sNewText;
						sLine += "\"";

						nLastLangPos++;
						nPos++;

						pLines->Insert( new ByteString( sLine ), nLastLangPos );
					}
				}
			}
		}

		delete pResData;
	}

	for ( sal_uLong i = 0; i < pLines->Count(); i++ )
		aDestination.WriteLine( *pLines->GetObject( i ));

	aDestination.Close();
	return sal_True;
}
