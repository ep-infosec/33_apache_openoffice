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



#if ! defined INCLUDED_DP_LIB_CONTAINER_H
#define INCLUDED_DP_LIB_CONTAINER_H

#include "rtl/ustring.hxx"
#include "xmlscript/xmllib_imexp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"


namespace css = ::com::sun::star;

namespace dp_registry {
namespace backend {
namespace script {

//==============================================================================
class LibraryContainer
{
public:
    static ::rtl::OUString get_libname(
        ::rtl::OUString const & url,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );
};

}
}
}

#endif
