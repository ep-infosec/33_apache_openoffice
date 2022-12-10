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



#ifndef SC_TRANSOBJ_HXX
#define SC_TRANSOBJ_HXX

#include <svtools/transfer.hxx>
#include "global.hxx"
#include "address.hxx"

class ScDocShell;
class ScMarkData;
class SfxObjectShell;

namespace com { namespace sun { namespace star {
	namespace sheet {
		class XSheetCellRanges;
	}
}}}

#include <sfx2/objsh.hxx>

class ScTransferObj : public TransferableHelper
{
private:
	ScDocument*						pDoc;
	ScRange							aBlock;
	SCROW							nNonFiltered;		// non-filtered rows
	TransferableDataHelper			aOleData;
	TransferableObjectDescriptor	aObjDesc;
//REMOVE		SvEmbeddedObjectRef				aDocShellRef;
//REMOVE		SvEmbeddedObjectRef				aDrawPersistRef;
	SfxObjectShellRef				aDocShellRef;
	SfxObjectShellRef				aDrawPersistRef;
	com::sun::star::uno::Reference<com::sun::star::sheet::XSheetCellRanges> xDragSourceRanges;
	SCCOL							nDragHandleX;
	SCROW							nDragHandleY;
	SCTAB							nVisibleTab;
	sal_uInt16							nDragSourceFlags;
	sal_Bool							bDragWasInternal;
	sal_Bool							bUsedForLink;
    bool                            bHasFiltered;       // if has filtered rows
    bool                            bUseInApi;          // to recognize clipboard content copied from API

    // #123405# added parameter to allow size calculation without limitation
    // to PageSize, e.g. used for Metafile creation for clipboard.
	void		InitDocShell(bool bLimitToPageSize);
	static void	StripRefs( ScDocument* pDoc, SCCOL nStartX, SCROW nStartY,
							SCCOL nEndX, SCROW nEndY,
							ScDocument* pDestDoc=0,
							SCCOL nSubX=0, SCROW nSubY=0 );
	static void PaintToDev( OutputDevice* pDev, ScDocument* pDoc, double nPrintFactor,
							const ScRange& rBlock, sal_Bool bMetaFile );
	static void GetAreaSize( ScDocument* pDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol );

public:
			ScTransferObj( ScDocument* pClipDoc, const TransferableObjectDescriptor& rDesc );
	virtual ~ScTransferObj();

	virtual void		AddSupportedFormats();
	virtual sal_Bool	GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
	virtual sal_Bool	WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
										const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
	virtual void		ObjectReleased();
	virtual void		DragFinished( sal_Int8 nDropAction );

	ScDocument*			GetDocument()			{ return pDoc; }		// owned by ScTransferObj
	const ScRange&		GetRange() const		{ return aBlock; }
	SCROW				GetNonFilteredRows() const { return nNonFiltered; }
	SCCOL				GetDragHandleX() const	{ return nDragHandleX; }
	SCROW				GetDragHandleY() const	{ return nDragHandleY; }
	SCTAB				GetVisibleTab() const	{ return nVisibleTab; }
	sal_uInt16				GetDragSourceFlags() const	{ return nDragSourceFlags; }
    bool                HasFilteredRows() const { return bHasFiltered; }
    bool                GetUseInApi() const     { return bUseInApi; }
	ScDocShell*			GetSourceDocShell();
	ScDocument*			GetSourceDocument();
	ScMarkData			GetSourceMarkData();

	void				SetDrawPersist( const SfxObjectShellRef& rRef );
	void				SetDragHandlePos( SCCOL nX, SCROW nY );
	void				SetVisibleTab( SCTAB nNew );
	void				SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark );
	void				SetDragSourceFlags( sal_uInt16 nFlags );
	void				SetDragWasInternal();
    SC_DLLPUBLIC void   SetUseInApi( bool bSet );

	static SC_DLLPUBLIC ScTransferObj* GetOwnClipboard( Window* pUIWin );

	static SfxObjectShell*	SetDrawClipDoc( sal_Bool bAnyOle );		// update ScGlobal::pDrawClipDocShellRef
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException );
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
};

#endif

