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



#ifndef SC_ANYREFDG_HXX
#define SC_ANYREFDG_HXX

#ifndef _IMAGEBTN_HXX
#include <vcl/button.hxx>
#endif
#ifndef _EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _ACCEL_HXX
#include <vcl/accel.hxx>
#endif
#include <sfx2/basedlgs.hxx>
#include "address.hxx"
#include "cell.hxx"
#include "compiler.hxx"
#include "formula/funcutl.hxx"
#include "IAnyRefDialog.hxx"
#include "scresid.hxx"
#include <memory>

class SfxObjectShell;
class ScRange;
class ScDocument;
class ScTabViewShell;
//The class of ScAnyRefDlg is rewritten by PengYunQuan for Validity Cell Range Picker
//class ScAnyRefDlg;
class ScRefHandler;
class ScRangeList;
//<!--Added by PengYunQuan for Validity Cell Range Picker
class SfxShell;
#include "scmod.hxx"

typedef    formula::RefButton	ScRefButton;
typedef        formula::RefEdit	ScRefEdit;
//-->Added by PengYunQuan for Validity Cell Range Picker
class ScFormulaReferenceHelper
{
    IAnyRefDialog*      m_pDlg;
    ::std::auto_ptr<ScFormulaCell>      pRefCell;
    ::std::auto_ptr<ScCompiler>         pRefComp;
    formula::RefEdit*    pRefEdit;               // aktives Eingabefeld
    formula::RefButton*  pRefBtn;                // Button dazu
    Window*             m_pWindow;
    SfxBindings*        m_pBindings;
    ::std::auto_ptr<Accelerator>        
                        pAccel;                 // fuer Enter/Escape
    sal_Bool*               pHiddenMarks;           // Merkfeld fuer versteckte Controls
    SCTAB               nRefTab;                // used for ShowReference

    String              sOldDialogText;         // Originaltitel des Dialogfensters
    Size                aOldDialogSize;         // Originalgroesse Dialogfenster
    Point               aOldEditPos;            // Originalposition des Eingabefeldes
    Size                aOldEditSize;           // Originalgroesse des Eingabefeldes
    Point               aOldButtonPos;          // Originalpositiuon des Buttons

    sal_Bool                bEnableColorRef;
    sal_Bool                bHighLightRef;
    sal_Bool                bAccInserted;
    
    DECL_LINK( AccelSelectHdl, Accelerator* );
    
public:
    ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings);
    ~ScFormulaReferenceHelper();

    void                ShowSimpleReference( const XubString& rStr );
    void                ShowFormulaReference( const XubString& rStr );
    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );
    void                Init();

    void                ShowReference( const XubString& rStr );
    void                ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                HideReference( sal_Bool bDoneRefMode = sal_True );
    void                RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void                RefInputDone( sal_Bool bForced = sal_False );
    void                ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    inline void         SetWindow(Window* _pWindow) { m_pWindow = _pWindow; }
    sal_Bool                DoClose( sal_uInt16 nId );
    void                SetDispatcherLock( sal_Bool bLock );
    void                EnableSpreadsheets( sal_Bool bFlag = sal_True, sal_Bool bChilds = sal_True );
    void                ViewShellChanged( ScTabViewShell* pScViewShell );

    static              void enableInput(sal_Bool _bInput);
//<!--Added by PengYunQuan for Validity Cell Range Picker
protected:
	Window		*		GetWindow(){ return m_pWindow; }
public:
	bool				CanInputStart( const ScRefEdit *pEdit ){ return !!pEdit; }
	bool				CanInputDone( sal_Bool bForced ){	return pRefEdit && (bForced || !pRefBtn);	}
//<!--Added by PengYunQuan for Validity Cell Range Picker
};
//============================================================================

//The class of ScAnyRefDlg is rewritten by PengYunQuan for Validity Cell Range Picker
class SC_DLLPUBLIC ScRefHandler : //public SfxModelessDialog,
                    public IAnyRefDialog
{
//<!--Added by PengYunQuan for Validity Cell Range Picker
	Window &	m_rWindow;
	bool		m_bInRefMode;
public:
	operator Window	*(){ return &m_rWindow; }
	Window	* operator ->() { return static_cast<Window	*>(*this); }
	template<class,bool> friend class ScRefHdlrImplBase;
//-->Added by PengYunQuan for Validity Cell Range Picker
    friend class        formula::RefButton;
    friend class        formula::RefEdit;

private:
    ScFormulaReferenceHelper 
                        m_aHelper;
    SfxBindings*        pMyBindings;
    
    Window*             pActiveWin;
    Timer               aTimer;
    String              aDocName;               // document on which the dialog was opened

    DECL_LINK( UpdateFocusHdl, Timer* );
    

protected:
    virtual sal_Bool        DoClose( sal_uInt16 nId );

    void                SetDispatcherLock( sal_Bool bLock );

	//Overwrite TWindow will implemented by ScRefHdlrImplBase
    //virtual long        PreNotify( NotifyEvent& rNEvt );

    virtual void        RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        RefInputDone( sal_Bool bForced = sal_False );
    void                ShowSimpleReference( const XubString& rStr );
    void                ShowFormulaReference( const XubString& rStr );

    bool                ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc );

