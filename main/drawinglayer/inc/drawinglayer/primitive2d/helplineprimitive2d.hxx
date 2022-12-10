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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// HelplinePrimitive2D class

namespace drawinglayer
{
	namespace primitive2d
	{
        /** HelplineStyle2D definition

            The available styles of Helplines
        */
		enum HelplineStyle2D
		{
			HELPLINESTYLE2D_POINT,
			HELPLINESTYLE2D_LINE
		};

        /** HelplinePrimitive2D class

            This primitive provides a view-dependent helpline definition. The Helpline
            is defined by a line equation (Point and vector) and a style. When the style 
            is a line, dependent from Viewport the visible part of that Helpline is 
            constructed. For Point, a cross is constructed. This primitive is highly 
            view-dependent.

            The visualisation uses the two given colors to create a dashed line with
            the given dash length.
         */
		class DRAWINGLAYER_DLLPUBLIC HelplinePrimitive2D : public BufferedDecompositionPrimitive2D
		{
		private:
            /// Helpline geometry definition
			basegfx::B2DPoint								maPosition;
			basegfx::B2DVector								maDirection;
			HelplineStyle2D									meStyle;

            /// Helpline style definition
			basegfx::BColor									maRGBColA;
			basegfx::BColor									maRGBColB;
			double											mfDiscreteDashLength;

			/** the last used object to view transformtion and the last Viewport, 
                used from getDecomposition for decide buffering
             */
			basegfx::B2DHomMatrix							maLastObjectToViewTransformation;
			basegfx::B2DRange								maLastViewport;

		protected:
			/// create local decomposition
			virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

		public:
            /// constructor
			HelplinePrimitive2D(
				const basegfx::B2DPoint& rPosition, 
				const basegfx::B2DVector& rDirection, 
				HelplineStyle2D eStyle, 
				const basegfx::BColor& rRGBColA, 
				const basegfx::BColor& aRGBColB, 
				double fDiscreteDashLength);

			/// data read access
			const basegfx::B2DPoint& getPosition() const { return maPosition; }
			const basegfx::B2DVector& getDirection() const { return maDirection; }
			HelplineStyle2D getStyle() const { return meStyle; }
			const basegfx::BColor& getRGBColA() const { return maRGBColA; }
			const basegfx::BColor& getRGBColB() const { return maRGBColB; }
			double getDiscreteDashLength() const { return mfDiscreteDashLength; }

			/// compare operator
			virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

			/// provide unique ID
			DeclPrimitrive2DIDBlock()

			/// Overload standard getDecomposition call to be view-dependent here
			virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
		};
	} // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
