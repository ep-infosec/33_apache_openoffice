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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <sfx2/linkmgr.hxx>

#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/datetime.hxx>

#include <svx/svdogrp.hxx>

#include <sfx2/lnkbase.hxx>
#include <tools/urlobj.hxx>

#include <svl/urihelper.hxx>

#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include "svx/svditer.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdattrx.hxx>  // NotPersistItems
#include <svx/svdoedge.hxx>  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname

#include <svx/svxids.hrc>
#include <svl/whiter.hxx>
#include <svx/svdpool.hxx>
#include <svx/sdr/properties/groupproperties.hxx>

// #110094#
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@   @@@@  @@@@@   @@@@  @@  @@ @@@@@
//  @@  @@ @@  @@     @@  @@     @@  @@ @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@ @@@ @@@@@  @@  @@ @@  @@ @@@@@
//  @@  @@ @@  @@ @@  @@  @@  @@ @@  @@ @@  @@ @@  @@ @@
//   @@@@  @@@@@   @@@@    @@@@@ @@  @@  @@@@   @@@@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrObjGroup::CreateObjectSpecificProperties()
{
	return new sdr::properties::GroupProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* SdrObjGroup::CreateObjectSpecificViewContact()
{
	return new sdr::contact::ViewContactOfGroup(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrObjGroup,SdrObject);

SdrObjGroup::SdrObjGroup()
{
	pSub=new SdrObjList(NULL,NULL);
	pSub->SetOwnerObj(this);
	pSub->SetListKind(SDROBJLIST_GROUPOBJ);
	bRefPoint=sal_False;
	bClosedObj=sal_False;
}


SdrObjGroup::~SdrObjGroup()
{
	delete pSub;
}

void SdrObjGroup::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
	rInfo.bNoContortion=sal_False;
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		SdrObjTransformInfoRec aInfo;
		pObj->TakeObjInfo(aInfo);
		if (!aInfo.bMoveAllowed            ) rInfo.bMoveAllowed            =sal_False;
		if (!aInfo.bResizeFreeAllowed      ) rInfo.bResizeFreeAllowed      =sal_False;
		if (!aInfo.bResizePropAllowed      ) rInfo.bResizePropAllowed      =sal_False;
		if (!aInfo.bRotateFreeAllowed      ) rInfo.bRotateFreeAllowed      =sal_False;
		if (!aInfo.bRotate90Allowed        ) rInfo.bRotate90Allowed        =sal_False;
		if (!aInfo.bMirrorFreeAllowed      ) rInfo.bMirrorFreeAllowed      =sal_False;
		if (!aInfo.bMirror45Allowed        ) rInfo.bMirror45Allowed        =sal_False;
		if (!aInfo.bMirror90Allowed        ) rInfo.bMirror90Allowed        =sal_False;
		if (!aInfo.bShearAllowed           ) rInfo.bShearAllowed           =sal_False;
		if (!aInfo.bEdgeRadiusAllowed	   ) rInfo.bEdgeRadiusAllowed	   =sal_False;
		if (!aInfo.bNoOrthoDesired         ) rInfo.bNoOrthoDesired         =sal_False;
		if (aInfo.bNoContortion            ) rInfo.bNoContortion           =sal_True;
		if (!aInfo.bCanConvToPath          ) rInfo.bCanConvToPath          =sal_False;

		if(!aInfo.bCanConvToContour)
			rInfo.bCanConvToContour = sal_False;

		if (!aInfo.bCanConvToPoly          ) rInfo.bCanConvToPoly          =sal_False;
		if (!aInfo.bCanConvToPathLineToArea) rInfo.bCanConvToPathLineToArea=sal_False;
		if (!aInfo.bCanConvToPolyLineToArea) rInfo.bCanConvToPolyLineToArea=sal_False;
	}
	if (nObjAnz==0) {
		rInfo.bRotateFreeAllowed=sal_False;
		rInfo.bRotate90Allowed  =sal_False;
		rInfo.bMirrorFreeAllowed=sal_False;
		rInfo.bMirror45Allowed  =sal_False;
		rInfo.bMirror90Allowed  =sal_False;
		rInfo.bTransparenceAllowed = sal_False;
		rInfo.bGradientAllowed = sal_False;
		rInfo.bShearAllowed     =sal_False;
		rInfo.bEdgeRadiusAllowed=sal_False;
		rInfo.bNoContortion     =sal_True;
	}
	if(nObjAnz != 1)
	{
		// only allowed if single object selected
		rInfo.bTransparenceAllowed = sal_False;
		rInfo.bGradientAllowed = sal_False;
	}
}


