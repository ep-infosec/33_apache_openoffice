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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <basegfx/vector/b2enums.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
	class BColor;
}

namespace drawinglayer { namespace attribute {
	class ImpSdrLineAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace attribute
	{
		class DRAWINGLAYER_DLLPUBLIC SdrLineAttribute
		{
        private:
            ImpSdrLineAttribute*                mpSdrLineAttribute;

		public:
            /// constructors/assignmentoperator/destructor
			SdrLineAttribute(
				basegfx::B2DLineJoin eJoin, 
                double fWidth, 
                double fTransparence, 
                const basegfx::BColor& rColor,
                com::sun::star::drawing::LineCap eCap,
				const ::std::vector< double >& rDotDashArray, 
                double fFullDotDashLen);
            SdrLineAttribute(const basegfx::BColor& rColor);
			SdrLineAttribute();
			SdrLineAttribute(const SdrLineAttribute& rCandidate);
			SdrLineAttribute& operator=(const SdrLineAttribute& rCandidate);
			~SdrLineAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
			bool operator==(const SdrLineAttribute& rCandidate) const;

			// data access
			basegfx::B2DLineJoin getJoin() const;
			double getWidth() const;
			double getTransparence() const;
			const basegfx::BColor& getColor() const;
			const ::std::vector< double >& getDotDashArray() const;
			double getFullDotDashLen() const;
            com::sun::star::drawing::LineCap getCap() const;

			// bool access
			bool isDashed() const;
		};
	} // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX

// eof
