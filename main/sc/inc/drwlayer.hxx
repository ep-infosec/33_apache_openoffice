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



#ifndef SC_DRWLAYER_HXX
#define SC_DRWLAYER_HXX

#include <vcl/graph.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdundo.hxx>
#include "global.hxx"

class ScDocument;
class SfxViewShell;
class SfxObjectShell;
class ScDrawObjData;
class ScIMapInfo;
class ScMacroInfo;
class IMapObject;
class ScMarkData;
class SdrOle2Obj;
class ScRange;
class ScAddress;

// -----------------------------------------------------------------------

class ScTabDeletedHint : public SfxHint
{
private:
	SCTAB	nTab;
public:
			TYPEINFO();
			ScTabDeletedHint( SCTAB nTabNo = SCTAB_MAX );
	virtual	~ScTabDeletedHint();

	SCTAB	GetTab()	{ return nTab; }
};

class ScTabSizeChangedHint : public SfxHint
{
private:
	SCTAB	nTab;
public:
			TYPEINFO();
			ScTabSizeChangedHint( SCTAB nTabNo = SCTAB_MAX );
	virtual	~ScTabSizeChangedHint();

	SCTAB	GetTab()	{ return nTab; }
};

// -----------------------------------------------------------------------
//
//  Das Anpassen der Detektiv-UserData muss zusammen mit den Draw-Undo's
//  in der SdrUndoGroup liegen, darum von SdrUndoAction abgeleitet:

class ScUndoObjData : public SdrUndoObj
{
private:
    ScAddress   aOldStt;
    ScAddress   aOldEnd;
    ScAddress   aNewStt;
    ScAddress   aNewEnd;
    sal_Bool        bHasNew;
public:
                ScUndoObjData( SdrObject* pObj, const ScAddress& rOS, const ScAddress& rOE,
                                                const ScAddress& rNS, const ScAddress& rNE );
                ~ScUndoObjData();

    virtual void     Undo();
    virtual void     Redo();
};

// -----------------------------------------------------------------------

class SC_DLLPUBLIC ScDrawLayer : public FmFormModel
{
private:
//REMOVE		SotStorageRef	xPictureStorage;
	String			aName;
	ScDocument*		pDoc;
	SdrUndoGroup*	pUndoGroup;
	sal_Bool			bRecording;
	sal_Bool			bAdjustEnabled;
	sal_Bool			bHyphenatorSet;
    bool            mbUndoAllowed;

private:
	void			MoveAreaTwips( SCTAB nTab, const Rectangle& rArea, const Point& rMove,
								const Point& rTopLeft );
	void			MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
								SCsCOL nDx,SCsROW nDy, bool bUpdateNoteCaptionPos );

    void            RecalcPos( SdrObject* pObj, const ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos );

public:
					ScDrawLayer( ScDocument* pDocument, const String& rName );
	virtual			~ScDrawLayer();

	virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
	virtual SdrModel* AllocModel() const;
	virtual void	SetChanged( sal_Bool bFlg = sal_True );

	virtual Window* GetCurDocViewWin();
	virtual SvStream* GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const;

	virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

//REMOVE		void			ReleasePictureStorage();

	sal_Bool			HasObjects() const;

    sal_Bool            ScAddPage( SCTAB nTab );
	void			ScRemovePage( SCTAB nTab );
	void			ScRenamePage( SCTAB nTab, const String& rNewName );
	void			ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
					// inkl. Inhalt, bAlloc=sal_False -> nur Inhalt
	void			ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos, sal_Bool bAlloc );

	ScDocument*		GetDocument() const { return pDoc; }

	void			UpdateBasic();				// DocShell-Basic in DrawPages setzen
	void			UseHyphenator();

	sal_Bool			GetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const;

					//		automatische Anpassungen

	void			EnableAdjust( sal_Bool bSet = sal_True )	{ bAdjustEnabled = bSet; }

	void			BeginCalcUndo(bool bDisableTextEditUsesCommonUndoManager);
	SdrUndoGroup*	GetCalcUndo();
	sal_Bool			IsRecording() const			{ return bRecording; }
	void			AddCalcUndo( SdrUndoAction* pUndo );

    template< typename TUndoAction, typename TArg >
    inline void			AddCalcUndo( const TArg & rArg )    {   if( this->IsUndoAllowed() )  this->AddCalcUndo( new TUndoAction( rArg ) ); }

    template< typename TUndoAction, typename TArg >
    inline void			AddCalcUndo( TArg & rArg )    {   if( this->IsUndoAllowed() )  this->AddCalcUndo( new TUndoAction( rArg ) ); }
	
    template< typename TUndoAction, typename TArg1, typename TArg2 >
    inline void			AddCalcUndo( TArg1 & rArg1, TArg2 & rArg2 ) {   if( this->IsUndoAllowed() )  this->AddCalcUndo( new TUndoAction( rArg1, rArg2 ) ); }

    template< typename TUndoAction, typename TArg1, typename TArg2 >
    inline void			AddCalcUndo( const TArg1 & rArg1, const TArg2 & rArg2 ) {   if( this->IsUndoAllowed() )  this->AddCalcUndo( new TUndoAction( rArg1, rArg2 ) ); }

    template< typename TUndoAction, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5 >
    inline void			AddCalcUndo( const TArg1 & rArg1, const TArg2 & rArg2, const TArg3 & rArg3, const TArg4 & rArg4, const TArg5 & rArg5 )    {   if( this->IsUndoAllowed() )  this->AddCalcUndo( new TUndoAction( rArg1, rArg2, rArg3, rArg4, rArg5 ) ); }

	void			MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
								SCsCOL nDx,SCsROW nDy, sal_Bool bInsDel, bool bUpdateNoteCaptionPos = true );
	void			WidthChanged( SCTAB nTab, SCCOL nCol, long nDifTwips );
	void			HeightChanged( SCTAB nTab, SCROW nRow, long nDifTwips );

        sal_Bool            HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow, bool bIncludeNotes = true );

	void			DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
											SCCOL nCol2,SCROW nRow2 );
	void			DeleteObjectsInSelection( const ScMarkData& rMark );
