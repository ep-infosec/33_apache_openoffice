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


#ifndef _SC_DLGFACT_HXX
#define _SC_DLGFACT_HXX

// include ---------------------------------------------------------------
#include "scabstdlg.hxx" //CHINA001
class Dialog;
class ScImportAsciiDlg;
class ScAutoFormatDlg;
class ScColRowLabelDlg;
class ScDataPilotDatabaseDlg;
class ScDataPilotSourceTypeDlg;
class ScDataPilotServiceDlg;
class ScDeleteCellDlg;
class ScDeleteContentsDlg;
class ScFillSeriesDlg;
class ScGroupDlg;
class ScInsertCellDlg;
class ScInsertContentsDlg;
class ScInsertTableDlg;
class ScSelEntryDlg;
class ScLinkedAreaDlg;
class ScMetricInputDlg;
class ScMoveTableDlg;
class ScNameCreateDlg;
class ScNamePasteDlg;
class ScPivotFilterDlg;
class ScDPFunctionDlg;
class ScDPSubtotalDlg;
class ScDPNumGroupDlg;
class ScDPDateGroupDlg;
class ScDPShowDetailDlg;
class ScNewScenarioDlg;
class ScShowTabDlg;
class ScStringInputDlg;
class ScTabBgColorDlg;
class ScImportOptionsDlg;
class SfxTabDialog;
class ScSortWarningDlg;
class ScTextImportOptionsDlg;

#define DECL_ABSTDLG_BASE(Class,DialogClass)		\
	DialogClass* 		pDlg;                   	\
public:												\
					Class( DialogClass* p)			\
					 : pDlg(p)						\
					 {}                             \
	virtual			~Class();               		\
    virtual short	Execute() ;
//	virtual void	Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 )

#define DECL_ABSTDLG2_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual void    StartExecuteModal( const Link& rEndDialogHdl ); \
    long            GetResult();

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                    					\
{                                                   \
	delete pDlg;                                 	\
}													\
short Class::Execute()                             \
{                                                   \
	return pDlg->Execute();                      	\
}

#define IMPL_ABSTDLG2_BASE(Class)                   \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
void Class::StartExecuteModal( const Link& rEndDialogHdl ) \
{                                                   \
    pDlg->StartExecuteModal( rEndDialogHdl ) ;      \
}                                                   \
long Class::GetResult()                             \
{                                                   \
    return pDlg->GetResult();                       \
}

class VclAbstractDialog_Impl : public VclAbstractDialog //add for ScColOrRowDlg
{
	DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
};

class AbstractScImportAsciiDlg_Impl : public AbstractScImportAsciiDlg  //add for ScImportAsciiDlg
{
    DECL_ABSTDLG_BASE(AbstractScImportAsciiDlg_Impl, ScImportAsciiDlg)
    virtual void                        GetOptions( ScAsciiOptions& rOpt );
    virtual void                        SetTextToColumnsMode();
    virtual void                        SaveParameters();
};

class AbstractScAutoFormatDlg_Impl : public AbstractScAutoFormatDlg  //add for ScAutoFormatDlg
{
	DECL_ABSTDLG_BASE(AbstractScAutoFormatDlg_Impl, ScAutoFormatDlg)
	virtual sal_uInt16 GetIndex() const;
	virtual String GetCurrFormatName();
};

class AbstractScColRowLabelDlg_Impl : public AbstractScColRowLabelDlg  //add for ScColRowLabelDlg
{
	DECL_ABSTDLG_BASE(AbstractScColRowLabelDlg_Impl,ScColRowLabelDlg)
	virtual sal_Bool IsCol();
	virtual sal_Bool IsRow();
};

class AbstractScDataPilotDatabaseDlg_Impl  :public AbstractScDataPilotDatabaseDlg  //add for ScDataPilotDatabaseDlg
{
	DECL_ABSTDLG_BASE(AbstractScDataPilotDatabaseDlg_Impl, ScDataPilotDatabaseDlg)
	virtual void	GetValues( ScImportSourceDesc& rDesc );
};

class AbstractScDataPilotSourceTypeDlg_Impl  :public AbstractScDataPilotSourceTypeDlg  //add for ScDataPilotSourceTypeDlg
{
	DECL_ABSTDLG_BASE(AbstractScDataPilotSourceTypeDlg_Impl, ScDataPilotSourceTypeDlg)
	virtual sal_Bool	IsDatabase() const;
	virtual sal_Bool	IsExternal() const;
};

