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



#ifndef _SV_SALPRN_HXX
#define _SV_SALPRN_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/prntypes.hxx>

#include <vector>

class SalGraphics;
class SalFrame;
struct ImplJobSetup;
namespace vcl { class PrinterController; }

// -----------------------
// - SalPrinterQueueInfo -
// -----------------------

struct VCL_PLUGIN_PUBLIC SalPrinterQueueInfo
{
	XubString				maPrinterName;
	XubString				maDriver;
	XubString				maLocation;
	XubString				maComment;
	sal_uLong					mnStatus;
	sal_uLong					mnJobs;
	void*					mpSysData;

							SalPrinterQueueInfo();
							~SalPrinterQueueInfo();
};

// ------------------
// - SalInfoPrinter -
// ------------------

class VCL_PLUGIN_PUBLIC SalInfoPrinter
{
public:
    std::vector< PaperInfo  >			m_aPaperFormats;	// all printer supported formats
    bool								m_bPapersInit;		// set to true after InitPaperFormats
    bool                                m_bCompatMetrics;

    SalInfoPrinter() : m_bPapersInit( false ), m_bCompatMetrics( false ) {}
    virtual ~SalInfoPrinter();

	// SalGraphics or NULL, but two Graphics for all SalFrames
	// must be returned
	virtual SalGraphics*			GetGraphics() = 0;
	virtual void					ReleaseGraphics( SalGraphics* pGraphics ) = 0;

	virtual sal_Bool					Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) = 0;
    // This function set the driver data and
    // set the new indepen data in pSetupData
	virtual sal_Bool					SetPrinterData( ImplJobSetup* pSetupData ) = 0;
    // This function merged the indepen driver data
    // and set the new indepen data in pSetupData
    // Only the data must changed, where the bit
    // in nFlags is set
	virtual sal_Bool					SetData( sal_uLong nFlags, ImplJobSetup* pSetupData ) = 0;

	virtual void					GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) = 0;
	virtual sal_uLong					GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType ) = 0;
	virtual sal_uLong					GetPaperBinCount( const ImplJobSetup* pSetupData ) = 0;
	virtual String					GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin ) = 0;
	// fills m_aPaperFormats and sets m_bPapersInit to true
    virtual void					InitPaperFormats( const ImplJobSetup* pSetupData ) = 0;
    // returns angle that a landscape page will be turned counterclockwise wrt to portrait
    virtual int					GetLandscapeAngle( const ImplJobSetup* pSetupData ) = 0;
};

// --------------
// - SalPrinter -
// --------------

class VCL_PLUGIN_PUBLIC SalPrinter
{
public: 					// public for Sal Implementation
    SalPrinter() {}
    virtual ~SalPrinter();

	virtual sal_Bool					StartJob( const String* pFileName,
                                              const String& rJobName,
                                              const String& rAppName,
                                              sal_uLong nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData ) = 0;

    // implement for pull model print systems only,
    // default implementations (see salvtables.cxx) just returns sal_False
    virtual sal_Bool                    StartJob( const String* pFileName,
                                              const String& rJobName,
                                              const String& rAppName,
                                              ImplJobSetup* pSetupData,
                                              vcl::PrinterController& rController );

	virtual sal_Bool					EndJob() = 0;
	virtual sal_Bool					AbortJob() = 0;
	virtual SalGraphics*			StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData ) = 0;
	virtual sal_Bool					EndPage() = 0;
	virtual sal_uLong					GetErrorCode() = 0;
    
};

#endif // _SV_SALPRN_HXX
