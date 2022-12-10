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



#ifndef _XMLOFF_TRANSFORMERTOKENMAPS_HXX
#define _XMLOFF_TRANSFORMERTOKENMAPS_HXX

#include <rtl/ustring.hxx>
#include <comphelper/stl_types.hxx>
#include <hash_map>
#include <xmloff/xmltoken.hxx>
#ifndef _XMLOFF_TRANSFORMERTOKENMAP_HXX
#include "TransformerTokenMap.hxx"
#endif



class XMLTransformerTokenMap :
	public ::std::hash_map< ::rtl::OUString, ::xmloff::token::XMLTokenEnum, 
						    ::rtl::OUStringHash, ::comphelper::UStringEqual >
{
public:
	XMLTransformerTokenMap( ::xmloff::token::XMLTokenEnum *pInit );
	~XMLTransformerTokenMap();
};

#endif	//  _XMLOFF_TRANSFORMERTOKENMAPS_HXX
