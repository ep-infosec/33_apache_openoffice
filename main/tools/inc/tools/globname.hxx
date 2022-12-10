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


#ifndef _GLOBNAME_HXX
#define _GLOBNAME_HXX

#include "tools/toolsdllapi.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>

/*************************************************************************
*************************************************************************/
struct ImpSvGlobalName
{
    sal_uInt8       szData[ 16 ];
    sal_uInt16      nRefCount;

                ImpSvGlobalName()
                {
                    nRefCount = 0;
                }
                ImpSvGlobalName( const ImpSvGlobalName & rObj );
                ImpSvGlobalName( int );

    sal_Bool    operator == ( const ImpSvGlobalName & rObj ) const;
};

#ifdef WNT
struct _GUID;
typedef struct _GUID GUID;
#else
struct GUID;
#endif
typedef GUID CLSID;
class SvStream;
class SvGlobalNameList;
class TOOLS_DLLPUBLIC SvGlobalName
{
friend class SvGlobalNameList;
    ImpSvGlobalName * pImp;
    void    NewImp();
public:
            SvGlobalName();
            SvGlobalName( const SvGlobalName & rObj )
            {
                pImp = rObj.pImp;
                pImp->nRefCount++;
            }
            SvGlobalName( ImpSvGlobalName * pImpP )
            {
                pImp = pImpP;
                pImp->nRefCount++;
            }
            SvGlobalName( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                          sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                          sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

            // create SvGlobalName from a platform independent representation
            SvGlobalName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
            ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & operator << ( SvStream &, const SvGlobalName & );

    sal_Bool            operator < ( const SvGlobalName & rObj ) const;
    SvGlobalName &  operator += ( sal_uInt32 );
    SvGlobalName &  operator ++ () { return operator += ( 1 ); }

    sal_Bool    operator == ( const SvGlobalName & rObj ) const;
    sal_Bool    operator != ( const SvGlobalName & rObj ) const
            { return !(*this == rObj); }

    void    MakeFromMemory( void * pData );
    sal_Bool    MakeId( const String & rId );
    String  GetctorName() const;
    String  GetHexName() const;
    String  GetRegDbName() const
			{
				String a = '{';
				a += GetHexName();
				a += '}';
				return a;
			}

                  SvGlobalName( const CLSID & rId );
    const CLSID & GetCLSID() const { return *(CLSID *)pImp->szData; }
	const sal_uInt8* GetBytes() const { return pImp->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

class SvGlobalNameList
{
    List aList;
public:
                    SvGlobalNameList();
                    ~SvGlobalNameList();

    void            Append( const SvGlobalName & );
    SvGlobalName    GetObject( sal_uLong );
    sal_Bool            IsEntry( const SvGlobalName & rName );
    sal_uInt32           Count() const { return aList.Count(); }
private:
                // nicht erlaubt
                SvGlobalNameList( const SvGlobalNameList & );
    SvGlobalNameList & operator = ( const SvGlobalNameList & );
};

#endif // _GLOBNAME_HXX

