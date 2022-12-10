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


#ifndef _LNKBASE_HXX
#define _LNKBASE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sot/exchange.hxx>
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _LINKSOURCE_HXX
#include <sfx2/linksrc.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno
{
	class Any;
	//class Type;
}}}}

namespace sfx2
{

struct ImplBaseLinkData;
class LinkManager;
class SvLinkSource;
class FileDialogHelper;

#ifndef OBJECT_DDE_EXTERN
#define	OBJECT_INTERN		0x00
//#define	OBJECT_SO_EXTERN	0x01
#define	OBJECT_DDE_EXTERN	0x02
#endif

#define	OBJECT_CLIENT_SO			0x80 // ein Link
#define	OBJECT_CLIENT_DDE			0x81
//#define	OBJECT_CLIENT_OLE			0x82 // ein Ole-Link
//#define	OBJECT_CLIENT_OLE_CACHE  	0x83 // ein Ole-Link mit SvEmbeddedObject
#define	OBJECT_CLIENT_FILE			0x90
#define	OBJECT_CLIENT_GRF			0x91
#define	OBJECT_CLIENT_OLE			0x92 // embedded link

enum sfxlink {
	// Ole2 compatibel und persistent
	LINKUPDATE_ALWAYS = 1,
	LINKUPDATE_ONCALL = 3,

	LINKUPDATE_END		// dummy!
};

struct BaseLink_Impl;

class SFX2_DLLPUBLIC SvBaseLink : public SvRefBase
{
private:
	friend class LinkManager;
	friend class SvLinkSource;

	SvLinkSourceRef			xObj;
	String					aLinkName;
    BaseLink_Impl*          pImpl;
	sal_uInt16 					nObjType;
	sal_Bool					bVisible : 1;
	sal_Bool					bSynchron : 1;
	sal_Bool					bUseCache : 1;		// fuer GrafikLinks!
    sal_Bool                    bWasLastEditOK : 1;

    DECL_LINK( EndEditHdl, String* );

    bool                    ExecuteEdit( const String& _rNewName );

protected:
	void			SetObjType( sal_uInt16 );

					// setzen des LinkSourceName ohne aktion
	void			SetName( const String & rLn );
					// LinkSourceName der im SvLinkBase steht
	String		 	GetName() const;

	ImplBaseLinkData* pImplData;

    sal_Bool            m_bIsReadOnly;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
                        m_xInputStreamToLoadFrom;

					SvBaseLink();
                    SvBaseLink( sal_uInt16 nLinkType, sal_uIntPtr nContentType = FORMAT_STRING );
	virtual 		~SvBaseLink();

    void            _GetRealObject( sal_Bool bConnect = sal_True );

    SvLinkSource*   GetRealObject()
					{
						if( !xObj.Is() )
                            _GetRealObject();
						return xObj;
					}

public:
					TYPEINFO();
					// ask JP
	virtual void    Closed();
                    SvBaseLink( const String& rNm, sal_uInt16 nObjectType,
                                 SvLinkSource* );

	sal_uInt16			GetObjType() const { return nObjType; }

	void			SetObj( SvLinkSource * pObj );
	SvLinkSource*	GetObj() const	{ return xObj; }

	void    		SetLinkSourceName( const String & rName );
	String		 	GetLinkSourceName() const;

	virtual void 	DataChanged( const String & rMimeType,
								const ::com::sun::star::uno::Any & rValue );

	void			SetUpdateMode( sal_uInt16 );
	sal_uInt16 			GetUpdateMode() const;
	sal_uIntPtr  			GetContentType() const;
	sal_Bool 			SetContentType( sal_uIntPtr nType );

    LinkManager*          GetLinkManager();
    const LinkManager*    GetLinkManager() const;
    void                    SetLinkManager( LinkManager* _pMgr );

	sal_Bool			Update();
	void			Disconnect();

    // Link impl: DECL_LINK( MyEndDialogHdl, SvBaseLink* ); <= param is this
    virtual void    Edit( Window*, const Link& rEndEditHdl );

		// soll der Link im Dialog angezeigt werden ? (Links im Link im ...)
	sal_Bool 	        IsVisible() const   		{ return bVisible; }
	void 	        SetVisible( sal_Bool bFlag )	{ bVisible = bFlag; }
		// soll der Link synchron oder asynchron geladen werden?
	sal_Bool 	        IsSynchron() const   		{ return bSynchron; }
	void 	        SetSynchron( sal_Bool bFlag )	{ bSynchron = bFlag; }

	sal_Bool 	        IsUseCache() const   		{ return bUseCache; }
	void 			SetUseCache( sal_Bool bFlag )	{ bUseCache = bFlag; }

    void            setStreamToLoadFrom(
                        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,
                        sal_Bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }
    // --> OD 2008-06-18 #i88291#
    void            clearStreamToLoadFrom();
    // <--

    inline sal_Bool         WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper*   GetFileDialog( sal_uInt32 nFlags, const String& rFactory ) const;
};

SV_DECL_IMPL_REF(SvBaseLink);

}

#endif
