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



#ifndef _SDR_PROPERTIES_RECTANGLEPROPERTIES_HXX
#define _SDR_PROPERTIES_RECTANGLEPROPERTIES_HXX

#include <svx/sdr/properties/textproperties.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace properties
	{
		class SVX_DLLPUBLIC RectangleProperties : public TextProperties
		{
		protected:
			// react on ItemSet changes
			virtual void ItemSetChanged(const SfxItemSet& rSet);

		public:
			// basic constructor
			RectangleProperties(SdrObject& rObj);

			// constructor for copying, but using new object
			RectangleProperties(const RectangleProperties& rProps, SdrObject& rObj);

			// destructor
			virtual ~RectangleProperties();
			
			// Clone() operator, normally just calls the local copy constructor
			virtual BaseProperties& Clone(SdrObject& rObj) const;

			// set a new StyleSheet and broadcast
			virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);
		};
	} // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_RECTANGLEPROPERTIES_HXX

// eof
