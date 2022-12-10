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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <tools/string.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive2d
	{
        /** ObjectInfoPrimitive2D class

            Info hierarchy helper class to hold contents like Name, Title and 
            Description which are valid for the child content, e.g. created for
            primitives based on DrawingLayer objects or SVG parts. It decomposes 
            to it's content, so all direct renderers may ignore it. May e.g.
            be used when re-creating graphical content from a sequence of primitives
         */
		class DRAWINGLAYER_DLLPUBLIC ObjectInfoPrimitive2D : public GroupPrimitive2D
		{
		private:
            rtl::OUString                           maName;
            rtl::OUString                           maTitle;
            rtl::OUString                           maDesc;

        public:
            /// constructor
			ObjectInfoPrimitive2D(
                const Primitive2DSequence& rChildren,
                const rtl::OUString& rName,
                const rtl::OUString& rTitle,
                const rtl::OUString& rDesc);

			/// data read access
			const rtl::OUString& getName() const { return maName; }
			const rtl::OUString& getTitle() const { return maTitle; }
			const rtl::OUString& getDesc() const { return maDesc; }

			/// compare operator
			virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
			DeclPrimitrive2DIDBlock()
		};
	} // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_INFOHIERARCHYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
