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
#include "precompiled_sfx2.hxx"

#ifndef _INETDEF_HXX
#include <svl/inetdef.hxx>
#endif
#include "svtools/htmlkywd.hxx"

//!(dv) #include <chaos2/cntapi.hxx>
#include <rtl/tencinfo.h>

#include <unotools/configmgr.hxx>
#include "svl/urihelper.hxx"
#include <tools/datetime.hxx>

#include <sfx2/frmhtmlw.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/sfx.hrc>
#include "bastyp.hrc"

#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


// -----------------------------------------------------------------------

using namespace ::com::sun::star;

static sal_Char __READONLY_DATA sHTML_SC_yes[] =	"YES";
static sal_Char __READONLY_DATA sHTML_SC_no[] =		"NO";
static sal_Char __READONLY_DATA sHTML_SC_auto[] =	"AUTO";
static sal_Char __READONLY_DATA sHTML_MIME_text_html[] =	"text/html; charset=";

/* not used anymore?
static HTMLOutEvent __FAR_DATA aFrameSetEventTable[] =
{
	{ sHTML_O_SDonload,		sHTML_O_onload,		SFX_EVENT_OPENDOC	},
	{ sHTML_O_SDonunload,	sHTML_O_onunload,	SFX_EVENT_PREPARECLOSEDOC	},
	{ sHTML_O_SDonfocus,	sHTML_O_onfocus,	SFX_EVENT_ACTIVATEDOC	},
	{ sHTML_O_SDonblur,		sHTML_O_onblur,		SFX_EVENT_DEACTIVATEDOC	},
	{ 0,					0,				  	0					}
};
*/

#if defined(UNX)
const sal_Char SfxFrameHTMLWriter::sNewLine[] = "\012";
#else
const sal_Char __FAR_DATA SfxFrameHTMLWriter::sNewLine[] = "\015\012";
#endif

void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
								  const sal_Char *pIndent,
								  const String& rName,
								  const String& rContent, sal_Bool bHTTPEquiv,
							   	  rtl_TextEncoding eDestEnc,
								  String *pNonConvertableChars	)
{
	rStrm << sNewLine;
	if( pIndent )
		rStrm << pIndent;

	ByteString sOut( '<' );
	(((sOut += OOO_STRING_SVTOOLS_HTML_meta) += ' ')
		+= (bHTTPEquiv ? OOO_STRING_SVTOOLS_HTML_O_httpequiv : OOO_STRING_SVTOOLS_HTML_O_name))	+= "=\"";
	rStrm << sOut.GetBuffer();

	HTMLOutFuncs::Out_String( rStrm, rName, eDestEnc, pNonConvertableChars );

	((sOut = "\" ") += OOO_STRING_SVTOOLS_HTML_O_content) += "=\"";
	rStrm << sOut.GetBuffer();

	HTMLOutFuncs::Out_String( rStrm, rContent, eDestEnc, pNonConvertableChars ) << "\">";
}

