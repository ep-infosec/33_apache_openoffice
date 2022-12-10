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



#ifndef PAGECOLLECTOR_HXX
#define PAGECOLLECTOR_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <vector>

class PageCollector
{
	public:

	struct MasterPageEntity
	{
		com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > xMasterPage;
		sal_Bool bUsed;
	};
	static void CollectCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, const rtl::OUString& rCustomShow, std::vector< com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > >& );
	static void CollectNonCustomShowPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, const rtl::OUString& rCustomShow, std::vector< com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > >& );
	static void CollectMasterPages( const com::sun::star::uno::Reference< com::sun::star::frame::XModel >&, std::vector< MasterPageEntity >& );
};

#endif // PAGECOLLECTOR_HXX
