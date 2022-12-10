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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
	class ImpSdr3DObjectAttribute;
    class MaterialAttribute3D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace attribute
	{
		class DRAWINGLAYER_DLLPUBLIC Sdr3DObjectAttribute
		{
		private:
			ImpSdr3DObjectAttribute*            mpSdr3DObjectAttribute;

		public:
			// constructors/destructor
			Sdr3DObjectAttribute(
				::com::sun::star::drawing::NormalsKind	aNormalsKind,
				::com::sun::star::drawing::TextureProjectionMode aTextureProjectionX,
				::com::sun::star::drawing::TextureProjectionMode aTextureProjectionY,
				::com::sun::star::drawing::TextureKind2 aTextureKind,
				::com::sun::star::drawing::TextureMode aTextureMode,
				const MaterialAttribute3D& rMaterial,
				bool bNormalsInvert,
				bool bDoubleSided,
				bool bShadow3D,
				bool bTextureFilter,
				bool bReducedLineGeometry);
			Sdr3DObjectAttribute();
			Sdr3DObjectAttribute(const Sdr3DObjectAttribute& rCandidate);
			~Sdr3DObjectAttribute();

			// assignment operator
			Sdr3DObjectAttribute& operator=(const Sdr3DObjectAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
			bool operator==(const Sdr3DObjectAttribute& rCandidate) const;

			// data read access
			::com::sun::star::drawing::NormalsKind getNormalsKind() const;
			::com::sun::star::drawing::TextureProjectionMode getTextureProjectionX() const;
			::com::sun::star::drawing::TextureProjectionMode getTextureProjectionY() const;
			::com::sun::star::drawing::TextureKind2 getTextureKind() const;
			::com::sun::star::drawing::TextureMode getTextureMode() const;
			const MaterialAttribute3D& getMaterial() const;
			bool getNormalsInvert() const;
			bool getDoubleSided() const;
			bool getShadow3D() const;
			bool getTextureFilter() const;
			bool getReducedLineGeometry() const;
		};
	} // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

// eof
