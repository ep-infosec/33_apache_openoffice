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



#ifndef SC_CHARTARR_HXX
#define SC_CHARTARR_HXX

// -----------------------------------------------------------------------

#include "collect.hxx"
#include "rangelst.hxx"
#include "chartpos.hxx"

class ScAddress;
class Table;
class ScDocument;


// ScMemChart is a stripped-down SchMemChart from old chart,
// used only to transport a rectangular data array for the UNO API,
// contains only column/row header text and data values.

class ScMemChart
{
    short           nRowCnt;
    short           nColCnt;
    double*         pData;
    String*         pColText;
    String*         pRowText;

    ScMemChart(const ScMemChart& rMemChart);      // not implemented

public:
    ScMemChart(short nCols, short nRows);
    ~ScMemChart();

    short GetColCount() const { return nColCnt;	}
    short GetRowCount() const {	return nRowCnt;	}
    const String& GetColText(short nCol) const { return pColText[nCol]; }
    const String& GetRowText(short nRow) const { return pRowText[nRow]; }
    double GetData(short nCol, short nRow) const { return pData[nCol * nRowCnt + nRow]; }
    void SetData(short nCol, short nRow, const double& rVal) { pData[nCol * nRowCnt + nRow] = rVal; }
    void SetColText(short nCol, const String& rText) { pColText[nCol] = rText; }
    void SetRowText(short nRow, const String& rText) { pRowText[nRow] = rText; }
};


class SC_DLLPUBLIC ScChartArray : public ScDataObject				// nur noch Parameter-Struct
{
	String		aName;
	ScDocument*	pDocument;
	ScChartPositioner aPositioner;
	sal_Bool		bValid;				// fuer Erzeugung aus SchMemChart

private:
    ScMemChart* CreateMemChartSingle();
    ScMemChart* CreateMemChartMulti();
public:
	ScChartArray( ScDocument* pDoc, SCTAB nTab,
					SCCOL nStartColP, SCROW nStartRowP,
					SCCOL nEndColP, SCROW nEndRowP,
					const String& rChartName );
	ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
					const String& rChartName );
	ScChartArray( const ScChartArray& rArr );

	virtual	~ScChartArray();
	virtual	ScDataObject* Clone() const;

	const ScRangeListRef&	GetRangeList() const { return aPositioner.GetRangeList(); }
	void	SetRangeList( const ScRangeListRef& rNew ) { aPositioner.SetRangeList(rNew); }
	void	SetRangeList( const ScRange& rNew ) { aPositioner.SetRangeList(rNew); }
    const   ScChartPositionMap* GetPositionMap() { return aPositioner.GetPositionMap(); }

	void	SetHeaders(sal_Bool bCol, sal_Bool bRow) { aPositioner.SetHeaders(bCol, bRow); }
	sal_Bool	HasColHeaders() const			 { return aPositioner.HasColHeaders(); }
	sal_Bool	HasRowHeaders() const			 { return aPositioner.HasRowHeaders(); }
	sal_Bool	IsValid() const					 { return bValid; }
	void	SetName(const String& rNew)		 { aName = rNew; }
	const String& GetName() const			 { return aName; }

	sal_Bool	operator==(const ScChartArray& rCmp) const;

    ScMemChart* CreateMemChart();
};

class ScChartCollection : public ScCollection
{
public:
	ScChartCollection() : ScCollection( 4,4 ) {}
	ScChartCollection( const ScChartCollection& rColl ):
			ScCollection( rColl ) {}

	virtual	ScDataObject*	Clone() const;
	ScChartArray*		operator[](sal_uInt16 nIndex) const
						{ return (ScChartArray*)At(nIndex); }

	sal_Bool	operator==(const ScChartCollection& rCmp) const;
};



#endif

