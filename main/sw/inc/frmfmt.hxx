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


#ifndef _FRMFMT_HXX
#define _FRMFMT_HXX

#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <cppuhelper/weakref.hxx>
#include <tools/gen.hxx>
#include <format.hxx>
#include "swdllapi.h"

class SwFlyFrm;
class SwAnchoredObject;
class Graphic;
class Point;
class ImageMap;
class IMapObject;
class SwRect;
class SwContact;
class SdrObject;

class SW_DLLPUBLIC SwFrmFmt: public SwFmt
{
	friend class SwDoc;
	friend class SwPageDesc;	//darf den protected CTor rufen.

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> m_wXObject;

    //UUUU DrawingLayer FillAttributes in a preprocessed form for primitive usage
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFillAttributes;

protected:
    SwFrmFmt* pCaptionFmt;

    SwFrmFmt( 
        SwAttrPool& rPool, 
        const sal_Char* pFmtNm,
        SwFrmFmt *pDrvdFrm, 
        sal_uInt16 nFmtWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = 0);

    SwFrmFmt( 
        SwAttrPool& rPool, 
        const String &rFmtNm,
        SwFrmFmt *pDrvdFrm, 
        sal_uInt16 nFmtWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = 0);

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:
	TYPEINFO();		//Bereits in Basisklasse Client drin.
    //UUUU
    ~SwFrmFmt();

	//Vernichtet alle Frms in aDepend (Frms werden per PTR_CAST erkannt).
	virtual void DelFrms();

	//Erzeugt die Ansichten
	virtual void MakeFrms();

	virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

	// returnt das IMapObject, das an dem Format (Fly), in der ImageMap
	// an der Point Position definiert ist.
	// 	rPoint - teste auf der DocPosition
	//	pFly - optionaler FlyFrame, falls der schon bekannt ist.
	IMapObject* GetIMapObject( const Point& rPoint,
								const SwFlyFrm *pFly = 0 ) const;

	// Gibt die tatsaechlche Groesse des Frames zurueck bzw. ein leeres
	// Rechteck, wenn kein Layout existiert. Wird pPoint angegeben, dann
	// wird der am dichtesten liegende Frame gesucht.
	SwRect FindLayoutRect( const sal_Bool bPrtArea = sal_False,
							const Point* pPoint = 0,
							const sal_Bool bCalcFrm = sal_False ) const;

	// Sucht das SdrObject. Der SdrObjUserCall ist Client vom Format.
	// Der UserCall kennt sein SdrObject.
		  SwContact *FindContactObj();
	const SwContact *FindContactObj() const
		{ return ((SwFrmFmt*)this)->FindContactObj(); }

	// returns the SdrObject, that ist connected to the ContactObject.
	// Only DrawFrmFmts are connected to the "real SdrObject". FlyFrmFmts
	// are connected to a Master and all FlyFrms has the "real SdrObject".
	// "Real SdrObject" has position and a Z-order.
		  SdrObject *FindSdrObject();
	const SdrObject *FindSdrObject() const
		{ return ((SwFrmFmt*)this)->FindSdrObject(); }

		  SdrObject *FindRealSdrObject();
	const SdrObject *FindRealSdrObject() const
		{ return ((SwFrmFmt*)this)->FindRealSdrObject(); }

	sal_Bool IsLowerOf( const SwFrmFmt& rFmt ) const;

    // --> OD 2004-07-27 #i31698#
    enum tLayoutDir
    {
        HORI_L2R,
        HORI_R2L,
        VERT_R2L,
        VERT_L2R    // not supported yet
    };

    virtual SwFrmFmt::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir );
    // <--

    // --> OD 2004-08-06 #i28749#
    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );
    // <--

    virtual String GetDescription() const;
	sal_Bool HasCaption() const;
	void SetCaptionFmt(SwFrmFmt* pFmt);
	SwFrmFmt* GetCaptionFmt() const;

    SW_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::uno::XInterface> const& GetXObject() const
            { return m_wXObject; }
    SW_DLLPRIVATE void SetXObject(::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XInterface> const& xObject)
            { m_wXObject = xObject; }

	DECL_FIXEDMEMPOOL_NEWDEL_DLL(SwFrmFmt)
    void RegisterToFormat( SwFmt& rFmt );

    //UUUU Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;
};

//Das FlyFrame-Format ------------------------------

class SW_DLLPUBLIC SwFlyFrmFmt: public SwFrmFmt
{
	friend class SwDoc;
	String msTitle;
	String msDesc;

    // #i972:
    // it stores the previous position of Prt rectangle from RequestObjectResize
    // so it can be used to move frames of non-resizable objects to align them correctly
    // when they get borders (this is done in SwWrtShell::CalcAndGetScale)
    Point   m_aLastFlyFrmPrtRectPos; 
    
    //Beide nicht vorhanden.
	SwFlyFrmFmt( const SwFlyFrmFmt &rCpy );
	SwFlyFrmFmt &operator=( const SwFlyFrmFmt &rCpy );

protected:
	SwFlyFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
					SwFrmFmt *pDrvdFrm )
		: SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FLYFRMFMT )
	{}
	SwFlyFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
					SwFrmFmt *pDrvdFrm )
		: SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FLYFRMFMT )
	{}

