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


#ifndef _SC_ABSTDLG_HXX
#define _SC_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/syswin.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/field.hxx>
#include "sc.hrc"
#include "global.hxx"
#include "pivot.hxx"
#include "i18npool/lang.h"

class ScAsciiOptions;
class ScAutoFormat;
class ScAutoFormatData;
class ScDocument;
struct ScImportSourceDesc;
class ScViewData;
class ScRangeName;
class ScQueryItem;
class ScImportOptions;
class SfxStyleSheetBase;
class ScDPObject;
struct ScPivotFuncData;
struct ScDPNumGroupInfo;
//<!--Added by PengYunQuan for Validity Cell Range Picker
class ScTabViewShell;
//-->Added by PengYunQuan for Validity Cell Range Picker

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }

class AbstractScImportAsciiDlg : public VclAbstractDialog  //add for ScImportAsciiDlg
{
public:
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) = 0;
    virtual void                        SetTextToColumnsMode() = 0;
    virtual void                        SaveParameters() = 0;
};


class AbstractScAutoFormatDlg : public VclAbstractDialog  //add for ScAutoFormatDlg
{
public:
	virtual sal_uInt16 GetIndex() const = 0 ;
	virtual String GetCurrFormatName() = 0;
};

class AbstractScColRowLabelDlg : public VclAbstractDialog  //add for ScColRowLabelDlg
{
public:
	virtual sal_Bool IsCol() = 0;
	virtual sal_Bool IsRow() = 0;
};

class AbstractScDataPilotDatabaseDlg  :public VclAbstractDialog  //add for ScDataPilotDatabaseDlg
{
public:
	virtual void	GetValues( ScImportSourceDesc& rDesc ) = 0;
};

class AbstractScDataPilotSourceTypeDlg  : public VclAbstractDialog  //add for ScDataPilotSourceTypeDlg
{
public:
	virtual sal_Bool	IsDatabase() const = 0;
	virtual sal_Bool	IsExternal() const = 0;
};

class AbstractScDataPilotServiceDlg : public VclAbstractDialog  //add for ScDataPilotServiceDlg
{
public:
	virtual String	GetServiceName() const = 0;
	virtual String	GetParSource() const = 0 ;
	virtual String	GetParName() const = 0 ;
	virtual String	GetParUser() const = 0;
	virtual String	GetParPass() const = 0;
};

class AbstractScDeleteCellDlg : public VclAbstractDialog  //add for ScDeleteCellDlg
{
public:
	virtual DelCellCmd GetDelCellCmd() const = 0;
};

class AbstractScDeleteContentsDlg: public VclAbstractDialog  //add for ScDeleteContentsDlg
{
public:
	virtual void	DisableObjects() = 0 ;
	virtual sal_uInt16	GetDelContentsCmdBits() const = 0;
};

class AbstractScFillSeriesDlg: public VclAbstractDialog  //add for ScFillSeriesDlg
{
public:
	virtual FillDir		GetFillDir() const = 0;
	virtual FillCmd		GetFillCmd() const = 0;
	virtual FillDateCmd	GetFillDateCmd() const = 0;
	virtual double		GetStart() const = 0;
	virtual double		GetStep() const	= 0;
	virtual double		GetMax() const = 0;
	virtual String		GetStartStr() const = 0;
	virtual void		SetEdStartValEnabled(sal_Bool bFlag=sal_False) = 0;
};

class AbstractScGroupDlg :  public VclAbstractDialog  //add for ScGroupDlg
{
public:
	virtual sal_Bool GetColsChecked() const = 0;
};

class AbstractScInsertCellDlg : public VclAbstractDialog  //add for ScInsertCellDlg
{
public:
	virtual InsCellCmd GetInsCellCmd() const = 0;
};

class AbstractScInsertContentsDlg : public VclAbstractDialog  //add for ScInsertContentsDlg
{
public:
	virtual sal_uInt16		GetInsContentsCmdBits() const = 0;
	virtual sal_uInt16		GetFormulaCmdBits() const = 0 ;
	virtual sal_Bool		IsSkipEmptyCells() const = 0;
	virtual sal_Bool 		IsLink() const = 0;
	virtual void	SetFillMode( sal_Bool bSet ) = 0;
	virtual void	SetOtherDoc( sal_Bool bSet ) = 0;
	virtual sal_Bool 		IsTranspose() const = 0;
	virtual void	SetChangeTrack( sal_Bool bSet ) = 0;
	virtual void	SetCellShiftDisabled( int nDisable ) = 0;
	virtual InsCellCmd	GetMoveMode() = 0;
};

