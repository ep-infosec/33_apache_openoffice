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



#ifndef _XMLOFF_ANIMATIONEXPORT_HXX
#define _XMLOFF_ANIMATIONEXPORT_HXX

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/uniref.hxx>

class SvXMLExport;

namespace xmloff
{
class AnimationsExporterImpl;

class AnimationsExporter : public UniRefBase
{
	AnimationsExporterImpl*	mpImpl;	

public:
	AnimationsExporter( SvXMLExport& rExport, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPageProps  );
	virtual ~AnimationsExporter();

	void prepare( ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > xRootNode );
	void exportAnimations( ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > xRootNode );
};

}

#endif	//  _XMLOFF_ANIMATIONEXPORT_HXX

