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

#ifndef _SV_REGIONBAND_HXX
#define _SV_REGIONBAND_HXX

#include <vcl/regband.hxx>
#include <vcl/region.hxx>

#ifdef DBG_UTIL
const char* ImplDbgTestRegionBand(const void*);
#endif

//////////////////////////////////////////////////////////////////////////////

class RegionBand
{
private:
    friend const char* ImplDbgTestRegionBand(const void*);

	ImplRegionBand* 	        mpFirstBand;		// root of the list with y-bands
	ImplRegionBand* 	        mpLastCheckedBand;

    void implReset();

public:
    RegionBand();
    RegionBand(const RegionBand&);
    RegionBand& operator=(const RegionBand&);
    RegionBand(const Rectangle&);
    ~RegionBand();

    bool operator==( const RegionBand& rRegionBand ) const;
    bool operator!=( const RegionBand& rRegionBand ) const { return !(RegionBand::operator==( rRegionBand )); }
    
    void load(SvStream& rIStrm);
    void save(SvStream& rIStrm) const;

    bool isSingleRectangle() const;
    ImplRegionBand* ImplGetFirstRegionBand() const { return mpFirstBand; }
    void ImplAddMissingBands(const long nTop, const long nBottom);
    void InsertBand(ImplRegionBand* pPreviousBand, ImplRegionBand* pBandToInsert);
    void processPoints();
    void CreateBandRange(long nYTop, long nYBottom);
    bool InsertLine(const Point& rStartPt, const Point& rEndPt, long nLineId);
    bool InsertPoint(const Point &rPoint, long nLineID, bool bEndPoint, LineType eLineType);
    bool OptimizeBandList();
    void Move(long nHorzMove, long nVertMove);
    void Scale(double fScaleX, double fScaleY);
    void InsertBands(long nTop, long nBottom);
    bool InsertSingleBand(ImplRegionBand* pBand, long nYBandPosition);
    void Union(long nLeft, long nTop, long nRight, long nBottom);
    void Intersect(long nLeft, long nTop, long nRight, long nBottom);
    void Union(const RegionBand& rSource);
    void Exclude(long nLeft, long nTop, long nRight, long nBottom);
    void XOr(long nLeft, long nTop, long nRight, long nBottom);
    void Intersect(const RegionBand& rSource);
    bool Exclude(const RegionBand& rSource);
    void XOr(const RegionBand& rSource);
    Rectangle GetBoundRect() const;
    bool IsInside(const Point& rPoint) const;
    sal_uInt32 getRectangleCount() const; // only users are Region::Intersect and PSWriter::ImplBmp
    void GetRegionRectangles(RectangleVector& rTarget) const;
};

#endif	// _SV_REGIONBAND_HXX

//////////////////////////////////////////////////////////////////////////////
//eof
