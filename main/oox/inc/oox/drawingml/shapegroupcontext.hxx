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



#ifndef OOX_DRAWINGML_SHAPEGROUPCONTEXT_HXX
#define OOX_DRAWINGML_SHAPEGROUPCONTEXT_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/shapecontext.hxx"

namespace oox { namespace drawingml {

class ShapeGroupContext : public ::oox::core::ContextHandler
{
public:
    ShapeGroupContext( ::oox::core::ContextHandler& rParent, ShapePtr pMasterShapePtr, ShapePtr pGroupShapePtr );
	virtual ~ShapeGroupContext();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

protected:

    ShapePtr            mpGroupShapePtr;
    ShapePtr            mpMasterShapePtr;
};

} }

#endif  //  OOX_DRAWINGML_SHAPEGROUPCONTEXT_HXX
