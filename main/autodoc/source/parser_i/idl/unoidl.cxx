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

#include <precomp.h>
#include <parser/unoidl.hxx>


// NOT FULLY DECLARED SERVICES
#include <stdlib.h>
#include <cosv/file.hxx>
#include <ary/ary.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <../parser/inc/x_docu.hxx>
#include <parser/parserinfo.hxx>
#include <tools/filecoll.hxx>
#include <tools/tkpchars.hxx>
#include <s2_luidl/tkp_uidl.hxx>
#include <s2_luidl/distrib.hxx>
#include <s2_luidl/pe_file2.hxx>
#include <s2_dsapi/cx_dsapi.hxx>
#include <adc_msg.hxx>
#include <x_parse2.hxx>



namespace autodoc
{


class FileParsePerformers
{
  public:
						FileParsePerformers(
						    ary::Repository &
						                        io_rRepository,
						    ParserInfo &        io_rParserInfo );

	void				ParseFile(
							const char *		i_sFullPath );

    void                ConnectLinks();

  private:
	CharacterSource		aFileLoader;
	Dyn<csi::uidl::TokenParser_Uidl>
						pTokens;
	csi::uidl::TokenDistributor
                        aDistributor;
	Dyn<csi::uidl::PE_File>
                        pFileParseEnvironment;
	ary::Repository &
                        rRepository;
    ParserInfo &        rParserInfo;
};


IdlParser::IdlParser( ary::Repository & io_rRepository )
	:	pRepository(&io_rRepository)
{
}

void
IdlParser::Run( const autodoc::FileCollector_Ifc & i_rFiles )
{
	Dyn<FileParsePerformers>
		pFileParsePerformers(
		    new FileParsePerformers(*pRepository,
									static_cast< ParserInfo& >(*this)) );

	FileCollector::const_iterator iEnd = i_rFiles.End();
	for ( FileCollector::const_iterator iter = i_rFiles.Begin();
		  iter != iEnd;
		  ++iter )
	{
		Cout() << (*iter) << " ..."<< Endl();

        try
        {
    		pFileParsePerformers->ParseFile(*iter);
        }
        catch (X_AutodocParser &)
        {
            /// Ignore and goon
            TheMessages().Out_ParseError(CurFile(), CurLine());
    		pFileParsePerformers
    		    = new FileParsePerformers(*pRepository,
                                          static_cast< ParserInfo& >(*this));
        }
        catch (X_Docu & xd)
        {
            // Currently thic catches only wrong since tags, while since tags are
            // transformed. In this case the program shall be terminated.
            Cerr() << xd << Endl();
            exit(1);
        }
        catch (...)
        {
            Cout() << "Unknown error." << Endl();
            exit(0);
//    		pFileParsePerformers = new FileParsePerformers( *pRepository );
        }
	}

    pFileParsePerformers->ConnectLinks();
}

FileParsePerformers::FileParsePerformers( ary::Repository & io_rRepository,
										  ParserInfo &			 io_rParserInfo )
	:   pTokens(0),
	    aDistributor(io_rRepository, io_rParserInfo),
		rRepository( io_rRepository ),
		rParserInfo(io_rParserInfo)
{
	DYN csi::dsapi::Context_Docu *
	    dpDocuContext
			= new csi::dsapi::Context_Docu( aDistributor.DocuTokens_Receiver() );
	pTokens = new csi::uidl::TokenParser_Uidl( aDistributor.CodeTokens_Receiver(), *dpDocuContext );
	pFileParseEnvironment
			= new csi::uidl::PE_File(aDistributor,rParserInfo);

	aDistributor.SetTokenProvider(*pTokens);
	aDistributor.SetTopParseEnvironment(*pFileParseEnvironment);
}

void
FileParsePerformers::ParseFile( const char * i_sFullPath )
{
	csv::File aFile(i_sFullPath);

	aFile.open( csv::CFM_READ );
	csv_assert( aFile.is_open() );
	aFileLoader.LoadText(aFile);
	aFile.close();

    rParserInfo.Set_CurFile(i_sFullPath, true); // true = count lines
	pTokens->Start(aFileLoader);
    aDistributor.Reset();

	do {
		aDistributor.TradeToken();
	} while ( NOT aFileLoader.IsFinished() );
}

void
FileParsePerformers::ConnectLinks()
{
    // KORR_FUTURE ?
//  rRepository.RwGate_Idl().ConnectAdditionalLinks();
}

}   // namespace autodoc
