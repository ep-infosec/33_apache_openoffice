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


#ifndef _SFX_HELPER_HXX
#define _SFX_HELPER_HXX

// include ---------------------------------------------------------------

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/NameClash.hpp>

#include <tools/string.hxx>
#include <tools/errcode.hxx>
#include <tools/bigint.hxx>

// class SfxContentHelper ------------------------------------------------

class SfxContentHelper
{
private:
    static sal_Bool             Transfer_Impl( const String& rSource, const String& rDest, sal_Bool bMoveData,
                                                    sal_Int32 nNameClash );

public:
	static sal_Bool				IsDocument( const String& rContent );
	static sal_Bool				IsFolder( const String& rContent );
	static sal_Bool				GetTitle( const String& rContent, String& rTitle );
	static sal_Bool				Kill( const String& rContent );

	static ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted = sal_False );
	static ::com::sun::star::uno::Sequence< ::rtl::OUString >
								GetFolderContentProperties( const String& rFolder, sal_Bool bFolder );
	static ::com::sun::star::uno::Sequence< ::rtl::OUString >
								GetResultSet( const String& rURL );
	static ::com::sun::star::uno::Sequence< ::rtl::OUString >
								GetHelpTreeViewContents( const String& rURL );
	static String				GetActiveHelpString( const String& rURL );
	static sal_Bool				IsHelpErrorDocument( const String& rURL );

	static sal_Bool				CopyTo( const String& rSource, const String& rDest );
    static sal_Bool             MoveTo( const String& rSource, const String& rDest, sal_Int32 nNameClash = com::sun::star::ucb::NameClash::ERROR );

	static sal_Bool				MakeFolder( const String& rFolder );
	static ErrCode				QueryDiskSpace( const String& rPath, sal_Int64& rFreeBytes );
	static sal_uIntPtr				GetSize( const String& rContent );

	// please don't use this!
	static sal_Bool				Exists( const String& rContent );
	static sal_Bool				Find( const String& rFolder, const String& rName, String& rFile );
};

#endif // #ifndef _SFX_HELPER_HXX


