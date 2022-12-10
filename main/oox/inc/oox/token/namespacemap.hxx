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



#ifndef OOX_TOKEN_NAMESPACEMAP_HXX
#define OOX_TOKEN_NAMESPACEMAP_HXX

#include <map>
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>

namespace oox {

// ============================================================================

/** A map that contains all XML namespace URLs used in the filters. */
struct NamespaceMap : public ::std::map< sal_Int32, ::rtl::OUString > { NamespaceMap(); };

/** Thread-save singleton of a map of all supported XML namespace URLs. */
struct StaticNamespaceMap : public ::rtl::Static< NamespaceMap, StaticNamespaceMap > {};

// ============================================================================

} // namespace oox

#endif
