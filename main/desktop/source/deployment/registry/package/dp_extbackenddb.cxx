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

#include "dp_extbackenddb.hxx"


namespace css = ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define EXTENSION_REG_NS "http://openoffice.org/extensionmanager/extension-registry/2010"
#define NS_PREFIX "ext"
#define ROOT_ELEMENT_NAME "extension-backend-db"
#define KEY_ELEMENT_NAME "extension"

namespace dp_registry {
namespace backend {
namespace bundle {

ExtensionBackendDb::ExtensionBackendDb(
    Reference<XComponentContext> const &  xContext,
    ::rtl::OUString const & url):BackendDb(xContext, url)
{

}

OUString ExtensionBackendDb::getDbNSName()
{
    return OUSTR(EXTENSION_REG_NS);
}

OUString ExtensionBackendDb::getNSPrefix()
{
    return OUSTR(NS_PREFIX);
}

OUString ExtensionBackendDb::getRootElementName()
{
    return OUSTR(ROOT_ELEMENT_NAME);
}

OUString ExtensionBackendDb::getKeyElementName()
{
    return OUSTR(KEY_ELEMENT_NAME);
}

void ExtensionBackendDb::addEntry(::rtl::OUString const & url, Data const & data)
{
    try{
        //reactive revoked entry if possible.
        if (!activateEntry(url))
        {
            Reference<css::xml::dom::XNode> extensionNodeNode = writeKeyElement(url);
            writeVectorOfPair(
                data.items,
                OUSTR("extension-items"),
                OUSTR("item"),
                OUSTR("url"),
                OUSTR("media-type"),
                extensionNodeNode);
            save();
        }
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to write data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

ExtensionBackendDb::Data ExtensionBackendDb::getEntry(::rtl::OUString const & url)
{
    try
    {
        ExtensionBackendDb::Data retData;
        Reference<css::xml::dom::XNode> aNode = getKeyElement(url);

        if (aNode.is())
        {
            retData.items =
                readVectorOfPair(
                    aNode,
                    OUSTR("extension-items"),
                    OUSTR("item"),
                    OUSTR("url"),
                    OUSTR("media-type"));
        }
        return retData;
    }
    catch(css::uno::Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw css::deployment::DeploymentException(
            OUSTR("Extension Manager: failed to read data entry in backend db: ") +
            m_urlDb, 0, exc);
    }
}

} // namespace bundle
} // namespace backend
} // namespace dp_registry

