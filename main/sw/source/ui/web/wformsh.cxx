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
#include "precompiled_sw.hxx"



#include <sfx2/msg.hxx>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svl/srchitem.hxx>
#include <sfx2/objface.hxx>
#include <tools/globname.hxx>
#include <sfx2/app.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "wformsh.hxx"
#include "globals.hrc"
#include "web.hrc"
#include "popup.hrc"
#include "shells.hrc"

#define SwWebDrawFormShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebDrawFormShell, SwDrawFormShell, SW_RES(0))
{
	SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRAWFORM_POPUPMENU));
	SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TEXT_TOOLBOX));
}

TYPEINIT1(SwWebDrawFormShell, SwDrawFormShell)

SwWebDrawFormShell::SwWebDrawFormShell(SwView& rVw) :
	SwDrawFormShell(rVw)
{
}

__EXPORT SwWebDrawFormShell::~SwWebDrawFormShell()
{
}