class AbstractScDataPilotServiceDlg_Impl : public AbstractScDataPilotServiceDlg  //add for ScDataPilotServiceDlg
{
	DECL_ABSTDLG_BASE(AbstractScDataPilotServiceDlg_Impl, ScDataPilotServiceDlg)
	virtual String	GetServiceName() const;
	virtual String	GetParSource() const;
	virtual String	GetParName() const;
	virtual String	GetParUser() const;
	virtual String	GetParPass() const;
};

class AbstractScDeleteCellDlg_Impl : public AbstractScDeleteCellDlg  //add for ScDeleteCellDlg
{
	DECL_ABSTDLG_BASE(AbstractScDeleteCellDlg_Impl,ScDeleteCellDlg)
	virtual DelCellCmd GetDelCellCmd() const;
};

class AbstractScDeleteContentsDlg_Impl : public AbstractScDeleteContentsDlg  //add for ScDeleteContentsDlg
{
	DECL_ABSTDLG_BASE( AbstractScDeleteContentsDlg_Impl,ScDeleteContentsDlg)
	virtual void	DisableObjects();
	virtual sal_uInt16	GetDelContentsCmdBits() const;
};

class AbstractScFillSeriesDlg_Impl:public AbstractScFillSeriesDlg  //add for ScFillSeriesDlg
{
	DECL_ABSTDLG_BASE(AbstractScFillSeriesDlg_Impl, ScFillSeriesDlg)
	virtual FillDir		GetFillDir() const;
	virtual FillCmd		GetFillCmd() const;
	virtual FillDateCmd	GetFillDateCmd() const;
	virtual double		GetStart() const;
	virtual double		GetStep() const;
	virtual double		GetMax() const;
	virtual String		GetStartStr() const;
	virtual void		SetEdStartValEnabled(sal_Bool bFlag=sal_False);
};

class AbstractScGroupDlg_Impl :  public AbstractScGroupDlg  //add for ScGroupDlg
{
	DECL_ABSTDLG_BASE( AbstractScGroupDlg_Impl, ScGroupDlg)
	virtual sal_Bool GetColsChecked() const;
};

class AbstractScInsertCellDlg_Impl : public AbstractScInsertCellDlg  //add for ScInsertCellDlg
{
	DECL_ABSTDLG_BASE( AbstractScInsertCellDlg_Impl, ScInsertCellDlg)
	virtual InsCellCmd GetInsCellCmd() const ;
};

class AbstractScInsertContentsDlg_Impl : public AbstractScInsertContentsDlg  //add for ScInsertContentsDlg
{
	DECL_ABSTDLG_BASE(AbstractScInsertContentsDlg_Impl, ScInsertContentsDlg)
	virtual sal_uInt16		GetInsContentsCmdBits() const;
	virtual sal_uInt16		GetFormulaCmdBits() const;
	virtual sal_Bool		IsSkipEmptyCells() const;
	virtual sal_Bool 		IsLink() const;
	virtual void	SetFillMode( sal_Bool bSet );
	virtual void	SetOtherDoc( sal_Bool bSet );
	virtual sal_Bool 		IsTranspose() const;
	virtual void	SetChangeTrack( sal_Bool bSet );
	virtual void	SetCellShiftDisabled( int nDisable );
	virtual InsCellCmd	GetMoveMode();
};

class AbstractScInsertTableDlg_Impl : public AbstractScInsertTableDlg  //add for ScInsertTableDlg
{
	DECL_ABSTDLG_BASE( AbstractScInsertTableDlg_Impl, ScInsertTableDlg)
	virtual sal_Bool			GetTablesFromFile();
	virtual sal_Bool			GetTablesAsLink();
	virtual const String*	GetFirstTable( sal_uInt16* pN = NULL );
	virtual ScDocShell*		GetDocShellTables();
	virtual sal_Bool			IsTableBefore();
	virtual sal_uInt16			GetTableCount();
	virtual const String*	GetNextTable( sal_uInt16* pN = NULL );

};

class AbstractScSelEntryDlg_Impl : public AbstractScSelEntryDlg  //add for ScSelEntryDlg
{
	DECL_ABSTDLG_BASE( AbstractScSelEntryDlg_Impl, ScSelEntryDlg )
	virtual String GetSelectEntry() const;
};

class AbstractScLinkedAreaDlg_Impl : public AbstractScLinkedAreaDlg  //add for ScLinkedAreaDlg
{
    DECL_ABSTDLG2_BASE( AbstractScLinkedAreaDlg_Impl, ScLinkedAreaDlg)

	virtual void			InitFromOldLink( const String& rFile, const String& rFilter,
										const String& rOptions, const String& rSource,
										sal_uLong nRefresh );
	virtual String			GetURL();
	virtual String			GetFilter();		// may be empty
	virtual String			GetOptions();		// filter options
	virtual String			GetSource();		// separated by ";"
	virtual sal_uLong			GetRefresh();		// 0 if disabled
};

