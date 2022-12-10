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



#ifndef _SVDOTEXT_HXX
#define _SVDOTEXT_HXX

#include <vcl/field.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdtrans.hxx> // GeoStat
#include <tools/datetime.hxx>
#include <svx/xtextit0.hxx>
#include "svdtext.hxx"
#include <vector>
#include <boost/shared_ptr.hpp>
#include "svx/svxdllapi.h"
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svx/svdpagv.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SvxFieldItem;
class ImpSdrObjTextLink;
class EditStatus;

namespace sdr { namespace properties {
	class TextProperties;
}}

namespace drawinglayer { namespace primitive2d {
	class SdrContourTextPrimitive2D;
	class SdrPathTextPrimitive2D;
	class SdrBlockTextPrimitive2D;
	class SdrStretchTextPrimitive2D;
}}

namespace drawinglayer { namespace animation {
	class AnimationEntryList;
}}

namespace drawinglayer { namespace geometry {
	class ViewInformation2D;
}}

namespace sdr { namespace table {
	class Cell;
	class SdrTableRtfExporter;
	class SdrTableRTFParser;
}}

//************************************************************
//   Defines
//************************************************************

#define SDRUSERDATA_OBJTEXTLINK			(1)

//************************************************************
//   Hilfsklasse SdrTextObjGeoData
//************************************************************

class SdrTextObjGeoData : public SdrObjGeoData
{
public:
	Rectangle					aRect;
	GeoStat						aGeo;
};

//************************************************************
//   Hilfsklasse ImpSdrObjTextLinkUserData
//************************************************************

class ImpSdrObjTextLinkUserData : public SdrObjUserData
{
	friend class				SdrTextObj;
	friend class				ImpSdrObjTextLink;

	SdrTextObj*					pObj;
	String						aFileName;   // Name des referenzierten Dokuments
	String						aFilterName; // ggf. ein Filter
	DateTime					aFileDate0;  // Unnoetiges neuladen vermeiden
	ImpSdrObjTextLink*			pLink;
	rtl_TextEncoding			eCharSet;

public:
	TYPEINFO();
	ImpSdrObjTextLinkUserData(SdrTextObj* pObj1);
	virtual ~ImpSdrObjTextLinkUserData();

	virtual SdrObjUserData* Clone(SdrObject* pObj1) const;
};

namespace sdr
{
	namespace properties
	{
		class CustomShapeProperties;
		class CellProperties;
	} // end of namespace properties
} // end of namespace sdr

//************************************************************
//   SdrTextObj
//************************************************************

class SVX_DLLPUBLIC SdrTextObj : public SdrAttrObj
{
private:
	// Cell needs access to ImpGetDrawOutliner();

	friend class				sdr::table::Cell;
	friend class				sdr::table::SdrTableRtfExporter;
	friend class				sdr::table::SdrTableRTFParser;

	// CustomShapeproperties need to access the "bTextFrame" member:
	friend class sdr::properties::CustomShapeProperties;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
	virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
	// This method is only allowed for sdr::properties::TextProperties
	SVX_DLLPRIVATE SdrOutliner* GetTextEditOutliner() const 
	{ 
		return pEdtOutl; 
	}

	// This method is only allowed for sdr::properties::TextProperties
	SVX_DLLPRIVATE void SetPortionInfoChecked(sal_Bool bNew)
	{
		bPortionInfoChecked = bNew;
	}

	// to allow sdr::properties::TextProperties access to SetPortionInfoChecked()
	// and GetTextEditOutliner()
	friend class sdr::properties::TextProperties;
	friend class sdr::properties::CellProperties;

