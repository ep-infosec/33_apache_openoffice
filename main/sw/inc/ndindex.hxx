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


#ifndef SW_NDINDEX_HXX
#define SW_NDINDEX_HXX

#include <limits.h>

#include <tools/solar.h>

#include <node.hxx>


class SwNode;
class SwNodes;

class SW_DLLPUBLIC SwNodeIndex
{
	friend void SwNodes::RegisterIndex( SwNodeIndex& );
	friend void SwNodes::DeRegisterIndex( SwNodeIndex& );
	friend void SwNodes::RemoveNode( sal_uLong, sal_uLong, sal_Bool );

#ifdef DBG_UTIL
	static int nSerial;
	int MySerial;
#endif

	SwNode* pNd;
	SwNodeIndex *pNext, *pPrev;

	void Remove();					// Ausketten

	// diese sind nicht erlaubt!
	SwNodeIndex( SwNodes& rNds, sal_uInt16 nIdx );
	SwNodeIndex( SwNodes& rNds, int nIdx );

public:
	SwNodeIndex( SwNodes& rNds, sal_uLong nIdx = 0 );
	SwNodeIndex( const SwNodeIndex &, long nDiff = 0 );
	SwNodeIndex( const SwNode&, long nDiff = 0 );
	~SwNodeIndex() { Remove(); }

	inline sal_uLong operator++();
	inline sal_uLong operator--();
#ifndef CFRONT
	inline sal_uLong operator++(int);
	inline sal_uLong operator--(int);
#endif

	inline sal_uLong operator+=( sal_uLong );
	inline sal_uLong operator-=( sal_uLong );
	inline sal_uLong operator+=( const  SwNodeIndex& );
	inline sal_uLong operator-=( const SwNodeIndex& );

	inline sal_Bool operator< ( const SwNodeIndex& ) const;
	inline sal_Bool operator<=( const SwNodeIndex& ) const;
	inline sal_Bool operator> ( const SwNodeIndex& ) const;
	inline sal_Bool operator>=( const SwNodeIndex& ) const;
	inline sal_Bool operator==( const SwNodeIndex& ) const;
	inline sal_Bool operator!=( const SwNodeIndex& ) const;

	inline sal_Bool operator< ( sal_uLong nWert ) const;
	inline sal_Bool operator<=( sal_uLong nWert ) const;
	inline sal_Bool operator> ( sal_uLong nWert ) const;
	inline sal_Bool operator>=( sal_uLong nWert ) const;
	inline sal_Bool operator==( sal_uLong nWert ) const;
	inline sal_Bool operator!=( sal_uLong nWert ) const;

	inline SwNodeIndex& operator=( sal_uLong );
		   SwNodeIndex& operator=( const SwNodeIndex& );
		   SwNodeIndex& operator=( const SwNode& );

	// gebe den Wert vom Index als sal_uLong zurueck
	inline sal_uLong GetIndex() const;

	// ermoeglicht Zuweisungen ohne Erzeugen eines temporaeren Objektes
	SwNodeIndex& Assign( SwNodes& rNds, sal_uLong );
	SwNodeIndex& Assign( const SwNode& rNd, long nOffset = 0 );

		// Herausgabe des Pointers auf das NodesArray,
	inline const SwNodes& GetNodes() const;
	inline	  	 SwNodes& GetNodes();

	SwNode& GetNode() const { return *pNd; }
};

/*
 * SwRange
 */
class SW_DLLPUBLIC SwNodeRange
{
public:
	SwNodeIndex aStart;
	SwNodeIndex aEnd;

	SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE );
	SwNodeRange( const SwNodeRange &rRange );

	SwNodeRange( SwNodes& rArr, sal_uLong nSttIdx = 0, sal_uLong nEndIdx = 0 );
	SwNodeRange( const SwNodeIndex& rS, long nSttDiff,
				 const SwNodeIndex& rE, long nEndDiff = 0 );
	SwNodeRange( const SwNode& rS, long nSttDiff,
				 const SwNode& rE, long nEndDiff = 0 );
};




