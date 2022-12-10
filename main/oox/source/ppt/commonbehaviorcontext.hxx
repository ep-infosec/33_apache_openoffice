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




#ifndef OOX_PPT_COMMONBEHAVIORCONTEXT
#define OOX_PPT_COMMONBEHAVIORCONTEXT

#include <rtl/ustring.hxx>
#include "oox/ppt/timenodelistcontext.hxx"
#include "oox/ppt/animationspersist.hxx"
#include "conditioncontext.hxx"
#include "pptfilterhelpers.hxx"

namespace oox { namespace ppt {

	struct Attribute
	{
		::rtl::OUString   name;
		MS_AttributeNames type;
	};


	/** CT_TLCommonBehaviorData */
	class CommonBehaviorContext
		: public TimeNodeContext
	{
	public:
        CommonBehaviorContext( ::oox::core::ContextHandler& rParent,
             const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
             const TimeNodePtr & pNode );
		~CommonBehaviorContext( )
			throw( );

		virtual void SAL_CALL endFastElement( sal_Int32 aElement )
			throw ( ::com::sun::star::xml::sax::SAXException,
							::com::sun::star::uno::RuntimeException );

		virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
			throw ( ::com::sun::star::xml::sax::SAXException,
							::com::sun::star::uno::RuntimeException );

		virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken,
																																																																 const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& /*xAttribs*/ )
			throw ( ::com::sun::star::xml::sax::SAXException,
							::com::sun::star::uno::RuntimeException );

	private:
		bool              mbInAttrList;
		bool              mbIsInAttrName;
		std::list< Attribute > maAttributes;
		::rtl::OUString   msCurrentAttribute;
	};


} }


#endif
