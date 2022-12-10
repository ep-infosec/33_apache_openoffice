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

#include "srciter.hxx"
#include "export.hxx"
#include "treeconfig.hxx"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include "tools/errcode.hxx"
#include "tools/fsys.hxx"

#ifndef L10NTOOLS_FILE_HXX
#define L10NTOOLS_FILE_HXX
#include <l10ntools/file.hxx>
#endif

namespace transex3
{

//
// SourceTreeLocalizer
//

const char *ExeTable[][5] = {
	{ "src", "transex3", "  -UTF8 -e", "negative", "noiso" },
	{ "hrc", "transex3", "  -UTF8 -e", "positive", "noiso" },
	{ "tree", "xhtex", "", "negative", "noiso" },
	{ "xtx", "xtxex", "", "negative", "noiso" },
	{ "ulf", "ulfex", " -e", "negative", "noiso" },
	{ "xrb", "xmlex", "-UTF8 -e", "negative", "iso" },
	{ "xxl", "xmlex", "-UTF8 -e", "negative", "iso" },
	{ "xgf", "xmlex", "-UTF8 -e -t:xgf", "negative", "iso" },
	{ "xcd", "cfgex", "-UTF8 -e", "negative", "iso" },
	{ "xcu", "cfgex", "-UTF8 -e", "negative", "iso" },
	{ "xcs", "cfgex", "-UTF8 -e -f", "negative", "iso" },
	{ "xrm", "xrmex", "-UTF8 -e", "negative", "iso" },
	{ "xhp", "helpex", " -e", "negative", "noiso" },
	{ "properties", "jpropex", " -e", "negative", "noiso" },
	{ "NULL", "NULL", "NULL", "NULL", "NULL" }
};

const char *NegativeList[] = {
	"officecfg/data/org.openoffice.Office.Labels.xcd",
	"officecfg/data/org/openoffice/Office/Labels.xcd",
	"officecfg/data/org/openoffice/Office/SFX.xcd",
	"officecfg/data/org/openoffice/Office/Accelerators.xcu",
	"hidother.src",
	"NULL"
};

const char *PositiveList[] = {
	"svx/inc/globlmn_tmpl.hrc",
	"sw/source/ui/inc/swmn_tmpl.hrc",
	"sw/source/ui/inc/swacc_tmpl.hrc",
	"sw/source/ui/inc/toolbox_tmpl.hrc",
	"offmgr/inc/offmenu_tmpl.hrc",
	"offmgr/source/offapp/intro/intro_tmpl.hrc",
	"dbaccess/source/ui/inc/toolbox_tmpl.hrc",
	"svx/source/intro/intro_tmpl.hrc",
	"dbaccess/source/ui/dlg/AutoControls_tmpl.hrc",
	"svx/source/unodialogs/textconversiondlgs/chinese_direction_tmpl.hrc",
	"chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc",
	"chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc",
	"chart2/source/controller/dialogs/res_Statistic_tmpl.hrc",
	"chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc",
	"chart2/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc",
	"chart2/source/controller/menu/MenuItems_tmpl.hrc",
	"chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc",
	"chart2/source/controller/dialogs/res_Trendline_tmpl.hrc",
	"svx.link/inc/globlmn_tmpl.hrc",
	"sw.link/source/ui/inc/swmn_tmpl.hrc",
	"sw.link/source/ui/inc/swacc_tmpl.hrc",
	"sw.link/source/ui/inc/toolbox_tmpl.hrc",
	"offmgr.link/inc/offmenu_tmpl.hrc",
	"offmgr.link/source/offapp/intro/intro_tmpl.hrc",
	"dbaccess.link/source/ui/inc/toolbox_tmpl.hrc",
	"svx.link/source/intro/intro_tmpl.hrc",
	"dbaccess.link/source/ui/dlg/AutoControls_tmpl.hrc",
	"svx.link/source/unodialogs/textconversiondlgs/chinese_direction_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_DataLabel_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_LegendPosition_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_Statistic_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_Titlesx_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc",
	"chart2.link/source/controller/menu/MenuItems_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_ErrorBar_tmpl.hrc",
	"chart2.link/source/controller/dialogs/res_Trendline_tmpl.hrc",
	"NULL"
};


const char PRJ_DIR_NAME[] = "prj";
const char DLIST_NAME[] = "d.lst";

#define LOCALIZE_NONE		0x0000
#define LOCALIZE_EXTRACT	0x0001
#define LOCALIZE_MERGE		0x0002

class SourceTreeLocalizer : public SourceTreeIterator
{
private:
	SvFileStream aSDF;
	sal_uInt16 nMode;

