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



#include	<rtl/alloc.h>

#include	"specialtypemanager.hxx"

extern "C" 
{
sal_Bool SAL_CALL initTypeMapper( const sal_Char* pRegName );	
sal_uInt32 SAL_CALL getTypeBlop(const sal_Char* pTypeName, sal_uInt8** pBlop);
}

using namespace rtl;

SpecialTypeManager::SpecialTypeManager()
{
	m_pImpl = new SpecialTypeManagerImpl();
	acquire();	
}	

SpecialTypeManager::~SpecialTypeManager()
{
	release();
}	

void SpecialTypeManager::acquire()
{ 
	TypeManager::acquire();
}

void SpecialTypeManager::release()
{ 
	if (0 == TypeManager::release())
	{
		delete m_pImpl;
	}
}

sal_Bool SpecialTypeManager::init(const OString& registryName)
{
	return initTypeMapper( registryName.getStr() );
}	

TypeReader SpecialTypeManager::getTypeReader(const OString& name)
{
	TypeReader reader;

	sal_uInt8* pBlop = NULL;
	sal_uInt32 blopSize = 0;

	if ( (blopSize = getTypeBlop( name.getStr(), &pBlop)) > 0 )
	{
		reader = TypeReader(pBlop, blopSize, sal_True);
	}

	if ( pBlop )
	{
		rtl_freeMemory(pBlop);
	}

	return reader;
}	

RTTypeClass SpecialTypeManager::getTypeClass(const OString& name)
{
	if (m_pImpl->m_t2TypeClass.count(name) > 0)
	{
		return m_pImpl->m_t2TypeClass[name];		
	} else
	{
	}	

	return RT_TYPE_INVALID;	
}	

	
