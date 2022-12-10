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



#include <com/sun/star/uno/Reference.h>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/svtreebx.hxx>
#include <tools/resary.hxx>
#include <vcl/image.hxx>
#include <vcl/fixbrd.hxx>
#include <vcl/fixed.hxx>

#include <vector>

// static ----------------------------------------------------------------

sal_Bool            EnableSSO();
CreateTabPage   GetSSOCreator( void );

// class OfaOptionsTreeListBox -------------------------------------------

//!#define NUMBER_OF_OPTION_PAGES  12
class SfxModule;
class SfxShell;
class SfxItemSet;
class XColorList;
typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;

class OfaOptionsTreeListBox : public SvTreeListBox
{
	using  SvListView::Collapse;

private:
	sal_Bool 			bInCollapse;

public:
	OfaOptionsTreeListBox(Window* pParent, const ResId& rResId) :
		SvTreeListBox( pParent, rResId ), bInCollapse(sal_False) {}

	virtual sal_Bool    Collapse( SvLBoxEntry* pParent );
	sal_Bool			IsInCollapse()const {return bInCollapse;}
};

// struct OrderedEntry ---------------------------------------------------

struct OrderedEntry
{
    sal_Int32       m_nIndex;
    rtl::OUString   m_sId;

    OrderedEntry( sal_Int32 nIndex, const rtl::OUString& rId ) :
        m_nIndex( nIndex ), m_sId( rId ) {}
};

typedef std::vector< OrderedEntry* > VectorOfOrderedEntries;

// struct Module ---------------------------------------------------------

struct Module
{
    rtl::OUString           m_sName;
    bool                    m_bActive;
    VectorOfOrderedEntries  m_aNodeList;

    Module( const rtl::OUString& rName ) : m_sName( rName ), m_bActive( false ) {}
};

// struct OptionsLeaf ----------------------------------------------------

struct OptionsLeaf
{
    rtl::OUString   m_sId;
    rtl::OUString   m_sLabel;
    rtl::OUString   m_sPageURL;
    rtl::OUString   m_sEventHdl;
    rtl::OUString   m_sGroupId;
    sal_Int32       m_nGroupIndex;

    OptionsLeaf(    const rtl::OUString& rId,
                    const rtl::OUString& rLabel,
                    const rtl::OUString& rPageURL,
                    const rtl::OUString& rEventHdl,
                    const rtl::OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_sEventHdl( rEventHdl ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}
};

typedef ::std::vector< OptionsLeaf* > VectorOfLeaves;
typedef ::std::vector< VectorOfLeaves > VectorOfGroupedLeaves;

// struct OptionsNode ----------------------------------------------------

struct OptionsNode
{
    rtl::OUString           m_sId;
    rtl::OUString           m_sLabel;
    rtl::OUString           m_sPageURL;
    bool                    m_bAllModules;
    rtl::OUString           m_sGroupId;
    sal_Int32               m_nGroupIndex;
    VectorOfLeaves          m_aLeaves;
    VectorOfGroupedLeaves   m_aGroupedLeaves;

    OptionsNode(    const rtl::OUString& rId,
                    const rtl::OUString& rLabel,
                    const rtl::OUString& rPageURL,
                    bool bAllModules,
                    const rtl::OUString& rGroupId,
                    sal_Int32 nGroupIndex ) :
        m_sId( rId ),
        m_sLabel( rLabel ),
        m_sPageURL( rPageURL ),
        m_bAllModules( bAllModules ),
        m_sGroupId( rGroupId ),
        m_nGroupIndex( nGroupIndex ) {}

    ~OptionsNode()
    {
        for ( sal_uInt32 i = 0; i < m_aLeaves.size(); ++i )
            delete m_aLeaves[i];
        m_aLeaves.clear();
        m_aGroupedLeaves.clear();
    }
};

typedef ::std::vector< OptionsNode* > VectorOfNodes;

struct LastPageSaver
{
    sal_uInt16          m_nLastPageId;
    rtl::OUString   m_sLastPageURL_Tools;
    rtl::OUString   m_sLastPageURL_ExtMgr;

    LastPageSaver() : m_nLastPageId( USHRT_MAX ) {}
};

// class OfaTreeOptionsDialog --------------------------------------------

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }
namespace com { namespace sun { namespace star { namespace container { class XNameAccess; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowProvider; } } } }

struct OptionsPageInfo;
struct Module;
class ExtensionsTabPage;
typedef std::vector< ExtensionsTabPage* > VectorOfPages;

class OfaTreeOptionsDialog : public SfxModalDialog
{
private:
	OKButton		aOkPB;
	CancelButton    aCancelPB;
	HelpButton 		aHelpPB;
	PushButton 		aBackPB;

	FixedBorder		aHiddenGB;
	FixedText 		aPageTitleFT;
	FixedLine		aLine1FL;
	FixedText		aHelpFT;
	FixedImage 		aHelpImg;

    ImageList       aPageImages;
    ImageList       aPageImagesHC;

	ResStringArray	aHelpTextsArr;

	OfaOptionsTreeListBox	aTreeLB;

	String 			sTitle;
	String			sNotLoadedError;

    SvLBoxEntry*    pCurrentPageEntry;

	// for the ColorTabPage
	SfxItemSet*		    pColorPageItemSet;
	XColorListSharedPtr maColorTab;
	sal_uInt16			nChangeType;
	sal_uInt16 			nUnknownType;
	sal_uInt16			nUnknownPos;
	sal_Bool			bIsAreaTP;

	sal_Bool			bForgetSelection;
	sal_Bool			bExternBrowserActive;
	sal_Bool			bImageResized;
	bool            bInSelectHdl_Impl;
    bool            bIsFromExtensionManager;