	friend class				ImpTextPortionHandler;
	friend class				ImpSdrObjTextLink;
	friend class				ImpSdrObjTextLinkUserData;
	friend class				SdrPowerPointImport; // fuer PowerPointImport
	friend class				SdrExchangeView; // fuer ImpGetDrawOutliner
	friend class				SdrView;         // fuer ImpGetDrawOutliner
	friend class				SdrObjEditView;  // fuer TextEdit
	friend class				SdrMeasureObj;   // fuer ImpGetDrawOutliner
	friend class				SvxMSDffManager; // fuer ImpGetDrawOutliner
	friend class				SdrObjCustomShape;// fuer ImpGetDrawOutliner
	friend class				SdrText;		// fuer ImpGetDrawOutliner

protected:
	// Das aRect ist gleichzeig auch das Rect vom RectObj und CircObj.
	// Bei bTextFrame=sal_True wird der Text in dieses Rect hineinformatiert.
	// Bei bTextFrame=sal_False wird der Text am Mittelpunkt des Rect zentriert.
	Rectangle					aRect;

	// Der GeoStat enthaelt den Drehwinkel und einen Shearwinkel
	GeoStat						aGeo;

	// this is the active text
	SdrText*					mpText;

	// Hier merke ich mir die Ausmasse des Textes (n.i.)
	Size						aTextSize;

	// Ein Outliner*, damit
	// 1. das TextObj nicht von mehreren Views gleichzeitig editiert und
	// 2. beim Streamen waerend des Editierens ein Flush() ausgefuehrt
	// werden kann
	SdrOutliner*   				pEdtOutl;

	// Bei Fontwork muss soviel auf's BoundRect draufgerechnet werden
	// damit es ausreichend gross ist.
	Rectangle*					pFormTextBoundRect;

	// Moegliche Werte fuer eTextKind sind:
	//     OBJ_TEXT         normaler Textrahmen
	//     OBJ_TEXTEXT      Textfortsetzungsrahmen
	//     OBJ_TITLETEXT    TitleText fuer StarDraw-Praesentation
	//     OBJ_OUTLINETEXT  OutlineText fuer StarDraw-Praesentation
	// eTextKind hat nur Bedeutung, wenn bTextFrame=sal_True, da es sich sonst
	// um ein beschriftetes Grafikobjekt handelt.
	SdrObjKind					eTextKind;

	// #108784#
	// For text editing in SW Haeder/Footer it is necessary to be
	// able to set an offset for the text edit to allow text editing at the
	// position of the virtual object. This offset is used when setting up
	// and maintaining the OutlinerView.
	Point						maTextEditOffset;
public:
	const Point& GetTextEditOffset() const { return maTextEditOffset; }
	void SetTextEditOffset(const Point& rNew) { maTextEditOffset = rNew; }

protected:
	// Fuer beschriftete Zeichenobjekte ist bTextFrame=FALSE. Der Textblock
	// wird dann hoizontal und vertikal an aRect zentriert. Bei bTextFrame=
	// sal_True wird der Text in aRect hineinformatiert. Der eigentliche Textrahmen
	// ist durch ein SdrRectObj mit bTextFrame=sal_True realisiert.
	sal_Bool						bTextFrame : 1;
	sal_Bool						bPortionInfoChecked : 1; // Fuer Optimierung von Textobjekten
	sal_Bool						bNoShear : 1;            // Obj darf nicht gesheart werden   (->Graf+Ole+TextFrame)
	sal_Bool						bNoRotate : 1;           // Obj darf nicht gedreht werden    (->Ole)
	sal_Bool						bNoMirror : 1;           // Obj darf nicht gespiegelt werden (->Ole,TextFrame)
	sal_Bool						bTextSizeDirty : 1;

	// #101684#
	sal_Bool						mbInEditMode : 1;	// Is this text obejct in edit mode?

	// Fuer Objekt mit freier Groesse im Draw (Mengentext). Das Flag wird vom
	// der App beim Create gesetzt.
	// Wenn das Objekt dann spaeter in der Breite resized wird, wird
	// AutoGrowWidth abgeschaltet (Hart auf sal_False attributiert). Das Flag wird
	// dann ebenfalls auf sal_False gesetzt, sodass sich das Objekt anschliessend
	// wie ein normales Textobjekt verhaelt.
	// Resize in der Breite kann sein:
	// - Interaktives Resize in Einfach- oder Mehrfachselektion
	// - Positions+Groesse Dialog
	sal_Bool						bDisableAutoWidthOnDragging : 1;