void SdrObjGroup::SetBoundRectDirty()
{
    // avoid resetting aOutRect which in case of this object is model data,
    // not re-creatable view data
}

sal_uInt16 SdrObjGroup::GetObjIdentifier() const
{
	return sal_uInt16(OBJ_GRUP);
}


SdrLayerID SdrObjGroup::GetLayer() const
{
	FASTBOOL b1st=sal_True;
	SdrLayerID nLay=SdrLayerID(SdrObject::GetLayer());
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrLayerID nLay1=pOL->GetObj(i)->GetLayer();
		if (b1st) { nLay=nLay1; b1st=sal_False; }
		else if (nLay1!=nLay) return 0;
	}
	return nLay;
}


void SdrObjGroup::NbcSetLayer(SdrLayerID nLayer)
{
	SdrObject::NbcSetLayer(nLayer);
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		pOL->GetObj(i)->NbcSetLayer(nLayer);
	}
}


void SdrObjGroup::SetObjList(SdrObjList* pNewObjList)
{
	SdrObject::SetObjList(pNewObjList);
	pSub->SetUpList(pNewObjList);
}


void SdrObjGroup::SetPage(SdrPage* pNewPage)
{
	SdrObject::SetPage(pNewPage);
	pSub->SetPage(pNewPage);
}


void SdrObjGroup::SetModel(SdrModel* pNewModel)
{
	if(pNewModel!=pModel)
	{
		// #i30648#
		// This method also needs to migrate the used ItemSet
		// when the destination model uses a different pool
		// than the current one. Else it is possible to create
		// SdrObjGroups which reference the old pool which might
		// be destroyed (as the bug shows).
		SdrModel* pOldModel = pModel;

		// test for correct pool in ItemSet; move to new pool if necessary
		if(pNewModel && GetObjectItemPool() && GetObjectItemPool() != &pNewModel->GetItemPool())
		{
			MigrateItemPool(GetObjectItemPool(), &pNewModel->GetItemPool(), pNewModel);
		}

		// call parent
		SdrObject::SetModel(pNewModel);

		// set new model at content
		pSub->SetModel(pNewModel);

		// modify properties
		GetProperties().SetModel(pOldModel, pNewModel);
	}
}


FASTBOOL SdrObjGroup::HasRefPoint() const
{
	return bRefPoint;
}


Point SdrObjGroup::GetRefPoint() const
{
	return aRefPoint;
}


void SdrObjGroup::SetRefPoint(const Point& rPnt)
{
	bRefPoint=sal_True;
	aRefPoint=rPnt;
}


SdrObjList* SdrObjGroup::GetSubList() const
{
	return pSub;
}

const Rectangle& SdrObjGroup::GetCurrentBoundRect() const
{
    // --> OD 2007-02-01 #144962#
    // <aOutRect> has to contain the bounding rectangle
    if ( pSub->GetObjCount()!=0 )
    {
        const_cast<SdrObjGroup*>(this)->aOutRect = pSub->GetAllObjBoundRect();
    }

    return aOutRect;
    // <--
}

const Rectangle& SdrObjGroup::GetSnapRect() const
{
    // --> OD 2007-02-01 #144962#
    // <aOutRect> has to contain the bounding rectangle
    if ( pSub->GetObjCount()!=0 )
    {
        return pSub->GetAllObjSnapRect();
    }
    else
    {
        return aOutRect;
    }
    // <--
}

void SdrObjGroup::operator=(const SdrObject& rObj)
{
	if(rObj.IsGroupObject())
	{
		// copy SdrObject stuff
		SdrObject::operator=(rObj);

		// #i36404#
		// copy SubList, init model and page first
		SdrObjList& rSourceSubList = *rObj.GetSubList();
		pSub->SetPage(rSourceSubList.GetPage());
		pSub->SetModel(rSourceSubList.GetModel());
		pSub->CopyObjects(*rObj.GetSubList());

		// copy local paremeters
		aRefPoint  =((SdrObjGroup&)rObj).aRefPoint;
		bRefPoint  =((SdrObjGroup&)rObj).bRefPoint;
	}
}


void SdrObjGroup::TakeObjNameSingul(XubString& rName) const
{
	if(!pSub->GetObjCount())
	{
		rName = ImpGetResStr(STR_ObjNameSingulGRUPEMPTY);
	}
	else
	{
		rName = ImpGetResStr(STR_ObjNameSingulGRUP);
	}

	const String aName(GetName());

	if(aName.Len())
	{
		rName += sal_Unicode(' ');
		rName += sal_Unicode('\'');
		rName += aName;
		rName += sal_Unicode('\'');
	}
}