class AbstractScInsertTableDlg : public VclAbstractDialog  //add for ScInsertTableDlg
{
public:
	virtual sal_Bool			GetTablesFromFile()	= 0;
	virtual sal_Bool			GetTablesAsLink()	= 0;
	virtual const String*	GetFirstTable( sal_uInt16* pN = NULL ) = 0;
	virtual ScDocShell*		GetDocShellTables()	= 0;
	virtual sal_Bool			IsTableBefore() = 0;
	virtual sal_uInt16			GetTableCount() = 0;
	virtual const String*	GetNextTable( sal_uInt16* pN = NULL ) = 0;

};

class AbstractScSelEntryDlg : public VclAbstractDialog  //add for ScSelEntryDlg
{
public:
	virtual String GetSelectEntry() const = 0;
};

class AbstractScLinkedAreaDlg : public VclAbstractDialog2  //add for ScLinkedAreaDlg
{
public:
	virtual void			InitFromOldLink( const String& rFile, const String& rFilter,
										const String& rOptions, const String& rSource,
										sal_uLong nRefresh ) = 0;
	virtual String			GetURL() = 0;
	virtual String			GetFilter() = 0;		// may be empty
	virtual String			GetOptions() = 0;		// filter options
	virtual String			GetSource() = 0;		// separated by ";"
	virtual sal_uLong			GetRefresh() = 0;		// 0 if disabled
};

class AbstractScMetricInputDlg : public VclAbstractDialog  //add for ScMetricInputDlg
{
public:
	virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const = 0;
};

class AbstractScMoveTableDlg : public VclAbstractDialog  //add for ScMoveTableDlg
{
public:
	virtual sal_uInt16	GetSelectedDocument		() const = 0;
	virtual sal_uInt16	GetSelectedTable		() const = 0;
	virtual sal_Bool	GetCopyTable			() const = 0;
	virtual void	SetCopyTable			(sal_Bool bFlag=sal_True) = 0;
	virtual void	EnableCopyTable			(sal_Bool bFlag=sal_True) = 0;
};

class AbstractScNameCreateDlg : public VclAbstractDialog  //add for ScNameCreateDlg
{
public:
	virtual sal_uInt16			GetFlags() const = 0;
};

class AbstractScNamePasteDlg : public VclAbstractDialog  //add for ScNamePasteDlg
{
public:
	virtual String			GetSelectedName() const = 0;
};

class AbstractScPivotFilterDlg : public VclAbstractDialog  //add for ScPivotFilterDlg
{
public:
	virtual const ScQueryItem&	GetOutputItem() = 0;
};

class AbstractScDPFunctionDlg : public VclAbstractDialog  //add for ScDPFunctionDlg
{
public:
    virtual sal_uInt16  GetFuncMask() const = 0;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const = 0;
};

class AbstractScDPSubtotalDlg : public VclAbstractDialog  //add for ScDPSubtotalDlg
{
public:
    virtual sal_uInt16  GetFuncMask() const = 0;
    virtual void    FillLabelData( ScDPLabelData& rLabelData ) const = 0;
};

class AbstractScDPNumGroupDlg : public VclAbstractDialog
{
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
};

class AbstractScDPDateGroupDlg : public VclAbstractDialog
{
public:
    virtual ScDPNumGroupInfo GetGroupInfo() const = 0;
    virtual sal_Int32 GetDatePart() const = 0;
};

class AbstractScDPShowDetailDlg : public VclAbstractDialog  //add for ScDPShowDetailDlg
{
public:
    virtual String  GetDimensionName() const = 0;
};

class AbstractScNewScenarioDlg : public VclAbstractDialog  //add for ScNewScenarioDlg
{
public:

	virtual void SetScenarioData( const String& rName, const String& rComment,
							const Color& rColor, sal_uInt16 nFlags ) = 0;

	virtual void GetScenarioData( String& rName, String& rComment,
							Color& rColor, sal_uInt16& rFlags ) const = 0;
};