	// #111096#
	// Allow text suppression
	sal_Bool						mbTextHidden : 1;

	// #111096#
	// Flag for allowing text animation. Default is sal_true.
	sal_Bool						mbTextAnimationAllowed : 1;

	SdrOutliner& ImpGetDrawOutliner() const;

private:
	SVX_DLLPRIVATE void ImpCheckMasterCachable();
	// #101029#: Extracted from ImpGetDrawOutliner()
    SVX_DLLPRIVATE void ImpInitDrawOutliner( SdrOutliner& rOutl ) const;
	// #101029#: Extracted from Paint()
    SVX_DLLPRIVATE void ImpSetupDrawOutlinerForPaint( FASTBOOL 		bContourFrame, 
                                       SdrOutliner& 	rOutliner, 
                                       Rectangle& 		rTextRect, 
                                       Rectangle& 		rAnchorRect, 
                                       Rectangle& 		rPaintRect, 
                                       Fraction& 		aFitXKorreg ) const;
	SVX_DLLPRIVATE SdrObject* ImpConvertContainedTextToSdrPathObjs(bool bToPoly) const;
	SVX_DLLPRIVATE void ImpLinkAnmeldung();
	SVX_DLLPRIVATE void ImpLinkAbmeldung();
	SVX_DLLPRIVATE ImpSdrObjTextLinkUserData* GetLinkUserData() const;
//	void ImpCheckItemSetChanges(const SfxItemSet& rAttr);

protected:
	bool ImpCanConvTextToCurve() const;
	SdrObject* ImpConvertMakeObj(const basegfx::B2DPolyPolygon& rPolyPolygon, sal_Bool bClosed, sal_Bool bBezier, sal_Bool bNoSetAttr = sal_False) const;
	SdrObject* ImpConvertAddText(SdrObject* pObj, FASTBOOL bBezier) const;
	void ImpSetTextStyleSheetListeners();
	void ImpSetCharStretching(SdrOutliner& rOutliner, const Rectangle& rTextRect, const Rectangle& rAnchorRect, Fraction& rFitXKorreg) const;
	void ImpJustifyRect(Rectangle& rRect) const;
	void ImpCheckShear();
	Rectangle ImpDragCalcRect(const SdrDragStat& rDrag) const;
	void ImpSetTextEditParams() const;
	void SetTextSizeDirty() { bTextSizeDirty=sal_True; }

	// rAnchorRect ist InOut-Parameter!
	void ImpSetContourPolygon( SdrOutliner& rOutliner, Rectangle& rAnchorRect, sal_Bool bLineWidth ) const;

	virtual SdrObjGeoData* NewGeoData() const;
	virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
	virtual void RestGeoData(const SdrObjGeoData& rGeo);
	FASTBOOL NbcSetEckenradius(long nRad);
	FASTBOOL NbcSetAutoGrowHeight(bool bAuto);
	FASTBOOL NbcSetMaxTextFrameHeight(long nHgt);
	FASTBOOL NbcSetAutoGrowWidth(bool bAuto);
	FASTBOOL NbcSetMaxTextFrameWidth(long nWdt);
	FASTBOOL NbcSetFitToSize(SdrFitToSizeType eFit);

    // #115391# new method for SdrObjCustomShape and SdrTextObj to correctly handle and set
    // SdrTextMinFrameWidthItem and SdrTextMinFrameHeightItem based on all settings, necessities
    // and object sizes
    virtual void AdaptTextMinSize();

	// Konstruktoren fuer beschriftete Zeichenobjekte
	SdrTextObj();
	SdrTextObj(const Rectangle& rNewRect);

	// Konstruktoren fuer Textrahmen
	SdrTextObj(SdrObjKind eNewTextKind);
	SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect);

	// der sal_uInt16 eFormat nimmt Werte des enum EETextFormat entgegen
    SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect, SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);
	virtual ~SdrTextObj();