public:
                        ScRefHandler( Window &rWindow, SfxBindings* pB/*, SfxChildWindow* pCW,
                                     Window* pParent, sal_uInt16 nResId*/, bool bBindRef );
    virtual             ~ScRefHandler();

    virtual void        SetReference( const ScRange& rRef, ScDocument* pDoc ) = 0;
    virtual void        AddRefEntry();

    virtual sal_Bool        IsRefInputMode() const;
    virtual sal_Bool        IsTableLocked() const;
    virtual sal_Bool        IsDocAllowed( SfxObjectShell* pDocSh ) const;

    virtual void        ShowReference( const XubString& rStr );
    virtual void        HideReference( sal_Bool bDoneRefMode = sal_True );

    virtual void        ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void        ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );

    virtual void        ViewShellChanged( ScTabViewShell* pScViewShell );
    void                SwitchToDocument();
    //SfxBindings&        GetBindings();

    virtual void        SetActive() = 0;
//  virtual sal_Bool        Close();
	//Overwrite TWindow will implemented by ScRefHdlrImplBase
    //virtual void        StateChanged( StateChangedType nStateChange );

//<!--Added by PengYunQuan for Validity Cell Range Picker
public:
    bool                EnterRefMode();
    bool                LeaveRefMode();
	inline	bool		CanInputStart( const ScRefEdit *pEdit );
	inline	bool		CanInputDone( sal_Bool bForced );
//-->Added by PengYunQuan for Validity Cell Range Picker
};


//============================================================================
//<!--Added by PengYunQuan for Validity Cell Range Picker
template<  class TWindow, bool bBindRef = true >
class ScRefHdlrImplBase:public TWindow, public ScRefHandler
{
public:
	//Overwrite TWindow
	virtual long        PreNotify( NotifyEvent& rNEvt );
	virtual void        StateChanged( StateChangedType nStateChange );

private:
	template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
	ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
		TParentWindow* pParent, TResId nResId);

	template<class TParentWindow, class TResId, class TArg>
	ScRefHdlrImplBase( TParentWindow* pParent, TResId nResId, const TArg &rArg, SfxBindings *pB = NULL );

	~ScRefHdlrImplBase();

	template<class, class, bool> friend struct ScRefHdlrImpl;
};

template<class TWindow, bool bBindRef>
template<class TBindings, class TChildWindow, class TParentWindow, class TResId>
ScRefHdlrImplBase<TWindow, bBindRef>::ScRefHdlrImplBase( TBindings* pB, TChildWindow* pCW,
				 TParentWindow* pParent, TResId nResId):TWindow(pB, pCW, pParent, ScResId(static_cast<sal_uInt16>( nResId ) ) ), ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef ){}

template<class TWindow, bool bBindRef >
template<class TParentWindow, class TResId, class TArg>
ScRefHdlrImplBase<TWindow,bBindRef>::ScRefHdlrImplBase( TParentWindow* pParent, TResId nResIdP, const TArg &rArg, SfxBindings *pB /*= NULL*/ )
:TWindow( pParent, ScResId(static_cast<sal_uInt16>( nResIdP )), rArg ), ScRefHandler( *static_cast<TWindow*>(this), pB, bBindRef ){}

template<class TWindow, bool bBindRef >
ScRefHdlrImplBase<TWindow,bBindRef>::~ScRefHdlrImplBase(){}

//============================================================================
template<class TDerived, class TBase, bool bBindRef = true>
struct ScRefHdlrImpl: ScRefHdlrImplBase<TBase, bBindRef >
{
	enum { UNKNOWN_SLOTID = 0U, SLOTID = UNKNOWN_SLOTID };

	template<class T1, class T2, class T3, class T4>
	ScRefHdlrImpl( const T1 & rt1, const T2 & rt2, const T3 & rt3, const T4 & rt4 ):ScRefHdlrImplBase<TBase, bBindRef >(rt1, rt2, rt3, rt4 )
	{
		SC_MOD()->RegisterRefWindow( static_cast<sal_uInt16>( static_cast<TDerived*>(this)->SLOTID ), this );
	}

	~ScRefHdlrImpl()
	{
		SC_MOD()->UnregisterRefWindow( static_cast<sal_uInt16>( static_cast<TDerived*>(this)->SLOTID ), this );
	}
};
//============================================================================
struct ScAnyRefDlg : ::ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>
{
	template<class T1, class T2, class T3, class T4>
	ScAnyRefDlg( const T1 & rt1, const T2 & rt2, const T3 & rt3, const T4 & rt4 ):ScRefHdlrImpl< ScAnyRefDlg, SfxModelessDialog>(rt1, rt2, rt3, rt4){}
};
//============================================================================

inline bool ScRefHandler::CanInputStart( const ScRefEdit *pEdit )
{
	return m_aHelper.CanInputStart( pEdit );
}

inline	bool ScRefHandler::CanInputDone( sal_Bool bForced )
{
	return m_aHelper.CanInputDone( bForced );
}

template <> SC_DLLPUBLIC void ScRefHdlrImplBase<SfxModelessDialog,true>::StateChanged( StateChangedType nStateChange );
template <> SC_DLLPUBLIC long ScRefHdlrImplBase<SfxModelessDialog,true>::PreNotify( NotifyEvent& rNEvt );
#include <sfx2/tabdlg.hxx>
template <> SC_DLLPUBLIC void ScRefHdlrImplBase<SfxTabDialog,false>::StateChanged( StateChangedType nStateChange );
template <> SC_DLLPUBLIC long ScRefHdlrImplBase<SfxTabDialog,false>::PreNotify( NotifyEvent& rNEvt );

//<!--Added by PengYunQuan for Validity Cell Range Picker
#endif // SC_ANYREFDG_HXX

