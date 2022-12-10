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
#include "precompiled_tools.hxx"

#include <stdio.h>

#include <../../inc/tools/string.hxx>
#include <../../inc/tools/list.hxx>

class TextFilter
{
protected:
	FILE			*pIn, *pOut;
	virtual void	Filter();
public:
					TextFilter( ByteString aInFile = "stdin", 
						ByteString aOutFile = "stdout" );
	virtual			~TextFilter();

	virtual void	Execute();
};

TextFilter::TextFilter( ByteString aInFile, ByteString aOutFile )
{
	if ( aInFile == "stdin" )
		pIn = stdin;
	else
		if (( pIn = fopen( aInFile.GetBuffer(), "r" )) == NULL )
			printf( "Can't read %s\n", aInFile.GetBuffer() );

	if ( aOutFile == "stdout" )
		pOut = stdout;
	else
		if (( pOut = fopen( aOutFile.GetBuffer(), "w" )) == NULL )
			printf( "Can't write %s\n", aOutFile.GetBuffer() );
}

TextFilter::~TextFilter()
{
	fclose( pOut );
	fclose( pIn );
}

void TextFilter::Execute()
{
	Filter();
}

void TextFilter::Filter()
{
	int c;
	while ( (c = fgetc( pIn )) != EOF )
		fputc( c, pOut );
}

#define LINE_LEN 2048

class ByteStringList;

class MkLine
{
public:
	ByteString			aLine;
	ByteStringList*		pPrivateTnrLst;
	sal_Bool			bOut;
	sal_Bool			bHier;
		
					MkLine();
};

MkLine::MkLine()
{
	bOut = sal_False;
	bHier = sal_False;
	pPrivateTnrLst = NULL;
}

DECLARE_LIST( ByteStringList, MkLine * )

class MkFilter : public TextFilter
{
	static ByteString	aTnr;
	ByteStringList		*pLst;
	ByteStringList		*pTnrLst;
protected:
	virtual void	Filter();
public:
					MkFilter( ByteString aInFile = "stdin", ByteString aOutFile = "stdout");
					~MkFilter();
};

MkFilter::MkFilter( ByteString aInFile, ByteString aOutFile ) :
	TextFilter( aInFile, aOutFile )
{
	pLst = new ByteStringList;
	pTnrLst = new ByteStringList;
}

MkFilter::~MkFilter()
{
	delete pTnrLst;
	delete pLst;
}

ByteString MkFilter::aTnr="$(TNR)";

void MkFilter::Filter()
{
	char aLineBuf[LINE_LEN];
	int nState = 0;

	while(( fgets(aLineBuf, LINE_LEN, pIn)) != NULL )
	{
		ByteString aLine( aLineBuf );
		//fprintf(stderr, "aLine :%s\n", aLine.GetBuffer());
		if ( aLine.Search("mkfilter1" ) != STRING_NOTFOUND )
		{
			// Zeilen unterdruecken 
			fprintf( stderr, "mkfilter1\n" );
			nState = 0;
		}
		else if ( aLine.Search("unroll begin" ) != STRING_NOTFOUND )
		{
			// Zeilen raus schreiben mit ersetzen von $(TNR) nach int n
			fprintf( stderr, "\nunroll begin\n" );
			nState = 1;
		}
		;
		
		if ( nState == 0  )
		{
			fprintf( stderr, "." );
			MkLine *pMkLine = new MkLine();
			ByteString *pStr = new ByteString( aLineBuf );
			pMkLine->aLine = *pStr;
			pMkLine->bOut = sal_False;
		
			pLst->Insert( pMkLine, LIST_APPEND );
		}
		else if ( nState == 1 )
		{
			sal_Bool bInTnrList = sal_True;
			fprintf( stderr, ":" );
			MkLine *pMkLine = new MkLine();
			if ( aLine.Search("unroll end") != STRING_NOTFOUND )
			{
				fprintf( stderr, ";\nunroll end\n" );
				MkLine *p_MkLine = new MkLine();
				p_MkLine->bHier = sal_True;
				ByteString *pByteString = new ByteString("# do not delete this line === mkfilter3i\n");
				p_MkLine->aLine = *pByteString;
				p_MkLine->bOut = sal_False;
				p_MkLine->pPrivateTnrLst = pTnrLst;
				pTnrLst = new ByteStringList();
				pLst->Insert( p_MkLine, LIST_APPEND );
				nState = 0;
				bInTnrList = sal_False;
			}
			ByteString *pStr = new ByteString( aLineBuf );
			pMkLine->aLine = *pStr;
			pMkLine->bOut = sal_False;
		
			if ( bInTnrList )
				pTnrLst->Insert( pMkLine, LIST_APPEND );
		}
		else {
			/* Zeilen ignorieren */;
        }
	}	// End Of File
	fprintf( stderr, "\n" );
	
	// das File wieder ausgegeben
	sal_uIntPtr nLines = pLst->Count();
	for ( sal_uIntPtr j=0; j<nLines; j++ )
	{
		MkLine *pLine = pLst->GetObject( j );
		if ( pLine->bHier )
		{
			// die List n - Mal abarbeiten
			for ( sal_uInt16 n=1; n<11; n++)
			{
				sal_uIntPtr nCount = pLine->pPrivateTnrLst->Count();
				for ( sal_uIntPtr i=0; i<nCount; i++ )
				{
					MkLine *pMkLine = pLine->pPrivateTnrLst->GetObject(i);
					ByteString aLine = pMkLine->aLine;
					while( aLine.SearchAndReplace( aTnr, ByteString::CreateFromInt32( n )) != (sal_uInt16)-1 ) ;
					fputs( aLine.GetBuffer(), pOut );
					fprintf( stderr, "o" );
				}
			}
			if ( pLine->pPrivateTnrLst != NULL )
				delete pLine->pPrivateTnrLst;
			pLine->pPrivateTnrLst = NULL;
		}
		if ( pLine->bOut )
				fputs(pLine->aLine.GetBuffer(), pOut );			
	}
	fprintf( stderr, "\n" );
}

int main()
{
	int nRet = 0;

	TextFilter *pFlt = new MkFilter();
	pFlt->Execute();
	delete pFlt;

	return nRet;
}
