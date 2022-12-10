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



#ifndef SC_ATTRDLG_HXX
#define SC_ATTRDLG_HXX

#include <sfx2/tabdlg.hxx>

class Window;
class SfxViewFrame;
class SfxItemSet;

#ifndef LAYOUT_SFX_TABDIALOG_BROKEN
#define LAYOUT_SFX_TABDIALOG_BROKEN 1
#endif /* !LAYOUT_SFX_TABDIALOG_BROKEN */

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>
#endif

//==================================================================

class ScAttrDlg : public SfxTabDialog
{
public:
				ScAttrDlg( SfxViewFrame*	 pFrame,
						   Window*			 pParent,
						   const SfxItemSet* pCellAttrs );
				~ScAttrDlg();

protected:
	virtual void	PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage );

private:
	DECL_LINK( OkHandler, void* ); // fuer DoppelClick-Beenden in TabPages
};

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-post.hxx>
#endif

#endif // SC_ATTRDLG_HXX


