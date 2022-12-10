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


#ifndef _SVX_THESDLG_HXX
#define _SVX_THESDLG_HXX


#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include "svx/stddlg.hxx"
#include "svx/svxdllapi.h"

#include <memory>


/////////////////////////////////////////////////////////////////

struct SvxThesaurusDialog_Impl;

class SvxThesaurusDialog : public SvxStandardDialog
{
    std::auto_ptr< SvxThesaurusDialog_Impl > m_pImpl;

    SVX_DLLPRIVATE virtual void     Apply();

public:
	SvxThesaurusDialog( Window* pParent,
                        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                        const String &rWord, LanguageType nLanguage );
	~SvxThesaurusDialog();

    void            SetWindowTitle( LanguageType nLanguage );
    String          GetWord();
    sal_uInt16      GetLanguage() const;
};

#endif

