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



#if ! defined INCLUDED_DP_PROPERTIES_HXX
#define INCLUDED_DP_PROPERTIES_HXX



#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "boost/optional.hpp"


namespace css = ::com::sun::star;

namespace dp_manager {



/**

 */
class ExtensionProperties
{
protected:    
    ::rtl::OUString m_propFileUrl;
    const css::uno::Reference<css::ucb::XCommandEnvironment> m_xCmdEnv;
    ::boost::optional< ::rtl::OUString> m_prop_suppress_license;
    ::boost::optional< ::rtl::OUString> m_prop_extension_update;

    ::rtl::OUString getPropertyValue(css::beans::NamedValue const & v);
public:
    
    virtual ~ExtensionProperties() {};
    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);
    
    ExtensionProperties(::rtl::OUString const & urlExtension,
                        css::uno::Sequence<css::beans::NamedValue> const & properties,
                        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

    void write();

    bool isSuppressedLicense();

    bool isExtensionUpdate();
};
}




#endif