void SdrObjGroup::TakeObjNamePlural(XubString& rName) const
{
	if (pSub->GetObjCount()==0) {
		rName=ImpGetResStr(STR_ObjNamePluralGRUPEMPTY);
	} else {
		rName=ImpGetResStr(STR_ObjNamePluralGRUP);
	}
}


void SdrObjGroup::RecalcSnapRect()
{
	// nicht erforderlich, da die Rects von der SubList verwendet werden.
}

basegfx::B2DPolyPolygon SdrObjGroup::TakeXorPoly() const
{
	basegfx::B2DPolyPolygon aRetval;
	const sal_uInt32 nObjCount(pSub->GetObjCount());

	for(sal_uInt32 a(0L); a < nObjCount; a++)
	{
		SdrObject* pObj = pSub->GetObj(a);
		aRetval.append(pObj->TakeXorPoly());
	}

	if(!aRetval.count())
	{
		const basegfx::B2DRange aRange(aOutRect.Left(), aOutRect.Top(), aOutRect.Right(), aOutRect.Bottom());
		aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
	}

	return aRetval;
}

bool SdrObjGroup::beginSpecialDrag(SdrDragStat& /*rDrag*/) const
{
	return false;
}


FASTBOOL SdrObjGroup::BegCreate(SdrDragStat& /*rStat*/)
{
	return sal_False;
}


long SdrObjGroup::GetRotateAngle() const
{
    const sal_uInt32 nObjCount(pSub->GetObjCount());
    long nRetval(0);

    if(nObjCount)
    {
        SdrObject* pObj = pSub->GetObj(0);

        nRetval = pObj->GetRotateAngle();
    }

    return nRetval;
}


long SdrObjGroup::GetShearAngle(FASTBOOL /*bVertical*/) const
{
    const sal_uInt32 nObjCount(pSub->GetObjCount());
    long nRetval(0);

    if(nObjCount)
    {
        SdrObject* pObj = pSub->GetObj(0);

        nRetval = pObj->GetShearAngle();
    }

    return nRetval;
}


void SdrObjGroup::NbcSetSnapRect(const Rectangle& rRect)
{
	Rectangle aOld(GetSnapRect());
	long nMulX=rRect.Right()-rRect.Left();
	long nDivX=aOld.Right()-aOld.Left();
	long nMulY=rRect.Bottom()-rRect.Top();
	long nDivY=aOld.Bottom()-aOld.Top();
	if (nDivX==0) { nMulX=1; nDivX=1; }
	if (nDivY==0) { nMulY=1; nDivY=1; }
	if (nMulX!=nDivX || nMulY!=nDivY) {
		Fraction aX(nMulX,nDivX);
		Fraction aY(nMulY,nDivY);
		NbcResize(aOld.TopLeft(),aX,aY);
	}
	if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
		NbcMove(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
	}
}


void SdrObjGroup::NbcSetLogicRect(const Rectangle& rRect)
{
	NbcSetSnapRect(rRect);
}


void SdrObjGroup::NbcMove(const Size& rSiz)
{
	MovePoint(aRefPoint,rSiz);
	if (pSub->GetObjCount()!=0) {
		SdrObjList* pOL=pSub;
		sal_uIntPtr nObjAnz=pOL->GetObjCount();
		for (sal_uIntPtr i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			pObj->NbcMove(rSiz);
		}
	} else {
		MoveRect(aOutRect,rSiz);
		SetRectsDirty();
	}
}


void SdrObjGroup::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
	FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
	FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
	if (bXMirr || bYMirr) {
		Point aRef1(GetSnapRect().Center());
		if (bXMirr) {
			Point aRef2(aRef1);
			aRef2.Y()++;
			NbcMirrorGluePoints(aRef1,aRef2);
		}
		if (bYMirr) {
			Point aRef2(aRef1);
			aRef2.X()++;
			NbcMirrorGluePoints(aRef1,aRef2);
		}
	}
	ResizePoint(aRefPoint,rRef,xFact,yFact);
	if (pSub->GetObjCount()!=0) {
		SdrObjList* pOL=pSub;
		sal_uIntPtr nObjAnz=pOL->GetObjCount();
		for (sal_uIntPtr i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			pObj->NbcResize(rRef,xFact,yFact);
		}
	} else {
		ResizeRect(aOutRect,rRef,xFact,yFact);
		SetRectsDirty();
	}
}


