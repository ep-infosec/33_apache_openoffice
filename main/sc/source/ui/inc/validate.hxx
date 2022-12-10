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



#ifndef SC_VALIDATE_HXX
#define SC_VALIDATE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>
//<!--Added by PengYunQuan for Validity Cell Range Picker
#include "anyrefdg.hxx"
//-->Added by PengYunQuan for Validity Cell Range Picker

// ============================================================================

//<!--Added by PengYunQuan for Validity Cell Range Picker
struct	ScRefHandlerCaller{
	virtual ~ScRefHandlerCaller(){}
};
class ScRefHandlerHelper
{
protected:
	ScRefHandlerCaller	*m_pHandler;
	void            (ScRefHandlerCaller::*m_pSetReferenceHdl)( const ScRange& , ScDocument* );
	void            (ScRefHandlerCaller::*m_pSetActiveHdl)();
	void            (ScRefHandlerCaller::*m_pRefInputStartPreHdl)( ScRefEdit* pEdit, ScRefButton* pButton );
	void            (ScRefHandlerCaller::*m_pRefInputStartPostHdl)( ScRefEdit* pEdit, ScRefButton* pButton );
	void            (ScRefHandlerCaller::*m_pRefInputDonePreHdl)();
	void            (ScRefHandlerCaller::*m_pRefInputDonePostHdl)();
	
public:
	typedef void            (ScRefHandlerCaller::*PFUNCSETREFHDLTYPE)( const ScRange& , ScDocument* );
	typedef void			(ScRefHandlerCaller::*PCOMMONHDLTYPE)();
	typedef void			(ScRefHandlerCaller::*PINPUTSTARTDLTYPE)(  ScRefEdit* pEdit, ScRefButton* pButton );

	PFUNCSETREFHDLTYPE	SetSetRefHdl(  PFUNCSETREFHDLTYPE pNewHdl )
	{
		PFUNCSETREFHDLTYPE pOldHdl = m_pSetReferenceHdl;
		m_pSetReferenceHdl = pNewHdl;
		return pOldHdl;
	}

	PCOMMONHDLTYPE	SetSetActHdl(  PCOMMONHDLTYPE pNewHdl )
	{
		PCOMMONHDLTYPE pOldHdl = m_pSetActiveHdl;
		m_pSetActiveHdl = pNewHdl;
		return pOldHdl;
	}

	ScRefHandlerCaller	*SetHandler( ScRefHandlerCaller	*pNewHandler )
	{
		ScRefHandlerCaller	*pOldHandler = m_pHandler;
		m_pHandler = pNewHandler;
		return pOldHandler;
	}
	void	SetRefInputStartPreHdl(	PINPUTSTARTDLTYPE pNewHdl	){	m_pRefInputStartPreHdl = pNewHdl;	}
	void	SetRefInputDonePostHdl( void            (ScRefHandlerCaller::*pNewHdl)()	){	m_pRefInputDonePostHdl = pNewHdl;	}
	void	SetRefInputStartPostHdl(	PINPUTSTARTDLTYPE pNewHdl	){	m_pRefInputStartPostHdl = pNewHdl;	}
	void	SetRefInputDonePreHdl( void            (ScRefHandlerCaller::*pNewHdl)()	){	m_pRefInputDonePreHdl = pNewHdl;	}

	ScRefHandlerHelper():m_pHandler(NULL), m_pSetReferenceHdl( NULL ), m_pSetActiveHdl(NULL),  m_pRefInputStartPreHdl( NULL ), m_pRefInputStartPostHdl( NULL ), m_pRefInputDonePreHdl( NULL ),	m_pRefInputDonePostHdl( NULL ){}
};
//-->Added by PengYunQuan for Validity Cell Range Picker