void SfxFrameHTMLWriter::Out_DocInfo( SvStream& rStrm, const String& rBaseURL,
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
		const sal_Char *pIndent,
        rtl_TextEncoding eDestEnc,
		String *pNonConvertableChars	)
{
	const sal_Char *pCharSet =
				rtl_getBestMimeCharsetFromTextEncoding( eDestEnc );

	if( pCharSet )
	{
		String aContentType = String::CreateFromAscii( sHTML_MIME_text_html );
		aContentType.AppendAscii( pCharSet );
		OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_content_type, aContentType, sal_True,
                 eDestEnc, pNonConvertableChars );
	}

	// Titel (auch wenn er leer ist)
	rStrm << sNewLine;
	if( pIndent )
		rStrm << pIndent;
	HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title );
	if( i_xDocProps.is() )
	{
		const String& rTitle = i_xDocProps->getTitle();
		if( rTitle.Len() )
			HTMLOutFuncs::Out_String( rStrm, rTitle, eDestEnc, pNonConvertableChars );
	}
	HTMLOutFuncs::Out_AsciiTag( rStrm, OOO_STRING_SVTOOLS_HTML_title, sal_False );

	// Target-Frame
	if( i_xDocProps.is() )
	{
		const String& rTarget = i_xDocProps->getDefaultTarget();
		if( rTarget.Len() )
		{
			rStrm << sNewLine;
			if( pIndent )
				rStrm << pIndent;

			ByteString sOut( '<' );
			(((sOut += OOO_STRING_SVTOOLS_HTML_base) += ' ') += OOO_STRING_SVTOOLS_HTML_O_target) += "=\"";
			rStrm << sOut.GetBuffer();
			HTMLOutFuncs::Out_String( rStrm, rTarget, eDestEnc, pNonConvertableChars )
				<< "\">";
		}
	}

	// Who we are
	String sGenerator( SfxResId( STR_HTML_GENERATOR ) );
	sGenerator.SearchAndReplaceAscii( "%1", String( DEFINE_CONST_UNICODE( TOOLS_INETDEF_OS ) ) );
	OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_generator, sGenerator, sal_False, eDestEnc, pNonConvertableChars );

	if( i_xDocProps.is() )
	{
		// Reload
        if( (i_xDocProps->getAutoloadSecs() != 0) ||
            !i_xDocProps->getAutoloadURL().equalsAscii("") )
		{
			String sContent = String::CreateFromInt32(
								i_xDocProps->getAutoloadSecs() );

			const String &rReloadURL = i_xDocProps->getAutoloadURL();
			if( rReloadURL.Len() )
			{
				sContent.AppendAscii( ";URL=" );
                sContent += String(
                    URIHelper::simpleNormalizedMakeRelative(
                        rBaseURL, rReloadURL));
			}

			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_refresh, sContent, sal_True,
                     eDestEnc, pNonConvertableChars );
		}

		// Author
		const String& rAuthor = i_xDocProps->getAuthor();
		if( rAuthor.Len() )
			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_author, rAuthor, sal_False,
                     eDestEnc, pNonConvertableChars );

		// created
        ::util::DateTime uDT = i_xDocProps->getCreationDate();
        Date aD(uDT.Day, uDT.Month, uDT.Year);
        Time aT(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        String sOut = String::CreateFromInt32(aD.GetDate());
        sOut += ';';
        sOut += String::CreateFromInt32(aT.GetTime());
		OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_created, sOut, sal_False,
                 eDestEnc, pNonConvertableChars );

		// changedby
		const String& rChangedBy = i_xDocProps->getModifiedBy();
		if( rChangedBy.Len() )
			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changedby, rChangedBy, sal_False,
                     eDestEnc, pNonConvertableChars );

		// changed
        uDT = i_xDocProps->getModificationDate();
        Date aD2(uDT.Day, uDT.Month, uDT.Year);
        Time aT2(uDT.Hours, uDT.Minutes, uDT.Seconds, uDT.HundredthSeconds);
        sOut = String::CreateFromInt32(aD2.GetDate());
        sOut += ';';
        sOut += String::CreateFromInt32(aT2.GetTime());
		OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_changed, sOut, sal_False,
                 eDestEnc, pNonConvertableChars );

		// Subject
		const String& rTheme = i_xDocProps->getSubject();
		if( rTheme.Len() )
			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_classification, rTheme, sal_False,
                     eDestEnc, pNonConvertableChars );

		// Description
		const String& rComment = i_xDocProps->getDescription();
		if( rComment.Len() )
			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_description, rComment, sal_False,
                     eDestEnc, pNonConvertableChars);

		// Keywords
		String Keywords = ::comphelper::string::convertCommaSeparated(
            i_xDocProps->getKeywords());
		if( Keywords.Len() )
			OutMeta( rStrm, pIndent, OOO_STRING_SVTOOLS_HTML_META_keywords, Keywords, sal_False,
                     eDestEnc, pNonConvertableChars);

        uno::Reference < script::XTypeConverter > xConverter(
            ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString::createFromAscii("com.sun.star.script.Converter")),
                uno::UNO_QUERY_THROW );
        uno::Reference<beans::XPropertySet> xUserDefinedProps(
            i_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        DBG_ASSERT(xUserDefinedProps.is(), "UserDefinedProperties is null");
        uno::Reference<beans::XPropertySetInfo> xPropInfo =
            xUserDefinedProps->getPropertySetInfo();
        DBG_ASSERT(xPropInfo.is(), "UserDefinedProperties Info is null");
        uno::Sequence<beans::Property> props = xPropInfo->getProperties();
        for (sal_Int32 i = 0; i < props.getLength(); ++i) {
            try {
                ::rtl::OUString name = props[i].Name;
                ::rtl::OUString str;
                uno::Any aStr = xConverter->convertToSimpleType(
                        xUserDefinedProps->getPropertyValue(name),
                        uno::TypeClass_STRING);
                aStr >>= str;
                String valstr(str);
                valstr.EraseTrailingChars();
                OutMeta( rStrm, pIndent, name, valstr, sal_False,
                         eDestEnc, pNonConvertableChars );
            } catch (uno::Exception &) {
                // may happen with concurrent modification...
                DBG_WARNING("SfxFrameHTMLWriter::Out_DocInfo: exception");
            }
        }
	}
}
/*
void SfxFrameHTMLWriter::OutHeader( rtl_TextEncoding eDestEnc )
{
	// <HTML>
	// <HEAD>
	// <TITLE>Titel</TITLE>
	// </HEAD>
	HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_html ) << sNewLine;
	HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head );

	Out_DocInfo( Strm(), &pDoc->GetDocInfo(), "\t", eDestEnc );
	Strm() << sNewLine;
	HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_head, sal_False ) << sNewLine;

//!	OutScript();			// Hier fehlen noch die Scripten im Header
}
*/

