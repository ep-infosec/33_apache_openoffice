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

#ifndef SVX_SIDEBAR_SB_FONT_NAME_BOX_HXX
#define SVX_SIDEBAR_SB_FONT_NAME_BOX_HXX

#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>

/*
#include <sfx2/sectionpage.hxx>
#include <svx/svxdllapi.h>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>
#include <svl/lstner.hxx>
#include <vcl/toolbox.hxx>

#include <svx/tbxcolorupdate.hxx>
#include <svx/svxenum.hxx>
#include <svx/fhgtitem.hxx>

#define FONT_COLOR			1
#define BACK_COLOR			2
*/
namespace svx
{
    class ToolboxButtonColorUpdater;
}

namespace svx { namespace sidebar {

class SvxSBFontNameBox : public FontNameBox//, public SfxListener
{
private:
	const FontList*	pFontList;
	Font			aCurFont;
	String			aCurText;
	sal_uInt16 nFtCount;
   	bool bInput;
	void			EnableControls_Impl();
	SfxBindings*	pBindings;//
protected:
	virtual void 	Select();
	
public:
    SvxSBFontNameBox( Window* pParent, const ResId& rResId  );
	void			FillList();
	sal_uInt16 GetListCount() { return nFtCount; }
	void			Clear() { FontNameBox::Clear(); nFtCount = 0; }
	void			Fill( const FontList* pList )
						{ FontNameBox::Fill( pList );
						  nFtCount = pList->GetFontNameCount(); }
	void			SetBindings(SfxBindings* pBinding);//
	virtual long	PreNotify( NotifyEvent& rNEvt );
	virtual long	Notify(  NotifyEvent& rNEvt );//
};

} } // end of namespace svx::sidebar

#endif