public:
	TYPEINFO();

	// #101684#
	sal_Bool IsInEditMode() const { return mbInEditMode; }

	// via eCharSet kann der CharSet der vorliegenden Datei uebergeben werden.
	// Bei RTL_TEXTENCODING_DONTKNOW wird der CharSet der aktuellen Plattform verwendet.
	// Derzeit unterstuetzt wird ASCII und RTF wobei ich die Unterscheidung
	// selbst treffe. rFilterName ist noch ohne Bedeutung und muss leer gelassen
	// werden.
	// I.d.R. muss die App nur SetTextLink() rufen - der Rest geschieht von
	// selbst (SfxLinkManager). Die Methode LoadText() kann ausserdem verwendet
	// werden, um eine Datei in ein Textobjekt zu laden (ohne Verknuepfung).
	// TextLinks koennen nicht editiert werden (allenfalls spaeter mal ReadOnly).
	// Eine Attributierung kann nur am Textrahmen vollzogen werden.
	void SetTextLink(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet);
	void ReleaseTextLink();
	FASTBOOL IsLinkedText() const { return pPlusData!=NULL && GetLinkUserData()!=NULL; }
    FASTBOOL ReloadLinkedText(FASTBOOL bForceLoad=sal_False);
    FASTBOOL LoadText(const String& rFileName, const String& rFilterName, rtl_TextEncoding eCharSet);

	virtual FASTBOOL AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt=sal_True, FASTBOOL bWdt=sal_True) const;
	virtual FASTBOOL NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt=sal_True, FASTBOOL bWdt=sal_True);
	virtual FASTBOOL AdjustTextFrameWidthAndHeight(FASTBOOL bHgt=sal_True, FASTBOOL bWdt=sal_True);
	void NbcResizeTextAttributes(const Fraction& xFact, const Fraction& yFact);
	FASTBOOL IsTextFrame() const { return bTextFrame; }
	FASTBOOL IsOutlText() const { return bTextFrame && (eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT); }
	SdrObjKind GetTextKind() const { return eTextKind; }

    // #121917#
    virtual bool HasText() const;

	FASTBOOL HasEditText() const;
	sal_Bool IsTextEditActive() const { return (pEdtOutl != 0L); }

	/** returns the currently active text. */
	virtual SdrText* getActiveText() const;

	/** returns the nth available text. */
	virtual SdrText* getText( sal_Int32 nIndex ) const;

	/** returns the number of texts available for this object. */
	virtual sal_Int32 getTextCount() const;

	/** returns true only if we are in edit mode and the user actually changed anything */
	virtual bool IsRealyEdited() const;

	/** changes the current active text */
	virtual void setActiveText( sal_Int32 nIndex );

	/** returns the index of the text that contains the given point or -1 */
	virtual sal_Int32 CheckTextHit(const Point& rPnt) const;

	void SetDisableAutoWidthOnDragging(FASTBOOL bOn) { bDisableAutoWidthOnDragging=bOn; }
	FASTBOOL IsDisableAutoWidthOnDragging() { return bDisableAutoWidthOnDragging; }
	void NbcSetText(const String& rStr);
	void SetText(const String& rStr);
    void NbcSetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);
    void SetText(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);

	// FitToSize und Fontwork wird bei GetTextSize() nicht berueksichtigt!
	virtual const Size& GetTextSize() const;
	void FitFrameToTextSize();

	// Gleichzeitig wird der Text in den Outliner gesetzt (ggf.
	// der des EditOutliners) und die PaperSize gesetzt.
	virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText=sal_False,
		Rectangle* pAnchorRect=NULL, sal_Bool bLineWidth=sal_True ) const;
	virtual void TakeTextAnchorRect(::Rectangle& rAnchorRect) const;
	const GeoStat& GetGeoStat() const { return aGeo; }

	long GetEckenradius() const;
	virtual FASTBOOL IsAutoGrowHeight() const;
	long GetMinTextFrameHeight() const;
	long GetMaxTextFrameHeight() const;
	virtual FASTBOOL IsAutoGrowWidth() const;
	long GetMinTextFrameWidth() const;
	long GetMaxTextFrameWidth() const;

	SdrFitToSizeType GetFitToSize() const;
	const Rectangle &GetGeoRect() const { return aRect; }

	// Feststellen, ob TextFontwork
	virtual FASTBOOL IsFontwork() const;

	// Soll die Fontwork-Kontour versteckt werden?
	FASTBOOL IsHideContour() const;

	// Textfluss innerhalb Kontur
	FASTBOOL IsContourTextFrame() const;

	// Horizontale Textausrichtung
	SdrTextHorzAdjust GetTextHorizontalAdjust(const SfxItemSet& rSet) const;
	SdrTextHorzAdjust GetTextHorizontalAdjust() const;

	// Vertikale Textausrichtung
	SdrTextVertAdjust GetTextVerticalAdjust(const SfxItemSet& rSet) const;
	SdrTextVertAdjust GetTextVerticalAdjust() const;

	// Textrahmenabstaende
	long GetTextLeftDistance() const;
	long GetTextRightDistance() const;
	long GetTextUpperDistance() const;
	long GetTextLowerDistance() const;
	SdrTextAniKind GetTextAniKind() const;
	SdrTextAniDirection GetTextAniDirection() const;

	virtual void SetPage(SdrPage* pNewPage);
	virtual void SetModel(SdrModel* pNewModel);
	virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
	virtual sal_uInt16 GetObjIdentifier() const;

	// Wird zur Bestimmung des Textankerbereichs benoetigt
	virtual void TakeUnrotatedSnapRect(Rectangle& rRect) const;
	virtual void TakeObjNameSingul(String& rName) const;
	virtual void TakeObjNamePlural(String& rName) const;
	virtual void operator=(const SdrObject& rObj);
	virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
	virtual basegfx::B2DPolyPolygon TakeContour() const;
	virtual void RecalcSnapRect();
	virtual void NbcSetSnapRect(const Rectangle& rRect);
	virtual void NbcSetLogicRect(const Rectangle& rRect);
	virtual const Rectangle& GetLogicRect() const;
	virtual long GetRotateAngle() const;
	virtual long GetShearAngle(FASTBOOL bVertical=sal_False) const;

	virtual sal_uInt32 GetSnapPointCount() const;
	virtual Point GetSnapPoint(sal_uInt32 i) const;

	virtual sal_uInt32 GetHdlCount() const;
	virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const;
	
    // special drag methods
    virtual bool hasSpecialDrag() const;
	virtual bool applySpecialDrag(SdrDragStat& rDrag);
	virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

	virtual FASTBOOL BegCreate(SdrDragStat& rStat);
	virtual FASTBOOL MovCreate(SdrDragStat& rStat);
	virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
	virtual FASTBOOL BckCreate(SdrDragStat& rStat);
	virtual void BrkCreate(SdrDragStat& rStat);
	virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
	virtual Pointer GetCreatePointer() const;

	virtual void NbcMove(const Size& rSiz);
	virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
	virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
	virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
	virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);

	virtual FASTBOOL HasTextEdit() const;
	virtual sal_Bool BegTextEdit(SdrOutliner& rOutl);
	virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
	virtual void EndTextEdit(SdrOutliner& rOutl);
	virtual sal_uInt16 GetOutlinerViewAnchorMode() const;

	void StartTextAnimation(OutputDevice* pOutDev, const Point& rOffset, long nExtraData=0L);
	void StopTextAnimation(OutputDevice* pOutDev=NULL, long nExtraData=0L);

	virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);
	void NbcSetOutlinerParaObjectForText( OutlinerParaObject* pTextObject, SdrText* pText );
	virtual OutlinerParaObject* GetOutlinerParaObject() const;
	virtual OutlinerParaObject* GetEditOutlinerParaObject() const;
	virtual bool HasOutlinerParaObject() const;

	virtual void NbcReformatText();
	virtual void ReformatText();

	virtual FASTBOOL CalcFieldValue(const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
		FASTBOOL bEdit, Color*& rpTxtColor, Color*& rpFldColor, String& rRet) const;

	virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

	void SetTextEditOutliner(SdrOutliner* pOutl) { pEdtOutl=pOutl; }

    /** Setup given Outliner equivalently to SdrTextObj::Paint()

    	To setup an arbitrary Outliner in the same way as the draw
    	outliner on SdrTextObj::Paint(). Among others, the paper size,
    	control word and character stretching are initialized, such
    	that the formatting should match the screen representation.
    	The textual content of the outliner is not touched, i.e. no
    	Init() or Clear() is called on the Outliner.

        @param rOutl
        The Outliner to setup.

        @param rPaintRect
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void SetupOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const;

    /** Update given Outliner equivalently to SdrTextObj::Paint()

    	Same functionality as in SetupOutlinerFormatting(), except
    	that the outliner content is not cleared.

        @param rOutl
        The Outliner to update.

        @param rPaintRect
        The region to paint the outliner content into. This is useful
        to e.g. determine the top, left position of text in shapes.
     */
    void UpdateOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const;
	void ForceOutlinerParaObject();
	virtual sal_Bool IsVerticalWriting() const;
	virtual void SetVerticalWriting(sal_Bool bVertical);

	/** called from the SdrObjEditView during text edit when the status of the edit outliner changes */
	virtual void onEditOutlinerStatusEvent( EditStatus* pEditStatus );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// transformation interface for StarOfficeAPI. This implements support for
	// homogen 3x3 matrices containing the transformation of the SdrObject. At the
	// moment it contains a shearX, rotation and translation, but for setting all linear
	// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
	// with the base geometry and returns TRUE. Otherwise it returns FALSE.
	virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;

	// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
	// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
	// to use (0,0) as upper left and will be scaled to the given size in the matrix.
	virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);

	// #103836# iterates over the paragraphs of a given SdrObject and removes all
	//			hard set character attributes with the which ids contained in the
	//			given vector
	virtual void RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds );

	// #111096#
	// Access to thext hidden flag
	sal_Bool GetTextHidden() const;
	void NbcSetTextHidden(sal_Bool bNew);

	// #111096#
	// Get necessary data for text scroll animation. ATM base it on a Text-Metafile and a
	// painting rectangle. Rotation is taken from the object.
	GDIMetaFile* GetTextScrollMetaFileAndRectangle(Rectangle& rScrollRectangle, Rectangle& rPaintRectangle);

	// #111096#
	// Access to TextAnimationAllowed flag
	bool IsTextAnimationAllowed() const;
	void SetTextAnimationAllowed(sal_Bool bNew);

    // #i8824#
    // Set single item at the local ItemSet. *Does not use* AllowItemChange(),
	// ItemChange(), PostItemChange() and ItemSetChanged() calls.
	void SetObjectItemNoBroadcast(const SfxPoolItem& rItem);

