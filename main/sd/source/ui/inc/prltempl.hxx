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




#ifndef SD_PRLTEMPL_HXX
#define SD_PRLTEMPL_HXX

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#include <sfx2/tabdlg.hxx>
#include <svx/tabarea.hxx>

#include "prlayout.hxx" // fuer enum PresentationObjects


class SfxObjectShell;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;

/*************************************************************************
|*
|* Vorlagen-Tab-Dialog
|*
\************************************************************************/
class SdPresLayoutTemplateDlg : public SfxTabDialog
{
private:
	const SfxObjectShell*	mpDocShell;

	XColorListSharedPtr     maColorTab;
	XGradientListSharedPtr  maGradientList;
	XHatchListSharedPtr     maHatchingList;
	XBitmapListSharedPtr    maBitmapList;
	XDashListSharedPtr      maDashList;
	XLineEndListSharedPtr   maLineEndList;

	sal_uInt16				nPageType;
	sal_uInt16				nDlgType;
	sal_uInt16				nPos;

	ChangeType			nColorTableState;
	ChangeType			nBitmapListState;
	ChangeType			nGradientListState;
	ChangeType			nHatchingListState;
	ChangeType			nLineEndListState;
	ChangeType			nDashListState;

	PresentationObjects	ePO;

	virtual void		PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

	// fuers Maping mit dem neuen SvxNumBulletItem
	SfxItemSet aInputSet;
	SfxItemSet* pOutSet;
	const SfxItemSet* pOrgSet;

	sal_uInt16 GetOutlineLevel() const;

	using SfxTabDialog::GetOutputItemSet;

public:
	SdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
	~SdPresLayoutTemplateDlg();

	const SfxItemSet* GetOutputItemSet() const;
};


#endif // SD_PRLTEMPL_HXX