public:
	TYPEINFO();
	~SwFlyFrmFmt();

	//Erzeugt die Ansichten
	virtual void MakeFrms();

	SwFlyFrm* GetFrm( const Point* pDocPos = 0,
						const sal_Bool bCalcFrm = sal_False ) const;

    SwAnchoredObject* GetAnchoredObj( const Point* pDocPos = 0,
                                      const sal_Bool bCalcFrm = sal_False ) const;

	virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

	virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;

    // --> OD 2009-07-14 #i73249#
    const String GetObjTitle() const;
    void SetObjTitle( const String& rTitle,
                      bool bBroadcast = false );
    const String GetObjDescription() const;
    void SetObjDescription( const String& rDescription,
                            bool bBroadcast = false );
    // <--

    /** SwFlyFrmFmt::IsBackgroundTransparent - for #99657#

        OD 22.08.2002 - overloading virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @author OD

        @return true, if background color is transparent, but not "no fill"
        or a existing background graphic is transparent.
    */
    virtual sal_Bool IsBackgroundTransparent() const;

    /** SwFlyFrmFmt::IsBackgroundBrushInherited - for #103898#

        OD 08.10.2002 - method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @author OD

        @return true, if background brush is "inherited" from parent/grandparent
    */
    sal_Bool IsBackgroundBrushInherited() const;

    const Point & GetLastFlyFrmPrtRectPos() const       { return m_aLastFlyFrmPrtRectPos; }
    void SetLastFlyFrmPrtRectPos( const Point &rPoint ) { m_aLastFlyFrmPrtRectPos = rPoint; }

	DECL_FIXEDMEMPOOL_NEWDEL(SwFlyFrmFmt)
};

//Das DrawFrame-Format -----------------------------

class SW_DLLPUBLIC SwDrawFrmFmt: public SwFrmFmt
{
	friend class SwDoc;

    mutable const SdrObject * pSdrObjCached;
    mutable String sSdrObjCachedComment;

	//Beide nicht vorhanden.
	SwDrawFrmFmt( const SwDrawFrmFmt &rCpy );
	SwDrawFrmFmt &operator=( const SwDrawFrmFmt &rCpy );

    // --> OD 2004-07-27 #i31698#
    SwFrmFmt::tLayoutDir meLayoutDir;
    // <--
    // --> OD 2004-08-06 #i28749#
    sal_Int16 mnPositionLayoutDir;
    // <--
    // --> OD 2005-03-11 #i44334#, #i44681#
    bool mbPosAttrSet;
    // <--
protected:
	SwDrawFrmFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
					SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),
          // --> OD 2004-07-28 #i31698#
          meLayoutDir( SwFrmFmt::HORI_L2R ),
          // <--
          // --> OD 2004-08-06 #i28749#
          // --> OD 2005-03-10 #i44344#, #i44681# - undo change of issue #i36010#
          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),
          // <--
          // --> OD 2005-03-11 #i44334#, #i44681#
          mbPosAttrSet( false )
          // <--

	{}
	SwDrawFrmFmt( SwAttrPool& rPool, const String &rFmtNm,
					SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_DRAWFRMFMT ),
          pSdrObjCached(NULL),
          // --> OD 2004-07-28 #i31698#
          meLayoutDir( SwFrmFmt::HORI_L2R ),
          // <--
          // --> OD 2004-08-06 #i28749#
          // --> OD 2005-03-10 #i44344#, #i44681# - undo change of issue #i36010#
          mnPositionLayoutDir( com::sun::star::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),
          // <--
          // --> OD 2005-03-11 #i44334#, #i44681#
          mbPosAttrSet( false )
          // <--
	{}

public:
	TYPEINFO();
	~SwDrawFrmFmt();

	//DrawObjecte werden aus den Arrays am Layout entfernt. Die DrawObjecte
	//werden als geloescht gekennzeichnet.
	virtual void DelFrms();

	//Anmelden der DrawObjecte in den Arrays am Layout. Loeschkennzeichen
	//werden zurueckgesetzt.
	virtual void MakeFrms();

	virtual Graphic MakeGraphic( ImageMap* pMap = NULL );

    // --> OD 2004-07-27 #i31698#
    virtual SwFrmFmt::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrmFmt::tLayoutDir _eLayoutDir );
    // <--

    // --> OD 2004-08-06 #i28749#
    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );
    // <--

    // --> OD 2005-03-11 #i44334#, #i44681#
    inline bool IsPosAttrSet() const { return mbPosAttrSet; }
    inline void PosAttrSet() { mbPosAttrSet = true; }
    // <--

    // --> OD 2005-08-16 #i53320#
    inline void ResetPosAttr()
    {
        mbPosAttrSet = false;
    }
    // <--

    virtual String GetDescription() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwDrawFrmFmt);
};


#endif

