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



#ifndef SC_DOCFUNC_HXX
#define SC_DOCFUNC_HXX

#include <tools/link.hxx>
#include "global.hxx"
#include "formula/grammar.hxx"
#include "tabbgcolor.hxx"

class ScEditEngineDefaulter;
class SdrUndoAction;
class ScAddress;
class ScDocShell;
class ScMarkData;
class ScPatternAttr;
class ScRange;
class ScRangeName;
class ScBaseCell;
class ScTokenArray;
struct ScTabOpParam;
class ScTableProtection;

// ---------------------------------------------------------------------------

class ScDocFunc
{
private:
	ScDocShell&		rDocShell;

	sal_Bool			AdjustRowHeight( const ScRange& rRange, sal_Bool bPaint = sal_True );
	void			CreateOneName( ScRangeName& rList,
									SCCOL nPosX, SCROW nPosY, SCTAB nTab,
									SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
									sal_Bool& rCancel, sal_Bool bApi );
	void			NotifyInputHandler( const ScAddress& rPos );

public:
					ScDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
					~ScDocFunc() {}

    DECL_LINK( NotifyDrawUndo, SdrUndoAction* );

	sal_Bool			DetectiveAddPred(const ScAddress& rPos);
	sal_Bool			DetectiveDelPred(const ScAddress& rPos);
	sal_Bool			DetectiveAddSucc(const ScAddress& rPos);
	sal_Bool			DetectiveDelSucc(const ScAddress& rPos);
	sal_Bool			DetectiveAddError(const ScAddress& rPos);
	sal_Bool			DetectiveMarkInvalid(SCTAB nTab);
	sal_Bool			DetectiveDelAll(SCTAB nTab);
	sal_Bool			DetectiveRefresh(sal_Bool bAutomatic = sal_False);

	sal_Bool			DeleteContents( const ScMarkData& rMark, sal_uInt16 nFlags,
									sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			TransliterateText( const ScMarkData& rMark, sal_Int32 nType,
									sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			SetNormalString( const ScAddress& rPos, const String& rText, sal_Bool bApi );
	sal_Bool			PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi );
	sal_Bool			PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine,
								sal_Bool bInterpret, sal_Bool bApi );
	sal_Bool			SetCellText( const ScAddress& rPos, const String& rText,
									sal_Bool bInterpret, sal_Bool bEnglish, sal_Bool bApi,
                                    const String& rFormulaNmsp,
                                    const formula::FormulaGrammar::Grammar eGrammar );

					// creates a new cell for use with PutCell
	ScBaseCell*		InterpretEnglishString( const ScAddress& rPos, const String& rText,
                        const String& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammar,
                        short* pRetFormatType = NULL );

	bool			ShowNote( const ScAddress& rPos, bool bShow = true );
	inline bool		HideNote( const ScAddress& rPos ) { return ShowNote( rPos, false ); }

    bool            SetNoteText( const ScAddress& rPos, const String& rNoteText, sal_Bool bApi );
    bool            ReplaceNote( const ScAddress& rPos, const String& rNoteText, const String* pAuthor, const String* pDate, sal_Bool bApi );

