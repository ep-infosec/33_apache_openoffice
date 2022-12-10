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



#ifndef DBAUI_DBWIZ_HXX
#define DBAUI_DBWIZ_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef DBAUI_ITEMSETHELPER_HXX
#include "IItemSetHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#include <memory>

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

namespace dbaccess
{
    class ODsnTypeCollection;
}
//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= ODbTypeWizDialog
//=========================================================================
class OGeneralPage;
class ODbDataSourceAdministrationHelper;
/** tab dialog for administrating the office wide registered data sources
*/
class ODbTypeWizDialog : public svt::OWizardMachine , public IItemSetHelper, public IDatabaseSettingsDialog,public dbaui::OModuleClient
{
private:
	OModuleClient m_aModuleClient;
	::std::auto_ptr<ODbDataSourceAdministrationHelper>	m_pImpl;
	SfxItemSet*				m_pOutSet;
    ::dbaccess::ODsnTypeCollection*	    
                            m_pCollection;	/// the DSN type collection instance
	::rtl::OUString         m_eType;

	sal_Bool				m_bResetting : 1;	/// sal_True while we're resetting the pages
	sal_Bool				m_bApplied : 1;		/// sal_True if any changes have been applied while the dialog was executing
	sal_Bool				m_bUIEnabled : 1;	/// <TRUE/> if the UI is enabled, false otherwise. Cannot be switched back to <TRUE/>, once it is <FALSE/>

public:
	/** ctor. The itemset given should have been created by <method>createItemSet</method> and should be destroyed
		after the dialog has been destroyed
	*/
	ODbTypeWizDialog(Window* pParent
		,SfxItemSet* _pItems
		,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
		,const ::com::sun::star::uno::Any& _aDataSourceName
		);
	virtual ~ODbTypeWizDialog();

	virtual const SfxItemSet* getOutputSet() const;
	virtual SfxItemSet* getWriteOutputSet();

	// forwards to ODbDataSourceAdministrationHelper
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const;
	virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >	getDriver();
	virtual ::rtl::OUString getDatasourceType(const SfxItemSet& _rSet) const;
	virtual void clearPassword();
	virtual sal_Bool saveDatasource();
	virtual void setTitle(const ::rtl::OUString& _sTitle);
    virtual void enableConfirmSettings( bool _bEnable );

protected:
	/// to override to create new pages
	virtual TabPage*	createPage(WizardState _nState);
	virtual WizardState	determineNextState(WizardState _nCurrentState) const;
	virtual	sal_Bool	leaveState(WizardState _nState);
	virtual ::svt::IWizardPageController*
                        getPageController( TabPage* _pCurrentPage ) const;
	virtual sal_Bool    onFinish();

protected:
	inline sal_Bool	isUIEnabled() const { return m_bUIEnabled; }
	inline void		disabledUI() { m_bUIEnabled = sal_False; }

	/// select a datasource with a given name, adjust the item set accordingly, and everything like that ..
	void implSelectDatasource(const ::rtl::OUString& _rRegisteredName);
	void resetPages(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatasource);

	enum ApplyResult
	{
		AR_LEAVE_MODIFIED,		// somthing was modified and has successfully been committed
		AR_LEAVE_UNCHANGED,		// no changes were made
		AR_KEEP					// don't leave the page (e.g. because an error occurred)
	};
	/** apply all changes made
	*/
	ApplyResult	implApplyChanges();

private:
	DECL_LINK(OnTypeSelected, OGeneralPage*);
};

//.........................................................................
}	// namespace dbaui
//.........................................................................

#endif // DBAUI_DBWIZ_HXX

