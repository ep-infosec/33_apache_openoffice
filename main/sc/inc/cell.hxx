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

#ifndef SC_CELL_HXX
#define SC_CELL_HXX

#include <stddef.h>

#include <set>
#include <tools/mempool.hxx>
#include <svl/listener.hxx>
#include "global.hxx"
#include "rangenam.hxx"
#include "formula/grammar.hxx"
#include "tokenarray.hxx"
#include "formularesult.hxx"
#include <rtl/ustrbuf.hxx>
#include <unotools/fontcvt.hxx>
#include "scdllapi.h"

#include <cellform.hxx>

#define USE_MEMPOOL
#define TEXTWIDTH_DIRTY		0xffff

// in addition to SCRIPTTYPE_... flags from scripttypeitem.hxx:
// set (in nScriptType) if type has not been determined yet
#define SC_SCRIPTTYPE_UNKNOWN	0x08

class ScDocument;
class EditTextObject;
class ScMatrix;
class SvtBroadcaster;
class ScCodeArray;
class ScProgress;
class ScPostIt;

// ============================================================================

/** Default cell clone flags: do not start listening, do not adjust 3D refs to
    old position, clone note captions of cell notes. */
const int SC_CLONECELL_DEFAULT          = 0x0000;

/** If set, cloned formula cells will start to listen to the document. */
const int SC_CLONECELL_STARTLISTENING   = 0x0001;

/** If set, relative 3D references of cloned formula cells will be adjusted to
    old position (used while swapping cells for sorting a cell range). */
const int SC_CLONECELL_ADJUST3DREL      = 0x0002;

/** If set, the caption object of a cell note will not be cloned (used while
    copying cells to undo document, where captions are handled in drawing undo). */
const int SC_CLONECELL_NOCAPTION        = 0x0004;

// ============================================================================

class SC_DLLPUBLIC ScBaseCell
{
protected:
					~ScBaseCell();	// nicht virtuell -> darf nicht direkt aufgerufen werden

public:
    explicit        ScBaseCell( CellType eNewType );

    /** Base copy constructor. Does NOT clone cell note or broadcaster! */
                    ScBaseCell( const ScBaseCell& rCell );

    /** Returns a clone of this cell at the same position, cell note and
        broadcaster will not be cloned. */
    ScBaseCell*     CloneWithoutNote( ScDocument& rDestDoc, int nCloneFlags = SC_CLONECELL_DEFAULT ) const;

    /** Returns a clone of this cell for the passed document position, cell
        note and broadcaster will not be cloned. */
    ScBaseCell*     CloneWithoutNote( ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags = SC_CLONECELL_DEFAULT ) const;

    /** Returns a clone of this cell, clones cell note and caption object too
        (unless SC_CLONECELL_NOCAPTION flag is set). Broadcaster will not be cloned. */
    ScBaseCell*     CloneWithNote( const ScAddress& rOwnPos, ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags = SC_CLONECELL_DEFAULT ) const;

    /** Due to the fact that ScBaseCell does not have a vtable, this function
        deletes the cell by calling the appropriate d'tor of the derived class. */
    void            Delete();

    inline CellType GetCellType() const	{ return (CellType)eCellType; }

    /** Returns true, if the cell is empty (neither value nor formula nor cell note).
        Returns false for formula cells returning nothing, use HasEmptyData() for that. */
    bool            IsBlank( bool bIgnoreNotes = false ) const;

// fuer Idle-Berechnung
    inline sal_uInt16   GetTextWidth() const { return nTextWidth; }
    inline void     SetTextWidth( sal_uInt16 nNew ) { nTextWidth = nNew; }

    inline sal_uInt8     GetScriptType() const { return nScriptType; }
    inline void     SetScriptType( sal_uInt8 nNew ) { nScriptType = nNew; }

