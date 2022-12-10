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



#ifndef COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX
#define COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX

#include <comphelper/comphelperdllapi.h>

/** === begin UNO includes === **/
#include <com/sun/star/uno/XComponentContext.hpp>
/** === end UNO includes === **/
#include <rtl/ustring.hxx>

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

	//====================================================================
	//= OfficeResourceBundle
	//====================================================================
    class ResourceBundle_Impl;
    /** wraps the <type scope="com::sun::star::resource">OfficeResourceAccess</type> service
    */
	class COMPHELPER_DLLPUBLIC OfficeResourceBundle
	{
    private:
        ::std::auto_ptr< ResourceBundle_Impl >  m_pImpl;

    public:
        /** constructs a resource bundle
            @param  _context
                the component context to operate in
            @param  _bundleBaseName
                the base name of the resource file which should be accessed (*without* the SUPD!)
            @raises ::com::sun::star::lang::NullPointerException
                if the given component context is <NULL/>
        */
        OfficeResourceBundle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _context,
            const ::rtl::OUString& _bundleBaseName
        );

        /** constructs a resource bundle with the resource bundle given as 8-bit ASCII name

            This is a convenience constructor only, it does nothing different than the constructor
            taking an unicode string.

            @param  _context
                the component context to operate in
            @param  _bundleBaseName
                the base name of the resource file which should be accessed (*without* the SUPD!)
            @raises ::com::sun::star::lang::NullPointerException
                if the given component context is <NULL/>
        */
        OfficeResourceBundle(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _context,
            const sal_Char* _bundleBaseAsciiName
        );

        /** destroys the instance
        */
        ~OfficeResourceBundle();

        /** loads the string with the given resource id from the resource bundle
            @param  _resourceId
                the id of the string to load
            @return
                the requested resource string. If no string with the given id exists in the resource bundle,
                an empty string is returned. In a non-product version, an OSL_ENSURE will notify you of this
                then.
        */
        ::rtl::OUString loadString( sal_Int32 _resourceId ) const;

        /** determines whether the resource bundle has a string with the given id
            @param  _resourceId
                the id of the string whose existence is to be checked
            @return
                <TRUE/> if and only if a string with the given ID exists in the resource
                bundle.
        */
        bool            hasString( sal_Int32 _resourceId ) const;
	};

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_OFFICE_RESOURCE_BUNDLE_HXX

