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



#ifndef _SVX_SUBSETMAP_HXX
#define _SVX_SUBSETMAP_HXX

#include "svx/svxdllapi.h"

#include <svx/ucsubset.hrc>
#include <vcl/metric.hxx>

#include <list>

class FontCharMap;

// classes Subset & SubsetMap --------------------------------------------
// TODO: should be moved into Font Attributes territory,
// we let them mature here though because this is currently the only use

class Subset
{
public:
    Subset( sal_UCS4 nMin, sal_UCS4 nMax, const String& aName )
    : mnRangeMin(nMin), mnRangeMax(nMax), maRangeName(aName)
    {}

    Subset( sal_UCS4 nMin, sal_UCS4 nMax, int resId );

    sal_UCS4        GetRangeMin() const { return mnRangeMin;}
    sal_UCS4        GetRangeMax() const { return mnRangeMax;}
    const String    GetName()     const { return maRangeName;}

private:
    sal_UCS4        mnRangeMin;
    sal_UCS4        mnRangeMax;
    String          maRangeName;
};

typedef ::std::list<Subset> SubsetList;

class SVX_DLLPUBLIC SubsetMap : private Resource
{
public:
    SubsetMap( const FontCharMap* );

    const Subset*   GetSubsetByUnicode( sal_UCS4 ) const;
    const Subset*   GetNextSubset( bool bFirst ) const;

private:
    SubsetList      maSubsets;
    mutable SubsetList::const_iterator maSubsetIterator;

    SVX_DLLPRIVATE void            InitList();
    SVX_DLLPRIVATE void            ApplyCharMap( const FontCharMap* );
};

#endif