class AbstractScMetricInputDlg_Impl : public AbstractScMetricInputDlg  //add for ScMetricInputDlg
{
	DECL_ABSTDLG_BASE( AbstractScMetricInputDlg_Impl, ScMetricInputDlg)
	virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const;
};

class AbstractScMoveTableDlg_Impl : public AbstractScMoveTableDlg  //add for ScMoveTableDlg
{
	DECL_ABSTDLG_BASE( AbstractScMoveTableDlg_Impl, ScMoveTableDlg)
	virtual sal_uInt16	GetSelectedDocument		() const;
	virtual sal_uInt16	GetSelectedTable		() const;
	virtual sal_Bool	GetCopyTable			() const;
	virtual void	SetCopyTable			(sal_Bool bFlag=sal_True);
	virtual void	EnableCopyTable			(sal_Bool bFlag=sal_True);
};

class AbstractScNameCreateDlg_Impl : public AbstractScNameCreateDlg  //add for ScNameCreateDlg
{
	DECL_ABSTDLG_BASE( AbstractScNameCreateDlg_Impl, ScNameCreateDlg)
	virtual sal_uInt16			GetFlags() const;
};

class AbstractScNamePasteDlg_Impl : public AbstractScNamePasteDlg  //add for ScNamePasteDlg
{
	DECL_ABSTDLG_BASE( AbstractScNamePasteDlg_Impl, ScNamePasteDlg )
	virtual String			GetSelectedName() const;
};

class AbstractScPivotFilterDlg_Impl : public AbstractScPivotFilterDlg  //add for ScPivotFilterDlg
{
	DECL_ABSTDLG_BASE( AbstractScPivotFilterDlg_Impl, ScPivotFilterDlg)
	virtual const ScQueryItem&	GetOutputItem();
};

class AbstractScDPFunctionDlg_Impl : public AbstractScDPFunctionDlg  //add for ScDPFunctionDlg
{
	DECL_ABSTDLG_BASE( AbstractScDPFunctionDlg_Impl, ScDPFunctionDlg)
    virtual sal_uInt16 GetFuncMask() const;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const;
};

class AbstractScDPSubtotalDlg_Impl : public AbstractScDPSubtotalDlg  //add for ScDPSubtotalDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPSubtotalDlg_Impl, ScDPSubtotalDlg)
    virtual sal_uInt16 GetFuncMask() const;
    virtual void FillLabelData( ScDPLabelData& rLabelData ) const;
};

class AbstractScDPNumGroupDlg_Impl : public AbstractScDPNumGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPNumGroupDlg_Impl, ScDPNumGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const;
};

class AbstractScDPDateGroupDlg_Impl : public AbstractScDPDateGroupDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPDateGroupDlg_Impl, ScDPDateGroupDlg )
    virtual ScDPNumGroupInfo GetGroupInfo() const;
    virtual sal_Int32 GetDatePart() const;
};

class AbstractScDPShowDetailDlg_Impl : public AbstractScDPShowDetailDlg
{
    DECL_ABSTDLG_BASE( AbstractScDPShowDetailDlg_Impl, ScDPShowDetailDlg)
    virtual String GetDimensionName() const;
};

class AbstractScNewScenarioDlg_Impl : public AbstractScNewScenarioDlg  //add for ScNewScenarioDlg
{
	DECL_ABSTDLG_BASE( AbstractScNewScenarioDlg_Impl, ScNewScenarioDlg )
	virtual void SetScenarioData( const String& rName, const String& rComment,
							const Color& rColor, sal_uInt16 nFlags );

	virtual void GetScenarioData( String& rName, String& rComment,
							Color& rColor, sal_uInt16& rFlags ) const;
};

class AbstractScShowTabDlg_Impl : public AbstractScShowTabDlg  //add for ScShowTabDlg
{
	DECL_ABSTDLG_BASE(AbstractScShowTabDlg_Impl,ScShowTabDlg)
    virtual void    Insert( const String& rString, sal_Bool bSelected );
	virtual sal_uInt16	GetSelectEntryCount() const;
	virtual void SetDescription(const String& rTitle, const String& rFixedText, const rtl::OString& sDlgHelpId, const rtl::OString& sLbHelpId );
	virtual String	GetSelectEntry(sal_uInt16 nPos) const;
    virtual sal_uInt16  GetSelectEntryPos(sal_uInt16 nPos) const;
};

