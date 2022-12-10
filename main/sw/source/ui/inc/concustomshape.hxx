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



#ifndef _SW_CONCUSTOMSHAPE_HXX
#define _SW_CONCUSTOMSHAPE_HXX

#include <rtl/ustring.hxx>
#include "drawbase.hxx"

class SdrObject;
class SfxRequest;

/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class ConstCustomShape : public SwDrawBase
{

	rtl::OUString aCustomShape;

	void SetAttributes( SdrObject* pObj );

 public:

	ConstCustomShape( SwWrtShell* pSh, SwEditWin* pWin, SwView* pView, SfxRequest& rReq );

									   // Mouse- & Key-Events
	virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
	virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

	virtual void Activate(const sal_uInt16 nSlotId);	// Function aktivieren

    rtl::OUString GetShapeType() const;
    static rtl::OUString GetShapeTypeFromRequest( SfxRequest& rReq );

    virtual void CreateDefaultObject();

	// #i33136#
	virtual bool doConstructOrthogonal() const;
};



#endif		// _SW_CONRECT_HXX