/** The "Validity" tab dialog. */
//<!--Modified by PengYunQuan for Validity Cell Range Picker
//class ScValidationDlg : public SfxTabDialog
class ScValidationDlg :public ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false>, public ScRefHandlerHelper
//-->Modified by PengYunQuan for Validity Cell Range Picker
{
	//<!--Added by PengYunQuan for Validity Cell Range Picker
	typedef ScRefHdlrImpl<ScValidationDlg, SfxTabDialog, false>	ScValidationDlgBase;

	DECL_LINK( OkHdl, Button * );

	bool	m_bOwnRefHdlr:1;

	ScTabViewShell *m_pTabVwSh;
	bool	m_bRefInputting:1;
	bool	EnterRefStatus();
	bool	LeaveRefStatus();
	//-->Added by PengYunQuan for Validity Cell Range Picker
public:
	//<!--Modified by PengYunQuan for Validity Cell Range Picker
    //explicit                    ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet );
	explicit ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet, ScTabViewShell * pTabViewSh, SfxBindings *pB = NULL );
    //-->Modified by PengYunQuan for Validity Cell Range Picker
	//<!--Added by PengYunQuan for Validity Cell Range Picker
	virtual                     ~ScValidationDlg();
	inline static ScValidationDlg * Find1AliveObject( Window *pAncestor );
	bool	IsAlive();
	inline	ScTabViewShell * GetTabViewShell();

	bool	SetupRefDlg();
	bool	RemoveRefDlg( sal_Bool bRestoreModal = sal_True );

	virtual void			SetModal( sal_Bool bModal ){ ScValidationDlgBase::SetModalInputMode( bModal ); }

	virtual void            SetReference( const ScRange& rRef, ScDocument* pDoc )
	{
		if ( m_pHandler && m_pSetReferenceHdl )
			(m_pHandler->*m_pSetReferenceHdl)( rRef, pDoc );
	}

	virtual void            SetActive()
	{
		if ( m_pHandler && m_pSetActiveHdl )
			(m_pHandler->*m_pSetActiveHdl)();
	}

	void		CloseRefDialog()
	{
		DBG_ASSERT( false, "should not execute here!!!when the edit kill focus, should remove refhandler.\r\n" );

		if ( IsInExecute() )
			EndDialog( sal_False );
		else if ( GetStyle() & WB_CLOSEABLE )
			Close();
	}

	bool IsRefInputting(){	return m_bRefInputting;	}

	virtual void        RefInputStart( ScRefEdit* pEdit, ScRefButton* pButton = NULL )
	{
		if( !CanInputStart( pEdit ) )
			return;

		if ( m_pHandler && m_pRefInputStartPreHdl )
			(m_pHandler->*m_pRefInputStartPreHdl)( pEdit, pButton );
		m_bRefInputting = true;
		ScValidationDlgBase::RefInputStart( pEdit, pButton );
		if ( m_pHandler && m_pRefInputStartPostHdl )
			(m_pHandler->*m_pRefInputStartPostHdl)( pEdit, pButton );
	}

	virtual void        RefInputDone( sal_Bool bForced = sal_False )
	{
		if( !CanInputDone( bForced ) )
			return;

		if ( m_pHandler && m_pRefInputDonePreHdl )
			(m_pHandler->*m_pRefInputDonePreHdl)();

		ScValidationDlgBase::RefInputDone( bForced );
		m_bRefInputting = false;

		if ( m_pHandler && m_pRefInputDonePostHdl )
			(m_pHandler->*m_pRefInputDonePostHdl)();
	}

	sal_Bool IsChildFocus();

	enum { SLOTID = SID_VALIDITY_REFERENCE };

	sal_Bool Close();
	//-->Added by PengYunQuan for Validity Cell Range Picker
};


// ============================================================================