    /** Returns true, if the cell contains a note. */
    inline bool     HasNote() const { return mpNote != 0; }
    /** Returns the pointer to a cell note object (read-only). */
    inline const ScPostIt* GetNote() const { return mpNote; }
    /** Returns the pointer to a cell note object. */
    inline ScPostIt* GetNote() { return mpNote; }
    /** Takes ownership of the passed cell note object. */
    void            TakeNote( ScPostIt* pNote );
    /** Returns and forgets the own cell note object. Caller takes ownership! */
    ScPostIt*       ReleaseNote();
    /** Deletes the own cell note object. */
    void            DeleteNote();

    /** Returns true, if the cell contains a broadcaster. */
    inline bool     HasBroadcaster() const { return mpBroadcaster != 0; }
    /** Returns the pointer to the cell broadcaster. */
    inline SvtBroadcaster* GetBroadcaster() const { return mpBroadcaster; }
    /** Takes ownership of the passed cell broadcaster. */
    void            TakeBroadcaster( SvtBroadcaster* pBroadcaster );
    /** Returns and forgets the own cell broadcaster. Caller takes ownership! */
    SvtBroadcaster* ReleaseBroadcaster();
    /** Deletes the own cell broadcaster. */
    void            DeleteBroadcaster();

						// String- oder EditCell
	static ScBaseCell* CreateTextCell( const String& rString, ScDocument* );

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
	void			StartListeningTo( ScDocument* pDoc );
	void			EndListeningTo( ScDocument* pDoc,
									ScTokenArray* pArr = NULL,
									ScAddress aPos = ScAddress() );

    /** Error code if ScFormulaCell, else 0. */
    sal_uInt16          GetErrorCode() const;
    /** ScFormulaCell with formula::svEmptyCell result, or ScNoteCell (may have been
        created due to reference to empty cell). */
	sal_Bool			HasEmptyData() const;
	sal_Bool			HasValueData() const;
	sal_Bool			HasStringData() const;
	String			GetStringData() const;			// nur echte Strings

	static sal_Bool		CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 );

private:
    ScBaseCell&     operator=( const ScBaseCell& );

private:
    ScPostIt*       mpNote;         /// The cell note. Cell takes ownership!
    SvtBroadcaster* mpBroadcaster;  /// Broadcaster for changed values. Cell takes ownership!

protected:
    sal_uInt16          nTextWidth;
    sal_uInt8            eCellType;      // enum CellType - sal_uInt8 spart Speicher
    sal_uInt8            nScriptType;
};

// ============================================================================

class SC_DLLPUBLIC ScNoteCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
	DECL_FIXEDMEMPOOL_NEWDEL( ScNoteCell )
#endif

    /** Cell takes ownership of the passed broadcaster. */
    explicit        ScNoteCell( SvtBroadcaster* pBC = 0 );
    /** Cell takes ownership of the passed note and broadcaster. */
    explicit        ScNoteCell( ScPostIt* pNote, SvtBroadcaster* pBC = 0 );

#ifdef DBG_UTIL
					~ScNoteCell();
#endif

private:
                    ScNoteCell( const ScNoteCell& );
};

// ============================================================================

class SC_DLLPUBLIC ScValueCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
	DECL_FIXEDMEMPOOL_NEWDEL( ScValueCell )
#endif
					ScValueCell();
    explicit		ScValueCell( double fValue );

#ifdef DBG_UTIL
					~ScValueCell();
#endif

    inline void     SetValue( double fValue ) { mfValue = fValue; }
    inline double   GetValue() const { return mfValue; }

private:
	double		    mfValue;
};

// ============================================================================

class SC_DLLPUBLIC ScStringCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
	DECL_FIXEDMEMPOOL_NEWDEL( ScStringCell )
#endif

					ScStringCell();
    explicit		ScStringCell( const String& rString );

#ifdef DBG_UTIL
					~ScStringCell();
#endif

    inline void		SetString( const String& rString ) { maString = rString; }
    inline void		GetString( String& rString ) const { rString = maString; }
	inline const String& GetString() const { return maString; }

private:
	String		    maString;
};

// ============================================================================

