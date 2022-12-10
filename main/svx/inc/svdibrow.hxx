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



#ifndef _SVDIBROW_HXX
#define _SVDIBROW_HXX

#include <svtools/brwbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/floatwin.hxx>

class SfxItemSet;
class ImpItemListRow;
class BrowserMouseEvent;

class _SdrItemBrowserControl: public BrowseBox 
{
friend class ImpItemEdit;
	Container aList;
	long nAktPaintRow;
	Edit* pEditControl;
	XubString aWNamMerk;
	Link aEntryChangedHdl;
	Link aSetDirtyHdl;
	ImpItemListRow* pAktChangeEntry;
	long   nLastWhichOfs;
	sal_uInt16 nLastWhich;
	sal_uInt16 nLastWhichOben;
	sal_uInt16 nLastWhichUnten;
	FASTBOOL bWhichesButNames;
	FASTBOOL bDontHideIneffectiveItems;
	FASTBOOL bDontSortItems;
	FASTBOOL bShowWhichIds;
	FASTBOOL bShowRealValues;
private:
#if _SOLAR__PRIVATE
	void ImpCtor();
	void ImpSetEntry(const ImpItemListRow& rEntry, sal_uIntPtr nEntryNum);
	ImpItemListRow* ImpGetEntry(sal_uIntPtr nPos) const { return (ImpItemListRow*)aList.GetObject(nPos); }
	void ImpSaveWhich();
	void ImpRestoreWhich();
#endif // __PRIVATE
protected:
	virtual long GetRowCount() const;
	virtual sal_Bool SeekRow(long nRow);
	virtual void PaintField(OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId) const;
	virtual void DoubleClick(const BrowserMouseEvent&);
	virtual void KeyInput(const KeyEvent& rEvt);
	virtual void Select();
	virtual void SetDirty(); // wird z.B. bei Modusumschaltungen gerufen
	virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
	virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
public:
	_SdrItemBrowserControl(Window* pParent, WinBits nBits=WB_3DLOOK|WB_BORDER|WB_TABSTOP);
	virtual ~_SdrItemBrowserControl();
	void Clear();
	void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet=NULL);
	sal_uIntPtr GetCurrentPos() const;
	sal_uInt16 GetCurrentWhich() const;
	virtual FASTBOOL BegChangeEntry(sal_uIntPtr nPos);
	virtual FASTBOOL EndChangeEntry();
	virtual void     BrkChangeEntry();
	
	/** GetCellText returns the text at the given position
		@param	_nRow
			the number of the row
		@param	_nColId
			the ID of the column
		@return
			the text out of the cell
	*/
	virtual String	GetCellText(long _nRow, sal_uInt16 _nColId) const;

	const ImpItemListRow* GetAktChangeEntry() const { return pAktChangeEntry; }
	XubString GetNewEntryValue() const                 { return pEditControl->GetText(); }
	void SetEntryChangedHdl(const Link& rLink)      { aEntryChangedHdl=rLink; }
	const Link& GetEntryChangedHdl() const          { return aEntryChangedHdl; }
	void SetSetDirtyHdl(const Link& rLink)          { aSetDirtyHdl=rLink; }
	const Link& GetSetDirtyHdl() const              { return aSetDirtyHdl; }
};

#define WB_STDSIZEABLEDOCKWIN  (WB_STDDOCKWIN|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEMOVE)
#define WB_STDSIZEABLEFLOATWIN (WB_STDFLOATWIN|WB_3DLOOK|WB_CLOSEABLE|WB_SIZEMOVE)

class _SdrItemBrowserWindow: public FloatingWindow {
	_SdrItemBrowserControl aBrowse;
public:
	_SdrItemBrowserWindow(Window* pParent, WinBits nBits=WB_STDSIZEABLEDOCKWIN);
	virtual ~_SdrItemBrowserWindow();
	virtual void Resize();
	virtual void GetFocus();
	void Clear()                                            { aBrowse.Clear(); }
	void SetAttributes(const SfxItemSet* pAttr, const SfxItemSet* p2ndSet=NULL) { aBrowse.SetAttributes(pAttr,p2ndSet); }
	void SetFloatingMode(FASTBOOL /*bOn*/) {}
	const _SdrItemBrowserControl& GetBrowserControl() const { return aBrowse; }
	_SdrItemBrowserControl& GetBrowserControl()             { return aBrowse; }
};

class SdrView;

class SdrItemBrowser: public _SdrItemBrowserWindow {
	Timer aIdleTimer;
	SdrView* pView;
	FASTBOOL bDirty;
private:
	static Window* ImpGetViewWin(SdrView& rView);
	DECL_LINK(IdleHdl,Timer*);
	DECL_LINK(ChangedHdl,_SdrItemBrowserControl*);
	DECL_LINK(SetDirtyHdl,_SdrItemBrowserControl*);
public:
	SdrItemBrowser(SdrView& rView);
	void ForceParent();
	void SetView(SdrView& rView) { pView=&rView; ForceParent(); SetDirty(); }
	void SetDirty();
	void Undirty();
	void ForceUndirty() { if (bDirty) Undirty(); }
};

#endif //_SVDIBROW_HXX