#if 0
	void			DeleteObjects( SCTAB nTab );
#endif

	void			CopyToClip( ScDocument* pClipDoc, SCTAB nTab, const Rectangle& rRange );
	void			CopyFromClip( ScDrawLayer* pClipModel,
									SCTAB nSourceTab, const Rectangle& rSourceRange,
									const ScAddress& rDestPos, const Rectangle& rDestRange );

	void			SetPageSize( sal_uInt16 nPageNo, const Size& rSize, bool bUpdateNoteCaptionPos = true );

					//	mirror or move between positive and negative positions for RTL
	void			MirrorRTL( SdrObject* pObj );
	static void		MirrorRectRTL( Rectangle& rRect );		// for bounding rectangles etc.

    /** Returns the rectangle for the passed cell address in 1/100 mm.
        @param bMergedCell  True = regards merged cells. False = use single column/row size. */
    static Rectangle GetCellRect( ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell );

					//	GetVisibleName: name for navigator etc: GetPersistName or GetName
					//	(ChartListenerCollection etc. must use GetPersistName directly)
	static String	GetVisibleName( SdrObject* pObj );

	SdrObject*		GetNamedObject( const String& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const;
                    // if pnCounter != NULL, the search for a name starts with this index + 1,
                    // and the index really used is returned.
    String          GetNewGraphicName( long* pnCounter = NULL ) const;
	void			EnsureGraphicNames();

	// Verankerung setzen und ermitteln
	static void		SetAnchor( SdrObject*, ScAnchorType );
	static ScAnchorType	GetAnchor( const SdrObject* );

	// Positionen fuer Detektivlinien
	static ScDrawObjData* GetObjData( SdrObject* pObj, sal_Bool bCreate=sal_False );

    // The sheet information in ScDrawObjData isn't updated when sheets are inserted/deleted.
    // Use this method to get an object with positions on the specified sheet (should be the
    // sheet on which the object is inserted).
    static ScDrawObjData* GetObjDataTab( SdrObject* pObj, SCTAB nTab );

    /** Returns true, if the passed object is the caption of a cell note. */
    static bool     IsNoteCaption( SdrObject* pObj );

    /** Returns the object data, if the passed object is a cell note caption. */
    static ScDrawObjData* GetNoteCaptionData( SdrObject* pObj, SCTAB nTab );

	// Image-Map
	static ScIMapInfo* GetIMapInfo( SdrObject* pObj );

	static IMapObject* GetHitIMapObject( SdrObject* pObject,
							const Point& rWinPoint, const Window& rCmpWnd );

    static ScMacroInfo* GetMacroInfo( SdrObject* pObj, sal_Bool bCreate = sal_False );
	virtual ImageMap* GetImageMapForObject(SdrObject* pObj);
	virtual sal_Int32 GetHyperlinkCount(SdrObject* pObj);

private:
	static SfxObjectShell* pGlobalDrawPersist;			// fuer AllocModel
public:
	static void		SetGlobalDrawPersist(SfxObjectShell* pPersist);
protected:
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();
public:
    inline void SetUndoAllowed( bool bUndoAllowed ){ mbUndoAllowed = bUndoAllowed; }
    inline bool IsUndoAllowed() const{ return mbUndoAllowed; }
};


#endif