class AbstractScStringInputDlg_Impl :  public AbstractScStringInputDlg  //add for ScStringInputDlg
{
	DECL_ABSTDLG_BASE( AbstractScStringInputDlg_Impl, ScStringInputDlg )
	virtual void GetInputString( String& rString ) const;
};

class AbstractScTabBgColorDlg_Impl :  public AbstractScTabBgColorDlg  //add for ScTabBgColorDlg
{
    DECL_ABSTDLG_BASE( AbstractScTabBgColorDlg_Impl, ScTabBgColorDlg )
    virtual void GetSelectedColor( Color& rColor ) const;
};

class AbstractScImportOptionsDlg_Impl : public AbstractScImportOptionsDlg  //add for ScImportOptionsDlg
{
	DECL_ABSTDLG_BASE( AbstractScImportOptionsDlg_Impl, ScImportOptionsDlg)
	virtual void GetImportOptions( ScImportOptions& rOptions ) const;
};

class AbstractScTextImportOptionsDlg_Impl : public AbstractScTextImportOptionsDlg
{
    DECL_ABSTDLG_BASE( AbstractScTextImportOptionsDlg_Impl, ScTextImportOptionsDlg)
    virtual LanguageType GetLanguageType() const;
    virtual bool IsDateConversionSet() const;
};