/** The tab page "Criteria" from the Validation dialog. */
//<!--Modified by PengYunQuan for Validity Cell Range Picker
//class ScTPValidationValue : public SfxTabPage
class ScTPValidationValue : public ScRefHandlerCaller, public SfxTabPage
//-->Modified by PengYunQuan for Validity Cell Range Picker
{
public:
    explicit                    ScTPValidationValue( Window* pParent, const SfxItemSet& rArgSet );
    virtual                     ~ScTPValidationValue();

    static SfxTabPage*          Create( Window* pParent, const SfxItemSet& rArgSet );
    static sal_uInt16*              GetRanges();

    virtual sal_Bool                FillItemSet( SfxItemSet& rArgSet );
    virtual void                Reset( const SfxItemSet& rArgSet );

private:
    void                        Init();

    String                      GetFirstFormula() const;
    String                      GetSecondFormula() const;

    void                        SetFirstFormula( const String& rFmlaStr );
    void                        SetSecondFormula( const String& rFmlaStr );

                                DECL_LINK( SelectHdl, ListBox* );
                                DECL_LINK( CheckHdl, CheckBox* );

    FixedText                   maFtAllow;
    ListBox                     maLbAllow;
    CheckBox                    maCbAllow;      /// Allow blank cells.
    CheckBox                    maCbShow;       /// Show selection list in cell.
    CheckBox                    maCbSort;       /// Sort selection list in cell.
    FixedText                   maFtValue;
    ListBox                     maLbValue;
    FixedText                   maFtMin;
	//<!--Modified by PengYunQuan for Validity Cell Range Picker
    //Edit                        maEdMin;
    ScRefEdit                        maEdMin;
	//-->Modified by PengYunQuan for Validity Cell Range Picker
    MultiLineEdit               maEdList;       /// Entries for explicit list
    FixedText                   maFtMax;
    //<!--Modified by PengYunQuan for Validity Cell Range Picker
	//Edit                        maEdMax;
    ScRefEdit                        maEdMax;
	//-->Modified by PengYunQuan for Validity Cell Range Picker
    FixedText                   maFtHint;       /// Hint text for cell range validity.

    String                      maStrMin;
    String                      maStrMax;
    String                      maStrValue;
    String                      maStrRange;
    String                      maStrList;
    sal_Unicode                 mcFmlaSep;      /// List separator in formulas.
	//<!--Added by PengYunQuan for Validity Cell Range Picker
	DECL_LINK( EditSetFocusHdl, Edit *);
	DECL_LINK( KillFocusHdl, Window *);
	void	OnClick( Button *pBtn );
	ScRefEdit	*m_pRefEdit;
	class ScRefButtonEx:public ::ScRefButton
	{
		void Click();
	public:
		ScRefButtonEx( Window* pParent, const ResId& rResId, ScRefEdit* pEdit = NULL, ScRefHandler *pRefHdlr = NULL ): ::ScRefButton( pParent, rResId, pEdit, pRefHdlr ){}
	}m_btnRef;
	friend class ScRefButtonEx;
	void            SetReferenceHdl( const ScRange& , ScDocument* );
	void            SetActiveHdl();
	void			RefInputStartPreHdl( ScRefEdit* pEdit, ScRefButton* pButton );
	void			RefInputDonePreHdl();
	void			RefInputDonePostHdl();
	ScValidationDlg * GetValidationDlg();

	//TYPEINFO();
	void			TidyListBoxes();
public:
	sal_uInt16			GetAllowEntryPos();
	String			GetMinText();
	void	SetupRefDlg();
	void	RemoveRefDlg();
	//-->Added by PengYunQuan for Validity Cell Range Picker
};


//==================================================================

class ScTPValidationHelp : public SfxTabPage
{
private:
	TriStateBox		aTsbHelp;
    FixedLine       aFlContent;
	FixedText		aFtTitle;
	Edit			aEdtTitle;
	FixedText		aFtInputHelp;
	MultiLineEdit	aEdInputHelp;

    const SfxItemSet& mrArgSet;

	void	Init();

	// Handler ------------------------
	// DECL_LINK( SelectHdl, ListBox * );

public:
			ScTPValidationHelp( Window* pParent, const SfxItemSet& rArgSet );
			~ScTPValidationHelp();

	static	SfxTabPage*	Create		( Window* pParent, const SfxItemSet& rArgSet );
	static	sal_uInt16*		GetRanges	();
	virtual	sal_Bool		FillItemSet	( SfxItemSet& rArgSet );
	virtual	void		Reset		( const SfxItemSet& rArgSet );
};

//==================================================================

class ScTPValidationError : public SfxTabPage
{
private:
	TriStateBox		aTsbShow;
    FixedLine       aFlContent;
	FixedText		aFtAction;
	ListBox			aLbAction;
	PushButton		aBtnSearch;
	FixedText		aFtTitle;
	Edit			aEdtTitle;
	FixedText		aFtError;
	MultiLineEdit	aEdError;

    const SfxItemSet& mrArgSet;

	void	Init();

	// Handler ------------------------
    DECL_LINK( SelectActionHdl, ListBox * );
	DECL_LINK( ClickSearchHdl, PushButton * );

public:
			ScTPValidationError( Window* pParent, const SfxItemSet& rArgSet );
			~ScTPValidationError();

	static	SfxTabPage*	Create		( Window* pParent, const SfxItemSet& rArgSet );
	static	sal_uInt16*		GetRanges	();
	virtual	sal_Bool		FillItemSet	( SfxItemSet& rArgSet );
	virtual	void		Reset		( const SfxItemSet& rArgSet );
};

//<!--Added by PengYunQuan for Validity Cell Range Picker
inline ScTabViewShell *ScValidationDlg::GetTabViewShell()
{
	return m_pTabVwSh;
}

inline ScValidationDlg * ScValidationDlg::Find1AliveObject( Window *pAncestor )
{
	return static_cast<ScValidationDlg *>( SC_MOD()->Find1RefWindow( SLOTID, pAncestor ) );
}
//-->Added by PengYunQuan for Validity Cell Range Picker
#endif // SC_VALIDATE_HXX

