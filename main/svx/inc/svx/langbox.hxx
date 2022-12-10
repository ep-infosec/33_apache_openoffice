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


#ifndef _SVX_LANGBOX_HXX
#define _SVX_LANGBOX_HXX

// include ---------------------------------------------------------------

#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/lstbox.hxx>
#include "svx/svxdllapi.h"
#include <vcl/image.hxx>
#include <tools/solar.h>
#include <layout/layout.hxx>

class SvtLanguageTable;

#define LANG_LIST_EMPTY             0x0000
#define LANG_LIST_ALL               0x0001
#define LANG_LIST_WESTERN           0x0002
#define LANG_LIST_CTL               0x0004
#define LANG_LIST_CJK               0x0008
#define LANG_LIST_FBD_CHARS         0x0010
#define LANG_LIST_SPELL_AVAIL       0x0020
#define LANG_LIST_HYPH_AVAIL        0x0040
#define LANG_LIST_THES_AVAIL        0x0080
#define LANG_LIST_ONLY_KNOWN        0x0100  // list only locales provided by I18N
#define LANG_LIST_SPELL_USED        0x0200
#define LANG_LIST_HYPH_USED         0x0400
#define LANG_LIST_THES_USED         0x0800
#define LANG_LIST_ALSO_PRIMARY_ONLY 0x1000  // Do not exclude primary-only 
                                            // languages that do not form a 
                                            // locale, such as Arabic as 
                                            // opposed to Arabic-Egypt.


// load language strings from resource
SVX_DLLPUBLIC String 	GetDicInfoStr( const String& rName, const sal_uInt16 nLang, sal_Bool bNeg );

class SVX_DLLPUBLIC SvxLanguageBox : public ListBox
{
public:

private:
	Image					m_aNotCheckedImage;
	Image					m_aCheckedImage;
    Image                   m_aCheckedImageHC;
	String					m_aAllString;
    com::sun::star::uno::Sequence< sal_Int16 >  *m_pSpellUsedLang;
    SvtLanguageTable*       m_pLangTable;
	sal_Int16					m_nLangList;
	sal_Bool                    m_bHasLangNone;
	sal_Bool					m_bLangNoneIsLangAll;
	sal_Bool					m_bWithCheckmark;

    SVX_DLLPRIVATE void                    Init();
    SVX_DLLPRIVATE sal_uInt16                  ImplInsertImgEntry( const String& rEntry, sal_uInt16 nPos, bool bChecked );
    SVX_DLLPRIVATE sal_uInt16                  ImplInsertLanguage(LanguageType, sal_uInt16, sal_Int16 );

public:
    SvxLanguageBox( Window* pParent, WinBits nWinStyle, sal_Bool bCheck = sal_False);
    SvxLanguageBox( Window* pParent, const ResId& rResId, sal_Bool bCheck = sal_False);
	~SvxLanguageBox();

	void			SetLanguageList( sal_Int16 nLangList,
							sal_Bool bHasLangNone, sal_Bool bLangNoneIsLangAll = sal_False,
							sal_Bool bCheckSpellAvail = sal_False );

	sal_uInt16			InsertLanguage( const LanguageType eLangType, sal_uInt16 nPos = LISTBOX_APPEND );
	sal_uInt16			InsertDefaultLanguage( sal_Int16 nType, sal_uInt16 nPos = LISTBOX_APPEND );
    sal_uInt16          InsertLanguage( const LanguageType eLangType,
                            sal_Bool bCheckEntry, sal_uInt16 nPos = LISTBOX_APPEND );
	void			RemoveLanguage( const LanguageType eLangType );
	void			SelectLanguage( const LanguageType eLangType, sal_Bool bSelect = sal_True );
	LanguageType	GetSelectLanguage() const;
	sal_Bool			IsLanguageSelected( const LanguageType eLangType ) const;
};

#if ENABLE_LAYOUT
namespace layout
{
class SvxLanguageBoxImpl;
class SVX_DLLPUBLIC SvxLanguageBox : public ListBox
{
    /*DECL_GET_IMPL( SvxLanguageBox );
    DECL_CONSTRUCTORS( SvxLanguageBox, ListBox, WB_BORDER );
    DECL_GET_WINDOW (SvxLanguageBox);*/

public:
	SvxLanguageBox( Context*, const char*, sal_Bool bCheck = sal_False );
    ~SvxLanguageBox ();
    void SetLanguageList (sal_Int16 list, bool hasLangNone, bool langNoneIsLangAll=false, bool checkSpellAvailable=false);

    sal_uInt16 InsertLanguage (LanguageType const type, sal_uInt16 pos=LISTBOX_APPEND);
    sal_uInt16 InsertLanguage (LanguageType const type, bool checkEntry, sal_uInt16 pos=LISTBOX_APPEND);
    void RemoveLanguage (LanguageType const type);
    void SelectLanguage (LanguageType const type, bool select=true);
    LanguageType GetSelectLanguage () const;
    bool IsLanguageSelected( LanguageType const type) const;
};
};
#endif

#endif

