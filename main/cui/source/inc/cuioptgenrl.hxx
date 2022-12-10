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


#ifndef _SVX_CUIOPTGENRL_HXX
#define _SVX_CUIOPTGENRL_HXX

// include ---------------------------------------------------------------

#include <svx/optgenrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/group.hxx>

// forward ---------------------------------------------------------------

struct GeneralTabPage_Impl;

#define SfxGeneralTabPage   SvxGeneralTabPage
#define INDEX_NOTSET        ((sal_Int16)-1)

// class SvxUserEdit -----------------------------------------------------

class SvxUserEdit : public Edit
{
private:
    sal_Int16   m_nIndex;
    FixedText*  m_pLabel;

public:
    SvxUserEdit( Window* pParent, const ResId& rResId,
                 sal_Int16 nIndex = INDEX_NOTSET, FixedText* pLabel = NULL ) :
        Edit( pParent, rResId, true ), m_nIndex( nIndex ), m_pLabel( pLabel ) {}

    inline void         SetIndex( sal_Int16 nIndex ) { m_nIndex = nIndex; }
    inline sal_Int16    GetIndex() const { return m_nIndex; }
    inline void         SetLabel( FixedText* pLabel ) { m_pLabel = pLabel; }
    inline FixedText*   GetLabel() const { return m_pLabel; }
};

// class SvxGeneralTabPage -----------------------------------------------

class SvxGeneralTabPage : public SfxTabPage
{
	using TabPage::DeactivatePage;
private:
    FixedLine           aAddrFrm;
	FixedText 			aCompanyLbl;
    SvxUserEdit         aCompanyEdit;
	FixedText 			aNameLbl;
	FixedText 			aNameLblRuss;
	FixedText 			aNameLblEastern;
    SvxUserEdit         aFirstName;
    SvxUserEdit         aFatherName;
    SvxUserEdit         aName;
    SvxUserEdit         aShortName;
	FixedText 			aStreetLbl;
	FixedText 			aStreetLblRuss;
    SvxUserEdit         aStreetEdit;
    SvxUserEdit         aApartmentNrEdit;
	FixedText 			aCityLbl;
    SvxUserEdit         aPLZEdit;
    SvxUserEdit         aCityEdit;
    SvxUserEdit         aUsCityEdit;
    SvxUserEdit         aUsStateEdit;
    SvxUserEdit         aUsZipEdit;
	FixedText			aCountryLbl;
    SvxUserEdit         aCountryEdit;
	FixedText 			aTitlePosLbl;
    SvxUserEdit         aTitleEdit;
    SvxUserEdit         aPositionEdit;
	FixedText 			aPhoneLbl;
    SvxUserEdit         aTelPrivEdit;
    SvxUserEdit         aTelCompanyEdit;
	FixedText 			aFaxMailLbl;
    SvxUserEdit         aFaxEdit;
    SvxUserEdit         aEmailEdit;
	CheckBox			aUseDataCB;

	GeneralTabPage_Impl*	pImpl;

#ifdef _SVX_OPTGENRL_CXX
	DECL_LINK( ModifyHdl_Impl, Edit * );

    sal_Bool            GetAddress_Impl();
    void                SetAddress_Impl();
#endif

protected:
	virtual int			DeactivatePage( SfxItemSet* pSet );

public:
	SvxGeneralTabPage( Window* pParent, const SfxItemSet& rSet );
	~SvxGeneralTabPage();

	static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rAttrSet );

	virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
	virtual	void 		Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_CUIOPTGENRL_HXX


