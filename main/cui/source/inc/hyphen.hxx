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


#ifndef _SVX_HYPHEN_HXX
#define _SVX_HYPHEN_HXX

// include ---------------------------------------------------------------

#include <memory>

#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/basedlgs.hxx>
#include <com/sun/star/uno/Reference.hxx>

// forward ---------------------------------------------------------------

namespace com{namespace sun{namespace star{
namespace linguistic2{
	class XHyphenator;
	class XPossibleHyphens;
}}}}

class SvxSpellWrapper;

// class SvxHyphenWordDialog ---------------------------------------------

struct SvxHyphenWordDialog_Impl;

class SvxHyphenWordDialog : public SfxModalDialog
{
    std::auto_ptr< SvxHyphenWordDialog_Impl > m_pImpl;

public:
	SvxHyphenWordDialog( const String &rWord, LanguageType nLang,
						 Window* pParent,
                         ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenator >  &xHyphen,
						 SvxSpellWrapper* pWrapper );
    virtual ~SvxHyphenWordDialog();

    void            SetWindowTitle( LanguageType nLang );
    void            SelLeft();
    void            SelRight();
};


#endif

