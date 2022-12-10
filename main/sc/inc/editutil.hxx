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



#ifndef SC_EDITUTIL_HXX
#define SC_EDITUTIL_HXX

#include "scdllapi.h"
#include "address.hxx"
#include <editeng/editeng.hxx>
#include <svx/pageitem.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>


class OutputDevice;
class ScDocument;
class ScPatternAttr;
class ScEditEngineDefaulter;

class ScEditUtil
{
	ScDocument*		pDoc;
	SCCOL			nCol;
	SCROW			nRow;
	SCTAB			nTab;
	Point			aScrPos;
	OutputDevice*	pDev;			// MapMode muss eingestellt sein
	double			nPPTX;
	double			nPPTY;
	Fraction		aZoomX;
	Fraction		aZoomY;

	static const char __FAR_DATA pCalcDelimiters[];

public:
	static String ModifyDelimiters( const String& rOld );

    /// Retrieves string with paragraphs delimited by spaces
	static String GetSpaceDelimitedString( const EditEngine& rEngine );

    /// Retrieves string with paragraphs delimited by new lines ('\n').
    static String GetMultilineString( const EditEngine& rEngine );

public:
				ScEditUtil( ScDocument* pDocument, SCCOL nX, SCROW nY, SCTAB nZ,
							const Point& rScrPosPixel,
							OutputDevice* pDevice, double nScaleX, double nScaleY,
							const Fraction& rX, const Fraction& rY ) :
					pDoc(pDocument),nCol(nX),nRow(nY),nTab(nZ),
					aScrPos(rScrPosPixel),pDev(pDevice),
					nPPTX(nScaleX),nPPTY(nScaleY),aZoomX(rX),aZoomY(rY) {}

	Rectangle	GetEditArea( const ScPatternAttr* pPattern, sal_Bool bForceToTop );
};


class ScEditAttrTester
{
	ScEditEngineDefaulter* pEngine;
	SfxItemSet*	pEditAttrs;
	sal_Bool		bNeedsObject;
	sal_Bool		bNeedsCellAttr;

public:
				ScEditAttrTester( ScEditEngineDefaulter* pEng );
				~ScEditAttrTester();

	sal_Bool				NeedsObject() const		{ return bNeedsObject; }
	sal_Bool				NeedsCellAttr() const	{ return bNeedsCellAttr; }
	const SfxItemSet&	GetAttribs() const		{ return *pEditAttrs; }
};


// construct pool before constructing EditEngine, destroy pool after EditEngine
class ScEnginePoolHelper
{
protected:
	SfxItemPool*	pEnginePool;
	SfxItemSet*		pDefaults;
	sal_Bool			bDeleteEnginePool;
	sal_Bool			bDeleteDefaults;

					ScEnginePoolHelper( SfxItemPool* pEnginePool,
						sal_Bool bDeleteEnginePool = sal_False );
					ScEnginePoolHelper( const ScEnginePoolHelper& rOrg );
	virtual			~ScEnginePoolHelper();
};


