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


#ifndef _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_
#define _XMLOFF_XMLFONTSTYLESCONTEXT_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _XMLOFF_XMLSTYLE_HXX_
#include <xmloff/xmlstyle.hxx>
#endif

struct XMLPropertyState;
class SvXMLTokenMap;
class XMLFontFamilyNamePropHdl;
class XMLFontFamilyPropHdl;
class XMLFontPitchPropHdl;
class XMLFontEncodingPropHdl;

class XMLOFF_DLLPUBLIC XMLFontStylesContext : public SvXMLStylesContext
{
	XMLFontFamilyNamePropHdl 	*pFamilyNameHdl;
	XMLFontFamilyPropHdl 		*pFamilyHdl;
	XMLFontPitchPropHdl 		*pPitchHdl;
	XMLFontEncodingPropHdl 		*pEncHdl;

	SvXMLTokenMap			*pFontStyleAttrTokenMap;

	rtl_TextEncoding		eDfltEncoding;

protected:

	virtual SvXMLStyleContext *CreateStyleChildContext( sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const ::com::sun::star::uno::Reference<
			::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:

	TYPEINFO();

	XMLFontStylesContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
			const ::rtl::OUString& rLName,
			const ::com::sun::star::uno::Reference<
				::com::sun::star::xml::sax::XAttributeList > & xAttrList,
			rtl_TextEncoding eDfltEnc );
	virtual ~XMLFontStylesContext();

	const SvXMLTokenMap& GetFontStyleAttrTokenMap() const
	{
		return *pFontStyleAttrTokenMap;
	}

	sal_Bool FillProperties( const ::rtl::OUString& rName,
						 ::std::vector< XMLPropertyState > &rProps,
						 sal_Int32 nFamilyNameIdx,
						 sal_Int32 nStyleNameIdx,
						 sal_Int32 nFamilyIdx,
						 sal_Int32 nPitchIdx,
						 sal_Int32 nCharsetIdx ) const;

	rtl_TextEncoding GetDfltCharset() const { return eDfltEncoding; }

	XMLFontFamilyNamePropHdl& GetFamilyNameHdl() const { return *pFamilyNameHdl; }
	XMLFontFamilyPropHdl& GetFamilyHdl() const { return *pFamilyHdl; }
	XMLFontPitchPropHdl& GetPitchHdl() const { return *pPitchHdl; }
	XMLFontEncodingPropHdl& GetEncodingHdl() const { return *pEncHdl; }

};

#endif
