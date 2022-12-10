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

#include <svx/svdopage.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/itemset.hxx>
#include <svx/sdr/properties/pageproperties.hxx>

// #111111#
#include <svx/sdr/contact/viewcontactofpageobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrPageObj::CreateObjectSpecificProperties()
{
	return new sdr::properties::PageProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrPageObj::CreateObjectSpecificViewContact()
{
	return new sdr::contact::ViewContactOfPageObj(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// this method is called form the destructor of the referenced page.
// do all necessary action to forget the page. It is not necessary to call
// RemovePageUser(), that is done form the destructor.
void SdrPageObj::PageInDestruction(const SdrPage& rPage)
{
	if(mpShownPage && mpShownPage == &rPage)
	{
        // #i58769# Do not call ActionChanged() here, because that would
        // lead to the construction of a view contact object for a page that
        // is being destroyed.

        mpShownPage = 0L;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrPageObj,SdrObject);

SdrPageObj::SdrPageObj(SdrPage* pNewPage)
:	mpShownPage(pNewPage)
{
	if(mpShownPage)
	{
		mpShownPage->AddPageUser(*this);
	}
}

SdrPageObj::SdrPageObj(const Rectangle& rRect, SdrPage* pNewPage)
:	mpShownPage(pNewPage)
{
	if(mpShownPage)
	{
		mpShownPage->AddPageUser(*this);
	}

	aOutRect = rRect;
}

SdrPageObj::~SdrPageObj()
{
	// #111111#
	if(mpShownPage)
	{
		mpShownPage->RemovePageUser(*this);
	}
}

// #111111#
SdrPage* SdrPageObj::GetReferencedPage() const
{
	return mpShownPage;
}

// #111111#
void SdrPageObj::SetReferencedPage(SdrPage* pNewPage)
{
	if(mpShownPage != pNewPage)
	{
		if(mpShownPage)
		{
			mpShownPage->RemovePageUser(*this);
		}

		mpShownPage = pNewPage;

		if(mpShownPage)
		{
			mpShownPage->AddPageUser(*this);
		}

		SetChanged(); 
		BroadcastObjectChange(); 
	}
}

// #i96598#
void SdrPageObj::SetBoundRectDirty()
{
    // avoid resetting aOutRect which in case of this object is model data,
    // not re-creatable view data
}

sal_uInt16 SdrPageObj::GetObjIdentifier() const
{
	return sal_uInt16(OBJ_PAGE);
}

void SdrPageObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
	rInfo.bRotateFreeAllowed=sal_False;
	rInfo.bRotate90Allowed  =sal_False;
	rInfo.bMirrorFreeAllowed=sal_False;
	rInfo.bMirror45Allowed  =sal_False;
	rInfo.bMirror90Allowed  =sal_False;
	rInfo.bTransparenceAllowed = sal_False;
	rInfo.bGradientAllowed = sal_False;
	rInfo.bShearAllowed     =sal_False;
	rInfo.bEdgeRadiusAllowed=sal_False;
	rInfo.bNoOrthoDesired   =sal_False;
	rInfo.bCanConvToPath    =sal_False;
	rInfo.bCanConvToPoly    =sal_False;
	rInfo.bCanConvToPathLineToArea=sal_False;
	rInfo.bCanConvToPolyLineToArea=sal_False;
}

void SdrPageObj::operator=(const SdrObject& rObj)
{
	SdrObject::operator=(rObj);
	SetReferencedPage(((const SdrPageObj&)rObj).GetReferencedPage());
}

void SdrPageObj::TakeObjNameSingul(XubString& rName) const
{
	rName=ImpGetResStr(STR_ObjNameSingulPAGE);

	String aName( GetName() );
	if(aName.Len())
	{
		rName += sal_Unicode(' ');
		rName += sal_Unicode('\'');
		rName += aName;
		rName += sal_Unicode('\'');
	}
}

void SdrPageObj::TakeObjNamePlural(XubString& rName) const
{
	rName=ImpGetResStr(STR_ObjNamePluralPAGE);
}

// eof