void SdrObjGroup::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
	SetGlueReallyAbsolute(sal_True);
	RotatePoint(aRefPoint,rRef,sn,cs);
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		pObj->NbcRotate(rRef,nWink,sn,cs);
	}
	NbcRotateGluePoints(rRef,nWink,sn,cs);
	SetGlueReallyAbsolute(sal_False);
}


void SdrObjGroup::NbcMirror(const Point& rRef1, const Point& rRef2)
{
	SetGlueReallyAbsolute(sal_True);
	MirrorPoint(aRefPoint,rRef1,rRef2); // fehlende Implementation in SvdEtc !!!
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		pObj->NbcMirror(rRef1,rRef2);
	}
	NbcMirrorGluePoints(rRef1,rRef2);
	SetGlueReallyAbsolute(sal_False);
}


void SdrObjGroup::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
	SetGlueReallyAbsolute(sal_True);
	ShearPoint(aRefPoint,rRef,tn);
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		pObj->NbcShear(rRef,nWink,tn,bVShear);
	}
	NbcShearGluePoints(rRef,nWink,tn,bVShear);
	SetGlueReallyAbsolute(sal_False);
}


void SdrObjGroup::NbcSetAnchorPos(const Point& rPnt)
{
	aAnchor=rPnt;
	Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
	MovePoint(aRefPoint,aSiz);
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	for (sal_uIntPtr i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		pObj->NbcSetAnchorPos(rPnt);
	}
}


void SdrObjGroup::SetSnapRect(const Rectangle& rRect)
{
	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
	Rectangle aOld(GetSnapRect());
	long nMulX=rRect.Right()-rRect.Left();
	long nDivX=aOld.Right()-aOld.Left();
	long nMulY=rRect.Bottom()-rRect.Top();
	long nDivY=aOld.Bottom()-aOld.Top();
	if (nDivX==0) { nMulX=1; nDivX=1; }
	if (nDivY==0) { nMulY=1; nDivY=1; }
	if (nMulX!=nDivX || nMulY!=nDivY) {
		Fraction aX(nMulX,nDivX);
		Fraction aY(nMulY,nDivY);
		Resize(aOld.TopLeft(),aX,aY);
	}
	if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
		Move(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
	}

	SetChanged();
	BroadcastObjectChange();
	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::SetLogicRect(const Rectangle& rRect)
{
	SetSnapRect(rRect);
}


void SdrObjGroup::Move(const Size& rSiz)
{
	if (rSiz.Width()!=0 || rSiz.Height()!=0) {
		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
		MovePoint(aRefPoint,rSiz);
		if (pSub->GetObjCount()!=0) {
			// #32383# Erst die Verbinder verschieben, dann den Rest
			SdrObjList* pOL=pSub;
			sal_uIntPtr nObjAnz=pOL->GetObjCount();
			sal_uIntPtr i;
			for (i=0; i<nObjAnz; i++) {
				SdrObject* pObj=pOL->GetObj(i);
				if (pObj->IsEdgeObj()) pObj->Move(rSiz);
			}
			for (i=0; i<nObjAnz; i++) {
				SdrObject* pObj=pOL->GetObj(i);
				if (!pObj->IsEdgeObj()) pObj->Move(rSiz);
			}
		} else {
			// #110094#-14 SendRepaintBroadcast();
			MoveRect(aOutRect,rSiz);
			SetRectsDirty();
		}

		SetChanged();
		BroadcastObjectChange();
		SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
	}
}


void SdrObjGroup::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
	if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
		FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
		FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
		if (bXMirr || bYMirr) {
			Point aRef1(GetSnapRect().Center());
			if (bXMirr) {
				Point aRef2(aRef1);
				aRef2.Y()++;
				NbcMirrorGluePoints(aRef1,aRef2);
			}
			if (bYMirr) {
				Point aRef2(aRef1);
				aRef2.X()++;
				NbcMirrorGluePoints(aRef1,aRef2);
			}
		}
		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
		ResizePoint(aRefPoint,rRef,xFact,yFact);
		if (pSub->GetObjCount()!=0) {
			// #32383# Erst die Verbinder verschieben, dann den Rest
			SdrObjList* pOL=pSub;
			sal_uIntPtr nObjAnz=pOL->GetObjCount();
			sal_uIntPtr i;
			for (i=0; i<nObjAnz; i++) {
				SdrObject* pObj=pOL->GetObj(i);
				if (pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
			}
			for (i=0; i<nObjAnz; i++) {
				SdrObject* pObj=pOL->GetObj(i);
				if (!pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact);
			}
		} else {
			// #110094#-14 SendRepaintBroadcast();
			ResizeRect(aOutRect,rRef,xFact,yFact);
			SetRectsDirty();
		}

		SetChanged();
		BroadcastObjectChange();
		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
	}
}


