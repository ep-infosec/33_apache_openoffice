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



#ifndef _EXTENSIONS_DBP_GRIDWIZARD_HXX_
#define _EXTENSIONS_DBP_GRIDWIZARD_HXX_

#include "controlwizard.hxx"
#include "commonpagesdbp.hxx"

//.........................................................................
namespace dbp
{
//.........................................................................

	//=====================================================================
	//= OGridSettings
	//=====================================================================
	struct OGridSettings : public OControlWizardSettings
	{
		::com::sun::star::uno::Sequence< ::rtl::OUString >		aSelectedFields;
	};

	//=====================================================================
	//= OGridWizard
	//=====================================================================
	class OGridWizard : public OControlWizard
	{
	protected:
		OGridSettings	m_aSettings;
		sal_Bool		m_bHadDataSelection : 1;

	public:
		OGridWizard(
			Window* _pParent,
			const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
			const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
		);

		OGridSettings& getSettings() { return m_aSettings; }

	protected:
		// OWizardMachine overridables
        virtual ::svt::OWizardPage*	createPage( WizardState _nState );
		virtual WizardState		    determineNextState( WizardState _nCurrentState ) const;
		virtual	void				enterState( WizardState _nState );
		virtual	sal_Bool			leaveState( WizardState _nState );
		virtual sal_Bool            onFinish();

		virtual sal_Bool approveControl(sal_Int16 _nClassId);

	protected:
		void implApplySettings();
	};

	//=====================================================================
	//= OGridPage
	//=====================================================================
	class OGridPage : public OControlWizardPage
	{
	public:
		OGridPage( OGridWizard* _pParent, const ResId& _rId ) : OControlWizardPage(_pParent, _rId) { }

	protected:
		OGridSettings& getSettings() { return static_cast<OGridWizard*>(getDialog())->getSettings(); }
	};

	//=====================================================================
	//= OGridFieldsSelection
	//=====================================================================
	class OGridFieldsSelection : public OGridPage
	{
	protected:
		FixedLine		m_aFrame;
		FixedText		m_aExistFieldsLabel;
		ListBox			m_aExistFields;
		PushButton		m_aSelectOne;
		PushButton		m_aSelectAll;
		PushButton		m_aDeselectOne;
		PushButton		m_aDeselectAll;
		FixedText		m_aSelFieldsLabel;
		ListBox			m_aSelFields;

	public:
		OGridFieldsSelection( OGridWizard* _pParent );

	protected:
		// TabPage overridables
		virtual void ActivatePage();

		// OWizardPage overridables
		virtual void		initializePage();
		virtual sal_Bool	commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
		virtual bool        canAdvance() const;

	protected:
		DECL_LINK(OnMoveOneEntry, PushButton*);
		DECL_LINK(OnMoveAllEntries, PushButton*);
		DECL_LINK(OnEntrySelected, ListBox*);
		DECL_LINK(OnEntryDoubleClicked, ListBox*);

		void implCheckButtons();
		void implApplySettings();

		::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > implGetColumns(sal_Bool _bShowError = sal_True);
	};

//.........................................................................
}	// namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_GRIDWIZARD_HXX_