public:
	//////////////////////////////////////////////////////////////////////////////
	// text primitive decomposition helpers
	void impDecomposeContourTextPrimitive(
		drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
		const drawinglayer::primitive2d::SdrContourTextPrimitive2D& rSdrContourTextPrimitive,
		const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
	void impDecomposePathTextPrimitive(
		drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
		const drawinglayer::primitive2d::SdrPathTextPrimitive2D& rSdrPathTextPrimitive,
		const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
	void impDecomposeBlockTextPrimitive(
		drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
		const drawinglayer::primitive2d::SdrBlockTextPrimitive2D& rSdrBlockTextPrimitive,
		const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;
	void impDecomposeStretchTextPrimitive(
		drawinglayer::primitive2d::Primitive2DSequence& rTarget, 
		const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
		const drawinglayer::geometry::ViewInformation2D& aViewInformation) const;

	//////////////////////////////////////////////////////////////////////////////
	// timing generators
	void impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const;
	void impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const;

	/** returns false if the given pointer is NULL
		or if the given SdrOutliner contains no text.
		Also checks for one empty paragraph.
	*/
	static bool HasTextImpl( SdrOutliner* pOutliner );

	/** returns a new created and non shared outliner.
		The outliner will not get updated when the SdrModel is changed.
	*/
	boost::shared_ptr< SdrOutliner > CreateDrawOutliner();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOTEXT_HXX