void SdrObjGroup::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
	if (nWink!=0) {
		SetGlueReallyAbsolute(sal_True);
		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
		RotatePoint(aRefPoint,rRef,sn,cs);
		// #32383# Erst die Verbinder verschieben, dann den Rest
		SdrObjList* pOL=pSub;
		sal_uIntPtr nObjAnz=pOL->GetObjCount();
		sal_uIntPtr i;
		for (i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			if (pObj->IsEdgeObj()) pObj->Rotate(rRef,nWink,sn,cs);
		}
		for (i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			if (!pObj->IsEdgeObj()) pObj->Rotate(rRef,nWink,sn,cs);
		}
		NbcRotateGluePoints(rRef,nWink,sn,cs);
		SetGlueReallyAbsolute(sal_False);
		SetChanged();
		BroadcastObjectChange();
		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
	}
}


void SdrObjGroup::Mirror(const Point& rRef1, const Point& rRef2)
{
	SetGlueReallyAbsolute(sal_True);
	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
	MirrorPoint(aRefPoint,rRef1,rRef2); // fehlende Implementation in SvdEtc !!!
	// #32383# Erst die Verbinder verschieben, dann den Rest
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	sal_uIntPtr i;
	for (i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		if (pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
	}
	for (i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		if (!pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
	}
	NbcMirrorGluePoints(rRef1,rRef2);
	SetGlueReallyAbsolute(sal_False);
	SetChanged();
	BroadcastObjectChange();
	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
	if (nWink!=0) {
		SetGlueReallyAbsolute(sal_True);
		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
		ShearPoint(aRefPoint,rRef,tn);
		// #32383# Erst die Verbinder verschieben, dann den Rest
		SdrObjList* pOL=pSub;
		sal_uIntPtr nObjAnz=pOL->GetObjCount();
		sal_uIntPtr i;
		for (i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			if (pObj->IsEdgeObj()) pObj->Shear(rRef,nWink,tn,bVShear);
		}
		for (i=0; i<nObjAnz; i++) {
			SdrObject* pObj=pOL->GetObj(i);
			if (!pObj->IsEdgeObj()) pObj->Shear(rRef,nWink,tn,bVShear);
		}
		NbcShearGluePoints(rRef,nWink,tn,bVShear);
		SetGlueReallyAbsolute(sal_False);
		SetChanged();
		BroadcastObjectChange();
		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
	}
}


void SdrObjGroup::SetAnchorPos(const Point& rPnt)
{
	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
	FASTBOOL bChg=aAnchor!=rPnt;
	aAnchor=rPnt;
	Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
	MovePoint(aRefPoint,aSiz);
	// #32383# Erst die Verbinder verschieben, dann den Rest
	SdrObjList* pOL=pSub;
	sal_uIntPtr nObjAnz=pOL->GetObjCount();
	sal_uIntPtr i;
	for (i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		if (pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
	}
	for (i=0; i<nObjAnz; i++) {
		SdrObject* pObj=pOL->GetObj(i);
		if (!pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
	}
	if (bChg) {
		SetChanged();
		BroadcastObjectChange();
		SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
	}
}



void SdrObjGroup::NbcSetRelativePos(const Point& rPnt)
{
	Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
	Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
	NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

void SdrObjGroup::SetRelativePos(const Point& rPnt)
{
	Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
	Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
	if (aSiz.Width()!=0 || aSiz.Height()!=0) Move(aSiz); // Der ruft auch das SetRectsDirty() und Broadcast, ...
}

void SdrObjGroup::NbcReformatText()
{
	pSub->NbcReformatAllTextObjects();
}

void SdrObjGroup::ReformatText()
{
	pSub->ReformatAllTextObjects();
}

SdrObject* SdrObjGroup::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
	SdrObject* pGroup = new SdrObjGroup;
	pGroup->SetModel(GetModel());

	for(sal_uInt32 a=0;a<pSub->GetObjCount();a++)
	{
		SdrObject* pIterObj = pSub->GetObj(a);
        SdrObject* pResult = pIterObj->DoConvertToPolyObj(bBezier, bAddText);

        // pResult can be NULL e.g. for empty objects
        if( pResult )
            pGroup->GetSubList()->NbcInsertObject(pResult);
	}

	return pGroup;
}

// eof
