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

#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive2d
	{
		ObjectInfoPrimitive2D::ObjectInfoPrimitive2D(
            const Primitive2DSequence& rChildren,
            const rtl::OUString& rName,
            const rtl::OUString& rTitle,
            const rtl::OUString& rDesc)
		:	GroupPrimitive2D(rChildren),
            maName(rName),
            maTitle(rTitle),
            maDesc(rDesc)
		{
		}

		bool ObjectInfoPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
		{
			if(GroupPrimitive2D::operator==(rPrimitive))
			{
				const ObjectInfoPrimitive2D& rCompare = (ObjectInfoPrimitive2D&)rPrimitive;

				return (getName() == rCompare.getName()
                    && getTitle() == rCompare.getTitle()
                    && getDesc() == rCompare.getDesc());
			}

			return false;
		}

        // provide unique ID
		ImplPrimitrive2DIDBlock(ObjectInfoPrimitive2D, PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
