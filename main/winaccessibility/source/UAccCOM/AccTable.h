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

#ifndef __ACCTABLE_H_
#define __ACCTABLE_H_

#include "resource.h"       // main symbols

#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include "UNOXWrapper.h"

/**
 * CAccTable implements IAccessibleTable interface.
 */
class ATL_NO_VTABLE CAccTable :
            public CComObjectRoot,
            public CComCoClass<CAccTable, &CLSID_AccTable>,
            public IAccessibleTable,
            public CUNOXWrapper

{
public:
    CAccTable()
    {
            }
    virtual ~CAccTable()
    {
            }

    BEGIN_COM_MAP(CAccTable)
    COM_INTERFACE_ENTRY(IAccessibleTable)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccTable*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_ACCTABLE)

    // IAccessibleTable
public:
    // IAccessibleTable

    // Gets accessible table cell.
    STDMETHOD(get_accessibleAt)(long row, long column, IUnknown * * accessible);

    // Gets accessible table caption.
    STDMETHOD(get_caption)(IUnknown * * accessible);

    // Gets accessible column description (as string).
    STDMETHOD(get_columnDescription)(long column, BSTR * description);

    // Gets number of columns spanned by table cell.
    STDMETHOD(get_columnExtentAt)(long row, long column, long * nColumnsSpanned);

    // Gets accessible column header.
    STDMETHOD(get_columnHeader)(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingRowIndex);

    // Gets total number of columns in table.
    STDMETHOD(get_nColumns)(long * columnCount);

    // Gets total number of rows in table.
    STDMETHOD(get_nRows)(long * rowCount);

    // Gets total number of selected columns.
    STDMETHOD(get_nSelectedColumns)(long * columnCount);

    // Gets total number of selected rows.
    STDMETHOD(get_nSelectedRows)(long * rowCount);

    // Gets accessible row description (as string).
    STDMETHOD(get_rowDescription)(long row, BSTR * description);

    // Gets number of rows spanned by a table cell.
    STDMETHOD(get_rowExtentAt)(long row, long column, long * nRowsSpanned);

    // Gets accessible row header.
    STDMETHOD(get_rowHeader)(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingColumnIndex);

    // Gets list of row indexes currently selected (0-based).
    STDMETHOD(get_selectedRows)(long maxRows, long **rows, long * nRows);

    // Gets list of column indexes currently selected (0-based).
    STDMETHOD(get_selectedColumns)(long maxColumns, long **columns, long * numColumns);

    // Gets accessible table summary.
    STDMETHOD(get_summary)(IUnknown * * accessible);

    // Determines if table column is selected.
    STDMETHOD(get_isColumnSelected)(long column, unsigned char * isSelected);

    // Determines if table row is selected.
    STDMETHOD(get_isRowSelected)(long row, unsigned char * isSelected);

    // Determines if table cell is selected.
    STDMETHOD(get_isSelected)(long row, long column, unsigned char * isSelected);

    // Selects a row and unselect all previously selected rows.
    STDMETHOD(selectRow)(long row );


    // Selects a column and unselect all previously selected columns.

    STDMETHOD(selectColumn)(long column);

    // Unselects one row, leaving other selected rows selected (if any).
    STDMETHOD(unselectRow)(long row);

    // Unselects one column, leaving other selected columns selected (if any).
    STDMETHOD(unselectColumn)(long column);

    //get Column index
    STDMETHOD(get_columnIndex)(long childIndex, long * columnIndex);

    STDMETHOD(get_rowIndex)(long childIndex, long * rowIndex);

    STDMETHOD(get_childIndex)(long rowIndex,long columnIndex, long * childIndex);

    STDMETHOD(get_nSelectedChildren)(long *childCount);

    STDMETHOD(get_selectedChildren)(long maxChildren, long **children, long *nChildren);

    STDMETHOD(get_rowColumnExtentsAtIndex)( long index,
                                            long  *row,
                                            long  *column,
                                            long  *rowExtents,
                                            long  *columnExtents,
                                            boolean  *isSelected) ;

    STDMETHOD(get_modelChange)(IA2TableModelChange  *modelChange);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(long pXInterface);

private:

    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessibleTable> pRXTable;

    inline com::sun::star::accessibility::XAccessibleTable* GetXInterface()
    {
        return pRXTable.get();
    }
};

#endif //__ACCTABLE_H_
