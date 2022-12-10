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



#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "uno/current_context.h"
#include "uno/dispatcher.hxx"
#include "uno/lbnames.h"

#include "currentcontext.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

namespace current_context {

css::uno::UnoInterfaceReference get() {
    css::uno::UnoInterfaceReference cc;
    if (!uno_getCurrentContext(
            reinterpret_cast< void ** >(&cc.m_pUnoI),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)).pData, 0))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("uno_getCurrentContext failed")),
            css::uno::Reference< css::uno::XInterface >());
    }
    return cc;
}

void set(css::uno::UnoInterfaceReference const & value) {
    css::uno::UnoInterfaceReference cc(value);
    if (!uno_setCurrentContext(
            cc.m_pUnoI,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO)).pData, 0))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("uno_setCurrentContext failed")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

}

}
