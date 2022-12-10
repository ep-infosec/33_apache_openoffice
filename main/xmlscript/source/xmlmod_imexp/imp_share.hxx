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



#include <xmlscript/xmlmod_imexp.hxx>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <com/sun/star/xml/input/XRoot.hpp>

#include <vector>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

//==================================================================================================
// Script module import

//==================================================================================================
struct ModuleImport
    : public ::cppu::WeakImplHelper1< xml::input::XRoot >
{
    friend class ModuleElement;
    
    ModuleDescriptor& mrModuleDesc;

    sal_Int32 XMLNS_SCRIPT_UID;
    sal_Int32 XMLNS_LIBRARY_UID;
    sal_Int32 XMLNS_XLINK_UID;
    
public:
    inline ModuleImport( ModuleDescriptor& rModuleDesc )
        SAL_THROW( () )
        : mrModuleDesc( rModuleDesc ) {}
    virtual ~ModuleImport()
        SAL_THROW( () );
    
    // XRoot
    virtual void SAL_CALL startDocument(
        Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        Reference< xml::sax::XLocator > const & xLocator )
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class ModuleElement
    : public ::cppu::WeakImplHelper1< xml::input::XElement >
{
protected:  
    ModuleImport * _pImport;
    ModuleElement * _pParent;
    
    OUString _aLocalName;
    Reference< xml::input::XAttributes > _xAttributes;
    ::rtl::OUStringBuffer _StrBuffer;
    
public:
    ModuleElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ModuleElement * pParent, ModuleImport * pImport )
        SAL_THROW( () );
    virtual ~ModuleElement()
        SAL_THROW( () );
    
    // XElement
    virtual Reference< xml::input::XElement > SAL_CALL getParent()
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalName()
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (RuntimeException);
    virtual Reference< xml::input::XAttributes > SAL_CALL getAttributes()
        throw (RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================

}
