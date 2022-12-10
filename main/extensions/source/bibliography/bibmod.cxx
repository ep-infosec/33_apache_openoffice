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
#include "precompiled_extensions.hxx"


#include <tools/resmgr.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/util/XLocalizedAliases.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <tools/debug.hxx>

#include "bibmod.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
static PtrBibModul pBibModul=NULL;
static sal_uInt32 nBibModulCount=0;
#include <ucbhelper/content.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

HdlBibModul	OpenBibModul()
{
	if(pBibModul==NULL)
	{
		pBibModul=new BibModul();
	}
	nBibModulCount++;
	return &pBibModul;
}

void CloseBibModul(HdlBibModul ppBibModul)
{
	nBibModulCount--;
	if(nBibModulCount==0 && ppBibModul!=NULL)
	{
		delete pBibModul;
		pBibModul=NULL;
	}
}

BibResId::BibResId( sal_uInt16 nId ) :
	ResId( nId, *pBibModul->GetResMgr() )
{
}
BibConfig* BibModul::pBibConfig = 0;
BibModul::BibModul()
{
	pResMgr = ResMgr::CreateResMgr( "bib" );
}

BibModul::~BibModul()
{
	delete pResMgr;
	delete pBibConfig;
	pBibConfig = 0;
}

BibDataManager*	 BibModul::createDataManager()
{
	return new BibDataManager();
}
//-----------------------------------------------------------------------------
BibConfig*	BibModul::GetConfig()
{
	if(! pBibConfig)
		pBibConfig = new BibConfig;
	return pBibConfig;
}


// PropertyNames
#define STATIC_USTRING(a,b) rtl::OUString a(b)
STATIC_USTRING(FM_PROP_LABEL,C2U("Label"));
STATIC_USTRING(FM_PROP_CONTROLSOURCE,C2U("DataField"));
STATIC_USTRING(FM_PROP_NAME,C2U("Name"));
STATIC_USTRING(FM_PROP_FORMATKEY,C2U("FormatKey"));
#ifdef TF_SDBAPI
#else // !TF_SDBAPI
STATIC_USTRING(FM_PROP_EDITMODE,C2U("RecordMode"));
STATIC_USTRING(FM_PROP_CURSORSOURCETYPE,C2U("DataSelectionType"));
STATIC_USTRING(FM_PROP_CURSORSOURCE,C2U("DataSelection"));
STATIC_USTRING(FM_PROP_DATASOURCE, C2U("DataSource"));
#endif // !TF_SDBAPI
STATIC_USTRING(FM_PROP_VALUE,C2U("Value"));
STATIC_USTRING(FM_PROP_TEXT,C2U("Text"));
