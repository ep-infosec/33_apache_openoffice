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



#ifndef _FETC_HXX_
#define _FETC_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/**********************************************************************
 stl container elements must fulfill the following requirements:
 1. they need a copy ctor and assignment operator(?)
 2. they must be compareable
 because the FORMATETC structure has a pointer to a TARGETDEVICE
 structure we need a simple wrapper class to fulfill these needs
***********************************************************************/

class CFormatEtc
{
public:
	CFormatEtc( );
	CFormatEtc( const FORMATETC& aFormatEtc );
	CFormatEtc( CLIPFORMAT cf, DWORD tymed = TYMED_HGLOBAL, DVTARGETDEVICE* ptd = NULL, DWORD dwAspect = DVASPECT_CONTENT, LONG lindex = -1 );
	CFormatEtc( const CFormatEtc& theOther );

	~CFormatEtc( );

	CFormatEtc& operator=( const CFormatEtc& theOther );
	operator FORMATETC*( );
	operator FORMATETC( );

	void getFORMATETC( LPFORMATETC lpFormatEtc );

	CLIPFORMAT getClipformat( ) const;
	DWORD      getTymed( ) const;
	void       getTargetDevice( DVTARGETDEVICE** ptd ) const;
	DWORD      getDvAspect( ) const;
	LONG       getLindex( ) const;

	void setClipformat( CLIPFORMAT cf );
	void setTymed( DWORD tymed );
	void setTargetDevice( DVTARGETDEVICE* ptd );
	void setDvAspect( DWORD dwAspect );
	void setLindex( LONG lindex );

private:
	FORMATETC m_FormatEtc;

	friend sal_Int32 operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
	friend sal_Int32 operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );
};

sal_Int32 operator==( const CFormatEtc& lhs, const CFormatEtc& rhs );
sal_Int32 operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs );


#endif
