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
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlmetae.hxx>
#include <editeng/langitem.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "docstat.hxx"
#include "docsh.hxx"
#include <doc.hxx>
#include "xmlimp.hxx"
#include "xmlexp.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

// ---------------------------------------------------------------------

uno::Reference<document::XDocumentProperties>
SwXMLImport::GetDocumentProperties() const
{
    if (IsOrganizerMode() || IsStylesOnlyMode() ||
        IsBlockMode() || IsInsertMode())
    {
        return 0;
    }
    uno::Reference<document::XDocumentPropertiesSupplier> const xDPS(
        GetModel(), UNO_QUERY_THROW);
    return xDPS->getDocumentProperties();
}

SvXMLImportContext *SwXMLImport::CreateMetaContext(
									   const OUString& rLocalName )
{
	SvXMLImportContext *pContext = 0;

    if (getImportFlags() & IMPORT_META)
    {
        uno::Reference<xml::sax::XDocumentHandler> const xDocBuilder(
            mxServiceFactory->createInstance(::rtl::OUString::createFromAscii(
                "com.sun.star.xml.dom.SAXDocumentBuilder")),
            uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> const xDocProps(
                GetDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this,
                    XML_NAMESPACE_OFFICE, rLocalName, xDocProps, xDocBuilder);
    }

	if( !pContext )
		pContext = new SvXMLImportContext( *this,
                        XML_NAMESPACE_OFFICE, rLocalName );

	return pContext;
}

// ---------------------------------------------------------------------

enum SvXMLTokenMapAttrs
{
	XML_TOK_META_STAT_TABLE = 1,
	XML_TOK_META_STAT_IMAGE = 2,
	XML_TOK_META_STAT_OLE = 4,
	XML_TOK_META_STAT_PAGE = 8,
	XML_TOK_META_STAT_PARA = 16,
	XML_TOK_META_STAT_WORD = 32,
	XML_TOK_META_STAT_CHAR = 64,
	XML_TOK_META_STAT_END=XML_TOK_UNKNOWN
};

/*
static __FAR_DATA SvXMLTokenMapEntry aMetaStatAttrTokenMap[] =
{
	{ XML_NAMESPACE_META, XML_TABLE_COUNT,      XML_TOK_META_STAT_TABLE	},
	{ XML_NAMESPACE_META, XML_IMAGE_COUNT,      XML_TOK_META_STAT_IMAGE	},
	{ XML_NAMESPACE_META, XML_OBJECT_COUNT,     XML_TOK_META_STAT_OLE	},
	{ XML_NAMESPACE_META, XML_PARAGRAPH_COUNT,  XML_TOK_META_STAT_PARA	},
	{ XML_NAMESPACE_META, XML_PAGE_COUNT,       XML_TOK_META_STAT_PAGE	},
	{ XML_NAMESPACE_META, XML_WORD_COUNT,       XML_TOK_META_STAT_WORD	},
	{ XML_NAMESPACE_META, XML_CHARACTER_COUNT,  XML_TOK_META_STAT_CHAR	},
	XML_TOKEN_MAP_END
};
*/

struct statistic {
    SvXMLTokenMapAttrs token;
    const char* name;
    sal_uInt16 SwDocStat::* target16;
    sal_uLong  SwDocStat::* target32; /* or 64, on LP64 platforms */
};

static const struct statistic s_stats [] = {
    { XML_TOK_META_STAT_TABLE, "TableCount",     &SwDocStat::nTbl, 0  },
    { XML_TOK_META_STAT_IMAGE, "ImageCount",     &SwDocStat::nGrf, 0  },
    { XML_TOK_META_STAT_OLE,   "ObjectCount",    &SwDocStat::nOLE, 0  },
    { XML_TOK_META_STAT_PAGE,  "PageCount",      0, &SwDocStat::nPage },
    { XML_TOK_META_STAT_PARA,  "ParagraphCount", 0, &SwDocStat::nPara },
    { XML_TOK_META_STAT_WORD,  "WordCount",      0, &SwDocStat::nWord },
    { XML_TOK_META_STAT_CHAR,  "CharacterCount", 0, &SwDocStat::nChar },
    { XML_TOK_META_STAT_END,   0,                0, 0                 }
};

void SwXMLImport::SetStatistics(
        const Sequence< beans::NamedValue > & i_rStats)
{
	if( IsStylesOnlyMode() || IsInsertMode() )
		return;

    SvXMLImport::SetStatistics(i_rStats);

	SwDoc *pDoc = SwImport::GetDocFromXMLImport( *this );
	SwDocStat aDocStat( pDoc->GetDocStat() );

	sal_uInt32 nTokens = 0;

    for (sal_Int32 i = 0; i < i_rStats.getLength(); ++i) {
        for (struct statistic const* pStat = s_stats; pStat->name != 0;
                ++pStat) {
            if (i_rStats[i].Name.equalsAscii(pStat->name)) {
                sal_Int32 val = 0;
                if (i_rStats[i].Value >>= val) {
                    if (pStat->target16 != 0) {
                        aDocStat.*(pStat->target16)
                            = static_cast<sal_uInt16> (val);
                    } else {
                        aDocStat.*(pStat->target32)
                            = static_cast<sal_uInt32> (val);
                    }
                    nTokens |= pStat->token;
                } else {
                    DBG_ERROR("SwXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

	if( 127 == nTokens )
		aDocStat.bModified = sal_False;
	if( nTokens )
		pDoc->SetDocStat( aDocStat );

	// set progress bar reference to #paragraphs. If not available,
	// use #pages*10, or guesstimate 250 paragraphs. Additionally
	// guesstimate PROGRESS_BAR_STEPS each for meta+settings, styles,
	// and autostyles.
	sal_Int32 nProgressReference = 250;
	if( nTokens & XML_TOK_META_STAT_PARA )
		nProgressReference = (sal_Int32)aDocStat.nPara;
	else if ( nTokens & XML_TOK_META_STAT_PAGE )
		nProgressReference = 10 * (sal_Int32)aDocStat.nPage;
	ProgressBarHelper* pProgress = GetProgressBarHelper();
	pProgress->SetReference( nProgressReference + 3*PROGRESS_BAR_STEP );
	pProgress->SetValue( 0 );
}

// ---------------------------------------------------------------------

void SwXMLExport::_ExportMeta()
{
	SvXMLExport::_ExportMeta();

	if( !IsBlockMode() )
	{

		if( IsShowProgress() )
		{
			ProgressBarHelper *pProgress = GetProgressBarHelper();
			pProgress->SetValue( pProgress->GetValue() + 2 );
		}
	}
}