class AbstractScShowTabDlg : public VclAbstractDialog  //add for ScShowTabDlg
{
public:
    virtual void    Insert( const String& rString, sal_Bool bSelected ) = 0;
	virtual sal_uInt16	GetSelectEntryCount() const = 0;
	virtual void SetDescription(const String& rTitle, const String& rFixedText, const rtl::OString& nDlgHelpId, const rtl::OString& nLbHelpId ) = 0;
	virtual String	GetSelectEntry(sal_uInt16 nPos) const = 0;
    virtual sal_uInt16  GetSelectEntryPos(sal_uInt16 nPos) const = 0;
};

class AbstractScStringInputDlg :  public VclAbstractDialog  //add for ScStringInputDlg
{
public:
	virtual void GetInputString( String& rString ) const = 0;
};

class AbstractScTabBgColorDlg : public VclAbstractDialog  //add for ScTabBgColorDlg
{
public:
    virtual void GetSelectedColor( Color& rColor ) const = 0;
};

class AbstractScImportOptionsDlg : public VclAbstractDialog  //add for ScImportOptionsDlg
{
public:
	virtual void GetImportOptions( ScImportOptions& rOptions ) const = 0;
};

class AbstractScTextImportOptionsDlg : public VclAbstractDialog //add for ScLangChooserDlg
{
public:
    virtual LanguageType GetLanguageType() const = 0;
    virtual bool IsDateConversionSet() const = 0;
};

//-------Scabstract fractory ---------------------------
class ScAbstractDialogFactory
{
public:
	static ScAbstractDialogFactory* 	Create();