// fuer die inlines wird aber der node.hxx benoetigt. Dieses braucht aber
// auch wieder dieses. Also alle Inlines, die auf pNd zugreifen werden
// hier implementiert.

inline sal_uLong SwNodeIndex::GetIndex() const
{
	return pNd->GetIndex();
}
inline const SwNodes& SwNodeIndex::GetNodes() const
{
	return pNd->GetNodes();
}
inline SwNodes& SwNodeIndex::GetNodes()
{
	return pNd->GetNodes();
}
inline sal_Bool SwNodeIndex::operator< ( sal_uLong nWert ) const
{
	return pNd->GetIndex() < nWert;
}
inline sal_Bool SwNodeIndex::operator<=( sal_uLong nWert ) const
{
	return pNd->GetIndex() <= nWert;
}
inline sal_Bool SwNodeIndex::operator> ( sal_uLong nWert ) const
{
	return pNd->GetIndex() > nWert;
}
inline sal_Bool SwNodeIndex::operator>=( sal_uLong nWert ) const
{
	return pNd->GetIndex() >= nWert;
}
inline sal_Bool SwNodeIndex::operator==( sal_uLong nWert ) const
{
	return pNd->GetIndex() == nWert;
}
inline sal_Bool SwNodeIndex::operator!=( sal_uLong nWert ) const
{
	return pNd->GetIndex() != nWert;
}
inline sal_Bool SwNodeIndex::operator<( const SwNodeIndex& rIndex ) const
{
	return pNd->GetIndex() < rIndex.GetIndex();
}
inline sal_Bool SwNodeIndex::operator<=( const SwNodeIndex& rIndex ) const
{
	return pNd->GetIndex() <= rIndex.GetIndex();
}
inline sal_Bool SwNodeIndex::operator>( const SwNodeIndex& rIndex ) const
{
	return pNd->GetIndex() > rIndex.GetIndex();
}
inline sal_Bool SwNodeIndex::operator>=( const SwNodeIndex& rIndex ) const
{
	return pNd->GetIndex() >= rIndex.GetIndex();
}
inline sal_Bool SwNodeIndex::operator==( const SwNodeIndex& rIdx ) const
{
	return pNd == rIdx.pNd;
}
inline sal_Bool SwNodeIndex::operator!=( const SwNodeIndex& rIdx ) const
{
	return pNd != rIdx.pNd;
}

inline sal_uLong SwNodeIndex::operator++()
{
	return ( pNd = GetNodes()[ pNd->GetIndex()+1 ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator--()
{
	return ( pNd = GetNodes()[ pNd->GetIndex()-1 ] )->GetIndex();
}
#ifndef CFRONT
inline sal_uLong SwNodeIndex::operator++(int)
{
	sal_uLong nOldIndex = pNd->GetIndex();
	pNd = GetNodes()[ nOldIndex + 1 ];
	return nOldIndex;
}
inline sal_uLong SwNodeIndex::operator--(int)
{
	sal_uLong nOldIndex = pNd->GetIndex();
	pNd = GetNodes()[ nOldIndex - 1 ];
	return nOldIndex;
}
#endif

inline sal_uLong SwNodeIndex::operator+=( sal_uLong nWert )
{
	return ( pNd = GetNodes()[ pNd->GetIndex() + nWert ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator-=( sal_uLong nWert )
{
	return ( pNd = GetNodes()[ pNd->GetIndex() - nWert ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator+=( const  SwNodeIndex& rIndex )
{
	return ( pNd = GetNodes()[ pNd->GetIndex() + rIndex.GetIndex() ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator-=( const SwNodeIndex& rIndex )
{
	return ( pNd = GetNodes()[ pNd->GetIndex() - rIndex.GetIndex() ] )->GetIndex();
}

inline SwNodeIndex& SwNodeIndex::operator=( sal_uLong nWert )
{
	pNd = GetNodes()[ nWert ];
	return *this;
}

#endif
