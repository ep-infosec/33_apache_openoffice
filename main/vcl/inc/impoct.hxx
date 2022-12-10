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



#ifndef _SV_IMPOCT_HXX
#define _SV_IMPOCT_HXX

#include <vcl/octree.hxx>

// ----------------
// - ImpErrorQuad -
// ----------------

class ImpErrorQuad
{
	long					nRed;
	long					nGreen;
	long					nBlue;
	long					nReserved;
							
public:						
							
	inline 					ImpErrorQuad() {}
	inline					ImpErrorQuad( const BitmapColor& rColor ) :
								nRed	( (long) rColor.GetRed() << 5L ),
								nGreen	( (long) rColor.GetGreen() << 5L ),
								nBlue	( (long) rColor.GetBlue() << 5L ) {}
							
	inline void				operator=( const BitmapColor& rColor );
	inline ImpErrorQuad&	operator-=( const BitmapColor& rColor );
							
	inline void				ImplAddColorError1( const ImpErrorQuad& rErrQuad );
	inline void				ImplAddColorError3( const ImpErrorQuad& rErrQuad );
	inline void				ImplAddColorError5( const ImpErrorQuad& rErrQuad );
	inline void				ImplAddColorError7( const ImpErrorQuad& rErrQuad );
							
	inline BitmapColor		ImplGetColor();
};

// ------------------------------------------------------------------------

inline void ImpErrorQuad::operator=( const BitmapColor& rColor )
{
	nRed = (long) rColor.GetRed() << 5L;
	nGreen = (long) rColor.GetGreen() << 5L;
	nBlue = (long) rColor.GetBlue() << 5L;
}

// ------------------------------------------------------------------------

inline ImpErrorQuad& ImpErrorQuad::operator-=( const BitmapColor& rColor )
{
	nRed -= ( (long) rColor.GetRed() << 5L );
	nGreen -= ( (long) rColor.GetGreen() << 5L );
	nBlue -= ( (long) rColor.GetBlue() << 5L );

	return *this;
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError1( const ImpErrorQuad& rErrQuad )
{
	nRed += ( rErrQuad.nRed >> 4L );
	nGreen += ( rErrQuad.nGreen >> 4L );
	nBlue += ( rErrQuad.nBlue >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError3( const ImpErrorQuad& rErrQuad )
{
	nRed += ( rErrQuad.nRed * 3L >> 4L );
	nGreen += ( rErrQuad.nGreen * 3L >> 4L );
	nBlue += ( rErrQuad.nBlue * 3L >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError5( const ImpErrorQuad& rErrQuad )
{
	nRed += ( rErrQuad.nRed * 5L >> 4L );
	nGreen += ( rErrQuad.nGreen * 5L >> 4L );
	nBlue += ( rErrQuad.nBlue * 5L >> 4L );
}

// ------------------------------------------------------------------------

inline void ImpErrorQuad::ImplAddColorError7( const ImpErrorQuad& rErrQuad )
{
	nRed += ( rErrQuad.nRed * 7L >> 4L );
	nGreen += ( rErrQuad.nGreen * 7L >> 4L );
	nBlue += ( rErrQuad.nBlue *7L >> 4L );
}

// ------------------------------------------------------------------------

inline BitmapColor ImpErrorQuad::ImplGetColor()
{
	return BitmapColor( (sal_uInt8) ( ( nRed < 0L ? 0L : nRed > 8160L ? 8160L : nRed ) >> 5L ), 
						(sal_uInt8) ( ( nGreen < 0L ? 0L : nGreen > 8160L ? 8160L : nGreen ) >> 5L ), 
						(sal_uInt8) ( ( nBlue < 0L ? 0L : nBlue > 8160L ? 8160L : nBlue ) >> 5L ) );
}

// -------------
// - NodeCache -
// -------------

class ImpNodeCache
{
	OctreeNode*			pActNode;
	sal_uLong				nNew;
	sal_uLong				nDelete;
	sal_uLong				nGet;
	sal_uLong				nRelease;
						
public:					
						
						ImpNodeCache( const sal_uLong nInitSize );
						~ImpNodeCache();

	inline OctreeNode*	ImplGetFreeNode();
	inline void			ImplReleaseNode( OctreeNode* pNode );
};

// ------------------------------------------------------------------------

inline OctreeNode* ImpNodeCache::ImplGetFreeNode()
{
	OctreeNode* pNode;

	if ( !pActNode )
	{
		pActNode = new NODE;
		pActNode->pNextInCache = NULL;
	}

	pNode = pActNode;
	pActNode = pNode->pNextInCache;
	memset( pNode, 0, sizeof( NODE ) );

	return pNode;
}

// ------------------------------------------------------------------------

inline void ImpNodeCache::ImplReleaseNode( OctreeNode* pNode )
{
	pNode->pNextInCache = pActNode;
	pActNode = pNode;
}

#endif // _SV_IMPOCT_HXX