    // check "for the current document only" and set focus to "Western" languages box
    bool            bIsForSetDocumentLanguage;

    Timer           aSelectTimer;

    com::sun::star::uno::Reference < com::sun::star::awt::XContainerWindowProvider >
                    m_xContainerWinProvider;

    static LastPageSaver*   pLastPageSaver;

	SfxItemSet*		CreateItemSet( sal_uInt16 nId );
	void			ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    void            InitTreeAndHandler();
    void            Initialize( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame );
	void			ResizeTreeLB( void );	// resizes dialog so that treelistbox has no horizontal scroll bar

    void            LoadExtensionOptions( const rtl::OUString& rExtensionId );
    rtl::OUString   GetModuleIdentifier( const com::sun::star::uno::Reference<
                                            com::sun::star::lang::XMultiServiceFactory >& xMFac,
                                         const com::sun::star::uno::Reference<
                                            com::sun::star::frame::XFrame >& xFrame );
    Module*         LoadModule( const rtl::OUString& rModuleIdentifier,
                                const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess >& xRoot );
    void            LoadNodes( const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess >& xRoot,
                               Module* pModule,
                               const rtl::OUString& rExtensionId,
                               VectorOfNodes& rOutNodeList );
    void            InsertNodes( const VectorOfNodes& rNodeList );

protected:
    DECL_LINK(ExpandedHdl_Impl, SvTreeListBox* );
    DECL_LINK(ShowPageHdl_Impl, SvTreeListBox* );
    DECL_LINK(BackHdl_Impl, PushButton* );
    DECL_LINK( OKHdl_Impl, Button * );
    DECL_LINK( HintHdl_Impl, Timer * );
    DECL_LINK( SelectHdl_Impl, Timer * );

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual short   Execute();

public:
    OfaTreeOptionsDialog( Window* pParent,
        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& _xFrame,
        bool bActivateLastSelection = true );
    OfaTreeOptionsDialog( Window* pParent, const rtl::OUString& rExtensionId );
    ~OfaTreeOptionsDialog();

    OptionsPageInfo*    AddTabPage( sal_uInt16 nId, const String& rPageName, sal_uInt16 nGroup );
    sal_uInt16              AddGroup(   const String& rGroupName,  SfxShell* pCreateShell,
                                    SfxModule* pCreateModule, sal_uInt16 nDialogId );

    void                ActivateLastSelection();
    void                ActivatePage( sal_uInt16 nResId );
    void                ActivatePage( const String& rPageURL );
    void                ApplyItemSets();

    sal_uInt16              GetColorChanged() const { return nChangeType; }
    XColorListSharedPtr GetColorTable() { return maColorTab; }

	// helper functions to call the language settings TabPage from the SpellDialog
    static void         ApplyLanguageOptions(const SfxItemSet& rSet);
};

// class OfaPageResource -------------------------------------------------

class OfaPageResource : public Resource
{
	ResStringArray		aGeneralDlgAry;
	ResStringArray	 	aInetDlgAry;
    ResStringArray      aLangDlgAry;
    ResStringArray      aTextDlgAry;
	ResStringArray		aHTMLDlgAry;
	ResStringArray		aCalcDlgAry;
	ResStringArray		aStarMathDlgAry;
	ResStringArray		aImpressDlgAry;
	ResStringArray		aDrawDlgAry;
	ResStringArray		aChartDlgAry;
	ResStringArray		aFilterDlgAry;
	ResStringArray		aDatasourcesDlgAry;

public:
	OfaPageResource();

	ResStringArray& GetGeneralArray() 		{return aGeneralDlgAry;}
	ResStringArray& GetInetArray() 			{return aInetDlgAry;}
    ResStringArray& GetLangArray()			{return aLangDlgAry;}
	ResStringArray& GetTextArray() 			{return aTextDlgAry;}
	ResStringArray& GetHTMLArray() 			{return aHTMLDlgAry;}
	ResStringArray& GetCalcArray() 			{return aCalcDlgAry;}
	ResStringArray& GetStarMathArray()		{return aStarMathDlgAry;}
	ResStringArray& GetImpressArray() 		{return aImpressDlgAry;}
	ResStringArray& GetDrawArray() 			{return aDrawDlgAry;}
	ResStringArray& GetChartArray()			{return aChartDlgAry;}
	ResStringArray& GetFilterArray() 		{return aFilterDlgAry;}
	ResStringArray& GetDatasourcesArray()	{return aDatasourcesDlgAry;}
};

// class ExtensionsTabPage -----------------------------------------------

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace awt { class XContainerWindowEventHandler; } } } }

class ExtensionsTabPage : public TabPage
{
private:
    rtl::OUString       m_sPageURL;
    com::sun::star::uno::Reference< com::sun::star::awt::XWindow >
                        m_xPage;
    rtl::OUString       m_sEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowEventHandler >
                        m_xEventHdl;
    com::sun::star::uno::Reference< com::sun::star::awt::XContainerWindowProvider >
                        m_xWinProvider;
    bool                m_bIsWindowHidden;

    void                CreateDialogWithHandler();
    sal_Bool            DispatchAction( const rtl::OUString& rAction );

public:
    ExtensionsTabPage(
        Window* pParent, WinBits nStyle,
        const rtl::OUString& rPageURL, const rtl::OUString& rEvtHdl,
        const com::sun::star::uno::Reference<
            com::sun::star::awt::XContainerWindowProvider >& rProvider );

    virtual ~ExtensionsTabPage();

    virtual void    ActivatePage();
    virtual void    DeactivatePage();

    void            ResetPage();
    void            SavePage();
};

