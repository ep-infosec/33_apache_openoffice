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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive3d
	{
		GroupPrimitive3D::GroupPrimitive3D( const Primitive3DSequence& rChildren )
		:	BasePrimitive3D(),
			maChildren(rChildren)
		{
		}

		/** The compare opertator uses the Sequence::==operator, so only checking if
			the rererences are equal. All non-equal references are interpreted as
			non-equal.
		 */
		bool GroupPrimitive3D::operator==( const BasePrimitive3D& rPrimitive ) const
		{
			if(BasePrimitive3D::operator==(rPrimitive))
			{
				const GroupPrimitive3D& rCompare = static_cast< const GroupPrimitive3D& >(rPrimitive);

				return (arePrimitive3DSequencesEqual(getChildren(), rCompare.getChildren()));
			}

			return false;
		}

		/// default: just return children, so all renderers not supporting group will use it's content
		Primitive3DSequence GroupPrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
		{
			return getChildren();
		}

		// provide unique ID
		ImplPrimitrive3DIDBlock(GroupPrimitive3D, PRIMITIVE3D_ID_GROUPPRIMITIVE3D)

	} // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
