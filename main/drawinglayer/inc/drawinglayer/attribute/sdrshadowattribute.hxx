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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrShadowAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace attribute
	{
		class DRAWINGLAYER_DLLPUBLIC SdrShadowAttribute
		{
        private:
            ImpSdrShadowAttribute*              mpSdrShadowAttribute;

		public:
            /// constructors/assignmentoperator/destructor
			SdrShadowAttribute(
				const basegfx::B2DVector& rOffset, 
                double fTransparence, 
                const basegfx::BColor& rColor);
			SdrShadowAttribute();
			SdrShadowAttribute(const SdrShadowAttribute& rCandidate);
			SdrShadowAttribute& operator=(const SdrShadowAttribute& rCandidate);
			~SdrShadowAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
			bool operator==(const SdrShadowAttribute& rCandidate) const;

            // data access
			const basegfx::B2DVector& getOffset() const;
			double getTransparence() const;
			const basegfx::BColor& getColor() const;
		};
	} // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX

// eof
