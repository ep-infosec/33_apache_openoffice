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



#ifndef _PADMIN_DESKTOPCONTEXT_HXX_
#define _PADMIN_DESKTOPCONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <uno/current_context.hxx>

#define DESKTOP_ENVIRONMENT_NAME "system.desktop-environment"

namespace padmin
{
    class DesktopContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
    {
    public:
	DesktopContext( const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > & ctx);

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
            com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_xNextContext;
    };
}

#endif // _PADMIN_DESKTOPCONTEXT_HXX_
