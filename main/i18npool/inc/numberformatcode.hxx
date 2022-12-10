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



#ifndef _I18N_NUMBERFORMATCODE_HXX_
#define _I18N_NUMBERFORMATCODE_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

class NumberFormatCodeMapper : public cppu::WeakImplHelper2
<
	::com::sun::star::i18n::XNumberFormatCode,
	::com::sun::star::lang::XServiceInfo
>
{
public:
	NumberFormatCodeMapper( const ::com::sun::star::uno::Reference <
					::com::sun::star::lang::XMultiServiceFactory >& rxMSF );
	~NumberFormatCodeMapper();

    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::i18n::NumberFormatCode SAL_CALL getFormatCode( sal_Int16 nFormatIndex, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCode( sal_Int16 nFormatUsage, const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > SAL_CALL getAllFormatCodes( const ::com::sun::star::lang::Locale& rLocale ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );

private:
	::com::sun::star::lang::Locale aLocale;
	::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xMSF;
	::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > aFormatSeq;
	::com::sun::star::uno::Reference < ::com::sun::star::i18n::XLocaleData > xlocaleData;
	sal_Bool bFormatsValid;

	void setupLocale( const ::com::sun::star::lang::Locale& rLocale );
	void getFormats( const ::com::sun::star::lang::Locale& rLocale );
	::rtl::OUString mapElementTypeShortToString(sal_Int16 formatType);
	sal_Int16 mapElementTypeStringToShort(const ::rtl::OUString& formatType);
	::rtl::OUString mapElementUsageShortToString(sal_Int16 formatUsage);
	sal_Int16 mapElementUsageStringToShort(const ::rtl::OUString& formatUsage);
	void createLocaleDataObject();
};


#endif // _I18N_NUMBERFORMATCODE_HXX_