	ByteString sLanguageRestriction;

	ByteString sOutputFile;

	int nFileCnt;

	const ByteString GetProjectName( sal_Bool bAbs = sal_False );
	const ByteString GetProjectRootRel();


	sal_Bool CheckNegativeList( const ByteString &rFileName );
	sal_Bool CheckPositiveList( const ByteString &rFileName );

	void WorkOnFile(
		const ByteString &rFileName,
		const ByteString &rExecutable,
		const ByteString &rParameter
	);

	void WorkOnFileType(
		const ByteString &rDirectory,
		const ByteString &rExtension,
		const ByteString &rExecutable,
		const ByteString &rParameter,
		const ByteString &rCollectMode
	);
	void WorkOnDirectory( const ByteString &rDirectory );
	sal_Bool ExecuteMerge();
	sal_Bool MergeSingleFile(
		const ByteString &rPrj,
		const ByteString &rFile,
		const ByteString &rSDFFile
	);

public:
	SourceTreeLocalizer( const ByteString &rRoot, const ByteString &rVersion , bool bLocal , bool skip_links );
	~SourceTreeLocalizer();

	ByteString getSourceLanguages( ByteString sLanguageRestriction , ByteString sCommand );

	void SetLanguageRestriction( const ByteString& rRestrictions )
		{ sLanguageRestriction = rRestrictions; }
	int getFileCnt();
	sal_Bool Extract( const ByteString &rDestinationFile );
	sal_Bool Merge( const ByteString &rSourceFile , const ByteString &rOutput );
	int GetFileCnt();
	virtual void OnExecuteDirectory( const rtl::OUString &rDirectory );
};

/*****************************************************************************/
SourceTreeLocalizer::SourceTreeLocalizer(
	const ByteString &rRoot, const ByteString &rVersion, bool bLocal_in , bool skip_links )
/*****************************************************************************/
				: SourceTreeIterator( rRoot, rVersion , bLocal_in ),
				nMode( LOCALIZE_NONE ),
				nFileCnt( 0 )
{
		bSkipLinks = skip_links ;
}

/*****************************************************************************/
SourceTreeLocalizer::~SourceTreeLocalizer()
/*****************************************************************************/
{
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectName( sal_Bool bAbs )
/*****************************************************************************/
{
	sal_Bool bFound = sal_False;
	DirEntry aCur;
	aCur.ToAbs();

	for ( ; ! bFound && aCur.Level() > 1; aCur.CutName() )
	{
		DirEntry aTest = aCur + DirEntry(PRJ_DIR_NAME) + DirEntry(DLIST_NAME);
		if ( aTest.Exists() )
		{
			if ( bAbs )
				return ByteString( aCur.GetFull(), RTL_TEXTENCODING_ASCII_US );
			else
				return ByteString( aCur.GetName(), RTL_TEXTENCODING_ASCII_US );
		}
	}

	return "";
}
/*****************************************************************************/
int SourceTreeLocalizer::GetFileCnt(){
/*****************************************************************************/
	return nFileCnt;
}

/*****************************************************************************/
const ByteString SourceTreeLocalizer::GetProjectRootRel()
/*****************************************************************************/
{
	ByteString sProjectRoot( GetProjectName( sal_True ));
	DirEntry aCur;
	aCur.ToAbs();
	ByteString sCur( aCur.GetFull(), RTL_TEXTENCODING_ASCII_US );

	if( sCur.SearchAndReplace( sProjectRoot, "" ) == STRING_NOTFOUND )
		return "";

	ByteString sDelimiter(
		DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

	sCur.SearchAndReplaceAll( sDelimiter, "/" );
	sCur.EraseLeadingChars( '/' );
	sal_uLong nCount = sCur.GetTokenCount( '/' );

	ByteString sProjectRootRel;
	for ( sal_uLong i = 0; i < nCount; i++ ) {
		if ( sProjectRootRel.Len())
			sProjectRootRel += sDelimiter;
		sProjectRootRel += "..";
	}
	if ( sProjectRootRel.Len())
		return sProjectRootRel;

	return ".";
}

bool skipProject( ByteString sPrj )
{
	static const ByteString READLICENSE( "readlicense" );
	return sPrj.EqualsIgnoreCaseAscii( READLICENSE );
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFile(
	const ByteString &rFileName, const ByteString &rExecutable,
	const ByteString &rParameter )
/*****************************************************************************/
{
		String sFull( rFileName, RTL_TEXTENCODING_ASCII_US );
		DirEntry aEntry( sFull );
		ByteString sFileName( aEntry.GetName(), RTL_TEXTENCODING_ASCII_US );

		// set current working directory
		DirEntry aPath( aEntry.GetPath());
		DirEntry aOldCWD;
		aPath.SetCWD();

		ByteString sPrj( GetProjectName());
		if ( sPrj.Len() && !skipProject( sPrj ) )
		{
			ByteString sRoot( GetProjectRootRel());

			DirEntry aTemp( Export::GetTempFile());
			ByteString sTempFile( aTemp.GetFull(), RTL_TEXTENCODING_ASCII_US );

			ByteString sDel;
#if defined(WNT) || defined(OS2)
			sDel=ByteString("\\");
#else
			sDel=ByteString("/");
#endif
			ByteString sPath1( Export::GetEnv("SOLARVER") );
			ByteString sPath2( Export::GetEnv("INPATH") );
			ByteString sPath3( "bin" );
			ByteString sPath4( Export::GetEnv("UPDMINOREXT") );
			ByteString sExecutable( sPath1 );
			sExecutable += sDel ;
			sExecutable += sPath2 ;
			sExecutable += sDel;
			sExecutable += sPath3 ;
			sExecutable += sPath4 ;
			sExecutable += sDel ;
			sExecutable += rExecutable ;


		ByteString sCommand( sExecutable );
		sCommand += " ";
		sCommand += rParameter;
		sCommand += " -p ";
		sCommand += sPrj;
		sCommand += " -r ";
		sCommand += sRoot;
		sCommand += " -i ";
		sCommand += sFileName;
		sCommand += " -o ";
		sCommand += sTempFile;
		if ( sLanguageRestriction.Len()) {
			sCommand += " -l ";
		sCommand += getSourceLanguages( sLanguageRestriction , sCommand );
		}

			//printf("DBG: %s\n",sCommand.GetBuffer());
			if (system(sCommand.GetBuffer()) == -1)
				fprintf(stderr, "%s failed\n", sCommand.GetBuffer());
			nFileCnt++;
			printf(".");
			fflush( stdout );

			SvFileStream aSDFIn( aTemp.GetFull(), STREAM_READ );
			ByteString sLine;
			while ( aSDFIn.IsOpen() && !aSDFIn.IsEof()) {
				aSDFIn.ReadLine( sLine );
				if ( sLine.Len()) {
					aSDF.WriteLine( sLine );
				}
			}
			aSDFIn.Close();

			aTemp.Kill();

		}
		// reset current working directory
		aOldCWD.SetCWD();
}

ByteString SourceTreeLocalizer::getSourceLanguages( ByteString sLanguageRestriction_inout , ByteString sCommand )
{
	// Source languages in helpcontent2 and macromigration en-US only!
	if( sCommand.Search("helpex") != STRING_NOTFOUND ) {
		sLanguageRestriction_inout.Assign( ByteString("en-US") );
	}
	else if( sCommand.Search("xmlex") != STRING_NOTFOUND ){
		sLanguageRestriction_inout.Assign( ByteString("en-US") );
	}
	return sLanguageRestriction_inout;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckNegativeList( const ByteString &rFileName )
/*****************************************************************************/
{
	sal_uLong nIndex = 0;
	sal_Bool bReturn = sal_True;

	ByteString sDelimiter(
		DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

	ByteString sFileName( rFileName );
	sFileName.ToLowerAscii();

	ByteString sNegative( NegativeList[ nIndex ] );
	while( !sNegative.Equals( "NULL" ) && bReturn ) {
		sNegative.SearchAndReplaceAll( "\\", sDelimiter );
		sNegative.SearchAndReplaceAll( "/", sDelimiter );
		sNegative.ToLowerAscii();

		if( sFileName.Search( sNegative ) == sFileName.Len() - sNegative.Len())
			bReturn = sal_False;

		nIndex++;
		sNegative = NegativeList[ nIndex ];
	}

	return bReturn;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::CheckPositiveList( const ByteString &rFileName )
/*****************************************************************************/
{
	sal_uLong nIndex = 0;
	sal_Bool bReturn = sal_False;

	ByteString sDelimiter(
		DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

	ByteString sFileName( rFileName );
	sFileName.ToLowerAscii();

	ByteString sNegative( PositiveList[ nIndex ] );
	while( !sNegative.Equals( "NULL" ) && !bReturn ) {
		sNegative.SearchAndReplaceAll( "\\", sDelimiter );
		sNegative.SearchAndReplaceAll( "/", sDelimiter );
		sNegative.ToLowerAscii();

		if( sFileName.Search( sNegative ) == sFileName.Len() - sNegative.Len())
			bReturn = sal_True;

		nIndex++;
		sNegative = PositiveList[ nIndex ];
	}

	return bReturn;
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnFileType(
	const ByteString &rDirectory, const ByteString &rExtension,
	const ByteString &rExecutable, const ByteString &rParameter,
	const ByteString &rCollectMode
)
/*****************************************************************************/
{
	String sWild( rDirectory, RTL_TEXTENCODING_ASCII_US );
	sWild += DirEntry::GetAccessDelimiter();
	sWild += String::CreateFromAscii( "*." );
	sWild += String( rExtension, RTL_TEXTENCODING_ASCII_US );

	DirEntry aEntry( sWild );
	Dir aDir( sWild, FSYS_KIND_FILE );

	for ( sal_uInt16 i = 0; i < aDir.Count(); i++ ) {
		DirEntry aFile( aDir[ i ] );
		ByteString sFile( aFile.GetFull(), RTL_TEXTENCODING_ASCII_US );

		sal_Bool bAllowed = sal_True;

		if ( rCollectMode.Equals( "negative" ))
			bAllowed = CheckNegativeList( sFile );
		else if ( rCollectMode.Equals( "positive" ))
			bAllowed = CheckPositiveList( sFile );

		if ( bAllowed )
			WorkOnFile( sFile, rExecutable, rParameter );
	}
}

/*****************************************************************************/
void SourceTreeLocalizer::WorkOnDirectory( const ByteString &rDirectory )
/*****************************************************************************/
{
	//printf("Working on Directory %s\n",rDirectory.GetBuffer());
	sal_uLong nIndex = 0;
	ByteString sExtension( ExeTable[ nIndex ][ 0 ] );
	ByteString sExecutable( ExeTable[ nIndex ][ 1 ] );
	ByteString sParameter( ExeTable[ nIndex ][ 2 ] );
	ByteString sCollectMode( ExeTable[ nIndex ][ 3 ] );

	while( !sExtension.Equals( "NULL" )) {
		WorkOnFileType(
			rDirectory,
			sExtension,
			sExecutable,
			sParameter,
			sCollectMode
		);

		nIndex++;

		sExtension = ExeTable[ nIndex ][ 0 ];
		sExecutable = ExeTable[ nIndex ][ 1 ];
		sParameter = ExeTable[ nIndex ][ 2 ];
		sCollectMode = ExeTable[ nIndex ][ 3 ];
	}
}

void SourceTreeLocalizer::OnExecuteDirectory( const rtl::OUString &aDirectory )
{
	ByteString rDirectory( rtl::OUStringToOString( aDirectory , RTL_TEXTENCODING_UTF8 , aDirectory.getLength() ) ) ;
	if ( nMode == LOCALIZE_NONE ){
	}
	else
		WorkOnDirectory( rDirectory );
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::Extract( const ByteString &rDestinationFile )
/*****************************************************************************/
{
	nMode = LOCALIZE_EXTRACT;

	aSDF.Open( String( rDestinationFile , RTL_TEXTENCODING_ASCII_US ) , STREAM_STD_WRITE );
	aSDF.SetLineDelimiter( LINEEND_CRLF );

	sal_Bool bReturn = aSDF.IsOpen();
	if ( bReturn ) {
		aSDF.Seek( STREAM_SEEK_TO_END );
		bReturn = StartExecute();
		aSDF.Close();
	}
	else{
		printf("ERROR: Can't create file %s\n", rDestinationFile.GetBuffer() );
	}
	nMode = LOCALIZE_NONE;
	aSDF.Close();
	return bReturn;
}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::MergeSingleFile(
	const ByteString &rPrj,
	const ByteString &rFile,
	const ByteString &rSDFFile
)
/*****************************************************************************/
{
	//printf("MergeSingleFile(%s,%s,%s)",rPrj.GetBuffer(),rFile.GetBuffer(),rSDFFile.GetBuffer());
	if ( !rFile.Len())
		return sal_True;

	ByteString sRoot( Export::GetEnv( "SRC_ROOT" ));
	DirEntry aEntry( String( sRoot, RTL_TEXTENCODING_ASCII_US ));
	aEntry += DirEntry( String( rPrj, RTL_TEXTENCODING_ASCII_US ));

	ByteString sDelimiter(
		DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );

	ByteString sCur( rFile );
	sCur.SearchAndReplaceAll( "\\", sDelimiter );
	sCur.SearchAndReplaceAll( "/", sDelimiter );

	aEntry += DirEntry( String( sCur, RTL_TEXTENCODING_ASCII_US ));
	ByteString sFile( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

	ByteString sBCur( aEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );

	sal_uLong nIndex = 0;
	ByteString sExtension( aEntry.GetExtension(), RTL_TEXTENCODING_ASCII_US );
	ByteString sCandidate( ExeTable[ nIndex ][ 0 ] );

	while( !sCandidate.Equals ("NULL") && !sCandidate.Equals(sExtension) )
		sCandidate = ExeTable[ ++nIndex ][ 0 ];

	if ( !sCandidate.Equals( "NULL" ) ) {
		if( !aEntry.Exists()) {
			DirEntryKind theDir=FSYS_KIND_FILE;
			Dir myDir( aEntry.GetPath(), theDir);
			DirEntry current;
			sal_Bool found=sal_False;
			for( sal_uInt16 x=0; x < myDir.Count() && !found;){
				current=myDir[x++];
				StringCompare result=current.GetName().CompareIgnoreCaseToAscii( aEntry.GetName() );
				if( result==COMPARE_EQUAL ){
					fprintf(stderr,"WARNING: %s not found\n", ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US).GetBuffer() );
					fprintf(stderr,"but use %s instead \n" , ByteString(current.GetFull(), RTL_TEXTENCODING_ASCII_US).GetBuffer() );
					aEntry=current;
					found=sal_True;
				}
			}
			if(!found)	return sal_True;

		}

		DirEntry aOut( Export::GetTempFile() );
		ByteString sOutput;
		if( sOutputFile.Len() == 0 )
			sOutput = ByteString ( aOut.GetFull(), RTL_TEXTENCODING_ASCII_US );
		else
			sOutput = sOutputFile;
		ByteString sCommand( ExeTable[ nIndex ][ 1 ] );
		sCommand += " -i ";
		sCommand += ByteString( aEntry.GetName(), RTL_TEXTENCODING_ASCII_US );
		sCommand += " -m ";
		sCommand += rSDFFile;
		sCommand += " -o ";
		sCommand += sOutput;
		sCommand += " ";
		sCommand += ByteString( ExeTable[ nIndex ][ 2 ] );
		if ( sLanguageRestriction.Len()) {
			sCommand += " -l ";
			sCommand += sLanguageRestriction;
		}

		DirEntry aPath( aEntry.GetPath());
		DirEntry aOldCWD;
		aPath.SetCWD();

		if (system(sCommand.GetBuffer()) == -1)
			fprintf(stderr, "%s failed\n", sCommand.GetBuffer());
		nFileCnt++;
		printf(".");
		SvFileStream aInStream( aOut.GetFull(), STREAM_READ );
		if ( !aInStream.IsOpen()) {
			fprintf( stderr,
				"ERROR: Unable to open file %s for reading!\n",
				sOutput.GetBuffer());
		}
		else {
			FileStat::SetReadOnlyFlag( aEntry, sal_False );
			String myStr2(aEntry.GetFull());
			String aTemp22 = String::CreateFromAscii("_tmp");
			myStr2.Append(aTemp22);

			ByteString test(myStr2,RTL_TEXTENCODING_ASCII_US);
			SvFileStream aOutStream( myStr2, STREAM_STD_WRITE | STREAM_TRUNC );
			if ( !aOutStream.IsOpen()) {
				ByteString test2(myStr2,RTL_TEXTENCODING_ASCII_US);
				fprintf( stderr,"ERROR: Unable to open file %s for modification!\n", test2.GetBuffer());
				aInStream.Close();
			}

			else {
				ByteString sLine;
				aOutStream.SetLineDelimiter( LINEEND_LF );

				aInStream.ReadLine( sLine );
				while ( !aInStream.IsEof()) {
					aOutStream.WriteLine( sLine );
					aInStream.ReadLine( sLine );
				}
				aInStream.Close();
				aOutStream.Close();


					DirEntry myTempFile(ByteString(myStr2,RTL_TEXTENCODING_ASCII_US));		// xxx_tmp ->
					DirEntry myFile(ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US));// xxx

					DirEntry oldFile(ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US));

					if(oldFile.Kill()==ERRCODE_NONE){
						if(myTempFile.MoveTo(myFile)!=ERRCODE_NONE){
							fprintf( stderr, "ERROR: Can't rename file %s\n",ByteString(myStr2,RTL_TEXTENCODING_ASCII_US).GetBuffer());
						}
					}
					else{
						fprintf( stderr, "ERROR: Can't remove file %s\n",ByteString(aEntry.GetFull(),RTL_TEXTENCODING_ASCII_US).GetBuffer());
					}
				} // else

				aOldCWD.SetCWD();
				aOut.Kill();
			}	// else
		}
	return sal_True;
}
/*****************************************************************************/
sal_Bool SourceTreeLocalizer::ExecuteMerge( )
/*****************************************************************************/
{
	DirEntry aEntry( Export::GetTempFile());
	sal_Bool bReturn = sal_True;
	bool bMerged = false;

	ByteString sFileName;
	ByteString sCurFile;
	ByteString sLine;
	ByteString sFileKey;

	SvFileStream aFile;

	ByteString sOutputFileName = sOutputFile;
	ByteString sInpath(".");
	sInpath += Export::GetEnv("INPATH");
	ByteString sBlank("");

	sOutputFileName.SearchAndReplaceAll( sInpath , sBlank );

	String sDel = DirEntry::GetAccessDelimiter();
	ByteString sBDel( sDel.GetBuffer() , sDel.Len() , RTL_TEXTENCODING_UTF8 );
	if( bLocal ){
		xub_StrLen nPos = sOutputFileName.SearchBackward( sBDel.GetChar(0) );
		sOutputFileName = sOutputFileName.Copy( nPos+1 , sOutputFileName.Len()-nPos-1 );
	}
	ByteStringBoolHashMap aFileHM;
	// Read all possible files
	while ( !aSDF.IsEof()) {
		aSDF.ReadLine( sLine );
		sFileName = sLine.GetToken( 0, '\t' );
		sFileName += "#";
		sFileName += sLine.GetToken( 1, '\t' );
		aFileHM[sFileName]=true;
	}

	for( ByteStringBoolHashMap::iterator iter = aFileHM.begin(); iter != aFileHM.end(); ++iter ){
		sFileKey = iter->first;
		aSDF.Seek( 0 );
		aFile.Open( aEntry.GetFull(), STREAM_STD_WRITE |STREAM_TRUNC );

		while ( !aSDF.IsEof()) {
			aSDF.ReadLine( sLine );
			sFileName = sLine.GetToken( 0, '\t' );
			sFileName += "#";
			sFileName += sLine.GetToken( 1, '\t' );
			if( sFileName.Len() && ( sFileName.CompareTo(sFileKey) == COMPARE_EQUAL ) ){
				if ( aFile.IsOpen() && sLine.Len())
					aFile.WriteLine( sLine );
			}
		}
		if ( aFile.IsOpen())
			aFile.Close();

		ByteString sPrj( sFileKey.GetToken( 0, '#' ));
		ByteString sFile( sFileKey.GetToken( 1, '#' ));
		ByteString sSDFFile( aFile.GetFileName(), RTL_TEXTENCODING_ASCII_US );

		//printf("localize test sPrj = %s , sFile = %s , sSDFFile = %s sOutputFileName = %s\n",sPrj.GetBuffer(), sFile.GetBuffer() , sSDFFile.GetBuffer() , sOutputFileName.GetBuffer() );

		// Test
		bLocal = true;
		// Test

		if( bLocal ){
			sal_uInt16 nPos = sFile.SearchBackward( '\\' );
			ByteString sTmp = sFile.Copy( nPos+1 , sFile.Len()-nPos-1 );
			//printf("'%s'='%s'\n",sTmp.GetBuffer(), sOutputFileName.GetBuffer());
			if( sTmp.CompareTo(sOutputFileName) == COMPARE_EQUAL ){
					bMerged = true;
					if ( !MergeSingleFile( sPrj, sFile, sSDFFile ))
						bReturn = sal_False;
			}else{
				bMerged = true;
				//printf("MergeSingleFile('%s','%s','%s')\n",sPrj.GetBuffer(),sFile.GetBuffer(),sSDFFile.GetBuffer());
				if ( !MergeSingleFile( sPrj, sFile, sSDFFile ))
					bReturn = sal_False;
			}
		}
	}
	aEntry.Kill();
	// If Outputfile not included in the SDF file copy it without merge

	if( bLocal && !bMerged ){
		DirEntry aSourceFile( sOutputFileName.GetBuffer() );
		FSysError aErr = aSourceFile.CopyTo( DirEntry ( sOutputFile.GetBuffer() ) , FSYS_ACTION_COPYFILE );
		if( aErr != FSYS_ERR_OK ){
			printf("ERROR: Can't copy file '%s' to '%s' %d\n",sOutputFileName.GetBuffer(),sOutputFile.GetBuffer(),sal::static_int_cast<int>(aErr));
		}
	}
	return bReturn;

}

/*****************************************************************************/
sal_Bool SourceTreeLocalizer::Merge( const ByteString &rSourceFile , const ByteString &rOutput )
/*****************************************************************************/
{
	sOutputFile = rOutput;
	nMode = LOCALIZE_MERGE;
	aSDF.Open( String( rSourceFile, RTL_TEXTENCODING_ASCII_US ),
		STREAM_STD_READ );

	sal_Bool bReturn = aSDF.IsOpen();
	if ( bReturn ) {
		bReturn = ExecuteMerge();
//		aSDF.Close();
	}
	aSDF.Close();
	nMode = LOCALIZE_NONE;
	return bReturn;
}

}
using namespace transex3;

#define STATE_NONE		0x0000
#define STATE_EXPORT	0x0001
#define STATE_MERGE		0x0002
#define STATE_ISOCODE	0x0003
#define STATE_LANGUAGES	0x0004
#define STATE_FILENAME	0x0005
#define STATE_OUTPUT	0x0006

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
	fprintf( stdout,
		"localize (c)2001 by Sun Microsystems\n"
		"====================================\n" );
	fprintf( stdout,
		"As part of the L10N framework, localize extracts and merges translations\n"
		"out of and into the whole source tree.\n\n"
		"Syntax: localize -e -l en-US -f FileName\n"
		"Parameter:\n"
		"\t-e: Extract mode\n"
		"\tFileName: Output file when extract mode, input file when merge mode\n"
		"\tl1...ln: supported languages (\"all\" for all languages).\n"
	);

	fprintf( stdout,
		"Valid language codes for l1...ln and f1...fn are:\n" );
	fprintf( stdout,
		"\nExample 1:\n"
		"==========\n"
		"localize -e -l en-US -f MyFile\n\n"
		"All strings will be extracted for language de and language en-US.\n"
	);
}

/*****************************************************************************/
int Error()
/*****************************************************************************/
{
	Help();
	return 1;
}

/*****************************************************************************/
sal_Bool CheckLanguages( ByteString &rLanguages )
/*****************************************************************************/
{
	ByteString sTmp( rLanguages );
	return true;
}

/*****************************************************************************/
#if defined(UNX) || defined(OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
	String sTempBase( String::CreateFromAscii( "loc" ));
	DirEntry::SetTempNameBase( sTempBase );
	sal_uInt16 nState = STATE_NONE;

	sal_Bool bExport = sal_False;
	sal_Bool bMerge = sal_False;
	bool bSkipLinks = false;

	ByteString sLanguages;
	ByteString sFileName;
	ByteString sOutput;

	bExport = sal_True;

	for( int i = 1; i < argc; i++ ) {
		ByteString sSwitch( argv[ i ] );
		sSwitch.ToUpperAscii();

		if ( sSwitch.Equals( "-E" )) {
			nState = STATE_EXPORT;
			if ( bMerge )
				return Error();
			bExport = sal_True;
		}
		else if ( sSwitch.Equals( "-I" ) )
			nState = STATE_ISOCODE;
		else if ( sSwitch.Equals( "-L" ) )
			nState = STATE_LANGUAGES;
		else if ( sSwitch.Equals( "-F" ) )
			nState = STATE_FILENAME;
		else if ( ByteString( argv[ i ]).ToUpperAscii().Equals( "-O" ) )
			nState = STATE_OUTPUT;
		else {
			switch ( nState ) {
				case STATE_NONE:
					return Error();
				case STATE_OUTPUT:
					if ( sOutput.Len())
						return Error();
					sOutput = ByteString( argv[ i ] );
					nState = STATE_NONE;
				break;
				case STATE_LANGUAGES:
					if ( sLanguages.Len())
						return Error();
					sLanguages = ByteString( argv[ i ] );
					nState = STATE_NONE;
				break;
				case STATE_FILENAME:
					if ( sFileName.Len())
						return Error();
					sFileName = ByteString( argv[ i ] );
					nState = STATE_NONE;
				break;
				default:
					return Error();
			}
		}
	}
	if ( !bMerge && !bExport ) {
		Help();
		return 1;
	}

	ByteString sSolarVer( Export::GetEnv( "WORK_STAMP" ));
	ByteString sVersion( Export::GetEnv( "WORK_STAMP" ));

	if ( !sSolarVer.Len() || !sVersion.Len()) {
		fprintf( stderr, "ERROR: No environment set!\n" );
		return 1;
	}

	if ( !CheckLanguages( sLanguages ))
		return 2;

	if ( !sFileName.Len()) {
		fprintf( stderr, "ERROR: No filename given\n" );
		return 3;
	}

	DirEntry aEntry( String( sFileName , RTL_TEXTENCODING_ASCII_US ));
	aEntry.ToAbs();
	String sFullEntry = aEntry.GetFull();
	ByteString sFileABS( aEntry.GetFull(), gsl_getSystemTextEncoding());
	//printf("B %s\nA %s\n",rDestinationFile.GetBuffer(), sFile.GetBuffer());
	sFileName = sFileABS;

	Treeconfig treeconfig;
	vector<string> repos;
	bool hasPwd = treeconfig.getActiveRepositories( repos );
	if( hasPwd ) cout << "\nFound special path!\n";

	string minor_ext;
	bool has_minor_ext;

	if( Export::GetEnv("UPDMINOREXT") != NULL )
	{
		minor_ext     = string( Export::GetEnv("UPDMINOREXT") );
		has_minor_ext = minor_ext.size();
	}
	else
		has_minor_ext = false;

	// localize through all repositories
	for( vector<string>::iterator iter = repos.begin(); iter != repos.end() ; ++iter )
	{
		string curRepository;
		if( has_minor_ext )
			curRepository = string( Export::GetEnv("SOURCE_ROOT_DIR") ) + "/" + *iter + minor_ext;
		else
			curRepository = string( Export::GetEnv("SOURCE_ROOT_DIR") ) + "/" + *iter;
		cout << "Localizing repository " << curRepository << "\n";
		SourceTreeLocalizer aIter( ByteString( curRepository.c_str() ) , sVersion , (sOutput.Len() > 0) , bSkipLinks );
		aIter.SetLanguageRestriction( sLanguages );
		if ( bExport ){
			fflush( stdout );
			if( *iter == "ooo" )
				aIter.Extract( sFileName );
			else
			{
				ByteString sFileNameWithExt( sFileName );
				sFileNameWithExt += ByteString( "." );
				sFileNameWithExt += ByteString( (*iter).c_str() );
				aIter.Extract( sFileNameWithExt );
			}
			printf("\n%d files found!\n",aIter.GetFileCnt());
		}
	}
	if( hasPwd )
	{
		string pwd;
		Export::getCurrentDir( pwd );
		cout << "Localizing repository " << pwd << "\n";
		SourceTreeLocalizer aIter( ByteString( pwd.c_str() ) , sVersion , (sOutput.Len() > 0) , bSkipLinks );
		aIter.SetLanguageRestriction( sLanguages );
		if ( bExport ){
			fflush( stdout );
			aIter.Extract( sFileName );
			printf("\n%d files found!\n",aIter.GetFileCnt());
		}

	}

	return 0;
}
