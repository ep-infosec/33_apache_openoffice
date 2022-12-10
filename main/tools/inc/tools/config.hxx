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


#ifndef _CONFIG_HXX
#define _CONFIG_HXX

#include "tools/toolsdllapi.h"
#include <tools/string.hxx>

struct ImplConfigData;
struct ImplGroupData;

// ----------
// - Config -
// ----------

class TOOLS_DLLPUBLIC Config
{
private:
	XubString			maFileName;
	ByteString			maGroupName;
	ImplConfigData* 	mpData;
	ImplGroupData*		mpActGroup;
	sal_uIntPtr				mnDataUpdateId;
	sal_uInt16				mnLockCount;
	sal_Bool				mbPersistence;
	sal_Bool				mbDummy1;

#ifdef _CONFIG_CXX
	TOOLS_DLLPRIVATE sal_Bool				ImplUpdateConfig() const;
	TOOLS_DLLPRIVATE ImplGroupData*		ImplGetGroup() const;
#endif

public:
						Config();
						Config( const XubString& rFileName );
						~Config();

	const XubString&	GetPathName() const { return maFileName; }
	static XubString	GetDefDirectory();
	static XubString	GetConfigName( const XubString& rPath, const XubString& rBaseName );

	void				SetGroup( const ByteString& rGroup );
	const ByteString&	GetGroup() const { return maGroupName; }
	void				DeleteGroup( const ByteString& rGroup );
	ByteString			GetGroupName( sal_uInt16 nGroup ) const;
	sal_uInt16				GetGroupCount() const;
	sal_Bool				HasGroup( const ByteString& rGroup ) const;

	ByteString			ReadKey( const ByteString& rKey ) const;
	UniString			ReadKey( const ByteString& rKey, rtl_TextEncoding eEncoding ) const;
	ByteString			ReadKey( const ByteString& rKey, const ByteString& rDefault ) const;
	void				WriteKey( const ByteString& rKey, const ByteString& rValue );
	void				WriteKey( const ByteString& rKey, const UniString& rValue, rtl_TextEncoding eEncoding );
	void				DeleteKey( const ByteString& rKey );
	ByteString			GetKeyName( sal_uInt16 nKey ) const;
	ByteString			ReadKey( sal_uInt16 nKey ) const;
	sal_uInt16				GetKeyCount() const;

	void				EnterLock();
	void				LeaveLock();
	sal_Bool				IsLocked() const { return (mnLockCount != 0); }
	sal_Bool				Update();
	void				Flush();

	void				EnablePersistence( sal_Bool bPersistence = sal_True )
							{ mbPersistence = bPersistence; }
	sal_Bool				IsPersistenceEnabled() const { return mbPersistence; }

	void				SetLineEnd( LineEnd eLineEnd );
	LineEnd 			GetLineEnd() const;

private:
	TOOLS_DLLPRIVATE 				Config( const Config& rConfig );
	TOOLS_DLLPRIVATE Config& 			operator = ( const Config& rConfig );
};

#endif // _SV_CONFIG_HXX