class SC_DLLPUBLIC ScEditCell : public ScBaseCell
{
private:
	EditTextObject*		pData;
	String*				pString;		// fuer schnelleren Zugriff von Formeln
	ScDocument*			pDoc;			// fuer EditEngine Zugriff mit Pool

	void			SetTextObject( const EditTextObject* pObject,
									const SfxItemPool* pFromPool );

					// not implemented
					ScEditCell( const ScEditCell& );

public:

#ifdef USE_MEMPOOL
	DECL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

					~ScEditCell();				// wegen pData immer!

					ScEditCell( const EditTextObject* pObject, ScDocument*,
								const SfxItemPool* pFromPool /* = NULL */ );
                    ScEditCell( const ScEditCell& rCell, ScDocument& rDoc );
					// fuer Zeilenumbrueche
					ScEditCell( const String& rString, ScDocument* );

	void			SetData( const EditTextObject* pObject,
							const SfxItemPool* pFromPool /* = NULL */ );
	void			GetData( const EditTextObject*& rpObject ) const;
	void			GetString( String& rString ) const;

	const EditTextObject* GetData() const	{ return pData; }
};

// ============================================================================

enum ScMatrixMode {
    MM_NONE      = 0,                   // No matrix formula
    MM_FORMULA   = 1,                   // Upper left matrix formula cell
    MM_REFERENCE = 2,                   // Remaining cells, via ocMatRef reference token
    MM_FAKE      = 3                    // Interpret "as-if" matrix formula (legacy)
};

class SC_DLLPUBLIC ScFormulaCell : public ScBaseCell, public SvtListener
{
private:
    ScFormulaResult aResult;
    formula::FormulaGrammar::Grammar  eTempGrammar;   // used between string (creation) and (re)compilation
    ScTokenArray*   pCode;              // The (new) token array
    ScDocument*     pDocument;
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;
    sal_uLong           nFormatIndex;       // Number format set by calculation
    short           nFormatType;        // Number format type set by calculation
    sal_uInt16          nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    sal_uInt8            cMatrixFlag;        // One of ScMatrixMode
    sal_Bool            bDirty         : 1; // Must be (re)calculated
    sal_Bool            bChanged       : 1; // Whether something changed regarding display/representation
    sal_Bool            bRunning       : 1; // Already interpreting right now
    sal_Bool            bCompile       : 1; // Must be (re)compiled
    sal_Bool            bSubTotal      : 1; // Cell is part of or contains a SubTotal
    sal_Bool            bIsIterCell    : 1; // Cell is part of a circular reference
    sal_Bool            bInChangeTrack : 1; // Cell is in ChangeTrack
    sal_Bool            bTableOpDirty  : 1; // Dirty flag for TableOp
    sal_Bool            bNeedListening : 1; // Listeners need to be re-established after UpdateReference
    ScToken*            pValidRefToken; // i120962, get the valid reference token if the cell was applied a reference formula

                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
	void			InterpretTail( ScInterpretTailParameter );

    ScFormulaCell( const ScFormulaCell& );

public:

#ifdef USE_MEMPOOL
	DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

	ScAddress		aPos;

					~ScFormulaCell();
					ScFormulaCell();

    /** Empty formula cell, or with a preconstructed token array. */
    ScFormulaCell( ScDocument*, const ScAddress&, const ScTokenArray* = NULL,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    sal_uInt8 = MM_NONE );

