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


#ifndef _CLONELIST_HXX_
#define _CLONELIST_HXX_

#include <sal/types.h>
#include <tools/list.hxx>

// predeclarations
class SdrObject;

// #i13033#
// New mechanism to hold a ist of all original and cloned objects for later
// re-creating the connections for contained connectors
class CloneList
{
	List						maOriginalList;
	List						maCloneList;

public:
	CloneList();
	~CloneList();

	void AddPair(const SdrObject* pOriginal, SdrObject* pClone);
	sal_uInt32 Count() const;

	const SdrObject* GetOriginal(sal_uInt32 nIndex) const;
	SdrObject* GetClone(sal_uInt32 nIndex) const;

	void CopyConnections() const;
};

#endif // _CLONELIST_HXX_