void SfxFrameHTMLWriter::Out_FrameDescriptor(
    SvStream& rOut, const String& rBaseURL, const uno::Reference < beans::XPropertySet >& xSet,
    rtl_TextEncoding eDestEnc, String *pNonConvertableChars )
{
    try
    {
        ByteString sOut;
        ::rtl::OUString aStr;
        uno::Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameURL") );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            String aURL = INetURLObject( aStr ).GetMainURL( INetURLObject::DECODE_TO_IURI );
            if( aURL.Len() )
            {
                aURL = URIHelper::simpleNormalizedMakeRelative(
                    rBaseURL, aURL );
                ((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_src) += "=\"";
                rOut << sOut.GetBuffer();
                HTMLOutFuncs::Out_String( rOut, aURL, eDestEnc, pNonConvertableChars );
                sOut = '\"';
            }
        }

        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameName") );
        if ( (aAny >>= aStr) && aStr.getLength() )
        {
            ((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_name) += "=\"";
            rOut << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rOut, aStr, eDestEnc, pNonConvertableChars );
            sOut = '\"';
        }

        sal_Int32 nVal = SIZE_NOT_SET;
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginWidth") );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
            (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_marginwidth) += '=') += ByteString::CreateFromInt32( nVal );
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameMarginHeight") );
        if ( (aAny >>= nVal) && nVal != SIZE_NOT_SET )
            (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_marginheight) += '=') += ByteString::CreateFromInt32( nVal );

        sal_Bool bVal = sal_True;
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoScroll") );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsScrollingMode") );
            if ( aAny >>= bVal )
            {
                const sal_Char *pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_scrolling) += '=') += pStr;
            }
        }

        // frame border (MS+Netscape-Erweiterung)
        aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsAutoBorder") );
        if ( (aAny >>= bVal) && !bVal )
        {
            aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("FrameIsBorder") );
            if ( aAny >>= bVal )
            {
                const char* pStr = bVal ? sHTML_SC_yes : sHTML_SC_no;
                (((sOut += ' ') += OOO_STRING_SVTOOLS_HTML_O_frameborder) += '=') += pStr;
            }
        }

        // TODO/LATER: currently not supported attributes
        // resize
        //if( !pFrame->IsResizable() )
        //    (sOut += ' ') += sHTML_O_noresize;
        //
        //if ( pFrame->GetWallpaper() )
        //{
        //    ((sOut += ' ') += sHTML_O_bordercolor) += '=';
        //    rOut << sOut.GetBuffer();
        //    HTMLOutFuncs::Out_Color( rOut, pFrame->GetWallpaper()->GetColor(), eDestEnc );
        //}
        //else
            rOut << sOut.GetBuffer();
    }
    catch ( uno::Exception& )
    {
    }
}

String SfxFrameHTMLWriter::CreateURL( SfxFrame* pFrame )
{
	String aRet;
	SfxObjectShell* pShell = pFrame->GetCurrentDocument();
	if( !aRet.Len() && pShell )
	{
		aRet = pShell->GetMedium()->GetName();
//!(dv)		CntAnchor::ToPresentationURL( aRet );
	}

	return aRet;
}


