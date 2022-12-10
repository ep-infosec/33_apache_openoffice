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



#ifndef _SVX_EXTRUSION_BAR_HXX
#define _SVX_EXTRUSION_BAR_HXX

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <svx/ifaceids.hxx>
#include "svx/svxdllapi.h"

class SfxViewShell;
class SdrView;

/************************************************************************/

namespace svx
{

SVX_DLLPUBLIC bool checkForSelectedCustomShapes( SdrView* pSdrView, bool bOnlyExtruded );

class SVX_DLLPUBLIC ExtrusionBar : public SfxShell
{
public:
	TYPEINFO();
	SFX_DECL_INTERFACE(SVX_INTERFACE_EXTRUSION_BAR)

	 ExtrusionBar(SfxViewShell* pViewShell );
	~ExtrusionBar();

	static void execute( SdrView* pSdrView, SfxRequest& rReq, SfxBindings& rBindings );
	static void getState( SdrView* pSdrView, SfxItemSet& rSet );
};

}

#endif			// _SVX_EXTRUSION_BAR_HXX
