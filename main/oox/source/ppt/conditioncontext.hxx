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




#ifndef OOX_PPT_CONDITIONCONTEXT
#define OOX_PPT_CONDITIONCONTEXT

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/animations/Event.hpp>

#include "oox/core/fragmenthandler.hxx"
#include "oox/ppt/timenode.hxx"
#include "oox/ppt/timenodelistcontext.hxx"
#include "oox/ppt/animationspersist.hxx"

namespace oox { namespace ppt {


	/** CT_TLTimeCondition */
	class CondContext
		: public TimeNodeContext
	{
	public:
        CondContext( ::oox::core::ContextHandler& rParent,
					 const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
					 const TimeNodePtr & pNode, AnimationCondition & aCond );
		~CondContext( ) throw( );
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

	private:
//		::com::sun::star::uno::Any &         maCond;
		::com::sun::star::animations::Event  maEvent;
//		AnimTargetElementPtr         mpTarget;
		AnimationCondition &                 maCond;
	};



	/** CT_TLTimeConditionList */
	class CondListContext
		: public TimeNodeContext
	{
	public:
        CondListContext( ::oox::core::ContextHandler& rParent,
             sal_Int32  aElement,
             const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs,
             const TimeNodePtr & pNode, AnimationConditionList & aCondList );
		~CondListContext( ) throw( );

		virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& /*xAttribs*/ ) throw ( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

	private:
		AnimationConditionList     & maConditions;
	};


} }


#endif