class SC_DLLPUBLIC ScEditEngineDefaulter : public ScEnginePoolHelper, public EditEngine
{
private:
    using EditEngine::SetText;

public:
					/// bDeleteEnginePool: Engine becomes the owner of the pool
					/// and deletes it on destruction
					ScEditEngineDefaulter( SfxItemPool* pEnginePool,
						sal_Bool bDeleteEnginePool = sal_False );
					/// If rOrg.bDeleteEnginePool: pool gets cloned and will be
					/// deleted on destruction. Defaults are not set.
					ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg );
	virtual			~ScEditEngineDefaulter();

					/// Creates a copy of SfxItemSet if bRememberCopy set
	void			SetDefaults( const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );

					/// Becomes the owner of the SfxItemSet if bTakeOwnership set
	void			SetDefaults( SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

					/// Set the item in the default ItemSet which is created
					/// if it doesn't exist yet.
					/// The default ItemSet is then applied to each paragraph.
	void			SetDefaultItem( const SfxPoolItem& rItem );

                    /// Returns the stored defaults, used to find non-default character attributes
    const SfxItemSet& GetDefaults();

					/// Overwritten method to be able to apply defaults already set
	void			SetText( const EditTextObject& rTextObject );
					/// Current defaults are not applied, new defaults are applied
	void			SetTextNewDefaults( const EditTextObject& rTextObject,
						const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );
					/// Current defaults are not applied, new defaults are applied
	void			SetTextNewDefaults( const EditTextObject& rTextObject,
						SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

					/// Overwritten method to be able to apply defaults already set
	void			SetText( const String& rText );
					/// Current defaults are not applied, new defaults are applied
	void			SetTextNewDefaults( const String& rText,
						const SfxItemSet& rDefaults, sal_Bool bRememberCopy = sal_True );
					/// Current defaults are not applied, new defaults are applied
	void			SetTextNewDefaults( const String& rText,
						SfxItemSet* pDefaults, sal_Bool bTakeOwnership = sal_True );

					/// Paragraph attributes that are not defaults are copied to
					/// character attributes and all paragraph attributes reset
	void			RemoveParaAttribs();

					/// Re-apply existing defaults if set, same as in SetText,
					/// but without EnableUndo/SetUpdateMode.
	void			RepeatDefaults();
};


// 1/100 mm
class SC_DLLPUBLIC ScTabEditEngine : public ScEditEngineDefaulter
{
private:
	void	Init(const ScPatternAttr& rPattern);
public:
	ScTabEditEngine( ScDocument* pDoc );			// Default
	// pEnginePool = ScDocument.GetEnginePool()
	// pTextObjectPool = ScDocument.GetEditPool()
	ScTabEditEngine( const ScPatternAttr& rPattern,
					SfxItemPool* pEnginePool,
					SfxItemPool* pTextObjectPool = NULL );
};


struct ScHeaderFieldData
{
	String		aTitle;				// Titel oder Dateiname wenn kein Titel
	String		aLongDocName;		// Pfad und Dateiname
	String		aShortDocName;		// nur Dateiname
	String		aTabName;
	Date		aDate;
	Time		aTime;
	long		nPageNo;
	long		nTotalPages;
	SvxNumType	eNumType;

	ScHeaderFieldData();
};


// fuer Feldbefehle in der Tabelle
class SC_DLLPUBLIC ScFieldEditEngine : public ScEditEngineDefaulter
{
private:
	sal_Bool	bExecuteURL;

public:
	// pEnginePool = ScDocument.GetEnginePool()
	// pTextObjectPool = ScDocument.GetEditPool()
	ScFieldEditEngine( SfxItemPool* pEnginePool,
						SfxItemPool* pTextObjectPool = NULL,
						sal_Bool bDeleteEnginePool = sal_False );

	void	SetExecuteURL(sal_Bool bSet)	{ bExecuteURL = bSet; }

	virtual void	FieldClicked( const SvxFieldItem& rField, sal_uInt16, sal_uInt16 );
	virtual String	CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );
};


// for headers/footers with fields
class SC_DLLPUBLIC ScHeaderEditEngine : public ScEditEngineDefaulter
{
private:
	ScHeaderFieldData	aData;

public:
	ScHeaderEditEngine( SfxItemPool* pEnginePool, sal_Bool bDeleteEnginePool = sal_False );
	virtual String CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor );

	void SetTitle(const String& rNew)				{ aData.aTitle = rNew; }
	void SetLongDocName(const String& rNew)			{ aData.aLongDocName = rNew; }
	void SetShortDocName(const String& rNew)		{ aData.aShortDocName = rNew; }
	void SetTabName(const String& rNew)				{ aData.aTabName = rNew; }
	void SetDate(const Date& rNew)					{ aData.aDate = rNew; }
	void SetTime(const Time& rNew)					{ aData.aTime = rNew; }
	void SetPageNo(long nNew)						{ aData.nPageNo = nNew; }
	void SetTotalPages(long nNew)					{ aData.nTotalPages = nNew; }
	void SetNumType(SvxNumType eNew)				{ aData.eNumType = eNew; }
	void SetData(const ScHeaderFieldData& rNew)		{ aData = rNew; }
};

// for Note text objects.
class ScNoteEditEngine : public ScEditEngineDefaulter
{

public:
	// pEnginePool = ScDocument.GetEnginePool()
	// pTextObjectPool = ScDocument.GetEditPool()
	ScNoteEditEngine( SfxItemPool* pEnginePool,
			    SfxItemPool* pTextObjectPool = NULL,
				sal_Bool bDeleteEnginePool = sal_False );

};

//	SvxFieldData-Ableitungen sind nach Svx verschoben


#endif


