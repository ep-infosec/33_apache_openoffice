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



#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#define _XMLOFF_GRADIENTSTYLE_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <sal/types.h>

class SvXMLImport;
class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace uno { template<class A> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace uno { class Any; }
} } }
namespace rtl { class OUString; }


class XMLOFF_DLLPUBLIC XMLGradientStyleImport
{
    SvXMLImport& rImport;

public:
	XMLGradientStyleImport( SvXMLImport& rImport );
	~XMLGradientStyleImport();

	sal_Bool importXML( 
        const ::com::sun::star::uno::Reference< 
           ::com::sun::star::xml::sax::XAttributeList >& xAttrList, 
        ::com::sun::star::uno::Any& rValue, 
        ::rtl::OUString& rStrName );
};


class XMLOFF_DLLPUBLIC XMLGradientStyleExport
{
    SvXMLExport& rExport;

public:
	XMLGradientStyleExport( SvXMLExport& rExport );
	~XMLGradientStyleExport();

	sal_Bool exportXML( 
        const ::rtl::OUString& rStrName, 
        const ::com::sun::star::uno::Any& rValue );
};

#endif // _XMLOFF_GRADIENTSTYLE_HXX