	virtual 	AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName, //add for ScImportAsciiDlg
																	SvStream* pInStream, int nId,
																	sal_Unicode cSep = '\t') = 0;

    virtual     AbstractScTextImportOptionsDlg * CreateScTextImportOptionsDlg( Window* pParent, int nId ) = 0;

	virtual		AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*					pParent, //add for ScAutoFormatDlg
																ScAutoFormat*				pAutoFormat,
																const ScAutoFormatData*    pSelFormatData,
																ScDocument*                pDoc,
																int nId) = 0;
	virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent, //add for ScColRowLabelDlg
																int nId,
																sal_Bool bCol = sal_False,
																sal_Bool bRow = sal_False) = 0;

	virtual VclAbstractDialog * CreateScColOrRowDlg( Window*			pParent, //add for ScColOrRowDlg
													const String&	rStrTitle,
													const String&	rStrLabel,
													int nId,
													sal_Bool				bColDefault = sal_True ) = 0;
    virtual VclAbstractDialog * CreateScSortWarningDlg ( Window* pParent, const String& rExtendText, const String& rCurrentText, int nId ) = 0;  //add for ScSortWarningDlg
	virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,int nId ) = 0; //add for ScDataPilotDatabaseDlg

	virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg ( Window* pParent, sal_Bool bEnableExternal, int nId ) = 0; //add for ScDataPilotSourceTypeDlg

	virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent, //add for ScDataPilotServiceDlg
																		const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
																		int nId ) = 0;

	virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg( Window* pParent, int nId, sal_Bool bDisallowCellMove = sal_False ) = 0 ; //add for ScDeleteCellDlg

	virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,int nId, //add for ScDeleteContentsDlg
																 sal_uInt16  nCheckDefaults = 0 ) = 0;
	virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*		pParent, //add for ScFillSeriesDlg
															ScDocument&	rDocument,
															FillDir		eFillDir,
															FillCmd		eFillCmd,
															FillDateCmd	eFillDateCmd,
															String			aStartStr,
															double			fStep,
															double			fMax,
															sal_uInt16			nPossDir,
															int nId) = 0;

	virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent, //add for ScGroupDlg
															sal_uInt16	nResId,
															int nId,
															sal_Bool	bUnGroup = sal_False,
															sal_Bool	bRows    = sal_True  ) = 0;

	virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
																int nId,
															sal_Bool bDisallowCellMove = sal_False ) = 0;

	virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*		pParent, //add for ScInsertContentsDlg
																	int nId,
																	sal_uInt16  		nCheckDefaults = 0,
																	const String*	pStrTitle = NULL ) = 0;

	virtual AbstractScInsertTableDlg * CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,  //add for ScInsertTableDlg
                                                                SCTAB nTabCount, bool bFromFile, int nId) = 0;

	virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent, // add for ScSelEntryDlg
															sal_uInt16	nResId,
														const String& aTitle,
														const String& aLbTitle,
																List& 	aEntryList,
															int nId ) = 0;
	virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg (  Window* pParent, int nId) = 0; //add for ScLinkedAreaDlg

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
																long          nLast     = 100 ) = 0;

	virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(  Window* pParent, int nId ) = 0; //add for ScMoveTableDlg

	virtual AbstractScNameCreateDlg * CreateScNameCreateDlg ( Window * pParent, sal_uInt16 nFlags, int nId ) = 0; //add for ScNameCreateDlg

	virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList, //add for ScNamePasteDlg
															int nId , sal_Bool bInsList=sal_True ) = 0;

	virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent, //add for ScPivotFilterDlg
																const SfxItemSet&	rArgSet, sal_uInt16 nSourceTab , int nId ) = 0;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, int nId,
                                                                const ScDPLabelDataVector& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData ) = 0;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScPivotFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout ) = 0;

    virtual AbstractScDPNumGroupDlg * CreateScDPNumGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo ) = 0;

    virtual AbstractScDPDateGroupDlg * CreateScDPDateGroupDlg( Window* pParent,
                                                                int nId,
                                                                const ScDPNumGroupInfo& rInfo,
                                                                sal_Int32 nDatePart,
                                                                const Date& rNullDate ) = 0;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, int nId,
                                                                ScDPObject& rDPObj,
                                                                sal_uInt16 nOrient ) = 0;

	virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName, //add for ScNewScenarioDlg
																int nId,
																sal_Bool bEdit = sal_False, sal_Bool bSheetProtected = sal_False ) = 0;
	virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, int nId ) = 0; //add for ScShowTabDlg

	virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent, //add for ScStringInputDlg
																const String& rTitle,
																const String& rEditTitle,
																const String& rDefault,
																const rtl::OString& sHelpId, const rtl::OString& sEditHelpId,
																int nId ) = 0;

    virtual AbstractScTabBgColorDlg * CreateScTabBgColorDlg (  Window* pParent, //add for ScTabBgColorDlg
                                                                const String& rTitle, //Dialog Title
                                                                const String& rTabBgColorNoColorText, //Label for no tab color
                                                                const Color& rDefaultColor, //Currently selected Color
                                                                const rtl::OString& ,
                                                                int nId ) = 0;

	virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window* 				pParent, //add for ScImportOptionsDlg
																	int nId,
																	sal_Bool                    bAscii = sal_True,
																	const ScImportOptions*	pOptions = NULL,
																	const String*			pStrTitle = NULL,
																	sal_Bool                    bMultiByte = sal_False,
																	sal_Bool                    bOnlyDbtoolsEncodings = sal_False,
																	sal_Bool                    bImport = sal_True ) = 0;

	virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*	 pFrame, //add for ScAttrDlg
													Window*			 pParent,
													const SfxItemSet* pCellAttrs,
													int nId) = 0;

	virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*		pFrame, //add for ScHFEditDlg
													Window*			pParent,
													const SfxItemSet&	rCoreSet,
													const String&		rPageStyle,
													int nId,
													sal_uInt16				nResId = RID_SCDLG_HFEDIT ) = 0;

	virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*				pParent,//add for ScStyleDlg
													SfxStyleSheetBase&	rStyleBase,
													sal_uInt16				nRscId,
													int nId) = 0;

	virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*				pParent, //add for ScSubTotalDlg
														const SfxItemSet*	pArgSet,
														int nId) = 0;

	virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,//add for ScCharDlg
													const SfxObjectShell* pDocShell, int nId ) = 0;

	virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,//add for ScParagraphDlg
															int nId ) = 0;

	//<!--Modified by PengYunQuan for Validity Cell Range Picker
	//virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
	//												const SfxItemSet* pArgSet,int nId  ) = 0;

	virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
														const SfxItemSet* pArgSet,int nId, ScTabViewShell *pTabVwSh  ) = 0;
	//-->Modified by PengYunQuan for Validity Cell Range Picker
	virtual SfxAbstractTabDialog * CreateScSortDlg( Window*			 pParent, //add for ScSortDlg
													const SfxItemSet* pArgSet,int nId ) = 0;
	// for tabpage
	virtual CreateTabPage				GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;
	virtual GetTabPageRanges			GetTabPageRangesFunc( sal_uInt16 nId ) = 0;
};
#endif

