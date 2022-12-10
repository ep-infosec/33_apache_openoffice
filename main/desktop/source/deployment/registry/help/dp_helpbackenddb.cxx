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
#include "precompiled_desktop.hxx"

#include "rtl/string.h"
#include "rtl/bootstrap.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "dp_misc.h"

#include "dp_helpbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/help-registry/2010"
#define NS_PREFIX "help"
#define ROOT_ELEMENT_NAME "help-backend-db"
#define KEY_ELEMENT_NAME "help"

namespace dp_registry {
namespace backend {
namespace help {

HelpBackendDb::HelpBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):BackendDb(xContext, url)
{

}

OUString HelpBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString HelpBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString HelpBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

OUString HelpBackendDb::getKeyElementName()
{
    return OUSTR(KEY_ELEMENT_NAME);
}


void HelpBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> helpNode
                = writeKeyElement(url);
            
            writeSimpleElement(OUSTR("data-url"), data.dataUrl, helpNode);
            save();
        }
    }
    catch (css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in help backend db: ") +
            m_urlDb, 0, exc);
    }
}


::boost::optional<HelpBackendDb::Data>
HelpBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        HelpBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);
        if (aNode.is())
        {
            retData.dataUrl = readSimpleElement(OUSTR("data-url"), aNode);
        }
        else
        {
            return ::boost::optional<Data>();
        }
        return ::boost::optional<Data>(retData);
    }
    catch (css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in help backend db: ") +
            m_urlDb, 0, exc);
    }
}

::std::list<OUString> HelpBackendDb::getAllDataUrls()
{
    try
    {
        ::std::list<OUString> listRet;
        Reference<css::xml::dom::XDocument> doc = getDocument();
        Reference<css::xml::dom::XNode> root = doc->getFirstChild();

        Reference<css::xml::xpath::XXPathAPI> xpathApi = getXPathAPI();
        const OUString sPrefix = getNSPrefix();
        OUString sExpression(
            sPrefix + OUSTR(":help/") + sPrefix + OUSTR(":data-url/text()"));
        Reference<css::xml::dom::XNodeList> nodes =
            xpathApi->selectNodeList(root, sExpression);
        if (nodes.is())
        {
            sal_Int32 length = nodes->getLength();
            for (sal_Int32 i = 0; i < length; i++)
                listRet.push_back(nodes->item(i)->getNodeValue());
        }
        return listRet;
    }
    catch (css::deployment::DeploymentException& )
    {
        throw;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in help backend db: ") +
            m_urlDb, 0, exc);
    }
}


} // namespace help
} // namespace backend
} // namespace dp_registry