	sal_Bool			ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
									sal_Bool bRecord, sal_Bool bApi );
	sal_Bool			ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
									sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			InsertCells( const ScRange& rRange,const ScMarkData* pTabMark,
                                 InsCellCmd eCmd, sal_Bool bRecord, sal_Bool bApi,
									sal_Bool bPartOfPaste = sal_False );
	sal_Bool			DeleteCells( const ScRange& rRange, const ScMarkData* pTabMark,
                                 DelCellCmd eCmd, sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			MoveBlock( const ScRange& rSource, const ScAddress& rDestPos,
								sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

	sal_Bool			InsertTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi );
	sal_Bool			RenameTable( SCTAB nTab, const String& rName, sal_Bool bRecord, sal_Bool bApi );
	sal_Bool			DeleteTable( SCTAB nTab, sal_Bool bRecord, sal_Bool bApi );

    bool            SetTabBgColor( SCTAB nTab, const Color& rColor, bool bRecord, bool bApi );
    bool            SetTabBgColor( ScUndoTabColorInfo::List& rUndoTabColorList, bool bRecord, bool bApi );

	sal_Bool			SetTableVisible( SCTAB nTab, sal_Bool bVisible, sal_Bool bApi );

	sal_Bool			SetLayoutRTL( SCTAB nTab, sal_Bool bRTL, sal_Bool bApi );

//UNUSED2009-05 sal_Bool	 	    SetGrammar( formula::FormulaGrammar::Grammar eGrammar );

	SC_DLLPUBLIC sal_Bool			SetWidthOrHeight( sal_Bool bWidth, SCCOLROW nRangeCnt, SCCOLROW* pRanges,
									SCTAB nTab, ScSizeMode eMode, sal_uInt16 nSizeTwips,
									sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			InsertPageBreak( sal_Bool bColumn, const ScAddress& rPos,
									sal_Bool bRecord, sal_Bool bSetModified, sal_Bool bApi );
	sal_Bool			RemovePageBreak( sal_Bool bColumn, const ScAddress& rPos,
									sal_Bool bRecord, sal_Bool bSetModified, sal_Bool bApi );

    void            ProtectSheet( SCTAB nTab, const ScTableProtection& rProtect );

	sal_Bool			Protect( SCTAB nTab, const String& rPassword, sal_Bool bApi );
	sal_Bool			Unprotect( SCTAB nTab, const String& rPassword, sal_Bool bApi );

	sal_Bool			ClearItems( const ScMarkData& rMark, const sal_uInt16* pWhich, sal_Bool bApi );
	sal_Bool			ChangeIndent( const ScMarkData& rMark, sal_Bool bIncrement, sal_Bool bApi );
	sal_Bool			AutoFormat( const ScRange& rRange, const ScMarkData* pTabMark,
									sal_uInt16 nFormatNo, sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			EnterMatrix( const ScRange& rRange, const ScMarkData* pTabMark,
                                    const ScTokenArray* pTokenArray,
									const String& rString, sal_Bool bApi, sal_Bool bEnglish,
                                    const String& rFormulaNmsp,
                                    const formula::FormulaGrammar::Grammar );

	sal_Bool			TabOp( const ScRange& rRange, const ScMarkData* pTabMark,
							const ScTabOpParam& rParam, sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			FillSimple( const ScRange& rRange, const ScMarkData* pTabMark,
								FillDir eDir, sal_Bool bRecord, sal_Bool bApi );
	sal_Bool			FillSeries( const ScRange& rRange, const ScMarkData* pTabMark,
								FillDir	eDir, FillCmd eCmd, FillDateCmd	eDateCmd,
								double fStart, double fStep, double fMax,
								sal_Bool bRecord, sal_Bool bApi );
					// FillAuto: rRange wird von Source-Range auf Dest-Range angepasst
	sal_Bool			FillAuto( ScRange& rRange, const ScMarkData* pTabMark,
								FillDir eDir, sal_uLong nCount, sal_Bool bRecord, sal_Bool bApi );

	sal_Bool			ResizeMatrix( const ScRange& rOldRange, const ScAddress& rNewEnd, sal_Bool bApi );

	sal_Bool			MergeCells( const ScRange& rRange, sal_Bool bContents,
								sal_Bool bRecord, sal_Bool bApi );
	sal_Bool			UnmergeCells( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            SetNewRangeNames( ScRangeName* pNewRanges, sal_Bool bApi );     // takes ownership of pNewRanges
	sal_Bool			ModifyRangeNames( const ScRangeName& rNewRanges, sal_Bool bApi );

	sal_Bool			CreateNames( const ScRange& rRange, sal_uInt16 nFlags, sal_Bool bApi );
	sal_Bool			InsertNameList( const ScAddress& rStartPos, sal_Bool bApi );

	sal_Bool			InsertAreaLink( const String& rFile, const String& rFilter,
									const String& rOptions, const String& rSource,
									const ScRange& rDestRange, sal_uLong nRefresh,
									sal_Bool bFitBlock, sal_Bool bApi );
};



#endif