//add for ScAttrDlg , ScHFEditDlg, ScStyleDlg, ScSubTotalDlg, ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
	DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
	virtual void				SetCurPageId( sal_uInt16 nId );
	virtual const SfxItemSet* 	GetOutputItemSet() const;
	virtual const sal_uInt16*		GetInputRanges( const SfxItemPool& pItem );	//add by CHINA001
	virtual void				SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
		//From class Window.
    virtual void		SetText( const XubString& rStr ); //add by CHINA001
    virtual String   	GetText() const; //add by CHINA001
};
#if ENABLE_LAYOUT
namespace layout
{
//add for ScAttrDlg , ScHFEditDlg, ScStyleDlg, ScSubTotalDlg, ScCharDlg, ScParagraphDlg, ScValidationDlg, ScSortDlg
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
	DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
	virtual void				SetCurPageId( sal_uInt16 nId );
	virtual const SfxItemSet* 	GetOutputItemSet() const;
	virtual const sal_uInt16*		GetInputRanges( const SfxItemPool& pItem );	//add by CHINA001
	virtual void				SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
		//From class Window.
    virtual void		SetText( const XubString& rStr ); //add by CHINA001
    virtual String   	GetText() const; //add by CHINA001
};
} // end namespace layout
#endif /* ENABLE_LAYOUT */
//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
	//CHINA001 AbstractSwSaveLabelDlg* 			CreateSwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec, int nId );

	virtual 	AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName, //add for ScImportAsciiDlg
																	SvStream* pInStream, int nId,
																	sal_Unicode cSep = '\t');

    virtual AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg( Window* pParent, int nId );

	virtual		AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*					pParent, //add for ScAutoFormatDlg
																ScAutoFormat*				pAutoFormat,
																const ScAutoFormatData*    pSelFormatData,
																ScDocument*                pDoc,
																int nId);
	virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent, //add for ScColRowLabelDlg
																int nId,
																sal_Bool bCol = sal_False,
																sal_Bool bRow = sal_False);

	virtual VclAbstractDialog * CreateScColOrRowDlg( Window*			pParent, //add for ScColOrRowDlg
													const String&	rStrTitle,
													const String&	rStrLabel,
													int nId,
													sal_Bool				bColDefault = sal_True );

    virtual VclAbstractDialog * CreateScSortWarningDlg( Window* pParent, const String& rExtendText, const String& rCurrentText, int nId );

	virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,int nId ); //add for ScDataPilotDatabaseDlg

	virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg (  Window* pParent, sal_Bool bEnableExternal, int nId ) ; //add for ScDataPilotSourceTypeDlg

	virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent, //add for ScDataPilotServiceDlg
																		const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
																		int nId );
	virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg( Window* pParent, int nId,
															sal_Bool bDisallowCellMove = sal_False ); //add for ScDeleteCellDlg

	virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,int nId, //add for ScDeleteContentsDlg
																 sal_uInt16  nCheckDefaults = 0 );

	virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*		pParent, //add for ScFillSeriesDlg
															ScDocument&	rDocument,
															FillDir		eFillDir,
															FillCmd		eFillCmd,
															FillDateCmd	eFillDateCmd,
															String			aStartStr,
															double			fStep,
															double			fMax,
															sal_uInt16			nPossDir,
															int nId);
	virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent, //add for ScGroupDlg
															sal_uInt16	nResId,
															int nId,
															sal_Bool	bUnGroup = sal_False,
															sal_Bool	bRows    = sal_True  );

	virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
																int nId,
															sal_Bool bDisallowCellMove = sal_False );

	virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*		pParent, //add for ScInsertContentsDlg
																	int nId,
																	sal_uInt16  		nCheckDefaults = 0,
																	const String*	pStrTitle = NULL );

	virtual AbstractScInsertTableDlg * CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,  //add for ScInsertTableDlg
                                                                SCTAB nTabCount, bool bFromFile, int nId);

	virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent, // add for ScSelEntryDlg
															sal_uInt16	nResId,
														const String& aTitle,
														const String& aLbTitle,
																List& 	aEntryList,
															int nId );

	virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg (  Window* pParent, //add for ScLinkedAreaDlg
																int nId);

	virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  Window*		pParent, //add for ScMetricInputDlg
																sal_uInt16		nResId,		// Ableitung fuer jeden Dialog!
																long			nCurrent,
																long			nDefault,
																int nId ,
																FieldUnit		eFUnit	  = FUNIT_MM,
																sal_uInt16		nDecimals = 2,
																long			nMaximum  = 1000,
																long			nMinimum  = 0,
																long			nFirst	  = 1,
																long          nLast     = 100 );

	virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(  Window* pParent, int nId ); //add for ScMoveTableDlg
	virtual AbstractScNameCreateDlg * CreateScNameCreateDlg ( Window * pParent, sal_uInt16 nFlags, int nId ); //add for ScNameCreateDlg

	virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList, //add for ScNamePasteDlg
															int nId , sal_Bool bInsList=sal_True );

	virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent, //add for ScPivotFilterDlg
																const SfxItemSet&	rArgSet, sal_uInt16 nSourceTab , int nId );

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, int nId,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData );

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout );

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo );

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate );

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient );

	virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName, //add for ScNewScenarioDlg
																int nId,
																sal_Bool bEdit = sal_False, sal_Bool bSheetProtected = sal_False );
	virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, int nId ); //add for ScShowTabDlg

	virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent, //add for ScStringInputDlg
																const String& rTitle,
																const String& rEditTitle,
																const String& rDefault,
																const rtl::OString& sHelpId, const rtl::OString& sEditHelpId,
																int nId );

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  Window* pParent, //add for ScStringInputDlg
                                                                const String& rTitle, //Dialog Title
                                                                const String& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const rtl::OString& sHelpId ,
                                                                int nId );

	virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window* 				pParent, //add for ScImportOptionsDlg
																	int nId,
																	sal_Bool                    bAscii = sal_True,
																	const ScImportOptions*	pOptions = NULL,
																	const String*			pStrTitle = NULL,
																	sal_Bool                    bMultiByte = sal_False,
																	sal_Bool                    bOnlyDbtoolsEncodings = sal_False,
																	sal_Bool                    bImport = sal_True );
	virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*	 pFrame, //add for ScAttrDlg
													Window*			 pParent,
													const SfxItemSet* pCellAttrs,
													int nId);

	virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*		pFrame, //add for ScHFEditDlg
													Window*			pParent,
													const SfxItemSet&	rCoreSet,
													const String&		rPageStyle,
													int nId,
													sal_uInt16				nResId = RID_SCDLG_HFEDIT );

	virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*				pParent,//add for ScStyleDlg
													SfxStyleSheetBase&	rStyleBase,
													sal_uInt16				nRscId,
													int nId);

	virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*				pParent, //add for ScSubTotalDlg
														const SfxItemSet*	pArgSet,
														int nId);
	virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,//add for ScCharDlg
													const SfxObjectShell* pDocShell, int nId );

	virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,//add for ScParagraphDlg
															int nId );

	virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
	//<!--Modified by PengYunQuan for Validity Cell Range Picker
	//													const SfxItemSet* pArgSet,int nId  );
														const SfxItemSet* pArgSet,int nId, ScTabViewShell *pTabVwSh  );
	//-->Modified by PengYunQuan for Validity Cell Range Picker

	virtual SfxAbstractTabDialog * CreateScSortDlg( Window*			 pParent, //add for ScSortDlg
													const SfxItemSet* pArgSet,int nId );
	// For TabPage
	virtual CreateTabPage				GetTabPageCreatorFunc( sal_uInt16 nId );

	virtual GetTabPageRanges			GetTabPageRangesFunc( sal_uInt16 nId );

};

//CHINA001 struct ScDialogsResMgr
//CHINA001 {
//CHINA001 static ResMgr*		GetResMgr();
//CHINA001 };

#endif