    /** With formula string and grammar to compile with.
       formula::FormulaGrammar::GRAM_DEFAULT effectively isformula::FormulaGrammar::GRAM_NATIVE_UI that
        also includes formula::FormulaGrammar::CONV_UNSPECIFIED, therefor uses the address
        convention associated with rPos::nTab by default. */
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                    const String& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    sal_uInt8 cMatInd = MM_NONE );

    ScFormulaCell( const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, int nCloneFlags = SC_CLONECELL_DEFAULT );

	void			GetFormula( String& rFormula,
								const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
	void			GetFormula( rtl::OUStringBuffer& rBuffer,
								const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

	void			SetDirty();
	inline void		SetDirtyVar() { bDirty = sal_True; }
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
	inline void		ResetTableOpDirtyVar() { bTableOpDirty = sal_False; }
	void			SetTableOpDirty();
	sal_Bool			IsDirtyOrInTableOpDirty() const;
	sal_Bool			GetDirty() const { return bDirty; }
    sal_Bool            NeedsListening() const { return bNeedListening; }
    void            SetNeedsListening( sal_Bool bVar ) { bNeedListening = bVar; }
	void			Compile(const String& rFormula,
							sal_Bool bNoListening = sal_False,
                            const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
	void			CompileTokenArray( sal_Bool bNoListening = sal_False );
	void			CompileXML( ScProgress& rProgress );		// compile temporary string tokens
	void			CalcAfterLoad();
    bool            MarkUsedExternalReferences();
	void			Interpret();
    inline sal_Bool     IsIterCell() const { return bIsIterCell; }
    inline sal_uInt16   GetSeenInIteration() const { return nSeenInIteration; }

	sal_Bool			HasOneReference( ScRange& r ) const;
    /* Checks if the formula contains reference list that can be
       expressed by one reference (like A1;A2;A3:A5 -> A1:A5). The
       reference list is not required to be sorted (i.e. A3;A1;A2 is
       still recognized as A1:A3), but no overlapping is allowed.
       If one reference is recognized, the rRange is filled.

       It is similar to HasOneReference(), but more general.
     */
    bool HasRefListExpressibleAsOneReference(ScRange& rRange) const;
	sal_Bool			HasRelNameReference() const;
	sal_Bool			HasColRowName() const;

	void			UpdateReference(UpdateRefMode eUpdateRefMode,
									const ScRange& r,
									SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc = NULL,
                                    const ScAddress* pUndoCellPos = NULL );

	void			TransposeReference();
	void			UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
										ScDocument* pUndoDoc );

	void			UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

	void			UpdateInsertTab(SCTAB nTable);
	void 			UpdateInsertTabAbs(SCTAB nTable);
	sal_Bool			UpdateDeleteTab(SCTAB nTable, sal_Bool bIsMove = sal_False);
	void			UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
	void			UpdateRenameTab(SCTAB nTable, const String& rName);
	sal_Bool 			TestTabRefAbs(SCTAB nTable);
	void			UpdateCompile( sal_Bool bForceIfNameInUse = sal_False );
	sal_Bool			IsRangeNameInUse(sal_uInt16 nIndex) const;
    void            FindRangeNamesInUse(std::set<sal_uInt16>& rIndexes) const;
	void 			ReplaceRangeNamesInUse( const ScRangeData::IndexMap& rMap );
	sal_Bool			IsSubTotal() const 						{ return bSubTotal; }
	sal_Bool			IsChanged() const  						{ return bChanged; }
	void			ResetChanged()							{ bChanged = sal_False; }
	sal_Bool			IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
	sal_Bool			IsEmptyDisplayedAsString();
	sal_Bool			IsValue();      // also sal_True if formula::svEmptyCell
	double			GetValue();
	double			GetValueAlways();	// ignore errors
	void			GetString( String& rString );
	const ScMatrix* GetMatrix();
	sal_Bool			GetMatrixOrigin( ScAddress& rPos ) const;
	void			GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
	sal_uInt16 			GetMatrixEdge( ScAddress& rOrgPos );
	sal_uInt16			GetErrCode();   // interpret first if necessary
	sal_uInt16			GetRawError();  // don't interpret, just return code or result error
	short			GetFormatType() const   				{ return nFormatType; }
	sal_uLong			GetFormatIndex() const					{ return nFormatIndex; }
	void			GetFormatInfo( short& nType, sal_uLong& nIndex ) const
						{ nType = nFormatType; nIndex = nFormatIndex; }
	sal_uInt8			GetMatrixFlag() const   				{ return cMatrixFlag; }
	ScTokenArray*   GetCode() const                         { return pCode; }

	sal_Bool			IsRunning() const						{ return bRunning; }
	void			SetRunning( sal_Bool bVal )					{ bRunning = bVal; }
	void 			CompileDBFormula();
	void 			CompileDBFormula( sal_Bool bCreateFormulaString );
	void 			CompileNameFormula( sal_Bool bCreateFormulaString );
	void 			CompileColRowNameFormula();
	ScFormulaCell*	GetPrevious() const					{ return pPrevious; }
	ScFormulaCell*	GetNext() const						{ return pNext; }
	void			SetPrevious( ScFormulaCell* pF )	{ pPrevious = pF; }
	void			SetNext( ScFormulaCell* pF )		{ pNext = pF; }
	ScFormulaCell*	GetPreviousTrack() const				{ return pPreviousTrack; }
	ScFormulaCell*	GetNextTrack() const					{ return pNextTrack; }
	void			SetPreviousTrack( ScFormulaCell* pF )	{ pPreviousTrack = pF; }
	void			SetNextTrack( ScFormulaCell* pF )		{ pNextTrack = pF; }

	virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint);
	void			SetCompile( sal_Bool bVal ) { bCompile = bVal; }
	ScDocument*		GetDocument() const		{ return pDocument; }
	void			SetMatColsRows( SCCOL nCols, SCROW nRows );
	void			GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

					// ob Zelle im ChangeTrack und nicht im echten Dokument ist
	void			SetInChangeTrack( sal_Bool bVal ) { bInChangeTrack = bVal; }
	sal_Bool			IsInChangeTrack() const { return bInChangeTrack; }

					// Zu Typ und Format das entsprechende Standardformat.
					// Bei Format "Standard" evtl. das in die Formelzelle
					// uebernommene Format.
	sal_uLong			GetStandardFormat( SvNumberFormatter& rFormatter, sal_uLong nFormat ) const;

	// For import filters!
    void 			AddRecalcMode( formula::ScRecalcMode );
    /** For import only: set a double result. */
	void			SetHybridDouble( double n )     { aResult.SetHybridDouble( n); }
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridString( const String& r )
                        { aResult.SetHybridString( r); }
    /** For import only: set a temporary formula string to be compiled later.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridFormula( const String& r,
                                    const formula::FormulaGrammar::Grammar eGrammar )
                        { aResult.SetHybridFormula( r); eTempGrammar = eGrammar; }
	void			SetErrCode( sal_uInt16 n );
	inline sal_Bool		IsHyperLinkCell() const { return pCode && pCode->IsHyperLink(); }
	EditTextObject*		CreateURLObject() ;
    void            GetURLResult( String& rURL, String& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    bool            IsMultilineResult();
    ScToken*        GetValidRefToken() { return pValidRefToken; }   // i120962
};

//			Iterator fuer Referenzen in einer Formelzelle
class ScDetectiveRefIter
{
private:
	ScTokenArray* pCode;
	ScAddress aPos;
public:
				ScDetectiveRefIter( ScFormulaCell* pCell );
	sal_Bool		GetNextRef( ScRange& rRange );
};

// ============================================================================
inline double GetValueFromCell( const ScBaseCell * pCell )
{
    switch (pCell->GetCellType())
    {
    case CELLTYPE_VALUE:
        return ((ScValueCell*)pCell)->GetValue();
    case CELLTYPE_FORMULA:
        {
            if (((ScFormulaCell*)pCell)->IsValue())
                return ((ScFormulaCell*)pCell)->GetValue();
            else
                return 0.0;
        }
    default:
        return 0.0;
    }
}
// ============================================================================


inline String GetStringFromCell( const ScBaseCell * pCell, sal_uLong nFormat, SvNumberFormatter* pFormatter )
{
    if (pCell->GetCellType() != CELLTYPE_NOTE)
    {
        String strResult;
	Color* pColor = NULL;
        ScCellFormat::GetString( const_cast<ScBaseCell*>(pCell), nFormat, strResult, &pColor, *(pFormatter) );
        return strResult;
    }
    else
        return String();
}

#endif

